pkglib_LTLIBRARIES = libohm_prolog.la
pkglibdir          = @OHM_PLUGIN_DIR@

libohm_prolog_la_SOURCES = prolog.c
libohm_prolog_la_CFLAGS = @OHM_PLUGIN_CFLAGS@ @PROLOG_CFLAGS@ \
			  -DDEFAULT_PROLOG_DIR=\"${pkglibdir}/prolog\"
libohm_prolog_la_LIBADD = @OHM_PLUGIN_LIBS@ @PROLOG_LDFLAGS@ \
                          ../src/libprolog.la
libohm_prolog_la_LDFLAGS = -module -avoid-version

INCLUDES = -I$(top_builddir)/include

MAINTAINERCLEANFILES = Makefile.in

clean-local:
	rm -f *~
