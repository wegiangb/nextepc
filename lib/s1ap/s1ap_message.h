#ifndef _S1AP_MESSAGE_H__
#define _S1AP_MESSAGE_H__

#include "core_pkbuf.h"
#include "s1ap_ies_defs.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Octets */
#define S1AP_CLEAR_DATA(__dATA) \
    do { \
        d_assert((__dATA), , "Null param"); \
        if ((__dATA)->buf) \
        { \
            core_free((__dATA)->buf); \
            (__dATA)->buf = NULL; \
            (__dATA)->size = 0; \
        } \
    } while(0)
#define S1AP_STORE_DATA(__dST, __sRC) \
    do { \
        d_assert((__sRC),, "Null param") \
        d_assert((__sRC)->buf,, "Null param") \
        d_assert((__dST),, "Null param") \
        S1AP_CLEAR_DATA(__dST); \
        (__dST)->size = (__sRC)->size; \
        (__dST)->buf = core_calloc(1, sizeof((__dST)->size)); \
        memcpy((__dST)->buf, (__sRC)->buf, (__dST)->size); \
    } while(0)

CORE_DECLARE(int) s1ap_decode_pdu(s1ap_message_t *message, pkbuf_t *pkbuf);
CORE_DECLARE(int) s1ap_encode_pdu(pkbuf_t **pkbuf, s1ap_message_t *message);
CORE_DECLARE(int) s1ap_free_pdu(s1ap_message_t *message);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

