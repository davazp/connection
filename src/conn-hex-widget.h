/* conn-hex-widget.h --- Hex board implementation as GTK Widget. */

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

#ifndef CONN_HEX_WIDGET_H
#define CONN_HEX_WIDGET_H

#include <gtk/gtk.h>

#define TYPE_HEXBOARD (hexboard_get_type())

#define HEXBOARD(obj)                                   \
  (GTK_CHECK_CAST((obj), TYPE_HEXBOARD, Hexboard))

#define HEXBOARD_CLASS(class)                                   \
  (GTK_CHECK_CLASS_CAST((class), TYPE_HEXBOARD, HexboardClass))

#define IS_HEXBOARD(obj)                        \
  (GTK_CHECK_TYPE((obj), TYPE_HEXBOARD))

#define IS_HEXBOARD_CLASS(obj)                          \
  (G_TYPE_CHECK_CLASS_TYPE ((obj), TYPE_HEXBOARD))

#define HEXBOARD_GET_CLASS(obj)                                         \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_HEXBOARD, HexboardClass))


typedef struct _Hexboard Hexboard;
typedef struct _HexboardClass HexboardClass;

struct _Hexboard {
  GtkDrawingArea widget;
};

struct _HexboardClass {
  GtkDrawingAreaClass parent_class;
  void (*cell_clicked) (Hexboard *hexboard, gint i, gint j, gpointer user_data);
};

GtkType hexboard_get_type(void);

GtkWidget * hexboard_new (void);

size_t hexboard_get_size (Hexboard * hex);

/* Default color value is white. */
gboolean hexboard_cell_set_color (Hexboard * board, gint i, gint j,
                                  double r, double g, double b);
gboolean hexboard_cell_get_color (Hexboard * board, gint i, gint j,
                                  double *r, double *g, double *b);

/* Default border value is 1.0 */
gboolean hexboard_cell_set_border (Hexboard * board, gint i, gint j, double border);
gboolean hexboard_cell_get_border (Hexboard * board, gint i, gint j, double * border);

gboolean hexboard_save_as_image (Hexboard * hex, const char * filename,
                                 const char * type, guint width, guint height);

#endif  /* CONN_HEX_WIDGET_H */

/* conn-hex-widget.h ends here */
