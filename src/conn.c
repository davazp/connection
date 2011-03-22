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

#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <math.h>
#include "config.h"
#include "conn-hex-widget.h"

#define GET_OBJECT(obj) (GTK_WIDGET(gtk_builder_get_object (builder, (obj))))
static GtkBuilder * builder;

gboolean
ui_about (GtkWidget * window, gpointer data)
{
  GtkWidget * dialog = GET_OBJECT ("about");
  gtk_dialog_run (GTK_DIALOG (dialog));
  gtk_widget_hide (dialog);
}


gboolean
ui_quit (GtkWidget * window, gpointer data)
{
  gtk_main_quit();
  return TRUE;
}

void
ui_cell_clicked (GtkWidget * widget, gint i, gint j, gpointer data)
{
  printf ("(%d, %d)\n", i, j);
  hexboard_set_color (HEXBOARD(widget), i, j, 0, 0, 0);
}


int
main (int argc, char * argv[])
{
  GtkWidget * window;
  GtkWidget * box;
  GtkWidget * hexboard;
  GtkWidget * about;
  gtk_init (&argc, &argv);
  builder = gtk_builder_new();
  gtk_builder_add_from_file (builder, "./connection.ui", NULL);
  gtk_builder_connect_signals (builder, NULL);

  window = GET_OBJECT("window");
  box = GET_OBJECT("box");
  about = GET_OBJECT ("about");

  gtk_about_dialog_set_version (GTK_ABOUT_DIALOG (about), PACKAGE_VERSION);
  hexboard = hexboard_new();
  g_signal_connect (GTK_WIDGET(hexboard), "cell_clicked", G_CALLBACK(ui_cell_clicked), NULL);

  gtk_container_add (GTK_CONTAINER(box), hexboard);

  gtk_widget_show_all (window);
  gtk_main();
  return 0;
}

/* end of conn.c */
