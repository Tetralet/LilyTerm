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
// for L10n
#include <locale.h>
#include <glib/gi18n.h>
// for exit()
#include <stdlib.h>
// for va_start(), va_arg(), va_end()
#include <stdarg.h>
// for strcmp()
#include <string.h>

#include "lilyterm.h"

gchar *get_help_message(gchar *profile);
