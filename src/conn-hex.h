/* conn-hex.h --- Hex game logic (Header) */

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

#ifndef CONN_HEX_H
#define CONN_HEX_H

#include "utils.h"
#include <stdlib.h>
#include <stdio.h>

typedef struct hex_s * hex_t;

typedef enum {
  HEX_SUCCESS,
  HEX_BUSY_CELL,
  HEX_END_OF_GAME,
  HEX_INVALID_CELL
} hex_status_t;

/* Construction and destruction */
hex_t hex_new (size_t size);
size_t hex_size (hex_t hex);
void hex_reset (hex_t hex);
void hex_free (hex_t hex);

/* History */
unsigned int hex_history_jump (hex_t hex, unsigned int n);
unsigned int hex_history_current (hex_t hex);
unsigned int hex_history_size (hex_t hex);
void hex_truncate_history (hex_t hex);
boolean hex_history_last_move (hex_t hex, uint *i, uint *j);

/* Gaming */
hex_status_t hex_move (hex_t hex, uint i, uint j);
int hex_get_player (hex_t hex);
boolean hex_end_of_game_p (hex_t hex);

/* Load/Save */
boolean hex_save_sgf (hex_t hex, char * filename);

/* Examining the board */
int hex_cell_player        (hex_t hex, uint i, uint j);
int hex_cell_busy_p        (hex_t hex, uint i, uint j);
int hex_cell_free_p        (hex_t hex, uint i, uint j);
int hex_cell_player1_p     (hex_t hex, uint i, uint j);
int hex_cell_player2_p     (hex_t hex, uint i, uint j);
int hex_cell_a_connected_p (hex_t hex, uint i, uint j);
int hex_cell_z_connected_p (hex_t hex, uint i, uint j);

#endif  /* CONN_HEX_H */

/* conn-hex.h ends here */
