#ifndef __CORE_ARCH_NETWORK_H__
#define __CORE_ARCH_NETWORK_H__

#include "core_list.h"
#include "core_index.h"
#include "core_network.h"

#if HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif

#if HAVE_NETDB_H
#include <netdb.h>
#endif

#if HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct _sock_t {
    lnode_t node;
    index_t index;
    
    int family;
    int fd;

    sock_handler handler;
    void *data;

    c_int32_t options;
} sock_t;

#define sock_is_option_set(skt, option)  \
    (((skt)->options & (option)) == (option))

#define sock_set_option(skt, option, on) \
    do {                                 \
        if (on)                          \
            (skt)->options |= (option);         \
        else                             \
            (skt)->options &= ~(option);        \
    } while (0)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif