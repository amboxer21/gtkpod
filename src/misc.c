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

#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include "song.h"
#include "interface.h"
#include "misc.h"
#include "prefs.h"
#include "support.h"
#include "itunesdb.h"
#include "display.h"
#include "confirmation.h"
#include "prefs_window.h"
#include "dirbrowser.h"
#include "file.h"

GtkWidget *gtkpod_window = NULL;
static GtkWidget *about_window = NULL;
static GtkWidget *file_selector = NULL;
static GtkWidget *pl_file_selector = NULL;
static GtkWidget *gtkpod_statusbar = NULL;
static GtkWidget *gtkpod_songs_statusbar = NULL;
static guint statusbar_timeout_id = 0;

/* --------------------------------------------------------------*/
/* are widgets blocked at the moment? */
gboolean widgets_blocked = FALSE;
struct blocked_widget { /* struct to be kept in blocked_widgets */
    GtkWidget *widget;   /* widget that has been turned insensitive */
    gboolean  sensitive; /* state of the widget before */
};
/* --------------------------------------------------------------*/

/* turn the file selector insensitive (if it's open) */
static void block_file_selector (void)
{
    if (file_selector)
	gtk_widget_set_sensitive (file_selector, FALSE);
    if (pl_file_selector)
	gtk_widget_set_sensitive (pl_file_selector, FALSE);
}

/* turn the file selector sensitive (if it's open) */
static void release_file_selector (void)
{
    if (file_selector)
	gtk_widget_set_sensitive (file_selector, TRUE);
    if (pl_file_selector)
	gtk_widget_set_sensitive (pl_file_selector, TRUE);
}

/*------------------------------------------------------------------*\
 *                                                                  *
 *             Add Files File Selector                              *
 *                                                                  *
\*------------------------------------------------------------------*/

static void add_files_ok_button (GtkWidget *button, GtkFileSelection *selector)
{
  gchar **names;
  gint i;

  block_widgets ();
  names = gtk_file_selection_get_selections (GTK_FILE_SELECTION (selector));
  for (i=0; names[i] != NULL; ++i)
  {
      add_song_by_filename (names[i], NULL, NULL, NULL);
      if(i == 0)
	  prefs_set_last_dir_browse(names[i]);
  }
  /* clear log of non-updated songs */
  display_non_updated ((void *)-1, NULL);
  /* display log of updated songs */
  display_updated (NULL, NULL);
  /* display log of detected duplicates */
  remove_duplicate (NULL, NULL);
  gtkpod_statusbar_message(_("Successly Added Files"));
  gtkpod_songs_statusbar_update();
  release_widgets ();
  g_strfreev (names);
}

/* called when the file selector is closed */
static void add_files_close (GtkWidget *w1, GtkWidget *w2)
{
    if (file_selector)    gtk_widget_destroy(file_selector),
    gtkpod_songs_statusbar_update();
    file_selector = NULL;
}


void create_add_files_fileselector (void)
{
    if (file_selector) return; /* file selector already open -- abort */
    /* Create the selector */
    file_selector = gtk_file_selection_new (_("Select files or directories to add."));
    gtk_file_selection_set_select_multiple (GTK_FILE_SELECTION (file_selector),
					    TRUE);
    gtk_file_selection_set_filename(GTK_FILE_SELECTION (file_selector),
				    cfg->last_dir.browse);

    g_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION (file_selector)->ok_button),
		      "clicked",
		      G_CALLBACK (add_files_ok_button),
		      file_selector);

    /* Ensure that file_selector is set to NULL when window is deleted */
    g_signal_connect_swapped (GTK_OBJECT (file_selector),
			      "delete_event",
			      G_CALLBACK (add_files_close), 
			      (gpointer) file_selector); 

    /* Ensure that the dialog box is deleted when the user clicks a button. */
    g_signal_connect_swapped (GTK_OBJECT (GTK_FILE_SELECTION (file_selector)->ok_button),
			      "clicked",
			      G_CALLBACK (add_files_close), 
			      (gpointer) file_selector); 

    g_signal_connect_swapped (GTK_OBJECT (GTK_FILE_SELECTION (file_selector)->cancel_button),
			      "clicked",
			      G_CALLBACK (add_files_close),
			      (gpointer) file_selector); 

    /* Display that dialog */
    gtk_widget_show (file_selector);
}



/*------------------------------------------------------------------*\
 *                                                                  *
 *             Add Playlists File Selector                          *
 *                                                                  *
\*------------------------------------------------------------------*/


static void add_playlists_ok_button (GtkWidget *button, GtkFileSelection *selector)
{
  gchar **names;
  gint i;

  block_widgets ();
  names = gtk_file_selection_get_selections (GTK_FILE_SELECTION (selector));
  for (i=0; names[i] != NULL; ++i)
    {
      add_playlist_by_filename (names[i], NULL, NULL, NULL);
      if(i == 0)
	  prefs_set_last_dir_browse(names[i]);
    }
  gtkpod_songs_statusbar_update();
  release_widgets ();
  g_strfreev (names);
}

/* called when the file selector is closed */
static void add_playlists_close (GtkWidget *w1, GtkWidget *w2)
{
    if (pl_file_selector)    gtk_widget_destroy(pl_file_selector),
    gtkpod_songs_statusbar_update();
    pl_file_selector = NULL;
}


void create_add_playlists_fileselector (void)
{
    if (pl_file_selector) return; /* file selector already open -- abort */
    /* Create the selector */
    pl_file_selector = gtk_file_selection_new (_("Select Playlists to add."));
    gtk_file_selection_set_select_multiple (GTK_FILE_SELECTION (pl_file_selector),
					    TRUE);
    gtk_file_selection_set_filename(GTK_FILE_SELECTION (pl_file_selector),
				    cfg->last_dir.browse);

    g_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION (pl_file_selector)->ok_button),
		      "clicked",
		      G_CALLBACK (add_playlists_ok_button),
		      pl_file_selector);

    /* Ensure that pl_file_selector is set to NULL when window is deleted */
    g_signal_connect_swapped (GTK_OBJECT (pl_file_selector),
			      "delete_event",
			      G_CALLBACK (add_playlists_close), 
			      (gpointer) pl_file_selector); 

    /* Ensure that the dialog box is deleted when the user clicks a button. */
    g_signal_connect_swapped (GTK_OBJECT (GTK_FILE_SELECTION (pl_file_selector)->ok_button),
			      "clicked",
			      G_CALLBACK (add_playlists_close), 
			      (gpointer) pl_file_selector); 

    g_signal_connect_swapped (GTK_OBJECT (GTK_FILE_SELECTION (pl_file_selector)->cancel_button),
			      "clicked",
			      G_CALLBACK (add_playlists_close),
			      (gpointer) pl_file_selector); 

    /* Display that dialog */
    gtk_widget_show (pl_file_selector);
}



/* Concats "dir" and "file" into full filename, taking
   into account that "dir" may or may not end with "/".
   You must free the return string after use
   This code tries to take into account some stupid constellations
   when either "dir" or "file" is not set, or file starts with a "/"
   (ignore) etc.  */
gchar *concat_dir (G_CONST_RETURN gchar *dir, G_CONST_RETURN gchar *file)
{
    return itunesdb_concat_dir (dir, file);
}

/* Calculate the time in ms passed since @old_time. @old_time is
   updated with the current time if @update is TRUE*/
float get_ms_since (GTimeVal *old_time, gboolean update)
{
    GTimeVal new_time;
    float result;

    g_get_current_time (&new_time);
    result = (new_time.tv_sec - old_time->tv_sec) * 1000 +
	(float)(new_time.tv_usec - old_time->tv_usec) / 1000;
    if (update)
    {
	old_time->tv_sec = new_time.tv_sec;
	old_time->tv_usec = new_time.tv_usec;
    }
    return result;
}



void open_about_window ()
{
  GtkLabel *about_label;
  gchar *buffer_text, *label_text;
  GtkTextView *textview;

  if (about_window != NULL) return;
  about_window = create_gtkpod_about_window ();
  about_label = GTK_LABEL (lookup_widget (about_window, "about_label"));
  label_text = g_strdup_printf (_("gtkpod Version %s: Cross-Platform Multi-Lingual Interface to Apple's iPod(tm)."), VERSION);
  gtk_label_set_text (about_label, label_text);
  g_free (label_text);
  buffer_text = g_strdup_printf ("%s\n%s\n\n%s\n\n%s\n\n%s\n\n%s\n\n%s",
				 _("    (C) 2002 - 2003\n    Jorg Schuler (jcsjcs at users.sourceforge.net)"),
				 _("    Corey Donohoe (atmos at atmos.org)"),
				 _("This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.\n\nThis program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.\n\nYou should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA."),
				 _("The code handling the reading and writing of the iTunesDB was ported from mktunes.pl of the gnuPod package written by Adrian Ulrich (http://www.blinkenlights.ch/cgi-bin/fm.pl?get=ipode). Adrian Ulrich ported the playlist part."),
				 _("This program borrows code from the following projects:"),
				 _("  xmms:    dirbrowser\n  easytag: reading and writing of ID3 tags\n  GNOMAD:  playlength detection"),
				 _("The GUI was created with the help of glade-2."));
  textview = GTK_TEXT_VIEW (lookup_widget (about_window, "credits_textview"));
  gtk_text_buffer_set_text (gtk_text_view_get_buffer (textview),
			    buffer_text, -1);
  g_free (buffer_text);

  buffer_text = g_strdup_printf ("%s\n%s",
				 _("German:   Jorg Schuler (jcsjcs at users.sourceforge.net)"),
				 _("Japanese: Ayako Sano"));
  textview = GTK_TEXT_VIEW (lookup_widget (about_window, "translators_textview"));
  gtk_text_buffer_set_text (gtk_text_view_get_buffer (textview),
			    buffer_text, -1);
  g_free (buffer_text);

  gtk_widget_show (about_window);
}

void close_about_window (void)
{
  g_return_if_fail (about_window != NULL);
  gtk_widget_destroy (about_window);
  about_window = NULL;
}

/* parse a bunch of ipod ids delimited by \n
 * @s - address of the character string we're parsing (gets updated)
 * @id - pointer the ipod id parsed from the string
 * Returns FALSE when the string is empty, TRUE when the string can still be
 * 	parsed
 */
gboolean
parse_ipod_id_from_string(gchar **s, guint32 *id)
{
    if(s && (*s))
    {
	gchar *str = *s;
	gchar *strp = strchr (str, '\n');

	if (strp == NULL)
	{
	    *id = 0;
	    *s = NULL;
	    return FALSE;
	}
	*id = (guint32)atoi(str);
	++strp;
	if (*strp) *s = strp;
	else       *s = NULL;
	return TRUE;
    }
    return FALSE;
}



/* DND: add a list of iPod IDs to Playlist @pl */
void add_idlist_to_playlist (Playlist *pl, gchar *str)
{
    guint32 id = 0;
	    
    if (!pl) return;
    while(parse_ipod_id_from_string(&str,&id))
    {
	add_songid_to_playlist(pl, id, TRUE);
    }
    data_changed();
}

/* DND: add a list of files to Playlist @pl.  @pl: playlist to add to
   or NULL. If NULL, a "New Playlist" will be created for adding
   songs and when adding a playlist file, a playlist with the name of the
   playlist file will be added.
   @songaddfunc: passed on to add_song_by_filename() etc. */
void add_text_plain_to_playlist (Playlist *pl, gchar *str, gint pl_pos,
				 AddSongFunc songaddfunc, gpointer data)
{
    gchar **files = NULL, **filesp = NULL;
    gchar *file = NULL;
    Playlist *pl_playlist = pl; /* playlist for playlist file */

    if (!str)  return;

    /*   printf("pl: %x, pl_pos: %d\n%s\n", pl, pl_pos, str);*/

    block_widgets ();
    files = g_strsplit (str, "\n", -1);
    if (files)
    {
	filesp = files;
	while (*filesp)
	{
	    gboolean added;

	    added = FALSE;
	    file = *filesp;
	    if (strncmp (file, "file:", 5) == 0)
	    {
		file += 5;
		if (g_file_test (file, G_FILE_TEST_IS_DIR))
		{   /* directory */
		    if (!pl)
		    {  /* no playlist yet -- create new one */
			pl = add_new_playlist (_("New Playlist"), pl_pos);
		    }
		    add_directory_recursively (file, pl, songaddfunc, data);
		    added = TRUE;
		}
		if (g_file_test (file, G_FILE_TEST_IS_REGULAR))
		{   /* regular file */
		    gint len = strlen (file);

		    if (len >= 4)
		    {
			if (strcasecmp (&file[len-4], ".mp3") == 0)
			{   /* mp3 file */
			    if (!pl)
			    {  /* no playlist yet -- create new one */
				pl = add_new_playlist (
				    _("New Playlist"), pl_pos);
			    }
			    add_song_by_filename (file, pl, songaddfunc, data);
			    added = TRUE;
			}
			else if ((strcasecmp (&file[len-4], ".plu") == 0) ||
				 (strcasecmp (&file[len-4], ".m3u") == 0))
			{
			    add_playlist_by_filename (file, pl_playlist,
						      songaddfunc, data);
			    added = TRUE;
			}
		    }
		}
	    }
	    if (!added)
	    {
		if (strlen (*filesp) != 0)
		    gtkpod_warning (_("drag and drop: ignored '%s'"), *filesp);
	    }
	    ++filesp;
	}
	g_strfreev (files);
    }
    /* display log of non-updated songs */
    display_non_updated (NULL, NULL);
    /* display log updated songs */
    display_updated (NULL, NULL);
    /* display log of detected duplicates */
    remove_duplicate (NULL, NULL);

    release_widgets ();
}


void cleanup_backup_and_extended_files (void)
{
  gchar *cfgdir, *cft, *cfe;

  cfgdir = prefs_get_cfgdir ();
  /* in offline mode, there are no backup files! */
  if (cfgdir && !cfg->offline)
    {
      cft = concat_dir (cfgdir, "iTunesDB");
      cfe = concat_dir (cfgdir, "iTunesDB.ext");
      if (!cfg->write_extended_info)
	/* delete extended info file from computer */
	if (g_file_test (cfe, G_FILE_TEST_EXISTS))
	  if (remove (cfe) != 0)
	    gtkpod_warning (_("Could not delete backup file: \"%s\"\n"), cfe);
      if (!cfg->keep_backups)
	if(g_file_test (cft, G_FILE_TEST_EXISTS))
	  if (remove (cft) != 0)
	    gtkpod_warning (_("Could not delete backup file: \"%s\"\n"), cft);
      g_free (cft);
      g_free (cfe);
    }
  C_FREE (cfgdir);
}


/**
 * gtkpod_main_quit
 * 
 * return value: FALSE if it's OK to quit.
 */
gboolean
gtkpod_main_quit(void)
{
    GtkWidget *dialog;
    gint result = GTK_RESPONSE_YES;

    if (!files_are_saved ())
    {
	dialog = gtk_message_dialog_new (
	    GTK_WINDOW (gtkpod_window),
	    GTK_DIALOG_DESTROY_WITH_PARENT,
	    GTK_MESSAGE_WARNING,
	    GTK_BUTTONS_YES_NO,
	    _("Data has been changed and not been saved.\nOK to exit gtkpod?"));
	result = gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);
    }

    if (result == GTK_RESPONSE_YES)
    {
	remove_all_playlists ();  /* first remove playlists, then
				   * songs! (otherwise non-existing
				   *songs may be accessed) */
	remove_all_songs ();
	display_cleanup ();
	write_prefs (); /* FIXME: how can we avoid saving options set by
			 * command line? */
	gtk_main_quit ();
	return FALSE;
    }
    return TRUE;
}

/**
 * disable_import_buttons
 * Upon successfull itunes db importing we want to disable the import
 * buttons.  This retrieves the import buttons from the main gtkpod widget
 * and disables them from taking input.
 */
void
disable_gtkpod_import_buttons(void)
{
    GtkWidget *w = NULL;
    
    if(gtkpod_window)
    {
	if((w = lookup_widget(gtkpod_window, "import_button")))
	{
	    gtk_widget_set_sensitive(w, FALSE);
	    /* in case this widget has been blocked, we need to tell
	       update the desired state upon release */
	    update_blocked_widget (w, FALSE);
	}
	if((w = lookup_widget(gtkpod_window, "import_itunes_mi")))
	{
	    gtk_widget_set_sensitive(w, FALSE);
	    /* in case this widget has been blocked, we need to tell
	       update the desired state upon release */
	    update_blocked_widget (w, FALSE);
	}
    }
}


/* Let the user select a sort tab number */
/* @text: text to be displayed */
/* return value: -1: user selected cancel 
   0...prefs_get_sort_tab_number()-1: selected tab */
gint get_sort_tab_number (gchar *text)
{
    static gint last_nr = 1;
    GtkWidget *mdialog;
    GtkDialog *dialog;
    GtkWidget *combo;
    gint result;
    gint i, nr;
    GList *list=NULL, *lnk;
    gchar buf[20], *bufp;

    mdialog = gtk_message_dialog_new (
	GTK_WINDOW (gtkpod_window),
	GTK_DIALOG_DESTROY_WITH_PARENT,
	GTK_MESSAGE_QUESTION,
	GTK_BUTTONS_OK_CANCEL,
	text);

    dialog = GTK_DIALOG (mdialog);

    combo = gtk_combo_new ();
    gtk_widget_show (combo);
    gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox), combo);

    /* Create list */
    for (i=1; i<=prefs_get_sort_tab_num (); ++i)
    {
	bufp = g_strdup_printf ("%d", i);
	list = g_list_append (list, bufp);
    }

    /* set pull down items */
    gtk_combo_set_popdown_strings (GTK_COMBO (combo), list);
    /* set standard entry */
    snprintf (buf, 20, "%d", last_nr);
    gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (combo)->entry), buf);

    result = gtk_dialog_run (GTK_DIALOG (mdialog));

    /* free the list */
    for (lnk = list; lnk; lnk = lnk->next)
    {
	C_FREE (lnk->data);
    }
    g_list_free (list);
    list = NULL;

    if (result == GTK_RESPONSE_CANCEL)
    {
	nr = -1;  /* no selection */
    }
    else
    {
	bufp = gtk_editable_get_chars (GTK_EDITABLE (GTK_COMBO (combo)->entry),
				      0, -1);
	nr = atoi (bufp)-1;
	last_nr = nr;
	C_FREE (bufp);
    }

    gtk_widget_destroy (mdialog);

    return nr;
}



void
gtkpod_statusbar_init(GtkWidget *sb)
{
    gtkpod_statusbar = sb;
}

static gint
gtkpod_statusbar_clear(gpointer data)
{
    if(gtkpod_statusbar)
    {
	gtk_statusbar_pop(GTK_STATUSBAR(gtkpod_statusbar), 1);
    }
    statusbar_timeout_id = 0; /* indicate that timeout handler is
				 clear (0 cannot be a handler id) */
    return FALSE;
}

void
gtkpod_statusbar_message(const gchar *message)
{
    if(gtkpod_statusbar)
    {
	gchar buf[PATH_MAX];
	guint context = 1;
	     
	snprintf(buf, PATH_MAX, "  %s", message);
	gtk_statusbar_pop(GTK_STATUSBAR(gtkpod_statusbar), context);
	gtk_statusbar_push(GTK_STATUSBAR(gtkpod_statusbar), context,  buf);
	if (statusbar_timeout_id != 0) /* remove last timeout, if still present */
	    gtk_timeout_remove (statusbar_timeout_id);
	statusbar_timeout_id = gtk_timeout_add(prefs_get_statusbar_timeout (),
					       (GtkFunction) gtkpod_statusbar_clear,
					       NULL);
    }
}

void 
gtkpod_songs_statusbar_init(GtkWidget *w)
{
    gtkpod_songs_statusbar = w;
    gtkpod_songs_statusbar_update();
}

void 
gtkpod_songs_statusbar_update(void)
{
    if(gtkpod_songs_statusbar)
    {
	gchar *buf;

	buf = g_strdup_printf (_(" P:%d S:%d/%d"),
			       get_nr_of_playlists () - 1,
			       sm_get_nr_of_songs (),
			       get_nr_of_songs ());
	gtk_statusbar_pop(GTK_STATUSBAR(gtkpod_songs_statusbar), 1);
	gtk_statusbar_push(GTK_STATUSBAR(gtkpod_songs_statusbar), 1,  buf);
	g_free (buf);
    }

}


/* translates a SM_COLUMN_... (defined in display.h) into a
 * S_... (defined in song.h). Returns -1 in case a translation is not
 * possible */
S_item SM_to_S (SM_item sm)
{
    switch (sm)
    {
    case SM_COLUMN_TITLE:       return S_TITLE;
    case SM_COLUMN_ARTIST:      return S_ARTIST;
    case SM_COLUMN_ALBUM:       return S_ALBUM;
    case SM_COLUMN_GENRE:       return S_GENRE;
    case SM_COLUMN_COMPOSER:    return S_COMPOSER;
    case SM_COLUMN_TRACK_NR:    return S_TRACK_NR;
    case SM_COLUMN_IPOD_ID:     return S_IPOD_ID;
    case SM_COLUMN_PC_PATH:     return S_PC_PATH;
    case SM_COLUMN_TRANSFERRED: return S_TRANSFERRED;
    case SM_COLUMN_NONE:        return S_NONE;
    case SM_NUM_COLUMNS:        return -1;
    }
    return -1;
}


/* translates a ST_CAT_... (defined in display.h) into a
 * S_... (defined in song.h). Returns -1 in case a translation is not
 * possible */
S_item ST_to_S (ST_CAT_item st)
{
    switch (st)
    {
    case ST_CAT_ARTIST:      return S_ARTIST;
    case ST_CAT_ALBUM:       return S_ALBUM;
    case ST_CAT_GENRE:       return S_GENRE;
    case ST_CAT_COMPOSER:    return S_COMPOSER;
    case ST_CAT_TITLE:       return S_TITLE;
    case ST_CAT_NUM:         return -1;
    }
    return -1;
}

/* return some sensible input about the "song". Yo must free the
 * return string after use. */
gchar *get_song_info (Song *song)
{
    if (!song) return NULL;
    if (song->pc_path_utf8 && strlen(song->pc_path_utf8))
	return g_path_get_basename (song->pc_path_utf8);
    else if ((song->title && strlen(song->title)))
	return g_strdup (song->title);
    else if ((song->album && strlen(song->album)))
	return g_strdup (song->album);
    else if ((song->artist && strlen(song->artist)))
	return g_strdup (song->artist);
    return g_strdup_printf ("iPod ID: %d", song->ipod_id);
}

/*------------------------------------------------------------------*\
 *                                                                  *
 *             Functions for blocking widgets                       *
 *                                                                  *
\*------------------------------------------------------------------*/

enum {
    BR_BLOCK,
    BR_RELEASE,
    BR_UPDATE
};

/* function to add one widget to the blocked_widgets list */
static GList *add_blocked_widget (GList *blocked_widgets, gchar *name)
{
    GtkWidget *w;
    struct blocked_widget *bw;

    if((w = lookup_widget(gtkpod_window,  name)))
    {
	bw = g_malloc0 (sizeof (struct blocked_widget));
	bw->widget = w;
	/* we don't have to set the sensitive flag right now. It's
	 * done in "block_widgets ()" */
	blocked_widgets = g_list_append (blocked_widgets, bw);
    }
    return blocked_widgets;
}

/* called by block_widgets() and release_widgets() */
/* "block": TRUE = block, FALSE = release */
static void block_release_widgets (gint action, GtkWidget *w, gboolean sens)
{
    /* list with the widgets that are turned insensitive during
       import/export...*/
    static GList *bws = NULL;
    static gint count = 0; /* how many times are the widgets blocked? */
    GList *l;
    struct blocked_widget *bw;

    /* Create a list of widgets that are to be turned insensitive when
     * importing/exporting, adding songs or directories etc. */
    if (bws == NULL)
    {
	bws = add_blocked_widget (bws, "menubar");
	bws = add_blocked_widget (bws, "import_button");
	bws = add_blocked_widget (bws, "add_files_button");
	bws = add_blocked_widget (bws, "add_dirs_button");
	bws = add_blocked_widget (bws, "add_PL_button");
	bws = add_blocked_widget (bws, "new_PL_button");
	bws = add_blocked_widget (bws, "export_button");
	widgets_blocked = FALSE;
    }

    switch (action)
    {
    case BR_BLOCK:
	/* we must block the widgets */
	++count;  /* increase number of locks */
	if (!widgets_blocked)
	{ /* only block widgets, if they are not already blocked */
	    for (l = bws; l; l = l->next)
	    {
		bw = (struct blocked_widget *)l->data;
		/* remember the state the widget was in before */
		bw->sensitive = GTK_WIDGET_SENSITIVE (bw->widget);
		gtk_widget_set_sensitive (bw->widget, FALSE);
	    }
	    block_prefs_window ();
	    block_file_selector ();
	    block_dirbrowser ();
	    widgets_blocked = TRUE;
	}
	break;
    case BR_RELEASE:
	/* release the widgets if --count == 0 */
	if (widgets_blocked)
	{ /* only release widgets, if they are blocked */
	    --count;
	    if (count == 0)
	    {
		for (l = bws; l; l = l->next)
		{
		    bw = (struct blocked_widget *)l->data;
		    gtk_widget_set_sensitive (bw->widget, bw->sensitive);
		}
		release_prefs_window ();
		release_file_selector ();
		release_dirbrowser ();
		widgets_blocked = FALSE;
	    }
	}
	break;
    case BR_UPDATE:
	if (widgets_blocked)
	{ /* only update widgets, if they are blocked */
	    for (l = bws; l; l = l->next)
	    { /* find the required widget */
		bw = (struct blocked_widget *)l->data;
		if (bw->widget == w)
		{ /* found -> set to new desired state */
		    bw->sensitive = sens;
		    break;
		}
	    }
	}
	break;
    }
}


/* Block widgets (turn insensitive) listed in "bws" */
void block_widgets (void)
{
    block_release_widgets (BR_BLOCK, NULL, FALSE);
}

/* Release widgets (i.e. return them to their state before
   "block_widgets() was called */
void release_widgets (void)
{
    block_release_widgets (BR_RELEASE, NULL, FALSE);
}

void update_blocked_widget (GtkWidget *w, gboolean sens)
{
    block_release_widgets (BR_UPDATE, w, sens);
}

/*------------------------------------------------------------------*\
 *                                                                  *
 *             Create iPod directory hierarchy                      *
 *                                                                  *
\*------------------------------------------------------------------*/


/* ok handler for ipod directory creation */
/* @user_data1 is the mount point of the iPod */
static void ipod_directories_ok (gpointer user_data1, gpointer user_data2)
{
    gchar *mp = user_data1;
    gboolean success = TRUE;
    gchar pbuf[PATH_MAX+1];
    gchar *buf;
    gint i;

    if (mp)
    {
	snprintf(pbuf, PATH_MAX, "%s/Calendars", mp);
	if((mkdir(pbuf, 0755) != 0)) success = FALSE;
	snprintf(pbuf, PATH_MAX, "%s/Contacts", mp);
	if((mkdir(pbuf, 0755) != 0)) success = FALSE;
	snprintf(pbuf, PATH_MAX, "%s/iPod_Control", mp);
	if((mkdir(pbuf, 0755) != 0)) success = FALSE;
	snprintf(pbuf, PATH_MAX, "%s/iPod_Control/Music", mp);
	if((mkdir(pbuf, 0755) != 0)) success = FALSE;
	snprintf(pbuf, PATH_MAX, "%s/iPod_Control/iTunes", mp);
	if((mkdir(pbuf, 0755) != 0)) success = FALSE;
	for(i = 0; i < 20; i++)
	{
	    snprintf(pbuf, PATH_MAX, "%s/iPod_Control/Music/F%02d", mp, i);
	    if((mkdir(pbuf, 0755) != 0)) success = FALSE;
	}

	if (success)
	    buf = g_strdup_printf (_("Successfully created iPod directories in '%s'."), mp);
	else
	    buf = g_strdup_printf (_("Problem creating iPod directories in '%s'."), mp);
	gtkpod_statusbar_message(buf);
	g_free (buf);
	g_free (mp);
    }
}


/* cancel handler for ipod directory creation */
static void ipod_directories_cancel (gpointer user_data1, gpointer user_data2)
{
    C_FREE (user_data1);
}


/* Pop up the confirmation window for creation of ipod directory
   hierarchy */
void ipod_directories_head (void)
{
    gchar *mp;
    GString *str;

    mp = prefs_get_ipod_mount ();
    if (mp)
    {
	if (strlen (mp) > 0)
	{ /* make sure the mount point does not end in "/" */
	    if (mp[strlen (mp) - 1] == '/')
		mp[strlen (mp) - 1] = 0;
	}
    }
    else
    {
	mp = g_strdup (".");
    }
    str = g_string_sized_new (2000);
    g_string_append_printf (str, "%s/Calendars\n", mp);
    g_string_append_printf (str, "%s/Contacts\n", mp);
    g_string_append_printf (str, "%s/iPod_Control\n", mp);
    g_string_append_printf (str, "%s/iPod_Control/Music\n", mp);
    g_string_append_printf (str, "%s/iPod_Control/iTunes\n", mp);
    g_string_append_printf (str, "%s/iPod_Control/Music/F00\n...\n", mp);
    g_string_append_printf (str, "%s/iPod_Control/Music/F19\n", mp);

    if (!gtkpod_confirmation (CONF_ID_IPOD_DIR,    /* gint id, */
			 FALSE,               /* gboolean modal, */
			 _("Create iPod directories"), /* title */
			 _("OK to create the following directories?"),
			 str->str,
			 TRUE,               /* gboolean confirm_again, */
			 NULL, /* ConfHandlerCA confirm_again_handler, */
			 ipod_directories_ok, /* ConfHandler ok_handler,*/
			 CONF_NO_BUTTON,      /* don't show "Apply" */
			 ipod_directories_cancel, /* cancel_handler,*/
			 mp,                  /* gpointer user_data1,*/
			 NULL))              /* gpointer user_data2,*/
    { /* creation failed */
	g_free (mp);
    }
    g_string_free (str, TRUE);
}


/*------------------------------------------------------------------*\
 *                                                                  *
 *             Delete Playlist                                      *
 *                                                                  *
\*------------------------------------------------------------------*/


/* ok handler for delete playlist */
/* @user_data1 is the selected playlist */
static void delete_playlist_ok (gpointer user_data1, gpointer user_data2)
{
    Playlist *selected_playlist = (Playlist *)user_data1;
    gchar *buf;

    buf = g_strdup_printf (_("Deleted playlist '%s'"),
			   selected_playlist->name);
    remove_playlist (selected_playlist);
    gtkpod_statusbar_message (buf);
    g_free (buf);
}

void delete_playlist_head (void)
{
    gchar *buf;
    Playlist *pl = NULL;

    pl = get_currently_selected_playlist();
    if (!pl)
    { /* no playlist selected */
	gtkpod_statusbar_message (_("No playlist selected."));
	return;
    }
    if (pl->type == PL_TYPE_MPL)
    { /* master playlist */
	gtkpod_statusbar_message (_("Cannot delete master playlist."));
	return;
    }
    
    buf = g_strdup_printf(_("Are you sure you want to delete the playlist '%s'?"), pl->name);

    gtkpod_confirmation
	(-1,                   /* gint id, */
	 TRUE,                 /* gboolean modal, */
	 _("Delete Playlist?"), /* title */
	 buf,                   /* label */
	 NULL,                  /* scrolled text */
	 prefs_get_playlist_deletion (),   /* gboolean confirm_again, */
	 prefs_set_playlist_deletion, /* ConfHandlerCA confirm_again_handler,*/
	 delete_playlist_ok, /* ConfHandler ok_handler,*/
	 CONF_NO_BUTTON,     /* don't show "Apply" button */
	 NULL,               /* cancel_handler,*/
	 pl,                 /* gpointer user_data1,*/
	 NULL);              /* gpointer user_data2,*/

    g_free (buf);
}



/*------------------------------------------------------------------*\
 *                                                                  *
 *             Delete Songs                                         *
 *                                                                  *
\*------------------------------------------------------------------*/


/* This is the same for delete_song_head() and delete_st_head(), so I
 * moved it here to make changes easier */
void delete_populate_settings (Playlist *pl, GList *selected_songs,
			       gchar **label, gchar **title,
			       gboolean *confirm_again,
			       ConfHandlerCA *confirm_again_handler,
			       GString **str)
{
    Song *s;
    GList *l;
    guint n;

    /* write title and label */
    n = g_list_length (selected_songs);
    if(pl->type == PL_TYPE_MPL)
    {
	*label = g_strdup (ngettext ("Are you sure you want to delete the following song\ncompletely from your ipod?", "Are you sure you want to delete the following songs\ncompletely from your ipod?", n));
	*title = ngettext (_("Delete Song Completely?"), _("Delete Songs Completey?"), n);
	*confirm_again = prefs_get_song_ipod_file_deletion ();
	*confirm_again_handler = prefs_set_song_ipod_file_deletion;
    }
    else /* normal playlist */
    {
	*label = g_strdup_printf(ngettext ("Are you sure you want to delete the following song\nfrom the playlist \"%s\"?", "Are you sure you want to delete the following songs\nfrom the playlist \"%s\"?", n), pl->name);
	*title = ngettext (_("Delete Song From Playlist?"), _("Delete Songs From Playlist?"), n);
	*confirm_again = prefs_get_song_playlist_deletion ();
	*confirm_again_handler = prefs_set_song_playlist_deletion;
    }

    /* Write names of songs */
    *str = g_string_sized_new (2000);
    for(l = selected_songs; l; l = l->next)
    {
	s = (Song*)l->data;
	g_string_append_printf (*str, "%s-%s\n", s->artist, s->title);
    }
}



/* ok handler for delete song */
/* @user_data1 the selected playlist, @user_data2 are the selected songs */
static void delete_song_ok (gpointer user_data1, gpointer user_data2)
{
    Playlist *pl = user_data1;
    GList *selected_songs = user_data2;
    gint n;
    gchar *buf;
    GList *l;

    /* sanity checks */
    if (!pl)
    {
	if (selected_songs)
	    g_list_free (selected_songs);
	return;
    }
    if (!selected_songs)
	return;

    n = g_list_length (selected_songs); /* nr of songs to be deleted */
    if (pl->type == PL_TYPE_MPL)
    {
	buf = g_strdup_printf (ngettext (_("Deleted one song completely from iPod"), _("Deleted %d songs completely from iPod"), n), n);
    }
    else /* normal playlist */
    {
	buf = g_strdup_printf (ngettext (_("Deleted song from playlist '%s'"), _("Deleted songs from playlist '%s'"), n), pl->name);
    }

    for (l = selected_songs; l; l = l->next)
    {
	remove_song_from_playlist(pl, (Song *)l->data);
    }

    gtkpod_statusbar_message (buf);
    g_list_free (selected_songs);
    g_free (buf);
}

/* cancel handler for delete song */
/* @user_data1 the selected playlist, @user_data2 are the selected songs */
static void delete_song_cancel (gpointer user_data1, gpointer user_data2)
{
    GList *selected_songs = user_data2;

    g_list_free (selected_songs);
}

void delete_song_head (void)
{
    GList *selected_songs;
    Playlist *pl;
    GString *str;
    gchar *label, *title;
    gboolean confirm_again;
    ConfHandlerCA confirm_again_handler;

    pl = get_currently_selected_playlist();
    if (pl == NULL)
    { /* no playlist??? Cannot happen, but... */
	gtkpod_statusbar_message (_("No playlist selected."));
	return;
    }
    selected_songs = get_currently_selected_songs();
    if (selected_songs == NULL)
    {  /* no songs selected */
	gtkpod_statusbar_message (_("No songs selected."));
	return;
    }
    delete_populate_settings (pl, selected_songs,
			      &label, &title,
			      &confirm_again, &confirm_again_handler,
			      &str);
    /* open window */
    gtkpod_confirmation
	(-1,                   /* gint id, */
	 TRUE,                 /* gboolean modal, */
	 title,                /* title */
	 label,                /* label */
	 str->str,             /* scrolled text */
	 confirm_again,        /* gboolean confirm_again, */
	 confirm_again_handler,/* ConfHandlerCA confirm_again_handler,*/
	 delete_song_ok,       /* ConfHandler ok_handler,*/
	 CONF_NO_BUTTON,       /* don't show "Apply" button */
	 delete_song_cancel,   /* cancel_handler,*/
	 pl,                   /* gpointer user_data1,*/
	 selected_songs);      /* gpointer user_data2,*/

    g_free (label);
    g_string_free (str, TRUE);
}

void
gtkpod_main_window_set_active(gboolean active)
{
    if(gtkpod_window)
    {
	gtk_widget_set_sensitive(gtkpod_window, active);
    }
}



/*------------------------------------------------------------------*\
 *                                                                  *
 *             Delete songs in st entry                             *
 *                                                                  *
\*------------------------------------------------------------------*/

/* ok handler for delete tab entry */
/* @user_data1 the selected playlist, @user_data2 are the selected songs */
static void delete_entry_ok (gpointer user_data1, gpointer user_data2)
{
    Playlist *pl = user_data1;
    GList *selected_songs = user_data2;
    TabEntry *entry;
    guint32 inst;

    /* We put the instance at the first position in
     * selected_songs. Retrieve it and delete it from the list */
    inst = (guint32)g_list_nth_data (selected_songs, 0);
    selected_songs = g_list_remove (selected_songs, (gpointer)inst);
    /* Same with the selected entry */
    entry = g_list_nth_data (selected_songs, 0);
    selected_songs = g_list_remove (selected_songs, entry);

    /* Delete the songs */
    delete_song_ok (pl, selected_songs);
    /* Delete the entry */
    st_remove_entry (entry, inst);
}


/* cancel handler for delete tab entry */
/* @user_data1 the selected playlist, @user_data2 are the selected songs */
static void delete_entry_cancel (gpointer user_data1, gpointer user_data2)
{
    GList *selected_songs = user_data2;

    g_list_free (selected_songs);
}


void delete_entry_head (gint inst)
{
    GList *selected_songs;
    Playlist *pl;
    GString *str;
    gchar *label, *title;
    gboolean confirm_again;
    ConfHandlerCA confirm_again_handler;
    TabEntry *entry;

    if ((inst < 0) || (inst > prefs_get_sort_tab_num ()))   return;
    pl = get_currently_selected_playlist();
    if (pl == NULL)
    { /* no playlist??? Cannot happen, but... */
	gtkpod_statusbar_message (_("No playlist selected."));
	return;
    }
    if (inst == -1)
    { /* this should not happen... */
	gtkpod_warning ("delete_entry_head(): Programming error: inst == -1\n");
	return;
    }
    entry = st_get_selected_entry (inst);
    if (entry == NULL)
    {  /* no entry selected */
	gtkpod_statusbar_message (_("No entry selected."));
	return;
    }
    if (entry->members == NULL)
    {  /* no songs in entry -> just remove entry */
	if (!entry->master)  st_remove_entry (entry, inst);
	else   gtkpod_statusbar_message (_("Cannot remove entry 'All'"));
	return;
    }
    selected_songs = g_list_copy (entry->members);

    delete_populate_settings (pl, selected_songs,
			      &label, &title,
			      &confirm_again, &confirm_again_handler,
			      &str);
    /* add "entry" to beginning of the "selected_songs" list -- we can
     * only pass two args, so this is the easiest way */
    selected_songs = g_list_prepend (selected_songs, entry);
    /* add "inst" the same way */
    selected_songs = g_list_prepend (selected_songs, (gpointer)inst);

    /* open window */
    gtkpod_confirmation
	(-1,                   /* gint id, */
	 TRUE,                 /* gboolean modal, */
	 title,                /* title */
	 label,                /* label */
	 str->str,             /* scrolled text */
	 confirm_again,        /* gboolean confirm_again, */
	 confirm_again_handler,/* ConfHandlerCA confirm_again_handler,*/
	 delete_entry_ok,      /* ConfHandler ok_handler,*/
	 CONF_NO_BUTTON,       /* don't show "Apply" button */
	 delete_entry_cancel,  /* cancel_handler,*/
	 pl,                   /* gpointer user_data1,*/
	 selected_songs);      /* gpointer user_data2,*/

    g_free (label);
    g_string_free (str, TRUE);
}
