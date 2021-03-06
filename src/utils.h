/* utils.h --- Functions and macros to use across of the project (Header) */

/* Copyright (C) 2011 David Vázquez Púa  */

/* This file is part of Connection.
 *
 * Connection is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Connection is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Connection.  If not, see <http://www.gnu.org/licenses/>. */

#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>
#include <stdarg.h>
#include <locale.h>
#include <libintl.h>

#ifndef TRUE
# define TRUE 1
#endif

#ifndef FALSE
# define FALSE 0
#endif

typedef int boolean;

/* I18N */
#define _(str)  gettext(str)
#define N_(str) ngettext(str)

/* The following macros are given by command-line at compile-time. You
   can use autoconf & automake to customize the values. */
/* #undef LOCALEDIR */
/* #undef PKGDATADIR */

#endif  /* UTILS_H */

/* utils.h ends here */
