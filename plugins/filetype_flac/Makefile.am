plugin_name = filetype_flac
plugin_file = $(plugin_name).plugin

# Plugin UI file
filetype_flac_uidir = $(gtkpod_ui_dir)
filetype_flac_ui_DATA =

# Plugin Glade file
filetype_flac_gladedir = $(gtkpod_glade_dir)
filetype_flac_glade_DATA =

# Plugin Icon file
filetype_flac_pixmapsdir = $(gtkpod_image_dir)
filetype_flac_pixmaps_DATA =

# Where to install the plugin
filetype_flac_plugindir = $(gtkpod_plugin_dir)
filetype_flac_plugin_DATA =

SUBDIRS =

if HAVE_PLUGIN_FILETYPE_FLAC

include ../plugins.mk
filetype_flac.plugin: build-plugin-file

# The plugin
plugin_lib = lib$(plugin_name).so
plugin_LTLIBRARIES = libfiletype_flac.la

# Plugin sources
libfiletype_flac_la_SOURCES = plugin.c plugin.h \
							  flacfile.c flacfile.h
libfiletype_flac_la_CFLAGS = $(FLAC_CFLAGS)
libfiletype_flac_la_LDFLAGS = $(GTKPOD_PLUGIN_LDFLAGS)

# Plugin dependencies
libfiletype_flac_la_LIBADD = \
    $(GTKPOD_LIBS) \
    $(LIBANJUTA_LIBS) \
    $(FLAC_LIBS)

endif

EXTRA_DIST = \
	$(plugin_file).in \
	$(filetype_flac_plugin_DATA) \
	$(filetype_flac_ui_DATA) \
	$(filetype_flac_glade_DATA) \
	$(filetype_flac_pixmaps_DATA)
