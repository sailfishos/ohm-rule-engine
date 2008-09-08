pkglib_LTLIBRARIES = fact.la
pkglibdir          = ${libdir}/prolog/extensions

fact_la_SOURCES = pl_fact.c
fact_la_CFLAGS  = @PROLOG_CFLAGS@ @GLIB_CFLAGS@ @LIBOHMFACT_CFLAGS@
fact_la_LDFLAGS = -module -avoid-version 			    \
                 @PROLOG_LIBS@ @LIBOHMFACT_LIBS@

fact_la_LIBADD  = @GLIB_LIBS@

INCLUDES = -I$(top_builddir)/include

MAINTAINERCLEANFILES = Makefile.in

clean-local:
	rm -f *~
