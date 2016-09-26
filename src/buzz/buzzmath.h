#ifndef BUZZMATH_H
#define BUZZMATH_H

#include <buzz/buzzvm.h>

#ifdef __cplusplus
extern "C" {
#endif

   extern int buzzmath_register(buzzvm_t vm);

   extern int buzzmath_abs(buzzvm_t vm);

   extern int buzzmath_log(buzzvm_t vm);

   extern int buzzmath_log2(buzzvm_t vm);

   extern int buzzmath_log10(buzzvm_t vm);

   extern int buzzmath_exp(buzzvm_t vm);

   extern int buzzmath_sqrt(buzzvm_t vm);

   extern int buzzmath_sin(buzzvm_t vm);

   extern int buzzmath_cos(buzzvm_t vm);

   extern int buzzmath_tan(buzzvm_t vm);

   extern int buzzmath_asin(buzzvm_t vm);

   extern int buzzmath_acos(buzzvm_t vm);

   extern int buzzmath_atan(buzzvm_t vm);

   extern int buzzmath_min(buzzvm_t vm);

   extern int buzzmath_max(buzzvm_t vm);

   extern int buzzmath_rng_setseed(buzzvm_t vm);

   extern int buzzmath_rng_uniform(buzzvm_t vm);

   extern int buzzmath_rng_gaussian(buzzvm_t vm);

   extern int buzzmath_rng_exponential(buzzvm_t vm);

#ifdef __cplusplus
}
#endif


#endif
