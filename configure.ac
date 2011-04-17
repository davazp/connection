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
GETTEXT_PACKAGE=connection
IT_PROG_INTLTOOL
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE], ["$GETTEXT_PACKAGE"], [The domain to use with gettext])
AM_GLIB_GNU_GETTEXT

dnl Check for Loudmouth
PKG_CHECK_MODULES(LOUDMOUTH, loudmouth-1.0)
AC_SUBST(LOUDMOUTH_CFLAGS)
AC_SUBST(LOUDMOUTH_LIBS)

dnl Generate output files
AC_CONFIG_FILES([Makefile data/Makefile src/Makefile po/Makefile.in])
AC_OUTPUT

dnl end ofg configure.ac
