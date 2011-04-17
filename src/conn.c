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
#include "utils.h"
#include <string.h>
#include <gtk/gtk.h>
#include "conn-ui.h"

int
main (int argc, char * argv[])
{
  /* Internationalization */
  setlocale(LC_ALL, "");
  bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
  textdomain(GETTEXT_PACKAGE);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  /* Initialize GTK library and run the Connection user interface. */
  gtk_init (&argc, &argv);
  xmpp_init();
  ui_run();
  return 0;
}

/* conn.c ends here */
