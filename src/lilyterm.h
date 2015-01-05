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


#ifndef LILYTERM_H
#define LILYTERM_H

#include "data.h"

typedef gchar StrAddr;
typedef gchar StrLists;

//
// **************************** misc.c ****************************
//

#ifdef USE_GTK_ALT_DIALOG_BUTTON_ORDER
gboolean gtk_alt_dialog_button_order();
#endif
gboolean check_if_default_proc_dir_exist(gchar *proc_dir);
gchar *convert_array_to_string(gchar **array, gchar separator);
gchar *convert_str_to_utf8(gchar *string, gchar *encoding_str);
gchar *convert_escape_sequence_to_string(const gchar *string);
gchar *convert_escape_sequence_from_string(const gchar *string);
gboolean compare_strings(const gchar *string_a, const gchar *string_b, gboolean case_sensitive);
void set_VTE_CJK_WIDTH_environ(gint VTE_CJK_WIDTH);
gchar *get_VTE_CJK_WIDTH_str(gint VTE_CJK_WIDTH);
gint get_default_VTE_CJK_WIDTH();
void restore_SYSTEM_VTE_CJK_WIDTH_STR();
void set_env(const gchar *variable, const gchar *value, gboolean overwrite);
const gchar *get_default_lc_data(gint lc_type);
gchar *get_encoding_from_locale(const gchar *locale);
gboolean check_string_in_array(gchar *str, gchar **lists);
gchar *get_proc_data(pid_t pid, gchar *file, gsize *length);
gchar **split_string(const gchar *str, const gchar *split, gint max_tokens);
gint count_char_in_string(const gchar *str, const gchar split);
gchar **get_pid_stat(pid_t pid, gint max_tokens);
gchar *convert_text_to_html(StrAddr **text, gboolean free_text, gchar *color, StrLists *tag, ...);
gchar *join_strings_to_string(const gchar separator, const gint total, const StrLists *string, ...);
gchar *colorful_max_new_lines(gchar *string, gint max, gint output_line);
#ifdef ENABLE_RGBA
GdkRGBA convert_color_to_rgba(GdkColor color);
GdkColor convert_rgba_to_color(GdkRGBA rgba);
gchar *dirty_gdk_rgba_to_string(GdkRGBA *rgba);
gboolean dirty_gdk_color_parse(const gchar *spec, GdkRGBA *color);
#endif
GtkWidget *dirty_gtk_vbox_new(gboolean homogeneous, gint spacing);
GtkWidget *dirty_gtk_hbox_new(gboolean homogeneous, gint spacing);
#if defined(ENABLE_VTE_BACKGROUND) || defined(FORCE_ENABLE_VTE_BACKGROUND) || defined(UNIT_TEST)
void dirty_vte_terminal_set_background_tint_color(VteTerminal *vte, const GdkRGBA rgba);
#endif
#if defined(GEOMETRY) || defined(UNIT_TEST)
void widget_size_allocate (GtkWidget *widget, GtkAllocation *allocation, gchar *name);
#endif

#if defined(OUT_OF_MEMORY) || defined(UNIT_TEST)
gchar *fake_g_strdup(const gchar *gchar);
gchar *fake_g_strdup_printf(const StrLists *format, ...);
#endif

//
// **************************** main.c ****************************
//

gboolean init_socket_data();
gboolean query_socket();
gboolean send_socket(int argc, char *argv[], gboolean wait);
void main_quit(GtkWidget *widget, struct Window *win_data);
void quit_gtk();

//
// **************************** console.c ****************************
//

void command_option(int argc, char *argv[]);
gchar *get_help_message_usage(gchar *profile, gboolean convert_to_html);
gchar *get_help_message_key_binding(gboolean convert_to_html);

//
// **************************** window.c ****************************
//

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
			struct Page *page_data_orig);
gchar *get_init_dir(pid_t pid, gchar *pwd, gchar *home);
#if defined(USE_GTK2_GEOMETRY_METHOD) || defined(UNIT_TEST)
void keep_gtk2_window_size (struct Window *win_data, GtkWidget *vte, guint keep_vte_size);
#endif
#if defined(USE_GTK3_GEOMETRY_METHOD) || defined(UNIT_TEST)
void keep_gtk3_window_size(struct Window *win_data, gboolean idle);
gboolean show_or_hide_tabs_bar_and_scroll_bar();
gboolean idle_set_vte_font_to_selected(struct Window *win_data);
#endif
void dim_window(struct Window *win_data, gint dim_window);
void set_window_icon(GtkWidget *window);
gboolean window_quit(GtkWidget *window, GdkEvent *event, struct Window *win_data);
GString *close_multi_tabs(struct Window *win_data, int window_no);
gboolean display_child_process_dialog (GString *child_process_list, struct Window *win_data, gsize style);
GString *get_child_process_list(GtkWidget *window, gint window_no, gint page_no, GString *process_list, pid_t pid, struct Window *win_data, gboolean show_foreground);
void clean_process_data();
gboolean deal_key_press(GtkWidget *window, Key_Bindings type, struct Window *win_data);
#ifdef DISABLE_PAGE_ADDED
void notebook_page_added(GtkNotebook *notebook, GtkWidget *child, guint page_num, struct Window *win_data);
#endif
void show_close_button_on_tab(struct Window *win_data, struct Page *page_data);
void set_fill_tabs_bar(GtkNotebook *notebook, gboolean fill_tabs_bar, struct Page *page_data);
void remove_notebook_page (GtkNotebook *notebook, GtkWidget *child, guint page_num, struct Window *win_data, gboolean run_quit_gtk);
void update_window_hint(struct Window *win_data, struct Page *page_data);
void window_resizable(GtkWidget *window, GtkWidget *vte, Hints_Type hints_type);
gboolean hide_and_show_tabs_bar(struct Window *win_data , Switch_Type show_tabs_bar);
void set_widget_can_not_get_focus(GtkWidget *widget);
gboolean hide_scrollback_lines(GtkWidget *widget, struct Window *win_data);
#if defined(FATAL) || defined(UNIT_TEST)
void print_array(gchar *name, gchar **data);
#endif
void clear_win_data(struct Window *win_data);
gboolean confirm_to_paste_form_clipboard(Clipboard_Type type, struct Window *win_data, struct Page *page_data);
gboolean show_clipboard_dialog(Clipboard_Type type, struct Window *win_data,
			       struct Page *page_data, Dialog_Type_Flags dialog_type);

void print_color(gint no, gchar *name, GdkRGBA color);

//
// **************************** profile.c ****************************
//
void convert_system_color_to_rgba();
void init_page_parameters(struct Window *win_data, struct Page *page_data);
void init_user_color(struct Window *win_data, gchar *theme_name);
void init_locale_restrict_data(gchar *lc_messages);
GString *save_user_settings(GtkWidget *widget, struct Window *win_data);
gchar *get_user_profile_path(struct Window *win_data, int argc, char *argv[]);
void get_user_settings(struct Window *win_data, const gchar *encoding);
void init_prime_user_datas(struct Window *win_data);
void get_prime_user_settings(GKeyFile *keyfile, struct Window *win_data, gchar *encoding);
gboolean check_boolean_value(GKeyFile *keyfile, const gchar *group_name, const gchar *key, const gboolean default_value);
void check_profile_version (GKeyFile *keyfile, struct Window *win_data);
void profile_is_invalid_dialog(GError *error, struct Window *win_data);
void convert_string_to_user_key(gint i, gchar *value, struct Window *win_data);
gchar *get_profile();
#if defined(ENABLE_RGBA) || defined(UNIT_TEST)
void init_rgba(struct Window *win_data);
#endif
void get_row_and_column_from_geometry_str(glong *column, glong *row, glong *default_column, glong *default_row, gchar *geometry_str);

//
// **************************** property.c ****************************
//

void create_theme_color_data(GdkRGBA color[COLOR], GdkRGBA color_orig[COLOR], gdouble color_brightness, gboolean invert_color,
			     gboolean default_vte_theme, gboolean dim_fg_color);
void adjust_ansi_color(GdkRGBA *color, GdkRGBA *color_orig, gdouble color_brightness);
void generate_all_color_datas(struct Window *win_data);
GdkRGBA *get_current_color_theme(struct Window *win_data);
void init_new_page(struct Window *win_data, struct Page *page_data, glong column, glong row);
void set_cursor_blink(struct Window *win_data, struct Page *page_data);
void set_hyperlink(struct Window *win_data, struct Page *page_data);
void clean_hyperlink(struct Window *win_data, struct Page *page_data);
void enable_custom_cursor_color(GtkWidget *vte, gboolean custom_cursor_color, GdkRGBA *cursor_color);
void set_vte_color(GtkWidget *vte, gboolean default_vte_color, gboolean custom_cursor_color, GdkRGBA cursor_color, GdkRGBA color[COLOR],
		   gboolean update_fg_only, gboolean over_16_colors);
gboolean use_default_vte_theme(struct Window *win_data);
void set_page_width(struct Window *win_data, struct Page *page_data);
void pack_vte_and_scroll_bar_to_hbox(struct Window *win_data, struct Page *page_data);
void add_remove_page_timeout_id(struct Window *win_data, struct Page *page_data);
void add_remove_window_title_changed_signal(struct Page *page_data);
#if defined(ENABLE_VTE_BACKGROUND) || defined(FORCE_ENABLE_VTE_BACKGROUND) || defined(UNIT_TEST)
gboolean set_background_saturation(GtkRange *range, GtkScrollType scroll, gdouble value, GtkWidget *vte);
#endif
gboolean set_window_opacity (GtkRange *range, GtkScrollType scroll, gdouble value, struct Window *win_data);
#if defined(vte_terminal_get_padding) || defined(UNIT_TEST)
void fake_vte_terminal_get_padding(VteTerminal *vte, gint *width, gint *height);
#endif
void apply_new_win_data_to_page (struct Window *win_data_orig, struct Window *win_data, struct Page *page_data);
gboolean compare_color(GdkRGBA *a, GdkRGBA *b);
gboolean check_show_or_hide_scroll_bar(struct Window *win_data);
void show_and_hide_scroll_bar(struct Page *page_data, gboolean show_scroll_bar);
void set_widget_thickness(GtkWidget *widget, gint thickness);

//
// **************************** notebook.c ****************************
//

struct Page *add_page(struct Window *win_data,
		      struct Page *page_data_prev,
		      GtkWidget *menuitem_encoding,
		      gchar *encoding,
		      gchar *runtime_locale_encoding,
		      gchar *locale,
		      gchar *environments,
		      gchar *user_environ,
		      const gchar *VTE_CJK_WIDTH_STR,
		      gboolean add_to_next);
void dim_vte_text (struct Window *win_data, struct Page *page_data, gint dim_text);
gboolean close_page(GtkWidget *vte, gint close_type);
gboolean open_url_with_external_command (gchar *url, gint tag, struct Window *win_data, struct Page *page_data);
struct Page *get_page_data_from_nth_page(struct Window *win_data, guint page_no);

//
// **************************** font.c ****************************
//

void set_vte_font(GtkWidget *widget, Font_Set_Type type);
void apply_font_to_every_vte(GtkWidget *window, gchar *new_font_name, glong column, glong row);
gboolean check_if_every_vte_is_using_restore_font_name (struct Window *win_data);
void fake_vte_terminal_set_font_from_string(GtkWidget *vte, const char *font_name, gboolean anti_alias);

//
// **************************** pagename.c ****************************
//

void update_window_title(GtkWidget *window, gchar *name, gboolean window_title_append_package_name);
void init_monitor_cmdline_datas(struct Window *win_data, struct Page *page_data);
gboolean monitor_cmdline(struct Page *page_data);
void update_page_window_title (VteTerminal *vte, struct Page *page_data);
gchar *get_cmdline(const pid_t tpgid);
gboolean check_is_root(pid_t tpgid);
gint get_tpgid(pid_t pid);
void check_and_update_window_title(struct Window *win_data, gboolean custom_window_title, gint page_no,
				   gchar *custom_page_name, gchar *page_name);
gboolean get_and_update_page_name(struct Page *page_data, gboolean lost_focus);
void reorder_page_number (GtkNotebook *notebook, GtkWidget *child, guint page_num, GtkWidget *window);
gchar *get_tab_name_with_current_dir(pid_t pid);
void update_page_name_wintitle(StrAddr **page_name,
			       StrAddr **page_color,
			       struct Window *win_data,
			       struct Page *page_data);
void update_page_name_cmdline(StrAddr **page_name,
			      StrAddr **page_color,
			      struct Window *win_data,
			      struct Page *page_data);
void update_page_name_pwd(StrAddr **page_name,
			  StrAddr **page_color,
			  struct Window *win_data,
			  struct Page *page_data,
			  gboolean lost_focus);
gboolean update_page_name(GtkWidget *window, GtkWidget *vte, gchar *page_name, GtkWidget *label,
			  gint page_no, gchar *custom_page_name, const gchar *tab_color, gboolean is_root,
			  gboolean is_bold, gboolean show_encoding, gchar *encoding_str,
			  gboolean custom_window_title, gboolean lost_focus);
void update_page_name_normal(StrAddr **page_name,
			     StrAddr **page_color,
			     struct Window *win_data,
			     struct Page *page_data);

//
// **************************** dialog.c ****************************
//

GtkResponseType dialog(GtkWidget *widget, gsize style);
gint get_color_index(gboolean invert_color, gint index);
gboolean find_str_in_vte(GtkWidget *vte, Dialog_Find_Type type);
void set_markup_key_value(gboolean bold, gchar *color, gchar *key_value, GtkWidget *label);
gboolean check_and_add_locale_to_warned_locale_list(struct Window *win_data, gchar *new_locale);
void create_invalid_locale_error_message(gchar *locale);
void error_dialog(GtkWidget *window, gchar *title_translation, gchar *title,
		  gchar *icon, gchar *message, gchar *encoding);
#if defined(FATAL) || defined(UNIT_TEST)
void print_switch_out_of_range_error_dialog(gchar *function, gchar *var, gint value);
#endif
GdkRGBA get_inactive_color(GdkRGBA original_fg_color, gdouble new_brightness, gdouble old_brightness);
gboolean upgrade_dialog(gchar *version_str);
gchar *get_colorful_profile(struct Window *win_data);

//
// **************************** menu.c ****************************
//

void recreate_theme_menu_items(struct Window *win_data);
gboolean refresh_locale_and_encoding_list(struct Window *win_data);
void set_encoding(GtkWidget *menuitem, gpointer user_data);
gboolean create_menu(struct Window *win_data);
void set_urgent_bell(GtkWidget *widget, struct Window *win_data);
void set_vte_urgent_bell(struct Window *win_data, struct Page *page_data);
gboolean stop_urgency_hint(GtkWidget *window, GdkEvent  *event, struct Window *win_data);
GtkWidget *add_radio_menuitem_to_sub_menu(GSList *encoding_group,
					  GtkWidget *sub_menu,
					  const gchar *name,
					  GSourceFunc func,
					  gpointer func_data);
void refresh_profile_list (struct Window *win_data);
long get_profile_dir_modtime();
gboolean check_if_win_data_is_still_alive(struct Window *win_data);
void clean_scrollback_lines(GtkWidget *widget, struct Window *win_data);

#if defined(FATAL) || defined(UNIT_TEST)
void print_active_window_is_null_error_dialog(gchar *function);
#endif

#endif
