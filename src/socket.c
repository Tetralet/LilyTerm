/*
 * Copyright (c) 2008-2016 Lu, Chao-Ming (Tetralet).  All rights reserved.
 *
 * GtkSocket is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * GtkSocket is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GtkSocket.  If not, see <http://www.gnu.org/licenses/>.
 */

// The Main Function Usage:
//
// Unix_Socket_States init_gtk_socket(gchar *package_name, gchar *socket_str, GSourceFunc read_function)
//
// package_name: The uniq name of package.
//		 The socket server will create an unix soket file at /tmp/.PACKAGENAME_USERNAME:DISPLAY
// socket_str: A string will be sent to the socket server, if it exist.
// read_function: A function that will headle the socket_str as it's the socket server.
//		  gboolean read_function(gchar *socket_str)
//
// Example:
//	if (init_gtk_socket(PACKAGE, socket_str, (GSourceFunc)read_function) == UNIX_SOCKET_DATA_SENT)
//		/* Data is sent. Quiting... */
//
// return: UNIX_SOCKET_SERVER_INITED: had been inited as a unix socket server.
//	   UNIX_SOCKET_DATA_SENT: the data had been sent to an unix socket server.
//	   UNIX_SOCKET_ERROR: error occured.

//
// init_gtk_socket() -----------------(N)------------------------------------> return FALSE
//	|				^					^
//   init_socket_data()			|					|
//   socket_fd -------------------(N)---|					|
//   set_fd_non_block ------------(N)---/					|
//	|									|
// query_socket()								|
//   connect ---------------------(N)-------> init_socket_server()--------------|
//	|				^	unlink				|
//   send_socket()			|	bind -----------------------(N)-|
// * g_io_channel_unix_new -------(N)---|	listen ---------------------(N)-|
//   g_io_channel_set_encoding ---(N)---|     * g_io_channel_unix_new ------(N)-|
//   g_io_channel_set_buffered		|	g_io_channel_set_encoding --(N)-|
//   g_io_channel_write_chars ----(N)---|	g_io_add_watch -------------(N)-/
//   g_io_channel_flush ----------(N)---/	|
// + clear_channel()				|---- accept_socket()
//     g_io_channel_shutdown			|	(condition)
//     (g_io_channel_unref)			|	accept
//      |					|	set_fd_non_block
//   exit()					|     * g_io_channel_unix_new
//						|	g_io_channel_set_encoding
//						|	g_io_add_watch
//						|		|
//						|		`-- read_socket()
//						|		    g_io_channel_read_line
//						|		    [READ_FUNC]
//						|		  + clear_channel()
//						|		    g_io_channel_shutdown
//						|		    (g_io_channel_unref)
//						\---- g_atexit()
//							shutdown_socket_server()
//							  unlink
//							+ clear_channel()
//							  g_io_channel_shutdown
//							  (g_io_channel_unref)


#include <gtk/gtk.h>

// for exit()
#include <stdlib.h>
// for socket()
#include <sys/un.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "socket.h"

// the max size of saddr.sun_path in Linux is 108!
#define UNIX_PATH_MAX 108
# define MAX_BACKLOG 100

typedef enum {
	GTK_SOCKET_ERROR_CREATE_SOCKET_FILE,
	GTK_SOCKET_ERROR_CONNECT_SOCKET,
	GTK_SOCKET_ERROR_BIND_SOCKET,
	GTK_SOCKET_ERROR_LISTEN_SOCKET,
	GTK_SOCKET_ERROR_WATCH_SOCKET,
	GTK_SOCKET_ERROR_ACCEPT_REQUEST,
	GTK_SOCKET_ERROR_RECEIVE_DATA,
	GTK_SOCKET_ERROR_FCNTL,
	GTK_SOCKET_ERROR_CHANNEL_ENCODING,
	GTK_SOCKET_ERROR_SHUTDOWN_CHANNEL,
	GTK_SOCKET_ERROR_WRITE_CHANNEL,
	GTK_SOCKET_ERROR_CREATE_CHANNEL,
	GTK_SOCKET_ERROR_WRITE_BUFFER,
	GTK_SOCKET_ERROR_GOT_NULL_STRING,
	GTK_SOCKET_ERROR_TMP_DIR,
} Unix_Socket_Error;

gboolean init_socket_fd();
gboolean init_socket_data(gchar *package_name);
gboolean set_fd_non_block(gint *fd);
gboolean query_socket();
gboolean send_socket(gchar *socket_str);
gboolean clear_channel(GIOChannel *channel, gboolean unref);
gboolean init_socket_server(GSourceFunc read_function);
gboolean accept_socket(GIOChannel *source, GIOCondition condition, GSourceFunc read_function);
gboolean read_socket(GIOChannel *channel, GIOCondition condition, GSourceFunc read_function);
gint shutdown_socket_server(gpointer data);
gboolean socket_fault(Unix_Socket_Error type, GError *error, GIOChannel *channel, gboolean unref);

// for unix socket
gint socket_fd = 0;
struct sockaddr_un address = {0};
int address_len = -1;
GIOChannel *main_channel = NULL;

#if ! GLIB_CHECK_VERSION(2,5,0)
	// SICNE glib-2.5.0/glib/gmessages.h:g_debug()
	#define g_debug(...) g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, __VA_ARGS__)
#endif
#if ! GTK_CHECK_VERSION(2,23,0)
	// END: gtk+-2.23.0/gtk/gtkmain.h: gtk_quit_add() GTK_DISABLE_DEPRECATED
	#define g_atexit(x) gtk_quit_add(0, (GtkFunction)x, NULL)
#endif
#if GLIB_CHECK_VERSION(2,31,2)
	// END: glib-2.31.2/glib/gutils.h: g_atexit () GLIB_DEPRECATED
	#define g_atexit atexit
#endif

// it will return UNIX_SOCKET_SERVER_INITED if succeed
Unix_Socket_States init_gtk_socket(gchar *package_name, gchar *socket_str, GSourceFunc read_function)
{
#ifdef DETAIL
	g_debug("! Launch init_gtk_socket() with package_name = %s, socket_str = %s", package_name, socket_str);
#endif
	// if Socket Server call init_gtk_socket() again...
	if (address_len > 0)
	{
		if (init_socket_fd() && query_socket() && (send_socket(socket_str)))
			return UNIX_SOCKET_DATA_SENT;
	}

	// init socket data
	if (init_socket_data(package_name))
	{
		// trying to connect to an existing Socket Server
		if (query_socket())
		{
			// success, sent the socket_str then quit
			if (send_socket(socket_str))
			{
				exit (0);
				return UNIX_SOCKET_DATA_SENT;
			}
		}

		// no socket server exist. create a socket server
		if (init_socket_server(read_function))
		{
			g_atexit((GVoidFunc)shutdown_socket_server);
			return UNIX_SOCKET_SERVER_INITED;
		}
	}

	return UNIX_SOCKET_ERROR;
}

gboolean init_socket_fd()
{
#ifdef DETAIL
	g_debug("! Launch init_socket_fd()");
#endif
	GError *error = NULL;
	// init socket
	socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (socket_fd < 0)
		// main_channel is NULL, so that we don't need to launch clear_channel()
		return socket_fault(GTK_SOCKET_ERROR_CREATE_SOCKET_FILE, error, NULL, FALSE);
	return TRUE;
}

// it will return TRUE if init socket data successfully
gboolean init_socket_data(gchar *package_name)
{
#ifdef DETAIL
	g_debug("! Launch init_socket_data() to init %s socket!", PACKAGE);
#endif
	GError *error = NULL;

	// clean data first
	bzero(&address, sizeof(address));
	// init the address of socket
	address.sun_family = AF_UNIX;

	const gchar *tmp_dir = g_get_tmp_dir();

	if (tmp_dir)
		g_snprintf(address.sun_path, UNIX_PATH_MAX, "%s/.%s_%s%s",
			   tmp_dir ,package_name, g_get_user_name(),
			   gdk_display_get_name(gdk_display_get_default()));
	else
		return socket_fault(GTK_SOCKET_ERROR_TMP_DIR, error, NULL, FALSE);

	address.sun_path[UNIX_PATH_MAX-1] = address.sun_path[UNIX_PATH_MAX-2] = '\0';
	// g_debug("The socket file is %s", address.sun_path);
	address_len = sizeof(address);

	if (! init_socket_fd()) return FALSE;

	// set this socket is non-block
	return set_fd_non_block(&socket_fd);
}


// it will return TRUE if scucceed
gboolean set_fd_non_block(gint *fd)
{
#ifdef DETAIL
	if (fd)
		g_debug("! Launch set_fd_non_block() with fd = %d!", *fd);
	else
		g_debug("! Launch set_fd_non_block() with fd = (%p)!", fd);
#endif
#ifdef SAFEMODE
	if (fd==NULL) return FALSE;
#endif
	GError *error = NULL;
	gint flags = fcntl(*fd, F_GETFL, 0);
	if (fcntl(*fd, F_SETFL, O_NONBLOCK|flags) < 0)
		// main_channel is NULL, so that we don't need to launch clear_channel()
		return socket_fault(GTK_SOCKET_ERROR_FCNTL, error, NULL, FALSE);
	return TRUE;
}


// it will return TRUE if scucceed
gboolean query_socket()
{
#ifdef DETAIL
	g_debug("! Launch query_socket() to connect to an existing %s !", PACKAGE);
#endif
	GError *error = NULL;

	if (connect(socket_fd, (struct sockaddr *)&address, address_len) < 0)
		// main_channel is NULL, so that we don't need to launch clear_channel()
		return socket_fault(GTK_SOCKET_ERROR_CONNECT_SOCKET, error, NULL, FALSE);
	return TRUE;
}


// it will return TRUE if scucceed
gboolean send_socket(gchar *socket_str)
{
#ifdef DETAIL
	g_debug("! Launch send_socket() to send data to the exiting %s with socket_str = \"%s\"!", PACKAGE, socket_str);
#endif
	// write data!
#ifdef SAFEMODE
	if (fcntl(socket_fd, F_GETFL) < 0) return FALSE;
#endif
	GError *error = NULL;
	GIOChannel *channel = g_io_channel_unix_new(socket_fd);
	gsize len;

	// main_channel is NULL, so that we don't need to launch clear_channel()
	if (!channel) return socket_fault(GTK_SOCKET_ERROR_CREATE_CHANNEL, NULL, NULL, FALSE);

	// set the channel to read binary file
	if (g_io_channel_set_encoding(channel, NULL, &error) == G_IO_STATUS_ERROR)
		return socket_fault(GTK_SOCKET_ERROR_CHANNEL_ENCODING, error, channel, TRUE);

	g_io_channel_set_buffered (channel, FALSE);

	if (g_io_channel_write_chars(channel, socket_str, -1, &len, &error)==G_IO_STATUS_ERROR)
		// main_channel is NULL, so that we don't need to launch clear_channel()
		return socket_fault(GTK_SOCKET_ERROR_WRITE_CHANNEL, error, channel, TRUE);

	// flush writing datas
	// there are three results:
	//   G_IO_STATUS_AGAIN  this means temporarily anvailable, so try again
	//   G_IO_STATUS_NORMAL
	//   G_IO_STATUS_ERROR
	GIOStatus ioStatus;
	do ioStatus = g_io_channel_flush(channel, &error);
	while (ioStatus == G_IO_STATUS_AGAIN);

	if (ioStatus == G_IO_STATUS_ERROR)
		// main_channel is NULL, so that we don't need to launch clear_channel()
		return socket_fault(GTK_SOCKET_ERROR_WRITE_BUFFER, error, channel, TRUE);

	// So far so good. shutdown and clear channel!
	clear_channel(channel, TRUE);

	// FIXME: sleep for 1 sec to wait the socket server. any better idea?
	sleep(1);

	return TRUE;
}


// it will return TRUE if scucceed
gboolean clear_channel(GIOChannel *channel, gboolean unref)
{
#ifdef DETAIL
	g_debug("! Launch clear_channel() to clear channel data !");
#endif
	if (channel == NULL) return TRUE;

	gboolean return_value = TRUE;
	GError *error = NULL;

	if (g_io_channel_shutdown(channel, TRUE, &error) == G_IO_STATUS_ERROR)
		return_value = socket_fault(GTK_SOCKET_ERROR_SHUTDOWN_CHANNEL, error, NULL, FALSE);

	if (return_value && unref)
	{
		g_io_channel_unref(channel);
		channel = NULL;
	}

	return return_value;
}


// it will return TRUE if succeed
gboolean init_socket_server(GSourceFunc read_function)
{
#ifdef DETAIL
	g_debug("! Launch init_socket_server() to init a %s socket server !", PACKAGE);
#endif
	GError *error = NULL;

	// clear the prev file
	if (address.sun_path[0]) unlink(address.sun_path);

	// fchmod() on socket may cause unspecified behaviour.

	// bind the socket on a file
	if (bind(socket_fd, (struct sockaddr *)&address, address_len) < 0)
		// main_channel is NULL, so that we don't need to launch clear_channel()
		return socket_fault(GTK_SOCKET_ERROR_BIND_SOCKET, error, NULL, FALSE);

	// create socket queue
	if (listen(socket_fd, MAX_BACKLOG) < 0)
		// main_channel is NULL, so that we don't need to launch clear_channel()
		return socket_fault(GTK_SOCKET_ERROR_LISTEN_SOCKET, error, NULL, FALSE);

	main_channel = g_io_channel_unix_new(socket_fd);
	if (!main_channel) return socket_fault(GTK_SOCKET_ERROR_CREATE_CHANNEL, NULL, NULL, FALSE);

	// set the channel to read binary file
	if (g_io_channel_set_encoding(main_channel, NULL, &error) == G_IO_STATUS_ERROR)
		return socket_fault(GTK_SOCKET_ERROR_CHANNEL_ENCODING, error, main_channel, TRUE);

	// if any request from client, call accept_socket().
	// the channel will be clean when shutdown_socket_server()
	if ( ! g_io_add_watch(main_channel, G_IO_IN|G_IO_HUP, (GIOFunc)accept_socket, (GSourceFunc)read_function))
		return socket_fault(GTK_SOCKET_ERROR_WATCH_SOCKET, error, NULL, TRUE);

	return TRUE;
}


// it will return TRUE if succeed
gboolean accept_socket(GIOChannel *source, GIOCondition condition, GSourceFunc read_function)
{
#ifdef DETAIL
	g_debug("! Launch accept_socket() to accept the request from client !");
#endif
#ifdef SAFEMODE
	if (source==NULL) return FALSE;
#endif
	GError *error = NULL;

	if (condition & G_IO_HUP)
		return socket_fault(GTK_SOCKET_ERROR_ACCEPT_REQUEST, error, source, FALSE);
	else
	{
		gint read_fd = accept(g_io_channel_unix_get_fd(source), NULL, NULL);

		if (read_fd < 0) return socket_fault(GTK_SOCKET_ERROR_CREATE_SOCKET_FILE, error, source, FALSE);

		if ( ! set_fd_non_block(&read_fd)) return FALSE;

		GIOChannel* channel = g_io_channel_unix_new(read_fd);
		// channel is NULL, so that we don't need to launch clear_channel()
		if (!channel) return socket_fault(GTK_SOCKET_ERROR_CREATE_CHANNEL, NULL, channel, FALSE);
		// set the channel to read binary file
		if (g_io_channel_set_encoding(channel, NULL, &error) == G_IO_STATUS_ERROR)
			return socket_fault(GTK_SOCKET_ERROR_CHANNEL_ENCODING, error, channel, TRUE);

		// read the data that client sent.
		if ( ! g_io_add_watch(channel, G_IO_HUP, (GIOFunc)read_socket, read_function))
			return socket_fault(GTK_SOCKET_ERROR_WATCH_SOCKET, error, channel, TRUE);
	}
	return TRUE;
}

// it will return TRUE if succeed
gboolean read_socket(GIOChannel *channel, GIOCondition condition, GSourceFunc read_function)
{
#ifdef DETAIL
	g_debug("! Launch read_socket() to read data !");
#endif
#ifdef SAFEMODE
	if (channel==NULL) return FALSE;
#endif
	GError *error = NULL;
	gchar *str = NULL;
	gsize len = 0;
	gsize term;

	if (g_io_channel_read_line (channel, &str, &len, &term, &error) == G_IO_STATUS_ERROR)
		socket_fault(GTK_SOCKET_ERROR_RECEIVE_DATA, error, channel, TRUE);

	// read_socket_functions...
	if ((len == 0) || (str==NULL))
		socket_fault(GTK_SOCKET_ERROR_GOT_NULL_STRING, NULL, NULL, FALSE);
	else
		read_function(str);

	g_free(str);

	clear_channel(channel, TRUE);

	// return FALSE means this connection is finished.
	return FALSE;
}


// It should always return 0.
gint shutdown_socket_server(gpointer data)
{
#ifdef DETAIL
	g_debug("! Launch shutdown_socket_server() to shutdown the %s socket server!", PACKAGE);
#endif
	if (main_channel) clear_channel(main_channel, TRUE);
	if (address.sun_path[0]) unlink(address.sun_path);
	return 0;
}


// it will always return FALSE
gboolean socket_fault(Unix_Socket_Error type, GError *error, GIOChannel *channel, gboolean unref)
{
#ifdef DETAIL
	g_debug("! Launch socket_fault(%d) to show the error message !", type);
#endif
#ifdef UNIT_TEST
#  define G_WARNING g_message
#else
#  define G_WARNING g_warning
#endif
	switch (type)
	{
		case GTK_SOCKET_ERROR_CREATE_SOCKET_FILE:
			G_WARNING("Error when create the socket file \"%s\": %s",
				  address.sun_path, g_strerror (errno));
			break;
		case GTK_SOCKET_ERROR_CONNECT_SOCKET:
			g_message("Can NOT connect to a existing unix socket!");
			break;
		case GTK_SOCKET_ERROR_BIND_SOCKET:
			G_WARNING("Can NOT bind on the socket!");
			break;
		case GTK_SOCKET_ERROR_LISTEN_SOCKET:
			G_WARNING("Can NOT listen on the socket!");
			break;
		case GTK_SOCKET_ERROR_WATCH_SOCKET:
			G_WARNING("Can not watch on the socket!");
			break;
		case GTK_SOCKET_ERROR_ACCEPT_REQUEST:
			G_WARNING("Error when accepting client request via socket!");
			break;
		case GTK_SOCKET_ERROR_RECEIVE_DATA:
			G_WARNING("Error when reading the data client sent via socket!");
			break;
		case GTK_SOCKET_ERROR_FCNTL:
			G_WARNING("Error when running fcntl command on socket!");
			break;
		case GTK_SOCKET_ERROR_CHANNEL_ENCODING:
			if (error)
				G_WARNING("Error when setting the encoding of channel: %s", error->message);
			else
				G_WARNING("Error when setting the encoding of channel: (No further information)");
			break;
		case GTK_SOCKET_ERROR_SHUTDOWN_CHANNEL:
			if (error)
				G_WARNING("Error when shutdowning a channel: %s", error->message);
			else
				G_WARNING("Error when shutdowning a channel: (No further information)");
			break;
		case GTK_SOCKET_ERROR_WRITE_CHANNEL:
			if (error)
				G_WARNING("Error when writing data to the channel: %s", error->message);
			else
				G_WARNING("Error when writing data to the channel: (No further information)");
			break;
		case GTK_SOCKET_ERROR_CREATE_CHANNEL:
			G_WARNING("Can NOT create a channel for this socket");
			break;
		case GTK_SOCKET_ERROR_WRITE_BUFFER:
			if (error)
				G_WARNING("Error when flushing the write buffer for the channel: %s", error->message);
			else
				G_WARNING("Error when flushing the write buffer for the channel: (No further information)");
			break;
		case GTK_SOCKET_ERROR_GOT_NULL_STRING:
			G_WARNING("Got a NULL string from the socket!");
			break;
		case GTK_SOCKET_ERROR_TMP_DIR:
			G_WARNING("The /tmp dir is NOT exist!!");
			break;
		default:
#ifdef FATAL
			G_WARNING("The inputed %d for socket_fault() is Out of Range !!", type);
#endif
			break;
	}

	if (error) g_clear_error (&error);
	clear_channel(channel, unref);

	if (type != GTK_SOCKET_ERROR_CONNECT_SOCKET) address_len = -1;

	return FALSE;
}

