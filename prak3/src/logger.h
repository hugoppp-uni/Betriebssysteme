/**
 * @file logger.h
 * @author Franz Korf, HAW Hamburg 
 * @date Dec 2015
 * @brief Header file of the logger module. It is based on the logger function 
 *        of the reference implementation of Wolfgang Fohl.
 */

#ifndef LOGGER_H
#define LOGGER_H

/** 
 * Event struct for logging 
 */
struct logevent {
    int req_pageno;    //!< requested page number
    int replaced_page; //!< replaced page number
    int alloc_frame;   //!< selected frame
    int pf_count;      //!< current number of page faults
    int g_count;       //!< gobal quasi time stamp
};

#define MMANAGE_LOGFNAME "./logfile.txt"  //!< logfile name 

/**
 *****************************************************************************************
 *  @brief      This function creates a new logfile
 *
 *  @return     void 
 ****************************************************************************************/
void open_logger(void);

/**
 *****************************************************************************************
 *  @brief      This function closes the current logfile
 *
 *  @return     void 
 ****************************************************************************************/
void close_logger(void);

/**
 *****************************************************************************************
 *  @brief      This function writes a log entity to the logfile.
 *
 *  @param      le This stucture describes the entity that should be logged.
 *
 *  @return     void 
 ****************************************************************************************/
void logger(struct logevent le);

#endif /* LOGGER_H */
