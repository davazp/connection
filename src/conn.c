/* conn.c --- */

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

#include "config.h"
#include <getopt.h>
#include <gtk/gtk.h>
#include "conn-utils.h"
#include "conn-ui.h"

static const char short_options[] = "vh";

static const struct option long_options[] = {
  {"version", no_argument, NULL, 'v'},
  NULL};

int
main (int argc, char * argv[])
{
  int opt;
  while ((opt = getopt_long (argc, argv, "v", long_options, NULL)) != -1)
    {
      switch (opt)
        {
        case 'v':
          puts (PACKAGE_STRING);
          puts ("Copyright (C) 2011  David Vázquez Púa.");
          puts ("This is free software; see the source for copying conditions.");
          puts ("There is NO warranty; not even for MERCHANTABILITY or FITNESS FOR A");
          puts ("PARTICULAR PURPOSE.");
          exit (EXIT_SUCCESS);
          break;
        case '?':
          exit (EXIT_FAILURE);
          break;
        }
    }

  /* Internationalization */
  setlocale(LC_ALL, "");
  bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
  textdomain(GETTEXT_PACKAGE);
  /* Initialize GTK library and run the Connection user interface. */
  gtk_init (&argc, &argv);
  ui_main();
  return 0;
}

/* conn.c ends here */
