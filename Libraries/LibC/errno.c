#include "errno.h"
#include "math.h"

#ifdef __cplusplus
extern "C" {
#endif

const char* error_what(int e)
{
    e = abs(e);

    switch (e) {
    /* 0 */   case 0:   return DESC_E_UNKNOWN;
    /* 1 */   case 1:   return DESC_E_PERMISSION;
    /* 2 */   case 2:   return DESC_E_VSFENTRY;
    /* 3 */   /* DESC_E_UNKNOWN */
    /* 4 */   /* DESC_E_UNKNOWN */
    /* 5 */   /* DESC_E_UNKNOWN */
    /* 6 */   /* DESC_E_UNKNOWN */
    /* 7 */   case 7:   return DESC_E_OVERFLOW;
    /* 8 */   case 8:   return DESC_E_INVALIDEXEC;
    /* 9 */   case 9:   return DESC_E_INVALIDFD;
    /* 10 */  case 10:  return DESC_E_NOCHILD;
    /* 11 */  case 11:  return DESC_E_NOMEM;
    /* 12 */  case 12:  return DESC_E_ACCES;
    /* 13 */  /* DESC_E_UNKNOWN */
    /* 14 */  case 14:  return DESC_E_FAULT;
    /* 15 */  /* DESC_E_UNKNOWN */
    /* 16 */  /* DESC_E_UNKNOWN */
    /* 17 */  /* DESC_E_UNKNOWN */
    /* 18 */  /* DESC_E_UNKNOWN */
    /* 19 */  /* DESC_E_UNKNOWN */
    /* 20 */  /* DESC_E_UNKNOWN */
    /* 21 */  /* DESC_E_UNKNOWN */
    /* 22 */  case 22:  return DESC_E_INVAL;
    /* 23 */  /* DESC_E_UNKNOWN */
    /* 24 */  /* DESC_E_UNKNOWN */
    /* 25 */  /* DESC_E_UNKNOWN */
    /* 26 */  /* DESC_E_UNKNOWN */
    /* 27 */  /* DESC_E_UNKNOWN */
    /* 28 */  /* DESC_E_UNKNOWN */
    /* 29 */  /* DESC_E_UNKNOWN */
    /* 30 */  /* DESC_E_UNKNOWN */
    /* 31 */  /* DESC_E_UNKNOWN */
    /* 32 */  /* DESC_E_UNKNOWN */
    /* 33 */  /* DESC_E_UNKNOWN */
    /* 34 */  /* DESC_E_UNKNOWN */
    /* 35 */  /* DESC_E_UNKNOWN */
    default:
        return DESC_E_UNKNOWN;
    }
}

#ifdef __cplusplus
}
#endif
