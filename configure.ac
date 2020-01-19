#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([pushmac], [0.0.1], [github.com/flynnzac/pushmac])
AC_CONFIG_SRCDIR([pushmac.pshm])

# Check if any version of pushmac is installed
AC_CHECK_PROG([pushmac_avail], [pushmac], [true], [false])
AM_CONDITIONAL([PUSHMAC], [test x$pushmac_avail = xtrue])

# Check for libraries
AC_CHECK_LIB([unistring], [u8_next])

# Defines
AC_DEFINE([PAGE_MACRO], 100)
AC_DEFINE([PAGE_BUFFER], 10000)
AC_DEFINE([PAGE_STACK], 100)
AC_DEFINE([DEFINE], ['@'])
AC_DEFINE([SHELL], ['|'])
AC_DEFINE([PUSH], ['\#'])
AC_DEFINE([PUSH2], ['='])
AC_DEFINE([REF], ['~'])
AC_DEFINE([EXPAND], [36])
AC_DEFINE([QUESTION], ['?'])
AC_DEFINE([IGNORE], ['%'])
AC_DEFINE([SILENCE], ['^'])
AC_DEFINE([SPEAK], ['!'])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC

AM_INIT_AUTOMAKE([foreign])
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
