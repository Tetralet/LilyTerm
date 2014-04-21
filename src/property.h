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
// for strlen()
#include <string.h>
// for aoti() aotf() canonicalize_file_name()
#include <stdlib.h>
// for access()
#include <unistd.h>

#include "lilyterm.h"
#if defined(USE_GTK3_GEOMETRY_METHOD) || defined(UNIT_TEST)
void get_hint_min_size(GtkWidget *notebook, GtkWidget *scrollbar, gint *min_width, gint *min_height);
#endif
