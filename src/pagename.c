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

#include "pagename.h"

extern gint dialog_activated;
extern gboolean proc_exist;
extern gchar *proc_file_system_path;

// cmdline always = "" under *Ubuntu ONLY* if argv = split_string("", " ", -1);
// Don't ask me why...
gboolean vte_fork_cmdline_returned_empty = FALSE;

void reorder_page_number(GtkNotebook *notebook, GtkWidget *child, guint page_num, GtkWidget *window)
{
#ifdef DETAIL
	g_debug("! Launch reorder_page_number() with notebook = %p, page_num = %d, window = %p",
		notebook, page_num, window);
#endif
#ifdef SAFEMODE
	if (window==NULL) return;
#endif
	struct Window *win_data = (struct Window *)g_object_get_data(G_OBJECT(window), "Win_Data");
#ifdef SAFEMODE
	if (win_data==NULL) return;
#endif
	gint total_page = gtk_notebook_get_n_pages(GTK_NOTEBOOK(notebook));
	// g_debug("total_page = %d", total_page);
	struct Page *page_data;
	gint i;

	for (i=0; i<total_page; i++)
	{
		page_data = get_page_data_from_nth_page (win_data, i);
#ifdef SAFEMODE
		if (page_data==NULL) continue;
#endif
		// g_debug("Got page_data = %p", page_data);
		// we store current_page_no in "struct Page" for performance.
		page_data->page_no = i;

		// g_debug("Launch get_and_update_page_name() in reorder_page_number()");

		// g_debug("page_data->label_text = %p "
		//	"page_data->page_name = %s "
		//	"page_data->custom_page_name = %s "
		//	"page_data->tab_color = %s "
		//	"win_data->default_encoding = %p "
		//	"page_data->encoding_str = %s",
		//	page_data->label_text,
		//	page_data->page_name,
		//	page_data->custom_page_name,
		//	page_data->tab_color,
		//	win_data->default_menuitem_encoding,
		//	page_data->encoding_str);

		// Don't update page name when win_data->kill_color_demo_vte.
		// Or LilyTerm will got warning: "Failed to set text from markup due to error parsing markup"
		if (((win_data->page_shows_number) || win_data->adding_page) &&
		    (dialog_activated==0) &&
		    (! win_data->kill_color_demo_vte))
			update_page_name(window,
					 page_data->vte,
					 page_data->page_name,
					 page_data->label_text,
					 i+1,
					 page_data->custom_page_name,
					 page_data->tab_color,
					 page_data->is_root,
					 page_data->is_bold,
					 compare_strings(win_data->runtime_encoding,
							 page_data->encoding_str,
							 FALSE),
					 page_data->encoding_str,
					 page_data->custom_window_title,
					 FALSE);
	}
}


void init_monitor_cmdline_datas(struct Window *win_data, struct Page *page_data)
{
#ifdef DETAIL
	g_debug("! Launch init_monitor_cmdline_datas() with win_data = %p, page_data = %p", win_data, page_data);
#endif
#ifdef SAFEMODE
	if ((win_data==NULL) || (page_data==NULL)) return;
#endif

	page_data->window_title_updated = -1;

	page_data->check_root_privileges = win_data->check_root_privileges;
	page_data->page_shows_window_title = win_data->page_shows_window_title;
	page_data->page_shows_current_dir = win_data->page_shows_current_dir;
	page_data->page_shows_current_cmdline = win_data->page_shows_current_cmdline;
	page_data->bold_action_page_name = win_data->bold_action_page_name;

	page_data->window_title_tpgid = &(win_data->window_title_tpgid);
	page_data->lost_focus = &(win_data->lost_focus);
#ifdef USE_GTK2_GEOMETRY_METHOD
	page_data->keep_vte_size = &(win_data->keep_vte_size);
#endif
	page_data->current_vte = &(win_data->current_vte);
	// page_data->update_window_title_only = &(win_data->update_window_title_only);
	page_data->custom_window_title = (win_data->custom_window_title_str != NULL);
}


gboolean monitor_cmdline(struct Page *page_data)
{
#ifdef FULL
	g_debug("! Launch monitor_cmdline() with page_data = %p", page_data);
#endif
#ifdef SAFEMODE
	if ((page_data==NULL) || (page_data->lost_focus==NULL) || (page_data->keep_vte_size==NULL) ||
	    (page_data->current_vte==NULL) || (page_data->window_title_tpgid==NULL)) return FALSE;
#endif
	if (page_data->pid<1) return FALSE;

	gboolean lost_focus = *(page_data->lost_focus);

	// g_debug("Get page_data->keep_vte_size = %d in monitor_cmdline()", *(page_data->keep_vte_size));

	// g_debug ("\nvte_terminal_get_status_line = %s,"
	//	"vte_terminal_get_icon_title = %s,\n"
	//	"vte_terminal_get_window_title = %s\n",
	//	vte_terminal_get_status_line(VTE_TERMINAL(page_data->vte)),
	//	vte_terminal_get_icon_title(VTE_TERMINAL(page_data->vte)),
	//	vte_terminal_get_window_title(VTE_TERMINAL(page_data->vte)));

	// The pagename won't be updated if LilyTerm is not on focus or when resizing.
	// But it will still update the window title.
	// 0xfe = 11,111,110
	if ((lost_focus && (*(page_data->current_vte) != (page_data->vte))) ||
#ifdef USE_GTK2_GEOMETRY_METHOD
	    (*(page_data->keep_vte_size)&0xfffc) ||
#endif
	    page_data->custom_page_name ||
	    dialog_activated)
		return TRUE;
	// if we should call get_and_update_page_name or not
	gboolean page_name_changed = FALSE;

	// backup page_data datas
	gint page_update_method = page_data->page_update_method;

	page_data->page_update_method = PAGE_METHOD_AUTOMATIC;
	// g_debug("INIT: lost_focus = %d", lost_focus);
	// g_debug("INIT: page_data->window_title_updated = %d", page_data->window_title_updated);

	if (lost_focus)
		page_name_changed = check_cmdline(page_data, *(page_data->window_title_tpgid));
	else
	{
		page_name_changed = check_cmdline(page_data, page_data->displayed_tpgid);

		if ((page_name_changed)	&& page_data->check_root_privileges)
			page_data->is_root = check_is_root(page_data->current_tpgid);
	}

	// g_debug("lost_focus = %d", lost_focus);
	// g_debug("page_data->window_title_updated = %d", page_data->window_title_updated);

	if (check_window_title(page_data, lost_focus))
		page_name_changed = TRUE;

	gchar *old_pwd;
	if (lost_focus)
		old_pwd = page_data->window_title_pwd;
	else
		old_pwd = page_data->pwd;

	gchar *new_pwd = get_tab_name_with_current_dir(page_data->current_tpgid);

	if (check_pwd(page_data, old_pwd, new_pwd, page_update_method))
	{
		page_name_changed = TRUE;
		if (lost_focus)
			page_data->window_title_pwd = new_pwd;
		else
			page_data->pwd = new_pwd;
		new_pwd = old_pwd;
	}

	// g_debug("check_tpgid = %d, and current_tpgid = %d", *check_tpgid, page_data->current_tpgid);
	// only update the page name when tpgid is updated.
	// g_debug("Final: page_data->page_update_method = %d", page_data->page_update_method);
	if (page_name_changed)
	{
		// g_debug("!!! Update the page name for page_no %d", page_data->page_no);
		page_data->is_bold = page_data->bold_action_page_name;
		page_data->should_be_bold = TRUE;
		// g_debug("page_data->is_bold = %d for page_data %p", page_data->is_bold, page_data);

		// g_debug("Launch get_and_update_page_name() in monitor_cmdline()");
		if (( ! get_and_update_page_name(page_data, *(page_data->lost_focus))) && (! lost_focus))
		{
			// if update "tab title" fault. upate window title will never fault.
			// g_debug("!! Tab name is NOT changed to '%s'", page_data->pwd);
			// exchange old_pwd and new_pwd again!
			new_pwd = page_data->pwd;
			page_data->pwd = old_pwd;
			// g_debug("Restore page_data->pwd to %s", page_data->pwd);
		}
	}
	else
	{
		page_data->page_update_method = page_update_method;
		// g_debug("Restore page_data->page_update_method to %d!!!", page_data->page_update_method);
	}

	g_free(new_pwd);

	return TRUE;
}

gboolean check_cmdline(struct Page *page_data, pid_t check_tpgid)
{
#ifdef FULL
	g_debug("! Launch check_cmdline() with page_data = %p, check_tpgid = %d", page_data, check_tpgid);
#endif
#ifdef SAFEMODE
	if ((page_data==NULL) || (check_tpgid < 1)) return FALSE;
#endif
	// g_debug("check_tpgid = %d", check_tpgid);

	gboolean page_name_changed = FALSE;
	if (page_data->page_shows_current_cmdline || page_data->page_shows_current_dir)
	{
		// find the current_tpgid
		page_data->current_tpgid = get_tpgid(page_data->pid);
		// g_debug("Get page_data->current_tpgid = %d, check_tpgid = %d",
		//	page_data->current_tpgid, check_tpgid);

		if (check_tpgid != page_data->current_tpgid)
		{
			// g_debug("Trying to update Cmdline!!!");
			page_name_changed = TRUE;
			page_data->page_update_method = PAGE_METHOD_CMDLINE;
		}
		// else if (page_data->current_tpgid==page_data->pid)
		else if (page_data->window_title_updated == 0)
			page_data->window_title_updated = -1;

		// g_debug("page_data->window_title_updated = %d", page_data->window_title_updated);
		// g_debug("page_data->page_update_method = %d", page_data->page_update_method);
	}
	// g_debug("the original tpgid is %d, and got tpgid from get_tpgid() is: %d",
	//	page_data->displayed_tpgid, page_data->current_tpgid);
	return page_name_changed;
}

gboolean check_window_title (struct Page *page_data, gboolean lost_focus)
{
#ifdef FULL
	g_debug("! Launch check_window_title() with page_data = %p, lost_focus = %d", page_data, lost_focus);
#endif
#ifdef SAFEMODE
	if (page_data==NULL) return FALSE;
#endif
	gboolean page_name_changed = FALSE;

	if ((page_data->page_shows_window_title) && (page_data->window_title_updated >= 1))
	{
		// don't update window title when lost focus
		// g_debug("lost_focus = %d", lost_focus);
		// g_debug("page_data->window_title_updated = %d", page_data->window_title_updated);
		if (! (lost_focus && (page_data->window_title_updated == 2)))
		{
			// g_debug("Trying to update Window title!!!");
			page_name_changed = TRUE;
			page_data->page_update_method = PAGE_METHOD_WINDOW_TITLE;
			page_data->window_title_updated = 1;
		}
	}
	return page_name_changed;
}

gboolean check_pwd(struct Page *page_data, gchar *pwd, gchar *new_pwd, gint page_update_method)
{
#ifdef FULL
	g_debug("! Launch check_pwd() with page_data = %p, pwd = %s, new_pwd = %s, page_update_method = %d",
		page_data, pwd, new_pwd, page_update_method);
#endif
#ifdef SAFEMODE
	if ((page_data==NULL) || (page_data->window_title_tpgid==NULL)) return FALSE;
#endif
	// g_debug("pwd = %s", pwd);
	// g_debug("new_pwd = %s", new_pwd);

	gboolean page_name_changed = FALSE;

	// update the page name with PWD when pid=tpgid
	// g_debug ("page_name_changed = %d, ", page_name_changed);
	if (page_data->page_shows_current_dir &&
	    (page_data->current_tpgid == page_data->pid) &&
	    (page_data->window_title_updated == -1))
	{
		page_name_changed = compare_strings(pwd, new_pwd, TRUE);

		if ((*(page_data->window_title_tpgid) != page_data->displayed_tpgid) &&
		    (page_update_method == PAGE_METHOD_CMDLINE))
			page_name_changed = TRUE;

		if (page_name_changed)
		{
			// g_debug("Trying to update PWD!!!");
			page_data->page_update_method = PAGE_METHOD_PWD;
			page_name_changed = TRUE;
		}
		// g_debug("page_data->window_title_pwd = %s\npage_name_changed = %d",
		//	page_data->window_title_pwd, page_name_changed);
	}
	return page_name_changed;
}


// it will update the text in label ,page_name, and the title of window
// return FALSE if update the label (and page_name) fault.
// updating the title of window will never fault.
gboolean get_and_update_page_name(struct Page *page_data, gboolean lost_focus)
{
#ifdef DETAIL
	g_debug("! Launch get_and_update_page_name() with page_data = %p, lost_focus = %d", page_data, lost_focus);
#endif
#ifdef SAFEMODE
	if ((page_data==NULL) || (page_data->window==NULL)) return FALSE;
#endif
	struct Window *win_data = (struct Window *)g_object_get_data(G_OBJECT(page_data->window), "Win_Data");
	// g_debug("Get win_data = %p (page_data->window = %p) when update tab name!", win_data, page_data->window);
#ifdef SAFEMODE
	if (win_data==NULL) return FALSE;
#endif
	// page_color should not be free().
	gchar *page_name = NULL, *page_color = NULL;

	// g_debug("Get pid=%d, tpgid=%d", page_data->pid, page_data->current_tpgid);
	// g_debug("win_data->page_shows_window_title = %d", win_data->page_shows_window_title);
	// g_debug("page_data->window_title_updated = %d", page_data->window_title_updated);
	// g_debug("lost_focus = %d", lost_focus);
	// g_debug("page_data->page_update_method = %d", page_data->page_update_method);

	switch (page_data->page_update_method)
	{
		case PAGE_METHOD_AUTOMATIC:
		case PAGE_METHOD_REFRESH:
			break;
		case PAGE_METHOD_WINDOW_TITLE:
			update_page_name_wintitle(&page_name, &page_color, win_data, page_data);
			break;
		case PAGE_METHOD_CMDLINE:
			update_page_name_cmdline(&page_name, &page_color, win_data, page_data);
			break;
		case PAGE_METHOD_PWD:
			update_page_name_pwd(&page_name, &page_color, win_data, page_data, lost_focus);
			break;
		case PAGE_METHOD_NORMAL:
			update_page_name_normal(&page_name, &page_color, win_data, page_data);
			break;
		default:
#ifdef FATAL
			print_switch_out_of_range_error_dialog("get_and_update_page_name",
							       "page_data->page_update_method",
							       page_data->page_update_method);
#endif
			return FALSE;
	}
	if (page_name==NULL)
	{
		update_page_name_wintitle(&page_name, &page_color, win_data, page_data);
		update_page_name_cmdline(&page_name, &page_color, win_data, page_data);
		update_page_name_pwd(&page_name, &page_color, win_data, page_data, lost_focus);
		update_page_name_normal(&page_name, &page_color, win_data, page_data);
	}

#ifdef SAFEMODE
	// if we can NOT got any page_name... it should not happen!
	if (page_name==NULL) return FALSE;
#endif

	if (page_data->is_root)
		page_color = win_data->user_page_color[4];

	// even though custom_page_name is setted, we still need to set the page_name for it.
	// therefore if we disable custom_page_name sometime, we still have the page_name for use.
	if (page_data->custom_page_name!=NULL)
	{
		page_color = win_data->user_page_color[3];
		// g_debug("page_shows_custom_dir : page_name = %s, color = %s", page_name, page_color);
	}

	// g_debug("Final : page_name = %s, color = %s", page_name, page_color);

	if (win_data->use_color_page)
		page_data->tab_color = page_color;
	else
		// we Must clean the value for page_color here. for drag&drop tabs.
		page_data->tab_color = NULL;

	g_free(page_data->page_name);
	gchar *local_page_name = convert_str_to_utf8(page_name, page_data->encoding_str);
	gchar *converted_page_name = convert_escape_sequence_to_string(local_page_name);
#ifdef SAFEMODE
	if (converted_page_name == NULL)
		page_data->page_name = page_name;
	else
	{
#endif
		g_free(page_name);
		page_data->page_name = converted_page_name;
#ifdef SAFEMODE
	}
#endif
	g_free(local_page_name);

	gboolean return_value = FALSE;
	// g_debug ("Launch update_page_name() in get_and_update_page_name()!!!");
	if (update_page_name(page_data->window, page_data->vte, page_data->page_name, page_data->label_text,
			     page_data->page_no+1, page_data->custom_page_name, page_color, page_data->is_root,
			     page_data->is_bold, compare_strings(win_data->runtime_encoding,
								 page_data->encoding_str,
								 FALSE),
			     page_data->encoding_str, page_data->custom_window_title, lost_focus))
	{
		// g_debug("Update page_data->displayed_tpgid (%d) = page_data->current_tpgid (%d)",
		//	page_data->displayed_tpgid, page_data->current_tpgid);
		if (! lost_focus)
		{
			page_data->displayed_tpgid = page_data->current_tpgid;
			g_free(page_data->window_title_pwd);
			page_data->window_title_pwd = g_strdup(page_data->pwd);
		}
		return_value = TRUE;
	}
	else
	{
		// update_page_name() is failed, but the pagename datas is updated already.
		// We set page_data->displayed_tpgid = -1 here to mark that the update_page_name() is failed.
		page_data->displayed_tpgid = -1;
		if (page_data->page_update_method == PAGE_METHOD_WINDOW_TITLE)
			page_data->window_title_updated = PAGE_METHOD_CMDLINE;
	}

	// update page_data->pwd or page_data->window_title_pwd, if it was not updated
	if (page_data->page_update_method == PAGE_METHOD_WINDOW_TITLE)
	{
		if (win_data->page_shows_current_cmdline)
			page_data->displayed_tpgid = page_data->current_tpgid;

		if (win_data->page_shows_current_dir)
		{
			g_free(page_data->pwd);
			page_data->pwd = get_tab_name_with_current_dir(page_data->current_tpgid);
		}
	}

	win_data->window_title_tpgid = page_data->current_tpgid;
	// g_debug("Final: page_data->displayed_tpgid = %d", page_data->displayed_tpgid);
	// g_debug("Final: page_data->window_title_tpgid = %d", *(page_data->window_title_tpgid));

	// g_debug("(get_and_update_page_name): return_value = %d", return_value);
	return return_value;
}

void update_page_name_wintitle(StrAddr **page_name,
			       StrAddr **page_color,
			       struct Window *win_data,
			       struct Page *page_data)
{
#ifdef DETAIL
	g_debug("! Launch update_page_name_wintitle() with page_name = %s, page_color = %s, win_data = %p, page_data = %p",
		*page_name, *page_color, win_data, page_data);
#endif
#ifdef SAFEMODE
	if ((page_name==NULL) || (page_color==NULL) || (win_data==NULL) || (page_data==NULL)) return;
#endif
	// g_debug("Trying to update Window Title!");
	if (win_data->page_shows_window_title && (*page_name==NULL) && (page_data->window_title_updated == 1))
	{
		*page_name = g_strdup(vte_terminal_get_window_title(VTE_TERMINAL(page_data->vte)));
		*page_color = win_data->user_page_color[0];
		page_data->window_title_updated = 0;
		// g_debug("Window title updated: *page_name = %s, color = %s", *page_name, *page_color);
		page_data->page_update_method = PAGE_METHOD_WINDOW_TITLE;
	}
}

void update_page_name_cmdline(StrAddr **page_name,
			      StrAddr **page_color,
			      struct Window *win_data,
			      struct Page *page_data)
{
#ifdef DETAIL
	g_debug("! Launch update_page_name_cmdline() with page_name = %s, page_color = %s, win_data = %p, page_data = %p",
		*page_name, *page_color, win_data, page_data);
#endif
#ifdef SAFEMODE
	if ((page_name==NULL) || (page_color==NULL) || (win_data==NULL) || (page_data==NULL)) return;
#endif
	// g_debug("Trying to update Cmdline!");
	// if win_data->page_shows_current_dir = 0 && win_data->page_shows_window_title = 0
	// We need to show /bin/bash on the tab name.
	// g_debug("page_name = %p (%s)", *page_name, *page_name);
	// g_debug("win_data->page_shows_current_cmdline = %d", win_data->page_shows_current_cmdline);
	// g_debug("page_data->pid = %d, page_data->current_tpgid = %d", page_data->pid, page_data->current_tpgid);
	if ( (*page_name==NULL) && win_data->page_shows_current_cmdline &&
	    ((! (win_data->page_shows_current_dir ||
		 ( win_data->page_shows_window_title &&
		   (vte_terminal_get_window_title(VTE_TERMINAL(page_data->vte))!=NULL) &&
		   (page_data->window_title_updated != -1)))) ||
	     (page_data->pid!=page_data->current_tpgid)))
	{
		*page_name = get_tab_name_with_cmdline(page_data);
#ifdef SAFEMODE
		if (page_name)
		{
#endif
			*page_color = win_data->user_page_color[1];
			// g_debug("Cmdline updated: *page_name = %s, color = %s", *page_name, *page_color);
			page_data->page_update_method = PAGE_METHOD_CMDLINE;
#ifdef SAFEMODE
		}
#endif
	}
}

void update_page_name_pwd(StrAddr **page_name,
			  StrAddr **page_color,
			  struct Window *win_data,
			  struct Page *page_data,
			  gboolean lost_focus)
{
#ifdef DETAIL
	g_debug("! Launch update_page_name_pwd() with page_name = %s, "
		"page_color = %s, win_data = %p, page_data = %p, lost_focus = %d",
		*page_name, *page_color, win_data, page_data, lost_focus);
#endif
#ifdef SAFEMODE
	if ((page_name==NULL) || (page_color==NULL) || (win_data==NULL) || (page_data==NULL)) return;
#endif
	// g_debug("Trying to update PWD!");
	if (win_data->page_shows_current_dir && (*page_name==NULL))
	{
		// g_free(page_data->pwd);
		// g_debug("!!! Getting new page_data->pwd");
		// page_data->pwd = get_tab_name_with_current_dir(page_data->current_tpgid);
		// page_name = get_tab_name_with_current_dir(pid);
		if (lost_focus)
			*page_name = g_strdup(page_data->window_title_pwd);
		else
		{
			// g_debug("page_data->pwd (get_and_update_page_name())= %s", page_data->pwd);
			*page_name = g_strdup(page_data->pwd);
		}

		if (*page_name)
		{
			*page_color = win_data->user_page_color[2];
			page_data->window_title_updated = -1;
		}
		// g_debug("PWD updated: *page_name = %s, color = %s", *page_name, *page_color);
		page_data->page_update_method = PAGE_METHOD_PWD;
	}
}

void update_page_name_normal(StrAddr **page_name,
			     StrAddr **page_color,
			     struct Window *win_data,
			     struct Page *page_data)
{
#ifdef DETAIL
	g_debug("! Launch update_page_name_normal() with page_name = %s, page_color = %s, win_data = %p, page_data = %p",
		*page_name, *page_color, win_data, page_data);
#endif
#ifdef SAFEMODE
	if ((page_name==NULL) || (page_color==NULL) || (win_data==NULL) || (page_data==NULL)) return;
#endif
	// g_debug("Trying to update to NORMAL page!");
	if (*page_name==NULL)
	{
		if (page_data->page_name && (page_data->page_update_method != PAGE_METHOD_REFRESH))
			*page_name = g_strdup(page_data->page_name);
		else
			*page_name = get_tab_name_with_page_names(win_data);

		if (win_data->use_color_page)
			*page_color = win_data->user_page_color[5];
		else
			*page_color = NULL;
		// g_debug("page_shows_normal_dir : page_name = %s, color = %s", *page_name, *page_color);
		if (page_data->page_update_method==PAGE_METHOD_REFRESH) win_data->page_names_no++;
		// g_debug("page_data->page_update_method = %d", page_data->page_update_method);
		page_data->page_update_method = PAGE_METHOD_NORMAL;
	}
}

gboolean update_page_name(GtkWidget *window, GtkWidget *vte, gchar *page_name, GtkWidget *label,
			  gint page_no, gchar *custom_page_name, const gchar *tab_color, gboolean is_root,
			  gboolean is_bold, gboolean show_encoding, gchar *encoding_str,
			  gboolean custom_window_title, gboolean lost_focus)
{
#ifdef DETAIL
	g_debug("! Launch update_page_name() with vte = %p, label_name = %s, page_no = %d, "
		"custom_page_name = %s, tab_color = %s, is_root = %d, is_bold = %d, "
		"show_encoding = %d, encoding_str = %s",
		vte, page_name, page_no, custom_page_name, tab_color, is_root, is_bold,
		show_encoding, encoding_str);
#endif
#ifdef SAFEMODE
		if (vte==NULL) return FALSE;
#endif
	// page_name = NULL when initing a new page.
	if (page_name == NULL)
	{
		struct Page *page_data = (struct Page *)g_object_get_data(G_OBJECT(vte), "Page_Data");
#ifdef SAFEMODE
		if (page_data==NULL) return FALSE;
#endif
		// g_debug("page_name = NULL!! trying to call get_and_update_page_name()");
		get_and_update_page_name(page_data, FALSE);
	}

	gboolean page_name_updated = FALSE;
	struct Window *win_data = (struct Window *)g_object_get_data(G_OBJECT(window), "Win_Data");
#ifdef SAFEMODE
	if (win_data==NULL) return FALSE;
#endif
	// We don't update label name when the size of window is changing.
	// 0xfe = 11,111,110
	// g_debug("win_data->keep_vte_size = %x", win_data->keep_vte_size);
#ifdef USE_GTK2_GEOMETRY_METHOD
	if ((!(win_data->keep_vte_size&0xfffc)) || (!gtk_widget_get_mapped(win_data->window)))
	{
#endif
		// g_debug("Updating %d page name to %s...", page_no, page_name);
		gchar *label_name = NULL;

		if (custom_page_name==NULL)
			label_name = page_name;
		else
			label_name = custom_page_name;

#ifdef ENABLE_SET_TOOLTIP_TEXT
		if (! lost_focus)
			gtk_widget_set_tooltip_text(label, label_name);
#endif
		// g_debug("[1] Updating %d page name to %s...", page_no, label_name);

		// FIXME: label_name = NULL when initing page.
		if (label_name==NULL) return FALSE;

		if (win_data->page_shows_number)
			label_name = g_strdup_printf("(%d) %s", page_no, label_name);
		else
			label_name = g_strdup(label_name);
		// g_debug("[2] Updating %d page name to %s...", page_no, label_name);
		// label_name will be freed later.

		if (win_data->page_shows_encoding && show_encoding && encoding_str)
		{
			gchar *temp_str = g_strdup_printf("%s (%s)", label_name, encoding_str);
			g_free(label_name);
			label_name = temp_str;
			// g_debug("[3] Updating %d page name to %s...", page_no, label_name);
		}

		// if the window is lost focus, don't update the tab name
		// g_debug("lost_focus = %d", lost_focus);
		if (! lost_focus)
		{
			// win_data->keep_vte_size |= 1;
			// g_debug("window_resizable in update_page_name! and keep_vte_size =%d", keep_vte_size);
			// window_resizable(window, vte, 2, 1);

			// DANGEROUS: remark the following keep_window_size()
			//	      will break the geometry of window when drag and drop.
			// g_debug("update_page_name(): launch keep_window_size()!");
#ifdef USE_GTK2_GEOMETRY_METHOD
#  ifdef GEOMETRY
			g_debug("@ update_page_name(): Call keep_gtk2_window_size() with keep_vte_size = %x",
				win_data->keep_vte_size);
#  endif
			keep_gtk2_window_size (win_data, vte, 0x3);
#endif
			if (win_data->use_color_page && (tab_color != NULL))
			{
				// g_debug("[Debug] Updating %d page name to %s...", page_no, label_name);
				gchar *color_label_name;
				if (is_bold)
					color_label_name = convert_text_to_html (&label_name, FALSE,
										 (gchar *)tab_color, "b", NULL);
				else
					color_label_name = convert_text_to_html (&label_name, FALSE,
										 (gchar *)tab_color, NULL);
				gtk_label_set_markup (GTK_LABEL(label), color_label_name);
				// g_debug("[FINAL] Updating %d page name to %s...", page_no, color_label_name);
				g_free(color_label_name);
			}
			else
				gtk_label_set_text(GTK_LABEL(label), label_name);
			// g_debug("Updated the tab name to %s!", page_name);
			page_name_updated = TRUE;
#ifdef USE_GTK3_GEOMETRY_METHOD
			if (win_data->window_status != WINDOW_APPLY_PROFILE_NORMAL)
			{
#  ifdef GEOMETRY
				fprintf(stderr, "\033[1;%dm!! update_page_name(%s)(win_data %p): "
						"Calling keep_gtk3_window_size() with hints_type = %d, win_data->window_status = %d\033[0m\n",
				ANSI_COLOR_MAGENTA, label_name, win_data, win_data->hints_type, win_data->window_status);
#  endif
				win_data->resize_type = GEOMETRY_AUTOMATIC;
				keep_gtk3_window_size(win_data, FALSE);
			}
#endif
		}
#ifdef DEBUG
		// else
		//	g_debug("!!! the window is lost focus, don't update the tab name");
#endif
		// free the data
		g_free(label_name);
#ifdef USE_GTK2_GEOMETRY_METHOD
	}
#  ifdef DEBUG
	// else
	//	g_debug("!!! the window is renaming, don't update the tab name");
#  endif
#endif
	// we should update window title if page name changed.
	check_and_update_window_title(win_data, custom_window_title, page_no, custom_page_name, page_name);

	// g_debug("(update_page_name): page_name_updated = %d", page_name_updated);

	return page_name_updated;
}

void check_and_update_window_title(struct Window *win_data, gboolean custom_window_title, gint page_no,
				   gchar *custom_page_name, gchar *page_name)
{
#ifdef DETAIL
	g_debug("! Launch check_and_update_window_title with win_data = %p, custom_window_title = %d, "
		"page_no = %d, custom_page_name = %s, page_name = %s",
		win_data, custom_window_title, page_no, custom_page_name, page_name);
#endif
#ifdef SAFEMODE
	if ((win_data==NULL) || (win_data->notebook==NULL)) return;
#endif
	if ((win_data->window_title_shows_current_page == FALSE) || (custom_window_title == TRUE)) return;

	gint current_page_no = gtk_notebook_get_current_page(GTK_NOTEBOOK(win_data->notebook));

	// we only update the window title for current page
	// g_debug("current_page_no = %d, page_no = %d", current_page_no, page_no);

	// FIXME: A dirty hack... current_page_no=-1 when creating a 2rd new window.
	// if (win_data->adding_page && (current_page_no<0)) current_page_no = 0;

	if ((current_page_no == (page_no-1)) || ((current_page_no==-1) && (page_no==1)))
	{
		gchar *window_title = NULL;
		if (custom_page_name)
			window_title = custom_page_name;
		else
			window_title = page_name;

		if (window_title)
			// g_debug("page_name = %s before calling update_window_title()", page_name);
			update_window_title(win_data->window, window_title,
					    win_data->window_title_append_package_name);
	}
}

void update_window_title(GtkWidget *window, gchar *name, gboolean window_title_append_package_name)
{
#ifdef DETAIL
	g_debug("! Launch update_window_title() with window = %p, and name = %s, "
		"window_title_append_package_name = %d", window, name, window_title_append_package_name);
#endif
#ifdef SAFEMODE
	if ((window==NULL) || (name == NULL)) return;
#endif
	if (window_title_append_package_name)
	{
		gchar *window_title = g_strdup_printf("%s - %s", name, PACKAGE);
		// g_debug("Update the window title to %s!", window_title);
#ifdef SAFEMODE
		if (window_title)
#endif
			gtk_window_set_title(GTK_WINDOW(window), window_title);
		g_free(window_title);
	}
	else
		gtk_window_set_title(GTK_WINDOW(window), name);
}

// The returned string should be freed when no longer needed.
gchar *get_tab_name_with_page_names(struct Window *win_data)
{
#ifdef FULL
	g_debug("! Launch get_tab_name_with_page_names() with win_data = %p", win_data);
#endif
#ifdef SAFEMODE
	if (win_data==NULL) return NULL;
#endif
	// g_debug("Get win_data = %d when get tab name with page names!", win_data);
	// g_debug("Got win_data->page_names_no = %d, win_data->max_page_names_no = %d",
	//	win_data->page_names_no, win_data->max_page_names_no);

	win_data->page_names_no = CLAMP(win_data->page_names_no, 0, win_data->max_page_names_no);

#ifdef SAFEMODE
	if (win_data->reuse_page_names && win_data->splited_page_names &&
	    (win_data->splited_page_names[win_data->page_names_no]==NULL))
#else
	if (win_data->reuse_page_names && (win_data->splited_page_names[win_data->page_names_no]==NULL))
#endif
		win_data->page_names_no=0;

	// g_debug("Got win_data->page_names_no = %d", win_data->page_names_no);
#ifdef SAFEMODE
	if (win_data->splited_page_names &&
	    (win_data->splited_page_names[win_data->page_names_no]!=NULL))
#else
	if (win_data->splited_page_names[win_data->page_names_no]!=NULL)
#endif
		return g_strdup(win_data->splited_page_names[win_data->page_names_no]);
	else
		return g_strdup(win_data->page_name);
}

// It will return NULL if fault
// The returned string should be freed when no longer needed.
gchar *get_tab_name_with_cmdline(struct Page *page_data)
{
#ifdef DETAIL
	if (page_data)
		g_debug("! Launch get_tab_name_with_cmdline() for tpgid %d", page_data->current_tpgid);
#endif
#ifdef SAFEMODE
	if (page_data==NULL) return NULL;
#endif
	// if cmdline always be "", don't call get_cmdline(). It always get "".
	if (! vte_fork_cmdline_returned_empty)
	{
		if (page_data->pid_cmdline == NULL) page_data->pid_cmdline = get_cmdline(page_data->pid);
		if (page_data->pid_cmdline == NULL) vte_fork_cmdline_returned_empty = TRUE;
	}

#ifdef OUT_OF_MEMORY
	return NULL;
#endif
	if (page_data->current_tpgid>0)
	{
		gchar *tpgid_cmdline = get_cmdline(page_data->current_tpgid);
		//if ((page_data->pid == page_data->current_tpgid) ||
		//    (compare_strings(page_data->pid_cmdline, tpgid_cmdline, TRUE)))
		if ((page_data->pid == page_data->current_tpgid) ||
		    (tpgid_cmdline && page_data->pid_cmdline &&
		     (! g_str_has_suffix(tpgid_cmdline, page_data->pid_cmdline))))
			return tpgid_cmdline;
		else
		{
			// g_debug("pid = %d, tpgid = %d, pid_cmdline = %s", pid, tpgid, pid_cmdline);
#ifdef DEBUG
			g_message("Got (%s), Trying to reread the %s/%d/cmdline...",
				  proc_file_system_path, tpgid_cmdline, (gint)page_data->current_tpgid);
#endif
			g_free(tpgid_cmdline);
			// Magic number: we wait for 0.15 sec then reread cmdline again.
			usleep(150000);
			tpgid_cmdline = get_cmdline(page_data->current_tpgid);
#ifdef DEBUG
			g_message("Got (%s) after reread the %s/%d/cmdline.",
				  proc_file_system_path, tpgid_cmdline, (gint)page_data->current_tpgid);
#endif
			return tpgid_cmdline;
		}
	}
	else
		return NULL;
}

// It will return NULL if fault
// The returned string should be freed when no longer needed.
gchar *get_tab_name_with_current_dir(pid_t pid)
{
#ifdef FULL
	g_debug("! Launch get_tab_name_with_current_dir() for pid %d", pid);
#endif
	if (! proc_exist) return NULL;

	if (pid>0)
	{
		gchar *cwd_path = g_strdup_printf("%s/%d/cwd", proc_file_system_path, pid);
#ifdef SAFEMODE
		if (cwd_path)
		{
#endif
			gchar *current_dir = g_file_read_link(cwd_path, NULL);
			g_free(cwd_path);
			return current_dir;
#ifdef SAFEMODE
		}
#endif
	}
	return NULL;
}


gint get_tpgid(pid_t pid)
{
#ifdef FULL
	g_debug("! Launch get_tpgid() for pid %d", pid);
#endif
	if (pid<1) return 0;

	pid_t tmp_tpgid = 0;
	pid_t current_tpgid = pid;

	while (tmp_tpgid != current_tpgid)
	{
		tmp_tpgid = current_tpgid;
		gchar **stats = get_pid_stat(tmp_tpgid, 11);
		if (stats) current_tpgid = atoi(stats[9]);
		g_strfreev(stats);
	}
	// g_debug("The pid =%d, tpgid=%d", pid, current_tpgid);
	return current_tpgid;
}

// It will return NULL if fault
// The returned string should be freed when no longer needed.
gchar *get_cmdline(const pid_t tpgid)
{
#ifdef DETAIL
	g_debug("! Launch get_cmdline() for tpgid %d", tpgid);
#endif
#ifdef SAFEMODE
	if (tpgid<1) return NULL;
#endif
	gsize length = 0;
	gchar *cmdline = get_proc_data(tpgid, "cmdline", &length);
	if (length)
	{
		// g_debug("Get length = %d, cmdline = %s", length, cmdline);
		gint i;
		// convert '\0' to ' '
		for (i=0; i<(length-1); i++)
			if (*(cmdline+i)==0)
				*(cmdline+i)=' ';
	}
	return cmdline;
}

gboolean check_is_root(pid_t tpgid)
{
#ifdef DETAIL
	g_debug("! Launch check_is_root() for tpgid %d", tpgid);
#endif
#ifdef SAFEMODE
	if (tpgid<1) return FALSE;
#endif
	gchar *tpgid_path = g_strdup_printf("%s/%d", proc_file_system_path, tpgid);
#ifdef SAFEMODE
	if (tpgid_path==NULL) return FALSE;
#endif
	gboolean is_root = FALSE;
	struct stat buf;
	// Don't use g_stat here. 'CAUSE A BUG IN GDK, IT MAY CAUSE SEGFAULT UNDER I386 MACHINE!!
	if (! stat(tpgid_path, &buf))
	{
		// g_debug("Got uid of %s is %d", tpgid_path, buf.st_uid);
		// g_debug("Got gid of %s is %d", tpgid_path, buf.st_gid);
		if ((buf.st_uid==0) ||(buf.st_gid==0))
			is_root = TRUE;
	}
	g_free(tpgid_path);
	return is_root;
}

//gboolean check_is_root(pid_t tpgid)
//{
//#ifdef DETAIL
//	g_debug("! Launch check_is_root() for tpgid %d", tpgid);
//#endif
//#ifdef SAFEMODE
//	if (tpgid<1) return FALSE;
//#endif
//	gboolean is_root=FALSE;
//	gboolean uid_checked = FALSE, gid_checked = FALSE;
//
//	gsize length = 0;
//	gchar *status = get_proc_data(tpgid, "status", &length);
//
//#ifdef SAFEMODE
//	if (status)
//	{
//#endif
//		gchar **status_line = split_string(status, "\n", -1);
//		gchar **status_data;
//		gint i=0;
//
//#ifdef SAFEMODE
//		if (status_line)
//		{
//#endif
//			while (status_line[i]!=NULL)
//			{
//				// g_debug("%d) %s",i ,status_line[i]);
//				status_data = split_string(status_line[i], "\t", -1);
//#ifdef SAFEMODE
//				if (status_data)
//				{
//#endif
//					if (strcmp(status_data[0], "Uid:")==0)
//					{
//						is_root = check_status_data(status_data);
//						uid_checked = TRUE;
//					}
//					else if (strcmp(status_data[0], "Gid:")==0)
//					{
//						is_root = check_status_data(status_data);
//						gid_checked = TRUE;
//					}
//#ifdef SAFEMODE
//				}
//#endif
//				g_strfreev(status_data);
//				if ((is_root) || (uid_checked && gid_checked))
//					break;
//				i++;
//			}
//#ifdef SAFEMODE
//		}
//#endif
//		g_strfreev(status_line);
//#ifdef SAFEMODE
//	}
//#endif
//	g_free(status);
//	return is_root;
//}
//
//gboolean check_status_data(gchar **status_data)
//{
//#ifdef DETAIL
//	g_debug("! Launch check_status_data() !");
//#endif
//
//	gint i;
//	for (i=1; i<5; i++)
//	{
//		// g_debug("Checking %s...", status_data[i]);
//		if (atoi(status_data[i])==0)
//		{
//			// g_debug("IT IS ROOT (%d)!", atoi(status_data[i]));
//			return TRUE;
//		}
//	}
//	return FALSE;
//}

void update_page_window_title (VteTerminal *vte, struct Page *page_data)
{
#ifdef DETAIL
	g_debug("! Launch update_page_window_title() !");
#endif
#ifdef SAFEMODE
	if (page_data==NULL) return;
#endif
	page_data->window_title_updated = 1;
	// get_and_update_page_name(page_data);
	monitor_cmdline(page_data);
}

// The returned string should be freed when no longer needed.
// gchar *get_current_pwd_by_pid(pid_t pid)
// {
// #ifdef DETAIL
//	g_debug("! Launch get_current_pwd_by_pid() with pid = %d", pid);
// #endif
//	if (! proc_exist) return NULL;
//
//	gchar *priv_pwd = g_strdup_printf("%s/%d/cwd", proc_file_system_path, pid);
//	gchar *pwd = g_file_read_link(priv_pwd, NULL);
//	g_free(priv_pwd);
//	// g_debug("use the directory %s (%s/%d/cwd)", directory, proc_file_system_path, prev_page->pid);
//	return pwd;
//}

//void monitor_cmdline(GFileMonitor *monitor, pid_t pid)
//{
//	GError *error = NULL;
//	gchar *stat_path = g_strdup_printf("%s/%d/stat", proc_file_system_path, (gint) pid);
//	GFile *file = g_file_new_for_path(stat_path);
//	monitor = g_file_monitor_file (file, 0, NULL, &error);
//	if (monitor)
//		g_signal_connect (monitor, "changed", (GCallback)monitor_cmdline_changed, NULL);
//	else
//	{
//		g_free (monitor);
//		monitor = NULL;
//	}
//}

//void monitor_cmdline_changed(GFileMonitor *monitor, GFile *file, GFile *other_file,
//			     GFileMonitorEvent event_type, gpointer user_data)
//{
//	g_debug("Got monitor_cmdline_changed message");
//	struct Page *page_data = (struct Page *)g_object_get_data(G_OBJECT(current_vte), "Page_Data");
//	gint current_page_no = gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook));
//
//	get_and_update_page_name(page_data->label_text, page_data->pid, &(page_data->tpgid), current_page_no+1);
//}

//void monitor_cmdline_cancel(GFileMonitor *monitor)
//{
//	g_io_channel_close(channel);
//	g_file_monitor_cancel (monitor);
//}

