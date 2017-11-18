#define TRACE_MODULE _socket

#include "core_debug.h"
#include "core_pkbuf.h"

#include "core_arch_network.h"

#define MAX_SOCK_POOL_SIZE          512

static int max_fd;
static list_t fd_list;
static fd_set read_fds;

index_declare(sock_pool, sock_t, MAX_SOCK_POOL_SIZE);

static status_t sononblock(int sd);
static status_t soblock(int sd);
static void set_fds(fd_set *fds);
static void fd_dispatch(fd_set *fds);

status_t sock_init(void)
{
    index_init(&sock_pool, MAX_SOCK_POOL_SIZE);

    max_fd = 0;
    list_init(&fd_list);
    memset(&read_fds, 0, sizeof(fd_set));

    return CORE_OK;
}
status_t sock_final(void)
{
    index_final(&sock_pool);

    return CORE_OK;
}

status_t sock_create(sock_id *new, int family, int type, int protocol)
{
    sock_t *sock = NULL;

    index_alloc(&sock_pool, &sock);
    d_assert(sock, return CORE_ENOMEM,);
    memset(sock, 0, sizeof(sock_t));

    sock->family = family;
    sock->fd = socket(sock->family, type, protocol);
    if (sock->fd < 0)
    {
        d_warn("socket create(%d:%d:%d) failed(%d:%s)",
                sock->family, type, protocol, errno, strerror(errno));
        return CORE_ERROR;
    }

    d_trace(1, "socket create(%d:%d:%d)\n", sock->family, type, protocol);

    *new = (sock_id)sock;

    return CORE_OK;
}

status_t sock_delete(sock_id id)
{
    sock_t *sock = (sock_t *)id;
    d_assert(id, return CORE_ERROR,);

    if (sock_is_registered(id))
        sock_unregister(id);
    if (sock->fd >= 0)
        close(sock->fd);
    sock->fd = -1;

    index_free(&sock_pool, sock);
    return CORE_OK;
}

status_t sock_bind(sock_id id, c_sockaddr_t *sa)
{
    sock_t *sock = (sock_t *)id;
    char buf[CORE_ADDRSTRLEN];

    d_assert(sock, return CORE_ERROR,);
    d_assert(sa, return CORE_ERROR,);

    if (bind(sock->fd, &sa->sa, sa->sa_len) != 0)
    {
        d_error("socket bind(%s:%d) failed(%d:%s)",
                CORE_NTOP(sa, buf), sa->sin.sin_port, errno, strerror(errno));
        return CORE_ERROR;
    }

    d_trace(1, "socket bind %s:%d\n", CORE_NTOP(sa, buf), sa->sin.sin_port);

    return CORE_OK;
}

status_t sock_connect(sock_id id, c_sockaddr_t *sa)
{
    sock_t *sock = (sock_t *)id;
    char buf[CORE_ADDRSTRLEN];

    d_assert(sock, return CORE_ERROR,);
    d_assert(sa, return CORE_ERROR,);

    if (connect(sock->fd, &sa->sa, sa->sa_len) != 0)
    {
        d_error("socket connect(%s:%d) failed(%d:%s)",
                CORE_NTOP(sa, buf), sa->sin.sin_port, errno, strerror(errno));
        return CORE_ERROR;
    }

    d_trace(1, "socket connect %s:%d\n", CORE_NTOP(sa, buf), sa->sin.sin_port);

    return CORE_OK;
}

status_t sock_listen(sock_id id)
{
    int rc;
    sock_t *sock = (sock_t *)id;
    d_assert(sock, return CORE_ERROR,);

    rc = listen(sock->fd, 5);
    if (rc < 0)
    {
        d_error("listen failed(%d:%s)", errno, strerror(errno));
        return CORE_ERROR;
    }

    return CORE_OK;
}

status_t sock_accept(sock_id *new, c_sockaddr_t *addr, sock_id id)
{
    sock_t *sock = (sock_t *)id;
    sock_t *new_sock = NULL;

    int new_fd = -1;
    socklen_t addrlen = sizeof(c_sockaddr_t);

    d_assert(id, return CORE_ERROR,);
    d_assert(addr, return CORE_ERROR,);

    new_fd = accept(sock->fd, &addr->sa, &addrlen);
    if (new_fd < 0)
    {
        d_error("accept failed(%d:%s)", errno, strerror(errno));
        return CORE_ERROR;
    }
    addr->sa_len = addrlen;

    index_alloc(&sock_pool, &new_sock);
    d_assert(new_sock, return CORE_ENOMEM,);
    memset(new_sock, 0, sizeof(sock_t));

    new_sock->family = sock->family;
    new_sock->fd = new_fd;

    *new = (sock_id)new_sock;

    return CORE_OK;
}

ssize_t core_send(sock_id id, const void *buf, size_t len, int flags)
{
    sock_t *sock = (sock_t *)id;
    ssize_t size;

    d_assert(id, return -1,);

    size = send(sock->fd, buf, len, flags);
    if (size < 0)
    {
        d_error("sock_send(len:%ld) failed(%d:%s)",
                len, errno, strerror(errno));
    }

    return size;
}

ssize_t core_sendto(sock_id id,
        const void *buf, size_t len, int flags, const c_sockaddr_t *to)
{
    sock_t *sock = (sock_t *)id;
    ssize_t size;

    d_assert(id, return -1,);
    d_assert(to, return -1,);

    size = sendto(sock->fd, buf, len, flags, &to->sa, to->sa_len);
    if (size < 0)
    {
        d_error("sock_sendto(len:%ld) failed(%d:%s)",
                len, errno, strerror(errno));
    }

    return size;
}

ssize_t core_recv(sock_id id, void *buf, size_t len, int flags)
{
    sock_t *sock = (sock_t *)id;
    ssize_t size;

    d_assert(id, return -1,);

    size = recv(sock->fd, buf, len, flags);
    if (size < 0)
    {
        d_error("sock_recvfrom(len:%ld) failed(%d:%s)",
                len, errno, strerror(errno));
    }

    return size;
}

ssize_t core_recvfrom(sock_id id,
        void *buf, size_t len, int flags, c_sockaddr_t *from)
{
    sock_t *sock = (sock_t *)id;
    ssize_t size;
    socklen_t addrlen = sizeof(c_sockaddr_t);

    d_assert(id, return -1,);
    d_assert(from, return -1,);

    size = recvfrom(sock->fd, buf, len, flags, &from->sa, &addrlen);
    if (size < 0)
    {
        d_error("sock_recvfrom(len:%ld) failed(%d:%s)",
                len, errno, strerror(errno));
    }
    from->sa_len = addrlen;

    return size;
}

status_t sock_register(sock_id id, sock_handler handler, void *data) 
{
    sock_t *sock = (sock_t *)id;

    d_assert(id, return CORE_ERROR,);

    if (sock_is_registered(id))
    {
        d_error("socket has already been registered");
        return CORE_ERROR;
    }

    if (sock_setsockopt(id, SOCK_O_NONBLOCK, 1) == CORE_ERROR)
    {
        d_error("cannot set socket to non-block");
        return CORE_ERROR;
    }

    if (sock->fd > max_fd)
    {
        max_fd = sock->fd;
    }
    sock->handler = handler;
    sock->data = data;

    list_append(&fd_list, sock);

    return CORE_OK;
}

status_t sock_unregister(sock_id id)
{
    d_assert(id, return CORE_ERROR,);

    list_remove(&fd_list, id);

    return CORE_OK;
}

int sock_is_registered(sock_id id)
{
    sock_t *sock = (sock_t *)id;
    sock_t *iter = NULL;

    d_assert(id, return CORE_ERROR,);
    for (iter = list_first(&fd_list); iter != NULL; iter = list_next(iter))
    {
        if (iter->index == sock->index)
        {
            return 1;
        }
    }

    return 0;
}

int sock_select_loop(c_time_t timeout)
{
    struct timeval tv;
    int rc;

    if (timeout > 0)
    {
        tv.tv_sec = time_sec(timeout);
        tv.tv_usec = time_usec(timeout);
    }

    set_fds(&read_fds);

    rc = select(max_fd + 1, &read_fds, NULL, NULL, timeout > 0 ? &tv : NULL);
    if (rc < 0)
    {
        if (errno != EINTR && errno != 0)
        {
            if (errno == EBADF)
            {
                d_error("[FIXME] socket should be closed here(%d:%s)", 
                        errno, strerror(errno));
            }
            else
            {
                d_error("select failed(%d:%s)", errno, strerror(errno));
            }
        }
        return rc;
    }

    /* Timeout */
    if (rc == 0)
    {
        return rc;
    }

    /* Dispatch Handler */
    fd_dispatch(&read_fds);

    return 0;
}

status_t sock_setsockopt(sock_id id, c_int32_t opt, c_int32_t on)
{
    sock_t *sock = (sock_t *)id;
    int one;
    status_t rv;

    d_assert(sock, return CORE_ERROR,);
    if (on)
        one = 1;
    else
        one = 0;

    switch(opt)
    {
        case SOCK_O_REUSEADDR:
            if (on != sock_is_option_set(sock, SOCK_O_REUSEADDR))
            {
                if (setsockopt(sock->fd, SOL_SOCKET, SO_REUSEADDR,
                            (void *)&one, sizeof(int)) == -1)
                {
                    return errno;
                }
                sock_set_option(sock, SOCK_O_REUSEADDR, on);
            }
            break;
        case SOCK_O_NONBLOCK:
            if (sock_is_option_set(sock, SOCK_O_NONBLOCK) != on)
            {
                if (on)
                {
                    if ((rv = sononblock(sock->fd)) != CORE_OK) 
                        return rv;
                }
                else
                {
                    if ((rv = soblock(sock->fd)) != CORE_OK)
                        return rv;
                }
                sock_set_option(sock, SOCK_O_NONBLOCK, on);
            }
            break;
        default:
            d_error("Not implemented(%d)", opt);
            return CORE_EINVAL;
    }

    return CORE_OK; 
}         

status_t core_getaddrinfo(c_sockaddr_t **sa, 
        int family, const char *hostname, c_uint16_t port, int flags)
{
    int rc;
    char service[NI_MAXSERV];
    struct addrinfo hints, *ai, *ai_list;
    c_sockaddr_t *prev_sa;
    char buf[CORE_ADDRSTRLEN];

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = family;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = flags;

    if (hostname == NULL)
    {
        hints.ai_flags |= AI_PASSIVE;
        hostname = family == AF_INET6 ? "::" : "0.0.0.0";
    }

    snprintf(service, sizeof(service), "%u", port);

    rc = getaddrinfo(hostname, service, &hints, &ai_list);
    if (rc != 0)
    {
        d_error("getaddrinfo(%s:%d) failed(%d:%s)",
                hostname, port, errno, strerror(errno));
        return CORE_ERROR;
    }

    prev_sa = NULL;
    ai = ai_list;
    while(ai)
    {
        c_sockaddr_t *new_sa;
        if (ai->ai_family != AF_INET && ai->ai_family != AF_INET6)
        {
            ai = ai->ai_next;
            continue;
        }

        new_sa = core_calloc(1, sizeof(c_sockaddr_t));
        memcpy(&new_sa->sa, ai->ai_addr, ai->ai_addrlen);
        new_sa->sa_len = ai->ai_addrlen;
        new_sa->sin.sin_port = htons(port);
        d_trace(3, "addr:%s, port:%d\n", CORE_NTOP(new_sa, buf), port);

        if (!prev_sa)
            *sa = new_sa;
        else
            prev_sa->next = new_sa;

        prev_sa = new_sa;
        ai = ai->ai_next;
    }
    freeaddrinfo(ai_list);

    if (prev_sa == NULL)
    {
        d_error("core_getaddrinfo(%d:%s:%d:%d) failed(%d:%s)",
                family, hostname, port, flags, errno, strerror(errno));
        return CORE_ERROR;
    }

    return CORE_OK;
}

status_t core_freeaddrinfo(c_sockaddr_t *sa)
{
    c_sockaddr_t *next_sa = NULL;
    while(sa)
    {
        next_sa = sa->next;
        core_free(sa);
        sa = next_sa;
    }

    return CORE_OK;
}

const char *core_ntop(c_sockaddr_t *sockaddr, char *buf, int buflen)
{
    int family;
    d_assert(buf, return NULL,);
    d_assert(sockaddr, return NULL,);

    family = sockaddr->sa.sa_family;
    switch(family)
    {
        case AF_INET:
            d_assert(buflen >= INET_ADDRSTRLEN, return NULL,);
            return inet_ntop(family,
                    &sockaddr->sin.sin_addr, buf, INET_ADDRSTRLEN);
        case AF_INET6:
            d_assert(buflen >= CORE_ADDRSTRLEN, return NULL,);
            return inet_ntop(family,
                    &sockaddr->sin6.sin6_addr, buf, INET6_ADDRSTRLEN);
        default:
            d_assert(0, return NULL,
                    "Unknown family(%d)", sockaddr->sa.sa_family);
    }
}

int sockaddr_is_equal(c_sockaddr_t *a, c_sockaddr_t *b)
{
    d_assert(a, return 0,);
    d_assert(b, return 0,);

    if (a->sa.sa_family != b->sa.sa_family)
        return 0;

    if (a->sa.sa_family == AF_INET && memcmp(
        &a->sin.sin_addr, &b->sin.sin_addr, sizeof(struct in_addr)) == 0)
        return 1;
    else if (a->sa.sa_family == AF_INET6 && memcmp(
        &a->sin6.sin6_addr, &b->sin6.sin6_addr, sizeof(struct in6_addr)) == 0)
        return 1;
    else
        d_assert(0, return 0, "Unknown family(%d)", a->sa.sa_family);

    return 0;
}

static status_t soblock(int sd)
{
/* BeOS uses setsockopt at present for non blocking... */
#ifndef BEOS
    int fd_flags;

    fd_flags = fcntl(sd, F_GETFL, 0);
#if defined(O_NONBLOCK)
    fd_flags &= ~O_NONBLOCK;
#elif defined(O_NDELAY)
    fd_flags &= ~O_NDELAY;
#elif defined(FNDELAY)
    fd_flags &= ~FNDELAY;
#else
#error Please teach CORE how to make sockets blocking on your platform.
#endif
    if (fcntl(sd, F_SETFL, fd_flags) == -1)
    {
        return errno;
    }
#else
    int on = 0;
    if (setsockopt(sd, SOL_SOCKET, SO_NONBLOCK, &on, sizeof(int)) < 0)
        return errno;
#endif /* BEOS */
    return CORE_OK;
}

static status_t sononblock(int sd)
{
#ifndef BEOS
    int fd_flags;

    fd_flags = fcntl(sd, F_GETFL, 0);
#if defined(O_NONBLOCK)
    fd_flags |= O_NONBLOCK;
#elif defined(O_NDELAY)
    fd_flags |= O_NDELAY;
#elif defined(FNDELAY)
    fd_flags |= FNDELAY;
#else
#error Please teach CORE how to make sockets non-blocking on your platform.
#endif
    if (fcntl(sd, F_SETFL, fd_flags) == -1)
    {
        return errno;
    }
#else
    int on = 1;
    if (setsockopt(sd, SOL_SOCKET, SO_NONBLOCK, &on, sizeof(int)) < 0)
        return errno;
#endif /* BEOS */
    return CORE_OK;
}

static void set_fds(fd_set *fds)
{
    sock_t *sock = NULL;

    FD_ZERO(fds);
    for (sock = list_first(&fd_list); sock != NULL; sock = list_next(sock))
    {
        FD_SET(sock->fd, fds);
    }
}

static void fd_dispatch(fd_set *fds)
{
    sock_t *sock = NULL;

    for (sock = list_first(&fd_list); sock != NULL; sock = list_next(sock))
    {
        if (FD_ISSET(sock->fd, fds))
        {
            if (sock->handler)
            {
                sock->handler((sock_id)sock, sock->data);
            }
        }
    }
}