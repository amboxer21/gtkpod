/* Time-stamp: <2005-01-06 00:23:52 jcs>
|
|  Copyright (C) 2002-2003 Jorg Schuler <jcsjcs at users.sourceforge.net>
|  Part of the gtkpod project.
|
|  URL: http://gtkpod.sourceforge.net/
|
|  The code contained in this file is free software; you can redistribute
|  it and/or modify it under the terms of the GNU Lesser General Public
|  License as published by the Free Software Foundation; either version
|  2.1 of the License, or (at your option) any later version.
|
|  This file is distributed in the hope that it will be useful,
|  but WITHOUT ANY WARRANTY; without even the implied warranty of
|  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
|  Lesser General Public License for more details.
|
|  You should have received a copy of the GNU Lesser General Public
|  License along with this code; if not, write to the Free Software
|  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
|
|  iTunes and iPod are trademarks of Apple
|
|  This product is not supported/written/published by Apple!
|
|  $Id$
*/

#include "itdb_private.h"
#include <string.h>

/* Generate a new Itdb_Track structure */
Itdb_Track *itdb_track_new (void)
{
    Itdb_Track *track = g_new0 (Itdb_Track, 1);

    track->unk020 = 1;
    return track;
}


/* Add @track to @itdb->tracks at position @pos (or at the end if pos
   is -1). Application is responsible to also add it to the master
   playlist. */
void itdb_track_add (Itdb_iTunesDB *itdb, Itdb_Track *track, gint32 pos)
{
    g_return_if_fail (itdb);
    g_return_if_fail (track);

    track->itdb = itdb;

    if (pos == -1)  itdb->tracks = g_list_append (itdb->tracks, track);
    else  itdb->tracks = g_list_insert (itdb->tracks, track, pos);
}

/* Free the memory taken by @track */
void itdb_track_free (Itdb_Track *track)
{
    g_return_if_fail (track);

    g_free (track->album);
    g_free (track->artist);
    g_free (track->title);
    g_free (track->genre);
    g_free (track->comment);
    g_free (track->composer);
    g_free (track->fdesc);
    g_free (track->grouping);
    g_free (track->pc_path_locale);
    g_free (track->ipod_path);
    if (track->userdata && track->userdata_destroy)
	(*track->userdata_destroy) (track->userdata);
    g_free (track);
}

/* Remove track @track and free memory */
void itdb_track_remove (Itdb_Track *track)
{
    Itdb_iTunesDB *itdb;

    g_return_if_fail (track);
    itdb = track->itdb;
    g_return_if_fail (itdb);

    itdb->tracks = g_list_remove (itdb->tracks, track);
    itdb_track_free (track);
}

/* Remove track @track but do not free memory */
void itdb_track_unlink (Itdb_Track *track)
{
    Itdb_iTunesDB *itdb;

    g_return_if_fail (track);
    itdb = track->itdb;
    g_return_if_fail (itdb);

    itdb->tracks = g_list_remove (itdb->tracks, track);
}

/* Returns the track with the ID @id or NULL if the ID cannot be
 * found. */
Itdb_Track *itdb_track_by_id (Itdb_iTunesDB *itdb, guint32 id)
{
    GList *gl;

    g_return_val_if_fail (itdb, NULL);

    for (gl=itdb->tracks; gl; gl=gl->next)
    {
	Itdb_Track *track = gl->data;
	if (track->id == id)  return track;
    }
    return NULL;
}

/* Returns the track with the filename @name or NULL, if none can be
 * found. This function also works if @name is on the iPod. */
/* If @filename == NULL, NULL is returned. */
Itdb_Track *itdb_track_by_filename (Itdb_iTunesDB *itdb, gchar *filename)
{
    g_return_val_if_fail (itdb, NULL);

    if (!filename) return NULL;

    if (itdb->mountpoint && 
	(strncmp (filename, itdb->mountpoint,
		  strlen (itdb->mountpoint)) == 0))
    {   /* handle track on iPod */
	GList *gl;
	for (gl=itdb->tracks; gl; gl=gl->next)
	{
	    Itdb_Track *track = gl->data;
	    g_return_val_if_fail (track, NULL);
	    gchar *ipod_path = itdb_filename_on_ipod (itdb->mountpoint,
						      track);
	    if (ipod_path)
	    {
		if (strcmp (ipod_path, filename) == 0)
		{
		    g_free (ipod_path);
		    return track;
		}
		g_free (ipod_path);
	    }
	}
    }
    else
    {   /* handle track on local filesystem */
	GList *gl;
	for (gl=itdb->tracks; gl; gl=gl->next)
	{
	    Itdb_Track *track = gl->data;
	    g_return_val_if_fail (track, NULL);
	    if (track->pc_path_locale)
		if (strcmp (track->pc_path_locale, filename) == 0)
		    return track;
	}
    }
    return NULL;
}