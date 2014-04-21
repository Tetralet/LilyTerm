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
// for aoti()
#include <stdlib.h>
// for strlen()
#include <string.h>
// for usleep()
#include <unistd.h>
// for stat()
#include <sys/stat.h>
#include "lilyterm.h"

gboolean check_cmdline (struct Page *page_data, pid_t check_tpgid);
gboolean check_window_title (struct Page *page_data, gboolean lost_focus);
gboolean check_pwd(struct Page *page_data, gchar *pwd, gchar *new_pwd, gint page_update_method);
gchar *get_tab_name_with_page_names(struct Window *win_data);
gchar *get_tab_name_with_cmdline(struct Page *page_data);
