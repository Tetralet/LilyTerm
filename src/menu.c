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

#include "menu.h"

// Sometimes we may popup a menu on a non-active (not focus) window...
extern GList *window_list;
extern GtkWidget *menu_active_window;
extern struct Page_Color page_color[PAGE_COLOR];
extern struct GdkRGBA_Theme system_color_theme[THEME];
extern struct Erase_Binding erase_binding[ERASE_BINDING];
#ifdef ENABLE_CURSOR_SHAPE
extern struct Cursor_Shape cursor_shape[CURSOR_SHAPE];
#endif
extern gchar *system_locale_list;
extern gchar *init_LC_CTYPE;
extern gchar *init_encoding;
extern gchar *profile_dir;
extern gboolean safe_mode;

gboolean create_menu(struct Window *win_data)
{
#ifdef DETAIL
	g_debug("! Launch create_menu() for win_data %p", win_data);
#endif

#ifdef OUT_OF_MEMORY
#  undef g_strdup
#  undef g_strdup_printf
#  undef g_strsplit
#  undef g_strsplit_set
#endif
#ifdef SAFEMODE
	if (win_data==NULL) return FALSE;
#endif
	GtkWidget *sub_menu=NULL;
	gsize i=0;
	//gboolean enable_vte_cjk_width = win_data->VTE_CJK_WIDTH ? 1 : 0;
	//g_debug("enable_vte_cjk_width = %d, and VTE_CJK_WIDTH (environ) = %s",
	//	enable_vte_cjk_width, g_getenv("VTE_CJK_WIDTH"));
	win_data->menu = gtk_menu_new();

	win_data->encoding_locale_menuitems[0] = create_sub_item_subitem (_("Change text encoding"), GTK_FAKE_STOCK_DND);
	win_data->encoding_locale_menuitems[1] = create_sub_item_subitem (_("New tab"), GTK_FAKE_STOCK_DND_MULTIPLE);
	win_data->encoding_locale_menuitems[2] = create_sub_item_subitem (_("New tab with specified locale"),
									  GTK_FAKE_STOCK_DND_MULTIPLE);
	for (i=0; i<3; i++)
		gtk_menu_shell_append (GTK_MENU_SHELL (win_data->menu), win_data->encoding_locale_menuitems[i]);

	// g_debug("win_data->encoding_locale_menuitems is created for win_data (%p)!", win_data);

	if (refresh_locale_and_encoding_list(win_data)==FALSE) return FALSE;

	win_data->menuitem_new_window_from_list = create_sub_item_subitem (_("New window with specified profile"), GTK_FAKE_STOCK_NEW);
	// g_debug("Trying to append win_data->menuitem_new_window_from_list(%p) to win_data->menu(%p) for win_data = %p",
	//	win_data->menuitem_new_window_from_list, win_data->menu, win_data);
	gtk_menu_shell_append (GTK_MENU_SHELL (win_data->menu), win_data->menuitem_new_window_from_list);
	// win_data->subitem_new_window_from_list = create_sub_item_submenu (win_data->menu, win_data->menuitem_new_window_from_list);

	// ----------------------------------------
	add_separator_menu (win_data->menu);

	// Change the font for every tab
	create_menu_item (IMAGE_MENU_ITEM, win_data->menu, _("Change the font"), NULL, GTK_FAKE_STOCK_SELECT_FONT,
			  (GSourceFunc)select_font, win_data);

#ifdef ENABLE_GDKCOLOR_TO_STRING
	if (win_data->show_color_selection_menu)
	{
		// The submenu of Change color
		sub_menu = create_sub_item (win_data->menu, _("Change colors"), GTK_FAKE_STOCK_SELECT_COLOR);

		// Change the cursor color for every tab
		create_menu_item (IMAGE_MENU_ITEM, sub_menu, _("Change the cursor color"), NULL, GTK_FAKE_STOCK_SELECT_COLOR,
				  (GSourceFunc)dialog, GINT_TO_POINTER (CHANGE_THE_CURSOR_COLOR));

		// Change the foreground color for every tab
		win_data->ansi_theme_menuitem[0] = create_menu_item (IMAGE_MENU_ITEM, sub_menu, _("Change the foreground color"),
								     NULL, GTK_FAKE_STOCK_SELECT_COLOR, (GSourceFunc)dialog,
								     GINT_TO_POINTER (CHANGE_THE_FOREGROUND_COLOR));

		// Change the foreground color for every tab
		win_data->ansi_theme_menuitem[1] = create_menu_item (IMAGE_MENU_ITEM, sub_menu, _("Change the ANSI colors"),
								     NULL, GTK_FAKE_STOCK_SELECT_COLOR, (GSourceFunc)dialog,
								     GINT_TO_POINTER (CHANGE_THE_ANSI_COLORS));

		// Change the background color for every tab
		win_data->ansi_theme_menuitem[2] = create_menu_item (IMAGE_MENU_ITEM, sub_menu, _("Change the background color"),
								     NULL, GTK_FAKE_STOCK_SELECT_COLOR, (GSourceFunc)dialog,
								     GINT_TO_POINTER (CHANGE_THE_BACKGROUND_COLOR));

		// Change the background color for every tab
		win_data->ansi_theme_menuitem[3] = create_menu_item (IMAGE_MENU_ITEM, sub_menu, _("Adjust the brightness of ANSI colors"),
								     NULL, GTK_FAKE_STOCK_SELECT_COLOR, (GSourceFunc)dialog,
								     GINT_TO_POINTER (ADJUST_THE_BRIGHTNESS_OF_ANSI_COLORS_USED_IN_TERMINAL));

		win_data->ansi_theme_menuitem[4] = create_menu_item (IMAGE_MENU_ITEM, sub_menu, _("Adjust the brightness when inactive"),
								     NULL, GTK_FAKE_STOCK_SELECT_COLOR, (GSourceFunc)dialog,
								     GINT_TO_POINTER (ADJUST_THE_BRIGHTNESS_OF_ANSI_COLORS_WHEN_INACTIVE));

		if (win_data->use_color_page)
		{
			// ----------------------------------------
			GtkWidget *separator_menu = add_separator_menu (sub_menu);
			gboolean hide_separator_menu = TRUE;

			for (i=0; i<PAGE_COLOR; i++)
			{
				gchar *label_name = g_strdup_printf(_("Change the %s color on tab"),
								    page_color[i].comment);
				if (create_menu_item (IMAGE_MENU_ITEM, sub_menu, label_name,
						      NULL, GTK_FAKE_STOCK_SELECT_COLOR, (GSourceFunc)dialog,
						      GINT_TO_POINTER (CHANGE_THE_TEXT_COLOR_OF_WINDOW_TITLE+i)))
					hide_separator_menu = FALSE;
				g_free(label_name);
			}
			gtk_widget_set_no_show_all(separator_menu, hide_separator_menu);
		}
	}
#endif
	// The ansi theme
	win_data->ansi_color_sub_menu = create_sub_item_subitem (_("Change ANSI color theme"), GTK_FAKE_STOCK_SELECT_COLOR);
	gtk_menu_shell_append (GTK_MENU_SHELL (win_data->menu), win_data->ansi_color_sub_menu);
	recreate_theme_menu_items(win_data);

#if defined(ENABLE_VTE_BACKGROUND) || defined(FORCE_ENABLE_VTE_BACKGROUND)
	if (win_data->show_background_menu)
#else
	if (win_data->show_background_menu && (win_data->use_rgba == -1))
#endif
	{
		GtkWidget *background_sub_menu = create_sub_item (win_data->menu, _("Background"), GTK_FAKE_STOCK_PAGE_SETUP);
#ifdef ENABLE_RGBA
		if (win_data->use_rgba == -1)
		{
			// Transparent Window
			win_data->menuitem_trans_win = create_menu_item (CHECK_MENU_ITEM, background_sub_menu,
									 _("Transparent window"),
									 NULL, NULL, (GSourceFunc)set_trans_win,
									 win_data->window);

			// Window opacity
			create_menu_item (IMAGE_MENU_ITEM, background_sub_menu, _("Window opacity"), NULL, GTK_FAKE_STOCK_EXECUTE,
					  (GSourceFunc)dialog, GINT_TO_POINTER (CHANGE_THE_OPACITY_OF_WINDOW));

			create_menu_item (IMAGE_MENU_ITEM, background_sub_menu, _("Window opacity when inactive"),
					   NULL, GTK_FAKE_STOCK_EXECUTE, (GSourceFunc)dialog,
					   GINT_TO_POINTER (CHANGE_THE_OPACITY_OF_WINDOW_WHEN_INACTIVE));

			// ----------------------------------------
			add_separator_menu (background_sub_menu);
		}
#endif
#if defined(ENABLE_VTE_BACKGROUND) || defined(FORCE_ENABLE_VTE_BACKGROUND)
		// Transparent Background
		win_data->menuitem_trans_bg = create_menu_item (CHECK_MENU_ITEM, background_sub_menu, _("Transparent background"), NULL, NULL,
								(GSourceFunc)set_trans_bg, win_data);

		create_menu_item (IMAGE_MENU_ITEM, background_sub_menu, _("Background saturation"), NULL, GTK_FAKE_STOCK_EXECUTE,
				  (GSourceFunc)dialog, GINT_TO_POINTER (CHANGE_BACKGROUND_SATURATION));

		// Load background from file
		create_menu_item (IMAGE_MENU_ITEM, background_sub_menu, _("Set background image"), NULL, GTK_FAKE_STOCK_ABOUT,
								(GSourceFunc)load_background_image_from_file, win_data);
#endif
	}
	// ----------------------------------------
	add_separator_menu (win_data->menu);

	if (win_data->show_copy_paste_menu)
	{
		GtkWidget *clip_sub_menu = NULL;
		if (win_data->embedded_copy_paste_menu)
			clip_sub_menu = win_data->menu;
		else
			clip_sub_menu = create_sub_item (win_data->menu, _("Clipboard"), GTK_FAKE_STOCK_EDIT);

		if (win_data->enable_hyperlink)
			// copy_url
			win_data->menuitem_copy_url = create_menu_item (IMAGE_MENU_ITEM, clip_sub_menu, _("Copy URL"), NULL,
									GTK_FAKE_STOCK_COPY, (GSourceFunc)copy_url_clipboard,
									NULL);

		// copy
		win_data->menuitem_copy = create_menu_item (IMAGE_MENU_ITEM, clip_sub_menu, _("Copy"), NULL, GTK_FAKE_STOCK_COPY,
							    (GSourceFunc)copy_clipboard, win_data);

		// paste
		win_data->menuitem_paste = create_menu_item (IMAGE_MENU_ITEM, clip_sub_menu, _("Paste"), NULL, GTK_FAKE_STOCK_PASTE,
							     (GSourceFunc)paste_clipboard, win_data);

		// ----------------------------------------
		add_separator_menu (clip_sub_menu);

		win_data->menuitem_clipboard = create_menu_item (IMAGE_MENU_ITEM, clip_sub_menu,
								 _("View clipboard"), NULL, GTK_FAKE_STOCK_FILE,
								 (GSourceFunc)view_clipboard, win_data);

		win_data->menuitem_primary = create_menu_item (IMAGE_MENU_ITEM, clip_sub_menu,
							       _("View primary clipboard"), NULL, GTK_FAKE_STOCK_FILE,
							       (GSourceFunc)view_primary, win_data);

		// ----------------------------------------
		add_separator_menu (win_data->menu);

	}

	// use scrollback lines
	// g_debug("menuitem_hide_scroll_bar)->active = %d", win_data->scrollback_lines ? 1 : 0);
	// GTK_CHECK_MENU_ITEM(menuitem_hide_scroll_bar)->active = win_data->scrollback_lines ? 1 : 0;
	win_data->menuitem_hide_scroll_bar = create_menu_item (CHECK_MENU_ITEM, win_data->menu, _("Scrollback lines"), NULL, NULL,
							       (GSourceFunc)hide_scrollback_lines, win_data);
	// GTK_CHECK_MENU_ITEM(win_data->menuitem_hide_scroll_bar)->active = win_data->show_scroll_bar;
	gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(win_data->menuitem_hide_scroll_bar),
					check_show_or_hide_scroll_bar(win_data));

	// g_debug("create_menu(): check_show_or_hide_scroll_bar() = %d", check_show_or_hide_scroll_bar(win_data));

	// clean scrollback lines
	create_menu_item (IMAGE_MENU_ITEM, win_data->menu, _("Clean scrollback lines"), NULL, GTK_FAKE_STOCK_CLEAR,
			  (GSourceFunc)clean_scrollback_lines, win_data);

	// ----------------------------------------
	add_separator_menu (win_data->menu);

	GtkWidget *misc_sub_menu = create_sub_item (win_data->menu, _("Misc"), GTK_FAKE_STOCK_CONVERT);

	create_menu_item (IMAGE_MENU_ITEM, misc_sub_menu, _("Paste to every Vte Terminal"), NULL, GTK_FAKE_STOCK_PASTE,
							     (GSourceFunc)paste_to_every_vte, win_data);

	create_menu_item (IMAGE_MENU_ITEM, misc_sub_menu, _("Launch file manager"), NULL, GTK_FAKE_STOCK_PASTE,
							     (GSourceFunc)open_current_dir_with_file_manager, win_data);

	// ----------------------------------------
	add_separator_menu (misc_sub_menu);
	create_menu_item (IMAGE_MENU_ITEM, misc_sub_menu, _("View page info"), NULL, GTK_FAKE_STOCK_DIALOG_QUESTION,
							     (GSourceFunc)view_current_page_info, win_data);



	// ----------------------------------------
	add_separator_menu (misc_sub_menu);

	sub_menu = create_sub_item (misc_sub_menu, _("Erase Binding"), GTK_FAKE_STOCK_GO_BACK);
	GSList *erase_binding_group = NULL;
	for (i=0; i<ERASE_BINDING; i++)
	{
		win_data->menuitem_erase_binding[i] = add_radio_menuitem_to_sub_menu (erase_binding_group,
									      sub_menu,
									      erase_binding[i].name,
									      (GSourceFunc)set_erase_binding,
									      GINT_TO_POINTER(erase_binding[i].value));
		erase_binding_group = gtk_radio_menu_item_get_group (GTK_RADIO_MENU_ITEM (win_data->menuitem_erase_binding[i]));
		if (win_data->erase_binding == erase_binding[i].value)
			win_data->current_menuitem_erase_binding = win_data->menuitem_erase_binding[i];
	}

#ifdef ENABLE_CURSOR_SHAPE
	sub_menu = create_sub_item (misc_sub_menu, _("Cursor Shape"), GTK_FAKE_STOCK_GO_BACK);
	GSList *cursor_shape_group = NULL;
	for (i=0; i<CURSOR_SHAPE; i++)
	{
		win_data->menuitem_cursor_shape[i] = add_radio_menuitem_to_sub_menu (cursor_shape_group,
									      sub_menu,
									      cursor_shape[i].name,
									      (GSourceFunc)set_cursor_shape,
									      GINT_TO_POINTER(cursor_shape[i].value));
		cursor_shape_group = gtk_radio_menu_item_get_group (GTK_RADIO_MENU_ITEM (win_data->menuitem_cursor_shape[i]));
		if (win_data->cursor_shape == cursor_shape[i].value)
			win_data->current_menuitem_cursor_shape = win_data->menuitem_cursor_shape[i];
	}
#endif

	// Dim when inactive
	win_data->ansi_theme_menuitem[5] = win_data->menuitem_dim_text = create_menu_item (CHECK_MENU_ITEM, misc_sub_menu,
											   _("Dim text when inactive"), NULL, NULL,
											   (GSourceFunc)set_dim_text, win_data);
#ifdef ENABLE_RGBA
	if (win_data->use_rgba == -1)
		win_data->menuitem_dim_window = create_menu_item (CHECK_MENU_ITEM, misc_sub_menu,
								  _("Dim window when inactive"), NULL, NULL,
								  (GSourceFunc)set_dim_window, win_data);
#endif

	// Cursor Blinks
	win_data->menuitem_cursor_blinks = create_menu_item (CHECK_MENU_ITEM, misc_sub_menu, _("Cursor blinks"), NULL, NULL,
							     (GSourceFunc)set_cursor_blinks, win_data);

	// Bold text
	win_data->menuitem_allow_bold_text = create_menu_item (CHECK_MENU_ITEM, misc_sub_menu, _("Allow bold text"), NULL, NULL,
							       (GSourceFunc)set_allow_bold_text, win_data);

	// ----------------------------------------
	add_separator_menu (misc_sub_menu);

	// Need <Ctrl> pressed to open URL
	win_data->menuitem_open_url_with_ctrl_pressed = create_menu_item (CHECK_MENU_ITEM, misc_sub_menu, _("Need <Ctrl> pressed to open URL"), NULL, NULL,
									  (GSourceFunc)set_open_url_with_ctrl_pressed, win_data);

	win_data->menuitem_disable_url_when_ctrl_pressed = create_menu_item (CHECK_MENU_ITEM, misc_sub_menu, _("Disable URL when <Ctrl> pressed"), NULL, NULL,
									  (GSourceFunc)set_disable_url_when_ctrl_pressed, win_data);

	// ----------------------------------------
	add_separator_menu (misc_sub_menu);

	// Audible bell
	win_data->menuitem_audible_bell = create_menu_item (CHECK_MENU_ITEM, misc_sub_menu, _("Audible bell"), NULL, NULL,
								(GSourceFunc)set_audible_bell, win_data);
#if defined(ENABLE_VISIBLE_BELL) || defined(UNIT_TEST)
	// Visible bell
	win_data->menuitem_visible_bell = create_menu_item (CHECK_MENU_ITEM, misc_sub_menu, _("Visible bell"), NULL, NULL,
								(GSourceFunc)set_visible_bell, win_data);
#endif
#ifdef ENABLE_BEEP_SINGAL
	// Urgent bell
	win_data->menuitem_urgent_bell = create_menu_item (CHECK_MENU_ITEM, misc_sub_menu, _("Urgent bell"), NULL, NULL,
								(GSourceFunc)set_urgent_bell, win_data);
#endif
	// ----------------------------------------
	add_separator_menu (misc_sub_menu);

#ifdef ENABLE_IM_APPEND_MENUITEMS
	// Input Method
	if (win_data->show_input_method_menu)
	{
		sub_menu = create_sub_item (misc_sub_menu, _("Switch input methods"), GTK_FAKE_STOCK_INDEX);
		vte_terminal_im_append_menuitems (VTE_TERMINAL(win_data->current_vte), GTK_MENU_SHELL (sub_menu));
	}
#endif
	// Edit tab name
	if (win_data->show_change_page_name_menu)
		create_menu_item (IMAGE_MENU_ITEM, misc_sub_menu, _("Rename this tab"), NULL, GTK_FAKE_STOCK_EDIT,
			  (GSourceFunc)dialog, GINT_TO_POINTER(EDIT_LABEL));

	// ----------------------------------------
#ifdef ENABLE_IM_APPEND_MENUITEMS
	if ((win_data->show_input_method_menu || win_data->show_change_page_name_menu) &&
#else
	if (win_data->show_change_page_name_menu &&
#endif
	    win_data->show_resize_menu)
		add_separator_menu (misc_sub_menu);

	// Show the tabs bar
	win_data->menuitem_show_tabs_bar = create_menu_item (IMAGE_MENU_ITEM, misc_sub_menu, _("Show the tabs bar"), NULL,
							     GTK_FAKE_STOCK_GOTO_BOTTOM,
							     (GSourceFunc)launch_hide_and_show_tabs_bar,
							     GINT_TO_POINTER(TEMPORARY_ON));

	// Hide the tabs bar
	win_data->menuitem_hide_tabs_bar = create_menu_item (IMAGE_MENU_ITEM, misc_sub_menu, _("Hide the tabs bar"), NULL,
							     GTK_FAKE_STOCK_GOTO_TOP,
							     (GSourceFunc)launch_hide_and_show_tabs_bar,
							     GINT_TO_POINTER(TEMPORARY_OFF));

	// Always shows the tabs bar
	win_data->menuitem_always_show_tabs_bar = create_menu_item (IMAGE_MENU_ITEM, misc_sub_menu,
								   _("Always show the tabs bar"),
								   NULL, GTK_FAKE_STOCK_GOTO_BOTTOM,
								   (GSourceFunc)launch_hide_and_show_tabs_bar,
								   GINT_TO_POINTER(FORCE_ON));

	// Always hide the tabs bar
	win_data->menuitem_always_hide_tabs_bar = create_menu_item (IMAGE_MENU_ITEM, misc_sub_menu,
								   _("Always hide the tabs bar"),
								   NULL, GTK_FAKE_STOCK_GOTO_TOP,
								   (GSourceFunc)launch_hide_and_show_tabs_bar,
								   GINT_TO_POINTER(FORCE_OFF));

	if (win_data->show_resize_menu)
	{
		add_separator_menu (misc_sub_menu);

		// Window Size
		create_menu_item (IMAGE_MENU_ITEM, misc_sub_menu, _("Increase window size"), NULL, GTK_FAKE_STOCK_ZOOM_IN,
				  (GSourceFunc)set_vte_font, GINT_TO_POINTER(FONT_ZOOM_OUT));

		create_menu_item (IMAGE_MENU_ITEM, misc_sub_menu, _("Decrease window size"), NULL, GTK_FAKE_STOCK_ZOOM_OUT,
				  (GSourceFunc)set_vte_font, GINT_TO_POINTER(FONT_ZOOM_IN));

		// ----------------------------------------
		add_separator_menu (misc_sub_menu);

		// Reset font and window size
		create_menu_item (IMAGE_MENU_ITEM, misc_sub_menu, _("Reset to user custom font/size"), NULL, GTK_FAKE_STOCK_ZOOM_100,
				  (GSourceFunc)set_vte_font, GINT_TO_POINTER(FONT_RESET_DEFAULT));

		create_menu_item (IMAGE_MENU_ITEM, misc_sub_menu, _("Reset to system font/size"), NULL, GTK_FAKE_STOCK_ZOOM_100,
				  (GSourceFunc)set_vte_font, GINT_TO_POINTER(FONT_RESET_SYSTEM));
	}

	// Reset
	create_menu_item (IMAGE_MENU_ITEM, win_data->menu, _("Reset the content"), NULL, GTK_FAKE_STOCK_REFRESH,
			  (GSourceFunc)reset_vte, win_data);

	// ----------------------------------------
	add_separator_menu (win_data->menu);

	// Usage
	create_menu_item (IMAGE_MENU_ITEM, win_data->menu, _("Usage"), NULL, GTK_FAKE_STOCK_HELP,
				  (GSourceFunc)dialog, GINT_TO_POINTER(USAGE_MESSAGE));

	// Get the key value for using in profile
	create_menu_item (IMAGE_MENU_ITEM, win_data->menu, _("Set key binding"), NULL, GTK_FAKE_STOCK_PREFERENCES,
				  (GSourceFunc)dialog, GINT_TO_POINTER(SET_KEY_BINDING));


	win_data->menuitem_load_profile_from_list = create_sub_item_subitem (_("User profile"), GTK_FAKE_STOCK_PROPERTIES);
	gtk_menu_shell_append (GTK_MENU_SHELL (win_data->menu), win_data->menuitem_load_profile_from_list);
	// win_data->subitem_load_profile_from_list = create_sub_item_submenu (win_data->menu, win_data->menuitem_load_profile_from_list);
	refresh_profile_list(win_data);

	if (win_data->show_exit_menu)
	{
		// Exit LilyTerm
		gchar *exit_str = g_strdup_printf(_("Quit %s"), PACKAGE);
		create_menu_item (IMAGE_MENU_ITEM, win_data->menu, exit_str, NULL, GTK_FAKE_STOCK_QUIT,
				  (GSourceFunc)main_quit, win_data);
		g_free(exit_str);
	}

	gtk_widget_show_all(win_data->menu);

#ifdef OUT_OF_MEMORY
	#define g_strdup fake_g_strdup
	#define g_strdup_printf(...) NULL
	#define g_strsplit fake_g_strsplit
	#define g_strsplit_set(x,y,z) NULL
#endif

	return TRUE;
}

void recreate_theme_menu_items(struct Window *win_data)
{
#ifdef DETAIL
	g_debug("! Launch recreate_theme_menu_items() with win_data = %p", win_data);
#endif
#ifdef SAFEMODE
	if (win_data==NULL) return;
#endif
	if (win_data->ansi_color_menuitem)
	{
		gtk_widget_destroy(win_data->ansi_color_menuitem);
		// g_debug("recreate_theme_menu_items(): set win_data->current_menuitem_theme = NULL");
		win_data->current_menuitem_theme = NULL;
	}
	win_data->ansi_color_menuitem = gtk_menu_new ();
#ifdef SAFEMODE
	if (win_data->ansi_color_sub_menu)
#endif
		gtk_menu_item_set_submenu (GTK_MENU_ITEM (win_data->ansi_color_sub_menu), win_data->ansi_color_menuitem);

	win_data->ansi_theme_menuitem[6] = win_data->menuitem_invert_color = create_menu_item(CHECK_MENU_ITEM, win_data->ansi_color_menuitem,
											      _("Invert color"), NULL, NULL,
											      (GSourceFunc)invert_color_theme, win_data);
	add_separator_menu (win_data->ansi_color_menuitem);

	gint i;
	GSList *theme_group = NULL;
	// g_debug("win_data->color_theme_str = %s", win_data->color_theme_str);
	for (i=0; i<THEME; i++)
		theme_group = create_theme_menu_items(win_data, win_data->ansi_color_menuitem, theme_group, i, 0);
	if (win_data->have_custom_color || win_data->use_custom_theme)
	{
		add_separator_menu (win_data->ansi_color_menuitem);
		for (i=0; i<THEME; i++)
			theme_group = create_theme_menu_items(win_data, win_data->ansi_color_menuitem, theme_group, i, 1);
	}
	gtk_widget_show_all(win_data->ansi_color_menuitem);
}

GSList *create_theme_menu_items(struct Window *win_data, GtkWidget *sub_menu, GSList *theme_group, gint current_theme, gint custom_theme)
{
#ifdef DETAIL
	g_debug("! Launch create_theme_menu_items() with win_data = %p, sub_menu = %p, theme_group = %p, current_theme = %d, custom_theme = %d",
		win_data, sub_menu, theme_group, current_theme, custom_theme);
#endif
#ifdef SAFEMODE
	if (win_data==NULL) return NULL;
#endif
	gchar *item_label = NULL;
	switch (current_theme)
	{
		case THEME-2:
			item_label = _("grayscale");
			break;
		case THEME-1:
			item_label = _("(build-in)");
			break;
		default:
			item_label = system_color_theme[current_theme].name;
	}

	gint index = system_color_theme[current_theme].index;
	if (custom_theme)
		index = win_data->custom_color_theme[current_theme].index;

	// don't create 256/true colors + custom
	if (index == (2*THEME-1)) return theme_group;

	if (custom_theme) item_label = g_strdup_printf(_("%s + custom"), item_label);

	win_data->menuitem_theme[current_theme + custom_theme*THEME] = add_radio_menuitem_to_sub_menu (theme_group,
												       sub_menu,
												       item_label,
												       (GSourceFunc)select_ansi_theme,
												       GINT_TO_POINTER(index));
#ifdef SAFEMODE
	if (win_data->menuitem_theme[current_theme]==NULL) return NULL;
#endif
		theme_group = gtk_radio_menu_item_get_group (GTK_RADIO_MENU_ITEM (win_data->menuitem_theme[current_theme]));

	// g_debug("Checking win_data->color_theme_str (%s) and color_theme[current_theme].name (%s)",
	//	win_data->color_theme_str, color_theme[current_theme].name);

	if (win_data->current_menuitem_theme==NULL)
	{
		// g_debug("custom_theme = %d, win_data->use_custom_theme = %d, win_data->color_theme_index = %d, current_theme = %d",
		// 	custom_theme, win_data->use_custom_theme, win_data->color_theme_index, current_theme);
		if ((custom_theme == win_data->use_custom_theme) && (win_data->color_theme_index == current_theme))
			win_data->current_menuitem_theme = win_data->menuitem_theme[current_theme + custom_theme*THEME];
	}
	// g_debug("Got win_data->current_menuitem_theme = %p (%d), created (%p)",
	//	win_data->current_menuitem_theme, current_theme, win_data->menuitem_theme[current_theme + custom_theme*THEME]);

	// g_debug("Set the color theme %s (%p) to %p...",
	//	system_color_theme[current_theme].name, win_data->menuitem_theme[current_theme], system_color_theme[current_theme].color);

	if (current_theme==0)
		gtk_widget_set_name(win_data->menuitem_theme[current_theme], "");

	if (custom_theme) g_free(item_label);

	return theme_group;
}

void clean_scrollback_lines(GtkWidget *widget, struct Window *win_data)
{
#ifdef DETAIL
	g_debug("! Launch clean_scrollback_lines()!");
#endif
#ifdef SAFEMODE
	if ((win_data==NULL) || (win_data->current_vte==NULL)) return;
#endif
	vte_terminal_set_scrollback_lines (VTE_TERMINAL(win_data->current_vte), 0);
	vte_terminal_set_scrollback_lines (VTE_TERMINAL(win_data->current_vte), win_data->scrollback_lines);
}

void copy_url_clipboard(GtkWidget *widget, gpointer user_data)
{
#ifdef DETAIL
	g_debug("! Launch copy_url_clipboard()!");
#endif
#ifdef SAFEMODE
	if (widget==NULL) return;
#endif
	const gchar *widget_name = gtk_widget_get_name(widget);
	if (widget_name)
	{
		extern GtkClipboard *selection_clipboard;
		gtk_clipboard_set_text(selection_clipboard, widget_name, -1);
	}
}

void copy_clipboard(GtkWidget *widget, struct Window *win_data)
{
#ifdef DETAIL
	g_debug("! Launch copy_clipboard()!");
#endif
#ifdef SAFEMODE
	if ((win_data==NULL) || (win_data->current_vte==NULL)) return;
#endif
	vte_terminal_copy_clipboard(VTE_TERMINAL(win_data->current_vte));
}

void paste_clipboard(GtkWidget *widget, struct Window *win_data)
{
#ifdef DETAIL
	g_debug("! Launch paste_clipboard()!");
#endif
#ifdef SAFEMODE
	if (win_data==NULL) return;
#endif
	// vte_terminal_paste_clipboard(VTE_TERMINAL(win_data->current_vte));
	deal_key_press(win_data->window, KEY_PASTE_CLIPBOARD, win_data);
}

void paste_to_every_vte(GtkWidget *widget, struct Window *win_data)
{
#ifdef DETAIL
	g_debug("! Launch paste_clipboard()!");
#endif
#ifdef SAFEMODE
	if (win_data==NULL) return;
#endif
	gint type = PASTE_TEXTS_TO_EVERY_VTE_TERMINAL;
	while (dialog(NULL, type)==GTK_RESPONSE_OK)
	{
		if (type == PASTE_TEXTS_TO_EVERY_VTE_TERMINAL)
			type = PASTE_GRABBED_KEY_TO_EVERY_VTE_TERMINAL;
		else
			type = PASTE_TEXTS_TO_EVERY_VTE_TERMINAL;
	}
}

void open_current_dir_with_file_manager(GtkWidget *widget, struct Window *win_data)
{
#ifdef DETAIL
	g_debug("! Launch open_current_dir_with_file_manager() with win_data = %p!", win_data);
#endif
#ifdef SAFEMODE
	if ((win_data==NULL) || (win_data->current_vte==NULL)) return;
#endif
	struct Page *page_data = (struct Page *)g_object_get_data(G_OBJECT(win_data->current_vte), "Page_Data");
#ifdef SAFEMODE
	if (page_data==NULL) return;
#endif
	open_url_with_external_command (page_data->pwd, TAG_FILE, win_data, page_data);
}

void view_current_page_info(GtkWidget *widget, struct Window *win_data)
{
#ifdef DETAIL
	g_debug("! Launch view_current_page_info() with win_data = %p!", win_data);
#endif
#ifdef SAFEMODE
	if ((win_data==NULL) || (win_data->current_vte==NULL)) return;
#endif
	struct Page *page_data = (struct Page *)g_object_get_data(G_OBJECT(win_data->current_vte), "Page_Data");
#ifdef SAFEMODE
	if (page_data==NULL) return;
#endif
	gchar *old_temp_data = win_data->temp_data;
	win_data->temp_data = g_strdup_printf("%s\x10%s\x10"
					      "Encoding=%s\n"
					      "VTE_CJK_WIDTH=%s\n"
					      "WINDOWID=%ld",
					      _("View current page information"), "View current page information",
					      page_data->encoding_str,
					      page_data->VTE_CJK_WIDTH_STR,
					      (gtk_widget_get_window (page_data->vte))?GDK_WINDOW_XID (gtk_widget_get_window (page_data->vte)):0);
	dialog(NULL, GENERAL_INFO);
	g_free(win_data->temp_data);
	win_data->temp_data=old_temp_data;

}

void view_clipboard(GtkWidget *widget, struct Window *win_data)
{
#ifdef DETAIL
	g_debug("! Launch paste_clipboard()!");
#endif
#ifdef SAFEMODE
	if (win_data==NULL) return;
#endif
	show_clipboard_dialog(SELECTION_CLIPBOARD, win_data, NULL, GENERAL_INFO);
}

void view_primary(GtkWidget *widget, struct Window *win_data)
{
#ifdef DETAIL
	g_debug("! Launch paste_clipboard()!");
#endif
#ifdef SAFEMODE
	if (win_data==NULL) return;
#endif
	show_clipboard_dialog(SELECTION_PRIMARY, win_data, NULL, GENERAL_INFO);
}

void set_dim_text(GtkWidget *menuitem_dim_text, struct Window *win_data)
{
#ifdef DETAIL
	g_debug("! Launch set_dim_text() with win_data = %p", win_data);
#endif
#ifdef SAFEMODE
	if ((menuitem_dim_text==NULL) || (win_data==NULL)) return;
#endif
	// win_data->dim_text = GTK_CHECK_MENU_ITEM(menuitem_dim_text)->active;
	win_data->dim_text = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem_dim_text));
	dim_vte_text(win_data, NULL, 2);
}

#ifdef ENABLE_RGBA
void set_dim_window(GtkWidget *menuitem_dim_text, struct Window *win_data)
{
#  ifdef DETAIL
	g_debug("! Launch set_dim_window() with win_data = %p", win_data);
#  endif
#  ifdef SAFEMODE
	if (win_data==NULL) return;
	if (win_data->menuitem_dim_window==NULL) return;
#  endif
	// win_data->dim_window = GTK_CHECK_MENU_ITEM(win_data->menuitem_dim_window)->active;
	win_data->dim_window = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(win_data->menuitem_dim_window));
	dim_window(win_data, 2);
}
#endif

void set_cursor_blinks(GtkWidget *menuitem_cursor_blinks, struct Window *win_data)
{
#ifdef DETAIL
	g_debug("! Launch set_cursor_blinks() with win_data = %p", win_data);
#endif
#ifdef SAFEMODE
	if ((menuitem_cursor_blinks==NULL) || (win_data==NULL)) return;
#endif
#ifdef USE_NEW_VTE_CURSOR_BLINKS_MODE
	// win_data->cursor_blinks = GTK_CHECK_MENU_ITEM(menuitem_cursor_blinks)->active ?
	//			  VTE_CURSOR_BLINK_ON : VTE_CURSOR_BLINK_OFF;
	win_data->cursor_blinks = gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM(menuitem_cursor_blinks)) ?
				  VTE_CURSOR_BLINK_ON : VTE_CURSOR_BLINK_OFF;
#else
	// vte_terminal_set_cursor_blinks is deprecated since vte 0.17.1
	// win_data->cursor_blinks = GTK_CHECK_MENU_ITEM(menuitem_cursor_blinks)->active;
	win_data->cursor_blinks = gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM(menuitem_cursor_blinks));
#endif

	gint i;
	struct Page *page_data = NULL;
	for (i=0; i<gtk_notebook_get_n_pages(GTK_NOTEBOOK(win_data->notebook)); i++)
	{
		page_data = get_page_data_from_nth_page(win_data, i);
#ifdef SAFEMODE
		if (page_data)
#endif
			set_cursor_blink(win_data, page_data);
	}
}

void set_allow_bold_text(GtkWidget *menuitem_allow_bold_text, struct Window *win_data)
{
#ifdef DETAIL
	g_debug("! Launch set_allow_bold_text() with win_data = %p", win_data);
#endif
#ifdef SAFEMODE
	if ((menuitem_allow_bold_text==NULL) || (win_data==NULL)) return;
#endif
	win_data->allow_bold_text = gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM(menuitem_allow_bold_text));

	gint i;
	struct Page *page_data = NULL;
	for (i=0; i<gtk_notebook_get_n_pages(GTK_NOTEBOOK(win_data->notebook)); i++)
	{
		page_data = get_page_data_from_nth_page(win_data, i);
#ifdef SAFEMODE
		if (page_data)
#endif
			vte_terminal_set_allow_bold(VTE_TERMINAL(page_data->vte), win_data->allow_bold_text);
	}
}

void set_open_url_with_ctrl_pressed(GtkWidget *menuitem_open_url_with_ctrl_pressed, struct Window *win_data)
{
#ifdef DETAIL
	g_debug("! Launch set_open_url_with_ctrl_pressed() with win_data = %p", win_data);
#endif
#ifdef SAFEMODE
	if ((menuitem_open_url_with_ctrl_pressed==NULL) || (win_data==NULL)) return;
#endif
	win_data->open_url_with_ctrl_pressed = gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM(menuitem_open_url_with_ctrl_pressed));
	if (win_data->open_url_with_ctrl_pressed) win_data->disable_url_when_ctrl_pressed = FALSE;
}

void set_disable_url_when_ctrl_pressed(GtkWidget *menuitem_disable_url_when_ctrl_pressed, struct Window *win_data)
{
#ifdef DETAIL
	g_debug("! Launch set_disable_url_when_ctrl_pressed() with win_data = %p", win_data);
#endif
#ifdef SAFEMODE
	if ((menuitem_disable_url_when_ctrl_pressed==NULL) || (win_data==NULL)) return;
#endif
	win_data->disable_url_when_ctrl_pressed = gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM(menuitem_disable_url_when_ctrl_pressed));
	if (win_data->disable_url_when_ctrl_pressed) win_data->open_url_with_ctrl_pressed = FALSE;
	// g_debug("set_disable_url_when_ctrl_pressed(): set win_data->disable_url_when_ctrl_pressed = %d", win_data->disable_url_when_ctrl_pressed);
}

void set_audible_bell(GtkWidget *menuitem_audible_bell, struct Window *win_data)
{
#ifdef DETAIL
	g_debug("! Launch set_audible_bell() with win_data = %p", win_data);
#endif
#ifdef SAFEMODE
	if ((menuitem_audible_bell==NULL) || (win_data==NULL)) return;
#endif
	// win_data->audible_bell = GTK_CHECK_MENU_ITEM(menuitem_audible_bell)->active;
	win_data->audible_bell = gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM(menuitem_audible_bell));
	gint i;
	struct Page *page_data = NULL;

	for (i=0; i<gtk_notebook_get_n_pages(GTK_NOTEBOOK(win_data->notebook)); i++)
	{
		page_data = get_page_data_from_nth_page(win_data, i);
#ifdef SAFEMODE
		if (page_data)
#endif
			vte_terminal_set_audible_bell (VTE_TERMINAL(page_data->vte), win_data->audible_bell);
	}
}

#if defined(ENABLE_VISIBLE_BELL) || defined(UNIT_TEST)
void set_visible_bell(GtkWidget *menuitem_visible_bell, struct Window *win_data)
{
#ifdef DETAIL
	g_debug("! Launch set_visible_bell() with win_data = %p", win_data);
#endif
#ifdef SAFEMODE
	if ((menuitem_visible_bell==NULL) || (win_data==NULL)) return;
#endif
	// win_data->visible_bell = GTK_CHECK_MENU_ITEM(menuitem_visible_bell)->active;
	win_data->visible_bell = gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM(menuitem_visible_bell));
	gint i;
	struct Page *page_data = NULL;

	for (i=0; i<gtk_notebook_get_n_pages(GTK_NOTEBOOK(win_data->notebook)); i++)
	{
		page_data = get_page_data_from_nth_page(win_data, i);
#ifdef SAFEMODE
		if (page_data)
#endif
			vte_terminal_set_visible_bell (VTE_TERMINAL(page_data->vte), win_data->visible_bell);
	}
}
#endif

#if defined(ENABLE_BEEP_SINGAL) || defined(UNIT_TEST)
void set_urgent_bell(GtkWidget *menuitem_urgent_bell, struct Window *win_data)
{
#ifdef DETAIL
	g_debug("! Launch set_urgent_bell() with win_data = %p", win_data);
#endif
#ifdef SAFEMODE
	if ((win_data==NULL) || (win_data->notebook==NULL)) return;
#endif
	// win_data->urgent_bell = GTK_CHECK_MENU_ITEM(menuitem_urgent_bell)->active;
	if (menuitem_urgent_bell)
	{
		win_data->urgent_bell = gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM(menuitem_urgent_bell));

		// Don't do urgent bell if settings is not changed,
		if (win_data->urgent_bell_status == win_data->urgent_bell)
			return;
	}

	gint i;
	struct Page *page_data = NULL;

	for (i=0; i<gtk_notebook_get_n_pages(GTK_NOTEBOOK(win_data->notebook)); i++)
	{
		page_data = get_page_data_from_nth_page(win_data, i);
#ifdef SAFEMODE
		if (page_data)
#endif
			set_vte_urgent_bell(win_data, page_data);
	}
	win_data->urgent_bell_status = win_data->urgent_bell;
}

void set_vte_urgent_bell(struct Window *win_data, struct Page *page_data)
{
#ifdef DETAIL
	g_debug("! Launch set_urgent_bell() with win_data = %p, page_data = %p", win_data, page_data);
#endif
#ifdef SAFEMODE
	if ((win_data==NULL) || (page_data==NULL)) return;
#endif
	extern gboolean menu_activated;
	extern gint dialog_activated;
	// g_debug("win_data->urgent_bell = %d, win_data->lost_focus = %d, menu_activated = %d, dialog_activated = %d",
	//	win_data->urgent_bell, win_data->lost_focus, menu_activated, dialog_activated);

	if (win_data->urgent_bell && win_data->lost_focus && (menu_activated == FALSE) && (dialog_activated == FALSE))
	{
		// g_debug("set_vte_urgent_bell: get page_data->urgent_bell_handler_id = %ld", page_data->urgent_bell_handler_id);
		if (page_data->urgent_bell_handler_id==0)
			page_data->urgent_bell_handler_id = g_signal_connect(G_OBJECT(page_data->vte), "beep",
									     G_CALLBACK(urgent_beep), page_data);
	}
	else
	{
		if (page_data->urgent_bell_handler_id)
			g_signal_handler_disconnect(G_OBJECT(page_data->vte), page_data->urgent_bell_handler_id);
		page_data->urgent_bell_handler_id = 0;
	}
}

void urgent_beep(GtkWidget *window, struct Page *page_data)
{
#ifdef DETAIL
	g_debug("! Launch urgent_beep() with page_data = %p", page_data);
#endif
#ifdef SAFEMODE
	if ((page_data==NULL) || (page_data->window==NULL)) return;
#endif
	struct Window *win_data = (struct Window *)g_object_get_data(G_OBJECT(page_data->window), "Win_Data");
#ifdef SAFEMODE
	if (win_data==NULL) return;
#endif
	if (win_data->urgent_bell_focus_in_event_id==0)
	{
		// g_debug("Create win_data->vte focus-in-event event!");
		win_data->urgent_bell_focus_in_event_id = g_signal_connect(G_OBJECT(win_data->window), "focus-in-event",
									   G_CALLBACK(stop_urgency_hint), win_data);
		if (gtk_window_get_urgency_hint(GTK_WINDOW(win_data->window))==FALSE)
			gtk_window_set_urgency_hint(GTK_WINDOW(win_data->window), TRUE);
	}
}

gboolean stop_urgency_hint(GtkWidget *window, GdkEvent *event, struct Window *win_data)
{
#ifdef DETAIL
	g_debug("! Launch stop_urgency_hint() with win_data= %p", win_data);
#endif
#ifdef SAFEMODE
	if (win_data==NULL) return FALSE;
#endif
	if (win_data->urgent_bell_focus_in_event_id)
	{
		if (gtk_window_get_urgency_hint(GTK_WINDOW(win_data->window)))
			gtk_window_set_urgency_hint(GTK_WINDOW(win_data->window), FALSE);
		g_signal_handler_disconnect(G_OBJECT(win_data->window), win_data->urgent_bell_focus_in_event_id);
		win_data->urgent_bell_focus_in_event_id = 0;
	}

	return FALSE;
}
#endif

void launch_hide_and_show_tabs_bar(GtkWidget *widget, Switch_Type show_tabs_bar)
{
#ifdef DETAIL
	g_debug("! Launch launch_hide_and_show_tabs_bar() with show_tabs_bar = %d", show_tabs_bar);
#endif
#ifdef FATAL
	// g_debug("menu_active_window = %p", menu_active_window);
	if (menu_active_window==NULL)
		return print_active_window_is_null_error_dialog("launch_hide_and_show_tabs_bar()");
#endif
#ifdef SAFEMODE
	if (menu_active_window==NULL) return;
#endif
	struct Window *win_data = (struct Window *)g_object_get_data(G_OBJECT(menu_active_window), "Win_Data");
#ifdef SAFEMODE
	if (win_data==NULL) return;
#endif
	gint total_page = gtk_notebook_get_n_pages(GTK_NOTEBOOK(win_data->notebook));
	switch(show_tabs_bar)
	{
		case TEMPORARY_ON:
			if (total_page > 1)
				win_data->show_tabs_bar = AUTOMATIC;
			else
				win_data->show_tabs_bar = TEMPORARY_ON;
			break;
		case TEMPORARY_OFF:
			if (total_page > 1)
				win_data->show_tabs_bar = TEMPORARY_OFF;
			else
				win_data->show_tabs_bar = AUTOMATIC;
			break;
		case FORCE_ON:
			win_data->show_tabs_bar = FORCE_ON;
			break;
		case FORCE_OFF:
			win_data->show_tabs_bar = FORCE_OFF;
			break;
		default:
#  ifdef FATAL
			print_switch_out_of_range_error_dialog("launch_hide_and_show_tabs_bar",
							       "show_tabs_bar",
							       show_tabs_bar);
#  endif
			break;
	}
	// g_debug("launch_hide_and_show_tabs_bar(): call hide_and_show_tabs_bar() with show_tabs_bar = %d", show_tabs_bar);
	hide_and_show_tabs_bar(win_data , show_tabs_bar);
}

GtkWidget *check_name_in_menuitem(GtkWidget *sub_menu, const gchar *name, gboolean case_sensitive)
{
#ifdef DETAIL
	g_debug("! Launch check_name_in_menuitem() with name = %s", name);
#endif
#ifdef OUT_OF_MEMORY
#  undef gtk_widget_get_name
#endif
#ifdef SAFEMODE
	// FIXME: if sub_menu == NULL...
	if ((sub_menu==NULL) || (name==NULL)) return NULL;
#endif
	// GList *widget_list = GTK_MENU_SHELL(sub_menu)->children;
	GList *widget_list = gtk_container_get_children(GTK_CONTAINER(sub_menu));
	GtkWidget *return_menuitem = NULL;
	GtkWidget *menu_item = NULL;

	while (widget_list!=NULL)
	{
		menu_item = widget_list->data;
		// g_debug("Got the menu_item = %p", menu_item);
		if (menu_item==NULL) break;

		// g_debug("Checking %s for %s...", gtk_widget_get_name(menu_item), name);
		if (! compare_strings(gtk_widget_get_name(menu_item), name, case_sensitive))
			return_menuitem = menu_item;

		// g_debug("Checking %s and %s... (Got %p)",
		//	gtk_widget_get_name(menu_item), name, return_menuitem);

		if (return_menuitem) return return_menuitem;
		widget_list = widget_list->next;
	}
	return NULL;
#ifdef OUT_OF_MEMORY
	#define gtk_widget_get_name(x) NULL
#endif
}

GtkWidget *add_radio_menuitem_to_sub_menu(GSList *encoding_group,
					  GtkWidget *sub_menu,
					  const gchar *name,
					  GSourceFunc func,
					  gpointer func_data)
{
#ifdef DETAIL
	g_debug("! Launch add_radio_menuitem_to_sub_menu() with encoding_group = %p, sub_menu = %p, name = %s",
		encoding_group, sub_menu, name);
#endif
#ifdef SAFEMODE
	if ((sub_menu==NULL) ||(name==NULL)) return NULL;
#endif
	GtkWidget *menu_item = gtk_radio_menu_item_new_with_label(encoding_group, name);
	gtk_widget_set_name(menu_item, name);
	gtk_menu_shell_append(GTK_MENU_SHELL(sub_menu), menu_item);
	g_signal_connect(menu_item, "activate", G_CALLBACK(func), func_data);

	return menu_item;
}

// it will return how many items have been created.
gint add_menuitem_to_locale_sub_menu(struct Window *win_data,
				     gint no,
				     gchar *name)
{
#ifdef DETAIL
	g_debug("! Launch add_menuitem_to_locale_sub_menu() with win_data = %p, no = %d, name = %s",
		win_data, no, name);
#endif
#ifdef OUT_OF_MEMORY
#  undef g_strdup_printf
#endif
#ifdef SAFEMODE
	if ((win_data==NULL) || (name==NULL)) return 0;
#endif
	gchar *encoding = get_encoding_from_locale(name);
	gint item_created = 1;

	// if name = "zh_TW.UTF-8" or just "UTF-8"
	// g_debug("add_menuitem_to_locale_sub_menu(): Got encoding = %s", encoding);

	if (encoding==NULL && (compare_strings(name, "UTF-8", FALSE)))
	{

		if (check_and_add_locale_to_warned_locale_list(win_data, name))
			create_invalid_locale_error_message(name);
		if (check_if_win_data_is_still_alive(win_data) == FALSE)
		{
			item_created = -1;
			goto FINISH;
		}
		// g_debug("add_menuitem_to_locale_sub_menu(): win_data (%p) is still alive!!!", win_data);
		// Check if win_data is still alive
		return 0;
	}

	if ((! compare_strings(encoding, "UTF-8", FALSE)) ||
	    (! compare_strings(name, "UTF-8", FALSE)))
		item_created = 2;

	gsize i;
	for (i=0; i<item_created; i++)
	{
		gchar *item_name = NULL;
		if (i) item_name = g_strdup_printf("%s%s", name, _(" (Wide)"));
		if (item_name==NULL) item_name = name;
		GtkWidget *menu_item = gtk_image_menu_item_new_with_label(item_name);
		gtk_widget_set_name(menu_item, name);
		// g_debug("Set the %d locale to %s", no, item_name);
		gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_item),
					      gtk_image_new_from_stock(GTK_FAKE_STOCK_DND_MULTIPLE, GTK_ICON_SIZE_MENU));
#ifdef SAFEMODE
		if (win_data->locale_sub_menu)
#endif
			gtk_menu_shell_append(GTK_MENU_SHELL(win_data->locale_sub_menu), menu_item);
		// g_debug("win_data->locale_group = %p", win_data->locale_group);
		// win_data->locale_group = gtk_radio_menu_item_get_group (GTK_RADIO_MENU_ITEM (menu_item));
		g_signal_connect(menu_item, "activate", G_CALLBACK(new_tab_with_locale), (gsize *)i);
		if (item_name != name) g_free(item_name);
	}

FINISH:
	g_free(encoding);
	setlocale(LC_CTYPE, init_LC_CTYPE);
	// g_debug("init_LC_CTYPE = %s, Recover to %s", init_LC_CTYPE, (char*)g_getenv("LC_CTYPE"));
	return item_created;

#ifdef OUT_OF_MEMORY
	#define g_strdup_printf(...) NULL
#endif
}


//gboolean check_locale_in_menuitem(GList *locale_list, const gchar *name)
//{
//	//g_debug("Calling check_locale_in_menuitem with locale_list = %p, name =%s, total = %d",
//	//	locale_list, name, g_list_length(locale_list));
//	GList *widget_list = NULL;
//	int i;
//
//	for (i=0; i< g_list_length(locale_list); i++)
//	{
//		widget_list = g_list_nth(locale_list, i);
//		// g_debug("Checking the locale %s with %s", (gchar *)widget_list->data, name);
//		if (g_ascii_strcasecmp (widget_list->data, name)==0)
//			return TRUE;
//	}
//	return FALSE;
//}

void reset_vte(GtkWidget *widget, struct Window *win_data)
{
#ifdef DETAIL
	g_debug("! Launch reset_vte()!");
#endif
#ifdef SAFEMODE
	if ((win_data==NULL) || (win_data->current_vte==NULL)) return;
#endif
	vte_terminal_reset(VTE_TERMINAL(win_data->current_vte), TRUE, FALSE);
}
#if defined(ENABLE_VTE_BACKGROUND) || defined(FORCE_ENABLE_VTE_BACKGROUND) || defined(UNIT_TEST)
void set_trans_bg(GtkWidget *menuitem_trans_bg, struct Window *win_data)
{
#  ifdef DETAIL
	g_debug("! Launch set_trans_bg() for win_data %p", win_data);
#  endif
#  ifdef SAFEMODE
	if ((menuitem_trans_bg==NULL) || (win_data==NULL)) return;
#  endif
	// win_data->transparent_background = GTK_CHECK_MENU_ITEM(menuitem_trans_bg)->active;
	win_data->transparent_background = gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM(menuitem_trans_bg));

	gint i;
	struct Page *page_data = NULL;

	for (i=0; i<gtk_notebook_get_n_pages(GTK_NOTEBOOK(win_data->notebook)); i++)
	{
		page_data = get_page_data_from_nth_page(win_data, i);
#  ifdef SAFEMODE
		if (page_data)
#  endif
			set_background_saturation (NULL, 0, win_data->background_saturation, page_data->vte);
	}
}
#endif

#ifdef ENABLE_RGBA
void set_trans_win(GtkWidget *widget, GtkWidget *window)
{
#ifdef DETAIL
	g_debug("! Launch set_trans_win() for window %p!", window);
#endif
#ifdef SAFEMODE
	if (window==NULL) return;
#endif
	struct Window *win_data = (struct Window *)g_object_get_data(G_OBJECT(window), "Win_Data");
#ifdef SAFEMODE
	if (win_data==NULL) return;
#endif
	// g_debug("Get win_data = %d when set_trans_win!", win_data);

	// win_data->transparent_window = GTK_CHECK_MENU_ITEM(win_data->menuitem_trans_win)->active;
	win_data->transparent_window = gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM(win_data->menuitem_trans_win));
	set_window_opacity (NULL, 0, win_data->window_opacity, win_data);
}
#endif

void invert_color_theme(GtkWidget *menuitem, struct Window *win_data)
{
#ifdef DETAIL
	g_debug("! Launch invert_color_theme() with menuitem = %p", menuitem);
#endif
#ifdef SAFEMODE
	if (win_data==NULL) return;
#endif
	if (win_data->checking_menu_item) return;

	win_data->color_brightness = - win_data->color_brightness;
	win_data->color_brightness_inactive = - win_data->color_brightness_inactive;
	// g_debug("invert_color_theme(): win_data->color_brightness = %0.3f, win_data->color_brightness_inactive = %0.3f",
	//	win_data->color_brightness, win_data->color_brightness_inactive);
#ifdef SAFEMODE
	if (menuitem)
#endif
		set_ansi_theme(menuitem, ANSI_THEME_INVERT_COLOR, win_data->use_custom_theme,
			       gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM(menuitem)), -1, win_data);
}

void select_ansi_theme(GtkWidget *menuitem, gint index)
{
#ifdef DETAIL
	g_debug("! Launch select_ansi_theme() with menuitem = %p, index = %d", menuitem, index);
#endif
#ifdef FATAL
	// g_debug("menu_active_window = %p", menu_active_window);
	if (menu_active_window==NULL)
		return print_active_window_is_null_error_dialog("set_ansi_theme()");
#endif
#ifdef SAFEMODE
	if ((index < 0) || (index >= (COLOR*2))) return;
	if (menu_active_window==NULL) return;
#endif
	struct Window *win_data = (struct Window *)g_object_get_data(G_OBJECT(menu_active_window), "Win_Data");
#ifdef SAFEMODE
	if (win_data==NULL) return;
#endif
	// g_debug("select_ansi_theme(): win_data->invert_color = %d", win_data->invert_color);
	if (index < THEME)
		set_ansi_theme(menuitem, ANSI_THEME_SET_ANSI_THEME, FALSE, win_data->invert_color, index, win_data);
	else
		set_ansi_theme(menuitem, ANSI_THEME_SET_ANSI_THEME, TRUE, win_data->invert_color, index - THEME, win_data);

	enable_disable_theme_menus(win_data, (index!=(THEME-1)));
}

void set_ansi_theme(GtkWidget *menuitem, Set_ANSI_Theme_Type type, gboolean use_custom_theme, gboolean invert_color,
		    gint theme_index, struct Window *win_data)
{
#ifdef DETAIL
	g_debug("! Launch set_ansi_theme() with type = %d, invert_color = %d, theme_index = %d, win_data = %p",
		type, invert_color, theme_index, win_data);
#endif
#ifdef SAFEMODE
	if (win_data==NULL) return;
#endif
	if (type == ANSI_THEME_INVERT_COLOR)
	{
		if (invert_color == win_data->invert_color) return;
		win_data->invert_color = invert_color;
	}
	else
	{
		if ((win_data->checking_menu_item) ||
		    ((win_data->use_custom_theme == use_custom_theme) && (win_data->color_theme_index == theme_index))) return;
		win_data->color_theme_index = theme_index;
		win_data->use_custom_theme = use_custom_theme;
	}

	gint i;

	generate_all_color_datas (win_data);

	struct Page *page_data = NULL;
	gboolean default_vte_theme = use_default_vte_theme(win_data);

	// g_debug("Get default_vte_theme = %d", default_vte_theme);

	if (menuitem)
	{
		for (i=0; i<gtk_notebook_get_n_pages(GTK_NOTEBOOK(win_data->notebook)); i++)
		{
			page_data = get_page_data_from_nth_page(win_data, i);
#ifdef SAFEMODE
			if (page_data)
#endif
				set_vte_color(page_data->vte, default_vte_theme, win_data->custom_cursor_color, win_data->cursor_color,
					      win_data->color, FALSE, (win_data->color_theme_index==(THEME-1)));
		}

		if (menuitem != win_data->menuitem_invert_color)
		{
			// g_debug("Set the color theme to %s!", gtk_menu_item_get_label(GTK_MENU_ITEM(menuitem)));
			win_data->current_menuitem_theme = menuitem;
		}
	}
	else
	{
#ifdef SAFEMODE
		if (win_data->current_vte)
		{
#endif
			struct Page *page_data = (struct Page *)g_object_get_data(G_OBJECT(win_data->current_vte), "Page_Data");
#ifdef SAFEMODE
			if (page_data)
#endif
				set_vte_color(page_data->vte, default_vte_theme, win_data->custom_cursor_color, win_data->cursor_color,
					      win_data->color, FALSE, (win_data->color_theme_index==(THEME-1)));
#ifdef SAFEMODE
		}
#endif
	}
}

void set_auto_save(GtkWidget *menuitem, struct Window *win_data)
{
#ifdef DETAIL
	g_debug("! Launch set_auto_save() with menuitem = %p, win_data = %p", menuitem, win_data);
#endif
#ifdef SAFEMODE
	if ((win_data==NULL) || (menuitem==NULL)) return;
#endif
	if (win_data->checking_menu_item) return;

	win_data->auto_save = gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM(menuitem));
	// g_debug("Set win_data->auto_save to %d", win_data->auto_save);
}

void set_erase_binding (GtkWidget *menuitem, gint value)
{
#ifdef DETAIL
	g_debug("! Launch set_erase_binding() with menuitem = %p!, value = %d", menuitem, value);
#endif
#ifdef FATAL
	// g_debug("menu_active_window = %p", menu_active_window);
	if (menu_active_window==NULL)
		return print_active_window_is_null_error_dialog("set_erase_binding()");
#endif
#ifdef SAFEMODE
	if (menu_active_window==NULL) return;
#endif
	struct Window *win_data = (struct Window *)g_object_get_data(G_OBJECT(menu_active_window), "Win_Data");
#ifdef SAFEMODE
	if (win_data==NULL) return;
#endif
	if ((win_data->checking_menu_item) || (win_data->erase_binding == value)) return;
	win_data->erase_binding = value;

	gint i;
	struct Page *page_data = NULL;
	for (i=0; i<gtk_notebook_get_n_pages(GTK_NOTEBOOK(win_data->notebook)); i++)
	{
		page_data = get_page_data_from_nth_page(win_data, i);
#ifdef SAFEMODE
		if (page_data)
#endif
			vte_terminal_set_backspace_binding (VTE_TERMINAL(page_data->vte), win_data->erase_binding);
	}
	win_data->current_menuitem_erase_binding = menuitem;
}

#if defined(ENABLE_CURSOR_SHAPE) || defined(UNIT_TEST)
void set_cursor_shape (GtkWidget *menuitem, gint value)
{
#ifdef DETAIL
	g_debug("! Launch set_cursor_shape() with menuitem = %p!, value = %d", menuitem, value);
#endif
#ifdef FATAL
	// g_debug("menu_active_window = %p", menu_active_window);
	if (menu_active_window==NULL)
		return print_active_window_is_null_error_dialog("set_cursor_shape()");
#endif
#ifdef SAFEMODE
	if (menu_active_window==NULL) return;
#endif
	struct Window *win_data = (struct Window *)g_object_get_data(G_OBJECT(menu_active_window), "Win_Data");
#ifdef SAFEMODE
	if (win_data==NULL) return;
#endif
	if ((win_data->checking_menu_item) || (win_data->cursor_shape == value)) return;
	win_data->cursor_shape = value;

	gint i;
	struct Page *page_data = NULL;
	for (i=0; i<gtk_notebook_get_n_pages(GTK_NOTEBOOK(win_data->notebook)); i++)
	{
		page_data = get_page_data_from_nth_page(win_data, i);
#ifdef SAFEMODE
		if (page_data)
#endif
			vte_terminal_set_cursor_shape(VTE_TERMINAL(page_data->vte), win_data->cursor_shape);
	}
	win_data->current_menuitem_cursor_shape = menuitem;
}
#endif

// it is OK to use either zh_TW.Big5 or Big5 here
void set_encoding(GtkWidget *menuitem, gpointer user_data)
{
#ifdef SAFEMODE
	if (menuitem==NULL) return;
#endif
	// g_debug("GTK_CHECK_MENU_ITEM(menuitem)->active = %d", GTK_CHECK_MENU_ITEM(menuitem)->active);
	// if (!GTK_CHECK_MENU_ITEM(menuitem)->active) return;
	if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem)) == FALSE) return;
#ifdef DETAIL
	g_debug("! Launch set_encoding()");
#endif
#ifdef FATAL
	// g_debug("menu_active_window = %p", menu_active_window);
	if (menu_active_window==NULL)
		return print_active_window_is_null_error_dialog("set_encoding()");
#endif
#ifdef SAFEMODE
	if (menu_active_window==NULL) return;
#endif
	struct Window *win_data = (struct Window *)g_object_get_data(G_OBJECT(menu_active_window), "Win_Data");
#ifdef SAFEMODE
	if (win_data==NULL) return;
#endif
	// No action when just right click the mouse
	if (win_data->checking_menu_item) return;

	struct Page *page_data = (struct Page *)g_object_get_data(G_OBJECT(win_data->current_vte), "Page_Data");
#ifdef SAFEMODE
	if (page_data==NULL) return;
#endif
	const gchar *encoding = gtk_widget_get_name(menuitem);
	g_free (page_data->encoding_str);
	page_data->encoding_str = g_strdup(encoding);

	// g_debug("Trying to set %d page (%p) to encoding %s...", page_data->page_no, vte, encoding);
	vte_terminal_set_encoding(VTE_TERMINAL(win_data->current_vte), encoding);

	// page_data->page_name==NULL: add_page() will call set_encoding() before page_data->label is created.
	// And it will cause the following error: "Document ended unexpectedly while inside an attribute value"
	if (win_data->page_shows_encoding && page_data->page_name!=NULL)
		update_page_name (page_data->window, win_data->current_vte, page_data->page_name,
				  page_data->label_text, page_data->page_no+1,
				  page_data->custom_page_name, page_data->tab_color,
				  page_data->is_root, page_data->is_bold,
				  compare_strings(win_data->runtime_encoding,
						  page_data->encoding_str,
						  FALSE),
				  page_data->encoding_str, page_data->custom_window_title,
				  FALSE);
}

void new_tab_with_locale(GtkWidget *locale_menuitem, gboolean VTE_CJK_WIDTH)
{
#ifdef DETAIL
	if (locale_menuitem)
		g_debug("! Launch new_tab_with_locale() with locale = %s and VTE_CJK_WIDTH = %d",
			gtk_widget_get_name(locale_menuitem), VTE_CJK_WIDTH);
	else
		g_debug("! Launch new_tab_with_locale() with VTE_CJK_WIDTH = %d", VTE_CJK_WIDTH);
#endif
#ifdef FATAL
	// g_debug("menu_active_window = %p", menu_active_window);
	if (menu_active_window==NULL)
		return print_active_window_is_null_error_dialog("new_tab_with_locale()");
#endif
#ifdef SAFEMODE
	if (menu_active_window==NULL) return;
#endif
	// g_debug("locale_menuitem->name = %s", gtk_widget_get_name(locale_menuitem);
	struct Window *win_data = (struct Window *)g_object_get_data(G_OBJECT(menu_active_window), "Win_Data");
#ifdef SAFEMODE
	if (win_data==NULL) return;
#endif
	// g_debug("Got the VTE_CJK_WIDTH = %s", gtk_widget_get_name(locale_menuitem);
	struct Page *page_data = (struct Page *)g_object_get_data(G_OBJECT(win_data->current_vte), "Page_Data");
	// menuitem == NULL: The locales_list in profile is empty. Only VTE_CJK_WIDTH is setted.
	// if (menuitem)
	//	g_debug("Add page by %p to locale %s", win_data->current_vte, gtk_widget_get_name(menuitem);

	// locale should not be free()
	const gchar *locale = gtk_widget_get_name(locale_menuitem);
	gchar *encoding = NULL;
	if (compare_strings(locale, "UTF-8", FALSE))
	{
		encoding = get_encoding_from_locale(locale);
		if (encoding==NULL)
		{
			if (check_and_add_locale_to_warned_locale_list(win_data, (gchar *)locale))
				create_invalid_locale_error_message((gchar *)locale);
			goto FINISH;
		}
	}
	else
	{
		locale = "POSIX";
		encoding = g_strdup("UTF-8");
	}
	// g_debug("Get locale = %s, encoding = %s", locale, encoding);

	// struct Page *add_page(struct Window *win_data,
	//		      struct Page *page_data_prev,
	//		      GtkWidget *menuitem_encoding,
	//		      gchar *encoding,
	//		      gchar *locale,
	//		      gchar *environments,
	//		      gchar *user_environ,
	//		      gboolean run_once,
	//		      gchar *VTE_CJK_WIDTH_STR,
	//		      gboolean add_to_next)
#ifdef SAFEMODE
	if (page_data)
#endif
		add_page(win_data,
			 page_data,
			 NULL,
			 (gchar *) encoding,
			 page_data->encoding_str,
			 (gchar *) locale,
			 NULL,
			 NULL,
			 get_VTE_CJK_WIDTH_str(VTE_CJK_WIDTH+1),
			 TRUE);
#ifdef SAFEMODE
	else
		add_page(win_data,
			 NULL,
			 NULL,
			 (gchar *) encoding,
			 encoding,
			 (gchar *) locale,
			 NULL,
			 NULL,
			 get_VTE_CJK_WIDTH_str(VTE_CJK_WIDTH+1),
			 TRUE);

#endif
FINISH:
	setlocale(LC_CTYPE, init_LC_CTYPE);
	g_free(encoding);
}

void select_font(GtkWidget *widget, struct Window *win_data)
{
#ifdef DETAIL
	g_debug("! Launch select_font() for win_data %p", win_data);
#endif
#ifdef SAFEMODE
	if ((win_data==NULL) || (win_data->current_vte==NULL)) return;
#endif
	struct Page *page_data = (struct Page *)g_object_get_data(G_OBJECT(win_data->current_vte), "Page_Data");
#ifdef SAFEMODE
	if (page_data==NULL) return;
#endif
	GtkWidget *dialog = gtk_font_chooser_dialog_new(_("Font Selection"), GTK_WINDOW(win_data->window));
	gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(win_data->window));
	gtk_window_set_destroy_with_parent (GTK_WINDOW (dialog), TRUE);
	// set the default font name in gtk_font_selection_dialog
#ifdef SAFEMODE
	if (page_data->font_name)
#endif
		gtk_font_chooser_set_font(GTK_FONT_CHOOSER(dialog), page_data->font_name);
	if (gtk_dialog_run(GTK_DIALOG(dialog))==GTK_RESPONSE_OK)
	{
		// g_debug("Trying to change font name!");
		g_free(page_data->font_name);
		page_data->font_name = gtk_font_chooser_get_font(GTK_FONT_CHOOSER(dialog));
#ifdef USE_GTK2_GEOMETRY_METHOD
		set_vte_font(NULL, FONT_SET_TO_SELECTED);
#else
		g_idle_add((GSourceFunc)idle_set_vte_font_to_selected, win_data);
#endif
	}
	gtk_widget_destroy(dialog);
}

#ifdef OUT_OF_MEMORY
#  undef g_strdup
#  undef g_strdup_printf
#  undef g_strsplit
#  undef g_strsplit_set
#endif

gboolean refresh_locale_and_encoding_list(struct Window *win_data)
{
#ifdef DETAIL
	g_debug("! Launch refresh_locale_and_encoding_list() for win_data = %p", win_data);
#endif
#ifdef SAFEMODE
	if (win_data==NULL) return FALSE;
#endif
	gboolean response = TRUE;
	gint i = 0;
	GtkWidget *menu_item = NULL;

	for (i=0; i<3; i++)
	{
#ifdef SAFEMODE
		if (win_data->encoding_locale_menuitems[i])
		{
#endif
			gtk_widget_hide(win_data->encoding_locale_menuitems[i]);
			// g_debug("Set not show win_data->encoding_locale_menuitems[%d] (%p)!",
			//	i, win_data->encoding_locale_menuitems[i]);
			gtk_widget_set_no_show_all(win_data->encoding_locale_menuitems[i], TRUE);
#ifdef SAFEMODE
		}
#endif
	}

	// g_debug("Got locales_list = '%s' ( %d bytes)",win_data->locales_list, strlen(win_data->locales_list));
	gint utf8_in_supported_locales = FALSE;
	// g_debug("system_locale_list = %s", system_locale_list);
	// g_debug("win_data->runtime_locale_list = %s", win_data->runtime_locale_list);
	// g_debug("win_data->default_locale = %s", win_data->default_locale);
	// g_debug("win_data->locales_list = %s", win_data->locales_list);
	gchar *full_locales_list = join_strings_to_string(' ', 4,
							  system_locale_list,
							  win_data->runtime_locale_list,
							  win_data->default_locale,
							  win_data->locales_list);
	// g_debug("create_menu(): full_locales_list = %s", full_locales_list);
	GString *encoding_list_from_locale = g_string_new ("");

	gchar **supported_locales = split_string(full_locales_list, " ", -1);
	// print_array("refresh_locale_and_encoding_list(): supported_locales", supported_locales);

	// g_debug("Get current_encoding = %s", (gchar *)vte_terminal_get_encoding(
	//								VTE_TERMINAL(win_data->current_vte)));
	// g_debug("lc_all = %s", lc_all);
#ifdef SAFEMODE
	if (supported_locales)
	{
#endif
		gchar *encoding = NULL;
		i=0;
		while (supported_locales[i])
		{
			encoding = get_encoding_from_locale(supported_locales[i]);
			// g_debug("Got get_encoding_from_locale for %s = %s",
			//	supported_locales[i], encoding);
			if (encoding)
			{
#ifdef SAFEMODE
				if (encoding_list_from_locale)
				{
#endif
					if (encoding_list_from_locale->len)
						g_string_append_printf(encoding_list_from_locale, "%c%s", ' ', encoding);
					else
						g_string_append_printf(encoding_list_from_locale, "%s", encoding);
				}
#ifdef SAFEMODE
			}
#endif
			if (! compare_strings(encoding, "UTF-8", FALSE))
				utf8_in_supported_locales = TRUE;
			g_free(encoding);
			i++;
		}
		setlocale(LC_CTYPE, init_LC_CTYPE);
#ifdef SAFEMODE
	}
#endif
	// g_debug("init_LC_CTYPE = %s, Recover to %s", init_LC_CTYPE, (char*)g_getenv("LC_CTYPE"));
	// g_debug("create_menu(): encoding_list_from_locale = %s", encoding_list_from_locale->str);
	// g_debug("vte_terminal_get_encoding() = %s", (gchar *)vte_terminal_get_encoding(
	//								VTE_TERMINAL(win_data->current_vte)));
	// g_debug("init_encoding = %s", init_encoding);
	// g_debug("win_data->runtime_encoding = %s", win_data->runtime_encoding);
	// g_debug("win_data->default_encoding = %s", win_data->default_encoding);
	// g_debug("encoding_list_from_locale->str = %s", encoding_list_from_locale->str);

	gchar *full_encodings_list;

#ifdef SAFEMODE
	if (encoding_list_from_locale==NULL)
	{
		if (win_data->current_vte)
			full_encodings_list = join_strings_to_string(' ', 4,
								     (gchar *)vte_terminal_get_encoding(
										VTE_TERMINAL(win_data->current_vte)),
								     "UTF-8",
								     init_encoding,
								     win_data->runtime_encoding);
		else
			full_encodings_list = join_strings_to_string(' ', 3,
								     "UTF-8",
								     init_encoding,
								     win_data->runtime_encoding);

	}
	else
	{
		if (win_data->current_vte)
#endif
			full_encodings_list = join_strings_to_string(' ', 5,
								     (gchar *)vte_terminal_get_encoding(
										VTE_TERMINAL(win_data->current_vte)),
								     "UTF-8",
								     init_encoding,
								     win_data->runtime_encoding,
								     encoding_list_from_locale->str);
#ifdef SAFEMODE
		else
			full_encodings_list = join_strings_to_string(' ', 4,
								     "UTF-8",
								     init_encoding,
								     win_data->runtime_encoding,
								     encoding_list_from_locale->str);
	}
#endif
	g_string_free(encoding_list_from_locale, TRUE);
	// g_debug("create_menu(): full_encodings_list = %s", full_encodings_list);
	gint count_encoding_menu=0;
	gchar **supported_encodings = split_string(full_encodings_list, " ", -1);
	// print_array ("supported_encodings", supported_encodings);
#ifdef SAFEMODE
	if (supported_encodings)
	{
#endif
		if (win_data->encoding_sub_menu) gtk_widget_destroy(win_data->encoding_sub_menu);
		win_data->encoding_sub_menu = gtk_menu_new ();
#ifdef SAFEMODE
		if (win_data->encoding_locale_menuitems[0])
#endif
			gtk_menu_item_set_submenu (GTK_MENU_ITEM (win_data->encoding_locale_menuitems[0]),
						   win_data->encoding_sub_menu);
		GSList *encoding_group = NULL;
		i=0;
		while (supported_encodings[i]!=NULL)
		{
			// g_debug("Checking (%d) %s...", i, supported_encodings[i]);
			if (supported_encodings[i] &&
			     !(check_name_in_menuitem(win_data->encoding_sub_menu,
						      supported_encodings[i],
						      FALSE)))
			{
				menu_item = add_radio_menuitem_to_sub_menu (encoding_group,
									    win_data->encoding_sub_menu,
									    supported_encodings[i],
									    (GSourceFunc)set_encoding,
									    NULL);
				// g_debug("Add [ (%ld) %s ] to encoding_sub_menu", i, supported_encodings[i]);
				encoding_group = gtk_radio_menu_item_get_group (GTK_RADIO_MENU_ITEM (menu_item));
				if (i==0)
				{
					// g_debug("Set [ %s ] as win_data->default_menuitem_encoding",
					//	supported_encodings[i]);
					win_data->default_menuitem_encoding = menu_item;
				}
				count_encoding_menu ++;
			}
			i++;
		}
		if (count_encoding_menu>1)
		{
			// g_debug("Show win_data->encoding_locale_menuitems[0] (%p)!",
			//	win_data->encoding_locale_menuitems[0]);
#ifdef SAFEMODE
			if (win_data->encoding_locale_menuitems[0])
#endif
				gtk_widget_set_no_show_all(win_data->encoding_locale_menuitems[0], FALSE);
		}
		else
		{
			// g_debug("Set win_data->encoding_sub_menu = NULL");
			gtk_widget_destroy(win_data->encoding_sub_menu);
			win_data->encoding_sub_menu = NULL;
		}
		g_strfreev(supported_encodings);
#ifdef SAFEMODE
	}
#endif
	g_free(full_encodings_list);

	gint count_locale_menu=0;
	// print_array("refresh_locale_and_encoding_list(): supported_locales", supported_locales);
	if (supported_locales)
	{
		int j;

		for (j=0; j<2; j++)
		{
			GtkWidget *sub_menu_item=NULL;
			if (j)
				sub_menu_item = win_data->encoding_locale_menuitems[1];
			else
				sub_menu_item = win_data->encoding_locale_menuitems[2];
			// g_debug("Set sub_menu_item = %p for win_data (%p)", sub_menu_item, win_data);
			if (win_data->locale_sub_menu) gtk_widget_destroy(win_data->locale_sub_menu);
			win_data->locale_sub_menu = gtk_menu_new ();
#ifdef SAFEMODE
			if (sub_menu_item)
#endif
				gtk_menu_item_set_submenu (GTK_MENU_ITEM (sub_menu_item), win_data->locale_sub_menu);
			i=0;
			while (supported_locales[i]!=NULL)
			{
				// g_debug("Checking %s...", supported_locales[i]);
				if ((supported_locales[i][0] == '\0') ||
				    ((j==0) && utf8_in_supported_locales &&
				    (compare_strings (supported_locales[i], "UTF-8", TRUE)==0)))
				{
					i++;
					continue;
				}
				// if NULL: Not find in sub_menu
				gint menu_created = 0;
				if (!(check_name_in_menuitem(win_data->locale_sub_menu,
							     supported_locales[i],
							     TRUE)))
				{
					menu_created = add_menuitem_to_locale_sub_menu(win_data, i, supported_locales[i]);
					if (menu_created==-1)
					{
						response = FALSE;
						goto FINISH;
					}
				}

				count_locale_menu += menu_created;

				i++;
			}

			if ((count_locale_menu>1) || (compare_strings(full_locales_list, "UTF-8", FALSE)))
			{
				// if the total of menu item is >1, add it to menu.
				// gtk_menu_shell_append (GTK_MENU_SHELL (win_data->menu), sub_menu_item);
				// don't create "New tab" submenu.
				// g_debug("Show sub_menu_item(%p)!", sub_menu_item);
#ifdef DEBUG
				// if ((GTK_IS_WIDGET(sub_menu_item)==FALSE) || (sub_menu_item==NULL))
				//	g_debug("! refresh_locale_and_encoding_list(): "
				//		"sub_menu_item = %p for win_data (%p) is not a widget!",
				//		sub_menu_item, win_data);
#endif
#ifdef SAFEMODE
				if (GTK_IS_WIDGET(sub_menu_item)==FALSE)
				{
					response = FALSE;
					goto FINISH;
				}
#endif
				gtk_widget_set_no_show_all(sub_menu_item, FALSE);
				j=2;
			}
			else
				count_locale_menu = 0;
		}

		if (win_data->encoding_sub_menu)
		{
			if (count_encoding_menu)
				add_separator_menu (win_data->encoding_sub_menu);

			create_menu_item (IMAGE_MENU_ITEM, win_data->encoding_sub_menu, _("Add other locales..."),
					  NULL, GTK_FAKE_STOCK_ADD,  (GSourceFunc)dialog, GINT_TO_POINTER (ADD_NEW_LOCALES));
		}

		if (count_locale_menu)
			add_separator_menu (win_data->locale_sub_menu);

		create_menu_item (IMAGE_MENU_ITEM, win_data->locale_sub_menu, _("Add other locales..."),
				  NULL, GTK_FAKE_STOCK_ADD,  (GSourceFunc)dialog, GINT_TO_POINTER (ADD_NEW_LOCALES));
	}
	for (i=0; i<3; i++)
#ifdef SAFEMODE
		if (win_data->encoding_locale_menuitems[i])
#endif
			gtk_widget_show_all(win_data->encoding_locale_menuitems[i]);

FINISH:
	g_free(full_locales_list);
	g_strfreev(supported_locales);

	return response;
}

void refresh_profile_list (struct Window *win_data)
{
#ifdef DETAIL
	g_debug("! Launch refresh_profile_list() with win_data = %p", win_data);
#endif
#ifdef SAFEMODE
	if (win_data==NULL) return;
#endif
	long new_profile_dir_modtime = get_profile_dir_modtime();
	// g_debug("new_profile_dir_modtime = %ld", new_profile_dir_modtime);
	if ((new_profile_dir_modtime != win_data->profile_dir_modtime) ||
	    (new_profile_dir_modtime == -1))
	{
		// g_debug("Update the profile list!!!");
		// g_debug("refresh_profile_list: win_data->subitem_new_window_from_list = %p, "
		//	"win_data->subitem_load_profile_from_list = %p.",
		//	win_data->subitem_new_window_from_list,
		//	win_data->subitem_load_profile_from_list);
		win_data->subitem_new_window_from_list =
			recreate_profile_menu_item(win_data->menuitem_new_window_from_list,
						   win_data->subitem_new_window_from_list,
						   win_data, NEW_WINDOW_FROM_PROFILE);
		win_data->subitem_load_profile_from_list =
			recreate_profile_menu_item(win_data->menuitem_load_profile_from_list,
						   win_data->subitem_load_profile_from_list,
						   win_data, LOAD_FROM_PROFILE);
		win_data->profile_dir_modtime = new_profile_dir_modtime;

		// g_debug("refresh_profile_list: win_data->subitem_new_window_from_list = %p, "
		//	"win_data->subitem_load_profile_from_list = %p.",
		//	win_data->subitem_new_window_from_list,
		//	win_data->subitem_load_profile_from_list);
	}
}

GtkWidget *recreate_profile_menu_item(GtkWidget *menuitem, GtkWidget *subitem,
				       struct Window *win_data, Apply_Profile_Type type)
{
#ifdef DETAIL
	g_debug("! Launch recreate_profile_menu_item()! with menuitem = %p, subitem = %p, win_data = %p, "
		"type = %d", menuitem, subitem, win_data, type);
#endif
#ifdef SAFEMODE
	if ((win_data==NULL) || (menuitem==NULL)) return NULL;
#endif
	// g_debug("Trying to destroy subitem (%p)!!", subitem);
	if (subitem) gtk_widget_destroy(subitem);
	subitem = gtk_menu_new ();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuitem), subitem);

	if (type == NEW_WINDOW_FROM_PROFILE)
		create_new_window_from_menu_items(subitem, GTK_FAKE_STOCK_NEW);
	else
		create_load_profile_from_menu_items(subitem, GTK_FAKE_STOCK_APPLY, win_data);
	// g_debug("SHOW ALL: %p", subitem);
	gtk_widget_show_all(subitem);
	return subitem;
}

void create_new_window_from_menu_items(GtkWidget *sub_menu, const gchar *stock_id)
{
#ifdef DETAIL
	g_debug("! Launch create_new_window_from_menu_items() with sub_menu = %p, stock_id = %s",
		sub_menu, stock_id);
#endif
	if (create_profile_menu_list(sub_menu, stock_id, (GSourceFunc)apply_profile_from_menu_item,
				     GINT_TO_POINTER(NEW_WINDOW_FROM_PROFILE)))
		add_separator_menu (sub_menu);
	create_menu_item (IMAGE_MENU_ITEM, sub_menu, _("Other settings..."), NULL, GTK_FAKE_STOCK_ADD,
			  (GSourceFunc)apply_profile_from_file_dialog, GINT_TO_POINTER(NEW_WINDOW_FROM_PROFILE));
}

void create_load_profile_from_menu_items(GtkWidget *sub_menu, const gchar *stock_id, struct Window *win_data)
{
#ifdef DETAIL
	g_debug("! Launch create_load_profile_from_menu_items() with sub_menu = %p, "
		"stock_id = %s", sub_menu, stock_id);
#endif
#ifdef SAFEMODE
	if (win_data==NULL) return;
#endif
	// Profile
	//menu_item = gtk_image_menu_item_new_with_label(_("Profile sample"));
	win_data->menuitem_auto_save = create_menu_item(CHECK_MENU_ITEM, sub_menu, _("Save settings automatically"), NULL, NULL,
							(GSourceFunc)set_auto_save, win_data);

	add_separator_menu (sub_menu);

	create_menu_item (IMAGE_MENU_ITEM, sub_menu, _("Save settings"), NULL, GTK_FAKE_STOCK_SAVE,
			  (GSourceFunc)save_user_settings, win_data);

	create_menu_item (IMAGE_MENU_ITEM, sub_menu, _("Save settings as..."), NULL, GTK_FAKE_STOCK_SAVE_AS,
			  (GSourceFunc)save_user_settings_as, win_data);

	add_separator_menu (sub_menu);

	// Reload Settings
	create_menu_item (IMAGE_MENU_ITEM, sub_menu, _("Reload settings"), NULL, GTK_FAKE_STOCK_REVERT_TO_SAVED,
			  (GSourceFunc)reload_settings, win_data);

//	gchar *menu_label = g_strdup_printf(_("Apply to every %s window"), PACKAGE);
//#ifdef SAFEMODE
//	if (menu_label)
//	{
//#endif
//		create_menu_item (IMAGE_MENU_ITEM, sub_menu, menu_label, NULL, GTK_FAKE_STOCK_REFRESH,
//				  (GSourceFunc)apply_to_every_window, win_data);
//#ifdef SAFEMODE
//	}
//#endif
//	g_free(menu_label);

	add_separator_menu(sub_menu);

	if (create_profile_menu_list(sub_menu, GTK_FAKE_STOCK_APPLY,
				 (GSourceFunc)apply_profile_from_menu_item, GINT_TO_POINTER(LOAD_FROM_PROFILE)))
		add_separator_menu (sub_menu);

	create_menu_item (IMAGE_MENU_ITEM, sub_menu, _("Other settings..."), NULL, GTK_FAKE_STOCK_ADD,
			  (GSourceFunc)apply_profile_from_file_dialog, GINT_TO_POINTER(LOAD_FROM_PROFILE));
}

gboolean create_profile_menu_list(GtkWidget *sub_menu, const gchar *stock_id, GSourceFunc func, gpointer func_data)
{
#ifdef DETAIL
	g_debug("! Launch create_profile_menu_list() with sub_menu = %p, stock_id = %s", sub_menu, stock_id);
#endif
#ifdef SAFEMODE
	if (sub_menu==NULL) return FALSE;
#endif
	gboolean create_separator = FALSE;
	// LilyTerm Buildin Configures
	gchar *menu_label = g_strdup_printf(_("%s default"), PACKAGE);
#ifdef SAFEMODE
	if (menu_label)
	{
#endif
		create_menu_item (IMAGE_MENU_ITEM, sub_menu, menu_label, "", stock_id,
				  (GSourceFunc)func, func_data);
		create_separator = TRUE;
#ifdef SAFEMODE
	}
#endif
	g_free(menu_label);

	// System Profile
	gpointer system_func_data;
	if (func_data==GINT_TO_POINTER(NEW_WINDOW_FROM_PROFILE))
		system_func_data = GINT_TO_POINTER(NEW_WINDOW_FROM_SYSTEM_PROFILE);
	else
		system_func_data = GINT_TO_POINTER(LOAD_FROM_SYSTEM_PROFILE);

	if (g_file_test(SYS_PROFILE, G_FILE_TEST_EXISTS))
	{
		create_menu_item (IMAGE_MENU_ITEM, sub_menu, _("System default"),
				  SYS_PROFILE, stock_id, (GSourceFunc)func, system_func_data);
		create_separator = TRUE;
	}
	if (create_separator) add_separator_menu(sub_menu);
#ifdef SAFEMODE
	if (profile_dir == NULL) return FALSE;
#endif

	gboolean menu_list_created = FALSE;
	// User's main profile
	// g_debug("profile_dir = %s", profile_dir);
	GDir *dir = g_dir_open(profile_dir, 0, NULL);

	// dir==NULL: the profile dir is not exist.
	if (dir==NULL) return FALSE;

	const gchar *entry = g_dir_read_name(dir);
	gchar *user_profile;
	if (entry==NULL) goto FINISH;
	do
	{
		if (entry[0]=='.') continue;
		user_profile = g_strdup_printf("%s/%s", profile_dir, entry);
		// g_debug("create_profile_menu_list(): user_profile = %s", user_profile);
		create_menu_item (IMAGE_MENU_ITEM, sub_menu, entry, user_profile, stock_id,
				  (GSourceFunc)func, func_data);
		g_free(user_profile);
		menu_list_created = TRUE;
	}
	while ((entry = g_dir_read_name(dir)) != NULL);

FINISH:
	g_dir_close(dir);
	// g_debug("create_profile_menu_list(): menu_list_created = %d", menu_list_created);
	return menu_list_created;
}

#ifdef OUT_OF_MEMORY
	#define g_strdup fake_g_strdup
	#define g_strdup_printf(...) NULL
	#define g_strsplit fake_g_strsplit
	#define g_strsplit_set(x,y,z) NULL
#endif

void apply_profile_from_file_dialog(GtkWidget *menu_item, Apply_Profile_Type type)
{
#ifdef DETAIL
	g_debug("! Launch apply_profile_from_file_dialog() with menu_item = %p, type = %d", menu_item, type);
#endif
#ifdef FATAL
	// g_debug("menu_active_window = %p", menu_active_window);
	if (menu_active_window==NULL)
		return print_active_window_is_null_error_dialog("apply_profile_from_file_dialog()");
#endif
#ifdef SAFEMODE
	if (menu_active_window==NULL) return;
#endif
	struct Window *win_data = (struct Window *)g_object_get_data(G_OBJECT(menu_active_window), "Win_Data");
#ifdef SAFEMODE
	if (win_data==NULL) return;
#endif
	GtkWidget *dialog;

	if (win_data->use_custom_profile)
		dialog = create_load_file_dialog(GTK_FILE_CHOOSER_ACTION_OPEN, win_data->window, GTK_FAKE_STOCK_OPEN, win_data->profile);
	else
	{
		gchar *profile = get_profile();
		dialog = create_load_file_dialog(GTK_FILE_CHOOSER_ACTION_OPEN, win_data->window, GTK_FAKE_STOCK_OPEN, profile);
		g_free(profile);
	}

	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
	{
		// Create new win_data
		gchar *filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
		apply_profile_from_file(filename, type);
		g_free (filename);
	}
	gtk_widget_destroy (dialog);
}

void apply_profile_from_menu_item(GtkWidget *menu_item, Apply_Profile_Type type)
{
#ifdef DETAIL
	g_debug("! Launch apply_profile_from_menu_item() with menu_item= %p, type = %d", menu_item, type);
#endif
	// g_debug("The path of menu_item is %s", gtk_widget_get_name(menu_item));
#ifdef SAFEMODE
	if (menu_item==NULL) return;
#endif
	apply_profile_from_file(gtk_widget_get_name(menu_item), type);
}

void apply_profile_from_file(const gchar *path, Apply_Profile_Type type)
{
#ifdef DETAIL
	g_debug("! Launch apply_profile_from_file() with path= %s, type = %d", path, type);
#endif
#ifdef SAFEMODE
	if (path==NULL) return;
#endif
	gint argc = 0;

#ifdef FATAL
	// g_debug("menu_active_window = %p", menu_active_window);
	if (menu_active_window==NULL)
		return print_active_window_is_null_error_dialog("apply_profile_from_file()");
#endif
#ifdef SAFEMODE
	if (menu_active_window==NULL) return;
#endif
	struct Window *win_data = (struct Window *)g_object_get_data(G_OBJECT(menu_active_window), "Win_Data");
#ifdef SAFEMODE
	if (win_data==NULL) return;
#endif

	if ((type == NEW_WINDOW_FROM_PROFILE) || (type == NEW_WINDOW_FROM_SYSTEM_PROFILE))
	{
		gchar *argv_str = NULL;
#ifdef SAFEMODE
		if ((path) && path[0])
#else
		if (path[0])
#endif
		{
			if (type == NEW_WINDOW_FROM_SYSTEM_PROFILE)
			{
				argv_str = g_strdup_printf("-u\x10%s\x10--specified_profile\x10%s",
							   path, win_data->profile);
				argc = 5;
			}
			else
			{
				argv_str = g_strdup_printf("-u\x10%s", path);
				argc = 3;
			}
		}
		else
		{
			argv_str = g_strdup_printf("--safe-mode\x10--specified_profile\x10%s", win_data->profile);
			argc = 4;
		}
		// g_debug("! apply_profile_from_file(): argv_str = %s", argv_str);

		gchar **argv = NULL;
#ifdef SAFEMODE
		if (argv_str==NULL)
			argc = 0;
		else
#endif
			argv = split_string(argv_str, "\x10", -1);
		// print_array("! apply_profile_from_file(): argv", argv);

		init_socket_data();
		query_socket();
		send_socket(argc, argv, FALSE);

		g_free (argv_str);
		g_strfreev(argv);
	}
	else
	{
		// gtk_widget_hide(menu_active_window);
		struct Window *win_data_backup = g_new0(struct Window, 1);
#ifdef SAFEMODE
		if (win_data_backup==NULL) return;
#endif
		// Switch the data in win_data_backup and win_data.
		// The win_data will become {0}, and win_data_backup will stores the old datas from win_data
		memcpy(win_data_backup, win_data, sizeof(* win_data));
		// g_debug("apply_profile_from_file(): Append win_data(%p) to window_list!", win_data_backup);
		window_list = g_list_append (window_list, win_data_backup);
		memset(win_data, 0, sizeof(* win_data));

		GString *invild_settings = g_string_new(NULL);

		win_data->window = win_data_backup->window;
		win_data->notebook = win_data_backup->notebook;
		win_data->current_vte = win_data_backup->current_vte;

#ifdef SAFEMODE
		if ((path) && path[0])
#else
		if (path[0])
#endif
		{
			// g_debug("Got win_data->use_custom_profile = %d, path = %s, win_data_backup->profile = %s",
			//	win_data->use_custom_profile, path, win_data_backup->profile);
			switch (type)
			{
				case LOAD_FROM_SYSTEM_PROFILE:
					win_data->use_custom_profile = win_data_backup->use_custom_profile;
					win_data->specified_profile = g_strdup(win_data_backup->profile);
					break;
				default:
					win_data->use_custom_profile = TRUE;
					break;
			}
			win_data->profile = g_strdup(path);
		}
		else
		{
			safe_mode = TRUE;
			win_data->use_custom_profile = win_data_backup->use_custom_profile;
			win_data->profile = g_strdup(win_data_backup->profile);
		}
		win_data->warned_locale_list = g_string_new(win_data_backup->warned_locale_list->str);
		win_data->runtime_encoding = g_strdup(win_data_backup->runtime_encoding);
		win_data->runtime_LC_MESSAGES = g_strdup(win_data_backup->runtime_LC_MESSAGES);
		win_data->use_rgba = win_data_backup->use_rgba;
		win_data->runtime_locale_list = g_strdup(win_data_backup->runtime_locale_list);
		win_data->wmclass_name = g_strdup(win_data_backup->wmclass_name);
		win_data->wmclass_class = g_strdup(win_data_backup->wmclass_class);
		win_data->shell = g_strdup(win_data_backup->shell);
		win_data->home = g_strdup(win_data_backup->home);
		win_data->restore_font_name = NULL;
		win_data->geometry = NULL;

		// g_debug("before: win_data->use_rgba = %d, win_data_backup->use_rgba = %d",
		//	win_data->use_rgba, win_data_backup->use_rgba);
		get_user_settings(win_data, win_data->runtime_encoding);
		if (((win_data->use_rgba_orig == 0) && (win_data->use_rgba == -1)) ||
		    ((win_data->use_rgba_orig == 1) && (win_data->use_rgba == -2)))
			g_string_append_printf(invild_settings, "\tuse_rgba = %d\n",
					       win_data->use_rgba_orig);

		win_data->init_dir = g_strdup(win_data_backup->init_dir);
		win_data->environment = g_strdup(win_data_backup->environment);
		win_data->login_shell = win_data_backup->login_shell;
		win_data->utmp = win_data_backup->utmp;
		win_data->custom_window_title_str = g_strdup(win_data_backup->custom_window_title_str);
		win_data->custom_tab_names_str = NULL;
		win_data->custom_tab_names_strs = NULL;
		win_data->window_title_tpgid = win_data_backup->window_title_tpgid;
		// win_data->hints_type will setted to 1 in apply_font_to_every_vte()
		// win_data->hints_type = win_data_backup->hints_type;
		// win_data->resize_type = win_data_backup->resize_type;
#ifdef USE_GTK3_GEOMETRY_METHOD
		win_data->resize_type = GEOMETRY_CUSTOM;
#endif
		// win_data->geometry_width = win_data_backup->geometry_width;
		// win_data->geometry_height = win_data_backup->geometry_height;
		// win_data->keep_vte_size = win_data_backup->keep_vte_size;
		if (win_data->page_names && win_data_backup->page_names &&
		    (! compare_strings (win_data->page_names, win_data_backup->page_names, TRUE)) &&
		    ((win_data->page_shows_current_cmdline ||
		      win_data->page_shows_current_dir ||
		      win_data->page_shows_window_title) == FALSE))
			win_data->page_names_no = win_data_backup->page_names_no;
		// We need to check the profile version again...
		// win_data->confirmed_profile_is_invalid = win_data_backup->confirmed_profile_is_invalid;

		if (win_data->tabs_bar_position)
			gtk_notebook_set_tab_pos(GTK_NOTEBOOK(win_data->notebook), GTK_POS_BOTTOM);
		win_data->profile_dir_modtime = -1;

#ifdef USE_GTK3_GEOMETRY_METHOD
		// g_debug("win_data_backup->window_status = %d", win_data_backup->window_status);
		if (win_data->window_status == WINDOW_NORMAL) win_data->window_status = WINDOW_APPLY_PROFILE_NORMAL;
		if (win_data->window_status == WINDOW_START_WITH_FULL_SCREEN) win_data->window_status = WINDOW_APPLY_PROFILE_FULL_SCREEN;
#endif

		create_menu(win_data);

#ifdef USE_GTK2_GEOMETRY_METHOD
		win_data->fullscreen = win_data_backup->fullscreen;
		win_data->true_fullscreen = win_data_backup->true_fullscreen;
#endif
		// g_debug("apply_profile_from_file(): Got win_data->show_tabs_bar = %d", win_data->show_tabs_bar);
		hide_and_show_tabs_bar(win_data, win_data->show_tabs_bar);
		// g_debug("win_data_backup->fullscreen_show_scroll_bar = %d", win_data_backup->fullscreen_show_scroll_bar);
		// win_data->fullscreen_show_scroll_bar = win_data_backup->fullscreen_show_scroll_bar;

		gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(win_data->menuitem_hide_scroll_bar),
								    check_show_or_hide_scroll_bar(win_data));
		hide_scrollback_lines(win_data->menuitem_hide_scroll_bar, win_data);
		// g_debug("apply_profile_from_file(): check_show_or_hide_scroll_bar() = %d",
		//	check_show_or_hide_scroll_bar(win_data));

#ifdef USE_GTK2_GEOMETRY_METHOD
		// g_debug("win_data_backup->window_status = %d", win_data_backup->window_status);
		win_data->window_status = win_data_backup->window_status;
		// g_debug("win_data->startup_fullscreen = %d, win_data_backup->startup_fullscreen = %d",
		//	win_data->startup_fullscreen, win_data_backup->startup_fullscreen);
		if (win_data->startup_fullscreen != win_data_backup->startup_fullscreen)
			deal_key_press(win_data->window, KEY_FULL_SCREEN, win_data);
#endif
#ifdef USE_GTK3_GEOMETRY_METHOD
		// g_debug("win_data->window_status = %d, win_data_backup->window_status = %d",
		//	win_data->window_status, win_data_backup->window_status);
		switch (win_data->window_status)
		{
			case WINDOW_APPLY_PROFILE_NORMAL:
				switch (win_data_backup->window_status)
				{
					case WINDOW_MAX_WINDOW:
						deal_key_press(win_data->window, KEY_MAX_WINDOW, win_data);
						break;
					case WINDOW_MAX_WINDOW_TO_FULL_SCREEN:
					case WINDOW_FULL_SCREEN:
						deal_key_press(win_data->window, KEY_FULL_SCREEN, win_data);
						break;
					default:
						break;
				}
				win_data->resize_type = GEOMETRY_CUSTOM;
				keep_gtk3_window_size(win_data, FALSE);
				break;
			case WINDOW_APPLY_PROFILE_FULL_SCREEN:
				switch (win_data_backup->window_status)
				{
					case WINDOW_NORMAL:
					case WINDOW_MAX_WINDOW_TO_FULL_SCREEN:
					case WINDOW_MAX_WINDOW:
						deal_key_press(win_data->window, KEY_FULL_SCREEN, win_data);
						break;
					default:
						break;
				}
				break;
			default:
				break;
		}
#endif
#ifdef SAFEMODE
		if (win_data->default_locale && (win_data->default_locale[0] != '\0') &&
		    compare_strings(win_data->default_locale, win_data_backup->default_locale, TRUE))
#else
		if ((win_data->default_locale[0] != '\0') &&
		    compare_strings(win_data->default_locale, win_data_backup->default_locale, TRUE))

#endif
			g_string_append_printf(invild_settings, "\tdefault_locale = %s\n",
					       win_data->default_locale);

		if (compare_strings(win_data->VTE_CJK_WIDTH_STR, win_data_backup->VTE_CJK_WIDTH_STR, FALSE))
			g_string_append_printf(invild_settings, "\tVTE_CJK_WIDTH = %d\n",
					       win_data->VTE_CJK_WIDTH);

		if (win_data_backup->find_string)
		{
			g_free(win_data->find_string);
			win_data->find_string = g_strdup(win_data_backup->find_string);
		}
		win_data->find_case_sensitive = win_data_backup->find_case_sensitive;
		win_data->find_use_perl_regular_expressions = win_data_backup->find_use_perl_regular_expressions;
		win_data->find_entry_bg_color = win_data_backup->find_entry_bg_color;
		win_data->find_entry_current_bg_color = win_data_backup->find_entry_current_bg_color;

#ifdef SAFEMODE
		if (invild_settings && invild_settings->len)
#else
		if (invild_settings->len)
#endif
		{
			setlocale(LC_MESSAGES, win_data->runtime_LC_MESSAGES);
			gchar *color_settings = convert_text_to_html(&(invild_settings->str), FALSE, "blue", NULL);
			gchar *message = g_strdup_printf(_("The following settings:\n\n%s\n"
							   "Can NOT be applied to the terminal boxes of current window.\n"
							   "Please use [%s] instead."),
							   color_settings,
							   _("New window with specified profile"));
			error_dialog(win_data->window,
				     _("The following settings can NOT be applied"),
				     "The following settings can NOT be applied",
				     GTK_FAKE_STOCK_DIALOG_WARNING,
				     message,
				     NULL);
			g_free(color_settings);
			g_free(message);
		}
		g_string_free(invild_settings, TRUE);

		gint i;
		gboolean refresh_match = FALSE;
		for (i=0; i<COMMAND; i++)
		{
			if (compare_strings (win_data->user_command[i].match_regex, win_data_backup->user_command[i].match_regex, TRUE))
			{
				refresh_match = TRUE;
				break;
			}
		}

		struct Page *page_data = NULL;
		for (i=0; i<gtk_notebook_get_n_pages(GTK_NOTEBOOK(win_data->notebook)); i++)
		{
			page_data = get_page_data_from_nth_page(win_data, i);
#ifdef SAFEMODE
			if (page_data==NULL) continue;
#endif
			if (page_data->page_update_method == PAGE_METHOD_WINDOW_TITLE) page_data->window_title_updated = 1;
			apply_new_win_data_to_page (win_data_backup, win_data, page_data);
			// Don't like drag a vte to another lilyterm window,
			// It will not call page_added()
			// get_and_update_page_name(page_data, FALSE);

			if (refresh_match) set_hyperlink(win_data, page_data);
		}

		glong column=0, row=0;
		get_row_and_column_from_geometry_str(&column, &row, &(win_data->default_column), &(win_data->default_row), win_data->geometry);
		if (win_data->geometry && (win_data->geometry[0]!='\0'))
		{
			// may not work!!
			gtk_window_parse_geometry(GTK_WINDOW(win_data->window), win_data->geometry);
		}
#ifdef USE_GTK2_GEOMETRY_METHOD
#  ifdef GEOMETRY
		g_debug("@ apply_profile_from_file (for %p): Trying set the geometry to %ld x %ld",
			win_data->window, column, row);
#  endif
		apply_font_to_every_vte(win_data->window, win_data->default_font_name,
					column, row);
#endif
#ifdef USE_GTK3_GEOMETRY_METHOD
		win_data->geometry_width = column;
		win_data->geometry_height = row;

		page_data = (struct Page *)g_object_get_data(G_OBJECT(win_data->current_vte), "Page_Data");
#  ifdef SAFEMODE
		if (page_data!=NULL)
		{
#  endif
			g_free(page_data->font_name);
			page_data->font_name = g_strdup(win_data->default_font_name);
#  ifdef SAFEMODE
		}
#  endif
		g_idle_add((GSourceFunc)idle_set_vte_font_to_selected, win_data);
#endif
		// g_debug("apply_profile_from_file(): Remove win_data(%p) from window_list!", win_data);
		window_list = g_list_remove (window_list, win_data_backup);
		clear_win_data(win_data_backup);
#ifdef SAFEMODE
		win_data_backup = NULL;
#endif
		// gtk_widget_show_all(win_data->window);
#ifdef USE_GTK3_GEOMETRY_METHOD
		// g_debug("win_data->window_status = %d, win_data_backup->window_status = %d",
		//	win_data->window_status, win_data_backup->window_status);
		switch (win_data->window_status)
		{
			case WINDOW_APPLY_PROFILE_NORMAL:
				win_data->window_status = WINDOW_NORMAL;
				break;
			case WINDOW_APPLY_PROFILE_FULL_SCREEN:
				win_data->window_status = WINDOW_FULL_SCREEN;
				break;
			default:
				break;
		}
#endif
	}
}

#if defined(ENABLE_VTE_BACKGROUND) || defined(FORCE_ENABLE_VTE_BACKGROUND) || defined(UNIT_TEST)
void load_background_image_from_file(GtkWidget *widget, struct Window *win_data)
{
#  ifdef DETAIL
	g_debug("! Launch load_background_from_file() with widget = %p, win_data = %p", widget, win_data);
#  endif
#  ifdef SAFEMODE
	if (win_data==NULL) return;
#  endif
	gchar *background_image_path = "";
	if (compare_strings(win_data->background_image, NULL_DEVICE, TRUE))
		background_image_path = win_data->background_image;
	GtkWidget *dialog = create_load_file_dialog(GTK_FILE_CHOOSER_ACTION_OPEN, win_data->window,
						    GTK_FAKE_STOCK_OPEN, background_image_path);

	struct Preview *preview = g_new0(struct Preview, 1);
#  ifdef SAFEMODE
	if (preview==NULL) return;
#  endif

	preview->vbox = dirty_gtk_vbox_new (FALSE, 0);
	preview->image = gtk_image_new();
	g_signal_connect (dialog, "update-preview", G_CALLBACK (update_preview_image), preview);
	gtk_box_pack_end (GTK_BOX(preview->vbox), preview->image, FALSE, FALSE, 0);
	preview->no_image_text = gtk_label_new(NULL);
	set_markup_key_value(FALSE, "blue",
			     _("Disable: Trying to use [Transparent Background] instead."),
			     preview->no_image_text);
	gtk_box_pack_end (GTK_BOX(preview->vbox), preview->no_image_text, FALSE, FALSE, 0);
	preview->frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type (GTK_FRAME (preview->frame), GTK_SHADOW_IN);
	gtk_container_add (GTK_CONTAINER (preview->frame), preview->vbox);
	preview->scroll_background = gtk_check_button_new_with_label(_("Scroll the background image along with the text"));
	// GTK_TOGGLE_BUTTON(preview->scroll_background)->active = win_data->scroll_background;
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(preview->scroll_background),
				      win_data->scroll_background);

	preview->mainbox = dirty_gtk_vbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX(preview->mainbox), preview->frame, FALSE, FALSE, 5);
	gtk_box_pack_start (GTK_BOX(preview->mainbox), preview->scroll_background, FALSE, FALSE, 0);
	gtk_file_chooser_set_extra_widget (GTK_FILE_CHOOSER(dialog), preview->mainbox);
	gtk_widget_show_all(dialog);
	preview->default_filename = g_strdup(background_image_path);
	update_preview_image (GTK_FILE_CHOOSER(dialog), preview);
	GtkResponseType response;
#  ifdef UNIT_TEST
	for (response=GTK_RESPONSE_HELP; response<=GTK_RESPONSE_NONE; response++)
#  else
	response = gtk_dialog_run (GTK_DIALOG (dialog));
	if (response == GTK_RESPONSE_ACCEPT)
#  endif
	{
		// g_debug("gtk_widget_get_mapped(preview->no_image_text) = %d",
		//	gtk_widget_get_mapped (preview->no_image_text));
		g_free(win_data->background_image);
		win_data->transparent_background = gtk_widget_get_mapped (preview->no_image_text);
		if (win_data->transparent_background)
			win_data->background_image = g_strdup(NULL_DEVICE);
		else
			win_data->background_image = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));

		// g_debug("Get new win_data->background_image = %s", win_data->background_image);
		win_data->scroll_background = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(preview->scroll_background));
		gint i;
		struct Page *page_data = NULL;
#  ifdef SAFEMODE
		if (win_data->notebook)
		{
#  endif
			for (i=0; i<gtk_notebook_get_n_pages(GTK_NOTEBOOK(win_data->notebook)); i++)
			{
				page_data = get_page_data_from_nth_page(win_data, i);
#  ifdef SAFEMODE
				if (page_data)
#  endif
					set_background_saturation (NULL, 0, win_data->background_saturation,
								   page_data->vte);
			}
#  ifdef SAFEMODE
		}
#  endif
	}
	gtk_widget_destroy (dialog);
	g_free(preview);
}
#endif

#if defined(ENABLE_VTE_BACKGROUND) || defined(FORCE_ENABLE_VTE_BACKGROUND) || defined(UNIT_TEST)
void update_preview_image (GtkFileChooser *dialog, struct Preview *preview)
{
#  ifdef DETAIL
	g_debug("! Launch update_preview_image() with dialog = %p, preview = %p!", dialog, preview);
#  endif
#  ifdef SAFEMODE
	if ((preview==NULL) || (dialog==NULL)) return;
#  endif
	gchar *filename = preview->default_filename;
	if (filename)
		// filename will be free() later.
		preview->default_filename = NULL;
	else
		filename = gtk_file_chooser_get_preview_filename (dialog);
	// g_debug("Get filename = %s in gtk_file_chooser_get_preview_filename()", filename);
	if (filename!=NULL)
	{
		gtk_widget_show_all(preview->frame);
		GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file (filename, NULL);
		GdkPixbuf *show_pixbuf = NULL;
		if (pixbuf)
		{
			gint width = gdk_pixbuf_get_width(pixbuf);
			gint height = gdk_pixbuf_get_height(pixbuf);

			// g_debug("Get the current size of vbox is %d x %d",
			//	preview->vbox->allocation.width, preview->vbox->allocation.height);
			gint allocation_width;
#  ifdef USE_GTK_ALLOCATION
			GtkAllocation allocation;
			gtk_widget_get_allocation(preview->vbox, &allocation);
			allocation_width = allocation.width;

#  else
			allocation_width = preview->vbox->allocation.width;
#  endif
			if (width > allocation_width) width = allocation_width;

			GtkRequisition requisition;
			gtk_window_get_size(GTK_WINDOW(dialog),
					    &(requisition.width),
					    &(requisition.height));
			// g_debug("Get the window size of dialog is %d x %d, Child width should be %d",
			//	requisition.width, requisition.height, requisition.width-28);
			if (height > requisition.height * 0.4) height = requisition.height * 0.4;

			show_pixbuf = gdk_pixbuf_new_from_file_at_size (filename, width, height, NULL);
			if (show_pixbuf)
			{
				// g_debug("Set the background to %s...", filename);
				gtk_widget_show(preview->image);
				gtk_image_set_from_pixbuf (GTK_IMAGE (preview->image), show_pixbuf);
				gtk_widget_hide (preview->no_image_text);
				g_object_unref (show_pixbuf);
			}
			g_object_unref (pixbuf);
			goto FINISH;
		}
		// g_debug("preview->image = %p", preview->image);
		gtk_image_set_from_pixbuf (GTK_IMAGE (preview->image), NULL);
	}

FINISH:
	g_free (filename);
}
#endif

void reload_settings(GtkWidget *menu_item, struct Window *win_data)
{
#ifdef DETAIL
	g_debug("! Launch reload_settings() with win_data = %p", win_data);
#endif
#ifdef SAFEMODE
	if (win_data==NULL) return;
#endif
	// FIXME: NOT CHECKED!!!!
	// g_debug("win_data->profile = %s", win_data->profile);

	gchar *current_profile = NULL;
	if (win_data->profile)
		current_profile = g_strdup(win_data->profile);
	else
		current_profile = g_strdup_printf("%s/%s", profile_dir, USER_PROFILE);

	apply_profile_from_file(current_profile, LOAD_FROM_PROFILE);
	g_free(current_profile);
}

void apply_to_every_window(GtkWidget *menu_item, struct Window *win_data)
{
#ifdef DETAIL
	g_debug("! Launch apply_to_every_window() with win_data = %p!", win_data);
#endif
	GList *win_list = window_list;

	while (win_list)
	{
		// if (win_data != win_list->data)
		//	apply_new_win_data(win_list->data, win_data);
		win_list = win_list->next;
	}
}

void save_user_settings_as(GtkWidget *widget, struct Window *win_data)
{
#ifdef DETAIL
	g_debug("! Launch save_user_settings_as() with win_data = %p", win_data);
#endif
#ifdef SAFEMODE
	if (win_data==NULL) return;
#endif
	gchar *profile = get_profile();
	// g_debug("Get new profile = %s", profile);
#ifdef SAFEMODE
	if (profile==NULL) return;
#endif
	GtkWidget *dialog;
	if (win_data->use_custom_profile)
		dialog = create_load_file_dialog(GTK_FILE_CHOOSER_ACTION_SAVE, win_data->window, GTK_FAKE_STOCK_SAVE, win_data->profile);
	else
		dialog = create_load_file_dialog(GTK_FILE_CHOOSER_ACTION_SAVE, win_data->window, GTK_FAKE_STOCK_SAVE, profile);

	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
	{
		win_data->use_custom_profile = TRUE;
		g_free(win_data->profile);
		win_data->profile = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
		save_user_settings(NULL, win_data);
	}
	gtk_widget_destroy (dialog);
	g_free(profile);
}

GtkWidget *create_load_file_dialog(GtkFileChooserAction action, GtkWidget *window, gchar *button_text, gchar *filename)
{
#ifdef DETAIL
	g_debug("! Launch create_load_file_dialog() with window = %p, filename =%s",
		window, filename);
#endif
	gchar *window_title = _("Open File");
	if (action == GTK_FILE_CHOOSER_ACTION_SAVE)
		window_title = _("Save File");

	GtkWidget *dialog;
	if (gtk_alternative_dialog_button_order(NULL))
		dialog = gtk_file_chooser_dialog_new (window_title,
						      GTK_WINDOW(window),
						      action,
						      button_text, GTK_RESPONSE_ACCEPT,
						      GTK_FAKE_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
						      NULL);
	else
		dialog = gtk_file_chooser_dialog_new (window_title,
						      GTK_WINDOW(window),
						      action,
						      GTK_FAKE_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
						      button_text, GTK_RESPONSE_ACCEPT,
						      NULL);

	if (filename && (filename[0]!='\0'))
	{
		// g_debug("Set the filename to %s", filename);
		gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (dialog), filename);

		if ((action==GTK_FILE_CHOOSER_ACTION_SAVE) &&
		    (! g_file_test(filename, G_FILE_TEST_EXISTS)))
		{
			gchar *basename = g_path_get_basename (filename);
			gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (dialog), basename);
			g_free(basename);
		}
	}

	if (action==GTK_FILE_CHOOSER_ACTION_SAVE)
		gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (dialog), TRUE);

	return dialog;
}

GtkWidget *create_menu_item (Menu_Itemn_Type type, GtkWidget *sub_menu, const gchar *label, const gchar *label_name,
			     const gchar *stock_id, GSourceFunc func, gpointer func_data)
{
#ifdef DETAIL
	g_debug("! Launch create_menu_item() with sub_menu = %p, label = %s, "
		"label_name = %s, stock_id = %s, func = %p, func_data = %p",
		sub_menu, label, label_name, stock_id, func, func_data);
#endif
#ifdef SAFEMODE
	if ((label==NULL) || (sub_menu==NULL)) return NULL;
#endif
	GtkWidget *menu_item = NULL;
	switch (type)
	{
		case IMAGE_MENU_ITEM:
			menu_item = gtk_image_menu_item_new_with_label(label);
			gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_item),
						      gtk_image_new_from_stock(stock_id, GTK_ICON_SIZE_MENU));
			break;
		case CHECK_MENU_ITEM:
			menu_item = gtk_check_menu_item_new_with_label (label);
			break;
		default:
#ifdef FATAL
			print_switch_out_of_range_error_dialog("create_menu_item", "type", type);
#endif
			return NULL;
	}
	// g_debug("set the name of menuitem to : %s", label_name);
	if (label_name) gtk_widget_set_name(menu_item, label_name);
	gtk_menu_shell_append(GTK_MENU_SHELL(sub_menu), menu_item);
	g_signal_connect(menu_item, "activate", G_CALLBACK(func), func_data);
	return menu_item;
}

GtkWidget *create_sub_item (GtkWidget *menu, gchar *label, const gchar *stock_id)
{
#ifdef DETAIL
	g_debug("! Launch create_sub_item() with menu = %p, label = %s, stock_id = %s", menu, label, stock_id);
#endif
#ifdef SAFEMODE
	if (label==NULL) return NULL;
#endif
	GtkWidget *menu_item = create_sub_item_subitem (label, stock_id);
	return create_sub_item_submenu (menu, menu_item);
}

GtkWidget *create_sub_item_subitem (gchar *label, const gchar *stock_id)
{
#ifdef DETAIL
	g_debug("! Launch create_sub_item_subitem() with label = %s, stock_id = %s", label, stock_id);
#endif
#ifdef SAFEMODE
	if (label==NULL) return NULL;
#endif
	GtkWidget *menu_item = gtk_image_menu_item_new_with_label(label);
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_item),
				      gtk_image_new_from_stock(stock_id, GTK_ICON_SIZE_MENU));
	return menu_item;
}

GtkWidget *create_sub_item_submenu (GtkWidget *menu, GtkWidget *menu_item)
{
#ifdef DETAIL
	g_debug("! Launch create_sub_item() with menu = %p, menu_item = %p", menu, menu_item);
#endif
#ifdef SAFEMODE
	if (menu==NULL) return NULL;
#endif
	GtkWidget *sub_menu = gtk_menu_new ();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_item), sub_menu);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);
	return sub_menu;
}

GtkWidget *add_separator_menu (GtkWidget *sub_menu)
{
#ifdef DETAIL
	g_debug("! Launch add_separator_menu() with sub_menu = %p", sub_menu);
#endif
#ifdef SAFEMODE
	if (sub_menu==NULL) return NULL;
#endif
	GtkWidget *menu_item = gtk_separator_menu_item_new();
	gtk_menu_shell_append (GTK_MENU_SHELL (sub_menu), menu_item);
	return menu_item;
}

long get_profile_dir_modtime()
{
#ifdef DETAIL
	g_debug("! Launch get_profile_dir_modtime()");
#endif
#ifdef SAFEMODE
	if (profile_dir==NULL) return -1;
#endif
	struct stat buffer;

	if (stat(profile_dir, &buffer))
		return 0;
	return buffer.st_mtime;
}

void set_menuitem_label(GtkWidget *menu_item, gchar *text)
{
#ifdef DETAIL
	g_debug("! Launch set_menuitem_label() with menu_item = %p, text = %s!", menu_item, text);
#endif
#ifdef SAFEMODE
	if ((menu_item==NULL) || (text==NULL)) return;
#endif
	gtk_menu_item_set_label(GTK_MENU_ITEM(menu_item), text);
}

#ifdef FATAL
void print_active_window_is_null_error_dialog(gchar *function)
{
	gchar *err_msg = g_strdup_printf("%s: menu_active_window = NULL\n\n"
					 "Please report bug to %s, Thanks!",
					 function, BUGREPORT);
#ifdef SAFEMODE
	if (err_msg)
#endif
#ifdef UNIT_TEST
		g_debug(_("The following error occurred: %s"), err_msg);
#else
		error_dialog(NULL, _("The following error occurred:"),
			     "The following error occurred:",
			     GTK_FAKE_STOCK_DIALOG_ERROR, err_msg, NULL);
#endif
	g_free(err_msg);
}
#endif


gboolean check_if_win_data_is_still_alive(struct Window *win_data)
{
#ifdef DETAIL
	g_debug("! Launch check_if_win_data_is_still_alive() with win_data = %p!", win_data);
#endif
	GList *win_list = window_list;

	while (win_list)
	{
		if (win_data == win_list->data) return TRUE;
		win_list = win_list->next;
	}
#ifdef DEBUG
	fprintf(stderr, "\033[1;%dm** check_if_win_data_is_still_alive(): win_data (%p) is NOT alive!\033[0m\n",
		ANSI_COLOR_RED, win_data);
#endif
	// g_debug("check_if_win_data_is_still_alive: win_data (%p) is NOT alive!!!", win_data);
	return FALSE;
}

void enable_disable_theme_menus(struct Window *win_data, gboolean show)
{
#ifdef DETAIL
	g_debug("! Launch enable_disable_theme_menus() with win_data = %p, show = %d!", win_data, show);
#endif
#ifdef SAFEMODE
	if (win_data==NULL) return;
#endif
	gint i;
	for (i=0; i<ANSI_THEME_MENUITEM; i++)
	{
#ifdef SAFEMODE
		if (win_data->ansi_theme_menuitem[i]==NULL) continue;
#endif
		gtk_widget_set_sensitive(win_data->ansi_theme_menuitem[i], show);
	}
}
