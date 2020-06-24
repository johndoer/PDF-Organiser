AC_INIT([pdforganiser], [0.1], [johndoer99@protonmail.com])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_PROG_CXX
AC_CONFIG_FILES([
 Makefile
 src/Makefile
 pixmaps/Makefile
 data/Makefile
])
dnl Test for pkg-config
PKG_PROG_PKG_CONFIG([0.22])
dnl Test for gtk+
PKG_CHECK_MODULES([GTK], [gtk+-3.0 >= 3.10])
dnl Test for json-glib
PKG_CHECK_MODULES([JSONGLIB], [json-glib-1.0 >= 1.0])
AC_OUTPUT
