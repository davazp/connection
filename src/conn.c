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
#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <math.h>
#include "conn-utils.h"
#include "conn-hex.h"
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
ui_cell_clicked (GtkWidget * widget, gint i, gint j, hex_t game)
{
  double colors[3][3] = {{1,1,1}, {0,1,0}, {1,0,0}};
  int player;
  hex_status_t status;
  player = hex_get_player (game);
  status = hex_move (game, i, j);
  printf ("status=%u\n", status);
  printf ("player=%u\n", player);
  printf ("a-connected=%u\n", hex_cell_a_connected_p(game,i,j));
  printf ("z-connected=%u\n", hex_cell_z_connected_p(game,i,j));
  puts ("---");
  if (status == HEX_SUCCESS)
    hexboard_set_color (HEXBOARD(widget), i, j,
                        colors[player][0],
                        colors[player][1],
                        colors[player][2]);
  else
    gdk_beep();
}


int
main (int argc, char * argv[])
{
  GtkWidget * window;
  GtkWidget * box;
  GtkWidget * hexboard;
  GtkWidget * about;
  hex_t game;

  gtk_init (&argc, &argv);
  builder = gtk_builder_new();
  gtk_builder_add_from_file (builder, "./connection.ui", NULL);
  gtk_builder_connect_signals (builder, NULL);

  window = GET_OBJECT("window");
  box = GET_OBJECT("box");
  about = GET_OBJECT ("about");

  game = hex_new (13);

  gtk_about_dialog_set_version (GTK_ABOUT_DIALOG (about), PACKAGE_VERSION);
  hexboard = hexboard_new();
  g_signal_connect (GTK_WIDGET(hexboard), "cell_clicked", G_CALLBACK(ui_cell_clicked), game);

  gtk_container_add (GTK_CONTAINER(box), hexboard);

  gtk_widget_show_all (window);
  gtk_main();
  hex_free (game);

  return 0;
}

/* conn.c ends here */
