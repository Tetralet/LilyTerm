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

#include "font.h"

// extern GtkWidget *current_vte;
extern GtkWidget *menu_active_window;

void set_vte_font(GtkWidget *widget, Font_Set_Type type)
{
#ifdef DETAIL
	g_debug("! Launch set_vte_font() with type = %d", type);
#endif
#ifdef SAFEMODE
	if (menu_active_window==NULL) return;
#endif

	// GtkWidget *vte = current_vte;
	struct Window *win_data = (struct Window *)g_object_get_data(G_OBJECT(menu_active_window), "Win_Data");
#ifdef SAFEMODE
	if (win_data==NULL) return;
#endif
	GtkWidget *vte = win_data->current_vte;
	gchar *new_font_name = NULL;

	// type 0, FONT_RESET: reset current page's font size
	// type 1, FONT_INCREASE: increase current page's font size
	// type 2, FONT_DECREASE: decrease current page's font size
	// type 3, FONT_ZOOM_OUT: increase window size & font size for every vte
	// type 4, FONT_ZOOM_IN: decrease window size & font size for every vte
	// type 5, FONT_RESET_DEFAULT: reset window size & font size to default for every vte
	// type 6, FONT_RESET_SYSTEM: reset window size & font size to system for every vte
	// type 7, FONT_SET_TO_SELECTED: change every vte to the selected font name

	switch (type)
	{
		case FONT_RESET:		// 0
			// reset current page's font size
			new_font_name = get_resize_font(vte, FONT_NAME_RESTORE);
			reset_vte_size(vte, new_font_name, RESET_CURRENT_TAB_FONT);
			break;
		case FONT_INCREASE:		// 1
			// increase current page's font size
			new_font_name = get_resize_font(vte, FONT_NAME_INCREASE);
			reset_vte_size(vte, new_font_name, RESET_CURRENT_TAB_FONT);
			break;
		case FONT_DECREASE:		// 2
			// decrease current page's font size
			new_font_name = get_resize_font(vte, FONT_NAME_DECREASE);
			reset_vte_size(vte, new_font_name, RESET_CURRENT_TAB_FONT);
			break;
		case FONT_ZOOM_OUT:		// 3
			// increase window size & font size for every vte
			new_font_name = get_resize_font(vte, FONT_NAME_ZOOM_OUT);
			reset_vte_size(vte, new_font_name, RESET_ALL_TO_CURRENT_TAB);
			break;
		case FONT_ZOOM_IN:		// 4
			// decrease window size & font size for every vte
			new_font_name = get_resize_font(vte, FONT_NAME_ZOOM_IN);
			reset_vte_size(vte, new_font_name, RESET_ALL_TO_CURRENT_TAB);
			break;
		case FONT_RESET_DEFAULT:	// 5
			// reset window size & font size to default for every vte
			new_font_name = get_resize_font(vte, FONT_NAME_DEFAULT);
			reset_vte_size(vte, new_font_name, RESET_ALL_TO_DEFAULT);
			break;
		case FONT_RESET_SYSTEM:		// 6
			// reset window size & font size to system for every vte
			new_font_name = get_resize_font(vte, FONT_NAME_SYSTEM);
			reset_vte_size(vte, new_font_name, RESET_ALL_TO_SYSTEM);
			break;
		case FONT_SET_TO_SELECTED:	// 7
			// change every vte to the selected font name
			new_font_name = get_resize_font(vte, FONT_NAME_UPDATE);
			reset_vte_size(vte, new_font_name, RESET_ALL_TO_CURRENT_TAB);
			break;
		default:
#ifdef FATAL
			print_switch_out_of_range_error_dialog("set_vte_font", "type", type);
#endif
			break;
	}
	g_free(new_font_name);
}

// it will update the new_font_name and page_data->font_name
gchar *get_resize_font(GtkWidget *vte, Font_Name_Type type)
{
#ifdef DETAIL
	g_debug("! Launch get_resize_font() for vte %p with type %d", vte, type);
#endif
#ifdef SAFEMODE
	if (vte==NULL) return NULL;
#endif
	// we must insure that vte!=NULL
	struct Page *page_data = (struct Page *)g_object_get_data(G_OBJECT(vte), "Page_Data");
#ifdef SAFEMODE
	if (page_data==NULL) return NULL;
#endif
	struct Window *win_data = (struct Window *)g_object_get_data(G_OBJECT(page_data->window), "Win_Data");
#ifdef SAFEMODE
	if (win_data==NULL) return NULL;
#endif
	// g_debug("Get win_data = %d when get resize font!", win_data);
	// type 0, FONT_NAME_DEFAULT: restore font to default_font_name
	// type 1, FONT_NAME_SYSTEM: restore font to system_font_name
	// type 2, FONT_NAME_RESTORE: restore font to restore_font_name
	// type 3, FONT_NAME_UPDATE: do nothing but only update new_font_name
	// type 4, FONT_NAME_ZOOM_OUT: increase window by *1.1 or +1
	// type 5, FONT_NAME_ZOOM_IN: decrease window by /1.1 or -1
	// type 6, FONT_NAME_INCREASE: increase font by *1.1 or +1
	// type 7, FONT_NAME_DECREASE: decrease font by /1.1 or -1

	if (win_data->restore_font_name == NULL)
	{
		// win_data->restore_font_name = g_strdup(page_data->font_name);
		win_data->restore_font_name = g_strdup(win_data->default_font_name);
		// g_debug("Restore the font to %s!", win_data->restore_font_name);
		if (type==FONT_NAME_RESTORE)
			return g_strdup(page_data->font_name);
	}

	switch (type)
	{
		case FONT_NAME_DEFAULT:
		case FONT_NAME_SYSTEM:
		case FONT_NAME_RESTORE:
			g_free(page_data->font_name);
			break;
		default:
			break;
	}

	// we use font_size to save current font size
	// font_size = (the size in font_name) * PANGO_SCALE
	// if font_size == 0 -> use the data in font_name

	switch (type)
	{
		case FONT_NAME_DEFAULT:
			// restore font to default_font_name
			page_data->font_name = g_strdup(win_data->default_font_name);
			page_data->font_size = 0;
			break;
		case FONT_NAME_SYSTEM:
			// restore font to default_font_name
			page_data->font_name = g_strdup(SYSTEM_FONT_NAME);
			page_data->font_size = 0;
			break;
		case FONT_NAME_RESTORE:
			// restore font to default_font_name
			page_data->font_name = g_strdup(win_data->restore_font_name);
			page_data->font_size = 0;
			break;
		case FONT_NAME_UPDATE:
			break;
		case FONT_NAME_ZOOM_OUT:
		case FONT_NAME_ZOOM_IN:
		case FONT_NAME_INCREASE:
		case FONT_NAME_DECREASE:
		{
#ifdef SAFEMODE
			if (page_data->font_name==NULL) break;
#endif
			gint oldfontsize=0, fontsize=0;

			// g_debug("old font name: %s", page_data->font_name);
			PangoFontDescription *font_desc = pango_font_description_from_string(
								page_data->font_name);
			// increase/decrease font
			oldfontsize = (pango_font_description_get_size(font_desc)/PANGO_SCALE);
			if (page_data->font_size==0)
				page_data->font_size = pango_font_description_get_size(font_desc);

			switch (type)
			{
				// g_debug("window_resize_ratio = %f",  win_data->window_resize_ratio);
				case FONT_NAME_ZOOM_OUT:
					if (win_data->window_resize_ratio)
						page_data->font_size = page_data->font_size *
								       win_data->window_resize_ratio +
								       0.5;
					else
						page_data->font_size = page_data->font_size + PANGO_SCALE;
					break;
				case FONT_NAME_ZOOM_IN:
					if (win_data->window_resize_ratio)
						page_data->font_size = page_data->font_size /
								       win_data->window_resize_ratio +
								       0.5;
					else
						page_data->font_size = page_data->font_size - PANGO_SCALE;
					break;
				case FONT_NAME_INCREASE:
					if (win_data->font_resize_ratio)
						page_data->font_size = page_data->font_size *
								       win_data->font_resize_ratio +
								       0.5;
					else
						page_data->font_size = page_data->font_size + PANGO_SCALE;
					break;
				case FONT_NAME_DECREASE:
					if (win_data->font_resize_ratio)
						page_data->font_size = page_data->font_size /
								       win_data->font_resize_ratio +
								       0.5;
					else
						page_data->font_size = page_data->font_size - PANGO_SCALE;
					break;
				default:
					break;
			}
			// g_debug("font_size = %d", page_data->font_size);
			fontsize = (page_data->font_size)/PANGO_SCALE;

			// to avoid the fontsize is unchanged or = 0
			switch (type)
			{
				case FONT_NAME_ZOOM_OUT:
				case FONT_NAME_INCREASE:
					if (oldfontsize==fontsize)
						fontsize++;
					break;
				case FONT_NAME_ZOOM_IN:
				case FONT_NAME_DECREASE:
					if (oldfontsize==fontsize)
						fontsize--;
					if (fontsize<1)
						fontsize=1;
					break;
				default:
					break;
			}

			// g_debug("Trying to change the font size to %d.", fontsize);
			pango_font_description_set_size(font_desc, fontsize*PANGO_SCALE);
			g_free(page_data->font_name);
			page_data->font_name = pango_font_description_to_string(font_desc);
			pango_font_description_free(font_desc);
			break;
		default:
#ifdef FATAL
			print_switch_out_of_range_error_dialog("get_resize_font", "type", type);
#endif
			return NULL;
		}
	}

	// g_debug("new font name: %s", page_data->font_name);

	switch (type)
	{
		case FONT_NAME_DEFAULT:
		case FONT_NAME_SYSTEM:
		case FONT_NAME_UPDATE:
		case FONT_NAME_ZOOM_OUT:
		case FONT_NAME_ZOOM_IN:
			// if not using <Ctrl><+ - enter> to change the font size.
			g_free(win_data->restore_font_name);
			win_data->restore_font_name = g_strdup(page_data->font_name);
			break;
		case FONT_NAME_INCREASE:
		case FONT_NAME_DECREASE:
			// Check if we can specify page_data->font_size = 0
			if ( ! compare_strings(page_data->font_name, win_data->restore_font_name, TRUE))
			{
				page_data->font_size = 0;
				// g_debug("The font is restored to win_data->restore_font_name");
			}
			break;
		default:
			break;
	}
	return g_strdup(page_data->font_name);
}

void reset_vte_size(GtkWidget *vte, gchar *new_font_name, Font_Reset_Type type)
{
#ifdef DETAIL
	g_debug("! Launch reset_vte_size() with vte = %p, new_font_name = %s, type = %d",
		vte, new_font_name, type);
#endif
#ifdef SAFEMODE
	if ((vte==NULL) || (new_font_name==NULL)) return;
#endif

	// type 0, RESET_CURRENT_TAB_FONT: change current page's font
	// type 1, RESET_ALL_TO_CURRENT_TAB: apply current column & row to every vte
	// type 2, RESET_ALL_TO_DEFAULT: apply default column & row to every vte
	// type 3, RESET_ALL_TO_SYSTEM: apply system column & row to every vte

	struct Page *page_data = (struct Page *)g_object_get_data(G_OBJECT(vte), "Page_Data");
#ifdef SAFEMODE
	if (page_data==NULL) return;
#endif
	struct Window *win_data = (struct Window *)g_object_get_data(G_OBJECT(page_data->window), "Win_Data");
#ifdef SAFEMODE
	if (win_data==NULL) return;
#endif
	// g_debug("Get win_data = %d when reset vte size!", win_data);

	switch (type)
	{
		case RESET_CURRENT_TAB_FONT:
			// We need to apply a new font to a single vte.
			// so that we should insure that this won't change the size of window.
			// g_debug("Trying to apply font %s to vte", current_font_name);
			fake_vte_terminal_set_font_from_string( vte,
								new_font_name,
								win_data->font_anti_alias);
			// g_debug("reset_vte_size(): call window_resizable() with run_once = %d", win_data->hints_type);
			// g_debug("reset_vte_size(): launch update_window_hint()!");
#  ifdef GEOMETRY
			fprintf(stderr, "\033[1;37m!! reset_vte_size(): call update_window_hint()\033[0m\n");
#  endif
			update_window_hint(win_data, page_data);
			break;
		case RESET_ALL_TO_CURRENT_TAB:
			// increase/decrease window size & font size for every vte
			// g_debug("Trying to apply font %s to every vte", current_font_name);
			// struct Page *page_data = (struct Page *)g_object_get_data(G_OBJECT(current_vte), "Page_Data");
#ifdef USE_GTK2_GEOMETRY_METHOD
			apply_font_to_every_vte(page_data->window, new_font_name,
						vte_terminal_get_column_count(VTE_TERMINAL(win_data->current_vte)),
						vte_terminal_get_row_count(VTE_TERMINAL(win_data->current_vte)));
#endif
#ifdef USE_GTK3_GEOMETRY_METHOD
			apply_font_to_every_vte( page_data->window, new_font_name, win_data->geometry_width, win_data->geometry_height);
#endif
			break;
		case RESET_ALL_TO_DEFAULT:
			// reset window size & font size for every vte
			// g_debug("Trying to apply font %s to every vte", current_font_name);
			apply_font_to_every_vte(page_data->window, new_font_name,
						win_data->default_column, win_data->default_row);
			break;
		case RESET_ALL_TO_SYSTEM:
			// reset window size & font size for every vte
			// g_debug("Trying to apply font %s to every vte", current_font_name);
			apply_font_to_every_vte(page_data->window, new_font_name,
						SYSTEM_COLUMN, SYSTEM_ROW);
			break;
		default:
#ifdef FATAL
			print_switch_out_of_range_error_dialog("reset_vte_size", "type", type);
#endif
			break;
	}
}

void apply_font_to_every_vte(GtkWidget *window, gchar *new_font_name, glong column, glong row)
{
#ifdef DETAIL
	g_debug("! Launch apply_font_to_every_vte() with window = %p, new_font_name = %s,"
		" column = %ld, row = %ld", window, new_font_name, column, row);
#endif
#ifdef SAFEMODE
	if ((window==NULL) || (new_font_name==NULL) || (column<1) || (row<1)) return;
#endif
	struct Window *win_data = (struct Window *)g_object_get_data(G_OBJECT(window), "Win_Data");
	// g_debug("Get win_data = %d when apply font to every vte!", win_data);
#ifdef SAFEMODE
	if (win_data==NULL) return;
#endif

	struct Page *page_data = NULL;
	gint i;

	// g_debug("Trying to apply every vte to %dx%d!", column, row);
	// g_debug("Trying to apply font %s to every vte!", new_font_name);

	for (i=0; i<gtk_notebook_get_n_pages(GTK_NOTEBOOK(win_data->notebook)); i++)
	{
		page_data = get_page_data_from_nth_page(win_data, i);
#ifdef SAFEMODE
		if (page_data==NULL) continue;
#endif
		// g_debug("The default font for %d page is: %s (%s)", i, page_data->font_name, new_font_name);
		fake_vte_terminal_set_font_from_string( page_data->vte,
							new_font_name,
							win_data->font_anti_alias);
		vte_terminal_set_size(VTE_TERMINAL(page_data->vte), column, row);
		g_free(page_data->font_name);
		page_data->font_name = g_strdup(new_font_name);
		// g_debug("The new font for %d page is: %s (%s)", i, page_data->font_name, new_font_name);
	}

	// g_debug("* Set hints to HINTS_FONT_BASE!, win_data->window_status = %d", win_data->window_status);
	win_data->hints_type = HINTS_FONT_BASE;

	// win_data->keep_vte_size |= 0x30;
	// g_debug("window_resizable in apply_font_to_every_vte!");
	// window_resizable(window, page_data->vte, 2, 1);
	// g_debug("apply_font_to_every_vte(): launch keep_window_size()!");

	// Don't need to call keep_gtk2_window_size() when fullscreen
	switch (win_data->window_status)
	{
#ifdef USE_GTK2_GEOMETRY_METHOD
		case FULLSCREEN_NORMAL:
		case FULLSCREEN_UNFS_OK:
#  ifdef GEOMETRY
			g_debug("@ apply_font_to_every_vte(): Call keep_gtk2_window_size() with keep_vte_size = %x",
				win_data->keep_vte_size);
#  endif
			keep_gtk2_window_size (win_data, page_data->vte, 0x380);
#endif
#ifdef USE_GTK3_GEOMETRY_METHOD
		case WINDOW_NORMAL:
		case WINDOW_APPLY_PROFILE_NORMAL:
#  ifdef GEOMETRY
			fprintf(stderr, "\033[1;%dm!! apply_font_to_every_vte(win_data %p): Calling keep_gtk3_window_size() with hints_type = %d\n",
				ANSI_COLOR_MAGENTA, win_data, win_data->hints_type);
#  endif
			window_resizable(win_data->window, win_data->current_vte, win_data->hints_type);
			if (win_data->window_status==WINDOW_NORMAL)
				win_data->resize_type = GEOMETRY_AUTOMATIC;
			else
				win_data->resize_type = GEOMETRY_CUSTOM;
			win_data->geometry_width = column;
			win_data->geometry_height = row;
			keep_gtk3_window_size(win_data, TRUE);
#endif
			break;
		default:
			break;
	}
}

gboolean check_if_every_vte_is_using_restore_font_name(struct Window *win_data)
{
#ifdef DETAIL
	g_debug("! Launch check_if_every_vte_is_using_restore_font_name() with win_data = %p", win_data);
#endif
#ifdef SAFEMODE
	if ((win_data==NULL) || (win_data->notebook==NULL)) return FALSE;
#endif
	if (win_data->restore_font_name == NULL)
	//	win_data->restore_font_name = g_strdup(page_data->font_name);
		win_data->restore_font_name = g_strdup(win_data->default_font_name);

	gint i;
	struct Page *page_data = NULL;
	gboolean return_value = TRUE;
	for (i=0; i<gtk_notebook_get_n_pages(GTK_NOTEBOOK(win_data->notebook)); i++)
	{
		page_data = get_page_data_from_nth_page(win_data, i);
#ifdef SAFEMODE
		if (page_data==NULL) continue;
#endif
		if (compare_strings(page_data->font_name, win_data->restore_font_name, TRUE))
		{
			return_value = FALSE;
			break;
		}
	}
	return return_value;
}

void fake_vte_terminal_set_font_from_string(GtkWidget *vte, const char *font_name, gboolean anti_alias)
{
#ifdef DETAIL
	g_debug("! Launch fake_vte_terminal_set_font_from_string() with vte = %p, font_name = %s, anti_alias = %d", vte, font_name, anti_alias);
#endif
#ifdef SAFEMODE
	if ((vte==NULL) || (font_name==NULL)) return;
#endif

#ifdef USE_VTE_TERMINAL_SET_FONT
	PangoFontDescription *font_desc = pango_font_description_from_string(font_name);
	vte_terminal_set_font(VTE_TERMINAL(vte), font_desc);
#else
	vte_terminal_set_font_from_string_full( VTE_TERMINAL(vte), font_name, anti_alias);
#endif
}
