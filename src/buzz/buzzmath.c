#include "buzzmath.h"
#include "buzztype.h"
#include <stdio.h>
#include <math.h>
#include <sys/time.h>

/****************************************/
/****************************************/

/* RNG period parameters */
#define N          624
#define M          397
#define MATRIX_A   0x9908b0dfUL /* constant vector a */
#define UPPER_MASK 0x80000000UL /* most significant w-r bits */
#define LOWER_MASK 0x7fffffffUL /* least significant r bits */
#define INT_MAX    0xFFFFFFFFUL

/****************************************/
/****************************************/

/* Sets the seed of a Mersenne-Twister random number generator */
void mt_setseed(buzzvm_t vm,
                uint32_t seed) {
   vm->rngstate[0] = seed & 0xffffffffUL;
   for(vm->rngidx = 1; vm->rngidx < N; ++vm->rngidx) {
      vm->rngstate[vm->rngidx] =
         (1812433253UL * (vm->rngstate[vm->rngidx-1] ^ (vm->rngstate[vm->rngidx-1] >> 30)) + vm->rngidx);
      vm->rngstate[vm->rngidx] &= 0xffffffffUL;
   }
}

uint32_t mt_uniform32(buzzvm_t vm) {
   uint32_t y;
   static uint32_t mag01[2] = { 0x0UL, MATRIX_A };
   /* mag01[x] = x * MATRIX_A  for x=0,1 */
   if (vm->rngidx >= N) { /* generate N words at one time */
      int32_t kk;
      for (kk = 0; kk < N - M; ++kk) {
         y = (vm->rngstate[kk] & UPPER_MASK) | (vm->rngstate[kk+1] & LOWER_MASK);
         vm->rngstate[kk] = vm->rngstate[kk+M] ^ (y >> 1) ^ mag01[y & 0x1UL];
      }
      for (; kk < N - 1; ++kk) {
         y = (vm->rngstate[kk] & UPPER_MASK) | (vm->rngstate[kk+1] & LOWER_MASK);
         vm->rngstate[kk] = vm->rngstate[kk+(M-N)] ^ (y >> 1) ^ mag01[y & 0x1UL];
      }
      y = (vm->rngstate[N-1] & UPPER_MASK) | (vm->rngstate[0] & LOWER_MASK);
      vm->rngstate[N-1] = vm->rngstate[M-1] ^ (y >> 1) ^ mag01[y & 0x1UL];
      vm->rngidx = 0;
   }
   y = vm->rngstate[vm->rngidx++];
   /* Tempering */
   y ^= (y >> 11);
   y ^= (y << 7) & 0x9d2c5680UL;
   y ^= (y << 15) & 0xefc60000UL;
   y ^= (y >> 18);
   return y;
}

/****************************************/
/****************************************/

#define buzzmath_error(obj) {                         \
      buzzvm_seterror(vm,                             \
                      BUZZVM_ERROR_TYPE,              \
                      "expected %s or %s, got %s",    \
                      buzztype_desc[BUZZTYPE_FLOAT],  \
                      buzztype_desc[BUZZTYPE_INT],    \
                      buzztype_desc[obj->o.type]);    \
      return vm->state;                               \
   }


/****************************************/
/****************************************/

#define function_register(FNAME)                                       \
   buzzvm_dup(vm);                                                      \
   buzzvm_pushs(vm, buzzvm_string_register(vm, #FNAME, 1));             \
   buzzvm_pushcc(vm, buzzvm_function_register(vm, buzzmath_ ## FNAME)); \
   buzzvm_tput(vm);

#define rng_function_register(FNAME)                                    \
   buzzvm_dup(vm);                                                      \
   buzzvm_pushs(vm, buzzvm_string_register(vm, #FNAME, 1));             \
   buzzvm_pushcc(vm, buzzvm_function_register(vm, buzzmath_rng_ ## FNAME)); \
   buzzvm_tput(vm);

#define constant_register(FNAME, VALUE)                                 \
   buzzvm_dup(vm);                                                      \
   buzzvm_pushs(vm, buzzvm_string_register(vm, (FNAME), 1));            \
   buzzvm_pushf(vm, (VALUE));                                           \
   buzzvm_tput(vm);

int buzzmath_register(buzzvm_t vm) {
   /* Push math table symbol */
   buzzvm_pushs(vm, buzzvm_string_register(vm, "math", 1));
   /* Make "math" table */
   buzzvm_pusht(vm);
   /* Register methods */
   function_register(abs);
   function_register(log);
   function_register(log2);
   function_register(log10);
   function_register(exp);
   function_register(sqrt);
   function_register(sin);
   function_register(cos);
   function_register(tan);
   function_register(asin);
   function_register(acos);
   function_register(atan);
   function_register(min);
   function_register(max);
   /* Register constants */
   constant_register("pi", 3.14159265358979323846);
   /* Push "math.rng" table symbol */
   buzzvm_pushs(vm, buzzvm_string_register(vm, "rng", 1));
   /* Make math.rng table */
   buzzvm_pusht(vm);
   /* Register methods */
   rng_function_register(setseed);
   rng_function_register(uniform);
   rng_function_register(gaussian);
   rng_function_register(exponential);
   /* Register math.rng table */
   buzzvm_gstore(vm);
   /* Register math table */
   buzzvm_gstore(vm);
   /* Initialize random number generator */
   vm->rngstate = (int32_t*)malloc(N * sizeof(int32_t));
   vm->rngidx = N + 1;
   struct timeval tv;
   gettimeofday(&tv, NULL);
   mt_setseed(vm, tv.tv_sec);
   /* All done */
   return vm->state;
}

/****************************************/
/****************************************/

int buzzmath_abs(buzzvm_t vm) {
   buzzvm_lnum_assert(vm, 1);
   /* Get argument */
   buzzvm_lload(vm, 1);
   buzzobj_t o = buzzvm_stack_at(vm, 1);
   if(o->o.type == BUZZTYPE_FLOAT)    buzzvm_pushf(vm, fabsf(o->f.value));
   else if(o->o.type == BUZZTYPE_INT) buzzvm_pushi(vm, abs(o->i.value));
   else buzzmath_error(o);
   /* Return result */
   return buzzvm_ret1(vm);
}

/****************************************/
/****************************************/

int buzzmath_floor(buzzvm_t vm) {
   buzzvm_lnum_assert(vm, 1);
   /* Get argument */
   buzzvm_lload(vm, 1);
   buzzobj_t o = buzzvm_stack_at(vm, 1);
   if(o->o.type == BUZZTYPE_FLOAT)    buzzvm_pushi(vm, floor(o->f.value));
   else if(o->o.type == BUZZTYPE_INT)    buzzvm_pushi(vm, o->i.value);
   else buzzmath_error(o);
   /* Return result */
   return buzzvm_ret1(vm);
}

/****************************************/
/****************************************/

int buzzmath_log(buzzvm_t vm) {
   buzzvm_lnum_assert(vm, 1);
   /* Get argument */
   float arg;
   buzzvm_lload(vm, 1);
   buzzobj_t o = buzzvm_stack_at(vm, 1);
   if(o->o.type == BUZZTYPE_FLOAT)    arg = o->f.value;
   else if(o->o.type == BUZZTYPE_INT) arg = o->i.value;
   else buzzmath_error(o);
   /* Push result */
   buzzvm_pushf(vm, logf(arg));
   /* Return result */
   return buzzvm_ret1(vm);
}

/****************************************/
/****************************************/

int buzzmath_log2(buzzvm_t vm) {
   buzzvm_lnum_assert(vm, 1);
   /* Get argument */
   float arg;
   buzzvm_lload(vm, 1);
   buzzobj_t o = buzzvm_stack_at(vm, 1);
   if(o->o.type == BUZZTYPE_FLOAT)    arg = o->f.value;
   else if(o->o.type == BUZZTYPE_INT) arg = o->i.value;
   else buzzmath_error(o);
   /* Push result */
   buzzvm_pushf(vm, log2f(arg));
   /* Return result */
   return buzzvm_ret1(vm);
}

/****************************************/
/****************************************/

int buzzmath_log10(buzzvm_t vm) {
   buzzvm_lnum_assert(vm, 1);
   /* Get argument */
   float arg;
   buzzvm_lload(vm, 1);
   buzzobj_t o = buzzvm_stack_at(vm, 1);
   if(o->o.type == BUZZTYPE_FLOAT)    arg = o->f.value;
   else if(o->o.type == BUZZTYPE_INT) arg = o->i.value;
   else buzzmath_error(o);
   /* Push result */
   buzzvm_pushf(vm, log10f(arg));
   /* Return result */
   return buzzvm_ret1(vm);
}

/****************************************/
/****************************************/

int buzzmath_exp(buzzvm_t vm) {
   buzzvm_lnum_assert(vm, 1);
   /* Get argument */
   float arg;
   buzzvm_lload(vm, 1);
   buzzobj_t o = buzzvm_stack_at(vm, 1);
   if(o->o.type == BUZZTYPE_FLOAT)    arg = o->f.value;
   else if(o->o.type == BUZZTYPE_INT) arg = o->i.value;
   else buzzmath_error(o);
   /* Push result */
   buzzvm_pushf(vm, expf(arg));
   /* Return result */
   return buzzvm_ret1(vm);
}

/****************************************/
/****************************************/

int buzzmath_sqrt(buzzvm_t vm) {
   buzzvm_lnum_assert(vm, 1);
   /* Get argument */
   float arg;
   buzzvm_lload(vm, 1);
   buzzobj_t o = buzzvm_stack_at(vm, 1);
   if(o->o.type == BUZZTYPE_FLOAT)    arg = o->f.value;
   else if(o->o.type == BUZZTYPE_INT) arg = o->i.value;
   else buzzmath_error(o);
   /* Push result */
   buzzvm_pushf(vm, sqrtf(arg));
   /* Return result */
   return buzzvm_ret1(vm);
}

/****************************************/
/****************************************/

int buzzmath_sin(buzzvm_t vm) {
   buzzvm_lnum_assert(vm, 1);
   /* Get argument */
   float arg;
   buzzvm_lload(vm, 1);
   buzzobj_t o = buzzvm_stack_at(vm, 1);
   if(o->o.type == BUZZTYPE_FLOAT)    arg = o->f.value;
   else if(o->o.type == BUZZTYPE_INT) arg = o->i.value;
   else buzzmath_error(o);
   /* Push result */
   buzzvm_pushf(vm, sinf(arg));
   /* Return result */
   return buzzvm_ret1(vm);
}

/****************************************/
/****************************************/

int buzzmath_cos(buzzvm_t vm) {
   buzzvm_lnum_assert(vm, 1);
   /* Get argument */
   float arg;
   buzzvm_lload(vm, 1);
   buzzobj_t o = buzzvm_stack_at(vm, 1);
   if(o->o.type == BUZZTYPE_FLOAT)    arg = o->f.value;
   else if(o->o.type == BUZZTYPE_INT) arg = o->i.value;
   else buzzmath_error(o);
   /* Push result */
   buzzvm_pushf(vm, cosf(arg));
   /* Return result */
   return buzzvm_ret1(vm);
}

/****************************************/
/****************************************/

int buzzmath_tan(buzzvm_t vm) {
   buzzvm_lnum_assert(vm, 1);
   /* Get argument */
   float arg;
   buzzvm_lload(vm, 1);
   buzzobj_t o = buzzvm_stack_at(vm, 1);
   if(o->o.type == BUZZTYPE_FLOAT)    arg = o->f.value;
   else if(o->o.type == BUZZTYPE_INT) arg = o->i.value;
   else buzzmath_error(o);
   /* Push result */
   buzzvm_pushf(vm, tanf(arg));
   /* Return result */
   return buzzvm_ret1(vm);
}

/****************************************/
/****************************************/

int buzzmath_asin(buzzvm_t vm) {
   buzzvm_lnum_assert(vm, 1);
   /* Get argument */
   float arg;
   buzzvm_lload(vm, 1);
   buzzobj_t o = buzzvm_stack_at(vm, 1);
   if(o->o.type == BUZZTYPE_FLOAT)    arg = o->f.value;
   else if(o->o.type == BUZZTYPE_INT) arg = o->i.value;
   else buzzmath_error(o);
   /* Push result */
   buzzvm_pushf(vm, asinf(arg));
   /* Return result */
   return buzzvm_ret1(vm);
}

/****************************************/
/****************************************/

int buzzmath_acos(buzzvm_t vm) {
   buzzvm_lnum_assert(vm, 1);
   /* Get argument */
   float arg;
   buzzvm_lload(vm, 1);
   buzzobj_t o = buzzvm_stack_at(vm, 1);
   if(o->o.type == BUZZTYPE_FLOAT)    arg = o->f.value;
   else if(o->o.type == BUZZTYPE_INT) arg = o->i.value;
   else buzzmath_error(o);
   /* Push result */
   buzzvm_pushf(vm, acosf(arg));
   /* Return result */
   return buzzvm_ret1(vm);
}

/****************************************/
/****************************************/

int buzzmath_atan(buzzvm_t vm) {
   buzzvm_lnum_assert(vm, 2);
   /* Get first argument */
   float y;
   buzzvm_lload(vm, 1);
   buzzobj_t o = buzzvm_stack_at(vm, 1);
   if(o->o.type == BUZZTYPE_FLOAT)    y = o->f.value;
   else if(o->o.type == BUZZTYPE_INT) y = o->i.value;
   else {
      buzzvm_seterror(vm,
                      BUZZVM_ERROR_TYPE,
                      "expected %s or %s for argument 1, got %s",
                      buzztype_desc[BUZZTYPE_FLOAT],
                      buzztype_desc[BUZZTYPE_INT],
                      buzztype_desc[o->o.type]);
      return vm->state;
   }
   /* Get second argument */
   float x;
   buzzvm_lload(vm, 2);
   o = buzzvm_stack_at(vm, 1);
   if(o->o.type == BUZZTYPE_FLOAT)    x = o->f.value;
   else if(o->o.type == BUZZTYPE_INT) x = o->i.value;
   else {
      buzzvm_seterror(vm,
                      BUZZVM_ERROR_TYPE,
                      "expected %s or %s for argument 2, got %s",
                      buzztype_desc[BUZZTYPE_FLOAT],
                      buzztype_desc[BUZZTYPE_INT],
                      buzztype_desc[o->o.type]);
      return vm->state;
   }
   /* Push result */
   buzzvm_pushf(vm, atan2f(y, x));
   /* Return result */
   return buzzvm_ret1(vm);
}

/****************************************/
/****************************************/

int buzzmath_min(buzzvm_t vm) {
   buzzvm_lnum_assert(vm, 2);
   /* Get arguments */
   buzzvm_lload(vm, 1);
   buzzobj_t a = buzzvm_stack_at(vm, 1);
   buzzvm_lload(vm, 2);
   buzzobj_t b = buzzvm_stack_at(vm, 1);
   /* Compare them and return the smaller one */
   if(buzzobj_cmp(a, b) <= 0)
      buzzvm_push(vm, a);
   else
      buzzvm_push(vm, b);
   return buzzvm_ret1(vm);
}

/****************************************/
/****************************************/

int buzzmath_max(buzzvm_t vm) {
   buzzvm_lnum_assert(vm, 2);
   /* Get arguments */
   buzzvm_lload(vm, 1);
   buzzobj_t a = buzzvm_stack_at(vm, 1);
   buzzvm_lload(vm, 2);
   buzzobj_t b = buzzvm_stack_at(vm, 1);
   /* Compare them and return the bigger one */
   if(buzzobj_cmp(a, b) >= 0)
      buzzvm_push(vm, a);
   else
      buzzvm_push(vm, b);
   return buzzvm_ret1(vm);
}

/****************************************/
/****************************************/

int buzzmath_rng_setseed(buzzvm_t vm) {
   buzzvm_lnum_assert(vm, 1);
   /* Get argument */
   buzzvm_lload(vm, 1);
   buzzvm_type_assert(vm, 1, BUZZTYPE_INT);
   /* Set the random seed */
   mt_setseed(vm, buzzvm_stack_at(vm, 1)->i.value);
   return buzzvm_ret0(vm);
}

/****************************************/
/****************************************/

int buzzmath_rng_uniform(buzzvm_t vm) {
   /*
    * - No arguments: return value in [-int_max,int_max]
    * - One argument A:
    *   - A is BUZZTYPE_INT: return value in [0, A]
    *   - A is BUZZTYPE_FLOAT: return value in [0.0, A]
    *   - Otherwise: error
    * - Two arguments (A,B):
    *   - Both A and B are BUZZTYPE_INT: return value in [A, B] as BUZZTYPE_INT
    *   - A is BUZZTYPE_INT and B is BUZZTYPE_FLOAT: return value in [A.0, B] as BUZZTYPE_FLOAT
    *   - A is BUZZTYPE_FLOAT and B is BUZZTYPE_INT: return value in [A, B.0] as BUZZTYPE_FLOAT
    *   - Both A and B are BUZZTYPE_FLOAT: return value in [A, B] as BUZZTYPE_FLOAT
    *   - Otherwise: error
    * - Otherwise: error
    */
   /* Parse arguments */
   if(buzzvm_lnum(vm) == 0) {
      /* No arguments */
      buzzvm_pushi(vm, mt_uniform32(vm));
   }
   else if(buzzvm_lnum(vm) == 1) {
      /* One argument */
      buzzvm_lload(vm, 1);
      buzzobj_t max = buzzvm_stack_at(vm, 1);
      if(max->o.type == BUZZTYPE_INT) {
         /* Integer value */
         buzzvm_pushi(vm,
                      (uint64_t)mt_uniform32(vm) * max->i.value / INT_MAX);
      }
      else if(max->o.type == BUZZTYPE_FLOAT) {
         /* Float value */
         buzzvm_pushf(vm,
                      mt_uniform32(vm) * max->f.value / INT_MAX);
      }
      else buzzmath_error(max);
   }
   else if(buzzvm_lnum(vm) == 2) {
      /* Two arguments */
      buzzvm_lload(vm, 1); /* min */
      buzzvm_lload(vm, 2); /* max */
      buzzobj_t min = buzzvm_stack_at(vm, 2);
      buzzobj_t max = buzzvm_stack_at(vm, 1);
      if(min->o.type == BUZZTYPE_INT &&
         max->o.type == BUZZTYPE_INT) {
         /* Both integers */
         buzzvm_pushi(vm,
                      (uint64_t)mt_uniform32(vm) * (max->i.value - min->i.value) / INT_MAX + min->i.value);
      }
      else if(min->o.type == BUZZTYPE_FLOAT &&
              max->o.type == BUZZTYPE_INT) {
         /* Min is float, max is integer */
         buzzvm_pushf(vm,
                      (uint64_t)mt_uniform32(vm) * (max->i.value - min->f.value) / INT_MAX + min->f.value);
      }
      else if(min->o.type == BUZZTYPE_INT &&
              max->o.type == BUZZTYPE_FLOAT) {
         /* Min is integer, max is integer float */
         buzzvm_pushf(vm,
                      (uint64_t)mt_uniform32(vm) * (max->f.value - min->i.value) / INT_MAX + min->i.value);
      }
      else if(min->o.type == BUZZTYPE_FLOAT &&
              max->o.type == BUZZTYPE_FLOAT) {
         /* Both float */
         buzzvm_pushf(vm,
                      (uint64_t)mt_uniform32(vm) * (max->f.value - min->f.value) / INT_MAX + min->f.value);
      }
      else {
         /* Error */
         buzzvm_seterror(vm,
                         BUZZVM_ERROR_TYPE,
                         "expected %s or %s for arguments, got %s for argument 1 and %s for argument 2)",
                         buzztype_desc[BUZZTYPE_FLOAT],
                         buzztype_desc[BUZZTYPE_INT],
                         buzztype_desc[min->o.type],
                         buzztype_desc[max->o.type]);
         return vm->state;
      }
   }
   else {
      /* Error */
      buzzvm_seterror(vm,
                      BUZZVM_ERROR_LNUM,
                      "expected 0, 1, or 2 parameters, got %" PRId64,
                      buzzvm_lnum(vm));
      return vm->state;
   }
   /* All OK, return random number */
   return buzzvm_ret1(vm);
}

/****************************************/
/****************************************/

int buzzmath_rng_gaussian(buzzvm_t vm) {
   /*
    * - No arguments: 1 stddev, 0 mean
    * - 1 argument A: A stddev, 0 mean
    * - 2 arguments (A,B): A stddev, B mean
    */
   /* Parse arguments */
   float stddev = 1.0f;
   float mean = 0.0f;
   if(buzzvm_lnum(vm) > 0) {
      /* Take first argument */
      buzzvm_lload(vm, 1);
      buzzobj_t o = buzzvm_stack_at(vm, 1);
      if(o->o.type == BUZZTYPE_FLOAT)    stddev = o->f.value;
      else if(o->o.type == BUZZTYPE_INT) stddev = o->i.value;
      else {
         buzzvm_seterror(vm,
                         BUZZVM_ERROR_TYPE,
                         "expected %s or %s for argument 1, got %s",
                         buzztype_desc[BUZZTYPE_FLOAT],
                         buzztype_desc[BUZZTYPE_INT],
                         buzztype_desc[o->o.type]);
         return vm->state;
      }
      if(buzzvm_lnum(vm) == 2) {
         /* Take second argument */
         buzzvm_lload(vm, 1);
         o = buzzvm_stack_at(vm, 1);
         if(o->o.type == BUZZTYPE_FLOAT)    mean = o->f.value;
         else if(o->o.type == BUZZTYPE_INT) mean = o->i.value;
         else {
            buzzvm_seterror(vm,
                            BUZZVM_ERROR_TYPE,
                            "expected %s or %s for argument 2, got %s",
                            buzztype_desc[BUZZTYPE_FLOAT],
                            buzztype_desc[BUZZTYPE_INT],
                            buzztype_desc[o->o.type]);
            return vm->state;
         }
      }
      else {
         /* Error */
         buzzvm_seterror(vm,
                         BUZZVM_ERROR_LNUM,
                         "expected 0, 1, or 2 parameters, got %" PRId64,
                         buzzvm_lnum(vm));
         return vm->state;
      }
   }
   /* If we are here, stddev and mean have been parsed */
   /* This is the Box-Muller method in its cartesian variant, see
    * http://www.dspguru.com/dsp/howtos/how-to-generate-white-gaussian-noise
   */
   float n1, n2, sq;
   do {
      n1 = (float)mt_uniform32(vm) * 2.0f / INT_MAX - 1.0;
      n2 = (float)mt_uniform32(vm) * 2.0f / INT_MAX - 1.0;
      sq = n1 * n1 + n2 * n2;
   }
   while(sq >= 1.0f);
   buzzvm_pushf(vm,
                mean + stddev * n1 * sqrt(-2.0f * logf(sq) / sq));
   return buzzvm_ret1(vm);
}

/****************************************/
/****************************************/

int buzzmath_rng_exponential(buzzvm_t vm) {
   /* Get the mean */
   buzzvm_lnum_assert(vm, 1);
   buzzobj_t o = buzzvm_stack_at(vm, 1);
   float mean;
   if(o->o.type == BUZZTYPE_FLOAT)    mean = o->f.value;
   else if(o->o.type == BUZZTYPE_INT) mean = o->i.value;
   else buzzmath_error(o);
   /* Push result */
   buzzvm_pushf(vm, -logf((float)mt_uniform32(vm) / INT_MAX) * mean);
   return buzzvm_ret1(vm);
}

/****************************************/
/****************************************/
