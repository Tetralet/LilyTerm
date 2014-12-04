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

#define _GNU_SOURCE

#include <gtk/gtk.h>
#include <vte/vte.h>
// for L10n
#include <locale.h>
#include <glib/gi18n.h>
// for aoti() aotf() canonicalize_file_name()
#include <stdlib.h>
// for access()
#include <unistd.h>
// for strcmp()
#include <string.h>
// for XParseGeometry()
#include <X11/Xlib.h>

#include "lilyterm.h"

#define DEFAULT_FOREGROUND_COLOR "white"
#define DEFAULT_BACKGROUND_COLOR "black"
#define DEFAULT_CURSOR_COLOR "#44738B"

#ifdef BSD
// chars in a path name including nul
// which is already defined in /usr/include/sys/syslimits.h
// #define PATH_MAX 4096
#endif

typedef enum {
	DISABLE_EMPTY_STR,
	ENABLE_EMPTY_STR,
} Check_Empty;

typedef enum {
	DISABLE_ZERO,
	ENABLE_ZERO,
} Check_Zero;

typedef enum {
	NO_CHECK_MIN,
	CHECK_MIN,
} Check_Min;

typedef enum {
	NO_CHECK_MAX,
	CHECK_MAX,
} Check_Max;

void init_command();
void init_user_command(struct Window *win_data);
void init_window_parameters(struct Window *win_data);
void init_user_keys(struct Window *win_data);
void init_key_bindings_name_and_group();
void init_key_bindings();
void init_page_color_data();
void init_page_color_data_comment();
void init_mod_keys();
void init_colors();
gchar *load_profile_from_dir(const gchar *dir, const gchar* profile);
gdouble check_double_value(GKeyFile *keyfile, const gchar *group_name, const gchar *key, const gdouble default_value,
			   Check_Empty enable_empty, gdouble empty_value,
			   Check_Min check_min, gdouble min,
			   Check_Max check_max, gdouble max);
glong check_integer_value(GKeyFile *keyfile, const gchar *group_name, const gchar *key,
			  const glong default_value, Check_Empty enable_empty, glong empty_value,
			  Check_Zero enable_zero, Check_Min check_min, glong min,
			  Check_Max check_max, glong max);
gchar *check_string_value(GKeyFile *keyfile, const gchar *group_name,
			  const gchar *key, gchar *original_value, gboolean free_original_value, Check_Empty enable_empty);
gboolean check_color_value (const gchar *key_name, const gchar *color_name, GdkRGBA *color, const GdkRGBA *default_color);
gboolean accelerator_parse(const gchar *key_name, const gchar *key_value, guint *key, guint *mods);
void create_save_failed_dialog(struct Window *win_data, gchar *message);
