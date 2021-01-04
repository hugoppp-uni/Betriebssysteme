/**
 * @file vmaccess.c
 * @author Prof. Dr. Wolfgang Fohl, HAW Hamburg
 * @date 2010
 * @brief The access functions to virtual memory.
 */

#include "vmaccess.h"
#include <sys/ipc.h>
#include <sys/shm.h>

#include "syncdataexchange.h"
#include "vmem.h"
#include "debug.h"
#include <semaphore.h>
#include <signal.h>
#include <sys/types.h>
#include <string.h>

/*
 * static variables
 */

static struct vmem_struct *vmem = NULL; //!< Reference to virtual memory

/**
 * The progression of time is simulated by the counter g_count, which is incremented by 
 * vmaccess on each memory access. The memory manager will be informed by a command, whenever 
 * a fixed period of time has passed. Hence the memory manager must be informed, whenever 
 * g_count % TIME_WINDOW == 0. 
 * Based on this information, memory manager will update aging information
 */

static int g_count = 0;    //!< global acces counter as quasi-timestamp - will be increment by each memory access
#define TIME_WINDOW   20

/**
 *****************************************************************************************
 *  @brief      This function setup the connection to virtual memory.
 *              The virtual memory has to be created by mmanage.c module.
 *
 *  @return     void
 ****************************************************************************************/
static void vmem_init(void) {
    /* Create System V shared memory */
    /* We are only using the shm, don't set the IPC_CREAT flag */
    int key = ftok(SHMKEY, SHMPROCID);
    TEST_AND_EXIT_ERRNO(key == -1, strerror(errno))
    int id = shmget(key, sizeof(struct vmem_struct), 0);
    TEST_AND_EXIT_ERRNO(id == -1, strerror(errno))

    /* attach shared memory to vmem */
    vmem = shmat(id, 0, 0);
    TEST_AND_EXIT_ERRNO(vmem == (void *) -1, strerror(errno))
}

//int get_physical_address(int address, int write_access) {
//    if (vmem == NULL) {
//        vmem_init();
//    }
//
//    int page_num = address / VMEM_PAGESIZE;
//    int offset = address % VMEM_PAGESIZE;
//
//    if (!vmem->pt[page_num].flags & PTF_PRESENT) {
//        struct msg msg = {.cmd = CMD_PAGEFAULT, .value = page_num};
//        sendMsgToMmanager(msg);
//    }
//
//    if (write_access)
//        vmem->pt[page_num].flags |= PTF_DIRTY;
//    vmem->pt[page_num].flags |= PTF_REF;
//
//    return vmem->pt[page_num].frame * VMEM_PAGESIZE + offset;
//}

/**
 *****************************************************************************************
 *  @brief      This function puts a page into memory (if required). Ref Bit of page table
 *              entry will be updated.
 *              If the time window handle by g_count has reached, the window window message
 *              will be send to the memory manager. 
 *              To keep conform with this log files, g_count must be increased before 
 *              the time window will be checked.
 *              vmem_read and vmem_write call this function.
 *
 *  @param      address The page that stores the contents of this address will be 
 *              put in (if required).
 * 
 *  @return     void
 ****************************************************************************************/
static void vmem_put_page_into_mem(int address) {
// TODO wtf is this supposed to be DONE
	//put page into memory(if required = if it is not already in memory)
	int page_num = address / VMEM_PAGESIZE; //get page number
	if (!(vmem->pt[page_num].flags & PTF_PRESENT)) {
	        struct msg msg = {.cmd = CMD_PAGEFAULT, .value = page_num, .g_count = g_count};
	        sendMsgToMmanager(msg);
	}
	//update Ref Bit of pt_entry
	vmem->pt[page_num].flags |= PTF_REF;
	//increment g_count by each memory access
	g_count++;
	//send message to memory manager, if time window handle by g_count has reached
	if((g_count % TIME_WINDOW) == 0){
		struct msg msg = {.cmd = CMD_TIME_INTER_VAL, .value = page_num, .g_count = g_count};
		sendMsgToMmanager(msg);
		//now memory manager will update aging information. See mmanage.c update_age_reset_ref()
	}
	//
}

int vmem_read(int address) {
	if (vmem == NULL) {
		vmem_init();
	}
	vmem_put_page_into_mem(address);
	int page_num = address / VMEM_PAGESIZE;
	int offset = address % VMEM_PAGESIZE;
	int phys_data = vmem->pt[page_num].frame * VMEM_PAGESIZE + offset;
	return vmem->mainMemory[phys_data];
}

void vmem_write(int address, int data) {
	if (vmem == NULL) {
			vmem_init();
	}
	vmem_put_page_into_mem(address);
	int page_num = address / VMEM_PAGESIZE;
	int offset = address % VMEM_PAGESIZE;
	vmem->pt[page_num].flags |= PTF_DIRTY;
	int phys_data = vmem->pt[page_num].frame * VMEM_PAGESIZE + offset;
    vmem->mainMemory[phys_data] = data;
}
// EOF
