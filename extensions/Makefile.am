pkglib_LTLIBRARIES = relation.la set.la
pkglibdir          = ${datadir}/prolog/extensions

relation_la_SOURCES = pl_relation.c
relation_la_CFLAGS  = @PROLOG_CFLAGS@
relation_la_LDFLAGS = -module -avoid-version 			         \
                      @PROLOG_LDFLAGS@ -L../datalibs/.libs -lrelation

relation_la_LIBADD  = 

set_la_SOURCES = pl_set.c
set_la_CFLAGS  = @PROLOG_CFLAGS@
set_la_LDFLAGS = -module -avoid-version 			    \
                 @PROLOG_LDFLAGS@ -L../datalibs/.libs -lset

set_la_LIBADD  = 

INCLUDES = -I$(top_builddir)/include

MAINTAINERCLEANFILES = Makefile.in

clean-local:
	rm -f *~
