/* Time-stamp: <2004-09-20 22:14:06 jcs>
|
|  Copyright (C) 2002-2003 Jorg Schuler <jcsjcs at users.sourceforge.net>
|  Part of the gtkpod project.
|
|  URL: http://gtkpod.sourceforge.net/
|
|  Most of the code in this file has been ported from the perl
|  script "mktunes.pl" (part of the gnupod-tools collection) written
|  by Adrian Ulrich <pab at blinkenlights.ch>.
|
|  gnupod-tools: http://www.blinkenlights.ch/cgi-bin/fm.pl?get=ipod
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




/* Some notes on how to use the functions in this file:


   *** Reading the iTunesDB ***

   gboolean itunesdb_parse (gchar *path); /+ path to mountpoint +/
   will read an iTunesDB and pass the data over to your program. Your
   programm is responsible to keep a representation of the data.

   The information given in the "Play Counts" file is also read if
   available and the playcounts, star rating and the time last played
   is updated.

   For each track itunesdb_parse() will pass a filled out Track structure
   to "it_add_track()", which has to be provided. The return value is
   TRUE on success and FALSE on error. At the time being, the return
   value is ignored, however.

   The minimal Track structure looks like this (feel free to have
   it_add_track() do with it as it pleases -- and yes, you are
   responsible to free the memory):

   typedef struct
   {
     gunichar2 *album_utf16;    /+ album (utf16)         +/
     gunichar2 *artist_utf16;   /+ artist (utf16)        +/
     gunichar2 *title_utf16;    /+ title (utf16)         +/
     gunichar2 *genre_utf16;    /+ genre (utf16)         +/
     gunichar2 *comment_utf16;  /+ comment (utf16)       +/
     gunichar2 *composer_utf16; /+ Composer (utf16)      +/
     gunichar2 *fdesc_utf16;    /+ Filetype descr (utf16)+/
     gunichar2 *ipod_path_utf16;/+ name of file on iPod: uses ":" instead of "/" +/
     guint32 ipod_id;           /+ unique ID of track    +/
     gint32  size;              /+ size of file in bytes +/
     gint32  tracklen;          /+ Length of track in ms +/
     gint32  cd_nr;             /+ CD number             +/
     gint32  cds;               /+ number of CDs         +/
     gint32  track_nr;          /+ track number          +/
     gint32  tracks;            /+ number of tracks      +/
     gint32  year;              /+ year                  +/
     gint32  bitrate;           /+ bitrate               +/
     gint32  volume;            /+ volume adjustment     +/
     guint32 soundcheck;        /+ volume adjustment "soundcheck"   +/
     guint32 time_created;      /+ time when added (Mac type)       +/
     guint32 time_played;       /+ time of last play (Mac type)     +/
     guint32 time_modified;     /+ time of last modific. (Mac type) +/
     guint32 rating;            /+ star rating (stars * 20)         +/
     guint32 playcount;         /+ number of times track was played +/
     guint32 recent_playcount;  /+ times track was played since last sync+/
     gboolean transferred;      /+ has file been transferred to iPod?    +/
   } Track;

   "transferred" will be set to TRUE because all tracks read from a
   iTunesDB are obviously (or hopefully) already transferred to the
   iPod.

   "recent_playcount" is for information only and will not be stored
   to the iPod.

   By #defining ITUNESDB_PROVIDE_UTF8, itunesdb_parse() will also
   provide utf8 versions of the above utf16 strings. You must then add
   members "gchar *album"... to the Track structure.

   For each new playlist in the iTunesDB, it_add_playlist() is
   called with a pointer to the following Playlist struct:

   typedef struct
   {
     gunichar2 *name_utf16;
     guint32 type;         /+ 1: master play list (PL_TYPE_MPL) +/
   } Playlist;

   Again, by #defining ITUNESDB_PROVIDE_UTF8, a member "gchar *name"
   will be initialized with a utf8 version of the playlist name.

   it_add_playlist() must return a pointer under which it wants the
   playlist to be referenced when it_add_track_to_playlist() is called.

   For each track in the playlist, it_add_trackid_to_playlist() is called
   with the above mentioned pointer to the playlist and the trackid to
   be added.

   gboolean it_add_track (Track *track);
   Playlist *it_add_playlist (Playlist *plitem);
   void it_add_trackid_to_playlist (Playlist *plitem, guint32 id);


   *** Writing the iTunesDB ***

   gboolean itunesdb_write (gchar *path), /+ path to mountpoint +/
   will write an updated version of the iTunesDB.

   The "Play Counts" file is renamed to "Play Counts.bak" if it exists
   to avoid it being read multiple times.

   It uses the following functions to retrieve the data necessary data
   from memory:

   guint it_get_nr_of_tracks (void);
   Track *it_get_track_by_nr (guint32 n);
   guint32 it_get_nr_of_playlists (void);
   Playlist *it_get_playlist_by_nr (guint32 n);
   guint32 it_get_nr_of_tracks_in_playlist (Playlist *plitem);
   Track *it_get_track_in_playlist_by_nr (Playlist *plitem, guint32 n);

   The master playlist is expected to be "it_get_playlist_by_nr(0)". Only
   the utf16 strings in the Playlist and Track struct are being used.

   Please note that non-transferred tracks are not automatically
   transferred to the iPod. A function

   gboolean itunesdb_copy_track_to_ipod (gchar *path, Track *track, gchar *pcfile)

   is provided to help you do that, however.

   The following functions most likely will also come in handy:

   Track *itunesdb_new_track (void);
   Use itunesdb_new_track() to get an "initialized" track structure
   (the "unknowns" are initialized with reasonable values).

   gboolean itunesdb_cp (gchar *from_file, gchar *to_file);
   guint32 itunesdb_time_get_mac_time (void);
   time_t itunesdb_time_mac_to_host (guint32 mactime);
   guint32 itunesdb_time_host_to_mac (time_t time);
   void itunesdb_convert_filename_fs2ipod(gchar *ipod_file);
   void itunesdb_convert_filename_ipod2fs(gchar *ipod_file);

   void itunesdb_rename_files (const gchar *dirname);

   (Renames/removes some files on the iPod (Playcounts, OTG
   semaphore). Needs to be called if you write the iTunesDB not
   directly to the iPod but to some other location and then manually
   copy the file from there to the iPod. That's much faster in the
   case of using an iPod mounted in sync'ed mode.)

   Define "itunesdb_warning()" as you need (or simply use g_print and
   change the default g_print handler with g_set_print_handler() as is
   done in gtkpod).

   Jorg Schuler, 19.12.2002 */


/* call itunesdb_parse () to read the iTunesDB  */
/* call itunesdb_write () to write the iTunesDB */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "itunesdb.h"
#include "support.h"
#include "file.h"

#ifndef P_tmpdir
#define P_tmpdir	"/tmp"
#endif

#ifdef IS_GTKPOD
/* we're being linked with gtkpod */
#define itunesdb_warning(...) g_print(__VA_ARGS__)
#else
/* The following prints the error messages to the shell, converting
 * UTF8 to the current locale on the fly: */
#define itunesdb_warning(...) do { gchar *utf8=g_strdup_printf (__VA_ARGS__); gchar *loc=g_locale_from_utf8 (utf8, -1, NULL, NULL, NULL); fprintf (stderr, "%s", loc); g_free (loc); g_free (utf8);} while (FALSE)
#endif

/* We instruct itunesdb_parse to provide utf8 versions of the strings */
#define ITUNESDB_PROVIDE_UTF8

#define ITUNESDB_DEBUG 0
#define ITUNESDB_MHIT_DEBUG 0

#define ITUNESDB_COPYBLK 262144      /* blocksize for cp () */

/* list with the contents of the Play Count file for use when
 * importing the iTunesDB */
static GList *playcounts = NULL;

/* structure to hold the contents of one entry of the Play Count file */
struct playcount {
    guint32 playcount;
    guint32 time_played;
    guint32 rating;
};

enum {
  MHOD_ID_TITLE = 1,
  MHOD_ID_PATH = 2,
  MHOD_ID_ALBUM = 3,
  MHOD_ID_ARTIST = 4,
  MHOD_ID_GENRE = 5,
  MHOD_ID_FDESC = 6,
  MHOD_ID_COMMENT = 8,
  MHOD_ID_COMPOSER = 12,
  MHOD_ID_PLAYLIST = 100
};

static struct playcount *get_next_playcount (void);

static guint32 utf16_strlen(gunichar2 *utf16_string);

/* Compare the two data. TRUE if identical */
static gboolean cmp_n_bytes (gchar *data1, gchar *data2, gint n)
{
  gint i;

  for(i=0; i<n; ++i)
    {
      if (data1[i] != data2[i]) return FALSE;
    }
  return TRUE;
}


/* Seeks to position "seek", then reads "n" bytes. Returns -1 on error
   during seek, or the number of bytes actually read */
static gint seek_get_n_bytes (FILE *file, gchar *data, glong seek, gint n)
{
  gint i;
  gint read;

  if (fseek (file, seek, SEEK_SET) != 0) return -1;

  for(i=0; i<n; ++i)
    {
      read = fgetc (file);
      if (read == EOF) return i;
      *data++ = (gchar)read;
    }
  return i;
}


/* Get the 4-byte-number stored at position "seek" in "file"
   (or -1 when an error occured) */
static guint32 get4int(FILE *file, glong seek)
{
  guchar data[4];
  guint32 n;

  if (seek_get_n_bytes (file, data, seek, 4) != 4) return -1;
  n =  ((guint32)data[3]) << 24;
  n += ((guint32)data[2]) << 16;
  n += ((guint32)data[1]) << 8;
  n += ((guint32)data[0]);
  return n;
}




/* Fix UTF16 String for BIGENDIAN machines (like PPC) */
static gunichar2 *fixup_utf16(gunichar2 *utf16_string) {
#if (G_BYTE_ORDER == G_BIG_ENDIAN)
gint32 i;
 if (utf16_string)
 {
     for(i=0; i<utf16_strlen(utf16_string); i++)
     {
	 utf16_string[i] = ((utf16_string[i]<<8) & 0xff00) |
	     ((utf16_string[i]>>8) & 0xff);
     }
 }
#endif
return utf16_string;
}


/* return the length of the header *ml, the genre number *mty,
   and a string with the entry (in UTF16?). After use you must
   free the string with g_free (). Returns NULL in case of error. */
static gunichar2 *get_mhod (FILE *file, glong seek, gint32 *ml, gint32 *mty)
{
  gchar data[4];
  gunichar2 *entry_utf16 = NULL;
  gint32 xl;

#if ITUNESDB_DEBUG
  fprintf(stderr, "get_mhod seek: %x\n", (int)seek);
#endif

  if (seek_get_n_bytes (file, data, seek, 4) != 4)
    {
      *ml = -1;
      return NULL;
    }
  if (cmp_n_bytes (data, "mhod", 4) == FALSE )
    {
      *ml = -1;
      return NULL;
    }
  *ml = get4int (file, seek+8);       /* length         */
  *mty = get4int (file, seek+12);     /* mhod_id number */
  xl = get4int (file, seek+28);       /* entry length   */

#if ITUNESDB_DEBUG
  fprintf(stderr, "ml: %x mty: %x, xl: %x\n", *ml, *mty, xl);
#endif

  switch (*mty)
  {
  case MHOD_ID_PLAYLIST: /* do something with the "weird mhod" */
      break;
  default:
      entry_utf16 = g_malloc (xl+2);
      if (seek_get_n_bytes (file, (gchar *)entry_utf16, seek+40, xl) != xl) {
	  g_free (entry_utf16);
	  entry_utf16 = NULL;
	  *ml = -1;
      }
      else
      {
	  entry_utf16[xl/2] = 0; /* add trailing 0 */
      }
      break;
  }
  return fixup_utf16(entry_utf16);
}



/* get a PL, return pos where next PL should be, name and content */
static glong get_pl(FILE *file, glong seek)
{
  gunichar2 *plname_utf16 = NULL, *plname_utf16_maybe;
#ifdef ITUNESDB_PROVIDE_UTF8
  gchar *plname_utf8;
#endif
  guint32 type, pltype, tracknum, n;
  guint32 nextseek;
  gint32 zip;
  Playlist *plitem;
  guint32 ref;

  gchar data[4];


#if ITUNESDB_DEBUG
  fprintf(stderr, "mhyp seek: %x\n", (int)seek);
#endif

  if (seek_get_n_bytes (file, data, seek, 4) != 4) return -1;
  if (cmp_n_bytes (data, "mhyp", 4) == FALSE)      return -1; /* not pl */
  /* Some Playlists have added 256 to their type -- I don't know what
     it's for, so we just ignore it for now -> & 0xff */
  pltype = get4int (file, seek+20) & 0xff;  /* Type of playlist (1= MPL) */
  tracknum = get4int (file, seek+16); /* number of tracks in playlist */
  nextseek = seek + get4int (file, seek+8); /* possible begin of next PL */
  zip = get4int (file, seek+4); /* length of header */
  if (zip == 0) return -1;      /* error! */
  do
  {
      seek += zip;
      if (seek_get_n_bytes (file, data, seek, 4) != 4) return -1;
      plname_utf16_maybe = get_mhod(file, seek, &zip, &type); /* PL name */
      if (zip != -1) switch (type)
      {
      case MHOD_ID_PLAYLIST:
	  break; /* here we could do something about the "weird mhod" */
      case MHOD_ID_TITLE:
	  if (plname_utf16_maybe)
	  {
	      /* sometimes there seem to be two mhod TITLE headers */
	      if (plname_utf16) g_free (plname_utf16);
	      plname_utf16 = plname_utf16_maybe;
	  }
	  break;
      }
  } while (zip != -1); /* read all MHODs */
  if (!plname_utf16)
  {   /* we did not read a valid mhod TITLE header -> */
      /* we simply make up our own name */
	if (pltype == PL_TYPE_MPL)
	    plname_utf16 = g_utf8_to_utf16 (_("Master-PL"),
					    -1, NULL, NULL, NULL);
	else plname_utf16 = g_utf8_to_utf16 (_("Playlist"),
					     -1, NULL, NULL, NULL);
  }
#ifdef ITUNESDB_PROVIDE_UTF8
  plname_utf8 = g_utf16_to_utf8 (plname_utf16, -1, NULL, NULL, NULL);
#endif


#if ITUNESDB_DEBUG
  fprintf(stderr, "pln: %s(%d Tracks) \n", plname_utf8, (int)tracknum);
#endif

  plitem = g_malloc0 (sizeof (Playlist));

#ifdef ITUNESDB_PROVIDE_UTF8
  plitem->name = plname_utf8;
#endif
  plitem->name_utf16 = plname_utf16;
  plitem->type = pltype;

  /* create new playlist */
  plitem = it_add_playlist(plitem);

#if ITUNESDB_DEBUG
  fprintf(stderr, "added pl: %s", plname_utf8);
#endif

  n = 0;  /* number of tracks read */
  while (n < tracknum)
    {
      /* We read the mhip headers and skip everything else. If we
	 find a mhyp header before n==tracknum, something is wrong */
      if (seek_get_n_bytes (file, data, seek, 4) != 4) return -1;
      if (cmp_n_bytes (data, "mhyp", 4) == TRUE) return -1; /* Wrong!!! */
      if (cmp_n_bytes (data, "mhip", 4) == TRUE)
	{
	  ref = get4int(file, seek+24);
	  it_add_trackid_to_playlist(plitem, ref);
	  ++n;
	}
      seek += get4int (file, seek+8);
    }
  return nextseek;
}


static glong get_mhit(FILE *file, glong seek)
{
  Track *track;
  gchar data[4];
#ifdef ITUNESDB_PROVIDE_UTF8
  gchar *entry_utf8;
#endif
  gunichar2 *entry_utf16;
  gint type;
  gint zip = 0;
  struct playcount *playcount;
  guint32 temp;

#if ITUNESDB_DEBUG
  fprintf(stderr, "get_mhit seek: %x\n", (int)seek);
#endif

  if (seek_get_n_bytes (file, data, seek, 4) != 4) return -1;
  if (cmp_n_bytes (data, "mhit", 4) == FALSE ) return -1; /* we are lost! */

  track = itunesdb_new_track ();

  track->ipod_id = get4int(file, seek+16);       /* iPod ID          */
  track->unk020 = get4int (file, seek+20);
  track->unk024 = get4int (file, seek+24);
  track->rating = get4int (file, seek+28)>>24;   /* rating          */
  temp = get4int (file, seek+32);
  track->compilation = (temp & 0x00ff0000) >> 16;
  track->type = temp & 0x0000ffff;
  track->time_created = get4int(file, seek+32);  /* creation time    */
  track->size = get4int(file, seek+36);          /* file size        */
  track->tracklen = get4int(file, seek+40);      /* time             */
  track->track_nr = get4int(file, seek+44);      /* track number     */
  track->tracks = get4int(file, seek+48);        /* nr of tracks     */
  track->year = get4int(file, seek+52);          /* year             */
  track->bitrate = get4int(file, seek+56);       /* bitrate          */
  track->samplerate = get4int(file,seek+60)>>16; /* sample rate      */
  track->volume = get4int(file, seek+64);        /* volume adjust    */
  track->starttime = get4int (file, seek+68);
  track->stoptime = get4int (file, seek+72);
  track->soundcheck = get4int (file, seek+76);   /* soundcheck       */
  track->playcount = get4int (file, seek+80);    /* playcount        */
  track->unk084 = get4int (file, seek+84);
  track->time_played = get4int(file, seek+88);   /* last time played */
  track->cd_nr = get4int(file, seek+92);         /* CD nr            */
  track->cds = get4int(file, seek+96);           /* CD nr of..       */
  track->unk100 = get4int (file, seek+100);
  track->time_modified = get4int(file, seek+104);/* last mod. time   */
  track->unk108 = get4int (file, seek+108);
  track->unk112 = get4int (file, seek+112);
  track->unk116 = get4int (file, seek+116);
  temp = get4int (file, seek+120);
  track->BPM = temp >> 16;
  track->app_rating = (temp & 0xff00)>> 8;/* The rating set by * the
					     application, as opposed to
					     the rating set on the iPod
					     itself */
  track->checked = temp & 0xff;           /* Checked/Unchecked: 0/1 */
  track->unk124 = get4int (file, seek+124);
  track->unk128 = get4int (file, seek+128);
  track->unk132 = get4int (file, seek+132);
  track->unk136 = get4int (file, seek+136);
  track->unk140 = get4int (file, seek+140);
  track->unk144 = get4int (file, seek+144);
  track->unk148 = get4int (file, seek+148);
  track->unk152 = get4int (file, seek+152);

  track->transferred = TRUE;                   /* track is on iPod! */

#if ITUNESDB_MHIT_DEBUG
time_t time_mac_to_host (guint32 mactime);
gchar *time_time_to_string (time_t time);
#define printf_mhit(sk, str)  printf ("%3d: %d (%s)\n", sk, get4int (file, seek+sk), str);
#define printf_mhit_time(sk, str) { gchar *buf = time_time_to_string (itunesdb_time_mac_to_host (get4int (file, seek+sk))); printf ("%3d: %s (%s)\n", sk, buf, str); g_free (buf); }
  {
      printf ("\nmhit: seek=%lu\n", seek);
      printf_mhit (  4, "header size");
      printf_mhit (  8, "mhit size");
      printf_mhit ( 12, "nr of mhods");
      printf_mhit ( 16, "iPod ID");
      printf_mhit ( 20, "?");
      printf_mhit ( 24, "?");
      printf (" 28: %u (type)\n", get4int (file, seek+28) & 0xffffff);
      printf (" 28: %u (rating)\n", get4int (file, seek+28) >> 24);
      printf_mhit ( 32, "timestamp file");
      printf_mhit_time ( 32, "timestamp file");
      printf_mhit ( 36, "size");
      printf_mhit ( 40, "tracklen (ms)");
      printf_mhit ( 44, "track_nr");
      printf_mhit ( 48, "total tracks");
      printf_mhit ( 52, "year");
      printf_mhit ( 56, "bitrate");
      printf_mhit ( 60, "sample rate");
      printf (" 60: %u (sample rate LSB)\n", get4int (file, seek+60) & 0xffff);
      printf (" 60: %u (sample rate HSB)\n", (get4int (file, seek+60) >> 16));
      printf_mhit ( 64, "?");
      printf_mhit ( 68, "?");
      printf_mhit ( 72, "?");
      printf_mhit ( 76, "?");
      printf_mhit ( 80, "playcount");
      printf_mhit ( 84, "?");
      printf_mhit ( 88, "last played");
      printf_mhit_time ( 88, "last played");
      printf_mhit ( 92, "CD");
      printf_mhit ( 96, "total CDs");
      printf_mhit (100, "?");
      printf_mhit (104, "?");
      printf_mhit_time (104, "?");
      printf_mhit (108, "?");
      printf_mhit (112, "?");
      printf_mhit (116, "?");
      printf_mhit (120, "?");
      printf_mhit (124, "?");
      printf_mhit (128, "?");
      printf_mhit (132, "?");
      printf_mhit (136, "?");
      printf_mhit (140, "?");
      printf_mhit (144, "?");
      printf_mhit (148, "?");
      printf_mhit (152, "?");
  }
#undef printf_mhit_time
#undef printf_mhit
#endif

  seek += get4int (file, seek+4);             /* 1st mhod starts here! */
  while(zip != -1)
    {
     seek += zip;
     entry_utf16 = get_mhod (file, seek, &zip, &type);
     if (entry_utf16 != NULL) {
#ifdef ITUNESDB_PROVIDE_UTF8
       entry_utf8 = g_utf16_to_utf8 (entry_utf16, -1, NULL, NULL, NULL);
#endif
       switch (type)
	 {
	 case MHOD_ID_ALBUM:
#ifdef ITUNESDB_PROVIDE_UTF8
	   track->album = entry_utf8;
#endif
	   track->album_utf16 = entry_utf16;
	   break;
	 case MHOD_ID_ARTIST:
#ifdef ITUNESDB_PROVIDE_UTF8
	   track->artist = entry_utf8;
#endif
	   track->artist_utf16 = entry_utf16;
	   break;
	 case MHOD_ID_TITLE:
#ifdef ITUNESDB_PROVIDE_UTF8
	   track->title = entry_utf8;
#endif
	   track->title_utf16 = entry_utf16;
	   break;
	 case MHOD_ID_GENRE:
#ifdef ITUNESDB_PROVIDE_UTF8
	   track->genre = entry_utf8;
#endif
	   track->genre_utf16 = entry_utf16;
	   break;
	 case MHOD_ID_PATH:
#ifdef ITUNESDB_PROVIDE_UTF8
	   track->ipod_path = entry_utf8;
#endif
	   track->ipod_path_utf16 = entry_utf16;
	   break;
	 case MHOD_ID_FDESC:
#ifdef ITUNESDB_PROVIDE_UTF8
	   track->fdesc = entry_utf8;
#endif
	   track->fdesc_utf16 = entry_utf16;
	   break;
	 case MHOD_ID_COMMENT:
#ifdef ITUNESDB_PROVIDE_UTF8
	   track->comment = entry_utf8;
#endif
	   track->comment_utf16 = entry_utf16;
	   break;
	 case MHOD_ID_COMPOSER:
#ifdef ITUNESDB_PROVIDE_UTF8
	   track->composer = entry_utf8;
#endif
	   track->composer_utf16 = entry_utf16;
	   break;
	 default: /* unknown entry -- discard */
#ifdef ITUNESDB_PROVIDE_UTF8
	   g_free (entry_utf8);
#endif
	   g_free (entry_utf16);
	   break;
	 }
     }
    }

  playcount = get_next_playcount ();
  if (playcount)
  {
      if (playcount->rating)  track->rating = playcount->rating;
      if (playcount->time_played) track->time_played = playcount->time_played;
      track->playcount += playcount->playcount;
      track->recent_playcount = playcount->playcount;
      g_free (playcount);
  }
  it_add_track (track);
  return seek;   /* no more black magic */
}

/* get next playcount, that is the first entry of GList
 * playcounts. This entry is removed from the list. You must free the
 * return value after use */
static struct playcount *get_next_playcount (void)
{
    struct playcount *playcount = g_list_nth_data (playcounts, 0);

    if (playcount)  playcounts = g_list_remove (playcounts, playcount);
    return playcount;
}

/* delete all entries of GList *playcounts */
static void reset_playcounts (void)
{
    struct playcount *playcount;
    while ((playcount=get_next_playcount())) g_free (playcount);
}

/* Read the Play Count file (formed by adding "Play Counts" to the
 * directory contained in @dirname) and set up the GList *playcounts
 * */
static void init_playcounts (const gchar *dirname)
{
//  gchar *plcname = g_build_filename (dirname, "Play Counts", NULL);
  const gchar *db[] = {"Play Counts", NULL};
  gchar *plcname = resolve_path (dirname, db);
  FILE *plycts = NULL;
  gboolean error = TRUE;

  reset_playcounts ();

  if (plcname)  plycts = fopen (plcname, "r");

  if (plycts) do
  {
      gchar data[4];
      guint32 header_length, entry_length, entry_num, i=0;
      time_t tt = time (NULL);

      localtime (&tt);  /* set the ext. variable 'timezone' (see below) */
      if (seek_get_n_bytes (plycts, data, 0, 4) != 4)  break;
      if (cmp_n_bytes (data, "mhdp", 4) == FALSE)      break;
      header_length = get4int (plycts, 4);
      /* all the headers I know are 0x60 long -- if this one is longer
	 we can simply ignore the additional information */
      if (header_length < 0x60)                        break;
      entry_length = get4int (plycts, 8);
      /* all the entries I know are 0x0c (firmware 1.3) or 0x10
       * (firmware 2.0) in length */
      if (entry_length < 0x0c)                         break;
      /* number of entries */
      entry_num = get4int (plycts, 12);
      for (i=0; i<entry_num; ++i)
      {
	  struct playcount *playcount = g_malloc0 (sizeof (struct playcount));
	  glong seek = header_length + i*entry_length;

	  playcounts = g_list_append (playcounts, playcount);
	  /* check if entry exists by reading its last four bytes */
	  if (seek_get_n_bytes (plycts, data,
				seek+entry_length-4, 4) != 4) break;
	  playcount->playcount = get4int (plycts, seek);
	  playcount->time_played = get4int (plycts, seek+4);
          /* NOTE:
	   *
	   * The iPod (firmware 1.3) doesn't seem to use the timezone
	   * information correctly -- no matter what you set iPod's
	   * timezone to it will always record in UTC -- we need to
	   * subtract the difference between current timezone and UTC
	   * to get a correct display. 'timezone' (initialized above)
	   * contains the difference in seconds.
           */
	  if (playcount->time_played)
	      playcount->time_played += timezone;

	  /* rating only exists if the entry length is at least 0x10 */
	  if (entry_length >= 0x10)
	      playcount->rating = get4int (plycts, seek+12);
      }
      if (i == entry_num)  error = FALSE;
  } while (FALSE);
  if (plycts)  fclose (plycts);
  if (error)   reset_playcounts ();
  g_free (plcname);
}



/* Called by read_OTG_playlists(): reads and processes OTG playlist
 * file @filename by adding a new playlist (named @plname) with the
 * tracks specified in @filename. If @plname is NULL, a standard name
 * will be substituted */
/* Returns FALSE on error, TRUE on success */
static gboolean process_OTG_file (const gchar *filename,
				  const gchar *plname)
{
    FILE *otgf = NULL;
    gboolean result = FALSE;

    if (filename)  otgf = fopen (filename, "r");
    if (otgf)
    {
	gchar data[4];
	guint32 header_length, entry_length, entry_num, i=0;

	if (!plname) plname = _("OTG Playlist");

	if (seek_get_n_bytes (otgf, data, 0, 4) != 4)  goto end;
	if (cmp_n_bytes (data, "mhpo", 4) == FALSE)    goto end;
	header_length = get4int (otgf, 4);
	/* all the headers I know are 0x14 long -- if this one is
	 longer we can simply ignore the additional information */
	if (header_length < 0x14)                      goto end;
	entry_length = get4int (otgf, 8);
	/* all the entries I know are 0x04 long */
	if (entry_length < 0x04)                       goto end;
	/* number of entries */
	entry_num = get4int (otgf, 12);
	if (entry_num > 0)
	{
	    /* Create new playlist */
	    Playlist *pl = g_malloc0 (sizeof (Playlist));
#ifdef ITUNESDB_PROVIDE_UTF8
	    pl->name = g_strdup (plname);
#endif
	    pl->name_utf16 = g_utf8_to_utf16 (plname, -1,
					      NULL, NULL, NULL);
	    pl->type = PL_TYPE_NORM;
	    pl = it_add_playlist (pl);

	    /* Add items */
	    for (i=0; i<entry_num; ++i)
	    {
		guint32 id = get4int (otgf,
				      header_length + entry_length *i);
		it_add_trackid_to_playlist (pl, id);
	    }
	}
	result = TRUE;
    }
  end:
    if (otgf)   fclose (otgf);
    return result;
}


/* Add the On-The-Go Playlist(s) to the main playlist */
/* The OTG-Files are located in @dirname */
static void read_OTG_playlists (const gchar *dirname)
{
    gchar *filename;
    gint i=1;
    gchar *db[] = {"OTGPlaylistInfo", NULL};
    gchar *otgname = resolve_path (dirname, (const gchar **)db);

    /* only parse if "OTGPlaylistInfo" exists */
    if (otgname) do
    {
	db[0] = g_strdup_printf ("OTGPlaylistInfo_%d", i);
	filename = resolve_path (dirname, (const gchar **)db);
	g_free (db[0]);
	if (filename)
	{
	    gchar *plname = g_strdup_printf (_("OTG Playlist %d"), i);
	    process_OTG_file (filename, plname);
	    g_free (filename);
	    g_free (plname);
	    ++i;
	}
    } while (filename);
    g_free (otgname);
}


/* Parse the iTunesDB and store the tracks using it_addtrack () defined
   in track.c.
   Returns TRUE on success, FALSE on error.
   "path" should point to the mount point of the iPod,
   e.g. "/mnt/ipod" and be in local encoding */
/* Support for playlists should be added later */
gboolean itunesdb_parse (const gchar *path)
{
  gchar *filename;
  const gchar *db[] = {"iPod_Control","iTunes","iTunesDB",NULL};
  gboolean result;

  filename = resolve_path(path, db);
  result = itunesdb_parse_file (filename);
  g_free (filename);
  return result;
}

/* Same as itunesdb_parse(), but let's specify the filename directly */
gboolean itunesdb_parse_file (const gchar *filename)
{
  FILE *itunes = NULL;
  gboolean result = FALSE;
  gchar data[8];
  glong seek=0, pl_mhsd=0;
  guint32 zip, nr_tracks=0, nr_playlists=0;
  gboolean swapped_mhsd = FALSE;
  gchar *dirname=NULL;

#if ITUNESDB_DEBUG
  fprintf(stderr, "Parsing %s\nenter: %4d\n", filename, it_get_nr_of_tracks ());
#endif

  if (!filename) return FALSE;

  /* extract valid directory base from @filename */
  dirname = g_path_get_dirname (filename);

  itunes = fopen (filename, "r");
  do
  { /* dummy loop for easier error handling */
      if (itunes == NULL)
      {
	  itunesdb_warning (_("Could not open iTunesDB \"%s\" for reading.\n"),
			  filename);
	  break;
      }
      if (seek_get_n_bytes (itunes, data, seek, 4) != 4)
      {
	  itunesdb_warning (_("Error reading \"%s\".\n"), filename);
	  break;
      }
      /* for(i=0; i<8; ++i)  printf("%02x ", data[i]); printf("\n");*/
      if (cmp_n_bytes (data, "mhbd", 4) == FALSE)
      {
	  itunesdb_warning (_("\"%s\" is not a iTunesDB.\n"), filename);
	  break;
      }
      seek = get4int (itunes, 4);
      /* all the headers I know are 0x68 long -- if this one is longer
	 we can simply ignore the additional information */
      /* we don't need any information of the mhbd header... */
      /*      if (seek < 0x68)
      {
	  itunesdb_warning (_("\"%s\" is not a iTunesDB.\n"), filename);
	  break;
	  }*/
      do
      {
	  if (seek_get_n_bytes (itunes, data, seek, 8) != 8)  break;
	  if (cmp_n_bytes (data, "mhsd", 4) == TRUE)
	  { /* mhsd header -> determine start of playlists */
	      if (get4int (itunes, seek + 12) == 1)
	      { /* OK, tracklist, save start of playlists */
		  if (!swapped_mhsd)
		      pl_mhsd = seek + get4int (itunes, seek+8);
	      }
	      else if (get4int (itunes, seek + 12) == 2)
	      { /* bad: these are playlists... switch */
		  if (swapped_mhsd)
		  { /* already switched once -> forget it */
		      break;
		  }
		  else
		  {
		      pl_mhsd = seek;
		      seek += get4int (itunes, seek+8);
		      swapped_mhsd = TRUE;
		  }
	      }
	      else
	      { /* neither playlist nor track MHSD --> skip it */
		  seek += get4int (itunes, seek+8);
	      }
	  }
	  if (cmp_n_bytes (data, "mhlt", 4) == TRUE)
	  { /* mhlt header -> number of tracks */
	      nr_tracks = get4int (itunes, seek+8);
	      if (nr_tracks == 0)
	      {   /* no tracks -- skip directly to next mhsd */
		  result = TRUE;
		  break;
	      }
	  }
	  if (cmp_n_bytes (data, "mhit", 4) == TRUE)
	  { /* mhit header -> start of tracks*/
	      result = TRUE;
	      break;
	  }
	  zip = get4int (itunes, seek+4);
	  if (zip == 0)  break;
	  seek += zip;
      } while (result == FALSE);
      if (result == FALSE)  break; /* some error occured */
      result = FALSE;
      /* now we should be at the first MHIT */

      /* Read Play Count file if available */
      init_playcounts (dirname);

      /* get every file entry */
      if (nr_tracks)  while(seek != -1) {
	  /* get_mhit returns where it's guessing the next MHIT,
	     if it fails, it returns '-1' */
	  seek = get_mhit (itunes, seek);
      }

      /* next: playlists */
      seek = pl_mhsd;
      do
      {
	  if (seek_get_n_bytes (itunes, data, seek, 8) != 8)  break;
	  if (cmp_n_bytes (data, "mhsd", 4) == TRUE)
	  { /* mhsd header */
	      if (get4int (itunes, seek + 12) != 2)
	      {  /* this is not a playlist MHSD -> skip it */
		  seek += get4int (itunes, seek+8);
	      }
	  }
	  if (cmp_n_bytes (data, "mhlp", 4) == TRUE)
	  { /* mhlp header -> number of playlists */
	      nr_playlists = get4int (itunes, seek+8);
	  }
	  if (cmp_n_bytes (data, "mhyp", 4) == TRUE)
	  { /* mhyp header -> start of playlists */
	      result = TRUE;
	      break;
	  }
	  zip = get4int (itunes, seek+4);
	  if (zip == 0)  break;
	  seek += zip;
      } while (result == FALSE);
      if (result == FALSE)  break; /* some error occured */
      result = FALSE;

#if ITUNESDB_DEBUG
    fprintf(stderr, "iTunesDB part2 starts at: %x\n", (int)seek);
#endif

    while(seek != -1) {
	seek = get_pl(itunes, seek);
    }

    result = TRUE;
  } while (FALSE);

  if (itunes != NULL)     fclose (itunes);

  /* Read OTG playlists */
  read_OTG_playlists (dirname);

  g_free (dirname);
#if ITUNESDB_DEBUG
  fprintf(stderr, "exit:  %4d\n", it_get_nr_of_tracks ());
#endif
  return result;
}


/* up to here we had the functions for reading the iTunesDB               */
/* ---------------------------------------------------------------------- */
/* from here on we have the functions for writing the iTunesDB            */

/* Name of the device in utf16 */
gunichar2 ipod_name[] = { 'g', 't', 'k', 'p', 'o', 'd', 0 };


/* Get length of utf16 string in number of characters (words) */
static guint32 utf16_strlen (gunichar2 *utf16)
{
  guint32 i=0;
  while (utf16[i] != 0) ++i;
  return i;
}


/* Write 4-byte-integer "n" in correct order to "data".
   "data" must be sufficiently long ... */
static void store4int (guint32 n, guchar *data)
{
  data[3] = (n >> 24) & 0xff;
  data[2] = (n >> 16) & 0xff;
  data[1] = (n >>  8) & 0xff;
  data[0] =  n & 0xff;
}


/* Write "data", "n" bytes long to current position in file.
   Returns TRUE on success, FALSE otherwise */
static gboolean put_data_cur (FILE *file, gchar *data, gint n)
{
  if (fwrite (data, 1, n, file) != n) return FALSE;
  return TRUE;
}

/* Write 4-byte integer "n" to "file".
   Returns TRUE on success, FALSE otherwise */
static gboolean put_4int_cur (FILE *file, guint32 n)
{
  gchar data[4];

  store4int (n, data);
  return put_data_cur (file, data, 4);
}


/* Write 4-byte integer "n" to "file" at position "seek".
   After writing, the file position indicator is set
   to the end of the file.
   Returns TRUE on success, FALSE otherwise */
static gboolean put_4int_seek (FILE *file, guint32 n, gint seek)
{
  gboolean result;

  if (fseek (file, seek, SEEK_SET) != 0) return FALSE;
  result = put_4int_cur (file, n);
  if (fseek (file, 0, SEEK_END) != 0) return FALSE;
  return result;
}


/* Write "n" times 4-byte-zero at current position
   Returns TRUE on success, FALSE otherwise */
static gboolean put_n0_cur (FILE*file, guint32 n)
{
  guint32 i;
  gboolean result = TRUE;

  for (i=0; i<n; ++i)  result &= put_4int_cur (file, 0);
  return result;
}



/* Write out the mhbd header. Size will be written later */
static void mk_mhbd (FILE *file)
{
  put_data_cur (file, "mhbd", 4);
  put_4int_cur (file, 104); /* header size */
  put_4int_cur (file, -1);  /* size of whole mhdb -- fill in later */
  put_4int_cur (file, 1);   /* ? */
  put_4int_cur (file, 1);   /*  - changed to 2 from itunes2 to 3 ..
				    version? We are iTunes version 1 ;) */
  put_4int_cur (file, 2);   /* ? */
  put_n0_cur (file, 20);    /* dummy space */
}

/* Fill in the missing items of the mhsd header:
   total size and number of mhods */
static void fix_mhbd (FILE *file, glong mhbd_seek, glong cur)
{
  put_4int_seek (file, cur-mhbd_seek, mhbd_seek+8); /* size of whole mhit */
}


/* Write out the mhsd header. Size will be written later */
static void mk_mhsd (FILE *file, guint32 type)
{
  put_data_cur (file, "mhsd", 4);
  put_4int_cur (file, 96);   /* Headersize */
  put_4int_cur (file, -1);   /* size of whole mhsd -- fill in later */
  put_4int_cur (file, type); /* type: 1 = track, 2 = playlist */
  put_n0_cur (file, 20);    /* dummy space */
}


/* Fill in the missing items of the mhsd header:
   total size and number of mhods */
static void fix_mhsd (FILE *file, glong mhsd_seek, glong cur)
{
  put_4int_seek (file, cur-mhsd_seek, mhsd_seek+8); /* size of whole mhit */
}


/* Write out the mhlt header. */
static void mk_mhlt (FILE *file, guint32 track_num)
{
  put_data_cur (file, "mhlt", 4);
  put_4int_cur (file, 92);         /* Headersize */
  put_4int_cur (file, track_num);   /* tracks in this itunesdb */
  put_n0_cur (file, 20);           /* dummy space */
}


/* Write out the mhit header. Size will be written later */
static void mk_mhit (FILE *file, Track *track)
{
  put_data_cur (file, "mhit", 4);
  put_4int_cur (file, 156);  /* header size */
  put_4int_cur (file, -1);   /* size of whole mhit -- fill in later */
  put_4int_cur (file, -1);   /* nr of mhods in this mhit -- later   */
  put_4int_cur (file, track->ipod_id); /* track index number
					* */
  put_4int_cur (file, track->unk020);
  put_4int_cur (file, track->unk024);
  put_4int_cur (file, (track->rating << 24) |
		(track->compilation << 16) |
		(track->type & 0x0000ffff));/* rating, compil., type */

  put_4int_cur (file, track->time_created); /* timestamp             */
  put_4int_cur (file, track->size);    /* filesize                  */
  put_4int_cur (file, track->tracklen); /* length of track in ms     */
  put_4int_cur (file, track->track_nr);/* track number               */
  put_4int_cur (file, track->tracks);  /* number of tracks           */
  put_4int_cur (file, track->year);    /* the year                   */
  put_4int_cur (file, track->bitrate); /* bitrate                    */
  put_4int_cur (file, track->samplerate << 16);
  put_4int_cur (file, track->volume);  /* volume adjust              */
  put_4int_cur (file, track->starttime);
  put_4int_cur (file, track->stoptime);
  put_4int_cur (file, track->soundcheck);
  put_4int_cur (file, track->playcount);/* playcount                 */
  put_4int_cur (file, track->unk084);
  put_4int_cur (file, track->time_played); /* last time played       */
  put_4int_cur (file, track->cd_nr);   /* CD number                  */
  put_4int_cur (file, track->cds);     /* number of CDs              */
  put_4int_cur (file, track->unk100);
  put_4int_cur (file, track->time_modified); /* timestamp            */
  put_4int_cur (file, track->unk108);
  put_4int_cur (file, track->unk112);
  put_4int_cur (file, track->unk116);
  put_4int_cur (file, (track->BPM << 16) |
		((track->app_rating & 0xff) << 8) |
		(track->checked & 0xff));
  put_4int_cur (file, track->unk124);
  put_4int_cur (file, track->unk128);
  put_4int_cur (file, track->unk132);
  put_4int_cur (file, track->unk136);
  put_4int_cur (file, track->unk140);
  put_4int_cur (file, track->unk144);
  put_4int_cur (file, track->unk148);
  put_4int_cur (file, track->unk152);
}


/* Fill in the missing items of the mhit header:
   total size and number of mhods */
static void fix_mhit (FILE *file, glong mhit_seek, glong cur, gint mhod_num)
{
  put_4int_seek (file, cur-mhit_seek, mhit_seek+8); /* size of whole mhit */
  put_4int_seek (file, mhod_num, mhit_seek+12);     /* nr of mhods        */
}


/* Write out one mhod header.
     type: see enum of MHMOD_IDs;
     string: utf16 string to pack
     fqid: will be used for playlists -- use 1 for tracks */
static void mk_mhod (FILE *file, guint32 type,
		     gunichar2 *string, guint32 fqid)
{
  guint32 mod;
  guint32 len;

  if (fqid == 1) mod = 40;   /* normal mhod */
  else           mod = 44;   /* playlist entry */

  len = utf16_strlen (string);         /* length of string in _words_     */

  put_data_cur (file, "mhod", 4);      /* header                          */
  put_4int_cur (file, 24);             /* size of header                  */
  put_4int_cur (file, 2*len+mod);      /* size of header + body           */
  put_4int_cur (file, type);           /* type of the entry               */
  put_n0_cur (file, 2);                /* dummy space                     */
  put_4int_cur (file, fqid);           /* refers to this ID if a PL item,
					  otherwise always 1              */
  put_4int_cur (file, 2*len);          /* size of string                  */
  if (type < 100)
    {                                     /* no PL mhod */
      put_n0_cur (file, 2);               /* trash      */
      /* FIXME: this assumes "string" is writable.
	 However, this might not be the case,
	 e.g. ipod_name might be in read-only mem. */
      string = fixup_utf16(string);
      put_data_cur (file, (gchar *)string, 2*len); /* the string */
      string = fixup_utf16(string);
    }
  else
    {
      put_n0_cur (file, 3);     /* PL mhods are different ... */
    }
}


/* Write out the mhlp header. Size will be written later */
static void mk_mhlp (FILE *file, guint32 lists)
{
  put_data_cur (file, "mhlp", 4);      /* header                   */
  put_4int_cur (file, 92);             /* size of header           */
  put_4int_cur (file, lists);          /* playlists on iPod (including main!) */
  put_n0_cur (file, 20);               /* dummy space              */
}


/* Fix the mhlp header */
static void fix_mhlp (FILE *file, glong mhlp_seek, gint playlist_num)
{
  put_4int_seek (file, playlist_num, mhlp_seek+8); /* nr of playlists    */
}



/* Write out the "weird" header.
   This seems to be an itunespref thing.. dunno know this
   but if we set everything to 0, itunes doesn't show any data
   even if you drag an mp3 to your ipod: nothing is shown, but itunes
   will copy the file!
   .. so we create a hardcoded-pref.. this will change in future
   Seems to be a Preferences mhod, every PL has such a thing
   FIXME !!! */
static void mk_weired (FILE *file)
{
  put_data_cur (file, "mhod", 4);      /* header                   */
  put_4int_cur (file, 0x18);           /* size of header  ?        */
  put_4int_cur (file, 0x0288);         /* size of header + body    */
  put_4int_cur (file, 0x64);           /* type of the entry        */
  put_n0_cur (file, 6);
  put_4int_cur (file, 0x010084);       /* ? */
  put_4int_cur (file, 0x05);           /* ? */
  put_4int_cur (file, 0x09);           /* ? */
  put_4int_cur (file, 0x03);           /* ? */
  put_4int_cur (file, 0x120001);       /* ? */
  put_n0_cur (file, 3);
  put_4int_cur (file, 0xc80002);       /* ? */
  put_n0_cur (file, 3);
  put_4int_cur (file, 0x3c000d);       /* ? */
  put_n0_cur (file, 3);
  put_4int_cur (file, 0x7d0004);       /* ? */
  put_n0_cur (file, 3);
  put_4int_cur (file, 0x7d0003);       /* ? */
  put_n0_cur (file, 3);
  put_4int_cur (file, 0x640008);       /* ? */
  put_n0_cur (file, 3);
  put_4int_cur (file, 0x640017);       /* ? */
  put_4int_cur (file, 0x01);           /* bool? (visible? / colums?) */
  put_n0_cur (file, 2);
  put_4int_cur (file, 0x500014);       /* ? */
  put_4int_cur (file, 0x01);           /* bool? (visible?) */
  put_n0_cur (file, 2);
  put_4int_cur (file, 0x7d0015);       /* ? */
  put_4int_cur (file, 0x01);           /* bool? (visible?) */
  put_n0_cur (file, 114);
}


/* Write out the mhyp header. Size will be written later */
static void mk_mhyp (FILE *file, gunichar2 *listname,
		     guint32 type, guint32 track_num)
{
  put_data_cur (file, "mhyp", 4);      /* header                   */
  put_4int_cur (file, 108);            /* length		   */
  put_4int_cur (file, -1);             /* size -> later            */
  put_4int_cur (file, 2);              /* ?                        */
  put_4int_cur (file, track_num);       /* number of tracks in plist */
  put_4int_cur (file, type);           /* 1 = main, 0 = visible    */
  put_4int_cur (file, 0);              /* ?                        */
  put_4int_cur (file, 0);              /* ?                        */
  put_4int_cur (file, 0);              /* ?                        */
  put_n0_cur (file, 18);               /* dummy space              */
  mk_weired (file);
  mk_mhod (file, MHOD_ID_TITLE, listname, 1);
}


/* Fix the mhyp header */
static void fix_mhyp (FILE *file, glong mhyp_seek, glong cur)
{
  put_4int_seek (file, cur-mhyp_seek, mhyp_seek+8);
    /* size */
}


/* Header for new PL item */
static void mk_mhip (FILE *file, guint32 id)
{
  put_data_cur (file, "mhip", 4);
  put_4int_cur (file, 76);
  put_4int_cur (file, 76);
  put_4int_cur (file, 1);
  put_4int_cur (file, 0);
  put_4int_cur (file, id);  /* track id in playlist */
  put_4int_cur (file, id);  /* ditto.. don't know the difference, but this
                               seems to work. Maybe a special ID used for
			       playlists? */
  put_n0_cur (file, 12);
}

static void
write_mhsd_one(FILE *file)
{
    Track *track;
    guint32 i, track_num, mhod_num;
    glong mhsd_seek, mhit_seek, mhlt_seek;

    track_num = it_get_nr_of_tracks();

    mhsd_seek = ftell (file);  /* get position of mhsd header */
    mk_mhsd (file, 1);         /* write header: type 1: track  */
    mhlt_seek = ftell (file);  /* get position of mhlt header */
    mk_mhlt (file, track_num);  /* write header with nr. of tracks */
    for (i=0; i<track_num; ++i)  /* Write each track */
    {
	if((track = it_get_track_by_nr (i)) == 0)
	{
	    g_warning ("Invalid track Index!\n");
	    break;
	}
	mhit_seek = ftell (file);
	mk_mhit (file, track);
	mhod_num = 0;
	if (utf16_strlen (track->title_utf16) != 0)
	{
	    mk_mhod (file, MHOD_ID_TITLE, track->title_utf16, 1);
	    ++mhod_num;
	}
	if (utf16_strlen (track->ipod_path_utf16) != 0)
	{
	    mk_mhod (file, MHOD_ID_PATH, track->ipod_path_utf16, 1);
	    ++mhod_num;
	}
	if (utf16_strlen (track->album_utf16) != 0)
	{
	    mk_mhod (file, MHOD_ID_ALBUM, track->album_utf16, 1);
	    ++mhod_num;
	}
	if (utf16_strlen (track->artist_utf16) != 0)
	{
	    mk_mhod (file, MHOD_ID_ARTIST, track->artist_utf16, 1);
	    ++mhod_num;
	}
	if (utf16_strlen (track->genre_utf16) != 0)
	{
	    mk_mhod (file, MHOD_ID_GENRE, track->genre_utf16, 1);
	    ++mhod_num;
	}
	if (utf16_strlen (track->fdesc_utf16) != 0)
	{
	    mk_mhod (file, MHOD_ID_FDESC, track->fdesc_utf16, 1);
	    ++mhod_num;
	}
	if (utf16_strlen (track->comment_utf16) != 0)
	{
	    mk_mhod (file, MHOD_ID_COMMENT, track->comment_utf16, 1);
	    ++mhod_num;
	}
	if (utf16_strlen (track->composer_utf16) != 0)
	{
	    mk_mhod (file, MHOD_ID_COMPOSER, track->composer_utf16, 1);
	    ++mhod_num;
	}
        /* Fill in the missing items of the mhit header */
	fix_mhit (file, mhit_seek, ftell (file), mhod_num);
    }
    fix_mhsd (file, mhsd_seek, ftell (file));
}

static void
write_playlist(FILE *file, Playlist *pl)
{
    guint32 i, n;
    glong mhyp_seek;
    gunichar2 empty = 0;

    mhyp_seek = ftell(file);
    n = it_get_nr_of_tracks_in_playlist (pl);
#if ITUNESDB_DEBUG
  fprintf(stderr, "Playlist: %s (%d tracks)\n", pl->name, n);
#endif
    mk_mhyp(file, pl->name_utf16, pl->type, n);
    for (i=0; i<n; ++i)
    {
	Track *track;
        if((track = it_get_track_in_playlist_by_nr (pl, i)))
	{
	    mk_mhip(file, track->ipod_id);
	    mk_mhod(file, MHOD_ID_PLAYLIST, &empty, track->ipod_id);
	}
    }
   fix_mhyp (file, mhyp_seek, ftell(file));
}



/* Expects the master playlist to be (it_get_playlist_by_nr (0)) */
static void
write_mhsd_two(FILE *file)
{
    guint32 playlists, i;
    glong mhsd_seek, mhlp_seek;

    mhsd_seek = ftell (file);  /* get position of mhsd header */
    mk_mhsd (file, 2);         /* write header: type 2: playlists  */
    mhlp_seek = ftell (file);
    playlists = it_get_nr_of_playlists();
    mk_mhlp (file, playlists);
    for(i = 0; i < playlists; i++)
    {
	write_playlist(file, it_get_playlist_by_nr(i));
    }
    fix_mhlp (file, mhlp_seek, playlists);
    fix_mhsd (file, mhsd_seek, ftell (file));
}


/* Do the actual writing to the iTunesDB */
gboolean
write_it (FILE *file)
{
    glong mhbd_seek;

    mhbd_seek = 0;
    mk_mhbd (file);
    write_mhsd_one(file);		/* write tracks mhsd */
    write_mhsd_two(file);		/* write playlists mhsd */
    fix_mhbd (file, mhbd_seek, ftell (file));
    return TRUE;
}

/* Write out an iTunesDB.
   Note: only the _utf16 entries in the Track-struct are used

   An existing "Play Counts" file is renamed to "Play Counts.bak" if
   the export was successful.
   An existing "OTGPlaylistInfo" file is removed if the export was
   successful.

   Returns TRUE on success, FALSE on error.
   "path" should point to the mount point of the
   iPod, e.g. "/mnt/ipod" and be in local encoding */
gboolean itunesdb_write (const gchar *path)
{
    gchar *itunes_filename, *itunes_path;
/*    gchar *tmp_filename = g_build_filename (P_tmpdir, "itunesDB.XXXXXX", NULL);*/
/*     int fd; */
    const gchar *itunes[] = {"iPod_Control","iTunes",NULL};
    gboolean result = FALSE;

    itunes_path = resolve_path(path, itunes);
    
    if(!itunes_path)
      return FALSE;
    
    itunes_filename = g_build_filename (itunes_path, "iTunesDB", NULL);
    /* We write to /tmp/ first, then copy to the iPod. For me this
       means a speed increase from 38 seconds to 4 seconds (my iPod is
       mounted "sync" -- that means all changes are written to the
       iPod immediately. Because we are seeking around the file,
       writing is slow. */
/*     printf ("%s: %s\n", P_tmpdir, tmp_filename); */
/*     fd = mkstemp (tmp_filename); */
/*     printf ("%s: %d %d\n", tmp_filename, fd, errno); */
/*     if (fd != -1) */
/*     { */
/* 	close (fd); */
/* 	result = itunesdb_write_to_file (tmp_filename); */
/* 	if (result) */
/* 	{ */
/* 	    itunesdb_cp (tmp_filename, itunes_filename); */
/* 	    remove (tmp_filename); */
/* 	} */
/*     } */
/*     else */
/*     {   /\* tmp file could not be opened *\/ */
/* 	result = itunesdb_write_to_file (itunes_filename); */
/*     } */
/*  g_free(tmp_filename);*/
    result = itunesdb_write_to_file (itunes_filename);
    g_free(itunes_filename);
    g_free(itunes_path);
    return result;
}

/* Same as itnuesdb_write (), but you specify the filename directly */
gboolean itunesdb_write_to_file (const gchar *filename)
{
  FILE *file = NULL;
  gboolean result = FALSE;

#if ITUNESDB_DEBUG
  fprintf(stderr, "Writing to %s\n", filename);
#endif

  if((file = fopen (filename, "w+")))
    {
      write_it (file);
      fclose(file);
      result = TRUE;
    }
  else
    {
      itunesdb_warning (_("Could not open iTunesDB \"%s\" for writing.\n"),
		      filename);
      result = FALSE;
    }
  if (result == TRUE)
  {
      gchar *dirname = g_path_get_dirname (filename);
      itunesdb_rename_files (dirname);
      g_free (dirname);
  }
  return result;
}


void itunesdb_rename_files (const gchar *dirname)
{
    const gchar *db_plc_o[] = {"Play Counts", NULL};
    const gchar *db_plc_n[] = {"Play Counts.bak", NULL};
    const gchar *db_otg[] = {"OTGPlaylistInfo", NULL};
    gchar *plcname_o = resolve_path (dirname, db_plc_o);
    gchar *plcname_n = resolve_path (dirname, db_plc_n);
    gchar *otgname = resolve_path (dirname, db_otg);

    /* rename "Play Counts" to "Play Counts.bak" */
    if (plcname_o)
    {
	if (rename (plcname_o, plcname_n) == -1)
	{   /* an error occured */
	    itunesdb_warning (_("Error renaming '%s' to '%s' (%s).\n"),
			      plcname_o, plcname_n, g_strerror (errno));
	}
    }
    /* remove "OTGPlaylistInfo" (the iPod will remove the remaining
     * files */
    if (otgname)
    {
	if (unlink (otgname) == -1)
	{   /* an error occured */
	    itunesdb_warning (_("Error removing '%s'.\n"), otgname);
	}
    }
    g_free (plcname_o);
    g_free (plcname_n);
    g_free (otgname);
}

/* Convert string from casual PC file name to iPod iTunesDB format
 * using ':' instead of slashes
 */
void itunesdb_convert_filename_fs2ipod (gchar *ipod_file)
{
    g_strdelimit (ipod_file, G_DIR_SEPARATOR_S, ':');
}

/* Convert string from iPod iTunesDB format to casual PC file name
 * using slashes instead of ':'
 */
void itunesdb_convert_filename_ipod2fs (gchar *ipod_file)
{
    g_strdelimit (ipod_file, ":", G_DIR_SEPARATOR);
}

/* create a new track structure with the "unknowns" filled in correctly
 * */ 
Track *itunesdb_new_track (void)
{
    Track *track = g_malloc0 (sizeof (Track));
    track->unk020 = 1;
    return track;
}

/* Copy one track to the ipod. The PC-Filename is
   "pcfile" and is taken literally.
   "path" is assumed to be the mountpoint of the iPod (in local
   encoding).
   For storage, the directories "f00 ... f19" will be
   cycled through. The filename is constructed from
   "track->ipod_id": "gtkpod_id" and written to
   "track->ipod_path_utf8" and "track->ipod_path_utf16" */
gboolean itunesdb_copy_track_to_ipod (const gchar *path,
				      Track *track,
				      const gchar *pcfile)
{
  static gint dir_num = -1;
  gchar *track_db_path = NULL, *ipod_fullfile = NULL;
  gchar *dest_components[] = {"","iPod_Control","Music",NULL,NULL,NULL},
    *parent_dir_filename;
  gchar *original_suffix;
  gchar dir_num_str[5];
  gboolean success;
  gint32 oops = 0;
  gint pathlen = 0;

#if ITUNESDB_DEBUG
  fprintf(stderr, "Entered itunesdb_copy_track_to_ipod: '%s', %p, '%s'\n", path, track, pcfile);
#endif
  if (track == NULL)
    {
      g_warning ("Programming error: copy_track_to_ipod () called NULL-track\n");
      return FALSE;
    }
  if(track->transferred == TRUE) return TRUE; /* nothing to do */ 

  if (path) pathlen = strlen (path); /* length of path in bytes */
  
  if (dir_num == -1) dir_num = g_random_int_range (0, 20);
  else dir_num = (dir_num + 1) % 20;
  
  g_snprintf(dir_num_str,5,"F%02d",dir_num);
  dest_components[3] = dir_num_str;
  
  parent_dir_filename = resolve_path(path,(const gchar **)dest_components);
  if(parent_dir_filename == NULL) {
          /* Can't find the parent of the filenames we're going to generate to copy into */
          return FALSE;
  }

  /* we may need the original suffix of pcfile to construct a correct
     ipod filename */
  original_suffix = strrchr (pcfile, '.');
  /* If there is no ".mp3", ".m4a" etc, set original_suffix to empty
     string. Note: the iPod will most certainly ignore this file... */
  if (!original_suffix) original_suffix = "";

  /* If track->ipod_path exists, we use that one instead. */

  for (ipod_fullfile = itunesdb_get_track_name_on_ipod (path, track) ; !ipod_fullfile ; oops++)
  { /* we need to loop until we find an unused filename */
      dest_components[4] = 
        g_strdup_printf("gtkpod%05d%s",track->ipod_id + oops,original_suffix);
      ipod_fullfile = resolve_path(path,
                                   (const gchar **)dest_components);
      if(ipod_fullfile)
      {
              g_free(ipod_fullfile);
              ipod_fullfile = NULL;
      }
      else
      {
        ipod_fullfile = g_build_filename(parent_dir_filename,
					 dest_components[4], NULL);
        track_db_path = g_strjoinv(":", dest_components);
/* 	printf ("ff: %s\ndb: %s\n", ipod_fullfile, track_db_path); */
      }
      g_free(dest_components[4]);
  }
  
  if(!track_db_path)
    track_db_path = g_strdup(track->ipod_path);
  

#if ITUNESDB_DEBUG
  fprintf(stderr, "ipod_fullfile: '%s'\n", ipod_fullfile);
#endif

  success = itunesdb_cp (pcfile, ipod_fullfile);
  if (success)
  {
      track->transferred = TRUE;
#ifdef ITUNESDB_PROVIDE_UTF8
      if (track->ipod_path) g_free (track->ipod_path);
      track->ipod_path = g_strdup (track_db_path);
#endif
      if (track->ipod_path_utf16) g_free (track->ipod_path_utf16);
      track->ipod_path_utf16 = g_utf8_to_utf16 (track_db_path, -1, NULL, NULL, NULL);
  }
  
  g_free(parent_dir_filename);
  g_free (track_db_path);
  g_free (ipod_fullfile);
  return success;
}


/* Return the full iPod filename as stored in @track. Return value
   must be g_free()d after use.
   @path: mount point of the iPod file system (in local encoding)
   @track: track
   Return value: full filename to @track on the iPod or NULL if no
   filename is set in @track. NOTE: the file does not necessarily
   exist. NOTE: this code works around a problem on some systems (see
   below) and might return a filename with different case than the
   original filename. Don't copy it back to @track */
gchar *itunesdb_get_track_name_on_ipod (const gchar *path, Track *track)
{
  gchar *result,*buf,*good_path,**components,*resolved;

  if(!track || !track->ipod_path || !*track->ipod_path)
    return NULL;
        
  buf = g_strdup (track->ipod_path);
  itunesdb_convert_filename_ipod2fs (buf);
  result = g_build_filename (path, buf, NULL);
  g_free(buf);
  if (g_file_test (result, G_FILE_TEST_EXISTS))
    return result;
    
  g_free(result);
  
  good_path = g_filename_from_utf8(path,-1,NULL,NULL,NULL);
  components = g_strsplit(track->ipod_path,":",10);
  
  resolved = resolve_path(good_path,(const gchar **)components);
  
  g_free(good_path);
  g_strfreev(components);
  
  return resolved;
}


/* Copy file "from_file" to "to_file".
   Returns TRUE on success, FALSE otherwise */
gboolean itunesdb_cp (const gchar *from_file, const gchar *to_file)
{
  gchar *data=g_malloc (ITUNESDB_COPYBLK);
  glong bread, bwrite;
  gboolean success = TRUE;
  FILE *file_in = NULL;
  FILE *file_out = NULL;

#if ITUNESDB_DEBUG
  fprintf(stderr, "Entered itunesdb_cp: '%s', '%s'\n", from_file, to_file);
#endif

  do { /* dummy loop for easier error handling */
    file_in = fopen (from_file, "r");
    if (file_in == NULL)
      {
	itunesdb_warning (_("Could not open file \"%s\" for reading.\n"), from_file);
	success = FALSE;
	break;
      }
    file_out = fopen (to_file, "w");
    if (file_out == NULL)
      {
	itunesdb_warning (_("Could not open file \"%s\" for writing.\n"), to_file);
	success = FALSE;
	break;
      }
    do {
      bread = fread (data, 1, ITUNESDB_COPYBLK, file_in);
#if ITUNESDB_DEBUG
      fprintf(stderr, "itunesdb_cp: read %ld bytes\n", bread);
#endif
      if (bread == 0)
	{
	  if (feof (file_in) == 0)
	    { /* error -- not end of file! */
	      itunesdb_warning (_("Error reading file \"%s\".\n"), from_file);
	      success = FALSE;
	    }
	}
      else
	{
	  bwrite = fwrite (data, 1, bread, file_out);
#if ITUNESDB_DEBUG
      fprintf(stderr, "itunesdb_cp: wrote %ld bytes\n", bwrite);
#endif
	  if (bwrite != bread)
	    {
	      itunesdb_warning (_("Error writing PC file \"%s\".\n"),to_file);
	      success = FALSE;
	    }
	}
    } while (success && (bread != 0));
  } while (FALSE);
  if (file_in)  fclose (file_in);
  if (file_out)
    {
      fclose (file_out);
      if (!success)
      { /* error occured -> delete to_file */
#if ITUNESDB_DEBUG
	  fprintf(stderr, "itunesdb_cp: copy unsuccessful, removing '%s'\n", to_file);
#endif
	remove (to_file);
      }
    }
  g_free (data);
  return success;
}

/*------------------------------------------------------------------*\
 *                                                                  *
 *                       Timestamp stuff                            *
 *                                                                  *
\*------------------------------------------------------------------*/

guint32 itunesdb_time_get_mac_time (void)
{
    GTimeVal time;

    g_get_current_time (&time);
    return itunesdb_time_host_to_mac (time.tv_sec);
}


/* convert Macintosh timestamp to host system time stamp -- modify
 * this function if necessary to port to host systems with different
 * start of Epoch */
/* A "0" time will not be converted */
time_t itunesdb_time_mac_to_host (guint32 mactime)
{
    if (mactime != 0)  return ((time_t)mactime) - 2082844800;
    else               return (time_t)mactime;
}


/* convert host system timestamp to Macintosh time stamp -- modify
 * this function if necessary to port to host systems with different
 * start of Epoch */
guint32 itunesdb_time_host_to_mac (time_t time)
{
    return (guint32)(time + 2082844800);
}
