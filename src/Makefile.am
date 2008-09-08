lib_LTLIBRARIES = libprolog.la

libprolog_la_SOURCES = prolog.c
libprolog_la_LIBADD  =
libprolog_la_LDFLAGS = @PROLOG_LIBS@
libprolog_la_CFLAGS  = -I$(top_builddir)/include \
                       @PROLOG_CFLAGS@ -DPROLOG_HOME=\"@PROLOG_HOME@\"

INCLUDES = -I$(top_builddir)/include

MAINTAINERCLEANFILES = Makefile.in

clean-local:
	rm -f *~
