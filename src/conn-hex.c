/* conn-hex.c --- Hex game logic */

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

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "conn-utils.h"
#include "conn-hex.h"

struct hex_cell_s {
  unsigned int a_connected : 1;
  unsigned int z_connected : 1;
  unsigned int player : 2;
};

typedef int history_entry[2];
struct hex_s
{
  size_t size;
  boolean end_of_game_p;
  int player;
  struct hex_cell_s * board;
  /* History */
  unsigned int history_size;
  unsigned int history_count;
  history_entry *history;
};

typedef unsigned char hex_cell_t;

/* Static functions */
static void expand_a_connection (hex_t hex, uint i, uint j);
static void expand_z_connection (hex_t hex, uint i, uint j);
static void recompute_connection (hex_t hex);
static hex_status_t hex_put_cell (hex_t hex, int player, uint i, uint j);

/* Macro to easy board access */
#define CELL(hex,i,j) ((hex)->board[(j)*((hex)->size) + (i)])

/* Check if there is a (i,j)-cell in the board HEX. */
#define IN_BOARD_P(hex,i,j) (i>=0 && i<(hex)->size && j>=0 && j<=(hex)->size)


/* Construction and destruction */
hex_t
hex_new (size_t size)
{
  hex_t hex;
  hex = conn_malloc (sizeof(struct hex_s));
  hex->board = conn_malloc (size*size * sizeof(struct hex_cell_s));
  hex->history = conn_malloc (sizeof(history_entry) * size);
  hex->size = size;
  hex_reset (hex);
  return hex;
}

void
hex_reset (hex_t hex)
{
  size_t size = hex->size;
  memset (hex->board, 0, sizeof(struct hex_cell_s) * size * size);
  memset (hex->history, 0, sizeof(history_entry) * size);
  hex->player = 1;
  hex->end_of_game_p = 0;
  hex->history_size  = 0;
  hex->history_count = 0;
}

size_t
hex_size (hex_t hex)
{
  return hex->size;
}

void
hex_free (hex_t hex)
{
  conn_free (hex->board);
  conn_free (hex);
}

/* History */

boolean
hex_history_backward (hex_t hex)
{
  unsigned int count;
  int i, j;
  if (hex->history_count == 0)
    return FALSE;
  hex->history_count--;
  count = hex->history_count;
  i = hex->history[count][0];
  j = hex->history[count][1];
  CELL(hex,i,j).player = 0;
  hex->player = hex->player%2 + 1;
  recompute_connection (hex);
  return TRUE;
}

boolean
hex_history_forward (hex_t hex)
{
  unsigned int count = hex->history_count;
  int i, j;
  if (hex->history_count == hex->history_size)
    return FALSE;
  i = hex->history[count][0];
  j = hex->history[count][1];
  hex_put_cell (hex, hex->player, i, j);
  hex->player = hex->player%2 + 1;
  hex->history_count++;
  return TRUE;
}

unsigned int
hex_history_size (hex_t hex)
{
  return hex->history_size;
}

unsigned int
hex_history_count (hex_t hex)
{
  return hex->history_count;
}


/* Examining the board */

int
hex_cell_player (hex_t hex, uint i, uint j)
{
  if (IN_BOARD_P(hex,i,j))
    return CELL(hex, i, j).player;
  else
    return -1;
}


int
hex_cell_player1_p (hex_t hex, uint i, uint j)
{
  if (IN_BOARD_P(hex,i,j))
    return CELL(hex, i, j).player == 1;
  else
    return -1;
}

int
hex_cell_player2_p (hex_t hex, uint i, uint j)
{
  if (IN_BOARD_P(hex,i,j))
    return CELL(hex,i,j).player == 2;
  else
    return -1;
}

int
hex_cell_a_connected_p (hex_t hex, uint i, uint j)
{
  if (IN_BOARD_P(hex, i, j) && CELL(hex,i,j).player != 0)
    return CELL(hex,i,j).a_connected;
  else
    return -1;
}

int
hex_cell_z_connected_p (hex_t hex, uint i, uint j)
{
  if (IN_BOARD_P(hex, i, j) && CELL(hex,i,j).player != 0)
    return CELL(hex,i,j).z_connected;
  else
    return -1;
}

int
hex_cell_busy_p (hex_t hex, uint i, uint j)
{
  if (IN_BOARD_P(hex, i, j))
    return CELL(hex,i,j).player != 0;
  else
    return -1;
}

int
hex_cell_free_p (hex_t hex, uint i, uint j)
{
  if (IN_BOARD_P(hex, i, j))
    return CELL(hex, i, j).player == 0;
  else
    return -1;
}


/* Gaming */
boolean
hex_end_of_game_p (hex_t hex)
{
  return hex->end_of_game_p;
}

int
hex_get_player (hex_t hex)
{
  return hex->player;
}

static void
expand_a_connection (hex_t hex, uint i, uint j)
{
  int neighbors[6][2] = {{+1, 0}, {+1, +1}, {0, +1},
                         {-1, 0}, {-1, -1}, {0, -1}};
  int t;
  int player;
  player = CELL(hex,i,j).player;
  assert (player != 0);
  if (!CELL(hex, i, j).a_connected)
    return;

  for (t=0; t<6; t++)
    {
      int i1 = i + neighbors[t][0];
      int j1 = j + neighbors[t][1];
      if (IN_BOARD_P (hex, i1, j1) && CELL(hex,i1, j1).player == player
          && !CELL(hex,i1, j1).a_connected)
        {
          CELL(hex,i1, j1).a_connected = 1;
          expand_a_connection (hex, i1, j1);
        }
    }
}


static void
expand_z_connection (hex_t hex, uint i, uint j)
{
  int neighbors[6][2] = {{+1, 0}, {+1, +1}, {0, +1},
                         {-1, 0}, {-1, -1}, {0, -1}};
  int t;
  int player;
  player = CELL(hex,i,j).player;
  assert (player != 0);
  if (!CELL(hex,i, j).z_connected)
    return;

  for (t=0; t<6; t++)
    {
      int i1 = i + neighbors[t][0];
      int j1 = j + neighbors[t][1];
      if (IN_BOARD_P (hex, i1, j1) && CELL(hex,i1, j1).player == player
          && !CELL(hex,i1, j1).z_connected)
        {
          CELL(hex,i1, j1).z_connected = 1;
          expand_z_connection (hex, i1, j1);
        }
    }
}

/* Recompute all connection properties for a board. It is required
   when you remove some information from the board. For example, after
   a `undo'. */
static void
recompute_connection (hex_t hex)
{
  size_t size = hex->size;
  int i,j;
  /* Clean previous connection properties */
  for (j=0; j<size; j++)
    {
      for (i=0; i<size; i++)
        CELL(hex,i,j).a_connected =
          CELL(hex,i,j).z_connected = 0;
    }
  /* a-connection for player 1 */
  for (i=0; i<size; i++)
    {
      if (CELL(hex,i,0).player == 1)
        {
          CELL(hex,i,0).a_connected = 1;
          expand_a_connection (hex, i, 0);
        }
    }
  /* z-connection for player 1 */
  for (i=0; i<size; i++)
    {
      if (CELL(hex,i,size-1).player == 1)
        {
          CELL(hex,i,size-1).z_connected = 1;
          expand_z_connection (hex, i, size-1);
        }
    }
  /* a-connection for player 2 */
  for (j=0; j<size; j++)
    {
      if (CELL(hex,0,j).player == 2)
        {
          CELL(hex,0,j).a_connected = 1;
          expand_a_connection (hex, 0, j);
        }
    }
  /* z-connection for player 2 */
  for (j=0; j<size; j++)
    {
      if (CELL(hex,size-1,j).player == 2)
        {
          CELL(hex,size-1,j).z_connected = 1;
          expand_z_connection (hex, size-1, j);
        }
    }
}



static hex_status_t
hex_put_cell (hex_t hex, int player, uint i, uint j)
{
  if (! IN_BOARD_P (hex, i, j) )
    return HEX_INVALID_CELL;

  if (hex->end_of_game_p)
    return HEX_END_OF_GAME;

  if (hex_cell_free_p(hex, i, j))
    {
      int player = hex->player;
      int size = hex->size;
      int neighbors[6][2] = {{+1, 0}, {+1, +1}, {0, +1},
                             {-1, 0}, {-1, -1}, {0, -1}};
      int t;
      CELL(hex,i,j).player = player;
      /* Borders are connected for each user. */
      if (player == 1)
        {
          CELL(hex,i,j).a_connected = (j == 0);
          CELL(hex,i,j).z_connected = (j == size-1);
        }
      else
        {
          CELL(hex,i,j).a_connected = (i == 0);
          CELL(hex,i,j).z_connected = (i == size-1);
        }
      /* Inherit connection properties. If some adjacent cell is
         a-connected (resp. z-connected), then the cell is a-connected
         (resp. z-connected). */
      for (t=0; t<6; t++)
        {

          int di = neighbors[t][0];
          int dj = neighbors[t][1];
          int i1 = i+di;
          int j1 = j+dj;
          if (IN_BOARD_P (hex, i1, j1) && CELL(hex, i1, j1).player == player)
            {
              CELL(hex, i, j).a_connected |= CELL(hex, i1, j1).a_connected;
              CELL(hex, i, j).z_connected |= CELL(hex, i1, j1).z_connected;
            }
        }
      /* Propagate connection properties */
      expand_a_connection (hex, i, j);
      expand_z_connection (hex, i, j);

      /* Check game over */
      if (CELL(hex,i,j).a_connected && CELL(hex,i,j).z_connected)
        hex->end_of_game_p = 1;

      return HEX_SUCCESS;
    }
  else
    {
      return HEX_BUSY_CELL;
    }
}

hex_status_t
hex_move (hex_t hex, uint i, uint j)
{
  hex_status_t status;
  unsigned int count;
  status = hex_put_cell (hex, hex->player, i, j);
  hex->player = hex->player%2 + 1;
  /* Truncate the 'future' history */
  if (hex->history_count < hex->history_size)
    recompute_connection (hex);
  count = hex->history_count;
  hex->history[count][0] = i;
  hex->history[count][1] = j;
  hex->history_size = ++hex->history_count;
  return status;
}


/* conn-hex.c ends here */
