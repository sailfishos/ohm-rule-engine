EXTRA_DIST = autogen.sh build-aux/git-version-gen
SUBDIRS    = extensions ohm-plugin

if PD_SUPPORT
SUBDIRS += doc
endif

MAINTAINERCLEANFILES = Makefile.in

# see build-aux/git-version-gen
BUILT_SOURCES = $(top_srcdir)/.version
$(top_srcdir)/.version:
	echo $(VERSION) > $@-t && mv $@-t $@

dist-hook:
	echo $(VERSION) > $(distdir)/.tarball-version


clean-local:
	rm -f *~
