/*
 * Copyright (c) 2016 Lu, Chao-Ming (Tetralet).  All rights reserved.
 *
 * GtkSocket is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * GTK_Socket is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GtkSocket.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SOCKET_H
#define SOCKET_H
typedef enum {
	UNIX_SOCKET_SERVER_INITED,	// 0
	UNIX_SOCKET_DATA_SENT,		// 1
	UNIX_SOCKET_ERROR,		// 2
} Unix_Socket_States;
#endif

Unix_Socket_States init_gtk_socket(gchar *package_name, gchar *socket_str, GSourceFunc read_function);

