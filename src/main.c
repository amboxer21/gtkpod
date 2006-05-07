/* Time-stamp: <2006-05-08 00:55:06 jcs>
|
|  Copyright (C) 2002-2005 Jorg Schuler <jcsjcs at users sourceforge net>
|  Part of the gtkpod project.
| 
|  URL: http://www.gtkpod.org/
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

#include <stdlib.h>
#include <time.h>

#include "display.h"
#include "clientserver.h"
#include "prefs.h"
#include "misc.h"
#include "file.h"
//#include "podcast.h"

/* path to gtkpod.glade */
gchar *xml_file;

int
main (int argc, char *argv[])
{
    gchar *progname;
	
  

#ifdef ENABLE_NLS
  bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  textdomain (GETTEXT_PACKAGE);
#endif

#ifdef G_THREADS_ENABLED
  /* this must be called before gtk_init () */
  g_thread_init (NULL);
  /* FIXME: this call causes gtkpod to freeze as soon as tracks should be
     displayed */
/*   gdk_threads_init (); */
#endif

  gtk_init (&argc, &argv);

  srand(time(NULL));

  /* initialize xml_file: if gtkpod is called in the build directory
     (".../src/gtkpod") use the local gtkpod.glade (the symlink in the
     pixmaps directory), otherwise use
     "PACKAGE_DATA_DIR/PACKAGE/pixmaps/gtkpod.glade" */

  progname = g_find_program_in_path (argv[0]);
  if (progname)
  {
      static const gchar *SEPsrcSEPgtkpod = G_DIR_SEPARATOR_S "src" G_DIR_SEPARATOR_S "gtkpod";

      if (!g_path_is_absolute (progname))
      {
	  gchar *cur_dir = g_get_current_dir ();
	  gchar *prog_absolute;

	  if (g_str_has_prefix (progname, "." G_DIR_SEPARATOR_S))
	      prog_absolute = g_build_filename (cur_dir,progname+2,NULL);
	  else
	      prog_absolute = g_build_filename (cur_dir,progname,NULL);
	  g_free (progname);
	  g_free (cur_dir);
	  progname = prog_absolute;
      }

      if (g_str_has_suffix (progname, SEPsrcSEPgtkpod))
      {
	  gchar *suffix = g_strrstr (progname, SEPsrcSEPgtkpod);
	  if (suffix)
	  {
	      *suffix = 0;
	      xml_file = g_build_filename (progname, "pixmaps", "gtkpod.glade", NULL);
	  }
      }
      g_free (progname);
      if (!g_file_test (xml_file, G_FILE_TEST_EXISTS))
      {
	  g_free (xml_file);
	  xml_file = NULL;
      }
  }
  if (!xml_file)
      xml_file = g_build_filename (PACKAGE_DATA_DIR, PACKAGE, "pixmaps", "gtkpod.glade", NULL);
  else
  {
      printf ("Using local gtkpod.glade file since program was started from source directory:\n%s\n", xml_file);
  }

  main_window_xml = glade_xml_new (xml_file, "gtkpod", NULL);

  glade_xml_signal_autoconnect (main_window_xml);
  
  gtkpod_window = gtkpod_xml_get_widget (main_window_xml, "gtkpod");
  
  if (!read_prefs_old (gtkpod_window, argc, argv)) return 0;
	init_prefs(argc, argv);
  
	display_create ();

  init_data (gtkpod_window);   /* setup base data */

  /* stuff to be done before starting gtkpod */
  call_script ("gtkpod.in");

  gtk_widget_show (gtkpod_window);

/*  if(prefs_get_automount())      mount_ipod();*/
  if(prefs_get_autoimport() || prefs_get_autoimport_commandline())
      gp_merge_ipod_itdbs ();

  server_setup ();   /* start server to accept playcount updates */

/*   gdk_threads_enter (); */
  gtk_main ();
/*   gdk_threads_leave (); */

  /* all the cleanup is already done in gtkpod_main_quit() in misc.c */
  return 0;
}
