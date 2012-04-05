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

#include "property.h"

extern gboolean proc_exist;
extern struct Command command[COMMAND];

void adjust_ansi_color(GdkColor color[COLOR], GdkColor color_orig[COLOR], gdouble color_brightness, gboolean invert_color)
{
#ifdef DETAIL
	g_debug("! Launch adjust_ansi_color() with color_brightness = %f, invert_color = %d",  color_brightness, invert_color);
#endif
#ifdef DEFENSIVE
	if ((color==NULL) || (color_orig==NULL)) return;
#endif
	// g_debug("Get win_data = %d when set background saturation!", win_data);
	color_brightness = CLAMP(color_brightness, -1, 1);

	gint i;
	if (invert_color)
	{
		gint half = COLOR/2;
		for (i=0; i<half; i++)
		{
			adjust_ansi_color_severally(&color[i], &color_orig[i+half], color_brightness);
			adjust_ansi_color_severally(&color[i+half], &color_orig[i], color_brightness);
		}
	}
	else
	{
		for (i=0; i<COLOR; i++)
		{
			adjust_ansi_color_severally(&color[i], &color_orig[i], color_brightness);
			//g_debug("color_orig[%d] = %x, %x, %x",
			//	i,
			//	color_orig[i].red,
			//	color_orig[i].green,
			//	color_orig[i].blue);

			//g_debug("color[%d] = %x, %x, %x",
			//	i,
			//	color[i].red,
			//	color[i].green,
			//	color[i].blue);
		}
	}
}

void adjust_ansi_color_severally(GdkColor *color, GdkColor *color_orig, gdouble color_brightness)
{
#ifdef DETAIL
	g_debug("! Launch adjust_ansi_color_severally()");
#endif
#ifdef DEFENSIVE
	if ((color==NULL) || (color_orig==NULL)) return;
#endif
	if (color_brightness>=0)
	{
		color->red = (0xffff - color_orig->red) * color_brightness + color_orig->red;
		color->green = (0xffff - color_orig->green) * color_brightness + color_orig->green;
		color->blue = (0xffff - color_orig->blue) * color_brightness + color_orig->blue;
	}
	else
	{
		color->red = (1 + color_brightness) * color_orig->red;
		color->green = (1 + color_brightness) * color_orig->green;
		color->blue = (1 + color_brightness) * color_orig->blue;
	}
}

void set_color_brightness(struct Window *win_data)
{
#ifdef DETAIL
	g_debug("! Launch set_color_brightness() with win_data = %p", win_data);
#endif
#ifdef DEFENSIVE
	if (win_data==NULL) return;
#endif
	// g_debug("win_data->color_brightness = %3f", win_data->color_brightness);
	win_data->fg_color_inactive = get_inactive_color (win_data->fg_color,
							  win_data->color_brightness_inactive,
							  win_data->color_brightness);
	adjust_ansi_color(win_data->color, win_data->color_orig, win_data->color_brightness, win_data->invert_color);
	adjust_ansi_color(win_data->color_inactive, win_data->color_orig, win_data->color_brightness_inactive, win_data->invert_color);
}

// to init a new page
void init_new_page(struct Window *win_data,
		   struct Page *page_data,
		   glong column,
		   glong row)
{
#ifdef DETAIL
	g_debug("! Launch init_new_page() with win_data = %p, page_data = %p, "
		" column = %ld, row = %ld", win_data, page_data, column, row);
#endif
#ifdef DEFENSIVE
	if ((win_data==NULL) || (page_data==NULL) || (page_data->vte==NULL)) return;
#endif
	// g_debug("Get win_data = %d when initing new page!", win_data);

#ifdef DEFENSIVE
	if (page_data->font_name)
	{
#endif
		// set font
		// g_debug("Set Font AA = %d", win_data->font_anti_alias);
		vte_terminal_set_font_from_string_full (VTE_TERMINAL(page_data->vte),
							page_data->font_name,
							win_data->font_anti_alias);
#ifdef DEFENSIVE
	}
#endif
	//g_debug("Got font size from %s: %d", font_name, pango_font_description_get_size (
	//	  pango_font_description_from_string(font_name))/PANGO_SCALE);

	// set terminal size
	// g_debug("Set the vte size to: %dx%d", column, row);
#ifdef DEFENSIVE
	if (page_data->vte)
#endif
		vte_terminal_set_size(VTE_TERMINAL(page_data->vte), column, row);

#  ifdef GEOMETRY
	g_debug("@ init_new_page(for %p, vte = %p): Set win_data->keep_vte_size to %d, and column = %ld, row = %ld",
		win_data->window, page_data->vte, win_data->keep_vte_size, column, row);
#  endif

	set_vte_color(win_data, page_data);

	// set transparent
	set_background_saturation(NULL, 0, win_data->background_saturation, page_data->vte);

	// other settings
	vte_terminal_set_word_chars(VTE_TERMINAL(page_data->vte), win_data->word_chars);
	vte_terminal_set_scrollback_lines(VTE_TERMINAL(page_data->vte), win_data->scrollback_lines);

	// some fixed parameter
	vte_terminal_set_scroll_on_output(VTE_TERMINAL(page_data->vte), FALSE);
	vte_terminal_set_scroll_on_keystroke(VTE_TERMINAL(page_data->vte), TRUE);
	// vte_terminal_set_backspace_binding (VTE_TERMINAL(page_data->vte), VTE_ERASE_ASCII_DELETE);
	// vte_terminal_set_delete_binding (VTE_TERMINAL(page_data->vte), VTE_ERASE_ASCII_DELETE);
	// vte_terminal_set_emulation (VTE_TERMINAL(page_data->vte), "xterm");
#ifdef ENABLE_FIND_STRING
	vte_terminal_search_set_wrap_around (VTE_TERMINAL(page_data->vte), TRUE);
#endif

	set_hyprelink(win_data, page_data);
	set_cursor_blink(win_data, page_data);

	vte_terminal_set_audible_bell (VTE_TERMINAL(page_data->vte), win_data->audible_bell);
	vte_terminal_set_visible_bell (VTE_TERMINAL(page_data->vte), win_data->visible_bell);
	// g_debug("init_new_page(): call set_vte_urgent_bell()");
	set_vte_urgent_bell(win_data, page_data);
	vte_terminal_set_backspace_binding (VTE_TERMINAL(page_data->vte), win_data->erase_binding);
#ifdef ENABLE_CURSOR_SHAPE
	vte_terminal_set_cursor_shape(VTE_TERMINAL(page_data->vte), win_data->cursor_shape);
#endif
	vte_terminal_set_emulation (VTE_TERMINAL(page_data->vte), win_data->emulate_term);
}


void set_cursor_blink(struct Window *win_data, struct Page *page_data)
{
#ifdef DETAIL
	g_debug("! Launch set_cursor_blink() with win_data = %p, page_data = %p", win_data, page_data);
#endif
#ifdef DEFENSIVE
	if ((win_data==NULL) || (page_data==NULL) || (page_data->vte==NULL)) return;
#endif
	// g_debug("set_cursor_blink(): win_data->cursor_blinks = %d", win_data->cursor_blinks);


#ifdef USE_NEW_VTE_CURSOR_BLINKS_MODE
	vte_terminal_set_cursor_blink_mode (VTE_TERMINAL(page_data->vte), win_data->cursor_blinks);
#else
	vte_terminal_set_cursor_blinks (VTE_TERMINAL(page_data->vte), win_data->cursor_blinks);
#endif
}

void set_hyprelink(struct Window *win_data, struct Page *page_data)
{
#ifdef DETAIL
	g_debug("! Launch set_hyprelink() with win_data = %p, page_data = %p", win_data, page_data);
#endif
#ifdef DEFENSIVE
	if ((win_data==NULL) || (page_data==NULL) || (page_data->vte==NULL)) return;
#endif
	if (win_data->enable_hyperlink && win_data->enable_function_key)
	{
		gint i;
		for (i=0; i<COMMAND; i++)
		{
#ifdef USE_NEW_VTE_MATCH_ADD_GREGEX
			GRegex *regex = g_regex_new (command[i].match, G_REGEX_CASELESS | G_REGEX_OPTIMIZE,
						     0, NULL);
			page_data->tag[i] = vte_terminal_match_add_gregex (VTE_TERMINAL(page_data->vte),
									   regex, 0);
			g_regex_unref (regex);
#else
			page_data->tag[i] = vte_terminal_match_add (VTE_TERMINAL(page_data->vte),
								    command[i].match);
#endif
			vte_terminal_match_set_cursor_type(VTE_TERMINAL(page_data->vte),
							   page_data->tag[i],
							   GDK_HAND2);
		}
	}
	else
		vte_terminal_match_clear_all(VTE_TERMINAL(page_data->vte));
}

void set_vte_color(struct Window *win_data, struct Page *page_data)
{
#ifdef DETAIL
	g_debug("! Launch set_vte_color() with win_data = %p, page_data = %p", win_data, page_data);
#endif
#ifdef DEFENSIVE
	if ((win_data==NULL) || (page_data==NULL) || (page_data->vte==NULL)) return;
#endif
	// set font/background colors
	vte_terminal_set_default_colors(VTE_TERMINAL(page_data->vte));
	// g_debug("win_data->using_custom_color = %d", win_data->using_custom_color);
	if (win_data->using_custom_color)
	{
		// g_debug("win_data->using_custom_color = %d", win_data->using_custom_color);
		vte_terminal_set_colors(VTE_TERMINAL(page_data->vte),
					&(win_data->fg_color),
					&(win_data->bg_color),
					win_data->color, 16);
	}
	else
	{
		vte_terminal_set_color_foreground(VTE_TERMINAL(page_data->vte), &(win_data->fg_color));
		vte_terminal_set_color_background(VTE_TERMINAL(page_data->vte), &(win_data->bg_color));
	}
	vte_terminal_set_color_bold (VTE_TERMINAL(page_data->vte), &(win_data->fg_color));
	vte_terminal_set_background_tint_color (VTE_TERMINAL(page_data->vte), &(win_data->bg_color));
}

void switch_color(struct Window *win_data)
{
#ifdef DETAIL
	g_debug("! Launch switch_color() with win_data = %p", win_data);
#endif
#ifdef DEFENSIVE
	if ((win_data==NULL)) return;
#endif
	GdkColor tmp_color = win_data->fg_color;
	win_data->fg_color = win_data->bg_color;
	win_data->bg_color = tmp_color;

	gchar *color = win_data->foreground_color;
	win_data->foreground_color = win_data->background_color;
	win_data->background_color = color;

	gdouble color_brightness = win_data->color_brightness;
	win_data->color_brightness = win_data->color_brightness_inactive;
	win_data->color_brightness_inactive = color_brightness;
}

void set_page_width(struct Window *win_data, struct Page *page_data)
{
#ifdef DETAIL
	g_debug("! Launch set_page_width() with win_data = %p, page_data = %p", win_data, page_data);
#endif
#ifdef DEFENSIVE
	if ((win_data==NULL) || (page_data==NULL) || (page_data->label_text==NULL)) return;
#endif
	gtk_label_set_width_chars(GTK_LABEL(page_data->label_text), win_data->page_width);
}

void pack_vte_and_scroll_bar_to_hbox(struct Window *win_data, struct Page *page_data)
{
#ifdef DETAIL
	g_debug("! Launch pack_vte_and_scroll_bar_to_hbox() with win_data = %p, page_data = %p", win_data, page_data);
#endif
#ifdef DEFENSIVE
	if ((win_data==NULL) || (page_data==NULL) || (page_data->hbox==NULL)) return;
#endif
	if (win_data->scroll_bar_position)
	{
		gtk_box_pack_start(GTK_BOX(page_data->hbox), page_data->vte, TRUE, TRUE, 0);
		gtk_box_pack_start(GTK_BOX(page_data->hbox), page_data->scroll_bar, FALSE, FALSE, 0);
	}
	else
	{
		gtk_box_pack_end(GTK_BOX(page_data->hbox), page_data->vte, TRUE, TRUE, 0);
		gtk_box_pack_end(GTK_BOX(page_data->hbox), page_data->scroll_bar, FALSE, FALSE, 0);
	}
	if (! win_data->show_scroll_bar)
		gtk_widget_set_no_show_all (page_data->scroll_bar, TRUE);
}

gboolean check_show_or_hide_scroll_bar(struct Window *win_data)
{
#ifdef DETAIL
	g_debug("! Launch check_show_or_hide_scroll_bar() with win_data = %p", win_data);
#endif
#ifdef DEFENSIVE
	if (win_data==NULL) return FALSE;
#endif
	gboolean show = FALSE;
	// g_debug("check_show_or_hide_scroll_bar(): win_data->show_scroll_bar = %d", win_data->show_scroll_bar);
	switch (win_data->show_scroll_bar)
	{
		case AUTOMATIC:
			show = ! win_data->true_fullscreen;
			break;
		case ON:
		case FORCE_ON:
			show = TRUE;
			break;
		default:
			break;
	}
	// g_debug("check_show_or_hide_scroll_bar(): Got win_data->show_scroll_bar = %d, show = %d",
	//	win_data->show_scroll_bar, show);
	return show;
}

void show_and_hide_scroll_bar(struct Page *page_data, gboolean show_scroll_bar)
{
#ifdef DETAIL
	g_debug("! Launch hide_scroll_bar() with page_data = %p", page_data);
#endif
#ifdef DEFENSIVE
	if ((page_data==NULL) || (page_data->scroll_bar==NULL)) return;
#endif
	if (show_scroll_bar)
		gtk_widget_show (page_data->scroll_bar);
	else
		gtk_widget_hide (page_data->scroll_bar);
}

void add_remove_page_timeout_id(struct Window *win_data, struct Page *page_data)
{
#ifdef DETAIL
	g_debug("! Launch add_remove_page_timeout_id() with win_data = %p, page_data = %p", win_data, page_data);
#endif
#ifdef DEFENSIVE
	if ((win_data==NULL) || (page_data==NULL)) return;
#endif
	if (page_data->timeout_id)
	{
		g_source_remove (page_data->timeout_id);
		// get_and_update_page_name(page_data);
		// win_data->page_names_no++;
		//if ((page_data->custom_page_name == NULL) && (page_data->label_text != NULL))
		//	 page_data->custom_page_name = g_strdup(page_data->page_name);
		page_data->timeout_id = 0;
	}
	else
		// monitor_cmdline(page_data->monitor, page_data->pid);
		// monitor_cmdline(page_data->channel, page_data->pid);
		page_data->timeout_id = g_timeout_add_seconds (1,
							       (GSourceFunc)monitor_cmdline,
							       page_data);
}

void add_remove_window_title_changed_signal(struct Page *page_data)
{
#ifdef DETAIL
	g_debug("! Launch add_remove_window_title_changed_signal() with page_data = %p", page_data);
#endif
#ifdef DEFENSIVE
	if (page_data==NULL) return;
#endif
	if (page_data->page_shows_window_title)
		page_data->window_title_signal = g_signal_connect(page_data->vte, "window-title-changed",
								  G_CALLBACK(update_page_window_title), page_data);
	else if (page_data->window_title_signal)
	{
		g_signal_handler_disconnect(page_data->vte, page_data->window_title_signal);
		page_data->window_title_signal = 0;
	}

}

gboolean set_background_saturation(GtkRange *range, GtkScrollType scroll, gdouble value, GtkWidget *vte)
{
#ifdef DETAIL
	g_debug("! Launch set_background_saturation() with value = %f, vte = %p", value, vte);
#endif
#ifdef DEFENSIVE
	if (vte==NULL) return FALSE;
#endif
	struct Page *page_data = (struct Page *)g_object_get_data(G_OBJECT(vte), "Page_Data");
#ifdef DEFENSIVE
	if (page_data==NULL || (page_data->window==NULL)) return FALSE;
#endif
	struct Window *win_data = (struct Window *)g_object_get_data(G_OBJECT(page_data->window), "Win_Data");
#ifdef DEFENSIVE
	if (win_data==NULL) return FALSE;
#endif
	// g_debug("Get win_data = %d when set background saturation!", win_data);

	value = CLAMP(value, 0, 1);

#ifdef ENABLE_RGBA
	if (win_data->use_rgba == -1)
	{
		if (win_data->transparent_background)
			vte_terminal_set_opacity(VTE_TERMINAL(vte), (1-value) * 65535);
		else
			vte_terminal_set_opacity(VTE_TERMINAL(vte), 65535);
	}
	else
#endif
		vte_terminal_set_background_transparent(VTE_TERMINAL(vte), win_data->transparent_background);

	// g_debug("set_background_saturation(): win_data->transparent_background = %d, value = %1.3f",
	//	win_data->transparent_background, value);
	// g_debug("set_background_saturation(): win_data->background_image = %s", win_data->background_image);
	if (win_data->transparent_background)
	{
		vte_terminal_set_background_image_file (VTE_TERMINAL(vte), NULL_DEVICE);
		vte_terminal_set_background_saturation( VTE_TERMINAL(vte), value);
	}
	else
	{
		if (compare_strings(win_data->background_image, NULL_DEVICE, TRUE))
		{
			vte_terminal_set_background_saturation( VTE_TERMINAL(vte), value);
			vte_terminal_set_background_image_file (VTE_TERMINAL(vte), win_data->background_image);
			vte_terminal_set_scroll_background(VTE_TERMINAL(vte), win_data->scroll_background);
		}
		else
			vte_terminal_set_background_saturation( VTE_TERMINAL(vte), 0);
	}

	vte_terminal_set_background_tint_color (VTE_TERMINAL(page_data->vte), &(win_data->bg_color));
	return FALSE;
}

#if defined(ENABLE_RGBA) || defined(UNIT_TEST)
gboolean set_window_opacity(GtkRange *range, GtkScrollType scroll, gdouble value, struct Window *win_data)
{
#ifdef DETAIL
	g_debug("! Launch set_window_opacity() with value = %f, win_data = %p", value, win_data);
#endif
#ifdef DEFENSIVE
	if (win_data==NULL) return FALSE;
#endif
	// g_debug("Get win_data = %d when set window opacity!", win_data);

	value = CLAMP(value, 0, 1);
	if (win_data->use_rgba == -1)
	{
#ifdef ENABLE_RGBA
		if (win_data->transparent_window)
			gtk_window_set_opacity (GTK_WINDOW(win_data->window), 1-value);
		else
			gtk_window_set_opacity (GTK_WINDOW(win_data->window), 1);
#endif
	}
	return FALSE;
}
#endif

// set the window hints information
void window_resizable(GtkWidget *window, GtkWidget *vte, gint set_hints_inc)
{
#ifdef DEFENSIVE
	if ((window==NULL) || (vte==NULL)) return;
#endif
#ifdef DETAIL
	g_debug("! Launch window_resizable() with window = %p, vte = %p, set_hints_inc = %d",
		window, vte, set_hints_inc);
#endif

	// DIRTY HACK: don't run window_resizable too much times before window is shown!
	if ((set_hints_inc != 1) && (gtk_widget_get_mapped(window) == FALSE)) return;

	// vte=NULL when creating a new root window with drag & drop.
	// if (vte==NULL) return;

	GdkGeometry hints = {0};
	// g_debug("Trying to get padding...");
	vte_terminal_get_padding (VTE_TERMINAL(vte), &(hints.base_width), &(hints.base_height));
	// g_debug("hints.base_width = %d, hints.base_height = %d", hints.base_width, hints.base_height);

	switch (set_hints_inc)
	{
		case 1:
			hints.width_inc = vte_terminal_get_char_width(VTE_TERMINAL(vte));
			hints.height_inc = vte_terminal_get_char_height(VTE_TERMINAL(vte));
			break;
		case 2:
			hints.width_inc = 1;
			hints.height_inc = 1;
			break;
	}

	// g_debug("hints.width_inc = %d, hints.height_inc = %d",
	//	hints.width_inc, hints.height_inc);

	// // minsize = -1: the size of vte can NOT be changed.
	// if (minsize == -1)
	// {
	//	hints.min_width = minsize;
	//	hints.min_height = minsize;
	// }
	// else
	// {
		hints.min_width = hints.base_width + hints.width_inc;
		hints.min_height = hints.base_height + hints.height_inc;
	// }

	// g_debug("Tring to set geometry on %p, and set_hints_inc = %d", vte, set_hints_inc);
	gtk_window_set_geometry_hints (GTK_WINDOW (window), GTK_WIDGET (vte), &hints,
					GDK_HINT_RESIZE_INC | GDK_HINT_MIN_SIZE | GDK_HINT_BASE_SIZE);

	//g_debug("current the size of vte %p whith hinting = %ld x %ld",
	//			vte,
	//			vte_terminal_get_column_count(VTE_TERMINAL(vte)),
	//			vte_terminal_get_row_count(VTE_TERMINAL(vte)));
}

#if defined(vte_terminal_get_padding) || defined(UNIT_TEST)
void fake_vte_terminal_get_padding(VteTerminal *vte, gint *width, gint *height)
{
#ifdef DETAIL
	g_debug("! Launch fake_vte_terminal_get_padding() with vte = %p", vte);
#endif
#ifdef DEFENSIVE
	if (vte==NULL) return;
#endif
	GtkBorder *inner_border = NULL;
	gtk_widget_style_get(GTK_WIDGET(vte), "inner-border", &inner_border, NULL);
#  ifdef DEFENSIVE
	if (inner_border)
	{
#  endif
#ifdef DEFENSIVE
		if (width)
#endif
			*width = inner_border->left + inner_border->right;
#ifdef DEFENSIVE
		if (height)
#endif
			*height = inner_border->top + inner_border->bottom;
#  ifdef DEFENSIVE
	}
#  endif
	gtk_border_free (inner_border);
}
#endif

void apply_new_win_data_to_page (struct Window *win_data_orig,
				 struct Window *win_data,
				 struct Page *page_data)
{
#ifdef DETAIL
	g_debug("! Launch apply_new_win_data_to_page() with win_data_orig = %p, win_data = %p, page_data = %p",
		win_data_orig, win_data, page_data);
#endif
#ifdef DEFENSIVE
	if ((win_data_orig==NULL) || (win_data==NULL) || (page_data==NULL) || (page_data->vte==NULL)) return;
#endif
	// if (win_data_orig->use_rgba != win_data->use_rgba)
	//	init_rgba(win_data);
#ifdef ENABLE_RGBA
    	set_window_opacity (NULL, 0, win_data->window_opacity, win_data);
#endif

	init_monitor_cmdline_datas(win_data, page_data);

	if (win_data_orig->enable_hyperlink != win_data->enable_hyperlink)
		set_hyprelink(win_data, page_data);

// ---- the color used in vte ---- //
	gboolean update_color = FALSE;

	if (compare_color(&(win_data_orig->fg_color), &(win_data->fg_color)) ||
	    compare_color(&(win_data_orig->bg_color), &(win_data->bg_color)) ||
	    (win_data_orig->using_custom_color != win_data->using_custom_color) ||
	    (win_data_orig->color_brightness != win_data->color_brightness))
	    	update_color = TRUE;

	gint i;
	if ( ! update_color)
	{
		for (i=0; i<COLOR; i++)
			if (compare_color(&(win_data_orig->color[i]), &(win_data->color[i])))
				update_color = TRUE;
	}
	if (update_color)
		set_vte_color(win_data, page_data);

// ---- tabs on notebook ---- //

	if (win_data_orig->tabs_bar_position != win_data->tabs_bar_position)
	{
		if (win_data->tabs_bar_position)
			gtk_notebook_set_tab_pos(GTK_NOTEBOOK(win_data->notebook), GTK_POS_BOTTOM);
		else
			gtk_notebook_set_tab_pos(GTK_NOTEBOOK(win_data->notebook), GTK_POS_TOP);
	}

	// the fill_tabs_bar may not always work, so we should call set_fill_tabs_bar() every time.
	// if (win_data_orig->fill_tabs_bar != win_data->fill_tabs_bar)
		set_fill_tabs_bar(GTK_NOTEBOOK(win_data->notebook), win_data->fill_tabs_bar, page_data);

	if (win_data_orig->page_width != win_data->page_width)
		set_page_width(win_data, page_data);

	// g_debug("ORI: %d", win_data_orig->page_shows_current_cmdline ||
	//		   win_data_orig->page_shows_current_dir ||
	//		   win_data_orig->page_shows_window_title);
	// g_debug("NEW: %d", win_data->page_shows_current_cmdline ||
	//		   win_data->page_shows_current_dir ||
	//		   win_data->page_shows_window_title);
	if ((proc_exist) &&
	    ((page_data->page_update_method != PAGE_METHOD_NORMAL) ||
	    (win_data->page_shows_current_cmdline ||
	     win_data->page_shows_current_dir ||
	     win_data->page_shows_window_title)))
	{
		// FIXME: Is it necessary?
		if (page_data->page_update_method == PAGE_METHOD_WINDOW_TITLE) page_data->window_title_updated = 1;
		page_data->page_update_method = PAGE_METHOD_REFRESH;
	}
	// g_debug("page_data->page_update_method = %d", page_data->page_update_method);

	if (win_data->page_shows_window_title != win_data_orig->page_shows_window_title)
		add_remove_window_title_changed_signal(page_data);

	if (win_data->check_root_privileges != win_data_orig->check_root_privileges)
	{
		if (win_data->check_root_privileges)
			page_data->is_root = check_is_root(page_data->displayed_tpgid);
		else
			page_data->is_root = FALSE;
		// g_debug("apply_new_win_data_to_page(): page_data->is_root = %d", page_data->is_root);
	}

	if (page_data->is_bold)
	{
		if (page_data->vte == win_data->current_vte)
		{
			if (win_data->bold_current_page_name == FALSE)
			{
				page_data->is_bold = page_data->should_be_bold;
				if (win_data->bold_action_page_name == FALSE)
					page_data->is_bold = FALSE;
			}
		}
		else
		{
			if (win_data->bold_action_page_name == FALSE)
				page_data->is_bold = FALSE;
		}
	}
	else
	{
		if (page_data->vte == win_data->current_vte)
			page_data->is_bold = win_data->bold_current_page_name;
		else if (win_data->bold_action_page_name == TRUE)
			page_data->is_bold = page_data->should_be_bold;
	}

	if ((win_data_orig->window_title_shows_current_page != win_data->window_title_shows_current_page) ||
	    (win_data_orig->window_title_append_package_name != win_data->window_title_append_package_name))
	{
		if (win_data->window_title_shows_current_page)
			check_and_update_window_title(win_data, page_data->custom_window_title,
						      page_data->page_no+1, page_data->custom_page_name,
						      page_data->page_name);
		else
			check_and_update_window_title(win_data, TRUE, page_data->page_no+1,
						      PACKAGE, NULL);
		// g_debug("FINAL: New window title = %s", gtk_window_get_title(GTK_WINDOW(win_data->window)));
	}

	get_and_update_page_name(page_data, FALSE);

	if ((win_data_orig->show_close_button_on_tab != win_data->show_close_button_on_tab) ||
	    (win_data_orig->show_close_button_on_all_tabs != win_data->show_close_button_on_all_tabs))
		show_close_button_on_tab(win_data, page_data);

// ---- font ---- //
	if (win_data_orig->font_anti_alias != win_data->font_anti_alias)
		vte_terminal_set_font_from_string_full (VTE_TERMINAL(page_data->vte),
							page_data->font_name,
							win_data->font_anti_alias);

// ---- other settings for init a vte ---- //

	if (compare_strings(win_data_orig->word_chars, win_data->word_chars, TRUE))
		vte_terminal_set_word_chars(VTE_TERMINAL(page_data->vte), win_data->word_chars);

	if (win_data_orig->show_scroll_bar != win_data->show_scroll_bar)
		// hide_scroll_bar(win_data, page_data);
		show_and_hide_scroll_bar(page_data, check_show_or_hide_scroll_bar(win_data));

	if (win_data_orig->scroll_bar_position != win_data->scroll_bar_position)
	{
		g_object_ref(page_data->vte);
		gtk_container_remove (GTK_CONTAINER(page_data->hbox), page_data->vte);
		g_object_ref(page_data->scroll_bar);
		gtk_container_remove (GTK_CONTAINER(page_data->hbox), page_data->scroll_bar);

		pack_vte_and_scroll_bar_to_hbox(win_data, page_data);

		g_object_unref(page_data->vte);
		g_object_unref(page_data->scroll_bar);
	}

	if (compare_color(&(win_data_orig->bg_color), &(win_data->bg_color)) ||
	    (win_data_orig->transparent_background != win_data->transparent_background) ||
	    (win_data_orig->background_saturation != win_data->background_saturation) ||
	    (win_data_orig->scroll_background != win_data->scroll_background) ||
	    compare_strings (win_data_orig->background_image, win_data->background_image, TRUE))
		set_background_saturation (NULL, 0, win_data->background_saturation, page_data->vte);

	if (win_data_orig->scrollback_lines != win_data->scrollback_lines)
		vte_terminal_set_scrollback_lines (VTE_TERMINAL(page_data->vte), win_data->scrollback_lines);

	if (win_data_orig->cursor_blinks != win_data->cursor_blinks)
		set_cursor_blink(win_data, page_data);

	if (win_data_orig->audible_bell != win_data->audible_bell)
		vte_terminal_set_audible_bell (VTE_TERMINAL(page_data->vte), win_data->audible_bell);

	if (win_data_orig->visible_bell != win_data->visible_bell)
		vte_terminal_set_visible_bell (VTE_TERMINAL(page_data->vte), win_data->visible_bell);

	if (win_data_orig->urgent_bell != win_data->urgent_bell)
		set_vte_urgent_bell(win_data, page_data);

	if (win_data_orig->erase_binding != win_data->erase_binding)
		vte_terminal_set_backspace_binding (VTE_TERMINAL(page_data->vte), win_data->erase_binding);
#ifdef ENABLE_CURSOR_SHAPE
	if (win_data_orig->cursor_shape != win_data->cursor_shape)
		vte_terminal_set_cursor_shape(VTE_TERMINAL(page_data->vte), win_data->cursor_shape);
#endif
	if (compare_strings(win_data_orig->emulate_term, win_data->emulate_term, TRUE))
		vte_terminal_set_emulation (VTE_TERMINAL(page_data->vte), win_data->emulate_term);
}

// Will return TRUE if a and b are NOT the same.
gboolean compare_color(GdkColor *a, GdkColor *b)
{
#ifdef DETAIL
	g_debug("! Launch compare_color()!");
#endif

#ifdef DEFENSIVE
	if ((a==NULL) && (b==NULL)) return FALSE;
	if ((a==NULL) || (b==NULL)) return TRUE;
#endif
	if ((a->pixel != b->pixel) ||
	    (a->red != b->red) ||
	    (a->green != b->green ) ||
	    (a->blue != b->blue))
		return TRUE;
	else
		return FALSE;
}

void set_widget_thickness(GtkWidget *widget, gint thickness)
{
#ifdef DETAIL
	g_debug("! Launch set_widget_thickness() with widget = %p, thickness = %d!", widget, thickness);
#endif

#ifdef DEFENSIVE
	if (widget==NULL) return;
#endif
	GtkRcStyle *rc_style = gtk_rc_style_new();
#ifdef DEFENSIVE
	if (rc_style)
	{
#endif
		rc_style->xthickness = rc_style->ythickness = thickness;
		gtk_widget_modify_style(widget, rc_style);
#ifdef DEFENSIVE
	}
#endif
	g_object_unref(rc_style);
}
