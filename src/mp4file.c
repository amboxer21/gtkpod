/* Time-stamp: <2005-05-12 23:23:37 jcs>
|
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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "charset.h"
#include "itdb.h"
#include "misc.h"
#include "mp4file.h"
#include "prefs.h"

/* ------------------------------------------------------------

   Info on how to implement new file formats.

   You need to supply a function

   Track *xxx_get_file_info (gchar *filename)

   that returns a new Track structure with as many of the following
   fields filled in (in UTF8):

   gchar   *album;            /+ album (utf8)          +/
   gchar   *artist;           /+ artist (utf8)         +/
   gchar   *title;            /+ title (utf8)          +/
   gchar   *genre;            /+ genre (utf8)          +/
   gchar   *comment;          /+ comment (utf8)        +/
   gchar   *composer;         /+ Composer (utf8)       +/
   gchar   *fdesc;            /+ Format description (utf8) +/
   gchar   *charset;          /+ charset used for tags +/
   gint32  cd_nr;             /+ CD number             +/
   gint32  cds;               /+ number of CDs         +/
   gint32  track_nr;          /+ track number          +/
   gint32  tracks;            /+ number of tracks      +/
   gint32  year;              /+ year                  +/
   gint32  tracklen;          /+ Length of track in ms +/
   gint32  bitrate;           /+ bitrate in kbps       +/
   guint16 samplerate;        /+ e.g.: CD is 44100     +/
   guint32 peak_signal;	      /+ LAME Peak Signal * 0x800000         +/
   gint32  radio_gain;	      /+ RadioGain in dB*10
				(as defined by www.replaygain.org)   +/
   gint32  audiophile_gain;   /+ AudiophileGain in dB*10 
				(as defined by www.replaygain.org)   +/
   gboolean peak_signal_set;  /+ has the peak signal been set?       +/
   gboolean radio_gain_set;   /+ has the radio gain been set?        +/
   gboolean audiophile_gain_set;/+ has the audiophile gain been set? +/


   If prefs_get_tag_readtags() returns FALSE you only should fill in
   tracklen, bitrate and fdesc

   Please note that the iPod will only play as much of the track as
   specified in "tracklen".

   You don't have to fill in the value for charset if you use the
   default charset (i.e. you use charset_to_utf8() to convert to
   UTF8). Otherwise please specify the charset used.

   When an error occurs, the function returns NULL and logs an error
   message using gtkpod_warning().

   You need to add your handler to get_track_info_from_file() in
   file.c


   You also have to write a function to write TAGs back to the
   file. That function should be named

   gboolean xxx_write_file_info (gchar *filename, Track *track, T_item tag_id)

   and return TRUE on success or FALSE on error. In that case it
   should log an error message using gtkpod_warning().

   @tag_id determines which TAGs of @track out of the list above are
   to be updated. If @tag_id==T_ALL, upudate all (supported) TAGs.

   You need to add your handler to file_write_info() in file.c


   Finally, you may want to provide a function that can
   read/calculate the replay gain

   gboolean xxx_get_gain (gchar *filename, Track *track)

   and return TRUE when the replay gain could be determined (as an
   example see mp3_get_gain() in mp3file.c.

   You need to add your handler to get_gain() in file.c.

   ------------------------------------------------------------ */



#ifdef HAVE_LIBMP4V2
/* Use mp4v2 from the mpeg4ip project to handle mp4 (m4a, m4p, m4b) files
   (http://mpeg4ip.sourceforge.net/) */
/* Copyright note: code for mp4_get_file_info() is based on
 * mp4info.cpp of the mpeg4ip project */

/* define metadata bug is present (see note at mp4_write_file_info()) */
#define MP4V2_HAS_METADATA_BUG TRUE

#include <sys/types.h>
#include <sys/param.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include "mp4.h"
#undef PACKAGE
#undef VERSION
/* need to include config.h again because mp4.h seems to #undef
   PACKAGE */
#include <config.h>

Track *mp4_get_file_info (gchar *mp4FileName)
{
    Track *track = NULL;
    MP4FileHandle mp4File = MP4Read(mp4FileName, 0);

    if (mp4File != MP4_INVALID_FILE_HANDLE)
    {
	MP4TrackId trackId;
	const char *trackType;

	trackId = MP4FindTrackId(mp4File, 0, NULL, 0);
	trackType = MP4GetTrackType(mp4File, trackId);
	if (trackType && (strcmp(trackType, MP4_AUDIO_TRACK_TYPE) == 0))
	{
	    gchar *value;
	    guint16 numvalue, numvalue2;
	    MP4Duration trackDuration = MP4GetTrackDuration(mp4File, trackId);
	    double msDuration = 
		(double)MP4ConvertFromTrackDuration(mp4File, trackId,
						    trackDuration,
						    MP4_MSECS_TIME_SCALE);
	    guint32 avgBitRate = MP4GetTrackBitRate(mp4File, trackId);
	    guint32 samplerate = MP4GetTrackTimeScale(mp4File, trackId);

	    track = gp_track_new ();

	    track->tracklen = msDuration;
	    track->bitrate = avgBitRate/1000;
	    track->samplerate = samplerate;
	    value = strrchr (mp4FileName, '.');
	    if (value)
	    {
		if (g_strcasecmp (value, ".m4a") == 0)
		    track->fdesc = g_strdup ("AAC audio file");
		if (g_strcasecmp (value, ".m4p") == 0)
		    track->fdesc = g_strdup ("Protected AAC audio file");
		if (g_strcasecmp (value, ".m4b") == 0)
		    track->fdesc = g_strdup ("AAC audio book file");
	    }
	    if (prefs_get_readtags ())
	    {
		if (MP4GetMetadataName(mp4File, &value) && value != NULL)
		{
		    track->title = charset_to_utf8 (value);
		    g_free(value);
		}
		if (MP4GetMetadataArtist(mp4File, &value) && value != NULL)
		{
		    track->artist = charset_to_utf8 (value);
		    g_free(value);
		}
		if (MP4GetMetadataWriter(mp4File, &value) && value != NULL)
		{
		    track->composer = charset_to_utf8 (value);
		    g_free(value);
		}
		if (MP4GetMetadataComment(mp4File, &value) && value != NULL)
		{
		    track->comment = charset_to_utf8 (value);
		    g_free(value);
		}
		if (MP4GetMetadataYear(mp4File, &value) && value != NULL)
		{
		    track->year = atoi (value);
		    g_free(value);
		}
		if (MP4GetMetadataAlbum(mp4File, &value) && value != NULL)
		{
		    track->album = charset_to_utf8 (value);
		    g_free(value);
		}
		if (MP4GetMetadataTrack(mp4File, &numvalue, &numvalue2))
		{
		    track->track_nr = numvalue;
		    track->tracks = numvalue2;
		}
		if (MP4GetMetadataDisk(mp4File, &numvalue, &numvalue2))
		{
		    track->cd_nr = numvalue;
		    track->cds = numvalue2;
		}
		if (MP4GetMetadataGenre(mp4File, &value) && value != NULL)
		{
		    track->genre = charset_to_utf8 (value);
		    g_free(value);
		}
	    }
	}
	else
	{
	    gchar *filename = charset_to_utf8 (mp4FileName);
	    gtkpod_warning (_("'%s' does not appear to be a mp4 audio file.\n"),
			    filename);
	    g_free (filename);
	}
	MP4Close(mp4File);
    }
    else
    {
	gchar *filename = charset_to_utf8 (mp4FileName);
	gtkpod_warning (
	    _("Could not open '%s' for reading, or file is not an mp4 file.\n"),
	    filename);
	g_free (filename);
    }

    return track;
}


gboolean mp4_write_file_info (gchar *mp4FileName, Track *track)
{
    gboolean result = TRUE;
    MP4FileHandle mp4File = MP4Modify(mp4FileName, 0, FALSE);

    if (mp4File != MP4_INVALID_FILE_HANDLE)
    {
	MP4TrackId trackId;
	const char *trackType;

	trackId = MP4FindTrackId(mp4File, 0, NULL, 0);
	trackType = MP4GetTrackType(mp4File, trackId);
	if (trackType && (strcmp(trackType, MP4_AUDIO_TRACK_TYPE) == 0))
	{
	    gchar *value;

#if MP4V2_HAS_METADATA_BUG
	    /* It could have been so easy. But: due to a bug in mp4v2
	     * you have to delete all meta data before modifying
	     * it. Therefore we have to read it first to avoid data
	     * loss. (Bug present in mpeg4ip-1.0RC1.) */
/*	    gchar *m_name = NULL, *m_artist = NULL;
	    gchar *m_writer = NULL, *m_comment = NULL;
	    gchar *m_year = NULL;
	    gchar *m_album = NULL, *m_genre = NULL;*/
	    gchar *m_tool = NULL;
/*	    guint16 m_track, m_tracks, m_disk, m_disks; */
	    guint16 m_tempo;
	    guint8 *m_covert = NULL, m_cpl;
	    guint32 m_size;
/*	    gboolean has_track = MP4GetMetadataTrack (mp4File,
						      &m_track, &m_tracks);
	    gboolean has_disk = MP4GetMetadataDisk (mp4File,
	    &m_disk, &m_disks);*/
	    gboolean has_tempo = MP4GetMetadataTempo (mp4File,
						      &m_tempo);
	    gboolean has_compilation = MP4GetMetadataCompilation (mp4File,
								  &m_cpl);
/*	    MP4GetMetadataName (mp4File, &m_name);
	    MP4GetMetadataArtist (mp4File, &m_artist);
	    MP4GetMetadataWriter (mp4File, &m_writer);
	    MP4GetMetadataComment (mp4File, &m_comment);
	    MP4GetMetadataYear (mp4File, &m_year);
	    MP4GetMetadataAlbum (mp4File, &m_album);
	    MP4GetMetadataGenre (mp4File, &m_genre);*/
	    MP4GetMetadataTool (mp4File, &m_tool);
	    MP4GetMetadataCoverArt (mp4File, &m_covert, &m_size);
	    MP4MetadataDelete (mp4File);
#endif
	    value = charset_from_utf8 (track->title);
	    MP4SetMetadataName (mp4File, value);
	    g_free (value);

	    value = charset_from_utf8 (track->artist);
	    MP4SetMetadataArtist (mp4File, value);
	    g_free (value);

	    value = charset_from_utf8 (track->composer);
	    MP4SetMetadataWriter (mp4File, value);
	    g_free (value);

	    value = charset_from_utf8 (track->comment);
	    MP4SetMetadataComment (mp4File, value);
	    g_free (value);

	    value = g_strdup_printf ("%d", track->year);
	    MP4SetMetadataYear (mp4File, value);
	    g_free (value);

	    value = charset_from_utf8 (track->album);
	    MP4SetMetadataAlbum (mp4File, value);
	    g_free (value);

	    MP4SetMetadataTrack (mp4File, track->track_nr, track->tracks);

	    MP4SetMetadataDisk (mp4File, track->cd_nr, track->cds);

	    value = charset_from_utf8 (track->genre);
	    MP4SetMetadataGenre (mp4File, value);
	    g_free (value);

#if MP4V2_HAS_METADATA_BUG
	    if (has_tempo) MP4SetMetadataTempo (mp4File, m_tempo);
	    if (has_compilation) MP4SetMetadataCompilation (mp4File, m_cpl);
	    if (m_tool)     MP4SetMetadataTool (mp4File, m_tool);
	    if (m_covert)   MP4SetMetadataCoverArt (mp4File, m_covert, m_size);
/*	    g_free (m_name);
	    g_free (m_artist);
	    g_free (m_writer);
	    g_free (m_comment);
	    g_free (m_year);
	    g_free (m_album);
	    g_free (m_genre);*/
	    g_free (m_tool);
	    g_free (m_covert);
#endif
	}
	else
	{
	    gchar *filename = charset_to_utf8 (mp4FileName);
	    gtkpod_warning (_("'%s' does not appear to be a mp4 audio file.\n"),
			    filename);
	    g_free (filename);
	    result = FALSE;
	}
	MP4Close (mp4File);
    }
    else
    {
	gchar *filename = charset_to_utf8 (mp4FileName);
	gtkpod_warning (
	    _("Could not open '%s' for writing, or file is not an mp4 file.\n"),
	    filename);
	g_free (filename);
	result = FALSE;
    }

    return result;
}

#else
/* We don't support mp4 without the mp4v2 library */
Track *mp4_get_file_info (gchar *name)
{
    gtkpod_warning (_("Import of '%s' failed: m4a/m4p/m4b not supported without the mp4v2 library. You must compile the gtkpod source together with the mp4v2 library.\n"), name);
    return NULL;
}

gboolean mp4_write_file_info (gchar *filename, Track *track)
{
    gtkpod_warning (_("m4a/m4p/m4b metadata update for '%s' failed: m4a/m4p/m4b not supported without the mp4v2 library. You must compile the gtkpod source together with the mp4v2 library.\n"), filename);
    return FALSE;
}
#endif
