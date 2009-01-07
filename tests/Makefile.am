if HAVE_CHECK
TESTS = check-libprolog
else
TESTS =
endif

noinst_PROGRAMS = check-libprolog

check_libprolog_SOURCES = check-libprolog.c \
			  check-libprolog-init.c \
			  check-libprolog-predicate.c
check_libprolog_CFLAGS  = -I$(top_builddir)/include \
			  @PROLOG_CFLAGS@ \
			  @CHECK_CFLAGS@
check_libprolog_LDADD   = ../src/libprolog.la \
			  @PROLOG_LIBS@ \
			  @CHECK_LIBS@

