#pragma once

#include "defines.h"

#if defined(SE_DATA_FROM_CODE_COPY) || defined(SE_DATA_FROM_CODE_DIRECT)

   #include "str_pk0_ntt_8192_1053818881.h"
   #include "str_pk1_ntt_8192_1053818881.h"
   #include "str_pk0_ntt_8192_1054015489.h"
   #include "str_pk1_ntt_8192_1054015489.h"
   #include "str_pk0_ntt_8192_1054212097.h"
   #include "str_pk1_ntt_8192_1054212097.h"
   #include "str_pk0_ntt_8192_1055260673.h"
   #include "str_pk1_ntt_8192_1055260673.h"
   #include "str_pk0_ntt_8192_1056178177.h"
   #include "str_pk1_ntt_8192_1056178177.h"
   #include "str_pk0_ntt_8192_1056440321.h"
   #include "str_pk1_ntt_8192_1056440321.h"

ZZ* pk_prime_addr[6][2] = 
{
    {&(pk0_prime0[0]), &(pk1_prime0[0])},
    {&(pk0_prime1[0]), &(pk1_prime1[0])},
    {&(pk0_prime2[0]), &(pk1_prime2[0])},
    {&(pk0_prime3[0]), &(pk1_prime3[0])},
    {&(pk0_prime4[0]), &(pk1_prime4[0])},
    {&(pk0_prime5[0]), &(pk1_prime5[0])}
};
#endif
