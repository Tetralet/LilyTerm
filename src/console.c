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

#include "console.h"

extern gboolean single_process;
extern gchar *profile_dir;

void command_option(int   argc,
		    char *argv[])
{
#ifdef DETAIL
	g_debug("! Launch command_option()!");
#endif

#ifdef SAFEMODE
	if (argv==NULL) return;
#endif
	gint i;
	for (i=0; i<argc; i++)
	{
#ifdef SAFEMODE
		if (argv[i]==NULL) break;
#endif
		// g_debug("%2d (Total %d): %s",i, argc, argv[i]);
		if ((!strcmp(argv[i], "-v")) || (!strcmp(argv[i], "--version")))
		{
			g_print("%s %s\n", PACKAGE, VERSION);
			exit (0);
		}
		else if ((!strcmp(argv[i], "-?")) || (!strcmp(argv[i], "-h")) || (!strcmp(argv[i], "--help")))
		{

			// LilyTerm will exit immediately, so we don't need to free the get_help_message... XD
			g_print("\n%s\n", get_help_message(NULL));

			// GString *help_msg = get_help_message();
			// g_print("\n%s\n", help_msg->str);
			// g_string_free (help_msg, TRUE);

			exit (0);
		}
		else if ((!strcmp(argv[i], "-p")) || (!strcmp(argv[i], "--profile")))
		{
			GString *settings = save_user_settings(NULL, NULL);
#ifdef SAFEMODE
			if (settings)
#endif
				g_print("%s", settings->str);
			g_string_free(settings, TRUE);
			exit (0);
		}
		else if ((!strcmp(argv[i], "-s")) || (!strcmp(argv[i], "--separate")))
		{
			single_process = FALSE;
		}
		else if ((!strcmp(argv[i], "-e")) || (!strcmp(argv[i], "-x")) || (!strcmp(argv[i], "--execute")))
		{
			// If -e or -x options specified then ignore anything beyond them
			break;
		}
		else if (!strcmp(argv[i], "--name"))
		{
			if (++i==argc)
				g_critical("missing program name after --name!\n");
			else
			{
				extern gchar *wmclass_name;
				wmclass_name = argv[i];
			}
		}
		else if (!strcmp(argv[i], "--class"))
		{
			if (++i==argc)
				g_critical("missing program class after --class!\n");
			else
			{
				extern gchar *wmclass_class;
				wmclass_class = argv[i];
			}
		}
	}
}

// It will be ok if profile=NULL here.
gchar *get_help_message(gchar *profile)
{
#ifdef DETAIL
	g_debug("! Launch get_help_message()!");
#endif
	GString *help_message = g_string_new(NULL);
	gchar *usage = get_help_message_usage(profile, FALSE);
#ifdef SAFEMODE
	if (usage)
#endif
		g_string_append(help_message, usage);

	gchar *key_binding = get_help_message_key_binding(FALSE);
#ifdef SAFEMODE
	if (key_binding)
#endif
		g_string_append(help_message,  key_binding);

	g_string_append(help_message, "\n");
	g_string_append_printf(help_message, _("Please report bug at <%s>.\n"), ISSUES);
	g_string_append_printf(help_message, _("Thank you for using %s!"), PACKAGE);
	g_string_append(help_message, "\n");
	g_free(usage);
	g_free(key_binding);
	return g_string_free(help_message, FALSE);
}

// It will be ok if profile=NULL here.
gchar *get_help_message_usage(gchar *profile, gboolean convert_to_html)
{
#ifdef DETAIL
	g_debug("! Launch get_help_message_usage() with profile = %s, convert_to_html = %d!",
		profile, convert_to_html);
#endif
	GString *help_message = g_string_new(NULL);

	// TRANSLATE NOTE: Please be care of the length of the following usage translation,
	// TRANSLATE NOTE: It should be shorter than 80 columns.
	g_string_append_printf(help_message, _("%s is a libvte based X Terminal Emulator.\n\n"), PACKAGE);
	g_string_append(help_message,   _("Use -T/--title {title} to specify the window title.\n"));
	g_string_append(help_message,   _("Use -R/--role {role} to specify the WM_WINDOW_ROLE string of window.\n"));
	g_string_append(help_message,   _("Use -t/--tab {number} to open multi tabs when starting up.\n"));
	g_string_append(help_message,   _("Use -n/--tab_names {tab names} to specify tab names, separate with <space>.\n"));
	g_string_append(help_message,   _("Use -d/--directory {directory} to specify the init directory when starting up.\n"));
	g_string_append(help_message,   _("Use -g/--geometry {WIDTHxHEIGHT[+-]XOFFSET[+-]YOFFSET} to specify the geometry.\n"));
	g_string_append(help_message,   _("Use -l/-ls/--login to make the shell invoked as a login shell.\n"));
	g_string_append(help_message,   _("Use -ut to disable recording the session in lastlog, utmp and wtmp.\n"));
	g_string_append(help_message,   _("Use -H/--hold to hold the terminal window open when -e/-x command terminated.\n"));
//	g_string_append(help_message,   _("\t\t\tThis option will be ignored when using with -e/-x/--execute option.\n"));
	g_string_append(help_message,   _("Use -s/--separate to run in separate process.\n"));
	g_string_append(help_message,   _("Use -j/--join to integrate new created tabs to the last accessed window.\n"));
	g_string_append(help_message,   _("Use -p/--profile to get a profile sample.\n"));
	g_string_append_printf(help_message,
					_("Use -u/--user_profile {%s} to use a specified profile.\n"), PROFILE);
	g_string_append(help_message,   _("Use -v/--version to show the version information.\n"));
	g_string_append(help_message,   _("Use -e/-x/--execute {Command} to run a command. (Must be the final option).\n\n"));
	g_string_append_printf(help_message,
					_("The %s system configure file is: %s\n"), PACKAGE, SYS_PROFILE);

	gchar *current_profile = NULL;
	// g_debug("get_help_message_usage(): profile = %s", profile);
	if (profile)
		current_profile = g_strdup(profile);
	else
		current_profile = g_strdup_printf("%s/%s", profile_dir, USER_PROFILE);
#ifdef SAFEMODE
	if (current_profile)
	{
#endif
		if (convert_to_html)
		{
			gchar *msg_str = g_string_free(help_message, FALSE);
			gchar *new_help_message = convert_text_to_html (&msg_str, TRUE, NULL, "tt", NULL);
#ifdef SAFEMODE
			if (new_help_message)
#endif
				help_message = g_string_new(new_help_message);
#ifdef SAFEMODE
			else
				help_message = g_string_new("");
#endif
			g_free(new_help_message);
			current_profile = convert_text_to_html (&current_profile, TRUE,  "darkgreen", "tt", NULL);
		}
		else
		{
			gchar *new_current_profile = g_strdup_printf("%s\n\n", current_profile);
			g_free(current_profile);
			current_profile = new_current_profile;
		}

		gchar *profile_message = g_strdup_printf(_("And your %s profile is: "), PACKAGE);
#ifdef SAFEMODE
		if (profile_message)
#endif
			if (convert_to_html)
				profile_message =  convert_text_to_html(&profile_message, TRUE, NULL, "tt", NULL);

		// g_debug("FINAL: profile_message = %s", profile_message);

#ifdef SAFEMODE
		if (profile_message)
#endif
			g_string_append(help_message, profile_message);
#ifdef SAFEMODE
		if (current_profile)
#endif
			g_string_append(help_message, current_profile);
		g_free(profile_message);
#ifdef SAFEMODE
	}
#endif

	g_free(current_profile);
	return g_string_free(help_message, FALSE);
}

gchar *get_help_message_key_binding(gboolean convert_to_html)
{
#ifdef DETAIL
	g_debug("! Launch get_help_message_key_binding() with convert_to_html = %d!", convert_to_html);
#endif
	gchar *msg_head = _("Default key binding:");
	// TRANSLATE NOTE: Please be care of the spacing when translating the following key binding description.
	// TRANSLATE NOTE: Please check it in [Right Click Menu] -> [Usage] -> [Key binding] page after translating.
	gchar *disable_key_binding = _("  * <Ctrl><`>           Disable/Enable hyperlinks, function keys and menu.");
	GString *message = g_string_new(NULL);
	g_string_append(message, _("  * <Ctrl><T>           Add a New tab with current directory.\n"));
	g_string_append(message, _("  * <Ctrl><PgUp/PgDn>   Switch to Prev/Next tab.\n"));
	g_string_append(message, _("  * <Ctrl><Home/End>    Switch to First/Last tab.\n"));
	g_string_append(message, _("  * <Ctrl><[/]>         Move current tab Forward/Backward.\n"));
	g_string_append(message, _("  * <Ctrl><Up/Down>     Move current tab to First/Last.\n"));
	g_string_append(message, _("  * <Ctrl><F1~F12>      Switch to 1st ~ 12th tab.\n"));
	g_string_append(message, _("  * <Ctrl><+/-/Enter>   Increase/Decrease/Reset the font size of current tab.\n"));
	g_string_append(message, _("  * <Shift><Left/Right> Emulate a mouse Scroll Up/Down event on terminal.\n"));
	g_string_append(message, _("  * <Shift><Up/Down>    Asks to Scroll Up/Down 1 line on terminal.\n"));
	g_string_append(message, _("  * <Shift><PgUp/PgDn>  Gtk+ default behavior, Scroll Up/Down on terminal.\n"));
	g_string_append(message, _("  * <Shift><Home/End>   Gtk+ default behavior, Scroll terminal to Top/Bottom.\n"));
	g_string_append(message, _("  * <Alt><F11/Enter>    Switch between full/unfullwindow and full/unfullscreen.\n"));
	g_string_append(message, _("  * <Ctrl><F>           Find text in the terminal.\n"
				   "                        Use <F3>/<Shift><F3> to find Next/Prev.\n"));
	g_string_append(message, _("  * <Ctrl><O>           Select all the text in the terminal.\n"));
	g_string_append(message, _("  * <Ctrl><Del/Ins>     Copy/Paste the text in clipboard.\n"));
	g_string_append(message, _("  * <Shift><Del/Ins>    Copy/Paste the text in primary clipboard.\n"));
	g_string_append(message, _("                        (i.e. Emulate middle button mouse click to Copy/Paste)\n"));
	g_string_append(message,"\n");
	g_string_append(message, _("Some key bindings that disabled by default but maybe useful:\n"));
	g_string_append(message, _("  * <Ctrl><Q>           Trying to close current tab.\n"));
	g_string_append(message, _("                        (Using <Ctrl><D> or 'exit' to close tabs is recommended)\n"));
	g_string_append(message, _("  * <Ctrl><N>           Open a new window with current directory.\n"));
	g_string_append(message, _("  * <Ctrl><E>           Rename the current tab.\n"));

	gchar *final_message = NULL;
	if (convert_to_html)
	{
		gchar *str[5];

		str[0] = g_strdup_printf (_("TIP: These key bindings may custom or disable by right click menu [%s]."), _("Set key binding"));
		str[1] = convert_text_to_html(&msg_head, FALSE, NULL, "tt", NULL);
		str[2] = convert_text_to_html(&disable_key_binding, FALSE, "darkred", "tt", NULL);
		str[3] = convert_text_to_html(&(message->str), FALSE, NULL, "tt", NULL);
		str[4] = convert_text_to_html(&str[0], FALSE, "darkblue", "tt", "b", NULL);
		final_message = g_strdup_printf("%s\n%s\n%s\n%s", str[1], str[2], str[3], str[4]);
		gint i;
		for (i=0; i<5; i++) g_free(str[i]);
	}
	else
		final_message = g_strdup_printf("%s\n%s\n%s", msg_head, disable_key_binding, message->str);

	g_string_free (message, TRUE);
	return final_message;
}
