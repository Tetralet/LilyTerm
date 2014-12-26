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
#include <glib/gi18n.h>
#include <locale.h>
// for opendir() readdir() closedir()
#include <dirent.h>
// for stat()
#include <sys/stat.h>
// for memcpy() memset()
#include <string.h>
// for aoti()
#include <stdlib.h>
// for GDK_WINDOW_XID
#include <gdk/gdkx.h>

#include "lilyterm.h"

typedef enum {
	IMAGE_MENU_ITEM,
	CHECK_MENU_ITEM,
} Menu_Itemn_Type;

typedef enum {
	NEW_WINDOW_FROM_SYSTEM_PROFILE,
	NEW_WINDOW_FROM_PROFILE,
	LOAD_FROM_SYSTEM_PROFILE,
	LOAD_FROM_PROFILE,
	APPLY_FROM_NEW_WIN_DATA,
} Apply_Profile_Type;

struct Preview
{
	gchar *default_filename;
	GtkWidget *mainbox;
	GtkWidget *frame;
	GtkWidget *vbox;
	GtkWidget *image;
	GtkWidget *no_image_text;
	GtkWidget *scroll_background;
};

void new_tab_with_locale(GtkWidget *local_menuitem, gboolean VTE_CJK_WIDTH);
GtkWidget *check_name_in_menuitem(GtkWidget *sub_menu, const gchar *name, gboolean case_sensitive);
#if defined(ENABLE_VTE_BACKGROUND) || defined(FORCE_ENABLE_VTE_BACKGROUND) || defined(UNIT_TEST)
void set_trans_bg(GtkWidget *menuitem_trans_bg, struct Window *win_data);
void load_background_image_from_file(GtkWidget *widget, struct Window *win_data);
void update_preview_image (GtkFileChooser *dialog, struct Preview *preview);
#endif
void set_trans_win(GtkWidget *widget, GtkWidget *window);
void invert_color_theme(GtkWidget *menuitem, struct Window *win_data);
void select_ansi_theme(GtkWidget *menuitem, gint index);
void set_ansi_theme(GtkWidget *menuitem, Set_ANSI_Theme_Type type, gboolean use_custom_theme, gboolean invert_color,
		    gint theme_index, struct Window *win_data);
void set_auto_save(GtkWidget *menuitem, struct Window *win_data);
void set_erase_binding (GtkWidget *menuitem, gint value);
#if defined(ENABLE_CURSOR_SHAPE) || defined(UNIT_TEST)
void set_cursor_shape (GtkWidget *menuitem, gint value);
#endif
GSList *create_theme_menu_items(struct Window *win_data, GtkWidget *sub_menu, GSList *theme_group, gint current_theme, gint custom_theme);
void reset_vte(GtkWidget *widget, struct Window *win_data);
void select_font(GtkWidget *widget, struct Window *win_data);
void set_dim_text(GtkWidget *menuitem_dim_text, struct Window *win_data);
#ifdef ENABLE_RGBA
void set_dim_window(GtkWidget *menuitem_dim_text, struct Window *win_data);
#endif
void set_cursor_blinks(GtkWidget *widget, struct Window *win_data);
void set_allow_bold_text(GtkWidget *menuitem_allow_bold_text, struct Window *win_data);
void set_open_url_with_ctrl_pressed(GtkWidget *menuitem_open_url_with_ctrl_pressed, struct Window *win_data);
void set_disable_url_when_ctrl_pressed(GtkWidget *menuitem_disable_url_when_ctrl_pressed, struct Window *win_data);
void set_audible_bell(GtkWidget *widget, struct Window *win_data);
#if defined(ENABLE_VISIBLE_BELL) || defined(UNIT_TEST)
void set_visible_bell(GtkWidget *widget, struct Window *win_data);
#endif
void urgent_beep(GtkWidget *window, struct Page *page_data);
void launch_hide_and_show_tabs_bar(GtkWidget *widget, Switch_Type show_tabs_bar);
void copy_url_clipboard(GtkWidget *widget, gpointer user_data);
void copy_clipboard(GtkWidget *widget, struct Window *win_data);
void paste_clipboard(GtkWidget *widget, struct Window *win_data);
void paste_to_every_vte(GtkWidget *widget, struct Window *win_data);
void open_current_dir_with_file_manager(GtkWidget *widget, struct Window *win_data);
void view_current_page_info(GtkWidget *widget, struct Window *win_data);
void view_clipboard(GtkWidget *widget, struct Window *win_data);
void view_primary(GtkWidget *widget, struct Window *win_data);
gint add_menuitem_to_locale_sub_menu(struct Window *win_data, gint no, gchar *name);
GtkWidget *recreate_profile_menu_item(GtkWidget *menuitem, GtkWidget *subitem,
				       struct Window *win_data, Apply_Profile_Type type);
void create_new_window_from_menu_items(GtkWidget *sub_menu, const gchar *stock_id);
void create_load_profile_from_menu_items(GtkWidget *sub_menu, const gchar *stock_id, struct Window *win_data);
gboolean create_profile_menu_list(GtkWidget *sub_menu, const gchar *stock_id, GSourceFunc func, gpointer func_data);
void apply_profile_from_file_dialog(GtkWidget *menu_item, Apply_Profile_Type type);
void apply_profile_from_menu_item(GtkWidget *menu_item, Apply_Profile_Type type);
void apply_profile_from_file(const gchar *path, Apply_Profile_Type type);
void reload_settings(GtkWidget *menu_item, struct Window *win_data);
void apply_to_every_window(GtkWidget *menu_item, struct Window *win_data);
void save_user_settings_as(GtkWidget *widget, struct Window *win_data);
GtkWidget *create_load_file_dialog(GtkFileChooserAction action, GtkWidget *window, gchar *button_text, gchar *filename);
GtkWidget *create_menu_item (Menu_Itemn_Type type, GtkWidget *sub_menu, const gchar *label, const gchar *label_name,
			     const gchar *stock_id, GSourceFunc func, gpointer func_data);
GtkWidget *create_sub_item (GtkWidget *menu, gchar *label, const gchar *stock_id);
GtkWidget *create_sub_item_subitem (gchar *label, const gchar *stock_id);
GtkWidget *create_sub_item_submenu (GtkWidget *menu, GtkWidget *menu_item);
GtkWidget *add_separator_menu(GtkWidget *sub_menu);
void set_menuitem_label(GtkWidget *menu_item, gchar *text);
void enable_disable_theme_menus(struct Window *win_data, gboolean show);
