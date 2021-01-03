/**
 * @file vmaccess.h
 * @author Prof. Dr. Wolfgang Fohl, HAW Hamburg
 * @date 2010
 * @brief This module defines function to read from and write to
 * virtual memory.
 */

#ifndef VMACCESS_H
#define VMACCESS_H

/**
 *****************************************************************************************
 *  @brief      This function reads an integer value from virtual memory.
 *              If this functions access virtual memory for the first time, the 
 *              virtual memory will be setup and initialized.
 *
 *  @param      address The virtual memory address the integer value should be read from.
 * 
 *  @return     The int value read from virtual memory.
 ****************************************************************************************/
int vmem_read(int address);

/**
 *****************************************************************************************
 *  @brief      This function writes an integer value from virtual memory.
 *              If this functions access virtual memory for the first time, the 
 *              virtual memory will be setup and initialized.
 *
 *  @param      address The virtual memory address the integer value should be written to.
 *
 *  @param      data The integer value that should be written to virtual memory.
 * 
 *  @return     void
 ****************************************************************************************/
void vmem_write(int address, int data);


extern void vmem_close(void);

#endif
