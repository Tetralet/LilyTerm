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

#include "window.h"

extern gboolean proc_exist;

// if window_list = NULL, call gtk_window_quit()
extern GList *window_list;
gboolean checked_profile_version = FALSE;
extern gboolean single_process;
GtkWidget *active_window = NULL;
GtkWidget *last_active_window = NULL;
GtkWidget *menu_active_window = NULL;

gboolean exceed_PID_MAX_DEFAULT = FALSE;
struct Process_Data *process_data = NULL;
extern struct Error_Data error_data;

extern gboolean force_to_quit;
extern gint dialog_activated;
extern gboolean menu_activated;

gboolean safe_mode = FALSE;
extern gboolean confirm_to_execute;

// user_environ should be separated with <tab>
// win_data_orig and page_data_orig are used when dragging a vte to root window.
GtkNotebook *new_window(int argc,
			char *argv[],
			gchar *shell,
			gchar *environment,
			gchar *locale_list,
			gchar *PWD,
			gchar *HOME,
			const gchar *VTE_CJK_WIDTH_STR,
			gboolean VTE_CJK_WIDTH_STR_overwrite_profile,
			const gchar *wmclass_name,
			gchar *wmclass_class,
			gchar *user_environ,
			gchar *encoding,
			gboolean encoding_overwrite_profile,
			gchar *lc_messages,
			struct Window *win_data_orig,
			struct Page *page_data_orig)
{
#ifdef DETAIL
	g_debug("! Launch new_window() with argc = %d, VTE_CJK_WIDTH_STR = %s, "
		"user_environ = %s, encoding = %s, win_data_orig = %p, page_data_orig = %p",
		argc, VTE_CJK_WIDTH_STR, user_environ, encoding, win_data_orig, page_data_orig);
	// g_debug("! environment = %s", environment);
	print_array("! Launch new_window() with argv", argv);
#endif
	struct Window *win_data = g_new0(struct Window, 1);
#ifdef SAFEMODE
	if (win_data==NULL) return NULL;
#endif
	// g_debug("new_window(): Append win_data(%p) to window_list!", win_data);
	window_list = g_list_append (window_list, win_data);

// ---- create main window ---- //

	win_data->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
#ifdef NO_RESIZE_GRIP
	gtk_window_set_has_resize_grip(GTK_WINDOW(win_data->window), FALSE);
#endif
	gtk_window_set_title(GTK_WINDOW(win_data->window), PACKAGE);
	set_window_icon(win_data->window);

	// save the data first
	// g_debug("Save window = %p and win_data = %p when initing window!", win_data->window, win_data);
	g_object_set_data(G_OBJECT(win_data->window), "Win_Data", win_data);

	// win_data->argc = argc;
	// win_data->argv = argv;
	// win_data->command = argv[0];

	win_data->home = g_strdup(HOME);
	win_data->init_dir = get_init_dir(-1, PWD, HOME);
#ifdef SAFEMODE
	if (win_data->init_dir)
	{
#endif
		if (g_chdir(win_data->init_dir))
		{
			g_free(win_data->init_dir);
			win_data->init_dir = NULL;
		}
#ifdef SAFEMODE
	}
#endif

	if (shell && shell[0]!='\0')
		win_data->shell = g_strdup(shell);
	else
	{
		struct passwd *user_data = getpwuid(getuid());
		if (user_data) win_data->shell = g_strdup(user_data->pw_shell);
	}
	if ((win_data->shell==NULL) || (win_data->shell[0]=='\0'))
	{
		g_free(win_data->shell);
		win_data->shell = g_strdup("/bin/sh");
	}

	if ((wmclass_name == NULL) || (wmclass_name[0]=='\0')) wmclass_name = BINARY;
	if ((wmclass_class == NULL) || (wmclass_class[0]=='\0')) wmclass_class = PACKAGE;
	win_data->wmclass_name = g_strdup(wmclass_name);
	win_data->wmclass_class = g_strdup(wmclass_class);
#ifdef SAFEMODE
	if ((wmclass_class) && (wmclass_class[0]>='a') && (wmclass_class[0]<='z'))
#else
	if ((wmclass_class[0]>='a') && (wmclass_class[0]<='z'))
#endif
		win_data->wmclass_class[0] += ('A' - 'a');
	gtk_window_set_wmclass(GTK_WINDOW(win_data->window), win_data->wmclass_name, win_data->wmclass_class);

	win_data->runtime_locale_list = g_strdup(locale_list);
	win_data->runtime_encoding = g_strdup(encoding);
	win_data->runtime_LC_MESSAGES = g_strdup(lc_messages);
	// g_debug("win_data->runtime_encoding = %s", win_data->runtime_encoding);
	// g_debug("win_data->runtime_LC_MESSAGES = %s", win_data->runtime_LC_MESSAGES);

	win_data->environment = g_strdup(environment);

// ---- environ, command line option and user settings---- //

	if (win_data_orig==NULL)
	{
		if (! window_option(win_data, encoding, argc, argv))
		{
			// Don't execute the command!
			// g_debug("new_window(1): Remove win_data(%p) from window_list!", win_data);
			window_list = g_list_remove (window_list, win_data);
			destroy_window(win_data);
#ifdef SAFEMODE
			win_data = NULL;
#endif
			return NULL;
		}
		// if (-e option) and (win_data->execute_command_in_new_tab)...
		// g_debug("last_active_window = %p, win_data->argc = %d, win_data->execute_command_in_new_tab = %d",
		//	last_active_window, win_data->argc, win_data->execute_command_in_new_tab);
		if ((last_active_window) && (((win_data->argc) && (win_data->execute_command_in_new_tab)) || win_data->join_as_new_tab))
		{
			// g_debug ("Run the -e option on the new tab of current window!");
			gint i, init_tab_number = win_data->init_tab_number;
			struct Window *active_win_data = (struct Window *)g_object_get_data(G_OBJECT(
								last_active_window), "Win_Data");
#ifdef SAFEMODE
			// Treat it as win_data->execute_command_in_new_tab = FALSE
			if (active_win_data!=NULL)
			{
#endif
				struct Page *page_data = (struct Page *)g_object_get_data(G_OBJECT(
								active_win_data->current_vte), "Page_Data");
#ifdef SAFEMODE
				if (page_data!=NULL)
				{
#endif
					active_win_data->command = win_data->command;
					active_win_data->argv = win_data->argv;
					active_win_data->argc = win_data->argc;
					active_win_data->hold = win_data->hold;

					if (win_data->custom_tab_names_str)
					{
						if (active_win_data->custom_tab_names_str)
						{
							g_string_append_printf(active_win_data->custom_tab_names_str, "%c%s", '\x10',
									       win_data->custom_tab_names_str->str);
							g_strfreev(active_win_data->custom_tab_names_strs);
						}
						else
							active_win_data->custom_tab_names_str = g_string_new(win_data->custom_tab_names_str->str);

						active_win_data->custom_tab_names_strs = split_string(active_win_data->custom_tab_names_str->str, "\x10", -1);
						active_win_data->custom_tab_names_total = count_char_in_string(active_win_data->custom_tab_names_str->str, '\x10') + 1;
						// g_debug("(%p): Got win_data->custom_tab_names_str = %s win_data->custom_tab_names_total = %d "
						//	"win_data->custom_tab_names_current = %d",
						//	active_win_data, active_win_data->custom_tab_names_str->str,
						//	active_win_data->custom_tab_names_total, win_data->custom_tab_names_current);
					}

					gchar *FINAL_encoding = encoding;
					if ( (! encoding_overwrite_profile) &&
					     win_data->default_encoding &&
					     win_data->default_encoding[0]!='\0')
						FINAL_encoding = win_data->default_encoding;

					const gchar *FINAL_VTE_CJK_WIDTH = VTE_CJK_WIDTH_STR;
					if ( (! VTE_CJK_WIDTH_STR_overwrite_profile) &&
					     win_data->VTE_CJK_WIDTH_STR &&
					     win_data->VTE_CJK_WIDTH_STR[0] != '\0')
						FINAL_VTE_CJK_WIDTH = win_data->VTE_CJK_WIDTH_STR;
					// g_debug("send win_data->VTE_CJK_WIDTH_STR = %s to add_page()",
					//	FINAL_VTE_CJK_WIDTH_STR);
					for (i=0; i<init_tab_number; i++)
					{
						gint current_page_no = gtk_notebook_get_current_page(GTK_NOTEBOOK(active_win_data->notebook));
						// g_debug("send win_data->VTE_CJK_WIDTH_STR = %s to add_page()",
						//	win_data->VTE_CJK_WIDTH_STR);
						// g_debug("win_data->init_tab_number = %d, i = %d", win_data->init_tab_number, i);
						// g_debug("win_data->default_encoding = %s, encoding = %s",
						//	win_data->default_encoding, encoding);

						// g_debug("new_encoding = %s", new_encoding);
						// struct Page *add_page(struct Window *win_data,
						//			 struct Page *page_data_prev,
						//			 GtkWidget *menuitem_encoding,
						//			 gchar *encoding,
						//			 gchar *runtime_locale_encoding,
						//			 gchar *locale,
						//			 gchar *environments,
						//			 gchar *user_environ,
						//			 gchar *VTE_CJK_WIDTH_STR,
						//			 gboolean add_to_next)

						if ( ! add_page(active_win_data,
								page_data,
								NULL,
								FINAL_encoding,
								encoding,
								win_data->default_locale,
								environment,
								user_environ,
								FINAL_VTE_CJK_WIDTH,
								FALSE))
							gtk_notebook_set_current_page(GTK_NOTEBOOK(active_win_data->notebook),
										      current_page_no);
					}
					// g_debug("new_window(2): Remove win_data(%p) from window_list!", win_data);
					window_list = g_list_remove (window_list, win_data);
					destroy_window(win_data);
#ifdef SAFEMODE
					win_data=NULL;
#endif
					return NULL;
#ifdef SAFEMODE
				}
			}
#endif
		}
		get_user_settings(win_data, encoding);
	}
	else
	{
		win_data_dup(win_data_orig, win_data);
#ifdef ENABLE_RGBA
		// the init_rgba() must run before the window is mapped!
		if (win_data->use_rgba == -1) init_rgba(win_data);
#endif
	}

	if (VTE_CJK_WIDTH_STR && VTE_CJK_WIDTH_STR[0] != '\0')
	{
		g_free(win_data->VTE_CJK_WIDTH_STR);
		win_data->VTE_CJK_WIDTH_STR = g_strdup(VTE_CJK_WIDTH_STR);
	}
	// g_debug("Got win_data->VTE_CJK_WIDTH_STR = %s after get_user_settings()", win_data->VTE_CJK_WIDTH_STR);

// ---- the events of window---- //

	// close application if [Close Button] clicked
	g_signal_connect(G_OBJECT(win_data->window), "delete_event",
			 G_CALLBACK(window_quit), win_data);
	// if function key pressed
	g_signal_connect(G_OBJECT(win_data->window), "key-press-event",
			 G_CALLBACK(window_key_press), win_data);
	g_signal_connect(G_OBJECT(win_data->window), "key-release-event",
			 G_CALLBACK(window_key_release), win_data);
	// if get focus, the size of vte is NOT resizeable.
	g_signal_connect_after(G_OBJECT(win_data->window), "focus-in-event",
			       G_CALLBACK(window_get_focus), win_data);
	// if lost focus, the size of vte is resizeable.
	g_signal_connect(G_OBJECT(win_data->window), "focus-out-event",
			 G_CALLBACK(window_lost_focus), win_data);
	// if the theme/fonts changed
	g_signal_connect_after(G_OBJECT(win_data->window), "style-set",
			       G_CALLBACK(window_style_set), win_data);
#ifdef USE_GTK2_GEOMETRY_METHOD
	g_signal_connect(G_OBJECT(win_data->window), "size_request",
			 G_CALLBACK(window_size_request), win_data);
#endif
	g_signal_connect(G_OBJECT(win_data->window), "size-allocate",
			 G_CALLBACK(window_size_allocate), win_data);
#ifdef USE_GTK2_GEOMETRY_METHOD
	// fullscreen/unfullscreen
	g_signal_connect(G_OBJECT(win_data->window), "window-state-event",
			 G_CALLBACK(window_state_event), win_data);
#endif

// ---- create notebook ---- //

	win_data->notebook = gtk_notebook_new();
	set_widget_thickness(win_data->notebook, 0);

	gtk_notebook_set_scrollable(GTK_NOTEBOOK(win_data->notebook), TRUE);
	// if (win_data->show_tabs_bar != FORCE_ON)
	//	gtk_notebook_set_show_tabs(GTK_NOTEBOOK(win_data->notebook), FALSE);
	hide_and_show_tabs_bar(win_data , win_data->show_tabs_bar);

	gtk_notebook_set_show_border(GTK_NOTEBOOK(win_data->notebook), FALSE);
	// We used markup on tab names, so that we can't use popup on the notebook.
	// gtk_notebook_popup_enable(win_data->notebook);
	// Enable drag and drog
#ifdef ENABLE_DRAG_AND_DROP
	gtk_notebook_set_group_name (GTK_NOTEBOOK(win_data->notebook), PACKAGE);
#endif
#ifdef ENABLE_PAGE_ADDED
	g_signal_connect(G_OBJECT(win_data->notebook), "page-added",
			 G_CALLBACK(notebook_page_added), win_data);
#endif
	if (win_data->tabs_bar_position)
		gtk_notebook_set_tab_pos(GTK_NOTEBOOK(win_data->notebook), GTK_POS_BOTTOM);
	gtk_container_add(GTK_CONTAINER(win_data->window), win_data->notebook);
#ifdef ENABLE_TAB_REORDER
	g_signal_connect(G_OBJECT(win_data->notebook), "page-reordered", G_CALLBACK(reorder_page_number), win_data->window);
#endif
#ifdef ENABLE_DRAG_AND_DROP
	g_signal_connect(G_OBJECT(win_data->notebook), "create-window", G_CALLBACK(create_window), win_data);
#endif
#ifdef GEOMETRY
	g_signal_connect(G_OBJECT(win_data->notebook), "size-allocate", G_CALLBACK(widget_size_allocate), "notebook");
#endif

// ---- add a new page---- //

	gchar *FINAL_encoding = encoding;
	if ( (! encoding_overwrite_profile) &&
	     win_data->default_encoding &&
	     win_data->default_encoding[0]!='\0')
		FINAL_encoding = win_data->default_encoding;

	const gchar *FINAL_VTE_CJK_WIDTH = VTE_CJK_WIDTH_STR;
	if ( (! VTE_CJK_WIDTH_STR_overwrite_profile) &&
	     win_data->VTE_CJK_WIDTH_STR &&
	     win_data->VTE_CJK_WIDTH_STR[0] != '\0')
		FINAL_VTE_CJK_WIDTH = win_data->VTE_CJK_WIDTH_STR;

	// g_debug("new_window(): encoding = %s", encoding);
	// add a new page to notebook. run_once=TRUE.
	if (page_data_orig==NULL)
	{
		// FIXME: Will it different if win_data_orig==NULL or not?
		gint i;
		// g_debug("win_data->init_tab_number = %d", win_data->init_tab_number);
		for (i=0; i<win_data->init_tab_number; i++)
		{
			// g_debug("send win_data->VTE_CJK_WIDTH_STR = %s to add_page()",
			// 	win_data->VTE_CJK_WIDTH_STR);
			// g_debug("win_data->init_tab_number = %d, i = %d", win_data->init_tab_number, i);

			// add_page(struct Window *win_data,
			//	    struct Page *page_data_prev,
			//	    GtkWidget *menuitem_encoding,
			//	    gchar *encoding,
			//	    gchar *locale,
			//	    gchar *environments,
			//	    gchar *user_environ,
			//	    gchar *VTE_CJK_WIDTH_STR,
			//	    gboolean add_to_next)
			if ( ! add_page(win_data,
					NULL,
					NULL,
					FINAL_encoding,
					encoding,
					win_data->default_locale,
					NULL,
					user_environ,
					FINAL_VTE_CJK_WIDTH,
					TRUE))
				return NULL;
		}
	}
	else
	{
		// It is for the new window which dragged the vte tab to the root window
		// for create_menu()
#ifdef SAFEMODE
		if ((win_data != NULL) && (win_data_orig != NULL))
		{
#endif
			win_data->current_vte = win_data_orig->current_vte;
#if defined(USE_GTK3_GEOMETRY_METHOD) || defined(UNIT_TEST)
			win_data->hints_type = HINTS_SKIP_ONCE;
#  ifdef GEOMETRY
			fprintf(stderr, "\033[1;%dm!! new_window(win_data %p): set win_data->hints_type = %d !!\033[0m\n",
				ANSI_COLOR_BLUE, win_data, win_data->hints_type);
#  endif
			resize_to_exist_widget(win_data);
#endif
#ifdef SAFEMODE
		}
#endif
	}

	// gtk_window_set_gravity(GTK_WINDOW(win_data->window), GDK_GRAVITY_NORTH);
	// gtk_window_move (GTK_WINDOW(win_data->window), 0, 0);

// ---- finish! ---- //

#ifndef UNIT_TEST
	// Due to the bug(?) in GTK3+, window must shown before gtk_window_parse_geometry()
	// ERR MSG: gtk_window_parse_geometry() called on a window with no visible children;
	//	    the window should be set up before gtk_window_parse_geometry() is called.
	gtk_widget_show_all(win_data->window);

	// if (gtk_widget_get_window (win_data->window))
	//	g_debug("WINDOW(%p): WINDOWID = %ld", win_data->window, GDK_WINDOW_XID (gtk_widget_get_window (win_data->window)));

#endif

	if (win_data_orig==NULL)
	{
		if (win_data->geometry && (win_data->geometry[0]!='\0'))
		{
#ifdef GEOMETRY
			g_debug("@ new_window(for %p): Trying to set the geometry to %s...",
				win_data->window, win_data->geometry);
#endif
#ifdef USE_XPARSEGEOMETRY
			gint offset_x = 0, offset_y = 0;
			guint column, row;
			if (XParseGeometry (win_data->geometry, &offset_x, &offset_y, &column, &row))
				gtk_window_move (GTK_WINDOW(win_data->window), offset_x, offset_y);
#else
			gtk_window_parse_geometry(GTK_WINDOW(win_data->window), win_data->geometry);
#endif
		}
	}

	// gtk_window_set_focus(GTK_WINDOW(win_data->window), win_data->current_vte);
	// g_debug("new_window(): call window_resizable() with run_once = 1");
	// FIXME: I don't think we should call window_resizable() over and over.
	// window_resizable(win_data->window, win_data->current_vte, 1, 1);

	// g_debug("create menu!!");
	// input method menu can't not be shown before window is shown.
	if (create_menu(win_data) == FALSE) return NULL;

	//if (page_data_orig!=NULL)
	//{
	//	// fix the color data after menu had been created.
	//	// g_debug("Launch get_and_update_page_name() in new_window()!");
	//	get_and_update_page_name(page_data_orig);
	//}

	// fullscreen!
#ifdef USE_GTK2_GEOMETRY_METHOD
	if (win_data->startup_fullscreen)
#else
	if (win_data->window_status==WINDOW_START_WITH_FULL_SCREEN)
#endif
	{
#ifdef GEOMETRY
		fprintf(stderr, "\033[1;%dm!! new_window(win_data %p): calling deal_key_press(KEY_FULL_SCREEN)!!\033[0m\n",
			ANSI_COLOR_WHITE, win_data);
#endif
		GtkWidget *menu_active_windows_orig = menu_active_window;
		menu_active_window = win_data->window;
		// g_debug("win_data->show_scroll_bar = %d", win_data->show_scroll_bar);
		deal_key_press(win_data->window, KEY_FULL_SCREEN, win_data);
		menu_active_window = menu_active_windows_orig;
	}
#ifdef UNIT_TEST
	gtk_widget_destroy(win_data->window);
	return NULL;
#else
	return GTK_NOTEBOOK(win_data->notebook);
#endif
}

gchar *get_init_dir(pid_t pid, gchar *pwd, gchar *home)
{
#ifdef DETAIL
	g_debug("! Launch get_init_dir() with pwd = %s, home = %s", pwd, home);
#endif
	if (pwd && (g_file_test(pwd, G_FILE_TEST_EXISTS))) return g_strdup(pwd);
	if (pid>0)
	{
		gchar *dir = get_tab_name_with_current_dir(pid);
		// g_debug("get_init_dir: get_tab_name_with_current_dir(%d) = %s", pid, dir);
#ifdef SAFEMODE
		if (dir && (g_file_test(dir, G_FILE_TEST_EXISTS))) return dir;
#else
		if (g_file_test(dir, G_FILE_TEST_EXISTS)) return dir;
#endif
	}
	if (home && (g_file_test(home, G_FILE_TEST_EXISTS))) return g_strdup(home);
	gchar *dir = g_get_current_dir();
#ifdef SAFEMODE
	if (dir && (g_file_test(dir, G_FILE_TEST_EXISTS))) return dir;
#else
	if (g_file_test(dir, G_FILE_TEST_EXISTS)) return dir;
#endif
	return g_strdup("/");
}

void set_window_icon(GtkWidget *window)
{
#ifdef DETAIL
	g_debug("! Launch set_window_icon() on window = %p", window);
#endif
#ifdef SAFEMODE
	if (window==NULL) return;
#endif
	GdkPixbuf *icon = gdk_pixbuf_new_from_file(ICON_PATH, NULL);
	if (icon)
	{
		gtk_window_set_icon(GTK_WINDOW(window), icon);
		g_object_unref(icon);
	}
}

gboolean window_quit(GtkWidget *window, GdkEvent *event, struct Window *win_data)
{
#ifdef DETAIL
	g_debug("! Launch window_quit() with window = %p, win_data = %p", window, win_data);
#endif
#ifdef SAFEMODE
	if ((window==NULL) || (win_data==NULL)) return TRUE;
#endif
	menu_active_window = window;
	if (win_data->confirm_to_kill_running_command)
	{
		gint total_page = gtk_notebook_get_n_pages(GTK_NOTEBOOK(win_data->notebook));
		// g_debug("total_page = %d in window_quit()", total_page);
		if (total_page>1)
		{
			if (win_data->confirm_to_close_multi_tabs)
				// confirm to close multi pages.
				dialog(NULL, CONFIRM_TO_CLOSE_MULTI_PAGES);
			else
				close_multi_tabs(win_data, FALSE);
		}
		else
			close_page(win_data->current_vte, CLOSE_WITH_WINDOW_CLOSE_BUTTON);
	}
	else
	{
		force_to_quit=TRUE;
		close_page(win_data->current_vte, CLOSE_TAB_NORMAL);
	}

	// g_debug("Close window finish!");
	// It will be segmentation fault if retrun FALSE
	return TRUE;
}

GString *close_multi_tabs(struct Window *win_data, int window_no)
{
#ifdef DETAIL
	g_debug("! Launch close_multi_tabs() with win_data = %p", win_data);
#endif
#ifdef SAFEMODE
	if (win_data==NULL) return NULL;
#endif
	gint i;
	gint total_page = -1;
#ifdef SAFEMODE
	if (win_data->notebook)
#endif
		total_page = gtk_notebook_get_n_pages(GTK_NOTEBOOK(win_data->notebook));
	struct Page *page_data = NULL;

	// g_debug("total_page = %d in close_multi_tabs()", total_page);

	if (((! force_to_quit) && (total_page>1)) || window_no)
	{
		GString *child_process_list = g_string_new(NULL);
		process_data = g_new0(struct Process_Data, PID_MAX_DEFAULT);
#ifdef SAFEMODE
		if (process_data==NULL) return child_process_list;
#endif
		for (i=0; i<total_page; i++)
		{
			page_data = get_page_data_from_nth_page(win_data, i);
#ifdef SAFEMODE
			if (page_data==NULL) continue;
#endif
			if (window_no)
				child_process_list = get_child_process_list(win_data->window, window_no, i+1,
									    child_process_list, page_data->pid,
									    win_data, TRUE);
			else
				child_process_list = get_child_process_list(0, 0, i+1, child_process_list,
									    page_data->pid, win_data, TRUE);
		}
		//for (i=0; i<10; i++)
		//	g_debug("i=%d, window = %p, page_no = %d, ppid = %d, cmd = %s",
		//		i, process_data[i].window, process_data[i].page_no, process_data[i].ppid, process_data[i].cmd);
		if (window_no)
		{
			clean_process_data();
			return child_process_list;
		}
#ifdef SAFEMODE
		if (child_process_list && child_process_list->len)
#else
		if (child_process_list->len)
#endif
		{
			if (! display_child_process_dialog (child_process_list, win_data,
							    CONFIRM_TO_CLOSE_A_WINDOW_WITH_CHILD_PROCESS))
			{
				g_string_free(child_process_list, TRUE);
				clean_process_data();
				return NULL;
			}
		}
		g_string_free(child_process_list, TRUE);
	}

	for (i=total_page-1; i>-1; i--)
	{
		page_data = get_page_data_from_nth_page(win_data, i);
		if (close_page(page_data->vte, CLOSE_WITH_WINDOW_CLOSE_BUTTON)==FALSE)
			break;
	}
	force_to_quit = FALSE;
	clean_process_data();
	return NULL;
}

void clean_process_data()
{
#ifdef DETAIL
	g_debug("! Launch clean_process_data()");
#endif
	if (process_data)
	{
		gint i;
		for (i=0; i<PID_MAX_DEFAULT; i++)
			g_free(process_data[i].cmd);
		g_free(process_data);
		process_data = NULL;
		exceed_PID_MAX_DEFAULT = FALSE;
	}
}

gboolean display_child_process_dialog (GString *child_process_list, struct Window *win_data, gsize style)
{
#ifdef SAFEMODE
	if ((child_process_list==NULL) || (win_data==NULL)) return TRUE;
#endif
#ifdef DETAIL
	g_debug("! Launch display_child_process_dialog() with child_process_list = %s, win_data = %p, style = %ld",
		child_process_list->str, win_data, (long)style);
#endif
	gchar *old_temp_data = win_data->temp_data;
	win_data->temp_data = child_process_list->str;

	gboolean return_value = (dialog(NULL, style) == GTK_RESPONSE_OK);

	win_data->temp_data = old_temp_data;
	return return_value;
}

// window: shows "Window #%d" in process_list. don't show if 0.
// page_no: shows "Page #%d" in process_list. don't show if 0.
// pid: the root pid of this page
// tpgid: the foreground command of this page
// show_foreground: check tpgid=pid or not
// entry_pid: the pid list of /proc
// entry_tpgid: the the foreground command of entry_pid
// ppid: the parent of entry_tpgid
// cmd: the cmd of entry_tpgid
GString *get_child_process_list(GtkWidget *window, gint window_no, gint page_no, GString *process_list, pid_t pid, struct Window *win_data, gboolean show_foreground)
{
#ifdef SAFEMODE
	if (win_data==NULL) return NULL;
	if (process_list==NULL) process_list = g_string_new(NULL);
	if (process_list==NULL) return NULL;
#endif
#ifdef DETAIL
	g_debug("! Launch get_child_process_list() with window = %p, page_no = %d, process_list = %s, "
		"pid = %d, win_data = %p, show_foreground = %d",
		window, page_no, process_list->str, pid, win_data, show_foreground);
#endif
	// Don't return foreground running process
	// g_debug("Checking a running pid = %d, in get_child_process_list", pid);
	if (! proc_exist) return process_list;

	GDir *dir  = g_dir_open ("/proc", 0, NULL);
	if (dir==NULL) goto FINISH;

	const gchar *entry = g_dir_read_name(dir);
	if (entry==NULL) goto FINISH;

	// pid_t tpgid = get_tpgid(pid);

	// man 5 proc for more details
	do
	{
		pid_t entry_pid = atoi(entry);
		gchar **stats = NULL;
		// struct Process_Data *entry_data = g_new0(struct Process_Data, 1);
		struct Process_Data entry_data = {0};
#ifdef SAFEMODE
		if (entry_pid<PID_MAX_DEFAULT && process_data)
#else
		if (entry_pid<PID_MAX_DEFAULT)
#endif
		{
			// Trying to check if we have done before
			if (process_data[entry_pid].ppid==0)
				stats = set_process_data (entry_pid,
							  &(process_data[entry_pid].ppid),
							  &(process_data[entry_pid].cmd));
			entry_data.ppid = process_data[entry_pid].ppid;
			entry_data.cmd = process_data[entry_pid].cmd;
		}
		else
		{
			stats = set_process_data (entry_pid, &(entry_data.ppid), &(entry_data.cmd));
			exceed_PID_MAX_DEFAULT = TRUE;
		}

		if (entry_data.ppid)
		{

			// g_debug("Got pid = %d, entry_pid = %d, entry_ppid = %d, entry_cmd = %s",
			//	pid, entry_pid, entry_data.ppid, entry_data.cmd);
			if ((pid == entry_data.ppid) || (pid == entry_pid))
			{
				pid_t entry_tpgid = get_tpgid(entry_pid);
				// g_debug("Checkig the child process with "
				//	"pid = %d, entry_pid = %d, entry_data.ppid = %d, entry_tpgid = %d, "
				//	"cmd of entry_pid = %s, show_foreground = %d",
				//	pid, entry_pid, entry_data.ppid, entry_tpgid, entry_data.cmd,
				//	show_foreground);
				if (((pid != entry_pid) && (entry_pid != entry_tpgid) &&
				     (! check_string_in_array(entry_data.cmd, win_data->background_program_whitelists))) ||
				    (show_foreground && (pid == entry_pid) && (entry_pid == entry_tpgid) &&
				     (! check_string_in_array(entry_data.cmd, win_data->foreground_program_whitelists))))
				{
#ifdef SAFEMODE
					if (entry_pid<PID_MAX_DEFAULT && process_data)
#else
					if (entry_pid<PID_MAX_DEFAULT)
#endif
					{
						process_data[entry_pid].window = window;
						process_data[entry_pid].page_no = page_no;
					}
					gchar *cmdline = get_cmdline(entry_pid);
					if (cmdline)
					{
						// g_debug("List (%d) %s in child_process_list...",
						//	entry_pid, cmdline);
						if (window)
							g_string_append_printf(process_list ,
									       _("\tWindow #%d, Page #%d: (%d) %s\n"),
									       window_no, page_no, entry_pid, cmdline);
						else if (page_no)
							g_string_append_printf(process_list ,
									       _("\tPage #%d: (%d) %s\n"),
									       page_no, entry_pid, cmdline);
						else
							g_string_append_printf(process_list ,
									       "\t(%d) %s\n",
									       entry_pid, cmdline);
					}
					g_free(cmdline);
				}
				if (pid != entry_pid)
					process_list = get_child_process_list(window, window_no, page_no, process_list,
									      entry_pid, win_data, show_foreground);
			}
		}
		g_strfreev(stats);
		// g_free(entry_data);
	}
	while ((entry = g_dir_read_name(dir)) != NULL);

FINISH:
	g_dir_close(dir);
	return process_list;
}


char **set_process_data (pid_t entry_pid, gint *ppid, StrAddr **cmd)
{
#ifdef SAFEMODE
	if (ppid==NULL) return NULL;
#endif
#ifdef FULL
	if (ppid)
		g_debug("! Launch set_process_data() with entry_pid = %d, ppid = %d, cmd = %s", entry_pid, *ppid, *cmd);
	else
		g_debug("! Launch set_process_data() with entry_pid = %d, ppid = (%p), cmd = %s", entry_pid, ppid, *cmd);
#endif
	char **stats = get_pid_stat(entry_pid, 7);
	if (stats)
	{
		*ppid = atoi(stats[5]);
		// the ppid of init(1) = 0
		if (*ppid==0)
			*ppid = -1;
		*cmd = g_strdup(stats[2]);
	}
	return stats;
}

gboolean window_option(struct Window *win_data, gchar *encoding, int argc, char *argv[])
{
#ifdef DETAIL
	g_debug("! Launch window_option() with win_data = %p, encoding = %s, argc = %d", win_data, encoding, argc);
	print_array("! Launch window_option() with argv", argv);
#endif
#ifdef SAFEMODE
	if ((win_data==NULL) || (argv==NULL)) return FALSE;
#endif
	// g_debug("Get win_data = %d in window option!", win_data);
	win_data->init_tab_number = 1;
	win_data->profile = get_user_profile_path(win_data, argc, argv);
	// g_debug("Got win_data->profile = %s", win_data->profile);
	gchar *window_role = NULL;

	gint i;
	gint getting_tab_name_argc = -1;
	for (i=0; i<argc; i++)
	{
#ifdef SAFEMODE
		if (argv[i]==NULL) break;
#endif

		// g_debug("(%p): %2d (Total %d): %s", win_data, i, argc, argv[i]);
		if ((!strcmp(argv[i], "-T")) || (!strcmp(argv[i], "--title")))
		{
			if (++i==argc)
				g_critical("missing window title after -T/--title!\n");
			else
			{
				gchar *window_title = convert_str_to_utf8(argv[i], encoding);
#ifdef SAFEMODE
				if (window_title)
				{
#endif
					win_data->custom_window_title_str = window_title;
					update_window_title(win_data->window, win_data->custom_window_title_str,
							    win_data->window_title_append_package_name);
#ifdef SAFEMODE
				}
#endif
				// g_debug("The title of LilyTerm is specified to %s",
				//	win_data->custom_window_title_str);
			}
		}
		else if ((!strcmp(argv[i], "-R")) || (!strcmp(argv[i], "--role")))
		{
			if (++i==argc)
				g_critical("missing window role after -R/--role!\n");
			else
			{
				window_role = convert_str_to_utf8(argv[i], encoding);
#ifdef SAFEMODE
				if ((window_role) && (win_data->window))
#endif
					gtk_window_set_role (GTK_WINDOW (win_data->window), window_role);

				// g_debug("The role of LilyTerm is specified to %s", window_role);
			}
		}
		else if ((!strcmp(argv[i], "-t")) || (!strcmp(argv[i], "--tab")))
		{
			if (++i==argc)
				g_critical("missing tab number after -t/--tab!\n");
			else
				win_data->init_tab_number = atoi(argv[i]);
			if (win_data->init_tab_number<1)
				win_data->init_tab_number=1;
			// g_debug("Init LilyTerm with %d page(s)!", init_tab_number);
		}
		else if ((!strcmp(argv[i], "-l")) || (!strcmp(argv[i], "-ls")) || (!strcmp(argv[i], "--login")))
		{
			win_data->login_shell = TRUE;
		}
		else if (!strcmp(argv[i], "-ut"))
		{
			win_data->utmp = TRUE;
		}
		else if ((!strcmp(argv[i], "-H")) || (!strcmp(argv[i], "--hold")))
		{
			// g_debug("Set win_data->hold to TRUE!!");
			win_data->hold = TRUE;
		}
		else if ((!strcmp(argv[i], "-e")) || (!strcmp(argv[i], "-x")) || (!strcmp(argv[i], "--execute")))
		{
			if (++i==argc)
				g_critical("missing command after -e/-x/--execute option!\n");
			else
			{
				// g_debug ("window_option(): call init_prime_user_datas() for win_data %p!", win_data);
				init_prime_user_datas(win_data);
				if (win_data->profile)
				{
					GKeyFile *keyfile = g_key_file_new();
					GError *error = NULL;
					if (g_key_file_load_from_file(keyfile, win_data->profile,
								      G_KEY_FILE_NONE, &error))
						get_prime_user_settings(keyfile, win_data, encoding);
					else
						profile_is_invalid_dialog(error, win_data);
					// g_debug("Got win_data->confirm_to_execute_command = %d",
					//	win_data->confirm_to_execute_command);
					// g_debug("Got win_data->execute_command_in_new_tab = %d",
					//	win_data->execute_command_in_new_tab);
					g_key_file_free(keyfile);
				}
				win_data->execute_command_whitelists = split_string(win_data->execute_command_whitelist,
							   " ", -1);
				gboolean execute_command = TRUE;
				if (win_data->confirm_to_execute_command &&
				    confirm_to_execute &&
				    (! check_string_in_array(argv[i], win_data->execute_command_whitelists)))
				{
					gint j;
					GString *arg_str = g_string_new(NULL);
					for (j=0; j<argc-i; j++)
					{
						// g_debug("%2d = %s", j, argv[i+j]);
						g_string_append_printf(arg_str, "%s", argv[i+j]);
						if (j<argc-i-1)
							g_string_append(arg_str, " ");
					}
					// g_debug("arg_str = %s", arg_str->str);
					gchar *old_temp_data = win_data->temp_data;
#ifdef SAFEMODE
					if (arg_str==NULL)
						win_data->temp_data = convert_str_to_utf8(NULL, encoding);
					else
#endif
						win_data->temp_data = convert_str_to_utf8(arg_str->str, encoding);

					GtkWidget *menu_active_window_orig = menu_active_window;
					menu_active_window = win_data->window;
					execute_command = (dialog (NULL, CONFIRM_TO_EXECUTE_COMMAND) == GTK_RESPONSE_OK);
					menu_active_window = menu_active_window_orig;

					g_free(win_data->temp_data);
					win_data->temp_data = old_temp_data;
					g_string_free(arg_str, TRUE);

					if (! execute_command)
						return FALSE;
				}

				// IMPORTANT!
				// g_free(win_data->execute_command_whitelist);
				// win_data->execute_command_whitelist = NULL;
				// g_strfreev(win_data->execute_command_whitelists);
				// win_data->execute_command_whitelists = NULL;
				// g_free(win_data->default_locale);
				// win_data->default_locale = NULL;

				if (execute_command)
				{
					win_data->command = argv[i];
					win_data->argv = &(argv[i]);
					win_data->argc = argc-i;
				}

				// g_debug("Command = %s", win_data->command);
				// g_debug("argc = %d", win_data->argc);
				// g_debug("argv =");
				// gint j;
				// for (j=0; j<win_data->argc; j++)
				//	g_debug("\t%s", win_data->argv[j]);
			}
			break;
		}
		else if ((!strcmp(argv[i], "-d")) || (!strcmp(argv[i], "--directory")))
		{
			if (++i==argc)
				g_critical("missing tab number after -d/--directory!\n");
			else
			{
				g_free(win_data->init_dir);
				win_data->init_dir = g_strdup(argv[i]);
			}
			// g_debug("+ Init LilyTerm with directory %s !", win_data->init_dir);
		}
		else if ((!strcmp(argv[i], "-g")) || (!strcmp(argv[i], "--geometry")))
		{
			if (++i==argc)
				g_critical("missing window title after -g/--geometry!\n");
			else
			{
				g_free(win_data->geometry);
				win_data->geometry = g_strdup(argv[i]);
			}
			// g_debug("+ Init LilyTerm with geometry = %s!", win_data->geometry);
		}
		else if (!strcmp(argv[i], "--safe-mode"))
			safe_mode=TRUE;
		else if ((!strcmp(argv[i], "-j")) || (!strcmp(argv[i], "--join")))
			win_data->join_as_new_tab = TRUE;
		else if ((!strcmp(argv[i], "-n")) || (!strcmp(argv[i], "--tab_names")))
		{
			if (i==(argc-1))
				g_critical("missing tab names after -n/--tab_names!\n");
			getting_tab_name_argc = i+1;
		}
		else if (getting_tab_name_argc==i)
		{
			if (win_data->custom_tab_names_str)
				g_string_append_printf(win_data->custom_tab_names_str, "%c%s", '\x10', argv[i]);
			else
				win_data->custom_tab_names_str = g_string_new(argv[i]);
			getting_tab_name_argc = i+1;
		}
	}

	if (win_data->custom_tab_names_str)
	{
		win_data->custom_tab_names_strs = split_string(win_data->custom_tab_names_str->str, "\x10", -1);
		win_data->custom_tab_names_total = count_char_in_string(win_data->custom_tab_names_str->str, '\x10') + 1;
		// g_debug("(%p): Got win_data->custom_tab_names_str = %s win_data->custom_tab_names_total = %d",
		//	win_data, win_data->custom_tab_names_str->str, win_data->custom_tab_names_total);
		if (win_data->init_tab_number<win_data->custom_tab_names_total)
			win_data->init_tab_number = win_data->custom_tab_names_total;
	}

	if (window_role)
		g_free(window_role);
	else
	{
#ifdef SAFEMODE
		if (win_data->window)
#endif
			gtk_window_set_role (GTK_WINDOW (win_data->window), PACKAGE);
	}

	return TRUE;
}

gboolean window_key_press(GtkWidget *window, GdkEventKey *event, struct Window *win_data)
{
#ifdef FULL
	if (event)
		g_debug("! Launch window_key_press() with key = %X (%s), state = %X, win_data = %p",
			 event->keyval, gdk_keyval_name(event->keyval), event->state, win_data);
#endif
#ifdef SAFEMODE
	if ((win_data==NULL) || (event==NULL)) return FALSE;
#endif
	// g_debug ("Get win_data = %p in key_press", win_data);
	// g_debug ("win_data->keep_vte_size = %X, event->state = %X", win_data->keep_vte_size, event->state);

#ifdef USE_GTK2_GEOMETRY_METHOD
	if (win_data->keep_vte_size) return FALSE;
#endif
	// don't check if only shift key pressed!
	// FIXME: GDK_asciitilde = 0x7e, it is for some keys like <F3> only.
	if ((event->state & SHIFT_ONLY_MASK) || (event->keyval > GDK_KEY_asciitilde))
	{
		// g_debug("ALL_ACCELS_MASK = %X", ALL_ACCELS_MASK);
		// g_debug("window_key_press(): Got the function key: %s (%X) with status= %X)",
		//	gdk_keyval_name(event->keyval), event->keyval, event->state);
		// mods = (event->state|GDK_LOCK_MASK|GDK_MOD2_MASK)&GDK_MODIFIER_MASK;
		gint i, keyval, mods = event->state | DUD_MASK;
		if ((event->keyval>=GDK_KEY_a) && (event->keyval<=GDK_KEY_z))
			keyval = event->keyval - GDK_KEY_a + GDK_KEY_A;
		else
			keyval = event->keyval;

		if (win_data->enable_key_binding)
		{
			for (i=0; i<KEYS; i++)
			{
				// g_debug("%2d: Checking key (mods = %X, kayval = %X) with mod = %X, kayval = %X (%s)",
				//	i, mods, keyval, win_data->user_keys[i].mods, win_data->user_keys[i].key,
				//	gdk_keyval_name(win_data->user_keys[i].key));
				if ((mods==win_data->user_keys[i].mods) && (keyval==win_data->user_keys[i].key))
				{
					// deal the function key
					// g_debug("CALL deal_key_press(%d)!", i);
					return deal_key_press(window, i, win_data);
				}
			}
		}
		else
		{
			if ((mods==win_data->user_keys[KEY_DISABLE_FUNCTION].mods) &&
			    (keyval==win_data->user_keys[KEY_DISABLE_FUNCTION].key))
			{
				// deal the function key
				return deal_key_press(window, KEY_DISABLE_FUNCTION, win_data);
			}
		}
	}
	return FALSE;
}

// Default function key:
//  [0] <Ctrl><~>		Disable/Enable function keys
//  [1] <Ctrl><T>		New tab
//  [2] <Ctrl><Q>		Close current tab
//  [3] <Ctrl><E>		Rename current tab
//  [4] <Ctrl><PageUp>		Switch to prev tab
//  [5] <Ctrl><PageDown>	Switch to next tab
//  [6] <Ctrl><Home>		Switch to first tab
//  [7] <Ctrl><End>		Switch to last tab
//  [8] <Ctrl><Left>		Move current page forward
//  [9] <Ctrl><Right>		Move current page backward
// [10] <Ctrl><Up>		Move current page to first
// [11] <Ctrl><Down>		Move current page to last
// [12-23] <Ctrl><F1~F12>	Switch to 1st ~ 12th tab
// [24] <Ctrl><O>		Select all the text in the Vte Terminal box
// [25] <Ctrl><X>		Copy the text to clipboard
// [26] <Ctrl><V>		Paste the text in clipboard
// [27] <Ctrl><+>		Increase the font size of current vte
// [28] <Ctrl><->		Decrease the font size of current vte
// [29] <Ctrl><Enter>		Reset the font of current tab to original size
// [30] <Alt><F11>		Asks to place window in the fullwindow/unfullwindow state
// [31] <Alt><Enter>		Asks to place window in the fullscreen/unfullscreen state
// [32] <Ctrl><PrintScreen>	Dump the runtime data of lilyterm for debug
//	<Shift><Insert>		Pase clipboard.
gboolean deal_key_press(GtkWidget *window, Key_Bindings type, struct Window *win_data)
{
#ifdef DETAIL
	g_debug("! Launch deal_key_press() with window = %p, type = %d, win_data = %p", window, type, win_data);
#endif
#ifdef SAFEMODE
	if ((win_data==NULL) || (win_data->current_vte==NULL)) return FALSE;
#endif
	gint total_page = -1;
#ifdef SAFEMODE
		if (win_data->notebook)
#endif
			total_page = gtk_notebook_get_n_pages(GTK_NOTEBOOK(win_data->notebook));
	struct Page *page_data = (struct Page *)g_object_get_data(G_OBJECT(win_data->current_vte), "Page_Data");

	// restore match_regex first...
	if (! page_data->match_regex_setted) set_hyperlink(win_data, page_data);

	switch (type)
	{
		case KEY_DISABLE_FUNCTION:
			win_data->enable_key_binding = ! win_data->enable_key_binding;
			if (win_data->enable_hyperlink)
			{
				gint i;
				for (i=0; i<total_page; i++)
					set_hyperlink(win_data,
						      get_page_data_from_nth_page(win_data, i));
			}
			break;
		case KEY_NEW_TAB:
			// add a new page
			// add_page(struct Window *win_data,
			//	    struct Page *page_data_prev,
			//	    GtkWidget *menuitem_encoding,
			//	    gchar *encoding,
			//	    gchar *locale,
			//	    gchar *environments,
			//	    gchar *user_environ,
			//	    gchar *VTE_CJK_WIDTH_STR,
			//	    gboolean add_to_next)
#ifdef SAFEMODE
			if (page_data==NULL) return FALSE;
#endif
			add_page(win_data,
				 page_data,
				 NULL,
				 page_data->encoding_str,
				 page_data->encoding_str,
				 page_data->locale,
				 NULL,
				 page_data->environ,
				 page_data->VTE_CJK_WIDTH_STR,
				 TRUE);
			break;
		case KEY_CLOSE_TAB:
			// close page
			// g_debug("Trying to close page!");
			close_page (win_data->current_vte, CLOSE_WITH_KEY_STRIKE);
			break;
		case KEY_EDIT_LABEL:
			// edit page's label
			dialog(NULL, EDIT_LABEL);
			break;
#ifdef ENABLE_FIND_STRING
		case KEY_FIND:
			dialog(NULL, FIND_STRING);
			break;
		case KEY_FIND_PREV:
			if (win_data->find_string && (win_data->find_string[0]!=0))
				find_str_in_vte (win_data->current_vte, FIND_PREV);
			else
				dialog(NULL, FIND_STRING);
			break;
		case KEY_FIND_NEXT:
			if (win_data->find_string && (win_data->find_string[0]!=0))
				find_str_in_vte (win_data->current_vte, FIND_NEXT);
			else
				dialog(NULL, FIND_STRING);
			break;
#else
		case KEY_FIND:
		case KEY_FIND_PREV:
		case KEY_FIND_NEXT:
			upgrade_dialog(ENABLE_FIND_STRING_VER);
			break;
#endif
		case KEY_PREV_TAB:
#ifdef SAFEMODE
			if (page_data==NULL) return FALSE;
#endif
			// switch to pre page
			if (page_data->page_no)
				gtk_notebook_prev_page(GTK_NOTEBOOK(win_data->notebook));
			else
				gtk_notebook_set_current_page(GTK_NOTEBOOK(win_data->notebook), total_page -1);
			break;
		case KEY_NEXT_TAB:
#ifdef SAFEMODE
			if (page_data==NULL) return FALSE;
#endif
			// switch to next page
			if (page_data->page_no == (gtk_notebook_get_n_pages(GTK_NOTEBOOK(win_data->notebook)) - 1))
				gtk_notebook_set_current_page(GTK_NOTEBOOK(win_data->notebook), 0);
			else
				gtk_notebook_next_page(GTK_NOTEBOOK(win_data->notebook));
			break;
		case KEY_FIRST_TAB:
			// switch to first page
			gtk_notebook_set_current_page(GTK_NOTEBOOK(win_data->notebook), 0);
			break;
		case KEY_LAST_TAB:
			// switch to last page
			gtk_notebook_set_current_page(GTK_NOTEBOOK(win_data->notebook), total_page-1);
			break;
		case KEY_MOVE_TAB_FORWARD:
#ifdef SAFEMODE
			if (page_data==NULL) return FALSE;
#endif
			// move current page forward
			gtk_notebook_reorder_child(GTK_NOTEBOOK(win_data->notebook), page_data->hbox,
						   page_data->page_no -1);
			break;
		case KEY_MOVE_TAB_BACKWARD:
#ifdef SAFEMODE
			if (page_data==NULL) return FALSE;
#endif
			// move current page backward
			if (page_data->page_no == (gtk_notebook_get_n_pages(GTK_NOTEBOOK(win_data->notebook))-1))
				gtk_notebook_reorder_child(GTK_NOTEBOOK(win_data->notebook), page_data->hbox, 0);
			else
				gtk_notebook_reorder_child(GTK_NOTEBOOK(win_data->notebook),
							   page_data->hbox, page_data->page_no+1);
			break;
		case KEY_MOVE_TAB_FIRST:
#ifdef SAFEMODE
			if (page_data==NULL) return FALSE;
#endif
			// move current page to first
			gtk_notebook_reorder_child(GTK_NOTEBOOK(win_data->notebook), page_data->hbox, 0);
			break;
		case KEY_MOVE_TAB_LAST:
#ifdef SAFEMODE
			if (page_data==NULL) return FALSE;
#endif
			// move current page to last
			gtk_notebook_reorder_child(GTK_NOTEBOOK(win_data->notebook), page_data->hbox, -1);
			break;
		case KEY_SWITCH_TO_TAB_1:
		case KEY_SWITCH_TO_TAB_2:
		case KEY_SWITCH_TO_TAB_3:
		case KEY_SWITCH_TO_TAB_4:
		case KEY_SWITCH_TO_TAB_5:
		case KEY_SWITCH_TO_TAB_6:
		case KEY_SWITCH_TO_TAB_7:
		case KEY_SWITCH_TO_TAB_8:
		case KEY_SWITCH_TO_TAB_9:
		case KEY_SWITCH_TO_TAB_10:
		case KEY_SWITCH_TO_TAB_11:
		case KEY_SWITCH_TO_TAB_12:
			// switch to #%d page
			if (total_page > type-KEY_SWITCH_TO_TAB_1)
				gtk_notebook_set_current_page(GTK_NOTEBOOK(win_data->notebook), type-KEY_SWITCH_TO_TAB_1);
			break;
		case KEY_NEW_WINDOW:
			create_window(NULL, NULL, 0, 0, win_data);
			break;
		case KEY_SELECT_ALL:
#ifdef ENABLE_VTE_SELECT_ALL
			vte_terminal_select_all(VTE_TERMINAL(win_data->current_vte));
#else
			upgrade_dialog(ENABLE_VTE_SELECT_ALL_VER);
#endif
			break;
		case KEY_COPY_CLIPBOARD:
			vte_terminal_copy_clipboard(VTE_TERMINAL(win_data->current_vte));
			break;
		case KEY_PASTE_CLIPBOARD:
		{
			if ( (win_data->confirm_to_paste == FALSE) ||
			     ( ! confirm_to_paste_form_clipboard(SELECTION_CLIPBOARD, win_data, page_data)))
				vte_terminal_paste_clipboard(VTE_TERMINAL(win_data->current_vte));
			break;
		}
		case KEY_COPY_PRIMARY:
			vte_terminal_copy_primary(VTE_TERMINAL(win_data->current_vte));
			break;
		case KEY_PASTE_PRIMARY:
		{
			if (win_data->confirm_to_paste)
				return confirm_to_paste_form_clipboard(SELECTION_PRIMARY, win_data, page_data);
			else
				return FALSE;
			break;
		}
		case KEY_INCREASE_FONT_SIZE:
			set_vte_font(NULL, FONT_INCREASE);
			break;
		case KEY_DECREASE_FONT_SIZE:
			set_vte_font(NULL, FONT_DECREASE);
			break;
		case KEY_RESET_FONT_SIZE:
			set_vte_font(NULL, FONT_RESET);
			break;
		case KEY_MAX_WINDOW:
		{
#ifdef USE_GTK2_GEOMETRY_METHOD
			if (win_data->fullscreen)
				gtk_window_unfullscreen(GTK_WINDOW(win_data->window));
			else
				gtk_window_fullscreen(GTK_WINDOW(win_data->window));
			break;
#endif
#ifdef USE_GTK3_GEOMETRY_METHOD
			// g_debug("deal_key_press (KEY_MAX_WINDOW): win_data->window_status = %d", win_data->window_status);
			switch (win_data->window_status)
			{
				case WINDOW_NORMAL:
					win_data->window_status = WINDOW_MAX_WINDOW;
					gtk_window_fullscreen(GTK_WINDOW(win_data->window));
					break;
				case WINDOW_MAX_WINDOW:
					win_data->window_status = WINDOW_NORMAL;
					gtk_window_unfullscreen(GTK_WINDOW(win_data->window));
					break;
				case WINDOW_MAX_WINDOW_TO_FULL_SCREEN:
				case WINDOW_FULL_SCREEN:
					win_data->window_status = WINDOW_MAX_WINDOW;
					show_or_hide_tabs_bar_and_scroll_bar(win_data);
					break;
				case WINDOW_RESIZING_TO_NORMAL:
					break;
				default:
#ifdef FATAL
					print_switch_out_of_range_error_dialog("deal_key_press", "win_data->window_status", win_data->window_status);
#endif
					break;

			}
#endif
			break;
		}
		case KEY_FULL_SCREEN:
		{
#ifdef USE_GTK2_GEOMETRY_METHOD
			// g_debug("deal_key_press: win_data->fullscreen = %d", win_data->fullscreen);
			if (win_data->fullscreen)
			{
				win_data->true_fullscreen = FALSE;
				gtk_window_unfullscreen(GTK_WINDOW(win_data->window));

				if (win_data->startup_fullscreen)
					win_data->window_status = FULLSCREEN_UNFS_ING;
				else
					win_data->window_status = FULLSCREEN_UNFS_OK;
				// g_debug("deal_key_press (unfullscreen): win_data->true_fullscreen = %d", win_data->true_fullscreen);
				win_data->true_fullscreen = FALSE;
			}
			else
			{
				win_data->true_fullscreen = TRUE;

				gboolean show_tabs_bar = hide_and_show_tabs_bar(win_data, win_data->show_tabs_bar);
				gboolean show_scroll_bar = fullscreen_show_hide_scroll_bar(win_data);
				if ((show_tabs_bar == FALSE) && (show_scroll_bar == FALSE))
					gtk_window_fullscreen(GTK_WINDOW(win_data->window));

				win_data->window_status = FULLSCREEN_FS_OK;
				// g_debug("deal_key_press (fullscreen): win_data->true_fullscreen = %d", win_data->true_fullscreen);
			}
#endif
#if defined(USE_GTK3_GEOMETRY_METHOD) || defined(UNIT_TEST)
			// g_debug("deal_key_press: win_data->window_status = %d", win_data->window_status);
			switch (win_data->window_status)
			{
				case WINDOW_NORMAL:
					save_vte_geometry(win_data);
				case WINDOW_START_WITH_FULL_SCREEN:
				case WINDOW_APPLY_PROFILE_FULL_SCREEN:
				{
					gboolean idle = (win_data->window_status==WINDOW_APPLY_PROFILE_FULL_SCREEN);
					if ((win_data->window_status==WINDOW_NORMAL) ||
					    (win_data->window_status==WINDOW_START_WITH_FULL_SCREEN))
						win_data->window_status = WINDOW_FULL_SCREEN;
					// g_debug("deal_key_press(WINDOW_START_WITH_FULL_SCREEN): win_data->window_status = %d",
					//	win_data->window_status);
					show_or_hide_tabs_bar_and_scroll_bar(win_data);
					if (idle)
						g_idle_add((GSourceFunc)idle_gtk_window_fullscreen, win_data);
					else
						gtk_window_fullscreen(GTK_WINDOW(win_data->window));
					break;
				}
				case WINDOW_MAX_WINDOW:
					win_data->window_status = WINDOW_MAX_WINDOW_TO_FULL_SCREEN;
					show_or_hide_tabs_bar_and_scroll_bar(win_data);
					break;
				case WINDOW_MAX_WINDOW_TO_FULL_SCREEN:
					win_data->window_status = WINDOW_MAX_WINDOW;
					show_or_hide_tabs_bar_and_scroll_bar(win_data);
					break;
				case WINDOW_FULL_SCREEN:
				case WINDOW_APPLY_PROFILE_NORMAL:
					gtk_window_unfullscreen(GTK_WINDOW(win_data->window));
					if (win_data->window_status==WINDOW_FULL_SCREEN)
						win_data->window_status = WINDOW_RESIZING_TO_NORMAL;
					g_idle_add((GSourceFunc)idle_show_or_hide_tabs_bar_and_scroll_bar, win_data);
					break;
				default:
					break;
			}
#endif
			break;
		}
#ifdef ENABLE_MOUSE_SCROLL
#  ifndef SCROLL_LINE
		case KEY_SCROLL_UP:
#    ifdef SAFEMODE
			if (page_data==NULL) return FALSE;
#    endif
			gtk_test_widget_click(page_data->vte, 4, 0);
			break;
		case KEY_SCROLL_DOWN:
#    ifdef SAFEMODE
			if (page_data==NULL) return FALSE;
#    endif
			gtk_test_widget_click(page_data->vte, 5, 0);
			break;
#  else
		case KEY_SCROLL_UP:
		case KEY_SCROLL_DOWN:
#  endif
		case KEY_SCROLL_UP_1_LINE:
		case KEY_SCROLL_DOWN_1_LINE:
		{
#  ifdef SAFEMODE
			if (page_data==NULL) return FALSE;
#  endif
			gdouble value = gtk_adjustment_get_value(page_data->adjustment);
			switch (type)
			{
				case KEY_SCROLL_UP_1_LINE:
					value --;
					break;
				case KEY_SCROLL_DOWN_1_LINE:
					value ++;
					break;
#  ifdef SCROLL_LINE
				case KEY_SCROLL_UP:
					value -= SCROLL_LINE;
					break;
				case KEY_SCROLL_DOWN:
					value += SCROLL_LINE;
					break;
#  endif
				default:
					break;
			}
			value = CLAMP(value, 0,
				      gtk_adjustment_get_upper (page_data->adjustment) -
				      gtk_adjustment_get_page_increment(page_data->adjustment));
			gtk_adjustment_set_value(page_data->adjustment, value);
			break;
		}
#else
		case KEY_SCROLL_UP:
		case KEY_SCROLL_DOWN:
		case KEY_SCROLL_UP_1_LINE:
		case KEY_SCROLL_DOWN_1_LINE:
			upgrade_dialog(ENABLE_MOUSE_SCROLL_VER);
			break;
#endif
		case KEY_CLEAN_SCROLLBACK_LINES:
			clean_scrollback_lines(NULL, win_data);
			break;
		case KEY_DISABLE_URL_L:
		case KEY_DISABLE_URL_R:
			if (win_data->disable_url_when_ctrl_pressed)
				clean_hyperlink(win_data, page_data);
			break;
#ifdef FATAL
		case KEY_DUMP_DATA:
#ifdef SAFEMODE
			if (page_data==NULL) return FALSE;
#endif
			dump_data(win_data, page_data);
			break;
		default:
#ifdef FATAL
			print_switch_out_of_range_error_dialog("deal_key_press", "type", type);
#endif
			break;
#endif
	}

#ifdef DISABLE_TAB_REORDER
	switch (type)
	{
		case KEY_NEW_TAB:
		case KEY_CLOSE_TAB:
		case KEY_MOVE_TAB_FORWARD:
		case KEY_MOVE_TAB_BACKWARD:
		case KEY_MOVE_TAB_FIRST:
		case KEY_MOVE_TAB_LAST:
			reorder_page_number(GTK_NOTEBOOK(win_data->notebook), NULL, 0, win_data->window);
			break;
		default:
			break;
	}
#endif
	return TRUE;
}

gboolean window_key_release(GtkWidget *window, GdkEventKey *event, struct Window *win_data)
{
#ifdef FULL
	if (event)
		g_debug("! Launch window_key_release() with key = %X (%s), state = %X, win_data = %p",
			 event->keyval, gdk_keyval_name(event->keyval), event->state, win_data);
#endif
#ifdef SAFEMODE
	if ((win_data==NULL) || (event==NULL)) return FALSE;
#endif
	if ((win_data->disable_url_when_ctrl_pressed == FALSE) || (win_data->enable_key_binding == FALSE) || (win_data->keep_vte_size)) return FALSE;

	// g_debug ("Get win_data = %p in key_press", win_data);
	// g_debug ("win_data->keep_vte_size = %X, event->state = %X", win_data->keep_vte_size, event->state);

	// don't check if only shift key pressed!
	// FIXME: GDK_asciitilde = 0x7e, it is for some keys like <F3> only.
	if ((event->state & SHIFT_ONLY_MASK) || (event->keyval > GDK_KEY_asciitilde))
	{
		// g_debug("window_key_release(): ALL_ACCELS_MASK = %X", ALL_ACCELS_MASK);
		gint mods = event->state | DUD_MASK;

		// g_debug("window_key_release(): Got the function key: %s (%X) with status= %X [%X,%X] (%X,%X)(%X,%X))",
		//	gdk_keyval_name(event->keyval), event->keyval, event->state, mods, event->keyval,
		//	(win_data->user_keys[KEY_DISABLE_URL_L].mods|GDK_CONTROL_MASK), win_data->user_keys[KEY_DISABLE_URL_L].key,
		//	(win_data->user_keys[KEY_DISABLE_URL_R].mods|GDK_CONTROL_MASK), win_data->user_keys[KEY_DISABLE_URL_R].key);

		if (((mods==(win_data->user_keys[KEY_DISABLE_URL_L].mods|GDK_CONTROL_MASK)) && (event->keyval==win_data->user_keys[KEY_DISABLE_URL_L].key)) ||
		    ((mods==(win_data->user_keys[KEY_DISABLE_URL_R].mods|GDK_CONTROL_MASK)) && (event->keyval==win_data->user_keys[KEY_DISABLE_URL_R].key)))
		{
			// g_debug("window_key_release(): call set_hyperlink()");
			struct Page *page_data = (struct Page *)g_object_get_data(G_OBJECT(win_data->current_vte), "Page_Data");
			if (! page_data->match_regex_setted)
				set_hyperlink(win_data, page_data);
		}
	}
	return FALSE;
}

gboolean window_get_focus(GtkWidget *window, GdkEventFocus *event, struct Window *win_data)
{
#ifdef DETAIL
	g_debug("! Launch window_get_focus() with window =%p, win_data = %p", window, win_data);
#endif
#ifdef SAFEMODE
	if (win_data==NULL) return FALSE;
#endif
	// g_debug("Window get focus!");

	// update current_vte when window_get_focus!
	win_data->lost_focus = FALSE;

	// And, update the current_vte info
	// current_vte = win_data->current_vte;
	active_window = window;
	menu_active_window = window;
	last_active_window = window;
	// g_debug("window_get_focus(): win_data->runtime_LC_MESSAGES = %s", win_data->runtime_LC_MESSAGES);
	setlocale(LC_MESSAGES, win_data->runtime_LC_MESSAGES);
	// g_debug("set last_active_window = %p", last_active_window);

#ifdef ENABLE_RGBA
	dim_window(win_data, 0);
#endif
	dim_vte_text(win_data, NULL, 0);
#ifdef ENABLE_BEEP_SINGAL
	set_urgent_bell(NULL, win_data);
#endif
	menu_activated = FALSE;
	return FALSE;
}

gboolean window_lost_focus(GtkWidget *window, GdkEventFocus *event, struct Window *win_data)
{
#ifdef DETAIL
	g_debug("! Launch window_lost_focus() with window =%p, win_data = %p", window, win_data);
#endif
#ifdef SAFEMODE
	if (win_data==NULL) return FALSE;
#endif
	// g_debug("Window lost focus!, win_data->current_vte = %p", win_data->current_vte);

	if (win_data->current_vte == NULL) return FALSE;

	if (win_data->page_shows_window_title &&
	    (vte_terminal_get_window_title(VTE_TERMINAL(win_data->current_vte))!=NULL))
	{
		struct Page *page_data = (struct Page *)g_object_get_data(G_OBJECT(win_data->current_vte), "Page_Data");
#ifdef SAFEMODE
		if (page_data)
		{
#endif
			if (win_data->page_shows_current_cmdline)
				win_data->window_title_tpgid = page_data->displayed_tpgid;

			if (win_data->page_shows_current_dir)
			{
				g_free(page_data->window_title_pwd);
				page_data->window_title_pwd = get_tab_name_with_current_dir(page_data->current_tpgid);
			}
#ifdef SAFEMODE
		}
#endif
	}
	win_data->lost_focus = TRUE;

#ifdef ENABLE_RGBA
	dim_window(win_data, 1);
#endif

	dim_vte_text(win_data, NULL, 1);
#ifdef ENABLE_BEEP_SINGAL
	// g_debug("window_lost_focus(): call set_urgent_bell()!!");
	set_urgent_bell(NULL, win_data);
#endif
	active_window = NULL;
	return FALSE;
}

void window_style_set(GtkWidget *window, GtkStyle *previous_style, struct Window *win_data)
{
#ifdef DETAIL
	g_debug("! Launch window_style_set() with window =%p, win_data = %p", window, win_data);
#endif
#ifdef SAFEMODE
	if (win_data==NULL) return;
#endif
	// win_data->keep_vte_size |= 8;
	// g_debug("window_resizable in window_style_set! keep_vte_size = %d", keep_vte_size);
	// window_resizable(window, win_data->current_vte, 2, 1);
	// g_debug("window_style_set(): launch keep_window_size()!");
#ifdef USE_GTK2_GEOMETRY_METHOD
#  ifdef GEOMETRY
	g_debug("@ window_style_set(for %p): Call keep_gtk2_window_size() with keep_vte_size = %x",
		window, win_data->keep_vte_size);
#  endif
	keep_gtk2_window_size (win_data, win_data->current_vte, 0x60);
#endif
#ifdef USE_GTK3_GEOMETRY_METHOD
	if (win_data->hints_type != HINTS_SKIP_ONCE)
	{
#  ifdef GEOMETRY
		fprintf(stderr, "\033[1;%dm!! window_style_set(win_data %p): Calling keep_gtk3_window_size() with hints_type = %d\n",
			ANSI_COLOR_MAGENTA, win_data, win_data->hints_type);
#  endif
		win_data->resize_type = GEOMETRY_AUTOMATIC;
		keep_gtk3_window_size(win_data, FALSE);
	}
	else
	{
#  ifdef GEOMETRY
		fprintf(stderr, "\033[1;%dm!! window_style_set(win_data %p): Calling keep_gtk3_window_size() with hints_type = %d: DON'T WORK!!\n",
			ANSI_COLOR_RED, win_data, win_data->hints_type);
#  endif
	}
#endif
}

#if defined(USE_GTK2_GEOMETRY_METHOD) || defined(UNIT_TEST)
void window_size_request (GtkWidget *window, GtkRequisition *requisition, struct Window *win_data)
{
#  ifdef DETAIL
	if (win_data)
		g_debug("! Launch window_size_request() with window =%p, win_data = %p, keep_vte_size = %x",
			window, win_data, win_data->keep_vte_size);
	else
		g_debug("! Launch window_size_request() with window =%p, win_data = %p",
			window, win_data);
#  endif
#  ifdef SAFEMODE
	if (win_data==NULL) return;
#  endif

#  ifdef GEOMETRY
	g_debug("@ window_size_request(): Got keep_vte_size (before) = %x", win_data->keep_vte_size);

	GtkRequisition window_requisition;
#  ifdef SAFEMODE
	if (window)
#  endif
		gtk_widget_get_child_requisition (window, &window_requisition);
	g_debug("@ window_size_request(): request_size (before) = %d x %d",
		window_requisition.width, window_requisition.height);
#  endif
	// [Showing/Hiding tabs bar] and [Change the vte font] won't run the following codes.
	if (win_data->keep_vte_size&0xa94a)
	{
		// g_debug("Got keep_vte_size (before) = %d", win_data->keep_vte_size);
		GtkRequisition window_requisition;

		gtk_widget_get_child_requisition (window, &window_requisition);
		// g_debug("! The requested window (for %p) requisition is %d x %d",
		//	window, window_requisition.width, window_requisition.height);
		if ( window_requisition.width>0 && window_requisition.height>0 )
		{
			GtkRequisition current_window_requisition;
			gtk_window_get_size(GTK_WINDOW(window),
					    &current_window_requisition.width,
					    &current_window_requisition.height);
#  ifdef GEOMETRY
			g_debug("@ The current window size (for %p) is %d x %d, request size is %d x %d!!",
				window,
				current_window_requisition.width,
				current_window_requisition.height,
				window_requisition.width,
				window_requisition.height);
#  endif
			if ((window_requisition.width != current_window_requisition.width) ||
			    (window_requisition.height != current_window_requisition.height))
{
#  ifdef GEOMETRY
				g_debug("@ window_size_request(): Call gtk_window_resize() to resize to %d x %d!",
					window_requisition.width, window_requisition.height);
#  endif
				gtk_window_resize(GTK_WINDOW(window),
						  window_requisition.width,
						  window_requisition.height);
}
			// else
			//	g_debug("Skip to call gtk_window_resize for %p ()!!", window);

			// 0xed = 11,101,101
			// 'Showing/Hiding tab bar' and 'Change vte font' should call gtk_window_resize() once
			win_data->keep_vte_size &= 0x54a5;
		}
		else
			win_data->keep_vte_size = 0;
#  ifdef GEOMETRY
		g_debug("@ window_size_request: Got keep_vte_size (after) = %x", win_data->keep_vte_size);
#  endif
	}
}
#endif

#ifdef USE_GTK2_GEOMETRY_METHOD
void window_size_allocate(GtkWidget *window, GtkAllocation *allocation, struct Window *win_data)
{
#  ifdef DETAIL
	if (win_data)
	g_debug("! Launch window_size_allocate() with window =%p, win_data = %p, keep_vte_size = %x",
		window, win_data, win_data->keep_vte_size);
	else
		g_debug("! Launch window_size_allocate() with window =%p, win_data = %p",
			 window, win_data);
#  endif
#  ifdef SAFEMODE
	if (win_data==NULL) return;
#  endif
	// g_debug("window_size-allocate!, and win_data->keep_vte_size = %d", win_data->keep_vte_size);
#  ifdef GEOMETRY
	g_debug("@ window_size_allocate(for %p): Got keep_vte_size (finish) = %x", window, win_data->keep_vte_size);
#  endif
	if (win_data->keep_vte_size & 0x54a5)
	{
		win_data->keep_vte_size = 0;
		// g_debug("window_size_allocate(): call window_resizable() with run_once = %d", win_data->hints_type);
		// g_debug("window_size_allocate(): (1) launch window_resizable() with window = %p!", window);

		if (! win_data->window_status)
			window_resizable(window, win_data->current_vte, win_data->hints_type);
	}
#  ifdef GEOMETRY
	g_debug("@ window_size_allocate(for %p): Got keep_vte_size (final) = %x", window, win_data->keep_vte_size);
#  endif

	if ((win_data->keep_vte_size==0) && (win_data->window_status>FULLSCREEN_NORMAL))
	{
		// g_debug("Trying keep the size of window when unfullscreen, win_data->window_status = %d", win_data->window_status);
		win_data->window_status--;

		if (win_data->window_status == FULLSCREEN_NORMAL)
		{
			// g_debug("window_size_allocate(): (2) launch window_resizable()! with window = %p!",
			//	window);
			window_resizable(window, win_data->current_vte, HINTS_NONE);
			// g_debug("window_size_allocate(): win_data->show_tabs_bar = %d",
			//	win_data->show_tabs_bar);

			hide_and_show_tabs_bar(win_data , win_data->show_tabs_bar);

			fullscreen_show_hide_scroll_bar (win_data);

			// keep_gtk2_window_size (win_data, win_data->current_vte, 128);
		}
	}

	// g_debug("win_data->keep_vte_size = %d, win_data->window_status = %d",
	//	win_data->keep_vte_size, win_data->window_status);
	if ((win_data->keep_vte_size==0) && (win_data->window_status<FULLSCREEN_NORMAL))
	{
		// g_debug("Trying keep the size of window when fullscreen, win_data->window_status = %d", win_data->window_status);
		win_data->window_status++;

		if (win_data->window_status)
			window_resizable(window, win_data->current_vte, HINTS_NONE);
		else
			gtk_window_fullscreen(GTK_WINDOW(win_data->window));
	}

#  ifdef GEOMETRY
	GtkRequisition window_requisition;
#  ifdef SAFEMODE
	if (window)
#  endif
	gtk_window_get_size(GTK_WINDOW(window), &window_requisition.width, &window_requisition.height);

	if ((window_requisition.width <= 200) || (window_requisition.height <= 200))
		fprintf(stderr, "\033[1;31m!! The final window size (win_data %p) is %d x %d (keep_vte_size = %d)\033[0m\n",
			win_data, window_requisition.width, window_requisition.height, win_data->keep_vte_size);
	else
		g_debug("! The final window size (win_data %p) is %d x %d (keep_vte_size = %d)",
			win_data, window_requisition.width, window_requisition.height, win_data->keep_vte_size);
	widget_size_allocate (window, allocation, "window");
#  endif
}
#endif

#ifdef USE_GTK3_GEOMETRY_METHOD
void window_size_allocate(GtkWidget *window, GtkAllocation *allocation, struct Window *win_data)
{
#  ifdef DETAIL
	g_debug("! Launch window_size_allocate() with window =%p, win_data = %p", window, win_data);
#  endif
#  ifdef GEOMETRY
	GtkRequisition window_requisition;
	gtk_window_get_size(GTK_WINDOW(window), &window_requisition.width, &window_requisition.height);
	gint ansi_color = ANSI_COLOR_CYAN;
	if ((window_requisition.width < 408) || (window_requisition.height < 239))
		ansi_color = ANSI_COLOR_RED;

	fprintf(stderr, "\033[1;%dm!! window_size_allocate(win_data %p): "
			"The final window size (for %p) is Requisition: %d x %d (Allocation: %d x %d)\033[0m\n",
		ansi_color, win_data, window, window_requisition.width, window_requisition.height, allocation->width, allocation->height);
	widget_size_allocate (window, allocation, "window");
#  endif
}
#endif

#if defined(ENABLE_RGBA) || defined(UNIT_TEST)
// dim_window = 0: dim the window
// dim_window = 1: light up the window
// dim_window = 2: auto detect
void dim_window(struct Window *win_data, gint dim_window)
{
#ifdef DETAIL
	g_debug("! Launch dim_window() with win_data = %p, dim_window = %d", win_data, dim_window);
#endif
#ifdef SAFEMODE
	if (win_data==NULL) return;
#endif

	// win_data->dim_window <- record if "[ ] Dim the window when inactive" is clicked.

	// dim = TRUE: dim the window
	// dim = FALSE: light up the window
	gint dim = FALSE;

	if (dim_window==2)
	{
		if (win_data->dim_window)
			dim = win_data->dim_window_expect;
		else
			dim = FALSE;
	}
	else
	{
		win_data->dim_window_expect = FALSE;
		if (dim_window)
		{
			// Trying to check if we should dim the window
			if ((! menu_activated) && (dialog_activated==0) && (win_data->lost_focus))
			{
				win_data->dim_window_expect = TRUE;
				if (win_data->dim_window)
					dim = TRUE;
			}
		}
	}

	// g_debug("win_data->window_is_inactivated = %d, dim = %d", win_data->window_is_inactivated, dim);

	// for performance, if the window was dimmed already, don't dim it again.
	if (win_data->window_is_inactivated != dim)
	{
#ifdef ENABLE_RGBA
		if (dim)
			set_window_opacity( NULL, 0, win_data->window_opacity_inactive, win_data);
		else
			set_window_opacity( NULL, 0, win_data->window_opacity, win_data);
#endif
		win_data->window_is_inactivated = dim;
	}
}
#endif

// reload=FALSE: free all datas
void destroy_window(struct Window *win_data)
{
#ifdef DETAIL
	g_debug("! Launch destroy_window() with win_data = %p!", win_data);
#endif
#ifdef SAFEMODE
	if (win_data==NULL) return;
#endif
	// Clean active_window!
	if (active_window==win_data->window)
		active_window = NULL;
	if (menu_active_window==win_data->window)
		menu_active_window = NULL;
	if (last_active_window==win_data->window)
		last_active_window = NULL;
	// g_debug("set last_active_window = %p", last_active_window);
#ifdef ENABLE_BEEP_SINGAL
	if (win_data->urgent_bell_focus_in_event_id)
		stop_urgency_hint(NULL, NULL, win_data);
#endif
	if (win_data->window)
		gtk_widget_destroy(win_data->window);

	clear_win_data(win_data);
}

void clear_win_data(struct Window *win_data)
{
#ifdef DETAIL
	g_debug("! Launch clear_win_data() win_data = %p", win_data);
#endif
#ifdef SAFEMODE
	if (win_data==NULL) return;
#endif
	int i;

	g_free(win_data->environment);
	if (win_data->warned_locale_list) g_string_free(win_data->warned_locale_list, TRUE);
	g_free(win_data->runtime_encoding);
	g_free(win_data->default_encoding);
	g_free(win_data->default_locale);
	g_free(win_data->runtime_LC_MESSAGES);
	g_free(win_data->runtime_locale_list);
	g_free(win_data->locales_list);
	g_free(win_data->default_shell);
#ifdef ENABLE_SET_EMULATION
	g_free(win_data->emulate_term);
#endif
	g_free(win_data->VTE_CJK_WIDTH_STR);
	g_free(win_data->wmclass_name);
	g_free(win_data->wmclass_class);
	g_free(win_data->shell);
	g_free(win_data->home);
	// win_data->argv;
	// win_data->command;
	g_free(win_data->init_dir);
	g_free(win_data->geometry);
	g_free(win_data->profile);
	g_free(win_data->custom_window_title_str);
	for (i=0; i<KEYS; i++)
		g_free(win_data->user_keys[i].value);
	for (i=0; i<REGEX; i++)
		g_free(win_data->user_regex[i]);
	for (i=0; i<COMMAND; i++)
	{
		g_free(win_data->user_command[i].command);
		g_free(win_data->user_command[i].environ);
		g_strfreev(win_data->user_command[i].environments);
		g_free(win_data->user_command[i].locale);
		g_free(win_data->user_command[i].match_regex);
		g_free(win_data->user_command[i].match_regex_orig);
	}
	if (win_data->custom_tab_names_str) g_string_free(win_data->custom_tab_names_str, TRUE);
	g_strfreev(win_data->custom_tab_names_strs);

	for (i=0; i<PAGE_COLOR; i++)
		g_free(win_data->user_page_color[i]);
	g_free(win_data->page_name);
	g_free(win_data->page_names);
	g_strfreev(win_data->splited_page_names);
	g_free(win_data->default_font_name);
	g_free(win_data->restore_font_name);
#ifdef ENABLE_SET_WORD_CHARS
	g_free(win_data->word_chars);
#endif
#if defined(ENABLE_VTE_BACKGROUND) || defined(FORCE_ENABLE_VTE_BACKGROUND)
	g_free(win_data->background_image);
#endif
	g_free(win_data->execute_command_whitelist);
	// g_debug("win_data->execute_command_whitelist for win_data (%p) freed!", win_data);
	g_strfreev(win_data->execute_command_whitelists);
	g_free(win_data->foreground_program_whitelist);
	g_strfreev(win_data->foreground_program_whitelists);
	g_free(win_data->background_program_whitelist);
	g_strfreev(win_data->background_program_whitelists);
	g_free(win_data->paste_texts_whitelist);
	g_strfreev(win_data->paste_texts_whitelists);
	g_free(win_data->find_string);
	// win_data->error_data.string
	// win_data->error_data.encoding

	// g_debug("clear_win_data(): clear menu = %p for win_data = %p!", win_data->menu, win_data);
	if (win_data->menu!=NULL)
		gtk_widget_destroy(win_data->menu);

	// directory can NOT be freed.
	// They are a pointer for win_data->argv.
	// g_free(win_data->directory);

//#ifdef DETAIL
//	g_debug("+ Set current_vte to NULL!");
//#endif
//	if (active_window != NULL)
//	{
//#ifdef DETAIL
//		g_debug("Set current_vte = win_data->current_vte (%p)",
//			   win_data->current_vte);
//#endif
//		// active_window != NULL: We may trying to find the activating vte
//		// It only happened run 'lilyterm -e xxx' in a lilyterm box
//		struct Window *win_data = (struct Window *)g_object_get_data(G_OBJECT(
//								active_window), "Win_Data");
//		current_vte = win_data->current_vte;
//	}
//	else
//	{
//#ifdef DETAIL
//		g_debug("CAREFUL: current_vte = NULL!!");
//#endif
//		// No activating window: It only happened run 'lilyterm -e xxx' outside lilyterm.
//		current_vte = NULL;
//	}

#ifndef UNIT_TEST
	// g_debug("clear_win_data: clear win_data = %p for window = %p!", win_data, win_data->window);
	g_free(win_data);
#endif

	// win_data=NULL;
}

void notebook_page_added(GtkNotebook *notebook, GtkWidget *child, guint page_num, struct Window *win_data)
{
#ifdef DETAIL
	g_debug("! Launch notebook_page_added() with notebook = %p, page_num = %d, and win_data = %p",
		notebook, page_num, win_data);
#endif
#ifdef SAFEMODE
	if (win_data==NULL) return;
#endif
	// g_debug("notebook_page_added !");
	struct Page *page_data = get_page_data_from_nth_page(win_data, page_num);
#ifdef SAFEMODE
	if (page_data==NULL) return;
#endif
	// g_debug("Get page_data = %p", page_data);

	// FIXME: Calling remove_notebook_page() here
	// GtkNotebook may drag a page to itself,
	// So a GtkNotebook may be page_removed() -> page_added() with the same page
	// but the GtkNotebook may be destroyed after page_removed()
	// So that we should call page_removed() here, not using "page-removed" signal... -_-|||

	// page_data->window==win_data->window -> this page is drag to itself.
	// g_debug("notebook_page_added(): page_data->window = %p", page_data->window);
	// g_debug("notebook_page_added(): win_data->window = %p", win_data->window);
	if (page_data->window==win_data->window)
	{
		reorder_page_after_added_removed_page(win_data, page_num);
		return;
	}
	// page_data->window = NULL: it is launched by add_page(),
	// page_data->window != NULL: this page is dragged from another lilyterm window.
	else if (page_data->window)
	{
		struct Window *win_data_orig = (struct Window *)g_object_get_data(G_OBJECT(page_data->window),
										  "Win_Data");
#ifdef SAFEMODE
		if (win_data_orig==NULL) return;
#endif

		// g_debug("Set page_data->window = win_data->window in notebook_page_added()");
		page_data->window = win_data->window;
		page_data->notebook = win_data->notebook;
		// apply the new win_data to this page
		apply_new_win_data_to_page(win_data_orig, win_data, page_data);
		remove_notebook_page (GTK_NOTEBOOK(win_data_orig->notebook),
				      NULL,
				      page_data->page_no,
				      win_data_orig,
				      FALSE);
	}
	else
	{
		// g_debug("Set page_data->window (NULL) = win_data->window in notebook_page_added()");
		page_data->window = win_data->window;
		page_data->notebook = win_data->notebook;

#ifdef ENABLE_TAB_REORDER
		gtk_notebook_set_tab_reorderable(GTK_NOTEBOOK(notebook), page_data->hbox, TRUE);
		gtk_notebook_set_tab_detachable(GTK_NOTEBOOK(notebook), page_data->hbox, TRUE);
#endif
		set_fill_tabs_bar(GTK_NOTEBOOK(notebook), win_data->fill_tabs_bar, page_data);

		// page_data->pid = 0 -> it is launched by add_page()
		if (page_data->pid)
		{
			// Trying to update page_data->tab_color
			// g_debug("Launch get_and_update_page_name() when pid > 0 !");
			// get_and_update_page_name(page_data);
			monitor_cmdline(page_data);
			// g_debug("page_data->tab_color = %s", page_data->tab_color);
		}
	}

	reorder_page_after_added_removed_page(win_data, page_num);

	// show the page bar if necessary
	// g_debug("Total Page (Notebook):%u",
	//	      (gtk_notebook_get_n_pages(GTK_NOTEBOOK(page_data->notebook))));

	// g_debug("win_data->true_fullscreen = %d", win_data->true_fullscreen);

//	if (gtk_notebook_get_n_pages(GTK_NOTEBOOK(page_data->notebook))==2 &&
//	    (! win_data->startup_fullscreen) &&
//	    (win_data->show_tabs_bar != FORCE_OFF) &&
//	    (dialog_activated==0))
//	{
//		// It will done in update_page_name()
//		// keep_vte_size |= 2;
//		// while (win_data->keep_vte_size)
//		//	usleep(100000);
//		hide_and_show_tabs_bar(win_data, TRUE);
//	}

	hide_and_show_tabs_bar(win_data , win_data->show_tabs_bar);

	if (gtk_notebook_get_n_pages(GTK_NOTEBOOK(page_data->notebook))==1)
	{
		// g_debug("notebook_page_added(): launch update_window_hint()!");
		// g_debug("notebook_page_added(): page_data->font_name = %s, win_data->restore_font_name = %s", page_data->font_name, win_data->restore_font_name);
#  ifdef GEOMETRY
		fprintf(stderr, "\033[1;37m!! notebook_page_added(win_data %p): call update_window_hint() for page_data = %p\033[0m\n",
			win_data, page_data);
#  endif
		update_window_hint(win_data, page_data);
	}
#  ifdef USE_GTK3_GEOMETRY_METHOD
	else
		update_window_hint(win_data, page_data);
#  endif
}


void show_close_button_on_tab(struct Window *win_data, struct Page *page_data)
{
#ifdef DETAIL
	g_debug("! Launch show_close_button_on_tab() with win_data = %p, page_data = %p!",  win_data, page_data);
#endif
#ifdef SAFEMODE
	if ((win_data==NULL) || (page_data==NULL) || (page_data->label_button==NULL)) return;
#endif
	if ((win_data->show_close_button_on_tab && (page_data->vte == win_data->current_vte)) ||
	    win_data->show_close_button_on_all_tabs)
		gtk_widget_show(page_data->label_button);
	else
		gtk_widget_hide(page_data->label_button);
}


// force_fill
void set_fill_tabs_bar(GtkNotebook *notebook, gboolean fill_tabs_bar, struct Page *page_data)
{
#ifdef DETAIL
	g_debug("! Launch set_fill_tabs_bar() with notebook = %p, fill_tabs_bar = %d, page_data = %p",
		notebook, fill_tabs_bar, page_data);
#endif
#ifdef SAFEMODE
	if ((notebook==NULL) || (page_data==NULL) || (page_data->hbox==NULL)) return;
#endif
#ifdef USE_OLD_GTK_LABEL_PACKING
	gtk_notebook_set_tab_label_packing(GTK_NOTEBOOK(notebook), page_data->hbox,
					   fill_tabs_bar, fill_tabs_bar, GTK_PACK_START);
#else
	GValue value = { 0, };
	g_value_init (&value, G_TYPE_BOOLEAN);
	g_value_set_boolean (&value, fill_tabs_bar);
	gtk_container_child_set_property (GTK_CONTAINER(notebook), page_data->hbox,
					  (gchar*) "tab-expand", &value);
	gtk_container_child_set_property (GTK_CONTAINER(notebook), page_data->hbox,
					  (gchar*) "tab-fill", &value);
	g_value_unset (&value);
#endif
}

void reorder_page_after_added_removed_page(struct Window *win_data, guint page_num)
{
#ifdef DETAIL
	g_debug("! Launch reorder_page_after_added_removed_page() with win_data = %p, page_num = %d",
		win_data, page_num);
#endif
#ifdef SAFEMODE
	if ((win_data==NULL) || (win_data->notebook==NULL)) return;
#endif
	// g_debug("Reordering the page and update page name!");
	win_data->adding_page = TRUE;
	reorder_page_number(GTK_NOTEBOOK(win_data->notebook), NULL, 0, win_data->window);
	win_data->adding_page = FALSE;

	// g_debug("Set current page to %d!", page_num);
	// gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook), page_data->page_no);
	gtk_notebook_set_current_page(GTK_NOTEBOOK(win_data->notebook), page_num);
	// g_debug("Set focus to win_data->current_vte %p!", win_data->current_vte);
	struct Page *page_data = get_page_data_from_nth_page(win_data, page_num);
#ifdef SAFEMODE
	if (page_data==NULL) return;
#endif
	// gtk_window_set_focus(GTK_WINDOW(win_data->window), win_data->current_vte);
	// g_debug("Set focus to %p", page_data->vte);
	gtk_window_set_focus(GTK_WINDOW(win_data->window), page_data->vte);
}

void remove_notebook_page(GtkNotebook *notebook, GtkWidget *child, guint page_num, struct Window *win_data, gboolean run_quit_gtk)
{
#ifdef DETAIL
	g_debug("! Launch remove_notebook_page() with notebook = %p, and win_data = %p, page_num = %d", notebook, win_data, page_num);
#endif
#ifdef SAFEMODE
	if (win_data==NULL) return;
#endif
	// g_debug("remove_notebook_page !");
	gint total_page;
	if (notebook)
		total_page = gtk_notebook_get_n_pages(GTK_NOTEBOOK(notebook));
	else
		total_page = 0;
	// g_debug("Total Page after remove_notebook_page: %d", total_page);

	if (total_page)
	{
		// g_debug("Update the hint data!");
		struct Page *page_data = (struct Page *)g_object_get_data(G_OBJECT(win_data->current_vte),
									  "Page_Data");
#ifdef SAFEMODE
		if (page_data==NULL) return;
#endif
		// g_debug("win_data->current_vte = %p, page_data = %p", win_data->current_vte, page_data);
		// g_debug("remove_notebook_page(): launch update_window_hint()!");
#  ifdef GEOMETRY
		fprintf(stderr, "\033[1;37m!! remove_notebook_page(): call update_window_hint() for pate_data = %p\033[0m\n", page_data);
#  endif
		update_window_hint(win_data, page_data);

		// g_debug("hide the tab bar if necessary");
		// g_debug("win_data->kill_color_demo_vte = %d", win_data->kill_color_demo_vte);
		if (total_page == 1 && (!win_data->kill_color_demo_vte))
		{
			// hide the page bar
			// win_data->keep_vte_size |= 6;
			// we need to set the hints, or the window size may be incorrect.
			// g_debug("window_resizable in remove_page!");
			// window_resizable(win_data->window, win_data->current_vte, 2, 1);
			// keep_gtk2_window_size (win_data, win_data->current_vte, 0x1c);
			// gtk_notebook_set_show_tabs(GTK_NOTEBOOK(notebook), FALSE);
			// gtk_notebook_set_show_border(GTK_NOTEBOOK(notebook), FALSE);

			// window_resizable(page_data->window, page_data->vte, 2, -1);
#ifdef USE_GTK2_GEOMETRY_METHOD
			hide_and_show_tabs_bar(win_data , win_data->show_tabs_bar);
#endif
#if defined(USE_GTK3_GEOMETRY_METHOD) || defined(UNIT_TEST)
			g_idle_add((GSourceFunc)idle_hide_and_show_tabs_bar, win_data);
#endif
			// g_debug("remove_notebook_page(): page_data->font_name = %s, win_data->restore_font_name = %s",
			//	page_data->font_name, win_data->restore_font_name);
			if (! compare_strings(page_data->font_name, win_data->restore_font_name, TRUE))
			{
				win_data->hints_type = HINTS_FONT_BASE;
				// g_debug("win_data->hints_type = %d", win_data->hints_type);
#  ifdef GEOMETRY
				fprintf(stderr, "\033[1;%dm!! remove_notebook_page(win_data %p): set win_data->hints_type = %d !!\033[0m\n",
					ANSI_COLOR_BLUE, win_data, win_data->hints_type);
#  endif
			}
		}
		// g_debug("Reordering the page and update page name!");
		reorder_page_number(GTK_NOTEBOOK(notebook), NULL, 0, win_data->window);
	}
	// if it is the last page, close window!
	else
	{
		// g_debug("remove_notebook_page(): Remove win_data(%p) from window_list!", win_data);
		window_list = g_list_remove (window_list, win_data);
		// g_debug("window_list = %p, dialog_activated = %d", window_list, dialog_activated);
		if (window_list == NULL && (dialog_activated==0))
		{
#ifdef DETAIL
			g_debug("+ It is the last page, exit lilyterm!");
#endif
			if (run_quit_gtk)
			{
				// done in gtk_quit_add
				// if (single_process)
				//	shutdown_socket_server();
				// g_debug("remove_notebook_page(): call gtk_main_quit()");
				quit_gtk();
#ifdef DEBUG
				g_message("%s had been closed normally!", PACKAGE);
#endif
			}
#ifndef UNIT_TEST
			else
				exit (1);
#endif
		}
		else
			destroy_window(win_data);
	}
}

GtkNotebook *create_window (GtkNotebook *notebook, GtkWidget *page, gint x, gint y,
			    struct Window *win_data)
{
#ifdef DETAIL
	if (win_data)
		g_debug("! Launch create_window() with notebook = %p, win_data = %p, VTE_CJK_WIDTH_STR = %s",
			notebook, win_data, win_data->VTE_CJK_WIDTH_STR);
	else
		g_debug("! Launch create_window() with notebook = %p, win_data = %p",
			notebook, win_data);
#endif
#ifdef SAFEMODE
	if ((win_data==NULL) || (win_data->current_vte==NULL)) return NULL;
#endif
	struct Page *page_data = (struct Page *)g_object_get_data(G_OBJECT(win_data->current_vte), "Page_Data");
#ifdef SAFEMODE
	if (page_data==NULL) return NULL;
#endif
	// we set the encoding=NULL here,
	// because new_window() with page_data != NULL will not call add_page()
	//GtkNotebook *new_window(int argc,
	//			  char *argv[],
	//			  gchar *shell,
	//			  gchar *environment,
	//			  gchar *locale_list,
	//			  gchar *PWD,
	//			  gchar *HOME,
	//			  gchar *VTE_CJK_WIDTH_STR,
	//			  gboolean VTE_CJK_WIDTH_STR_overwrite_profile,
	//			  gchar *wmclass_name,
	//			  gchar *wmclass_class,
	//			  gchar *user_environ,
	//			  gchar *encoding,
	//			  gboolean encoding_overwrite_profile,
	//			  gchar *lc_messages,
	//			  struct Window *win_data_orig,
	//			  struct Page *page_data_orig)
	// g_debug("create_window(): win_data->runtime_encoding = %s", win_data->runtime_encoding);
	// g_debug("create_window(): win_data->runtime_locale_list = %s", win_data->runtime_locale_list);

	// notebook==NULL: open a new window with <Ctrl><N>
	return new_window(0,
			  NULL,
			  win_data->shell,
			  win_data->environment,
			  win_data->runtime_locale_list,
			  page_data->pwd,
			  win_data->home,
			  win_data->VTE_CJK_WIDTH_STR,
			  TRUE,
			  win_data->wmclass_name,
			  win_data->wmclass_class,
			  NULL,
			  win_data->runtime_encoding,
			  TRUE,
			  win_data->runtime_LC_MESSAGES,
			  win_data,
			  (notebook==NULL)? NULL :page_data);
}

#if defined(USE_GTK2_GEOMETRY_METHOD) || defined(UNIT_TEST)
gboolean window_state_event (GtkWidget *widget, GdkEventWindowState *event, struct Window *win_data)
{
#  ifdef DETAIL
	g_debug("! Launch window_state_event() with win_data = %p", win_data);
#  endif
#  ifdef SAFEMODE
	if ((win_data==NULL) || (event==NULL)) return FALSE;
#  endif
	// g_debug("win_data->keep_vte_size = %d", win_data->keep_vte_size);

	if (event->changed_mask & GDK_WINDOW_STATE_FULLSCREEN)
		win_data->fullscreen = event->new_window_state & GDK_WINDOW_STATE_FULLSCREEN;
	// g_debug("window_state_event: win_data->fullscreen = %d", win_data->fullscreen);

	if (! win_data->fullscreen)
		win_data->true_fullscreen = FALSE;
	else
		win_data->true_fullscreen = TRUE;
#  ifdef GEOMETRY
	fprintf(stderr, "\033[1;31m** window_state_event(): win_data->true_fullscreen = %d\033[0m\n", win_data->true_fullscreen);
#  endif
	return FALSE;
}
#endif

#if defined(USE_GTK2_GEOMETRY_METHOD) || defined(UNIT_TEST)
void keep_gtk2_window_size (struct Window *win_data, GtkWidget *vte, guint keep_vte_size)
{
#  ifdef DETAIL
	g_debug("! Launch keep_gtk2_window_size() with win_data = %p, vte = %p, keep_vte_size = %d",
		win_data, vte, keep_vte_size);
#  endif
#  ifdef SAFEMODE
	if (win_data==NULL) return;
#  endif
	gint new_keep_vte_size = win_data->keep_vte_size | keep_vte_size;
	// g_debug("compare new_keep_vte_size = %d and win_data->keep_vte_size = %d (keep_vte_size = %d)",
	//	new_keep_vte_size, win_data->keep_vte_size, keep_vte_size);
#  ifdef GEOMETRY
	g_debug("@ keep_gtk2_window_size(): Got keep_vte_size = %x, new_keep_vte_size = %x",
		win_data->keep_vte_size, new_keep_vte_size);
#  endif
	if (new_keep_vte_size != win_data->keep_vte_size)
	{
		if (new_keep_vte_size !=3)
			new_keep_vte_size = (new_keep_vte_size & 0xA94A) ? 0xFDEF : 0x56B5;
		// g_debug("win_data->keep_vte_size = %d, and keep_vte_size = %d, new_keep_vte_size = %d",
		//	win_data->keep_vte_size, keep_vte_size, new_keep_vte_size);

		win_data->keep_vte_size = new_keep_vte_size;

		// g_debug("keep_gtk2_window_size(): launch window_resizable()! with window = %p!", win_data->window);
		window_resizable(win_data->window, vte, HINTS_NONE);
	}
#  ifdef GEOMETRY
	g_debug("@ keep_gtk2_window_size(): Got keep_vte_size (final) = %x", win_data->keep_vte_size);
#  endif
}
#endif

#if defined(USE_GTK3_GEOMETRY_METHOD) || defined(UNIT_TEST)
void keep_gtk3_window_size(struct Window *win_data, gboolean idle)
{
#  ifdef DETAIL
	g_debug("! Launch keep_gtk3_window_size() with win_data = %p", win_data);
#  endif
#  ifdef SAFEMODE
	if (win_data==NULL) return;
#  endif
#  ifdef GEOMETRY
	// g_debug("keep_gtk3_window_size(): win_data->fullscreen = %d, win_data->window_status = %d, win_data->current_vte = %p",
	//	win_data->fullscreen, win_data->window_status, win_data->current_vte);
	fprintf(stderr, "\033[1;%dm** keep_gtk3_window_size(): win_data->window_status = %d, win_data->hints_type = %d\033[0m\n",
		ANSI_COLOR_MAGENTA, win_data->window_status, win_data->hints_type);
#  endif
	if ((win_data->window_status != WINDOW_NORMAL) || (win_data->current_vte==NULL)) return;

	switch (win_data->hints_type)
	{
		case HINTS_FONT_BASE:
		{
			if (win_data->resize_type == GEOMETRY_AUTOMATIC)
			{
				win_data->geometry_width = vte_terminal_get_column_count(VTE_TERMINAL(win_data->current_vte));
				win_data->geometry_height = vte_terminal_get_row_count(VTE_TERMINAL(win_data->current_vte));
			}
#  ifdef GEOMETRY
			gint ansi_color = ANSI_COLOR_MAGENTA;
			if ((win_data->geometry_width<40) || (win_data->geometry_height<12))
				ansi_color = ANSI_COLOR_RED;
			fprintf(stderr, "\033[1;%dm** keep_gtk3_window_size(HINTS_FONT_BASE)(win_data %p, resize_type = %d): "
				"final vte size = %ld x %ld (font: %ld x %ld)\033[0m\n",
				ansi_color, win_data, win_data->resize_type, win_data->geometry_width, win_data->geometry_height,
				vte_terminal_get_char_width(VTE_TERMINAL(win_data->current_vte)),
				vte_terminal_get_char_height(VTE_TERMINAL(win_data->current_vte)));
#  endif
			break;
		}
		case HINTS_NONE:
		{
			// find the current vte size
			GtkAllocation vte_size = {0};
			gtk_widget_get_allocation(win_data->current_vte, &vte_size);
			struct Page *page_data = (struct Page *)g_object_get_data(G_OBJECT(win_data->current_vte), "Page_Data");
#  ifdef GEOMETRY
			gint ansi_color = ANSI_COLOR_MAGENTA;
			if ((vte_size.width==0) || (vte_size.height==0) || (vte_size.width< 200) || (vte_size.height<239))
				ansi_color = ANSI_COLOR_RED;
			fprintf(stderr, "\033[1;%dm** keep_gtk3_window_size(HINTS_NONE)(win_data %p, current_vte %p, resize_type = %d):  "
				"final vte size = %ld x %ld (%d x %d) (%ld x %ld) "
				"(font: %ld x %ld) (border: L %d, R %d, T %d, B %d)\033[0m\n",
				ansi_color, win_data, win_data->current_vte, win_data->resize_type,
				win_data->geometry_width, win_data->geometry_height, vte_size.width, vte_size.height,
				vte_terminal_get_column_count(VTE_TERMINAL(win_data->current_vte)),
				vte_terminal_get_row_count(VTE_TERMINAL(win_data->current_vte)),
				vte_terminal_get_char_width(VTE_TERMINAL(win_data->current_vte)),
				vte_terminal_get_char_height(VTE_TERMINAL(win_data->current_vte)),
				page_data->border->left, page_data->border->right, page_data->border->top, page_data->border->bottom);
#  endif
			if (win_data->resize_type == GEOMETRY_AUTOMATIC)
			{
				win_data->geometry_width = vte_size.width - page_data->border->left - page_data->border->right;
				win_data->geometry_height = vte_size.height - page_data->border->top - page_data->border->bottom;
			}
			break;
		}
		case HINTS_SKIP_ONCE:
			return;
	}
	// g_debug("keep_gtk3_window_size(): FINAL: hints_type = %d, column = %ld, row = %ld",
	//	win_data->hints_type, win_data->geometry_width, win_data->geometry_height);
	// Dirty Hack: column=1 and row=1 when vte is initing...
	if (!((win_data->geometry_width < 0) && (win_data->geometry_height < 0)))
	{
		if (idle)
			g_idle_add((GSourceFunc)idle_to_resize_window, win_data);
		else
			idle_to_resize_window(win_data);
	}
#  ifdef GEOMETRY
	else
		fprintf(stderr, "\033[1;31m** keep_gtk3_window_size(): resize failed with win_data->hints_type = %d, column = %ld, row = %ld !!\033[0m\n",
			win_data->hints_type, win_data->geometry_width, win_data->geometry_height);
#  endif
}
#endif

#if defined(USE_GTK3_GEOMETRY_METHOD) || defined(UNIT_TEST)
void resize_to_exist_widget(struct Window *win_data)
{
#  ifdef DETAIL
	g_debug("! Launch resize_to_exist_widget() with win_data = %p", win_data);
#  endif
#  ifdef SAFEMODE
	if (win_data==NULL) return;
#  endif
	GtkWidget *widget = win_data->notebook;
	gboolean show_tabs_bar = get_hide_or_show_tabs_bar(win_data, win_data->show_tabs_bar);
	if (show_tabs_bar == FALSE)
	{
		// find the current vte size
#  ifdef SAFEMODE
		if (win_data->current_vte==NULL) return;
#  endif
		struct Page *page_data = (struct Page *)g_object_get_data(G_OBJECT(win_data->current_vte), "Page_Data");
#  ifdef SAFEMODE
		if (page_data) return;
#  endif
			widget = page_data->hbox;
	}

	GtkAllocation allocation = {0};
	gtk_widget_get_allocation(widget, &allocation);

	gtk_window_resize (GTK_WINDOW(win_data->window), allocation.width, allocation.height);
#  ifdef GEOMETRY
	fprintf(stderr, "\033[1;%dm!! resize_to_exist_widget(win_data %p, show_tabs_bar = %d): resize window size to %d x %d!!\033[0m\n",
		ANSI_COLOR_YELLOW, win_data, show_tabs_bar, allocation.width, allocation.height);
#  endif
}
#endif

#if defined(USE_GTK3_GEOMETRY_METHOD) || defined(UNIT_TEST)
void save_vte_geometry(struct Window *win_data)
{
#  ifdef DETAIL
	g_debug("! Launch save_vte_geometry() with win_data = %p", win_data);
#  endif
#  ifdef SAFEMODE
	if ((win_data==NULL) || (win_data->notebook==NULL)) return;
#  endif

	if (win_data->hints_type==HINTS_NONE)
	{
		GtkAllocation allocation = {0};
		gtk_widget_get_allocation(win_data->current_vte, &allocation);
		win_data->geometry_width = allocation.width;
		win_data->geometry_height = allocation.height;
	}
	else
	{
		win_data->geometry_width = vte_terminal_get_column_count(VTE_TERMINAL(win_data->current_vte));
		win_data->geometry_height = vte_terminal_get_row_count(VTE_TERMINAL(win_data->current_vte));
	}
#  ifdef GEOMETRY
	fprintf(stderr, "\033[1;35m!! save_vte_geometry(): set win_data->geometry_width = %ld, win_data->geometry_height = %ld\033[0m\n",
		win_data->geometry_width, win_data->geometry_height);
#  endif
}
#endif

#if defined(USE_GTK3_GEOMETRY_METHOD) || defined(UNIT_TEST)
gboolean idle_show_or_hide_tabs_bar_and_scroll_bar(struct Window *win_data)
{
	if (check_if_win_data_is_still_alive(win_data)) show_or_hide_tabs_bar_and_scroll_bar(win_data);
	return FALSE;
}
#endif

#if defined(USE_GTK3_GEOMETRY_METHOD) || defined(UNIT_TEST)
gboolean idle_gtk_window_fullscreen(struct Window *win_data)
{
	if (check_if_win_data_is_still_alive(win_data)) gtk_window_fullscreen(GTK_WINDOW(win_data->window));
	return FALSE;
}
#endif

#if defined(USE_GTK3_GEOMETRY_METHOD) || defined(UNIT_TEST)
gboolean show_or_hide_tabs_bar_and_scroll_bar(struct Window *win_data)
{
#  ifdef DETAIL
	g_debug("! Launch show_or_hide_tabs_bar_and_scroll_bar() with  win_data = %p", win_data);
#  endif
#  ifdef SAFEMODE
	if (win_data==NULL) return FALSE;
#  endif
	// g_debug("show_or_hide_tabs_bar_and_scroll_bar(): win_data->window_status = %d", win_data->window_status);
	hide_and_show_tabs_bar(win_data, win_data->show_tabs_bar);
	fullscreen_show_hide_scroll_bar(win_data);

	switch (win_data->window_status)
	{
		case WINDOW_NORMAL:
		case WINDOW_MAX_WINDOW:
			window_resizable(win_data->window, win_data->current_vte, win_data->hints_type);
			break;
		case WINDOW_RESIZING_TO_NORMAL:
			window_resizable(win_data->window, win_data->current_vte, win_data->hints_type);
			win_data->window_status = WINDOW_NORMAL;
			break;
		case WINDOW_MAX_WINDOW_TO_FULL_SCREEN:
		case WINDOW_FULL_SCREEN:
		case WINDOW_APPLY_PROFILE_NORMAL:
		case WINDOW_APPLY_PROFILE_FULL_SCREEN:
			break;
		default:
#  ifdef FATAL
			print_switch_out_of_range_error_dialog("show_or_hide_tabs_bar_and_scroll_bar",
							       "win_data->window_status", win_data->window_status);
#  endif
			break;
	}
	return FALSE;
}
#endif

#if defined(USE_GTK3_GEOMETRY_METHOD) || defined(UNIT_TEST)
gboolean idle_to_resize_window(struct Window *win_data)
{
#  ifdef DETAIL
	g_debug("! Launch idle_to_resize_window() with win_data = %p", win_data);
#  endif
	if (check_if_win_data_is_still_alive(win_data))
		gtk_window_resize_to_geometry(GTK_WINDOW(win_data->window), win_data->geometry_width, win_data->geometry_height);
#  ifdef GEOMETRY
	fprintf(stderr, "\033[1;%dm** idle_to_resize_window(): resize with hints_type = %d, column = %ld, row = %ld !!\033[0m\n",
		ANSI_COLOR_MAGENTA, win_data->hints_type, win_data->geometry_width, win_data->geometry_height);
#  endif
	return FALSE;
}
#endif

#if defined(USE_GTK3_GEOMETRY_METHOD) || defined(UNIT_TEST)
gboolean idle_set_vte_font_to_selected(struct Window *win_data)
{
	if (check_if_win_data_is_still_alive(win_data)) set_vte_font(NULL, FONT_SET_TO_SELECTED);
	return FALSE;
}
#endif

#if defined(USE_GTK3_GEOMETRY_METHOD) || defined(UNIT_TEST)
gboolean idle_hide_and_show_tabs_bar(struct Window *win_data)
{
#ifdef DETAIL
	g_debug("! Launch idle_hide_and_show_tabs_bar() with win_data = %p", win_data);
#endif
	if (check_if_win_data_is_still_alive(win_data)) hide_and_show_tabs_bar(win_data, win_data->show_tabs_bar);
	return FALSE;
}
#endif

#ifdef FATAL
void dump_data (struct Window *win_data, struct Page *page_data)
{
#ifdef SAFEMODE
	if ((win_data==NULL) || (page_data==NULL)) return;
#endif
	gint i, j;

// ---- win_data ---- //

	g_debug("- win_data->environment = %s", win_data->environment);
#ifdef SAFEMODE
	if (win_data->warned_locale_list)
#endif
		g_debug("- win_data->warned_locale_list->str = %s", win_data->warned_locale_list->str);
	g_debug("- win_data->runtime_encoding = %s", win_data->runtime_encoding);
	g_debug("- win_data->default_encoding = %s", win_data->default_encoding);
	g_debug("- win_data->runtime_locale_list = %s", win_data->runtime_locale_list);
	g_debug("- win_data->locales_list = %s", win_data->locales_list);
	g_debug("- win_data->default_shell = %s", win_data->default_shell);
#ifdef ENABLE_SET_EMULATION
	g_debug("- win_data->emulate_term = %s", win_data->emulate_term);
#endif
	g_debug("- win_data->VTE_CJK_WIDTH = %d", win_data->VTE_CJK_WIDTH);
	g_debug("- win_data->VTE_CJK_WIDTH_STR = %s", win_data->VTE_CJK_WIDTH_STR);
	g_debug("- win_data->wmclass_name = %s", win_data->wmclass_name);
	g_debug("- win_data->wmclass_class = %s", win_data->wmclass_class);
	g_debug("- win_data->shell = %s", win_data->shell);
	g_debug("- win_data->home = %s", win_data->home);
	g_debug("- win_data->argc = %d", win_data->argc);
	print_array("win_data->argv", win_data->argv);
	g_debug("- win_data->command = %s", win_data->command);
	g_debug("- win_data->hold = %d", win_data->hold);
	g_debug("- win_data->init_tab_number = %d", win_data->init_tab_number);
	g_debug("- win_data->login_shell = %d", win_data->login_shell);
	g_debug("- win_data->utmp = %d", win_data->utmp);
	g_debug("- win_data->init_dir = %s", win_data->init_dir);
	g_debug("- win_data->geometry = %s", win_data->geometry);
	g_debug("- win_data->subitem_new_window_from_list = %p", win_data->subitem_new_window_from_list);
	g_debug("- win_data->menuitem_new_window_from_list = %p", win_data->menuitem_new_window_from_list);
	g_debug("- win_data->subitem_load_profile_from_list = %p", win_data->subitem_load_profile_from_list);
	g_debug("- win_data->menuitem_load_profile_from_list = %p", win_data->menuitem_load_profile_from_list);
	g_debug("- win_data->use_custom_profile = %d", win_data->use_custom_profile);
	g_debug("- win_data->profile = %s", win_data->profile);
	g_debug("- win_data->specified_profile = %s", win_data->specified_profile);
	g_debug("- win_data->profile_dir_modtime = %ld", win_data->profile_dir_modtime);
	g_debug("- win_data->menuitem_auto_save = %p", win_data->menuitem_auto_save);
	g_debug("- win_data->auto_save = %d", win_data->auto_save);
#ifdef USE_GTK2_GEOMETRY_METHOD
	g_debug("- win_data->fullscreen = %d", win_data->fullscreen);
	g_debug("- win_data->true_fullscreen = %d", win_data->true_fullscreen);
	g_debug("- win_data->startup_fullscreen = %d", win_data->startup_fullscreen);
#endif
	g_debug("- win_data->show_tabs_bar = %d", win_data->show_tabs_bar);
	// g_debug("- win_data->fullscreen_show_scroll_bar = %d", win_data->fullscreen_show_scroll_bar);
	g_debug("- win_data->window_status = %d", win_data->window_status);
	g_debug("- win_data->window = %p", win_data->window);
	g_debug("- win_data->notebook = %p", win_data->notebook);
	g_debug("- win_data->show_close_button_on_tab = %d", win_data->show_close_button_on_tab);
	g_debug("- win_data->show_close_button_on_all_tabs = %d", win_data->show_close_button_on_all_tabs);
	g_debug("- win_data->current_vte = %p", win_data->current_vte);
	g_debug("- win_data->window_title_shows_current_page = %d", win_data->window_title_shows_current_page);
	g_debug("- win_data->window_title_append_package_name = %d", win_data->window_title_append_package_name);
	g_debug("- win_data->custom_window_title_str = %s", win_data->custom_window_title_str);
	if (win_data->custom_tab_names_str)
		g_debug("- win_data->win_data->custom_tab_names_str = %s", win_data->custom_tab_names_str->str);
	print_array("win_data->custom_tab_names_strs", win_data->custom_tab_names_strs);
	// g_debug("- win_data->update_window_title_only = %d", win_data->update_window_title_only);
	g_debug("- win_data->window_title_tpgid = %d", win_data->window_title_tpgid);
	g_debug("- win_data->use_rgba = %d", win_data->use_rgba);
	g_debug("- win_data->use_rgba_orig = %d", win_data->use_rgba_orig);
#ifdef ENABLE_RGBA
	g_debug("- win_data->transparent_window = %d", win_data->transparent_window);
	g_debug("- win_data->window_opacity = %1.3f", win_data->window_opacity);
	g_debug("- win_data->dim_window = %d", win_data->dim_window);
	g_debug("- win_data->transparent_window_inactive = %d", win_data->transparent_window_inactive);
	g_debug("- win_data->window_opacity_inactive = %1.3f", win_data->window_opacity_inactive);
	g_debug("- win_data->window_is_inactivated = %d", win_data->window_is_inactivated);
	g_debug("- win_data->dim_window_expect = %d", win_data->dim_window_expect);
	g_debug("- win_data->menuitem_dim_window = %p", win_data->menuitem_dim_window);
#endif
	g_debug("- win_data->enable_key_binding = %d", win_data->enable_key_binding);
	for (i=0; i<KEYS; i++)
	{
		g_debug("- win_data->user_keys[%d].value = %s", i, win_data->user_keys[i].value);
		g_debug("- win_data->user_keys[%d].key = 0x%x", i, win_data->user_keys[i].key);
		g_debug("- win_data->user_keys[%d].mods = 0x%x", i, win_data->user_keys[i].mods);
	}
	g_debug("- win_data->hints_type = %d", win_data->hints_type);
#ifdef USE_GTK3_GEOMETRY_METHOD
	g_debug("- win_data->resize_type = %d", win_data->resize_type);
	g_debug("- win_data->geometry_width = %ld", win_data->geometry_width);
	g_debug("- win_data->geometry_height = %ld", win_data->geometry_height);
#endif
	g_debug("- win_data->lost_focus = %d", win_data->lost_focus);
#ifdef USE_GTK2_GEOMETRY_METHOD
	g_debug("- win_data->keep_vte_size = %d", win_data->keep_vte_size);
#endif
	g_debug("- win_data->menu = %p", win_data->menu);
	g_debug("- win_data->menu_activated = %d", win_data->menu_activated);
	g_debug("- win_data->encoding_sub_menu = %p", win_data->encoding_sub_menu);
	// g_debug("- win_data->encoding_group = %p", win_data->encoding_group);
	g_debug("- win_data->default_menuitem_encoding = %p", win_data->default_menuitem_encoding);
	if (win_data->default_menuitem_encoding)
		g_debug("- win_data->default_encoding_str = %s", gtk_widget_get_name (win_data->default_menuitem_encoding));
#ifdef ENABLE_RGBA
	g_debug("- win_data->menuitem_trans_win = %p", win_data->menuitem_trans_win);
#endif
	g_debug("- win_data->menuitem_trans_bg = %p", win_data->menuitem_trans_bg);
//	g_debug("- win_data->menuitem_scrollback_lines = %p", win_data->menuitem_scrollback_lines);
	g_debug("- win_data->show_color_selection_menu = %d", win_data->show_color_selection_menu);
	g_debug("- win_data->show_resize_menu = %d", win_data->show_resize_menu);
	g_debug("- win_data->font_resize_ratio = %1.3f", win_data->font_resize_ratio);
	g_debug("- win_data->window_resize_ratio = %1.3f", win_data->window_resize_ratio);
	g_debug("- win_data->show_background_menu = %d", win_data->show_background_menu);
#ifdef ENABLE_IM_APPEND_MENUITEMS
	g_debug("- win_data->show_input_method_menu = %d", win_data->show_input_method_menu);
#endif
	g_debug("- win_data->show_exit_menu = %d", win_data->show_exit_menu);
	g_debug("- win_data->show_change_page_name_menu = %d", win_data->show_change_page_name_menu);
	g_debug("- win_data->enable_hyperlink = %d", win_data->enable_hyperlink);
	for (i=0; i<REGEX; i++)
		g_debug("- win_data->user_regex[%d] = %s", i, win_data->user_regex[i]);
	for (i=0; i<COMMAND; i++)
	{
		g_debug("- win_data->user_command[%d].command = %s", i, win_data->user_command[i].command);
		g_debug("- win_data->user_command[%d].method = %d", i, win_data->user_command[i].method);
		g_debug("- win_data->user_command[%d].environ = %s", i, win_data->user_command[i].environ);
		gchar *temp_str = g_strdup_printf("- win_data->user_command[%d].environments", i);
		// print_array(temp_str, win_data->user_command[i].environments);
		g_free(temp_str);
		g_debug("- win_data->user_command[%d].VTE_CJK_WIDTH = %d",
			i,
			win_data->user_command[i].VTE_CJK_WIDTH);
		g_debug("- win_data->user_command[%d].locale = %s", i, win_data->user_command[i].locale);
		g_debug("- win_data->user_command[%d].match_regex = %s", i, win_data->user_command[i].match_regex);
		g_debug("- win_data->user_command[%d].match_regex_orig = %s", i, win_data->user_command[i].match_regex_orig);
	}
	g_debug("- win_data->menuitem_copy_url = %p", win_data->menuitem_copy_url);
	if (win_data->menuitem_copy_url)
		g_debug("- win_data->menuitem_copy_url->name = %s", gtk_widget_get_name (win_data->menuitem_copy_url));

	g_debug("- win_data->menuitem_dim_text = %p", win_data->menuitem_dim_text);
	g_debug("- win_data->menuitem_cursor_blinks = %p", win_data->menuitem_cursor_blinks);
	g_debug("- win_data->menuitem_allow_bold_text = %p", win_data->menuitem_allow_bold_text);
	g_debug("- win_data->menuitem_open_url_with_ctrl_pressed = %p", win_data->menuitem_open_url_with_ctrl_pressed);
	g_debug("- win_data->menuitem_disable_url_when_ctrl_pressed = %p", win_data->menuitem_disable_url_when_ctrl_pressed);
	g_debug("- win_data->menuitem_audible_bell = %p", win_data->menuitem_audible_bell);
#ifdef ENABLE_VISIBLE_BELL
	g_debug("- win_data->menuitem_visible_bell = %p", win_data->menuitem_visible_bell);
#endif
#ifdef ENABLE_BEEP_SINGAL
	g_debug("- win_data->menuitem_urgent_bell = %p", win_data->menuitem_urgent_bell);
#endif
	g_debug("- win_data->menuitem_show_tabs_bar = %p", win_data->menuitem_show_tabs_bar);
	g_debug("- win_data->menuitem_hide_tabs_bar = %p", win_data->menuitem_hide_tabs_bar);
	g_debug("- win_data->menuitem_hide_scroll_bar = %p", win_data->menuitem_hide_scroll_bar);
	g_debug("- win_data->show_copy_paste_menu = %d", win_data->show_copy_paste_menu);
	g_debug("- win_data->embedded_copy_paste_menu = %d", win_data->embedded_copy_paste_menu);
	g_debug("- win_data->menuitem_copy = %p", win_data->menuitem_copy);
	g_debug("- win_data->menuitem_paste = %p", win_data->menuitem_paste);
	g_debug("- win_data->menuitem_clipboard = %p", win_data->menuitem_clipboard);
	g_debug("- win_data->menuitem_primary = %p", win_data->menuitem_primary);
	g_debug("- win_data->custom_cursor_color = %d", win_data->custom_cursor_color);
	print_color(-1, "win_data->cursor_color", win_data->cursor_color);
	for (i=0; i<THEME; i++)
	{
		for (j=0; j<COLOR; j++)
		{
			gchar *temp_str = g_strdup_printf("win_data->custom_color_theme[%d].color[%02d]", i, j);
			print_color(-1, temp_str, win_data->custom_color_theme[i].color[j]);
			g_free(temp_str);
		}
	}
	g_debug("- win_data->invert_color = %d", win_data->invert_color);
	g_debug("- win_data->menuitem_invert_color = %p", win_data->menuitem_invert_color);
	g_debug("- win_data->have_custom_color = %d", win_data->have_custom_color);
	g_debug("- win_data->use_custom_theme = %d", win_data->use_custom_theme);
	g_debug("- win_data->ansi_color_sub_menu = %p", win_data->ansi_color_sub_menu);
	g_debug("- win_data->ansi_color_menuitem = %p", win_data->ansi_color_menuitem);
	for (i=0; i<COLOR; i++)
	{
		print_color(i, "win_data->color", win_data->color[i]);
		print_color(i, "win_data->color_inactive", win_data->color_inactive[i]);
	}
	for (i=0; i<THEME*2; i++)
	{
		g_debug("- win_data->menuitem_theme[%d] = %p", i, win_data->menuitem_theme[i]);
		if (win_data->menuitem_theme[i])
			g_debug("- win_data->menuitem_theme[%d] (name)= %s", i, gtk_widget_get_name(win_data->menuitem_theme[i]));
	}
	for (i=0; i<ERASE_BINDING; i++)
		g_debug("- win_data->menuitem_erase_binding[%d] = %p", i, win_data->menuitem_erase_binding[i]);
	g_debug("- win_data->current_menuitem_erase_binding = %p", win_data->current_menuitem_erase_binding);
#ifdef ENABLE_CURSOR_SHAPE
	for (i=0; i<CURSOR_SHAPE; i++)
		g_debug("- win_data->menuitem_cursor_shape[%d] = %p", i, win_data->menuitem_cursor_shape[i]);
	g_debug("- win_data->current_menuitem_cursor_shape = %p", win_data->current_menuitem_cursor_shape);
#endif
	g_debug("- win_data->color_brightness = %1.3f", win_data->color_brightness);
	g_debug("- win_data->color_brightness_inactive = %1.3f", win_data->color_brightness_inactive);
	g_debug("- win_data->page_width = %d", win_data->page_width);
	g_debug("- win_data->fill_tabs_bar = %d", win_data->fill_tabs_bar);
	g_debug("- win_data->tabs_bar_position = %d", win_data->tabs_bar_position);
	g_debug("- win_data->page_name = %s", win_data->page_name);
	g_debug("- win_data->reuse_page_names = %d", win_data->reuse_page_names);
	g_debug("- win_data->page_names = %s", win_data->page_names);
	print_array("win_data->splited_page_names", win_data->splited_page_names);
	g_debug("- win_data->page_names_no = %d", win_data->page_names_no);
	g_debug("- win_data->max_page_names_no = %d", win_data->max_page_names_no);
	g_debug("- win_data->page_shows_number = %d", win_data->page_shows_number);
	g_debug("- win_data->page_shows_encoding = %d", win_data->page_shows_encoding);
	g_debug("- win_data->page_shows_current_cmdline = %d", win_data->page_shows_current_cmdline);
	g_debug("- win_data->page_shows_window_title = %d", win_data->page_shows_window_title);
	g_debug("- win_data->page_shows_current_dir = %d", win_data->page_shows_current_dir);
	g_debug("- win_data->use_color_page = %d", win_data->use_color_page);
	g_debug("- win_data->check_root_privileges = %d", win_data->check_root_privileges);
	g_debug("- win_data->bold_current_page_name = %d", win_data->bold_current_page_name);
	g_debug("- win_data->bold_action_page_name = %d", win_data->bold_action_page_name);
	for (i=0; i<PAGE_COLOR; i++)
		g_debug("- win_data->user_page_color[%d] = %s", i, win_data->user_page_color[i]);
	g_debug("- win_data->font_anti_alias = %d", win_data->font_anti_alias);
	g_debug("- win_data->default_font_name = %s", win_data->default_font_name);
	g_debug("- win_data->restore_font_name = %s", win_data->restore_font_name);
	g_debug("- win_data->default_column = %ld", win_data->default_column);
	g_debug("- win_data->default_row = %ld", win_data->default_row);
#ifdef ENABLE_SET_WORD_CHARS
	g_debug("- win_data->word_chars = %s", win_data->word_chars);
#endif
	g_debug("- win_data->show_scroll_bar = %d", win_data->show_scroll_bar);
	g_debug("- win_data->scroll_bar_position = %d", win_data->scroll_bar_position);
#if defined(ENABLE_VTE_BACKGROUND) || defined(FORCE_ENABLE_VTE_BACKGROUND)
	g_debug("- win_data->transparent_background = %d", win_data->transparent_background);
	g_debug("- win_data->background_saturation = %1.3f", win_data->background_saturation);
	g_debug("- win_data->scroll_background = %d", win_data->scroll_background);
	g_debug("- win_data->background_image = %s", win_data->background_image);
#endif
//	g_debug("- win_data->use_scrollback_lines = %d", win_data->use_scrollback_lines);
	g_debug("- win_data->scrollback_lines = %d", win_data->scrollback_lines);
	g_debug("- win_data->dim_text = %d", win_data->dim_text);
	g_debug("- win_data->cursor_blinks = %d", win_data->cursor_blinks);
	g_debug("- win_data->allow_bold_text = %d", win_data->allow_bold_text);
	g_debug("- win_data->open_url_with_ctrl_pressed = %d", win_data->open_url_with_ctrl_pressed);
	g_debug("- win_data->disable_url_when_ctrl_pressed = %d", win_data->disable_url_when_ctrl_pressed);
	g_debug("- win_data->audible_bell = %d", win_data->audible_bell);
#ifdef ENABLE_VISIBLE_BELL
	g_debug("- win_data->visible_bell = %d", win_data->visible_bell);
#endif
#ifdef ENABLE_BEEP_SINGAL
	g_debug("- win_data->urgent_bell = %d", win_data->urgent_bell);
	g_debug("- win_data->urgent_bell_status = %d", win_data->urgent_bell_status);
	g_debug("- win_data->urgent_bell_focus_in_event_id = %ld", win_data->urgent_bell_focus_in_event_id);
#endif
	g_debug("- win_data->erase_binding = %d", win_data->erase_binding);
#ifdef ENABLE_CURSOR_SHAPE
	g_debug("- win_data->cursor_shape = %d", win_data->cursor_shape);
#endif
	g_debug("- win_data->prime_user_datas_inited = %d", win_data->prime_user_datas_inited);
	g_debug("- win_data->prime_user_settings_inited = %d", win_data->prime_user_settings_inited);
	g_debug("- win_data->confirm_to_close_multi_tabs = %d", win_data->confirm_to_close_multi_tabs);
	g_debug("- win_data->confirm_to_execute_command = %d", win_data->confirm_to_execute_command);
	g_debug("- win_data->execute_command_whitelist = %s", win_data->execute_command_whitelist);
	print_array("win_data->execute_command_whitelists", win_data->execute_command_whitelists);
	g_debug("- win_data->execute_command_in_new_tab = %d", win_data->execute_command_in_new_tab);
	g_debug("- win_data->join_as_new_tab = %d", win_data->join_as_new_tab);
	g_debug("- win_data->foreground_program_whitelist = %s", win_data->foreground_program_whitelist);
	print_array("win_data->foreground_program_whitelists", win_data->foreground_program_whitelists);
	g_debug("- win_data->background_program_whitelist = %s", win_data->background_program_whitelist);
	print_array("win_data->background_program_whitelists", win_data->background_program_whitelists);
	g_debug("- win_data->confirm_to_paste = %d", win_data->confirm_to_paste);
	g_debug("- win_data->paste_texts_whitelist = %s", win_data->paste_texts_whitelist);
	print_array("win_data->paste_texts_whitelists", win_data->paste_texts_whitelists);

	g_debug("- win_data->find_string = %s", win_data->find_string);
	g_debug("- win_data->find_case_sensitive = %d", win_data->find_case_sensitive);
	g_debug("- win_data->find_use_perl_regular_expressions = %d", win_data->find_use_perl_regular_expressions);
	print_color(-1, "win_data->find_entry_bg_color", win_data->find_entry_bg_color);
	print_color(-1, "win_data->find_entry_current_bg_color", win_data->find_entry_current_bg_color);

	g_debug("- win_data->checking_menu_item = %d", win_data->checking_menu_item);
	g_debug("- win_data->kill_color_demo_vte = %d", win_data->kill_color_demo_vte);
//	g_debug("- win_data->checked_profile_version = %d", win_data->checked_profile_version);
	g_debug("- win_data->confirmed_profile_is_invalid = %d", win_data->confirmed_profile_is_invalid);
	g_debug("- win_data->temp_data = %s", win_data->temp_data);

// ---- page_data ---- //
	g_debug("- page_data->window = %p", page_data->window);
	g_debug("- page_data->notebook = %p", page_data->notebook);
	g_debug("- page_data->page_no = %d", page_data->page_no);
	g_debug("- page_data->label = %p", page_data->label);
	g_debug("- page_data->label_text = %p", page_data->label_text);
	g_debug("- page_data->label_button = %p", page_data->label_button);
	g_debug("- page_data->hbox = %p", page_data->hbox);
	g_debug("- page_data->vte = %p", page_data->vte);
	g_debug("- page_data->scroll_bar = %p", page_data->scroll_bar);
	g_debug("- page_data->encoding_str = %s", page_data->encoding_str);
	g_debug("- page_data->locale = %s", page_data->locale);
//	g_debug("- page_data->environ = %s", page_data->environ);
//	g_debug("- page_data->VTE_CJK_WIDTH = %d", page_data->VTE_CJK_WIDTH);
	g_debug("- page_data->VTE_CJK_WIDTH_STR = %s", page_data->VTE_CJK_WIDTH_STR);
	g_debug("- page_data->page_name = %s", page_data->page_name);
	gchar *cmdline = get_cmdline(page_data->pid);
	g_debug("- page_data->pid = %d, cmdline = %s", page_data->pid, cmdline);
	g_free(cmdline);
#ifdef SAFEMODE
	if (page_data->vte)
#endif
		g_debug("- page_data row x col = (%ld x %ld)", vte_terminal_get_row_count(VTE_TERMINAL(page_data->vte)),
							       vte_terminal_get_column_count(VTE_TERMINAL(page_data->vte)));
	cmdline = get_cmdline(page_data->current_tpgid);
	g_debug("- page_data->current_tpgid = %d, cmdline = %s", page_data->current_tpgid, cmdline);
	g_free(cmdline);
	cmdline = get_cmdline(page_data->displayed_tpgid);
	g_debug("- page_data->displayed_tpgid = %d, cmdline = %s", page_data->displayed_tpgid, cmdline);
	g_free(cmdline);
	g_debug("- page_data->pid_cmdline = %s", page_data->pid_cmdline);
	g_debug("- page_data->custom_page_name = %s", page_data->custom_page_name);
	g_debug("- page_data->tab_color = %s (%p)", page_data->tab_color, page_data->tab_color);
	g_debug("- page_data->pwd = %s", page_data->pwd);
	g_debug("- page_data->is_root = %d", page_data->is_root);
	g_debug("- page_data->is_bold = %d", page_data->is_bold);
	g_debug("- page_data->should_be_bold = %d", page_data->should_be_bold);
	g_debug("- page_data->window_title_updated = %d", page_data->window_title_updated);
	g_debug("- page_data->page_update_method = %d", page_data->page_update_method);
	g_debug("- page_data->window_title_signal = %ld", page_data->window_title_signal);
	g_debug("- page_data->timeout_id = %d", page_data->timeout_id);
	g_debug("- page_data->urgent_bell_handler_id = %ld", page_data->urgent_bell_handler_id);
	g_debug("- page_data->font_name = %s", page_data->font_name);
	g_debug("- page_data->font_size = %d", page_data->font_size);
	g_debug("- page_data->check_root_privileges = %d", page_data->check_root_privileges);
	g_debug("- page_data->page_shows_window_title = %d", page_data->page_shows_window_title);
	g_debug("- page_data->page_shows_current_dir = %d", page_data->page_shows_current_dir);
	g_debug("- page_data->page_shows_current_cmdline = %d", page_data->page_shows_current_cmdline);
	g_debug("- page_data->bold_action_page_name = %d", page_data->bold_action_page_name);
//	g_debug("- page_data->use_scrollback_lines = %d", page_data->use_scrollback_lines);
#ifdef SAFEMODE
	if ((page_data) && (page_data->window_title_tpgid))
	{
#endif
		cmdline = get_cmdline(*(page_data->window_title_tpgid));
		g_debug("- page_data->*window_title_tpgid = %d (%s)", *(page_data->window_title_tpgid), cmdline);
		g_free(cmdline);
#ifdef SAFEMODE
	}
	if (page_data->lost_focus)
#endif
		g_debug("- page_data->*lost_focus = %d", *(page_data->lost_focus));
#ifdef USE_GTK2_GEOMETRY_METHOD
#  ifdef SAFEMODE
	if (page_data->keep_vte_size)
#  endif
		g_debug("- page_data->*keep_vte_size = %d", *(page_data->keep_vte_size));
#endif
#ifdef SAFEMODE
	if (page_data->current_vte)
#endif
		g_debug("- page_data->*current_vte = %p", *(page_data->current_vte));
	g_debug("- page_data->window_title_pwd = %s", page_data->window_title_pwd);
	g_debug("- page_data->custom_window_title = %d", page_data->custom_window_title);
	// g_debug("- page_data->*update_window_title_only = %d", *(page_data->update_window_title_only));
	// g_debug("- page_data->force_using_cmdline = %d", page_data->force_using_cmdline);
	for (i=0; i<COMMAND; i++)
		g_debug("- page_data->tag[%d] = %d", i, page_data->tag[i]);
	g_debug("- page_data->match_regex_setted = %d", page_data->match_regex_setted);
}

void print_array(gchar *name, gchar **data)
{
	if (data)
	{
		gint i = 0;
		while (data[i])
		{
			g_debug("- %s %d = %s", name, i, data[i]);
			i++;
		}
	}
	else
		g_debug("- %s = (NULL)", name);
}

void print_color(gint no, gchar *name, GdkRGBA color)
{
#  ifdef USE_GDK_RGBA
	if (no<0)
		g_debug("- %s = %0.4f, %0.4f, %0.4f, %0.4f",
			name,
			color.alpha,
			color.red,
			color.green,
			color.blue);
	else
		g_debug("- %s (%2d) = %0.4f, %0.4f, %0.4f, %0.4f",
			name,
			no,
			color.alpha,
			color.red,
			color.green,
			color.blue);
#  else
	if (no<0)
		g_debug("- %s = %04X, %04X, %04X, %04X",
			name,
			color.pixel,
			color.red,
			color.green,
			color.blue);
	else
		g_debug("- %s (%2d) = %04X, %04X, %04X, %04X",
			name,
			no,
			color.pixel,
			color.red,
			color.green,
			color.blue);
#  endif
}
#endif

void win_data_dup(struct Window *win_data_orig, struct Window *win_data)
{
#ifdef DETAIL
	g_debug("! Launch win_data_dup() with win_data_orig = %p, win_data = %p", win_data_orig, win_data);
#endif
#ifdef SAFEMODE
	if ((win_data_orig==NULL) || (win_data==NULL)) return;
#endif
	gint i;

	// The win_data->window is created already before win_data_dup() is launched.
	GtkWidget *window_orig = win_data->window;

	// The following datas may come from new_window() and already do g_strdup();
	// We should save it before do memcpy()

	gchar *init_dir = win_data->init_dir;
	gchar *home = win_data->home;
	gchar *shell = win_data->shell;
	gchar *wmclass_name = win_data->wmclass_name;
	gchar *wmclass_class = win_data->wmclass_class;
	gchar *runtime_locale_list = win_data->runtime_locale_list;
	gchar *runtime_encoding = win_data->runtime_encoding;
	gchar *runtime_LC_MESSAGES = win_data->runtime_LC_MESSAGES;
	gchar *environment = win_data->environment;

	memcpy( win_data, win_data_orig, sizeof(* win_data_orig));
	// g_debug("Copyed size = %d", sizeof(* win_data_orig));


// ---- environ and command line option ---- //

	if (init_dir)
		win_data->init_dir = init_dir;
	else
		win_data->init_dir = g_strdup(win_data->init_dir);

	if (home)
		win_data->home = home;
	else
		win_data->home = g_strdup(win_data_orig->home);

	if (shell)
		win_data->shell = shell;
	else
		win_data->shell = g_strdup(win_data_orig->shell);

	if (wmclass_name)
		win_data->wmclass_name = wmclass_name;
	else
		win_data->wmclass_name = g_strdup(win_data_orig->wmclass_name);

	if (wmclass_class)
		win_data->wmclass_class = wmclass_class;
	else
		win_data->wmclass_class = g_strdup(win_data_orig->wmclass_class);

	if (runtime_locale_list)
		win_data->runtime_locale_list = runtime_locale_list;
	else
		win_data->runtime_locale_list= g_strdup(win_data_orig->runtime_locale_list);

	if (runtime_encoding)
		win_data->runtime_encoding = runtime_encoding;
	else
		win_data->runtime_encoding = g_strdup(win_data_orig->runtime_encoding);

	if (runtime_LC_MESSAGES)
		win_data->runtime_LC_MESSAGES = runtime_LC_MESSAGES;
	else
		win_data->runtime_LC_MESSAGES = g_strdup(win_data_orig->runtime_LC_MESSAGES);

	if (environment)
		win_data->environment = environment;
	else
		win_data->environment = g_strdup(win_data_orig->environment);

	win_data->geometry = g_strdup(win_data_orig->geometry);
#ifdef SAFEMODE
	if (win_data_orig->warned_locale_list)
#endif
		win_data->warned_locale_list = g_string_new(win_data_orig->warned_locale_list->str);
	for (i=0; i<3; i++) win_data->encoding_locale_menuitems[i] = NULL;
	win_data->default_encoding = g_strdup(win_data_orig->default_encoding);
	win_data->encoding_sub_menu = NULL;
	// win_data->default_menuitem_encoding;

	win_data->default_locale = g_strdup(win_data_orig->default_locale);
	win_data->locales_list = g_strdup(win_data_orig->locales_list);
	// g_debug("win_data->default_locale = %s", win_data->default_locale);
	// g_debug("win_data->locales_list = %s", win_data->locales_list);
	// g_debug("get_default_LC_TYPE() = %s", get_default_LC_TYPE());
	// g_debug("!!! Set win_data->locale_sub_menu to NULL!!!");
	win_data->locale_sub_menu = NULL;

	win_data->default_shell = g_strdup(win_data_orig->default_shell);
#ifdef ENABLE_SET_EMULATION
	win_data->emulate_term = g_strdup(win_data_orig->emulate_term);
#endif
	// win_data->VTE_CJK_WIDTH;
	win_data->VTE_CJK_WIDTH_STR = g_strdup(win_data_orig->VTE_CJK_WIDTH_STR);
	// g_debug("win_data->shell = %s, win_data_orig->shell = %s", win_data->shell, win_data_orig->shell);
	// win_data->argc;
	// win_data->argv;
	// win_data->command;
	// win_data->hold = FALSE;
	win_data->init_tab_number = 1;
	// win_data->login_shell;
	// win_data->utmp;
	win_data->subitem_new_window_from_list = NULL;
	win_data->menuitem_new_window_from_list = NULL;
	win_data->subitem_load_profile_from_list = NULL;
	win_data->menuitem_load_profile_from_list = NULL;

	// win_data->use_custom_profile;
	win_data->profile = g_strdup(win_data_orig->profile);
	win_data->specified_profile = NULL;
	win_data->profile_dir_modtime = -1;
	win_data->menuitem_auto_save = NULL;
	// win_data->auto_save;

	// g_debug("dup environ and command line option finish!");

// ---- the component of a single window ---- //

	// win_data->startup_fullscreen;
	// win_data->fullscreen;
	// win_data->true_fullscreen;
	// win_data->fullscreen_show_tabs_bar;
	// win_data->fullscreen_show_scroll_bar;
	// win_data->window_status;

	win_data->window = window_orig;
	win_data->notebook = NULL;
	// win_data->show_close_button_on_tab;
	// win_data->show_close_button_on_all_tab;
	win_data->current_vte = NULL;
	// win_data->window_title_shows_current_page;
	if (win_data->custom_window_title_str)
	{
		win_data->custom_window_title_str = g_strdup(win_data_orig->custom_window_title_str);
		update_window_title(win_data->window, win_data->custom_window_title_str,
				    win_data->window_title_append_package_name);
	}

	win_data->custom_tab_names_str = NULL;
	win_data->custom_tab_names_strs = NULL;

	win_data->window_title_tpgid=0;
	if (win_data_orig->use_rgba == -1) win_data->use_rgba = 1;
	// win_data->use_rgba_orig;
#ifdef ENABLE_RGBA
	// win_data->transparent_window;
	// win_data->window_opacity;
#endif
	// win_data->enable_key_binding;
	for (i=0; i<KEYS; i++)
		win_data->user_keys[i].value = g_strdup(win_data_orig->user_keys[i].value);
	// win_data->hints_type = win_data_orig->hints_type;
	win_data->hints_type = HINTS_FONT_BASE;
#  ifdef GEOMETRY
	fprintf(stderr, "\033[1;%dm!! win_data_dup(win_data %p): set win_data->hints_type = %d !!\033[0m\n",
		ANSI_COLOR_BLUE, win_data, win_data->hints_type);
#  endif
	// win_data->resize_type;
	// win_data->geometry_width;
	// win_data->geometry_height;
	// win_data->lost_focus;
	// win_data->keep_vte_size;

// ---- the component of a single menu ---- //

	win_data->menu = NULL;
	// win_data->menu_activated;
#ifdef ENABLE_RGBA
	win_data->menuitem_trans_win = NULL;
#endif
	win_data->menuitem_trans_bg = NULL;
	// win_data->show_color_selection_menu;
	// win_data->show_resize_menu;
	// win_data->font_resize_ratio;
	// win_data->window_resize_ratio;
	// win_data->show_background_menu;
#ifdef ENABLE_IM_APPEND_MENUITEMS
	// win_data->show_input_method_menu;
#endif
	// win_data->show_change_page_name_menu;
	// win_data->show_exit_menu;
	// win_data->enable_hyperlink;
	for (i=0; i<REGEX; i++)
		win_data->user_regex[i] = g_strdup(win_data_orig->user_regex[i]);
	for (i=0; i<COMMAND; i++)
	{
		win_data->user_command[i].command = g_strdup(win_data_orig->user_command[i].command);
		win_data->user_command[i].environ = g_strdup(win_data_orig->user_command[i].environ);
		win_data->user_command[i].environments = split_string(win_data->user_command[i].environ, " ", -1);
		win_data->user_command[i].locale = g_strdup(win_data_orig->user_command[i].locale);
		win_data->user_command[i].match_regex = g_strdup(win_data_orig->user_command[i].match_regex);
		win_data->user_command[i].match_regex_orig = g_strdup(win_data_orig->user_command[i].match_regex_orig);
	}
	win_data->menuitem_copy_url = NULL;
	win_data->menuitem_dim_text = NULL;
#ifdef ENABLE_RGBA
	win_data->menuitem_dim_window = NULL;
#endif
	win_data->menuitem_cursor_blinks = NULL;
	win_data->menuitem_allow_bold_text = NULL;
	win_data->menuitem_open_url_with_ctrl_pressed = NULL;
	win_data->menuitem_disable_url_when_ctrl_pressed = NULL;
	win_data->menuitem_audible_bell = NULL;
#ifdef ENABLE_VISIBLE_BELL
	win_data->menuitem_visible_bell = NULL;
#endif
#ifdef ENABLE_BEEP_SINGAL
	win_data->menuitem_urgent_bell = NULL;
#endif
	win_data->menuitem_show_tabs_bar = NULL;
	win_data->menuitem_hide_tabs_bar = NULL;
	win_data->menuitem_hide_scroll_bar = NULL;
	// win_data->show_copy_paste_menu;
	win_data->menuitem_copy = NULL;
	win_data->menuitem_paste = NULL;
	win_data->menuitem_clipboard = NULL;
	win_data->menuitem_primary = NULL;

// ---- the color used in vte ---- //

	// win_data->fg_color;
	// win_data->fg_color_inactive;
	// win_data->custom_cursor_color;
	// win_data->cursor_color;
	// win_data->bg_color;
	// win_data->menuitem_invert_color
	win_data->menuitem_invert_color = NULL;

	// win_data->color_theme_str_orig will always be NULL
	// win_data->color_theme_str_orig = g_strdup(win_data_orig->color_theme_str)orig);
	for (i=0; i<COLOR; i++)
	{
		win_data->color[i] = win_data_orig->color[i];
		win_data->color_inactive[i] = win_data_orig->color_inactive[i];
	}
	for (i=0; i<THEME*2; i++)
		win_data->menuitem_theme[i] = NULL;
	win_data->current_menuitem_theme = NULL;
	// win_data->have_custom_color;
	// win_data->use_custom_theme;
	for (i=0; i<ANSI_THEME_MENUITEM; i++) win_data->ansi_theme_menuitem[i] = NULL;
	win_data->ansi_color_sub_menu = NULL;
	win_data->ansi_color_menuitem = NULL;
	// win_data->color_brightness;
	// win_data->color_brightness_inactive;

// ---- tabs on notebook ---- //

	// win_data->page_width;
	// win_data->fill_tabs_bar;
	// win_data->tabs_bar_position;

	win_data->page_name = g_strdup(win_data_orig->page_name);
	// win_data->reuse_page_names;
	win_data->page_names = g_strdup(win_data_orig->page_names);
	win_data->splited_page_names = split_string(win_data->page_names, " ", -1);
	// print_array("COPY: win_data->splited_page_names", win_data->splited_page_names);
	// win_data->page_names_no;
	// win_data->max_page_names_no;

	// win_data->page_shows_number;
	// win_data->page_shows_encoding;
	// win_data->page_shows_current_cmdline;
	// win_data->page_shows_window_title;
	// win_data->page_shows_current_dir;
	// win_data->use_color_page;
	// win_data->check_root_privileges;
	// win_data->bold_current_page_name;
	// win_data->bold_action_page_name;

	for (i=0; i<PAGE_COLOR; i++)
		win_data->user_page_color[i] = g_strdup(win_data_orig->user_page_color[i]);

// ---- font ---- //
	// win_data->font_anti_alias;
	win_data->default_font_name = g_strdup(win_data_orig->default_font_name);
	win_data->restore_font_name = g_strdup(win_data_orig->restore_font_name);

// ---- other settings for init a vte ---- //

	// win_data->default_column;
	// win_data->default_row;
#ifdef ENABLE_SET_WORD_CHARS
	win_data->word_chars = g_strdup(win_data_orig->word_chars);
#endif
	// win_data->show_scroll_bar;
	// win_data->scroll_bar_position;
#if defined(ENABLE_VTE_BACKGROUND) || defined(FORCE_ENABLE_VTE_BACKGROUND)
	// win_data->transparent_background;
	// win_data->background_saturation;
	// win_data->scroll_background;
	win_data->background_image = g_strdup(win_data_orig->background_image);
#endif
	// win_data->scrollback_lines;
	// win_data->dim_text;
	// win_data->dim_window;
	// win_data->cursor_blinks;
	// win_data->allow_bold_text;
	// win_data->open_url_with_ctrl_pressed;
	// win_data->disable_url_when_ctrl_pressed;
	// win_data->audible_bell;
#ifdef ENABLE_VISIBLE_BELL
	// win_data->visible_bell;
#endif
	// win_data->urgent_bell;
	// win_data->urgent_bell_status;
#ifdef ENABLE_BEEP_SINGAL
	win_data->urgent_bell_focus_in_event_id = 0;
#endif
	// win_data->erase_binding;
	for (i=0; i<ERASE_BINDING; i++)
		win_data->menuitem_erase_binding[i] = NULL;
	win_data->current_menuitem_erase_binding = NULL;
#ifdef ENABLE_CURSOR_SHAPE
	// win_data->cursor_shape;
	for (i=0; i<CURSOR_SHAPE; i++)
		win_data->menuitem_cursor_shape[i] = NULL;
	win_data->current_menuitem_cursor_shape = NULL;
#endif
// ---- other ---- //

	// win_data->confirm_to_close_multi_tabs;
	// win_data->prime_user_datas_inited;
	// win_data->prime_user_settings_inited;
	// win_data->confirm_to_execute_command;
	win_data->execute_command_whitelist = g_strdup(win_data_orig->execute_command_whitelist);
	// g_debug("win_data->execute_command_whitelist for win_data (%p) duped!", win_data);
	win_data->execute_command_whitelists = split_string(win_data->execute_command_whitelist, " ", -1);

	// win_data->execute_command_in_new_tab;
	// win_data->join_as_new_tab;
	win_data->foreground_program_whitelist = g_strdup(win_data_orig->foreground_program_whitelist);
	win_data->foreground_program_whitelists = split_string(win_data->foreground_program_whitelist, " ", -1);
	win_data->background_program_whitelist = g_strdup(win_data_orig->background_program_whitelist);
	win_data->background_program_whitelists = split_string(win_data->background_program_whitelist, " ", -1);
	win_data->paste_texts_whitelist = g_strdup(win_data_orig->paste_texts_whitelist);
	win_data->paste_texts_whitelists = split_string(win_data->paste_texts_whitelist, " ", -1);

	win_data->find_string = g_strdup(win_data_orig->find_string);
	// win_data->find_case_sensitive;
	// win_data->find_use_perl_regular_expressions;
	// win_data->find_entry_bg_color;
	// win_data->find_entry_current_bg_color;

	// win_data->checking_menu_item;
	// win_data->dialog_activated;
	// win_data->kill_color_demo_vte;
	// win_data->adding_page;
	// win_data->confirmed_profile_is_invalid;
	win_data->temp_data = NULL;
}


void update_window_hint(struct Window *win_data,
			struct Page *page_data)
{
#ifdef DETAIL
	g_debug("! Launch update_window_hint() with win_data = %p, page_data = %p",
		win_data, page_data);
#endif
#ifdef SAFEMODE
	if ((win_data==NULL) || (page_data==NULL) || (page_data->notebook==NULL)) return;
#endif
	if (gtk_notebook_get_n_pages(GTK_NOTEBOOK(page_data->notebook)) == 1 &&
	    page_data->font_size == 0)
		win_data->hints_type = HINTS_FONT_BASE;
	else if ((check_if_every_vte_is_using_restore_font_name(win_data)) ||
#ifdef USE_GTK2_GEOMETRY_METHOD
		  win_data->fullscreen)
#else
		  win_data->window_status)
#endif
		win_data->hints_type = HINTS_FONT_BASE;
	else
		win_data->hints_type = HINTS_NONE;
#  ifdef GEOMETRY
	fprintf(stderr, "\033[1;%dm!! update_window_hint(win_data %p): set win_data->hints_type = %d (page_data->font_size = %d)!!\033[0m\n",
		ANSI_COLOR_BLUE, win_data, win_data->hints_type, page_data->font_size);
#  endif

	// g_debug("window_resizable in change current font!");
#ifdef GEOMETRY
	fprintf(stderr, "\033[1;37m!! update_window_hint(win_data %p): call window_resizable() for vte = %p with hints_type = %d\033[0m\n",
		win_data, page_data->vte, win_data->hints_type);
#endif
	window_resizable(page_data->window, page_data->vte, win_data->hints_type);
}

gboolean hide_and_show_tabs_bar(struct Window *win_data , Switch_Type show_tabs_bar)
{
#ifdef DETAIL
	g_debug("! Launch hide_and_show_tabs_bar() with win_data = %p, show_tabs_bar = %d",
		win_data, show_tabs_bar);
#endif
#ifdef SAFEMODE
	if ((win_data==NULL) || (win_data->notebook==NULL)) return FALSE;
#endif
	gboolean show = get_hide_or_show_tabs_bar(win_data, show_tabs_bar);
	// g_debug("hide_and_show_tabs_bar: show = %d", show);
	if (show == gtk_notebook_get_show_tabs(GTK_NOTEBOOK(win_data->notebook))) return FALSE;

	// win_data->keep_vte_size |= 6;
	// window_resizable(win_data->window, win_data->current_vte, 2, 1);
#ifdef USE_GTK2_GEOMETRY_METHOD
	if (! (win_data->fullscreen || win_data->window_status))
#else
	if (win_data->window_status == WINDOW_NORMAL)
#endif
	{
#ifdef USE_GTK2_GEOMETRY_METHOD
#  ifdef GEOMETRY
		g_debug("@ hide_and_show_tabs_bar(win_data %p): Call keep_gtk2_window_size() with keep_vte_size = %x",
			win_data, win_data->keep_vte_size);
#  endif
		// g_debug("hide_and_show_tabs_bar(): launch keep_gtk2_window_size()!");
		keep_gtk2_window_size (win_data, win_data->current_vte, 0x1c);
#endif
#ifdef USE_GTK3_GEOMETRY_METHOD
#  ifdef GEOMETRY
		fprintf(stderr, "\033[1;%dm!! hide_and_show_tabs_bar(win_data %p)(GEOMETRY_AUTOMATIC): "
				"Calling keep_gtk3_window_size() with hints_type = %d\n",
			ANSI_COLOR_MAGENTA, win_data, win_data->hints_type);
#  endif
		win_data->resize_type = GEOMETRY_AUTOMATIC;
		keep_gtk3_window_size(win_data, FALSE);
#endif
	}
#ifdef USE_GTK3_GEOMETRY_METHOD
	if (win_data->window_status == WINDOW_RESIZING_TO_NORMAL)
	{
#  ifdef GEOMETRY
		fprintf(stderr, "\033[1;%dm!! hide_and_show_tabs_bar(win_data %p)(GEOMETRY_CUSTOM): "
				"Calling keep_gtk3_window_size() with hints_type = %d\n",
			ANSI_COLOR_MAGENTA, win_data, win_data->hints_type);
#  endif
		win_data->resize_type = GEOMETRY_CUSTOM;
		keep_gtk3_window_size(win_data, FALSE);
	}
#endif

	gtk_notebook_set_show_tabs(GTK_NOTEBOOK(win_data->notebook), show);
	if (show)
		set_widget_thickness(win_data->notebook, 1);
	else
		set_widget_thickness(win_data->notebook, 0);
#ifdef GEOMETRY
		fprintf(stderr, "\033[1;34m!! hide_and_show_tabs_bar(win_data %p): call gtk_notebook_set_show_border() with show = %d\033[0m\n",
			win_data, show);
#endif
	gtk_notebook_set_show_border(GTK_NOTEBOOK(win_data->notebook), show);

	if (show)
		set_widget_can_not_get_focus(win_data->notebook);

	return TRUE;
}

gboolean get_hide_or_show_tabs_bar(struct Window *win_data, Switch_Type show_tabs_bar)
{
#ifdef DETAIL
	g_debug("! Launch get_hide_or_show_tabs_bar() with win_data = %p, show_tabs_bar = %d",
		win_data, show_tabs_bar);
#endif
#ifdef SAFEMODE
	if ((win_data==NULL) || (win_data->notebook==NULL)) return FALSE;
#endif

	gboolean show = gtk_notebook_get_show_tabs(GTK_NOTEBOOK(win_data->notebook));
	// g_debug("get_hide_or_show_tabs_bar: show = %d, win_data->window_status = %d", show, win_data->window_status);
	switch (show_tabs_bar)
	{
		case AUTOMATIC:
			if ((gtk_notebook_get_n_pages(GTK_NOTEBOOK(win_data->notebook)) > 1) &&
#ifdef USE_GTK2_GEOMETRY_METHOD
			     (win_data->true_fullscreen==FALSE))
#else
			     ((win_data->window_status==WINDOW_NORMAL) ||
			      (win_data->window_status==WINDOW_RESIZING_TO_NORMAL) ||
			      (win_data->window_status==WINDOW_APPLY_PROFILE_NORMAL) ||
			      (win_data->window_status==WINDOW_MAX_WINDOW)))
#endif
				show = TRUE;
			else
				show = FALSE;
			break;
		case TEMPORARY_ON:
			win_data->show_tabs_bar = AUTOMATIC;
		case ON:
		case FORCE_ON:
			show = TRUE;
			break;
		case TEMPORARY_OFF:
			win_data->show_tabs_bar = AUTOMATIC;
		case OFF:
		case FORCE_OFF:
			show = FALSE;
			break;
		default:
#  ifdef FATAL
			print_switch_out_of_range_error_dialog("hide_and_show_tabs_bar",
							       "show_tabs_bar",
							       show_tabs_bar);
#  endif
			break;
	}

	return show;
}

void set_widget_can_not_get_focus(GtkWidget *widget)
{
#ifdef DETAIL
	g_debug("! Launch set_widget_can_not_get_focus() with widget = %p", widget);
#endif
#ifdef SAFEMODE
	if (widget==NULL) return;
#endif
	gtk_widget_set_can_focus(GTK_WIDGET (widget), FALSE);
}

gboolean hide_scrollback_lines(GtkWidget *menu_item, struct Window *win_data)
{
#ifdef DETAIL
	g_debug("! Launch hide_scrollback_lines()");
#endif
#ifdef SAFEMODE
	if (win_data==NULL) return FALSE;
#endif
	gboolean show = TRUE;
#ifdef SAFEMODE
	if (win_data->menuitem_hide_scroll_bar)
#endif
		show = gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM(win_data->menuitem_hide_scroll_bar));

	win_data->show_scroll_bar = AUTOMATIC;
#ifdef USE_GTK2_GEOMETRY_METHOD
	if (show && win_data->true_fullscreen)
		win_data->show_scroll_bar = FORCE_ON;
	else if ((show == FALSE) && (win_data->true_fullscreen == FALSE))
		win_data->show_scroll_bar = FORCE_OFF;
#else
	switch (win_data->window_status)
	{
		case WINDOW_NORMAL:
		case WINDOW_RESIZING_TO_NORMAL:
		case WINDOW_MAX_WINDOW:
		case WINDOW_APPLY_PROFILE_NORMAL:
			if (show==FALSE) win_data->show_scroll_bar = FORCE_OFF;
			break;
		case WINDOW_MAX_WINDOW_TO_FULL_SCREEN:
		case WINDOW_FULL_SCREEN:
		case WINDOW_START_WITH_FULL_SCREEN:
		case WINDOW_APPLY_PROFILE_FULL_SCREEN:
			if (show) win_data->show_scroll_bar = FORCE_ON;
			break;
		default:
#ifdef FATAL
			print_switch_out_of_range_error_dialog("hide_scrollback_lines", "win_data->window_status", win_data->window_status);
#endif
			break;

	}
#endif
	// g_debug("hide_scrollback_lines(): show = %d, win_data->show_scroll_bar = %d",
	//	show, win_data->show_scroll_bar);

	gint i;
	struct Page *page_data = NULL;

	gint total_page = -1;
#ifdef SAFEMODE
	if (win_data->notebook)
#endif
		total_page = gtk_notebook_get_n_pages(GTK_NOTEBOOK(win_data->notebook));

	for (i=0; i<total_page; i++)
	{
		page_data = get_page_data_from_nth_page(win_data, i);
#ifdef SAFEMODE
		if (page_data==NULL) continue;
#endif
		show_and_hide_scroll_bar(page_data, show);
	}

	// Trying to keep vte size
#ifdef USE_GTK2_GEOMETRY_METHOD
	if (! (win_data->fullscreen || win_data->window_status))
	{
#  ifdef GEOMETRY
		g_debug("@ hide_scrollback_lines(for %p): Call keep_gtk2_window_size() with keep_vte_size = %x",
			win_data->window, win_data->keep_vte_size);
#  endif
		// g_debug("hide_scrollback_lines(): launch keep_gtk2_window_size()!");
		keep_gtk2_window_size (win_data, win_data->current_vte, 0xc00);
	}
#endif
#ifdef USE_GTK3_GEOMETRY_METHOD
	if (win_data->window_status == WINDOW_NORMAL)
	{
#  ifdef GEOMETRY
		fprintf(stderr, "\033[1;%dm!! hide_scrollback_lines(win_data %p)(GEOMETRY_AUTOMATIC): "
				"Calling keep_gtk3_window_size() with hints_type = %d\033[0m\n",
			ANSI_COLOR_MAGENTA, win_data, win_data->hints_type);
#  endif
		win_data->resize_type = GEOMETRY_AUTOMATIC;
		keep_gtk3_window_size(win_data, FALSE);
	}
#endif
#ifdef USE_GTK3_GEOMETRY_METHOD
	if (win_data->window_status == WINDOW_RESIZING_TO_NORMAL)
	{
#  ifdef GEOMETRY
		fprintf(stderr, "\033[1;%dm!! hide_scrollback_lines(win_data %p)(GEOMETRY_CUSTOM): "
				"Calling keep_gtk3_window_size() with hints_type = %d\033[0m\n",
			ANSI_COLOR_MAGENTA, win_data, win_data->hints_type);
#  endif
		win_data->resize_type = GEOMETRY_CUSTOM;
		keep_gtk3_window_size(win_data, FALSE);
	}
#endif
	return TRUE;
}

gboolean fullscreen_show_hide_scroll_bar(struct Window *win_data)
{
#ifdef DETAIL
	g_debug("! Launch fullscreen_show_hide_scroll_bar() with win_data = %p", win_data);
#endif
#ifdef SAFEMODE
	if ((win_data==NULL) || (win_data->menuitem_hide_scroll_bar==NULL)) return FALSE;
#endif
	gboolean show = check_show_or_hide_scroll_bar(win_data);
	gboolean current_show = gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM(win_data->menuitem_hide_scroll_bar));
	// g_debug("fullscreen_show_hide_scroll_bar(): show = %d, current_show = %d", show, current_show);
	if (show == current_show) return FALSE;

	gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(win_data->menuitem_hide_scroll_bar),
					show);
	// g_debug("fullscreen_show_hide_scroll_bar(): check_show_or_hide_scroll_bar() = %d", show);
	hide_scrollback_lines(win_data->menuitem_hide_scroll_bar, win_data);
	return TRUE;
}

gboolean confirm_to_paste_form_clipboard(Clipboard_Type type, struct Window *win_data, struct Page *page_data)
{
#ifdef DETAIL
	g_debug("! Launch confirm_to_paste_form_clipboard() with type = %d, win_data = %p, page_data = %p",
		type, win_data, page_data);
#endif
#ifdef SAFEMODE
	if (page_data==NULL) return FALSE;
#endif
	gchar **stats = get_pid_stat(get_tpgid(page_data->pid), 4);
	gboolean pasted = FALSE;
	if ((stats) &&
	    (check_string_in_array(stats[2], win_data->paste_texts_whitelists) == FALSE))
		pasted = show_clipboard_dialog(type, win_data, page_data, CONFIRM_TO_PASTE_TEXTS_TO_VTE_TERMINAL);

	g_strfreev(stats);
	return pasted;
}

gboolean show_clipboard_dialog(Clipboard_Type type, struct Window *win_data,
			       struct Page *page_data, Dialog_Type_Flags dialog_type)
{
#ifdef DETAIL
	g_debug("! Launch show_clipboard_dialog() with type = %d, win_data = %p, page_data = %p, dialog_type = %d",
		type, win_data, page_data, dialog_type);
#endif
#ifdef SAFEMODE
	if (win_data==NULL) return FALSE;
#endif
	gboolean pasted = FALSE;
	GtkClipboard *clipboard = NULL;
	switch (type)
	{
		case SELECTION_CLIPBOARD:
		{
			extern GtkClipboard *selection_clipboard;
			clipboard = selection_clipboard;
			break;
		}
		case SELECTION_PRIMARY:
		{
			extern GtkClipboard *selection_primary;
			clipboard = selection_primary;
			break;
		}
		default:
#ifdef FATAL
			print_switch_out_of_range_error_dialog("show_clipboard_dialog", "type", type);
#endif
			break;
	}
#ifdef SAFEMODE
	if (clipboard==NULL) return FALSE;
#endif
	gchar *clipboard_str = g_strdup(gtk_clipboard_wait_for_text(clipboard));
	gchar *old_temp_data = win_data->temp_data;
	switch (dialog_type)
	{
		case CONFIRM_TO_PASTE_TEXTS_TO_VTE_TERMINAL:
			win_data->temp_data = colorful_max_new_lines(clipboard_str, 0, 7);
			break;
		case GENERAL_INFO:
		{
			gchar *tmp_str = colorful_max_new_lines(clipboard_str, -1, 7);
			if (tmp_str)
				win_data->temp_data = g_strdup_printf("%s\x10%s\x10%s", _("Clipboard"), "Clipboard", tmp_str);
			else
				win_data->temp_data = NULL;
			g_free(tmp_str);
			break;
		}
		default:
#ifdef FATAL
			print_switch_out_of_range_error_dialog("show_clipboard_dialog", "dialog_type", dialog_type);
#endif
			break;
	}
	// g_debug("Trying to paste '%s' (%s) to vte...", win_data->temp_data, clipboard_str);
	// g_debug("show_clipboard_dialog(): win_data->temp_data = \"%s\"", win_data->temp_data);
	if (win_data->temp_data)
	{
		// g_debug("'%s' have new line in it!", win_data->temp_data);
		GtkResponseType response = dialog(NULL, dialog_type);
		if ((response==GTK_RESPONSE_OK) || (response==GTK_RESPONSE_YES) || (response==GTK_RESPONSE_ACCEPT))
		{
			if (dialog_type == CONFIRM_TO_PASTE_TEXTS_TO_VTE_TERMINAL)
			{
#ifdef SAFEMODE
				if (page_data!=NULL)
				{
#endif
					if (response==GTK_RESPONSE_ACCEPT)
					{
						gchar **old_clipboard_strs = split_string(clipboard_str, "\n\r", -1);
						gchar *new_clipboard_str = convert_array_to_string(old_clipboard_strs, '\0');
						// g_debug("Set clipboard to %s", new_clipboard_str);
						if (new_clipboard_str) gtk_clipboard_set_text(clipboard, new_clipboard_str, -1);
						g_free(new_clipboard_str);
						g_strfreev(old_clipboard_strs);
					}
					else if (response==GTK_RESPONSE_YES)
					{
						// remove "\\\r" and "\\\n", and replease it with ' '
						gchar *temp_clipboard_str = g_strdup(clipboard_str);
						gint i=0, j=0;
						while (clipboard_str[i])
						{
							if ((clipboard_str[i]=='\\') && ((clipboard_str[i+1]=='\n') || (clipboard_str[i+1]=='\r')))
							{
								i+=2;
								if (clipboard_str[i]=='>')
								{
									if (clipboard_str[i+1]==' ')
									i+=2;
								}
							}
							if ((clipboard_str[i]=='\n') || (clipboard_str[i]=='\r'))
							{
								if (clipboard_str[i+1])
								{
									if (clipboard_str[i+1]=='>' && clipboard_str[i+2]==' ')
									{
										temp_clipboard_str[j]=' ';
										i+=2;
										j++;
									}
									else
									{
										temp_clipboard_str[j]=' ';
										i++;
										j++;
									}
								}
							}
							temp_clipboard_str[j]=clipboard_str[i];
							i++;
							j++;
						}
						temp_clipboard_str[j]='\0';
						// g_debug ("show_clipboard_dialog(): temp_clipboard_str = %s", temp_clipboard_str);

						// remove '\n' and '\r'
						gchar **old_clipboard_strs = split_string(temp_clipboard_str, "\n\r", -1);
						gchar *new_clipboard_str = convert_array_to_string(old_clipboard_strs, '\0');
						// g_debug ("show_clipboard_dialog(): new_clipboard_str = %s", new_clipboard_str);

						// join ' ' and '\t'
						gchar *join_clipboard_str = g_strdup(new_clipboard_str);
						i=0, j=0;
						gint need_printed_space;
						while (new_clipboard_str[i])
						{
							need_printed_space = 0;
							while ((new_clipboard_str[i]==' ') || (new_clipboard_str[i]=='\t'))
							{
								need_printed_space = 1;
								i++;
							}
							if (need_printed_space && (j!=0))
							{
								join_clipboard_str[j]=' ';
								j++;
							}
							if (new_clipboard_str[i])
							{
								join_clipboard_str[j] = new_clipboard_str[i];
								i++;
								j++;
							}
						}
						if (join_clipboard_str[j-1]==' ')
							join_clipboard_str[j-1]='\0';
						else
							join_clipboard_str[j]='\0';
						// g_debug ("show_clipboard_dialog(): join_clipboard_str = %s", join_clipboard_str);

						// g_debug("Set clipboard to %s", join_clipboard_str);
						if (join_clipboard_str) gtk_clipboard_set_text(clipboard, join_clipboard_str, -1);
						g_free(temp_clipboard_str);
						g_free(new_clipboard_str);
						g_free(join_clipboard_str);
						g_strfreev(old_clipboard_strs);
					}
					switch (type)
					{
						case SELECTION_CLIPBOARD:
							vte_terminal_paste_clipboard(VTE_TERMINAL(page_data->vte));
							break;
						case SELECTION_PRIMARY:
							vte_terminal_paste_primary(VTE_TERMINAL(page_data->vte));
							break;
						default:
#ifdef FATAL
							print_switch_out_of_range_error_dialog("show_clipboard_dialog", "type", type);
#endif
							break;
					}

					// recover the text in clipboard
					if ((response==GTK_RESPONSE_ACCEPT) || (response==GTK_RESPONSE_YES))
					{
						// g_debug("Set clipboard to %s", clipboard_str);
						if (clipboard_str) gtk_clipboard_set_text(clipboard, clipboard_str, -1);
					}
#ifdef SAFEMODE
				}
#endif
			}
		}
		pasted = TRUE;
	}
	g_free(clipboard_str);
	g_free(win_data->temp_data);
	win_data->temp_data = old_temp_data;

	return pasted;
}
