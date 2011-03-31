/* conn-hex-widget.c --- Hex board implementation as GTK Widget. */

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
#include <math.h>
#include <gtk/gtk.h>
#include <cairo.h>
#include <cairo-pdf.h>
#include <cairo-svg.h>
#include "conn-hex-widget.h"
#include "conn-marshallers.h"

#define BOARD_MAX_SIZE 32

/* The following figure is included as illustration of the Hex board in
   order to you can read easily the geometry-related source code above.

                                ___
                               /   \
                           ___/     \___
                          /   \     /   \
                      ___/     \___/     \___
                     /   \     /   \     /   \
                    /     \___/     \___/     \
                    \     /   \     /   \     /
                     \___/     \___/     \___/
                   2     \     /   \     /     2
                          \___/     \___/
                       1      \     /     1
                               \___/
                            0        0

                ASCII Figure of a 3x3 Hex board.  */

#define HEXBOARD_BORDER_WIDTH 10

static void hexboard_init(Hexboard *hexboard);
static void hexboard_finalize(Hexboard *hexboard);
static void hexboard_class_init(HexboardClass *klass);
static gboolean hexboard_configure (GtkWidget * widget, GdkEventConfigure *event);
static gboolean hexboard_expose(GtkWidget * widget, GdkEventExpose *event);
static gboolean hexboard_button_press (GtkWidget * widget, GdkEventButton * event);

static void cell_to_pixel (Hexboard * hexboard, int i, int j,
                           int *output_x, int *output_y);

static void pixel_to_cell (Hexboard * hexboard, int x, int y,
                           int *output_i, int *output_j);

static void draw_hexagon (cairo_t *cr, double cx, double cy, double radious,
                          double r, double g, double b, double border);

static void draw_board (Hexboard * hexboard, cairo_t * cr,
                        gint width, gint height);


G_DEFINE_TYPE (Hexboard, hexboard, GTK_TYPE_DRAWING_AREA);

typedef struct _HexboardPrivate {
  int size;
  int border;
  /* Geometry of the board in the viewport widget. They are computed
     by the hexboard_configure function when the main window is
     resized. */
  float board_left;
  float board_top;
  float board_width;
  float board_height;
  float cell_width;
  float cell_height;
  struct {
    double r;
    double g;
    double b;
    double border;
  } look[BOARD_MAX_SIZE][BOARD_MAX_SIZE];
} HexboardPrivate;

#define HEXBOARD_GET_PRIVATE(obj)                                       \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), TYPE_HEXBOARD, HexboardPrivate))


/* Transform the cell coordinates (I,J) to pixel-based coordinates of
   in the viewport widget. The output coordinates are written in
   OUTPUT_X and OUTPUT_Y, and they are the coordinates of the center
   of the hexagon in he screen. */
static void
cell_to_pixel (Hexboard * hexboard, int i, int j, int *output_x, int *output_y)
{
  HexboardPrivate * st = HEXBOARD_GET_PRIVATE (hexboard);
  int x;
  int y;
  x = i * 3 * st->cell_width/4 - j * 3 * st->cell_width/4;
  y = i * st->cell_height/2  + j * st->cell_height/2;
  y = -y;
  x += st->board_left + st->board_width/2;
  y += st->board_top + st->board_height - st->cell_height/2;
  *output_x = x;
  *output_y = y;
}

static inline float
norm (float x, float y)
{
  /* Law of cosines. The vectors of the hex basis make a 120 angle. */
  return x*x + y*y - x*y;
}

static inline void
nearest_cell (float x, float y, int *i, int *j)
{
  int p,q;
  int nearest_i;
  int nearest_j;
  float min_dist;
  min_dist = 2.0;
  for(p=0; p<=1; p++)
    {
      for(q=0; q<=1; q++)
        {
          int i = floor(x) + p;
          int j = floor(y) + q;
          float dist = norm (x-i, y-j);
          if (dist < min_dist)
            {
              nearest_i = i;
              nearest_j = j;
              min_dist = dist;
            }
        }
    }
  *i = nearest_i;
  *j = nearest_j;
}


/* Transform the point (X,Y) in pixel-based coordinates of the
   viewport widget to the coordinate of a cell in the board. The
   output cell coordinates are written to OUTPUT_I and OUTPUT_J. */
static void
pixel_to_cell (Hexboard * hexboard, int x, int y, int *output_i, int *output_j)
{
  HexboardPrivate * st = HEXBOARD_GET_PRIVATE (hexboard);
  float r, s;
  x -= st->board_left + st->board_width/2;
  y -= st->board_top + st->board_height - st->cell_height/2;
  r =  2*x / (3*st->cell_width) - y / st->cell_height;
  s = -2*x / (3*st->cell_width) - y / st->cell_height;
  nearest_cell (r, s, output_i, output_j);
}


static void
hexboard_init(Hexboard * hexboard)
{
  HexboardPrivate * state = HEXBOARD_GET_PRIVATE (hexboard);
  int size = 13;
  int i,j;
  state->size = size;
  state->border = 30;
  for (i=0; i<size; i++)
    {
      for (j=0; j<size; j++)
        {
          state->look[i][j].r = 1;
          state->look[i][j].g = 1;
          state->look[i][j].b = 1;
          state->look[i][j].border = 1;
        }
    }
  gtk_widget_add_events (GTK_WIDGET (hexboard), GDK_BUTTON_PRESS_MASK);
}

static void
hexboard_class_init(HexboardClass * klass)
{
  GtkWidgetClass *widget_class;
  GtkObjectClass *object_class;
  widget_class = (GtkWidgetClass *) klass;
  object_class = (GtkObjectClass *) klass;
  widget_class->expose_event = hexboard_expose;
  widget_class->configure_event = hexboard_configure;
  widget_class->button_press_event = hexboard_button_press;
  /* Signal definitions */
  g_signal_new("cell_clicked",
               G_TYPE_FROM_CLASS (klass),
               G_SIGNAL_RUN_FIRST,
               G_STRUCT_OFFSET (HexboardClass, cell_clicked), NULL, NULL,
               conn_marshal_VOID__INT_INT, G_TYPE_NONE,
               2, G_TYPE_INT, G_TYPE_INT);
  g_type_class_add_private (object_class, sizeof(HexboardPrivate));
}


static gboolean
hexboard_configure (GtkWidget * widget, GdkEventConfigure *event)
{
  Hexboard * hexboard = HEXBOARD (widget);
  HexboardPrivate * st = HEXBOARD_GET_PRIVATE (hexboard);
  int border = st->border;
  int n = st->size;
  GtkAllocation rect;
  float height;
  float width;
  float k;
  float radious;
  float apothem;
  gtk_widget_get_allocation (widget, &rect);
  rect.width -= 2 * st->border;
  rect.height -= 2 * st->border;
  width = 3*n - 1;
  height = n * sqrt(3);
  k = MIN (rect.width / width, rect.height / height);
  height *= k;
  width *= k;
  radious = width / (3*n - 1);
  apothem = height / (2*n);
  /* Set widget private data */
  st->board_width = width;
  st->board_height = height;
  st->board_left = st->border + (rect.width - st->board_width) / 2;
  st->board_top = st->border + (rect.height - st->board_height) / 2;
  st->cell_width = 2 * radious;
  st->cell_height = 2 * apothem;
  return TRUE;
}


/* static void
 * draw_hexagon (cairo_t *cr, double cx, double cy, double radious,
 *               double r, double g, double b, double border)
 * {
 *   int i;
 *   cairo_set_source_rgb (cr, 0, 0, 0);
 *   cairo_move_to (cr, cx + radious, cy);
 *   cairo_set_line_width (cr, border);
 *   for (i=1; i<6; i++)
 *     {
 *       double angle = 2*i*M_PI/6;
 *       cairo_line_to (cr, cx + radious*cos(angle), cy + radious*sin(angle));
 *     }
 *   cairo_close_path (cr);
 *   cairo_stroke_preserve (cr);
 *   cairo_set_source_rgb (cr, r, g, b);
 *   cairo_fill (cr);
 * } */

static void
stroke_hexagon (cairo_t *cr, double cx, double cy, double radious, double border)
{
  int i;
  cairo_set_line_width (cr, border);
  cairo_set_source_rgb (cr, 0, 0, 0);
  cairo_move_to (cr, cx + radious, cy);
  for (i=1; i<6; i++)
    {
      double angle = 2*i*M_PI/6;
      cairo_line_to (cr, cx + radious*cos(angle), cy + radious*sin(angle));
    }
  cairo_close_path (cr);
  cairo_stroke (cr);
}

static void
fill_hexagon (cairo_t *cr, double cx, double cy, double radious,
              double r, double g, double b)
{
  int i;
  cairo_set_source_rgb (cr, r, g, b);
  cairo_move_to (cr, cx + radious, cy);
  for (i=1; i<6; i++)
    {
      double angle = 2*i*M_PI/6;
      cairo_line_to (cr, cx + radious*cos(angle), cy + radious*sin(angle));
    }
  cairo_close_path (cr);
  cairo_fill (cr);
}


static void
draw_border_rd (Hexboard * hexboard, cairo_t * cr)
{
  const int border = HEXBOARD_BORDER_WIDTH;
  HexboardPrivate * st = HEXBOARD_GET_PRIVATE (hexboard);
  int n = st->size;
  int x;
  int y;
  int i;
  cell_to_pixel (hexboard, 0, 0, &x, &y);
  cairo_move_to (cr, x + st->cell_width/2, y);
  cairo_line_to (cr, x + st->cell_width/4, y + st->cell_height/2);
  cairo_line_to (cr, x, y + st->cell_height/2);
  cairo_line_to (cr, x, y + st->cell_height/2 + border);
  cairo_line_to (cr, x + st->cell_width/4 + border/2, y + st->cell_height/2 + border);
  cairo_line_to (cr, x + st->cell_width/2 + border/2, y + border);
  for (i=1; i<n; i++)
    {
      cell_to_pixel (hexboard, i, 0, &x, &y);
      cairo_line_to (cr, x + st->cell_width/4 + border/2, y + st->cell_height/2 + border);
      cairo_line_to (cr, x + st->cell_width/2 + border/2, y + border);
    }
  cell_to_pixel (hexboard, n-1, 0, &x, &y);
  cairo_line_to (cr, x + st->cell_width/2 + border, y);
  cairo_line_to (cr, x + st->cell_width/2, y);
  cairo_close_path (cr);
  cairo_set_source_rgb (cr, 0, 1, 0);
  cairo_fill (cr);
}

static void
draw_border_ld (Hexboard * hexboard, cairo_t * cr)
{
  const int border = HEXBOARD_BORDER_WIDTH;
  HexboardPrivate * st = HEXBOARD_GET_PRIVATE (hexboard);
  int n = st->size;
  int x;
  int y;
  int j;
  cell_to_pixel (hexboard, 0, 0, &x, &y);
  cairo_move_to (cr, x - st->cell_width/2, y);
  cairo_line_to (cr, x - st->cell_width/4, y + st->cell_height/2);
  cairo_line_to (cr, x, y + st->cell_height/2);
  cairo_line_to (cr, x, y + st->cell_height/2 + border);
  cairo_line_to (cr, x - st->cell_width/4 - border/2, y + st->cell_height/2 + border);
  cairo_line_to (cr, x - st->cell_width/2 - border/2, y + border);
  for (j=1; j<n; j++)
    {
      cell_to_pixel (hexboard, 0, j, &x, &y);
      cairo_line_to (cr, x - st->cell_width/4 - border/2, y + st->cell_height/2 + border);
      cairo_line_to (cr, x - st->cell_width/2 - border/2, y + border);
    }
  cell_to_pixel (hexboard, 0, n-1, &x, &y);
  cairo_line_to (cr, x - st->cell_width/2 - border, y);
  cairo_line_to (cr, x - st->cell_width/2, y);
  cairo_close_path (cr);
  cairo_set_source_rgb (cr, 1, 0, 0);
  cairo_fill (cr);
}

static void
draw_border_lu (Hexboard * hexboard, cairo_t * cr)
{
  const int border = HEXBOARD_BORDER_WIDTH;
  HexboardPrivate * st = HEXBOARD_GET_PRIVATE (hexboard);
  int n = st->size;
  int x;
  int y;
  int i;
  cell_to_pixel (hexboard, n-1, n-1, &x, &y);
  cairo_move_to (cr, x - st->cell_width/2, y);
  cairo_line_to (cr, x - st->cell_width/4, y - st->cell_height/2);
  cairo_line_to (cr, x, y - st->cell_height/2);
  cairo_line_to (cr, x, y - st->cell_height/2 - border);
  cairo_line_to (cr, x - st->cell_width/4 - border/2, y - st->cell_height/2 - border);
  cairo_line_to (cr, x - st->cell_width/2 - border/2, y - border);
  for (i=n-1; i>=0; i--)
    {
      cell_to_pixel (hexboard, i, n-1, &x, &y);
      cairo_line_to (cr, x - st->cell_width/4 - border/2, y - st->cell_height/2 - border);
      cairo_line_to (cr, x - st->cell_width/2 - border/2, y - border);
    }
  cell_to_pixel (hexboard, 0, n-1, &x, &y);
  cairo_line_to (cr, x - st->cell_width/2 - border, y);
  cairo_line_to (cr, x - st->cell_width/2, y);
  cairo_close_path (cr);
  cairo_set_source_rgb (cr, 0, 1, 0);
  cairo_fill (cr);
}

static void
draw_border_ru (Hexboard * hexboard, cairo_t * cr)
{
  const int border = HEXBOARD_BORDER_WIDTH;
  HexboardPrivate * st = HEXBOARD_GET_PRIVATE (hexboard);
  int n = st->size;
  int x;
  int y;
  int j;
  cell_to_pixel (hexboard, n-1, n-1, &x, &y);
  cairo_move_to (cr, x + st->cell_width/2, y);
  cairo_line_to (cr, x + st->cell_width/4, y - st->cell_height/2);
  cairo_line_to (cr, x, y - st->cell_height/2);
  cairo_line_to (cr, x, y - st->cell_height/2 - border);
  cairo_line_to (cr, x + st->cell_width/4 + border/2, y - st->cell_height/2 - border);
  cairo_line_to (cr, x + st->cell_width/2 + border/2, y - border);
  for (j=n-1; j>=0; j--)
    {
      cell_to_pixel (hexboard, n-1, j, &x, &y);
      cairo_line_to (cr, x + st->cell_width/4 + border/2, y - st->cell_height/2 - border);
      cairo_line_to (cr, x + st->cell_width/2 + border/2, y - border);
    }
  cell_to_pixel (hexboard, n-1, 0, &x, &y);
  cairo_line_to (cr, x + st->cell_width/2 + border, y);
  cairo_line_to (cr, x + st->cell_width/2, y);
  cairo_close_path (cr);
  cairo_set_source_rgb (cr, 1, 0, 0);
  cairo_fill (cr);
}


static void
draw_board (Hexboard * hexboard, cairo_t * cr, gint width, gint height)
{
  HexboardPrivate * st = HEXBOARD_GET_PRIVATE (hexboard);
  int n = st->size;
  int i, j;
  draw_border_rd (hexboard, cr);
  draw_border_ld (hexboard, cr);
  draw_border_lu (hexboard, cr);
  draw_border_ru (hexboard, cr);
  /* Cells */
  for (j=0; j<n; j++)
    {
      for (i=0; i<n; i++)
        {
          int x,y;
          cell_to_pixel (hexboard, i, j, &x, &y);
          fill_hexagon (cr, x, y, st->cell_width/2,
                        st->look[i][j].r,
                        st->look[i][j].g,
                        st->look[i][j].b);
        }
    }
  for (j=0; j<n; j++)
    {
      for (i=0; i<n; i++)
        {
          int x,y;
          cell_to_pixel (hexboard, i, j, &x, &y);
          stroke_hexagon (cr, x, y, st->cell_width/2, st->look[i][j].border);
        }
    }
}

static gboolean
hexboard_button_press (GtkWidget * widget, GdkEventButton * event)
{
  Hexboard * hexboard = HEXBOARD (widget);
  HexboardPrivate * st = HEXBOARD_GET_PRIVATE(hexboard);
  int x = event->x;
  int y = event->y;
  int i, j;
  pixel_to_cell (hexboard, x, y, &i, &j);
  g_signal_emit_by_name (widget, "cell_clicked", i, j);
  return FALSE;
}


static gboolean
hexboard_expose(GtkWidget * widget, GdkEventExpose *event)
{
  Hexboard * hexboard = HEXBOARD (widget);
  HexboardPrivate * st = HEXBOARD_GET_PRIVATE (hexboard);
  cairo_t *cr;
  gint width, height;
  gdk_window_get_size (GTK_WIDGET(hexboard)->window, &width, &height);
  cr = gdk_cairo_create (GTK_WIDGET(hexboard)->window);
  /* Clip expose region */
  cairo_rectangle (cr, event->area.x, event->area.y, event->area.width, event->area.height);
  cairo_clip(cr);
  /* Background */
  cairo_set_source_rgb (cr, .8,.8,.8);
  cairo_paint (cr);
  /* Board */
  draw_board (hexboard, cr, width, height);
  cairo_destroy(cr);
  return TRUE;
}

GtkWidget *
hexboard_new (void)
{
  GtkWidget * widget;
  widget = g_object_new (TYPE_HEXBOARD, NULL);
  return widget;
}

size_t
hexboard_get_size (Hexboard * hex)
{
  HexboardPrivate * st = HEXBOARD_GET_PRIVATE (hex);
  return st->size;
}

gboolean
hexboard_cell_get_color (Hexboard * board, gint i, gint j, double *r, double *g, double *b)
{
  HexboardPrivate * st = HEXBOARD_GET_PRIVATE (board);
  if (i < 0 || i >= st->size || j < 0 || j >= st->size)
    return FALSE;
  *r = st->look[i][j].r;
  *g = st->look[i][j].g;
  *b = st->look[i][j].b;
  return TRUE;
}

gboolean
hexboard_cell_set_color (Hexboard * board, gint i, gint j, double r, double g, double b)
{
  HexboardPrivate * st = HEXBOARD_GET_PRIVATE (board);
  if (i < 0 || i >= st->size || j < 0 || j >= st->size)
    return FALSE;
  st->look[i][j].r = r;
  st->look[i][j].g = g;
  st->look[i][j].b = b;
  gtk_widget_queue_draw (GTK_WIDGET (board));
  return TRUE;
}

gboolean
hexboard_cell_get_border (Hexboard * board, gint i, gint j, double * border)
{
  HexboardPrivate * st = HEXBOARD_GET_PRIVATE (board);
  if (i < 0 || i >= st->size || j < 0 || j >= st->size)
    return FALSE;
  *border = st->look[i][j].border;
  return TRUE;
}

gboolean
hexboard_cell_set_border (Hexboard * board, gint i, gint j, double border)
{
  HexboardPrivate * st = HEXBOARD_GET_PRIVATE (board);
  if (i < 0 || i >= st->size || j < 0 || j >= st->size)
    return FALSE;
  st->look[i][j].border = border;
  gtk_widget_queue_draw (GTK_WIDGET (board));
  return TRUE;
}


gboolean
hexboard_save_as_image (Hexboard * hex, const char * filename, const char * type,
                        guint width, guint height)
{
  cairo_t *cr;
  cairo_surface_t * surf;
  if (!strcasecmp (type, "pdf"))
    surf = cairo_pdf_surface_create (filename, width, height);
  else if (!strcasecmp (type, "svg"))
    surf = cairo_svg_surface_create (filename, width, height);
  else if (!strcasecmp (type, "png"))
    surf = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, width, height);
  else
    surf = NULL;

  if (surf == NULL)
    return FALSE;

  cr = cairo_create (surf);
  draw_board (hex, cr, width, height);
  cairo_surface_flush (surf);

  if (!strcasecmp (type, "png"))
    cairo_surface_write_to_png (surf, filename);

  cairo_surface_destroy(surf);
  cairo_destroy(cr);
  return TRUE;
}


/* conn-hex-widget.c ends here */
