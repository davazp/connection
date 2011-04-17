/* conn-xmpp.h --- XMPP Multiplayer support (Header) */

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

#ifndef CONN_XMPP_H
#define CONN_XMPP_H

void xmpp_init (void);
boolean xmpp_connect (const char *user, const char * passwd, const char * server, unsigned short port)
void xmpp_disconnect (void);

#endif  /* CONN_XMPP_H */

/* conn-xmpp.h ends here */
