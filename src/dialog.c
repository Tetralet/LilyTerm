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

#include "dialog.h"
#define TEMPSTR 6
#define BORDER_SPACE 10

extern GtkWidget *menu_active_window;
GtkWidget *real_menu_active_window;
extern struct ModKey modkeys[MOD];
extern struct Page_Color page_color[PAGE_COLOR];
extern GList* window_list;
extern gchar *profile_dir;
extern gchar *key_groups[KEY_GROUP];
extern struct KeyValue system_keys[KEYS];
extern gchar *init_LC_CTYPE;
extern struct Color color[COLOR];
extern struct GdkRGBA_Theme system_color_theme[THEME];

gint dialog_activated = 0;
gboolean force_to_quit = FALSE;
gboolean using_kill = FALSE;

GdkRGBA entry_not_find_bg_color = {0, 0xffff, 0xbcbc, 0xbcbc};

// EDIT_LABEL,
// FIND_STRING,
// ADD_NEW_LOCALES,
// CHANGE_THE_FOREGROUND_COLOR,
// CHANGE_THE_ANSI_COLORS,
// CHANGE_THE_BACKGROUND_COLOR,
// CHANGE_THE_CURSOR_COLOR,
// ADJUST_THE_BRIGHTNESS_OF_ANSI_COLORS_USED_IN_TERMINAL,
// ADJUST_THE_BRIGHTNESS_OF_ANSI_COLORS_WHEN_INACTIVE,
// CHANGE_BACKGROUND_SATURATION,
// CHANGE_THE_OPACITY_OF_WINDOW,
// CHANGE_THE_OPACITY_OF_WINDOW_WHEN_INACTIVE,
// CHANGE_THE_TEXT_COLOR_OF_WINDOW_TITLE,
// CHANGE_THE_TEXT_COLOR_OF_CMDLINE,
// CHANGE_THE_TEXT_COLOR_OF_CURRENT_DIR,
// CHANGE_THE_TEXT_COLOR_OF_CUSTOM_PAGE_NAME,
// CHANGE_THE_TEXT_COLOR_OF_ROOT_PRIVILEGES_CMDLINE,
// CHANGE_THE_TEXT_COLOR_OF_NORMAL_TEXT,
// CONFIRM_TO_EXECUTE_COMMAND,
// CONFIRM_TO_CLOSE_MULTI_PAGES,
// CONFIRM_TO_CLOSE_RUNNING_APPLICATION,
// CONFIRM_TO_CLOSE_A_TAB_WITH_CHILD_PROCESS,
// CONFIRM_TO_CLOSE_A_WINDOW_WITH_CHILD_PROCESS,
// CONFIRM_TO_EXIT_WITH_CHILD_PROCESS,
// CONFIRM_TO_PASTE_TEXTS_TO_VTE_TERMINAL,
// GENERAL_INFO,
// PASTE_TEXTS_TO_EVERY_VTE_TERMINAL,
// PASTE_GRABBED_KEY_TO_EVERY_VTE_TERMINAL,
// SET_KEY_BINDING,
// USAGE_MESSAGE

GtkResponseType dialog(GtkWidget *widget, gsize style)
{
#ifdef DETAIL
	g_debug("! Launch dialog() with style = %ld", (long)style);
#endif
	dialog_activated++;
	// g_debug("Set dialog_activated = %d", dialog_activated);

	real_menu_active_window = menu_active_window;

	struct Dialog *dialog_data = g_new0(struct Dialog, 1);
#ifdef SAFEMODE
	if (real_menu_active_window)
#endif
		g_object_set_data(G_OBJECT(real_menu_active_window), "Dialog", dialog_data);
	dialog_data->type = style;
	gboolean selectable = FALSE;
	// Backup the datas in clipboard.
	// extern GtkClipboard *selection_clipboard;
	// gchar *selection_clipboard_str = g_strdup(gtk_clipboard_wait_for_text(selection_clipboard));
	extern GtkClipboard *selection_primary;
	gchar *selection_primary_str = NULL;
#ifdef SAFEMODE
	if (selection_primary)
#endif
		selection_primary_str = g_strdup(gtk_clipboard_wait_for_text(selection_primary));

	GtkResponseType dialog_response = GTK_RESPONSE_NONE;

#ifdef SAFEMODE
	if (dialog_data==NULL) goto FINISH;
#endif
	gchar *temp_str[TEMPSTR] = {NULL};

	struct Window *win_data = NULL;
	// A critical error message for current_vte = NULL.

#ifdef FATAL
	// real_menu_active_window==NULL: it should NOT happen!
	if (real_menu_active_window==NULL)
	{
		gchar *err_msg = g_strdup_printf("dialog(%ld): real_menu_active_window = NULL\n\n"
						  "Please report bug to %s, Thanks!",
						  (glong)style, BUGREPORT);
#ifdef SAFEMODE
		if (err_msg)
#endif
			error_dialog(NULL, _("The following error occurred:"),
				     "The following error occurred:",
				     GTK_FAKE_STOCK_DIALOG_ERROR, err_msg, NULL);
		g_free(err_msg);
		goto FINISH;
	}
	else
#endif
		win_data = (struct Window *)g_object_get_data(G_OBJECT(real_menu_active_window), "Win_Data");

#ifdef SAFEMODE
	if (win_data==NULL) goto FINISH;
#endif

	gboolean enable_key_binding = win_data->enable_key_binding;

	// g_debug("dialog(): win_data->runtime_LC_MESSAGES = %s", win_data->runtime_LC_MESSAGES);
	setlocale(LC_MESSAGES, win_data->runtime_LC_MESSAGES);

	struct Page *page_data = NULL;
	if (win_data->current_vte!=NULL)
	{
		page_data = (struct Page *)g_object_get_data(G_OBJECT(win_data->current_vte), "Page_Data");
#ifdef SAFEMODE
		if (page_data==NULL) goto FINISH;
#endif
		// g_debug("Get win_data = %p, page_data = %p, current_vte = %p when creating dialog!",
		//	win_data, page_data, win_data->current_vte);

		// For changing the text color of tab
		dialog_data->total_page = gtk_notebook_get_n_pages(GTK_NOTEBOOK(
									page_data->notebook));
		dialog_data->current_page_no = gtk_notebook_get_current_page(GTK_NOTEBOOK(
									page_data->notebook));
	}

	gboolean tabs_bar_is_shown = TRUE;
	gint create_entry_hbox = 0;
	gint i;

	// g_debug("Running dialog() with mod %d", style);

	/*
		/--------------main_hbox--------------\
		|/icon_vbox\/------state_vbox-----\/-\|
		||	   ||/----title_hbox-----\||r||
		||	   |||			 |||i||
		||	   ||\-------------------/||g||
		||	   ||/----entry_hbox-----\||h||
		||	   |||			 |||t||
		||	   |||			 ||| ||
		||	   ||\-------------------/||v||
		||	   ||/-state_bottom_hbox-\||b||
		||	   |||			  ||o||
		||	   ||\-------------------/||x||
		|\---------/\---------------------/\-/|
		|     /------------\ /------------\   |
		|     |	    OK	   | |	 CANCEL	  |   |
		|     \------------/ \------------/   |
		\-------------------------------------/
	*/

	switch (style)
	{
		case EDIT_LABEL:						// 1
			create_dialog(_("Rename this tab"),
				      "Rename this tab",
				      DIALOG_OK_CANCEL,
				      page_data->window,
				      FALSE,
				      FALSE,
				      10,
				      GTK_RESPONSE_OK,
				      GTK_FAKE_STOCK_DIALOG_INFO,
				      NULL,
				      selectable,
				      0,
				      TRUE,
				      BOX_HORIZONTAL,
				      25,
				      dialog_data);

			gchar *default_page_name;
			if (page_data->custom_page_name == NULL)
				default_page_name = page_data->page_name;
			else
				default_page_name = page_data->custom_page_name;
			dialog_data->operate[0] = create_entry_widget (dialog_data->box,
								  _("Please input the Label Name of this tab:"),
								  _("Label Name:"),
								  default_page_name,
								  TRUE);
			break;
		case FIND_STRING:
		{
#ifdef ENABLE_FIND_STRING
			create_dialog(_("Find string"),
				      "Find string",
				      DIALOG_OK,
				      page_data->window,
				      FALSE,
				      FALSE,
				      10,
				      GTK_RESPONSE_OK,
				      GTK_FAKE_STOCK_FIND,
				      NULL,
				      selectable,
				      0,
				      TRUE,
				      BOX_VERTICALITY,
				      5,
				      dialog_data);
			GtkWidget *hbox = dirty_gtk_hbox_new (FALSE, 0);
			GtkWidget *label = gtk_label_new(_("Find: "));
			gtk_box_pack_start (GTK_BOX(hbox), label, FALSE, FALSE, 0);
			dialog_data->operate[0] = gtk_entry_new ();
			g_signal_connect(G_OBJECT(dialog_data->operate[0]), "changed",
						  G_CALLBACK(refresh_regex_settings), win_data);
#ifdef USE_GDK_RGBA
			GtkStyleContext *rc_style = gtk_widget_get_style_context (dialog_data->operate[0]);
			// gtk_style_context_add_class(rc_style, GTK_STYLE_CLASS_ENTRY);
			gtk_style_context_get_background_color (rc_style, 0, &(win_data->find_entry_bg_color));
#else
			GtkStyle *rc_style = gtk_widget_get_style (dialog_data->operate[0]);
			win_data->find_entry_bg_color = rc_style->base[GTK_STATE_NORMAL];
			g_object_unref(rc_style);
#endif

			win_data->find_entry_current_bg_color = win_data->find_entry_bg_color;
			// print_color("win_data->find_entry_bg_color", win_data->find_entry_bg_color);

			// gtk_entry_set_icon_from_stock (GTK_ENTRY (dialog_data->operate[0]),
			//			       GTK_ENTRY_ICON_SECONDARY, GTK_FAKE_STOCK_CLEAR);

			gtk_box_pack_start (GTK_BOX(hbox), dialog_data->operate[0], TRUE, TRUE, 0);

			label = create_button_with_image(_("Find previous string"),
							 GTK_FAKE_STOCK_GO_UP,
							 TRUE,
							 (GSourceFunc)find_str,
							 GINT_TO_POINTER(FIND_PREV));
			gtk_box_pack_end (GTK_BOX(hbox), label, FALSE, FALSE, 0);
			label = create_button_with_image(_("Find next string"),
							 GTK_FAKE_STOCK_GO_DOWN,
							 TRUE,
							 (GSourceFunc)find_str,
							 GINT_TO_POINTER(FIND_NEXT));
			gtk_box_pack_end (GTK_BOX(hbox), label, FALSE, FALSE, 0);
			gtk_box_pack_start (GTK_BOX(dialog_data->box), hbox, FALSE, FALSE, 0);

			hbox = dirty_gtk_hbox_new (FALSE, 15);
			gtk_box_pack_start (GTK_BOX(dialog_data->box), hbox, FALSE, FALSE, 0);

			dialog_data->operate[1] = gtk_check_button_new_with_label(_("Case sensitive"));
			// GTK_TOGGLE_BUTTON(dialog_data->operate[1])->active = win_data->find_case_sensitive;
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(dialog_data->operate[1]),
						      win_data->find_case_sensitive);
			gtk_box_pack_start (GTK_BOX(hbox), dialog_data->operate[1], FALSE, FALSE, 0);
			g_signal_connect(G_OBJECT(dialog_data->operate[1]), "toggled",
						  G_CALLBACK(refresh_regex_settings), win_data);

			dialog_data->operate[2] = gtk_check_button_new_with_label(_("Use Perl compatible regular expressions"));
			// GTK_TOGGLE_BUTTON(dialog_data->operate[2])->active = win_data->find_use_perl_regular_expressions;
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(dialog_data->operate[2]),
						      win_data->find_use_perl_regular_expressions);
			gtk_box_pack_start (GTK_BOX(hbox), dialog_data->operate[2], FALSE, FALSE, 0);
			g_signal_connect(G_OBJECT(dialog_data->operate[2]), "toggled",
					 G_CALLBACK(refresh_regex_settings), win_data);
			set_widget_can_not_get_focus(dialog_data->operate[1]);
			set_widget_can_not_get_focus(dialog_data->operate[2]);
			hbox = dirty_gtk_hbox_new (FALSE, 5);
			dialog_data->operate[3] = gtk_label_new(NULL);
			gtk_widget_set_no_show_all (dialog_data->operate[3], TRUE);
			gtk_box_pack_end (GTK_BOX(hbox), dialog_data->operate[3], FALSE, FALSE, 0);
			gtk_box_pack_start (GTK_BOX(dialog_data->box), hbox, FALSE, FALSE, 0);

			// refresh_regex(win_data, dialog_data);

			// Dirty Hack
			gchar *clipboard_str = NULL;
			if (vte_terminal_get_has_selection(VTE_TERMINAL(win_data->current_vte)))
			{
				extern GtkClipboard *selection_primary;
#ifdef SAFEMODE
				if (selection_primary)
#endif
					clipboard_str = gtk_clipboard_wait_for_text (selection_primary);
				if (clipboard_str && (clipboard_str[0]!='\0'))
					gtk_entry_set_text(GTK_ENTRY(dialog_data->operate[0]),
							   strtok(clipboard_str, "\n\r"));
			}
			if (((clipboard_str==NULL) || (clipboard_str[0]=='\0')) &&
			    (win_data->find_string && (win_data->find_string[0]!='\0')))
				gtk_entry_set_text(GTK_ENTRY(dialog_data->operate[0]), win_data->find_string);
			g_free(clipboard_str);
#else
			upgrade_dialog(ENABLE_FIND_STRING_VER);
#endif
			break;
		}
		case PASTE_TEXTS_TO_EVERY_VTE_TERMINAL:
		{
			create_dialog(_("Paste texts to every Vte Terminal"),
				      "Paste texts to every Vte Terminal",
				      DIALOG_NONE,
				      page_data->window,
				      FALSE,
				      FALSE,
				      10,
				      GTK_RESPONSE_OK,
				      GTK_FAKE_STOCK_DIALOG_INFO,
				      NULL,
				      selectable,
				      0,
				      TRUE,
				      BOX_VERTICALITY,
				      10,
				      dialog_data);

			dialog_data->operate[0] = create_entry_widget (dialog_data->box,
								  _("Please input the texts that will be paste to the Vte Terminals:"),
								  _("Text:"),
								  NULL,
								  FALSE);
			g_signal_connect(G_OBJECT(dialog_data->operate[0]), "activate",
					 G_CALLBACK(paste_text_to_vte_terminal), dialog_data);

			// Check Button: Append a <NewLine> to the end of the text
			GtkWidget *hbox = dirty_gtk_hbox_new (FALSE, 5);
			gtk_box_pack_start (GTK_BOX(dialog_data->box), hbox, FALSE, FALSE, 0);
			dialog_data->operate[1] = gtk_check_button_new_with_label(_("Append a <NewLine> to the end of the text."));
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(dialog_data->operate[1]), TRUE);
			gtk_box_pack_end (GTK_BOX(hbox), dialog_data->operate[1], FALSE, FALSE, 0);

			// <Paste> and <Esc> Button
#ifdef HAVE_GTK_DIALOG_GET_ACTION_AREA
			GtkWidget *paste_button = gtk_button_new_from_stock (GTK_FAKE_STOCK_PASTE);
			if (gtk_alternative_dialog_button_order(NULL))
			{
				gtk_box_pack_end (GTK_BOX(gtk_dialog_get_action_area(GTK_DIALOG(dialog_data->window))),
						  paste_button, FALSE, FALSE, 0);
				gtk_dialog_add_button (GTK_DIALOG(dialog_data->window), GTK_FAKE_STOCK_QUIT, GTK_RESPONSE_CANCEL);
			}
			else
			{
				gtk_dialog_add_button (GTK_DIALOG(dialog_data->window), GTK_FAKE_STOCK_QUIT, GTK_RESPONSE_CANCEL);
				gtk_box_pack_end (GTK_BOX(gtk_dialog_get_action_area(GTK_DIALOG(dialog_data->window))),
						  paste_button, FALSE, FALSE, 0);
			}
#else
			GtkWidget *paste_button;
			if (gtk_alternative_dialog_button_order(NULL))
			{
				gtk_dialog_add_button (GTK_DIALOG(dialog_data->window), GTK_FAKE_STOCK_QUIT, GTK_RESPONSE_CANCEL);
				paste_button = gtk_dialog_add_button (GTK_DIALOG(dialog_data->window), GTK_FAKE_STOCK_PASTE, GTK_RESPONSE_OK);
			}
			else
			{
				paste_button = gtk_dialog_add_button (GTK_DIALOG(dialog_data->window), GTK_FAKE_STOCK_PASTE, GTK_RESPONSE_OK);
				gtk_dialog_add_button (GTK_DIALOG(dialog_data->window), GTK_FAKE_STOCK_QUIT, GTK_RESPONSE_CANCEL);
			}
			gtk_dialog_set_response_sensitive(GTK_DIALOG(dialog_data->window), GTK_RESPONSE_OK, FALSE);
#endif
			g_signal_connect(G_OBJECT(paste_button), "clicked",
					 G_CALLBACK(paste_text_to_vte_terminal), dialog_data);
			// <Grab keys> Button
			add_secondary_button(dialog_data->window, _("Grab keys"), GTK_RESPONSE_OK, GTK_FAKE_STOCK_REFRESH);
			break;
		}
		case PASTE_GRABBED_KEY_TO_EVERY_VTE_TERMINAL:
		{
			win_data->enable_key_binding = FALSE;
			temp_str[0] = g_markup_escape_text (_("Paste the grabbed key to the Vte Terminals.\n"
							      "It will be useful if you want to paste combined keys, like <Ctrl><C>."),
							    -1);
			create_dialog(_("Paste texts to every Vte Terminal"),
				      "Paste texts to every Vte Terminal",
				      DIALOG_QUIT,
				      page_data->window,
				      FALSE,
				      FALSE,
				      10,
				      GTK_RESPONSE_CANCEL,
				      GTK_FAKE_STOCK_DIALOG_INFO,
				      temp_str[0],
				      selectable,
				      0,
				      TRUE,
				      BOX_VERTICALITY,
				      10,
				      dialog_data);
			// The Key Value frame
			dialog_data->operate[0] = gtk_label_new(NULL);
			// set_markup_key_value(TRUE, "blue", "", dialog_data->operate[0]);
			gtk_label_set_selectable(GTK_LABEL(dialog_data->operate[0]), TRUE);
			create_frame_widget(dialog_data, _("Key Value:"), NULL, dialog_data->operate[0], 0);

			g_signal_connect(G_OBJECT(dialog_data->window), "key-press-event",
					 G_CALLBACK(grab_key_press), dialog_data);

			// <Switch> Button
			add_secondary_button(dialog_data->window, _("Entry"), GTK_RESPONSE_OK, GTK_FAKE_STOCK_REFRESH);
			break;
		}
		case ADD_NEW_LOCALES:
		{
			create_dialog(_("Add new locales and set the default locale"),
				      "Add new locales and set the default locale",
				      DIALOG_OK_CANCEL,
				      page_data->window,
				      FALSE,
				      FALSE,
				      10,
				      GTK_RESPONSE_OK,
				      GTK_FAKE_STOCK_DIALOG_INFO,
				      NULL,
				      selectable,
				      0,
				      TRUE,
				      BOX_VERTICALITY,
				      25,
				      dialog_data);
			// TRANSLATE NOTE: You may replace the following locales with the locale name of your country.
			// TRANSLATE NOTE: For example, replace "zh_TW" with "ru_RU", replace "zh_TW.Big5" with "ru_RU.KOI8-R",
			// TRANSLATE NOTE: or replace "zh_TW.UTF-8" with "ru_RU.UTF-8".
			gchar *contents = _("The default locale used when initing a Vte Terminal.\n"
					    "You may use \"zh_TW\", \"zh_TW.Big5\", or \"zh_TW.UTF-8\" here.");
			dialog_data->operate[0] = create_entry_widget (dialog_data->box,
								  contents,
								  _("Default locale:"),
								  win_data->default_locale,
								  TRUE);

			// TRANSLATE NOTE: You may replace the following locales with the locale name of your country.
			// TRANSLATE NOTE: For example, replace "zh_TW" with "ja_JP", replace "zh_TW.Big5" with "ja_JP.EUC-JP",
			// TRANSLATE NOTE: and replace "zh_TW.UTF-8" with "ja_JP.UTF-8".
			contents = _("The locales list on right click menu, separate with <space>.\n"
				     "You may use \"zh_TW\", \"zh_TW.Big5\", or \"zh_TW.UTF-8\" here.\n"
				     "You may want to use \"UTF-8\" here if you have no locale data installed.");
			dialog_data->operate[1] = create_entry_widget (dialog_data->box,
								  contents,
								  _("Locales list:"),
								  win_data->locales_list,
								  TRUE);
			break;
		}
#if defined(ENABLE_VTE_BACKGROUND) || defined(FORCE_ENABLE_VTE_BACKGROUND)
		case CHANGE_BACKGROUND_SATURATION:				// 2
		{
			create_dialog(_("Change the saturation of background"),
				      "Change the saturation of background",
				      DIALOG_OK_CANCEL,
				      page_data->window,
				      FALSE,
				      TRUE,
				      10,
				      GTK_RESPONSE_OK,
				      GTK_FAKE_STOCK_DIALOG_INFO,
				      _("Change the Saturation of background:"),
				      selectable,
				      0,
				      TRUE,
				      BOX_VERTICALITY,
				      0,
				      dialog_data);

			create_scale_widget(dialog_data, 0, 1, 0.001,
					    win_data->background_saturation,
					    (GSourceFunc)set_background_saturation,
					    win_data->current_vte);

			dialog_data->original_transparent_background = win_data->transparent_background;
			if (! compare_strings (win_data->background_image, NULL_DEVICE, TRUE))
				win_data->transparent_background = TRUE;
			set_background_saturation(NULL, 0, win_data->background_saturation,
						  win_data->current_vte);

			break;
		}
#endif
#ifdef ENABLE_GDKCOLOR_TO_STRING
		case CHANGE_THE_FOREGROUND_COLOR:				// 9
		{
			create_dialog(_("Change the foreground color in terminal"),
				      "Change the foreground color in terminal",
				      DIALOG_OK_CANCEL,
				      page_data->window,
				      FALSE,
				      FALSE,
				      15,
				      GTK_RESPONSE_OK,
				      NULL,
				      NULL,
				      selectable,
				      0,
				      FALSE,
				      BOX_HORIZONTAL,
				      0,
				      dialog_data);
			gint color_index = get_color_index(win_data->invert_color, COLOR-1);
			if (win_data->use_custom_theme)
				dialog_data->original_color = win_data->custom_color_theme[win_data->color_theme_index].color[color_index];
			else
				dialog_data->original_color = system_color_theme[win_data->color_theme_index].color[color_index];
			// g_debug("win_data->use_custom_theme = %d", win_data->use_custom_theme);
			// print_color(-1, "ORIG: dialog_data->original_color", dialog_data->original_color);
			create_color_selection_widget(dialog_data, (GSourceFunc)adjust_vte_color, win_data->current_vte);
			break;
		}
		case CHANGE_THE_BACKGROUND_COLOR:				// 10
			create_dialog(_("Change the background color in terminal"),
				      "Change the background color in terminal",
				      DIALOG_OK_CANCEL,
				      page_data->window,
				      FALSE,
				      FALSE,
				      15,
				      GTK_RESPONSE_OK,
				      NULL,
				      NULL,
				      selectable,
				      0,
				      FALSE,
				      BOX_HORIZONTAL,
				      0,
				      dialog_data);
			gint color_index = get_color_index(win_data->invert_color, 0);
			if (win_data->use_custom_theme)
				dialog_data->original_color = win_data->custom_color_theme[win_data->color_theme_index].color[color_index];
			else
				dialog_data->original_color = system_color_theme[win_data->color_theme_index].color[color_index];
			create_color_selection_widget(dialog_data, (GSourceFunc)adjust_vte_color, win_data->current_vte);
			break;
		case CHANGE_THE_CURSOR_COLOR:				// 9
			create_dialog(_("Change the cursor color in terminal"),
				      "Change the cursor color in terminal",
				      DIALOG_OK_CANCEL,
				      page_data->window,
				      FALSE,
				      FALSE,
				      15,
				      GTK_RESPONSE_OK,
				      NULL,
				      NULL,
				      selectable,
				      0,
				      FALSE,
				      BOX_VERTICALITY,
				      0,
				      dialog_data);
			dialog_data->original_color = win_data->cursor_color;
			create_color_selection_widget(dialog_data, (GSourceFunc)adjust_vte_color, win_data->current_vte);
			dialog_data->original_custom_cursor_color = win_data->custom_cursor_color;
#if defined(USE_OLD_GTK_COLOR_SELECTION) || defined(UNIT_TEST)
			// ---- Create the [Drawn the text under the cursor with foreground and background colors reversed] check button ----
			dialog_data->custom_cursor_color_checkbox = gtk_check_button_new_with_label(_("Drawn the text under the cursor with foreground and background colors reversed"));
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(dialog_data->custom_cursor_color_checkbox), ! win_data->custom_cursor_color);
			gtk_box_pack_start (GTK_BOX(dialog_data->box), dialog_data->custom_cursor_color_checkbox, FALSE, FALSE, 10);
			g_signal_connect(G_OBJECT(dialog_data->custom_cursor_color_checkbox), "toggled",
						  G_CALLBACK(update_custom_cursor_color), win_data);
#endif
			break;
		case CHANGE_THE_ANSI_COLORS:					// 9
		{
			// ---- Create the Dialog ----
			gchar *tmp_topic[2];
			tmp_topic[0] = g_strdup_printf(_("Change the ansi color [%s] in terminal"), color[COLOR-1].translation);
			tmp_topic[1] = g_strdup_printf("Change the ansi color [%s] in terminal", color[COLOR-1].translation);
			create_dialog(tmp_topic[0],
				      tmp_topic[1],
				      DIALOG_OK_CANCEL,
				      page_data->window,
				      FALSE,
				      FALSE,
				      15,
				      GTK_RESPONSE_OK,
				      NULL,
				      NULL,
				      selectable,
				      0,
				      FALSE,
				      BOX_VERTICALITY,
				      0,
				      dialog_data);

			// ---- Create the Color Selection ----
			gint color_index = get_color_index(win_data->invert_color, COLOR-1);
			if (win_data->use_custom_theme)
				dialog_data->original_color = win_data->custom_color_theme[win_data->color_theme_index].color[color_index];
			else
				dialog_data->original_color = system_color_theme[win_data->color_theme_index].color[color_index];
#ifdef USE_OLD_GTK_COLOR_SELECTION
			create_color_selection_widget(dialog_data, (GSourceFunc)adjust_vte_color, win_data->current_vte);
#endif
			// ---- Create the Range Scale ----
			GtkWidget *label = gtk_label_new(_("The brightness of ANSI colors:"));
			GtkWidget *box = dirty_gtk_hbox_new (FALSE, 0);

			GtkWidget *vbox = dirty_gtk_vbox_new (FALSE, 0);
			gtk_box_pack_start (GTK_BOX(dialog_data->box), vbox, FALSE, FALSE, 10);

			gtk_box_pack_start (GTK_BOX(box), label, FALSE, FALSE, 0);
			gtk_box_pack_start (GTK_BOX(vbox), box, FALSE, FALSE, 0);

			dialog_data->operate[1] = gtk_hscale_new_with_range(-1, 1, 0.001);
			gtk_range_set_value(GTK_RANGE(dialog_data->operate[1]), win_data->color_brightness);
			gtk_box_pack_start (GTK_BOX(vbox), dialog_data->operate[1], FALSE, FALSE, 0);
			g_signal_connect_after(dialog_data->operate[1], "change-value", G_CALLBACK(set_ansi_color), win_data->current_vte);
			dialog_data->original_color_brightness = win_data->color_brightness;
			dialog_data->original_color_brightness_inactive = win_data->color_brightness_inactive;

			label = gtk_label_new(_("ANSI Colors:"));
			box = dirty_gtk_hbox_new (FALSE, 0);
			gtk_box_pack_start (GTK_BOX(box), label, FALSE, FALSE, 0);

			// ---- Create the [Invert Color] check button ----
			GtkWidget *check_button = gtk_check_button_new_with_label(_("Invert color"));
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(check_button), win_data->invert_color);
			gtk_box_pack_end (GTK_BOX(box), check_button, FALSE, FALSE, 0);
			gtk_box_pack_start (GTK_BOX(dialog_data->box), box, FALSE, FALSE, 0);
			g_signal_connect(G_OBJECT(check_button), "toggled",
						  G_CALLBACK(dialog_invert_color_theme), win_data);
			dialog_data->original_invert_color = win_data->invert_color;
			dialog_data->original_have_custom_color = win_data->have_custom_color;
			dialog_data->original_use_custom_theme = win_data->use_custom_theme;

			// ---- Create the ANSI color buttons ----
#ifdef USE_GTK_GRID
			dialog_data->ansi_table = gtk_grid_new();
			gtk_grid_set_column_homogeneous(GTK_GRID(dialog_data->ansi_table), TRUE);
#else
			dialog_data->ansi_table = gtk_table_new (2, COLOR/2, TRUE);
#endif

			gint i;
			for (i=COLOR-1; i>=0; i--)
			{
#ifdef USE_OLD_GTK_COLOR_SELECTION
				dialog_data->color_button[i] = gtk_button_new();
				g_signal_connect(G_OBJECT(dialog_data->color_button[i]), "clicked", G_CALLBACK(update_ansi_color_info), GINT_TO_POINTER (i));
#else
				GdkRGBA color = {0};
				dialog_data->color_button[i] = gtk_color_button_new_with_rgba(&color);
				g_signal_connect(G_OBJECT(dialog_data->color_button[i]), "color-set", G_CALLBACK(adjust_vte_color_sample), GINT_TO_POINTER (i));
#endif
#ifdef USE_GTK_GRID
				gtk_grid_attach(GTK_GRID(dialog_data->ansi_table), dialog_data->color_button[i],
						(COLOR-1-i)%(COLOR/2), (COLOR-1-i)/(COLOR/2), 1, 1);
#else
				gtk_table_attach_defaults(GTK_TABLE(dialog_data->ansi_table), dialog_data->color_button[i],
							  (COLOR-1-i)%(COLOR/2), (COLOR-1-i)%(COLOR/2)+1,
							  (COLOR-1-i)/(COLOR/2), (COLOR-1-i)/(COLOR/2)+1);
#endif
			}
			init_dialog_ansi_colors_from_win_data(win_data, dialog_data);
			create_theme_color_data(dialog_data->ansi_colors, dialog_data->ansi_colors_orig,
						win_data->color_brightness, win_data->invert_color, FALSE, FALSE);

			update_color_buttons(win_data, dialog_data);
			gtk_box_pack_start (GTK_BOX(dialog_data->box), dialog_data->ansi_table, FALSE, FALSE, 0);
			gtk_window_set_focus(GTK_WINDOW(dialog_data->window), dialog_data->color_button[COLOR-1]);
			// win_data->temp_index will point to the color index in color_orig
			win_data->temp_index = get_color_index(win_data->invert_color, COLOR-1);

			g_free(tmp_topic[0]);
			g_free(tmp_topic[1]);
			break;
		}
		case CHANGE_THE_TEXT_COLOR_OF_WINDOW_TITLE:			// 11
		case CHANGE_THE_TEXT_COLOR_OF_CMDLINE:				// 12
		case CHANGE_THE_TEXT_COLOR_OF_CURRENT_DIR:			// 13
		case CHANGE_THE_TEXT_COLOR_OF_CUSTOM_PAGE_NAME:			// 14
		case CHANGE_THE_TEXT_COLOR_OF_ROOT_PRIVILEGES_CMDLINE:		// 15
		case CHANGE_THE_TEXT_COLOR_OF_NORMAL_TEXT:			// 16
		{
			Switch_Type show_tabs_bar = win_data->show_tabs_bar;
			tabs_bar_is_shown = gtk_notebook_get_show_tabs GTK_NOTEBOOK(win_data->notebook);
			if (tabs_bar_is_shown)
				win_data->show_tabs_bar = FORCE_ON;
			else
				win_data->show_tabs_bar = FORCE_OFF;

			init_locale_restrict_data(win_data->runtime_LC_MESSAGES);
			gint page_color_type = style - CHANGE_THE_TEXT_COLOR_OF_WINDOW_TITLE;
			temp_str[0] = g_strdup_printf(_("Change the %s color on tab"),
						      page_color[page_color_type].comment);
			temp_str[1] = g_strdup_printf("Change the %s color on tab",
						      page_color[page_color_type].comment);
			create_dialog(temp_str[0],
				      temp_str[1],
				      DIALOG_OK_CANCEL,
				      page_data->window,
				      FALSE,
				      FALSE,
				      15,
				      GTK_RESPONSE_OK,
				      NULL,
				      NULL,
				      selectable,
				      0,
				      FALSE,
				      BOX_HORIZONTAL,
				      0,
				      dialog_data);
			// save the dialog data first

			dialog_data->original_page_color = win_data->user_page_color[page_color_type];
			// g_debug("dialog_data->original_page_color = %s", dialog_data->original_page_color);
#ifdef SAFEMODE
			if (dialog_data->original_page_color)
#endif
				dirty_gdk_color_parse(win_data->user_page_color[page_color_type], &(dialog_data->original_color));

			// vte = the 1st page of current notebook.
			struct Page *page_data = get_page_data_from_nth_page(win_data, 0);
#ifdef SAFEMODE
			if (page_data)
			{
#endif
				create_color_selection_widget(dialog_data, (GSourceFunc)adjust_vte_color, page_data->vte);
				// store the is_bold data of 1st page...
				dialog_data->tab_1_is_bold = page_data->is_bold;
				// win_data->lost_focus = FALSE;
#ifdef SAFEMODE
			}
#endif
			// only bold the 1st page
			gboolean is_bold = TRUE;
			// the color for using in 1st page, can NOT be free()!
			gchar *current_color = NULL;
			// the text for using in 1st page, can NOT be free()!
			gchar *display_text = NULL;

			// add new pages to PAGE_COLOR+1.
			// g_debug("dialog_data->total_page = %d, current_page = %d",
			//	dialog_data->total_page, gtk_notebook_get_current_page(GTK_NOTEBOOK(win_data->notebook)));
			// The new pages will added next to 1st page!
			for (i=dialog_data->total_page; i<=PAGE_COLOR; i++)
				add_page(win_data,
					 page_data,
					 NULL,
					 NULL,
					 NULL,
					 NULL,
					 NULL,
					 NULL,
					 NULL,
					 FALSE);
			// we need to set the current page to 1st page after adding page.
			gtk_notebook_set_current_page(GTK_NOTEBOOK(win_data->notebook), 0);

			for (i=0; i<=PAGE_COLOR; i++)
			{
				// ste the page name and color for demo.
				struct Page *tmp_page = get_page_data_from_nth_page(win_data, i);
#ifdef SAFEMODE
				if (tmp_page)
				{
#endif
					dialog_data->original_update_method[i] = tmp_page->page_update_method;
					// g_debug ("??? Save: (%d) tmp_page->page_update_method = %d",
					//	i+1, tmp_page->page_update_method);
					if (i)
					{
						display_text = _(page_color[i-1].comment);
						current_color = win_data->user_page_color[i-1];
					}
					else
					{
						display_text = _("Bold Demo Text");
						current_color = dialog_data->original_page_color;
					}

					// get the demo_vte and demo_text.
					// the change of color will be shown immediately in 1st page and the page here.
					if ((i+CHANGE_THE_TEXT_COLOR_OF_WINDOW_TITLE-1)==style)
					{
						dialog_data->demo_vte = tmp_page->vte;
						dialog_data->demo_text = display_text;
					}
					// g_debug("!!! updating %d page name!", i+1);
					update_page_name(tmp_page->window,
							 tmp_page->vte,
							 NULL,
							 tmp_page->label_text,
							 i+1,
							 display_text,
							 current_color,
							 FALSE,
							 is_bold,
							 FALSE,
							 NULL,
							 tmp_page->custom_window_title,
							 FALSE);
					// only 1st page is bold.
					is_bold = FALSE;
#ifdef SAFEMODE
				}
#endif
			}
			// g_debug("Got tabs_bar_is_shown = %d", tabs_bar_is_shown);
			if (! tabs_bar_is_shown)
				hide_and_show_tabs_bar(win_data, ON);
			win_data->show_tabs_bar = show_tabs_bar;
			break;
		}
#else
		case CHANGE_THE_FOREGROUND_COLOR:
		case CHANGE_THE_BACKGROUND_COLOR:
		case CHANGE_THE_ANSI_COLORS:
		case CHANGE_THE_CURSOR_COLOR:
		case CHANGE_THE_TEXT_COLOR_OF_WINDOW_TITLE:			// 11
		case CHANGE_THE_TEXT_COLOR_OF_CMDLINE:				// 12
		case CHANGE_THE_TEXT_COLOR_OF_CURRENT_DIR:			// 13
		case CHANGE_THE_TEXT_COLOR_OF_CUSTOM_PAGE_NAME:			// 14
		case CHANGE_THE_TEXT_COLOR_OF_ROOT_PRIVILEGES_CMDLINE:		// 15
		case CHANGE_THE_TEXT_COLOR_OF_NORMAL_TEXT:			// 16
			upgrade_dialog(ENABLE_GDKCOLOR_TO_STRING_VER);
			goto FINISH;
			break;
#endif
		case CONFIRM_TO_CLOSE_MULTI_PAGES:				// 3
			if (g_list_length(window_list) > 1) create_entry_hbox = BOX_VERTICALITY;
			create_dialog(_("Confirm to close multi tabs"),
				      "Confirm to close multi tabs",
				      DIALOG_OK_CANCEL,
				      page_data->window,
				      FALSE,
				      FALSE,
				      10,
				      GTK_RESPONSE_OK,
				      GTK_FAKE_STOCK_DIALOG_QUESTION,
				      _("You are about to close multi tabs. Continue anyway?"),
				      selectable,
				      0,
				      TRUE,
				      create_entry_hbox,
				      5,
				      dialog_data);
			create_SIGKILL_and_EXIT_widget(dialog_data, FALSE, create_entry_hbox, _("those tabs"));

			break;
		case SET_KEY_BINDING:					// 4
		{
			init_locale_restrict_data(win_data->runtime_LC_MESSAGES);
			temp_str[0] = g_markup_escape_text(_("You may press <Ctrl + Alt + Shift + Win> + <any key> here\n"
							     "to get the key value used for the key binding."), -1);
			create_dialog(_("Get the key value for function key"),
				      "Get the key value for function key",
				      DIALOG_OK_CANCEL,
				      page_data->window,
				      FALSE,
				      FALSE,
				      10,
				      GTK_RESPONSE_OK,
				      NULL,
				      temp_str[0],
				      selectable,
				      0,
				      FALSE,
				      BOX_VERTICALITY,
				      5,
				      dialog_data);

			dialog_data->current_key_index = -1;
			// Copy the key values
			for (i=0; i<KEYS; i++)
				dialog_data->user_key_value[i] = g_strdup(win_data->user_keys[i].value);
			// The Key Group frame
			GtkTreeStore *store = gtk_tree_store_new (2, G_TYPE_STRING, G_TYPE_STRING);
			GtkTreeIter key_group_tree_iter[KEY_GROUP], tmp_tree_iter;
			gint key_index[KEYS]= {0};
			for (i=0; i<KEY_GROUP; i++)
			{
				if (i==KEY_GROUP_NONE) continue;
				gtk_tree_store_append (store, &(key_group_tree_iter[i]), NULL);
				gtk_tree_store_set (store, &(key_group_tree_iter[i]), 0, key_groups[i], -1);
			}
			for (i=0; i<KEYS; i++)
			{
				if (system_keys[i].group==KEY_GROUP_NONE) continue;
				gtk_tree_store_append (store, &tmp_tree_iter, &(key_group_tree_iter[system_keys[i].group]));
				gtk_tree_store_set (store, &tmp_tree_iter,
						    0, system_keys[i].topic,
						    1, win_data->user_keys[i].value,
						    -1);
				dialog_data->KeyTree[system_keys[i].group][key_index[system_keys[i].group]] = i;
				// g_debug("Set KeyTree[%d][%d] = %d", system_keys[i].group,
				//				    key_index[system_keys[i].group],
				//				    i);
				key_index[system_keys[i].group]++;
			}

			GtkTreeModel *model= GTK_TREE_MODEL(store);
			dialog_data->treeview = gtk_tree_view_new_with_model (model);
			gtk_tree_view_set_enable_search(GTK_TREE_VIEW (dialog_data->treeview), FALSE);

			g_object_unref (model);
			GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
			gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (dialog_data->treeview), -1,
								     _("Function Key Name"), renderer,
								     "text", 0, NULL);

			gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (dialog_data->treeview), -1,
								     _("Key Bindings"), renderer,
								     "text", 1, NULL);

			GtkTreeSelection *tree_selection = gtk_tree_view_get_selection(GTK_TREE_VIEW (dialog_data->treeview));
			g_signal_connect(G_OBJECT(tree_selection), "changed", G_CALLBACK(update_key_info), dialog_data);

			// Add the combo box to frame
			GtkWidget *hbox = dirty_gtk_hbox_new (FALSE, 0);
			GtkWidget *label = gtk_label_new(_("Key Group: "));
			gtk_box_pack_start (GTK_BOX(hbox), label, FALSE, FALSE, 0);
			dialog_data->operate[0] = gtk_label_new(NULL);
			set_markup_key_value(TRUE, NULL, "", dialog_data->operate[0]);
			gtk_box_pack_start (GTK_BOX(hbox), dialog_data->operate[0], FALSE, FALSE, 0);

			//// GtkWidget *vbox = create_frame_widget(dialog_data, NULL, hbox, combo, 0);
			GtkWidget *vbox = create_frame_widget(dialog_data, NULL, hbox, dialog_data->treeview, 0);
			hbox = dirty_gtk_hbox_new (FALSE, 0);
			GtkWidget *label_vbox = dirty_gtk_vbox_new (FALSE, 0);
			label = gtk_label_new(_("Note: "));
			gtk_box_pack_start (GTK_BOX(label_vbox), label, FALSE, FALSE, 0);
			gtk_box_pack_start (GTK_BOX(hbox), label_vbox, FALSE, FALSE, 0);
			dialog_data->operate[1] = gtk_label_new(NULL);
			set_markup_key_value(TRUE, "dark green", "", dialog_data->operate[1]);
			gtk_box_pack_start (GTK_BOX(hbox), dialog_data->operate[1], FALSE, FALSE, 0);
			gtk_box_pack_end (GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

			// The Key Value frame
			dialog_data->operate[2] = gtk_label_new(NULL);
			set_markup_key_value(TRUE, "blue", "", dialog_data->operate[2]);
			gtk_label_set_selectable(GTK_LABEL(dialog_data->operate[2]), TRUE);
			create_frame_widget(dialog_data, _("Key Value:"), NULL, dialog_data->operate[2], 0);

			g_signal_connect(G_OBJECT(dialog_data->window), "key-press-event",
					 G_CALLBACK(deal_dialog_key_press), dialog_data);

			dialog_data->operate[3] = create_button_with_image("", GTK_FAKE_STOCK_CLOSE, FALSE,
								      (GSourceFunc)clear_key_group, dialog_data);
			gtk_box_pack_end (GTK_BOX(dialog_data->box), dialog_data->operate[3], TRUE, TRUE, 0);
			gtk_widget_set_no_show_all (dialog_data->operate[3], TRUE);

			label = create_button_with_image(_("Disable all the function keys."),
							 GTK_FAKE_STOCK_CLOSE, FALSE, (GSourceFunc)clear_key_group_all, dialog_data);
			gtk_box_pack_end (GTK_BOX(dialog_data->box), label, TRUE, TRUE, 0);

			// Trying to get correct size of dialog_data->treeview, then put it into a gtk_scrolled_window
			gtk_widget_show_all (dialog_data->window);

			GtkRequisition requisition;
			gtk_widget_get_child_requisition (dialog_data->treeview, &requisition);
			requisition.height += BORDER_SPACE;

			GtkWidget *scrolled_window = gtk_scrolled_window_new (NULL, NULL);
			gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolled_window),
							     GTK_SHADOW_ETCHED_IN);
			gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
							GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
			g_object_ref(dialog_data->treeview);
			gtk_container_remove (GTK_CONTAINER(vbox), dialog_data->treeview);
			gtk_box_pack_start (GTK_BOX(vbox), scrolled_window, FALSE, FALSE, 0);
			gtk_container_add (GTK_CONTAINER (scrolled_window), dialog_data->treeview);
			gtk_widget_set_size_request(scrolled_window,
						    requisition.width, requisition.height);
			g_object_unref(dialog_data->treeview);

			break;
		}
		case USAGE_MESSAGE:						// 5
		{
			selectable = TRUE;
			gchar *str[18] = {NULL};
			gchar *temp_str;

			// Create dialog
			str[0] = g_strdup_printf(_("Usage for %s %s"),PACKAGE, VERSION);
			str[1] = g_strdup_printf("Usage for %s %s",PACKAGE, VERSION);
			create_dialog(str[0],
				      str[1],
				      DIALOG_OK,
				      page_data->window,
				      FALSE,
				      FALSE,
				      0,
				      GTK_RESPONSE_OK,
				      NULL,
				      NULL,
				      selectable,
				      0,
				      FALSE,
				      BOX_VERTICALITY,
				      0,
				      dialog_data);
			// Notebook
			GtkWidget *notebook = gtk_notebook_new();
			gtk_container_add (GTK_CONTAINER (gtk_dialog_get_content_area(GTK_DIALOG(dialog_data->window))), notebook);
			gtk_container_set_border_width (GTK_CONTAINER (notebook), 5);

			// Usage
			str[2] = get_help_message_usage(win_data->profile, TRUE);
			temp_str =_("Don't forget to save your settings after making any change!");
			str[3] = convert_text_to_html (&temp_str, FALSE, "darkred", "tt", "b", NULL);
			str[4] = g_strdup_printf("%s\n\n%s", str[2], str[3]);
			dialog_data->operate[0] = add_text_to_notebook(notebook, _("Usage"), GTK_FAKE_STOCK_HELP, str[4]);

			// Shortcut Keys
			str[5] = get_help_message_key_binding(TRUE);
			dialog_data->operate[1] = add_text_to_notebook(notebook, _("Key binding"), GTK_FAKE_STOCK_PREFERENCES, str[5]);

			// License
			// TRANSLATE NOTE: The following license declaration is just a notice, not full license text.
			// TRANSLATE NOTE: The full GPLv3 license text is included in LilyTerm source code.
			// TRANSLATE NOTE: Of corse, Translating the GPLv3 license text won't be a good idea,
			// TRANSLATE NOTE: But it seems be safe to translate the following license declaration.
			// TRANSLATE NOTE: Please just skip and leave it alone if you don't wanna translate it.
			str[6] = g_strdup_printf(_("Copyright (c) %s-%s  %s.  All rights reserved.\n\n"
						   "%s is free software: you can redistribute it and/or modify\n"
						   "it under the terms of the GNU General Public License as published by\n"
						   "the Free Software Foundation, either version 3 of the License, or\n"
						   "(at your option) any later version.\n\n"
						   "%s is distributed in the hope that it will be useful,\n"
						   "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
						   "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
						   "GNU General Public License for more details.\n\n"
						   "You should have received a copy of the GNU General Public License\n"
						   "along with %s.  If not, see <http://www.gnu.org/licenses/>."),
						   "2008", YEAR, AUTHOR, PACKAGE, PACKAGE, PACKAGE);
			str[7] = convert_text_to_html(&str[6], FALSE, NULL, "tt", NULL);
			dialog_data->operate[2] = add_text_to_notebook(notebook, _("License"), GTK_FAKE_STOCK_DIALOG_AUTHENTICATION, str[7]);

			// Translators
			temp_str = "Adrian Buyssens: Flemish/Dutch translation.\n"
				   "Bogusz Kowalski: Polish translation.\n"
				   "GoGoNKT: Simplified Chinese translation.\n"
				   "Marco Paolome: Italian translation.\n"
				   "Mario Blättermann: German translation.\n"
				   "Niels Martignène: Franch translation.\n"
				   "P.L. Francisco: Spanish translation.\n"
				   "Rafael Ferreira: Portuguese translation.\n"
				   "Samed Beyribey: Turkish translation.\n"
				   "Slavko: Slovak translation.\n"
				   "Vladimir Smolyar: Russian and Ukrainian translations.";
			str[8] = convert_text_to_html(&temp_str, FALSE, NULL, "tt", NULL);
			dialog_data->operate[3] = add_text_to_notebook(notebook, _("Translators"), GTK_FAKE_STOCK_CONVERT, str[8]);

			// About
			// TRANSLATE NOTE: Please be care of the spacing when translating the following site informatoins.
			// TRANSLATE NOTE: Please check it in [Right Click Menu] -> [Usage] -> [About] page after translating.
			str[9]  = g_strdup_printf (_("HomePage: %s"), MAINSITE);
			str[10] = g_strdup_printf (_("Github:   %s"), GITHUBURL);
			str[11] = g_strdup_printf (_("Blog:     %s"), BLOG);
			str[12] = g_strdup_printf (_("Wiki:     %s"), WIKI);
			str[13] = g_strdup_printf (_("Issues:   %s"), ISSUES);
			str[14] = g_strdup_printf (_("IRC:      %s"), IRC);
			str[15] = g_strdup_printf (_("Thank you for using %s!"), PACKAGE);
			str[16] = g_strdup_printf ("%s <%s>\n\n%s\n%s\n"
						   "%s\n%s\n\n%s",
						   AUTHOR, BUGREPORT, str[9] ,str[10], str[13],
						   str[14], str[15]);
			str[17] = convert_text_to_html(&str[16], FALSE, NULL, "tt", NULL);
			dialog_data->operate[4] = add_text_to_notebook(notebook, _("About"), GTK_FAKE_STOCK_ABOUT, str[17]);

			show_usage_text(notebook, NULL, 0, dialog_data);
			g_signal_connect(G_OBJECT(notebook), "switch-page", G_CALLBACK(show_usage_text), dialog_data);

			for (i=0; i<18; i++) g_free(str[i]);

			break;
		}
		case CONFIRM_TO_CLOSE_RUNNING_APPLICATION:			// 7
		{
			selectable = TRUE;
			// get the command line of running foreground program
			pid_t tpgid = get_tpgid(page_data->pid);
			temp_str[0] = get_cmdline(tpgid);
			temp_str[1] = g_strdup_printf("(%d) %s", tpgid, temp_str[0]);
			temp_str[2] = g_markup_escape_text(temp_str[1], -1);
			temp_str[3] = g_strdup_printf(_("There is still a running foreground program on #%d tab:"),
							 page_data->page_no+1);
			temp_str[4] = g_strdup_printf(_("Terminate it by using `kill -9 %d' is recommended "
							    "if it is non-response."), page_data->pid);
			temp_str[5] = g_strconcat(temp_str[3],
						  "\n\n\t<b><span foreground=\"blue\">",
						  temp_str[2],
						  "</span></b>\n\n",
						  _("Continue anyway?"),
						  "\n\n<span foreground=\"darkred\">",
						  _("Tip:"),
						  " ",
						  temp_str[4],
						  "</span>",
						  NULL);

			if ((g_list_length(window_list) > 1) || (dialog_data->total_page>1))
				create_entry_hbox = BOX_VERTICALITY;

			create_dialog(_("Confirm to close running foreground program"),
				      "Confirm to close running foreground program",
				      DIALOG_OK_CANCEL,
				      page_data->window,
				      FALSE,
				      FALSE,
				      10,
				      GTK_RESPONSE_OK,
				      GTK_FAKE_STOCK_DIALOG_QUESTION,
				      temp_str[5],
				      selectable,
				      70,
				      TRUE,
				      create_entry_hbox,
				      5,
				      dialog_data);

			create_SIGKILL_and_EXIT_widget(dialog_data, FALSE, create_entry_hbox, _("this tab"));
			break;
		}
		case CHANGE_THE_OPACITY_OF_WINDOW:				// 8
		case CHANGE_THE_OPACITY_OF_WINDOW_WHEN_INACTIVE:
		{
#ifdef ENABLE_RGBA
			gchar *translated_title = _("Change the opacity of window");
			gchar *title = "Change the opacity of window";
			gchar *text = _("Change the opacity of window:");
			gdouble window_opacity = win_data->window_opacity;

			if (style==CHANGE_THE_OPACITY_OF_WINDOW)
			{
				dialog_data->original_transparent_window = win_data->transparent_window;
				win_data->transparent_window = TRUE;
			}
			else
			{
				translated_title = _("Change the opacity of window when inactive");
				title = "Change the opacity of window when inactive";
				text = _("Change the opacity of window when inactive:");
				window_opacity = win_data->window_opacity_inactive;
				dialog_data->original_transparent_window = win_data->transparent_window_inactive;
				win_data->transparent_window_inactive = TRUE;
				dialog_data->original_dim_window = win_data->dim_window;
				win_data->dim_window = TRUE;
			}

			create_dialog(translated_title,
				      title,
				      DIALOG_OK_CANCEL,
				      page_data->window,
				      FALSE,
				      TRUE,
				      10,
				      GTK_RESPONSE_OK,
				      GTK_FAKE_STOCK_DIALOG_INFO,
				      text,
				      selectable,
				      0,
				      TRUE,
				      BOX_VERTICALITY,
				      0,
				      dialog_data);
			create_scale_widget(dialog_data, 0, 1, 0.001,
					    window_opacity,
					    (GSourceFunc)set_window_opacity,
					    win_data);
			dialog_data->original_window_opacity = window_opacity;
			// g_debug("window_opacity = %1.3f", window_opacity);
			set_window_opacity(NULL, 0, window_opacity, win_data);
#else
			upgrade_dialog(ENABLE_RGBA_VER);
			goto FINISH;
#endif
			break;
		}
		case ADJUST_THE_BRIGHTNESS_OF_ANSI_COLORS_USED_IN_TERMINAL:	// 19
		case ADJUST_THE_BRIGHTNESS_OF_ANSI_COLORS_WHEN_INACTIVE:	// 26
		{
			gchar *title[2] = {NULL};
			gdouble value = win_data->color_brightness;
			dialog_data->original_color_brightness = win_data->color_brightness;
			dialog_data->original_color_brightness_inactive = win_data->color_brightness_inactive;
			dialog_data->original_dim_text = win_data->dim_text;

			if (style == ADJUST_THE_BRIGHTNESS_OF_ANSI_COLORS_USED_IN_TERMINAL)
			{
				title[0] = _("Adjust the brightness of ANSI colors used in terminal");
				title[1] = "Adjust the brightness of ANSI colors used in terminal";
			}
			else
			{
				title[0] = _("Adjust the brightness of ANSI colors when inactive");
				title[1] = "Adjust the brightness of ANSI colors when inactive";
				value = win_data->color_brightness_inactive;
				if (win_data->use_custom_theme)
					set_new_ansi_color(win_data->current_vte, dialog_data->ansi_colors,
							   win_data->custom_color_theme[win_data->color_theme_index].color,
							   value, win_data->invert_color, FALSE, win_data->custom_cursor_color,
							   win_data->cursor_color, TRUE);
				else
					set_new_ansi_color(win_data->current_vte, dialog_data->ansi_colors,
							   system_color_theme[win_data->color_theme_index].color,
							   value, win_data->invert_color, FALSE, win_data->custom_cursor_color,
							   win_data->cursor_color, TRUE);
			}
			create_dialog(title[0],
				      title[1],
				      DIALOG_OK_CANCEL,
				      page_data->window,
				      FALSE,
				      TRUE,
				      10,
				      GTK_RESPONSE_OK,
				      GTK_FAKE_STOCK_DIALOG_INFO,
				      title[0],
				      selectable,
				      0,
				      TRUE,
				      BOX_VERTICALITY,
				      0,
				      dialog_data);
			// g_debug("value = %0.3f, win_data->color_brightness = %0.3f, win_data->color_brightness_inactive = %0.3f",
			//	value, win_data->color_brightness, win_data->color_brightness_inactive);
			create_scale_widget(dialog_data, -1, 1, 0.001,
					    value,
					    (GSourceFunc)set_ansi_color,
					    win_data->current_vte);
			init_dialog_ansi_colors_from_win_data(win_data, dialog_data);
			break;
		}
		case CONFIRM_TO_EXECUTE_COMMAND:				// 22
			// g_debug("error_data.string = %s", error_data.string);
			// g_debug("error_data.encoding = %s", error_data.encoding);
			temp_str[0] = g_markup_escape_text(win_data->temp_data, -1);
			temp_str[1] = g_strconcat(PACKAGE,
						  _(" is trying to execute the following command:"),
						  "\n\n\t<b><span foreground=\"",
						  "blue",
						  "\">",
						  temp_str[0],
						  "</span></b>\n\n",
						  _("Continue anyway?"),
						  NULL);

			create_dialog(_("Confirm to execute -e/-x/--execute command"),
				      "Confirm to execute -e/-x/--execute command",
				      DIALOG_OK_CANCEL,
				      real_menu_active_window,
				      TRUE,
				      FALSE,
				      10,
				      GTK_RESPONSE_CANCEL,
				      GTK_FAKE_STOCK_DIALOG_WARNING,
				      temp_str[1],
				      FALSE,
				      70,
				      TRUE,
				      BOX_NONE,
				      0,
				      dialog_data);
			break;
		case CONFIRM_TO_CLOSE_A_TAB_WITH_CHILD_PROCESS:		// 23
			selectable = TRUE;
			// get the command line of running foreground program
			temp_str[0] = g_strdup_printf(_("The following programs are still running under #%d tab:"),
							 page_data->page_no+1);
			temp_str[1] = g_markup_escape_text(_("Using <Ctrl><D> or 'exit' to close tabs is recommended"),
							     -1);
			temp_str[2] = g_markup_escape_text(win_data->temp_data, -1);
			temp_str[3] = g_strconcat(temp_str[0],
						  "\n\n<b><span foreground=\"blue\">",
						  temp_str[2],
						  "</span></b>\n",
						  _("Continue anyway?"),
						  "\n\n<span foreground=\"darkred\">",
						  _("Tip:"),
						  " ",
						  temp_str[1],
						  "</span>",
						  NULL);

			create_dialog(_("Confirm to close a tab with child process"),
				      "Confirm to close a tab with child process",
				      DIALOG_OK_CANCEL,
				      page_data->window,
				      FALSE,
				      FALSE,
				      10,
				      GTK_RESPONSE_CANCEL,
				      GTK_FAKE_STOCK_DIALOG_QUESTION,
				      temp_str[3],
				      TRUE,
				      70,
				      selectable,
				      BOX_VERTICALITY,
				      5,
				      dialog_data);

			create_entry_hbox = ((g_list_length(window_list) > 1) || (dialog_data->total_page>1));
			create_SIGKILL_and_EXIT_widget(dialog_data, TRUE, create_entry_hbox, _("this tab"));

			break;
		case CONFIRM_TO_CLOSE_A_WINDOW_WITH_CHILD_PROCESS:		// 24
			selectable = TRUE;
			temp_str[0] = g_markup_escape_text(win_data->temp_data, -1);
			temp_str[1] = g_strconcat(_("The following programs are still running under this window:"),
						  "\n\n<b><span foreground=\"blue\">",
						  temp_str[0],
						  "</span></b>\n",
						  _("Continue anyway?"),
						  NULL);
			create_dialog(_("Confirm to close a window with child process"),
				      "Confirm to close a window with child process",
				      DIALOG_OK_CANCEL,
				      page_data->window,
				      FALSE,
				      FALSE,
				      10,
				      GTK_RESPONSE_CANCEL,
				      GTK_FAKE_STOCK_DIALOG_QUESTION,
				      temp_str[1],
				      selectable,
				      70,
				      TRUE,
				      BOX_VERTICALITY,
				      5,
				      dialog_data);
			create_entry_hbox = (g_list_length(window_list) > 1);
			create_SIGKILL_and_EXIT_widget(dialog_data, TRUE, create_entry_hbox, _("those tabs"));
			break;
		case CONFIRM_TO_EXIT_WITH_CHILD_PROCESS:			// 25
			selectable = TRUE;
			temp_str[0]  = g_strdup_printf(_("Confirm to close %s"), PACKAGE);
			temp_str[1]  = g_strdup_printf("Confirm to close %s", PACKAGE);
			// get the command line of running foreground program
			temp_str[2] = g_strdup_printf(_("The following programs are still running under %s:"),
						      PACKAGE);
			temp_str[3] = g_markup_escape_text(win_data->temp_data, -1);
			temp_str[4] = g_strconcat(temp_str[2] ,
						  "\n\n<b><span foreground=\"blue\">",
						  temp_str[3],
						  "</span></b>\n",
						  _("Continue anyway?"),
						  NULL);
			create_dialog(temp_str[0],
				      temp_str[1],
				      DIALOG_OK_CANCEL,
				      page_data->window,
				      FALSE,
				      FALSE,
				      10,
				      GTK_RESPONSE_CANCEL,
				      GTK_FAKE_STOCK_DIALOG_QUESTION,
				      temp_str[4],
				      selectable,
				      70,
				      TRUE,
				      BOX_VERTICALITY,
				      5,
				      dialog_data);
			create_SIGKILL_and_EXIT_widget(dialog_data, TRUE, TRUE, PACKAGE);
			break;
		case CONFIRM_TO_PASTE_TEXTS_TO_VTE_TERMINAL:
			temp_str[0] = g_strconcat(_("Trying to paste the following texts to the Vte Terminal:"),
						  "\n\n",
						  win_data->temp_data,
						  "\n\n",
						  _("Continue anyway?"),
						  "\n",
						  NULL);
			create_dialog(_("Confirm to paste texts to the Vte Terminal"),
				      "Confirm to paste texts to the Vte Terminal",
				      DIALOG_OK_CANCEL,
				      page_data->window,
				      FALSE,
				      FALSE,
				      10,
				      GTK_RESPONSE_OK,
				      GTK_FAKE_STOCK_DIALOG_QUESTION,
				      temp_str[0],
				      selectable,
				      70,
				      FALSE,
				      BOX_NONE,
				      0,
				      dialog_data);
			// <Join and paste> Button
			GtkWidget *join_button = gtk_dialog_add_button (GTK_DIALOG(dialog_data->window),
									_("Join"), GTK_RESPONSE_ACCEPT);
#ifdef ENABLE_SET_TOOLTIP_TEXT
			gtk_widget_set_tooltip_text(join_button, _("Remove '<Return>' in the text"));
#endif
			gtk_button_set_image (GTK_BUTTON(join_button),
					      gtk_image_new_from_stock(GTK_FAKE_STOCK_PASTE, GTK_ICON_SIZE_BUTTON));
#ifdef HAVE_GTK_DIALOG_GET_ACTION_AREA
			gtk_button_box_set_child_secondary (GTK_BUTTON_BOX(gtk_dialog_get_action_area(GTK_DIALOG(dialog_data->window))),
							    join_button, TRUE);
#endif

			// <Strip and paste> Button
			GtkWidget *strip_button = gtk_dialog_add_button (GTK_DIALOG(dialog_data->window),
									_("Strip"), GTK_RESPONSE_YES);
#ifdef ENABLE_SET_TOOLTIP_TEXT
			gtk_widget_set_tooltip_text(strip_button, _("Remove '<Return>', \"\\<Return>\", and \"\\<Return><<Space>\" in the text, then join '<Tab>' and '<Space>' into a single '<Space>'"));
#endif
			gtk_button_set_image (GTK_BUTTON(strip_button),
					      gtk_image_new_from_stock(GTK_FAKE_STOCK_PASTE, GTK_ICON_SIZE_BUTTON));
#ifdef HAVE_GTK_DIALOG_GET_ACTION_AREA
			gtk_button_box_set_child_secondary (GTK_BUTTON_BOX(gtk_dialog_get_action_area(GTK_DIALOG(dialog_data->window))),
							    strip_button, TRUE);
#endif
			break;
		case GENERAL_INFO:
		{
			selectable = TRUE;
			gchar **strings = split_string(win_data->temp_data, "\x10", 3);
			if (strings==NULL) goto FINISH;
			create_dialog(strings[0],
				      strings[1],
				      DIALOG_OK,
				      page_data->window,
				      FALSE,
				      FALSE,
				      10,
				      GTK_RESPONSE_OK,
				      NULL,
				      strings[2],
				      selectable,
				      70,
				      FALSE,
				      BOX_NONE,
				      0,
				      dialog_data);
			g_strfreev(strings);
			break;
		}
		default:
#ifdef FATAL
			print_switch_out_of_range_error_dialog("dialog", "style", style);
#endif
			goto FINISH;
	}

	// We'll use gtk_dialog_run() to show the whole dialog
	// g_debug("Finish creating dialog!");
	gtk_widget_show_all (dialog_data->window);

	// GtkRequisition requisition;
	// gtk_window_get_size(GTK_WINDOW(dialog_data->window),
	//		    &requisition.width, &requisition.height);
	// g_debug("! The final window size is %d x %d",
	//	requisition.width, requisition.height);

	// FIXME: Due to a bug in the GTK+2(?), we should set the ellipsize after the dialog is shown.
	// Or, the vertical size of title label may be cut, and some text may not shown.
	if ((dialog_data->title_label) && (gtk_label_get_max_width_chars(GTK_LABEL(dialog_data->title_label)) > 0))
		gtk_label_set_ellipsize(GTK_LABEL(dialog_data->title_label), PANGO_ELLIPSIZE_MIDDLE);

#ifdef UNIT_TEST
	for (dialog_response=GTK_RESPONSE_HELP; dialog_response<=GTK_RESPONSE_NONE; dialog_response++)
	{
#else
	dialog_response = gtk_dialog_run(GTK_DIALOG(dialog_data->window));
#endif

	switch (dialog_response)
	{
		case GTK_RESPONSE_OK:
		{
			switch (style)
			{
				// style 1: change the tab's name
				case EDIT_LABEL:
					// g_debug("Got label name: %s",gtk_entry_get_text(GTK_ENTRY(entry)));
					g_free(page_data->custom_page_name);

					const gchar *text = gtk_entry_get_text(GTK_ENTRY(dialog_data->operate[0]));
#ifdef SAFEMODE
					if ((text) && (text[0]!='\0'))
#else
					if (text[0]!='\0')
#endif
						// store page_data->page_name
						page_data->custom_page_name = g_strdup(text);
					else
						page_data->custom_page_name = NULL;

					// g_debug("Get the page name = %s, color = %s",
					// 	page_data->custom_page_name, page_data->tab_color);
					if (page_data->page_update_method == PAGE_METHOD_WINDOW_TITLE)
						page_data->window_title_updated = 1;
					get_and_update_page_name(page_data, FALSE);

					break;
				case ADD_NEW_LOCALES:
				{
					const gchar *text = gtk_entry_get_text(GTK_ENTRY(dialog_data->operate[0]));
					g_free(win_data->default_locale);
					win_data->default_locale = g_strdup(text);

					text = gtk_entry_get_text(GTK_ENTRY(dialog_data->operate[1]));
					g_free(win_data->locales_list);
					win_data->locales_list = g_strdup(text);

					gtk_widget_destroy(dialog_data->window);
					dialog_data->window = NULL;

					refresh_locale_and_encoding_list(win_data);
					break;
				}
#if defined(ENABLE_VTE_BACKGROUND) || defined(FORCE_ENABLE_VTE_BACKGROUND)
				// style  2: change the saturation of background
				case CHANGE_BACKGROUND_SATURATION:
#endif
				// style  9: change the foreground color
				case CHANGE_THE_FOREGROUND_COLOR:
				// style 10: change the background color
				case CHANGE_THE_BACKGROUND_COLOR:
				// style  9: change the cursor color
				case CHANGE_THE_CURSOR_COLOR:
				// style 11: change the text color of window title
				case CHANGE_THE_TEXT_COLOR_OF_WINDOW_TITLE:
				// style 12: change the text color of cmdline
				case CHANGE_THE_TEXT_COLOR_OF_CMDLINE:
				// style 13: change the text color of current dir
				case CHANGE_THE_TEXT_COLOR_OF_CURRENT_DIR:
				// style 14: change the text color of custom page name
				case CHANGE_THE_TEXT_COLOR_OF_CUSTOM_PAGE_NAME:
				// style 15: change the text color of root privileges cmdline
				case CHANGE_THE_TEXT_COLOR_OF_ROOT_PRIVILEGES_CMDLINE:
				// style 16: change the text color of normal text
				case CHANGE_THE_TEXT_COLOR_OF_NORMAL_TEXT:
				{
					// g_debug("Setting the colors. Type = %d", style);
					GdkRGBA new_color;
					switch (style)
					{
						case CHANGE_THE_FOREGROUND_COLOR:
						case CHANGE_THE_BACKGROUND_COLOR:
						case CHANGE_THE_CURSOR_COLOR:
							gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(dialog_data->operate[0]), &new_color);
							break;
					}
					switch (style)
					{
#if defined(ENABLE_VTE_BACKGROUND) || defined(FORCE_ENABLE_VTE_BACKGROUND)
						case CHANGE_BACKGROUND_SATURATION:
							win_data->background_saturation = gtk_range_get_value(GTK_RANGE(dialog_data->operate[0])) + 0.0005;
							break;
#endif
#ifdef ENABLE_GDKCOLOR_TO_STRING
						case CHANGE_THE_FOREGROUND_COLOR:
							if (win_data->use_custom_theme == FALSE) clear_custom_colors_data(win_data, TRUE);
							update_fg_bg_color(win_data, new_color, TRUE);
							break;
						case CHANGE_THE_BACKGROUND_COLOR:
							if (win_data->use_custom_theme == FALSE) clear_custom_colors_data(win_data, FALSE);
							update_fg_bg_color(win_data, new_color, FALSE);
							break;
						case CHANGE_THE_CURSOR_COLOR:
#ifdef USE_OLD_GTK_COLOR_SELECTION
							win_data->custom_cursor_color = ! gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dialog_data->custom_cursor_color_checkbox));
#else
							win_data->custom_cursor_color = TRUE;
#endif
							win_data->cursor_color = new_color;
							break;
						case CHANGE_THE_TEXT_COLOR_OF_WINDOW_TITLE:
						case CHANGE_THE_TEXT_COLOR_OF_CMDLINE:
						case CHANGE_THE_TEXT_COLOR_OF_CURRENT_DIR:
						case CHANGE_THE_TEXT_COLOR_OF_CUSTOM_PAGE_NAME:
						case CHANGE_THE_TEXT_COLOR_OF_ROOT_PRIVILEGES_CMDLINE:
						case CHANGE_THE_TEXT_COLOR_OF_NORMAL_TEXT:
						{
							gint page_color_type = style - CHANGE_THE_TEXT_COLOR_OF_WINDOW_TITLE;
							gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(dialog_data->operate[0]), &new_color);
							gchar *new_color_str = dirty_gdk_rgba_to_string(&(new_color));

							struct Page *tmp_page_data = NULL;
							for (i=0; i<gtk_notebook_get_n_pages(GTK_NOTEBOOK(win_data->notebook)); i++)
							{
								tmp_page_data = get_page_data_from_nth_page(win_data, i);
#ifdef SAFEMODE
								if (tmp_page_data==NULL) goto DESTROY_WINDOW;
#endif
								if (tmp_page_data->tab_color == win_data->user_page_color[page_color_type])
									tmp_page_data->tab_color = new_color_str;
							}

							g_free(win_data->user_page_color[page_color_type]);
							win_data->user_page_color[page_color_type] = new_color_str;
							break;
						}
#endif
					}
					if (style>=CHANGE_THE_TEXT_COLOR_OF_WINDOW_TITLE &&
					    style<=CHANGE_THE_TEXT_COLOR_OF_NORMAL_TEXT)
						// recover the color of pages
						recover_page_colors(dialog_data->window, page_data->window, page_data->notebook);
					else
					{
						struct Page *tmp_page_data = NULL;
						// change the saturation of background and foreground/background color
						for (i=0; i<gtk_notebook_get_n_pages(GTK_NOTEBOOK(win_data->notebook)); i++)
						{
							tmp_page_data = get_page_data_from_nth_page(win_data, i);
#ifdef SAFEMODE
							if (tmp_page_data==NULL) goto DESTROY_WINDOW;
#endif
							switch (style)
							{
#if defined(ENABLE_VTE_BACKGROUND) || defined(FORCE_ENABLE_VTE_BACKGROUND)
								case CHANGE_BACKGROUND_SATURATION:
									set_background_saturation (NULL, 0,
												   win_data->background_saturation,
												   tmp_page_data->vte);
									break;
#endif
								case CHANGE_THE_FOREGROUND_COLOR:
								case CHANGE_THE_BACKGROUND_COLOR:
									adjust_vte_color(GTK_COLOR_CHOOSER(dialog_data->operate[0]),
											 NULL,
											 tmp_page_data->vte);
									break;
								case CHANGE_THE_CURSOR_COLOR:
#ifdef USE_OLD_GTK_COLOR_SELECTION
									adjust_vte_color(NULL,
											 NULL,
											 tmp_page_data->vte);
#else
									adjust_vte_color(GTK_COLOR_CHOOSER(dialog_data->operate[0]),
											 NULL,
											 tmp_page_data->vte);
#endif
									break;
							}
						}
					}
					break;
				}
				// style 3: confirm to close multi pages
				case CONFIRM_TO_CLOSE_MULTI_PAGES:
				{
					// we need to destroy the dialog before confirm a working vte.
					if ((create_entry_hbox) && gtk_toggle_button_get_active(
									GTK_TOGGLE_BUTTON(dialog_data->operate[0])))
					{
						// g_debug ("CONFIRM_TO_CLOSE_MULTI_PAGES: call gtk_main_quit()");
						quit_gtk();
						force_to_quit = TRUE;
						// g_debug("Set force_to_quit to %d", force_to_quit);
					}

					gtk_widget_destroy(dialog_data->window);
					dialog_data->window = NULL;
					close_multi_tabs(win_data, FALSE);

					break;
				}
				// style 4: get function key value
				case SET_KEY_BINDING:
				{
					for (i=0; i<KEYS; i++)
						convert_string_to_user_key(i, dialog_data->user_key_value[i], win_data);
					break;
				}
				// style 23: confirm to close a tab with child process
				case CONFIRM_TO_CLOSE_A_TAB_WITH_CHILD_PROCESS:
				// style 24: confirm to close a window with child process
				case CONFIRM_TO_CLOSE_A_WINDOW_WITH_CHILD_PROCESS:
				// style 25: confirm to close LilyTerm with child process
				case CONFIRM_TO_EXIT_WITH_CHILD_PROCESS:
					if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dialog_data->operate[1])))
						using_kill = TRUE;
					if ((style==CONFIRM_TO_CLOSE_A_WINDOW_WITH_CHILD_PROCESS) ||
					    (style==CONFIRM_TO_EXIT_WITH_CHILD_PROCESS))
						force_to_quit = TRUE;
				// style  7: confirm to close running application
				case CONFIRM_TO_CLOSE_RUNNING_APPLICATION:
					if ((create_entry_hbox) && (gtk_toggle_button_get_active(
									GTK_TOGGLE_BUTTON(dialog_data->operate[0]))))
					{
						// g_debug ("CONFIRM_TO_CLOSE_RUNNING_APPLICATION(): call gtk_main_quit()");
						quit_gtk();
						force_to_quit = TRUE;
					}
					break;
#ifdef ENABLE_RGBA
				// style 8: change the opacity of window
				case CHANGE_THE_OPACITY_OF_WINDOW:
					win_data->window_opacity =
						gtk_range_get_value(GTK_RANGE(dialog_data->operate[0]))+0.0005;
					break;
				case CHANGE_THE_OPACITY_OF_WINDOW_WHEN_INACTIVE:
					win_data->window_opacity_inactive =
						gtk_range_get_value(GTK_RANGE(dialog_data->operate[0]))+0.0005;
					set_window_opacity( NULL, 0, win_data->window_opacity, win_data);
					win_data->dim_window = TRUE;
					break;
				// style 19: Adjust the brightness of ANSI colors used in terminal
				case CHANGE_THE_ANSI_COLORS:
				case ADJUST_THE_BRIGHTNESS_OF_ANSI_COLORS_USED_IN_TERMINAL:
				{
					if (style==CHANGE_THE_ANSI_COLORS)
					{
						// g_debug("win_data->color_brightness = %0.3f, win_data->color_brightness_inactive = %0.3f",
						//	win_data->color_brightness, win_data->color_brightness_inactive);
						gint i;
						for (i=0; i<COLOR; i++)
						{
							gint j;
							if (compare_color(&(system_color_theme[win_data->color_theme_index].color[i]),
									  &(dialog_data->ansi_colors_orig[i])))
							{
								// print_color(i, "Color had been changed", dialog_data->ansi_colors_orig[i]);

								for (j=0; j<THEME; j++)
									win_data->custom_color_theme[j].color[i] = dialog_data->ansi_colors_orig[i];

								win_data->have_custom_color = TRUE;
								win_data->use_custom_theme = TRUE;
							}
							else
							{
								// restore to the system color theme
								for (j=0; j<THEME; j++)
									win_data->custom_color_theme[j].color[i] = system_color_theme[j].color[i];
							}
						}

						// g_debug("dialog_data->original_have_custom_color = %d, win_data->have_custom_color = %d,"
						//	" dialog_data->original_use_custom_theme = %d, win_data->use_custom_theme = %d",
						//	dialog_data->original_have_custom_color, win_data->have_custom_color,
						//	dialog_data->original_use_custom_theme, win_data->use_custom_theme);
						if ((dialog_data->original_have_custom_color != win_data->have_custom_color) ||
						    (dialog_data->original_use_custom_theme != win_data->use_custom_theme))
							recreate_theme_menu_items(win_data);
					}

					generate_all_color_datas(win_data);

					struct Page *page_data;
					gboolean default_vte_color = use_default_vte_theme(win_data);
					for (i=0; i<gtk_notebook_get_n_pages(GTK_NOTEBOOK(win_data->notebook)); i++)
					{
						page_data = get_page_data_from_nth_page(win_data, i);
#ifdef SAFEMODE
						if (page_data==NULL) goto DESTROY_WINDOW;
#endif
						set_vte_color(page_data->vte, default_vte_color, win_data->custom_cursor_color,
							      win_data->cursor_color, win_data->color, FALSE, FALSE);
					}
					// g_debug("win_data->color_brightness = %0.3f, win_data->color_brightness_inactive = %0.3f",
					//	win_data->color_brightness, win_data->color_brightness_inactive);
					break;
				}
				case ADJUST_THE_BRIGHTNESS_OF_ANSI_COLORS_WHEN_INACTIVE:
					win_data->color_brightness_inactive = win_data->color_brightness;
					win_data->color_brightness = dialog_data->original_color_brightness;
					// g_debug("win_data->color_brightness = %0.3f, win_data->color_brightness_inactive = %0.3f",
					//	win_data->color_brightness, win_data->color_brightness_inactive);
					win_data->dim_text = TRUE;
					generate_all_color_datas(win_data);
					set_vte_color(win_data->current_vte, use_default_vte_theme(win_data),
						      win_data->custom_cursor_color, win_data->cursor_color, win_data->color, FALSE, FALSE);
					break;
#endif
			}
			break;
		}
		case GTK_RESPONSE_ACCEPT:
			break;
		case GTK_RESPONSE_YES:
			break;
		default:
		{
			switch (style)
			{
				// FIXME: vte_terminal_search_set_gregex(VTE_TERMINAL(win_data->current_vte), NULL) don't work.
				// case FIND_STRING:
				//	vte_terminal_search_set_gregex(VTE_TERMINAL(win_data->current_vte), NULL);
				//	vte_terminal_search_find_previous(VTE_TERMINAL(win_data->current_vte));
				//	break;
#if defined(ENABLE_VTE_BACKGROUND) || defined(FORCE_ENABLE_VTE_BACKGROUND)
				// style  2: change the saturation of background
				case CHANGE_BACKGROUND_SATURATION:
#endif
				// style  9: change the foreground color
				case CHANGE_THE_FOREGROUND_COLOR:
				// style 10: change the background color
				case CHANGE_THE_BACKGROUND_COLOR:
				// style  9: change the cursor color
				case CHANGE_THE_CURSOR_COLOR:
#if defined(ENABLE_VTE_BACKGROUND) || defined(FORCE_ENABLE_VTE_BACKGROUND)
					if (style==CHANGE_BACKGROUND_SATURATION)
					{
						win_data->transparent_background = dialog_data->original_transparent_background;
						// g_debug("transparent_background = %d", transparent_background);
						set_background_saturation(NULL,
									  0,
									  win_data->background_saturation,
									  win_data->current_vte);
					}
#endif
					// print_color(-1, "RECOVER: dialog_data->original_color", dialog_data->original_color);
					if (style==CHANGE_THE_CURSOR_COLOR)
						win_data->custom_cursor_color = dialog_data->original_custom_cursor_color;
#if defined(ENABLE_VTE_BACKGROUND) || defined(FORCE_ENABLE_VTE_BACKGROUND)
					if (style!=CHANGE_BACKGROUND_SATURATION)
					{
						dialog_data->recover = TRUE;
						adjust_vte_color(GTK_COLOR_CHOOSER(dialog_data->operate[0]), &(dialog_data->original_color), win_data->current_vte);
					}
#endif
					break;
				// style 4: get function key value
				case SET_KEY_BINDING:
				{
					for (i=0; i<KEYS; i++)
						g_free(dialog_data->user_key_value[i]);
					break;
				}

#ifdef ENABLE_RGBA
				// style  8: change the opacity of window
				case CHANGE_THE_OPACITY_OF_WINDOW:
					win_data->transparent_window = dialog_data->original_transparent_window;
					win_data->window_opacity = dialog_data->original_window_opacity;
					dim_window(win_data, 2);
					break;
				case CHANGE_THE_OPACITY_OF_WINDOW_WHEN_INACTIVE:
					win_data->window_is_inactivated = TRUE;
					dim_window(win_data, 2);
					win_data->transparent_window_inactive = dialog_data->original_transparent_window;
					win_data->window_opacity_inactive = dialog_data->original_window_opacity;
					win_data->dim_window = dialog_data->original_dim_window;
					break;
#endif
				// style 11: change the text color of window title
				case CHANGE_THE_TEXT_COLOR_OF_WINDOW_TITLE:
				// style 12: change the text color of cmdline
				case CHANGE_THE_TEXT_COLOR_OF_CMDLINE:
				// style 13: change the text color of current dir
				case CHANGE_THE_TEXT_COLOR_OF_CURRENT_DIR:
				// style 14: change the text color of custom page name
				case CHANGE_THE_TEXT_COLOR_OF_CUSTOM_PAGE_NAME:
				// style 15: change the text color of root privileges cmdline
				case CHANGE_THE_TEXT_COLOR_OF_ROOT_PRIVILEGES_CMDLINE:
				// style 16: change the text color of normal text
				case CHANGE_THE_TEXT_COLOR_OF_NORMAL_TEXT:
					recover_page_colors(dialog_data->window, page_data->window, page_data->notebook);
					break;
				case CHANGE_THE_ANSI_COLORS:
				case ADJUST_THE_BRIGHTNESS_OF_ANSI_COLORS_USED_IN_TERMINAL:
				case ADJUST_THE_BRIGHTNESS_OF_ANSI_COLORS_WHEN_INACTIVE:
					if (style==CHANGE_THE_ANSI_COLORS)
						win_data->invert_color = dialog_data->original_invert_color;

					// g_debug("dialog_data->original_color_brightness = %0.3f", dialog_data->original_color_brightness);
					win_data->color_brightness = dialog_data->original_color_brightness;
					win_data->color_brightness_inactive = dialog_data->original_color_brightness_inactive;
					win_data->dim_text = dialog_data->original_dim_text;
					// g_debug("win_data->color_brightness = %0.3f, win_data->color_brightness_inactive = %0.3f",
					//	win_data->color_brightness, win_data->color_brightness_inactive);

					// recover the color of current vte
					if (win_data->use_custom_theme)
						set_new_ansi_color(win_data->current_vte, dialog_data->ansi_colors,
								   win_data->custom_color_theme[win_data->color_theme_index].color,
								   win_data->color_brightness, win_data->invert_color,
								   use_default_vte_theme(win_data), win_data->custom_cursor_color,
								   win_data->cursor_color, FALSE);
					else
						set_new_ansi_color(win_data->current_vte, dialog_data->ansi_colors,
								   system_color_theme[win_data->color_theme_index].color,
								   win_data->color_brightness, win_data->invert_color,
								   use_default_vte_theme(win_data), win_data->custom_cursor_color,
								   win_data->cursor_color, FALSE);
					break;
			}
			break;
		}
	}
#ifdef UNIT_TEST
	}
#endif
#ifdef SAFEMODE
DESTROY_WINDOW:
#endif
	if (selectable)
	{
		extern GtkClipboard *selection_primary;
		gchar *current_clipboard_str = NULL;
#ifdef SAFEMODE
		if (selection_primary)
#endif
			current_clipboard_str = gtk_clipboard_wait_for_text(selection_primary);
		if ((current_clipboard_str!=NULL) && (current_clipboard_str[0]!='\0'))
		{
			g_free(selection_primary_str);
			selection_primary_str = g_strdup(current_clipboard_str);
		}
	}

	// destroy dialog.
	// g_debug("destroy dialog: dialog_data->window = %p", dialog_data->window);
	if ((dialog_response != GTK_RESPONSE_NONE) && (dialog_data->window))
		gtk_widget_destroy(dialog_data->window);

	if ((style >= CHANGE_THE_TEXT_COLOR_OF_WINDOW_TITLE) && (style <= CHANGE_THE_TEXT_COLOR_OF_NORMAL_TEXT))
	{
		// g_debug("tabs_bar_is_shown = %d", tabs_bar_is_shown);
		if (tabs_bar_is_shown != gtk_notebook_get_show_tabs GTK_NOTEBOOK(win_data->notebook))
			hide_and_show_tabs_bar(win_data, OFF);
	}

	if (style==PASTE_GRABBED_KEY_TO_EVERY_VTE_TERMINAL)
	// for PASTE_GRABBED_KEY_TO_EVERY_VTE_TERMINAL Only...
	win_data->enable_key_binding = enable_key_binding;

	if (selectable)
	{
		if (selection_primary_str)
			gtk_clipboard_set_text(selection_primary, selection_primary_str, -1);
	}

FINISH:
	dialog_activated--;
	// g_debug("Set dialog_activated = %d, gtk_widget_get_mapped(win_data->window) = %d", dialog_activated, gtk_widget_get_mapped(win_data->window));
	g_free(dialog_data);
	g_free(selection_primary_str);
	for (i=0; i<TEMPSTR; i++) g_free(temp_str[i]);

	if (window_list == NULL && (dialog_activated==0))
	{
		quit_gtk();
#ifdef DETAIL
		g_debug("+ lilyterm had been close normally!");
#endif
	}

	return dialog_response;
}


void init_dialog_ansi_colors_from_win_data(struct Window *win_data, struct Dialog *dialog_data)
{
#ifdef DETAIL
	g_debug("! Launch init_dialog_ansi_colors_from_win_data() with win_data = %p, dialog_data = %p", win_data, dialog_data);
#endif
#ifdef SAFEMODE
	if ((win_data==NULL) || (dialog_data==NULL)) return;
#endif
	gint i;
	for (i=COLOR-1; i>=0; i--)
	{
		if (win_data->use_custom_theme)
			dialog_data->ansi_colors_orig[i] = win_data->custom_color_theme[win_data->color_theme_index].color[i];
		else
			dialog_data->ansi_colors_orig[i] = system_color_theme[win_data->color_theme_index].color[i];
	}
	// print_color(i, "dialog_data->ansi_colors", dialog_data->ansi_colors_orig[i]);
}

#if defined(USE_OLD_GTK_COLOR_SELECTION) || defined(UNIT_TEST)
void update_custom_cursor_color(GtkWidget *menuitem, struct Window *win_data)
{
#ifdef DETAIL
	g_debug("! Launch update_custom_cursor_color() with menuitem = %p, win_data = %p", menuitem, win_data);
#endif
#ifdef SAFEMODE
	if ((menuitem==NULL) || (win_data==NULL)) return;
#endif

	win_data->custom_cursor_color = ! gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(menuitem));
	enable_custom_cursor_color(win_data->current_vte, win_data->custom_cursor_color, &(win_data->cursor_color));
}
#endif

void dialog_invert_color_theme(GtkWidget *menuitem, struct Window *win_data)
{
#ifdef DETAIL
	g_debug("! Launch dialog_invert_color_theme() with menuitem = %p, win_data = %p", menuitem, win_data);
#endif
#ifdef SAFEMODE
	if ((menuitem==NULL) || (win_data==NULL)) return;
#endif
	struct Dialog *dialog_data = (struct Dialog *)g_object_get_data(G_OBJECT(real_menu_active_window), "Dialog");
#ifdef SAFEMODE
	if (dialog_data==NULL) return;
#endif
	// g_debug("dialog_invert_color_theme(): (before) win_data->temp_index = %d, win_data->invert_color = %d",
	//	win_data->temp_index, win_data->invert_color);

	// win_data->temp_index will point to the color index in color_orig

	// invert the color of current vte
	win_data->invert_color = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(menuitem));
	win_data->temp_index = get_color_index(TRUE, win_data->temp_index);
	// g_debug("dialog_invert_color_theme(): (after) win_data->temp_index = %d, win_data->invert_color = %d",
	//	win_data->temp_index, win_data->invert_color);
	win_data->color_brightness = - win_data->color_brightness;
	win_data->color_brightness_inactive = -win_data->color_brightness_inactive;
	// g_debug("dialog_invert_color_theme(): win_data->color_brightness = %0.3f, win_data->color_brightness_inactive = %0.3f",
	//	win_data->color_brightness, win_data->color_brightness_inactive);

	set_new_ansi_color(win_data->current_vte, dialog_data->ansi_colors, dialog_data->ansi_colors_orig, win_data->color_brightness,
			   win_data->invert_color, FALSE, win_data->custom_cursor_color, win_data->cursor_color, FALSE);

	// invert the brightness
	gtk_range_set_value(GTK_RANGE(dialog_data->operate[1]), win_data->color_brightness);

	// update the color selection info
	// g_debug("dialog_invert_color_theme(): call update_ansi_color_info() with win_data->temp_index to %d", win_data->temp_index);
	gint converted_index = get_color_index(win_data->invert_color, win_data->temp_index);
	update_ansi_color_info(NULL, converted_index);

	// update the color buttons
	update_color_buttons(win_data, dialog_data);

	// set focus color button
	gtk_window_set_focus(GTK_WINDOW(dialog_data->window), dialog_data->color_button[converted_index]);
}

void update_color_buttons(struct Window *win_data, struct Dialog *dialog_data)
{
#ifdef DETAIL
	g_debug("! Launch update_color_buttons() with win_data = %p, dialog_data = %p", win_data, dialog_data);
#endif
#ifdef SAFEMODE
	if ((win_data==NULL) || (dialog_data==NULL)) return;
#endif
#ifdef USE_OLD_GTK_COLOR_SELECTION
	// Get the default size of GTK_ICON_SIZE_MENU
	gint width = 16, height = 16;
	GtkSettings *settings = gtk_settings_get_default();
	if (settings) gtk_icon_size_lookup_for_settings (settings, GTK_ICON_SIZE_MENU, &width, &height);
#endif
	// Get the color theme
	GdkRGBA* temp_color;
	if (win_data->use_custom_theme)
		temp_color = win_data->custom_color_theme[win_data->color_theme_index].color;
	else
		temp_color = system_color_theme[win_data->color_theme_index].color;

	gint i, color_index;
	for (i=COLOR-1; i>=0; i--)
	{
		color_index = get_color_index(win_data->invert_color, i);
#ifdef USE_OLD_GTK_COLOR_SELECTION
		GdkPixbuf *pixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB, FALSE, 8, width, height);
#  ifdef USE_GDK_RGBA
		gdk_pixbuf_fill (pixbuf,
				 (guint32)(temp_color[color_index].red   *0xFF) << 24 |
				 (guint32)(temp_color[color_index].green *0xFF) << 16 |
				 (guint32)(temp_color[color_index].blue  *0xFF) << 8);
#  else
		gdk_pixbuf_fill (pixbuf,
				 (guint32) (temp_color[color_index].red   >> 8) << 24 |
					   (temp_color[color_index].green >> 8) << 16 |
					   (temp_color[color_index].blue  >> 8) << 8);
#  endif

		GtkWidget *image = gtk_image_new_from_pixbuf (pixbuf);
#  ifdef SAFEMODE
		if (dialog_data->color_button[i])
#  endif
			gtk_button_set_image(GTK_BUTTON(dialog_data->color_button[i]), image);
#else
#  ifdef SAFEMODE
		if (dialog_data->color_button[i])
#  endif
#  ifdef USE_GTK_COLOR_CHOOSER
			gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(dialog_data->color_button[i]), &(temp_color[color_index]));
#  else
			gtk_color_button_set_rgba(GTK_COLOR_BUTTON(dialog_data->color_button[i]), &(temp_color[color_index]));
#  endif
#endif
#ifdef ENABLE_GDKCOLOR_TO_STRING
#  ifdef DEBUG
		gchar *color_string = dirty_gdk_rgba_to_string(&(temp_color[color_index]));
		gchar *temp_str =  g_strdup_printf("%s [%d] - %s", color[i].translation, color_index, color_string);
#    ifdef ENABLE_SET_TOOLTIP_TEXT
#      ifdef SAFEMODE
		if (dialog_data->color_button[i])
#      endif
			gtk_widget_set_tooltip_text(dialog_data->color_button[i], temp_str);
#    endif
		g_free(temp_str);
		g_free(color_string);
#  else
#    ifdef ENABLE_SET_TOOLTIP_TEXT
#      ifdef SAFEMODE
		if (dialog_data->color_button[i])
#      endif
			gtk_widget_set_tooltip_text(dialog_data->color_button[i], color[i].translation);
#    endif
#  endif
#endif
#ifdef USE_OLD_GTK_COLOR_SELECTION
		g_object_unref (pixbuf);
#endif
	}
}

gint get_color_index(gboolean invert_color, gint index)
{
#ifdef DETAIL
	g_debug("! Launch get_color_index() with invert_color = %d, index = %d", invert_color, index);
#endif
	if ((index < 0) || (invert_color == FALSE)) return index;

	switch (index)
	{
		case 0:
			return COLOR-1;
		case (COLOR/2)-1:
			return COLOR/2;
		case (COLOR/2):
			return COLOR/2-1;
		case COLOR-1:
			return 0;
			break;
		default:
			return (index + 8) % 16;
	}
}

void clear_custom_colors_data(struct Window *win_data, gboolean update_fg)
{
#ifdef DETAIL
	g_debug("! Launch clear_custom_colors_data() with win_data = %p, update_fg = %d", win_data, update_fg);
#endif
#ifdef SAFEMODE
	if (win_data==NULL) return;
#endif
	gint i, j;

	for (i=0; i<THEME; i++)
		for (j=0; j<COLOR; j++)
			win_data->custom_color_theme[i].color[j] = system_color_theme[i].color[j];
}

void update_fg_bg_color(struct Window *win_data, GdkRGBA color, gboolean update_fg)
{
#ifdef DETAIL
	g_debug("! Launch update_fg_bg_color() with win_data = %p, update_fg = %d", win_data, update_fg);
#endif
#ifdef SAFEMODE
	if (win_data==NULL) return;
#endif
	gint color_index = 0;
	if (update_fg)
	{
		win_data->color[COLOR-1] = color;
		if (win_data->color_brightness > -1)
			adjust_ansi_color(&(win_data->color_inactive[COLOR-1]),
					  &(win_data->color[COLOR-1]),
					  win_data->color_brightness_inactive / (1 + win_data->color_brightness));
		color_index = COLOR-1;
	}
	else
	{
		win_data->color[0] = color;
		// FIXME: GtkColorSelection have no ALPHA-CHANGED signal.
		//	  so that the following code should be
		//	  marked for temporary.
		// background_saturation =
		//	gtk_color_selection_get_current_alpha(
		//		GTK_COLOR_CHOOSER(adjustment))/65535;
	}

	gint i, index = get_color_index(win_data->invert_color, color_index);
	// g_debug("update_fg_bg_color(): index = %d,  win_data->invert_color = %d", index, win_data->invert_color);
	// print_color(-1, "SAVE: color", color);
	for (i=0; i<THEME; i++)
	{
		win_data->custom_color_theme[i].color[index] = color;

		// gchar *temp_str = g_strdup_printf("%d (%02d): fg_color", i, index);
		// print_color (temp_str, win_data->custom_color_theme[i].color[index]);
		// g_free(temp_str);
		// temp_str = g_strdup_printf("%d (%02d): bg_color", i, get_color_index(win_data->invert_color, index));
		// print_color (temp_str, win_data->custom_color_theme[i].color[get_color_index(win_data->invert_color, index)]);
		// g_free(temp_str);
	}

	win_data->have_custom_color = TRUE;
	if (win_data->use_custom_theme == FALSE)
	{
		win_data->use_custom_theme = TRUE;
		recreate_theme_menu_items(win_data);
	}
}
// the win_data->temp_index will updated to get_color_index()!!
// the color_index here is the serial no of button!!
void update_ansi_color_info(GtkWidget *button, gint color_index)
{
#ifdef DETAIL
	g_debug("! Launch update_ansi_color_info() with button = %p, color_index = %d", button, color_index);
#endif
#ifdef SAFEMODE
	if (real_menu_active_window==NULL) return;
#endif
	struct Window *win_data = (struct Window *)g_object_get_data(G_OBJECT(real_menu_active_window), "Win_Data");
#ifdef SAFEMODE
	if ((win_data==NULL) || (win_data->current_vte==NULL)) return;
#endif
	struct Dialog *dialog_data = (struct Dialog *)g_object_get_data(G_OBJECT(real_menu_active_window), "Dialog");
#ifdef SAFEMODE
	if (dialog_data==NULL) return;
#endif
	gint convert_index = get_color_index(win_data->invert_color, color_index);

	gchar *tmp_topic = g_strdup_printf(_("Change the ansi color [%s] in terminal"), color[convert_index].translation);
	gtk_window_set_title(GTK_WINDOW(dialog_data->window), tmp_topic);

	// avoid to call adjust_vte_color()
	// g_debug("update_ansi_color_info(): set win_data->temp_index to -1");
	win_data->temp_index = -1;
#ifdef USE_OLD_GTK_COLOR_SELECTION
#  ifdef SAFEMODE
	if (dialog_data->operate[0]==NULL) return;
#  endif
	set_color_selection_colors(dialog_data->operate[0], &(dialog_data->ansi_colors_orig[convert_index]));
#else
#  ifdef USE_GTK_COLOR_CHOOSER
	gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(dialog_data->color_button[color_index]), &(dialog_data->ansi_colors_orig[convert_index]));
#  else
	gtk_color_button_set_rgba(GTK_COLOR_BUTTON(dialog_data->color_button[color_index]), &(dialog_data->ansi_colors_orig[convert_index]));
#  endif
#endif
	g_free(tmp_topic);
	// win_data->temp_index will point to the color index in color_orig
	win_data->temp_index = convert_index;
	// g_debug("Launch update_ansi_color_info(): win_data->temp_index = %d", win_data->temp_index);
}

GtkWidget *create_label_with_text(GtkWidget *box, gboolean set_markup, gboolean selectable, gint max_width_chars, const gchar *text)
{
#ifdef DETAIL
	g_debug("! Launch create_label_with_text() with box = %p, set_markup = %d, "
		"selectable = %d, max_width_chars = %d, text = %s",
		box, set_markup, selectable, max_width_chars, text);
#endif
#ifdef SAFEMODE
	if (text==NULL) return NULL;
#endif
	GtkWidget *label = gtk_label_new(NULL);
	if (set_markup)
		gtk_label_set_markup(GTK_LABEL (label), text);
	else
		gtk_label_set_text(GTK_LABEL (label), text);
	if (selectable) gtk_label_set_selectable(GTK_LABEL(label), TRUE);
	if (max_width_chars) gtk_label_set_max_width_chars (GTK_LABEL(label), max_width_chars);

	if (box) gtk_box_pack_start (GTK_BOX(box), label, FALSE, FALSE, 0);
	return label;
}

GtkWidget *add_secondary_button(GtkWidget *dialog, const gchar *text, gint response_id, const gchar *stock_id)
{
#ifdef DETAIL
	g_debug("! Launch add_secondary_button() with dialog = %p, text = %s, response_id = %d, stock_id = %s",
		dialog, text, response_id, stock_id);
#endif
#ifdef SAFEMODE
	if ((text==NULL) || (dialog==NULL)) return NULL;
#endif
	GtkWidget *button = gtk_dialog_add_button (GTK_DIALOG(dialog), text, response_id);
	if (stock_id)
		gtk_button_set_image (GTK_BUTTON(button),
				      gtk_image_new_from_stock(stock_id, GTK_ICON_SIZE_BUTTON));
#ifdef HAVE_GTK_DIALOG_GET_ACTION_AREA
	gtk_button_box_set_child_secondary (GTK_BUTTON_BOX(gtk_dialog_get_action_area(GTK_DIALOG(dialog))), button, TRUE);
#endif
	return button;
}

#if defined(ENABLE_FIND_STRING) || defined(UNIT_TEST)
void refresh_regex_settings(GtkWidget *widget, struct Window *win_data)
{
#ifdef DETAIL
	g_debug("! Launch refresh_regex_settings() with win_data = %p", win_data);
#endif
#ifdef SAFEMODE
	if ((win_data==NULL) || (win_data->window==NULL)) return;
#endif
	struct Dialog *dialog_data = (struct Dialog *)g_object_get_data(G_OBJECT(real_menu_active_window), "Dialog");
#ifdef SAFEMODE
	if (dialog_data==NULL) return;
#endif
	g_free(win_data->find_string);
#ifdef SAFEMODE
	if (dialog_data->operate[0]!=NULL)
#endif
		win_data->find_string = g_strdup(gtk_entry_get_text(GTK_ENTRY(dialog_data->operate[0])));
	// win_data->find_case_sensitive = GTK_TOGGLE_BUTTON(dialog_data->operate[1])->active;
#ifdef SAFEMODE
	if (dialog_data->operate[1]!=NULL)
#endif
		win_data->find_case_sensitive = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dialog_data->operate[1]));
	// win_data->find_use_perl_regular_expressions = GTK_TOGGLE_BUTTON(dialog_data->operate[2])->active;
#ifdef SAFEMODE
	if (dialog_data->operate[2]!=NULL)
#endif
		win_data->find_use_perl_regular_expressions = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dialog_data->operate[2]));

	// g_debug("Got win_data->find_string = %s", win_data->find_string);
	// g_debug("Got win_data->find_case_sensitive = %d", win_data->find_case_sensitive);
	// g_debug("win_data->find_use_perl_regular_expressions = %d", win_data->find_use_perl_regular_expressions);

	refresh_regex(win_data, dialog_data);
}

void refresh_regex(struct Window *win_data, struct Dialog *dialog_data)
{
#ifdef DETAIL
	g_debug("! Launch refresh_regex with win_data = %p, dialog_data = %p", win_data, dialog_data);
#endif
#ifdef SAFEMODE
	if ((win_data==NULL) || (win_data->current_vte==NULL) || (dialog_data == NULL)) return;
#endif
#ifdef ENABLE_FIND_STRING
	gint RegexCompileFlags = G_REGEX_OPTIMIZE;
	if (! win_data->find_case_sensitive) RegexCompileFlags |= G_REGEX_CASELESS;
	if ((! win_data->find_use_perl_regular_expressions) &&
	    (win_data->find_string && (win_data->find_string[0]!='\0')))
	{
		gchar *new_find_string = g_regex_escape_string(win_data->find_string, -1);
		g_free(win_data->find_string);
		win_data->find_string = new_find_string;
	}

	gchar *find_string = win_data->find_string;
	if (find_string==NULL) find_string = "";

	if (find_string[0]!='\0')
	{
		vte_terminal_search_find_next(VTE_TERMINAL(win_data->current_vte));
		GRegex *regex = NULL;
		regex = g_regex_new (find_string, RegexCompileFlags, 0, NULL);
		vte_terminal_search_set_gregex(VTE_TERMINAL(win_data->current_vte), regex);
		if (regex) g_regex_unref (regex);
	}
	else
	{
		// FIXME: It don't work!
		vte_terminal_search_set_gregex(VTE_TERMINAL(win_data->current_vte), NULL);
	}

	gboolean update_bg_color = FALSE;

	if ((find_string[0]=='\0') ||
	    vte_terminal_search_find_previous(VTE_TERMINAL(win_data->current_vte)) ||
	    vte_terminal_search_find_next(VTE_TERMINAL(win_data->current_vte)))
	{
		if (compare_color(&(win_data->find_entry_current_bg_color), &(win_data->find_entry_bg_color)))
		{
			update_bg_color = TRUE;
			win_data->find_entry_current_bg_color = win_data->find_entry_bg_color;
		}
	}
	else
	{
		if (compare_color(&(win_data->find_entry_current_bg_color), &(entry_not_find_bg_color)))
		{
			update_bg_color = TRUE;
			win_data->find_entry_current_bg_color = entry_not_find_bg_color;
		}
	}

#  ifdef SAFEMODE
	if (dialog_data->operate[0]!=NULL)
	{
#  endif
		if (update_bg_color)
		{
#  ifdef USING_OLD_GTK_RC_STYLE_NEW
			GtkRcStyle *rc_style = gtk_rc_style_new();
			rc_style->base[GTK_STATE_NORMAL] = win_data->find_entry_current_bg_color;
			rc_style->color_flags[GTK_STATE_NORMAL] |= GTK_RC_BASE;
			gtk_widget_modify_style (dialog_data->operate[0], rc_style);
			g_object_unref(rc_style);
#  else
			GdkRGBA gdkrgba;
			gchar *color_string = dirty_gdk_rgba_to_string(&(win_data->find_entry_current_bg_color));
			gdk_rgba_parse (&gdkrgba, color_string);
			gtk_widget_override_background_color(dialog_data->operate[0], 0, &gdkrgba);
			g_free(color_string);
#  endif
		}
#  ifdef SAFEMODE
	}
#  endif
#  ifdef SAFEMODE
	if (dialog_data->operate[3]!=NULL)
#  endif
		gtk_widget_hide(dialog_data->operate[3]);
#endif
}

void find_str(GtkWidget *widget, Dialog_Find_Type type)
{
#ifdef DETAIL
	g_debug("! Launch find_str() with type = %d", type);
#endif

#ifdef FATAL
	// g_debug("real_menu_active_window = %p", real_menu_active_window);
	if (real_menu_active_window==NULL)
		return print_active_window_is_null_error_dialog("find_str_in_vte()");
#endif
#ifdef SAFEMODE
	if (real_menu_active_window==NULL) return;
#endif
	struct Window *win_data = (struct Window *)g_object_get_data(G_OBJECT(real_menu_active_window), "Win_Data");
#ifdef SAFEMODE
	if ((win_data==NULL) || (win_data->current_vte==NULL)) return;
#endif
#ifdef ENABLE_FIND_STRING
	vte_terminal_search_set_wrap_around (VTE_TERMINAL(win_data->current_vte), FALSE);
	gboolean response = find_str_in_vte(win_data->current_vte, type);

	vte_terminal_search_set_wrap_around (VTE_TERMINAL(win_data->current_vte), TRUE);
	struct Dialog *dialog_data = (struct Dialog *)g_object_get_data(G_OBJECT(real_menu_active_window), "Dialog");
	if (response)
		gtk_widget_hide(dialog_data->operate[3]);
	else
	{
		switch (type)
		{
			case FIND_PREV:
				set_markup_key_value(FALSE, "darkred",
						     "Find hit top, continuing at bottom!",
						     dialog_data->operate[3]);
				break;
			case FIND_NEXT:
				set_markup_key_value(FALSE, "darkred",
						     "Find hit bottom, continuing at top!",
						     dialog_data->operate[3]);
				break;
#  ifdef FATAL
			default:
				print_switch_out_of_range_error_dialog("find_str", "type", type);
#  endif
		}
		gtk_widget_show(dialog_data->operate[3]);
		find_str_in_vte(win_data->current_vte, type);
	}
#endif
}

gboolean find_str_in_vte(GtkWidget *vte, Dialog_Find_Type type)
{
#ifdef DETAIL
	g_debug("! Launch find_str_in_vte() with type = %d", type);
#endif
#ifdef SAFEMODE
		if (vte==NULL) return FALSE;
#endif
	gboolean response = FALSE;

#ifdef ENABLE_FIND_STRING
	switch (type)
	{
		case FIND_PREV:
			response = vte_terminal_search_find_previous(VTE_TERMINAL(vte));
			break;
		case FIND_NEXT:
			response = vte_terminal_search_find_next(VTE_TERMINAL(vte));
			break;
#  ifdef FATAL
		default:
			print_switch_out_of_range_error_dialog("find_str_in_vte", "type", type);
#  endif
	}
#endif
	return response;
}
#endif


void paste_text_to_vte_terminal(GtkWidget *widget, struct Dialog *dialog_data)
{
#ifdef DETAIL
	g_debug("! Launch paste_text_to_vte_terminal()");
#endif
#ifdef SAFEMODE
	if ((dialog_data==NULL) || (dialog_data->operate[0]==NULL)) return;
	// g_debug("paste_text_to_vte_terminal() Got dialog_data->operate[0] = %p", dialog_data->operate[0]);
#endif
	const gchar *text = gtk_entry_get_text(GTK_ENTRY(dialog_data->operate[0]));
	if ((text == NULL) || (text[0] == '\0')) return;
#ifdef FATAL
	// g_debug("real_menu_active_window = %p", real_menu_active_window);
	if (real_menu_active_window==NULL)
		return print_active_window_is_null_error_dialog("paste_text_to_vte_terminal()");
#endif
#ifdef SAFEMODE
	if (real_menu_active_window==NULL) return;
#endif
	struct Window *win_data = (struct Window *)g_object_get_data(G_OBJECT(real_menu_active_window), "Win_Data");
#ifdef SAFEMODE
	if (win_data==NULL) return;
#endif

	gboolean append_new_line = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dialog_data->operate[1]));
	gchar *past_str = NULL;
	if (append_new_line)
		past_str = g_strdup_printf("%s\n", text);
	else
		past_str = g_strdup(text);

	gint i;
	struct Page *page_data = NULL;
	for (i=0; i<gtk_notebook_get_n_pages(GTK_NOTEBOOK(win_data->notebook)); i++)
	{
		page_data = get_page_data_from_nth_page(win_data, i);
#ifdef SAFEMODE
		if (page_data==NULL) continue;
#endif
		vte_terminal_feed_child(VTE_TERMINAL(page_data->vte), past_str, -1);
	}
	g_free(past_str);
	gtk_entry_set_text (GTK_ENTRY(dialog_data->operate[0]), "");
	gtk_window_set_focus(GTK_WINDOW(win_data->window), dialog_data->operate[0]);
}

GtkWidget *create_entry_widget (GtkWidget *box, gchar *contents, gchar *name, gchar *default_value, gboolean activates_default)
{
#ifdef DETAIL
	g_debug("! Launch create_entry_widget() with box = %p, contents = %s, name = %s, default_value = %s",
		box, contents, name, default_value);
#endif
#ifdef SAFEMODE
	if (box==NULL) return NULL;
#endif
	GtkWidget *mainbox = dirty_gtk_vbox_new(FALSE, 10);

	if (contents && contents[0]!='\0')
	{
		GtkWidget *hbox = dirty_gtk_hbox_new(FALSE, 0);
		GtkWidget *vbox = dirty_gtk_vbox_new(FALSE, 0);
		GtkWidget *label = gtk_label_new(contents);
		gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);
		gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 0);
		gtk_box_pack_start(GTK_BOX(mainbox), hbox, FALSE, FALSE, 0);
	}

	GtkWidget *hbox = dirty_gtk_hbox_new(FALSE, 5);
	GtkWidget *vbox = dirty_gtk_vbox_new(FALSE, 0);
	GtkWidget *label = gtk_label_new(name);
	gtk_box_pack_start (GTK_BOX(hbox), label, FALSE, FALSE, 0);
	GtkWidget *entry = gtk_entry_new ();
	if (default_value) gtk_entry_set_text(GTK_ENTRY(entry), default_value);
	gtk_entry_set_activates_default(GTK_ENTRY(entry), activates_default);
	gtk_box_pack_start(GTK_BOX (hbox), entry, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(mainbox), vbox, FALSE, FALSE, 0);

	gtk_box_pack_start(GTK_BOX(box), mainbox, FALSE, FALSE, 0);

	return entry;
}

GtkWidget *create_frame_widget (struct Dialog *dialog_data, gchar *label,
				GtkWidget *label_widget, GtkWidget *child, guint padding)
{
#ifdef DETAIL
	g_debug("! Launch create_frame_widget() with dialog_data = %p, label = %s, label_widget = %p, "
		"child = %p, padding = %d!", dialog_data, label, label_widget, child, padding);
#endif
#ifdef SAFEMODE
	if (dialog_data==NULL) return NULL;
#endif
	GtkWidget *frame = gtk_frame_new (label);
	if (label_widget) gtk_frame_set_label_widget (GTK_FRAME(frame), label_widget);
#ifdef SAFEMODE
	if (dialog_data->box!=NULL)
#endif
	gtk_box_pack_start (GTK_BOX(dialog_data->box), frame, FALSE, FALSE, padding);
	GtkWidget *vbox = dirty_gtk_vbox_new (FALSE, 15);
	gtk_container_set_border_width (GTK_CONTAINER (vbox), BORDER_SPACE);
	gtk_container_add (GTK_CONTAINER (frame), vbox);
	if (child) gtk_box_pack_start (GTK_BOX(vbox), child, FALSE, FALSE, 0);
	return vbox;
}

GtkWidget *create_button_with_image(gchar *label_text, const gchar *stock_id, gboolean set_tooltip_text,
				    GSourceFunc func, gpointer func_data)
{
#ifdef DETAIL
	g_debug("! Launch create_button_with_image() with label_text = %s, stock_id = %s!",
		label_text, stock_id);
#endif
	GtkWidget *label = NULL;

#ifdef ENABLE_SET_TOOLTIP_TEXT
	if (set_tooltip_text)
	{
		label = gtk_button_new();
		gtk_widget_set_tooltip_text (label, label_text);
	}
	else
#endif
		label = gtk_button_new_with_label(label_text);

	gtk_button_set_image (GTK_BUTTON(label),
			      gtk_image_new_from_stock(stock_id, GTK_ICON_SIZE_MENU));
	gtk_button_set_relief(GTK_BUTTON(label), GTK_RELIEF_NONE);
	gtk_button_set_focus_on_click(GTK_BUTTON(label), FALSE);
#ifdef USE_GTK_WIDGET_SET_HALIGN
	gtk_widget_set_halign (label, GTK_ALIGN_START);
#else
	gtk_button_set_alignment(GTK_BUTTON(label), 0, 0.5);
#endif
#ifdef SAFEMODE
	if (func)
#endif
		g_signal_connect(G_OBJECT(label), "clicked", G_CALLBACK(func), func_data);
	return label;
}

GtkWidget *create_hbox_with_text_and_image(gchar *text, const gchar *stock_id)
{
#ifdef DETAIL
	g_debug("! Launch create_hbox_with_text_and_image() with label_text = %s, stock_id = %s!",
		text, stock_id);
#endif
	GtkWidget *hbox=dirty_gtk_hbox_new(FALSE, 0);
	set_widget_thickness(hbox, 0);

	GtkWidget *label = gtk_label_new(text);
	gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, TRUE, 0);
	GtkWidget *image = gtk_image_new_from_stock(stock_id, GTK_ICON_SIZE_MENU);
	gtk_box_pack_start(GTK_BOX(hbox), image, FALSE, FALSE, 0);
	gtk_widget_show_all(hbox);

	return hbox;
}

void create_color_selection_widget(struct Dialog *dialog_data, GSourceFunc func, gpointer func_data)
{
#ifdef DETAIL
	g_debug("! Launch create_color_selection_widget() with dialog_data = %p!", dialog_data);
#endif
#ifdef SAFEMODE
	if (dialog_data==NULL) return;
#endif

#ifdef USE_GTK_COLOR_CHOOSER
	dialog_data->window = dialog_data->operate[0] = gtk_color_chooser_dialog_new(NULL, GTK_WINDOW(dialog_data->window));
	gtk_color_chooser_set_use_alpha(GTK_COLOR_CHOOSER(dialog_data->operate[0]), FALSE);
	// FIXME: the following codes seems no work?
	g_object_set(dialog_data->operate[0], "show-editor", TRUE, NULL);
	set_color_selection_colors(dialog_data->operate[0], &(dialog_data->original_color));

	// FIXME: The "color-changed" signal don't work in GTK3+ anymore.
#  ifdef SAFEMODE
	// if (func)
#  endif
	//	g_signal_connect_after(dialog_data->operate[0], "color-activated",
	//			       G_CALLBACK(func), func_data);
#endif
#ifdef USE_OLD_GTK_COLOR_SELECTION
	dialog_data->operate[0] = gtk_color_selection_new();
	// save the color data first
	gtk_color_selection_set_has_opacity_control(GTK_COLOR_CHOOSER(dialog_data->operate[0]),
						    FALSE);
	gtk_color_selection_set_has_palette(GTK_COLOR_CHOOSER(dialog_data->operate[0]), FALSE);
	dialog_data->recover = FALSE;

	set_color_selection_colors(dialog_data->operate[0], &(dialog_data->original_color));

#  ifdef SAFEMODE
	if (dialog_data->box!=NULL)
#  endif
	gtk_box_pack_start (GTK_BOX(dialog_data->box), dialog_data->operate[0], TRUE, TRUE, 0);

#  ifdef SAFEMODE
	if (func)
#  endif
		g_signal_connect_after(dialog_data->operate[0], "color-changed",
				       G_CALLBACK(func), func_data);
#endif
}

void set_color_selection_colors(GtkWidget *color_selection, GdkRGBA *color)
{
#ifdef SAFEMODE
	if (color_selection==NULL) return;
#endif
#ifdef USE_GTK_COLOR_CHOOSER
	gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(color_selection), color);
#else
	// set the previous/current color of GTK_COLOR_CHOOSER dialog
	gtk_color_selection_set_previous_rgba(GTK_COLOR_CHOOSER(color_selection), color);
	gtk_color_selection_set_current_rgba(GTK_COLOR_CHOOSER(color_selection), color);
#endif
}

void create_scale_widget(struct Dialog *dialog_data, gdouble min, gdouble max, gdouble step, gdouble value,
			 GSourceFunc func, gpointer func_data)
{
#ifdef DETAIL
	g_debug("! Launch create_scale_widget() with dialog_data = %p, min = %3f, max = %3f, step = %3f, value = %3f!",
		dialog_data, min, max, step, value);
#endif
#ifdef SAFEMODE
	if (dialog_data==NULL) return;
#endif
	GtkWidget *hbox1 = dirty_gtk_hbox_new (FALSE, 0);
#ifdef SAFEMODE
	if (dialog_data->box!=NULL)
#endif
		gtk_box_pack_start (GTK_BOX(dialog_data->box), hbox1, FALSE, FALSE, 0);
#ifdef SAFEMODE
	if ((min==max) || (step==0)) return;

	if (min>max)
	{
		gdouble tmp = max;
		max = min;
		min = tmp;
	}
#endif
	dialog_data->operate[0] = gtk_hscale_new_with_range(min, max, step);
	gtk_widget_set_size_request(dialog_data->operate[0], 210, -1);
	gtk_range_set_value(GTK_RANGE(dialog_data->operate[0]), value);
#ifdef SAFEMODE
	if (func)
#endif
		g_signal_connect_after(dialog_data->operate[0], "change-value", G_CALLBACK(func), func_data);
#ifdef SAFEMODE
	if (dialog_data->box!=NULL)
#endif
		gtk_box_pack_start (GTK_BOX(dialog_data->box), dialog_data->operate[0], TRUE, TRUE, 0);
	GtkWidget *hbox2 = dirty_gtk_hbox_new (FALSE, 0);
#ifdef SAFEMODE
	if (dialog_data->box!=NULL)
#endif
		gtk_box_pack_end (GTK_BOX(dialog_data->box), hbox2, FALSE, FALSE, 0);
}

void create_SIGKILL_and_EXIT_widget(struct Dialog *dialog_data, gboolean create_entry_hbox,
				    gboolean create_force_kill_hbox, gchar *count_str)
{
#ifdef DETAIL
	g_debug("! Launch create_SIGKILL_and_EXIT_widget() with dialog_data = %p, create_entry_hbox = %d, "
		"create_force_kill_hbox = %d, count_str = %s!",
		dialog_data, create_entry_hbox, create_force_kill_hbox, count_str);
#endif
#ifdef SAFEMODE
	if (dialog_data==NULL) return;
#endif
	if (create_entry_hbox)
	{
		gchar *message = g_strdup_printf(_("Try to kill %s with SIGKILL() "
					  "to avoid terminate those programs!\n"
					  "(MAY CAUSE DATA LOSS!!)"), count_str);
		dialog_data->operate[1] = gtk_check_button_new_with_label(message);
		g_free(message);
#ifdef SAFEMODE
		if (dialog_data->box!=NULL)
#endif
			gtk_box_pack_start (GTK_BOX(dialog_data->box), dialog_data->operate[1], FALSE, FALSE, 0);
		set_widget_can_not_get_focus(dialog_data->operate[1]);
	}

	if (create_force_kill_hbox)
	{
		gchar *exit_str = g_strdup_printf(_("Force to close all the tabs and windows, and exit %s!"),
						  PACKAGE);
		dialog_data->operate[0] = gtk_check_button_new_with_label(exit_str);
		g_free(exit_str);
#ifdef SAFEMODE
		if (dialog_data->box!=NULL)
#endif
			gtk_box_pack_start (GTK_BOX(dialog_data->box), dialog_data->operate[0], FALSE, FALSE, 0);
		set_widget_can_not_get_focus(dialog_data->operate[0]);
	}
}

// The returned string should be freed when no longer needed.
gchar *get_colorful_profile(struct Window *win_data)
{
#ifdef DETAIL
	g_debug("! Launch get_colorful_profile() with win_data = %p", win_data);
#endif
#ifdef SAFEMODE
	if (win_data==NULL) return NULL;
#endif

	gchar *profile;
	if (win_data->use_custom_profile)
		profile = g_strdup_printf("%s", win_data->profile);
	else
		profile = g_strdup_printf("%s/%s", profile_dir, USER_PROFILE);

	// g_debug("win_data->runtime_encoding = %s", win_data->runtime_encoding);
	// g_debug("profile = %s", profile);
	gchar *utf8_profile = convert_str_to_utf8(profile, win_data->runtime_encoding);
	g_free(profile);

	// g_debug("utf8_profile = %s", utf8_profile);
	return convert_text_to_html(&utf8_profile, TRUE, "blue", "b", NULL);
}

gboolean grab_key_press (GtkWidget *window, GdkEventKey *event, struct Dialog *dialog_data)
{
#ifdef DETAIL
	if (event) g_debug("! Launch grab_key_press() with key value = %d (%s)", event->keyval, gdk_keyval_name(event->keyval));
#endif
#ifdef SAFEMODE
	if (event==NULL) return FALSE;
#endif
	gchar *key_value = g_strdup("");
	gint i=0;

	// FIXME: Be careful for the setted key value (GDK_Shift_L abd GDK_Hyper_R) here...
	for (i=0; i<MOD; i++)
		if (event->state & modkeys[i].mod)
			key_value = deal_dialog_key_press_join_string(&key_value, "+", modkeys[i].name);
	key_value = deal_dialog_key_press_join_string(&key_value, " ", gdk_keyval_name(event->keyval));
	set_markup_key_value(TRUE, "blue", key_value, dialog_data->operate[0]);

	struct Window *win_data = (struct Window *)g_object_get_data(G_OBJECT(real_menu_active_window), "Win_Data");
#ifdef SAFEMODE
	if (win_data==NULL) return FALSE;
#endif
	struct Page *page_data = NULL;
	for (i=0; i<gtk_notebook_get_n_pages(GTK_NOTEBOOK(win_data->notebook)); i++)
	{
		page_data = get_page_data_from_nth_page(win_data, i);
#ifdef SAFEMODE
		if (page_data==NULL) continue;
#endif
		// g_debug("Send key %s to vte %p!!", key_value, page_data->vte);
		// gtk_test_widget_send_key (page_data->vte, event->keyval, event->state);
		gtk_widget_event(page_data->vte, (GdkEvent *)event);
	}
	g_free(key_value);
	return TRUE;
}

gboolean deal_dialog_key_press(GtkWidget *window, GdkEventKey *event, struct Dialog *dialog_data)
{
#ifdef DETAIL
	if (event) g_debug("! Launch deal_dialog_key_press() with key value = %s", gdk_keyval_name(event->keyval));
#endif
#ifdef SAFEMODE
	if ((dialog_data==NULL) || (event==NULL)) return FALSE;
#endif
	if (dialog_data->current_key_index < 0) return FALSE;

	gchar *key_value = g_strdup("");
	gint i=0;

	// FIXME: Be careful for the setted key value (GDK_Shift_L abd GDK_Hyper_R) here...
	if (((event->state & SHIFT_ONLY_MASK) || (event->keyval > GDK_KEY_asciitilde)) &&
	    ((event->keyval < GDK_KEY_Shift_L) || (event->keyval > GDK_KEY_Hyper_R)))
	{
		for (i=0; i<MOD; i++)
			if (event->state & modkeys[i].mod)
				key_value = deal_dialog_key_press_join_string(&key_value, "+", modkeys[i].name);
	}

	g_free(dialog_data->user_key_value[dialog_data->current_key_index]);
	gboolean key_value_need_free = FALSE;
#ifdef SAFEMODE
	if ((key_value && (key_value[0]!='\0')) ||
	    ((event->keyval >= GDK_KEY_F1) && (event->keyval <= GDK_KEY_F12)) ||
	    (event->keyval == GDK_KEY_Menu) || (event->keyval == GDK_KEY_Super_L) || (event->keyval == GDK_KEY_Super_R))
#else
	if (key_value[0]!='\0' ||
	    ((event->keyval >= GDK_KEY_F1) && (event->keyval <= GDK_KEY_F12)) ||
	    (event->keyval == GDK_KEY_Menu) || (event->keyval == GDK_KEY_Super_L) || (event->keyval == GDK_KEY_Super_R))
#endif
	{
		gint keyval = event->keyval;

		// Trying to show "Ctrl T" instead of "Ctrl t"
		if ((keyval>=GDK_KEY_a) && (keyval<=GDK_KEY_z)) keyval = keyval - GDK_KEY_a + GDK_KEY_A;
		key_value = deal_dialog_key_press_join_string(&key_value, " ", gdk_keyval_name(keyval));
		gboolean duplicated = FALSE;

		// Check if the key is duplicated
		for (i=0; i<KEYS; i++)
		{
			if ((i!=dialog_data->current_key_index) &&
			    (compare_strings(key_value, dialog_data->user_key_value[i], FALSE) == FALSE))
			{
				duplicated = TRUE;
				g_free(key_value);
				dialog_data->user_key_value[dialog_data->current_key_index] = g_strdup("");
				key_value_need_free = TRUE;
				key_value = g_strdup_printf(_("(Disabled, duplicated with [%s])"),
							    system_keys[i].topic);
			}
		}
		if (! duplicated) dialog_data->user_key_value[dialog_data->current_key_index] = key_value;
	}
	else
	{
		dialog_data->user_key_value[dialog_data->current_key_index] = key_value;
		key_value = _("(Disabled)");
	}

	// g_debug("Got function key = %s, and event->keyval = %X (%s)",
	//	key_value, event->keyval, gdk_keyval_name(event->keyval));

	set_markup_key_value(TRUE, "blue", key_value, dialog_data->operate[2]);

	GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(dialog_data->treeview));
	GtkTreeSelection *treeselection = gtk_tree_view_get_selection (GTK_TREE_VIEW(dialog_data->treeview));
	GtkTreeIter iter;
	if (gtk_tree_selection_get_selected (treeselection, &model, &iter))
		gtk_tree_store_set (GTK_TREE_STORE (model), &iter, 1,
				    dialog_data->user_key_value[dialog_data->current_key_index], -1);

	if (key_value_need_free) g_free(key_value);

	return TRUE;
}

void set_markup_key_value(gboolean bold, gchar *color, gchar *key_value, GtkWidget *label)
{
#ifdef DETAIL
	g_debug("! Launch set_markup_key_value() whith color = %s, key_value = %s, label = %p!",
		color, key_value, label);
#endif

#ifdef SAFEMODE
	if (label==NULL) return;
#endif

	gchar *markup_text;
	if (bold)
		markup_text = convert_text_to_html (&key_value, FALSE, color, "b", NULL);
	else
		markup_text = convert_text_to_html (&key_value, FALSE, color, NULL);
	gtk_label_set_markup (GTK_LABEL (label), markup_text);
	g_free(markup_text);
}

// WARNING: The "key_value" will free() in deal_dialog_key_press_join_string()
gchar *deal_dialog_key_press_join_string(StrAddr **key_value, gchar *separator, gchar *append)
{
#ifdef DETAIL
	g_debug("! Launch deal_dialog_key_press_join_string() with key_value = %s, separator = %s, append = %s", *key_value, separator, append);
#endif
#ifdef SAFEMODE
	if ((separator==NULL) || (append==NULL)) return *key_value;
#endif

	gchar *join_string = NULL;
#ifdef SAFEMODE
	if (*key_value && ((*key_value)[0]!='\0'))
#else
	if ((*key_value)[0]!='\0')
#endif
		join_string = g_strdup_printf("%s%s%s",*key_value, separator, append);
	else
		join_string = g_strdup(append);

	g_free(*key_value);
#ifdef SAFEMODE
	*key_value = NULL;
#endif
	return join_string;
}

#if defined(USE_GTK_COLOR_CHOOSER) || defined(UNIT_TEST)
void adjust_vte_color_sample(GtkColorButton* color_button, gint color_index)
{
#  ifdef DETAIL
	g_debug("! Launch adjust_vte_color_sample() with color_button = %p, color_index = %d", color_button, color_index);
#  endif
#  ifdef SAFEMODE
	if (color_button==NULL) return;
#  endif
	struct Window *win_data = (struct Window *)g_object_get_data(G_OBJECT(real_menu_active_window), "Win_Data");
	win_data->temp_index = get_color_index(win_data->invert_color, color_index);

	GdkRGBA color;
	// gtk_color_button_get_rgba(GTK_COLOR_BUTTON(color_button), &color);
	gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(color_button), &color);

	adjust_vte_color(NULL, &color, win_data->current_vte);
}
#endif

// ** README ** : GTK2+: #define adjust_vte_color(x,y,z) adjust_vte_color(x,z)
void adjust_vte_color(GtkColorChooser *color_selection, GdkRGBA *color, GtkWidget *vte)
{
#ifdef DETAIL
	g_debug("! Launch adjust_vte_color() with color_selection = %p, color = %p, vte = %p", color_selection, color, vte);
#endif
#ifdef SAFEMODE
	if (vte==NULL) return;
#endif
	GdkRGBA final_color;
#ifdef USE_OLD_GTK_COLOR_SELECTION
	if (color_selection) gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(color_selection), &final_color);
#else
	if (color==NULL)
		gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(color_selection), &final_color);
	else
		final_color=*color;
#endif
	// g_debug("Changing the color for vte %p", vte);
	struct Dialog *dialog_data = (struct Dialog *)g_object_get_data(G_OBJECT(real_menu_active_window), "Dialog");
	// g_debug("Get dialog_data = %p", dialog_data);
#ifdef SAFEMODE
	if (dialog_data==NULL) return;
#endif

	switch (dialog_data->type)
	{
		case CHANGE_THE_FOREGROUND_COLOR:
		case CHANGE_THE_BACKGROUND_COLOR:
		case CHANGE_THE_ANSI_COLORS:
		{
#ifdef SAFEMODE
			if (vte==NULL) return;
#endif
			struct Page *page_data = (struct Page *)g_object_get_data(G_OBJECT(vte), "Page_Data");
#ifdef SAFEMODE
			if ((page_data==NULL) || (page_data->window==NULL)) return;
#endif
			struct Window *win_data = (struct Window *)g_object_get_data(G_OBJECT(page_data->window), "Win_Data");
#ifdef SAFEMODE
			if (win_data==NULL) return;
#endif
			gint converted_index = get_color_index(win_data->invert_color, win_data->temp_index);
			// g_debug("adjust_vte_color(): win_data->temp_index = %d, converted_index = %d", win_data->temp_index, converted_index);

			if ((dialog_data->type==CHANGE_THE_FOREGROUND_COLOR) ||
			    ((dialog_data->type==CHANGE_THE_ANSI_COLORS) && (converted_index==(COLOR-1))))
			{
				// print_color(-1, "RECONVER 1: final_color", final_color);
				if (dialog_data->recover)
					final_color = dialog_data->original_color;

				// print_color(-1, "RECONVER 2: final_color", final_color);
				vte_terminal_set_color_foreground_rgba(VTE_TERMINAL(vte), &(final_color));
				vte_terminal_set_color_bold_rgba(VTE_TERMINAL(vte), &(final_color));

				if (dialog_data->type==CHANGE_THE_ANSI_COLORS)
				{
					dialog_data->ansi_colors[converted_index] = final_color;
					dialog_data->ansi_colors_orig[win_data->temp_index] = final_color;
				}
			}
			else if ((dialog_data->type==CHANGE_THE_BACKGROUND_COLOR) ||
			    ((dialog_data->type==CHANGE_THE_ANSI_COLORS) && (converted_index==0)))
			{
				if (dialog_data->recover)
				{
					final_color = dialog_data->original_color;
					// FIXME: GtkColorSelection have no ALPHA CHANGED signal.
					//	  so that the following code should be marked for temporary
					//if (use_rgba)
					//{
					//	gint alpha = gtk_color_selection_get_current_alpha(colorselection);
					//	g_debug("Current alpha = %d", alpha);
					//	vte_terminal_set_opacity(VTE_TERMINAL(vte), alpha);
					//	vte_terminal_set_background_saturation( VTE_TERMINAL(vte), 1-alpha/65535);
					//}
				}
				//else if (use_rgba)
				//	set_background_saturation(NULL, 0, background_saturation, vte);

				vte_terminal_set_color_background_rgba(VTE_TERMINAL(vte), &(final_color));
#if defined(ENABLE_VTE_BACKGROUND) || defined(FORCE_ENABLE_VTE_BACKGROUND)
				dirty_vte_terminal_set_background_tint_color(VTE_TERMINAL(vte), final_color);
#endif
				if (dialog_data->type==CHANGE_THE_ANSI_COLORS)
				{
					dialog_data->ansi_colors[converted_index] = final_color;
					dialog_data->ansi_colors_orig[win_data->temp_index] = final_color;
				}
			}
			else
			// dialog_data->type==CHANGE_THE_ANSI_COLORS && (! fg_color) && (! bg_color)
			{
				// win_data->temp_index == -1: update_ansi_color_info() is called.
				if (win_data->temp_index > -1)
				{
					GdkRGBA tmp_color;
#ifdef USE_OLD_GTK_COLOR_SELECTION
					gtk_color_chooser_get_rgba(color_selection, &tmp_color);
#else
					tmp_color = final_color;
#endif
					// g_debug("adjust_vte_color(): update the %d color of colors_orig", win_data->temp_index);
					dialog_data->ansi_colors_orig[win_data->temp_index] = tmp_color;
					adjust_ansi_color(&dialog_data->ansi_colors[get_color_index(win_data->invert_color, win_data->temp_index)],
							  &dialog_data->ansi_colors_orig[win_data->temp_index],
							  win_data->color_brightness);
					set_vte_color(win_data->current_vte, FALSE, win_data->custom_cursor_color,
						      win_data->cursor_color, dialog_data->ansi_colors, FALSE, FALSE);
					// set_new_ansi_color(win_data->current_vte, dialog_data->ansi_colors, dialog_data->ansi_colors_orig,
					//		   win_data->color_brightness, win_data->invert_color, FALSE, win_data->cursor_color);
				}
			}
			break;
		}
		case CHANGE_THE_CURSOR_COLOR:
		{
#ifdef SAFEMODE
			if (vte==NULL) return;
#endif
			struct Page *page_data = (struct Page *)g_object_get_data(G_OBJECT(vte), "Page_Data");
#ifdef SAFEMODE
			if ((page_data==NULL) || (page_data->window==NULL)) return;
#endif
			struct Window *win_data = (struct Window *)g_object_get_data(G_OBJECT(page_data->window), "Win_Data");
#ifdef SAFEMODE
				if (win_data==NULL) return;
#endif
#ifdef USE_OLD_GTK_COLOR_SELECTION
			//   [Cancel] is pressed       [OK] is pressed
			if ((dialog_data->recover) || (color_selection==NULL))
				final_color = win_data->cursor_color;
			else
				win_data->custom_cursor_color = TRUE;
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(dialog_data->custom_cursor_color_checkbox), ! win_data->custom_cursor_color);
#endif
			enable_custom_cursor_color(vte, win_data->custom_cursor_color, &(final_color));
			break;
		}
		case CHANGE_THE_TEXT_COLOR_OF_WINDOW_TITLE:
		case CHANGE_THE_TEXT_COLOR_OF_CMDLINE:
		case CHANGE_THE_TEXT_COLOR_OF_CURRENT_DIR:
		case CHANGE_THE_TEXT_COLOR_OF_CUSTOM_PAGE_NAME:
		case CHANGE_THE_TEXT_COLOR_OF_ROOT_PRIVILEGES_CMDLINE:
		case CHANGE_THE_TEXT_COLOR_OF_NORMAL_TEXT:
		{
			// the change of color will be shown(demo) in 1st page and demo page.
			gchar *current_color = NULL;
#ifdef ENABLE_GDKCOLOR_TO_STRING
			current_color = dirty_gdk_rgba_to_string(&(final_color));
#endif
			struct Page *page_data = (struct Page *)g_object_get_data(G_OBJECT(vte), "Page_Data");
#ifdef SAFEMODE
			if (page_data)
#endif
				update_page_name(page_data->window, vte, NULL, page_data->label_text, 1,
						  _("Bold Demo Text"), current_color, FALSE, TRUE, FALSE, NULL,
						  page_data->custom_window_title, FALSE);

			page_data = (struct Page *)g_object_get_data(G_OBJECT(dialog_data->demo_vte), "Page_Data");
#ifdef SAFEMODE
			if (page_data)
#endif
				update_page_name(page_data->window, dialog_data->demo_vte, NULL,
						 page_data->label_text, page_data->page_no+1,
						 dialog_data->demo_text, current_color, FALSE, FALSE,
						 FALSE, NULL, page_data->custom_window_title, FALSE);
			g_free(current_color);
			break;
		}
		default:
#ifdef FATAL
			print_switch_out_of_range_error_dialog("adjust_vte_color",
							       "dialog_data->type",
							       dialog_data->type);
#endif
			break;
	}
}

void recover_page_colors(GtkWidget *dialog_window, GtkWidget *window, GtkWidget *notebook)
{
#ifdef DETAIL
	g_debug("! Launch recover_page_colors() in window %p", window);
#endif

#ifdef SAFEMODE
	if ((dialog_window==NULL) || (window==NULL)) return;
#endif

	struct Dialog *dialog_data = (struct Dialog *)g_object_get_data(G_OBJECT(real_menu_active_window), "Dialog");
	struct Window *win_data = (struct Window *)g_object_get_data(G_OBJECT(window), "Win_Data");
#ifdef SAFEMODE
	if ((dialog_data==NULL) || (win_data==NULL)) return;
#endif
	gint i;
	struct Page *page_data = NULL;

	// g_debug("page_cmdline_color = %s", win_data->page_cmdline_color);
	// g_debug("page_dir_color = %s", win_data->page_dir_color);
	// g_debug("page_custom_color = %s", win_data->page_custom_color);
	// g_debug("page_root_color = %s", win_data->page_root_color);
	// g_debug("page_normal_color = %s", win_data->page_normal_color);

	// delete the tab that we add when demo
	win_data->kill_color_demo_vte = TRUE;
	for (i=PAGE_COLOR; i>=dialog_data->total_page; i--)
	{
		page_data = get_page_data_from_nth_page(win_data, i);
#ifdef SAFEMODE
		if (page_data)
#endif
			close_page (page_data->vte, CLOSE_WITH_WINDOW_CLOSE_BUTTON);
	}
	win_data->kill_color_demo_vte = FALSE;

	// reset to the current page
#ifdef SAFEMODE
	if (notebook)
#endif
		gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook), dialog_data->current_page_no);

	// recover the title/color of pages
	for (i=0; i<dialog_data->total_page; i++)
	{
		page_data = get_page_data_from_nth_page(win_data, i);

		// restore the is_bold of 1st page
		// g_debug("i = %d", i);
		if (i==0)
			page_data->is_bold = dialog_data->tab_1_is_bold;

		// g_debug("Launch get_and_update_page_name() in recover_page_colors() for %d page!", i);
		// g_debug("page_data->page_update_method = %d", page_data->page_update_method);
		if (i <= PAGE_COLOR) page_data->page_update_method = dialog_data->original_update_method[i];
		if (page_data->page_update_method == PAGE_METHOD_WINDOW_TITLE) page_data->window_title_updated = 1;
		get_and_update_page_name(page_data, FALSE);
	}
}

gboolean check_and_add_locale_to_warned_locale_list(struct Window *win_data, gchar *new_locale)
{
#ifdef DETAIL
	g_debug("! Launch check_and_add_locale_to_warned_locale_list() with win_data = %p, new_locale = %s!",
		win_data, new_locale);
#endif
#ifdef SAFEMODE
	if ((win_data==NULL) || (new_locale==NULL) ||
	    (win_data->warned_locale_list == NULL) ||
	    (win_data->warned_locale_list->str == NULL)) return TRUE;
#endif
	gchar *check_locale = g_strdup_printf(" %s ", new_locale);
#ifdef SAFEMODE
	if (check_locale==NULL) return TRUE;
#endif
	gboolean response = TRUE;

	// g_debug("check_and_add_locale_to_warned_locale_list(): win_data->warned_locale_list = %p",
	//	win_data->warned_locale_list);
	// g_debug("check_and_add_locale_to_warned_locale_list(): Checking '%s' in '%s'...",
	//	check_locale, win_data->warned_locale_list->str);
	if (strstr(win_data->warned_locale_list->str, check_locale))
		response = FALSE;
	else
		g_string_append_printf(win_data->warned_locale_list, "%s ", new_locale);
	g_free(check_locale);
	return response;
}

void create_invalid_locale_error_message(gchar *locale)
{
#ifdef DETAIL
	g_debug("! Launch create_invalid_locale_error_message() with locale = %s!", locale);
#endif
	gchar *color_locale = convert_text_to_html(&locale, FALSE, "darkblue", "b", NULL);
	gchar *err_msg = g_strdup_printf(_("The locale \"%s\" seems NOT supported by your system!\n"
					   "If you just added the locale datas recently,\n"
					   "Please close all the windows of %s and try again."),
					   color_locale, PACKAGE);
	error_dialog(NULL, _("Not supported locale!"), "Not supported locale!",
		     GTK_FAKE_STOCK_DIALOG_WARNING, err_msg, NULL);
	g_free(color_locale);
	g_free(err_msg);
}

// 1. real_menu_active_window = NULL
// 2. Not supported feature
// 3. The format of socket data is out of date
// 4. The following settings can NOT be applied
// 5. Not supported locale!
// 6. Error when creating child process
// 7. Error when writing profile
// 8. The format of your profile is out of date
// 9. The profile is invalid

void error_dialog(GtkWidget *window, gchar *title_translation, gchar *title,
		  gchar *icon, gchar *message, gchar *encoding)
{
#ifdef DETAIL
	g_debug("! Launch error_dialog() with window = %p, title = %s, icon = %s, message = %s, encodig = %s",
		window, title, icon, message, encoding);
#endif

	dialog_activated++;
	// g_debug("Set dialog_activated = %d", dialog_activated);

	gchar *utf8_message = convert_str_to_utf8(message, encoding);
#ifdef UNIT_TEST
	g_message("%s", utf8_message);
#else
	struct Dialog *dialog_data = g_new0(struct Dialog, 1);

	// void create_dialog(gchar *dialog_title, Dialog_Button_Type type, GtkWidget *window, gboolean center,
	//		      gboolean resizable, gint border_width, gint response, gchar *icon,
	//		      gchar *title, gboolean selectable, gint max_width_chars,
	//		      gboolean state_bottom, gint create_entry_hbox, struct Dialog *dialog_data)
	create_dialog(title_translation, title, DIALOG_OK, window, TRUE,
		      FALSE, 10, GTK_RESPONSE_OK, icon, utf8_message,
		      FALSE, 0, TRUE, BOX_NONE, 0, dialog_data);
	gtk_widget_show_all (dialog_data->window);
	gtk_dialog_run(GTK_DIALOG(dialog_data->window));
	gtk_widget_destroy(dialog_data->window);
	g_free(dialog_data);
#endif
	g_free(utf8_message);
	dialog_activated--;
	// g_debug("Set dialog_activated = %d", dialog_activated);
}

#ifdef FATAL
void print_switch_out_of_range_error_dialog(gchar *function, gchar *var, gint value)
{
	gchar *err_msg = g_strdup_printf("%s(): the var \"%s\" (%d) is out of range\n\n"
					 "Please report bug to %s, Thanks!",
					 function, var, value, BUGREPORT);
#ifdef SAFEMODE
	if (err_msg)
#endif
		error_dialog(NULL, _("The following error occurred:"),
			     "The following error occurred:",
			     GTK_FAKE_STOCK_DIALOG_ERROR, err_msg, NULL);
	g_free(err_msg);
}
#endif

gboolean upgrade_dialog(gchar *version_str)
{
#ifdef DETAIL
	g_debug("! Launch upgrade_dialog() with version_str = %s", version_str);
#endif
	gchar *err_msg = g_strdup_printf(_("You should upgrade to %s and recompile %s to support this feature."), version_str, PACKAGE);
	error_dialog(NULL, _("Not supported feature!"), "Not supported feature!",
		     GTK_FAKE_STOCK_DIALOG_WARNING, err_msg, NULL);
	g_free(err_msg);
	return FALSE;
}

gboolean set_ansi_color(GtkRange *range, GtkScrollType scroll, gdouble value, GtkWidget *vte)
{
#ifdef DETAIL
	g_debug("! Launch set_ansi_color() with range = %p, scroll = %u, value = %f, vte = %p",
		range, scroll, value, vte);
#endif

#ifdef SAFEMODE
	if (vte==NULL) return FALSE;
#endif
	struct Page *page_data = (struct Page *)g_object_get_data(G_OBJECT(vte), "Page_Data");
#ifdef SAFEMODE
	if ((page_data==NULL) || (page_data->window==NULL)) return FALSE;
#endif
	struct Window *win_data = (struct Window *)g_object_get_data(G_OBJECT(page_data->window),
								     "Win_Data");
#ifdef SAFEMODE
	if (win_data==NULL) return FALSE;
#endif

	value = CLAMP(value, -1, 1);
	win_data->color_brightness = value;
	// g_debug("set_ansi_color(): win_data->color_brightness = %0.3f, win_data->color_brightness_inactive = %0.3f",
	//	win_data->color_brightness, win_data->color_brightness_inactive);

	struct Dialog *dialog_data = (struct Dialog *)g_object_get_data(G_OBJECT(real_menu_active_window), "Dialog");
#ifdef SAFEMODE
	if (dialog_data==NULL) return FALSE;
#endif
	// We will not adject the value of fg_color
	// win_data->fg_color = get_inactive_color(dialog_data->original_fg_color,
	//					win_data->color_brightness,
	//					dialog_data->original_color_brightness);
	gboolean dim_fg_color = (dialog_data->type==ADJUST_THE_BRIGHTNESS_OF_ANSI_COLORS_WHEN_INACTIVE) ? TRUE : FALSE;
	set_new_ansi_color(win_data->current_vte, dialog_data->ansi_colors, dialog_data->ansi_colors_orig, win_data->color_brightness,
			   win_data->invert_color, FALSE, win_data->custom_cursor_color, win_data->cursor_color, dim_fg_color);
	return FALSE;
}


GdkRGBA get_inactive_color(GdkRGBA original_fg_color, gdouble new_brightness, gdouble old_brightness)
{
#ifdef DETAIL
	g_debug("! Launch get_inactive_color() with new_brightness = %3f, "
		"old_brightness = %3f!", new_brightness, old_brightness);
#endif
	GdkRGBA inactive_color;
	if (new_brightness < old_brightness)
		adjust_ansi_color(&inactive_color,
					    &(original_fg_color),
					    (new_brightness - old_brightness) /
						(1 + old_brightness));
	else
		adjust_ansi_color(&inactive_color,
					    &(original_fg_color),
					    (new_brightness - old_brightness) /
						(1 - old_brightness));
	return inactive_color;
}

void set_new_ansi_color(GtkWidget *vte, GdkRGBA color[COLOR], GdkRGBA color_orig[COLOR], gdouble color_brightness, gboolean invert_color,
			gboolean default_vte_color, gboolean custom_cursor_color, GdkRGBA cursor_color, gboolean dim_fg_color)
{
#ifdef DETAIL
	g_debug("! Launch set_new_ansi_color() with vte = %p, color_brightness = %3f, invert_color = %d, default_vte_color = %d",
		vte, color_brightness, invert_color, default_vte_color);
#endif
#ifdef SAFEMODE
	if ((vte==NULL) || (color_orig==NULL) || (color==NULL)) return;
#endif
	create_theme_color_data(color, color_orig, color_brightness, invert_color, default_vte_color, dim_fg_color);
	set_vte_color(vte, default_vte_color, custom_cursor_color, cursor_color, color, FALSE, FALSE);
}

void hide_combo_box_capital(GtkCellLayout *cell_layout, GtkCellRenderer *cell,
			    GtkTreeModel *tree_model, GtkTreeIter *iter, gpointer data)
{
#ifdef DETAIL
	g_debug("! Launch hide_combo_box_capital() with cell_layout = %p, cell = %p, tree_model = %p, iter = %p!",
		cell_layout, cell, tree_model, iter);
#endif
#ifdef SAFEMODE
	if ((tree_model==NULL) || (iter==NULL)) return;
#endif
	g_object_set (cell, "sensitive", !gtk_tree_model_iter_has_child(tree_model, iter), NULL);
}

void update_key_info (GtkTreeSelection *treeselection, struct Dialog *dialog_data)
{
#ifdef DETAIL
	g_debug("! Launch update_key_info() with treeselection = %p, dialog_data = %p!", treeselection, dialog_data);
#endif
#ifdef SAFEMODE
	if ((dialog_data==NULL) || (treeselection==NULL)) return;
#endif
	GtkTreeIter iter;
	GtkTreeModel *model = gtk_tree_view_get_model(gtk_tree_selection_get_tree_view(GTK_TREE_SELECTION (treeselection)));
	GtkTreePath *path = NULL;
	gchar *string = NULL;
	gchar **indices = NULL;
	if (gtk_tree_selection_get_selected (treeselection, &model, &iter))
	{
		gtk_widget_set_no_show_all (dialog_data->operate[3], FALSE);
		gtk_widget_show_all (dialog_data->operate[3]);

		path = gtk_tree_model_get_path(model, &iter);
		string = gtk_tree_path_to_string (path);
		// g_debug ("update_key_info(): got string = %s", string);
		indices = split_string(string, ":", -1);
#ifdef SAFEMODE
		if (indices)
		{
#endif
			if (indices[0]==NULL) goto FINISH;
			if (indices[1]==NULL)
			{
				dialog_data->current_key_index = -1;
				set_markup_key_value(TRUE, "dark green", "", dialog_data->operate[1]);
			set_markup_key_value(TRUE, "blue", "", dialog_data->operate[2]);
		}
		else
		{
			dialog_data->current_key_index = dialog_data->KeyTree[atoi(indices[0])][atoi(indices[1])];
			// g_debug("KeyTree[%d][%d] = %d",
			//	atoi(indices[0]), atoi(indices[1]), dialog_data->current_key_index);
			set_markup_key_value(TRUE, "dark green",
					     system_keys[dialog_data->current_key_index].translation,
					     dialog_data->operate[1]);
			// g_debug("dialog_data->current_key_index = %d, "
			//	   "dialog_data->user_key_value[dialog_data->current_key_index] = %s",
			//	   dialog_data->current_key_index,
			//	   dialog_data->user_key_value[dialog_data->current_key_index]);
			gchar *user_key_value = dialog_data->user_key_value[dialog_data->current_key_index];
			if ((user_key_value==NULL) || (user_key_value[0]=='\0'))
				user_key_value = _("(Disabled)");
			set_markup_key_value(TRUE, "blue", user_key_value, dialog_data->operate[2]);
		}

		gchar *group_name = key_groups[atoi(indices[0])];
		set_markup_key_value(TRUE, NULL,
				     group_name,
				     dialog_data->operate[0]);
		gchar *disable_text = g_strdup_printf(_("Disable all the function keys of [%s] group."),
						      group_name);
		gtk_button_set_label(GTK_BUTTON(dialog_data->operate[3]), disable_text);
		g_free(disable_text);
#ifdef SAFEMODE
		}
#endif
	}

FINISH:
	if (path) gtk_tree_path_free(path);
	if (string) g_free(string);
	if (indices) g_strfreev(indices);
	return;
}

void clear_key_groups(struct Dialog *dialog_data, gboolean clear_all)
{
#ifdef DETAIL
	g_debug("! Launch clear_key_groups() with dialog_data = %p, clear_all = %d!", dialog_data, clear_all);
#endif
#ifdef SAFEMODE
	if ((dialog_data==NULL) || (dialog_data->treeview==NULL))return;
#endif
	GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(dialog_data->treeview));
	GtkTreeSelection *treeselection = gtk_tree_view_get_selection (GTK_TREE_VIEW(dialog_data->treeview));
	GtkTreeIter iter_child;
	gint current_key_group = -1;

	if (dialog_data->current_key_index > -1) current_key_group = system_keys[dialog_data->current_key_index].group;

	gboolean iter_is_child = FALSE;
	GtkTreeIter iter, iter_parent;

	if (gtk_tree_selection_get_selected (treeselection, &model, &iter_child))
		// theck if the selected item is "group" or "key"
		iter_is_child = gtk_tree_model_iter_parent (model, &iter_parent, &iter_child);

	else
		// if nothing is selected, return!
		if (clear_all == FALSE) return;

	if (iter_is_child)
		set_markup_key_value(TRUE, "blue", _("(Disabled)"), dialog_data->operate[2]);
	else
		set_markup_key_value(TRUE, "blue", "", dialog_data->operate[2]);


	if (clear_all)
		// call clean_model_foreach for each entry
		gtk_tree_model_foreach(model, clean_model_foreach, NULL);
	else
	{
		// if the selected item is "group"...
		if (! iter_is_child)
		{
			iter_parent = iter_child;

			// Trying to find out 'dialog_data->current_key_index'
			GtkTreePath *path = gtk_tree_model_get_path(model, &iter_child);
			gchar *string = gtk_tree_path_to_string (path);
			// g_debug ("update_key_info(): got string = %s", string);
			current_key_group = atoi(string);
			gtk_tree_path_free(path);
			g_free(string);
		}

		gint i, child = gtk_tree_model_iter_n_children (model, &iter_parent);
		// clear entries of the group
		for (i=0; i< child; i++)
		{
			if (gtk_tree_model_iter_nth_child (model, &iter, &iter_parent, i))
				gtk_tree_store_set (GTK_TREE_STORE (model), &iter, 1, "", -1);
#ifdef SAFEMODE
			else
				break;
#endif
		}
	}

#ifdef SAFEMODE
	// it should be happen...
	if ((clear_all== FALSE) && (current_key_group < 0)) return;
#endif

	gint i;
	for (i=0; i<KEYS; i++)
	{
		if (clear_all || (system_keys[i].group == current_key_group))
		{
			g_free(dialog_data->user_key_value[i]);
			dialog_data->user_key_value[i] = g_strdup("");
		}
	}
}

gboolean clean_model_foreach(GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer userdata)
{
#ifdef DETAIL
	g_debug("! Launch clean_model_foreach() with model= %p, path = %p, iter = %p!", model, path, iter);
#endif
#ifdef SAFEMODE
	if (model==NULL) return FALSE;
#endif
	if (gtk_tree_model_iter_has_child (model, iter) == FALSE)
		gtk_tree_store_set (GTK_TREE_STORE (model), iter, 1, "", -1);
	return FALSE;
}


void clear_key_group(GtkButton *button, struct Dialog *dialog_data)
{
#ifdef DETAIL
	g_debug("! Launch clear_key_group() with button = %p, dialog_data = %p!", button, dialog_data);
#endif
	clear_key_groups(dialog_data, FALSE);
}

void clear_key_group_all(GtkButton *button, struct Dialog *dialog_data)
{
#ifdef DETAIL
	g_debug("! Launch clear_key_group_all() with button = %p, dialog_data = %p!", button, dialog_data);
#endif
	clear_key_groups(dialog_data, TRUE);
}

GtkWidget *add_text_to_notebook(GtkWidget *notebook, const gchar *label, const gchar *stock_id, const gchar *text)
{
#ifdef DETAIL
	g_debug("! Launch add_text_to_notebook() with notebook = %p, label = %s, stock_id = %s, text = %s",
		notebook, label, stock_id, text);
#endif
#ifdef SAFEMODE
	if (notebook==NULL) return NULL;
#endif
	GtkWidget *text_label = create_label_with_text(NULL, TRUE, TRUE, 0, text);
#ifdef SAFEMODE
	if (text_label==NULL) return NULL;
#endif
		set_widget_can_not_get_focus(text_label);

	GtkWidget *hbox = dirty_gtk_hbox_new(FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(hbox), 10);
	gtk_box_pack_start(GTK_BOX(hbox), text_label, TRUE, TRUE, 0);

	GtkWidget *label_hbox = dirty_gtk_hbox_new(FALSE, 0);
	set_widget_thickness(label_hbox, 0);
	GtkWidget *image = gtk_image_new_from_stock(stock_id, GTK_ICON_SIZE_MENU);
	gtk_box_pack_start(GTK_BOX(label_hbox), image, FALSE, FALSE, 0);
	GtkWidget *label_text = gtk_label_new(label);
	gtk_box_pack_start(GTK_BOX(label_hbox), label_text, TRUE, TRUE, 0);
	gtk_widget_show_all(label_hbox);

	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), hbox, label_hbox);

	return text_label;
}

void show_usage_text(GtkWidget *notebook, gpointer page, guint page_num, struct Dialog *dialog_data)
{
#ifdef DETAIL
	g_debug("! Launch show_usage_text() with notebook = %p, page = %p, page_num = %d, dialog_data = %p",
		notebook, page, page_num, dialog_data);
#endif
#ifdef SAFEMODE
	if (dialog_data==NULL) return;
#endif
	gint i;
	for (i=0; i<5; i++)
	{
#ifdef SAFEMODE
		if (dialog_data->operate[i]==NULL) continue;
#endif
		if (i==page_num)
			gtk_widget_set_no_show_all(dialog_data->operate[i], FALSE);
		else
		{
			gtk_widget_set_no_show_all(dialog_data->operate[i], TRUE);
			gtk_widget_hide(dialog_data->operate[i]);
		}
	}
#ifdef SAFEMODE
	if (dialog_data->window)
#endif
		gtk_widget_show_all(dialog_data->window);
}

//void err_page_data_is_null(gchar *function_name)
//{
//	gchar *err_msg = g_strdup_printf("%s: page_data = NULL\n\n"
//					 "Please report bug to %s, Thanks!",
//					 function_name,
//					 BUGREPORT);
//	error_dialog(NULL, err_msg, NULL, ERROR_MESSAGES);
//#ifdef DETAIL
//	g_debug("* free err_msg %p (%s) in set_encoding()", err_msg, err_msg);
//#endif
//	g_free(err_msg);
//}

void create_dialog(gchar *dialog_title_translation, gchar *dialog_title,  Dialog_Button_Type type,
		   GtkWidget *window, gboolean center, gboolean resizable, gint border_width,
		   gint response, gchar *icon, gchar *title, gboolean selectable, gint max_width_chars,
		   gboolean state_bottom, gint create_entry_hbox, gint entry_hbox_spacing,
		   struct Dialog *dialog_data)
{
#ifdef DETAIL
	g_debug("! Launch create_dialog() with dialog_title_translation = %s, "
		"dialog_title = %s, type = %d, window = %p, center = %d, resizable = %d, "
		"border_width = %d, response = %d, icon = %s, title = %s, selectable = %d, "
		"max_width_chars = %d, state_bottom = %d, create_entry_hbox = %d, dialog_data = %p",
		dialog_title_translation, dialog_title, type, window, center, resizable,
		border_width, response, icon, title, selectable, max_width_chars, state_bottom,
		create_entry_hbox, dialog_data);
#endif
#ifdef SAFEMODE
	if (dialog_data==NULL) return;
#endif
#ifdef USE_GTK_COLOR_CHOOSER
	switch (dialog_data->type)
	{
		case CHANGE_THE_FOREGROUND_COLOR:
		case CHANGE_THE_BACKGROUND_COLOR:
		case CHANGE_THE_CURSOR_COLOR:
		case CHANGE_THE_TEXT_COLOR_OF_WINDOW_TITLE:
		case CHANGE_THE_TEXT_COLOR_OF_CMDLINE:
		case CHANGE_THE_TEXT_COLOR_OF_CURRENT_DIR:
		case CHANGE_THE_TEXT_COLOR_OF_CUSTOM_PAGE_NAME:
		case CHANGE_THE_TEXT_COLOR_OF_ROOT_PRIVILEGES_CMDLINE:
		case CHANGE_THE_TEXT_COLOR_OF_NORMAL_TEXT:
			return;
	}
#endif
	gboolean BOTTON_ORDER = gtk_alternative_dialog_button_order(NULL);
	// g_debug("gtk_alternative_dialog_button_order = %d" ,gtk_alternative_dialog_button_order (NULL));

	// Strange behavior, If the locale is empty, or setted to 'C' or 'POSIX',
	// And 'default_locale' is setted in the profile,
	// All the UI will be translated to default_locale,
	// But the window title will be empty.
	// So, set it to un-translated string here.
	// g_debug("init_LC_CTYPE = %s", init_LC_CTYPE);
	if ((init_LC_CTYPE==NULL) || (init_LC_CTYPE[0]=='\0') ||
	    (! compare_strings(init_LC_CTYPE, "C", TRUE)) ||
	    (! compare_strings(init_LC_CTYPE, "\"C\"", TRUE)) ||
	    (! compare_strings(init_LC_CTYPE, "POSIX", TRUE)) ||
	    (! compare_strings(init_LC_CTYPE, "\"POSIX\"", TRUE)))
		dialog_title_translation = dialog_title;

	// g_debug("dialog_title_translation = %s", dialog_title_translation);
	switch (type)
	{
		case DIALOG_OK:
			dialog_data->window = gtk_dialog_new_with_buttons (dialog_title_translation,
								      GTK_WINDOW(window),
#ifdef EXIST_GTK_DIALOG_NO_SEPARATOR
								      GTK_DIALOG_NO_SEPARATOR |
#endif
									GTK_DIALOG_DESTROY_WITH_PARENT,
								      GTK_FAKE_STOCK_OK,
								      GTK_RESPONSE_OK,
								      NULL);
			break;
		case DIALOG_OK_CANCEL:
			if (BOTTON_ORDER)
				dialog_data->window = gtk_dialog_new_with_buttons (dialog_title_translation,
									      GTK_WINDOW(window),
#ifdef EXIST_GTK_DIALOG_NO_SEPARATOR
									      GTK_DIALOG_NO_SEPARATOR |
#endif
										GTK_DIALOG_DESTROY_WITH_PARENT,
									      GTK_FAKE_STOCK_OK,
									      GTK_RESPONSE_OK,
									      GTK_FAKE_STOCK_CANCEL,
									      GTK_RESPONSE_CANCEL,
									      NULL);
			else
				dialog_data->window = gtk_dialog_new_with_buttons (dialog_title_translation,
									      GTK_WINDOW(window),
#ifdef EXIST_GTK_DIALOG_NO_SEPARATOR
									      GTK_DIALOG_NO_SEPARATOR |
#endif
										GTK_DIALOG_DESTROY_WITH_PARENT,
									      GTK_FAKE_STOCK_CANCEL,
									      GTK_RESPONSE_CANCEL,
									      GTK_FAKE_STOCK_OK,
									      GTK_RESPONSE_OK,
									      NULL);
			break;
		case DIALOG_QUIT:
			dialog_data->window = gtk_dialog_new_with_buttons (dialog_title_translation,
								      GTK_WINDOW(window),
#ifdef EXIST_GTK_DIALOG_NO_SEPARATOR
								      GTK_DIALOG_NO_SEPARATOR |
#endif
									GTK_DIALOG_DESTROY_WITH_PARENT,
								      GTK_FAKE_STOCK_QUIT,
								      GTK_RESPONSE_CANCEL,
								      NULL);
			break;
		case DIALOG_NONE:
			dialog_data->window = gtk_dialog_new();
			gtk_window_set_title(GTK_WINDOW(dialog_data->window), dialog_title_translation);
			gtk_window_set_transient_for (GTK_WINDOW(dialog_data->window), GTK_WINDOW(window));
#ifdef EXIST_GTK_DIALOG_NO_SEPARATOR
			gtk_dialog_set_has_separator(GTK_DIALOG(dialog_data->window), FALSE);
#endif
			gtk_window_set_destroy_with_parent (GTK_WINDOW (dialog_data->window), TRUE);
			break;
		default:
#ifdef FATAL
			print_switch_out_of_range_error_dialog("create_dialog", "type", type);
#endif
			return;
	}

	// FIXME: It may be a bug in gtk+2?
	// if a dialog shown before main window is shown,
	// destroy the dialog will destroy the data of the icon, too.
	// and when showing main window with gtk_widget_show_all(win_data->window) later,
	// It will have no icon, and showing following error:
	// Gtk-CRITICAL **: gtk_window_realize_icon: assertion `info->icon_pixmap == NULL' failed
	// So, we need to set the icon for the dialog here.
	set_window_icon(dialog_data->window);

	if (center) gtk_window_set_position (GTK_WINDOW (dialog_data->window), GTK_WIN_POS_CENTER);
	gtk_window_set_resizable (GTK_WINDOW (dialog_data->window), resizable);
	gtk_container_set_border_width (GTK_CONTAINER (dialog_data->window), border_width);
	gtk_dialog_set_default_response(GTK_DIALOG(dialog_data->window), response);

	GtkWidget *main_hbox = dirty_gtk_hbox_new (FALSE, 5);
	gtk_container_add (GTK_CONTAINER (gtk_dialog_get_content_area(GTK_DIALOG(dialog_data->window))), main_hbox);
	GtkWidget *main_right_vbox = dirty_gtk_vbox_new (FALSE, 0);
	gtk_box_pack_end (GTK_BOX(main_hbox), main_right_vbox, FALSE, FALSE, 0);


	if (icon)
	{
		GtkWidget *icon_vbox = dirty_gtk_vbox_new (FALSE, 30);
		gtk_box_pack_start (GTK_BOX(main_hbox), icon_vbox, FALSE, FALSE, 10);
		GtkWidget *icon_image = gtk_image_new_from_stock (icon, GTK_ICON_SIZE_DIALOG);
		gtk_box_pack_start (GTK_BOX(icon_vbox), icon_image, FALSE, FALSE, 10);
	}

	GtkWidget *state_vbox =NULL;
	if (title)
		state_vbox = dirty_gtk_vbox_new (FALSE, 15);
	else
		state_vbox = dirty_gtk_vbox_new (FALSE, 0);

	gtk_box_pack_start (GTK_BOX(main_hbox), state_vbox, TRUE, TRUE, 0);

	if (title)
		dialog_data->title_label = create_label_with_text(state_vbox, TRUE, selectable, max_width_chars, title);

	if (state_bottom)
	{
		GtkWidget *state_bottom_hbox = dirty_gtk_hbox_new (FALSE, 3);
		gtk_box_pack_end (GTK_BOX(state_vbox), state_bottom_hbox, FALSE, FALSE, 0);
	}

	if (create_entry_hbox)
	{
		switch (create_entry_hbox)
		{
			case BOX_HORIZONTAL:
				dialog_data->box = dirty_gtk_hbox_new (FALSE, entry_hbox_spacing);
				break;
			case BOX_VERTICALITY:
				dialog_data->box = dirty_gtk_vbox_new (FALSE, entry_hbox_spacing);
				break;
			default:
#ifdef FATAL
				print_switch_out_of_range_error_dialog("create_dialog",
								       "create_entry_hbox",
								       create_entry_hbox);
#endif
				break;
		}
		if (title)
			gtk_box_pack_start (GTK_BOX(state_vbox), dialog_data->box, TRUE, TRUE, 10);
		else
			gtk_box_pack_start (GTK_BOX(state_vbox), dialog_data->box, TRUE, TRUE, 0);
	}
}
