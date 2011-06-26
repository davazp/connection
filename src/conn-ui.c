/* conn-ui.c --- */

/* Copyright (C) 2011 David Vázquez Púa  */
/* Copyright (C) 2011 Mario Castelán Castro  */

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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>
#include <gtk/gtk.h>
#include <assert.h>
#include "conn-hex.h"
#include "conn-hex-widget.h"

#define DEFAULT_BOARD_SIZE 13

/* The title of the main window. The %s will be replaced by the name
   of the current game filename. */
#define UI_WINDOW_TITLE "Connection - %s"

#define UI_BUILDER_FILENAME "connection.ui"
#define UI_BUILDER_FILE (PKGDATADIR "/" UI_BUILDER_FILENAME)
#define CELL_NORMAL_BORDER_WIDTH 1
#define CELL_SELECT_BORDER_WIDTH 3

#define GET_OBJECT(obj) (GTK_WIDGET(gtk_builder_get_object (builder, (obj))))

#define CLIP(x,a,b) (MIN (MAX((x),a), b))

/* Global variables */
static GtkBuilder * builder;
static GtkFileFilter * filter_auto;
static GtkFileFilter * filter_sgf;
static GtkFileFilter * filter_lg_sgf;

/* A couple of points in the history of the game. HISTORY_POINT stands
   for the point which the user is viewing in the widget. On the other
   hand, UNDO_POINT stands for the point where movements are done. */
static unsigned long history_marker;
static unsigned long undo_history_marker;

/* Hexboard widget. */
static GtkWidget * hexboard;

/* Keep player colors to paint cells and borders. */
static double hexboard_color[3][3] = {{1,1,1}, {0,1,0}, {1,0,0}};

/* The game logic. */
static hex_t game;

/* Save and load status */
static char * game_file = NULL;  /* Current file game. */
static hex_format_t game_format; /* The format of the current file game. */

static void hex_to_widget (Hexboard * widget, hex_t hex);
static void update_hexboard_colors (void);
static void update_history_buttons (void);
static void update_hexboard_sensitive (void);
static void update_window_title(void);
static void check_end_of_game (void);

/* Signals */

void
ui_signal_new (GtkMenuItem * item, gpointer data)
{
  Hexboard * board = HEXBOARD (hexboard);
  GtkWidget * dialog = GET_OBJECT ("window-new");
  GtkSpinButton * sizespin = GTK_SPIN_BUTTON (GET_OBJECT ("window-new-size"));
  GtkColorButton * color1 = GTK_COLOR_BUTTON (GET_OBJECT ("window-new-color1"));
  GtkColorButton * color2 = GTK_COLOR_BUTTON (GET_OBJECT ("window-new-color2"));
  gint ok;
  ok = gtk_dialog_run (GTK_DIALOG (dialog));
  if (ok)
    {
      gint size = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON (sizespin));
      GdkColor color;
      double r,g,b;
      hex_free (game);
      game = hex_new (size);
      hexboard_set_size (board, size);
      history_marker = undo_history_marker = 0;

      /* Colors */
      hexboard_color[0][0] = hexboard_color[0][1] = hexboard_color[0][2] = 1;

      gtk_color_button_get_color (color1, &color);
      r = hexboard_color[1][0] = color.red / 65535.0;;
      g = hexboard_color[1][1] = color.green / 65535.0;;
      b = hexboard_color[1][2] = color.blue / 65535.0;;
      hexboard_border_set_color (board, HEXBOARD_BORDER_SE, r,g,b);
      hexboard_border_set_color (board, HEXBOARD_BORDER_NW, r,g,b);

      gtk_color_button_get_color (color2, &color);
      r = hexboard_color[2][0] = color.red / 65535.0;;
      g = hexboard_color[2][1] = color.green / 65535.0;;
      b = hexboard_color[2][2] = color.blue / 65535.0;;
      hexboard_border_set_color (board, HEXBOARD_BORDER_NE, r,g,b);
      hexboard_border_set_color (board, HEXBOARD_BORDER_SW, r,g,b);

      game_file = NULL;
      update_window_title();
      update_hexboard_colors();
      update_hexboard_sensitive();
      update_history_buttons();
    }
  gtk_widget_hide (dialog);
}


void
ui_signal_about (GtkMenuItem * item, gpointer data)
{
  GtkWidget * dialog = GET_OBJECT ("window-about");
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

  dialog = gtk_file_chooser_dialog_new (_("Export"),
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
        g_message (_("An error ocurred while export the board."));

      g_log (G_LOG_DOMAIN, G_LOG_LEVEL_INFO, _("Board was exported to %s."), filename);
      g_free (filename);
    }
  gtk_widget_destroy (dialog);
}

static hex_format_t
dialog_selected_format (GtkWidget * dialog)
{
  GtkFileFilter * filter;
  filter = gtk_file_chooser_get_filter (GTK_FILE_CHOOSER (dialog));
  if (filter == filter_auto)
    return HEX_AUTO;
  else if (filter == filter_sgf)
    return HEX_SGF;
  else if (filter == filter_lg_sgf)
    return HEX_LG_SGF;
  else
    abort ();
}

void
ui_signal_save_as (GtkMenuItem * item, gpointer data)
{
  GtkWidget *dialog;
  GtkWidget *window = GET_OBJECT("window");
  dialog = gtk_file_chooser_dialog_new (_("Save"),
                                        GTK_WINDOW(window),
                                        GTK_FILE_CHOOSER_ACTION_SAVE,
                                        GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                        GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
                                        NULL);
  /* Set filters */
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter_sgf);
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter_lg_sgf);

  if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
    {
      if (game_file != NULL)
        g_free (game_file);
      game_file = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
      game_format = dialog_selected_format (dialog);
      gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (dialog), TRUE);
      if (game_format != HEX_AUTO)
        hex_save_sgf (game, game_format, game_file);
      update_window_title();
    }
  gtk_widget_destroy (dialog);
}

void
ui_signal_open_update_preview (GtkFileChooser *dialog, Hexboard * board)
{
  gchar * filename; 
  hex_t game;
  filename = gtk_file_chooser_get_filename (dialog);
  if (filename != NULL)
    game = hex_load_sgf (game_format, filename);
  else
    game = NULL;
  
  if (game != NULL)
    {
      size_t size = hex_size (game);
      hexboard_set_size (board, size);
      hex_to_widget (board, game);
      gtk_widget_set_visible (GTK_WIDGET(board), TRUE);
      hex_free (game);
    }
  else
    gtk_widget_set_visible (GTK_WIDGET(board), FALSE);
}

void
ui_signal_open (GtkMenuItem * item, gpointer data)
{
  GtkWidget *dialog;
  GtkWidget *window = GET_OBJECT("window");
  char * filename;
  Hexboard * board;

  dialog = gtk_file_chooser_dialog_new (_("Open"),
                                        GTK_WINDOW(window),
                                        GTK_FILE_CHOOSER_ACTION_OPEN,
                                        GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                        GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                                        NULL);

  gtk_widget_set_size_request(GTK_WIDGET(dialog), 800, 600);

  /* Configure the previewer widget */
  board = HEXBOARD (hexboard_new (10));
  gtk_widget_set_size_request(GTK_WIDGET(board), 340, 240);
  gtk_file_chooser_set_preview_widget (GTK_FILE_CHOOSER(dialog), GTK_WIDGET(board));
  g_signal_connect (dialog, "update-preview", G_CALLBACK (ui_signal_open_update_preview), board);

  /* Set filters */
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter_auto);
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter_sgf);
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter_lg_sgf);

  if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
    {
      Hexboard * board = HEXBOARD (hexboard);
      filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
      game_file = filename;
      update_window_title();
      hex_free (game);
      game_format = dialog_selected_format (dialog);
      game = hex_load_sgf (game_format, filename);
      hexboard_set_size (board, hex_size (game));
      g_free (filename);
      history_marker = undo_history_marker = hex_history_current (game);
      update_hexboard_colors ();
      update_hexboard_sensitive();
      update_history_buttons();
      check_end_of_game();
    }
  gtk_widget_destroy (dialog);
}

void
ui_signal_save (GtkMenuItem * item, gpointer data)
{
  if (game_file == NULL)
    ui_signal_save_as (item, data);
  else
    hex_save_sgf (game, game_format, game_file);
}

void
ui_signal_cell_clicked (GtkWidget * widget, gint i, gint j, hex_t ignore)
{
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
      double r = hexboard_color[player][0];
      double g = hexboard_color[player][1];
      double b = hexboard_color[player][2];
      if (!first_move_p)
        hexboard_cell_set_border (HEXBOARD(widget), old_i, old_j, CELL_NORMAL_BORDER_WIDTH);
      hexboard_cell_set_color (HEXBOARD(widget), i, j, r, g, b);
      hexboard_cell_set_border (HEXBOARD(widget), i, j, CELL_SELECT_BORDER_WIDTH);
      check_end_of_game();
    }
  else
    gdk_beep();
}


static void
hex_to_widget (Hexboard * widget, hex_t hex)
{
  size_t size = hex_size (hex);
  boolean first_move_p;
  int i, j;
  for (j=0; j<size; j++)
    {
      for (i=0; i<size; i++)
        {
          int player = hex_cell_player (hex, i, j);
          double r = hexboard_color[player][0];
          double g = hexboard_color[player][1];
          double b = hexboard_color[player][2];
          hexboard_cell_set_border (widget, i, j, CELL_NORMAL_BORDER_WIDTH);
          hexboard_cell_set_color (widget, i, j, r, g, b);
        }
    }
  first_move_p = !hex_history_last_move (hex, &i, &j);
  if (!first_move_p)
    hexboard_cell_set_border (widget, i, j, CELL_SELECT_BORDER_WIDTH);
}

/* Update the color of each cell of the Hexboard widget in screen,
   according to the hex_t board stored.*/
static void
update_hexboard_colors (void)
{
  hex_to_widget (HEXBOARD(hexboard), game);
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

static void
update_window_title (void)
{
  GtkWidget * window = GET_OBJECT ("window");
  const size_t size = 256;
  static gchar * buffer = NULL;
  g_free (buffer);
  buffer = g_malloc (size);
  if (game_file != NULL)
    snprintf (buffer, size, UI_WINDOW_TITLE, game_file);
  else
    snprintf (buffer, size, UI_WINDOW_TITLE, _("New game"));
  gtk_window_set_title (GTK_WINDOW (window), buffer);
}


/* Check if the game is over and draw a visual effect on the board. */
static void
check_end_of_game (void)
{
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
          double r = CLIP (hexboard_color[player][0] + alpha, 0, 1);
          double g = CLIP (hexboard_color[player][1] + alpha, 0, 1);
          double b = CLIP (hexboard_color[player][2] + alpha, 0, 1);
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
ui_signal_preferences (GtkMenuItem * item, gpointer data)
{
  GtkWidget * dialog = GET_OBJECT ("window-preferences");
  gtk_dialog_run (GTK_DIALOG (dialog));
  gtk_widget_hide (dialog);
}

void
ui_signal_connect (GtkMenuItem * item, gpointer data)
{
}

void
ui_signal_disconnect (GtkMenuItem * item, gpointer data)
{
}

void
ui_signal_network (GtkMenuItem * item, gpointer data)
{
}



/* Map G_LOG_LEVEL_MESSAGE logs to GTK error dialogs. */
static void
ui_log_level_message (const gchar * log_domain, GLogLevelFlags log_level,
                      const char * message, gpointer user_data)
{
  GtkWidget *window = GET_OBJECT("window");;
  GtkWidget * dialog;
  dialog = gtk_message_dialog_new (GTK_WINDOW(window),
                                   GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                   GTK_MESSAGE_ERROR,
                                   GTK_BUTTONS_OK,
                                   "%s", message);
  gtk_window_set_title (GTK_WINDOW (dialog), _("Error"));
  gtk_dialog_run (GTK_DIALOG (dialog));
  gtk_widget_destroy (dialog);
}

/* Map G_LOG_LEVEL_INFO logs to the window's status bar. */
static void
ui_log_level_info (const gchar * log_domain, GLogLevelFlags log_level,
                   const char * message, gpointer user_data)
{
  static GtkWidget * statusbar;
  static guint context;
  static guint initialized = 0;
  if (!initialized)
    {
      statusbar = GET_OBJECT ("statusbar");
      context = gtk_statusbar_get_context_id(GTK_STATUSBAR (statusbar), "Game messages");
      initialized = 1;
    }
  gtk_statusbar_pop (GTK_STATUSBAR (statusbar), context);
  gtk_statusbar_push (GTK_STATUSBAR (statusbar), context, message);
  gtk_main_iteration();
}


void
ui_run (void)
{
  GtkWidget * box;
  GtkWidget * about;
  GtkWidget * window;
  int success;

  builder = gtk_builder_new();
  success = gtk_builder_add_from_file (builder, UI_BUILDER_FILENAME, NULL);
  if (!success)
    success = gtk_builder_add_from_file (builder, UI_BUILDER_FILE, NULL);
  if (!success)
    g_error (_("User interface definition file '%s' was not found.\n"));

  gtk_builder_connect_signals (builder, NULL);
  window = GET_OBJECT("window");
  box = GET_OBJECT("box");
  about = GET_OBJECT ("window-about");

  g_log_set_handler (G_LOG_DOMAIN, G_LOG_LEVEL_MESSAGE, ui_log_level_message, NULL);
  g_log_set_handler (G_LOG_DOMAIN, G_LOG_LEVEL_INFO, ui_log_level_info, NULL);

  game = hex_new (DEFAULT_BOARD_SIZE);
  gtk_about_dialog_set_version (GTK_ABOUT_DIALOG (about), PACKAGE_VERSION);
  hexboard = hexboard_new(DEFAULT_BOARD_SIZE);

  filter_auto = gtk_file_filter_new ();
  filter_sgf = gtk_file_filter_new ();
  filter_lg_sgf = gtk_file_filter_new ();
  gtk_file_filter_set_name (filter_auto, "Select Automatically");
  gtk_file_filter_set_name (filter_sgf, "Smart Game Format (SGF)");
  gtk_file_filter_set_name (filter_lg_sgf, "LittleGolem's Smart Game Format (SGF)");
  gtk_file_filter_add_pattern (filter_auto, "*");
  gtk_file_filter_add_pattern (filter_sgf, "*.sgf");
  gtk_file_filter_add_pattern (filter_lg_sgf, "*.hsgf");
  gtk_file_filter_add_pattern (filter_lg_sgf, "*.sgf");
  g_object_ref_sink (filter_auto);
  g_object_ref_sink (filter_sgf);
  g_object_ref_sink (filter_lg_sgf);

  g_signal_connect (GTK_WIDGET(hexboard), "cell_clicked", G_CALLBACK(ui_signal_cell_clicked), game);
  gtk_container_add (GTK_CONTAINER(box), hexboard);
  gtk_widget_show_all (window);
  update_history_buttons();
  update_window_title();
  gtk_main();
  hex_free (game);
}


/* conn-ui.c ends here */
