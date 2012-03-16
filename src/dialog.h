/*
 * Copyright (c) 2008-2010 Lu, Chao-Ming (Tetralet).  All rights reserved.
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
// for strlen()
#include <string.h>
// for aoti()
#include <stdlib.h>

#include "lilyterm.h"

typedef enum {
	DIALOG_OK,
	DIALOG_OK_CANCEL,
	DIALOG_QUIT,
	DIALOG_NONE,
} Dialog_Button_Type;

typedef enum {
	BOX_NONE,
	BOX_HORIZONTAL,
	BOX_VERTICALITY,
} Box_Type;

typedef enum {
	ABOUT_BUTTON_USAGE,
	ABOUT_BUTTON_SHORTCUT_KEYS,
	ABOUT_BUTTON_LICENSE,
	ABOUT_BUTTON_TRANSLATORS,
	ABOUT_BUTTON_AUTHOR,
} About_Button_Type;

struct Color_Data
{
	// the function type.
	gint type;
	gboolean recover;

	gchar *demo_text;
	GtkWidget *demo_vte;

	gchar *original_page_color;
	GdkColor original_color;
	gchar *original_custom_page_name;
	GdkColor fg_color;
	GdkColor bg_color;

	gboolean transparent_background;
};

GtkWidget *create_label_with_text(GtkWidget *box, gboolean set_markup, gboolean selectable, gint max_width_chars, const gchar *text);
GtkWidget *add_secondary_button(GtkWidget *dialog, const gchar *text, gint response_id, const gchar *stock_id);
void paste_text_to_vte_terminal(GtkWidget *widget, struct Dialog *dialog_data);
void create_dialog(gchar *dialog_title_translation, gchar *dialog_title,  Dialog_Button_Type type,
		   GtkWidget *window, gboolean center, gboolean resizable, gint border_width,
		   gint response, gchar *icon, gchar *title, gboolean selectable, gint max_width_chars,
		   gboolean state_bottom, gint create_entry_hbox, gint entry_hbox_spacing,
		   struct Dialog *dialog_data);
void refresh_regex_settings(GtkWidget *widget, struct Window *win_data);
void refresh_regex(struct Window *win_data, struct Dialog *dialog_data);
void find_str(GtkWidget *widget, Dialog_Find_Type type);
GtkWidget *create_entry_widget (GtkWidget *box, gchar *contents, gchar *name, gchar *default_value, gboolean activates_default);
GtkWidget *create_frame_widget( struct Dialog *dialog_data, gchar *label,
				GtkWidget *label_widget, GtkWidget *child, guint padding);
GtkWidget *create_button_with_image(gchar *label_text, const gchar *stock_id, gboolean set_tooltip_text,
				    GSourceFunc func, gpointer func_data);
void create_color_selection_widget(struct Dialog *dialog_data, struct Color_Data *color_data,
				   Dialog_Type_Flags style, GSourceFunc func, gpointer func_data);
void create_scale_widget(struct Dialog *dialog_data, gdouble min, gdouble max, gdouble step, gdouble value,
			 GSourceFunc func, gpointer func_data);
void create_SIGKILL_and_EXIT_widget(struct Dialog *dialog_data, gboolean create_entry_hbox,
				    gboolean create_force_kill_hbox, gchar *count_str);
gboolean grab_key_press (GtkWidget *window, GdkEventKey *event, struct Dialog *dialog_data);
gboolean deal_dialog_key_press(GtkWidget *widget, GdkEventKey *event, struct Dialog *dialog_data);
gchar *deal_dialog_key_press_join_string(StrAddr **value, gchar *separator, gchar *mask);
gboolean clean_model_foreach(GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer userdata);
void recover_page_colors(GtkWidget *dialog, GtkWidget *window, GtkWidget *notebook);
gboolean set_ansi_color(GtkRange *range, GtkScrollType scroll, gdouble value, GtkWidget *vte);
void adjest_vte_color (GtkColorSelection *colorselection, GtkWidget *vte);
void set_new_ansi_color(struct Window *win_data);
void hide_combo_box_capital(GtkCellLayout *cell_layout, GtkCellRenderer *cell,
			    GtkTreeModel *tree_model, GtkTreeIter *iter, gpointer data);
void update_key_info (GtkTreeSelection *treeselection, struct Dialog *dialog_data);
void clear_key_group(GtkButton *button, struct Dialog *dialog_data);
void clear_key_group_all(GtkButton *button, struct Dialog *dialog_data);
void clear_key_groups(struct Dialog *dialog_data, gboolean clear_all);
GtkWidget *add_text_to_notebook(GtkWidget *notebook, const gchar *label, const gchar *stock_id, const gchar *text);
void show_usage_text(GtkWidget *notebook, gpointer page, guint page_num, struct Dialog *dialog_data);
// void err_page_data_is_null(gchar *function_name);
