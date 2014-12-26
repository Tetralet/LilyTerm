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

#include "property.h"

extern gboolean proc_exist;
extern struct Command command[COMMAND];
extern struct GdkRGBA_Theme system_color_theme[THEME];

void create_theme_color_data(GdkRGBA color[COLOR], GdkRGBA color_orig[COLOR], gdouble color_brightness, gboolean invert_color,
			     gboolean default_vte_theme, gboolean dim_fg_color)
{
#ifdef DETAIL
	g_debug("! Launch create_theme_color_data() with color = %p, color_orig = %p, color_brightness = %3f, invert_color = %d",
		color, color_orig, color_brightness, invert_color);
#endif
#ifdef SAFEMODE
	if ((color==NULL) || (color_orig==NULL)) return;
#endif
	if (! default_vte_theme)
	{
		// g_debug("Get win_data = %d when set background saturation!", win_data);
		color_brightness = CLAMP(color_brightness, -1, 1);

		gint i;
		for (i=1; i<COLOR-1; i++)
		{
			// g_debug("adjuset the color of %d", get_color_index(invert_color, i));
			adjust_ansi_color(&color[i], &color_orig[get_color_index(invert_color, i)], color_brightness);
			// print_color(get_color_index(invert_color, i), "create_theme_color_data(): color_orig ", color_orig[i]);
			// print_color(i, "create_theme_color_data(): new: color ", color[i]);
		}
	}

	// The fg_color and bg_color will not affect by color_brightness
	if (dim_fg_color)
		adjust_ansi_color(&color[COLOR-1], &color_orig[get_color_index(invert_color, COLOR-1)], color_brightness);
	else
		color[COLOR-1] = color_orig[get_color_index(invert_color, COLOR-1)];
	// print_color(get_color_index(invert_color, COLOR-1), "create_theme_color_data(): fg color_orig ", color_orig[COLOR-1]);
	// print_color(COLOR-1, "create_theme_color_data(): new: fg color ", color[COLOR-1]);
	color[0] = color_orig[get_color_index(invert_color, 0)];
	// print_color(get_color_index(invert_color, 0), "create_theme_color_data(): bg color_orig ", color_orig[0]);
	// print_color(0, "create_theme_color_data(): new: bg color ", color[0]);
}

void adjust_ansi_color(GdkRGBA *color, GdkRGBA *color_orig, gdouble color_brightness)
{
#ifdef DETAIL
	g_debug("! Launch adjust_ansi_color() with color = %p, color_orig = %p, color_brightness = %3f",
		color, color_orig, color_brightness);
#endif
#ifdef SAFEMODE
	if ((color==NULL) || (color_orig==NULL)) return;
#endif
	if (color_brightness>=0)
	{
		color->red = (MAX_COLOR - color_orig->red) * color_brightness + color_orig->red;
		color->green = (MAX_COLOR - color_orig->green) * color_brightness + color_orig->green;
		color->blue = (MAX_COLOR - color_orig->blue) * color_brightness + color_orig->blue;
	}
	else
	{
		color->red = (1 + color_brightness) * color_orig->red;
		color->green = (1 + color_brightness) * color_orig->green;
		color->blue = (1 + color_brightness) * color_orig->blue;
	}
}

void generate_all_color_datas(struct Window *win_data)
{
#ifdef DETAIL
	g_debug("! Launch generate_all_color_datas() with win_data = %p", win_data);
#endif
#ifdef SAFEMODE
	if (win_data==NULL) return;
#endif
	GdkRGBA *temp_color = get_current_color_theme(win_data);

	gboolean default_vte_theme = use_default_vte_theme(win_data);
	create_theme_color_data(win_data->color, temp_color, win_data->color_brightness, win_data->invert_color, default_vte_theme, FALSE);
	create_theme_color_data(win_data->color_inactive, temp_color, win_data->color_brightness_inactive, win_data->invert_color, default_vte_theme, TRUE);
}

GdkRGBA *get_current_color_theme(struct Window *win_data)
{
#ifdef DETAIL
	g_debug("! Launch current_color_theme() with win_data = %p", win_data);
#endif
#ifdef SAFEMODE
	if (win_data==NULL) return NULL;
#endif
	// g_debug("win_data->use_custom_theme = %d", win_data->use_custom_theme);
	if (win_data->use_custom_theme)
		return win_data->custom_color_theme[win_data->color_theme_index].color;
	else
		return system_color_theme[win_data->color_theme_index].color;
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
#ifdef SAFEMODE
	if ((win_data==NULL) || (page_data==NULL) || (page_data->vte==NULL)) return;
#endif
	// g_debug("Get win_data = %d when initing new page!", win_data);

#ifdef SAFEMODE
	if (page_data->font_name)
	{
#endif
		// set font
		// g_debug("Set Font AA = %d", win_data->font_anti_alias);
		fake_vte_terminal_set_font_from_string (page_data->vte,
							page_data->font_name,
							win_data->font_anti_alias);
#ifdef SAFEMODE
	}
#endif
	//g_debug("Got font size from %s: %d", font_name, pango_font_description_get_size (
	//	  pango_font_description_from_string(font_name))/PANGO_SCALE);

	// set terminal size
	// g_debug("Set the vte size to: %dx%d", column, row);
#ifdef SAFEMODE
	if (page_data->vte)
#endif
		vte_terminal_set_size(VTE_TERMINAL(page_data->vte), column, row);

#ifdef GEOMETRY
#  ifdef USE_GTK2_GEOMETRY_METHOD
	g_debug("@ init_new_page(for %p, vte = %p): Set win_data->keep_vte_size to %d, and column = %ld, row = %ld",
		win_data->window, page_data->vte, win_data->keep_vte_size, column, row);
#  else
	g_debug("@ init_new_page(for %p, vte = %p): Set column = %ld, row = %ld",
		win_data->window, page_data->vte, column, row);
#  endif
#endif

	set_vte_color(page_data->vte, use_default_vte_theme(win_data), win_data->custom_cursor_color, win_data->cursor_color,
		      win_data->color, FALSE, (win_data->color_theme_index==(THEME-1)));
#if defined(ENABLE_VTE_BACKGROUND) || defined(FORCE_ENABLE_VTE_BACKGROUND)
	// set transparent
	set_background_saturation(NULL, 0, win_data->background_saturation, page_data->vte);
#endif
	// other settings
#ifdef ENABLE_SET_WORD_CHARS
	vte_terminal_set_word_chars(VTE_TERMINAL(page_data->vte), win_data->word_chars);
#endif
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

	set_hyperlink(win_data, page_data);
	set_cursor_blink(win_data, page_data);

	vte_terminal_set_allow_bold(VTE_TERMINAL(page_data->vte), win_data->allow_bold_text);

	vte_terminal_set_audible_bell (VTE_TERMINAL(page_data->vte), win_data->audible_bell);
#ifdef ENABLE_VISIBLE_BELL
	vte_terminal_set_visible_bell (VTE_TERMINAL(page_data->vte), win_data->visible_bell);
#endif
	// g_debug("init_new_page(): call set_vte_urgent_bell()");
#ifdef ENABLE_BEEP_SINGAL
	set_vte_urgent_bell(win_data, page_data);
#endif
	vte_terminal_set_backspace_binding (VTE_TERMINAL(page_data->vte), win_data->erase_binding);
#ifdef ENABLE_CURSOR_SHAPE
	vte_terminal_set_cursor_shape(VTE_TERMINAL(page_data->vte), win_data->cursor_shape);
#endif
#ifdef ENABLE_SET_EMULATION
	vte_terminal_set_emulation (VTE_TERMINAL(page_data->vte), win_data->emulate_term);
#endif
}


void set_cursor_blink(struct Window *win_data, struct Page *page_data)
{
#ifdef DETAIL
	g_debug("! Launch set_cursor_blink() with win_data = %p, page_data = %p", win_data, page_data);
#endif
#ifdef SAFEMODE
	if ((win_data==NULL) || (page_data==NULL) || (page_data->vte==NULL)) return;
#endif
	// g_debug("set_cursor_blink(): win_data->cursor_blinks = %d", win_data->cursor_blinks);


#ifdef USE_NEW_VTE_CURSOR_BLINKS_MODE
	vte_terminal_set_cursor_blink_mode (VTE_TERMINAL(page_data->vte), win_data->cursor_blinks);
#else
	vte_terminal_set_cursor_blinks (VTE_TERMINAL(page_data->vte), win_data->cursor_blinks);
#endif
}

void set_hyperlink(struct Window *win_data, struct Page *page_data)
{
#ifdef DETAIL
	g_debug("! Launch set_hyperlink() with win_data = %p, page_data = %p", win_data, page_data);
#endif
#ifdef SAFEMODE
	if ((win_data==NULL) || (page_data==NULL) || (page_data->vte==NULL)) return;
#endif
	if (win_data->enable_hyperlink && win_data->enable_key_binding)
	{
		if (page_data->match_regex_setted) clean_hyperlink(win_data, page_data);

		gint i;
		for (i=0; i<COMMAND; i++)
		{
			gchar *match = (win_data->user_command[i].match_regex_orig)? win_data->user_command[i].match_regex_orig: win_data->user_command[i].match_regex;
			if ((match == NULL) || (match[0] == '\0'))
				match = command[i].match;

			// gchar *regex_str = convert_escape_sequence_to_string(match);
			// g_debug("set_hyperlink(): match = %s", regex_str);
			// g_free(regex_str);

#ifdef USE_NEW_VTE_MATCH_ADD_GREGEX
			GRegex *regex = g_regex_new (match, G_REGEX_CASELESS | G_REGEX_OPTIMIZE,
						     0, NULL);
			page_data->tag[i] = vte_terminal_match_add_gregex (VTE_TERMINAL(page_data->vte),
									   regex, 0);
			g_regex_unref (regex);
#else
			page_data->tag[i] = vte_terminal_match_add (VTE_TERMINAL(page_data->vte), match);
#endif
			vte_terminal_match_set_cursor_type(VTE_TERMINAL(page_data->vte),
							   page_data->tag[i],
							   GDK_HAND2);
		}
		page_data->match_regex_setted = TRUE;
		// g_debug("clean_hyperlink(): set page_data->match_regex_setted = %d", page_data->match_regex_setted);
	}
	else
		clean_hyperlink(win_data, page_data);
}

void clean_hyperlink(struct Window *win_data, struct Page *page_data)
{
#ifdef DETAIL
	g_debug("! Launch clean_hyperlink() with win_data = %p, page_data = %p", win_data, page_data);
#endif
#ifdef SAFEMODE
	if ((win_data==NULL) || (page_data==NULL) || (page_data->vte==NULL)) return;
#endif
	if (! page_data->match_regex_setted) return;

	page_data->match_regex_setted = FALSE;
	// g_debug("clean_hyperlink(): set page_data->match_regex_setted = %d", page_data->match_regex_setted);
	vte_terminal_match_remove_all(VTE_TERMINAL(page_data->vte));
}

void enable_custom_cursor_color(GtkWidget *vte, gboolean custom_cursor_color, GdkRGBA *cursor_color)
{
#ifdef DETAIL
	g_debug("! Launch enable_custom_cursor_color() with vte = %p, custom_cursor_color = %d, cursor_color = %p",
		vte, custom_cursor_color, cursor_color);
#endif
#ifdef SAFEMODE
	if (vte==NULL) return;
#endif
	if (custom_cursor_color)
		vte_terminal_set_color_cursor_rgba(VTE_TERMINAL(vte), cursor_color);
	else
		vte_terminal_set_color_cursor_rgba(VTE_TERMINAL(vte), NULL);
}

void set_vte_color(GtkWidget *vte, gboolean default_vte_color, gboolean custom_cursor_color, GdkRGBA cursor_color, GdkRGBA color[COLOR],
		   gboolean update_fg_only, gboolean over_16_colors)
{
#ifdef DETAIL
	g_debug("! Launch set_vte_color() with vte = %p, default_vte_color_theme = %d,  color = %p",
		vte, default_vte_color, color);
#endif
#ifdef SAFEMODE
	if ((vte==NULL) || (color ==NULL)) return;
#endif
	if (over_16_colors)
	{
		vte_terminal_set_default_colors(VTE_TERMINAL(vte));
		return;
	}

	// set font/background colors

	// gint i;
	// for (i=0; i< COLOR; i++)
	//	print_color(i, "set_vte_color():", color[i]);

	if (! update_fg_only)
	{
		vte_terminal_set_default_colors(VTE_TERMINAL(vte));

		if (default_vte_color)
			vte_terminal_set_color_background_rgba(VTE_TERMINAL(vte), &(color[0]));
		else
			vte_terminal_set_colors_rgba(VTE_TERMINAL(vte), &(color[COLOR-1]), &(color[0]), color, 16);
#if defined(ENABLE_VTE_BACKGROUND) || defined(FORCE_ENABLE_VTE_BACKGROUND)
		dirty_vte_terminal_set_background_tint_color(VTE_TERMINAL(vte), color[0]);
#endif
	}

	if (default_vte_color | update_fg_only)
		vte_terminal_set_color_foreground_rgba(VTE_TERMINAL(vte), &(color[COLOR-1]));

	vte_terminal_set_colors_rgba(VTE_TERMINAL(vte), &(color[COLOR-1]), &(color[0]), color, 16);

	// print_color(-1, "set_vte_color(): cursor_color", cursor_color);
	enable_custom_cursor_color(vte, custom_cursor_color, &(cursor_color));
}

gboolean use_default_vte_theme(struct Window *win_data)
{
#ifdef DETAIL
	g_debug("! Launch use_default_vte_theme() with win_data = %p", win_data);
#endif
#ifdef SAFEMODE
	if (win_data==NULL) return TRUE;
#endif
	return ! (win_data->color_theme_index || win_data->invert_color || win_data->color_brightness || win_data->color_brightness_inactive);
}

void set_page_width(struct Window *win_data, struct Page *page_data)
{
#ifdef DETAIL
	g_debug("! Launch set_page_width() with win_data = %p, page_data = %p", win_data, page_data);
#endif
#ifdef SAFEMODE
	if ((win_data==NULL) || (page_data==NULL) || (page_data->label_text==NULL)) return;
#endif
	gtk_label_set_width_chars(GTK_LABEL(page_data->label_text), win_data->page_width);
}

void pack_vte_and_scroll_bar_to_hbox(struct Window *win_data, struct Page *page_data)
{
#ifdef DETAIL
	g_debug("! Launch pack_vte_and_scroll_bar_to_hbox() with win_data = %p, page_data = %p", win_data, page_data);
#endif
#ifdef SAFEMODE
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
#ifdef SAFEMODE
	if (win_data==NULL) return FALSE;
#endif
	gboolean show = FALSE;
	// g_debug("check_show_or_hide_scroll_bar(): win_data->show_scroll_bar = %d", win_data->show_scroll_bar);
	switch (win_data->show_scroll_bar)
	{
		case AUTOMATIC:
#ifdef USE_GTK2_GEOMETRY_METHOD
			show = ! win_data->true_fullscreen;
#endif
#ifdef USE_GTK3_GEOMETRY_METHOD
			switch (win_data->window_status)
			{
				case WINDOW_NORMAL:
				case WINDOW_RESIZING_TO_NORMAL:
				case WINDOW_MAX_WINDOW:
				case WINDOW_APPLY_PROFILE_NORMAL:
					show = TRUE;
					break;
				default:
					break;
			}
#endif
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
#ifdef SAFEMODE
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
#ifdef SAFEMODE
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
#ifdef SAFEMODE
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

#if defined(ENABLE_VTE_BACKGROUND) || defined(FORCE_ENABLE_VTE_BACKGROUND) || defined(UNIT_TEST)
gboolean set_background_saturation(GtkRange *range, GtkScrollType scroll, gdouble value, GtkWidget *vte)
{
#  ifdef DETAIL
	g_debug("! Launch set_background_saturation() with value = %f, vte = %p", value, vte);
#  endif
#  ifdef SAFEMODE
	if (vte==NULL) return FALSE;
#  endif
	struct Page *page_data = (struct Page *)g_object_get_data(G_OBJECT(vte), "Page_Data");
#  ifdef SAFEMODE
	if (page_data==NULL || (page_data->window==NULL)) return FALSE;
#  endif
	struct Window *win_data = (struct Window *)g_object_get_data(G_OBJECT(page_data->window), "Win_Data");
#  ifdef SAFEMODE
	if (win_data==NULL) return FALSE;
#  endif
	// g_debug("Get win_data = %d when set background saturation!", win_data);

	value = CLAMP(value, 0, 1);

#  ifdef ENABLE_RGBA
#    ifdef FORCE_ENABLE_VTE_BACKGROUND
	G_GNUC_BEGIN_IGNORE_DEPRECATIONS;
#    endif
	if (win_data->use_rgba == -1)
	{
		if (win_data->transparent_background)
			vte_terminal_set_opacity(VTE_TERMINAL(vte), (1-value) * 65535);
		else
			vte_terminal_set_opacity(VTE_TERMINAL(vte), 65535);
	}
	else
#  endif
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
#    ifdef FORCE_ENABLE_VTE_BACKGROUND
	G_GNUC_END_IGNORE_DEPRECATIONS;
#    endif

	dirty_vte_terminal_set_background_tint_color(VTE_TERMINAL(page_data->vte), win_data->color[0]);
	return FALSE;
}
#endif

#if defined(ENABLE_RGBA) || defined(UNIT_TEST)
gboolean set_window_opacity(GtkRange *range, GtkScrollType scroll, gdouble value, struct Window *win_data)
{
#ifdef DETAIL
	g_debug("! Launch set_window_opacity() with value = %f, win_data = %p", value, win_data);
#endif
#ifdef SAFEMODE
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
void window_resizable(GtkWidget *window, GtkWidget *vte, Hints_Type hints_type)
{
#ifdef DETAIL
	fprintf(stderr, "\033[1;31m** Launch window_resizable() with window = %p, vte = %p, hints_type = %d\033[0m\n",
		window, vte, hints_type);
#endif
#ifdef SAFEMODE
	if ((window==NULL) || (vte==NULL)) return;
#endif

	// DIRTY HACK: don't run window_resizable too much times before window is shown!
	if ((hints_type != HINTS_FONT_BASE) && (gtk_widget_get_mapped(window) == FALSE)) return;

	// vte=NULL when creating a new root window with drag & drop.
	// if (vte==NULL) return;

	GdkGeometry hints = {0};
	// g_debug("Trying to get padding...");
	vte_terminal_get_padding (VTE_TERMINAL(vte), &(hints.base_width), &(hints.base_height));
	// g_debug("hints.base_width = %d, hints.base_height = %d", hints.base_width, hints.base_height);

	switch (hints_type)
	{
		case HINTS_FONT_BASE:
			hints.width_inc = vte_terminal_get_char_width(VTE_TERMINAL(vte));
			hints.height_inc = vte_terminal_get_char_height(VTE_TERMINAL(vte));
			break;
		case HINTS_NONE:
			hints.width_inc = 1;
			hints.height_inc = 1;
			break;
#if defined(USE_GTK3_GEOMETRY_METHOD) || defined(UNIT_TEST)
		case HINTS_SKIP_ONCE:
			return;
#endif
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
#if defined(USE_GTK3_GEOMETRY_METHOD) || defined(UNIT_TEST)
		gint min_width = 0, min_height = 0;
		struct Window *win_data = (struct Window *)g_object_get_data(G_OBJECT(window), "Win_Data");
		struct Page *page_data = (struct Page *)g_object_get_data(G_OBJECT(vte), "Page_Data");
		get_hint_min_size(win_data->notebook, page_data->scroll_bar, &min_width, &min_height);
		hints.min_width = hints.base_width + ((int)(min_width/hints.width_inc)+1)*hints.width_inc;
		hints.min_height = hints.base_height + ((int)(min_height/hints.height_inc)+1)*hints.height_inc;
#  ifdef GEOMETRY
		fprintf(stderr, "\033[1;37m** window_resizable(win_data %p): window = %p, vte = %p, hints_type = %d\033[0m\n",
			win_data, window, vte, hints_type);
#  endif
#endif
#ifdef USE_GTK2_GEOMETRY_METHOD
		hints.min_width = hints.base_width + hints.width_inc;
		hints.min_height = hints.base_height + hints.height_inc;
#endif
	// }
#ifdef GEOMETRY
	g_debug("@ hint data: hints.width_inc = %d, hints.height_inc = %d, hints.base_width = %d, "
		"@ hints.base_height = %d, hints.min_width = %d, hints.min_height = %d",
		hints.width_inc, hints.height_inc, hints.base_width, hints.base_height, hints.min_width, hints.min_height);
#endif

	// g_debug("Tring to set geometry on %p, and hints_type = %d", vte, hints_type);
	gtk_window_set_geometry_hints (GTK_WINDOW (window), GTK_WIDGET (vte), &hints,
				       GDK_HINT_RESIZE_INC | GDK_HINT_MIN_SIZE | GDK_HINT_BASE_SIZE);

	//g_debug("current the size of vte %p whith hinting = %ld x %ld",
	//			vte,
	//			vte_terminal_get_column_count(VTE_TERMINAL(vte)),
	//			vte_terminal_get_row_count(VTE_TERMINAL(vte)));
}

#if defined(USE_GTK3_GEOMETRY_METHOD) || defined(UNIT_TEST)
void get_hint_min_size(GtkWidget *notebook, GtkWidget *scrollbar, gint *min_width, gint *min_height)
{
#ifdef DETAIL
	g_debug("! Launch get_hint_min_size() with notebook = %p, scrollbar = %p", notebook, scrollbar);
#endif
#ifdef SAFEMODE
	if ((scrollbar==NULL) || (scrollbar==NULL) || (min_width==NULL) || (min_height==NULL)) return;
#endif
#ifdef USE_GTK3_GEOMETRY_METHOD
	gtk_widget_get_preferred_width(GTK_WIDGET(notebook), min_width, NULL);
	gtk_widget_get_preferred_height(GTK_WIDGET(notebook), min_height, NULL);
#  ifdef GEOMETRY
	g_debug("@ get_hint_min_size(): Get the preferred size of notebook = %d x %d", *min_width, *min_height);
#  endif
#endif
	gint stepper_size, stepper_spacing, trough_border, min_slider_length;
	gtk_widget_style_get(GTK_WIDGET(scrollbar), "stepper-size", &stepper_size, NULL);
	gtk_widget_style_get(GTK_WIDGET(scrollbar), "stepper-spacing", &stepper_spacing, NULL);
	gtk_widget_style_get(GTK_WIDGET(scrollbar), "trough-border", &trough_border, NULL);
	gtk_widget_style_get(GTK_WIDGET(scrollbar), "min-slider-length", &min_slider_length, NULL);
	*min_height = *min_height + stepper_size*2 + stepper_spacing*2 + trough_border*2 + min_slider_length;

#  ifdef GEOMETRY
	g_debug("@ get_hint_min_size(): stepper_size = %d, stepper_spacing = %d, trough_border = %d, min_slider_length = %d",
		 stepper_size, stepper_spacing, trough_border, min_slider_length);
	g_debug("@ get_hint_min_size(): ** FINAL: min_width = %d, min_height = %d", *min_width, *min_height);
#  endif
}
#endif

#if defined(vte_terminal_get_padding) || defined(UNIT_TEST)
// This function is for replce the removed vte_terminal_get_padding()
void fake_vte_terminal_get_padding(VteTerminal *vte, gint *width, gint *height)
{
#ifdef DETAIL
	g_debug("! Launch fake_vte_terminal_get_padding() with vte = %p", vte);
#endif
#ifdef SAFEMODE
	if ((vte==NULL) || (width==NULL) || (height==NULL)) return;
#endif
#  ifdef VTE_HAS_INNER_BORDER
	GtkBorder *inner_border = NULL;
	gtk_widget_style_get(GTK_WIDGET(vte), "inner-border", &inner_border, NULL);
#    ifdef SAFEMODE
	if (inner_border)
	{
#    endif
#  ifdef SAFEMODE
		if (width)
#  endif
			*width = inner_border->left + inner_border->right;
#  ifdef SAFEMODE
		if (height)
#  endif
			*height = inner_border->top + inner_border->bottom;
#    ifdef SAFEMODE
	}
#    endif
	gtk_border_free (inner_border);
#  else
	*width = 0;
	*height = 0;
#  endif
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
#ifdef SAFEMODE
	if ((win_data_orig==NULL) || (win_data==NULL) || (page_data==NULL) || (page_data->vte==NULL)) return;
#endif
	// if (win_data_orig->use_rgba != win_data->use_rgba)
	//	init_rgba(win_data);
#ifdef ENABLE_RGBA
    	set_window_opacity (NULL, 0, win_data->window_opacity, win_data);
#endif

	init_monitor_cmdline_datas(win_data, page_data);

	if (win_data_orig->enable_hyperlink != win_data->enable_hyperlink)
		set_hyperlink(win_data, page_data);

// ---- the color used in vte ---- //
	gboolean update_color = FALSE;

	if ((win_data_orig->custom_cursor_color != win_data->custom_cursor_color) ||
	    (compare_color(&(win_data_orig->cursor_color), &(win_data->cursor_color))) ||
	    (win_data_orig->have_custom_color != win_data->have_custom_color) ||
	    (win_data_orig->use_custom_theme != win_data->use_custom_theme) ||
	    (win_data_orig->color_brightness != win_data->color_brightness))
		update_color = TRUE;

	gint i;
	if (! update_color && (win_data->use_custom_theme))
	{
		for (i=0; i<COLOR; i++)
			if (compare_color(&(win_data_orig->color[i]), &(win_data->color[i])))
				update_color = TRUE;
	}
	if (update_color)
		set_vte_color(page_data->vte, use_default_vte_theme(win_data), win_data->custom_cursor_color,
			      win_data->cursor_color, win_data->color, FALSE, (win_data->color_theme_index==(THEME-1)));

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
		check_and_update_window_title(win_data, page_data->custom_window_title,
					      page_data->page_no+1, page_data->custom_page_name,
					      page_data->page_name);
		// g_debug("FINAL: New window title = %s", gtk_window_get_title(GTK_WINDOW(win_data->window)));
	}

	get_and_update_page_name(page_data, FALSE);

	if ((win_data_orig->show_close_button_on_tab != win_data->show_close_button_on_tab) ||
	    (win_data_orig->show_close_button_on_all_tabs != win_data->show_close_button_on_all_tabs))
		show_close_button_on_tab(win_data, page_data);

// ---- font ---- //
	if (win_data_orig->font_anti_alias != win_data->font_anti_alias)
		fake_vte_terminal_set_font_from_string (page_data->vte,
							page_data->font_name,
							win_data->font_anti_alias);

// ---- other settings for init a vte ---- //
#ifdef ENABLE_SET_WORD_CHARS
	if (compare_strings(win_data_orig->word_chars, win_data->word_chars, TRUE))
		vte_terminal_set_word_chars(VTE_TERMINAL(page_data->vte), win_data->word_chars);
#endif
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
#if defined(ENABLE_VTE_BACKGROUND) || defined(FORCE_ENABLE_VTE_BACKGROUND)
	if (compare_color(&(win_data_orig->color[0]), &(win_data->color[0])) ||
	    (win_data_orig->transparent_background != win_data->transparent_background) ||
	    (win_data_orig->background_saturation != win_data->background_saturation) ||
	    (win_data_orig->scroll_background != win_data->scroll_background) ||
	    compare_strings (win_data_orig->background_image, win_data->background_image, TRUE))
		set_background_saturation (NULL, 0, win_data->background_saturation, page_data->vte);
#endif
	if (win_data_orig->scrollback_lines != win_data->scrollback_lines)
		vte_terminal_set_scrollback_lines (VTE_TERMINAL(page_data->vte), win_data->scrollback_lines);

	if (win_data_orig->cursor_blinks != win_data->cursor_blinks)
		set_cursor_blink(win_data, page_data);

	if (win_data_orig->allow_bold_text != win_data->allow_bold_text)
		vte_terminal_set_allow_bold(VTE_TERMINAL(page_data->vte), win_data->allow_bold_text);

	if (win_data_orig->audible_bell != win_data->audible_bell)
		vte_terminal_set_audible_bell (VTE_TERMINAL(page_data->vte), win_data->audible_bell);
#ifdef ENABLE_VISIBLE_BELL
	if (win_data_orig->visible_bell != win_data->visible_bell)
		vte_terminal_set_visible_bell (VTE_TERMINAL(page_data->vte), win_data->visible_bell);
#endif
#ifdef ENABLE_BEEP_SINGAL
	if (win_data_orig->urgent_bell != win_data->urgent_bell)
		set_vte_urgent_bell(win_data, page_data);
#endif
	if (win_data_orig->erase_binding != win_data->erase_binding)
		vte_terminal_set_backspace_binding (VTE_TERMINAL(page_data->vte), win_data->erase_binding);
#ifdef ENABLE_CURSOR_SHAPE
	if (win_data_orig->cursor_shape != win_data->cursor_shape)
		vte_terminal_set_cursor_shape(VTE_TERMINAL(page_data->vte), win_data->cursor_shape);
#endif
#ifdef ENABLE_SET_EMULATION
	if (compare_strings(win_data_orig->emulate_term, win_data->emulate_term, TRUE))
		vte_terminal_set_emulation (VTE_TERMINAL(page_data->vte), win_data->emulate_term);
#endif
}

// Will return TRUE if a and b are NOT the same.
gboolean compare_color(GdkRGBA *a, GdkRGBA *b)
{
#ifdef DETAIL
	g_debug("! Launch compare_color()!");
#endif

#ifdef SAFEMODE
	if ((a==NULL) && (b==NULL)) return FALSE;
	if ((a==NULL) || (b==NULL)) return TRUE;
#endif
	// g_debug("compare_color(): Comparing %04X %04X %04X %04X and %04X %04X %04X %04X",
	//	a->pixel, a->red, a->green, a->blue, b->pixel, b->red, b->green, b->blue);
	// g_debug("compare_color(): Comparing %0.4f %0.4f %0.4f and %0.4f %0.4f %0.4f",
	//	a->red, a->green, a->blue, b->red, b->green, b->blue);
	if ((a->red != b->red) ||
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

#ifdef SAFEMODE
	if (widget==NULL) return;
#endif

#ifdef USING_OLD_GTK_RC_STYLE_NEW

	GtkRcStyle *rc_style = gtk_rc_style_new();
#  ifdef SAFEMODE
	if (rc_style)
	{
#  endif
		rc_style->xthickness = rc_style->ythickness = thickness;
		gtk_widget_modify_style(widget, rc_style);
#  ifdef SAFEMODE
	}
#  endif
	g_object_unref(rc_style);
#else
	GtkCssProvider *css = gtk_css_provider_new ();
	GtkStyleContext *context = gtk_widget_get_style_context(widget);
	gchar *modified_style = g_strdup_printf("* {\n"
						"   -GtkWidget-focus-line-width: 0;\n"
						"   -GtkWidget-focus-padding: 0;\n"
						"   padding: %dpx;\n"
						"}",
						thickness);
	if (gtk_css_provider_load_from_data (css, modified_style, -1, NULL))
		gtk_style_context_add_provider (context, GTK_STYLE_PROVIDER (css),
						GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
	g_object_unref (css);
	g_free (modified_style);
#endif
}
