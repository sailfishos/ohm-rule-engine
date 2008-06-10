pkglib_LTLIBRARIES = fact.la # relation.la set.la
pkglibdir          = ${datadir}/prolog/extensions

fact_la_SOURCES = pl_fact.c
fact_la_CFLAGS  = @PROLOG_CFLAGS@ @GLIB_CFLAGS@
fact_la_LDFLAGS = -module -avoid-version 			    \
                 @PROLOG_LDFLAGS@ -L../datalibs/.libs -lfact

fact_la_LIBADD  = @GLIB_LIBS@

#relation_la_SOURCES = pl_relation.c
#relation_la_CFLAGS  = @PROLOG_CFLAGS@
#relation_la_LDFLAGS = -module -avoid-version 			         \
#                      @PROLOG_LDFLAGS@ -L../datalibs/.libs -lrelation
#
#relation_la_LIBADD  = 
#
#set_la_SOURCES = pl_set.c
#set_la_CFLAGS  = @PROLOG_CFLAGS@
#set_la_LDFLAGS = -module -avoid-version 			    \
#                 @PROLOG_LDFLAGS@ -L../datalibs/.libs -lset
#
#set_la_LIBADD  = 

INCLUDES = -I$(top_builddir)/include

MAINTAINERCLEANFILES = Makefile.in

clean-local:
	rm -f *~
