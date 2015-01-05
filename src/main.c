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

#include "main.h"

// for using in socket
gboolean single_process = TRUE;
gint socket_fd = 0;
struct sockaddr_un address = {0};
int address_len = 0;
GIOChannel *main_channel = NULL;
GtkClipboard *selection_clipboard = NULL;
GtkClipboard *selection_primary = NULL;

// gchar *command_line_path;
// gchar **empty_environ;
gchar *system_locale_list;
gchar *init_LC_CTYPE;
gchar *init_LANGUAGE;
gchar *init_encoding;
gchar *init_LC_MESSAGES;
const gchar *SYSTEM_VTE_CJK_WIDTH_STR = NULL;

const gchar *wmclass_name = NULL;
gchar *wmclass_class = NULL;
const gchar *shell = NULL;
gchar *pwd = NULL;
const gchar *home = NULL;

GList *window_list = NULL;
gchar *profile_dir = NULL;
gboolean proc_exist = FALSE;
gchar *proc_file_system_path = NULL;

extern gboolean force_to_quit;
extern gchar *restricted_locale_message;

//
// single_process ----------------(N)------------------------------------> new_window() ------>	shutdown_socket_serve()
//	|				^					^		  unlink
// init_socket_data()			|					|		+ clear_channel()
//   socket_fd -------------------(N)---|					|		    g_io_channel_shutdown
//   set_fd_non_block ------------(N)---/					|		    (g_io_channel_unref)
//	|									|
// query_socket()								|
//   connect ---------------------(N)-------> init_socket_server() -------------|
//	|				^	unlink				|
// send_socket()			|	bind -----------------------(N)-|
// * g_io_channel_unix_new -------(N)---|	listen ---------------------(N)-|
//   g_io_channel_set_encoding ---(N)---|     * g_io_channel_unix_new ------(N)-|
//   g_io_channel_set_buffered		|	g_io_channel_set_encoding --(N)-|
//   g_io_channel_write_chars ----(N)---|	g_io_add_watch -------------(N)-/
//   g_io_channel_flush ----------(N)---/	  |
// + clear_channel()				  `---- accept_socket()
//     g_io_channel_shutdown				  (condition)
//     (g_io_channel_unref)				  accept
//	|						  set_fd_non_block
//   exit()						* g_io_channel_unix_new
//							  g_io_channel_set_encoding
//							  g_io_add_watch
//							    |
//							    `-- read_socket()
//								  g_io_channel_read_line
//								    new_window()
//								+ clear_channel()
//								    g_io_channel_shutdown
//								    (g_io_channel_unref)
//
#ifdef UNIT_TEST
int fake_main(int   argc,
	      char *argv[])
#else
int main( int   argc,
	  char *argv[])
#endif
{
	// command_line_path = argv[0];

	// g_debug ("argc = %d", argc);
	// print_array ("argv", argv);
	// i18n support. We need to support i18n under console, too.
	setlocale(LC_ALL, "");
	bindtextdomain (BINARY, LOCALEDIR);
	bind_textdomain_codeset (BINARY, "UTF-8");
	textdomain (BINARY);

	const gchar *user_config_dir = g_get_user_config_dir();

#if ! defined(SAFEMODE) && defined(DEVELOP)
	g_message("Running %s without SAFE MODE!", PACKAGE);
#endif

#ifdef OUT_OF_MEMORY
#  undef g_strdup_printf
#endif
	if (user_config_dir) profile_dir = g_strdup_printf("%s/%s", user_config_dir, BINARY);
#ifdef OUT_OF_MEMORY
	#define g_strdup_printf(...) NULL
#endif
	proc_exist = check_if_default_proc_dir_exist(NULL);
	// g_debug ("Get proc_exist = %d, proc_file_system_path = %s", proc_exist, proc_file_system_path);

	shell = g_getenv("SHELL");
	if (shell==NULL) shell = "";

	gboolean pwd_need_be_free = FALSE;
	pwd = (gchar *)g_getenv("PWD");
	if (pwd==NULL)
	{
		pwd_need_be_free = TRUE;
		pwd = g_get_current_dir();
	}
	if (pwd==NULL)
	{
		pwd_need_be_free = FALSE;
		pwd = "";
	}
	// g_debug("Got $PWD = %s", pwd);

	home = g_getenv("HOME");
	if (home==NULL) home = "";
	// g_debug("Get $HOME = %s", home);

	// deal the command line options
	command_option(argc, argv);
	if (wmclass_name==NULL) wmclass_name = g_getenv("RESOURCE_NAME");
	if (wmclass_name==NULL) wmclass_name = "";
	if (wmclass_class==NULL) wmclass_class = "";
	// g_debug("Got wmclass_name = %s, wmclass_class = %s", wmclass_name, wmclass_class);

	// init the gtk+2 engine

	// GTK3: get gtk_test_widget_click() working...
	// gdk_disable_multidevice();

#ifndef UNIT_TEST
	gtk_init(&argc, &argv);
#endif
	// FIXME: we should get the single_process from profile. Current is command-line option only.
	if (single_process)
	{
		// init socket data
		if (init_socket_data())
		{
			// trying to connect to an existing LilyTerm
			if (query_socket())
			{
				// success, sent the argc/argv to socket then quit
				// g_debug("A LilyTerm socket server is exist already. exiting...");
				if (send_socket(argc, argv, TRUE))
				{
					g_free(profile_dir);
					exit (0);
				}
			}
			// no LilyTerm exist. create a socket server
			// g_debug("Creating a LilyTerm socket server...");
			init_socket_server();
			g_atexit((GVoidFunc)shutdown_socket_server);
		}
	}

	// start LilyTerm

	// empty_environ = g_strsplit("", " ", -1);
	extern gchar **environ;
	// print_array("main(): environ", environ);
	gchar *environ_str = convert_array_to_string(environ, '\t');
	window_list = NULL;
	// g_debug("Got environ_str (in main.c) = %s", environ_str);
	selection_clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
	selection_primary = gtk_clipboard_get(GDK_SELECTION_PRIMARY);
	system_locale_list = get_locale_list();
	// g_debug("Got system_locale_list = %s", system_locale_list);
	init_LC_CTYPE = g_strdup(get_default_lc_data(LC_CTYPE));
	// g_debug("Got init_LC_CTYPE = %s", init_LC_CTYPE);
	init_LANGUAGE = g_strdup(get_default_lc_data(LANGUAGE));
	// g_debug("Got init_LANGUAGE = %s", init_LANGUAGE);
	init_LC_MESSAGES = g_strdup(get_default_lc_data(LC_MESSAGES));
	// g_debug("init_LC_MESSAGES = %s", init_LC_MESSAGES);
	init_encoding = (gchar *)get_encoding_from_locale(NULL);
	if (! compare_strings(init_encoding, "ANSI_X3.4-1968", TRUE))
	{
		g_free(init_encoding);
		init_encoding = g_strdup("UTF-8");
	}
	// g_debug("init_encoding = %s", init_encoding);
	SYSTEM_VTE_CJK_WIDTH_STR = g_getenv("VTE_CJK_WIDTH");
	// g_debug ("Got SYSTEM_VTE_CJK_WIDTH_STR = %s", SYSTEM_VTE_CJK_WIDTH_STR);
	// FIXME: signal(SIGCHLD, SIG_IGN);
	// The first window of LilyTerm

	// Convert the GdkColor to GdkRGBA
	convert_system_color_to_rgba();

	// g_debug("Got original encoding = %s", get_encoding_from_locale(NULL));
	//GtkNotebook *new_window(int argc,
	//			char *argv[],
	//			gchar *shell,
	//			gchar *environment,
	//			gchar *locale_list,
	//			gchar *PWD,
	//			gchar *HOME,
	//			gchar *VTE_CJK_WIDTH_STR,
	//			gboolean VTE_CJK_WIDTH_STR_overwrite_profile,
	//			gchar *wmclass_name,
	//			gchar *wmclass_class,
	//			gchar *user_environ,
	//			gchar *encoding,
	//			gboolean encoding_overwrite_profile,
	//			gchar *lc_messages,
	//			struct Window *win_data_orig,
	//			struct Page *page_data_orig)

	if ((new_window(argc,
			argv,
			(gchar *) shell,
			environ_str,
			system_locale_list,
			(gchar *) pwd,
			(gchar *) home,
			SYSTEM_VTE_CJK_WIDTH_STR,
			FALSE,
			wmclass_name,
			wmclass_class,
			NULL,
			init_encoding,
			FALSE,
			init_LC_MESSAGES,
			NULL,
			NULL)) ||
	     window_list)
	{
		// The argv of "main" LilyTerm can't be free.
		// Set it to NULL here to avoid double_free().
		argv=NULL;
#ifdef ENABLE_X_STARTUP_NOTIFICATION_ID
		gdk_notify_startup_complete_with_id(PACKAGE);
#endif
		// g_debug("gtk_main_level = %d", gtk_main_level());
		if (! gtk_main_level())
			gtk_main();
	}
#ifdef DETAIL
	else
	{
//		g_debug("Got window_list = %p", window_list);
//		GList *win_list = window_list;
//		gint i=0;
//
//		while (win_list)
//		{
//			g_debug("Got %d win_data = %p", ++i, win_list->data);
//			win_list = win_list->next;
//		}
		g_debug("??? The creation of first window is FAIL!!!");
	}
#endif
	extern struct KeyValue system_keys[KEYS];
	gint i;
	// g_debug("Clear function key data!!");
	for (i=KEY_SWITCH_TO_TAB_1; i<=KEY_SWITCH_TO_TAB_12; i++)
	{
		g_free(system_keys[i].name);
		g_free(system_keys[i].topic);
		g_free(system_keys[i].comment);
		g_free(system_keys[i].translation);
#ifdef UNIT_TEST
		system_keys[i].name = NULL;
		system_keys[i].topic = NULL;
		system_keys[i].comment = NULL;
		system_keys[i].translation = NULL;
#endif
	}

	// g_free(pwd);
	// g_strfreev(empty_environ);
	g_free(environ_str);
	g_free(init_encoding);
	g_free(system_locale_list);
	g_free(profile_dir);
	if (pwd_need_be_free) g_free(pwd);
	g_free(restricted_locale_message);
	g_list_free(window_list);
	g_free(init_LC_CTYPE);
	g_free(init_LC_MESSAGES);
#ifdef UNIT_TEST
	// empty_environ = NULL;
	environ_str = NULL;
	init_encoding = NULL;
	system_locale_list = NULL;
	profile_dir = NULL;
	restricted_locale_message = NULL;
	window_list = NULL;
	init_LC_CTYPE = NULL;
	init_LC_MESSAGES = NULL;
#endif
	return 0;
}


// it will return TRUE if init socket data successfully
gboolean init_socket_data()
{
#ifdef DETAIL
	g_debug("! Launch init_socket_data() to init LilyTerm socket!");
#endif
	GError *error = NULL;

	// clean data first
	bzero(&address, sizeof(address));
	// init the address of socket
	address.sun_family = AF_UNIX;

	const gchar *tmp_dir = g_get_tmp_dir();
#ifdef SAFEMODE
	if (tmp_dir)
	{
#endif
		gchar *display = gdk_get_display();
#if defined(DEVELOP)
		g_snprintf(address.sun_path, UNIX_PATH_MAX, "%s/.%s_dev_%s%s",
			   tmp_dir ,BINARY, g_get_user_name(), display);
#elif defined(DEBUG)
		g_snprintf(address.sun_path, UNIX_PATH_MAX, "%s/.%s_dbg_%s%s",
			   tmp_dir ,BINARY, g_get_user_name(), display);
#else
		g_snprintf(address.sun_path, UNIX_PATH_MAX, "%s/.%s_%s%s",
			   tmp_dir ,BINARY, g_get_user_name(), display);
#endif
		g_free(display);
#ifdef SAFEMODE
	}
#endif
	address.sun_path[UNIX_PATH_MAX-1] = address.sun_path[UNIX_PATH_MAX-2] = '\0';
	// g_debug("The socket file is %s", address.sun_path);
	address_len = sizeof(address);

	// init socket
	socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (socket_fd < 0)
		// main_channel is NULL, so that we don't need to launch clear_channel()
		return socket_fault(1, error, NULL, FALSE);

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
		return socket_fault(8, error, NULL, FALSE);
	return TRUE;
}

// it will return TRUE if scucceed
gboolean query_socket()
{
#ifdef DETAIL
	g_debug("! Launch query_socket() to connect to an existing LilyTerm !");
#endif
	GError *error = NULL;

	if (connect(socket_fd, (struct sockaddr *)&address, address_len) < 0)
		// main_channel is NULL, so that we don't need to launch clear_channel()
		return socket_fault(2, error, NULL, FALSE);
	return TRUE;
}

// it will return TRUE if scucceed
gboolean send_socket( int   argc,
		      char *argv[],
		      gboolean wait)
{
#ifdef DETAIL
	g_debug("! Launch send_socket() to send data to the exiting LilyTerm !");
	g_debug("! send_socket() argc = %d, wait = %d", argc, wait);
	print_array("! send_socket() argv", argv);
#endif

	GError *error = NULL;
	gsize len;
	extern gchar **environ;

	gchar *locale_list = get_locale_list();

	const gchar *VTE_CJK_WIDTH_STR = g_getenv("VTE_CJK_WIDTH");
	// VTE_CJK_WIDTH can't = NULL
	if (VTE_CJK_WIDTH_STR == NULL) VTE_CJK_WIDTH_STR = "";

	// g_debug("Got LOCALE = %s in send_socket...", get_encoding_from_locale(NULL));
	gchar *encoding = get_encoding_from_locale(NULL);
	if (! compare_strings(encoding, "ANSI_X3.4-1968", TRUE))
	{
		g_free(encoding);
		encoding = g_strdup("UTF-8");
	}
	// g_debug("Got encoding = %s in send_socket...", encoding);
	gchar *lc_messages = g_strdup(get_default_lc_data(LC_MESSAGES));

	gchar *environ_str = convert_array_to_string(environ, '\t');
	// print_array("! send_socket() environ", environ);
	// g_debug("environ_str = %s", environ_str);
	gchar *argv_str = convert_array_to_string(argv, '\x10');
#ifdef SAFEMODE
	gboolean need_free_argv_str = TRUE;
	if (argv_str==NULL) argv_str=g_strdup("");
	if (argv_str==NULL)
	{
		need_free_argv_str = FALSE;
		argv_str = "";
	}
#endif
	// g_debug("argv_str = %s", argv_str);

	// g_debug("SEND DATA: SOCKET_DATA_VERSION = %s", SOCKET_DATA_VERSION);
	// g_debug("SEND DATA: locale_list = %s", locale_list);
	// g_debug("SEND DATA: encoding = %s", encoding);
	// g_debug("SEND DATA: PWD = %s", PWD);
	// g_debug("SEND DATA: VTE_CJK_WIDTH_STR = %s", VTE_CJK_WIDTH_STR);
	// g_debug("SEND DATA: wmclass_name = %s", wmclass_name);
	// g_debug("SEND DATA: wmclass_class = %s", wmclass_class);
	// g_debug("SEND DATA: environ_str = %s", environ_str);
	// g_debug("SEND DATA: argv_str = %s", argv_str);
	//	      0			  1	2	    3	     4		 5   6	  7		    8		 9	       10      11
	// send data: SOCKET_DATA_VERSION SHELL LOCALE_LIST ENCODING LC_MESSAGES PWD HOME VTE_CJK_WIDTH_STR wmclass_name wmclass_class ENVIRON ARGV
	//				  0	1     2	    3	  4	5     6	    7	  8	9     10    11
	gchar *arg_str = g_strdup_printf("%s\x10%s\x10%s\x10%s\x10%s\x10%s\x10%s\x10%s\x10%s\x10%s\x10%s\x10%s\x10",
					 SOCKET_DATA_VERSION,
					 shell,
					 locale_list,
					 encoding,
					 lc_messages,
					 pwd,
					 home,
					 VTE_CJK_WIDTH_STR,
					 wmclass_name,
					 wmclass_class,
					 environ_str,
					 argv_str);
	// g_debug("arg_str = %s", arg_str);
	g_free(locale_list);
	g_free(encoding);
	g_free(lc_messages);
	g_free(environ_str);
#ifdef SAFEMODE
	if (need_free_argv_str)
#endif
		g_free(argv_str);

	// write data!
#ifdef SAFEMODE
	if (fcntl(socket_fd, F_GETFL) < 0) return FALSE;
#endif
	GIOChannel *channel = g_io_channel_unix_new(socket_fd);
	// main_channel is NULL, so that we don't need to launch clear_channel()
	if (!channel) return socket_fault(12, NULL, NULL, FALSE);
	// set the channel to read binary file
	if (g_io_channel_set_encoding(channel, NULL, &error) == G_IO_STATUS_ERROR)
		return socket_fault(9, error, channel, TRUE);
	g_io_channel_set_buffered (channel, FALSE);

#ifdef SAFEMODE
	if ((arg_str == NULL) ||
	    (g_io_channel_write_chars(channel, arg_str, -1, &len, &error)==G_IO_STATUS_ERROR))
#else
	if (g_io_channel_write_chars(channel, arg_str, -1, &len, &error)==G_IO_STATUS_ERROR)
#endif
		// main_channel is NULL, so that we don't need to launch clear_channel()
		return socket_fault(11, error, channel, TRUE);
	// flush writing datas
	if (g_io_channel_flush(channel, &error) == G_IO_STATUS_ERROR)
		// main_channel is NULL, so that we don't need to launch clear_channel()
		return socket_fault(13, error, channel, TRUE);

	g_free(arg_str);

	// So far so good. shutdown and clear channel!
	clear_channel(channel, TRUE);

	// FIXME: sleep for 1 sec to wait the socket server. any better idea?
	if (wait) sleep(1);

	return TRUE;
}

// it will return TRUE if succeed
gboolean init_socket_server()
{
#ifdef DETAIL
	g_debug("! Launch init_socket_server() to init a LilyTerm socket server !");
#endif
	GError *error = NULL;

	// clear the prev file
	if (address.sun_path) unlink(address.sun_path);

	// bind the socket on a file
	if (bind(socket_fd, (struct sockaddr *)&address, address_len) < 0)
		// main_channel is NULL, so that we don't need to launch clear_channel()
		return socket_fault(3, error, NULL, FALSE);

	// create socket queue
	if (listen(socket_fd, 5) < 0)
		// main_channel is NULL, so that we don't need to launch clear_channel()
		return socket_fault(4, error, NULL, FALSE);

	main_channel = g_io_channel_unix_new(socket_fd);
	if (!main_channel) return socket_fault(12, NULL, NULL, FALSE);
	// set the channel to read binary file
	if (g_io_channel_set_encoding(main_channel, NULL, &error) == G_IO_STATUS_ERROR)
		return socket_fault(9, error, main_channel, TRUE);

	// if any request from client, call accept_socket().
	// the channel will be clean when main_quit()
	if ( ! g_io_add_watch ( main_channel, G_IO_IN|G_IO_HUP, (GIOFunc)accept_socket, NULL))
		return socket_fault(5, error, NULL, TRUE);

	return TRUE;
}

// it will return TRUE if succeed
gboolean accept_socket(GIOChannel *source, GIOCondition condition, gpointer user_data)
{
#ifdef DETAIL
	g_debug("! Launch accept_socket() to accept the request from client !");
#endif
#ifdef SAFEMODE
	if (source==NULL) return FALSE;
#endif

	GError *error = NULL;

	if (condition & G_IO_HUP)
		return socket_fault(6, error, source, FALSE);
	else
	{
		gint read_fd = accept(g_io_channel_unix_get_fd(source), NULL, NULL);
		if (read_fd < 0)
			return socket_fault(1, error, source, FALSE);
		if ( ! set_fd_non_block(&read_fd)) return FALSE;

		GIOChannel* channel = g_io_channel_unix_new(read_fd);
		// channel is NULL, so that we don't need to launch clear_channel()
		if (!channel) return socket_fault(12, NULL, channel, FALSE);
		// set the channel to read binary file
		if (g_io_channel_set_encoding(channel, NULL, &error) == G_IO_STATUS_ERROR)
			return socket_fault(9, error, channel, TRUE);

		// read the data that client sent.
		if ( ! g_io_add_watch(channel, G_IO_HUP, read_socket, NULL))
			return socket_fault(5, error, channel, TRUE);
	}
	return TRUE;
}

// it will return TRUE if succeed
gboolean read_socket(GIOChannel *channel, GIOCondition condition, gpointer user_data)
{
#ifdef DETAIL
	g_debug("! Launch read_socket() to read data !");
#endif
#ifdef SAFEMODE
	if (channel==NULL) return FALSE;
#endif

	GError *error = NULL;
	gchar *data = NULL, **datas;
	gsize len = 0;
	gsize term;

	if (g_io_channel_read_line (channel, &data, &len, &term, &error) == G_IO_STATUS_ERROR)
		return socket_fault(7, error, channel, TRUE);
	// g_debug("Read %ld bytes from Lilyterm socket: '%s'", len, data);
	if (len > 0)
	{
		//	     0			 1     2	   3	    4		5   6	 7		   8		9	      10      11
		// get data: SOCKET_DATA_VERSION SHELL LOCALE_LIST ENCODING LC_MESSAGES PWD HOME VTE_CJK_WIDTH_STR wmclass_name wmclass_class ENVIRON ARGV
		// clear '\n' at the end of data[]
		data[len-1] = 0;

		datas = split_string(data, "\x10", 12);
		// g_debug("The SOCKET_DATA_VERSION = %s ,and the data sent via socket is %s",
		//	   SOCKET_DATA_VERSION, datas[0]);

		if (datas==NULL)
		{
			// A dirty hack for sometimes the received socket datas is empty.
			socket_fault(14, NULL, NULL, FALSE);
			new_window(0,
				   NULL,
				   NULL,
				   NULL,
				   NULL,
				   NULL,
				   NULL,
				   NULL,
				   FALSE,
				   NULL,
				   NULL,
				   NULL,
				   NULL,
				   FALSE,
				   NULL,
				   NULL,
				   NULL);
		}
		else if (compare_strings(SOCKET_DATA_VERSION, datas[0], TRUE))
		{
			// The SOCKET_DATA_VERSION != the data sent via socket
			gchar *received_socket_version = NULL;
			if (datas) received_socket_version = datas[0];

			gchar *message = g_strdup_printf(_("The data got from socket seems incorrect.\n\n"
							   "\tReceived socket version: %s\n"
							   "\tExpected socket version: %s\n\n"
							   "If you just updated %s recently,\n"
							   "Please close all the windows of %s and try again."),
							   received_socket_version, SOCKET_DATA_VERSION,
							   PACKAGE, PACKAGE);
			error_dialog(NULL,
				     _("The format of socket data is out of date"),
				     "The format of socket data is out of date",
				     GTK_FAKE_STOCK_DIALOG_ERROR,
				     message,
				     NULL);
			g_free(message);
		}
		else
		{
			gchar **argv = split_string(datas[11], "\x10", -1);
			gint argc = 0;
			if (argv)
				while (argv[argc])
					argc ++;

			// g_debug("Final:");
			// g_debug("\targc =%d", argc);
			// print_array("\targv", argv);
			// g_debug("\tSHELL = %s", datas[1]);
			// g_debug("\tenvironments = %s", datas[10]);
			// g_debug("\tlocale_list = %s", datas[2]);
			// g_debug("\tPWD = %s", datas[5]);
			// g_debug("\tHOME = %s", datas[6]);
			// g_debug("\tVTE_CJK_WIDTH_STR = %s", datas[7]);
			// g_debug("\twmclass_name = %s", datas[8]);
			// g_debug("\twmclass_class= %s", datas[9]);
			// g_debug("\tencoding = %s", datas[3]);
			// g_debug("\tlc_messages = %s", datas[4]);

			//GtkNotebook *new_window(int argc,
			//			char *argv[],
			//			gchar *shell,					// 1
			//			gchar *environment,				// 10
			//			gchar *locale_list,				// 2
			//			gchar *PWD,					// 5
			//			gchar *HOME,					// 6
			//			gchar *VTE_CJK_WIDTH_STR,			// 7
			//			gboolean VTE_CJK_WIDTH_STR_overwrite_profile,
			//			gchar *wmclass_name,				// 8
			//			gchar *wmclass_class,				// 9
			//			gchar *user_environ,
			//			gchar *encoding,				// 3
			//			gboolean encoding_overwrite_profile,
			//			gchar *lc_messages,				// 4
			//			struct Window *win_data_orig,
			//			struct Page *page_data_orig)

			new_window(argc,
				   argv,
				   datas[1],
				   datas[10],
				   datas[2],
				   datas[5],
				   datas[6],
				   datas[7],
				   FALSE,
				   datas[8],
				   datas[9],
				   NULL,
				   datas[3],
				   FALSE,
				   datas[4],
				   NULL,
				   NULL);
			g_strfreev(argv);
		}
		g_strfreev(datas);
		data[len-1] = '\n';
		g_free(data);
	}
	clear_channel(channel, TRUE);
	// return FALSE means this connection is finish.
	return FALSE;
}

// it will always return FALSE
gboolean socket_fault(int type, GError *error, GIOChannel *channel, gboolean unref)
{
#ifdef DETAIL
	g_debug("! Launch socket_fault() to show the error message !");
#endif
#ifdef UNIT_TEST
#  define G_WARNING g_message
#else
#  define G_WARNING g_warning
#endif
	switch (type)
	{
		case 1:
			G_WARNING("Error when create %s socket(%s): %s",
				  PACKAGE, address.sun_path, g_strerror (errno));
			break;
		case 2:
			g_message("Can NOT connect to a existing %s socket!", PACKAGE);
			break;
		case 3:
			G_WARNING("Can NOT bind on the socket!");
			break;
		case 4:
			G_WARNING("Can NOT listen on the socket!");
			break;
		case 5:
			G_WARNING("Can not watch on the socket!");
			break;
		case 6:
			G_WARNING("Error when accepting client request via socket!");
			break;
		case 7:
			G_WARNING("Error when reading the data client sent via socket!");
			break;
		case 8:
			G_WARNING("Error when running fcntl command on socket!");
			break;
		case 9:
#ifdef SAFEMODE
			if (error)
#endif
				G_WARNING("Error when setting the encoding of channel: %s", error->message);
			break;
		case 10:
#ifdef SAFEMODE
			if (error)
#endif
				G_WARNING("Error when shutdowning a channel: %s", error->message);
			break;
		case 11:
#ifdef SAFEMODE
			if (error)
#endif
				G_WARNING("Error when writing data to the channel: %s", error->message);
			break;
		case 12:
			G_WARNING("Can NOT create a channel for this socket");
			break;
		case 13:
#ifdef SAFEMODE
			if (error)
#endif
				G_WARNING("Error when flushing the write buffer for the channel: %s", error->message);
			break;
		case 14:
			G_WARNING("Got a NULL string from the socket!");
			break;
		default:
#ifdef FATAL
			print_switch_out_of_range_error_dialog("socket_fault", "type", type);
#endif
			break;
	}
	if (error) g_clear_error (&error);
	clear_channel(channel, unref);
#ifdef UNIT_TEST
#  undef G_WARNING
#endif
	return FALSE;
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
		return_value = socket_fault(10, error, NULL, FALSE);

	if (return_value && unref)
	{
		g_io_channel_unref(channel);
		channel = NULL;
	}

	return return_value;
}

// It should always return 0.
gint shutdown_socket_server(gpointer data)
{
#ifdef DETAIL
	g_debug("! Launch shutdown_socket_server() to shutdown the LilyTerm socket server!");
#endif
	if (main_channel) clear_channel(main_channel, TRUE);
	if (address.sun_path) unlink(address.sun_path);
	return 0;
}

void main_quit(GtkWidget *widget, struct Window *win_data)
{
#ifdef DETAIL
	g_debug("! Launch main_quit() with win_data = %p!", win_data);
#endif
	// g_debug("Total window = %d", g_list_length(window_list));
	if (g_list_length(window_list)==1)
	{
#ifdef SAFEMODE
		// g_debug ("main_quit(): win_data==NULL, call gtk_main_quit()");
		if (win_data==NULL) return quit_gtk();
#endif
		window_quit(win_data->window, NULL, win_data);
	}
	else
	{
		GString *all_process_list = g_string_new(NULL);
		GString *child_process_list;
		GList *win_list = window_list;
		struct Window *temp_win_data;
		gint i = 1;
		while (win_list)
		{
			temp_win_data = win_list->data;
			child_process_list = close_multi_tabs(temp_win_data, i);
#ifdef SAFEMODE
			if (child_process_list)
#endif
				g_string_append (all_process_list, child_process_list->str);
			g_string_free(child_process_list, TRUE);
			win_list = win_list->next;
			i++;
		}

		// g_debug("Got all_process_list =%s", all_process_list->str);
#ifdef SAFEMODE
		if ((all_process_list==NULL) || (all_process_list->len==0) ||
		    (display_child_process_dialog (all_process_list, win_data,
						   CONFIRM_TO_EXIT_WITH_CHILD_PROCESS)))
#else
		if ((all_process_list->len==0) ||
		    (display_child_process_dialog (all_process_list, win_data,
						   CONFIRM_TO_EXIT_WITH_CHILD_PROCESS)))
#endif
		{
			force_to_quit = TRUE;
			// g_debug ("main_quit(): call gtk_main_quit()");
			quit_gtk();
		}
		g_string_free(all_process_list, TRUE);
	}
}

void quit_gtk()
{
#ifdef DETAIL
	g_debug("! Launch quit_gtk()");
#endif
	if (gtk_main_level()) gtk_main_quit();
}

gchar *get_locale_list()
{
#ifdef DETAIL
	g_debug("! Launch get_locale_list()!");
#endif
#ifdef OUT_OF_MEMORY
#  undef g_getenv
#endif
	return join_strings_to_string(' ',
				      14,
				      g_getenv("LANG"),
				      g_getenv("LC_CTYPE"),
				      g_getenv("LC_NUMERIC"),
				      g_getenv("LC_TIME"),
				      g_getenv("LC_COLLATE"),
				      g_getenv("LC_MONETARY"),
				      g_getenv("LC_MESSAGES"),
				      g_getenv("LC_PAPER"),
				      g_getenv("LC_NAME"),
				      g_getenv("LC_ADDRESS"),
				      g_getenv("LC_TELEPHONE"),
				      g_getenv("LC_MEASUREMENT"),
				      g_getenv("LC_IDENTIFICATION"),
				      g_getenv("LC_ALL"));
#ifdef OUT_OF_MEMORY
	#define g_getenv(x) NULL
#endif
}

//// The returned GString should be freed when no longer needed.
//GString *convert_string_array_to_0x9_separated_gstring (gchar **string_array)
//{
//	gint i = 0;
//	GString *string = g_string_new("");
//
//	if (string_array!=NULL)
//	{
//		while (string_array[i]!=NULL)
//		{
//			// g_debug("%d: %s", i, string_array[i]);
//			g_string_append_printf(string, "%s\x10", string_array[i]);
//			i++;
//		}
//	}
//	return string ;
//}
