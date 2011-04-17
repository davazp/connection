/* conn-xmpp.c --- XMPP Multiplayer support */

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
#include "utils.h"
#include <stdlib.h>
#include <string.h>
#include <loudmouth/loudmouth.h>

#define CONN_XMPP_RESOURCE "CONN"

/* Keep a hash table with connected users. */
static GHashTable * user_table;

/* Internal data types and functions to keep XMPP User list. */
typedef struct xmpp_user_s
{
  const char * name;
  const char * resource;
  enum { OFFLINE, ONLINE } status;
} *xmpp_user_t;

static void
xmpp_user_destroy (gpointer data)
{
  xmpp_user_t user = (xmpp_user_t)data;
  if (data != NULL)
    {
      g_free ((void*)user->name);
      g_free ((void*)user->resource);
    }
  g_free (data);
}

static void
initialize_user_table (void)
{
  user_table = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, xmpp_user_destroy);
}

static void
destroy_user_table (void)
{
  g_hash_table_destroy (user_table);
}

/* Lookup an user in the user table. */
static xmpp_user_t
lookup_user (const char * jid)
{
  return g_hash_table_lookup (user_table, jid);
}

/* Lookup an user in the user table. If there is not exist, then
   insert an entry for JID and return it. */
static xmpp_user_t
intern_user (const char * jid)
{
  xmpp_user_t user;
  user = g_hash_table_lookup (user_table, jid);
  if (user == NULL)
    {
      user = g_malloc (sizeof(struct xmpp_user_s));
      user->name = NULL;
      user->resource = NULL;
      g_hash_table_insert (user_table, g_strdup (jid), user);
    }
  return user;
}



/* Keep the connection descriptor to use in Loudmouth functions. */
static LmConnection * connection;

static LmHandlerResult
xmpp_presence_callback (LmMessageHandler * handler, LmConnection * connection,
                       LmMessage * message, gpointer user_data)
{
  LmMessageNode * node = message->node;
  xmpp_user_t user;
  gchar * jid;
  gchar * resource = NULL;
  gchar * delimit;
  jid = g_strdup (lm_message_node_get_attribute (node, "from"));
  delimit = strchr (jid, '/');
  if (delimit != NULL)
    {
      *delimit = '\0';
      resource = delimit+1;
    }
  user = intern_user (jid);
  user->resource = g_strdup (resource);
  user->status = ONLINE;
  g_print ("%s (%s), %s\n", jid, user->name, user->resource);
  g_free (jid);
}

static LmHandlerResult
xmp_message_callback (LmMessageHandler * handler, LmConnection * connection,
                      LmMessage * message, gpointer user_data)
{
  /* nothing yet */
}



void
xmpp_init (void)
{
  LmMessageHandler * handler;
  initialize_user_table();
  connection = lm_connection_new (NULL);
  handler = lm_message_handler_new (xmpp_presence_callback, NULL, NULL);
  lm_connection_register_message_handler (connection, handler, LM_MESSAGE_TYPE_PRESENCE, 0);
}

boolean
xmpp_connect (const char *user, const char * passwd, const char * server, unsigned short port)
{

  GError * error;
  LmMessage * m;
  LmMessage * reply;
  LmMessageNode * query;
  LmMessageNode * item;
  gboolean success;

  lm_connection_set_server (connection, server);
  lm_connection_set_port (connection, port);

  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_INFO, _("Connecting to %s..."), server);
  success = lm_connection_open_and_block (connection, &error);
  if (!success) goto error;

  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_INFO, _("Authenticating with JID %s..."), user);
  success = lm_connection_authenticate_and_block (connection, user, passwd, CONN_XMPP_RESOURCE, &error);
  if (!success) goto error;
  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_INFO, _("Connected successfully."), user);

  /* Fetch roster */
  m = lm_message_new_with_sub_type (NULL, LM_MESSAGE_TYPE_IQ, LM_MESSAGE_SUB_TYPE_GET);
  query = lm_message_node_add_child (m->node, "query", NULL);
  lm_message_node_set_attributes (query, "xmlns", "jabber:iq:roster", NULL);
  reply = lm_connection_send_with_reply_and_block (connection, m, &error);
  query = lm_message_node_get_child (reply->node, "query");
  item  = lm_message_node_get_child (query, "item");
  while (item)
    {
      xmpp_user_t user;
      const gchar *jid, *name;
      jid = lm_message_node_get_attribute (item, "jid");
      name = lm_message_node_get_attribute (item, "name");
      user = intern_user (jid);
      user->name = g_strdup (name);
      user->status = OFFLINE;
      item = item->next;
    }
  lm_message_unref (reply);
  lm_message_unref (m);

  m = lm_message_new_with_sub_type (NULL, LM_MESSAGE_TYPE_PRESENCE, LM_MESSAGE_SUB_TYPE_AVAILABLE);
  lm_connection_send(connection, m, NULL);
  lm_message_unref (m);

  return TRUE;

  /* Error handling */
 error:
  g_message ("%s", error->message);
  return FALSE;
}


void
xmpp_disconnect (void)
{
  lm_connection_close (connection, NULL);
  lm_connection_unref (connection);
}


/* conn-xmpp.c ends here */
