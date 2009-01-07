lib_LTLIBRARIES = libprolog.la
EXTRA_DIST      = $(config_DATA)
configdir       = $(datadir)/libprolog
config_DATA     = libprolog.pl

INCLUDES = -I$(top_builddir)/include
DEFINES  = -DPROLOG_HOME=\"@PROLOG_HOME@\" \
           -DLIBPROLOG_HELPER=\"$(configdir)/$(config_DATA)\"

libprolog_la_CFLAGS  = $(INCLUDES) $(DEFINES) \
                       @PROLOG_CFLAGS@ \
		       @GLIB_CFLAGS@
libprolog_la_SOURCES = prolog-lib.c \
                       prolog-shell.c prolog-trace.c prolog-loader.c \
                       prolog-predicate.c prolog-object.c prolog-utils.c
libprolog_la_LDFLAGS = @PROLOG_LIBS@ @GLIB_LIBS@
#                       -version-info @LT_CURRENT@:@LT_REVISION@:@LT_AGE@
libprolog_la_LIBADD  =


MAINTAINERCLEANFILES = Makefile.in

clean-local:
	rm -f *~
