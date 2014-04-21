/*
 * Copyright (c) 2008-2014 Lu, Chao-Ming (Tetralet).  All rights reserved.
 *
 * This file is part of LilyTerm.
 *
 * LilyTerm is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * LilyTerm is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with LilyTerm.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <gtk/gtk.h>
// for g_get_tmp_dir()
#include <glib.h>
// for L10n
#include <locale.h>
#include <glib/gi18n.h>
// for exit()
#include <stdlib.h>
// for socket()
#include <sys/un.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "lilyterm.h"

// the max size of saddr.sun_path in Linux is 108!
#define UNIX_PATH_MAX 108

#ifdef UNIT_TEST
int fake_main(int argc, char *argv[]);
#endif

gboolean set_fd_non_block(gint *fd);
gboolean init_socket_server();
gboolean accept_socket(GIOChannel *source, GIOCondition condition, gpointer user_data);
gboolean read_socket(GIOChannel *channel, GIOCondition condition, gpointer user_data);
gboolean socket_fault(int i, GError *error, GIOChannel* channel, gboolean unref);
gboolean clear_channel(GIOChannel* channel, gboolean unref);
gint shutdown_socket_server(gpointer data);
gchar *get_locale_list();
