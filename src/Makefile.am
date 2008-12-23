lib_LTLIBRARIES = libprolog.la

#libprolog_la_SOURCES = prolog.c
libprolog_la_SOURCES = prolog-lib.c \
                       prolog-shell.c prolog-trace.c prolog-loader.c \
                       prolog-predicate.c prolog-object.c prolog-utils.c
libprolog_la_LIBADD  =
libprolog_la_LDFLAGS = @PROLOG_LIBS@ @GLIB_LIBS@
#                       -version-info @LT_CURRENT@:@LT_REVISION@:@LT_AGE@
libprolog_la_CFLAGS  = -I$(top_builddir)/include \
                       @PROLOG_CFLAGS@ -DPROLOG_HOME=\"@PROLOG_HOME@\" \
		       @GLIB_CFLAGS@

INCLUDES = -I$(top_builddir)/include

MAINTAINERCLEANFILES = Makefile.in

clean-local:
	rm -f *~
