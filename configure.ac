dnl Require autoconf version >= 2.60
AC_PREREQ(2.60)


dnl ############# Initialization

AC_INIT([k8055httpd], [0.1], [njh@aelius.com], k8055httpd)

AC_CONFIG_SRCDIR([src/k8055httpd.c])
AC_CONFIG_AUX_DIR(build-scripts)
AC_CONFIG_MACRO_DIR(build-scripts)
AM_INIT_AUTOMAKE([1.10])

LT_PREREQ([2.2])
LT_INIT


dnl ############# Compiler and tools Checks

AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_LIBTOOL

AC_C_BIGENDIAN
AC_C_INLINE
AC_C_CONST


dnl ############## Header and function checks

AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h])
AC_FUNC_MALLOC
AC_FUNC_REALLOC


dnl ############## Check for packages we depend upon

PKG_CHECK_MODULES(LIBUSB, libusb-1.0 >= 1.0.8)


dnl ############## Output files

AC_CONFIG_HEADERS([src/config.h])

AC_CONFIG_FILES([
  Makefile
  src/Makefile
  src/redhttp/Makefile
])

AC_OUTPUT
