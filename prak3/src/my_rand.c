/**
  * @file my_rand.c
  * @author Franz Korf, HAW Hamburg 
  * @date Apr. 2020
  * @brief UNIX basic random generator
  */

#include "my_rand.h"
#include <stdint.h>

#define A       1103515245
#define M       0x80000000
#define C       12345
#define X_0     0x28061961

static int32_t x_n  = X_0;

void my_srand(int32_t seed){
   x_n = seed;
}

int32_t my_rand(void){
   x_n = (A * x_n + C) % M;
   return x_n;
}

// EOF
