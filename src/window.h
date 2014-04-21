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
#include <vte/vte.h>
// for L10n
#include <locale.h>
#include <glib/gi18n.h>
// for exit()
#include <stdlib.h>
// for strcmp()
#include <string.h>
// for chdir()
#include <unistd.h>
// for opendir() readdir() closedir()
#include <dirent.h>
// for getpwuid()
#include <pwd.h>
// for g_chdir()
#include <glib/gstdio.h>
// for GDK_WINDOW_XID
// #include <gdk/gdkx.h>

#include "lilyterm.h"

#ifdef USE_XPARSEGEOMETRY
	// for XParseGeometry()
	#include <X11/Xlib.h>
#endif

gboolean window_option(struct Window *win_data, gchar *encoding, int argc, char *argv[]);
char **set_process_data (pid_t entry_pid, gint *ppid, StrAddr **cmd);
gboolean window_key_press(GtkWidget *widget, GdkEventKey *event, struct Window *win_data);
gboolean window_key_release(GtkWidget *widget, GdkEventKey *event, struct Window *win_data);
void window_style_set(GtkWidget *window, GtkStyle *previous_style, struct Window *win_data);
#if defined(USE_GTK2_GEOMETRY_METHOD) || defined(UNIT_TEST)
void window_size_request(GtkWidget *window, GtkRequisition *requisition, struct Window *win_data);
gboolean window_state_event(GtkWidget *widget, GdkEventWindowState *event, struct Window *win_data);
#endif
#if defined(USE_GTK3_GEOMETRY_METHOD) || defined(UNIT_TEST)
gboolean idle_show_or_hide_tabs_bar_and_scroll_bar(struct Window *win_data);
gboolean idle_gtk_window_fullscreen(struct Window *win_data);
void resize_to_exist_widget(struct Window *win_data);
void save_vte_geometry(struct Window *win_data);
gboolean idle_to_resize_window(struct Window *win_data);
gboolean idle_hide_and_show_tabs_bar(struct Window *win_data);
#endif
void window_size_allocate(GtkWidget *window, GtkAllocation *allocation, struct Window *win_data);
gboolean window_get_focus(GtkWidget *window, GdkEventFocus *event, struct Window *win_data);
gboolean window_lost_focus(GtkWidget *window, GdkEventFocus *event, struct Window *win_data);
#ifdef ENABLE_PAGE_ADDED
void notebook_page_added(GtkNotebook *notebook, GtkWidget *child, guint page_num, struct Window *win_data);
#endif
// void notebook_page_removed (GtkNotebook *notebook, GtkWidget *child, guint page_num, struct Window *win_data);
void reorder_page_after_added_removed_page(struct Window *win_data, guint page_num);
void destroy_window(struct Window *win_data);
GtkNotebook *create_window(GtkNotebook *notebook, GtkWidget *page, gint x, gint y,
			    struct Window *win_data);
#ifdef FATAL
void dump_data(struct Window *win_data, struct Page *page_data);
#endif
void win_data_dup(struct Window *win_data_orig, struct Window *win_data);
gboolean get_hide_or_show_tabs_bar(struct Window *win_data, Switch_Type show_tabs_bar);
gboolean fullscreen_show_hide_scroll_bar (struct Window *win_data);
