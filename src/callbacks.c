/*
|  Copyright (C) 2002-2003 Jorg Schuler <jcsjcs at users.sourceforge.net>
|  Part of the gtkpod project.
| 
|  URL: http://gtkpod.sourceforge.net/
| 
|  This program is free software; you can redistribute it and/or modify
|  it under the terms of the GNU General Public License as published by
|  the Free Software Foundation; either version 2 of the License, or
|  (at your option) any later version.
| 
|  This program is distributed in the hope that it will be useful,
|  but WITHOUT ANY WARRANTY; without even the implied warranty of
|  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
|  GNU General Public License for more details.
| 
|  You should have received a copy of the GNU General Public License
|  along with this program; if not, write to the Free Software
|  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
| 
|  iTunes and iPod are trademarks of Apple
| 
|  This product is not supported/written/published by Apple!
*/

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <stdlib.h>

#include "callbacks.h"
#include "interface.h"
#include "support.h"

#include "misc.h"
#include "prefs.h"
#include "dirbrowser.h"
#include "file.h"
#include "display.h"
#include "prefs_window.h"
#include "file_export.h"
#include "charset.h"

void
on_add_files1_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    create_add_files_fileselector ();
}


void
on_add_directory1_activate             (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  create_dir_browser ();
}


void
on_export_itunes1_activate             (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  handle_export ();
}


void
on_quit1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  if (!widgets_blocked) gtkpod_main_quit ();
}


void
on_new_playlist1_activate              (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  add_new_playlist (_("New Playlist"), -1);
}

void
on_about1_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  open_about_window (); /* in misc.c */
}


void
on_add_files1_button                   (GtkButton       *button,
                                        gpointer         user_data)
{
  create_add_files_fileselector ();
}


void
on_add_directory1_button               (GtkButton       *button,
                                        gpointer         user_data)
{
  create_dir_browser ();
}

void
on_export_itunes1_button               (GtkButton       *button,
                                        gpointer         user_data)
{
  handle_export ();
}

gboolean
on_gtkpod_delete_event                 (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
    if (!widgets_blocked)
    {
	return gtkpod_main_quit ();
    }
    return TRUE; /* don't quit -- would cause numerous error messages */
}

gboolean
on_about_window_close                  (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
  close_about_window (); /* in misc.c */
  return FALSE;
}


void
on_about_window_close_button           (GtkButton       *button,
                                        gpointer         user_data)
{
  close_about_window (); /* in misc.c */
}

void
on_new_playlist_button                 (GtkButton       *button,
                                        gpointer         user_data)
{
  add_new_playlist (_("New Playlist"), -1);
}

void
on_sorttab_switch_page                 (GtkNotebook     *notebook,
                                        GtkNotebookPage *page,
                                        guint            page_num,
                                        gpointer         user_data)
{
  st_page_selected (notebook, page_num);
}


void
on_playlist_treeview_drag_data_get     (GtkWidget       *widget,
                                        GdkDragContext  *drag_context,
                                        GtkSelectionData *data,
                                        guint            info,
                                        guint            time,
                                        gpointer         user_data)
{
    GtkTreeSelection *ts = NULL;
    GString *reply = g_string_sized_new (2000);

    /* printf("sm drag get info: %d\n", info);*/
    if((data) && (ts = gtk_tree_view_get_selection(GTK_TREE_VIEW(widget))))
    {
	switch (info)
	{
	case DND_GTKPOD_IDLIST:
	    gtk_tree_selection_selected_foreach(ts,
				    on_pm_dnd_get_id_foreach, reply);
	    break;
	case DND_GTKPOD_PM_PATHLIST:
	    gtk_tree_selection_selected_foreach(ts,
				    on_dnd_get_path_foreach, reply);
	    break;
	case DND_TEXT_PLAIN:
	    gtk_tree_selection_selected_foreach(ts,
				    on_pm_dnd_get_file_foreach, reply);
	    break;
	}
    }
    gtk_selection_data_set(data, data->target, 8, reply->str, reply->len);
    g_string_free (reply, TRUE);
}


void
on_playlist_treeview_drag_data_received
                                        (GtkWidget       *widget,
                                        GdkDragContext  *drag_context,
                                        gint             x,
                                        gint             y,
                                        GtkSelectionData *data,
                                        guint            info,
                                        guint            time,
                                        gpointer         user_data)
{
    GtkTreeIter i;
    GtkTreePath *path = NULL;
    GtkTreeModel *model = NULL;
    GtkTreeViewDropPosition pos = 0;
    gint position = -1;
    Playlist *pl = NULL;

    /* sometimes we get empty dnd data, ignore */
    if((!data) || (data->length < 0)) return;
    /* yet another check, i think it's an 8 bit per byte check */
    if(data->format != 8) return;
    if(gtk_tree_view_get_dest_row_at_pos(GTK_TREE_VIEW(widget),
					 x, y, &path, &pos))
    {
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
	if(gtk_tree_model_get_iter(model, &i, path))
	{
	    gtk_tree_model_get(model, &i, 0, &pl, -1);
	}
	/* get position of current path */
	position = atoi (gtk_tree_path_to_string (path));
	/* adjust position */
	if (pos == GTK_TREE_VIEW_DROP_AFTER)  ++position;
	/* don't allow drop _before_ MPL */
	if (position == 0) ++position;
	switch (info)
	{
	case DND_GTKPOD_IDLIST:
	    if(pl)
	    {
		if ((pos == GTK_TREE_VIEW_DROP_INTO_OR_BEFORE) ||
		    (pos == GTK_TREE_VIEW_DROP_INTO_OR_AFTER))
		{ /* drop into existing playlist */
		    if (pl->type == PL_TYPE_NORM)
		    {
			add_idlist_to_playlist (pl, data->data);
			gtk_drag_finish (drag_context, TRUE, FALSE, time);
		    }
		    else gtk_drag_finish (drag_context, FALSE, FALSE, time);
		}
		else
		{ /* drop between playlists */
		    Playlist *plitem = NULL;
		    plitem = add_new_playlist (_("New Playlist"), position);
		    add_idlist_to_playlist (plitem, data->data);
		    gtk_drag_finish (drag_context, TRUE, FALSE, time);
		}
	    }
	    else gtk_drag_finish (drag_context, FALSE, FALSE, time);
	    break;
	case DND_TEXT_PLAIN:
	    if(pl)
	    {
		if ((pos == GTK_TREE_VIEW_DROP_INTO_OR_BEFORE) ||
		    (pos == GTK_TREE_VIEW_DROP_INTO_OR_AFTER))
		{ /* drop into existing playlist */
		    add_text_plain_to_playlist (pl, data->data, 0, NULL, NULL);
		    gtk_drag_finish (drag_context, TRUE, FALSE, time);
		}
		else
		{ /* drop between playlists */
		    add_text_plain_to_playlist (NULL, data->data, position,
						NULL, NULL);
		    gtk_drag_finish (drag_context, TRUE, FALSE, time);
		}
	    }
	    else gtk_drag_finish (drag_context, FALSE, FALSE, time);
	    break;
	case DND_GTKPOD_PM_PATHLIST:
	    /* dont allow moves before MPL */
	    position = atoi (gtk_tree_path_to_string (path));
	    if (position == 0)  pos = GTK_TREE_VIEW_DROP_AFTER;
	    pm_move_pathlist (data->data, path, pos);
	    gtk_drag_finish (drag_context, TRUE, FALSE, time);
	    break;
	default:
	    puts ("not yet implemented");
	    gtk_drag_finish (drag_context, FALSE, FALSE, time);
	    break;
	}
	gtk_tree_path_free(path);
    }
}



void
on_song_treeview_drag_data_get         (GtkWidget       *widget,
                                        GdkDragContext  *drag_context,
                                        GtkSelectionData *data,
                                        guint            info,
                                        guint            time,
                                        gpointer         user_data)
{
    GtkTreeSelection *ts = NULL;
    GString *reply = g_string_sized_new (2000);

    /* printf("sm drag get info: %d\n", info);*/
    if((data) && (ts = gtk_tree_view_get_selection(GTK_TREE_VIEW(widget))))
    {
	switch (info)
	{
	case DND_GTKPOD_IDLIST:
	    gtk_tree_selection_selected_foreach(ts,
				    on_sm_dnd_get_id_foreach, reply);
	    break;
	case DND_GTKPOD_SM_PATHLIST:
	    gtk_tree_selection_selected_foreach(ts,
				    on_dnd_get_path_foreach, reply);
	    break;
	case DND_TEXT_PLAIN:
	    gtk_tree_selection_selected_foreach(ts,
				    on_sm_dnd_get_file_foreach, reply);
	    break;
	}
    }
    gtk_selection_data_set(data, data->target, 8, reply->str, reply->len);
    g_string_free (reply, TRUE);
}


gboolean
on_prefs_window_delete_event           (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
  prefs_window_delete ();
  gtkpod_statusbar_message(_("Preferences not updated"));
  return FALSE;
}


void
on_cfg_mount_point_changed             (GtkEditable     *editable,
                                        gpointer         user_data)
{
    gchar *buf = gtk_editable_get_chars(editable,0, -1);
    prefs_window_set_mount_point(buf);
    g_free (buf);
}


void
on_cfg_md5songs_toggled                (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
    prefs_window_set_md5songs(gtk_toggle_button_get_active(togglebutton));
}


void
on_cfg_id3_write_toggled                (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
    prefs_window_set_id3_write(gtk_toggle_button_get_active(togglebutton));
}


void
on_prefs_ok_clicked                    (GtkButton       *button,
                                        gpointer         user_data)
{
    prefs_window_ok();
}


void
on_prefs_cancel_clicked                (GtkButton       *button,
                                        gpointer         user_data)
{
    prefs_window_cancel();
    gtkpod_statusbar_message(_("Preferences not updated"));
}


void
on_prefs_apply_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
{
    prefs_window_apply ();
    gtkpod_statusbar_message(_("Preferences applied"));
}

void
on_edit_preferences1_activate          (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    if(!widgets_blocked)  prefs_window_create(); 
}

gboolean
on_playlist_treeview_key_release_event (GtkWidget       *widget,
                                        GdkEventKey     *event,
                                        gpointer         user_data)
{
    guint mods;

    mods = event->state;

    if(!widgets_blocked && (mods & GDK_CONTROL_MASK))
    {
	switch(event->keyval)
	{
	    case GDK_d:
		delete_playlist_head ();
		break;
	    case GDK_u:
		update_selected_playlist ();
		break;
	    case GDK_n:
		add_new_playlist (_("New Playlist"), -1);
		break;
	    default:
		break;
	}

    }
  return FALSE;
}


gboolean
on_song_treeview_key_release_event     (GtkWidget       *widget,
                                        GdkEventKey     *event,
                                        gpointer         user_data)
{
    guint mods;
    mods = event->state;

    if(!widgets_blocked && (mods & GDK_CONTROL_MASK))
    {
	switch(event->keyval)
	{
	    case GDK_d:
		delete_song_head ();
		break;
	    case GDK_u:
		update_selected_songs ();
		break;
	    default:
		break;
	}
    }
    return FALSE;
}


void
on_export_files_to_disk_activate       (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    file_export_init(get_currently_selected_songs());
}


void
on_cfg_delete_playlist_toggled         (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
    prefs_window_set_delete_playlist(
	    gtk_toggle_button_get_active(togglebutton));
}


void
on_cfg_delete_track_from_playlist_toggled
                                        (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
    prefs_window_set_delete_song_playlist(
	    gtk_toggle_button_get_active(togglebutton));
}


void
on_cfg_delete_track_from_ipod_toggled  (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
    prefs_window_set_delete_song_ipod(
	    gtk_toggle_button_get_active(togglebutton));
}


void
on_cfg_autoimport_toggled              (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
    prefs_window_set_auto_import(
	    gtk_toggle_button_get_active(togglebutton));
}


void
on_cfg_keep_backups_toggled            (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
    prefs_window_set_keep_backups(
	    gtk_toggle_button_get_active(togglebutton));
}


void
on_cfg_write_extended_info_toggled     (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
    prefs_window_set_write_extended_info(
	    gtk_toggle_button_get_active(togglebutton));
}


void
on_offline1_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  prefs_set_offline (
     gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (menuitem)));
}

void
on_import_itunes_mi_activate           (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  handle_import ();
}


void
on_import_button_clicked               (GtkButton       *button,
                                        gpointer         user_data)
{
  handle_import ();
}


void
on_song_treeview_drag_data_received    (GtkWidget       *widget,
                                        GdkDragContext  *drag_context,
                                        gint             x,
                                        gint             y,
                                        GtkSelectionData *data,
                                        guint            info,
                                        guint            time,
                                        gpointer         user_data)
{
    GtkTreePath *path = NULL;
    GtkTreeModel *model = NULL;
    GtkTreeViewDropPosition pos = 0;
    gboolean result = FALSE;

    /* printf ("sm drop received info: %d\n", info); */

    /* sometimes we get empty dnd data, ignore */
    if(widgets_blocked || (!data) || (data->length < 0)) return;
    /* yet another check, i think it's an 8 bit per byte check */
    if(data->format != 8) return;

    model = gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
    if(gtk_tree_view_get_dest_row_at_pos(GTK_TREE_VIEW(widget),
					 x, y, &path, &pos))
    {
	switch (info)
	{
	case DND_GTKPOD_SM_PATHLIST:
	    result = sm_move_pathlist (data->data, path, pos);
	    break;
	case DND_GTKPOD_IDLIST:
	    printf ("idlist not supported yet\n");
	    break;
	case DND_TEXT_PLAIN:
	    result = sm_add_filelist (data->data, path, pos);
	    break;
	default:
	    printf ("unknown drop not supported\n");
	    break;
	}
	gtk_tree_path_free(path);
    }
}


void
on_charset_combo_entry_changed          (GtkEditable     *editable,
                                        gpointer         user_data)
{
    gchar *descr, *charset;

    descr = gtk_editable_get_chars (editable, 0, -1);
    charset = charset_from_description (descr);
    prefs_window_set_charset (charset);
    C_FREE (descr);
    C_FREE (charset);
}

void
on_delete_song_menu                    (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    delete_song_head ();
}


void
on_delete_playlist_menu                (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    delete_playlist_head ();
}


void
on_ipod_directories_menu               (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    ipod_directories_head ();
}
void
on_gtkpod_status_realize               (GtkWidget       *widget,
                                        gpointer         user_data)
{
    gtkpod_statusbar_init(widget);
}


void
on_songs_statusbar_realize             (GtkWidget       *widget,
                                        gpointer         user_data)
{
    gtkpod_songs_statusbar_init(widget);
}

void
on_cfg_id3_writeall_toggled            (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
    prefs_window_set_id3_writeall(gtk_toggle_button_get_active(togglebutton));
}


void
on_st_treeview_drag_data_get           (GtkWidget       *widget,
                                        GdkDragContext  *drag_context,
                                        GtkSelectionData *data,
                                        guint            info,
                                        guint            time,
                                        gpointer         user_data)
{
    GtkTreeSelection *ts = NULL;
    
    if((data) && (ts = gtk_tree_view_get_selection(GTK_TREE_VIEW(widget))))
    {
	if(info == DND_GTKPOD_IDLIST)	/* gtkpod/file */
	{
	    GString *reply = g_string_sized_new (2000);
	    gtk_tree_selection_selected_foreach(ts,
				    on_st_listing_drag_foreach, reply);
	    if(reply->len)
	    {
		gtk_selection_data_set(data, data->target, 8, reply->str,
				       reply->len);
	    }
	    g_string_free (reply, TRUE);
	}
	else if(info == DND_TEXT_PLAIN)
	{
	    fprintf(stderr, "received file of type \"text/plain\"\n");
	}
    }

}

/* delete selected entry in sort tab */
gboolean
on_st_treeview_key_release_event       (GtkWidget       *widget,
                                        GdkEventKey     *event,
                                        gpointer         user_data)
{
    guint mods;
    mods = event->state;

    if(!widgets_blocked && (mods & GDK_CONTROL_MASK))
    {
	switch(event->keyval)
	{
	    case GDK_d:
		delete_entry_head (st_get_instance_from_treeview (
				       GTK_TREE_VIEW (widget)));
		break;
	    case GDK_u:
		update_selected_entry (st_get_instance_from_treeview (
					   GTK_TREE_VIEW (widget)));
		break;
	    default:
		break;
	}

    }
  return FALSE;
}

void
on_cfg_mpl_autoselect_toggled          (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
    prefs_window_set_mpl_autoselect(gtk_toggle_button_get_active(togglebutton));
}


void
on_cfg_block_display_toggled           (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
    prefs_window_set_block_display(gtk_toggle_button_get_active(togglebutton));
}

void
on_stop_button_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
{
    stop_display_update (-1);
}

#if 0
gboolean
on_playlist_treeview_button_release_event
                                        (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data)
{
    printf ("Event delivered to %x\n", widget);
    printf (" GDKType: %d, x/y: %f/%f, button: %d\n",
	    event->type, event->x, event->y, event->button);
    return FALSE;
}
#endif

void
on_add_PL_button_clicked               (GtkButton       *button,
                                        gpointer         user_data)
{
    create_add_playlists_fileselector ();
}


void
on_add_playlist1_activate              (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    create_add_playlists_fileselector ();
}

void
on_update_songs_in_selected_playlist1_activate (GtkMenuItem     *menuitem,
						gpointer         user_data)
{
    update_selected_playlist ();
}

void
on_update_selected_songs1_activate            (GtkMenuItem     *menuitem,
					       gpointer         user_data)
{
    update_selected_songs ();
}


void
on_cfg_update_existing_toggled         (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
    prefs_window_set_update_existing(
	gtk_toggle_button_get_active(togglebutton));
}

void
on_save_song_order1_activate           (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    sm_rows_reordered ();
    pm_rows_reordered ();
}


void
on_cfg_show_duplicates_toggled         (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
    prefs_window_set_show_duplicates(
	gtk_toggle_button_get_active(togglebutton));

}


void
on_cfg_show_updated_toggled            (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
    prefs_window_set_show_updated(
	gtk_toggle_button_get_active(togglebutton));

}


void
on_cfg_show_non_updated_toggled        (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
    prefs_window_set_show_non_updated(
	gtk_toggle_button_get_active(togglebutton));

}


void
on_cfg_save_sorted_order_toggled       (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
    prefs_window_set_save_sorted_order(
	gtk_toggle_button_get_active(togglebutton));

}


void
on_sort_tab_num_combo_entry_changed    (GtkEditable     *editable,
                                        gpointer         user_data)
{
    gchar *buf;
    gint num;

    buf = gtk_editable_get_chars (editable, 0, -1);
    num = atoi (buf);
    prefs_window_set_sort_tab_num (num);
    C_FREE (buf);
}

void
on_toolbar_menu_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    prefs_set_display_toolbar (
	gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (menuitem)));
}


void
on_cfg_display_toolbar_toggled         (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
    prefs_window_set_display_toolbar(
	gtk_toggle_button_get_active(togglebutton));
}


void
on_more_sort_tabs_activate             (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    prefs_set_sort_tab_num (prefs_get_sort_tab_num()+1, TRUE);
}


void
on_less_sort_tabs_activate             (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    prefs_set_sort_tab_num (prefs_get_sort_tab_num()-1, TRUE);
}

void
on_cfg_toolbar_style_both_toggled      (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
    if (gtk_toggle_button_get_active(togglebutton))
    {
	prefs_window_set_toolbar_style (GTK_TOOLBAR_BOTH);
    }
}


void
on_cfg_toolbar_style_text_toggled      (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
    if (gtk_toggle_button_get_active(togglebutton))
    {
	prefs_window_set_toolbar_style (GTK_TOOLBAR_TEXT);
    }
}


void
on_cfg_toolbar_style_icons_toggled      (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
    if (gtk_toggle_button_get_active(togglebutton))
    {
	prefs_window_set_toolbar_style (GTK_TOOLBAR_ICONS);
    }
}

void
on_alpha_playlists0_activate           (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    pm_sort (GTK_SORT_ASCENDING);
}


void
on_alpha_sort_tab0_activate            (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    gint inst = get_sort_tab_number (
	_("Sort which sort tab in ascending order?"));

    if (inst != -1) st_sort (inst, GTK_SORT_ASCENDING);
}


void
on_alpha_playlist1_activate            (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    pm_sort (GTK_SORT_DESCENDING);
}


void
on_alpha_sort_tab1_activate            (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    gint inst = get_sort_tab_number (
	_("Sort which sort tab in descending order?"));

    if (inst != -1) st_sort (inst, GTK_SORT_DESCENDING);

}


/* update songs in tab entry */
void
on_update_tab_entry_activate        (GtkMenuItem     *menuitem,
				     gpointer         user_data)
{
    gint inst = get_sort_tab_number (
	_("Update selected entry of which sort tab?"));

    if (inst != -1) update_selected_entry (inst);
}


void
on_delete_tab_entry_activate           (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    gint inst = get_sort_tab_number (
	_("Delete selected entry of which sort tab?"));

    if (inst != -1)   delete_entry_head (inst);
}


void
on_reset_sorting_activate              (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    display_reset ();
}

void
on_play_now_path_entry_changed         (GtkEditable     *editable,
                                        gpointer         user_data)
{
    gchar *buf = gtk_editable_get_chars(editable,0, -1);
    prefs_window_set_play_now_path (buf);
    g_free (buf);
}


void
on_play_enqueue_path_entry_changed     (GtkEditable     *editable,
                                        gpointer         user_data)
{
    gchar *buf = gtk_editable_get_chars(editable,0, -1);
    prefs_window_set_play_enqueue_path (buf);
    g_free (buf);
}

