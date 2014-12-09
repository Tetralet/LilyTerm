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

#include "profile.h"

extern GList *window_list;
extern GtkWidget *menu_active_window;
extern gchar *init_LC_CTYPE;
extern gchar *init_LC_MESSAGES;
extern gboolean checked_profile_version;
extern gboolean safe_mode;
extern gchar *system_locale_list;
extern gchar *profile_dir;

struct ModKey modkeys[MOD] = {{0}};
struct KeyValue system_keys[KEYS] = {{0}};
gchar *key_groups[KEY_GROUP] = {0};
gchar *regex_name[REGEX] = {0};
gchar *regex_str[REGEX] = {0};
struct Command command[COMMAND] = {{0}};
struct Color color[COLOR] = {{0}};
struct Page_Color page_color[PAGE_COLOR] = {{0}};
gchar *restricted_locale_message = NULL;

#ifdef USE_GDK_RGBA
struct GdkRGBA_Theme system_color_theme[THEME] = {{0}};
struct GdkColor_Theme temp_system_color_theme[THEME] =
#else
struct GdkColor_Theme system_color_theme[THEME] =
#endif
	{{0, "libvte",
	  {{ 0, 0x0000, 0x0000, 0x0000 },
	   { 0, 0xc0c0, 0x0000, 0x0000 },
	   { 0, 0x0000, 0xc0c0, 0x0000 },
	   { 0, 0xc0c0, 0xc0c0, 0x0000 },
	   { 0, 0x0000, 0x0000, 0xc0c0 },
	   { 0, 0xc0c0, 0x0000, 0xc0c0 },
	   { 0, 0x0000, 0xc0c0, 0xc0c0 },
	   { 0, 0xc0c0, 0xc0c0, 0xc0c0 },
	   { 0, 0x3f3f, 0x3f3f, 0x3f3f },
	   { 0, 0xffff, 0x3f3f, 0x3f3f },
	   { 0, 0x3f3f, 0xffff, 0x3f3f },
	   { 0, 0xffff, 0xffff, 0x3f3f },
	   { 0, 0x3f3f, 0x3f3f, 0xffff },
	   { 0, 0xffff, 0x3f3f, 0xffff },
	   { 0, 0x3f3f, 0xffff, 0xffff },
	   { 0, 0xffff, 0xffff, 0xffff }}},
	 {1, "tango",
	  {{ 0, 0x2e2e, 0x3434, 0x3636 },
	   { 0, 0xcccc, 0x0000, 0x0000 },
	   { 0, 0x4e4e, 0x9a9a, 0x0606 },
	   { 0, 0xc4c4, 0xa0a0, 0x0000 },
	   { 0, 0x3434, 0x6565, 0xa4a4 },
	   { 0, 0x7575, 0x5050, 0x7b7b },
	   { 0, 0x0606, 0x9820, 0x9a9a },
	   { 0, 0xd3d3, 0xd7d7, 0xcfcf },
	   { 0, 0x5555, 0x5757, 0x5353 },
	   { 0, 0xefef, 0x2929, 0x2929 },
	   { 0, 0x8a8a, 0xe2e2, 0x3434 },
	   { 0, 0xfcfc, 0xe9e9, 0x4f4f },
	   { 0, 0x7272, 0x9f9f, 0xcfcf },
	   { 0, 0xadad, 0x7f7f, 0xa8a8 },
	   { 0, 0x3434, 0xe2e2, 0xe2e2 },
	   { 0, 0xeeee, 0xeeee, 0xecec }}},
	 {2, "linux",
	  {{ 0, 0x0000, 0x0000, 0x0000 },
	   { 0, 0xaaaa, 0x0000, 0x0000 },
	   { 0, 0x0000, 0xaaaa, 0x0000 },
	   { 0, 0xaaaa, 0x5555, 0x0000 },
	   { 0, 0x0000, 0x0000, 0xaaaa },
	   { 0, 0xaaaa, 0x0000, 0xaaaa },
	   { 0, 0x0000, 0xaaaa, 0xaaaa },
	   { 0, 0xaaaa, 0xaaaa, 0xaaaa },
	   { 0, 0x5555, 0x5555, 0x5555 },
	   { 0, 0xffff, 0x5555, 0x5555 },
	   { 0, 0x5555, 0xffff, 0x5555 },
	   { 0, 0xffff, 0xffff, 0x5555 },
	   { 0, 0x5555, 0x5555, 0xffff },
	   { 0, 0xffff, 0x5555, 0xffff },
	   { 0, 0x5555, 0xffff, 0xffff },
	   { 0, 0xffff, 0xffff, 0xffff }}},
	 {3, "xterm",
	  {{ 0, 0x0000, 0x0000, 0x0000 },
	   { 0, 0xcdcd, 0x0000, 0x0000 },
	   { 0, 0x0000, 0xcdcd, 0x0000 },
	   { 0, 0xcdcd, 0xcdcd, 0x0000 },
	   { 0, 0x0000, 0x0000, 0xeeee },
	   { 0, 0xcdcd, 0x0000, 0xcdcd },
	   { 0, 0x0000, 0xcdcd, 0xcdcd },
	   { 0, 0xe5e5, 0xe5e5, 0xe5e5 },
	   { 0, 0x7f7f, 0x7f7f, 0x7f7f },
	   { 0, 0xffff, 0x0000, 0x0000 },
	   { 0, 0x0000, 0xffff, 0x0000 },
	   { 0, 0xffff, 0xffff, 0x0000 },
	   { 0, 0x5c5c, 0x5c5c, 0xffff },
	   { 0, 0xffff, 0x0000, 0xffff },
	   { 0, 0x0000, 0xffff, 0xffff },
	   { 0, 0xffff, 0xffff, 0xffff }}},
	 {4, "rxvt",
	  {{ 0, 0x0000, 0x0000, 0x0000 },
	   { 0, 0xcdcd, 0x0000, 0x0000 },
	   { 0, 0x0000, 0xcdcd, 0x0000 },
	   { 0, 0xcdcd, 0xcdcd, 0x0000 },
	   { 0, 0x0000, 0x0000, 0xcdcd },
	   { 0, 0xcdcd, 0x0000, 0xcdcd },
	   { 0, 0x0000, 0xcdcd, 0xcdcd },
	   { 0, 0xfafa, 0xebeb, 0xd7d7 },
	   { 0, 0x4040, 0x4040, 0x4040 },
	   { 0, 0xffff, 0x0000, 0x0000 },
	   { 0, 0x0000, 0xffff, 0x0000 },
	   { 0, 0xffff, 0xffff, 0x0000 },
	   { 0, 0x0000, 0x0000, 0xffff },
	   { 0, 0xffff, 0x0000, 0xffff },
	   { 0, 0x0000, 0xffff, 0xffff },
	   { 0, 0xffff, 0xffff, 0xffff }}},
	 {5, "solarized",			// Source: http://ethanschoonover.com/solarized#the-values
	  {{ 0, 0x0707, 0x3636, 0x4242 },	// base02;  black
	   { 0, 0xdcdc, 0x3232, 0x2f2f },	// red;     red
	   { 0, 0x8585, 0x9999, 0x0000 },	// green;   green
	   { 0, 0xb5b5, 0x8989, 0x0000 },	// yellow;  yellow
	   { 0, 0x2626, 0x8b8b, 0xd2d2 },	// blue;    blue
	   { 0, 0xd3d3, 0x3636, 0x8282 },	// magenta; magenta
	   { 0, 0x2a2a, 0xa1a1, 0x9898 },	// cyan;    cyan
	   { 0, 0xeeee, 0xe8e8, 0xd5d5 },	// base2;   white
	   { 0, 0x0000, 0x2b2b, 0x3636 },	// base03;  brblack
	   { 0, 0xcbcb, 0x4b4b, 0x1616 },	// orange;  brred
	   { 0, 0x5858, 0x6e6e, 0x7575 },	// base01;  brgreen
	   { 0, 0x6565, 0x7b7b, 0x8383 },	// base00;  bryellow
	   { 0, 0x8383, 0x9494, 0x9696 },	// base0;   brblue
	   { 0, 0x6c6c, 0x7171, 0xc4c4 },	// violet;  brmagenta
	   { 0, 0x9393, 0xa1a1, 0xa1a1 },	// base1;   brcyan
	   { 0, 0xfdfd, 0xf6f6, 0xe3e3 }}},	// base3; brwhite
	 {6, "bbs",
	  {{ 0, 0x0000, 0x0000, 0x0000 },
	   { 0, 0xcf01, 0x0000, 0x0000 },
	   { 0, 0x0000, 0x5294, 0x0000 },
	   { 0, 0x5e75, 0x5e75, 0x0000 },
	   { 0, 0x0000, 0x0000, 0x9d59 },
	   { 0, 0x5ef7, 0x0000, 0x5ef7 },
	   { 0, 0x0000, 0xa0a6, 0xa0a6 },
	   { 0, 0xc51e, 0xc51e, 0xc51e },
	   { 0, 0x70a3, 0x70a3, 0x70a3 },
	   { 0, 0xffff, 0x41ee, 0x41ee },
	   { 0, 0x0000, 0xe949, 0x0000 },
	   { 0, 0xffff, 0xffff, 0x5084 },
	   { 0, 0x94a5, 0x94a5, 0xffff },
	   { 0, 0xffff, 0x741d, 0xffff },
	   { 0, 0x2d2d, 0xffff, 0xffff },
	   { 0, 0xffff, 0xffff, 0xffff }}},
	 {7, BINARY,
	  {{ 0, 0x0808, 0x0808, 0x0808 },
	   { 0, 0xb0b0, 0x2828, 0x2525 },
	   { 0, 0x5c5c, 0x8080, 0x1a1a },
	   { 0, 0xa8a8, 0x6d6d, 0x0000 },
	   { 0, 0x1010, 0x3b3b, 0x5959 },
	   { 0, 0xa8a8, 0x2b2b, 0x6868 },
	   { 0, 0x2121, 0x8080, 0x7979 },
	   { 0, 0xbebe, 0xb9b9, 0xaaaa },
	   { 0, 0x5050, 0x4d4d, 0x4747 },
	   { 0, 0xe3e3, 0x3333, 0x3030 },
	   { 0, 0x6f6f, 0xadad, 0x0000 },
	   { 0, 0xdbdb, 0x8e8e, 0x0000 },
	   { 0, 0x2727, 0x9090, 0xdbdb },
	   { 0, 0xdbdb, 0x3838, 0x8686 },
	   { 0, 0x2e2e, 0xb3b3, 0xa8a8 },
	   { 0, 0xf1f1, 0xecec, 0xe0e0 }}},
	 {8, "grayscale",
	  {{ 0, 0x0000, 0x0000, 0x0000 },
	   { 0, 0x3f3f, 0x3f3f, 0x3f3f },
	   { 0, 0x4444, 0x4444, 0x4444 },
	   { 0, 0x8383, 0x8383, 0x8383 },
	   { 0, 0x4949, 0x4949, 0x4949 },
	   { 0, 0x8888, 0x8888, 0x8888 },
	   { 0, 0x8d8d, 0x8d8d, 0x8d8d },
	   { 0, 0xe9e9, 0xe9e9, 0xe9e9 },
	   { 0, 0x4040, 0x4040, 0x4040 },
	   { 0, 0x5f5f, 0x5f5f, 0x5f5f },
	   { 0, 0x5a5a, 0x5a5a, 0x5a5a },
	   { 0, 0xafaf, 0xafaf, 0xafaf },
	   { 0, 0x5050, 0x5050, 0x5050 },
	   { 0, 0xaaaa, 0xaaaa, 0xaaaa },
	   { 0, 0xa5a5, 0xa5a5, 0xa5a5 },
	   { 0, 0xffff, 0xffff, 0xffff }}},
	{9, "(build-in)",
	  {{ 0, 0x0000, 0x0000, 0x0000 },
	   { 0, 0x0000, 0x0000, 0x0000 },
	   { 0, 0x0000, 0x0000, 0x0000 },
	   { 0, 0x0000, 0x0000, 0x0000 },
	   { 0, 0x0000, 0x0000, 0x0000 },
	   { 0, 0x0000, 0x0000, 0x0000 },
	   { 0, 0x0000, 0x0000, 0x0000 },
	   { 0, 0x0000, 0x0000, 0x0000 },
	   { 0, 0x0000, 0x0000, 0x0000 },
	   { 0, 0x0000, 0x0000, 0x0000 },
	   { 0, 0x0000, 0x0000, 0x0000 },
	   { 0, 0x0000, 0x0000, 0x0000 },
	   { 0, 0x0000, 0x0000, 0x0000 },
	   { 0, 0x0000, 0x0000, 0x0000 },
	   { 0, 0x0000, 0x0000, 0x0000 },
	   { 0, 0x0000, 0x0000, 0x0000 }}}};

#ifdef ENABLE_VTE_ERASE_TTY
struct Erase_Binding erase_binding[ERASE_BINDING] =
	{{"AUTO",
	 VTE_ERASE_AUTO},
	 {"ASCII_BACKSPACE",
	 VTE_ERASE_ASCII_BACKSPACE},
	 {"ASCII_DELETE",
	 VTE_ERASE_ASCII_DELETE},
	 {"DELETE_SEQUENCE",
	 VTE_ERASE_DELETE_SEQUENCE},
	 {"TTY",
	 VTE_ERASE_TTY}};
#else
struct Erase_Binding erase_binding[ERASE_BINDING] =
	{{"AUTO",
	 VTE_ERASE_AUTO},
	 {"ASCII_BACKSPACE",
	 VTE_ERASE_ASCII_BACKSPACE},
	 {"ASCII_DELETE",
	 VTE_ERASE_ASCII_DELETE},
	 {"DELETE_SEQUENCE",
	 VTE_ERASE_DELETE_SEQUENCE}};
#endif

#ifdef ENABLE_CURSOR_SHAPE
struct Cursor_Shape cursor_shape[CURSOR_SHAPE] =
	{{"BLOCK",
	 VTE_CURSOR_SHAPE_BLOCK},
	 {"IBEAM",
	 VTE_CURSOR_SHAPE_IBEAM},
	 {"UNDERLINE",
	 VTE_CURSOR_SHAPE_UNDERLINE}};
#endif

void convert_system_color_to_rgba()
{
#ifdef USE_GDK_RGBA
	gint i, j;
	for (i=0; i<THEME; i++)
	{
		system_color_theme[i].index = temp_system_color_theme[i].index;
		system_color_theme[i].name = temp_system_color_theme[i].name;
		for (j=0; j<COLOR; j++)
			system_color_theme[i].color[j] = convert_color_to_rgba(temp_system_color_theme[i].color[j]);
	}
#endif
}

void init_command()
{
#ifdef DETAIL
	g_debug("! Launch init_command()!");
#endif

	regex_name[0] = "username_regex";
	regex_name[1] = "password_regex";
	regex_name[2] = "hostname_regex";
	regex_name[3] = "address_body_regex";
	regex_name[4] = "address_end_regex";

	#define USERNAME "[A-Za-z][-A-Za-z0-9.]*"
	regex_str[0] = USERNAME;
	#define PASSWORD "(:[^ \t\r\n]+)?"
	regex_str[1] = PASSWORD;
	#define LOGIN "(" USERNAME PASSWORD "@)?"
	#define HOSTNAME "[A-Za-z0-9][-A-Za-z0-9.]*\\.[A-Za-z0-9]+[-A-Za-z0-9.]*[-A-Za-z0-9]*"
	regex_str[2] = HOSTNAME;
	#define PORT "(:[0-9]+)?"
	#define ADDRESS_BODY "([^|.< \t\r\n\\\"]*([.][^|< \t\r\n\\\"])?[^|.< \t\r\n\\\"]*)*"
	regex_str[3] = ADDRESS_BODY;
	#define ADDRESS_END "[^<> \t\r\n,;|\\\"]*[^|.<> \t\r\n\\\"]"
	regex_str[4] = ADDRESS_END;

	// WWW
	command[TAG_WWW].name = "web_browser";
	command[TAG_WWW].match = "[Hh][Tt][Tt][Pp][Ss]?://" LOGIN HOSTNAME PORT "(/" ADDRESS_BODY ADDRESS_END ")?/*";
	command[TAG_WWW].comment = "# The web browser using for http(s)://";
	command[TAG_WWW].method_name = "web_method";
	command[TAG_WWW].environ_name = "web_environ";
	command[TAG_WWW].VTE_CJK_WIDTH_name = "web_VTE_CJK_WIDTH";
	command[TAG_WWW].encoding_name = "web_encoding";
	command[TAG_WWW].locale_name = "web_locale";
	command[TAG_WWW].match_regex_name = "web_match_regex";

	// FTP
	command[TAG_FTP].name = "ftp_client";
	command[TAG_FTP].match = "[Ff][Tt][Pp][Ss]?://" LOGIN HOSTNAME PORT "(/" ADDRESS_BODY ADDRESS_END ")?/*";
	command[TAG_FTP].comment = "# The ftp client using for ftp(s)://";
	command[TAG_FTP].method_name = "ftp_method";
	command[TAG_FTP].environ_name = "ftp_environ";
	command[TAG_FTP].VTE_CJK_WIDTH_name = "ftp_VTE_CJK_WIDTH";
	command[TAG_FTP].encoding_name = "ftp_encoding";
	command[TAG_FTP].locale_name = "ftp_locale";
	command[TAG_FTP].match_regex_name = "ftp_match_regex";

	// FILE
	command[TAG_FILE].name = "file_manager";
	command[TAG_FILE].match = "[Ff][Ii][Ll][Ee]:///" ADDRESS_BODY ADDRESS_END;
	command[TAG_FILE].comment = "# The file manager using for file:// and [Open current directory with file manager]";
	command[TAG_FILE].method_name = "file_method";
	command[TAG_FILE].environ_name = "file_environ";
	command[TAG_FILE].VTE_CJK_WIDTH_name = "file_VTE_CJK_WIDTH";
	command[TAG_FILE].encoding_name = "file_encoding";
	command[TAG_FILE].locale_name = "file_locale";
	command[TAG_FILE].match_regex_name = "file_match_regex";

	// MAIL
	command[TAG_MAIL].name = "email_client";
	command[TAG_MAIL].match = "([Mm][Aa][Ii][Ll][Tt][Oo]:)?" USERNAME "@" HOSTNAME;
	command[TAG_MAIL].comment = "# The email client using for user@host";
	command[TAG_MAIL].method_name = "email_method";
	command[TAG_MAIL].environ_name = "email_environ";
	command[TAG_MAIL].VTE_CJK_WIDTH_name = "email_VTE_CJK_WIDTH";
	command[TAG_MAIL].encoding_name = "email_encoding";
	command[TAG_MAIL].locale_name = "email_locale";
	command[TAG_MAIL].match_regex_name = "email_match_regex";
}

void init_user_command(struct Window *win_data)
{
#ifdef DETAIL
	g_debug("! Launch init_user_command() for win_data %p", win_data);
#endif
#ifdef SAFEMODE
		if (win_data==NULL) return;
#endif

	win_data->user_command[TAG_WWW].command = g_strdup("xdg-open");
	win_data->user_command[TAG_FTP].command = g_strdup("xdg-open");
	win_data->user_command[TAG_FILE].command = g_strdup("xdg-open");
	win_data->user_command[TAG_MAIL].command = g_strdup("xdg-open");

	gint i;

	// Keep it in NULL will be a better idea
	// for (i=0; i<REGEX; i++)
	//	win_data->user_regex[i] = g_strdup("");

	for (i=0; i<COMMAND; i++)
	{
		win_data->user_command[i].method = 1;
		win_data->user_command[i].environ = g_strdup("");
		win_data->user_command[i].VTE_CJK_WIDTH = 0;
		win_data->user_command[i].locale = g_strdup("");
		// Keep it in NULL will be a better idea
		// win_data->user_command[i].match_regex_orig = g_strdup("");
	}
}

void init_window_parameters(struct Window *win_data)
{
#ifdef DETAIL
	g_debug("! Launch init_window_parameters() with win_data = %p", win_data);
#endif
#ifdef SAFEMODE
		if (win_data==NULL) return;
#endif
	// win_data->environment;
	// win_data->warned_locale_list = g_string_new(" ");		// inited in init_prime_user_datas()
	// win_data->runtime_encoding;
	win_data->default_encoding = g_strdup("");
	// win_data->runtime_LC_MESSAGES;
	// win_data->supported_encodings;
	// win_data->encoding_sub_menu
	// win_data->default_menuitem_encoding
	// win_data->default_locale = g_strdup("");			// inited in init_prime_user_datas()
	// win_data->runtime_locale;
	win_data->locales_list = g_strdup("UTF-8");
	// win_data->supported_locales;
	// win_data->locale_sub_menu
	// win_data->default_shell;
#ifdef ENABLE_SET_EMULATION
	win_data->emulate_term = g_strdup("xterm");
#endif
	win_data->VTE_CJK_WIDTH = 1;
	// win_data->VTE_CJK_WIDTH_STR;
	// win_data->argc;
	// win_data->argv;
	// win_data->command;
	// win_data->hold;
	// win_data->init_tab_number;
	// win_data->init_dir;
	// win_data->login_shell;
	// win_data->utmp;
	// win_data->geometry;
	// win_data->subitem_new_window_from_list;
	// win_data->menuitem_new_window_from_list;
	// win_data->subitem_load_profile_from_list;
	// win_data->menuitem_load_profile_from_list;
	// win_data->use_custom_profile;
	// win_data->profile;
	// win_data->specified_profile;
	win_data->profile_dir_modtime = -1;
	// win_data->menuitem_auto_save = NULL;
	// win_data->auto_save = FALSE;
	// win_data->startup_fullscreen;
	// win_data->fullscreen;
	// win_data->true_fullscreen;
	win_data->show_tabs_bar = AUTOMATIC;
	// win_data->fullscreen_show_scroll_bar = 1;
	// win_data->window_status;
	// win_data->window;
	// win_data->notebook;
	win_data->show_close_button_on_tab = TRUE;
	// win_data->shwo_close_button_on_all_tab;
	// win_data->current_vte;
	win_data->window_title_shows_current_page = TRUE;
	win_data->window_title_append_package_name = TRUE;
	// win_data->custom_window_title_str;
	// win_data->window_title_tpgid;
	// 0: do NOT use rgba
	// 1: force to use rgba
	// 2: decide by gdk_screen_is_composited()
	// if (! win_data->use_rgba) win_data->use_rgba = 2;
	// win_data->custom_tab_names_str;
	// win_data->custom_tab_names_strs;
	// win_data->custom_tab_names_current;
	// win_data->custom_tab_names_total;

	win_data->use_rgba_orig = 2;
#ifdef ENABLE_RGBA
	win_data->transparent_window = 2;
	win_data->window_opacity = 0.05;

	win_data->dim_window = FALSE;
	win_data->window_opacity_inactive = 0.2;
#endif
	win_data->enable_key_binding = TRUE;
	// win_data->User_KeyValue user_keys[KEYS];
	// win_data->hints_type;
	// win_data->resize_type;
#ifdef USE_GTK3_GEOMETRY_METHOD
	win_data->geometry_width = SYSTEM_COLUMN;
	win_data->geometry_height = SYSTEM_ROW;
#endif
	win_data->lost_focus = FALSE;
	// win_data->keep_vte_size;
	// win_data->menu;
	// win_data->menu_activated;
#ifdef ENABLE_RGBA
	// win_data->menuitem_trans_win;
#endif
	// win_data->menuitem_trans_bg;
	win_data->show_color_selection_menu = TRUE;
	win_data->show_resize_menu = TRUE;
	win_data->font_resize_ratio = 0;
	win_data->window_resize_ratio = 1.12;
	win_data->show_background_menu = TRUE;
#ifdef ENABLE_IM_APPEND_MENUITEMS
	// win_data->show_input_method_menu = FALSE;
#endif
	win_data->show_change_page_name_menu = TRUE;
	win_data->show_exit_menu = TRUE;
	win_data->enable_hyperlink = TRUE;
	// win_data->User_Command user_command[COMMAND];
	// win_data->menuitem_copy_url;
	// win_data->menuitem_dim_text;
	// win_data->menuitem_cursor_blinks;
	// win_data->menuitem_allow_bold_text;
	// win_data->menuitem_open_url_with_ctrl_pressed;
	// win_data->menuitem_disable_url_when_ctrl_pressed;
	// win_data->menuitem_audible_bell;
#ifdef ENABLE_VISIBLE_BELL
	// win_data->menuitem_visible_bell;
#endif
	// win_data->menuitem_urgent_bell;
	// win_data->urgent_bell_focus_in_event_id;
	// win_data->menuitem_show_tabs_bar;
	// win_data->menuitem_hide_tabs_bar;
	// win_data->menuitem_hide_scroll_bar;
	win_data->show_copy_paste_menu = TRUE;
	win_data->embedded_copy_paste_menu = TRUE;
	// win_data->menuitem_copy;
	// win_data->menuitem_paste;
	// win_data->menuitem_clipboard;
	// win_data->menuitem_primary;
	// win_data->fg_color;
	// win_data->fg_color_inactive;
#ifdef ENABLE_GDKCOLOR_TO_STRING
	// win_data->cursor_color;
	dirty_gdk_color_parse (DEFAULT_CURSOR_COLOR, &(win_data->cursor_color));
	// win_data->custom_cursor_color = FALSE;
#endif
	// win_data->bg_color;
	// win_data->color_theme_str;
	// win_data->color_theme;
	// win_data->color_value[COLOR];
	// win_data->invert_color;
	// win_data->menuitem_invert_color;
	// win_data->menuitem_theme;
	// win_data->current_menuitem_theme;
	// win_data->have_custom_color;
	// win_data->use_custom_theme;
	// win_data->ansi_color_sub_menu;
	// win_data->ansi_color_menuitem;
	// win_data->color[COLOR];
	// win_data->color_inactive[COLOR];
	// win_data->color_orig[COLOR];
	win_data->color_brightness = 0.2;
	win_data->color_brightness_inactive = -2;
	win_data->page_width = 16;
	// win_data->fill_tabs_bar;
	// win_data->tabs_bar_position;
	win_data->page_name = g_strdup("Terminal");
	win_data->reuse_page_names = TRUE;
	win_data->page_names = g_strdup("Terminal");
	// win_data->splited_page_names;
	// win_data->page_names_no;
	// win_data->max_page_names_no;
	win_data->page_shows_number = TRUE;
	win_data->page_shows_encoding = TRUE;
	win_data->page_shows_current_cmdline = TRUE;
	win_data->page_shows_window_title = TRUE;
	win_data->page_shows_current_dir = TRUE;
	win_data->use_color_page = TRUE;
	win_data->check_root_privileges = TRUE;
	win_data->bold_current_page_name = TRUE;
	win_data->bold_action_page_name = TRUE;
	win_data->user_page_color[0] = g_strdup("#9A6401");
	win_data->user_page_color[1] = g_strdup("#1C1CDC");
	win_data->user_page_color[2] = g_strdup("#215E3E");
	win_data->user_page_color[3] = g_strdup("#9C0A81");
	win_data->user_page_color[4] = g_strdup("#BE0020");
	win_data->user_page_color[5] = g_strdup("#333333");
#ifndef vte_terminal_set_font_from_string_full
	win_data->font_anti_alias = VTE_ANTI_ALIAS_USE_DEFAULT;
#endif
	// default_font_name is got from profile
	win_data->default_font_name = g_strdup("Monospace 12");
	// system_font_name is a fixed value
	// win_data->system_font_name = g_strdup("Monospace 12");
	// win_data->restore_font_name;
	win_data->default_column = SYSTEM_COLUMN;
	win_data->default_row = SYSTEM_ROW;
#ifdef ENABLE_SET_WORD_CHARS
	win_data->word_chars = g_strdup("-A-Za-z0-9_.+!@&=/~%");
#endif
	win_data->show_scroll_bar = AUTOMATIC;
	// 0: left
	// 1: right
	win_data->scroll_bar_position = 1;
#if defined(ENABLE_VTE_BACKGROUND) || defined(FORCE_ENABLE_VTE_BACKGROUND)
	win_data->transparent_background = 2;
	win_data->background_saturation = 0.15;
	// win_data->scroll_background = 0;
	win_data->background_image = g_strdup(NULL_DEVICE);
#endif
	win_data->scrollback_lines = SCROLL_HISTORY;
	win_data->dim_text = TRUE;
#ifdef USE_NEW_VTE_CURSOR_BLINKS_MODE
	// win_data->cursor_blinks = 0;
#else
	win_data->cursor_blinks = TRUE;
#endif
	win_data->allow_bold_text = TRUE;
	win_data->open_url_with_ctrl_pressed = FALSE;
	win_data->disable_url_when_ctrl_pressed = TRUE;
	win_data->audible_bell = TRUE;
#ifdef ENABLE_VISIBLE_BELL
	// win_data->visible_bell = FALSE;
#endif
#ifdef ENABLE_BEEP_SINGAL
	win_data->urgent_bell = TRUE;
#endif
	// win_data->urgent_bell_status = FALSE;
	// win_data->urgent_bell_focus_in_event_id = 0;
	win_data->erase_binding = erase_binding[DEFAULT_ERASE_BINDING].value;
	// win_data->menuitem_erase_binding[ERASE_BINDING];
	// win_data->current_menuitem_erase_binding;
#ifdef ENABLE_CURSOR_SHAPE
	win_data->cursor_shape = cursor_shape[DEFAULT_CURSOR_SHAPE].value;
	// win_data->menuitem_cursor_shape[CURSOR_SHAPE];
	// win_data->current_menuitem_cursor_shape;
#endif
	// win_data->confirm_to_close_multi_tabs = FALSE;
	win_data->confirm_to_kill_running_command = TRUE;
	// win_data->confirm_to_execute_command = TRUE;			// inited in init_prime_user_datas()
	// Don't forget to edit windows.c if you change the default volue here.
	// win_data->execute_command_whitelist = g_strdup("");		// inited in init_prime_user_datas()
	// win_data->execute_command_whitelists
	// win_data->execute_command_in_new_tab = TRUE;			// inited in init_prime_user_datas()
	win_data->foreground_program_whitelist = g_strdup("bash dash csh ksh tcsh zsh screen");
	// win_data->foreground_program_whitelists
	win_data->background_program_whitelist = g_strdup("bash dash csh ksh tcsh zsh su");
	// win_data->background_program_whitelists
	win_data->confirm_to_paste = TRUE;
	win_data->paste_texts_whitelist	= g_strdup("editor vi vim elvis nano emacs emacs23 nano joe ne mg ssh");
	// win_data->paste_texts_whitelists
	win_data->find_string = g_strdup("");
	// win_data->find_case_sensitive;
	// win_data->find_use_perl_regular_expressions;
	// win_data->find_entry_bg_color;
	// win_data->find_entry_current_bg_color;
	// win_data->checking_menu_item;
	// win_data->dialog_activated;
	// win_data->kill_color_demo_vte;
	// win_data->adding_page;
	// win_data->confirmed_profile_is_invalid;
	// win_data->err_str;

	// g_debug ("init_window_parameters(): call init_prime_user_datas() for win_data %p!", win_data);
	init_prime_user_datas(win_data);
}

void init_page_parameters(struct Window *win_data, struct Page *page_data)
{
#ifdef DETAIL
	g_debug("! Launch init_page_parameters() with win_data = %p, page_data = %p", win_data, page_data);
#endif
#ifdef SAFEMODE
	if ((win_data==NULL) || (page_data==NULL)) return;
#endif
	// g_debug("Set page_data->window = win_data->window in init_page_parameters()");
	page_data->window = win_data->window;
	page_data->notebook = win_data->notebook;

//	page_data->page_no = 0;
//	page_data->label = NULL;
//	page_data->hbox = NULL;
//	page_data->vte = NULL;
//	page_data->scroll_bar = NULL;

	page_data->encoding_str = g_strdup(win_data->runtime_encoding);
	// page_data->locale = g_strdup(win_data->runtime_locale);
	page_data->environ = g_strdup("");
	// g_debug("g_strdupv(page_data_prev->environments)!");
	// page_data->environments = g_strdupv(win_data->environments);
	page_data->VTE_CJK_WIDTH_STR = g_strdup(win_data->VTE_CJK_WIDTH_STR);

//	page_data->page_name = NULL;
//	page_data->_t pid = 0;
//	page_data->_t current_tpgid = 0;
//	page_data->_t displayed_tpgid = 0;
//	page_data->custom_page_name = NULL;
//	page_data->tab_color = NULL;
//	page_data->pwd = g_strdup(win_data->init_dir);
//	page_data->is_root = 0;
//	page_data->is_bold = 0;
//	page_data->window_title_updated = 0;
//	page_data->page_update_method = PAGE_METHOD_AUTOMATIC;
//	page_data->window_title_signal = 0;

//	page_data->timeout_id = 0;
//	page_data->urgent_bell_handler_id = 0;

	page_data->font_name = g_strdup(win_data->default_font_name);
//	page_data->font_size = 0;

	// some data came from window. for the performance of monitor_cmdline
	init_monitor_cmdline_datas(win_data, page_data);

//	page_data->use_scrollback_lines = win_data->use_scrollback_lines;

//	page_data->window_title = NULL;

//	page_data->tag[COMMAND] = {{0}};
//	page_data->match_regex_setted = FALSE;
//
}


void init_user_keys(struct Window *win_data)
{
#ifdef DETAIL
	g_debug("! Launch init_user_keys() with win_data = %p", win_data);
#endif
#ifdef SAFEMODE
	if (win_data==NULL) return;
#endif
	// for disable/enable the function keys
	win_data->user_keys[KEY_DISABLE_FUNCTION].value = g_strdup("Ctrl grave");
	// New Page
	win_data->user_keys[KEY_NEW_TAB].value = g_strdup("Ctrl T");
	// ### Close Page, disabled
	// win_data->user_keys[KEY_CLOSE_TAB].value = g_strdup("Ctrl Q");
	// ### Edit Page Name, disabled
	// win_data->user_keys[KEY_EDIT_LABEL].value = g_strdup("Ctrl E");
	// Find String
	win_data->user_keys[KEY_FIND].value = g_strdup("Ctrl F");
	win_data->user_keys[KEY_FIND_NEXT].value = g_strdup("F3");
	win_data->user_keys[KEY_FIND_PREV].value = g_strdup("Shift F3");
	// Prev Page
	win_data->user_keys[KEY_PREV_TAB].value = g_strdup("Ctrl Page_Up");
	// Next Page
	win_data->user_keys[KEY_NEXT_TAB].value = g_strdup("Ctrl Page_Down");
	// First Page
	win_data->user_keys[KEY_FIRST_TAB].value = g_strdup("Ctrl Home");
	// Last Page
	win_data->user_keys[KEY_LAST_TAB].value = g_strdup("Ctrl End");
	// Move the label forward
	win_data->user_keys[KEY_MOVE_TAB_FORWARD].value = g_strdup("Ctrl bracketleft");
	// Move the label backward
	win_data->user_keys[KEY_MOVE_TAB_BACKWARD].value = g_strdup("Ctrl bracketright");
	// Move the label to first
	win_data->user_keys[KEY_MOVE_TAB_FIRST].value = g_strdup("Ctrl Up");
	// Move the label to last
	win_data->user_keys[KEY_MOVE_TAB_LAST].value = g_strdup("Ctrl Down");
	// switch to #%d page
	win_data->user_keys[KEY_SWITCH_TO_TAB_1].value = g_strdup("Ctrl F1");
	win_data->user_keys[KEY_SWITCH_TO_TAB_2].value = g_strdup("Ctrl F2");
	win_data->user_keys[KEY_SWITCH_TO_TAB_3].value = g_strdup("Ctrl F3");
	win_data->user_keys[KEY_SWITCH_TO_TAB_4].value = g_strdup("Ctrl F4");
	win_data->user_keys[KEY_SWITCH_TO_TAB_5].value = g_strdup("Ctrl F5");
	win_data->user_keys[KEY_SWITCH_TO_TAB_6].value = g_strdup("Ctrl F6");
	win_data->user_keys[KEY_SWITCH_TO_TAB_7].value = g_strdup("Ctrl F7");
	win_data->user_keys[KEY_SWITCH_TO_TAB_8].value = g_strdup("Ctrl F8");
	win_data->user_keys[KEY_SWITCH_TO_TAB_9].value = g_strdup("Ctrl F9");
	win_data->user_keys[KEY_SWITCH_TO_TAB_10].value = g_strdup("Ctrl F10");
	win_data->user_keys[KEY_SWITCH_TO_TAB_11].value = g_strdup("Ctrl F11");
	win_data->user_keys[KEY_SWITCH_TO_TAB_12].value = g_strdup("Ctrl F12");
	// #### open new window, disabled
	// win_data->user_keys[KEY_NEW_WINDOW].value = g_strdup("Ctrl N");
	// select all
	win_data->user_keys[KEY_SELECT_ALL].value = g_strdup("Ctrl O");
	// copy the text to clipboard
	win_data->user_keys[KEY_COPY_CLIPBOARD].value = g_strdup("Ctrl Delete");
	// paste the text in clipboard
	win_data->user_keys[KEY_PASTE_CLIPBOARD].value = g_strdup("Ctrl Insert");
	// copy the text in primary clipboard
	win_data->user_keys[KEY_COPY_PRIMARY].value = g_strdup("");
	// paste the text in primary clipboard
	win_data->user_keys[KEY_PASTE_PRIMARY].value = g_strdup("Shift Insert");
	// Increase the font size
	win_data->user_keys[KEY_INCREASE_FONT_SIZE].value = g_strdup("Ctrl equal");
	// decrease the font size
	win_data->user_keys[KEY_DECREASE_FONT_SIZE].value = g_strdup("Ctrl minus");
	// reset the font size
	win_data->user_keys[KEY_RESET_FONT_SIZE].value = g_strdup("Ctrl Return");
	// max window
	win_data->user_keys[KEY_MAX_WINDOW].value = g_strdup("Alt F11");
	// full screen
	win_data->user_keys[KEY_FULL_SCREEN].value = g_strdup("Alt Return");
	// Scroll up/down
	win_data->user_keys[KEY_SCROLL_UP].value = g_strdup("Shift Left");
	win_data->user_keys[KEY_SCROLL_DOWN].value = g_strdup("Shift Right");
	win_data->user_keys[KEY_SCROLL_UP_1_LINE].value = g_strdup("Shift Up");
	win_data->user_keys[KEY_SCROLL_DOWN_1_LINE].value = g_strdup("Shift Down");
	win_data->user_keys[KEY_CLEAN_SCROLLBACK_LINES].value = g_strdup("Ctrl H");
	win_data->user_keys[KEY_DISABLE_URL_L].value = g_strdup("Control_L");
	win_data->user_keys[KEY_DISABLE_URL_R].value = g_strdup("Control_R");
#ifdef FATAL
	// dump_data
	win_data->user_keys[KEY_DUMP_DATA].value = g_strdup("Ctrl Print");
#endif
}

void init_key_bindings_name_and_group()
{
#ifdef DETAIL
	g_debug("! Launch init_key_bindings_name_and_group()!");
#endif
	system_keys[KEY_DISABLE_FUNCTION].name = "disable_key_binding";
	system_keys[KEY_DISABLE_FUNCTION].group = KEY_GROUP_MISC;
	system_keys[KEY_NEW_TAB].name = "new_tab_key";
	system_keys[KEY_NEW_TAB].group = KEY_GROUP_TAB_OPERATION;
	system_keys[KEY_CLOSE_TAB].name = "close_tab_key";
	system_keys[KEY_CLOSE_TAB].group = KEY_GROUP_TAB_OPERATION;
	system_keys[KEY_EDIT_LABEL].name = "edit_label_key";
	system_keys[KEY_EDIT_LABEL].group = KEY_GROUP_TAB_OPERATION;
	system_keys[KEY_FIND].name = "find_key";
	system_keys[KEY_FIND].group = KEY_GROUP_TEXT;
	system_keys[KEY_FIND_PREV].name = "find_key_prev";
	system_keys[KEY_FIND_PREV].group = KEY_GROUP_TEXT;
	system_keys[KEY_FIND_NEXT].name = "find_key_next";
	system_keys[KEY_FIND_NEXT].group = KEY_GROUP_TEXT;
	system_keys[KEY_PREV_TAB].name = "prev_tab_key";
	system_keys[KEY_PREV_TAB].group = KEY_GROUP_SWITCH_TAB;
	system_keys[KEY_NEXT_TAB].name = "next_tab_key";
	system_keys[KEY_NEXT_TAB].group = KEY_GROUP_SWITCH_TAB;
	system_keys[KEY_FIRST_TAB].name = "first_tab_key";
	system_keys[KEY_FIRST_TAB].group = KEY_GROUP_SWITCH_TAB;
	system_keys[KEY_LAST_TAB].name = "last_tab_key";
	system_keys[KEY_LAST_TAB].group = KEY_GROUP_SWITCH_TAB;
	system_keys[KEY_MOVE_TAB_FORWARD].name = "move_tab_forward";
	system_keys[KEY_MOVE_TAB_FORWARD].group = KEY_GROUP_MOVE_TAB;
	system_keys[KEY_MOVE_TAB_BACKWARD].name = "move_tab_backward";
	system_keys[KEY_MOVE_TAB_BACKWARD].group = KEY_GROUP_MOVE_TAB;
	system_keys[KEY_MOVE_TAB_FIRST].name = "move_tab_first";
	system_keys[KEY_MOVE_TAB_FIRST].group = KEY_GROUP_MOVE_TAB;
	system_keys[KEY_MOVE_TAB_LAST].name = "move_tab_last";
	system_keys[KEY_MOVE_TAB_LAST].group = KEY_GROUP_MOVE_TAB;
	gint i;
	for (i=KEY_SWITCH_TO_TAB_1; i<=KEY_SWITCH_TO_TAB_12; i++)
	{
		system_keys[i].name = g_strdup_printf("switch_to_tab_%d", i-KEY_SWITCH_TO_TAB_1+1);
		system_keys[i].group = KEY_GROUP_SWITCH_TO_TAB;
	}
	system_keys[KEY_NEW_WINDOW].name = "new_window";
	system_keys[KEY_NEW_WINDOW].group = KEY_GROUP_MISC;
	system_keys[KEY_SELECT_ALL].name = "select_all";
	system_keys[KEY_SELECT_ALL].group = KEY_GROUP_TEXT;
	system_keys[KEY_COPY_CLIPBOARD].name = "copy_clipboard";
	system_keys[KEY_COPY_CLIPBOARD].group = KEY_GROUP_TEXT;
	system_keys[KEY_PASTE_CLIPBOARD].name = "paste_clipboard";
	system_keys[KEY_PASTE_CLIPBOARD].group = KEY_GROUP_TEXT;
	system_keys[KEY_COPY_PRIMARY].name = "copy_clipboard in primary";
	system_keys[KEY_COPY_PRIMARY].group = KEY_GROUP_TEXT;
	system_keys[KEY_PASTE_PRIMARY].name = "paste_clipboard in primary";
	system_keys[KEY_PASTE_PRIMARY].group = KEY_GROUP_TEXT;
	system_keys[KEY_INCREASE_FONT_SIZE].name = "increase_font_size";
	system_keys[KEY_INCREASE_FONT_SIZE].group = KEY_GROUP_FONT;
	system_keys[KEY_DECREASE_FONT_SIZE].name = "decrease_font_size";
	system_keys[KEY_DECREASE_FONT_SIZE].group = KEY_GROUP_FONT;
	system_keys[KEY_RESET_FONT_SIZE].name = "reset_font_size";
	system_keys[KEY_RESET_FONT_SIZE].group = KEY_GROUP_FONT;
	system_keys[KEY_MAX_WINDOW].name = "max_window";
	system_keys[KEY_MAX_WINDOW].group = KEY_GROUP_MISC;
	system_keys[KEY_FULL_SCREEN].name = "full_screen";
	system_keys[KEY_FULL_SCREEN].group = KEY_GROUP_MISC;
	system_keys[KEY_SCROLL_UP].name = "scroll_up";
	system_keys[KEY_SCROLL_UP].group = KEY_GROUP_SCROLL;
	system_keys[KEY_SCROLL_DOWN].name = "scroll_down";
	system_keys[KEY_SCROLL_DOWN].group = KEY_GROUP_SCROLL;
	system_keys[KEY_SCROLL_UP_1_LINE].name = "scroll_up_1_line";
	system_keys[KEY_SCROLL_UP_1_LINE].group = KEY_GROUP_SCROLL;
	system_keys[KEY_SCROLL_DOWN_1_LINE].name = "scroll_down_1_line";
	system_keys[KEY_SCROLL_DOWN_1_LINE].group = KEY_GROUP_SCROLL;
	system_keys[KEY_CLEAN_SCROLLBACK_LINES].name = "clean_scrollback_lines";
	system_keys[KEY_CLEAN_SCROLLBACK_LINES].group = KEY_GROUP_SCROLL;
	system_keys[KEY_DISABLE_URL_L].name = "disable_url_for_temporary_L";
	system_keys[KEY_DISABLE_URL_L].group = KEY_GROUP_NONE;
	system_keys[KEY_DISABLE_URL_R].name = "disable_url_for_temporary_R";
	system_keys[KEY_DISABLE_URL_R].group = KEY_GROUP_NONE;
#ifdef FATAL
	system_keys[KEY_DUMP_DATA].name = "dump_data";
	system_keys[KEY_DUMP_DATA].group = KEY_GROUP_MISC;
#endif
}

void init_key_bindings()
{
#ifdef DETAIL
	g_debug("! Launch init_key_bindings()!");
#endif

	key_groups[KEY_GROUP_MISC] = _("Misc");
	key_groups[KEY_GROUP_TAB_OPERATION] = _("Tab operation");
	key_groups[KEY_GROUP_SWITCH_TAB] = _("Switch tab");
	key_groups[KEY_GROUP_MOVE_TAB] = _("Move tab");
	key_groups[KEY_GROUP_SWITCH_TO_TAB] = _("Switch to #? tab");
	key_groups[KEY_GROUP_TEXT] = _("Text operation");
	key_groups[KEY_GROUP_SCROLL] = _("Scroll up/down");
	key_groups[KEY_GROUP_FONT] = _("Font");
	key_groups[KEY_GROUP_NONE] = "None";


	// for disable/enable the function keys
	system_keys[KEY_DISABLE_FUNCTION].topic = _("Disable/Enable functions");
	system_keys[KEY_DISABLE_FUNCTION].comment = "# Disable/Enable hyperlinks, function keys and right click menu.";
	system_keys[KEY_DISABLE_FUNCTION].translation = _("Disable/Enable hyperlinks, function keys and right click menu.");
	// New Page
	system_keys[KEY_NEW_TAB].topic = _("Add new tab");
	system_keys[KEY_NEW_TAB].comment = "# Add a new tab.";
	system_keys[KEY_NEW_TAB].translation = _("Add a new tab.");
	// Close Page
	system_keys[KEY_CLOSE_TAB].topic = _("Close tab");
	system_keys[KEY_CLOSE_TAB].comment = "# Close current tab.";
	system_keys[KEY_CLOSE_TAB].translation = _("Close current tab.");
	// Edit Page Name
	system_keys[KEY_EDIT_LABEL].topic = _("Rename page name");
	system_keys[KEY_EDIT_LABEL].comment = "# Rename the page name of current tab.";
	system_keys[KEY_EDIT_LABEL].translation = _("Rename the page name of current tab.");
	// Find
	system_keys[KEY_FIND].topic = _("Find string");
	system_keys[KEY_FIND].comment = "# Find the strings matching the search regex.";
	system_keys[KEY_FIND].translation = _("Find the strings matching the search regex.");
	system_keys[KEY_FIND_PREV].topic = _("Find previous");
	system_keys[KEY_FIND_PREV].comment = "# Find the previous string matching the search regex.";
	system_keys[KEY_FIND_PREV].translation = _("Find the previous string matching the search regex.");
	system_keys[KEY_FIND_NEXT].topic = _("Find next");
	system_keys[KEY_FIND_NEXT].comment = "# Find the next string matching the search regex.";
	system_keys[KEY_FIND_NEXT].translation = _("Find the next string matching the search regex.");
	// Pre Page
	system_keys[KEY_PREV_TAB].topic = _("Switch to prev tab");
	system_keys[KEY_PREV_TAB].comment = "# Switch to prev tab.";
	system_keys[KEY_PREV_TAB].translation = _("Switch to prev tab.");
	// Next Page
	system_keys[KEY_NEXT_TAB].topic = _("Switch to next tab");
	system_keys[KEY_NEXT_TAB].comment = "# Switch to next tab.";
	system_keys[KEY_NEXT_TAB].translation = _("Switch to next tab.");
	// First Page
	system_keys[KEY_FIRST_TAB].topic = _("Switch to first tab");
	system_keys[KEY_FIRST_TAB].comment = "# Switch to first tab.";
	system_keys[KEY_FIRST_TAB].translation = _("Switch to first tab.");
	// Last Page
	system_keys[KEY_LAST_TAB].topic = _("Switch to last tab");
	system_keys[KEY_LAST_TAB].comment = "# Switch to last tab.";
	system_keys[KEY_LAST_TAB].translation = _("Switch to last tab.");
	// Move the label forward
	system_keys[KEY_MOVE_TAB_FORWARD].topic = _("Move page forward");
	system_keys[KEY_MOVE_TAB_FORWARD].comment = "# Move current page forward.";
	system_keys[KEY_MOVE_TAB_FORWARD].translation = _("Move current page forward.");
	// Move the label backward
	system_keys[KEY_MOVE_TAB_BACKWARD].topic = _("Move page backward");
	system_keys[KEY_MOVE_TAB_BACKWARD].comment = "# Move current page backward.";
	system_keys[KEY_MOVE_TAB_BACKWARD].translation = _("Move current page backward.");
	// Move the label to first
	system_keys[KEY_MOVE_TAB_FIRST].topic = _("Move page to first");
	system_keys[KEY_MOVE_TAB_FIRST].comment = "# Move current page to first.";
	system_keys[KEY_MOVE_TAB_FIRST].translation = _("Move current page to first.");
	// Move the label to last
	system_keys[KEY_MOVE_TAB_LAST].topic = _("Move page to last");
	system_keys[KEY_MOVE_TAB_LAST].comment = "# Move current page to last.";
	system_keys[KEY_MOVE_TAB_LAST].translation = _("Move current page to last.");
	// switch to #%d page
	gint i;
	for (i=KEY_SWITCH_TO_TAB_1; i<=KEY_SWITCH_TO_TAB_12; i++)
	{
		system_keys[i].topic = g_strdup_printf(_("Switch to #%d tab"), i-KEY_SWITCH_TO_TAB_1+1);
		system_keys[i].comment = g_strdup_printf("# Switch to #%d tab directly.", i-KEY_SWITCH_TO_TAB_1+1);
		system_keys[i].translation = g_strdup_printf(_("Switch #%d tab directly."), i-KEY_SWITCH_TO_TAB_1+1);
	}

	// new_window
	system_keys[KEY_NEW_WINDOW].topic = _("New window");
	system_keys[KEY_NEW_WINDOW].comment = "# Open a new window with current dir.";
	system_keys[KEY_NEW_WINDOW].translation = _("Open a new window with current dir.");
	// select all
	system_keys[KEY_SELECT_ALL].topic = _("Select all");
	system_keys[KEY_SELECT_ALL].comment = "# Select all the text in the Vte Terminal box.";
	system_keys[KEY_SELECT_ALL].translation = _("Select all the text in the Vte Terminal box.");
	// copy the text to clipboard
	system_keys[KEY_COPY_CLIPBOARD].topic = _("Copy to clipboard");
	system_keys[KEY_COPY_CLIPBOARD].comment = "# Copy the text to clipboard.";
	system_keys[KEY_COPY_CLIPBOARD].translation = _("Copy the text to clipboard.");
	// paste the text in clipboard
	system_keys[KEY_PASTE_CLIPBOARD].topic = _("Paste the text");
	system_keys[KEY_PASTE_CLIPBOARD].comment = "# Paste the text in clipboard.";
	system_keys[KEY_PASTE_CLIPBOARD].translation = _("Paste the text in clipboard to the Vte Terminal box.");
	// copy the text to primary clipboard
	system_keys[KEY_COPY_PRIMARY].topic = _("Copy the text to primary clipboard");
	system_keys[KEY_COPY_PRIMARY].comment = "# Copy the text to the primary clipboard.";
	system_keys[KEY_COPY_PRIMARY].translation = _("Copy the text to the primary clipboard.\nNormally it is copied to the primary clipboard already\nwhen you selected some text with mouse.");
	// paste the text in primary clipboard
	system_keys[KEY_PASTE_PRIMARY].topic = _("Paste the text in primary clipboard");
	system_keys[KEY_PASTE_PRIMARY].comment = "# Paste the text in the primary clipboard.";
	system_keys[KEY_PASTE_PRIMARY].translation = _("Paste the text in the primary clipboard to the Vte Terminal box.\nThe default key binding is <Shift><Insert> in libvte if you disable it here.");
	// Increase the font size
	system_keys[KEY_INCREASE_FONT_SIZE].topic = _("Increase font size");
	system_keys[KEY_INCREASE_FONT_SIZE].comment = "# Increase the font size of current tab.";
	system_keys[KEY_INCREASE_FONT_SIZE].translation = _("Increase the font size of current tab.");
	// decrease the font size
	system_keys[KEY_DECREASE_FONT_SIZE].topic = _("Decrease font size");
	system_keys[KEY_DECREASE_FONT_SIZE].comment = "# Decrease the font size of current tab.";
	system_keys[KEY_DECREASE_FONT_SIZE].translation = _("Decrease the font size of current tab.");
	// reset the font size
	system_keys[KEY_RESET_FONT_SIZE].topic = _("Reset font size");
	system_keys[KEY_RESET_FONT_SIZE].comment = "# Reset the font of current tab to original size.";
	system_keys[KEY_RESET_FONT_SIZE].translation = _("Reset the font of current tab to original size.");
	// max window
	system_keys[KEY_MAX_WINDOW].topic = _("Maximize the window");
	system_keys[KEY_MAX_WINDOW].comment = "# Try to maximize the window to use all available space on your display.";
	system_keys[KEY_MAX_WINDOW].translation = _("Try to maximize the window to use all available space on your display.");
	// full screen
	system_keys[KEY_FULL_SCREEN].topic = _("Full screen");
	system_keys[KEY_FULL_SCREEN].comment = "# Asks to place window in the fullscreen/unfullscreen state.";
	system_keys[KEY_FULL_SCREEN].translation = _("Asks to place window in the fullscreen/unfullscreen state.");
	// scroll up
	system_keys[KEY_SCROLL_UP].topic = _("Scroll up");
	system_keys[KEY_SCROLL_UP].comment = "# Emulate a mouse scroll up event on Vte Terminal box.";
	system_keys[KEY_SCROLL_UP].translation = _("Emulate a mouse scroll up event on Vte Terminal box.");
	// scroll down
	system_keys[KEY_SCROLL_DOWN].topic = _("Scroll down");
	system_keys[KEY_SCROLL_DOWN].comment = "# Emulate a mouse scroll down event on Vte Terminal box.";
	system_keys[KEY_SCROLL_DOWN].translation = _("Emulate a mouse scroll down event on Vte Terminal box.");
	// scroll up 1 line
	system_keys[KEY_SCROLL_UP_1_LINE].topic = _("Scroll up 1 line");
	system_keys[KEY_SCROLL_UP_1_LINE].comment = "# Asks to scroll up 1 line on Vte Terminal box.";
	system_keys[KEY_SCROLL_UP_1_LINE].translation = _("Asks to scroll up 1 line on Vte Terminal box.");
	// scroll down 1 line
	system_keys[KEY_SCROLL_DOWN_1_LINE].topic = _("Scroll down 1 line");
	system_keys[KEY_SCROLL_DOWN_1_LINE].comment = "# Asks to scroll down 1 line on Vte Terminal box.";
	system_keys[KEY_SCROLL_DOWN_1_LINE].translation = _("Asks to scroll down 1 line on Vte Terminal box.");
	// clean scroll history
	system_keys[KEY_CLEAN_SCROLLBACK_LINES].topic = _("Clean scrollback lines");
	system_keys[KEY_CLEAN_SCROLLBACK_LINES].comment = "# Asks to clean scrollback lines.";
	system_keys[KEY_CLEAN_SCROLLBACK_LINES].translation = _("Asks to clean scrollback lines.");
	// disable URL for temporary
	system_keys[KEY_DISABLE_URL_L].topic = "Disable URL for temporary";
	system_keys[KEY_DISABLE_URL_L].comment = "# Disable URL for temporary.";
	system_keys[KEY_DISABLE_URL_L].translation = "Disable URL for temporary.";
	system_keys[KEY_DISABLE_URL_R].topic = "Disable URL for temporary";
	system_keys[KEY_DISABLE_URL_R].comment = "# Disable URL for temporary.";
	system_keys[KEY_DISABLE_URL_R].translation = "Disable URL for temporary.";
#ifdef FATAL
	system_keys[KEY_DUMP_DATA].topic = _("Dump runtime debug data");
	system_keys[KEY_DUMP_DATA].comment = "# Dump the runtime data of LilyTerm for debug.";
	system_keys[KEY_DUMP_DATA].translation = _("Dump the runtime data of LilyTerm for debug.");
#endif
}

void init_page_color_data()
{
#ifdef DETAIL
	g_debug("! Launch init_page_color_data()!");
#endif
	page_color[0].name = "page_win_title_color";
	page_color[0].comment_eng = "Window Title";
	page_color[1].name = "page_cmdline_color";
	page_color[1].comment_eng = "Running Command";
	page_color[2].name = "page_dir_color";
	page_color[2].comment_eng = "Current Dir";
	page_color[3].name = "page_custom_color";
	page_color[3].comment_eng = "Custom Tab Name";
	page_color[4].name = "page_root_color";
	page_color[4].comment_eng = "Root Privileges";
	page_color[5].name = "page_normal_color";
	page_color[5].comment_eng = "Normal Text";
}

void init_page_color_data_comment()
{
#ifdef DETAIL
	g_debug("! Launch init_page_color_data_comment()!");
#endif
	page_color[0].comment = _("Window Title");
	page_color[1].comment = _("Running Command");
	page_color[2].comment = _("Current Dir");
	page_color[3].comment = _("Custom Tab Name");
	page_color[4].comment = _("Root Privileges");
	page_color[5].comment = _("Normal Text");
}

void init_locale_restrict_data(gchar *lc_messages)
{
#ifdef DETAIL
	g_debug("! Launch init_locale_restrict_data() with lc_messages = %s!", lc_messages);
#endif

	// g_debug("lc_messages = %s", lc_messages);
	// g_debug("restricted_locale_message = %s", restricted_locale_message);
	if ((restricted_locale_message == NULL) ||
	    compare_strings(restricted_locale_message, lc_messages, TRUE))
	{
		g_free(restricted_locale_message);
		restricted_locale_message = g_strdup(lc_messages);

		gint i;
		for (i=KEY_SWITCH_TO_TAB_1; i<=KEY_SWITCH_TO_TAB_12; i++)
		{
			g_free(system_keys[i].topic);
			g_free(system_keys[i].comment);
			g_free(system_keys[i].translation);
		}
		init_key_bindings();
		init_page_color_data_comment();
	}
}

void init_mod_keys()
{
#ifdef DETAIL
	g_debug("! Launch init_mod_keys()!");
#endif
	modkeys[0].name = _("Shift");
	modkeys[0].mod = GDK_SHIFT_MASK;
	// modkeys[1].name = "NumLock";
	// modkeys[1].mod = GDK_LOCK_MASK ;
	modkeys[1].name = _("Ctrl");
	modkeys[1].mod = GDK_CONTROL_MASK;
	modkeys[2].name = _("Alt");
	modkeys[2].mod = GDK_MOD1_MASK ;
	// Mod2 = Alt
	// modkeys[2].name = "Mod1";
	// modkeys[2].mod = GDK_MOD1_MASK;
	// Mod2 = NumLock. We won't check it.
	// modkeys[3].name = "Mod2";
	// modkeys[3].mod = GDK_MOD2_MASK ;
	//// modkeys[3].name = "Mod3";
	//// modkeys[3].mod = GDK_MOD3_MASK;
	modkeys[3].name = _("Win");
	modkeys[3].mod = GDK_MOD4_MASK | GDK_SUPER_MASK;
	//// modkeys[5].name = "Mod5";
	//// modkeys[5].mod = GDK_MOD5_MASK ;
	// modkeys[6].name = "Button1";
	// modkeys[6].mod = GDK_BUTTON1_MASK;
	// modkeys[7].name = "Button2";
	// modkeys[7].mod = GDK_BUTTON2_MASK;
	// modkeys[8].name = "Button3";
	// modkeys[8].mod = GDK_BUTTON3_MASK;
	// modkeys[9].name = "Button4";
	// modkeys[9].mod = GDK_BUTTON4_MASK;
	// modkeys[10].name = "Button5";
	// modkeys[10].mod = GDK_BUTTON5_MASK;
	// modkeys[11].name = "Super";
	// modkeys[11].mod = GDK_SUPER_MASK;
	// modkeys[12].name = "Hyper";
	// modkeys[12].mod = GDK_HYPER_MASK;
	// modkeys[13].name = "Meta";
	// modkeys[13].mod = GDK_META_MASK ;
}

void init_colors()
{
#ifdef DETAIL
	g_debug("! Launch init_colors()");
#endif
	color[0].comment = "# The ANSI color code for Dark Black";
	color[1].comment = "# The ANSI color code for Dark Red";
	color[2].comment = "# The ANSI color code for Dark Green";
	color[3].comment = "# The ANSI color code for Dark Yellow";
	color[4].comment = "# The ANSI color code for Dark Blue";
	color[5].comment = "# The ANSI color code for Dark Magenta";
	color[6].comment = "# The ANSI color code for Dark Cyan";
	color[7].comment = "# The ANSI color code for Dark White";
	color[8].comment = "# The ANSI color code for Bright Black";
	color[9].comment = "# The ANSI color code for Bright Red";
	color[10].comment = "# The ANSI color code for Bright Green";
	color[11].comment = "# The ANSI color code for Bright Yellow";
	color[12].comment = "# The ANSI color code for Bright Blue";
	color[13].comment = "# The ANSI color code for Bright Magenta";
	color[14].comment = "# The ANSI color code for Bright Cyan";
	color[15].comment = "# The ANSI color code for Bright White";

	color[0].translation = _("Background Color");
	color[1].translation = _("Dark Red");
	color[2].translation = _("Dark Green");
	color[3].translation = _("Dark Yellow");
	color[4].translation = _("Dark Blue");
	color[5].translation = _("Dark Magenta");
	color[6].translation = _("Dark Cyan");
	color[7].translation = _("Dark White");
	color[8].translation = _("Bright Black");
	color[9].translation = _("Bright Red");
	color[10].translation = _("Bright Green");
	color[11].translation = _("Bright Yellow");
	color[12].translation = _("Bright Blue");
	color[13].translation = _("Bright Magenta");
	color[14].translation = _("Bright Cyan");
	color[15].translation = _("Foreground Color");

	gint i;
	for (i=0; i<COLOR; i++)
		color[i].name = g_strdup_printf("Color%d", i);
}

void init_user_color(struct Window *win_data, gchar *theme_name)
{
#ifdef DETAIL
	g_debug("! Launch init_user_color() with win_data = %p", win_data);
#endif
#ifdef SAFEMODE
	if (win_data==NULL) return;
#endif
	gint i, j;

	// copy colors from system_color_theme to custom_color_theme
	for (i=0; i<THEME; i++)
	{
		// g_debug("(%p) Set win_data->custom_color_theme[%d]->name = %s", win_data, i, system_color_theme[i].name);
		win_data->custom_color_theme[i].index = system_color_theme[i].index + THEME;
		win_data->custom_color_theme[i].name = system_color_theme[i].name;
		for (j=0; j<COLOR; j++)
			win_data->custom_color_theme[i].color[j] = system_color_theme[i].color[j];
	}

	// find the palette for theme_name
	for (i=1; i<THEME; i++)
	{
		if (!compare_strings(theme_name, system_color_theme[i].name, FALSE))
		{
			// g_debug("Get win_data->color_theme_str = %s", win_data->color_theme_str);
			win_data->color_theme_index = i;
			break;
		}
	}
}

gchar *get_user_profile_path(struct Window *win_data, int argc, char *argv[])
{
#ifdef DETAIL
	g_debug("! Launch get_user_profile_path() with win_data = %p, argc = %d", win_data, argc);
	// print_array("\targv", argv);
#endif
	gint i;
	gchar *profile=NULL;

#ifdef SAFEMODE
	if (argv)
	{
#endif
		for (i=0; i<argc; i++)
		{
#ifdef SAFEMODE
			if (argv[i] == NULL) continue;
#endif
			if ((!strcmp(argv[i], "-u")) || (!strcmp(argv[i], "--user_profile")))
			{
				if (++i==argc)
					g_critical("missing file name after -u/--user_profile!\n");
				else
				{
					profile = g_strdup(argv[i]);
					if (win_data)
						win_data->use_custom_profile = TRUE;
				}
				// g_debug ("Using Profile = %s", profile);
			}
			else if ((!strcmp(argv[i], "--specified_profile")))
			{
				if (++i==argc)
					g_critical("missing file name after --specified_profile!\n");
				else
				{
					if (win_data)
						win_data->specified_profile = g_strdup(argv[i]);
				}
				// g_debug ("Using Profile = %s", profile);
			}
			else if ((!strcmp(argv[i], "-e")) ||
				 (!strcmp(argv[i], "-x")) ||
				 (!strcmp(argv[i], "--execute")))
				break;
		}
#ifdef SAFEMODE
	}
#endif
	// trying to got witch profile to use
	if (profile==NULL)
	{
		profile = load_profile_from_dir(profile_dir, USER_PROFILE);
		if (profile==NULL)
			profile = load_profile_from_dir(ETCDIR, PROFILE);
	}
	// if (profile==NULL)
	//	g_message("Sorry, Can not find any profile. Using program defaults.");
	return profile;
}

// The returned string should be freed when no longer needed.
gchar *load_profile_from_dir(const gchar *dir, const gchar* profile)
{
#ifdef DETAIL
	g_debug("! Launch load_profile_from_dir() with dir = %s", dir);
#endif
#ifdef SAFEMODE
	if ((dir==NULL) || (profile==NULL)) return NULL;
#endif
	gchar *profile_path = g_strdup_printf("%s/%s", dir, profile);
#ifdef SAFEMODE
	if (profile_path && g_file_test(profile_path , G_FILE_TEST_EXISTS))
#else
	if ( g_file_test(profile_path , G_FILE_TEST_EXISTS))
#endif
		return profile_path;
	else
	{
		g_free(profile_path);
		return NULL;
	}
}

// get user settings from profile.
// encoding is used for display error message if the profile is invalid.
void get_user_settings(struct Window *win_data, const gchar *encoding)
{
#ifdef DETAIL
	g_debug("! Launch get_user_settings() with win_data = %p, encoding = %s",
		win_data, encoding);
#endif
#ifdef SAFEMODE
	if (win_data==NULL) return;
#endif
#ifdef OUT_OF_MEMORY
#  undef g_strdup
#  undef g_strdup_printf
#  undef g_strsplit
#  undef g_strsplit_set
#endif

	// g_debug("Get win_data = %d when get user settings!", win_data);

	init_window_parameters(win_data);
	init_user_keys(win_data);
	init_user_command(win_data);

	static gboolean static_data_inited = FALSE;
	if (! static_data_inited)
	{
		// Init some static datas.
		init_page_color_data();
		init_key_bindings_name_and_group();
		init_mod_keys();
		init_command();
		init_colors();
		static_data_inited = TRUE;
	}

	gint i;
	GError *error = NULL;

	// got the rc file
	GKeyFile *keyfile = g_key_file_new();
	// g_debug ("Using the profile: %s ", profile);

	gchar *fg_color_str = NULL, *bg_color_str = NULL, *color_theme_str = NULL, *cursor_color_str = NULL;

	// g_debug("safe_mode = %d", safe_mode);
	if (win_data->profile != NULL && (! safe_mode))
	{
		if (g_key_file_load_from_file(keyfile, win_data->profile, G_KEY_FILE_NONE, &error))
		{
			get_prime_user_settings(keyfile, win_data, (gchar *)encoding);

			win_data->auto_save = check_boolean_value(keyfile, "main", "auto_save", win_data->auto_save);

			win_data->default_font_name = check_string_value(keyfile, "main", "font_name", win_data->default_font_name,
									 TRUE, DISABLE_EMPTY_STR);

			PangoFontDescription *font_desc = pango_font_description_from_string(win_data->default_font_name);
			if ((pango_font_description_get_size(font_desc)/PANGO_SCALE)==0)
			{
				g_warning("Invalid font name: \"%s\"", win_data->default_font_name);
				g_free(win_data->default_font_name);
				win_data->default_font_name = g_strdup(SYSTEM_FONT_NAME);
			}

			win_data->default_column = check_integer_value( keyfile, "main", "column", win_data->default_column,
									DISABLE_EMPTY_STR, SYSTEM_COLUMN, DISABLE_ZERO, CHECK_MIN, 1, NO_CHECK_MAX, 0);

			win_data->default_row = check_integer_value(keyfile, "main", "row", win_data->default_row,
								    DISABLE_EMPTY_STR, SYSTEM_ROW, DISABLE_ZERO, CHECK_MIN, 1, NO_CHECK_MAX, 0);
#ifdef USE_GTK3_GEOMETRY_METHOD
			win_data->geometry_width = win_data->default_column;
			win_data->geometry_height = win_data->default_row;
#endif

#ifdef ENABLE_RGBA
			win_data->transparent_window = check_integer_value(keyfile, "main", "transparent_window",
							 win_data->transparent_window, DISABLE_EMPTY_STR, 0, ENABLE_ZERO, CHECK_MIN, 0, CHECK_MAX, 2);

			win_data->use_rgba_orig = check_integer_value(keyfile, "main", "use_rgba",
							 win_data->use_rgba_orig, DISABLE_EMPTY_STR, 0, ENABLE_ZERO, CHECK_MIN, 0, CHECK_MAX, 2);

			win_data->window_opacity = check_double_value ( keyfile, "main", "window_opacity",
									win_data->window_opacity, DISABLE_EMPTY_STR, 0,
									CHECK_MIN, 0, CHECK_MAX, 1);
			win_data->window_opacity_inactive = check_double_value ( keyfile, "main", "window_opacity_inactive",
									win_data->window_opacity_inactive,
									ENABLE_EMPTY_STR, -2,
									CHECK_MIN, 0, CHECK_MAX, 1);
			// g_debug("Got win_data->window_opacity_inactive = %1.3f", win_data->window_opacity_inactive);
#endif
#ifdef USE_GTK2_GEOMETRY_METHOD
			win_data->startup_fullscreen = check_boolean_value(keyfile, "main", "fullscreen",
									   win_data->fullscreen);
#endif
#ifdef USE_GTK3_GEOMETRY_METHOD
			gboolean fullscreen = check_boolean_value(keyfile, "main", "fullscreen", win_data->window_status);
			if (fullscreen) win_data->window_status = WINDOW_START_WITH_FULL_SCREEN;
#endif
#if defined(ENABLE_VTE_BACKGROUND) || defined(FORCE_ENABLE_VTE_BACKGROUND)
			win_data->transparent_background = check_integer_value(keyfile, "main", "transparent_background",
							win_data->transparent_background, DISABLE_EMPTY_STR, 0, ENABLE_ZERO, CHECK_MIN, 0, CHECK_MAX, 2);

			win_data->background_saturation = check_double_value(keyfile, "main", "background_saturation",
									     win_data->background_saturation,
									     DISABLE_EMPTY_STR, 0,
									     CHECK_MIN, 0, CHECK_MAX, 1);

			win_data->scroll_background = check_boolean_value(keyfile, "main", "scroll_background",
									  win_data->scroll_background);

			win_data->background_image = check_string_value(keyfile, "main", "background_image",
									win_data->background_image, TRUE, DISABLE_EMPTY_STR);
#endif
			win_data->foreground_program_whitelist = check_string_value(keyfile, "main",
										    "foreground_program_whitelist",
										    win_data->foreground_program_whitelist,
										    TRUE, ENABLE_EMPTY_STR);

			win_data->background_program_whitelist = check_string_value(keyfile, "main",
										    "background_program_whitelist",
										    win_data->background_program_whitelist,
										    TRUE, ENABLE_EMPTY_STR);

			win_data->confirm_to_paste = check_boolean_value(keyfile, "main",
									 "confirm_to_paste", win_data->confirm_to_paste);

			win_data->paste_texts_whitelist = check_string_value(keyfile, "main",
									    "paste_texts_whitelist",
									    win_data->paste_texts_whitelist,
									    TRUE, ENABLE_EMPTY_STR);

			win_data->confirm_to_close_multi_tabs = check_boolean_value(keyfile, "main",
										    "confirm_to_close_multi_tabs",
										    win_data->confirm_to_close_multi_tabs);

			win_data->confirm_to_kill_running_command = check_boolean_value(keyfile, "main",
											"confirm_to_kill_running_command",
											win_data->confirm_to_kill_running_command);

			win_data->show_background_menu = check_boolean_value(keyfile, "main", "show_background_menu",
									     win_data->show_background_menu);

			win_data->show_color_selection_menu = check_boolean_value(keyfile, "main",
						"show_color_selection_menu", win_data->show_color_selection_menu);

			fg_color_str = check_string_value(keyfile, "main", "foreground_color", DEFAULT_FOREGROUND_COLOR, FALSE, DISABLE_EMPTY_STR);

			bg_color_str = check_string_value(keyfile, "main", "background_color", DEFAULT_BACKGROUND_COLOR, FALSE, DISABLE_EMPTY_STR);

			cursor_color_str = check_string_value(keyfile, "main", "cursor_color", DEFAULT_CURSOR_COLOR, FALSE, DISABLE_EMPTY_STR);

			win_data->custom_cursor_color = ! check_boolean_value(keyfile, "main", "disable_custom_cursor_color",
									      ! win_data->custom_cursor_color);

			if (cursor_color_str == NULL) win_data->custom_cursor_color = FALSE;

			win_data->show_resize_menu = check_boolean_value(keyfile, "main", "show_resize_menu",
									 win_data->show_resize_menu);

#ifndef vte_terminal_set_font_from_string_full
			win_data->font_anti_alias = check_integer_value( keyfile, "main", "font_anti_alias",
					 win_data->font_anti_alias, DISABLE_EMPTY_STR, 0, ENABLE_ZERO, CHECK_MIN, 0, CHECK_MAX, 2);
#endif
			win_data->font_resize_ratio = check_double_value(keyfile, "main", "font_resize_ratio",
									 win_data->font_resize_ratio, DISABLE_EMPTY_STR, 0,
									 CHECK_MIN, 0, NO_CHECK_MAX, 0);

			win_data->window_resize_ratio = check_double_value(keyfile, "main", "window_resize_ratio",
									   win_data->window_resize_ratio,
									   DISABLE_EMPTY_STR, 0,
									   CHECK_MIN, 0, NO_CHECK_MAX, 0);
#ifdef ENABLE_SET_WORD_CHARS
			win_data->word_chars = check_string_value(keyfile, "main", "word_chars", win_data->word_chars,
								  TRUE, ENABLE_EMPTY_STR);
#endif
			win_data->scrollback_lines = check_integer_value(keyfile,
									  "main",
									  "scrollback_lines",
									  win_data->scrollback_lines,
									  DISABLE_EMPTY_STR, 0,
									  ENABLE_ZERO,
									  CHECK_MIN, -1,
									  NO_CHECK_MAX, 0);

			gint show_scroll_bar = check_integer_value(keyfile,
								  "main",
								  "show_scroll_bar",
								  2,
								  DISABLE_EMPTY_STR, 0,
								  ENABLE_ZERO,
								  CHECK_MIN, 0,
								  CHECK_MAX, 2);
			switch (show_scroll_bar)
			{
				case 0:
					win_data->show_scroll_bar = FORCE_OFF;
					break;
				case 1:
					win_data->show_scroll_bar = FORCE_ON;
					break;
				default:
					win_data->show_scroll_bar = AUTOMATIC;
					break;
			}
			// g_debug("Got show_scroll_bar = %d, win_data->show_scroll_bar = %d",
			//	show_scroll_bar, win_data->show_scroll_bar);

			win_data->scroll_bar_position = check_boolean_value(keyfile, "main",
						       "scroll_bar_position", win_data->scroll_bar_position);

#ifdef ENABLE_IM_APPEND_MENUITEMS
			win_data->show_input_method_menu = check_boolean_value(keyfile, "main", "show_input_method_menu",
								     win_data->show_input_method_menu);
#endif
			win_data->show_change_page_name_menu = check_boolean_value(keyfile, "main",
					"show_change_page_name_menu", win_data->show_change_page_name_menu);

			win_data->show_exit_menu = check_boolean_value(keyfile, "main",
					"show_exit_menu", win_data->show_exit_menu);

			win_data->enable_hyperlink = check_boolean_value(keyfile, "main",
							"enable_hyperlink", win_data->enable_hyperlink);
#ifdef USE_NEW_VTE_CURSOR_BLINKS_MODE
			win_data->cursor_blinks = check_integer_value(keyfile,
								      "main",
								      "cursor_blinks",
								      win_data->cursor_blinks,
								      ENABLE_EMPTY_STR, 0,
								      ENABLE_ZERO,
								      CHECK_MIN, 0,
								      CHECK_MAX, 2);
#else
			win_data->cursor_blinks = check_boolean_value(keyfile, "main", "cursor_blinks",
								      win_data->cursor_blinks);
#endif
			win_data->allow_bold_text = check_boolean_value(keyfile, "main", "allow_bold_text",
								  win_data->allow_bold_text);

			win_data->open_url_with_ctrl_pressed = check_boolean_value(keyfile, "main", "open_url_with_ctrl_pressed",
										   win_data->open_url_with_ctrl_pressed);

			win_data->disable_url_when_ctrl_pressed = check_boolean_value(keyfile, "main", "disable_url_when_ctrl_pressed",
										      win_data->disable_url_when_ctrl_pressed);
			if (win_data->disable_url_when_ctrl_pressed) win_data->open_url_with_ctrl_pressed = FALSE;

			win_data->show_copy_paste_menu =
				check_boolean_value(keyfile,
						    "main",
						    "show_copy_paste_menu",
						    win_data->show_copy_paste_menu);

			win_data->embedded_copy_paste_menu =
				check_boolean_value(keyfile,
						    "main",
						    "embedded_copy_paste_menu",
						    win_data->embedded_copy_paste_menu);

			win_data->audible_bell = check_boolean_value(keyfile,
								     "main",
								     "audible_bell",
								     win_data->audible_bell);
#ifdef ENABLE_VISIBLE_BELL
			win_data->visible_bell = check_boolean_value(keyfile,
								     "main",
								     "visible_bell",
								      win_data->visible_bell);
#endif
#ifdef ENABLE_BEEP_SINGAL
			win_data->urgent_bell = check_boolean_value(keyfile,
								    "main",
								    "urgent_bell",
								    win_data->urgent_bell);
#endif
			// g_debug("VTE_ERASE_AUTO = %d, VTE_ERASE_ASCII_BACKSPACE = %d, "
			//	"VTE_ERASE_ASCII_DELETE = %d, VTE_ERASE_DELETE_SEQUENCE %d",
			//	VTE_ERASE_AUTO, VTE_ERASE_ASCII_BACKSPACE,
			//	VTE_ERASE_ASCII_DELETE, VTE_ERASE_DELETE_SEQUENCE);
			win_data->erase_binding = check_integer_value(keyfile, "main",
								      "erase_binding",
								      win_data->erase_binding,
								      DISABLE_EMPTY_STR, 0,
								      ENABLE_ZERO,
								      CHECK_MIN, 0,
								      CHECK_MAX, ERASE_BINDING);
#ifdef ENABLE_CURSOR_SHAPE
			win_data->cursor_shape = check_integer_value(keyfile, "main",
								     "cursor_shape",
								     win_data->cursor_shape,
								     DISABLE_EMPTY_STR, 0,
								     ENABLE_ZERO,
								     CHECK_MIN, 0,
								     CHECK_MAX, CURSOR_SHAPE);
#endif
			win_data->locales_list = check_string_value( keyfile, "main", "locales_list",
								     win_data->locales_list, TRUE, ENABLE_EMPTY_STR);
			// g_debug("Got locales_list = %s from user's profile!", value);

			win_data->default_shell = check_string_value( keyfile, "main", "default_shell",
								      win_data->default_shell, TRUE, DISABLE_EMPTY_STR);
			if (win_data->default_shell)
			{
				g_free(win_data->shell);
				win_data->shell = g_strdup(win_data->default_shell);
			}
#ifdef ENABLE_SET_EMULATION
			win_data->emulate_term = check_string_value( keyfile, "main", "emulate_term",
								     win_data->emulate_term, TRUE, DISABLE_EMPTY_STR);
#endif
			win_data->VTE_CJK_WIDTH = check_integer_value( keyfile, "main", "VTE_CJK_WIDTH",
								       win_data->VTE_CJK_WIDTH,
								       DISABLE_EMPTY_STR, 0,
								       ENABLE_ZERO,
								       CHECK_MIN, 0,
								       CHECK_MAX, 2);

			if (win_data->geometry == NULL)
				win_data->geometry = check_string_value(keyfile, "main", "geometry", win_data->geometry, TRUE, ENABLE_EMPTY_STR);

			win_data->page_width = check_integer_value( keyfile, "page", "page_width",
								    win_data->page_width,
								    DISABLE_EMPTY_STR, 0,
								    DISABLE_ZERO,
								    CHECK_MIN, 1,
								    NO_CHECK_MAX, 0);
			gint show_tabs_bar = check_integer_value( keyfile, "page", "show_tabs_bar",
								  2,
								  DISABLE_EMPTY_STR, 0,
								  ENABLE_ZERO,
								  CHECK_MIN, 0,
								  CHECK_MAX, 2);
			switch (show_tabs_bar)
			{
				case 0:
					win_data->show_tabs_bar = FORCE_OFF;
					break;
				case 1:
					win_data->show_tabs_bar = FORCE_ON;
					break;
				default:
					win_data->show_tabs_bar = AUTOMATIC;
					break;
			}
			// g_debug("Get show_tabs_bar = %d, win_data->show_tabs_bar = %d",
			//	show_tabs_bar, win_data->show_tabs_bar);

			win_data->tabs_bar_position = check_boolean_value(keyfile, "page", "tabs_bar_position",
								     win_data->tabs_bar_position);

			win_data->fill_tabs_bar = check_boolean_value(keyfile, "page", "fill_tabs_bar",
								     win_data->fill_tabs_bar);


			win_data->page_name = check_string_value(keyfile, "page", "page_name", win_data->page_name, TRUE, DISABLE_EMPTY_STR);

			win_data->page_names = check_string_value(keyfile, "page", "page_names",
								  win_data->page_names, TRUE, ENABLE_EMPTY_STR);

			win_data->reuse_page_names = check_boolean_value(keyfile, "page", "reuse_page_names",
									 win_data->reuse_page_names);

			win_data->page_shows_number = check_boolean_value(keyfile, "page", "page_shows_number",
									  win_data->page_shows_number);

			win_data->page_shows_current_cmdline = check_boolean_value(keyfile, "page",
				 "page_shows_current_cmdline", win_data->page_shows_current_cmdline);

			win_data->page_shows_window_title = check_boolean_value(keyfile, "page", "page_shows_window_title",
									 win_data->page_shows_window_title);

			win_data->page_shows_current_dir = check_boolean_value(keyfile, "page", "page_shows_current_dir",
									 win_data->page_shows_current_dir);

			win_data->check_root_privileges = check_boolean_value(keyfile, "page", "check_root_privileges",
								    win_data->check_root_privileges);

			win_data->page_shows_encoding = check_boolean_value(keyfile, "page",
						"page_shows_encoding", win_data->page_shows_encoding);

			win_data->bold_current_page_name = check_boolean_value(keyfile, "page", "bold_current_page_name",
								     win_data->bold_current_page_name);

			win_data->bold_action_page_name = check_boolean_value(keyfile, "page", "bold_action_page_name",
								    win_data->bold_action_page_name);

			win_data->window_title_shows_current_page = check_boolean_value(keyfile, "page",
						"window_title_shows_current_page", win_data->window_title_shows_current_page);

			win_data->window_title_append_package_name = check_boolean_value(keyfile, "page",
					"window_title_append_package_name", win_data->window_title_append_package_name);

			win_data->show_close_button_on_tab = check_boolean_value(keyfile, "page",
						"show_close_button_on_tab", win_data->show_close_button_on_tab);

			win_data->show_close_button_on_all_tabs = check_boolean_value(keyfile, "page",
						"show_close_button_on_all_tabs", win_data->show_close_button_on_all_tabs);

			win_data->use_color_page = check_boolean_value( keyfile, "page", "use_color_page",
									win_data->use_color_page);

			for (i=0; i<PAGE_COLOR; i++)
				win_data->user_page_color[i] = check_string_value(keyfile,
										  "page",
										  page_color[i].name,
										  win_data->user_page_color[i],
										  TRUE,
										  DISABLE_EMPTY_STR);

			// g_debug("Key Value: Shift=%x, NumLock=%x, Control=%x, Mod1=%x,"
			//		"Mod2=%x, Mod3=%x, Mod4=%x, Mod5=%x\n",
			//		GDK_SHIFT_MASK, GDK_LOCK_MASK, GDK_CONTROL_MASK, GDK_MOD1_MASK,
			//		GDK_MOD2_MASK, GDK_MOD3_MASK, GDK_MOD4_MASK, GDK_MOD5_MASK);
			gchar *value = NULL;
			for (i=0; i<KEYS-FIXED_KEYS; i++)
			{
				// g_debug("Checking %s key, default value = %s (%p)...",
				//	system_keys[i].name, win_data->user_keys[i].value, win_data->user_keys[i].value);
				value = g_key_file_get_value(keyfile, "key", system_keys[i].name, NULL);
				// value = check_string_value(keyfile, "key", system_keys[i].name,
				//			   g_strdup(win_data->user_keys[i].value), ENABLE_EMPTY_STR);
				// g_debug("Get %s (%p) from g_key_file_get_value()...", value, value);
				convert_string_to_user_key(i, value, win_data);
			}

			for (i=0; i<REGEX; i++)
			{
				win_data->user_regex[i] = check_string_value(keyfile, "command",
									     regex_name[i], NULL, TRUE, DISABLE_EMPTY_STR);
				// gchar *regex_str = convert_escape_sequence_to_string(win_data->user_regex[i]);
				// g_debug("get_user_settings: Got win_data->user_regex[%d] = %s", i, regex_str);
				// g_free(regex_str);
			}

			if (win_data->user_regex[REGEX_USERNAME] || win_data->user_regex[REGEX_PASSWORD] || win_data->user_regex[REGEX_HOSTNAME] ||
			    win_data->user_regex[REGEX_ADDRESS_BODY] || win_data->user_regex[REGEX_ADDRESS_END])
			{
				gchar *username = (win_data->user_regex[REGEX_USERNAME] == NULL)? USERNAME: win_data->user_regex[REGEX_USERNAME];
				gchar *password = (win_data->user_regex[REGEX_PASSWORD] == NULL)? PASSWORD: win_data->user_regex[REGEX_PASSWORD];
				gchar *hostname = (win_data->user_regex[REGEX_HOSTNAME] == NULL)? HOSTNAME: win_data->user_regex[REGEX_HOSTNAME];
				gchar *address_body = (win_data->user_regex[REGEX_ADDRESS_BODY] == NULL)? ADDRESS_BODY: win_data->user_regex[REGEX_ADDRESS_BODY];
				gchar *address_end = (win_data->user_regex[REGEX_ADDRESS_END] == NULL)? ADDRESS_BODY: win_data->user_regex[REGEX_ADDRESS_END];

				// WWW
				gchar *regex_str = g_strdup_printf("[Hh][Tt][Tt][Pp][Ss]?://(%s%s@)?%s%s(/%s%s)?/*",
								    username, password, hostname, PORT, address_body, address_end);
				win_data->user_command[TAG_WWW].match_regex = convert_escape_sequence_from_string(regex_str);
				g_free(regex_str);

				// FTP
				regex_str = g_strdup_printf("[Ff][Tt][Pp][Ss]?://(%s%s@)?%s%s(/%s%s)?/*",
								    username, password, hostname, PORT, address_body, address_end);
				win_data->user_command[TAG_FTP].match_regex = convert_escape_sequence_from_string(regex_str);
				g_free(regex_str);

				// FILE
				if (win_data->user_regex[REGEX_ADDRESS_BODY] || win_data->user_regex[REGEX_ADDRESS_END])
				{
					regex_str = g_strdup_printf("[Ff][Ii][Ll][Ee]://%s%s", address_body, address_end);
					win_data->user_command[TAG_FILE].match_regex = convert_escape_sequence_from_string(regex_str);
					g_free(regex_str);
				}

				// MAIL
				if (win_data->user_regex[REGEX_USERNAME] || win_data->user_regex[REGEX_HOSTNAME])
				{
					regex_str = g_strdup_printf("([Mm][Aa][Ii][Ll][Tt][Oo]:)?%s@%s", username, hostname);
					win_data->user_command[TAG_MAIL].match_regex = convert_escape_sequence_from_string(regex_str);
					g_free(regex_str);
				}
			}
			// for (i=0; i<COMMAND; i++)
			// {
			//	gchar *regex_str = convert_escape_sequence_to_string(win_data->user_command[i].match_regex);
			//	g_debug("get_user_settings: Got win_data->user_command[%d].match_regex = %s", i, regex_str);
			//	g_free(regex_str);
			// }

			for (i=0; i<COMMAND; i++)
			{
				win_data->user_command[i].command = check_string_value(keyfile, "command",
						command[i].name, win_data->user_command[i].command, TRUE, DISABLE_EMPTY_STR);
				win_data->user_command[i].method = check_integer_value(
						keyfile, "command", command[i].method_name,
						win_data->user_command[i].method,
						DISABLE_EMPTY_STR, 0,
						ENABLE_ZERO,
						CHECK_MIN, 0,
						CHECK_MAX, 2);
				win_data->user_command[i].environ = check_string_value(keyfile, "command",
						command[i].environ_name, win_data->user_command[i].environ, TRUE, DISABLE_EMPTY_STR);
				win_data->user_command[i].VTE_CJK_WIDTH = check_integer_value(
						keyfile, "command", command[i].VTE_CJK_WIDTH_name,
						win_data->user_command[i].VTE_CJK_WIDTH,
						DISABLE_EMPTY_STR, 0,
						ENABLE_ZERO,
						CHECK_MIN, 0,
						CHECK_MAX, 2);
				win_data->user_command[i].locale = check_string_value(keyfile, "command",
						command[i].locale_name, win_data->user_command[i].locale, TRUE, DISABLE_EMPTY_STR);
				// g_debug("command[i].name = %s (%d)",
				//	win_data->user_command[i].command, win_data->user_command[i].method);

				gchar *match_regex = check_string_value(keyfile, "command",
									command[i].match_regex_name,
									NULL,
									TRUE,
									DISABLE_EMPTY_STR);
				win_data->user_command[i].match_regex_orig = convert_escape_sequence_from_string(match_regex);
				g_free(match_regex);

				// g_debug("command[%d].match_regex_orig = %s", i, win_data->user_command[i].match_regex_orig);
			}

			color_theme_str = check_string_value(keyfile, "color", "theme", NULL, FALSE, ENABLE_EMPTY_STR);
			// g_debug("get_user_settings: Got color_theme_str = %s", color_theme_str);
			init_user_color(win_data, color_theme_str);

			win_data->invert_color = check_boolean_value(keyfile, "color", "invert_color", win_data->invert_color);
			win_data->use_custom_theme = check_boolean_value(keyfile, "color", "custom_theme", win_data->use_custom_theme);

#ifdef ENABLE_GDKCOLOR_TO_STRING
			gchar *color_value;
			for (i=1; i<COLOR-1; i++)
			{
				color_value = check_string_value(keyfile, "color", color[i].name, NULL, FALSE, DISABLE_EMPTY_STR);
				// g_debug("color[%d].name = %s, color_value = %s",
				//	i, color[i].name, color_value);
				if (color_value)
				{
					GdkRGBA tmp_color;

					if (check_color_value(color[i].name, color_value, &tmp_color, NULL))
					{
						win_data->have_custom_color = TRUE;
						gint j;
						for (j=0; j<THEME; j++)
							win_data->custom_color_theme[j].color[i] = tmp_color;

						//g_debug("win_data->color_value[%d] = %s, "
						//	"win_data->color[%d] = %x, %x, %x, %x",
						//	i, win_data->color_value[i], i,
						//	win_data->color[i].pixel, win_data->color[i].red,
						//	win_data->color[i].green, win_data->color[i].blue );
					}

				}
				g_free(color_value);
			}
#endif
			win_data->color_brightness = check_double_value(keyfile,
									"color",
									"brightness",
									win_data->color_brightness,
									DISABLE_EMPTY_STR, 0,
									CHECK_MIN, -1, CHECK_MAX, 1);

			win_data->color_brightness_inactive = check_double_value(keyfile,
										 "color",
										 "inactive_brightness",
										 win_data->color_brightness,
										 ENABLE_EMPTY_STR, -2,
										 CHECK_MIN, -1, CHECK_MAX, 1);
		}
		else
		{
			profile_is_invalid_dialog(error, win_data);
			init_user_color(win_data, color_theme_str);
		}
	}
	else
	{
#ifdef FATAL
		g_message("Can NOT find any profile. Use program defaults.");
#else
		g_warning("Can NOT find any profile. Use program defaults.");
#endif
		init_user_color(win_data, color_theme_str);
		safe_mode = FALSE;
	}

	// Set the LC_MESSAGES
	// g_debug("get_user_settings(): win_data->runtime_LC_MESSAGES = %s", win_data->runtime_LC_MESSAGES);
	setlocale(LC_MESSAGES, win_data->runtime_LC_MESSAGES);
	init_locale_restrict_data(win_data->runtime_LC_MESSAGES);

 	for (i=0; i<KEYS; i++)
 	{
		if ( ! win_data->user_keys[i].key)
		{
			accelerator_parse(system_keys[i].name, win_data->user_keys[i].value,
					  &(win_data->user_keys[i].key), &(win_data->user_keys[i].mods));
 			// g_debug("Use default key %s, %x(%s), mods = %x.", pagekeys[i].name,
			//		pagekeys[i].key, gdk_keyval_name(pagekeys[i].key), pagekeys[i].mods);
		}
		// g_debug("* We'll use the key for %s: %x(%s), mods = %x.", pagekeys[i].name,
		//		pagekeys[i].key, gdk_keyval_name(pagekeys[i].key), pagekeys[i].mods);
 	}
	// win_data->fullscreen_show_scroll_bar = win_data->show_scroll_bar;

	for (i=0; i<COMMAND; i++)
	{
		// g_debug("Got the environ = %s", win_data->user_command[i].environ);
		win_data->user_command[i].environments = split_string(win_data->user_command[i].environ, " ", -1);
	}

	// some defaults
	glong column=0, row=0;
	get_row_and_column_from_geometry_str(&column, &row, &(win_data->default_column), &(win_data->default_row), win_data->geometry);
	win_data->default_column = column;
	win_data->default_row = row;

	win_data->splited_page_names = split_string(win_data->page_names, " ", -1);
#ifdef SAFEMODE
	if (win_data->splited_page_names==NULL)
		win_data->splited_page_names = g_strsplit("", " ", -1);
#endif
	win_data->max_page_names_no = 0;
#ifdef SAFEMODE
	if (win_data->splited_page_names)
#endif
		while (win_data->splited_page_names[win_data->max_page_names_no]!=NULL)
			win_data->max_page_names_no++;

	if (win_data->color_brightness_inactive < -1)
	{
		win_data->dim_text = FALSE;
		win_data->color_brightness_inactive = -win_data->color_brightness;
	}
	// g_debug("win_data->dim_text = %d, win_data->color_brightness = %0.3f, win_data->color_brightness_inactive = %0.3f",
	//	win_data->dim_text, win_data->color_brightness, win_data->color_brightness_inactive);

#ifdef ENABLE_RGBA
	// g_debug("Got win_data->window_opacity_inactive = %1.3f", win_data->window_opacity_inactive);
	if (win_data->window_opacity_inactive<-1)
		win_data->window_opacity_inactive = win_data->window_opacity;
	else
		win_data->dim_window = TRUE;
#endif

	if ((win_data->scrollback_lines == 0) && (win_data->show_scroll_bar == AUTOMATIC))
		win_data->show_scroll_bar = OFF;
	// g_debug("FINAL: win_data->show_scroll_bar = %d", win_data->show_scroll_bar);
	if (win_data->show_close_button_on_tab == 0) win_data->show_close_button_on_all_tabs = 0;
	if (win_data->window_title_shows_current_page == 0) win_data->window_title_append_package_name = 0;

	GdkRGBA fg_color, bg_color;
#ifdef ENABLE_GDKCOLOR_TO_STRING
	GdkRGBA cursor_color;

	// g_debug("win_data->foreground_color = %s, win_data->background_color = %s, win_data->cursor_color_str = %s",
	//	win_data->foreground_color, win_data->background_color, win_data->cursor_color_str);

	// Get the color data from profile...
	check_color_value ("foreground_color", fg_color_str, &(fg_color), &(system_color_theme[win_data->color_theme_index].color[COLOR-1]));
	// print_color(COLOR-1, "Get fg_color from profile:", fg_color);
	check_color_value ("foreground_color", bg_color_str, &(bg_color), &(system_color_theme[win_data->color_theme_index].color[0]));
	// print_color(0, "Get bg_color from profile:", bg_color);
	dirty_gdk_color_parse (DEFAULT_CURSOR_COLOR, &(cursor_color));
	check_color_value ("cursor_color", cursor_color_str, &(win_data->cursor_color), &(cursor_color));
	// print_color(-1, "Get cursor_color from profile:", win_data->cursor_color);
#endif

	g_free(fg_color_str);
	g_free(bg_color_str);
	g_free(color_theme_str);
	g_free(cursor_color_str);

#ifdef ENABLE_GDKCOLOR_TO_STRING
	// if the fg_color == bg_color, revert to the default color.
	if (! compare_color(&(fg_color), &(bg_color)))
	{
		// print_color (-1, "invild fg_color = color; fg_color", fg_color);
		// print_color (-1. "invild bg_color = color; bg_color", fg_color);
		dirty_gdk_color_parse (DEFAULT_FOREGROUND_COLOR, &(fg_color));
		dirty_gdk_color_parse (DEFAULT_BACKGROUND_COLOR, &(bg_color));
	}
#endif

	// check if using custom fg_color
	if (compare_color(&(fg_color), &(system_color_theme[win_data->color_theme_index].color[COLOR-1])))
	{
		// print_color(-1, "get_user_settings(): fg_color", fg_color);
		// print_color(-1, "get_user_settings(): win_data->color_orig[COLOR-1]",
		//	    win_data->color_orig[COLOR-1]);
		win_data->have_custom_color = TRUE;
		for (i=0; i<THEME; i++)
			win_data->custom_color_theme[i].color[COLOR-1] = fg_color;
	}

	// check if using custom bg_color
	if (compare_color(&(bg_color), &(system_color_theme[win_data->color_theme_index].color[0])))
	{
		// print_color(-1, "get_user_settings(): bg_color", bg_color);
		// print_color(-1, "get_user_settings(): win_data->color_orig[0]",
		//	    win_data->color_orig[0]);
		win_data->have_custom_color = TRUE;
		for (i=0; i<THEME; i++)
			win_data->custom_color_theme[i].color[0] = bg_color;
	}

	// print_color(-1, "get_user_settings(): win_data->fg_color : ", win_data->fg_color);
	// print_color(-1, "get_user_settings(): win_data->bg_color", win_data->bg_color);
#ifdef ENABLE_GDKCOLOR_TO_STRING
	// check if win_data->cursor_color == win_data->bg_color
	if ((win_data->invert_color && (compare_color(&(fg_color), &(win_data->cursor_color)) == FALSE)) ||
	    ((win_data->invert_color == FALSE) && (compare_color(&(bg_color), &(win_data->cursor_color)) == FALSE)))
		win_data->custom_cursor_color = FALSE;
#endif
	generate_all_color_datas(win_data);

	// g_debug("get_user_settings(): win_data->VTE_CJK_WIDTH_STR = %s", win_data->VTE_CJK_WIDTH_STR);
	if ((win_data->VTE_CJK_WIDTH_STR == NULL) || (win_data->VTE_CJK_WIDTH_STR[0] == '\0'))
	{
		g_free(win_data->VTE_CJK_WIDTH_STR);
		win_data->VTE_CJK_WIDTH_STR = g_strdup(get_VTE_CJK_WIDTH_str(win_data->VTE_CJK_WIDTH));
	}

#ifdef ENABLE_RGBA
	// If the system supports rgba, enable transparent background by default
	// g_debug ("get win_data->use_rgba = %d", win_data->use_rgba);
	if (win_data->use_rgba > -1)
	{
		win_data->use_rgba = win_data->use_rgba_orig;
		// g_debug("use_rgba_orig = %d", win_data->use_rgba_orig);
		init_rgba(win_data);
	}
	if (win_data->transparent_window==2)
		win_data->transparent_window = (win_data->use_rgba==-1)? 1: 0;
#if defined(ENABLE_VTE_BACKGROUND) || defined(FORCE_ENABLE_VTE_BACKGROUND)
	if (win_data->transparent_background==2)
		win_data->transparent_background = (win_data->use_rgba==-1)? 1 :0;
	// g_debug("win_data->transparent_window = %d", win_data->transparent_window);
	// g_debug("win_data->transparent_background = %d", win_data->transparent_background);
#  endif
	set_window_opacity (NULL, 0, win_data->window_opacity, win_data);
#endif

	// win_data->default_encoding_str will be init in add_menuitem_to_encoding_sub_menu() when creating window

	// get the default locale from environment
	// win_data->default_locale = get_default_locale();

//	// g_debug("win_data->default_VTE_CJK_WIDTH = %d in get_user_settings()", win_data->default_VTE_CJK_WIDTH);
//	if (win_data->default_VTE_CJK_WIDTH)
//		win_data->system_VTE_CJK_WIDTH = win_data->default_VTE_CJK_WIDTH;
//	else
//		win_data->system_VTE_CJK_WIDTH = get_default_VTE_CJK_WIDTH();

	if (win_data->execute_command_whitelists==NULL)
	{
		win_data->execute_command_whitelists = split_string(win_data->execute_command_whitelist, " ", -1);
	}
	win_data->foreground_program_whitelists = split_string(win_data->foreground_program_whitelist, " ", -1);
	win_data->background_program_whitelists = split_string(win_data->background_program_whitelist, " ", -1);
	win_data->paste_texts_whitelists = split_string(win_data->paste_texts_whitelist, " ", -1);

	if (win_data->font_resize_ratio <=1) win_data->font_resize_ratio = 0;
	if (win_data->window_resize_ratio <= 1) win_data->window_resize_ratio = 0;

	g_key_file_free(keyfile);
	// the win_data->profile will be free when close window
	// g_free(win_data->profile);

	if (win_data->specified_profile)
	{
		g_free(win_data->profile);
		win_data->profile = win_data->specified_profile;
		win_data->specified_profile = NULL;
	}

#ifdef OUT_OF_MEMORY
	#define g_strdup fake_g_strdup
	#define g_strdup_printf(...) NULL
	#define g_strsplit fake_g_strsplit
	#define g_strsplit_set(x,y,z) NULL
#endif

}

void init_prime_user_datas(struct Window *win_data)
{
#ifdef DETAIL
	g_debug("! Launch init_prime_user_datas() with win_data = %p!", win_data);
#endif
#ifdef SAFEMODE
	if (win_data==NULL) return;
#endif
	// g_debug("init_prime_user_datas(): win_data->prime_user_datas_inited = %d", win_data->prime_user_datas_inited);
	if (win_data->prime_user_datas_inited) return;

	win_data->prime_user_datas_inited = TRUE;

	win_data->warned_locale_list = g_string_new(" ");
	win_data->confirm_to_execute_command = TRUE;
	win_data->execute_command_in_new_tab = TRUE;
	// g_debug("init win_data->execute_command_whitelist (%s) for for win_data (%p)!",
	//	win_data->execute_command_whitelist, win_data);
	win_data->execute_command_whitelist = g_strdup("");
	win_data->default_locale = g_strdup("");
}

void get_prime_user_settings(GKeyFile *keyfile, struct Window *win_data, gchar *encoding)
{
#ifdef DETAIL
	g_debug("! Launch get_prime_user_settings() with keyfile = %p, win_data = %p, encoding = %s!",
		keyfile, win_data, encoding);
#endif
#ifdef SAFEMODE
	if ((keyfile==NULL) || (win_data==NULL)) return;
#endif
	if (win_data->prime_user_settings_inited) return;
	win_data->prime_user_settings_inited = TRUE;

	// Check the profile version
	check_profile_version (keyfile, win_data);

	// Check if confirm_to_execute_command
	win_data->confirm_to_execute_command = check_boolean_value(keyfile, "main", "confirm_to_execute_command",
								   win_data->confirm_to_execute_command);

	win_data->execute_command_whitelist = check_string_value(keyfile, "main",
								 "execute_command_whitelist",
								 win_data->execute_command_whitelist,
								 TRUE,
								 ENABLE_EMPTY_STR);
	// g_debug("win_data->execute_command_whitelist for win_data (%p) updated!", win_data);

	win_data->execute_command_in_new_tab = check_boolean_value(keyfile, "main", "execute_command_in_new_tab",
								   win_data->execute_command_in_new_tab);

	// g_debug("ReGet new win_data->default_locale with original value: %s...", win_data->default_locale);
	win_data->default_locale = check_string_value(keyfile, "main", "default_locale",
						      win_data->default_locale, TRUE, DISABLE_EMPTY_STR);
	// g_debug("win_data->default_locale = %s", win_data->default_locale);
#ifdef SAFEMODE
	if ( win_data->default_locale && (win_data->default_locale[0]!='\0'))
#else
	if (win_data->default_locale[0]!='\0')
#endif
	{
		gchar *encoding = get_encoding_from_locale(win_data->default_locale);
		if (encoding)
		{
			g_free(win_data->default_encoding);
			win_data->default_encoding = encoding;
			g_free(win_data->runtime_LC_MESSAGES);
			win_data->runtime_LC_MESSAGES = g_strdup(win_data->default_locale);
			// g_debug("get_prime_user_settings(): set win_data->runtime_LC_MESSAGES = %s",
			//	win_data->runtime_LC_MESSAGES);
		}
		else
			if (check_and_add_locale_to_warned_locale_list(win_data, win_data->default_locale))
				create_invalid_locale_error_message(win_data->default_locale);

		setlocale(LC_CTYPE, init_LC_CTYPE);
	}
}

gboolean check_boolean_value(GKeyFile *keyfile, const gchar *group_name, const gchar *key, const gboolean default_value)
{
#ifdef DETAIL
	g_debug("! Launch check_boolean_value() with keyfile = %p, group_name = %s, key = %s, default_value = %d",
		keyfile, group_name, key ,default_value);
#endif
#ifdef SAFEMODE
	if ((keyfile==NULL) || (group_name==NULL) || (key==NULL)) return FALSE;
#endif
	gchar *value = g_key_file_get_value(keyfile, group_name, key, NULL);
	gboolean setting;

	if (value)
	{
		if (value[0]!='\0')
			setting = g_key_file_get_boolean(keyfile, group_name, key, NULL);
		else
			setting = default_value;
		g_free(value);
	}
	else
		setting = default_value;

	// g_debug("Got key value \"%s = %d\"", key, setting);
	return setting;
}

gdouble check_double_value(GKeyFile *keyfile, const gchar *group_name, const gchar *key, const gdouble default_value,
			   Check_Empty enable_empty, gdouble empty_value,
			   Check_Min check_min, gdouble min,
			   Check_Max check_max, gdouble max)
{
#ifdef DETAIL
	g_debug("! Launch check_double_value() with keyfile = %p, group_name = %s, key = %s, default_value = %f",
		keyfile, group_name, key, default_value);
#endif
#ifdef SAFEMODE
	if ((keyfile==NULL) || (group_name==NULL) || (key==NULL)) return 0;
#endif
	gchar *value = g_key_file_get_value(keyfile, group_name, key, NULL);
	gdouble setting;

	if (value)
	{
		if (value[0]!='\0')
			setting = g_key_file_get_double(keyfile, group_name, key, NULL);
		else
		{
			// g_debug("enable_empty = %d, empty_value = %1.3f, default_value = %1.3f",
			//	enable_empty, empty_value, default_value);
			if (enable_empty)
			{
				setting = empty_value;
				goto FINISH;
			}
			else
				setting = default_value;
		}
	}
	else
		setting = default_value;

	if (check_min)
		if (setting < min)
			setting = default_value;

	if (check_max)
		if (setting > max)
			setting = default_value;

FINISH:
	g_free(value);
	// g_debug("Got key value \"%s = %1.3f\"", key, setting);
	return setting;
}

// enable_empty -> True: 0, False: default
glong check_integer_value(GKeyFile *keyfile, const gchar *group_name, const gchar *key,
			  const glong default_value, Check_Empty enable_empty, glong empty_value,
			  Check_Zero enable_zero, Check_Min check_min, glong min,
			  Check_Max check_max, glong max)
{
#ifdef DETAIL
	g_debug("! Launch check_integer_value() with keyfile = %p, group_name = %s, key = %s, "
		"default_value = %ld, enable_empty = %d, enable_zero = %d, "
		"check_min = %d, min = %ld, check_max = %d, max = %ld",
		keyfile, group_name, key, default_value, enable_empty, enable_zero,
		check_min, min, check_max, max);
#endif
#ifdef SAFEMODE
	if ((keyfile==NULL) || (group_name==NULL) || (key==NULL)) return 0;
#endif
	gchar *value = g_key_file_get_value(keyfile, group_name, key, NULL);
	gint setting;

	if (value)
	{
		if (value[0]!='\0')
			setting = g_key_file_get_integer(keyfile, group_name, key, NULL);
		else
		{
			if (enable_empty)
			{
				setting = empty_value;
				goto FINISH;
			}
			else
				setting = default_value;
		}
	}
	else
		setting = default_value;

	if (setting==0 && (! enable_zero))
		setting = default_value;

	if (check_min)
		if (setting < min)
			setting = default_value;

	if (check_max)
		if (setting > max)
			setting = default_value;

FINISH:
	g_free(value);
	// g_debug("Got key value \"%s = %d\"", key, setting);
	return setting;
}

// original_value will be free() in check_string_value()
// The returned string should be freed when no longer needed.
gchar *check_string_value(GKeyFile *keyfile, const gchar *group_name, const gchar *key, gchar *original_value,
			  gboolean free_original_value, Check_Empty enable_empty)
{
#ifdef DETAIL
	g_debug("! Launch check_string_value() with keyfile = %p, group_name = %s, "
		"key = %s, original_value = %s (%p), enable_empty = %d",
		keyfile, group_name, key, original_value, original_value, enable_empty);
#endif
#ifdef SAFEMODE
	if ((keyfile==NULL) || (group_name==NULL) || (key==NULL)) return NULL;
#endif
	gchar *setting = g_key_file_get_value(keyfile, group_name, key, NULL);
	// g_debug("check_string_value() with setting = %s (%p)", setting, setting);
	if (setting && (setting[0] == '\0') && (enable_empty==DISABLE_EMPTY_STR))
	{
		g_free(setting);
		setting = NULL;
	}

	if ((setting == NULL) && (original_value))
		setting = g_strdup(original_value);

#ifndef UNIT_TEST
	if (free_original_value) g_free(original_value);
#endif

	// g_debug("Got key value \"%s = %s\"", key, setting);
	return setting;
}

#ifdef ENABLE_GDKCOLOR_TO_STRING
gboolean check_color_value(const gchar *key_name, const gchar *color_name, GdkRGBA *color, const GdkRGBA *default_color)
{
#ifdef DETAIL
	g_debug("! Launch check_color_value() with key_name = %s, color_name = %s, color = %p",
		key_name, color_name, color);
#endif
#ifdef SAFEMODE
	if (color_name==NULL) return FALSE;
#endif
	if ((color_name) && (color_name[0]!='\0'))
	{
		if (dirty_gdk_color_parse(color_name, color))
		{
			// print_color(-1, "1. check_color_value():", *color);
			return TRUE;
		}
	}

	if (color && default_color)
	{
		*color = *default_color;
		// print_color(-1, "2. check_color_value():", *color);
		return TRUE;
	}

#ifdef UNIT_TEST
	g_message("\"%s = %s\" is not a valid color value! Please check!",
		  key_name, color_name);
#else
	g_warning("\"%s = %s\" is not a valid color value! Please check!",
		  key_name, color_name);
#endif
	return FALSE;
}
#endif

// return TRUE if 'key_name' is a valid key or NULL; or it will return FALSE.
gboolean accelerator_parse (const gchar *key_name, const gchar *key_value, guint *key, guint *mods)
{
#ifdef DETAIL
	if (key && mods)
		g_debug("! Launch accelerator_parse() with key_name = %s, key_value = %s, key = %d, mods = %d",
			key_name, key_value, *key, *mods);
	else
		g_debug("! Launch accelerator_parse() with key_name = %s, key_value = %s, key = (%p), mods = (%p)",
			key_name, key_value, key, mods);
#endif
#ifdef SAFEMODE
	if ((key_name==NULL) || (key_value==NULL)) return FALSE;
#endif
	// key_value example: "Ctrl+Shift Home"

	gchar **values = NULL, **functions = NULL;
	gint i=0, j;
	gint tempmods=0;
	gint tempkey=0;
	gboolean response = FALSE;

	// g_debug ("Ken Value = %s",key_value);
	// g_debug ("Checking '%s' is \"\" or NULL...%d, %d, %d",
	//	    key_value, key_value!=NULL, strlen(key_value)>0, g_ascii_strcasecmp(key_value, "NULL"));
	// got the Function key first
	// if ( key_value && (key_value[0]!='\0') && ( compare_strings(key_value, "NULL", FALSE)))
	if (key_value && (key_value[0]!='\0'))
	{
		// g_debug ("Start to checking '%s' in accelerator_parse()...", key_value);
		values = split_string(key_value, " ", -1);
		// split in to "Ctrl+Shift" and "Home" now...
		if (values)
		{
			functions = split_string(values[0], "+", -1);
			// got the function key ("Ctrl+Shift" for example)
#ifdef SAFEMODE
			if (functions)
			{
#endif
				while (functions[i]!=NULL)
				{
					for (j=0; j<MOD; j++)
					{
						// g_debug("dealing %s...", functions[i]);
						if ( ! compare_strings(functions[i], modkeys[j].name, FALSE))
							tempmods |= modkeys[j].mod;
					}
					i++;
				}
#ifdef SAFEMODE
			}
#endif
			// g_debug("masks = %x", tempmods);

			// if (! tempmods )
			// {
			//
			//	g_warning("no Function Key found in [%s] (%s)!", key_name, key_value);
			//	goto FINISH;
			// }
			gchar *check_value = (tempmods && values[1])? values[1]: values[0];

			if (check_value)
			{
				// got the key ("Home" for example)
				tempkey = gdk_keyval_from_name(check_value);
				// g_debug("key = %d", tempkey);
				if ((tempkey == GDK_KEY_VoidSymbol) || (tempkey == 0) ||
				    ((! tempmods) && (tempkey <= GDK_KEY_asciitilde)))
				{
					// not a valid key
					g_warning("\"%s\" in [%s] (%s) is not a valid key!",
						  check_value, key_name, key_value);
					goto FINISH;
				}
				else
				{
					// g_debug("%s (%s, value %x) is a valid key!",
					//		 check_value, gdk_keyval_name(tempkey), tempkey);
					// suppose that the key is always upper case
					if ((tempkey>=GDK_KEY_a) && (tempkey<=GDK_KEY_z))
						tempkey=tempkey-GDK_KEY_a+GDK_KEY_A;
					*key=tempkey;
					// suppose that CapsLock & NumLock always on
					// *mods=(tempmods|GDK_LOCK_MASK|GDK_MOD2_MASK)&GDK_MODIFIER_MASK;
					*mods = tempmods | DUD_MASK;
					response = TRUE;
					goto FINISH;
				}
			}
			// else
			// {
			//	// not a valid key, no space in key_name
			//	g_warning("%s for [%s] is not a valid key. There is no SPACE in it.",
			//		  key_value, key_name);
			//	goto FINISH;
			// }
		}
		goto FINISH;
	}
	else
	{
		// NULL
		// g_message("We Got a NULL Key (%s)!\n", key_value);
#ifdef SAFEMODE
		if ((key==NULL) || (mods==NULL)) goto FINISH;
#endif
		*key=GDK_KEY_VoidSymbol;
		*mods=-1;
		response = TRUE;
	}
FINISH:
	g_strfreev(values);
	g_strfreev(functions);
	return response;
}

// It is OK if widget=NULL and win_data=NULL here.
GString *save_user_settings(GtkWidget *widget, struct Window *win_data)
{
#ifdef DETAIL
	g_debug("! Launch save_user_settings()");
#endif
	struct Page *page_data = NULL;
	gint i;

#ifdef SAFEMODE
	if (win_data && (win_data->current_vte))
#else
	if (win_data)
#endif
	{
		page_data = (struct Page *)g_object_get_data(G_OBJECT(win_data->current_vte), "Page_Data");
#ifdef SAFEMODE
		if (page_data==NULL) return NULL;
#endif
	}
	else
	{
		// For -p command line option
		win_data = g_new0(struct Window, 1);
#ifdef SAFEMODE
		if (win_data==NULL) return NULL;
#endif
		page_data = g_new0(struct Page, 1);
#ifdef SAFEMODE
		if (page_data==NULL)
		{
			// g_debug("save_user_settings(): Trying to free win_data (%p)", win_data);
			g_free(win_data);
			return NULL;
		}
#endif
		// g_debug("Got win_data = %p, page_data = %p", win_data, page_data);
		init_window_parameters(win_data);
		init_user_keys(win_data);
		init_page_color_data();
		init_key_bindings_name_and_group();
		init_locale_restrict_data(NULL);
		init_command();
		init_user_command(win_data);
		init_colors();
		init_user_color(win_data, "");
	}

	gchar *lc_numeric = g_strdup((char*)g_getenv("LC_NUMERIC"));
	setlocale(LC_NUMERIC, "C");

	long column = 0, row = 0;
	GString *contents = g_string_new("[main]\n\n");
	g_string_append_printf(contents,"# Auto save settings when closing window.\n"
					"auto_save = %d\n\n", win_data->auto_save);
	g_string_append_printf(contents,"# The version of this profile's format. DO NOT EDIT IT!\n"
					"version = %s\n\n", PROFILE_FORMAT_VERSION);
	if (menu_active_window)
	{
		g_string_append_printf(contents,"# The default font name of vte terminal.\n"
						"font_name = %s\n\n", page_data->font_name);
		column = vte_terminal_get_column_count(VTE_TERMINAL(page_data->vte));
		g_string_append_printf(contents,"# The default column of vte terminal.\n"
						"column = %ld\n\n", column);
		row = vte_terminal_get_row_count(VTE_TERMINAL(page_data->vte));
		g_string_append_printf(contents,"# The default row of vte terminal.\n"
						"row = %ld\n\n", row);
		g_string_append_printf(contents,"# Use true opacity in vte box.\n"
						"# 0: do NOT use rgba, 1: force to use rgba.\n"
						"# Left it blank will enable it automatically\n"
						"# if the window manager were composited.\n"
						"# Disable it will disable transparent_window, too.\n");
		if (win_data->use_rgba_orig == 2)
			g_string_append(contents, "use_rgba = \n\n");
		else
			g_string_append_printf(contents, "use_rgba = %d\n\n", win_data->use_rgba_orig);
	}
	else
	{
		g_string_append_printf(contents,"# The default font name of vte terminal.\n"
						 "font_name = %s\n\n", SYSTEM_FONT_NAME);
		column = SYSTEM_COLUMN;
		g_string_append_printf(contents,"# The default column of vte terminal.\n"
						 "column = %ld\n\n", column);
		row = SYSTEM_ROW;
		g_string_append_printf(contents,"# The default row of vte terminal.\n"
						 "row = %ld\n\n", row);
		g_string_append_printf(contents,"# Use true opacity in vte box.\n"
						"# 0: do NOT use rgba, 1: force to use rgba.\n"
						"# Left it blank will enable it automatically\n"
						"# if the window manager were composited.\n"
						"# Disable it will disable transparent_window, too.\n"
						"use_rgba = \n\n");
	}
#ifdef USE_GTK2_GEOMETRY_METHOD
	g_string_append_printf(contents,"# Start up with fullscreen.\n"
					"fullscreen = %d\n\n", win_data->true_fullscreen);
#endif
#ifdef USE_GTK3_GEOMETRY_METHOD
	g_string_append_printf(contents,"# Start up with fullscreen.\n"
					"fullscreen = %d\n\n", (win_data->window_status==WINDOW_FULL_SCREEN));
#endif
#ifdef ENABLE_RGBA
	g_string_append_printf(contents,"# Transparent window. Only enabled when the window manager were composited.\n"
					"transparent_window = %d\n\n", (win_data->transparent_window==1));
	g_string_append_printf(contents,"# The opacity of transparent window.\n"
					"window_opacity = %1.3f\n\n", win_data->window_opacity);
	g_string_append_printf(contents,"# The opacity of transparent window when inactive.\n"
					"# Left it blank to disable this feature.\n");
	if (win_data->dim_window)
		g_string_append_printf( contents,"window_opacity_inactive = %1.3f\n\n", win_data->window_opacity_inactive);
	else
		g_string_append(contents,"window_opacity_inactive = \n\n");
#endif
#if defined(ENABLE_VTE_BACKGROUND) || defined(FORCE_ENABLE_VTE_BACKGROUND)
	g_string_append_printf(contents,"# Use transparent background.\n"
					"# It will use true transparent if the window manager were composited.\n"
					"transparent_background = %d\n\n", (win_data->transparent_background==1));
	g_string_append_printf(contents,"# The saturation of transparent background.\n"
					"background_saturation = %1.3f\n\n", win_data->background_saturation);
	g_string_append_printf(contents,"# Scroll the background image along with the text.\n"
					"scroll_background = %d\n\n", win_data->scroll_background);
	if (compare_strings (win_data->background_image, NULL_DEVICE, TRUE))
		g_string_append_printf(contents,"# Sets a background image.\n"
						"background_image = %s\n\n", win_data->background_image);
	else
		g_string_append(contents,"# Sets a background image.\n"
					 "background_image = \n\n");
#endif
	g_string_append_printf(contents,"# Confirm to execute command with -e/-x/--execute option.\n"
					"confirm_to_execute_command = %d\n\n", win_data->confirm_to_execute_command);
	g_string_append_printf(contents,"# Don't need to confirm for executing a program if it's in the whitelist,\n"
					"# separate with <space>.\n"
					"execute_command_whitelist = %s\n\n", win_data->execute_command_whitelist);
	g_string_append_printf(contents,"# Launching executed command in a new tab instead of opening a new window.\n"
					"execute_command_in_new_tab = %d\n\n", win_data->execute_command_in_new_tab);
	g_string_append_printf(contents,"# If a program is running on foreground,\n"
					"# Don't need to confirm for terminating it if it's in the whitelist,\n"
					"# separate with <space>.\n"
					"foreground_program_whitelist = %s\n\n", win_data->foreground_program_whitelist);
	g_string_append_printf(contents,"# If a program is running in background,\n"
					"# Don't need to confirm for terminating it if it's in the whitelist,\n"
					"# separate with <space>.\n"
					"background_program_whitelist = %s\n\n", win_data->background_program_whitelist);
	g_string_append_printf(contents,"# Confirm before pasting texts to vte terminal.\n"
					"confirm_to_paste = %d\n\n", win_data->confirm_to_paste);
	g_string_append_printf(contents,"# If the program is running on foreground,,\n"
					"# Don't need to confirm for pasting texts to it if it's in the whitelist,\n"
					"# separate with <space>.\n"
					"paste_texts_whitelist = %s\n\n", win_data->paste_texts_whitelist);
	g_string_append_printf(contents,"# Confirm to close multi tabs.\n"
					"confirm_to_close_multi_tabs = %d\n\n", win_data->confirm_to_close_multi_tabs);
	g_string_append_printf(contents,"# Confirm to kill running command when exiting.\n"
					"confirm_to_kill_running_command = %d\n\n", win_data->confirm_to_kill_running_command);
	g_string_append_printf(contents,"# Shows [Transparent Background], [Background Saturation]\n"
					"# [Transparent Window] and [Window Opacity] on right click menu.\n"
					"show_background_menu = %d\n\n", win_data->show_background_menu);
	g_string_append_printf(contents,"# Shows [Change the foreground color]\n"
					"# and [Change the background color] on right click menu.\n"
					"show_color_selection_menu = %d\n\n", win_data->show_color_selection_menu);
#ifdef ENABLE_GDKCOLOR_TO_STRING
	g_string_append_printf(contents,"# The normal text color used in vte terminal.\n"
					"# You may use black, #000000 or #000000000000 here.\n");

	if (compare_color(&(win_data->custom_color_theme[win_data->color_theme_index].color[COLOR-1]),
			  &(system_color_theme[win_data->color_theme_index].color[COLOR-1])))
	{
		gchar *color_str = dirty_gdk_rgba_to_string(&(win_data->custom_color_theme[win_data->color_theme_index].color[COLOR-1]));
		g_string_append_printf(contents,"foreground_color = %s\n\n", color_str);
		g_free (color_str);
	}
	else
		g_string_append(contents,"foreground_color = \n\n");

	g_string_append(contents,"# The background color used in vte terminal.\n"
					"# You may use black, #000000 or #000000000000 here.\n");
	if (compare_color(&(win_data->custom_color_theme[win_data->color_theme_index].color[0]),
			  &(system_color_theme[win_data->color_theme_index].color[0])))
	{
		gchar *color_str = dirty_gdk_rgba_to_string(&(win_data->custom_color_theme[win_data->color_theme_index].color[0]));
		g_string_append_printf(contents,"background_color = %s\n\n", color_str);
		g_free (color_str);
	}
	else
		g_string_append(contents,"background_color = \n\n");

	g_string_append_printf(contents,"# Drawn the text under the cursor with foreground and background colors reversed.\n"
					"disable_custom_cursor_color = %d\n\n", ! win_data->custom_cursor_color);
	g_string_append(contents,"# Sets the background color for text which is under the cursor.\n"
				 "# You may use black, #000000 or #000000000000 here.\n");

	gchar *color_str = dirty_gdk_rgba_to_string(&(win_data->cursor_color));
	g_string_append_printf(contents,"cursor_color = %s\n\n", color_str);
	g_free (color_str);
#endif
	g_string_append_printf(contents,"# Shows [Increase window size], [Decrease window size],\n"
					"# [Reset to default font/size] and [Reset to system font/size]\n"
					"# on right click menu.\n"
					"show_resize_menu = %d\n\n", win_data->show_resize_menu);
#ifndef vte_terminal_set_font_from_string_full
	g_string_append_printf(contents,"# Using antialias when showing fonts.\n"
					"# 0: default. 1: force enable. 2: force disable.\n"
					"font_anti_alias = %d\n\n", win_data->font_anti_alias);
#endif
	g_string_append_printf(contents,"# The ratio when resizing font via function key <Ctrl><+> and <Ctrl><->.\n"
					"# 0: the font size is +/- 1 when resizing.\n"
					"font_resize_ratio = %1.3f\n\n", win_data->font_resize_ratio);
	g_string_append_printf(contents,"# The ratio when resizing window via right click menu.\n"
					"# 0: the font size is +/- 1 when resizing window.\n"
					"window_resize_ratio = %1.3f\n\n", win_data->window_resize_ratio);
#ifdef ENABLE_SET_WORD_CHARS
	g_string_append_printf(contents,"# When user double clicks on a text, which character will be selected.\n"
					"word_chars = %s\n\n", win_data->word_chars);
#endif
	g_string_append_printf(contents,"# The lines of scrollback history. -1 means unlimited (vte >= 0.22.3).\n"
					"scrollback_lines = %d\n\n", win_data->scrollback_lines);
	g_string_append(contents,"# Shows scroll_bar or not.\n"
				 "# 0: Never shows the scroll_bar; 1: Always shows the scroll_bar.\n"
				 "# Left it blank: Hide when fullscreen, or scrollback_lines = 0.\n");
	switch (win_data->show_scroll_bar)
	{
		case FORCE_OFF:
			g_string_append(contents,"show_scroll_bar = 0\n\n");
			break;
		case FORCE_ON:
			g_string_append(contents,"show_scroll_bar = 1\n\n");
			break;
		default:
			g_string_append(contents,"show_scroll_bar =\n\n");
			break;
	}

	g_string_append_printf(contents,"# The position of scroll_bar.\n"
					"# 0: scroll_bar is on left; 1: scroll_bar is on right.\n"
					"scroll_bar_position = %d\n\n", win_data->scroll_bar_position);
#ifdef ENABLE_IM_APPEND_MENUITEMS
	g_string_append_printf(contents,"# Shows input method menu on right click menu.\n"
					"show_input_method_menu = %d\n\n", win_data->show_input_method_menu);
#endif
	g_string_append_printf(contents,"# Shows change page name menu on right click menu.\n"
					"show_change_page_name_menu = %d\n\n", win_data->show_change_page_name_menu);
	g_string_append_printf(contents,"# Shows exit menu on right click menu.\n"
					"show_exit_menu = %d\n\n", win_data->show_exit_menu);
	g_string_append_printf(contents,"# Enable hyperlink in vte terminal.\n"
					"enable_hyperlink = %d\n\n", win_data->enable_hyperlink);
	g_string_append_printf(contents,"# Sets whether or not the cursor will blink in vte terminal.\n"
#ifdef USE_NEW_VTE_CURSOR_BLINKS_MODE
					"# 0: Follow GTK+ settings for cursor blinking.\n"
					"# 1: Cursor blinks.\n"
					"# 2: Cursor does not blink.\n"
#endif
					"cursor_blinks = %d\n\n", win_data->cursor_blinks);
	g_string_append_printf(contents,"# Allow bold text in the terminal.\n"
					"allow_bold_text = %d\n\n", win_data->allow_bold_text);
	g_string_append_printf(contents,"# Need <Ctrl> to be pressed to open the URL when it's clicked.\n"
					"open_url_with_ctrl_pressed = %d\n\n", win_data->open_url_with_ctrl_pressed);
	g_string_append_printf(contents,"# Preese <Ctrl> to disable the URL match gregex temporarily.\n"
					"disable_url_when_ctrl_pressed = %d\n\n", win_data->disable_url_when_ctrl_pressed);
	g_string_append_printf(contents,"# Shows copy/paste menu on right click menu.\n"
					"show_copy_paste_menu = %d\n\n",
					win_data->show_copy_paste_menu);
	g_string_append_printf(contents,"# Embed the copy/paste menu to the main menu.\n"
					"embedded_copy_paste_menu = %d\n\n",
					win_data->embedded_copy_paste_menu);
	g_string_append_printf(contents,"# Sets whether or not the terminal will beep\n"
					"# when the child outputs the \"bl\" sequence.\n"
					"audible_bell = %d\n\n", win_data->audible_bell);
#ifdef ENABLE_VISIBLE_BELL
	g_string_append_printf(contents,"# Sets whether or not the terminal will flash\n"
					"# when the child outputs the \"bl\" sequence.\n"
					"visible_bell = %d\n\n", win_data->visible_bell);
#endif
#ifdef ENABLE_BEEP_SINGAL
	g_string_append_printf(contents,"# Sets whether or not the window's urgent tag will be set\n"
					"# when the child outputs the \"bl\" sequence.\n"
					"urgent_bell = %d\n\n", win_data->urgent_bell);
#endif
	g_string_append_printf(contents,"# Which string the terminal should send to an application\n"
					"# when the user presses the Delete or Backspace keys.\n"
					"# 0: VTE_ERASE_AUTO\n"
					"# 1: VTE_ERASE_ASCII_BACKSPACE\n"
					"# 2: VTE_ERASE_ASCII_DELETE\n"
					"# 3: VTE_ERASE_DELETE_SEQUENCE\n"
#ifdef ENABLE_VTE_ERASE_TTY
					"# 4: VTE_ERASE_TTY\n"
#endif
					"erase_binding = %d\n\n", win_data->erase_binding);
#ifdef ENABLE_CURSOR_SHAPE
	g_string_append_printf(contents,"# Sets the shape of the cursor drawn.\n"
					"# 0: VTE_CURSOR_SHAPE_BLOCK\n"
					"# 1: VTE_CURSOR_SHAPE_IBEAM\n"
					"# 2: VTE_CURSOR_SHAPE_UNDERLINE\n"
					"cursor_shape = %d\n\n", win_data->cursor_shape);
#endif
	g_string_append_printf(contents,"# The default locale used when initing a vte terminal.\n"
					"# You may use \"zh_TW\", \"zh_TW.Big5\", or \"zh_TW.UTF-8\" here.\n");
	g_string_append_printf(contents,"default_locale = %s\n\n", win_data->default_locale);
	g_string_append_printf(contents,"# The locales list on right click menu, separate with <space>.\n"
					"# You may use \"ja_JP\", \"ja_JP.EUC-JP\", or \"ja_JP.UTF-8\" here.\n"
					"# You may want to use \"UTF-8\" here if you have no locale data installed.\n"
					"# Left it blank will disable locale and encoding select menu items.\n"
					"locales_list = %s\n\n", win_data->locales_list);
	g_string_append_printf(contents,"# The default shell (for example: /bin/sh) used in LilyTerm.\n"
					"# The setting here will overwrite the SHELL environment.\n");
	if (win_data->default_shell)
		g_string_append_printf(contents,
					"default_shell = %s\n\n", win_data->default_shell);
	else
		g_string_append(contents,
					"default_shell = \n\n");
#ifdef ENABLE_SET_EMULATION
	g_string_append_printf(contents,"# Sets what type of terminal attempts to emulate.\n"
					"# It will also set the TERM environment.\n"
					"# Unless you are interested in this feature, always use \"xterm\".\n"
					"emulate_term = %s\n\n", win_data->emulate_term);
#endif
	g_string_append_printf(contents,"# The environment 'VTE_CJK_WIDTH' used when initing a vte terminal.\n"
					"# 0: get via environment; 1: use narrow ideograph; 2: use wide ideograph.\n"
					"VTE_CJK_WIDTH = %d\n\n", win_data->VTE_CJK_WIDTH);
	g_string_append(contents,"# The geometry of window when starting.\n"
				 "# A reasonable example value is \"80x24+0+0\",\n"
				 "# witch means \"WIDTH x HEIGHT {+-} XOFFSET {+-} YOFFSET\", and NO SPACE in it.\n"
				 "# Notice that it will overwrite the default column and row settings above.\n");
	if ((win_data->geometry) && (win_data->geometry[0]!='\0'))
	{
		gint xoffset = 0, yoffset = 0;
		gtk_window_get_position (GTK_WINDOW(win_data->window), &xoffset, &yoffset);
		// g_debug("Geto window position = %+d, %+d", xoffset, yoffset);
		g_string_append_printf(contents,"geometry = %ldx%ld%+d%+d\n\n",
				       column, row, xoffset, yoffset);
	}
	else
		g_string_append(contents,"geometry = \n\n");
	g_string_append_printf(contents,"\n");
	g_string_append_printf(contents,"[page]\n\n");
	g_string_append_printf(contents,"# The max character width of page name.\n"
					"page_width = %d\n\n", win_data->page_width);
	g_string_append_printf(contents,"# Show the tabs bar or not.\n"
					"# 0: Never shows the tabs ; 1: Always shows the tabs bar.\n"
					"# Left it blank: Hide when fullscreen, or tabs number = 1.\n");
	switch (win_data->show_tabs_bar)
	{
		case FORCE_OFF:
			g_string_append(contents, "show_tabs_bar = 0\n\n");
			break;
		case FORCE_ON:
			g_string_append(contents, "show_tabs_bar = 1\n\n");
			break;
		default:
			g_string_append(contents, "show_tabs_bar =\n\n");
			break;
	}
	g_string_append_printf(contents,"# The position of tabs bar.\n"
					"# 0: Top, 1: bottom.\n"
					"tabs_bar_position = %d\n\n", win_data->tabs_bar_position);
	g_string_append_printf(contents,"# The label of tabs will fill the tab bar.\n"
					"fill_tabs_bar = %d\n\n", win_data->fill_tabs_bar);
	g_string_append_printf(contents,"# The page name used for a new page.\n"
					"page_name = %s\n\n", win_data->page_name);
	g_string_append_printf(contents,"# The page names list used for new pages, separate with <space>.\n"
					"page_names = %s\n\n", win_data->page_names);
	g_string_append_printf(contents,"# Reuse the page name in the page names list.\n"
					"reuse_page_names = %d\n\n", win_data->reuse_page_names);
	g_string_append_printf(contents,"# Shows a (number no) on the page name.\n"
					"page_shows_number = %d\n\n", win_data->page_shows_number);
	g_string_append_printf(contents,"# Shows the foreground running command on the page name.\n"
					"page_shows_current_cmdline = %d\n\n", win_data->page_shows_current_cmdline);
	g_string_append_printf(contents,"# Shows the terminal's idea of what the window's title should be.\n"
					"page_shows_window_title = %d\n\n", win_data->page_shows_window_title);
	g_string_append_printf(contents,"# Shows current directory on the page name.\n"
					"page_shows_current_dir = %d\n\n", win_data->page_shows_current_dir);
	g_string_append_printf(contents,"# Check if the running command is root privileges.\n"
					"check_root_privileges = %d\n\n", win_data->check_root_privileges);
	g_string_append_printf(contents,"# Shows current encoding on the page name.\n"
					"page_shows_encoding = %d\n\n", win_data->page_shows_encoding);
	g_string_append_printf(contents,"# Bold the text of current page name.\n"
					"bold_current_page_name = %d\n\n", win_data->bold_current_page_name);
	g_string_append_printf(contents,"# Bold the text of action page name.\n"
					"bold_action_page_name = %d\n\n", win_data->bold_action_page_name);
	g_string_append_printf(contents,"# Shows the page name of current page on window title.\n"
					"window_title_shows_current_page = %d\n\n", win_data->window_title_shows_current_page);
	g_string_append_printf(contents,"# Append a package name (- LilyTerm) to the window title.\n"
					"window_title_append_package_name = %d\n\n", win_data->window_title_append_package_name);
	g_string_append_printf(contents,"# Shows a close button [X] on current tab.\n"
					"show_close_button_on_tab = %d\n\n", win_data->show_close_button_on_tab);
	g_string_append_printf(contents,"# Shows a close button [X] on all tabs.\n"
					"show_close_button_on_all_tabs = %d\n\n", win_data->show_close_button_on_all_tabs);
	g_string_append_printf(contents,"# Use colorful text on page.\n"
					"use_color_page = %d\n\n", win_data->use_color_page);
	for (i=0; i<PAGE_COLOR; i++)
	{
		g_string_append_printf( contents,
					"# The color used for showing %s on page name.\n"
					"# You may use black, #000000 or #000000000000 here.\n"
					"%s = %s\n\n",
					page_color[i].comment_eng, page_color[i].name, win_data->user_page_color[i]);
	}
	g_string_append_printf(contents,"\n[key]\n\n");
	for (i=0; i<KEYS-FIXED_KEYS; i++)
	{
		if (win_data->user_keys[i].value && (win_data->user_keys[i].value[0]!='\0'))
			g_string_append_printf( contents,"%s\n# Left it blank to disable this function key.\n%s = %s\n\n",
					system_keys[i].comment, system_keys[i].name, win_data->user_keys[i].value);
		else
			g_string_append_printf( contents,"%s\n# Left it blank to disable this function key.\n%s = \n\n",
					system_keys[i].comment, system_keys[i].name);
	}
	g_string_append_printf(contents,"\n");
	g_string_append_printf(contents,"[color]\n\n");
	g_string_append_printf(contents,"# The main ansi color theme used in vte.\n"
					"# Possible values are linux, xterm, rxvt, and tango.\n"
					"# or left it blank to use the default settings form libvte.\n");
	if (win_data->color_theme_index)
		g_string_append_printf(contents, "theme = %s\n\n", system_color_theme[win_data->color_theme_index].name);
	else
		g_string_append(contents, "theme = \n\n");
	g_string_append_printf(contents,"# Invert the ansi colors, like invert the darkred to red, darkblue to bule.\n"
					"invert_color = %d\n\n", win_data->invert_color);
	g_string_append_printf(contents,"# Enable the custom colors specified with Color# below.\n"
					"custom_theme = %d\n\n", win_data->use_custom_theme);
	g_string_append_printf(contents,"# The brightness for ansi colors used in terminal.\n"
					"brightness = %1.3f\n\n", win_data->color_brightness);
	g_string_append_printf(contents,"# The brightness for ansi colors used in terminal when inactive.\n"
					"# Left it blank to disable this feature.\n");
	if (win_data->dim_text)
		g_string_append_printf( contents,
					"inactive_brightness = %1.3f\n\n", win_data->color_brightness_inactive);
	else
		g_string_append(contents, "inactive_brightness = \n\n");

#ifdef ENABLE_GDKCOLOR_TO_STRING
	for (i=1; i<COLOR-1; i++)
	{
		g_string_append_printf( contents,"%s\n"
					"# You may use black, #000000 or #000000000000 here.\n", color[i].comment);

		if (compare_color(&(win_data->custom_color_theme[win_data->color_theme_index].color[i]),
			  &(system_color_theme[win_data->color_theme_index].color[i])))
		{
			color_str = dirty_gdk_rgba_to_string(&(win_data->custom_color_theme[win_data->color_theme_index].color[i]));
			g_string_append_printf(contents,"%s = %s\n\n",  color[i].name, color_str);
			g_free (color_str);
		}
		else
			g_string_append_printf(contents,"%s = \n\n",  color[i].name);
	}
#endif
	g_string_append_printf(contents,"\n");

	g_string_append_printf(contents,"[command]\n\n"
					"# The parameters of the APPLICATION should be separated with <tab>, if any.\n"
					"#\n"
					"# method = {0,1,2}\n"
					"# 0: Open the hyperlink in new tab.\n"
					"#    Use it if the command were using CLI, like w3m.\n"
					"# 1: Open the hyperlink with gdk_spawn_on_screen_with_pipes().\n"
					"#    Use it if the command were using GUI, like chromium.\n"
					"# 2: Open the hyperlink in new window,\n"
					"#    Use it if you not sure.\n"
					"#\n"
					"# VTE_CJK_WIDTH = {0,1,2}\n"
					"# 0: get via environment\n"
					"# 1: use narrow ideograph\n"
					"# 2: use wide ideograph.\n"
					"#\n"
					"# The ENVIRONS will apply to the application, separated with <tab>, too.\n"
					"#\n"
					"# The LOCALE will apply to the application as locale environs.\n"
					"# You may use \"zh_TW\", \"zh_TW.Big5\", or \"zh_TW.UTF-8\" here.\n"
					"# Left it blank to use the locale environs from current page.\n\n");

	for (i=0; i<REGEX; i++)
	{
		gchar *match_str=convert_escape_sequence_to_string(regex_str[i]);
		g_string_append_printf( contents,"# Default: %s = %s\n",
					regex_name[i], match_str);
		g_free(match_str);
		if (win_data->user_regex[i])
			g_string_append_printf( contents,"%s = %s\n\n", regex_name[i], win_data->user_regex[i]);
		else
			g_string_append_printf( contents,"%s = \n\n", regex_name[i]);
	}

	for (i=0; i<COMMAND; i++)
	{
		g_string_append_printf( contents,"%s\n%s = %s\n",
					command[i].comment, command[i].name, win_data->user_command[i].command);
		g_string_append_printf( contents,"%s = %d\n",
					command[i].method_name, win_data->user_command[i].method);
		g_string_append_printf( contents,"%s = %d\n",
					command[i].VTE_CJK_WIDTH_name, win_data->user_command[i].VTE_CJK_WIDTH);
		g_string_append_printf( contents,"%s = %s\n",
					command[i].environ_name, win_data->user_command[i].environ);
		g_string_append_printf( contents,"%s = %s\n",
					command[i].locale_name, win_data->user_command[i].locale);
		gchar *match_str=convert_escape_sequence_to_string(command[i].match);
		g_string_append_printf( contents,"# Default: %s = %s\n",
					command[i].match_regex_name, match_str);
		g_free(match_str);
		if (win_data->user_command[i].match_regex_orig)
			g_string_append_printf( contents,"%s = %s\n\n",
						command[i].match_regex_name, win_data->user_command[i].match_regex_orig);
		else
			g_string_append_printf( contents,"%s = \n\n",
						command[i].match_regex_name);
	}

	// g_debug("menu_active_window = %p", menu_active_window);
	if (menu_active_window==NULL)
		// For -p command line option
		// LilyTerm will exit immediately, so we don't need to free the datas used by LilyTerm. XD
		return contents;

	gchar *profile = NULL;
	GKeyFile *keyfile = g_key_file_new();
	GError *error = NULL;

	if (win_data->use_custom_profile)
		profile = g_strdup(win_data->profile);
	else if (menu_active_window)
	{
		profile = get_profile();
		g_free(win_data->profile);
		win_data->profile = g_strdup(profile);
	}
	if (profile==NULL) goto FINISH;
	// g_debug("save_user_settings(): profile = %s", profile);

	// g_debug("\n%s", contents->str);
#ifdef BSD
	gchar resolved_patch[PATH_MAX+1];
	gchar *real_file_name = g_strdup(realpath((const gchar *)profile, resolved_patch));
#else
	gchar *real_file_name = canonicalize_file_name((const gchar *)profile);
#endif
	if (real_file_name==NULL)
	{
		//if (access(profile, F_OK))
		if (g_file_test(profile, G_FILE_TEST_IS_SYMLINK))
		{
			create_save_failed_dialog(win_data, _("The target file does NOT exist"));
			goto FINISH;
		}
		else
			real_file_name = g_strdup(profile);
	}

	// g_debug("real_file_name = %s", real_file_name);
	if (g_file_test(real_file_name, G_FILE_TEST_IS_DIR))
	{
		// g_debug("The target file is a directory!!");
		create_save_failed_dialog(win_data, _("The target file is a directory"));
	}
	else
	{
		// g_debug("G_FILE_TEST_IS_REGULAR = TRUE");
#ifdef SAFEMODE
		if ( contents &&
		     (! g_file_set_contents(real_file_name, contents->str, -1, &error)))
#else
		if ( ! g_file_set_contents(real_file_name, contents->str, -1, &error))
#endif
		{
			// g_debug("Error while writing profile '%s': %s", real_file_name, error->message);
#ifdef SAFEMODE
			if (error)
#endif
				create_save_failed_dialog(win_data, error->message);
		}
		// else
		//	g_debug("The settings has been saved to %s!", real_file_name);
	}
	g_free(real_file_name);

FINISH:
	if (error!=NULL) g_clear_error(&error);
	g_free(profile);
	if (contents) g_string_free(contents, TRUE);
	g_key_file_free(keyfile);

	setlocale(LC_NUMERIC, lc_numeric);
	g_free(lc_numeric);

	return NULL;
}

void create_save_failed_dialog(struct Window *win_data, gchar *message)
{
#ifdef DETAIL
	g_debug("! Launch create_save_failed_dialog() with win_data = %p, message = %s!", win_data, message);
#endif
#ifdef SAFEMODE
	if (win_data==NULL) return;
#endif
	GtkWidget *window = NULL;
	if (win_data) window = win_data->window;

	gchar *temp_str[3];
	temp_str[0] = get_colorful_profile(win_data);
	temp_str[1] = g_strdup_printf(_("Error while writing profile '%s':"), temp_str[0]);
	temp_str[2] = g_strconcat(temp_str[1], "\n\n\t", message, "\n", NULL);

	// error_dialog(GtkWidget *window, gchar *title, gchar *icon, gchar *message, gchar *encoding)
	error_dialog(window,
		     _("Error when writing profile"),
		     "Error when writing profile",
		     GTK_FAKE_STOCK_DIALOG_ERROR,
		     temp_str[2],
		     NULL);
	gint i;
	for (i=0; i<3; i++)
		g_free(temp_str[i]);
}

gchar *get_profile()
{
#ifdef DETAIL
	g_debug("! Launch get_profile()");
#endif
	gchar *profile = g_strdup_printf("%s/%s", profile_dir, USER_PROFILE);
#ifdef SAFEMODE
	if (profile && (g_mkdir_with_parents(profile_dir, 0700) < 0))
#else
	if (g_mkdir_with_parents(profile_dir, 0700))
#endif
	{
#ifdef SAFEMODE
		g_message("Can NOT create the directory: %s", profile_dir);
#else
		g_critical("Can NOT create the directory: %s", profile_dir);
#endif
		g_free(profile);
		profile = NULL;
	}
	// g_debug("get_profile: got the final profile = %s", profile);
	return profile;
}

#if defined(ENABLE_RGBA) || defined(UNIT_TEST)
// init rgba to enable true transparent.
void init_rgba(struct Window *win_data)
{
#ifdef DETAIL
	if (win_data)
		g_debug("! Launch init_rgba() with win_data = %p, window = %p, win_data->use_rgba = %d",
			win_data, win_data->window, win_data->use_rgba);
	else
		g_debug("! Launch init_rgba() with win_data = %p", win_data);
#endif
#ifdef SAFEMODE
	if ((win_data==NULL) || (win_data->window==NULL)) return;
#endif
	if (win_data->use_rgba < 0) return;

	// g_debug("Get win_data = %d when initing rgba!", win_data);

	GdkScreen *screen = gtk_widget_get_screen(win_data->window);

	if (win_data->use_rgba==2)
#ifdef GTK3_LAME_GDK_SCREEN_IS_COMPOSITED
	{
		if (screen)
#endif
#ifdef ENABLE_RGBA
			win_data->use_rgba = gdk_screen_is_composited(screen);
#endif
#ifdef GTK3_LAME_GDK_SCREEN_IS_COMPOSITED
		else
			goto FINISH;
	}
#endif

	// if (win_data->use_rgba)
	//	g_debug("Yes, This screen supports rgba!");
	// else
	//	g_debug("No, This screen don't supports rgba!");

	if (win_data->use_rgba)
	{
#ifdef USE_GDK_SCREEN_GET_RGBA_VISUAL
#  ifdef GTK3_LAME_GDK_SCREEN_GET_RGBA_VISUAL
		// Due to the lame GTK3+!!!
		if (screen)
		{
#  endif
			GdkVisual *visual = gdk_screen_get_rgba_visual (screen);
			if (visual)
			{
				gtk_widget_set_visual (GTK_WIDGET (win_data->window), visual);
				win_data->use_rgba = -1;
				return;
			}
#  ifdef GTK3_LAME_GDK_SCREEN_GET_RGBA_VISUAL
		}
#  endif
#else
		GdkColormap *colormap = gdk_screen_get_rgba_colormap(screen);
		if (colormap)
		{
			gtk_widget_set_colormap(win_data->window, colormap);
			gdk_screen_set_default_colormap(screen, colormap);
			gtk_widget_set_default_colormap(colormap);
			win_data->use_rgba = -1;
			return;
		}
#endif
	}
#ifdef GTK3_LAME_GDK_SCREEN_IS_COMPOSITED
FINISH:
#endif
	win_data->use_rgba = -2;
	win_data->dim_window = FALSE;
//	g_debug("init_rgba() finished!");
}
#endif

void check_profile_version (GKeyFile *keyfile, struct Window *win_data)
{
#ifdef DETAIL
	g_debug("! Launch check_profile_version() with win_data = %p", win_data);
#endif
#ifdef SAFEMODE
	if ((keyfile==NULL) || (win_data==NULL)) return;
#endif
	GtkWidget *window = NULL;
	if (win_data) window = win_data->window;

	// if (win_data->checked_profile_version)
	if (checked_profile_version) return;
	// Check the profile version
	gchar *profile_version = check_string_value(keyfile, "main", "version", "", FALSE, DISABLE_EMPTY_STR);
	// g_debug("PROFILE_FORMAT_VERSION = %s, and profile_version = %s",
	//	PROFILE_FORMAT_VERSION,
	//	profile_version);

#ifdef SAFEMODE
	if (profile_version && (profile_version[0] != '\0') &&
	    (compare_strings(PROFILE_FORMAT_VERSION, profile_version, TRUE)))
#else
	if ((profile_version[0] != '\0') &&
	    (compare_strings(PROFILE_FORMAT_VERSION, profile_version, TRUE)))
#endif
	{
		gchar *temp_str[3] = {NULL};
		temp_str[0] = g_strdup(_("Some entry in profile is added, removed, "
					 "or changed in this version.\n"
					 "Please use [Save settings] on the right click menu "
					 "to save your settings,\n"
					 "and edit it manually if necessary.\n"));
		temp_str[1] = get_colorful_profile(win_data);
		temp_str[2] = g_strdup_printf(_("%s\nYour profile is %s."),
						 temp_str[0], temp_str[1]);
		error_dialog(window,
			     _("The format of your profile is out of date"),
			     "The format of your profile is out of date",
			     GTK_FAKE_STOCK_DIALOG_INFO,
			     temp_str[2],
			     NULL);
		gint i;
		for (i=0; i<3; i++)
			g_free(temp_str[i]);
#if !defined(DEBUG) && !defined(DETAIL) && !defined(FULL)
		checked_profile_version = TRUE;
#endif
	}
	g_free(profile_version);
}

void profile_is_invalid_dialog(GError *error, struct Window *win_data)
{
#ifdef DETAIL
	g_debug("! Launch profile_is_invalid_dialog() with win_data = %p", win_data);
#endif
#ifdef SAFEMODE
	if ((error==NULL) || (win_data==NULL)) return;
#endif
	if (win_data->confirmed_profile_is_invalid) return;

	gchar *err_msg;
	gchar *local_profile = get_colorful_profile(win_data);
	if (local_profile)
		err_msg = g_strdup_printf(_("The profile \"%s\" is invalid:\n\n\t%s"),
					    local_profile, error->message);
	else
		err_msg = g_strdup_printf(_("The profile \"%s\" is invalid:\n\n\t%s"),
					    win_data->profile, error->message);
	// g_warning("the profile \"%s\" is invalid: %s", win_data->profile, error->message);

	// gchar *markup_message = g_markup_escape_text(err_msg, -1);

	// error_dialog(win_data, err_msg, NULL, ERROR_MESSAGES);
	error_dialog(win_data->window,
		     _("The profile is invalid!"),
		     "The profile is invalid!",
		     GTK_FAKE_STOCK_DIALOG_ERROR,
		     err_msg,
		     NULL);
	g_clear_error (&error);
	g_free(local_profile);
	g_free(err_msg);
	// g_free(markup_message);
	win_data->confirmed_profile_is_invalid = TRUE;
}

void convert_string_to_user_key(gint i, gchar *value, struct Window *win_data)
{
#ifdef DETAIL
	g_debug("! Launch convert_string_to_user_key() with i = %d, value = %s (%p), win_data = %p!",
		i, value, value, win_data);
#endif
#ifdef SAFEMODE
	if (win_data==NULL) return;
#endif

	if (value)
	{
		// g_debug("Got %s key = %s form profile.", pagekeys[i].name, value);
		// g_debug("Call accelerator_parse() with system_keys[i].name = %s, value = %s (%p)",
		//	system_keys[i].name, value, value);
		if (accelerator_parse(system_keys[i].name, value,
				      &(win_data->user_keys[i].key), &(win_data->user_keys[i].mods)))
		{
			g_free(win_data->user_keys[i].value);
			win_data->user_keys[i].value = value;
		}
		else
		{
#ifdef UNIT_TEST
			g_debug("\"%s = %s\" is not a valid key! Please check!",
				system_keys[i].name, value);
#else
			g_critical("\"%s = %s\" is not a valid key! Please check!",
				   system_keys[i].name, value);
#endif
#ifndef UNIT_TEST
			g_free(value);
#endif
		}
		// if (pagekeys[i].key)
		//	g_debug("Got %s = key %x(%s), mods = %x.", pagekeys[i].name,
		//			pagekeys[i].key, gdk_keyval_name(pagekeys[i].key),
		//			pagekeys[i].mods);
	}
	// else
	//	g_debug("We can not find %s key in profile...", pagekeys[i].name);
}

void get_row_and_column_from_geometry_str(glong *column, glong *row, glong *default_column, glong *default_row, gchar *geometry_str)
{
#ifdef DETAIL
	g_debug("! Launch get_row_and_column_from_geometry_str() with column = %ld, row = %ld, "
		"default_column = %ld, default_row = %ld, geometry_str = %s",
		*column, *row, *default_column, *default_row, geometry_str);
#endif
#ifdef SAFEMODE
	if ((column==NULL) || (row==NULL) || (default_column==NULL) || (default_row==NULL)) return;
#endif
	if (geometry_str && (geometry_str[0]!='\0'))
	{
		gint offset_x = 0, offset_y = 0;
		guint new_column, new_row;
		if (XParseGeometry (geometry_str, &offset_x, &offset_y, &new_column, &new_row))
		{
			*column = new_column;
			*row = new_row;
		}
	}

	if (*column < 1) *column = *default_column;
	if (*row < 1) *row = *default_row;
}
