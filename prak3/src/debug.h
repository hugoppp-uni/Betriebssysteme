/**
 * @file debug.h
 * @author Franz Korf, HAW Hamburg 
 * @date Nov 2014
 * @brief This header file defines some macros for debug output.
 */

#ifndef DEBUG_H
#define DEBUG_H

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <signal.h>
#include <execinfo.h>
#include <string.h>

#ifdef DEBUG_MESSAGES
/**
 * Print debug message when DEBUG_MESSAGES is defined.
 */
#define PRINT_DEBUG(str) { \
        fprintf str; \
	fflush(stderr); \
}
#else
#define PRINT_DEBUG(str) { }
#endif /* DEBUG_MESSAGES */

#ifdef SIGTRAP
#define DEBUG_BREAK raise(SIGTRAP);
#else
#define DEBUG_BREAK
#endif

/**
 * Based on a condition this macro, break, generate an perror message and exit.
 */
#define TEST_AND_EXIT_ERRNO(cond, str) if (cond) { \
        fprintf(stderr, "[ERROR] [%s] %s \n\t\tat %s (%s:%d)\n", strerror(errno), str, __FUNCTION__, __FILE__, __LINE__); \
        DEBUG_BREAK;                               \
        exit(EXIT_FAILURE);                        \
}

/**
 * Based on a condition this macro prints an error message to stderr and exits.
 */
#define TEST_AND_EXIT(cond, str) if (cond) { \
        fprintf(stderr, "[ERROR] %s \n\t\tat %s %s:%d\n", str, __FUNCTION__, __FILE__, __LINE__); \
        exit(EXIT_FAILURE); \
}

#endif /* DEBUG_H */

