/**
 * @file logger.c
 * @author Franz Korf, HAW Hamburg 
 * @date Dec 2015
 * @brief This modules implements the logger for pagefault events. 
 *        It is based on the logger function  of the reference 
 *        implementation of Wolfgang Fohl.
 */

#include "logger.h"
#include "debug.h"

static FILE *logfile = NULL;  //!< Reference to logfile

void open_logger(void) {
    /* Open logfile */
    logfile = fopen(MMANAGE_LOGFNAME, "w");
    TEST_AND_EXIT_ERRNO(!logfile, "Error creating logfile");
}

void close_logger(void) {
    fclose(logfile);
}

/* Do not change!  */
void logger(struct logevent le) {
    fprintf(logfile, "Page fault %10d, Global count %10d:\n"
            "Removed: %10d, Allocated: %10d, Frame: %10d\n",
            le.pf_count, le.g_count,
            le.replaced_page, le.req_pageno, le.alloc_frame);
    fflush(logfile);
}

// EOF
