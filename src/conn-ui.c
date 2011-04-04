/* conn-ui.c --- */

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
#include <math.h>
#include <gtk/gtk.h>
#include <assert.h>
#include "conn-utils.h"
#include "conn-hex.h"
#include "conn-hex-widget.h"

#define DEFAULT_BOARD_SIZE 13

#define UI_BUILDER_FILENAME "connection.glade"
#define UI_BUILDER_FILE (PKGDATADIR "/" UI_BUILDER_FILENAME)
#define CELL_NORMAL_BORDER_WIDTH 1
#define CELL_SELECT_BORDER_WIDTH 3

#define GET_OBJECT(obj) (GTK_WIDGET(gtk_builder_get_object (builder, (obj))))

#define CLIP(x,a,b) (MIN (MAX((x),a), b))

/* Global variables */
static GtkBuilder * builder;

/* A couple of points in the history of the game. HISTORY_POINT stands
   for the point which the user is viewing in the widget. Otherwise,
   UNDO_POINT stands for the point where movements are inserted. */
static unsigned long history_marker;
static unsigned long undo_history_marker;

static GtkWidget * hexboard;
static hex_t game;

static void update_hexboard_colors (void);
static void update_history_buttons (void);
static void update_hexboard_sensitive (void);
static void check_end_of_game (void);

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

void
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
  history_marker = undo_history_marker = 0;
  update_hexboard_colors();
  update_hexboard_sensitive();
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
      char * ext;
      gint width, height;
      GtkFileFilter * filter;
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
        ui_error (_("An error ocurred while export the board."));

      g_free (filename);
    }
  gtk_widget_destroy (dialog);
}


void
ui_signal_cell_clicked (GtkWidget * widget, gint i, gint j, hex_t game)
{
  double colors[3][3] = {{1,1,1}, {0,1,0}, {1,0,0}};
  int player;
  int old_i, old_j;
  hex_status_t status;
  int first_move_p;
  first_move_p = !hex_history_last_move (game, &old_i, &old_j);
  player = hex_get_player (game);
  status = hex_move (game, i, j);
  undo_history_marker = history_marker = hex_history_current (game);
  update_history_buttons();
  if (status == HEX_SUCCESS)
    {
      double r = colors[player][0];
      double g = colors[player][1];
      double b = colors[player][2];
      if (!first_move_p)
        hexboard_cell_set_border (HEXBOARD(widget), old_i, old_j, CELL_NORMAL_BORDER_WIDTH);
      hexboard_cell_set_color (HEXBOARD(widget), i, j, r, g, b);
      hexboard_cell_set_border (HEXBOARD(widget), i, j, CELL_SELECT_BORDER_WIDTH);
      check_end_of_game();
    }
  else
    gdk_beep();
}


/* Update the color of each cell of the Hexboard widget in screen,
   according to the hex_t board stored.*/
static void
update_hexboard_colors (void)
{
  Hexboard * hex = HEXBOARD(hexboard);
  double colors[3][3] = {{1,1,1}, {0,1,0}, {1,0,0}};
  size_t size = hex_size (game);
  boolean first_move_p;
  int i, j;
  for (j=0; j<size; j++)
    {
      for (i=0; i<size; i++)
        {
          int player = hex_cell_player (game, i, j);
          double r = colors[player][0];
          double g = colors[player][1];
          double b = colors[player][2];
          hexboard_cell_set_border (HEXBOARD(hexboard), i, j, CELL_NORMAL_BORDER_WIDTH);
          hexboard_cell_set_color (hex, i, j, r, g, b);
        }
    }
  first_move_p = !hex_history_last_move (game, &i, &j);
  if (!first_move_p)
    hexboard_cell_set_border (HEXBOARD(hexboard), i, j, CELL_SELECT_BORDER_WIDTH);
}

/* Update the sensitive of history buttons according to the history
   status of the hex_t structure. */
static void
update_history_buttons (void)
{
  GtkWidget * first    = GET_OBJECT ("button-history-first");
  GtkWidget * backward = GET_OBJECT ("button-history-backward");
  GtkWidget * forward  = GET_OBJECT ("button-history-forward");
  GtkWidget * last     = GET_OBJECT ("button-history-last");
  GtkWidget * undo     = GET_OBJECT ("menu-undo");
  GtkWidget * redo     = GET_OBJECT ("menu-redo");
  int size = hex_history_size (game);
  /* Set sensitive attributes to history buttons. */
  gtk_widget_set_sensitive (first,    history_marker != 0);
  gtk_widget_set_sensitive (backward, history_marker != 0);
  gtk_widget_set_sensitive (last,     history_marker != undo_history_marker);
  gtk_widget_set_sensitive (forward,  history_marker != undo_history_marker);
  /* undo/redo */
  if (history_marker == undo_history_marker && undo_history_marker > 0)
    gtk_widget_set_sensitive (undo, TRUE);
  else
    gtk_widget_set_sensitive (undo, FALSE);

  if (history_marker == undo_history_marker && undo_history_marker < size)
    gtk_widget_set_sensitive (redo, TRUE);
  else
    gtk_widget_set_sensitive (redo, FALSE);
}


static void
update_hexboard_sensitive (void)
{
  boolean sensitivep;
  sensitivep = (history_marker == undo_history_marker && !hex_end_of_game_p (game));
  gtk_widget_set_sensitive (hexboard, sensitivep);
}


/* Check if the game is over and draw a visual effect on the board. */
static void
check_end_of_game (void)
{
  double colors[3][3] = {{1,1,1}, {0,1,0}, {1,0,0}};
  Hexboard * hex = HEXBOARD(hexboard);
  size_t size = hex_size (game);
  boolean first_move_p;
  int i, j;

  if (!hex_end_of_game_p (game))
    return;

  for (j=0; j<size; j++)
    {
      for (i=0; i<size; i++)
        {
          int player = hex_cell_player (game, i, j);
          int a_connected_p = hex_cell_a_connected_p (game, i, j) > 0;
          int z_connected_p = hex_cell_z_connected_p (game, i, j) > 0;
          double alpha = a_connected_p && z_connected_p? 0: -0.5;
          double r = CLIP (colors[player][0] + alpha, 0, 1);
          double g = CLIP (colors[player][1] + alpha, 0, 1);
          double b = CLIP (colors[player][2] + alpha, 0, 1);
          hexboard_cell_set_border (HEXBOARD(hexboard), i, j, CELL_NORMAL_BORDER_WIDTH);
          hexboard_cell_set_color (hex, i, j, r, g, b);
        }
    }
}

/* The following signal handlers assume the history of the game is in
   a good point to work that operation. The function
   `update_history_buttons' is reliable to enable/disable buttons in
   order to these signals can be emited. */
void
ui_signal_history_first (GtkToolButton * button, gpointer data)
{
  size_t size = hex_history_size (game);
  assert (history_marker > 0);
  hex_history_jump (game, 0);
  history_marker = 0;
  update_hexboard_colors();
  update_history_buttons();
  update_hexboard_sensitive();
}

void
ui_signal_history_backward (GtkToolButton * button, gpointer data)
{
  assert (history_marker > 0);
  hex_history_jump (game, --history_marker);
  update_hexboard_colors();
  update_history_buttons();
  update_hexboard_sensitive();
}

void
ui_signal_history_forward (GtkToolButton * button, gpointer data)
{
  assert (history_marker < undo_history_marker);
  hex_history_jump (game, ++history_marker);
  update_hexboard_colors();
  update_history_buttons();
  update_hexboard_sensitive();
  check_end_of_game();
}

void
ui_signal_history_last (GtkToolButton * button, gpointer data)
{
  assert (history_marker < undo_history_marker);
  hex_history_jump (game, undo_history_marker);
  history_marker = undo_history_marker;
  update_hexboard_colors();
  update_history_buttons();
  update_hexboard_sensitive();
  check_end_of_game();
}

void
ui_signal_undo (GtkMenuItem * item, gpointer data)
{
  assert (history_marker == undo_history_marker);
  assert (undo_history_marker > 0);
  undo_history_marker--;
  history_marker = undo_history_marker;
  hex_history_jump (game, undo_history_marker);
  update_hexboard_colors();
  update_history_buttons();
  update_hexboard_sensitive();
}

void
ui_signal_redo (GtkMenuItem * item, gpointer data)
{
  size_t size = hex_history_size (game);
  assert (history_marker == undo_history_marker);
  assert (undo_history_marker < size);
  undo_history_marker++;
  history_marker = undo_history_marker;
  hex_history_jump (game, history_marker);
  update_hexboard_colors();
  update_history_buttons();
  update_hexboard_sensitive();
  check_end_of_game();
}


void
ui_main (void)
{
  GtkWidget * box;
  GtkWidget * about;
  GtkWidget * window;
  int success;

  builder = gtk_builder_new();
  success = 0;
  success |= gtk_builder_add_from_file (builder, UI_BUILDER_FILE,     NULL);
  success |= gtk_builder_add_from_file (builder, UI_BUILDER_FILENAME, NULL);
  if (!success)
    fatal ("File '%s' not found.\n");

  gtk_builder_connect_signals (builder, NULL);
  window = GET_OBJECT("window");
  box = GET_OBJECT("box");
  about = GET_OBJECT ("about");

  game = hex_new (DEFAULT_BOARD_SIZE);
  gtk_about_dialog_set_version (GTK_ABOUT_DIALOG (about), PACKAGE_VERSION);
  hexboard = hexboard_new(DEFAULT_BOARD_SIZE);
  g_signal_connect (GTK_WIDGET(hexboard), "cell_clicked", G_CALLBACK(ui_signal_cell_clicked), game);
  gtk_container_add (GTK_CONTAINER(box), hexboard);
  gtk_widget_show_all (window);
  update_history_buttons();
  gtk_main();
  hex_free (game);
}


/* conn-ui.c ends here */
