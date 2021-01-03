/**
  * @file my_rand.h
  * @author Franz Korf, HAW Hamburg 
  * @date Apr. 2020
  * @brief This is the implementation of the standard Unix random number generator as described in 
  *                  https://www.gnu.org/software/gsl/doc/html/rng.html
  *        It is implemented to support the same random number sequence on different systms.
  *        gnu.org describes the generator aus follows: The standard Unix random number generators rand, 
  *        random and rand48 are provided as part of GSL. Although these generators are widely available 
  *        individually often they aren’t all available on the same platform. This makes it difficult to 
  *        write portable code using them and so we have included the complete set of Unix generators in 
  *        GSL for convenience. Note that these generators don’t produce high-quality randomness and aren’t 
  *        suitable for work requiring accurate statistics. However, if you won’t be measuring statistical 
  *        quantities and just want to introduce some variation into your program then these generators are 
  *        quite acceptable.
  *
  */

#include <stdint.h>

/**
  * @brief Same interface like srand 
  */
extern void my_srand(int32_t seed);

/**
  * @brief Same interface like rand 
  */
extern int32_t my_rand(void);

// EOF

