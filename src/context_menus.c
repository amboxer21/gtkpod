/*
|  Copyright (C) 2003 Corey Donohoe <atmos at atmos dot org>
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

#include "support.h"
#include "prefs.h"
#include "display.h"
#include "song.h"
#include "playlist.h"
#include "misc.h"
#include "file_export.h"

static guint entry_inst = -1;
static GList *selected_songs = NULL;
static Playlist *selected_playlist = NULL;
static TabEntry *selected_entry = NULL; 
/* types of context menus (PM/ST/SM) */
typedef enum {
    CM_PM = 0,
    CM_ST,
    CM_SM,
    CM_NUM
} CM_type;


/**
 * export_entries - export the currently selected files to disk
 * @mi - the menu item selected
 * @data - ignored, shoould be NULL
 */
static void 
export_entries(GtkWidget *w, gpointer data)
{
    if(selected_songs)
	file_export_init(selected_songs);
}

/**
 * edit_entries - open a dialog to edit the song(s) or playlist
 * @mi - the menu item selected
 * @data - Ignored, should be NULL
 * FIXME: How this should work needs to be decided
 */
#if 0
static void 
edit_entries(GtkButton *b, gpointer data)
{

    fprintf(stderr, "edit entries Selected\n");
}
#endif

/*
 * play_entries_now - play the entries currently selected in xmms
 * @mi - the menu item selected
 * @data - Ignored, should be NULL
 */
static void 
play_entries_now (GtkMenuItem *mi, gpointer data)
{
    play_songs (selected_songs);
}

/*
 * play_entries_now - play the entries currently selected in xmms
 * @mi - the menu item selected
 * @data - Ignored, should be NULL
 */
static void 
play_entries_enqueue (GtkMenuItem *mi, gpointer data)
{
    enqueue_songs (selected_songs);
}


/*
 * update_entries - update the entries currently selected
 * @mi - the menu item selected
 * @data - Ignored, should be NULL
 */
static void 
update_entries(GtkMenuItem *mi, gpointer data)
{
    if (selected_playlist)
	update_selected_playlist();
    else if(selected_entry)
	update_selected_entry(entry_inst);
    else if(selected_songs)
	update_selected_songs();
}

/**
 * delete_entries - delete the currently selected entry, be it a playlist,
 * items in a sort tab, or songs
 * @mi - the menu item selected
 * @data - ignored, should be NULL
 */
static void 
delete_entries(GtkMenuItem *mi, gpointer data)
{
    if (selected_playlist)
	delete_playlist_head();
    else if(selected_entry)
	delete_entry_head(entry_inst);
    else if(selected_songs)
	delete_song_head();
}


/* Attach a menu item to your context menu */
/* @m - the GtkMenu we're attaching to
 * @str - a gchar* with the menu label
 * @stock - name of the stock icon (or NULL if none is to be used)
 * @func - the callback for when the item is selected (or NULL)
 * @mi - the GtkWidget we're gonna hook into the menu
 */
GtkWidget *hookup_mi (GtkWidget *m, gchar *str, gchar *stock, GCallback func)
{
    GtkWidget *mi;

    if (stock)
    {
	GtkWidget *image;
	mi = gtk_image_menu_item_new_with_mnemonic (str);
	image = gtk_image_new_from_stock (stock, GTK_ICON_SIZE_MENU);
	gtk_widget_show (image);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (mi), image);
    }
    else
    {
	mi = gtk_menu_item_new_with_label(str);
    }
    gtk_widget_show(mi);
    gtk_widget_set_sensitive(mi, TRUE);
    if (func)
	g_signal_connect(G_OBJECT(mi), "activate", func, NULL);
    gtk_menu_append(m, mi);
    return mi;
}


void
create_context_menu(CM_type type)
{
    static GtkWidget *menu[CM_NUM];

    if(!menu[type])
    {
	menu[type] =  gtk_menu_new();
#if 0
	hookup_mi (menu[type], _("Edit"), NULL, G_CALLBACK (edit_entries));
#endif
	hookup_mi (menu[type], _("Play Now"), "gtk-cdrom",
		   G_CALLBACK (play_entries_now));
	hookup_mi (menu[type], _("Enqueue"), "gtk-cdrom",
		   G_CALLBACK (play_entries_enqueue));
	hookup_mi (menu[type], _("Export"), "gtk-floppy",
		   G_CALLBACK (export_entries));
	hookup_mi (menu[type], _("Update"), "gtk-refresh",
		   G_CALLBACK (update_entries));
	hookup_mi (menu[type], _("Delete"), "gtk-delete",
		   G_CALLBACK (delete_entries));
	if (type != CM_SM)
	{
	    GtkWidget *mi;
	    GtkWidget *sub = gtk_menu_new ();
	    gtk_widget_show (sub);
	    mi = hookup_mi (menu[type], _("Alphabetize"), NULL, NULL);
	    gtk_menu_item_set_submenu (GTK_MENU_ITEM (mi), sub);
	    hookup_mi (sub, _("Ascending"), "gtk-sort-ascending",
		       G_CALLBACK (NULL));
	    hookup_mi (sub, _("Descending"), "gtk-sort-descending",
		       G_CALLBACK (NULL));
	    hookup_mi (sub, _("Reset"), "gtk-undo", G_CALLBACK (NULL));
	}
    }
    gtk_menu_popup(GTK_MENU(menu[type]), NULL, NULL,
	    NULL, NULL, 3, gtk_get_current_event_time()); 
}

/**
 * sm_context_menu_init - initialize the right click menu for songs
 */
void
sm_context_menu_init(void)
{
    selected_entry = NULL; 
    selected_playlist = NULL;
    entry_inst = -1;
    if (selected_songs)  g_list_free (selected_songs);
    selected_songs = sm_get_selected_songs();
    if(selected_songs)
    {
	create_context_menu (CM_SM);
    }
}
/**
 * pm_context_menu_init - initialize the right click menu for playlists 
 */
void
pm_context_menu_init(void)
{
    if (selected_songs)  g_list_free (selected_songs);
    selected_songs = NULL;
    selected_entry = NULL;
    entry_inst = -1;
    selected_playlist = pm_get_selected_playlist();
    if(selected_playlist)
    {
	selected_songs = g_list_copy (selected_playlist->members);
	create_context_menu (CM_PM);
    }
}
/**
 * st_context_menu_init - initialize the right click menu for sort tabs 
 * FIXME: This needs to be hooked in.
 */
void
st_context_menu_init(gint inst)
{
    if (selected_songs)  g_list_free (selected_songs);
    selected_songs = NULL;
    selected_playlist = NULL;
    selected_entry = st_get_selected_entry(inst);
    if(selected_entry)
    {
	entry_inst = inst;
	selected_songs = g_list_copy (selected_entry->members);
	create_context_menu (CM_ST);
    }
}
