SUBDIRS = src include extensions ohm-plugin tests

if PD_SUPPORT
SUBDIRS += doc
endif

pkgconfigdir   = ${libdir}/pkgconfig
pkgconfig_DATA = libprolog.pc

MAINTAINERCLEANFILES = Makefile.in

clean-local:
	rm -f *~
