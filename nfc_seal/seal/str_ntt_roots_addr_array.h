#pragma once

#include "defines.h"

#if defined(SE_DATA_FROM_CODE_COPY) || defined(SE_DATA_FROM_CODE_DIRECT)

#include <stdint.h>

#ifdef SE_NTT_REG
   #include "str_ntt_roots_8192_1053818881.h"
   #include "str_ntt_roots_8192_1054015489.h"
   #include "str_ntt_roots_8192_1054212097.h"
   #include "str_ntt_roots_8192_1055260673.h"
   #include "str_ntt_roots_8192_1056178177.h"
   #include "str_ntt_roots_8192_1056440321.h"
#elif defined(SE_NTT_FAST)
   #include "str_ntt_fast_roots_8192_1053818881.h"
   #include "str_ntt_fast_roots_8192_1054015489.h"
   #include "str_ntt_fast_roots_8192_1054212097.h"
   #include "str_ntt_fast_roots_8192_1055260673.h"
   #include "str_ntt_fast_roots_8192_1056178177.h"
   #include "str_ntt_fast_roots_8192_1056440321.h"
#endif

ZZ* ntt_roots_addr[6] =
{
  &(((ZZ*)(ntt_roots_save_prime0))[0]),
  &(((ZZ*)(ntt_roots_save_prime1))[0]),
  &(((ZZ*)(ntt_roots_save_prime2))[0]),
  &(((ZZ*)(ntt_roots_save_prime3))[0]),
  &(((ZZ*)(ntt_roots_save_prime4))[0]),
  &(((ZZ*)(ntt_roots_save_prime5))[0])
};

#endif
