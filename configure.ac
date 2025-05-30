#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)

AC_INIT([prolog],
	m4_esyscmd([build-aux/git-version-gen --tags .tarball-version]),
	[multimedia at maemo.org])

AC_CONFIG_SRCDIR(ohm-plugin)
AC_CONFIG_MACRO_DIR([m4])

DOC_SUPPORT_INIT

AM_INIT_AUTOMAKE(AC_PACKAGE_NAME, AC_PACKAGE_VERSION)
AC_CONFIG_HEADER([config.h])

AC_SUBST(ACLOCAL_AMFLAGS, "-I m4")

m4_define(version_major, `echo $VERSION | cut -d. -f1 | cut -d- -f1 | cut -d+ -f1`)
m4_define(version_minor, `echo $VERSION | cut -d. -f2 | cut -d- -f1 | cut -d+ -f1`)
m4_define(version_patch, `echo $VERSION | cut -d. -f3 | cut -d- -f1 | cut -d+ -f1`)

AC_SUBST(VERSION)
AC_SUBST(VERSION_MAJOR, version_major)
AC_SUBST(VERSION_MINOR, version_minor)
AC_SUBST(VERSION_PATCH, version_patch)
AC_SUBST(VERSION_FULL, version_major.version_minor.version_patch)

# libtool API versioning
LIBPROLOG_VERSION_INFO="0:0:0"
AC_SUBST(LIBPROLOG_VERSION_INFO)

# Disable static libraries.
AC_DISABLE_STATIC

# Checks for programs we need.
AC_PROG_CC
AC_PROG_INSTALL
AM_PROG_CC_C_O
AM_PROG_LIBTOOL

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h stdlib.h string.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_SIZE_T
AC_HEADER_TIME

# If compiling with GCC, enable lost of warnings.
if test "$GCC" = "yes"; then
    CPPFLAGS="$CPPFLAGS -Wall -Wformat"
fi

# If compiling with GCC and requested, enable even more warnings.
AC_ARG_ENABLE(extra-warnings,
              [  --enable-extra-warnings enable extra compilation warnings],
              enable_extra_warnings=$enableval,enable_extra_warnings=no)
if test x$enable_extra_warnings = xyes ; then
    AC_MSG_CHECKING([for extra compiler warnings])
    if test "$GCC" = "yes"; then
        CPPFLAGS="$CPPFLAGS -Wextra"
        AC_MSG_RESULT(yes)
    else
        AC_MSG_RESULT([no (compiler not GCC)])
    fi
fi

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([gettimeofday memset putenv strdup strtoul])

# Check for Check (unit test framework).
PKG_CHECK_MODULES(CHECK, 
                  check >= 0.9.4,
                  [has_check="yes"], [has_check="no"])
AM_CONDITIONAL(HAVE_CHECK, test "x$has_check" = "xyes")
if test "x$has_check" = "xno"; then
  AC_MSG_WARN([Check framework not found, unit tests are DISABLED.])
fi

# Checks for glib and gobject.
PKG_CHECK_MODULES(GLIB, glib-2.0 gobject-2.0)
AC_SUBST(GLIB_CFLAGS)
AC_SUBST(GLIB_LIBS)

# Checks for gmodule.
PKG_CHECK_MODULES(GMODULE, gmodule-2.0)
AC_SUBST(GMODULE_CFLAGS)
AC_SUBST(GMODULE_LIBS)

# Checks for libprolog.
PKG_CHECK_MODULES(LIBPROLOG, libprolog >= 1.2.0)
AC_SUBST(LIBPROLOG_CFLAGS)
AC_SUBST(LIBPROLOG_LIBS)

# Checks for libohmplugin and libohmfact.
PKG_CHECK_MODULES(LIBOHMPLUGIN, libohmplugin)
AC_SUBST(LIBOHMPLUGIN_CFLAGS)
AC_SUBST(LIBOHMPLUGIN_LIBS)

PKG_CHECK_MODULES(LIBOHMFACT, libohmfact)
AC_SUBST(LIBOHMFACT_CFLAGS)
AC_SUBST(LIBOHMFACT_LIBS)

# For the OHM plugin, mimick the OHM build variables.
AC_SUBST(OHM_PLUGIN_CFLAGS, "$LIBOHMPLUGIN_CFLAGS $LIBOHMFACT_CFLAGS $GLIB_CFLAGS $GMODULE_CFLAGS")
AC_SUBST(OHM_PLUGIN_LIBS, "$LIBOHMPLUGIN_LIBS $LIBOHMFACT_LIBS $GLIB_LIBS $GMODULE_LIBS")

AC_SUBST(OHM_CONF_DIR, "\$(sysconfdir)/ohm")
AC_SUBST(OHM_PLUGIN_CONF_DIR, "\$(OHM_CONF_DIR)/plugins.d")
AC_SUBST(OHM_PLUGIN_DIR, "\$(libdir)/ohm")

# Check symbol visibility constraint support.
AC_CHECK_CCOPT_VISIBILITY
AC_SUBST(CCOPT_VISIBILITY_HIDDEN)

# Check for profiling support (using gprof).
AC_ARG_ENABLE(gprof,
              [  --enable-gprof       enable profiling with gprof],
              enable_gprof=$enableval,enable_gprof=no)
AC_MSG_CHECKING([for profiling (gprof) support])
if test x$enable_gprof = xyes ; then
    AC_MSG_RESULT(yes)
    CFLAGS="-pg $CFLAGS"
    LDFLAGS="-pg $LDFLAGS"
else
    AC_MSG_RESULT(no)
fi

PKG_CHECK_MODULES(PROLOG, swipl >= 7.0)
PROLOG_BASE="`pkg-config --variable=base swipl`"
PROLOG_STATICLIB=""

AC_SUBST(PROLOG_BASE)
AC_SUBST(PROLOG_CFLAGS)
AC_SUBST(PROLOG_LIBS)
AC_SUBST(PROLOG_STATICLIB)

# shave
SHAVE_INIT([build-aux], [enable])

# Generate these files.
AC_CONFIG_FILES([build-aux/shave
                 build-aux/shave-libtool
                 Makefile
		 extensions/Makefile
		 ohm-plugin/Makefile
		 doc/Makefile])
AC_OUTPUT


echo "Prolog configuration:"
echo "       BASE: $PROLOG_BASE"
echo "     CFLAGS: $PROLOG_CFLAGS"
echo "       LIBS: $PROLOG_LIBS"
echo "  STATICLIB: $PROLOG_STATICLIB"

