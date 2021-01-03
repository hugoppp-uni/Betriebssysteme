 /**
  * @file pagefile.c
  * @author Franz Korf, HAW Hamburg 
  * @date Dec 2015
  * @brief This is the pagefile manager module. It
  * stores pages to the pagefile and fetches 
  * pages from the pagefile.
  * It is based on an implementation of Wolfgang Fohl, HAW Hamburg.
  *
  */

#include <errno.h>
#include <limits.h>
#include "debug.h"
#include "vmem.h"
#include "my_rand.h"
#include "pagefile.h"

#define MMANAGE_PFNAME "./pagefile.bin" //!< Pagefile name 
#define SEED_PF        070514           //!< Get reproducable pseudo-random numbers to init pagefile

static FILE *pagefile = NULL;           //!< Reference to pagefile

void init_pagefile(void) {
    int i;
    /* Always generate a new file. 
       Otherwise: Run into problem if sizes change */
    pagefile = fopen(MMANAGE_PFNAME, "w+");
    TEST_AND_EXIT_ERRNO(!pagefile, "Error creating pagefile with w+");

    my_srand(SEED_PF);

    for(i = 0; i < (VMEM_PAGESIZE * VMEM_NPAGES * sizeof(int)); i++) {
        unsigned char rndval = my_rand() % (UCHAR_MAX + 1);
        fwrite(&rndval, 1, 1, pagefile);
    }
}

void fetch_page_from_pagefile(int pageNo, int *frame_start) {
    // check page number pt_itx
    TEST_AND_EXIT(pageNo <  0,           (stderr, "find_page: pageNo out of range\n"));
    TEST_AND_EXIT(pageNo >= VMEM_NPAGES, (stderr, "find_page: pageNo out of range\n"));
    
    int offset = pageNo * sizeof(int) * VMEM_PAGESIZE;

    TEST_AND_EXIT_ERRNO(fseek(pagefile, offset, SEEK_SET) == -1, "Positioning in pagefile failed!");
    TEST_AND_EXIT_ERRNO(fread(frame_start, sizeof(int), VMEM_PAGESIZE, pagefile) != VMEM_PAGESIZE, "Error reading page from disk");
}

void store_page_to_pagefile(int pageNo, int *frame_start) {
    // check page number pt_itx
    TEST_AND_EXIT(pageNo <  0,           (stderr, "store_page: pageNo out of range\n"));
    TEST_AND_EXIT(pageNo >= VMEM_NPAGES, (stderr, "store_page: pageNo out of range\n"));


    int offset = pageNo * sizeof(int) * VMEM_PAGESIZE;

    TEST_AND_EXIT_ERRNO(fseek(pagefile, offset, SEEK_SET) == -1, "Positioning in pagefile failed! ");
    TEST_AND_EXIT_ERRNO(fwrite(frame_start, sizeof(int), VMEM_PAGESIZE, pagefile) != VMEM_PAGESIZE, "Error writing page to disk");
}


void cleanup_pagefile(void) {
    TEST_AND_EXIT_ERRNO(fclose(pagefile) == -1, "fclose in cleanup_pagefile failed! ")
}

// EOF
