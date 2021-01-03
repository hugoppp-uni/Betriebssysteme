/**
 * @file vmappl.h
 * @author Wolfgang Fohl, HAW Hamburg 
 * @brief  This module contains the application that will be executed on the simulated virtual memory.
 * QuickSort will sort in array of int values. This array is located in the simulated virtual memory.
 * @date 2013
 */

#ifndef VMAPPL_H
#define VMAPPL_H

#ifndef SEED
#define SEED   2806    //!< Default value for setup of random number generator to initialized the array to be sorted
#endif

#define LENGTH 550     //!< Length of array to be sorted
#define RNDMOD 1000    //!< Second argument of modulo operator to shrink random numbers

#define NDISPLAYCOLS 8 //!< Number of values printed in one line

#define INIT_TYPE_SEED 0   // init array with random numbers
#define INIT_TYPE_UP   1   // init array with increasing numbers
#define INIT_TYPE_DOWN 2   // init array with decreasing numbers

#define QUICK_SORT     10  // use quick sort 
#define BUBBLE_SORT    11  // use bubble  sort 

#endif
