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
|
|  $Id$
*/

#ifndef __FILE_H__
#define __FILE_H__

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>
#include <stdio.h>
#include "track.h"
#include "playlist.h"

typedef void (*AddTrackFunc)(Playlist *plitem, Track *track, gpointer data);

gboolean add_track_by_filename (gchar *name, Playlist *plitem, gboolean descend,
			       AddTrackFunc addtrackfunc, gpointer data);
gboolean add_directory_by_name (gchar *name, Playlist *plitem,
				gboolean descend,
				AddTrackFunc addtrackfunc, gpointer data);
gboolean add_playlist_by_filename (gchar *plfile, Playlist *plitem,
				   AddTrackFunc addtrackfunc, gpointer data);
gboolean write_tags_to_file(Track *s);
void update_track_from_file (Track *track);
void do_selected_tracks (void (*do_func)(GList *trackids));
void do_selected_entry (void (*do_func)(GList *trackids), gint inst);
void do_selected_playlist (void (*do_func)(GList *trackids));
void update_trackids (GList *selected_trackids);
void sync_trackids (GList *selected_trackids);
void display_non_updated (Track *track, gchar *txt);
void display_updated (Track *track, gchar *txt);
void handle_import (void);
void handle_export (void);
gboolean files_are_saved (void);
void data_changed (void);
gchar *get_track_name_on_disk_verified (Track *track);
gchar* get_track_name_on_disk(Track *s);
gchar* get_track_name_on_ipod(Track *s);
gchar* get_preferred_track_name_format(Track *s);
void mark_track_for_deletion (Track *track);
void unmark_track_for_deletion (Track *track);
void fill_in_extended_info (Track *track);
double get_filesize_of_deleted_tracks (guint32 *num);
Track *get_track_info_from_file (gchar *name, Track *or_track);
void update_charset_info (Track *track);

#endif
