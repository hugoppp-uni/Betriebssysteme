/********************************************************************
 * @file    syncdataexchange.c
 * @author  Franz Korf 
 *          HAW-Hamburg
 *          Berliner Tor  7
 *          D-20099 Hamburg
 * @version V0.5
 * @date    Apr 2018
 * @brief   Dieses C Modul implementiert eine einfache synchrone Kommunikation
 *          zwischen vmapp (client) und dem Memory Manager (server). 
 *
 ******************************************************************
 */

#include "syncdataexchange.h"
#include <sys/types.h>
#include <sys/ipc.h>
#include <fcntl.h> 
#include <sys/shm.h>
#include <semaphore.h>
#include "debug.h"

#define SHMKEY_SYNC_COM            "./src/syncdataexchange.h"          //!< First paremater for shared memory generation via ftok function
#define SHMPROCID_SYNC_COM         3112                                //!< Second paremater for shared memory generation via ftok function

#define NAMED_SEM_WAKEUP_MMANAGER  "BS_A3_mmanager" //!< Semaphore to inform memory manager about new task
#define NAMED_SEM_WAKEUP_VMAPP     "BS_A3_vmapp"    //!< Semaphore to inform vmapp that task has been


/*
 * Globale Variablen, daher nur eine Instanz des Moduls pro Programm
 */

static int shm_id = -1;                      //!< Id zum Zugriff auf das shared memory
static struct msg *sharedData = NULL;
static sem_t *wakeupMManager = SEM_FAILED;   //!< Named semaphores that informs memory manager about a new task
static sem_t *wakeupVmApp = SEM_FAILED;      //!< Named semaphores that informs vmapp that task has been finished
static bool nextOpWaitForMsg = true;         //!< For checking correct order of waitForMsg and reply (sendAck)
static int refNoForAck = -1;	             //!< waitForMsg stores refCounter of msg for sendAck

/**
 * @brief  Diese Funktion erzeugt die Ressourcen, die zum synchronnen Austausch
 *         der Daten benötigt werden.
 * @param  isServer Ist dieses Flag true, so wird die Kommunkation für den Server 
 *                  aufgesetzt. Ansonsten für den Client.
 */
static void setupSyncDataExchangeInternal(bool isServer) {
   // create shared memory for data to be exchanged
   PRINT_DEBUG((stderr,"setupSyncDataExchangeInternal: Attach to shared memory\n"));
   key_t shm_key = ftok(SHMKEY_SYNC_COM, SHMPROCID_SYNC_COM);
   TEST_AND_EXIT_ERRNO(shm_key == -1, "setupSyncDataExchangeInternal:ftok failed!");
   // Use IPC:CREAT flag for server only
   shm_id = shmget(shm_key, sizeof(struct msg), 0664 | ((isServer)?IPC_CREAT:0));
   if (shm_id == -1){
      fprintf(stderr, "Shared memory from old run might still exists\n");
      fprintf(stderr, "   Use ipcs -ma for checking shared memory ressources\n");
      fprintf(stderr, "   Use ipcrm for deleting shared memory ressources\n");
   }
   TEST_AND_EXIT_ERRNO(shm_id == -1, "setupSyncDataExchangeInternal:shmget failed!");
   PRINT_DEBUG((stderr, "setupSyncDataExchangeInternal: shmget successfuly allocated %lu bytes\n", sizeof(struct msg)));
   sharedData = (struct msg *) shmat(shm_id, NULL, 0);
   TEST_AND_EXIT_ERRNO(sharedData == (struct msg *) -1, "setupSyncDataExchangeInternal: Error attaching shared memory");
   PRINT_DEBUG((stderr, "setupSyncDataExchangeInternal: Shared memory successfuly attached\n"));

   // Server: Delete old instances of the semaphores
   if (isServer) {
      if (sem_unlink(NAMED_SEM_WAKEUP_MMANAGER)) {
         TEST_AND_EXIT_ERRNO(errno != ENOENT, "setupSyncDataExchangeInternal: Cannot unlink old instance of semaphore");
      }
      if (sem_unlink(NAMED_SEM_WAKEUP_VMAPP)) {
         TEST_AND_EXIT_ERRNO(errno != ENOENT, "setupSyncDataExchangeInternal: Cannot unlink old instance of semaphore");
      }
   }
   // create semaphore for sync access
   wakeupMManager = (isServer) ? sem_open(NAMED_SEM_WAKEUP_MMANAGER, O_CREAT | O_EXCL, 0644, 0)
                               : sem_open(NAMED_SEM_WAKEUP_MMANAGER, 0);
   TEST_AND_EXIT_ERRNO(wakeupMManager  == SEM_FAILED, "setupSyncDataExchangeInternal: Error in creating named semaphore");
   wakeupVmApp = (isServer) ? sem_open(NAMED_SEM_WAKEUP_VMAPP, O_CREAT | O_EXCL, 0644, 0)
                            : sem_open(NAMED_SEM_WAKEUP_VMAPP, 0);
   TEST_AND_EXIT_ERRNO(wakeupVmApp  == SEM_FAILED, "setupSyncDataExchangeInternal: Error creating named semaphore");
   PRINT_DEBUG((stderr, "setupSyncDataExchangeInternal: semaphores successfully created\n"));
}

void setupSyncDataExchange(void) {
   setupSyncDataExchangeInternal(true);
}

void destroySyncDataExchange(void) {
   // distory shared memory 
   TEST_AND_EXIT_ERRNO(-1 ==  shmctl(shm_id, IPC_RMID, NULL), "distroySyncDataExchange: shmctl failed"); // Mark vmem for deletion 
   TEST_AND_EXIT_ERRNO(-1 == shmdt(sharedData), "distroySyncDataExchange: shmdt failed"); // detach shared memory
   fprintf(stderr, "distroySyncDataExchange: Shared memory successfully detached\n");

   // distory semaphores
    TEST_AND_EXIT_ERRNO(sem_close(wakeupMManager) == -1, "distroySyncDataExchange: sem_close failed");
    TEST_AND_EXIT_ERRNO(sem_unlink(NAMED_SEM_WAKEUP_MMANAGER) == -1, "distroySyncDataExchange: sem_unlink failed");
    TEST_AND_EXIT_ERRNO(sem_close(wakeupVmApp) == -1, "distroySyncDataExchange: sem_close failed");
    TEST_AND_EXIT_ERRNO(sem_unlink(NAMED_SEM_WAKEUP_VMAPP) == -1, "distroySyncDataExchange: sem_unlink failed");
    PRINT_DEBUG((stderr, "distroySyncDataExchange: Semaphore successfully destroyed\n"));
}

void sendMsgToMmanager(struct msg msg){
    static int refNo = 0; //!< Number of current reference send to memory manager
    msg.ref = refNo; // Wird zur Ueberpruefung der Kommunikation hoch gezaehlt.
    // Beim ersten Aufruf erzeugt der Client die Datenstrukturen
    if ((shm_id == -1) && (sharedData == NULL) && (wakeupMManager == SEM_FAILED) && (wakeupVmApp == SEM_FAILED)) {
       // Erster Aufruf durch den Client
       TEST_AND_EXIT(((shm_id != -1) || (sharedData != NULL) || (wakeupMManager != SEM_FAILED) || (wakeupVmApp != SEM_FAILED)), 
                     (stderr, "sendMsgToMmanager:Internal error detected\n"));
       setupSyncDataExchangeInternal(false);
    } // end if erzeuge Kommunikationstrukturen
    // Uebertrage Daten an den Server
    *sharedData = msg;
    TEST_AND_EXIT_ERRNO(sem_post(wakeupMManager) == -1, "sendMsgToMmanager:sem_post failed!");
    // Warte auf Antwort vom Server
    TEST_AND_EXIT_ERRNO(sem_wait(wakeupVmApp) == -1, "sendMsgToMmanager:sem_post:sem_wait failed!");
    TEST_AND_EXIT((sharedData->ref != refNo), (stderr, "Application and memory manager asynchronous"));
    TEST_AND_EXIT(sharedData->cmd != CMD_ACK, (stderr, "Unexpected answer from memory manager"));
    refNo++;
    PRINT_DEBUG((stderr, "Receive Msg form mem manager (cmd = %d, val = %d, ref = %d)\n", sharedData->cmd, sharedData->value, sharedData->ref));
}

struct msg waitForMsg(void){
   // Ueberpruefe Reihenfolge waitForMsg und SendAck
   TEST_AND_EXIT((!nextOpWaitForMsg), (stderr, "waitForMsg:Internal error, waitForMsg call not expected\n"));
   nextOpWaitForMsg = false;
   // Teste Kommunikationsparameter
   TEST_AND_EXIT(((shm_id == -1) || (sharedData == NULL) || (wakeupMManager == SEM_FAILED) || (wakeupVmApp == SEM_FAILED)), 
                 (stderr, "waitForMsg:Internal error detected\n"));
   // Warte auf Auftrag
   TEST_AND_EXIT_ERRNO(sem_wait(wakeupMManager) == -1, "waitForMsg:sem_post:sem_wait failed!");
   TEST_AND_EXIT(sharedData->cmd == CMD_ACK, (stderr, "waitForMsg: Unexpected command from vmapp"));
   refNoForAck = sharedData->ref;
   return *sharedData;
}

void sendAck(void){
   // Ueberpruefe Reihenfolge waitForMsg und SendAck
   TEST_AND_EXIT((nextOpWaitForMsg), (stderr, "sendAck:Internal error, sendAck call not expected\n"));
   nextOpWaitForMsg = true;
   // Teste Kommunikationsparameter
   TEST_AND_EXIT(((shm_id == -1) || (sharedData == NULL) || (wakeupMManager == SEM_FAILED) || (wakeupVmApp == SEM_FAILED)), 
                 (stderr, "sendAck:Internal error detected\n"));
   sharedData->cmd = CMD_ACK;
   sharedData->value = 0;
   sharedData->ref = refNoForAck;
   TEST_AND_EXIT_ERRNO(sem_post(wakeupVmApp) == -1, "sendAck:sem_post failed!");
}

//EOF

