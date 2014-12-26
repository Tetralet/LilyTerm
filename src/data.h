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

#ifndef DATA_H
#define DATA_H

#include <gtk/gtk.h>
#include <vte/vte.h>

//
// **************************** data.h ****************************
//

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef REALMODE
	#undef SAFEMODE
#else
	#ifndef SAFEMODE
		#define SAFEMODE
	#endif
#endif

#ifdef OUT_OF_MEMORY
// GString *g_string_append()
// GString *g_string_prepend()
// 	#define g_dir_read_name(x) NULL
	#define g_get_tmp_dir() NULL
// 	#define g_get_user_config_dir() NULL
	#define g_get_user_name() NULL
	#define g_getenv(x) NULL
/* 	#define g_new0(x,y) NULL */
	#define g_strerror(x) NULL
	#define gtk_entry_get_text(x) NULL
	#define gtk_widget_get_name(x) NULL
	#define gtk_file_chooser_get_preview_filename(x) NULL
	#define pango_font_description_to_string(x) NULL
	#define vte_terminal_match_check(a,b,c,d) NULL
	#define vte_terminal_get_encoding(x) NULL
	#define vte_terminal_get_window_title(x) NULL
/* 	#define g_strsplit(x,y,z) NULL */
	#define g_strsplit fake_g_strsplit
	#define g_strsplit_set(x,y,z) NULL
	#define g_convert_with_fallback(a,b,c,d,e,f,g,h) NULL
	#define g_file_read_link(x,y) NULL
	#define g_get_current_dir() NULL
	#define g_key_file_get_value(a,b,c,d) NULL
	#define g_markup_escape_text(x,y) NULL
	#define g_path_get_basename(x) NULL
	#define g_path_get_dirname(x) NULL
	#define g_strconcat(...) NULL
/* 	#define g_strdup(x) NULL */
	#define g_strdup fake_g_strdup
/* 	#define g_strdup_printf fake_g_strdup_printf */
	#define g_strdup_printf(...) NULL
	#define gdk_color_to_string(x) NULL
	#define gdk_keyval_name(x) NULL
	#define gtk_clipboard_wait_for_text(x) NULL
	#define gtk_file_chooser_get_filename(x) NULL
	#define gtk_font_selection_dialog_get_font_name(x) NULL
	#define gtk_tree_path_to_string(x) NULL
#endif

//	define get_pid_stat(x,y) NULL
//	define split_string(x,y,z) NULL
//	define check_string_value(a, b, c, d) NULL
//	define convert_array_to_string(x,y) NULL
//	define convert_str_to_utf8(x,y) NULL
//	define convert_text_to_html(a, b, c, d, ...) NULL
//	define dialog_key_press_join_string(x,y,z) NULL
//	define get_VTE_CJK_WIDTH_str(x) NULL
//	define get_cmdline(x) NULL
//	define get_colorful_profile(x) NULL
//	define get_current_pwd_by_pid(x) NULL
//	define get_default_lc_data(x) NULL
//	define get_encoding_from_locale(x) NULL
//	define get_help_message(x) NULL
//	define get_help_message_key_binding(x) NULL
//	define get_help_message_usage(x,y) NULL
//	define get_locale_list(x) NULL
//	define get_proc_data(x,y,z) NULL
//	define get_profile(x) NULL
//	define get_resize_font(x,y) NULL
//	define get_tab_name_with_cmdline(x) NULL
//	define get_tab_name_with_current_dir(x) NULL
//	define get_tab_name_with_page_names(x) NULL
//	define get_url (x,y,z) NULL
//	define get_user_profile_path(x,y,z) NULL
//	define join_strings_to_string(x,y,z,...) NULL
//	define load_profile_from_dir(x,y) NULL

//
// **************************** data.h ****************************
//

// for key value
#include <gdk/gdkkeysyms.h>

#define PROFILE BINARY ".conf"
#define SYS_PROFILE ETCDIR "/" PROFILE
#define USER_PROFILE "default.conf"
#define SOCKET_DATA_VERSION PACKAGE "-0.9.9~rc6"
#define PROFILE_FORMAT_VERSION "0.9.9.3"
#define NOTEBOOK_GROUP 8
#define ICON_PATH ICONDIR G_DIR_SEPARATOR_S BINARY ".png"
#define NULL_DEVICE "/dev/null"
#define SYSTEM_FONT_NAME "Monospace 12"
#define SYSTEM_COLUMN 80
#define SYSTEM_ROW 24
#define LANGUAGE 255

#if GTK_CHECK_VERSION(2,9,0)
	// SINCE: gtk+-2.9.0/gtk/gtknotebook.h: gtk_notebook_set_tab_reorderable()
	#define ENABLE_TAB_REORDER
	#define ENABLE_PAGE_ADDED
	// SINCE: gtk+-2.9.0/gtk/gtknotebook.h: gtk_notebook_set_group_id()
	// END:   gtk+-2.24.5/gtk/gtknotebook.h: gtk_notebook_set_group_id()
	#define ENABLE_DRAG_AND_DROP
#else
	#define DISABLE_TAB_REORDER
	#define DISABLE_PAGE_ADDED
  #ifndef GDK_SUPER_MASK
	// SINCE: gtk+-2.9.0/gdk/gdktypes.h: GDK_SUPER_MASK = 1 << 26
	#define GDK_SUPER_MASK 1<<26
  #endif
#endif
#if GTK_CHECK_VERSION(2,11,0)
	// SINCE: gtk+-2.11.0/gtk/gtkwindow.h: gtk_window_set_opacity()
	#define ENABLE_RGBA
	// SINCE: gtk+-2.11.0/gdk/gdkcolor.h: gdk_color_to_string()
	#define ENABLE_GDKCOLOR_TO_STRING
	// SINCE: gtk+-2.11.0/gdk/gdk.h: gdk_notify_startup_complete_with_id()
	#define ENABLE_X_STARTUP_NOTIFICATION_ID
#endif
#if GTK_CHECK_VERSION(2,11,3)
	// SINCE: gtk+-2.11.3/gtk/gtkwidget.h: gtk_widget_set_tooltip_text()
	#define ENABLE_SET_TOOLTIP_TEXT
#endif
#if GTK_CHECK_VERSION(2,13,0)
	// SINCE: gtk+-2.13.0/gtk/gtktestutils.h: gtk_test_widget_click()
	#define ENABLE_MOUSE_SCROLL
#else
	// SINCE: gtk+-2.13.0/gtk/gtkstock.h: #define GTK_STOCK_PAGE_SETUP "gtk-page-setup"
	#define GTK_FAKE_STOCK_PAGE_SETUP GTK_STOCK_INDEX
#endif
#if GTK_CHECK_VERSION(2,13,4)
	// SINCE: gtk+-2.13.4/gtk/gtkwidget.h: gtk_widget_get_allocation()
	#define USE_GTK_ALLOCATION
#else
	// SINCE: gtk+-2.13.4/gtk/gtkdialog.h: gtk_dialog_get_content_area()
	#define gtk_dialog_get_content_area(x) x->vbox
	// SINCE: gtk+-2.13.4/gtk/gtkdialog.h: gtk_dialog_get_action_area()
	#define gtk_dialog_get_action_area(x) x->action_area
	// SINCE: gtk+-2.13.4/gtk/gtkwidget.h: gtk_widget_get_window
	#define gtk_widget_get_window(x) x->window
#endif
#if ! GTK_CHECK_VERSION(2,15,0)
	// SINCE: gtk+-2.15.0/gtk/gtkmenuitem.h: gtk_menu_item_set_label()
	#define gtk_menu_item_set_label(x,y) gtk_label_set_text(GTK_LABEL(gtk_bin_get_child(GTK_BIN(x))), y)
#endif
#if ! GTK_CHECK_VERSION(2,17,5)
	// SINCE: gtk+-2.17.5/gtk/gtkwidget.h: gtk_widget_set_can_focus()
	#define gtk_widget_set_can_focus(x,FALSE) GTK_WIDGET_UNSET_FLAGS(x,GTK_CAN_FOCUS)
#  ifdef UNIT_TEST
	#define gtk_widget_get_visible gtk_widget_is_focus
	#define gtk_widget_get_allocation(x,y) gtk_widget_is_focus(x)
#  endif
#endif
#if ! GTK_CHECK_VERSION(2,19,5)
	// SINCE: gtk+-2.19.5/gtk/gtkwidget.h: gtk_widget_get_mapped()
	#define gtk_widget_get_mapped(x) GTK_WIDGET_MAPPED(x)
#endif
#if ! GTK_CHECK_VERSION(2,90,4)
	// END: gtk+-2.90.3/gtk/gtknotebook.h: gtk_notebook_set_tab_label_packing()
	#define USE_OLD_GTK_LABEL_PACKING
#endif
#if ! GTK_CHECK_VERSION(2,22,0)
	// SINCE: gtk+-2.22.0/gdk/gdkkeysyms.h: #define GDK_KEY_VoidSymbol 0xffffff
	#define GDK_KEY_VoidSymbol	GDK_VoidSymbol
	#define GDK_KEY_asciitilde	GDK_asciitilde
	#define GDK_KEY_Shift_L		GDK_Shift_L
	#define GDK_KEY_Hyper_R		GDK_Hyper_R
	#define GDK_KEY_F1		GDK_F1
	#define GDK_KEY_F12		GDK_F12
	#define GDK_KEY_Menu		GDK_Menu
	#define GDK_KEY_Super_R		GDK_Super_R
	#define GDK_KEY_Super_L		GDK_Super_L
	#define GDK_KEY_a		GDK_a
	#define GDK_KEY_z		GDK_z
	#define GDK_KEY_A		GDK_A
	#define GDK_KEY_Z		GDK_Z
#endif
#if ! GTK_CHECK_VERSION(2,23,0)
	// END: gtk+-2.23.0/gtk/gtkmain.h: gtk_quit_add() GTK_DISABLE_DEPRECATED
	#define g_atexit(x) gtk_quit_add(0, (GtkFunction)x, NULL)
#endif
#if GTK_CHECK_VERSION(2,3,2) && ( ! GTK_CHECK_VERSION(2,91,6))
	// SINCE: gtk+-2.3.2/gdk/gdkspawn.h: gdk_spawn_on_screen_with_pipes()
	// END:   gtk+-2.91.6/gdk/gdkspawn.h: gdk_spawn_on_screen_with_pipes()
	#define g_spawn_async_with_pipes(a,b,c,d,e,f,g,h,i,j,k) gdk_spawn_on_screen_with_pipes(gdk_screen_get_default(),a,b,c,d,e,f,g,h,i,j,k)
#endif
#if GTK_CHECK_VERSION(2,91,0)
	// SINCE: gtk+-2.91.0/gtk/gtkwidget.h GtkAlign gtk_widget_set_halign()
	#define USE_GTK_WIDGET_SET_HALIGN
#endif
#if ! GTK_CHECK_VERSION(2,90,6)
	// END: gtk+-2.90.6/gtk/gtkdialog.h: GTK_DIALOG_NO_SEPARATOR = 1 << 2
	#define EXIST_GTK_DIALOG_NO_SEPARATOR
#endif
#if GTK_CHECK_VERSION(2,90,7)
	// SINCE: gtk+-2.7.0/gdk/gdkscreen.h: gdk_screen_get_rgba_colormap()
	// END:   gtk+-2.90.7/gdk/gdkscreen.h: gdk_screen_get_rgba_colormap()
	// SINCE: gtk+-2.7.0/gdk/gdkscreen.h: gdk_screen_get_rgba_visual()
	#define USE_GDK_SCREEN_GET_RGBA_VISUAL
#else
	// SINCE: gtk+-2.9.0/gtk/gtknotebook.h: gtk_notebook_set_group()
	// END:   gtk+-2.90.7/gtk/gtknotebook.h: gtk_notebook_set_group()
	// SINCE: gtk+-2.23.0/gtk/gtknotebook.h: gtk_notebook_set_group_name()
	#define gtk_notebook_set_group_name(x,y) gtk_notebook_set_group(x,GINT_TO_POINTER (NOTEBOOK_GROUP))
#endif
#if GTK_CHECK_VERSION(2,91,0)
	// SINCE: gtk+-2.91.0/gtk/gtkwidget.h: gtk_widget_get_preferred_size()
	#define gtk_widget_get_child_requisition(x,y) gtk_widget_get_preferred_size(x,y,NULL)
#endif
#if GTK_CHECK_VERSION(2,91,1)
	// SINCE: gtk+-2.91.1/gtk/gtkwindow.h: gtk_window_set_has_resize_grip()
	// END: gtk+-3.13.4/gtk/gtkwindow.h: GDK_DEPRECATED_IN_3_14 void gtk_window_set_has_resize_grip()
#  if ! GTK_CHECK_VERSION(3,13,4)
		#define NO_RESIZE_GRIP
#  endif
	// FIXME: gtk_window_parse_geometry() works well in all GTK+2/3 versions?
	// #define USE_XPARSEGEOMETRY
	// END: gtk+-2.91.1/gtk/gtkwidget.h: gtk_widget_hide_all()
	#define gtk_widget_hide_all(x) gtk_widget_hide(x)
#endif
#if GTK_CHECK_VERSION(2,91,5)
	#define USE_GTK3_GEOMETRY_METHOD
#else
	#define USE_GTK2_GEOMETRY_METHOD
#  ifdef UNIT_TEST
	#define gtk_window_resize_to_geometry gtk_window_move
#  endif
#endif
#if GTK_CHECK_VERSION(2,91,7)
	#define GTK3_LAME_GDK_SCREEN_IS_COMPOSITED
	#define GTK3_LAME_GDK_SCREEN_GET_RGBA_VISUAL
	// SINCE: gtk+-2.91.7/gdk/gdkmain.h: void gdk_disable_multidevice ()
	// Need gdk_disable_multidevice() be called to get gtk_test_widget_click() working.
	#define SCROLL_LINE 5
#endif
#if GTK_CHECK_VERSION(3,1,12)
	// SINCE: gtk+-3.1.12/gtk/gtkfontchooser.h: gtk_font_chooser_set_filter_func()
	// DISABLE: it filtered too many Chinese fonts.
	// #define EXIST_GTK_FONT_CHOOSER_SET_FILTER_FUNC
#endif
#if GTK_CHECK_VERSION(3,3,4)
	// END: gtk+-3.3.4/gtk/deprecated/gtktable.h: gtk_table_attach_defaults()
	#define USE_GTK_GRID
	// END: gtk+-3.3.4/gtk/deprecated/gtkvscrollbar.h: gtk_vscrollbar_new()
	#define gtk_vscrollbar_new(x) gtk_scrollbar_new(GTK_ORIENTATION_VERTICAL,x)
	// END: gtk+-3.3.4/gtk/deprecated/gtkhscale.h: gtk_hscale_new_with_range()
	#define gtk_hscale_new_with_range(x,y,z) gtk_scale_new_with_range (GTK_ORIENTATION_HORIZONTAL,x,y,z)
#else
	// END: gtk+-3.3.4/gtk/deprecated/gtkstyle.h: gtk_widget_modify_style()
	#define USING_OLD_GTK_RC_STYLE_NEW
	// END: gtk+-3.3.4/gtk/deprecated/gtkfontsel.h: gtk_font_selection_dialog_new()
	#define GTK_FONT_CHOOSER GTK_FONT_SELECTION_DIALOG
	#define gtk_font_chooser_dialog_new(x,y) gtk_font_selection_dialog_new(x)
	#define gtk_font_chooser_get_font gtk_font_selection_dialog_get_font_name
	#define gtk_font_chooser_set_font gtk_font_selection_dialog_set_font_name
#  ifdef UNIT_TEST
	#define gtk_font_chooser_set_filter_func(a,b,c,d) NULL
#  endif
#endif
// FIXME: The replacement of GtkColorSelection in GTK3, GtkColorChooser, is a lame... orz
#if GTK_CHECK_VERSION(3,3,18)
	// SINCE: gtk+-3.3.18/gtk/gtkcolorchooserwidget.h: gtk_color_chooser_widget_new()
	#define USE_GTK_COLOR_CHOOSER
#else
	// END: gtk+-3.3.18/gtk/deprecated/gtkcolorsel.h: gtk_color_selection_new()
	#define USE_OLD_GTK_COLOR_SELECTION
	#define GTK_COLOR_CHOOSER GTK_COLOR_SELECTION
	#define GtkColorChooser GtkColorSelection
	#define gtk_color_chooser_get_rgba gtk_color_selection_get_current_color
	#define adjust_vte_color(x,y,z) adjust_vte_color(x,z)
#endif
#if GTK_CHECK_VERSION(3,8,0)
	// END: gtk+-3.8.0/gtk/gtkwindow.h: GDK_DEPRECATED_IN_3_8_FOR(gtk_widget_set_opacity)
	#define gtk_window_set_opacity(x,y) gtk_widget_set_opacity(GTK_WIDGET(x),y)
#endif
#if GTK_CHECK_VERSION(3,9,0)
	// END: gtk+-3.9.0/gdk/gdkmain.h: GDK_DEPRECATED_IN_3_8_FOR(gdk_display_get_name(gdk_display_get_default())) gdk_get_display()
	#define gdk_get_display() g_strdup(gdk_display_get_name(gdk_display_get_default()))
#endif
#if GTK_CHECK_VERSION(3,9,8)
	// END: gtk+-3.9.8/gtk/gtkbutton.h: GDK_DEPRECATED_IN_3_10_FOR(gtk_button_new_with_label) gtk_button_new_from_stock()
	#define gtk_button_new_from_stock(x) gtk_button_new_from_icon_name(x,GTK_ICON_SIZE_BUTTON)
	// END: gtk+-3.9.8/gtk/gtkimage.h: GDK_DEPRECATED_IN_3_10_FOR(gtk_image_new_from_icon_name) gtk_image_new_from_stock()
	#define gtk_image_new_from_stock gtk_image_new_from_icon_name
	// END: gtk+-3.9.8/gtk/gtkimagemenuitem.h: GDK_DEPRECATED_IN_3_10_FOR(gtk_menu_item_new_with_label) gtk_image_menu_item_new_with_label()
	#define gtk_image_menu_item_new_with_label gtk_menu_item_new_with_label
	// END: gtk+-3.9.8/gtk/gtkimagemenuitem.h: GDK_DEPRECATED_IN_3_10 gtk_image_menu_item_set_image()
	#define gtk_image_menu_item_set_image(a,b) NULL
#endif
#if GTK_CHECK_VERSION(3,9,10)
	#define	 GTK_FAKE_STOCK_ABOUT			"gtk-about"
	#define	 GTK_FAKE_STOCK_ADD			"gtk-add"
	#define	 GTK_FAKE_STOCK_APPLY			"gtk-apply"
	#define	 GTK_FAKE_STOCK_CANCEL			"gtk-cancel"
	#define	 GTK_FAKE_STOCK_CLEAR			"gtk-clear"
	#define	 GTK_FAKE_STOCK_CLOSE			"gtk-close"
	#define	 GTK_FAKE_STOCK_CONVERT			"gtk-convert"
	#define	 GTK_FAKE_STOCK_COPY			"gtk-copy"
	#define	 GTK_FAKE_STOCK_DIALOG_AUTHENTICATION	"gtk-dialog-authentication"
	#define	 GTK_FAKE_STOCK_DIALOG_INFO		"gtk-dialog-info"
	#define	 GTK_FAKE_STOCK_DIALOG_WARNING		"gtk-dialog-warning"
	#define	 GTK_FAKE_STOCK_DIALOG_ERROR		"gtk-dialog-error"
	#define	 GTK_FAKE_STOCK_DIALOG_QUESTION		"gtk-dialog-question"
	#define	 GTK_FAKE_STOCK_DND			"gtk-dnd"
	#define	 GTK_FAKE_STOCK_DND_MULTIPLE		"gtk-dnd-multiple"
	#define	 GTK_FAKE_STOCK_EDIT			"gtk-edit"
	#define	 GTK_FAKE_STOCK_EXECUTE			"gtk-execute"
	#define	 GTK_FAKE_STOCK_FILE			"gtk-file"
	#define	 GTK_FAKE_STOCK_FIND			"gtk-find"
	#define	 GTK_FAKE_STOCK_GOTO_BOTTOM		"gtk-goto-bottom"
	#define	 GTK_FAKE_STOCK_GOTO_TOP		"gtk-goto-top"
	#define	 GTK_FAKE_STOCK_GO_BACK			"gtk-go-back"
	#define	 GTK_FAKE_STOCK_GO_DOWN			"gtk-go-down"
	#define	 GTK_FAKE_STOCK_GO_UP			"gtk-go-up"
	#define	 GTK_FAKE_STOCK_HELP			"gtk-help"
	#define	 GTK_FAKE_STOCK_INDEX			"gtk-index"
	#define	 GTK_FAKE_STOCK_NEW			"gtk-new"
	#define	 GTK_FAKE_STOCK_OK			"gtk-ok"
	#define	 GTK_FAKE_STOCK_OPEN			"gtk-open"
	#define	 GTK_FAKE_STOCK_PAGE_SETUP		"gtk-page-setup"
	#define	 GTK_FAKE_STOCK_PASTE			"gtk-paste"
	#define	 GTK_FAKE_STOCK_PREFERENCES		"gtk-preferences"
	#define	 GTK_FAKE_STOCK_PROPERTIES		"gtk-properties"
	#define	 GTK_FAKE_STOCK_QUIT			"gtk-quit"
	#define	 GTK_FAKE_STOCK_REFRESH			"gtk-refresh"
	#define	 GTK_FAKE_STOCK_REVERT_TO_SAVED		"gtk-revert-to-saved"
	#define	 GTK_FAKE_STOCK_SAVE			"gtk-save"
	#define	 GTK_FAKE_STOCK_SAVE_AS			"gtk-save-as"
	#define	 GTK_FAKE_STOCK_SELECT_COLOR		"gtk-select-color"
	#define	 GTK_FAKE_STOCK_SELECT_FONT		"gtk-select-font"
	#define	 GTK_FAKE_STOCK_ZOOM_100		"gtk-zoom-100"
	#define	 GTK_FAKE_STOCK_ZOOM_IN			"gtk-zoom-in"
	#define	 GTK_FAKE_STOCK_ZOOM_OUT		"gtk-zoom-out"
#else
	#define GTK_FAKE_STOCK_ABOUT			GTK_STOCK_ABOUT
	#define GTK_FAKE_STOCK_ADD			GTK_STOCK_ADD
	#define GTK_FAKE_STOCK_APPLY			GTK_STOCK_APPLY
	#define GTK_FAKE_STOCK_CANCEL			GTK_STOCK_CANCEL
	#define GTK_FAKE_STOCK_CLEAR			GTK_STOCK_CLEAR
	#define GTK_FAKE_STOCK_CLOSE			GTK_STOCK_CLOSE
	#define GTK_FAKE_STOCK_CONVERT			GTK_STOCK_CONVERT
	#define GTK_FAKE_STOCK_COPY			GTK_STOCK_COPY
	#define GTK_FAKE_STOCK_DIALOG_AUTHENTICATION	GTK_STOCK_DIALOG_AUTHENTICATION
	#define GTK_FAKE_STOCK_DIALOG_ERROR		GTK_STOCK_DIALOG_ERROR
	#define GTK_FAKE_STOCK_DIALOG_INFO		GTK_STOCK_DIALOG_INFO
	#define GTK_FAKE_STOCK_DIALOG_QUESTION		GTK_STOCK_DIALOG_QUESTION
	#define GTK_FAKE_STOCK_DIALOG_WARNING		GTK_STOCK_DIALOG_WARNING
	#define GTK_FAKE_STOCK_DND			GTK_STOCK_DND
	#define GTK_FAKE_STOCK_DND_MULTIPLE		GTK_STOCK_DND_MULTIPLE
	#define GTK_FAKE_STOCK_EDIT			GTK_STOCK_EDIT
	#define GTK_FAKE_STOCK_EXECUTE			GTK_STOCK_EXECUTE
	#define GTK_FAKE_STOCK_FILE			GTK_STOCK_FILE
	#define GTK_FAKE_STOCK_FIND			GTK_STOCK_FIND
	#define GTK_FAKE_STOCK_GO_BACK			GTK_STOCK_GO_BACK
	#define GTK_FAKE_STOCK_GO_DOWN			GTK_STOCK_GO_DOWN
	#define GTK_FAKE_STOCK_GOTO_BOTTOM		GTK_STOCK_GOTO_BOTTOM
	#define GTK_FAKE_STOCK_GOTO_TOP			GTK_STOCK_GOTO_TOP
	#define GTK_FAKE_STOCK_GO_UP			GTK_STOCK_GO_UP
	#define GTK_FAKE_STOCK_HELP			GTK_STOCK_HELP
	#define GTK_FAKE_STOCK_INDEX			GTK_STOCK_INDEX
	#define GTK_FAKE_STOCK_NEW			GTK_STOCK_NEW
	#define GTK_FAKE_STOCK_OK			GTK_STOCK_OK
	#define GTK_FAKE_STOCK_OPEN			GTK_STOCK_OPEN
#ifndef GTK_FAKE_STOCK_PAGE_SETUP
	#define GTK_FAKE_STOCK_PAGE_SETUP		GTK_STOCK_PAGE_SETUP
#endif
	#define GTK_FAKE_STOCK_PASTE			GTK_STOCK_PASTE
	#define GTK_FAKE_STOCK_PREFERENCES		GTK_STOCK_PREFERENCES
	#define GTK_FAKE_STOCK_PROPERTIES		GTK_STOCK_PROPERTIES
	#define GTK_FAKE_STOCK_QUIT			GTK_STOCK_QUIT
	#define GTK_FAKE_STOCK_REFRESH			GTK_STOCK_REFRESH
	#define GTK_FAKE_STOCK_REVERT_TO_SAVED		GTK_STOCK_REVERT_TO_SAVED
	#define GTK_FAKE_STOCK_SAVE			GTK_STOCK_SAVE
	#define GTK_FAKE_STOCK_SAVE_AS			GTK_STOCK_SAVE_AS
	#define GTK_FAKE_STOCK_SELECT_COLOR		GTK_STOCK_SELECT_COLOR
	#define GTK_FAKE_STOCK_SELECT_FONT		GTK_STOCK_SELECT_FONT
	#define GTK_FAKE_STOCK_ZOOM_100			GTK_STOCK_ZOOM_100
	#define GTK_FAKE_STOCK_ZOOM_IN			GTK_STOCK_ZOOM_IN
	#define GTK_FAKE_STOCK_ZOOM_OUT			GTK_STOCK_ZOOM_OUT
#endif
#if GTK_CHECK_VERSION(3,11,5)
	// END: gtk+-3.11.5/gtk/gtkdialog.h: GDK_DEPRECATED_IN_3_10 gtk_alternative_dialog_button_order()
	#define USE_GTK_ALT_DIALOG_BUTTON_ORDER
	#define gtk_alternative_dialog_button_order(x) gtk_alt_dialog_button_order()
#else
	// END: gtk+-3.11.5/gtk/gtkdialog.h: GDK_DEPRECATED_IN_3_10 gtk_dialog_get_action_area()
	#define HAVE_GTK_DIALOG_GET_ACTION_AREA
#endif
#if ! GLIB_CHECK_VERSION(2,13,0)
	// SINCE: glib-2.13.0/glib/gmain.h: g_timeout_add_seconds()
	#define g_timeout_add_seconds(x,y,z) g_timeout_add(x*1000,y,z);
#endif
#if GLIB_CHECK_VERSION(2,31,2)
	// END: glib-2.31.2/glib/gutils.h: g_atexit () GLIB_DEPRECATED
	#define g_atexit atexit
#endif
#if ! GLIB_CHECK_VERSION(2,31,18)
	// SINCE: glib-2.31.18/glib/gmacros.h
	#define G_GNUC_BEGIN_IGNORE_DEPRECATIONS
	#define G_GNUC_END_IGNORE_DEPRECATIONS
#endif

#ifndef VTE_CHECK_VERSION
#define VTE_CHECK_VERSION(major,minor,micro) \
  (0 > (major) || (0 == (major) && 0 > (minor)) || (0 == (major) && 0 == (minor) && 0 >= (micro)))
#endif
#if VTE_CHECK_VERSION(0,15,3)
	// SINCE: vte-0.15.3/src/vte.h: vte_terminal_select_all()
	#define ENABLE_VTE_SELECT_ALL
#endif
#if VTE_CHECK_VERSION(0,17,1)
	// SINCE: vte-0.17.1/src/vte.h: vte_terminal_set_cursor_blink_mode()
	#define USE_NEW_VTE_CURSOR_BLINKS_MODE
	// SINCE: vte-0.17.1/src/vte.h: vte_terminal_match_add_gregex()
	#define USE_NEW_VTE_MATCH_ADD_GREGEX
#endif
#if VTE_CHECK_VERSION(0,19,1) && ! VTE_CHECK_VERSION(0,38,0)
	// SINCE: vte-0.19.1/src/vte.h: vte_terminal_set_cursor_shape()
	#define ENABLE_CURSOR_SHAPE
	// SINCE: vte-0.19.1/src/vteseq.c: g_signal_emit_by_name(terminal, "beep");
	// END: vte-0.37.90/src/vteseq.c: g_signal_emit_by_name(terminal, "beep");
	#define ENABLE_BEEP_SINGAL
#endif
#if VTE_CHECK_VERSION(0,20,4)
	// SINCE: vte-0.20.4/src/vte.h: VTE_ERASE_TTY
	#define ENABLE_VTE_ERASE_TTY
#endif
#if VTE_CHECK_VERSION(0,22,3)
	#define SCROLL_HISTORY -1
#else
	#define SCROLL_HISTORY 1024
#endif
#if VTE_CHECK_VERSION(0,24,0)
	// END: vte-0.24.0/src/vte.h: vte_terminal_get_padding() G_GNUC_DEPRECATED
	#define vte_terminal_get_padding fake_vte_terminal_get_padding
#endif
#if VTE_CHECK_VERSION(0,25,1)
	// SINCE: vte-0.25.1/src/vte.h: vte_terminal_search_set_gregex()
	#define ENABLE_FIND_STRING
	// END: vte-0.25.1/src/vtedeprecated.h: void vte_terminal_set_font_from_string_full()
	#define vte_terminal_set_font_from_string_full(x,y,z) vte_terminal_set_font_from_string(x,y)
#else
	// END: vte-0.25.1/src/vtedeprecated.h: vte_terminal_fork_command()
	#define USE_OLD_VTE_FORK_COMMAND
#endif
#if VTE_CHECK_VERSION(0,27,90) && GTK_CHECK_VERSION(2,91,2)
	// SINCE: vte-0.27.90/src/vte.h: void vte_terminal_set_colors_rgba()
	// SINCE: gtk+-2.91.2/gdk/gdktypes.h: typedef struct _GdkRGBA GdkRGBA;
	#define USE_GDK_RGBA
	#define MAX_COLOR 1
#else
	#define GdkRGBA GdkColor
	#define gdk_rgba_parse(x,y) gdk_color_parse(y,x)
	#define gtk_color_selection_set_previous_rgba gtk_color_selection_set_previous_color
	#define gtk_color_selection_set_current_rgba gtk_color_selection_set_current_color
	#define vte_terminal_set_colors_rgba vte_terminal_set_colors
	#define vte_terminal_set_color_foreground_rgba vte_terminal_set_color_foreground
	#define vte_terminal_set_color_background_rgba vte_terminal_set_color_background
	#define vte_terminal_set_color_bold_rgba vte_terminal_set_color_bold
	#define vte_terminal_set_color_cursor_rgba vte_terminal_set_color_cursor
	#define MAX_COLOR 0xFFFF
#endif
#if ! VTE_CHECK_VERSION(0,34,6)
	// END: vte-0.34.6/src/vtedeprecated.h: void vte_terminal_im_append_menuitems()
	#define ENABLE_IM_APPEND_MENUITEMS
#endif
#if VTE_CHECK_VERSION(0,34,8)
#  ifdef UNIT_TEST
	#define USE_FAKE_FUNCTIONS
	#define vte_terminal_set_opacity(x,y) NULL
	#define vte_terminal_set_background_transparent(x,y) vte_terminal_set_size(x,1,1)
	#define vte_terminal_set_background_image_file(x,y) vte_terminal_set_size(x,1,1)
	#define vte_terminal_set_background_saturation(x,y) vte_terminal_set_size(x,1,1)
	#define vte_terminal_set_scroll_background(x,y) vte_terminal_set_size(x,1,1)
	#define vte_terminal_set_background_tint_color(x,y) vte_terminal_set_size(x,1,1)
#  endif
#else
	// END: vte-0.34.8/src/vtedeprecated.h: void vte_terminal_set_opacity(VteTerminal *terminal, guint16 opacity) G_GNUC_DEPRECATED;
	// END: vte-0.34.8/src/vtedeprecated.h:	void vte_terminal_set_background_tint_color(VteTerminal *terminal,
	// END: vte-0.34.8/src/vtedeprecated.h: void vte_terminal_set_scroll_background(VteTerminal *terminal, gboolean scroll) G_GNUC_DEPRECATED;
	// END: vte-0.34.8/src/vtedeprecated.h: void vte_terminal_set_background_image(VteTerminal *terminal, GdkPixbuf *image) G_GNUC_DEPRECATED;
	// END: vte-0.34.8/src/vtedeprecated.h: void vte_terminal_set_background_image_file(VteTerminal *terminal, const char *path) G_GNUC_DEPRECATED;
	// END: vte-0.34.8/src/vtedeprecated.h: void vte_terminal_set_background_tint_color(VteTerminal *terminal, const GdkColor *color) G_GNUC_DEPRECATED;
	// END: vte-0.34.8/src/vtedeprecated.h: void vte_terminal_set_background_saturation(VteTerminal *terminal, double saturation) G_GNUC_DEPRECATED;
	// END: vte-0.34.8/src/vtedeprecated.h: void vte_terminal_set_background_transparent(VteTerminal *terminal, gboolean transparent) G_GNUC_DEPRECATED;
	// END: vte-0.34.8/src/vtedeprecated.h: void vte_terminal_set_opacity(VteTerminal *terminal, guint16 opacity) G_GNUC_DEPRECATED;
	#define ENABLE_VTE_BACKGROUND
#endif
#if VTE_CHECK_VERSION(0,34,8)
	// END: vte-0.34.8/src/vtedeprecated.h: GtkAdjustment *vte_terminal_get_adjustment(VteTerminal *terminal) G_GNUC_DEPRECATED;
	#define USE_GTK_SCROLLABLE
#endif
#if VTE_CHECK_VERSION(0,37,0)
	// SINCE: ./vte-0.37.0/src/vteterminal.h: void vte_terminal_search_set_gregex (VteTerminal *terminal, GRegex *regex, GRegexMatchFlags flags);
	#define vte_terminal_search_set_gregex(a,b) vte_terminal_search_set_gregex(a,b,0)
	// END: ./vte-0.36.3/src/vte.h: void vte_terminal_set_color_foreground_rgba()
	#define vte_terminal_set_colors_rgba vte_terminal_set_colors
	#define vte_terminal_set_color_foreground_rgba vte_terminal_set_color_foreground
	#define vte_terminal_set_color_background_rgba vte_terminal_set_color_background
	#define vte_terminal_set_color_bold_rgba vte_terminal_set_color_bold
	#define vte_terminal_set_color_cursor_rgba vte_terminal_set_color_cursor
	// SINCE: ./vte-0.37.0/src/vteterminal.h void vte_terminal_set_font()
	#define USE_VTE_TERMINAL_SET_FONT
	// END: vte-0.36.3/src/vtedeprecated.h void vte_terminal_set_opacity() G_GNUC_DEPRECATED;
	#undef FORCE_ENABLE_VTE_BACKGROUND
#else
	// SINCE: vte-0.37.0/src/vteterminal.h: gboolean vte_terminal_spawn_sync()
	#define vte_terminal_spawn_sync(a,b,c,d,e,f,g,h,i,j,k) vte_terminal_fork_command_full(a,b,c,d,e,f,g,h,i,k)
	// END: vte-0.36.3/src/vte.h: void vte_terminal_set_word_chars()
	#define ENABLE_SET_WORD_CHARS
	// END: vte-0.37.0/src/vteterminal.h: void vte_terminal_set_emulation()
	#define ENABLE_SET_EMULATION
	// END: vte-0.36.3/src/vte.h void vte_terminal_match_clear_all()
	#define vte_terminal_match_remove_all vte_terminal_match_clear_all
	// END: vte-0.36.3/src/vte-private.h: GtkBorder inner_border;
	#define VTE_HAS_INNER_BORDER
#endif
#if VTE_CHECK_VERSION(0,37,90)
	// SINCE: ./vte-0.37.90/src/vteterminal.h: gboolean vte_terminal_set_encoding(VteTerminal *terminal, const char *codeset, GError **error)
	#define vte_terminal_set_encoding(a,b) vte_terminal_set_encoding(a,b,NULL)
#endif
#if ! VTE_CHECK_VERSION(0,38,0)
	// END: ./vte-0.37.90/src/vteterminal.h: void vte_terminal_set_visible_bell()
	#define ENABLE_VISIBLE_BELL
#else
#  ifdef UNIT_TEST
	#define vte_terminal_set_visible_bell(x,y) vte_terminal_set_size(x,1,1)
#  endif
#endif

#define ENABLE_RGBA_VER "GTK 2.11"
#define ENABLE_GDKCOLOR_TO_STRING_VER "GTK 2.11"
#define ENABLE_MOUSE_SCROLL_VER "GTK 2.13"
#define ENABLE_VTE_SELECT_ALL_VER "VTE 0.15.3"
#define ENABLE_FIND_STRING_VER "VTE 0.25.1"

#define ALL_ACCELS_MASK (GDK_CONTROL_MASK | GDK_SHIFT_MASK | GDK_MOD1_MASK | GDK_MOD4_MASK | GDK_SUPER_MASK)
#define SHIFT_ONLY_MASK (GDK_CONTROL_MASK | GDK_MOD1_MASK | GDK_MOD4_MASK | GDK_SUPER_MASK)
// mods = (event->state | GDK_LOCK_MASK | GDK_MOD2_MASK) & GDK_MODIFIER_MASK;
#define DUD_MASK (GDK_LOCK_MASK | GDK_MOD2_MASK | ( ~GDK_MODIFIER_MASK))

#define KEY_GROUP 9
typedef enum {
	KEY_GROUP_MISC,						// 0
	KEY_GROUP_TAB_OPERATION,				// 1
	KEY_GROUP_SWITCH_TAB,					// 2
	KEY_GROUP_MOVE_TAB,					// 3
	KEY_GROUP_SWITCH_TO_TAB,				// 4
	KEY_GROUP_TEXT,						// 5
	KEY_GROUP_SCROLL,					// 6
	KEY_GROUP_FONT,						// 7
	KEY_GROUP_NONE,						// 8, MUST BE THE LAST ONE!!
} Key_Binding_Groups;

#ifdef FATAL
	#define KEYS 46
	#define FIXED_KEYS 3
#else
	#define KEYS 45
	#define FIXED_KEYS 2
#endif

typedef enum {
	KEY_DISABLE_FUNCTION,					// 0
	KEY_NEW_TAB,						// 1
	KEY_CLOSE_TAB,						// 2
	KEY_EDIT_LABEL,						// 3
	KEY_FIND,						// 4
	KEY_FIND_PREV,						// 5
	KEY_FIND_NEXT,						// 6
	KEY_PREV_TAB,						// 7
	KEY_NEXT_TAB,						// 8
	KEY_FIRST_TAB,						// 9
	KEY_LAST_TAB,						// 10
	KEY_MOVE_TAB_FORWARD,					// 11
	KEY_MOVE_TAB_BACKWARD,					// 12
	KEY_MOVE_TAB_FIRST,					// 13
	KEY_MOVE_TAB_LAST,					// 14
	KEY_SWITCH_TO_TAB_1,					// 15
	KEY_SWITCH_TO_TAB_2,					// 16
	KEY_SWITCH_TO_TAB_3,					// 17
	KEY_SWITCH_TO_TAB_4,					// 18
	KEY_SWITCH_TO_TAB_5,					// 19
	KEY_SWITCH_TO_TAB_6,					// 20
	KEY_SWITCH_TO_TAB_7,					// 21
	KEY_SWITCH_TO_TAB_8,					// 22
	KEY_SWITCH_TO_TAB_9,					// 23
	KEY_SWITCH_TO_TAB_10,					// 24
	KEY_SWITCH_TO_TAB_11,					// 25
	KEY_SWITCH_TO_TAB_12,					// 26
	KEY_NEW_WINDOW,						// NEW
	KEY_SELECT_ALL,						// 27
	KEY_COPY_CLIPBOARD,					// 28
	KEY_PASTE_CLIPBOARD,					// 29
	KEY_COPY_PRIMARY,					// NEW
	KEY_PASTE_PRIMARY,					// 39
	KEY_INCREASE_FONT_SIZE,					// 30
	KEY_DECREASE_FONT_SIZE,					// 31
	KEY_RESET_FONT_SIZE,					// 32
	KEY_MAX_WINDOW,						// 33
	KEY_FULL_SCREEN,					// 34
	KEY_SCROLL_UP,						// 35
	KEY_SCROLL_DOWN,					// 36
	KEY_SCROLL_UP_1_LINE,					// 37
	KEY_SCROLL_DOWN_1_LINE,					// 38
	KEY_CLEAN_SCROLLBACK_LINES,				// NEW
	KEY_DISABLE_URL_L,					// NEW
	KEY_DISABLE_URL_R,					// NEW
#ifdef FATAL
	KEY_DUMP_DATA,						// 40
#endif
} Key_Bindings;

typedef enum {
	CLOSE_TAB_NORMAL,
	CLOSE_WITH_TAB_CLOSE_BUTTON,
	CLOSE_WITH_WINDOW_CLOSE_BUTTON,
	CLOSE_WITH_KEY_STRIKE,
} Close_Tab_Type;

typedef enum {
	FONT_RESET,
	FONT_INCREASE,
	FONT_DECREASE,
	FONT_ZOOM_OUT,
	FONT_ZOOM_IN,
	FONT_RESET_DEFAULT,
	FONT_RESET_SYSTEM,
	FONT_SET_TO_SELECTED,
} Font_Set_Type;

typedef enum {
	FONT_NAME_DEFAULT,
	FONT_NAME_SYSTEM,
	FONT_NAME_RESTORE,
	FONT_NAME_UPDATE,
	FONT_NAME_ZOOM_OUT,
	FONT_NAME_ZOOM_IN,
	FONT_NAME_INCREASE,
	FONT_NAME_DECREASE,
} Font_Name_Type;

typedef enum {
	RESET_CURRENT_TAB_FONT,
	RESET_ALL_TO_CURRENT_TAB,
	RESET_ALL_TO_DEFAULT,
	RESET_ALL_TO_SYSTEM,
} Font_Reset_Type;

typedef enum {
	FIND_PREV,
	FIND_NEXT,
} Dialog_Find_Type;

typedef enum {
	AUTOMATIC,	// 0
	ON,		// 1
	OFF,		// 2
	TEMPORARY_ON,	// 3
	TEMPORARY_OFF,	// 4
	FORCE_ON,	// 5
	FORCE_OFF,	// 6
	UNSURE,		// 7
} Switch_Type;

typedef enum {
	TAG_WWW,
	TAG_FTP,
	TAG_FILE,
	TAG_MAIL,
} Command_Tag;

typedef enum {
	SELECTION_CLIPBOARD,
	SELECTION_PRIMARY,
} Clipboard_Type;

typedef enum {
	PAGE_METHOD_AUTOMATIC,
	PAGE_METHOD_WINDOW_TITLE,
	PAGE_METHOD_CMDLINE,
	PAGE_METHOD_PWD,
	PAGE_METHOD_NORMAL,
	PAGE_METHOD_REFRESH,
} Page_Update_Method;

typedef enum {
	ANSI_THEME_INVERT_COLOR,
	ANSI_THEME_SET_ANSI_THEME,
} Set_ANSI_Theme_Type;

#if defined(USE_GTK2_GEOMETRY_METHOD) || defined(UNIT_TEST)
typedef enum {
	FULLSCREEN_UNFS_OK =  2,
	FULLSCREEN_UNFS_ING = 1,
	FULLSCREEN_NORMAL =   0,
	FULLSCREEN_FS_ING =  -1,
	FULLSCREEN_FS_OK =   -2,
} FullScreen_Type;
#endif

typedef enum {
	HINTS_FONT_BASE,
	HINTS_NONE,
#if defined(USE_GTK3_GEOMETRY_METHOD) || defined(UNIT_TEST)
	HINTS_SKIP_ONCE,
#endif
} Hints_Type;

#if defined(USE_GTK3_GEOMETRY_METHOD) || defined(UNIT_TEST)
typedef enum {
	WINDOW_NORMAL,				// 0
	WINDOW_RESIZING_TO_NORMAL,		// 1
	WINDOW_MAX_WINDOW,			// 2
	WINDOW_MAX_WINDOW_TO_FULL_SCREEN,	// 3
	WINDOW_FULL_SCREEN,			// 4
	WINDOW_START_WITH_FULL_SCREEN,		// 5
	WINDOW_APPLY_PROFILE_NORMAL,		// 6
	WINDOW_APPLY_PROFILE_FULL_SCREEN,	// 7
} Window_Status;

typedef enum {
	GEOMETRY_AUTOMATIC,
	GEOMETRY_CUSTOM,
} Geometry_Resize_Type;
#endif

#if defined(GEOMETRY) || defined(UNIT_TEST) || defined(DEBUG)
typedef enum {
	ANSI_COLOR_BLACK =   30,
	ANSI_COLOR_RED =     31,
	ANSI_COLOR_GREEN =   32,
	ANSI_COLOR_YELLOW =  33,
	ANSI_COLOR_BLUE =    34,
	ANSI_COLOR_MAGENTA = 35,
	ANSI_COLOR_CYAN =    36,
	ANSI_COLOR_WHITE =   37,
} ANSI_Color;
#endif

// KeyValue: only need to init once when LilyTerm starts.
// so that we don't need to free them.
struct KeyValue
{
	gchar *name;
	gchar *topic;
	gchar *comment;
	gchar *translation;
	gint group;
};

// User_KeyValue: can be custom by profile.
// Every LilyTerm window has it's own User_KeyValue
struct User_KeyValue
{
	gchar *value;
	guint key;
	guint mods;
};

#define COMMAND 4
// 0 username
// 1 password
// 2 hostname
// 3 address_body
// 4 address_end
typedef enum {
	REGEX_USERNAME,
	REGEX_PASSWORD,
	REGEX_HOSTNAME,
	REGEX_ADDRESS_BODY,
	REGEX_ADDRESS_END,
} Regex_Type;
#define REGEX 5

struct Command
{
	gchar *name;
	gchar *match;
	gchar *comment;
	gchar *method_name;
	gchar *environ_name;
	gchar *VTE_CJK_WIDTH_name;
	gchar *locale_name;
	gchar *match_regex_name;
	gchar *encoding_name;
};

struct User_Command
{
	gchar *command;
	gint method;
	gchar *environ;
	gchar **environments;
	gint VTE_CJK_WIDTH;
	gchar *locale;
	gchar *match_regex_orig;
	gchar *match_regex;
};


#define COLOR 16

struct Color
{
	gchar *name;
	gchar *comment;
	gchar *translation;
};

#define THEME 10
#define DEFAULT_THEME 0
struct GdkColor_Theme
{
	gint index;
	gchar *name;
	GdkColor color[COLOR];
};

#ifdef USE_GDK_RGBA
struct GdkRGBA_Theme
{
	gint index;
	gchar *name;
	GdkRGBA color[COLOR];
};
#else
#  define GdkRGBA_Theme GdkColor_Theme
#endif

#ifdef ENABLE_VTE_ERASE_TTY
	#define ERASE_BINDING 5
#else
	#define ERASE_BINDING 4
#endif
#define DEFAULT_ERASE_BINDING 2

struct Erase_Binding
{
	gchar *name;
	gint value;
};

#if defined(ENABLE_CURSOR_SHAPE) || defined(UNIT_TEST)
	#define CURSOR_SHAPE 3
	#define DEFAULT_CURSOR_SHAPE 0

struct Cursor_Shape
{
	gchar *name;
	gint value;
};
#endif

// 1: WinTitle
// 2: CmdLine
// 3: PWD
// 4: Custom
// 5: Root
// 6: Normal

#define PAGE_COLOR 6
struct Page_Color
{
	gchar *name;
	gchar *comment;
	gchar *comment_eng;
};


// see /usr/include/linux/threads.h for more details.
#define PID_MAX_DEFAULT 0x8000
struct Process_Data
{
	GtkWidget *window;
	gint page_no;
	gint ppid;
	gchar* cmd;
};


struct Window
{

// ---- environ and command line option ---- //

	// environments: the environ for every lilyterm window, separated with <\t>
	gchar *environment;

	// Try to avoid the warning "The locale xx_XX seems NOT supported by your system!" too many times.
	GString *warned_locale_list;

	GtkWidget *encoding_locale_menuitems[3];
	// the runtime_encoding is got from get_encoding_from_locale()
	gchar *runtime_encoding;
	// the default_encoding is got from 'default locale' in profile
	gchar *default_encoding;
	// the encodings_list is generated from locale_list
	// gchar *encodings_list;
	// OLD: supported_encodings = g_strsplit_set(default_encoding + encodings_list +
	//					     custom_encoding + vte_terminal_get_encoding())
	// FIX: supported_encodings = g_strsplit_set(encodings_list)
	// gchar **supported_encodings;
	// GtkWidget *encoding_menuitem;
	GtkWidget *encoding_sub_menu;
	// GSList *encoding_group;
	// 1st item, System Default.
	// If the encoding of a vte is different with the default_encoding,
	// The encoding will be shown on the tab name.
	GtkWidget *default_menuitem_encoding;

	// the default_locale is got from profile
	gchar *default_locale;
	gchar *runtime_LC_MESSAGES;
	// the runtime_locale_list is got from environment
	gchar *runtime_locale_list;
	// the locales_list is got from profile
	gchar *locales_list;
	// supported_locales = g_strsplit_set(default_locale + locales_list + custom_locale + LC_CTYPE)
	// gchar **supported_locales;
	// GtkWidget *locale_menuitem_utf8;
	// GtkWidget *locale_menuitem_full;
	GtkWidget *locale_sub_menu;
	// GSList *locale_group;
	// GtkWidget *default_locale;

	gchar *default_shell;
	// emulate_term = xterm, don't change it
	gchar *emulate_term;

	// for env VTE_CJK_WIDTH.
	// VTE_CJK_WIDTH is got from profile
	gint VTE_CJK_WIDTH;
	// VTE_CJK_WIDTH_STR is got from environment
	gchar *VTE_CJK_WIDTH_STR;

	gchar *wmclass_name;
	gchar *wmclass_class;
	gchar *shell;
	gchar *home;

	// example: lilyterm -e vim /tmp/foo.txt
	// the argc after -e option
	// Will set to 0 after used.
	gint argc;
	// The argv[] after -e option, like '/tmp/foo.txt' in the example
	// Will set to NULL after used.
	gchar **argv;
	// The first command in -e option, like 'vim' in the example
	// Will set to NULL after used.
	// * Do NOT free it *
	gchar *command;
	gboolean hold;
	// For -t option
	gint init_tab_number;
	// For -d option
	gchar *init_dir;
	// For -l option
	gboolean login_shell;
	// For -ut option
	gboolean utmp;
	gchar *geometry;

	// If the profile is specify with -u option.
	GtkWidget *subitem_new_window_from_list;
	GtkWidget *menuitem_new_window_from_list;
	GtkWidget *subitem_load_profile_from_list;
	GtkWidget *menuitem_load_profile_from_list;
	gboolean use_custom_profile;
	gchar *profile;
	gchar *specified_profile;
	glong profile_dir_modtime;
	GtkWidget *menuitem_auto_save;
	gboolean auto_save;

// ---- the component of a single window ---- //

#if defined(USE_GTK2_GEOMETRY_METHOD) || defined(UNIT_TEST)
	// Startup with fullscreen
	gboolean startup_fullscreen;
	gboolean fullscreen;
	// true_fullscreen = <Alt><Enter>
	gboolean true_fullscreen;
	// 0: No work
	// 1: Normal
	// 2: Force ON
	// 3: Force OFF
	// gboolean fullscreen_show_scroll_bar;
	//  0: Normal
	// >0: unfullscreening
	// <0: fullscreening
#endif
#ifdef USE_GTK2_GEOMETRY_METHOD
	FullScreen_Type window_status;
#endif
#ifdef USE_GTK3_GEOMETRY_METHOD
	Window_Status window_status;
#endif
	Switch_Type show_tabs_bar;
	// the component of a single window
	GtkWidget *window;
	GtkWidget *notebook;
	gboolean show_close_button_on_tab;
	gboolean show_close_button_on_all_tabs;
	GtkWidget *current_vte;

	// When The window is lost-focus, update the window title only.
	// Or, The geometry of vte will be incorrect when add a new teb with [Menu]
	gboolean window_title_shows_current_page;
	gboolean window_title_append_package_name;
	// The title specified by -T option.
	// Please see http://www.debian.org/doc/debian-policy/ch-customized-programs.html#s11.8.3 for more details
	gchar *custom_window_title_str;
	// Only used to change the color of tab when launch the adjesting color dialog
	// gboolean update_window_title_only;
	// the pid of showing on the Window title
	pid_t window_title_tpgid;

	// The custom tab titles from -n option.
	GString *custom_tab_names_str;
	gchar **custom_tab_names_strs;
	gint custom_tab_names_current;
	gint custom_tab_names_total;

	// -1: rgba is inited already
	//  0: do NOT use rgba
	//  1: force to use rgba
	//  2: decide by gdk_screen_is_composited()
	gint use_rgba;
	gint use_rgba_orig;
#if defined(ENABLE_RGBA) || defined(UNIT_TEST)
	gint transparent_window;
	gdouble window_opacity;

	gboolean dim_window;
	gint transparent_window_inactive;
	gdouble window_opacity_inactive;
	gboolean window_is_inactivated;
	gboolean dim_window_expect;
	GtkWidget *menuitem_dim_window;
#endif

	gboolean enable_key_binding;
	struct User_KeyValue user_keys[KEYS];

	// 0: Update the hints with base size = 1
	// 1: Update the hints with base size = font char size
	Hints_Type hints_type;

#if defined(USE_GTK3_GEOMETRY_METHOD) || defined(UNIT_TEST)
	// Geometry_Resize_Type = GEOMETRY_AUTOMATIC, resize form current_vte
	// Geometry_Resize_Type = GEOMETRY_CUSTOM, resize from geometry_width and geometry_height
	Geometry_Resize_Type resize_type;
	// geometry_width, geometry_height = column * row when hints_type=HINTS_NONE
	// geometry_width, geometry_height = width * height (in pixel) when hints_type=HINTS_FONT_BASE
	glong geometry_width;
	glong geometry_height;
#endif

	gboolean lost_focus;

	// Trying to keep the vte size:
	//  1, When the page bar was hidden.
	//  2, When the page bar was shown.
	//  3, When the font was changed by right click menu.
	//  4, Increase/decrease window size.
	//  5, Resotre to system/default font.
	//  6, Theme has been changed.
	//  7, Using Dir/Cmdline on pagename.
	//  8, When clicking and dragging a tab
	//  9, Hide/Show scroll_bar
	// 10, Fullscreen/Unfullscreen

	// x: Trying to resize 'twice'. setted to 0 after resized.
	// y: Trying to resize 'once'. setted to 0 after resized.
	// 1: Means that the Window is resizing.
	//
	// 0000 0000 0000 00x1 (0x0003): Updating Page Name.
	// 0000 0000 000y x100 (0x001C): Showing/Hiding tabs bar, It should only run window_size_request() once.
	// 0000 0000 0x10 0000 (0x0060): Changing Themes.
	// 0000 00yx 1000 0000 (0x0380): Change the vte font, It should only run window_size_request() once.
	// 0000 x100 0000 0000 (0x0C00): Hide/Show scroll_bar
	// 00x1 0000 0000 0000 (0x3000): Changing the "Window Title" of foreground program. <- useless
	// x100 0000 0000 0000 (0xC000): Using in Fullscreen/Unfullscreen. <- useless

	// 1010 1001 0100 1010 (0xA94A): Check for x, Make y only run window_size_request() once.
	// 0101 0100 1010 0101 (0x54A5): Clean x and y, Means that it is resized.

	// 1111 1101 1110 1111 (0xFDEF): The mask result after check for x, but skip [Updating Page Name].
	// 0101 0110 1011 0101 (0x56B5): The mask result after check for y, but skip [Updating Page Name].

	guint keep_vte_size;

// ---- the component of a single menu ---- //

	GtkWidget *menu;
	gboolean menu_activated;

#if defined(ENABLE_RGBA) || defined(UNIT_TEST)
	GtkWidget *menuitem_trans_win;
#endif
	GtkWidget *menuitem_trans_bg;
//	GtkWidget *menuitem_scrollback_lines;

	gboolean show_color_selection_menu;

	gboolean show_resize_menu;
	gdouble font_resize_ratio;
	gdouble window_resize_ratio;

	gboolean show_background_menu;
#ifdef ENABLE_IM_APPEND_MENUITEMS
	gboolean show_input_method_menu;
#endif
	gboolean show_change_page_name_menu;
	gboolean show_exit_menu;

	// FIXME: May cause segfault
	// enable_hyperlink will enable [Copy URL] menuitem.
	// It can't copy to page_data
	gboolean enable_hyperlink;
	gchar *user_regex[REGEX];
	struct User_Command user_command[COMMAND];
	GtkWidget *menuitem_copy_url;

	GtkWidget *menuitem_dim_text;
	GtkWidget *menuitem_cursor_blinks;
	GtkWidget *menuitem_allow_bold_text;
	GtkWidget *menuitem_open_url_with_ctrl_pressed;
	GtkWidget *menuitem_disable_url_when_ctrl_pressed;
	GtkWidget *menuitem_audible_bell;
#if defined(ENABLE_VISIBLE_BELL) || defined(UNIT_TEST)
	GtkWidget *menuitem_visible_bell;
#endif
#if defined(ENABLE_BEEP_SINGAL) || defined(UNIT_TEST)
	GtkWidget *menuitem_urgent_bell;
#endif
	GtkWidget *menuitem_show_tabs_bar;
	GtkWidget *menuitem_hide_tabs_bar;
	GtkWidget *menuitem_always_show_tabs_bar;
	GtkWidget *menuitem_always_hide_tabs_bar;
	// the menuitem_hide_scroll_bar->active stores the boolean value of hide/show scroll_bar.
	GtkWidget *menuitem_hide_scroll_bar;

	gboolean show_copy_paste_menu;
	gboolean embedded_copy_paste_menu;
	GtkWidget *menuitem_copy;
	GtkWidget *menuitem_paste;
	GtkWidget *menuitem_clipboard;
	GtkWidget *menuitem_primary;

// ---- the color used in vte ---- //

	GdkRGBA cursor_color;
	gboolean custom_cursor_color;

	// color datas
	gint color_theme_index;
	gboolean invert_color;
	gdouble color_brightness;
	gdouble color_brightness_inactive;

	struct GdkRGBA_Theme custom_color_theme[THEME];
	// the adjusted (include invert_color and color_brightness for color themes) colors
	GdkRGBA color[COLOR];
	// the adjusted (include invert_color and color_brightness_inactive for color themes) colors
	GdkRGBA color_inactive[COLOR];

#define ANSI_THEME_MENUITEM 7
	GtkWidget *ansi_theme_menuitem[ANSI_THEME_MENUITEM];
	// color[] and color_orig[] will always be initd when creating a window.
	GtkWidget *ansi_color_sub_menu;
	GtkWidget *ansi_color_menuitem;
	GtkWidget *menuitem_invert_color;
	GtkWidget *menuitem_theme[THEME*2];
	GtkWidget *current_menuitem_theme;

	// default_vte_color = TRUE: use vte_terminal_set_color_foreground/background().
	// default_vte_color = FALSE: use vte_terminal_set_colors() to set the color of vte.
	// gboolean default_vte_color_theme;
	// default_vte_color_theme = ! (color_theme_index || invert_color || color_brightness || color_brightness_inactive)
	gboolean have_custom_color;;
	gboolean use_custom_theme;

// ---- tabs on notebook ---- //

	gint page_width;				/* Should be take care when drag to another window */
	gboolean fill_tabs_bar;
	gboolean tabs_bar_position;

	// page name
	gchar *page_name;				/* Should be take care when drag to another window */
	gboolean reuse_page_names;
	gchar *page_names;
	gchar **splited_page_names;
	gint page_names_no;
	gint max_page_names_no;

	gboolean page_shows_number;			/* Should be take care when drag to another window */
	gboolean page_shows_encoding;			/* Should be take care when drag to another window */

	gboolean page_shows_current_cmdline;
	gboolean page_shows_window_title;
	gboolean page_shows_current_dir;

	// page color and bold
	gboolean use_color_page;
	gboolean check_root_privileges;
	gboolean bold_current_page_name;
	gboolean bold_action_page_name;
	// We will NOT copy the following data to page_data for performance
	// but trying to keep them to sync with win_data
	gchar *user_page_color[PAGE_COLOR];		/* Should be take care when drag to another window */

// ---- font ---- //
	// WRANING: font_anti_alias is no use since VTE 0.20.0
	gboolean font_anti_alias;			/* Should be take care when drag to another window */
	gchar *default_font_name;			/* Should be take care when drag to another window */
	// Only using in <Ctrl><Enter>
	gchar *restore_font_name;

// ---- other settings for init a vte ---- //

	glong default_column;
	glong default_row;
#ifdef ENABLE_SET_WORD_CHARS
	gchar *word_chars;				/* Should be take care when drag to another window */
#endif
	Switch_Type show_scroll_bar;			/* Should be take care when drag to another window */
	// 0: left
	// 1: right
	gboolean scroll_bar_position;			/* Should be take care when drag to another window */
#if defined(ENABLE_VTE_BACKGROUND) || defined(FORCE_ENABLE_VTE_BACKGROUND) || defined(UNIT_TEST)
	gint transparent_background;
	gdouble background_saturation;
	gboolean scroll_background;
	gchar *background_image;
#endif
//	gboolean use_scrollback_lines;
	gint scrollback_lines;
	gboolean dim_text;
#ifdef USE_NEW_VTE_CURSOR_BLINKS_MODE
	gint cursor_blinks;
#else
	gboolean cursor_blinks;
#endif
	gboolean allow_bold_text;
	gboolean open_url_with_ctrl_pressed;
	gboolean disable_url_when_ctrl_pressed;
	gboolean audible_bell;
#if defined(ENABLE_VISIBLE_BELL) || defined(UNIT_TEST)
	gboolean visible_bell;
#endif
#if defined(ENABLE_BEEP_SINGAL) || defined(UNIT_TEST)
	// urgent_bell will stores the currect setting
	gboolean urgent_bell;
	// urgent_bell will stores the currect status
	gboolean urgent_bell_status;
	gulong urgent_bell_focus_in_event_id;
#endif
	gint erase_binding;
	GtkWidget *menuitem_erase_binding[ERASE_BINDING];
	GtkWidget *current_menuitem_erase_binding;
#if defined(ENABLE_CURSOR_SHAPE) || defined(UNIT_TEST)
	gint cursor_shape;
	GtkWidget *menuitem_cursor_shape[CURSOR_SHAPE];
	GtkWidget *current_menuitem_cursor_shape;
#endif

// ---- other ---- //

	gboolean prime_user_datas_inited;
	gboolean prime_user_settings_inited;

	gboolean confirm_to_close_multi_tabs;
	gboolean confirm_to_kill_running_command;
	gboolean confirm_to_execute_command;
	gchar *execute_command_whitelist;
	gchar **execute_command_whitelists;
	gboolean execute_command_in_new_tab;
	gboolean join_as_new_tab;
	gchar *foreground_program_whitelist;
	gchar **foreground_program_whitelists;
	gchar *background_program_whitelist;
	gchar **background_program_whitelists;
	gboolean confirm_to_paste;
	gchar *paste_texts_whitelist;
	gchar **paste_texts_whitelists;

	gchar *find_string;
	gboolean find_case_sensitive;
	gboolean find_use_perl_regular_expressions;
	GdkRGBA find_entry_bg_color;
	GdkRGBA find_entry_current_bg_color;

	gboolean checking_menu_item;
	gboolean kill_color_demo_vte;
	gboolean adding_page;
	// gboolean checked_profile_version;
	gboolean confirmed_profile_is_invalid;

	gchar *temp_data;
	gint temp_index;
};


struct Page
{

// ---- Some data from win_data ---- //

	GtkWidget *window;
	GtkWidget *notebook;
	// current page no on notebook. *for performance*
	guint page_no;
// ---- the component of a single vte ---- //

	GtkWidget *label;
	GtkWidget *label_text;
	GtkWidget *label_button;
	GtkWidget *hbox;
	GtkWidget *vte;
#if defined(USE_GTK3_GEOMETRY_METHOD) || defined(UNIT_TEST)
	GtkBorder *border;
#endif
	GtkAdjustment *adjustment;
	GtkWidget *scroll_bar;

// ---- environ and encoding ---- //

	// for drag&drop, or it will shows unnecessary "(UTF-8)" on the tab
	gchar *encoding_str;
	// locale: a string like "zh_TW.Big5". The locale of new tab.
	gchar *locale;
	// environ: The environ that user specify in profile
	gchar *environ;
	// for env VTE_CJK_WIDTH
	gchar *VTE_CJK_WIDTH_STR;

// ---- for the text shown on the tab of notebook ---- //

	// the page_name of this page.
	gchar *page_name;
	// the pid of vte
	pid_t pid;
	// the pid of foreground program
	pid_t current_tpgid;
	// the pid of showing on the tab name
	pid_t displayed_tpgid;
	// the original cmdline for pid
	gchar *pid_cmdline;
	// The custom page name which inputed by user
	gchar *custom_page_name;
	// [!] The color for this tab. *DON'T FREE IT*
	// It is a point for win_data->colors.
	gchar *tab_color;
	// Current Directory
	gchar *pwd;
	// The running command is root privileges or not
	gboolean is_root;
	// The text of current page is bold or not
	gboolean is_bold;
	gboolean should_be_bold;
	// The text of current vte is 'inactivated' or not, for performance.
	gboolean vte_is_inactivated;
	// The text of current vte is dimmed or not. for auto-detect.
	gboolean dim_text_expect;

	// 'sleep': a program that is not using window-title.
	// 'vim': a program that is using window-title.
	//
	// 'dash': a shell that is not using window-title.
	// 'bash'a; a shell that is using window-title.
	//
	// record "window-title-changed" signal
	// situation: (*: the pid and tpgid is NOT the same)
	//
	//   1: use 'dash' as 'shell'
	//	set shell_use_win_title = NO, USE PWD
	//  *2: run 'sleep' on 'dash'
	//	-> if win_data->window_title and new_window_title are both NULL
	//	use CMDLINE
	//   3: exit from 'sleep' on 'dash'
	//	-> if win_data->window_title and new_window_title are both NULL, and shell_use_win_title = NO
	//	use PWD
	//  *4: run 'vim' on 'dash'
	//	-> if win_data->window_title=NULL and new_window_title!=NULL
	//	use WINTITLE
	//   5: exit from 'vim' on 'dash'
	//	-> if win_data->window_title != new_window_title
	//	clean win_data->window_title, use PWD
	//   6: exit from 'vim' on 'dash', but it don't update the win-title when exiting
	//	-> if win_data->window_title = new_window_title, and shell_use_win_title = NO
	//	clean win_data->window_title, use PWD
	//   7: use 'bash' as 'shell'
	//	set shell_use_win_title = YES, USE WINTITLE
	//  *8: run 'sleep' on 'bash'
	//	-> if win_data->window_title and new_window_title are the same
	//	use CMDLINE
	//   9: exit from 'sleep' on 'bash'
	//	-> if win_data->window_title and new_window_title are the same, and shell_use_win_title = YES
	//	use WINTITLE
	// *10: run 'vim' on 'bash'
	//	-> if win_data->window_title != new_window_title
	//	use WINTITLE
	//  11: exit from 'vim' on 'bash'
	//	-> if win_data->window_title != new_window_title
	//	use WINTITLE
	//  12: exit from 'vim' on 'bash', but it don't update the win-title when exiting
	//	-> if win_data->window_title != new_window_title
	//	use WINTITLE
	//  13: the 'shell' is starting to use window-title.
	//	-> if win_data->window_title != new_window_title
	//	use WINTITLE, shell_use_win_title = YES
	//  14: the 'shell' stop to use window-title.
	//	-> if PWD is changed and window-title is not changed
	//	clean win_data->window_title, set shell_use_win_title = NO, use PWD
	//  15: the running commsnd cleaned the winw-title <- impossiable?
	//  16: the shell cleaned the winw-title <- impossiable?

	//  1: updated
	//  0: nothing happened
	// -1: "window-title-changed" stop working
	gint window_title_updated;
	// 0: auto (default)
	// 1: window title
	// 2: command line
	// 3: pwd
	// 4: common
	gint page_update_method;
	gulong window_title_signal;

	// the id of g_timeout_add_seconds()
	guint timeout_id;
	// the id of urgent_bell g_signal
	gulong urgent_bell_handler_id;

// ---- font ---- //

	// for resize font
	gchar *font_name;
	// font_size = 0: use the data in font_name.
	gint font_size;

// ---- some data came from window---- //

	gboolean check_root_privileges;
	gboolean page_shows_window_title;
	gboolean page_shows_current_dir;
	gboolean page_shows_current_cmdline;
	gboolean bold_action_page_name;

	// some data came from window. for the performance of monitor_cmdline
	pid_t *window_title_tpgid;
	gboolean *lost_focus;
	guint *keep_vte_size;
	GtkWidget **current_vte;
	// gboolean *update_window_title_only;
	// a "path" string shown on window_title
	gchar *window_title_pwd;
	gboolean custom_window_title;

// ---- other---- //

	gint tag[COMMAND];
	gboolean match_regex_setted;
};


//
// **************************** profile.c ****************************
//


#define MOD 4
struct ModKey
{
	gchar *name;
	guint mod;
};


//
// **************************** dialog.c ****************************
//

typedef enum {
	EDIT_LABEL,
	FIND_STRING,
	ADD_NEW_LOCALES,
	CHANGE_THE_FOREGROUND_COLOR,
	CHANGE_THE_ANSI_COLORS,
	CHANGE_THE_BACKGROUND_COLOR,
	CHANGE_THE_CURSOR_COLOR,
	ADJUST_THE_BRIGHTNESS_OF_ANSI_COLORS_USED_IN_TERMINAL,
	ADJUST_THE_BRIGHTNESS_OF_ANSI_COLORS_WHEN_INACTIVE,
#if defined(ENABLE_VTE_BACKGROUND) || defined(FORCE_ENABLE_VTE_BACKGROUND)
	CHANGE_BACKGROUND_SATURATION,
#endif
	CHANGE_THE_OPACITY_OF_WINDOW,
	CHANGE_THE_OPACITY_OF_WINDOW_WHEN_INACTIVE,
	CHANGE_THE_TEXT_COLOR_OF_WINDOW_TITLE,
	CHANGE_THE_TEXT_COLOR_OF_CMDLINE,
	CHANGE_THE_TEXT_COLOR_OF_CURRENT_DIR,
	CHANGE_THE_TEXT_COLOR_OF_CUSTOM_PAGE_NAME,
	CHANGE_THE_TEXT_COLOR_OF_ROOT_PRIVILEGES_CMDLINE,
	CHANGE_THE_TEXT_COLOR_OF_NORMAL_TEXT,
	CONFIRM_TO_EXECUTE_COMMAND,
	CONFIRM_TO_CLOSE_MULTI_PAGES,
	CONFIRM_TO_CLOSE_RUNNING_APPLICATION,
	CONFIRM_TO_CLOSE_A_TAB_WITH_CHILD_PROCESS,
	CONFIRM_TO_CLOSE_A_WINDOW_WITH_CHILD_PROCESS,
	CONFIRM_TO_EXIT_WITH_CHILD_PROCESS,
	CONFIRM_TO_PASTE_TEXTS_TO_VTE_TERMINAL,
	GENERAL_INFO,
	PASTE_TEXTS_TO_EVERY_VTE_TERMINAL,
	PASTE_GRABBED_KEY_TO_EVERY_VTE_TERMINAL,
	SET_KEY_BINDING,
	USAGE_MESSAGE,
} Dialog_Type_Flags;

#endif
