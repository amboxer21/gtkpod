set(GTKPOD_SOURCES
    src/autodetection.c
    src/charset.c
    src/clientserver.c
    src/confirmation.c
    src/context_menus.c
    src/details.c
    src/display.c
    src/display_coverart.c
    src/display_itdb.c
    src/display_photo.c
    src/display_playlists.c
    src/display_sorttabs.c
    src/display_spl.c
    src/display_tracks.c
    src/fetchcover.c
    src/file.c
    src/file_convert.c
    src/file_export.c
    src/file_itunesdb.c
    src/fileselection.c
    src/flacfile.c
    src/getopt1.c
    src/getopt.c
    src/help.c
    src/info.c
    src/infodlg.c
    src/ipod_init.c
    src/main.c
    src/misc.c
    src/misc_confirm.c
    src/misc_conversion.c
    src/misc_input.c
    src/misc_playlist.c
    src/misc_track.c
    src/mp3file.c
    src/mp4file.c
    src/oggfile.c
    src/podcast.c
    src/prefs.c
    src/prefsdlg.c
    src/rb_cell_renderer_rating.c
    src/rb_rating_helper.c
    src/repository.c
    src/sha1.c
    src/sort_window.c
    src/stock_icons.c
    src/syncdir.c
    src/tools.c
    src/wavfile.c
)

add_flex_files(GTKPOD_SOURCES src/date_parser.l src/date_parser2.l)
