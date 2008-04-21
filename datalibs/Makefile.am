lib_LTLIBRARIES = librelation.la libset.la libfact.la libfactmap.la

librelation_la_SOURCES = relation.c
librelation_la_LIBADD  = 
librelation_la_LDFLAGS =
librelation_la_CFLAGS  =

libset_la_SOURCES = set.c
libset_la_LIBADD  = 
libset_la_LDFLAGS =
libset_la_CFLAGS  =

libfactmap_la_SOURCES = factmap.c
libfactmap_la_LIBADD  = @GLIB_LIBS@
libfactmap_la_LDFLAGS =
libfactmap_la_CFLAGS  = @GLIB_CFLAGS@

# this is only temporarily here... it'll come from ohmd
libfact_la_SOURCES = ohm-fact.c
libfact_la_LIBADD  = @GLIB_LIBS@
libfact_la_LDFLAGS =
libfact_la_CFLAGS  = @GLIB_CFLAGS@

INCLUDES = -I$(top_builddir)/include

MAINTAINERCLEANFILES = Makefile.in

clean-local:
	rm -f *~
