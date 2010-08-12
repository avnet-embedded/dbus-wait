/*
 * Copyright (C) 2008 Intel Corporation.
 *
 * Author: Ross Burton <ross@linux.intel.com>
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <dbus/dbus.h>

static char *interface, *member, *path;

static void
alarm_handler (int signum)
{
  fprintf(stderr, "Timeout waiting for %s.%s\n", interface, member);
  exit (EXIT_SUCCESS);
}

static DBusHandlerResult
filter (DBusConnection *conn, DBusMessage *message, void *user_data)
{
  /* If the interface matches */
  if (strcmp (interface, dbus_message_get_interface (message)) == 0)
    /* And the member matches */
    if (strcmp (member, dbus_message_get_member (message)) == 0)
      /* And the path is NULL or matches */
      if (path == NULL || strcmp (path, dbus_message_get_path (message)) == 0)
        /* Then exit */
        exit (EXIT_SUCCESS);
  
  return DBUS_HANDLER_RESULT_HANDLED;
}

int
main (int argc, char **argv)
{
  DBusError error = DBUS_ERROR_INIT;
  DBusConnection *conn;
  char *match = NULL;

  if (argc < 3 || argc > 4) {
    fprintf (stderr, "$ dbus-watch <INTERFACE> <MEMBER> [PATH]\n");
    return EXIT_FAILURE;
  }

  signal (SIGALRM, alarm_handler);
  alarm (60);

  /* TODO: allow system or session */
  
  conn = dbus_bus_get (DBUS_BUS_SYSTEM, &error); 
  if (!conn) {
    fprintf (stderr, "Cannot get system bus: %s\n", error.message);
    dbus_error_free (&error);
    return EXIT_FAILURE;
  }

  switch (argc) {
  case 3:
    interface = argv[1];
    member = argv[2];
    path = NULL;
    asprintf (&match, "type='signal',interface='%s',member='%s'", interface, member);
    break;
  case 4:
    interface = argv[1];
    member = argv[2];
    path = argv[3];
    asprintf (&match, "type='signal',interface='%s',member='%s',path='%s'", interface, member, path);
    break;
  }

  dbus_bus_add_match (conn, match, &error);
  free (match);
  if (dbus_error_is_set (&error)) {
    fprintf (stderr, "Cannot add match: %s\n", error.message);
    dbus_error_free (&error);
    return EXIT_FAILURE;
  }

  dbus_connection_add_filter (conn, filter, NULL, NULL);
  
   while (dbus_connection_read_write_dispatch (conn, -1))
     ;
   
   dbus_connection_unref (conn);

   return EXIT_SUCCESS;
}
