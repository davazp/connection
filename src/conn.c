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
#include <string.h>
#include <stdarg.h>
#include <gtk/gtk.h>
#include <math.h>
#include "conn-utils.h"
#include "conn-hex.h"
#include "conn-hex-widget.h"

#define UI_BUILDER_FILENAME "./connection.ui"

#define GET_OBJECT(obj) (GTK_WIDGET(gtk_builder_get_object (builder, (obj))))

/* Global variables */
static GtkBuilder * builder;
GtkWidget * hexboard;
hex_t game;

static void update_hexboard_colors (void);
static void update_history_buttons (void);

void
ui_error (const gchar *fmt, ...)
{
  GtkWidget *window;
  GtkWidget * dialog;
  char buffer[256];
  va_list va;
  va_start (va,fmt);
  vsnprintf (buffer, sizeof(buffer), fmt, va);
  window = GET_OBJECT("window");
  g_logv (G_LOG_DOMAIN, G_LOG_LEVEL_WARNING, fmt, va);
  dialog = gtk_message_dialog_new (GTK_WINDOW(window),
                                   GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                   GTK_MESSAGE_ERROR,
                                   GTK_BUTTONS_OK,
                                   buffer);
  gtk_window_set_title (GTK_WINDOW (dialog), "Error");
  gtk_dialog_run (GTK_DIALOG (dialog));
  gtk_widget_destroy (dialog);
  va_end (va);
}

static void
ui_message (char * fmt, ...)
{
  char buffer[256];
  va_list va;
  static GtkWidget * statusbar;
  static guint context;
  static guint initialized = 0;
  va_start (va, fmt);
  if (!initialized)
    {
      statusbar = GET_OBJECT ("statusbar");
      context = gtk_statusbar_get_context_id(GTK_STATUSBAR (statusbar), "Game messages");
      initialized = 1;
    }
  vsnprintf (buffer, sizeof(buffer), fmt, va);
  gtk_statusbar_pop (GTK_STATUSBAR (statusbar), context);
  gtk_statusbar_push (GTK_STATUSBAR (statusbar), context, buffer);
  va_end (va);
}


/* Signals */


void
ui_signal_new (GtkMenuItem * item, gpointer data)
{
  int i,j;
  size_t size;
  size = hexboard_get_size (HEXBOARD (hexboard));
  hex_reset (game);
  for(j=0; j<size; j++)
    {
      for(i=0; i<size; i++)
        hexboard_set_color (HEXBOARD(hexboard), i, j, 1, 1, 1);
    }
  update_history_buttons();
}

void
ui_signal_about (GtkMenuItem * item, gpointer data)
{
  GtkWidget * dialog = GET_OBJECT ("about");
  gtk_dialog_run (GTK_DIALOG (dialog));
  gtk_widget_hide (dialog);
}

void
ui_signal_quit (GtkMenuItem * item, gpointer data)
{
  gtk_main_quit();
}

void
ui_signal_export (GtkMenuItem * item, gpointer data)
{
  GtkWidget *dialog;
  GtkWidget *window = GET_OBJECT("window");
  GtkFileFilter * filter_auto;
  GtkFileFilter * filter_pdf;
  GtkFileFilter * filter_svg;
  GtkFileFilter * filter_png;

  dialog = gtk_file_chooser_dialog_new ("Export",
                                        GTK_WINDOW(window),
                                        GTK_FILE_CHOOSER_ACTION_SAVE,
                                        GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                        GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
                                        NULL);
  /* Set filters */
  filter_auto = gtk_file_filter_new();
  filter_pdf = gtk_file_filter_new();
  filter_svg = gtk_file_filter_new();
  filter_png = gtk_file_filter_new();
  gtk_file_filter_add_pattern (filter_auto, "*");
  gtk_file_filter_add_mime_type (filter_pdf, "application/pdf");
  gtk_file_filter_add_mime_type (filter_svg, "image/svg+xml");
  gtk_file_filter_add_mime_type (filter_png, "image/png");
  gtk_file_filter_set_name (filter_pdf, "Portable Document Format (PDF)");
  gtk_file_filter_set_name (filter_svg, "Scalable Vector Graphcis (SVG)");
  gtk_file_filter_set_name (filter_png, "Portable Networks Graphcis (PNG)");
  gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (dialog), TRUE);
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter_png);
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter_pdf);
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter_svg);

  if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
    {
      char *filename;
      gint width, height;
      GtkFileFilter * filter;
      char * ext;
      gboolean successp;

      filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
      filter = gtk_file_chooser_get_filter (GTK_FILE_CHOOSER (dialog));
      gdk_window_get_size (hexboard->window, &width, &height);

      if (filter == filter_pdf)
        ext = "pdf";
      else if (filter == filter_png)
        ext = "png";
      else if (filter == filter_svg)
        ext = "svg";

      successp = hexboard_save_as_image (HEXBOARD(hexboard), filename, ext, width, height);
      if (!successp)
        ui_error ("An error ocurred while export the board.");

      g_free (filename);
    }

  gtk_widget_destroy (dialog);
}


void
ui_signal_cell_clicked (GtkWidget * widget, gint i, gint j, hex_t game)
{
  double colors[3][3] = {{1,1,1}, {0,1,0}, {1,0,0}};
  int player;
  hex_status_t status;
  player = hex_get_player (game);
  status = hex_move (game, i, j);
  update_history_buttons();
  if (status == HEX_SUCCESS)
    {
      hexboard_set_color (HEXBOARD(widget), i, j,
                          colors[player][0],
                          colors[player][1],
                          colors[player][2]);
    }
  else
    gdk_beep();
}


static void
update_hexboard_colors (void)
{
  Hexboard * hex = HEXBOARD(hexboard);
  double colors[3][3] = {{1,1,1}, {0,1,0}, {1,0,0}};
  size_t size = hex_size (game);
  int i, j;
  for (j=0; j<size; j++)
    {
      for (i=0; i<size; i++)
        {
          int player = hex_cell_player (game, i, j);
          hexboard_set_color (hex, i, j,
                              colors[player][0],
                              colors[player][1],
                              colors[player][2]);
        }
    }
}

static void
update_history_buttons (void)
{
  GtkWidget * first    = GET_OBJECT ("button-history-first");
  GtkWidget * backward = GET_OBJECT ("button-history-backward");
  GtkWidget * forward  = GET_OBJECT ("button-history-forward");
  GtkWidget * last     = GET_OBJECT ("button-history-last");
  GtkWidget * undo     = GET_OBJECT ("menu-undo");
  GtkWidget * redo     = GET_OBJECT ("menu-redo");
  size_t size;
  size_t count;
  size = hex_history_size (game);
  count = hex_history_count (game);
  /* Set sensitive attributes to history buttons. */
  gtk_widget_set_sensitive (first,    count==0?    FALSE: TRUE);
  gtk_widget_set_sensitive (backward, count==0?    FALSE: TRUE);
  gtk_widget_set_sensitive (last,     count==size? FALSE: TRUE);
  gtk_widget_set_sensitive (forward,  count==size? FALSE: TRUE);
  /* undo/redo */
  gtk_widget_set_sensitive (undo, count==size? TRUE: FALSE);
  gtk_widget_set_sensitive (redo, count==size? TRUE: FALSE);
}


void
ui_signal_history_first (GtkToolButton * button, gpointer data)
{
  int size = hex_history_size(game);
  int count = hex_history_count(game);
  while (count--)
    hex_history_backward (game);
  update_hexboard_colors();
  update_history_buttons();

  gtk_widget_set_sensitive (hexboard, size==0);
}

void
ui_signal_history_backward (GtkToolButton * button, gpointer data)
{
  int size = hex_history_size (game);
  hex_history_backward (game);
  update_hexboard_colors();
  update_history_buttons();
  gtk_widget_set_sensitive (hexboard, size==0);
}

void
ui_signal_history_forward (GtkToolButton * button, gpointer data)
{
  int size = hex_history_size (game);
  int count = hex_history_count (game);
  hex_history_forward (game);
  update_hexboard_colors();
  update_history_buttons();
  gtk_widget_set_sensitive (hexboard, size==count+1);
}

void
ui_signal_history_last (GtkToolButton * button, gpointer data)
{
  int size = hex_history_size(game);
  int count = hex_history_count(game);
  for (; count<size; count++)
    hex_history_forward (game);
  update_hexboard_colors();
  update_history_buttons();
  gtk_widget_set_sensitive (hexboard, TRUE);
}

void
ui_signal_undo (GtkMenuItem * item, gpointer data)
{
  hex_history_backward (game);
  update_hexboard_colors();
  gtk_widget_set_sensitive (hexboard, TRUE);
}

void
ui_signal_redo (GtkMenuItem * item, gpointer data)
{
  hex_history_backward (game);
  update_hexboard_colors();
  gtk_widget_set_sensitive (hexboard, TRUE);
}



int
main (int argc, char * argv[])
{
  GtkWidget * box;
  GtkWidget * about;
  GtkWidget * window;

  gtk_init (&argc, &argv);
  builder = gtk_builder_new();
  gtk_builder_add_from_file (builder, UI_BUILDER_FILENAME, NULL);
  gtk_builder_connect_signals (builder, NULL);
  window = GET_OBJECT("window");
  box = GET_OBJECT("box");
  about = GET_OBJECT ("about");

  game = hex_new (13);
  gtk_about_dialog_set_version (GTK_ABOUT_DIALOG (about), PACKAGE_VERSION);
  hexboard = hexboard_new();
  g_signal_connect (GTK_WIDGET(hexboard),
                    "cell_clicked",
                    G_CALLBACK(ui_signal_cell_clicked),
                    game);
  gtk_container_add (GTK_CONTAINER(box), hexboard);
  gtk_widget_show_all (window);

  update_history_buttons();

  gtk_main();
  hex_free (game);

  return 0;
}

/* conn.c ends here */
