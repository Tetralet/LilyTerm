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

#include "console.h"

extern gboolean single_process;
extern gchar *profile_dir;

void command_option(int   argc,
		    char *argv[])
{
#ifdef DETAIL
	g_debug("! Launch command_option()!");
#endif

#ifdef DEFENSIVE
	if (argv==NULL) return;
#endif
	gint i;
	for (i=0; i<argc; i++)
	{
#ifdef DEFENSIVE
		if (argv[i]==NULL) break;
#endif
		// g_debug("%2d (Total %d): %s",i, argc, argv[i]);
		if ((!strcmp(argv[i], "-v")) || (!strcmp(argv[i], "--version")))
		{
			g_print("%s %s\n", PACKAGE_NAME, VERSION);
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
#ifdef DEFENSIVE
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
	gchar *function_key = get_help_message_function_key(FALSE);

#ifdef DEFENSIVE
	if (usage)
#endif
		g_string_append( help_message, usage);
	// g_string_append( help_message,  _("Default shortcut key: (It may custom or disable by editing user's profile)"));
	g_string_append( help_message,  _("Default shortcut key: (It may custom or disable by right click menu [Set function key value])"));
	g_string_append( help_message, "\n\n");
#ifdef DEFENSIVE
	if (function_key)
#endif
		g_string_append( help_message,  function_key);
	g_string_append( help_message, "\n");
	g_string_append_printf( help_message,
					_("\nPlease report bug to %s. Thank you for using %s!"),
								PACKAGE_BUGREPORT, PACKAGE_NAME);
	g_string_append( help_message, "\n");
	g_free(usage);
	g_free(function_key);
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
	gint i;

	g_string_append_printf( help_message,
					_("%s is a libvte based X Terminal Emulator.\n\n"), PACKAGE_NAME);
	g_string_append( help_message,  _("Use -e/-x/--execute {Command} to run a command when starting up."
					  " (Must be the final option).\n"));
	g_string_append( help_message,  _("Use -T/--title {title} to specify the window title.\n"));
	g_string_append( help_message,  _("Use -t/--tab {number} to open multi tabs when starting up.\n"));
	g_string_append( help_message,  _("Use -d/--directory {directory} to specify the init directory when starting up.\n"));
	g_string_append( help_message,  _("Use -g/--geometry {WIDTHxHEIGHT[+-]XOFFSET[+-]YOFFSET} to specify the geometry of window.\n"));
	g_string_append( help_message,  _("Use -l/-ls/--login to make the shell invoked as a login shell.\n"));
//	g_string_append( help_message,  _("\t\t\tThis option will be ignored when using with -e/-x/--execute option.\n"));
	g_string_append( help_message,  _("Use -s/--separate to run in separate process.\n"));
	g_string_append( help_message,  _("Use -v/--version to show the version information.\n"));
	g_string_append( help_message,  _("Use -p/--profile to got a profile sample.\n"));
	g_string_append_printf( help_message,
					_("Use -u/--user_profile {%s} to use a specified profile.\n\n"), SYS_PROFILE);
	const char * const *system_dirs = g_get_system_config_dirs();
	if (system_dirs)
	{
		for (i=0; system_dirs[i] != NULL; i++)
			g_string_append_printf( help_message,
					_("The %s system configure file is: %s/%s\n"), PACKAGE_NAME, system_dirs[i], SYS_PROFILE);
	}

	gchar *current_profile = NULL;
	// g_debug("get_help_message_usage(): profile = %s", profile);
	if (profile)
		current_profile = g_strdup(profile);
	else
		current_profile = g_strdup_printf("%s/%s", profile_dir, USER_PROFILE);
#ifdef DEFENSIVE
	if (current_profile)
	{
#endif
		if (convert_to_html)
		{
			gchar *msg_str = g_string_free(help_message, FALSE);
			gchar *new_help_message = convert_text_to_html (&msg_str, TRUE, NULL, "tt", NULL);
#ifdef DEFENSIVE
			if (new_help_message)
#endif
				help_message = g_string_new(new_help_message);
#ifdef DEFENSIVE
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

		gchar *profile_message;
		if (system_dirs)
			profile_message = g_strdup_printf(_("And your %s profile is: "), PACKAGE_NAME);
		else
			profile_message = g_strdup_printf(_("Your %s profile is: "), PACKAGE_NAME);
#ifdef DEFENSIVE
		if (profile_message)
#endif
			if (convert_to_html)
				profile_message =  convert_text_to_html(&profile_message, TRUE, NULL, "tt", NULL);

		// g_debug("FINAL: profile_message = %s", profile_message);

#ifdef DEFENSIVE
		if (profile_message)
#endif
			g_string_append(help_message, profile_message);
#ifdef DEFENSIVE
		if (current_profile)
#endif
			g_string_append(help_message, current_profile);
		g_free(profile_message);
#ifdef DEFENSIVE
	}
#endif

	g_free(current_profile);
	return g_string_free(help_message, FALSE);
}

gchar *get_help_message_function_key(gboolean convert_to_html)
{
#ifdef DETAIL
	g_debug("! Launch get_help_message_function_key() with convert_to_html = %d!", convert_to_html);
#endif
	GString *help_message = g_string_new(NULL);

	gchar *disable_function_key = _("  * <Ctrl><`>\t\tDisable/Enable hyperlinks, function keys and right click menu.\n");
	g_string_append( help_message,  _("  * <Ctrl><T/Q>\t\tAdd a New tab/Close current tab.\n"));
	g_string_append( help_message,  _("\t\t\t(Using <Ctrl><D> or 'exit' to close tabs is recommended)\n"));
	g_string_append( help_message,  _("  * <Ctrl><E>\t\tRename current tab.\n"));
	g_string_append( help_message,  _("  * <Ctrl><PgUp/PgDn>\tSwitch to Prev/Next tab.\n"));
	g_string_append( help_message,  _("  * <Ctrl><Home/End>\tSwitch to First/Last tab.\n"));
	g_string_append( help_message,  _("  * <Ctrl><[/]>\t\tMove current tab Forward/Backward.\n"));
	g_string_append( help_message,  _("  * <Ctrl><Up/Down>\tMove current tab to First/Last.\n"));
	g_string_append( help_message,  _("  * <Ctrl><F1~F12>\tSwitch to 1st ~ 12th tab.\n"));
	g_string_append( help_message,  _("  * <Ctrl><O>\t\tSelect all the text in the Vte Terminal box.\n"));
	g_string_append( help_message,  _("  * <Ctrl><Del/Ins>\tCopy the text to clipboard / Paste the text in clipboard.\n"));
	g_string_append( help_message,  _("  * <Ctrl><+/-/Enter>\tIncrease/Decrease/Reset the font size of current tab.\n"));
	g_string_append( help_message,  _("  * <Shift><Left/Right>\tEmulate a mouse Scroll Up/Down event on Vte Terminal box.\n"));
	g_string_append( help_message,  _("  * <Shift><Up/Down>\tAsks to Scroll Up/Down 1 line on Vte Terminal box.\n"));
	g_string_append( help_message,  _("  * <Shift><PgUp/PgDn>\tGtk+ default behavior, Scroll Up/Down on Vte Terminal box.\n"));
	g_string_append( help_message,  _("  * <Shift><Home/End>\tGtk+ default behavior, Scroll the Terminal box to to Top/Bottom.\n"));
	g_string_append( help_message,  _("  * <Alt><F11/Enter>\tSwitch between fullwindow/unfullwindow and fullscreen/unfullscreen state.\n"));
	g_string_append( help_message,  _("  * <Shift><Insert>\tVte default behavior, Paste the text in primary clipboard.\n"));
	g_string_append( help_message,  _("\t\t\t(i.e. Emulate a middle button mouse click to paste the text)"));
	if (convert_to_html)
	{
		gchar *final_help_message = NULL;
		disable_function_key = convert_text_to_html(&disable_function_key, FALSE, "darkblue", "tt", NULL);
#ifdef DEFENSIVE
		if (disable_function_key)
		{
			gchar *html_help_message = NULL;
			if (help_message)
				html_help_message = convert_text_to_html(&(help_message->str),
									 FALSE, NULL, "tt", NULL);
#else
			gchar *html_help_message = html_help_message = convert_text_to_html(&(help_message->str),
											    FALSE, NULL, "tt",
											    NULL);
#endif

			final_help_message = g_strdup_printf("%s%s",
							     disable_function_key,
							     html_help_message);
			g_free (html_help_message);
#ifdef DEFENSIVE
		}
#endif
		g_free (disable_function_key);
		g_string_free (help_message, TRUE);
		// g_debug("Get html_help_message = %s", final_help_message);
		return final_help_message;
	}
	else
	{
		g_string_prepend (help_message, disable_function_key);
		return g_string_free(help_message, FALSE);
	}
}
