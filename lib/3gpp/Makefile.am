## Process this file with automake to produce Makefile.in

pkglib_LTLIBRARIES = lib3gpp.la

lib3gpp_la_SOURCES = \
	types.h types.c \
	$(NULL)

lib3gpp_la_DEPENDENCIES = \
    $(top_srcdir)/lib/core/src/libcore.la \
	$(NULL)

lib3gpp_la_LIBADD = \
    $(top_srcdir)/lib/core/src/libcore.la \
	$(NULL)

AM_LDFLAGS = \
	-version-info @LIBVERSION@ \
	$(NULL)

AM_CPPFLAGS = \
    -I$(top_srcdir)/lib/core/include \
	$(NULL)

AM_CFLAGS = \
	-Wall -Werror \
	$(NULL)

MAINTAINERCLEANFILES = Makefile.in 
MOSTLYCLEANFILES = *.stackdump
