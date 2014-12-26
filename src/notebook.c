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

#include <locale.h>

#include "notebook.h"

// GtkWidget *current_vte=NULL;
// extern GtkWidget *active_window;
extern GtkWidget *menu_active_window;
extern gboolean proc_exist;
extern gchar *init_LC_CTYPE;
extern gchar *init_LANGUAGE;

extern gint dialog_activated;
gboolean menu_activated = FALSE;

extern struct Process_Data *process_data;

extern gboolean force_to_quit;
extern gboolean using_kill;
gboolean confirm_to_execute = TRUE;
#ifdef USE_OLD_VTE_FORK_COMMAND
gchar *login_shell_str[] = {"-", NULL};
#else
gchar *login_shell_str[] = {NULL, "-", NULL};
gchar *full_argv[] = {"/bin/sh", "/bin/sh", NULL};
#endif

// prev_vte: the new vte will be the clone of prev_vte.
// menuitem_encoding: using right menu to open a new vtw with specified locale.
//		      Lilyterm will update page_data->encoding_str with it.
// locale: the environ LANG, LANGUAGE, and LC_ALL used in vte_terminal_fork_command().
// user_environ: it will be used in vte_terminal_fork_command(),
//		 and will be saved in page_data->environ.
//		 should be separated with <tab>.
// run_once: some data that should be inited for the first vte of a LilyTerm window.
// encoding: the encoding come from environ *or default_encoding in profile*. only for new window.
// runtime_locale_encoding: the encoding come from environ.
struct Page *add_page(struct Window *win_data,
		      struct Page *page_data_prev,
		      GtkWidget *menuitem_encoding,
		      gchar *encoding,
		      gchar *runtime_locale_encoding,
		      gchar *locale,
		      gchar *environments,
		      gchar *user_environ,
		      const gchar *VTE_CJK_WIDTH_STR,
		      gboolean add_to_next)
{
#ifdef DETAIL
	if (menuitem_encoding)
		g_debug("! Launch add_page() with win_data = %p, page_data_prev = %p, "
			"menuitem_encoding->name = %s, encoding = %s,"
			"locale = %s, environ = %s, VTE_CJK_WIDTH_STR = %s",
			win_data, page_data_prev, gtk_widget_get_name(menuitem_encoding), encoding,
			locale, user_environ, VTE_CJK_WIDTH_STR);
	else
		g_debug("! Launch add_page() with win_data = %p, page_data_prev = %p, "
			"encoding = %s, locale = %s, environ = %s, VTE_CJK_WIDTH_STR = %s",
			win_data, page_data_prev, encoding,
			locale, user_environ, VTE_CJK_WIDTH_STR);
#endif

#ifdef SAFEMODE
	if (win_data==NULL) return NULL;
#endif

// ---- Clone the page_data ---- //

	struct Page *page_data = g_new0(struct Page, 1);
#ifdef SAFEMODE
	if (page_data==NULL) return NULL;
#endif
	// g_debug ("init page_date = %p!!!", page_data);
	if (page_data_prev)
		page_data_dup(page_data_prev, page_data);
	else
		init_page_parameters(win_data, page_data);

// ---- deal the parameters and set the environs ----//

	GString *environ_str;
	if (environments)
		environ_str = g_string_new(environments);
	else
		environ_str = g_string_new(win_data->environment);
	// g_debug("environ_str = %s", environ_str->str);

	// set the encoding
	if ((encoding && (encoding[0]!='\0')) || menuitem_encoding)
	{
		// g_debug("encoding = %s", encoding);
		// g_debug("menuitem_encoding = %p", menuitem_encoding);
		g_free(page_data->encoding_str);

		if (encoding && (encoding[0]!='\0'))
			page_data->encoding_str = g_strdup(encoding);
		else
			page_data->encoding_str = g_strdup(gtk_widget_get_name(menuitem_encoding));
		// g_debug("FINAL: page_data->encoding_str = %s", page_data->encoding_str);
	}
	// g_debug("FINAL: page_data->encoding_str = %s", page_data->encoding_str);

	// set the locale.
	// g_debug("locale = %s", locale);
	if (locale && (locale[0]!='\0'))
	{
		gchar *lang = get_lang_str_from_locale(locale, ".");
		gchar *language = get_language_str_from_locales(locale, win_data->default_locale);

		g_free(page_data->locale);
		page_data->locale = g_strdup(locale);
		g_string_append_printf (environ_str,
					"\tLANG=%s\tLANGUAGE=%s\tLC_ALL=%s",
					lang, language, locale);
		// g_debug("lang = %s, language = %s", lang, language);
		g_free(lang);
		g_free(language);
	}

	// set the environ that user specify in profile
	if (user_environ)
	{
		g_free(page_data->environ);
		page_data->environ = g_strdup(user_environ);
#ifdef SAFEMODE
		if (environ_str && environ_str->len)
#else
		if (environ_str->len)
#endif
			environ_str = g_string_append(environ_str, "\t");
		g_string_append_printf (environ_str, "%s", user_environ);
	}
#ifdef SAFEMODE
	if (environ_str && environ_str->len)
#else
	if (environ_str->len)
#endif
		environ_str = g_string_append(environ_str, "\t");

#ifdef ENABLE_SET_EMULATION
#  ifdef SAFEMODE
	if (win_data->emulate_term == NULL)
		g_string_append_printf (environ_str, "TERM=xterm");
	else
#  endif
		g_string_append_printf (environ_str, "TERM=%s", win_data->emulate_term);
#endif
	// set colorterm
	g_string_append_printf (environ_str, "\tCOLORTERM=lilyterm");

// ---- Setting the VTE_CJK_WIDTH environment ---- //

	// g_debug("The VTE_CJK_WIDTH_STR got in add_page is %s", VTE_CJK_WIDTH_STR);
	if (VTE_CJK_WIDTH_STR && (VTE_CJK_WIDTH_STR[0] != '\0'))
	{
		// g_debug("VTE_CJK_WIDTH_STR = %s in add_page", VTE_CJK_WIDTH_STR);
		g_setenv("VTE_CJK_WIDTH", VTE_CJK_WIDTH_STR, TRUE);
		g_free(page_data->VTE_CJK_WIDTH_STR);
		page_data->VTE_CJK_WIDTH_STR = g_strdup(VTE_CJK_WIDTH_STR);
	}
	else
	{
		// g_debug("page_data->VTE_CJK_WIDTH_STR = %s", page_data->VTE_CJK_WIDTH_STR);
		if (page_data->VTE_CJK_WIDTH_STR && (page_data->VTE_CJK_WIDTH_STR[0] != '\0'))
			g_setenv("VTE_CJK_WIDTH", page_data->VTE_CJK_WIDTH_STR, TRUE);
		else
			g_unsetenv("VTE_CJK_WIDTH");
	}
	// g_debug("Get VTE_CJK_WIDTH = %s in add_page()", g_getenv("VTE_CJK_WIDTH"));

	if (page_data->VTE_CJK_WIDTH_STR && (page_data->VTE_CJK_WIDTH_STR[0] != '\0'))
	{
#ifdef SAFEMODE
		if (environ_str && environ_str->len)
#else
		if (environ_str->len)
#endif
			environ_str = g_string_append(environ_str, "\t");
		g_string_append_printf (environ_str, "VTE_CJK_WIDTH=%s", page_data->VTE_CJK_WIDTH_STR);
	}

// ---- Get the final environment data ---- //

	// g_debug("final (in add_page) : environ_str = %s", environ_str->str);
	gchar **new_environs = NULL;
#ifdef SAFEMODE
		if (environ_str && environ_str->len)
#else
		if (environ_str->len)
#endif
			new_environs = split_string(environ_str->str, "\t", -1);
	// print_array("! add_page() environ", new_environs);
	g_string_free(environ_str, TRUE);

// ---- create vte ---- //
	page_data->vte = vte_terminal_new();
	//g_debug("The default encoding of vte is %s",
	//	vte_terminal_get_encoding(VTE_TERMINAL(page_data->vte)));

#if (defined (VTE_HAS_INNER_BORDER) && defined(USE_GTK3_GEOMETRY_METHOD)) || defined(UNIT_TEST)
	gtk_widget_style_get(GTK_WIDGET(page_data->vte), "inner-border", &(page_data->border), NULL);
#endif

	// save the data first
	// g_debug("Save the data with page_data->vte = %p, page_data = %p", page_data->vte, page_data);
	g_object_set_data(G_OBJECT(page_data->vte), "Page_Data", page_data);

	// g_debug("call set_encoding() by %p to %s", page_data->vte, page_data->encoding_str);
#ifdef SAFEMODE
	if (page_data->encoding_str && (page_data->encoding_str[0]!='\0'))
#endif
		vte_terminal_set_encoding(VTE_TERMINAL(page_data->vte), page_data->encoding_str);
	//g_debug("The encoding of new vte is %s",
	//	vte_terminal_get_encoding(VTE_TERMINAL(page_data->vte)));
#ifdef USE_GTK_SCROLLABLE
	page_data->adjustment = gtk_scrollable_get_vadjustment(GTK_SCROLLABLE(page_data->vte));
#else
	page_data->adjustment = vte_terminal_get_adjustment(VTE_TERMINAL(page_data->vte));
#endif
// ---- Execute programs in the vte ---- //

	//if (command_line==NULL)
	//	command_line = g_getenv("SHELL");

#ifdef DETAIL
	if (win_data->command)
		g_debug("! add_page(): command line = %s", win_data->command);
	if (win_data->argv)
		print_array("! add_page(): argv", win_data->argv);
#endif

	if (page_data_prev)
	{
		// g_debug("page_data_prev->pwd = %s, win_data->home = %s", page_data_prev->pwd, win_data->home);
		// page_data->pwd = get_init_dir(get_tpgid(page_data_prev->pid), page_data_prev->pwd, win_data->home);
		page_data->pwd = get_init_dir(get_tpgid(page_data_prev->pid), NULL, win_data->home);
	}
	else
	{
		// g_debug("win_data->init_dir = %s, win_data->home = %s", win_data->init_dir, win_data->home);
		page_data->pwd = get_init_dir(-1, win_data->init_dir, win_data->home);
	}
	// g_debug("page_data->pwd = %s", page_data->pwd);

	// extern char **environ;
	// gchar **old_environ = environ;
	// extern gchar **empty_environ;
	// print_array("add_page(): environ:", environ);

	// const gchar *PATH = NULL;
	// if (win_data->command)
	// {
	//	// environ = new_environs;
	//	PATH = g_getenv("PATH");
	//	// g_debug("add_page(): get PATH = %s", PATH);
	//}

	// FIXME: Why clear the environ here?
	// environ = empty_environ;
	// if (win_data->command) setenv("PATH", PATH, TRUE);

	gboolean argv_need_be_free = FALSE;
	gchar **argv = get_argv(win_data, &argv_need_be_free);
	gchar **final_argv = argv;
#ifdef USE_OLD_VTE_FORK_COMMAND
	// pid_t vte_terminal_fork_command (VteTerminal *terminal,
	//				    const char *command,
	//				    char **argv,
	//				    char **envv,
	//				    const char *working_directory,
	//				    gboolean lastlog,
	//				    gboolean utmp,
	//				    gboolean wtmp);
	// if (win_data->command==NULL) win_data->command = win_data->shell;
	// if (win_data->command==NULL) win_data->command = "/bin/sh";

	if ((win_data->argv==NULL) && (win_data->login_shell)) final_argv = login_shell_str;

	// g_debug("win_data->command = %s", win_data->command);
	// print_array("argv = ", argv);
	// print_array("new_environs = ", new_environs);
	// g_debug("page_data->pwd = %s", page_data->pwd);

	page_data->pid = vte_terminal_fork_command(VTE_TERMINAL(page_data->vte),
						   win_data->command,
						   final_argv,
						   new_environs,
						   page_data->pwd,
						   win_data->utmp,
						   win_data->utmp,
						   win_data->utmp);
#else
	// gboolean vte_terminal_spawn_sync (VteTerminal *terminal,
	//				     VtePtyFlags pty_flags,
	//				     const char *working_directory,
	//				     char **argv,
	//				     char **envv,
	//				     GSpawnFlags spawn_flags,
	//				     GSpawnChildSetupFunc child_setup,
	//				     gpointer child_setup_data,
	//				     GPid *child_pid,
	//				     GCancellable *cancellable,
	//				     GError **error);
	gboolean fork_stats;
	GSpawnFlags spawn_flags = G_SPAWN_SEARCH_PATH | G_SPAWN_CHILD_INHERITS_STDIN;
	GError *error = NULL;
	gint final_argv_need_be_free = FALSE;

	login_shell_str[0] = win_data->shell;
	// g_debug("win_data->shell = %s", win_data->shell);
#  ifdef SAFEMODE
	if (login_shell_str[0]==NULL) login_shell_str[0] = "/bin/sh";
#  endif
	if (win_data->argv==NULL)
	{
		if (win_data->login_shell)
		{
			final_argv = login_shell_str;
			final_argv_need_be_free = FALSE;
			spawn_flags |= G_SPAWN_FILE_AND_ARGV_ZERO;
		}
	}
	else
	{
		// g_debug("win_data->command = %s", win_data->command);

		spawn_flags |= G_SPAWN_FILE_AND_ARGV_ZERO;

		gchar *argv_str = convert_array_to_string(argv, '\x10');
		gchar *final_argv_str = g_strdup_printf("%s\x10%s", win_data->command, argv_str);
		final_argv = split_string(final_argv_str, "\x10", -1);
		final_argv_need_be_free = TRUE;
#  ifdef SAFEMODE
		if (final_argv==NULL)
		{
			final_argv = full_argv;
			final_argv_need_be_free = FALSE;
		}
#  endif
		// print_array("add_page(): final_argv", final_argv);
		g_free(argv_str);
		g_free(final_argv_str);
	}

	// g_debug("vte_terminal_spawn_sync(): pty_flags = %d", VTE_PTY_DEFAULT);
	// g_debug("vte_terminal_spawn_sync(): working_directory = %s", page_data->pwd);
	// print_array("vte_terminal_spawn_sync(): real_arg", final_argv);
	// print_array("vte_terminal_spawn_sync(): envv", new_environs);
	// g_debug("vte_terminal_spawn_sync(): win_data->login_shell = %d", win_data->login_shell);
	// g_debug("vte_terminal_spawn_sync(): spawn_flags = %d", spawn_flags);

	gint pty_flags = VTE_PTY_DEFAULT;
	if (win_data->utmp)
		pty_flags = (VTE_PTY_NO_LASTLOG | VTE_PTY_NO_UTMP | VTE_PTY_NO_WTMP | VTE_PTY_DEFAULT);

	fork_stats = vte_terminal_spawn_sync (VTE_TERMINAL(page_data->vte),
						     pty_flags,
						     page_data->pwd,
						     final_argv,
						     new_environs,
						     spawn_flags,
						     NULL,
						     NULL,
						     &(page_data->pid),
						     NULL,
						     &error);
	if (final_argv_need_be_free) g_strfreev(final_argv);
	login_shell_str[0] = NULL;
#endif
	if (argv_need_be_free) g_strfreev(argv);
	// environ = old_environ;

	// g_debug("Got page_data->pid = %d", page_data->pid);
	// page_data->pid_cmdline = get_cmdline(page_data->pid);

	g_strfreev(new_environs);
	// treat '-e option' as `custom_page_name'
	// print_array ("win_data->argv", win_data->argv);
	if (win_data->argv || (win_data->custom_tab_names_total > win_data->custom_tab_names_current))
	{
		if (win_data->custom_tab_names_total > win_data->custom_tab_names_current)
		{
			page_data->custom_page_name = g_strdup(win_data->custom_tab_names_strs[win_data->custom_tab_names_current]);
			win_data->custom_tab_names_current++;
		}
		else if (win_data->page_shows_current_cmdline)
			page_data->custom_page_name = get_cmdline(page_data->pid);
		else
			page_data->custom_page_name = convert_array_to_string(win_data->argv, ' ');
	}
	// g_debug("Got page_data->custom_page_name = %s", page_data->custom_page_name);

// ---	Create a page and init the page data ---- //

	// create label
	// g_debug("Creating label!!");

	page_data->label=dirty_gtk_hbox_new(FALSE, 0);
	g_object_set_data(G_OBJECT(page_data->label), "VteBox", page_data->vte);
	set_widget_thickness(page_data->label, 0);

	page_data->label_text = gtk_label_new(win_data->page_name);
	gtk_box_pack_start(GTK_BOX(page_data->label), page_data->label_text, TRUE, TRUE, 0);
	set_page_width(win_data, page_data);
	gtk_label_set_ellipsize(GTK_LABEL(page_data->label_text), PANGO_ELLIPSIZE_MIDDLE);
#ifdef USE_GTK2_GEOMETRY_METHOD
	// when dragging the tab on a vte, or dragging a vte to itself, may change the size of vte.
	g_signal_connect(G_OBJECT(page_data->label_text), "size_request",
				 G_CALLBACK(label_size_request), page_data);
#endif
	page_data->label_button = gtk_button_new();
	set_widget_thickness(page_data->label_button, 0);
	GtkWidget *image = gtk_image_new_from_stock(GTK_FAKE_STOCK_CLOSE, GTK_ICON_SIZE_MENU);
	// gtk_button_set_image(GTK_BUTTON(page_data->label_button), image);
	gtk_container_add(GTK_CONTAINER(page_data->label_button), image);
	gtk_button_set_relief(GTK_BUTTON(page_data->label_button), GTK_RELIEF_NONE);
	gtk_button_set_focus_on_click(GTK_BUTTON(page_data->label_button), FALSE);

	// GtkSettings *settings = gtk_widget_get_settings(page_data->label_text);
	// int w, h;
	// gtk_icon_size_lookup_for_settings(settings, GTK_ICON_SIZE_MENU, &w, &h);
	// gtk_widget_set_size_request(page_data->label_button, w + 4, h + 4);

	gtk_box_pack_start(GTK_BOX(page_data->label), page_data->label_button, FALSE, FALSE, 0);
	g_signal_connect(G_OBJECT(page_data->label_button), "clicked", G_CALLBACK(close_page), GINT_TO_POINTER(CLOSE_WITH_TAB_CLOSE_BUTTON));
	gtk_widget_show_all(page_data->label);
	// done in notebook_page_added()
	// if (! (win_data->show_close_button_on_tab || win_data->show_close_button_on_all_tabs))
	//	gtk_widget_hide (page_data->label_button);

	// create a hbox
	page_data->hbox = dirty_gtk_hbox_new(FALSE, 0);
#ifdef GEOMETRY
	g_signal_connect(G_OBJECT(page_data->hbox), "size-allocate",
			 G_CALLBACK(widget_size_allocate), "hbox");
#endif
	// gtk_widget_show_all(page_data->hbox);
	// if (gtk_widget_get_window (page_data->hbox))
	//	g_debug("HBOX(%p): WINDOWID = %ld", page_data->hbox, GDK_WINDOW_XID (gtk_widget_get_window (page_data->hbox)));

	// Get current vte size. for init a new tab.
	glong column=SYSTEM_COLUMN, row=SYSTEM_ROW;
#ifdef SAFEMODE
	if (page_data_prev && (page_data_prev->vte))
#else
	if (page_data_prev)
#endif
	{
		column = vte_terminal_get_column_count(VTE_TERMINAL(page_data_prev->vte));
		row = vte_terminal_get_row_count(VTE_TERMINAL(page_data_prev->vte));
		// g_debug("got prev_vte = %d x %d", column, row);
	}
	else
	{
		column = win_data->default_column;
		row = win_data->default_row;
	}

#ifdef GEOMETRY
	g_debug("@ add_page(): Init New vte with %ld x %ld!", column, row);
	g_debug("@ add_page(): Using the font : %s", page_data->font_name);
#endif

// ---- Init the new page ---- //

	// Init new page. run_once: some settings only need run once.
	// run_once only = TRUE when initing LilyTerm in main().
	init_new_page(win_data, page_data, column, row);

#ifdef USE_GTK_SCROLLABLE
	page_data->scroll_bar = gtk_vscrollbar_new(gtk_scrollable_get_vadjustment(GTK_SCROLLABLE(page_data->vte)));
#else
	page_data->scroll_bar = gtk_vscrollbar_new(vte_terminal_get_adjustment(VTE_TERMINAL(page_data->vte)));
#endif
	pack_vte_and_scroll_bar_to_hbox(win_data, page_data);
	// g_debug("add_page(): check_show_or_hide_scroll_bar(win_data) = %d", check_show_or_hide_scroll_bar(win_data));
	show_and_hide_scroll_bar(page_data, check_show_or_hide_scroll_bar(win_data));
	gtk_widget_set_no_show_all(page_data->scroll_bar, TRUE);

#if defined(GEOMETRY) || defined(UNIT_TEST)
	g_signal_connect(G_OBJECT(page_data->vte), "size-allocate",
			 G_CALLBACK(vte_size_allocate), page_data);
#endif
	g_signal_connect(G_OBJECT(page_data->vte), "decrease-font-size",
			 G_CALLBACK(vte_size_changed), GINT_TO_POINTER(FONT_ZOOM_IN));
	g_signal_connect(G_OBJECT(page_data->vte), "increase-font-size",
			 G_CALLBACK(vte_size_changed), GINT_TO_POINTER(FONT_ZOOM_OUT));
	// the close page event
	if (! (win_data->hold && win_data->command))
		g_signal_connect(G_OBJECT(page_data->vte), "child_exited", G_CALLBACK(close_page), GINT_TO_POINTER(CLOSE_TAB_NORMAL));

	// when get focus, update `current_vte', hints, and window title
	g_signal_connect(G_OBJECT(page_data->vte), "grab-focus", G_CALLBACK(vte_grab_focus), NULL);

	// show the menu
	g_signal_connect(G_OBJECT(page_data->vte), "button-press-event",
			 G_CALLBACK(vte_button_press), page_data);
	g_signal_connect(G_OBJECT(page_data->vte), "button-release-event",
			 G_CALLBACK(vte_button_release), page_data);

	add_remove_window_title_changed_signal(page_data);

	// g_signal_connect(G_OBJECT(page_data->vte), "paste-clipboard",
	//		 G_CALLBACK(vte_paste_clipboard), GDK_SELECTION_PRIMARY);

//	GdkRGBA root_color;
//	dirty_gdk_color_parse("#FFFAFE", &root_color);
//	gtk_widget_modify_bg(notebook, GTK_STATE_NORMAL, &root_color);

// ---- Init other datas of the page ---- //

	// set the tab name.
	page_data->current_tpgid = page_data->pid;
	page_data->displayed_tpgid = 0;
	// page_data->tab_color = win_data->user_page_color[5];
	page_data->is_root = check_is_root(page_data->pid);
	page_data->is_bold = win_data->bold_current_page_name;

// ---- add the new page to notebook ---- //

	// Note that due to historical reasons,
	// GtkNotebook refuses to switch to a page unless the child widget is visible.
	// Therefore, it is recommended to show child widgets before adding them to a notebook.
	gtk_widget_show_all(page_data->hbox);

	// if (! win_data->show_scroll_bar)
	//	hide_scroll_bar(win_data, page_data);

	// FIXME: dirty hack
	// if (page_data->window==win_data->window), notebook_page_added() won't work.
	// g_debug("Set page_data->window = NULL in add_page()");
	page_data->window = NULL;
	// g_debug("page_data->vte = %p", page_data->vte);
	// g_debug("win_data->current_vte = %p", win_data->current_vte);
#ifdef SAFEMODE
	if (page_data->notebook) {
#endif
		page_data->page_no = gtk_notebook_append_page(GTK_NOTEBOOK(page_data->notebook),
						      page_data->hbox, page_data->label);
		// g_debug("Got page_data->page_no = %d", page_data->page_no);

#ifdef DISABLE_PAGE_ADDED
		notebook_page_added (GTK_NOTEBOOK(page_data->notebook), NULL, page_data->page_no, win_data);
#endif
		// g_debug("The new page no is %d", page_data->page_no);
		// move the page to next to original page
		if (page_data_prev && (add_to_next))
		{
			gtk_notebook_reorder_child(GTK_NOTEBOOK(page_data->notebook), page_data->hbox,
								page_data_prev->page_no + 1);
			// g_debug("New Page No after move to next to prev page = %d", page_data->page_no);
		}
#ifdef SAFEMODE
	}
#endif
	win_data->current_vte = page_data->vte;

// ---- Monitor cmdline ---- //

	// g_debug("!!! Launch get_and_update_page_name() in add_page()!");

	if ((proc_exist) &&
	    (win_data->page_shows_current_cmdline ||
	     win_data->page_shows_current_dir ||
	     win_data->page_shows_window_title))
	{
		add_remove_page_timeout_id(win_data, page_data);
		// g_debug("Timeout Added: %d (%d)", page_data->timeout_id, page_data->vte);
	}
	else
	{
		// g_debug("Launch get_and_update_page_name() in add_page()!");
		win_data->page_names_no++;
	}
	// g_debug("Got label name from get_and_update_page_name(): %s", page_data->page_name);

// ---- Restore the VTE_CJK_WIDTH environment ---- //

	restore_SYSTEM_VTE_CJK_WIDTH_STR();
	// g_debug("Restore VTE_CJK_WIDTH = %s in add_page()", g_getenv("VTE_CJK_WIDTH"));

// ---- Finish ---- //

	// gtk_window_set_focus(GTK_WINDOW(window), page_data->vte);

	// g_debug("The final encoding of vte is %s" ,vte_terminal_get_encoding(VTE_TERMINAL(page_data->vte)));

	// page_data->pid < 0: Error occurred when creating sub process.
	// FIXME: how about "current_vte != page_data->vte"??
#ifdef USE_OLD_VTE_FORK_COMMAND
	if (page_data->pid < 0)
#else
	if (fork_stats==0)
#endif
	{
		extern gchar *init_encoding;
		gchar *arg_str = NULL;
		if (win_data->argv) arg_str = convert_array_to_string(win_data->argv, ' ');

		// g_debug("win_data->runtime_encoding = %s", win_data->runtime_encoding);
#ifdef USE_OLD_VTE_FORK_COMMAND
		// g_debug("Error occurred when creating sub process");
		// g_debug("arg_str = %s", arg_str);
		// g_debug("init_encoding = %s", init_encoding);
		// g_debug("win_data->runtime_encoding = %s", win_data->runtime_encoding);
		// g_debug("page_data->encoding_str = %s", page_data->encoding_str);
		if ((compare_strings(init_encoding, runtime_locale_encoding, FALSE)) ||
		    (compare_strings(init_encoding, "UTF-8", FALSE)))
			create_utf8_child_process_failed_dialog (win_data, arg_str, runtime_locale_encoding);
		else
			create_child_process_failed_dialog(win_data, arg_str, win_data->runtime_encoding);
#else
		if ((compare_strings(init_encoding, runtime_locale_encoding, FALSE)) ||
		    (compare_strings(init_encoding, "UTF-8", FALSE)))
			create_utf8_child_process_failed_dialog (win_data, arg_str, runtime_locale_encoding);
		else
#ifdef SAFEMODE
		{
			if (error)
#endif
				create_child_process_failed_dialog(win_data, error->message,
								   win_data->runtime_encoding);
#ifdef SAFEMODE
		}
		if (error)
#endif
			g_clear_error(&error);
#endif
		g_free(arg_str);
		clear_arg(win_data);
		close_page (page_data->vte, CLOSE_TAB_NORMAL);
		return NULL;
	}
	else
	{
		// We MUST clear command_line and parameters after ran -e option.
		clear_arg(win_data);
		// FIXME: Calling notebook_page_added() here
		// page_data->window=NULL;
		// notebook_page_added(GTK_NOTEBOOK(page_data->notebook), NULL, page_data->page_no, win_data);
		return page_data;
	}
}

gchar **get_argv(struct Window *win_data, gboolean *argv_need_be_free)
{
#ifdef DETAIL
	g_debug("! Launch get_argv()! with win_data = %p", win_data);
#endif
#ifdef SAFEMODE
	if ((win_data==NULL) || (argv_need_be_free==NULL)) return NULL;
#endif
	gchar **argv = win_data->argv;
	*argv_need_be_free = FALSE;

#ifdef SAFEMODE
	if ((argv==NULL) && win_data->shell && (win_data->shell[0]!='\0'))
#else
	if ((argv==NULL) && (win_data->shell[0]!='\0'))
#endif
	{
		argv = split_string(win_data->shell, " ", -1);
		*argv_need_be_free = TRUE;
#ifdef SAFEMODE
	}

	if (argv==NULL)
	{
		argv = g_strsplit("/bin/sh", " ", -1);
		*argv_need_be_free = TRUE;
	}

	if (argv==NULL)
	{
		argv = login_shell_str;
		*argv_need_be_free = FALSE;
#endif
	}
	return argv;
}


void create_utf8_child_process_failed_dialog (struct Window *win_data, gchar *message, gchar *encoding)
{
#ifdef DETAIL
	g_debug("! Launch create_utf8_child_process_failed_dialog()! with win_data = %p, message = %s, "
		"encoding = %s", win_data, message, encoding);
#endif
	gchar *utf8_arg_str = convert_str_to_utf8(message, encoding);
	create_child_process_failed_dialog(win_data, utf8_arg_str, NULL);
	g_free(utf8_arg_str);
}

void create_child_process_failed_dialog(struct Window *win_data, gchar *message, gchar *encoding)
{
#ifdef DETAIL
	g_debug("! Launch create_child_process_failed_dialog() with message = %s, encoding = %s",
		message, encoding);
#endif
#ifdef SAFEMODE
	if (win_data==NULL) return;
#endif
	gint i;
	gchar *temp_str[3] = {NULL};
	temp_str[0] = convert_str_to_utf8(message, encoding);
	if (temp_str[0]!=NULL)
		temp_str[1] = g_markup_escape_text(temp_str[0], -1);
	if (temp_str[1])
		temp_str[2] = g_strconcat(_("Error while creating the child process:"),
					  "\n\n\t<b><span foreground=\"",
					  "blue",
					  "\">",
					  temp_str[1],
					  "</span></b>",
					  NULL);
#ifndef UNIT_TEST
	else
		temp_str[2] = _("Error while creating the child process!\n"
				"Please make sure the /dev/pts is mounted as a devpts file system!");
#endif
	// g_debug("message = %s", message);
	// for (i=0; i<3; i++)
	//	g_debug("temp_str[%d] = %s", i, temp_str[i]);
	// error_dialog(GtkWidget *window, gchar *title, gchar *icon, gchar *message, gchar *encoding)
	GtkWidget *window = NULL;
#ifdef SAFEMODE
	if (win_data)
#endif
		window = win_data->window;
	error_dialog(window,
		     _("Error when creating child process"),
		     "Error when creating child process",
		     GTK_FAKE_STOCK_DIALOG_ERROR,
		     temp_str[2],
		     NULL);
	if (temp_str[1]) g_free(temp_str[2]);
	for (i=0; i<2; i++)
		g_free(temp_str[i]);
}

void clear_arg(struct Window *win_data)
{
#ifdef DETAIL
	g_debug("! Launch clear_arg()");
#endif
#ifdef SAFEMODE
	if (win_data==NULL) return;
#endif
	// g_debug("clear_arg(): win_data = %p, win_data->command = %s, win_data->argc = %d", win_data, win_data->command, win_data->argc);
	// print_array("clear_arg(): win_data->argv", win_data->argv);

	win_data->command = NULL;
	win_data->hold = FALSE;
	win_data->argc = 0;
	win_data->argv = NULL;
}

#if defined (USE_GTK2_GEOMETRY_METHOD) || defined(UNIT_TEST)
void label_size_request (GtkWidget *label, GtkRequisition *requisition, struct Page *page_data)
{
#  ifdef DETAIL
	g_debug("! Launch label_size_request() with page_data = %p", page_data);
#  endif
#  ifdef SAFEMODE
	if ((page_data==NULL) || (page_data->window==NULL)) return;
#  endif
	struct Window *win_data = (struct Window *)g_object_get_data(G_OBJECT(page_data->window), "Win_Data");
#  ifdef SAFEMODE
	if (win_data==NULL) return;
#  endif
	// g_debug("label_size_request(): launch keep_window_size()!");

#  ifdef GEOMETRY
	g_debug("@ label_size_request(for %p): Call keep_gtk2_window_size() with keep_vte_size = %x",
		win_data->window, win_data->keep_vte_size);
#  endif
	keep_gtk2_window_size (win_data, page_data->vte, 0x3);
}
#endif

#if defined(GEOMETRY) || defined(UNIT_TEST)
void vte_size_allocate (GtkWidget *vte, GtkAllocation *allocation, struct Page *page_data)
{
#  ifdef DETAIL
	g_debug("! Launch vte_size_allocate() with vte = %p, page_data = %p", vte, page_data);
#  endif
#ifdef USE_GTK3_GEOMETRY_METHOD
	// fprintf(stderr, "\033[1;36m** vte_size_allocate(): the allocated size is %d x %d (%ldx%ld)\033[0m\n",
	//	allocation->width, allocation->height,
	//	vte_terminal_get_column_count(VTE_TERMINAL(vte)),
	//	vte_terminal_get_row_count(VTE_TERMINAL(vte)));
#  ifdef SAFEMODE
	if ((page_data==NULL) || (page_data->window==NULL)) return;
#  endif
	struct Window *win_data = (struct Window *)g_object_get_data(G_OBJECT(page_data->window), "Win_Data");

	glong column = vte_terminal_get_column_count(VTE_TERMINAL(vte));
	glong row = vte_terminal_get_row_count(VTE_TERMINAL(vte));
	gboolean check_col_row = (column < 80) || (row < 24);

	if (win_data->hints_type==HINTS_NONE)
	{
		column = vte_terminal_get_char_width(VTE_TERMINAL(vte))*column + page_data->border->left + page_data->border->right;
		row = vte_terminal_get_char_height(VTE_TERMINAL(vte))*row + page_data->border->top + page_data->border->bottom;
	}
	check_col_row |= (column != win_data->geometry_width);
	check_col_row |= (row != win_data->geometry_height);

	if (((win_data->window_status!=WINDOW_NORMAL) && (win_data->window_status!=WINDOW_RESIZING_TO_NORMAL)) && check_col_row)
	{
		fprintf(stderr, "\033[1;31m!! vte_size_allocate(win_data %p)(vte %p): the allocated size is %d x %d (%ldx%ld) (saved: %ld x %ld)\033[0m\n",
			win_data, vte, allocation->width, allocation->height, column, row, win_data->geometry_width, win_data->geometry_height);
	}
	else
#endif
		widget_size_allocate (vte, allocation, "vte");
}
#endif

void vte_size_changed(VteTerminal *vte, Font_Set_Type type)
{
#ifdef DETAIL
	g_debug("! Launch vte_size_changed() with vte = %p, type = %d", vte, type);
#endif
	set_vte_font(NULL, type);
}

// close_type = confirm to exit foreground running command or not
// close_type > 0: Not using 'exit' or '<Ctrl><D>' to close this page.
gboolean close_page(GtkWidget *vte, gint close_type)
{
#ifdef DETAIL
	g_debug("! Launch close_page() with vte = %p, and close_type = %d, force_to_quit = %d",
		vte, close_type, force_to_quit);
#endif
#ifdef SAFEMODE
	if (vte==NULL) return FALSE;
#endif
	if (close_type==CLOSE_WITH_TAB_CLOSE_BUTTON)
		vte=(GtkWidget *)g_object_get_data(G_OBJECT(gtk_widget_get_parent(vte)), "VteBox");

	struct Page *page_data = (struct Page *)g_object_get_data(G_OBJECT(vte), "Page_Data");
	// g_debug("Get page_data = %p, pid = %d, vte = %p when closing page!", page_data, page_data->pid, vte);
#ifdef SAFEMODE
	if ((page_data==NULL) || (page_data->window==NULL)) return FALSE;
#endif

	struct Window *win_data = (struct Window *)g_object_get_data(G_OBJECT(page_data->window),
									     "Win_Data");
#ifdef SAFEMODE
	if (win_data==NULL) return FALSE;
#endif

	if (win_data->confirm_to_kill_running_command == FALSE)
	{
		force_to_quit=TRUE;
		close_type=CLOSE_TAB_NORMAL;
	}

	GString *child_process_list = g_string_new(NULL);
	// close_type = TRUE: Not using 'exit' or '<Ctrl><D>' to close this page.
	if (close_type)
	{
		if (process_data==NULL)
		{
			process_data = g_new0(struct Process_Data, PID_MAX_DEFAULT);
#ifdef SAFEMODE
			if (process_data)
			{
#endif
				child_process_list = get_child_process_list(0, 0, 0, child_process_list,
									    page_data->pid, win_data, FALSE);
				clean_process_data();
#ifdef SAFEMODE
			}
#endif
		}
		else
		{
			gint i;
			for (i=2; i<PID_MAX_DEFAULT; i++)
			{
				if ((process_data[i].page_no == ((page_data->page_no)+1)) &&
				    ((process_data[i].window==0) || (process_data[i].window==page_data->window)))
				{
					gchar *cmdline = get_cmdline(i);
#ifdef SAFEMODE
					if (cmdline)
#endif
						g_string_append_printf(child_process_list ,"\t(%d) %s\n", i, cmdline);
					g_free(cmdline);
				}
			}
		}
	}
	// g_debug("Got child_process_list = %s", child_process_list->str);

	// g_debug("Deleting Page ID: %d", page_data->pid);

	// Confirm to close current running command...
	if ((! force_to_quit) && (page_data->pid>1) && (! win_data->kill_color_demo_vte))
	{
		pid_t tpgid = get_tpgid(page_data->pid);
		gchar **stats = get_pid_stat(tpgid, 4);
		if (stats)
		{
			// g_debug("Got tpgid = %s, cmd = %s", stats[9], stats[2]);
			// if ((page_data->pid != atoi(stats[9])) &&
			//    (! check_string_in_array(stats[2], win_data->foreground_program_whitelists)))
			if (! check_string_in_array(stats[2], win_data->foreground_program_whitelists))
			{
				// g_debug("Got page_data->pid = %d, tpgid = %d", page_data->pid, tpgid);

				// We need to set "win_data->current_vte = vte", or wrong page may be closed.
				GtkWidget *current_vte_orig = win_data->current_vte;
				win_data->current_vte = vte;

				GtkResponseType return_value = dialog(NULL, CONFIRM_TO_CLOSE_RUNNING_APPLICATION);
				// restore win_data->current_vte
				win_data->current_vte = current_vte_orig;

				if (return_value != GTK_RESPONSE_OK)
				{
					g_string_free(child_process_list, TRUE);
					g_strfreev(stats);
					return FALSE;
				}
			}
		}
		g_strfreev(stats);
	}

	// Confirm to close tab if it have child process...
#ifdef SAFEMODE
	if ((! force_to_quit) &&
	    (child_process_list && child_process_list->len) &&
	    (! win_data->kill_color_demo_vte))
#else
	if ((! force_to_quit) && child_process_list->len && (! win_data->kill_color_demo_vte))
#endif
	{
		// Switch win_data->current_vte here,
		// Or the "%d tab" of dialog will incorrect if we click the [X] on a tab that is not on focus.
		GtkWidget *oldvte = win_data->current_vte;
		win_data->current_vte = vte;
		if (!display_child_process_dialog(child_process_list, win_data,
						  CONFIRM_TO_CLOSE_A_TAB_WITH_CHILD_PROCESS))
		{
			g_string_free(child_process_list, TRUE);
			win_data->current_vte = oldvte;
			return FALSE;
		}
		// Switch back
		win_data->current_vte = oldvte;
	}

	if (win_data->auto_save &&
	    (gtk_notebook_get_n_pages(GTK_NOTEBOOK(win_data->notebook))==1))
		save_user_settings(NULL, win_data);

	// remove timeout event for page_shows_current_cmdline
	// if (page_data->page_shows_current_cmdline ||
	//    page_data->page_shows_current_dir ||
	//    page_data->page_shows_window_title)
	if (page_data->timeout_id)
		g_source_remove (page_data->timeout_id);
	if (page_data->urgent_bell_handler_id)
		g_signal_handler_disconnect(G_OBJECT(page_data->vte), page_data->urgent_bell_handler_id);

	// kill running shell
#ifdef SAFEMODE
	if (using_kill && child_process_list && child_process_list->len)
#else
	if (using_kill && child_process_list->len)
#endif
	{
		// if the tab is not close by <Ctrl><D>, we need to launch kill()
		// g_debug("Trying to kill %d!", page_data->pid);
		kill(page_data->pid, SIGKILL);
	}
	g_string_free(child_process_list, TRUE);

	// restore using_kill if only one page is closed.
	if (! force_to_quit)
		using_kill = FALSE;

	if (vte==win_data->current_vte)
	{
		win_data->current_vte = NULL;
		// set the current page
		// g_debug ("Setting current page to %d!", page_num);
		if (page_data->page_no==(gtk_notebook_get_n_pages(GTK_NOTEBOOK(page_data->notebook))-1))
			gtk_notebook_set_current_page(GTK_NOTEBOOK(page_data->notebook), page_data->page_no-1);
		else
			gtk_notebook_set_current_page(GTK_NOTEBOOK(page_data->notebook), page_data->page_no+1);
	}

#if defined(USE_GTK3_GEOMETRY_METHOD) || defined(UNIT_TEST)
	gtk_border_free(page_data->border);
#endif
	g_free(page_data->page_name);

	// Note that due to historical reasons,
	// GtkNotebook refuses to switch to a page unless the child widget is visible.
	// Therefore, it is recommended to show child widgets before adding them to a notebook.
	gtk_widget_hide_all(page_data->hbox);

	// gtk_widget_destroy(page_data->vte);
	// remove current page
	// use page_data->page_no. DANGEROUS!
	// g_debug ("The %d page is going to be removed!", page_data->page_no);

	// Due to the bugs in GTK3+(?), Using gtk_widget_destroy(windows) instead of closing the last page!
	gint total_page = gtk_notebook_get_n_pages(GTK_NOTEBOOK(page_data->notebook));
	gboolean run_quit_gtk = FALSE;
	if (total_page==1)
	{
		run_quit_gtk = gtk_widget_get_mapped(win_data->window);
		gtk_widget_destroy (win_data->window);
		win_data->window = NULL;
	}
	else
		gtk_notebook_remove_page(GTK_NOTEBOOK(page_data->notebook), page_data->page_no);

	// free the memory used by this page
	g_free(page_data->pid_cmdline);
	g_free(page_data->custom_page_name);
	g_free(page_data->pwd);
	g_free(page_data->font_name);
	g_free(page_data->encoding_str);
	g_free(page_data->locale);
	g_free(page_data->VTE_CJK_WIDTH_STR);
	g_free(page_data->window_title_pwd);
	g_free(page_data->environ);

	// FIXME: Calling remove_notebook_page() here
	// GtkNotebook may drag a page to itself,
	// So a GtkNotebook may be page_removed() -> page_added() with the same page
	// but the GtkNotebook may be destroyed after page_removed()
	// So that we should call page_removed() here, not using "page-removed" signal... -_-|||
	// g_debug("page_data->notebook = %p, page_data->page_no = %d, win_data = %p",
	//	page_data->notebook, page_data->page_no, win_data);
	if (total_page==1)
		remove_notebook_page (NULL, NULL, page_data->page_no, win_data, run_quit_gtk);
	else
		remove_notebook_page (GTK_NOTEBOOK(page_data->notebook), NULL, page_data->page_no, win_data, run_quit_gtk);

	// g_strfreev(page_data->environments);

	g_free(page_data);

	return TRUE;
}


void vte_grab_focus(GtkWidget *vte, gpointer user_data)
{
#ifdef DETAIL
	g_debug("! Launch vte_grab_focus() with vte = %p", vte);
#endif
#ifdef SAFEMODE
	if (vte==NULL) return;
#endif
	// g_debug("vte = %p grab focus !", vte);
	struct Page *page_data = (struct Page *)g_object_get_data(G_OBJECT(vte), "Page_Data");
#ifdef SAFEMODE
	if (page_data==NULL) return;
#endif
	struct Window *win_data = (struct Window *)g_object_get_data(G_OBJECT(page_data->window), "Win_Data");
#ifdef SAFEMODE
	if (win_data==NULL) return;
#endif
	// restore match_regex first...
	if (! page_data->match_regex_setted) set_hyperlink(win_data, page_data);

	// g_debug("Get win_data = %p (page_data->window = %p) when vte grab focus!", win_data, page_data->window);

	//if (win_data->lost_focus)
	//	return;

	// Recover the dim text of vte
	dim_vte_text (win_data, page_data, 0);

	// if (gtk_widget_get_window (page_data->vte))
	//	g_debug("VTE(%p): WINDOWID = %ld", page_data->vte, GDK_WINDOW_XID (gtk_widget_get_window (page_data->vte)));

	// Don't update page name when win_data->kill_color_demo_vte.
	// Or LilyTerm will got warning: "Failed to set text from markup due to error parsing markup"
	if ((win_data->current_vte != vte || (win_data->current_vte == NULL)) && (! win_data->kill_color_demo_vte))
	{
		// win_data->current_vte == NULL: when the creation of sub process failed.
		// like 'lilyterm -e xxx' in a lilyterm
		if (win_data->current_vte != NULL)
		{
			struct Page *prev_data = (struct Page *)g_object_get_data(
								G_OBJECT(win_data->current_vte), "Page_Data");
#ifdef SAFEMODE
			if (prev_data)
			{
#endif
				if (!win_data->show_close_button_on_all_tabs)
					gtk_widget_hide (prev_data->label_button);

				// g_debug("un_bold prev page.");
				if ((win_data->bold_current_page_name || win_data->bold_action_page_name))
				{
					prev_data->is_bold = FALSE;
					prev_data->should_be_bold = FALSE;
					// g_debug ("Launch update_page_name() to un_bold prev page!!!");
					update_page_name (prev_data->window, win_data->current_vte, prev_data->page_name,
							  prev_data->label_text, prev_data->page_no+1,
							  prev_data->custom_page_name,
							  prev_data->tab_color, prev_data->is_root, FALSE,
							  compare_strings(win_data->runtime_encoding,
									  prev_data->encoding_str,
									  FALSE),
							  prev_data->encoding_str, prev_data->custom_window_title,
							  FALSE);
				}
#ifdef SAFEMODE
			}
#endif
		}
		// g_debug ("Update current_vte! (%p), and hints_type = %d", vte, win_data->hints_type);
		// current_vte = vte;
		win_data->current_vte = vte;

		// We will not update the page name when close color demo vte
		// if (win_data->dialog_activated) return;

		//if (keep_vte_size==0)
		//{
		//	// g_debug("window_resizable in vte_grab_focus!");
		//	// we should bind the hints information on current vte.
		//	// Or the geometry of vte may be changed when deleting the vte hold hints info.
		//	// It can help to hold the correct vte size.
		//	g_debug("Update hints!")
		//	window_resizable(vte, hints_type, 1);
		//}

		// update the window title
		if (win_data->window_title_shows_current_page && (! page_data->custom_window_title))
			update_window_title(page_data->window, page_data->page_name,
					    win_data->window_title_append_package_name);

		show_close_button_on_tab(win_data, page_data);

		// bold/un_bold current page
		// g_debug("then, bold/un_bold current page.");
		if (win_data->bold_current_page_name || win_data->bold_action_page_name)
		{
			page_data->is_bold = win_data->bold_current_page_name;
			// g_debug("page_data->encoding = %p win_data->default_menuitem_encoding = %p",
			//	page_data->encoding, win_data->default_menuitem_encoding);
			// if (win_data->default_menuitem_encoding)
			//	g_debug("win_data->default_menuitem_encoding->name = %s, "
			//		"page_data->encoding_str = %s",
			//		gtk_widget_get_name(win_data->default_menuitem_encoding), page_data->encoding_str);
			update_page_name (page_data->window, vte, page_data->page_name, page_data->label_text,
					  page_data->page_no+1, page_data->custom_page_name, page_data->tab_color,
					  page_data->is_root, page_data->is_bold,
					  compare_strings(win_data->runtime_encoding,
							  page_data->encoding_str,
							  FALSE),
					  page_data->encoding_str, page_data->custom_window_title,
					  FALSE);
		}
	}
}

// dim_text = 0: dim the vte
// dim_text = 1: light up the vte
// dim_text = 2: auto detect
// NOTE: page_data may be NULL here!
void dim_vte_text (struct Window *win_data, struct Page *page_data, gint dim_text)
{
#ifdef DETAIL
	g_debug("! Launch dim_vte_text() with win_data = %p, page_data = %p, dim_text = %d",
		win_data, page_data, dim_text);
#endif
#ifdef SAFEMODE
	if (win_data==NULL) return;
#endif
	// The page_data==NULL && win_data->current_vte==NULL when -e options failed.
	if (page_data==NULL && (win_data->current_vte==NULL)) return;

	// dim_vte = TRUE: dim the vte
	// dim_vte = FALSE: light up the vte
	gint dim_vte = FALSE;

	// if page_data==NULL, using "win_data->current_vte"
	if (! page_data)
		page_data = (struct Page *)g_object_get_data(G_OBJECT(win_data->current_vte), "Page_Data");

	if (dim_text==2)
	{
		if (win_data->dim_text)
			dim_vte = page_data->dim_text_expect;
		else
			dim_vte = FALSE;
	}
	else
	{
		page_data->dim_text_expect = FALSE;
		if (dim_text)
		{
			// Trying to check if we should dim the vte
			// g_debug("menu_activated = %d, dialog_activated = %d, win_data->lost_focus = %d",
			//	menu_activated, dialog_activated, win_data->lost_focus);
			if ((! menu_activated) && (dialog_activated==0) && (win_data->lost_focus))
			{
				page_data->dim_text_expect = TRUE;
				if (win_data->dim_text)
					dim_vte = TRUE;
			}
		}
	}

	// FIXME: Sometimes we should to force light up the vte
	// if ((! win_data->use_set_color_fg_bg))
	//	dim_vte = FALSE;

	// g_debug("CHECK: dim_vte = %d, page_data->vte_is_inactivated = %d", dim_vte, page_data->vte_is_inactivated);

#ifdef SAFEMODE
	if (page_data->vte==NULL) return;
#endif
	// for performance, if the vte was dimmed already, don't dim it again.
	if (page_data->vte_is_inactivated != dim_vte)
	{
		if (dim_vte)
			set_vte_color(page_data->vte, TRUE, win_data->custom_cursor_color, win_data->cursor_color,
				      win_data->color_inactive, TRUE, (win_data->color_theme_index==(THEME-1)));
		else
			set_vte_color(page_data->vte, TRUE, win_data->custom_cursor_color, win_data->cursor_color,
				      win_data->color, TRUE, (win_data->color_theme_index==(THEME-1)));
		page_data->vte_is_inactivated = dim_vte;
	}
	// g_debug("FINAL: dim_vte = %d, page_data->vte_is_inactivated = %d", dim_vte, page_data->vte_is_inactivated);
}

gboolean vte_button_press(GtkWidget *vte, GdkEventButton *event, struct Page *page_data)
{
#ifdef DETAIL
	g_debug("! Launch vte_button_press() for vte %p (page_data = %p)", vte, page_data);
#endif
#ifdef SAFEMODE
	if ((page_data==NULL) || (event==NULL)) return FALSE;
#endif
	menu_active_window = page_data->window;
	// g_debug("set menu_active_window = %p", menu_active_window);
	struct Window *win_data = (struct Window *)g_object_get_data(G_OBJECT(page_data->window), "Win_Data");
#ifdef SAFEMODE
	if (win_data==NULL) return FALSE;
#endif
	// g_debug ("Get win_data = %d in show_menu", win_data);
	if (! win_data->enable_key_binding) return FALSE;

	if (event->button == 2)
	{
		if (win_data->confirm_to_paste)
			return confirm_to_paste_form_clipboard(SELECTION_PRIMARY, win_data, page_data);
		else
			return FALSE;
	}
	else if (event->button == 3)
	{
		menu_activated = TRUE;
		// Find/Add the encoding of current tab on right click menu
		if (win_data->locale_sub_menu)
		{
			// We will not update coding here.
			win_data->checking_menu_item = TRUE;
			//if (page_data->encoding!=NULL)
			//	g_debug ("Update the encoding of page %d in menu to %s",
			//		 page_data->page_no, gtk_widget_get_name(page_data->encoding));
			// g_debug("Set the menuitem %d t active", page_data->encoding);

			// menu is inited after add_page(). so that the encoding_str of 1st page is NULL.
			// if (page_data->encoding_str==NULL)
			//	page_data->encoding_str = g_strdup(win_data->runtime_encoding);

			// gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(page_data->encoding), TRUE);
			// Trying to find the right encoding menuitem...
			if (win_data->encoding_sub_menu)
			{
				GSList *menuitem_list = gtk_radio_menu_item_get_group(
								GTK_RADIO_MENU_ITEM(
									win_data->default_menuitem_encoding));
				while (menuitem_list)
				{
					// g_debug("Checking %s...", gtk_widget_get_name(GTK_WIDGET(menuitem_list->data)));
					if ( ! compare_win_page_encoding(menuitem_list->data,
									 page_data->encoding_str))
					{
						gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(
											menuitem_list->data), TRUE);
						break;
					}
					// g_debug("menuitem_list->next = %p", menuitem_list->next);
					// Not find in menu item list. Add it!
					if (menuitem_list->next == NULL)
					{
						GtkWidget *menuitem = add_radio_menuitem_to_sub_menu (
										    gtk_radio_menu_item_get_group (
											GTK_RADIO_MENU_ITEM (menuitem_list->data)),
										    win_data->encoding_sub_menu,
										    page_data->encoding_str,
										    (GSourceFunc)set_encoding,
										    NULL);
#ifdef SAFEMODE
						if (menuitem)
						{
#endif
							gtk_widget_show (menuitem);
							gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(menuitem),
											TRUE);
#ifdef SAFEMODE
						}
#endif
					}
					menuitem_list = menuitem_list->next;
				}
			}
			// query done
			win_data->checking_menu_item = FALSE;
		}

		// Update the profile list
		refresh_profile_list(win_data);

		win_data->checking_menu_item = TRUE;
#ifdef SAFEMODE
		if (win_data->current_menuitem_theme)
#endif
			gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(win_data->current_menuitem_theme), TRUE);
#ifdef SAFEMODE
		if (win_data->current_menuitem_erase_binding)
#endif
			gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(win_data->current_menuitem_erase_binding), TRUE);
#ifdef ENABLE_CURSOR_SHAPE
#  ifdef SAFEMODE
		if (win_data->current_menuitem_cursor_shape)
#  endif
			gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(win_data->current_menuitem_cursor_shape), TRUE);
#endif
#ifdef SAFEMODE
		if (win_data->menuitem_invert_color)
#endif
			gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(win_data->menuitem_invert_color), win_data->invert_color);
		// g_debug("vte_button_press(): set win_data->menuitem_invert_color(%p) to %d", win_data->menuitem_invert_color, win_data->invert_color);
#ifdef SAFEMODE
		if (win_data->menuitem_auto_save)
#endif
			gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(win_data->menuitem_auto_save), win_data->auto_save);
		win_data->checking_menu_item = FALSE;

		if (win_data->show_background_menu)
		{
#ifdef ENABLE_RGBA
			if (win_data->use_rgba == -1)
#  ifdef SAFEMODE
				if (win_data->menuitem_trans_win)
#  endif
					// GTK_CHECK_MENU_ITEM(win_data->menuitem_trans_win)->active =
					//	win_data->transparent_window;
					gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(win_data->menuitem_trans_win),
									win_data->transparent_window);
#endif
			// The following codes is only working with vte > 0.19
			// gint transparent;
			// g_object_get (vte,
			//	      "background-transparent", &transparent,
			//	      NULL);
			// GTK_CHECK_MENU_ITEM(win_data->menuitem_trans_bg)->active = transparent;
#if defined(ENABLE_VTE_BACKGROUND) || defined(FORCE_ENABLE_VTE_BACKGROUND)
#  ifdef SAFEMODE
			if (win_data->menuitem_trans_bg)
#  endif
				// GTK_CHECK_MENU_ITEM(win_data->menuitem_trans_bg)->active =
				//	win_data->transparent_background;
				gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(win_data->menuitem_trans_bg),
								win_data->transparent_background);
#endif
		}

		if (win_data->show_copy_paste_menu)
		{
			// enable win_data->menuitem_copy_url or not
			if ( win_data->enable_hyperlink)
			{
				gint tag;
				// g_free(win_data->menuitem_copy_url->name);
				gchar *url = get_url(event, page_data, &tag);
				// g_debug("url = %s", url);
				gtk_widget_set_name (win_data->menuitem_copy_url, url);
				// g_debug("name of win_data->menuitem_copy_url is %s",
				//	gtk_widget_get_name(win_data->menuitem_copy_url));
				gtk_widget_set_sensitive (win_data->menuitem_copy_url, url!=NULL);
				g_free(url);
			}

			// enable win_data->menuitem_copy or not
			gtk_widget_set_sensitive (win_data->menuitem_copy,
						  vte_terminal_get_has_selection (VTE_TERMINAL(vte)));

			// enable win_data->menuitem_paste or not
			extern GtkClipboard *selection_clipboard;
			gchar *temp_str = NULL;
#ifdef SAFEMODE
			if (selection_clipboard)
#endif
				temp_str = gtk_clipboard_wait_for_text (selection_clipboard);
			// g_debug("clipboard = %s", temp_str);
			gtk_widget_set_sensitive (win_data->menuitem_paste,
						  (temp_str != NULL));
			gtk_widget_set_sensitive (win_data->menuitem_clipboard,
						  (temp_str != NULL));

			extern GtkClipboard *selection_primary;
#ifdef SAFEMODE
			if (selection_primary)
#endif
				temp_str = gtk_clipboard_wait_for_text (selection_primary);
			gtk_widget_set_sensitive (win_data->menuitem_primary,
						  (temp_str != NULL));
			g_free(temp_str);
		}


		// if (page_data->use_scrollback_lines)
		// GTK_CHECK_MENU_ITEM(win_data->menuitem_scrollback_lines)->active = page_data->use_scrollback_lines;
		if (win_data->dim_text)
#ifdef SAFEMODE
			if (win_data->menuitem_dim_text)
#endif
				// GTK_CHECK_MENU_ITEM(win_data->menuitem_dim_text)->active = win_data->dim_text;
				gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(win_data->menuitem_dim_text),
								win_data->dim_text);
#ifdef ENABLE_RGBA
		if ((win_data->dim_window) && (win_data->use_rgba == -1))
#  ifdef SAFEMODE
			if (win_data->menuitem_dim_window)
#  endif
				// GTK_CHECK_MENU_ITEM(win_data->menuitem_dim_window)->active = win_data->dim_window;
				gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(win_data->menuitem_dim_window),
								win_data->dim_window);
#endif
#ifdef USE_NEW_VTE_CURSOR_BLINKS_MODE
		gboolean blinks, blinks_original = win_data->cursor_blinks;
		if (win_data->cursor_blinks ==0)
			g_object_get(gtk_widget_get_settings(vte), "gtk-cursor-blink", &blinks, NULL);
		else
			blinks = (win_data->cursor_blinks==1) ? TRUE : FALSE;
		// GTK_CHECK_MENU_ITEM(win_data->menuitem_cursor_blinks)->active = blinks;
		gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(win_data->menuitem_cursor_blinks), blinks);
		win_data->cursor_blinks = blinks_original;
#else
		// GTK_CHECK_MENU_ITEM(win_data->menuitem_cursor_blinks)->active = win_data->cursor_blinks;
		gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(win_data->menuitem_cursor_blinks),
						win_data->cursor_blinks);
#endif
		// GTK_CHECK_MENU_ITEM(win_data->menuitem_allow_bold_text)->active = win_data->allow_bold_text;
		gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(win_data->menuitem_allow_bold_text),
						win_data->allow_bold_text);

		// GTK_CHECK_MENU_ITEM(win_data->menuitem_open_url_with_ctrl_pressed)->active = win_data->open_url_with_ctrl_pressed;
		gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(win_data->menuitem_open_url_with_ctrl_pressed),
						win_data->open_url_with_ctrl_pressed);

		// GTK_CHECK_MENU_ITEM(win_data->menuitem_disable_url_when_ctrl_pressed)->active = win_data->disable_url_when_ctrl_pressed;
		gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(win_data->menuitem_disable_url_when_ctrl_pressed),
						win_data->disable_url_when_ctrl_pressed);

		// GTK_CHECK_MENU_ITEM(win_data->menuitem_audible_bell)->active = win_data->audible_bell;
		gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(win_data->menuitem_audible_bell),
						win_data->audible_bell);
#ifdef ENABLE_VISIBLE_BELL
		// GTK_CHECK_MENU_ITEM(win_data->menuitem_visible_bell)->active = win_data->visible_bell;
		gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(win_data->menuitem_visible_bell),
						win_data->visible_bell);
#endif
#ifdef ENABLE_BEEP_SINGAL
		// GTK_CHECK_MENU_ITEM(win_data->menuitem_urgent_bell)->active = win_data->urgent_bell;
		gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(win_data->menuitem_urgent_bell),
						win_data->urgent_bell);
#endif
		if (gtk_notebook_get_show_tabs GTK_NOTEBOOK(win_data->notebook))
		{
			gtk_widget_hide (win_data->menuitem_show_tabs_bar);
			gtk_widget_show (win_data->menuitem_hide_tabs_bar);
			gtk_widget_hide (win_data->menuitem_always_show_tabs_bar);
			gtk_widget_show (win_data->menuitem_always_hide_tabs_bar);
			// GTK_CHECK_MENU_ITEM(win_data->menuitem_always_hide_tabs_bar)->active =
			//	(win_data->always_show_tabs_bar==0)? 1 : 0;
		}
		else
		{
			gtk_widget_show (win_data->menuitem_show_tabs_bar);
			gtk_widget_hide (win_data->menuitem_hide_tabs_bar);
			gtk_widget_show (win_data->menuitem_always_show_tabs_bar);
			gtk_widget_hide (win_data->menuitem_always_hide_tabs_bar);
			// GTK_CHECK_MENU_ITEM(win_data->menuitem_always_show_tabs_bar)->active =
			//	(win_data->always_show_tabs_bar==1)? 1 : 0;
		}

		gtk_menu_popup(GTK_MENU(win_data->menu), NULL, NULL, NULL, NULL, event->button, event->time);
		return TRUE;
	}
	else if (event->button == 1)
	{
		// return if hyperlink is disabled.
		if (! win_data->enable_hyperlink) return FALSE;

		// g_debug("vte_button_press(): event->state & GDK_CONTROL_MASK = %d", event->state & GDK_CONTROL_MASK);
		if (win_data->open_url_with_ctrl_pressed && ((event->state & GDK_CONTROL_MASK)==0))
		{
			// clean the url first...
			clean_hyperlink(win_data, page_data);
			return FALSE;
		}

		gint tag;
		gchar *url = get_url(event, page_data, &tag);
		// g_debug("get url = %s", url);

		// url = NULL -> we don't need to free it.
		gboolean response = FALSE;
		if (url != NULL)
			response = open_url_with_external_command (url, tag, win_data, page_data);
		g_free(url);
		return response;
	}
	return FALSE;
}

gboolean vte_button_release(GtkWidget *vte, GdkEventButton *event, struct Page *page_data)
{
#ifdef DETAIL
	g_debug("! Launch vte_button_release() for vte %p", vte);
#endif
#ifdef SAFEMODE
	if ((page_data==NULL) || (event==NULL)) return FALSE;
#endif

	if (page_data->match_regex_setted == FALSE)
	{
		struct Window *win_data = (struct Window *)g_object_get_data(G_OBJECT(page_data->window), "Win_Data");
#ifdef SAFEMODE
		if (win_data==NULL) return FALSE;
#endif
		if (! win_data->enable_hyperlink) return FALSE;
		set_hyperlink(win_data, page_data);
	}

	return FALSE;
}

gboolean open_url_with_external_command (gchar *url, gint tag, struct Window *win_data, struct Page *page_data)
{
#ifdef DETAIL
	g_debug("! Launch open_url_with_external_command() with url = %s, tag = %d, win_data = %p, page_data = %p",
		url, tag, win_data, page_data);
#endif
#ifdef SAFEMODE
	if ((win_data==NULL) || (page_data==NULL)) return FALSE;
#endif

	gchar *full_command = g_strdup_printf("%s\t%s", win_data->user_command[tag].command, url);
	// g_debug ("full_command = %s", full_command);
	gchar **argv = split_string(full_command, "\t", -1);
#ifdef SAFEMODE
	if (argv==NULL)
	{
		g_free(full_command);
		return FALSE;
	}
#endif
	gint argc = 0;
	while (argv[argc])
		argc++;

	// g_debug("argc = %d", argc);
	// g_debug("win_data->user_command[tag].method = %d", win_data->user_command[tag].method);
	switch (win_data->user_command[tag].method)
	{
		case 0:
		{
			// 0: new tab
			win_data->command = argv[0];
			win_data->argv = argv;
			win_data->argc = argc;
			// g_debug("win_data->user_command[tag].locale = %s",
			//	 win_data->user_command[tag].locale);
			gchar *locale = page_data->locale;
#ifdef SAFEMODE
			if (win_data->user_command[tag].locale &&
			    (win_data->user_command[tag].locale[0] != '\0'))
#else
			if (win_data->user_command[tag].locale[0] != '\0')
#endif
				locale = win_data->user_command[tag].locale;
			// g_debug("hyperlink: locale = %s", locale);

			gchar *encoding = get_encoding_from_locale(locale);
			if (encoding == NULL)
			{
				if (check_and_add_locale_to_warned_locale_list(win_data,
					win_data->user_command[tag].locale))
					create_invalid_locale_error_message(locale);
				locale = page_data->locale;
				encoding = g_strdup(page_data->encoding_str);
			}
			// g_debug("hyperlink: encoding = %s", encoding);

			// g_debug("win_data->user_command[tag].VTE_CJK_WIDTH = %d, "
			//	"get_VTE_CJK_WIDTH_str(win_data->user_command[tag].VTE_CJK_WIDTH) = %s",
			//	win_data->user_command[tag].VTE_CJK_WIDTH ,
			//	get_VTE_CJK_WIDTH_str(win_data->user_command[tag].VTE_CJK_WIDTH));

			// add_page(struct Window *win_data,
			//	    struct Page *page_data_prev,
			//	    GtkWidget *menuitem_encoding,
			//	    gchar *encoding,
			//	    gchar *locale,
			//	    gchar *environments,
			//	    gchar *user_environ,
			//	    gchar *VTE_CJK_WIDTH_STR,
			//	    gboolean add_to_next)
			add_page(win_data,
				 page_data,
				 NULL,
				 encoding,
				 page_data->encoding_str,
				 locale,
				 NULL,
				 win_data->user_command[tag].environ,
				 get_VTE_CJK_WIDTH_str(win_data->user_command[tag].VTE_CJK_WIDTH),
				 TRUE);
			g_free(encoding);
			break;
		}
		case 1:
		{
			// FIXME: use vfork()/execvp() may cause zombie process.
			// But we can't get if the child process is succeed or not
			// if using system() here.
			// system(full_command);

			set_VTE_CJK_WIDTH_environ(win_data->user_command[tag].VTE_CJK_WIDTH);
			GString *environ_str = g_string_new(win_data->environment);
			if (win_data->user_command[tag].environ &&
			    win_data->user_command[tag].environ[0] != '\0')
				g_string_append_printf (environ_str,
							"\t%s",
							win_data->user_command[tag].environ);
#ifdef SAFEMODE
			if (win_data->user_command[tag].locale &&
			    win_data->user_command[tag].locale[0] != '\0')
#else
			if (win_data->user_command[tag].locale[0]!='\0')
#endif
			{
				gchar *lang = get_lang_str_from_locale(win_data->user_command[tag].locale, ".");
				gchar *language = get_language_str_from_locales(win_data->user_command[tag].locale, win_data->default_locale);
				g_string_append_printf (environ_str,
							"\tLANG=%s\tLANGUAGE=%s\tLC_ALL=%s",
							lang,
							language,
							win_data->user_command[tag].locale);
				g_free(language);
				g_free(lang);
			}
			// g_debug("gdk_spawn_on_screen_with_pipes: environ_str = %s", environ_str->str);
			gchar **new_environs = NULL;
#ifdef SAFEMODE
			if (environ_str && environ_str->len)
#else
			if (environ_str->len)
#endif
				new_environs = split_string(environ_str->str, "\t", -1);

			// gboolean g_spawn_async_with_pipes (const gchar *working_directory,
			//				      gchar **argv,
			//				      gchar **envp,
			//				      GSpawnFlags flags,
			//				      GSpawnChildSetupFunc child_setup,
			//				      gpointer user_data,
			//				      GPid *child_pid,
			//				      gint *standard_input,
			//				      gint *standard_output,
			//				      gint *standard_error,
			//				      GError **error);
			if (g_spawn_async_with_pipes (NULL,
						      argv,
						      new_environs,
						      G_SPAWN_SEARCH_PATH,
						      NULL,
						      NULL,
						      NULL,
						      NULL,
						      NULL,
						      NULL,
						      NULL) == FALSE)
			{
				gint i=0;
				while (full_command[i])
				{
					if (full_command[i]=='\t')
						full_command[i]=' ';
					i++;
				}
				create_child_process_failed_dialog(win_data,
									  full_command,
									  NULL);
			}
			restore_SYSTEM_VTE_CJK_WIDTH_STR();
			g_string_free(environ_str, TRUE);
			g_strfreev(new_environs);
			break;
		}
		case 2:
		{
			confirm_to_execute = FALSE;
			gchar *new_command = g_strdup_printf("-u\t%s\t-e\t%s",
							     win_data->profile,
							     full_command);
			gchar **new_argv = split_string(new_command, "\t", -1);
			// g_debug("encoding = %s", encoding);
			gchar *locale = win_data->default_locale;
#ifdef SAFEMODE
			if (win_data->user_command[tag].locale &&
			    (win_data->user_command[tag].locale[0]!='\0'))
#else
			if (win_data->user_command[tag].locale[0]!='\0')
#endif
				locale = win_data->user_command[tag].locale;
			gchar *encoding = (gchar *)get_encoding_from_locale(locale);
			if (encoding == NULL)
			{
				if (check_and_add_locale_to_warned_locale_list(win_data, locale))
					create_invalid_locale_error_message(locale);
				locale = page_data->locale;
				encoding = g_strdup(page_data->encoding_str);
			}

			gchar *lang = get_lang_str_from_locale(locale, ".");
			gchar *language = get_language_str_from_locales(locale, win_data->default_locale);
			gchar *new_environs = g_strdup_printf("%s\tLANG=%s\tLANGUAGE=%s\tLC_ALL=%s",
							      win_data->user_command[tag].environ,
							      lang, language, locale);
			g_free(lang);
			g_free(language);
			// g_debug("new_environs = %s", new_environs);
			// gchar *pwd = get_current_pwd_by_pid(page_data->pid);
			gchar *pwd = get_init_dir(get_tpgid(page_data->pid), page_data->pwd, win_data->home);
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
			new_window(argc+3,
				   new_argv,
				   win_data->shell,
				   win_data->environment,
				   win_data->runtime_locale_list,
				   pwd,
				   win_data->home,
				   get_VTE_CJK_WIDTH_str(win_data->user_command[tag].VTE_CJK_WIDTH),
				   TRUE,
				   win_data->wmclass_name,
				   win_data->wmclass_class,
				   new_environs,
				   encoding,
				   TRUE,
				   locale,
				   NULL,
				   NULL);
			g_free(new_environs);
			g_free(encoding);
			g_free (new_command);
			g_strfreev(new_argv);
			g_free(pwd);
			confirm_to_execute = TRUE;
			break;
		}
		default:
#ifdef FATAL
			print_switch_out_of_range_error_dialog("vte_button_press",
							       "win_data->user_command[tag].method",
							       win_data->user_command[tag].method);
#endif
			break;
	}
	g_strfreev(argv);
	g_free(full_command);
	setlocale(LC_CTYPE, init_LC_CTYPE);
	return TRUE;
}

gboolean compare_win_page_encoding(GtkWidget *menu_item_encoding, gchar *encoding_str)
{
#ifdef DETAIL
	g_debug("! Launch compare_win_page_encoding() with menu_item_encoding = %p, encoding_str = %s",
		 menu_item_encoding, encoding_str);
#endif
	// FIXME: Should we return TRUE or FALSE here?
	if (menu_item_encoding==NULL || encoding_str==NULL)
		return FALSE;

	return compare_strings(gtk_widget_get_name (menu_item_encoding), encoding_str, FALSE);
}

// The returned string should be freed when no longer needed.
gchar *get_url(GdkEventButton *event, struct Page *page_data, gint *tag)
{
#ifdef DETAIL
	if (tag)
		g_debug("! Launch get_url() with page_data = %p, tag = %d", page_data, *tag);
	else
		g_debug("! Launch get_url() with page_data = %p", page_data);
#endif
#ifdef SAFEMODE
	if ((page_data==NULL) || (event==NULL)) return NULL;
#endif
	gint pad_x=0, pad_y=0;
	vte_terminal_get_padding(VTE_TERMINAL(page_data->vte), &pad_x, &pad_y);
	pad_x /= 2;
	pad_y /= 2;
	// g_debug("pad_x = %d, pad_y = %d", pad_x, pad_y);
	return vte_terminal_match_check(VTE_TERMINAL(page_data->vte),
					(event->x - pad_x) /
						vte_terminal_get_char_width(
							VTE_TERMINAL(page_data->vte)),
					(event->y - pad_y) /
						vte_terminal_get_char_height(
							VTE_TERMINAL(page_data->vte)),
					tag);
}

void page_data_dup(struct Page *page_data_prev, struct Page *page_data)
{
#ifdef DETAIL
	g_debug("! Launch page_data_dup() with page_data_prev = %p, page_data = %p", page_data_prev, page_data);
#endif
#ifdef SAFEMODE
	if ((page_data==NULL) || (page_data_prev==NULL)) return;
#endif
	memcpy( page_data, page_data_prev, sizeof(* page_data_prev));

	// page_data->window;
	// page_data->notebook;
	page_data->page_no = 0;

	page_data->label = NULL;
	page_data->label_text = NULL;
	page_data->label_button = NULL;
	page_data->hbox = NULL;
	page_data->vte = NULL;
	page_data->scroll_bar = NULL;

	page_data->encoding_str = g_strdup(page_data_prev->encoding_str);
	page_data->locale = g_strdup(page_data_prev->locale);
	page_data->environ = g_strdup(page_data_prev->environ);
	// g_debug("g_strdupv(page_data_prev->environments)!");
	// page_data->environments = g_strdupv(page_data_prev->environments);
	page_data->VTE_CJK_WIDTH_STR = g_strdup(page_data_prev->VTE_CJK_WIDTH_STR);

	page_data->page_name = NULL;
	page_data->pid = -1;
	page_data->pid_cmdline = NULL;
	page_data->current_tpgid = 0;
	page_data->displayed_tpgid = 0;
	page_data->page_name = NULL;
	page_data->custom_page_name = NULL;
	page_data->tab_color = NULL;

	// page_data->pwd = get_current_pwd_by_pid(page_data_prev->pid);
	page_data->pwd = NULL;
	page_data->is_root = 0;
	page_data->is_bold = 0;

	page_data->window_title_updated = -1;
	page_data->page_update_method = PAGE_METHOD_AUTOMATIC;
	page_data->window_title_signal = 0;

	page_data->timeout_id = 0;
	page_data->urgent_bell_handler_id = 0;

	page_data->font_name = g_strdup(page_data_prev->font_name);
	// page_data->font_size = 0;

	// page_data->check_root_privileges;
	// page_data->page_shows_window_title;
	// page_data->page_shows_current_dir;
	// page_data->page_shows_current_cmdline;
	// page_data->bold_action_page_name;
	// page_data->window_title_tpgid;
	// page_data->lost_focus;
	// page_data->keep_vte_size;
	// page_data->current_vte;
	page_data->window_title_pwd = NULL;
	// page_data->custom_window_title;

	gint i;
	for (i=0; i<COMMAND; i++)
		page_data->tag[i] = 0;
}

struct Page *get_page_data_from_nth_page(struct Window *win_data, guint page_no)
{
#ifdef DETAIL
	g_debug("! Launch get_page_data_from_nth_page() with win_data = %p, page_no = %d", win_data, page_no);
#endif
#ifdef SAFEMODE
	if ((win_data==NULL) || (win_data->notebook==NULL)) return NULL;
#endif
	GtkWidget *vte = (GtkWidget *)g_object_get_data(G_OBJECT(gtk_notebook_get_tab_label(
					GTK_NOTEBOOK(win_data->notebook),
						gtk_notebook_get_nth_page(
							GTK_NOTEBOOK(win_data->notebook),
									 page_no))),
							"VteBox");
	return (struct Page *)g_object_get_data(G_OBJECT(vte), "Page_Data");
}

// Get LANG str from old and new locale data
gchar *get_language_str_from_locales(const gchar *new_locale, const gchar *old_locale)
{
#ifdef DETAIL
	g_debug("! Launch get_language_str_from_locales() with old_locale = %s, new_locale = %s, init_LANGUAGE = %s",
		old_locale, new_locale, init_LANGUAGE);
#endif
	gint i, j;
	const gchar *locale_list[4] = {new_locale, old_locale, init_LANGUAGE, "en"};
	GString *language_list = g_string_new(":");

	// Join the locales into "zh_TW.UTF-8:ja_JP.UTF8:zh_CN:en"
	for (i=0; i<4; i++)
	{
		if (locale_list[i])
		{
			g_string_append_printf(language_list, "%s:", locale_list[i]);
		}
	}
	// g_debug("get_language_str_from_locales(): Get language_list = %s", language_list->str);

	// Convert "zh_TW.UTF-8:ja_JP.UTF8:zh_CN:en" into "zh_TW:zh:ja_JP:ja:zh_CN:zh:en"
	GString *final_lang_list = g_string_new(":");
	gchar **lang_lists = split_string(language_list->str, ":", -1);
	gchar *new_lang[2];
	i=-1;
	while (lang_lists[++i])
	{
		// g_debug("get_language_str_from_locales(1): Checking lang_lists[%d] = \"%s\"", i, lang_lists[i]);
		new_lang[0] = get_lang_str_from_locale(lang_lists[i], ".");
		if (new_lang[0])
		{
			g_string_append_printf(final_lang_list, "%s:", new_lang[0]);
			new_lang[1] = get_lang_str_from_locale(new_lang[0], "_");
			if (new_lang[1])
			{
				g_string_append_printf(final_lang_list, "%s:", new_lang[1]);
				g_free(new_lang[1]);
			}
			g_free(new_lang[0]);
		}
	}
	// g_debug("get_language_str_from_locales(): Get final_lang_list = %s", final_lang_list->str);
	g_strfreev(lang_lists);
	g_string_free(language_list, TRUE);

	// Join "zh_TW:zh:ja_JP:ja:zh_CN:zh:en" into "zh_TW:zh:ja_JP:ja:zh_CN:en"
	GString *join_lang_list = g_string_new(NULL);
	lang_lists = split_string(final_lang_list->str, ":", -1);
	i=-1;
	while (lang_lists[++i])
	{
		// g_debug("get_language_str_from_locales(2): Checking lang_lists[%d] = \"%s\"", i, lang_lists[i]);
		gboolean different_strings = TRUE;
		if (lang_lists[i][0] == '\0')
			different_strings = FALSE;
		else
		{
			for (j=0; j<i; j++)
			{
				different_strings = compare_strings(lang_lists[i], lang_lists[j], TRUE);
				// g_debug("get_language_str_from_locales(2): Compare \"%s\" and \"%s\": %d",
				//	lang_lists[i], lang_lists[j], different_strings);
				if (different_strings==FALSE) break;
			}
		}
		if (different_strings)
		{
			if (join_lang_list->str[0] != '\0')
				g_string_append_printf(join_lang_list, ":%s", lang_lists[i]);
			else
				g_string_append_printf(join_lang_list, "%s", lang_lists[i]);
		}
		// g_debug("get_language_str_from_locales(2): join_lang_list = %s", join_lang_list->str);
	}
	// g_debug("get_language_str_from_locales(): Get join_lang_list = %s", join_lang_list->str);
	g_strfreev(lang_lists);
	g_string_free(final_lang_list, TRUE);


	return g_string_free(join_lang_list, FALSE);
}

// convert zh_TW.UTF-8 -> zh_TW
// See https://www.gnu.org/software/gettext/manual/html_node/The-LANGUAGE-variable.html for more details.
gchar *get_lang_str_from_locale(const gchar *locale, const gchar *split)
{
#ifdef DETAIL
	g_debug("! Launch get_lang_str_from_locale() with locale = %s", locale);
#endif
#ifdef SAFEMODE
	if (locale==NULL) return NULL;
#endif
	gchar *language = NULL;
	gchar **split_locales = split_string(locale, split, 2);

	if (split_locales)
		language=g_strdup(split_locales[0]);
	else
		language=g_strdup(locale);

	// print_array("! get_lang_str_from_locale() locale", split_locales);

	g_strfreev(split_locales);
	return language;
}
