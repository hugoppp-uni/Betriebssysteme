/**
 * @file pagefile.h
 * @author Franz Korf, HAW Hamburg 
 * @date Dec 2015
 * @brief Header file of module for input / output of memory file.
 */

#ifndef PAGEFILE_H
#define PAGEFILE_H

/**
 *****************************************************************************************
 *  @brief      This function creates and initializes a new pagefile.
 *
 *  @return     void 
 ****************************************************************************************/
void init_pagefile(void);

/**
 *****************************************************************************************
 *  @brief      This function fetches a page out of the pagefile and writes it into 
 *              virtual memory.
 *
 *  @param      pageNo Number of the page that should be fetched.
 * 
 *  @param      frame_start Starting address of frame that should store the page.
 *
 *  @return     void 
 ****************************************************************************************/
void fetch_page_from_pagefile(int pageNo, int *frame_start);

/**
 *****************************************************************************************
 *  @brief      This function writes a page to pagefile.
 *
 *  @param      pageNo Number of the page that should be written to pagefile.
 * 
 *  @param      frame_start Starting address of the frame that contains the page.
 *
 *  @return     void 
 ****************************************************************************************/
void store_page_to_pagefile(int pageNo, int *frame_start);

/**
 *****************************************************************************************
 *  @brief      This function cleans and closes page file module.
 *
 *  @return     void 
 ****************************************************************************************/
void cleanup_pagefile(void);

#endif
