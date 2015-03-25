#ifndef __TLAPISVP_H__
#define __TLAPISVP_H__

#include "TlApi/TlApiError.h"

/* Marshaled function parameters.
 * structs and union of marshaling parameters via tlApi_Svp.
 *
 * @note The structs can NEVER be packed !
 * @note The structs can NOT used via sizeof(..) !
 */
typedef struct {
    uint32_t commandId;
    uint32_t x; /* use it for a buffer length */
    uint32_t y; /* use it for buffer address (pointer) */
    uint32_t result;
} tlApiSvp_t, *tlApiSvp_ptr;

/** execute specified function.
 *
 * @param sid  session id
 * @param data Pointer to the tlApiSvp_t structure
 * @return TLAPI_OK if data has successfully been processed.
 *
 * note that [tlApiResult_t] is actually the same with [tciReturnCode_t] (both unsigned int)
 */
_TLAPI_EXTERN_C tlApiResult_t tlApiOperate(uint32_t sid, tlApiSvp_ptr data);

/** a single flag variable (32-bit) in the secured display driver.
 *  sets bit in the flag variable.
 *
 * @param bitToSet the bit need to be set
 */
_TLAPI_EXTERN_C void exDrmApi_setLinkFlag(uint32_t bitToSet);

/** a single flag variable (32-bit) in the secured display driver.
 *  clears specific bit in the flag variable.
 *
 * @param bitToSet the bit need to be cleared.
 */
_TLAPI_EXTERN_C void exDrmApi_clearLinkFlag(uint32_t bitToClear);

/** a single flag variable (32-bit) in the secured display driver.
 *  queries a single bit in the flag variable.
 *
 * @param bitToSet the bit need to be queried.
 * @return the value of the queried bit.
 */
_TLAPI_EXTERN_C uint32_t exDrmApi_testLinkFlag(uint32_t bitToTest);

/** dump OVL hw registers (to log output)
 *
 * @param sid session id
 */
_TLAPI_EXTERN_C void tlApiDumpOvlRegister(uint32_t sid);

/** dump display hw registers (to log output)
 *
 * @param sid session id
 */
_TLAPI_EXTERN_C void tlApiDumpDispRegister(uint32_t sid);

#endif // __TLAPISVP_H__

