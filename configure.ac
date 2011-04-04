AC_INIT([Connection],[0.1])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE(connection, 0.1)
AC_CONFIG_HEADERS([src/config.h])

dnl Compiler issues
AC_PROG_CC
AM_PROG_CC_C_O

dnl GTK
AM_PATH_GTK_2_0(2.0.0,,AC_MSG_ERROR(Connection needs GTK+2.0))

dnl gettext
AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.18.1])

dnl Generate output files
AC_CONFIG_FILES([Makefile src/Makefile po/Makefile.in])
AC_OUTPUT

dnl end ofg configure.ac
