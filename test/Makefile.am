INCLUDES = -I../include @PROLOG_CFLAGS@

noinst_PROGRAMS = prologtest facttest

prologtest_SOURCES = prologtest.c
prologtest_LDADD = ../src/libprolog.la @PROLOG_LDFLAGS@

facttest_SOURCES = facttest.c
facttest_CFLAGS  = @GLIB_CFLAGS@
facttest_LDADD   = ../src/libprolog.la        \
                   ../datalibs/librelation.la \
                   ../datalibs/libfactmap.la  \
                   ../datalibs/libfact.la     \
                   @PROLOG_LDFLAGS@ @GLIB_LIBS@

CLEANFILES =
