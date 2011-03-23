/* conn-utils.c --- Functions and macros to use across of the project */

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

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

/* Errors */

void
conn_error (const char * fmt, ...)
{
  va_list va;
  va_start (va, fmt);
  vfprintf (stderr, fmt, va);
  va_end (va);
}

void
conn_fatal(const char * fmt, ...)
{
  va_list va;
  va_start (va, fmt);
  vfprintf (stderr, fmt, va);
  va_end (va);
  exit (-1);
}


/* Memory management */

static void
insuficient_memory(void)
{
  conn_fatal ("ERROR: Insuficient memory.\n");
}

void*
conn_malloc(size_t size)
{
  void* p = malloc (size);
  if (p == NULL)
    insuficient_memory();
  return p;
}

void*
conn_calloc(size_t nmemb, size_t size)
{
  void* p = calloc (nmemb, size);
  if (p == NULL)
    insuficient_memory();
  return p;
}

void
conn_free(void * pointer)
{
  free (pointer);
}


/* conn-utils.c ends here */
