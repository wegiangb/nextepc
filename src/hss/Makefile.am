## Process this file with automake to produce Makefile.in.

noinst_LTLIBRARIES = libhss.la

libhss_la_SOURCES = \
	milenage.h hss_kdf.h hss_context.h hss_fd_path.h \
	milenage.c hss_kdf.c hss_init.c hss_context.c hss_fd_path.c \
	$(NULL)

libhss_la_DEPENDENCIES = \
    $(top_srcdir)/lib/fd/libfd.la \
    $(top_srcdir)/src/app/libapp.la \
	$(NULL)

libhss_la_LIBADD = \
    $(top_srcdir)/lib/fd/libfd.la \
    $(top_srcdir)/src/app/libapp.la \
	$(NULL)

AM_CPPFLAGS = \
	-I$(top_srcdir)/lib/core/include \
    -I$(top_srcdir)/lib/3gpp \
    -I$(top_srcdir)/lib/fd/s6a \
    -I$(top_srcdir)/lib/fd \
	-I$(top_srcdir)/lib/@FREEDIAMETER_DIR@/include \
	-I$(top_srcdir)/src/app \
	@MONGOC_CFLAGS@ \
	$(NULL)

AM_CFLAGS = \
	-Wall -Werror \
	$(NULL)

MAINTAINERCLEANFILES = Makefile.in
MOSTLYCLEANFILES = core *.stackdump
