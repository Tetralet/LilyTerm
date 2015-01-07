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

#include "misc.h"
extern gboolean proc_exist;
extern gchar *proc_file_system_path;

#ifdef USE_GTK_ALT_DIALOG_BUTTON_ORDER
gboolean gtk_alt_dialog_button_order()
{
#ifdef DETAIL
	g_debug("! Launch gtk_alt_dialog_button_order()");
#endif
	gboolean result;
	g_object_get (gtk_settings_get_default(), "gtk-alternative-button-order", &result, NULL);
	return result;
}
#endif

gboolean check_if_proc_dir_exist(gchar *proc_dir)
{
#ifdef DETAIL
	g_debug("! Launch check_if_proc_dir_exist() with proc_dir = %s", proc_dir);
#endif
	if (proc_dir == NULL) return FALSE;

	if (g_file_test(proc_dir, G_FILE_TEST_EXISTS))
	{
		gboolean proc_is_exist = FALSE;
		GDir *dir = g_dir_open (proc_dir, 0, NULL);
		if (dir)
		{
			const gchar *entry = g_dir_read_name(dir);
			if (entry)
			{
				g_free(proc_file_system_path);
				proc_file_system_path = g_strdup(proc_dir);
				proc_is_exist = TRUE;
			}
		}
		g_dir_close(dir);
		return proc_is_exist;
	}

	return FALSE;
}

gboolean check_if_default_proc_dir_exist(gchar *proc_dir)
{
#ifdef DETAIL
	g_debug("! Launch check_if_default_proc_dir_exist() with proc_dir = %s", proc_dir);
#endif
	if (check_if_proc_dir_exist(proc_dir)) return TRUE;
#ifdef BSD
	if (check_if_proc_dir_exist("/compat/linux/proc")) return TRUE;
#endif
	if (check_if_proc_dir_exist("/proc")) return TRUE;
	return FALSE;
}

// The returned string should be freed when no longer needed.
gchar *convert_array_to_string(gchar **array, gchar separator)
{
#ifdef DETAIL
	g_debug("! Launch convert_array_to_string()");
#endif
#ifdef SAFEMODE
	if (array==NULL) return NULL;
#endif
	GString *array_str = g_string_new (NULL);
	gint i=0;

	while (array[i]!=NULL)
	{
		// g_debug("%d: %s", i, array[i]);
		if (array_str->len && (separator!='\0'))
			g_string_append_printf(array_str, "%c%s", separator, array[i]);
		else
			g_string_append_printf(array_str, "%s", array[i]);
		i++;
	}
	// g_debug ("Got array_str = %s", array_str->str);
	return g_string_free(array_str, FALSE);
}

// please vist http://en.wikipedia.org/wiki/Escape_sequences_in_C for more details
gchar *convert_escape_sequence_to_string(const gchar *string)
{
	if (string == NULL) return NULL;

	GString *new_string = g_string_new("");

	long i=-1;
	while (string[++i])
	{
		switch (string[i])
		{
			case '\a':
				g_string_append(new_string, "\\a");
				break;
			case '\b':
				g_string_append(new_string, "\\b");
				break;
			case '\f':
				g_string_append(new_string, "\\f");
				break;
			case '\n':
				g_string_append(new_string, "\\n");
				break;
			case '\r':
				g_string_append(new_string, "\\r");
				break;
			case '\t':
				g_string_append(new_string, "\\t");
				break;
			case '\v':
				g_string_append(new_string, "\\v");
				break;
			case '\\':
				g_string_append(new_string, "\\\\");
				break;
			case '\'':
				g_string_append(new_string, "\\'");
				break;
			case '\"':
				g_string_append(new_string, "\\\"");
				break;
			default:
				g_string_append_printf(new_string, "%c", string[i]);
				break;
		}
	}
	return g_string_free(new_string, FALSE);
}

gchar *convert_escape_sequence_from_string(const gchar *string)
{
	if (string == NULL) return NULL;

	GString *new_string = g_string_new("");

	long i=-1;
	while (string[++i])
	{
		if (string[i]=='\\')
		{
			switch (string[++i])
			{
				case 'a':
					g_string_append(new_string, "\a");
					break;
				case 'b':
					g_string_append(new_string, "\b");
					break;
				case 'f':
					g_string_append(new_string, "\f");
					break;
				case 'n':
					g_string_append(new_string, "\n");
					break;
				case 'r':
					g_string_append(new_string, "\r");
					break;
				case 't':
					g_string_append(new_string, "\t");
					break;
				case 'v':
					g_string_append(new_string, "\v");
					break;
				case '\\':
					g_string_append(new_string, "\\");
					break;
				case '\'':
					g_string_append(new_string, "'");
					break;
				case '"':
					g_string_append(new_string, "\"");
					break;
				default:
					g_string_append_printf(new_string, "%c", string[i]);
					break;
			}
		}
		else
			g_string_append_printf(new_string, "%c", string[i]);
	}
	return g_string_free(new_string, FALSE);
}

// return FALSE if the strings are the same.
gboolean compare_strings(const gchar *string_a, const gchar *string_b, gboolean case_sensitive)
{
#ifdef FULL
	g_debug("! Launch compare_strings()! with string_a = %s, string_b = %s", string_a, string_b);
#endif
	if ((string_a == NULL) && (string_b == NULL)) return FALSE;
	if ((string_a == NULL) || (string_b == NULL)) return TRUE;
	if (case_sensitive)
	{
		if (strcmp(string_a, string_b)) return TRUE;
	}
	else
	{
		if (strcasecmp(string_a, string_b)) return TRUE;
	}
	return FALSE;
}

// The returned string should be freed when no longer needed.
gchar *convert_str_to_utf8(gchar *string, gchar *encoding_str)
{
#ifdef DETAIL
	g_debug("! Launch convert_str_to_utf8() with string = %s, encoding_str = %s", string, encoding_str);
#endif
	if (string==NULL) return NULL;

	gchar *local_string = NULL;
	if (encoding_str)
	{
		//g_debug("string = %s", string);
		//g_debug("encoding_str = %s", encoding_str);
		local_string = g_convert_with_fallback (string,
							-1,
							"UTF-8",
							encoding_str,
							"_",
							NULL,
							NULL,
							NULL);
		// g_debug("Get local_string = %s", local_string);

		if ((local_string == NULL ) || (local_string[0]=='\0'))
		{
			g_free(local_string);
			local_string = g_strdup(string);
			gint i = 0;
			while (local_string[i])
			{
				if (local_string[i] < 32 || local_string[i]>126)
					local_string[i] = '?';
				i++;
			}
		}

		// g_debug("FINAL: local_string = %s", local_string);
		return local_string;
	}
	else
		return g_strdup(string);
}

void set_VTE_CJK_WIDTH_environ(gint VTE_CJK_WIDTH)
{
#ifdef DETAIL
	g_debug("! Launch set_VTE_CJK_WIDTH_environ() with VTE_CJK_WIDTH = %d", VTE_CJK_WIDTH);
#endif
	// Set the VTE_CJK_WIDTH environment
	switch (VTE_CJK_WIDTH)
	{
		case 0:
			// Don't touch VTE_CJK_WIDTH
			break;
		//case 1:
		//	// The VTE_CJK_WIDTH will be setted depend on the LC environ in newer libvte version.
		//	unsetenv("VTE_CJK_WIDTH");
		//	break;
		case 1:
			// VTE_CJK_WIDTH=narrow only work for vte >= 0.16.14
			setenv("VTE_CJK_WIDTH", "narrow", TRUE);
			break;
		case 2:
			setenv("VTE_CJK_WIDTH", "wide", TRUE);
			break;
		default:
#ifdef FATAL
			print_switch_out_of_range_error_dialog("set_VTE_CJK_WIDTH_environ",
							       "VTE_CJK_WIDTH",
							       VTE_CJK_WIDTH);
#endif
			break;

	}
}

gchar *get_VTE_CJK_WIDTH_str(gint VTE_CJK_WIDTH)
{
#ifdef DETAIL
	g_debug("! Launch get_VTE_CJK_WIDTH_str() with VTE_CJK_WIDTH = %d", VTE_CJK_WIDTH);
#endif
	switch (VTE_CJK_WIDTH)
	{
		case 0:
			return NULL;
		case 1:
			return "narrow";
		case 2:
			return "wide";
		default:
#ifdef FATAL
			print_switch_out_of_range_error_dialog("get_VTE_CJK_WIDTH_str",
							       "VTE_CJK_WIDTH",
							       VTE_CJK_WIDTH);
#endif
			break;
	}
	return NULL;
}

gint get_default_VTE_CJK_WIDTH()
{
#ifdef DETAIL
	g_debug("! Launch get_default_VTE_CJK_WIDTH()");
#endif
	const gchar *VTE_CJK_WIDTH = g_getenv("VTE_CJK_WIDTH");
	if (VTE_CJK_WIDTH==NULL)
		return 0;
	else
	{
		// VTE_CJK_WIDTH only work under UTF-8
		if ((compare_strings (VTE_CJK_WIDTH, "wide", FALSE)==FALSE) ||
		    (compare_strings (VTE_CJK_WIDTH, "1", FALSE)==FALSE))
			return 2;
		else if ((compare_strings (VTE_CJK_WIDTH, "narrow", FALSE)==FALSE) ||
			 (compare_strings (VTE_CJK_WIDTH, "0", FALSE)==FALSE))
			return 1;
		else
			return 0;
	}
}

void restore_SYSTEM_VTE_CJK_WIDTH_STR()
{
#ifdef DETAIL
	g_debug("! Launch restore_SYSTEM_VTE_CJK_WIDTH_STR()");
#endif
	extern gchar *SYSTEM_VTE_CJK_WIDTH_STR;
	if (SYSTEM_VTE_CJK_WIDTH_STR)
		g_setenv("VTE_CJK_WIDTH", SYSTEM_VTE_CJK_WIDTH_STR, TRUE);
	else
		g_unsetenv("VTE_CJK_WIDTH");
}

void set_env(const gchar *variable, const gchar *value, gboolean overwrite)
{
#ifdef DETAIL
	g_debug("! Launch set_env() with variable = %s, value = %s, overwrite = %d", variable, value, overwrite);
#endif
#ifdef SAFEMODE
	if ((variable==NULL) || (variable[0]=='\0')) return;
#endif
	if (value)
		g_setenv(variable, value, overwrite);
	else
		g_unsetenv(variable);
}

// get default locale from environ
// The returned string CAN NOT be free()!
const gchar *get_default_lc_data(gint lc_type)
{
#ifdef DETAIL
	g_debug("! Launch get_default_lc_data() with lc_type= %d", lc_type);
#endif
	const char *lc_data;
	switch (lc_type)
	{
		case LC_CTYPE:
			lc_data = g_getenv("LC_CTYPE");
			break;
		case LC_MESSAGES:
			lc_data = g_getenv("LC_MESSAGES");
			break;
		case LANGUAGE:
			return g_getenv("LANGUAGE");
		default:
#ifdef FATAL
			print_switch_out_of_range_error_dialog("get_default_lc_data", "lc_type", lc_type);
#endif
			return "";
	}
	const char *lc_all = g_getenv("LC_ALL");
	const char *lang = g_getenv("LANG");

	if ( (!lc_data) && lang)
		lc_data = lang;

	if (lc_all)
		lc_data = lc_all;

//	g_debug("Get lc_data = %s", lc_data);
	if (lc_data)
		return lc_data;
	else
		return "";
}

// The returned string should be freed when no longer needed.
gchar *get_encoding_from_locale(const gchar *locale)
{
#ifdef DETAIL
	g_debug("! Launch get_encoding_from_locale() with locale = %s", locale);
#endif
#ifdef OUT_OF_MEMORY
#  undef g_strdup
#endif
	// locale==NULL: get the init encoding.

	G_CONST_RETURN char *locale_encoding = NULL;
	if (setlocale(LC_CTYPE, locale)) g_get_charset(&locale_encoding);
	return g_strdup(locale_encoding);

#ifdef OUT_OF_MEMORY
	#define g_strdup fake_g_strdup
#endif
}

gboolean check_string_in_array(gchar *str, gchar **lists)
{
#ifdef DETAIL
	g_debug("! Launch check_string_in_array() with str = %s", str);
#endif
#ifdef SAFEMODE
	if ((str==NULL) || (lists==NULL)) return FALSE;
#endif
	gint i=-1;

	while (lists[++i])
	{
		// g_debug("Checking %s and %s...", str, lists[i]);
		if (strcmp(str, lists[i])==0) return TRUE;
	}
	// g_debug("No, Can not find '%s' in the array!!", str);
	return FALSE;
}

//gchar *get_proc_data(pid_t pid, gchar *file, gsize *length)
//{
//#ifdef FULL
//	g_debug("! Launch get_proc_data() with pid = %d, file = %s", pid, file);
//#endif
//
//	if (! proc_exist) return NULL;
//	if (pid<1) return NULL;
//
//	*length=0;
//	gchar *contents=NULL;
//	gchar *file_path = g_strdup_printf("%s/%d/%s", proc_file_system_path, (gint)pid, file);
//	// g_debug("file_path = %s", file_path);
//
//	if (file_path && (g_file_test(file_path, G_FILE_TEST_EXISTS)))
//		g_file_get_contents (file_path, &contents, length, NULL);
//	g_free(file_path);
//	// g_debug("contents = %s", contents);
//	return contents;
//}

gchar *get_proc_data(pid_t pid, gchar *file, gsize *length)
{
#ifdef FULL
	g_debug("! Launch get_proc_data() with pid = %d, file = %s", pid, file);
#endif
	// g_debug("proc_exist = %d", proc_exist);
	if (! proc_exist) return NULL;
	if (pid<1) return NULL;
#ifdef UNIT_TEST
	if (file==NULL) return NULL;
#endif
	gchar *contents=NULL;
	gint timeout=0;
	gchar *proc_path = g_strdup_printf("%s/%d", proc_file_system_path, (gint)pid);
	// g_debug("proc_path = %s", proc_path);
	gchar *file_path = g_strdup_printf("%s/%s", proc_path, file);
	// g_debug("file_path = %s", file_path);

#if defined(OUT_OF_MEMORY) || defined(UNIT_TEST)
	if (proc_path && file_path)
	{
#endif
		while (g_file_test(proc_path, G_FILE_TEST_EXISTS) &&
		       g_file_get_contents (file_path, &contents, length, NULL))
		{
			// g_debug("Got the contents length is %d for %s", *length, file_path);
			if (*length==0)
			{
				// gsize len = 0;
				// gchar *stat = NULL;
				// gchar *stat_path = g_strdup_printf("%s/%d/stat", proc_file_system_path, (gint)pid);
				// if (g_file_get_contents (stat_path, &contents, &len, NULL))
				// {
				//	g_debug("Got len = %d, stat = %s", len, stat);
				//	if (len && stat)
				//	{
				//		gchar **stats = split_string(stat, " ()", 6);
				//		if (stats && stats[4][0]=='Z')
				//		{
				//			g_warning("The child process \"(%s) %s\" has died. Abort.",
				//				  stats[0], stats[2]);
				//			g_free(stat);
				//			g_strfreev(stats);
				//
				//			g_free(contents);
				//			contents = NULL;
				//			break;
				//		}
				//		g_strfreev(stats);
				//	}
				// }
				// g_free(stat);
				g_message("Waiting for %s/%d/%s...", proc_file_system_path, (gint)pid, file);
				// we should wait until "/proc/%d/file" is not empty
				usleep(100000);
				timeout++;
				// contents = "" here
				g_free(contents);
				contents = NULL;
			}
			else
				break;

			// we only try for 3 times
			if (timeout>2)
			{
#ifdef FATAL
				g_message("Failed when waiting for %s/%d/%s. Abort.", proc_file_system_path, (gint)pid, file);
#else
				g_warning("Failed when waiting for %s/%d/%s. Abort.", proc_file_system_path, (gint)pid, file);
#endif
				break;
			}
		}
#if defined(OUT_OF_MEMORY) || defined(UNIT_TEST)
	}
#endif
	g_free(file_path);
	g_free(proc_path);
	// g_debug("contents = %s", contents);
	return contents;
}

// it will check if the count of returned string array is < max_tokens
gchar **split_string(const gchar *str, const gchar *split, gint max_tokens)
{
#ifdef FULL
	g_debug("! Launch split_string with str = %s, split = %s, max_tokens = %d", str, split, max_tokens);
#endif
#ifdef SAFEMODE
	if ((str==NULL) || (split==NULL) || (split[0]=='\0')) return NULL;
#endif
	// g_debug("contents = %s", contents);
	gchar **datas = g_strsplit_set(str, split, max_tokens);
	gint i = -1;

	// while (datas[++i])
	//	g_debug("Got data[%d] = %s", i, datas[i]);
	// g_debug("Got i = %d in split_string()", i);
	// i = -1;
#ifdef SAFEMODE
	if ( datas && (max_tokens>0))
#else
	if (max_tokens>0)
#endif
	{
		while (datas[++i]);
		if (i<max_tokens)
		{
			g_strfreev(datas);
			datas = NULL;
		}
	}
	return datas;
}

gint count_char_in_string(const gchar *str, const gchar split)
{
#ifdef FULL
	g_debug("! Launch count_char_in_string with str = %s, split = %c", str, split);
#endif
	if (str==NULL) return -1;

	gint i = -1, count = 0;
	while (str[++i])
		if (str[i]==split) count++;

	return count;
}

gchar **get_pid_stat(pid_t pid, gint max_tokens)
{
#ifdef FULL
	g_debug("! Launch get_pid_stat with pid = %d, max_tokens = %d", pid, max_tokens);
#endif
	if (pid<1) return NULL;

	gsize length=0;
	gchar *stat = get_proc_data(pid, "stat", &length);
	gchar **stats = NULL;
	if (stat) stats = split_string(stat, " ()", max_tokens);
	g_free(stat);
	return stats;
}

// The returned string should be freed when no longer needed.
gchar *convert_text_to_html(StrAddr **text, gboolean free_text, gchar *color, StrLists *tag, ...)
{
#ifdef DETAIL
	g_debug("! Launch convert_text_to_html() with text = %s, color = %s, tag = %s",
		*text, color, tag);
#endif
#ifdef SAFEMODE
	if ((text==NULL) || (*text==NULL)) return NULL;
#endif
	gchar *markup_escape_text = g_markup_escape_text(*text, -1);

	if (color)
	{
		gchar *color_profile = g_strdup_printf ("<span foreground=\"%s\">%s</span>",
							color, markup_escape_text);
		g_free(markup_escape_text);
		markup_escape_text = color_profile;
	}

	va_list arg_ptr;
	va_start(arg_ptr, tag);
	while (tag)
	{
		gchar *new_html_string = g_strdup_printf("<%s>%s</%s>",
							 tag, markup_escape_text, tag);
		g_free(markup_escape_text);
		markup_escape_text = new_html_string;
		tag = va_arg(arg_ptr, char *);
	}
	va_end(arg_ptr);

	if (free_text)
	{
		g_free(*text);
#ifdef SAFEMODE
		*text = NULL;
#endif
	}
	return markup_escape_text;
}

// The returned string should be freed when no longer needed.
gchar *join_strings_to_string(const gchar separator, const gint total, const StrLists *string, ...)
{
#ifdef DETAIL
	g_debug("! Launch join_strings_to_string() with separator = '%c', total = %d",
		separator, total);
#endif
#ifdef SAFEMODE
	if (separator=='\0') return NULL;
#endif
	GString *strings = g_string_new ("");
	gint i;
	va_list arg_ptr;
	va_start(arg_ptr, string);
	for (i=0; i<total; i++)
	{
		if (string && string[0]!='\0')
		{
			if (strings->len)
				g_string_append_printf(strings, "%c%s", separator, string);
			else
				g_string_append_printf(strings, "%s", string);
		}
		// g_debug("[%2d] convert_strings_to_string(): strings = %s", i, strings->str);
		string = va_arg(arg_ptr, char *);
	}
	va_end(arg_ptr);
	// g_debug("convert_strings_to_string(): strings = %s", strings->str);

	return g_string_free(strings, FALSE);
}

// return NULL: No, it is less than MAX
// return String: Yes, It is more then MAX, and have been cut short to OUTPUT_LINE lines.
gchar *colorful_max_new_lines(gchar *string, gint max, gint output_line)
{
#ifdef DETAIL
	g_debug("! Launch count_max_new_lines() with string = '%s', max = %d",
		string, max);
#endif
	if (string==NULL) return NULL;

	gint i = 0;
	gint total = 0;
	gchar *color_str = g_strdup("");
	gint star_start = 0;

	while (string[i])
	{
		char separate;

		if ((string[i]=='\n') || (string[i]=='\r') || (string[i+1]=='\0'))
		{
			if ((string[i]=='\n') || (string[i]=='\r'))
				total++;

			if (total < output_line)
			{
				separate = string[i];
				if ((string[i]=='\n') || (string[i]=='\r'))
					string[i] = 0;
				gchar *str = &(string[star_start]);
				gchar *markup_str;
				if ((str==NULL) || (str[0] == '\0'))
					markup_str = g_strdup("");
				else
				{
					gchar *tmp_str = g_markup_escape_text(str, -1);
					markup_str = g_strconcat("<b><span foreground=\"blue\">",
								 tmp_str, "</span></b>", NULL);
					g_free(tmp_str);
				}
				gchar *new_str = NULL;
				if ((separate=='\n') || (separate=='\r'))
				{
					// TRANSLATE NOTE: The "↲" is a replace character for <Enter> (<CR>, or <New Line>).
					// TRANSLATE NOTE: You may translate it into something like "¶", "↵" or "↩".
					gchar *markup_enter = g_markup_escape_text(_("↲"), -1);
					if (string[i+1])
						new_str = g_strconcat(color_str,
								      markup_str,
								      "<small><span foreground=\"darkgray\">",
								      markup_enter,
								      "</span></small>\n", NULL);
					else
						new_str = g_strconcat(color_str,
								      markup_str,
								      "<small><span foreground=\"darkgray\">",
								      markup_enter,
								      "</span></small>", NULL);
					g_free(markup_enter);
				}
				else
					new_str = g_strconcat(color_str, markup_str, NULL);
				g_free(markup_str);
				g_free(color_str);
				color_str = new_str;
				string[i] = separate;
				star_start = i+1;

				// g_debug("colorful_max_new_lines(): color_str = %s", color_str);
			}
		}
		if (total>=output_line)
		{
			separate = string[i];
			string[i] = 0;
			gchar *output_str = g_strdup_printf("<tt>%s%s</tt>",
							    color_str, "<b>...</b>");
			g_free(color_str);
			string[i] = separate;
			return output_str;
		}
		i++;
	}
	// g_debug("colorful_max_new_lines(FINAL): color_str = %s", color_str);
	// g_debug("colorful_max_new_lines(): total = %d", total);
	if (total > max)
	{
		gchar *output_str = g_strdup_printf("<tt>%s</tt>", color_str);
		g_free(color_str);
		return output_str;
	}
	g_free(color_str);
	return NULL;
}

GdkRGBA convert_color_to_rgba(GdkColor color)
{
	GdkRGBA rgba;
#ifndef USE_GDK_RGBA
	rgba.pixel = 0;
#endif
	rgba.red = (gdouble)(color.red>>8)/0xFF;
	rgba.green = (gdouble)(color.green>>8)/0xFF;
	rgba.blue = (gdouble)(color.blue>>8)/0xFF;

	// g_debug("convert_color_to_rgba: convert color (%4X %4X %4X) to rgba (%0.4f %0.4f %0.4f)",
	//	color.red, color.green, color.blue, rgba.red, rgba.green, rgba.blue);

	return rgba;
}

GdkColor convert_rgba_to_color(GdkRGBA rgba)
{
	GdkColor color;

	color.pixel = 0;
	color.red = rgba.red*0xFFFF;
	color.green = rgba.green*0xFFFF;
	color.blue = rgba.blue*0xFFFF;

	return color;
}

#ifdef ENABLE_GDKCOLOR_TO_STRING
// The returned string should be freed when no longer needed.
gchar *dirty_gdk_rgba_to_string(GdkRGBA *rgba)
{
#ifdef SAFEMODE
	if (rgba==NULL) return NULL;
#endif
#  ifdef USE_GDK_RGBA
	return g_strdup_printf("#%04x%04x%04x", (unsigned int)(rgba->red*0xFFFF), (unsigned int)(rgba->green*0xFFFF), (unsigned int)(rgba->blue*0xFFFF));
#  else
	return gdk_color_to_string(rgba);
#  endif
}
#endif

#if defined(GEOMETRY) || defined(UNIT_TEST)
void widget_size_allocate (GtkWidget *widget, GtkAllocation *allocation, gchar *name)
{
#  ifdef DETAIL
	g_debug("! Launch widget_size_allocate() with widget = %p, name = %s", widget, name);
#  endif
	// 200x200: the default size of a GtkWindow
	gint ansi_color = ANSI_COLOR_BLACK;
#ifdef SAFEMODE
	if (allocation==NULL) return;
#endif
	if ((allocation->width <= 200) || (allocation->height <= 200))
		ansi_color = ANSI_COLOR_RED;
	fprintf(stderr, "\033[1;%dm!! %s_size_allocate(%p): the allocated size is %d x %d\033[0m\n",
		ansi_color, name, widget, allocation->width, allocation->height);
}
#endif

gboolean dirty_gdk_color_parse(const gchar *spec, GdkRGBA *color)
{
#ifdef DETAIL
	g_debug("! Launch dirty_gdk_color_parse() with spec = %s", spec);
#endif
	if (spec==NULL) return FALSE;

	gchar *new_spec = g_strdup(spec);
#ifdef OUT_OF_MEMORY
	if (new_spec==NULL) return FALSE;
#endif
	new_spec = g_strstrip(new_spec);
	gboolean response = gdk_rgba_parse(color, new_spec);
	g_free(new_spec);
	return response;
}

GtkWidget *dirty_gtk_vbox_new(gboolean homogeneous, gint spacing)
{
#ifdef DETAIL
	g_debug("! Launch dirty_gtk_vbox_new() with homogeneous = %d, spacing = %d", homogeneous, spacing);
#endif
#if GTK_CHECK_VERSION(2,90,0)
	GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, spacing);
	gtk_box_set_homogeneous(GTK_BOX(box), homogeneous);
#else
	GtkWidget *box = gtk_vbox_new(homogeneous, spacing);
#endif
	return box;
}

GtkWidget *dirty_gtk_hbox_new(gboolean homogeneous, gint spacing)
{
#ifdef DETAIL
	g_debug("! Launch dirty_gtk_hbox_new() with homogeneous = %d, spacing = %d", homogeneous, spacing);
#endif
#if GTK_CHECK_VERSION(2,90,0)
	GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, spacing);
	gtk_box_set_homogeneous(GTK_BOX(box), homogeneous);
#else
	GtkWidget *box = gtk_hbox_new(homogeneous, spacing);
#endif
	return box;
}

#if defined(ENABLE_VTE_BACKGROUND) || defined(FORCE_ENABLE_VTE_BACKGROUND) || defined(UNIT_TEST)
void dirty_vte_terminal_set_background_tint_color(VteTerminal *vte, const GdkRGBA rgba)
{
#  ifdef DETAIL
	g_debug("! Launch dirty_vte_terminal_set_background_tint_color() with vte = %p", vte);
#  endif
#  ifdef SAFEMODE
	if (vte==NULL) return;
#  endif
#  ifdef FORCE_ENABLE_VTE_BACKGROUND
	G_GNUC_BEGIN_IGNORE_DEPRECATIONS;
#  endif
#  ifdef USE_GDK_RGBA
#    ifndef USE_FAKE_FUNCTIONS
	GdkColor color = convert_rgba_to_color(rgba);
#    endif
	vte_terminal_set_background_tint_color(VTE_TERMINAL(vte), &(color));
#  else
	vte_terminal_set_background_tint_color(VTE_TERMINAL(vte), &(rgba));
#  endif
#  ifdef FORCE_ENABLE_VTE_BACKGROUND
	G_GNUC_END_IGNORE_DEPRECATIONS;
#  endif
}
#endif

#if defined(OUT_OF_MEMORY) || defined(UNIT_TEST)
gchar *fake_g_strdup(const gchar *str)
{
#  ifdef DETAIL
	g_debug("! fake_g_strdup(): Trying to strdup(%s)...", str);
#  endif

#  undef g_strdup
#  undef g_strsplit
	if (str==NULL) return NULL;

	gchar *return_str = NULL;
	gchar **strs = g_strsplit(str, " ", 2);
	if (strs)
	{
		if ((! compare_strings(strs[0], "Alt", FALSE)) ||
		    (! compare_strings(strs[0], "Shift", FALSE)) ||
		    (! compare_strings(strs[0], "Ctrl", FALSE)))
		{
			return_str = g_strdup(str);
			// g_debug("fake_g_strdup(): strdup(%s) succeed!!", str);
		}
	}
	g_strfreev (strs);
	return return_str;
#  define g_strdup fake_g_strdup
#  define g_strsplit fake_g_strsplit
}

gchar *fake_g_strdup_printf(const StrLists *format, ...)
{
#  ifdef DETAIL
	g_debug("! Trying to strdup_printf(%s)...", format);
#  endif
	return NULL;
}


gchar** fake_g_strsplit(const gchar *string, const gchar *delimiter, gint max_tokens)
{
#  ifdef DETAIL
	g_debug("! fake_g_strsplit(): Trying to strsplit('%s', '%s', %d)...", string, delimiter, max_tokens);
#  endif

	if ((string==NULL) || (delimiter==NULL) || (delimiter[0]=='\0')) return NULL;

#  undef g_strsplit

	gchar **return_array = NULL;
	gchar **strs = g_strsplit(string, " ", 2);
	if (strs)
	{
		if ((strs[0]=='\0') ||
		    (! compare_strings(strs[0], "Alt", FALSE)) ||
		    (! compare_strings(strs[0], "Shift", FALSE)) ||
		    (! compare_strings(strs[0], "Ctrl", FALSE)))
		{
			return_array = g_strsplit (string, delimiter, max_tokens);
			// g_debug("fake_g_strsplit(): strsplit('%s', '%s', %d) succeed!!",
			//	string, delimiter, max_tokens);
		}
	}
	g_strfreev (strs);

	if (! return_array)
	{
		if (! compare_strings(delimiter, "+", TRUE))
		{
			return_array = g_strsplit (string, delimiter, max_tokens);
			// g_debug("fake_g_strsplit(): strsplit('%s', '%s', %d) succeed!!",
			//	string, delimiter, max_tokens);
		}
	}

	return return_array;

#  define g_strsplit fake_g_strsplit
}

// A very dirty fix for unit test error.
gchar **g_listenv (void) { return NULL; }
#endif
