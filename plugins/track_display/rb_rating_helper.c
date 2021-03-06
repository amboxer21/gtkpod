/*
 *
 *  arch-tag: Implementation of functions shared by the rating widget and cell renderer.
 *
 *  Copyright (C) 2004 Christophe Fergeau <teuf@gnome.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA.
 *
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "rb_rating_helper.h"

#define RB_STOCK_SET_STAR	"star-set"
#define RB_STOCK_UNSET_STAR	"star-unset"
#define RB_STOCK_NO_STAR	"star-none"

struct _RBRatingPixbufs {
	GdkPixbuf *pix_star;
	GdkPixbuf *pix_dot;
	GdkPixbuf *pix_blank;
};

void
rb_rating_pixbufs_free (RBRatingPixbufs *pixbufs)
{
	if (pixbufs->pix_star != NULL)
		g_object_unref (pixbufs->pix_star);
	if (pixbufs->pix_dot != NULL)
		g_object_unref (pixbufs->pix_dot);
	if (pixbufs->pix_blank != NULL)
		g_object_unref (pixbufs->pix_blank);
}

void
rb_rating_install_rating_property (GObjectClass *klass, gulong prop)
{
	g_object_class_install_property (klass, prop,
					 g_param_spec_double ("rating",
							     ("Rating Value"),
							     ("Rating Value"),
							     0.0, (double)RB_RATING_MAX_SCORE,
							      (double)RB_RATING_MAX_SCORE/2.0,
							     G_PARAM_READWRITE));

}

RBRatingPixbufs *
rb_rating_pixbufs_new (void)
{
	RBRatingPixbufs *pixbufs;
	GtkIconTheme *theme;
	gint width;

	pixbufs = g_new0 (RBRatingPixbufs, 1);

	theme = gtk_icon_theme_get_default ();
	gtk_icon_size_lookup (GTK_ICON_SIZE_MENU, NULL, &width);

	pixbufs->pix_star = gtk_icon_theme_load_icon (theme,
						      RB_STOCK_SET_STAR,
						      width,
						      0,
						      NULL);
	pixbufs->pix_dot = gtk_icon_theme_load_icon (theme,
						     RB_STOCK_UNSET_STAR,
						     width,
						     0,
						     NULL);
	pixbufs->pix_blank = gtk_icon_theme_load_icon (theme,
						       RB_STOCK_NO_STAR,
						       width,
						       0,
						       NULL);
	if (pixbufs->pix_star != NULL &&
	    pixbufs->pix_dot != NULL &&
	    pixbufs->pix_blank != NULL) {
		return pixbufs;
	}
	else
	{
	    rb_rating_pixbufs_free (pixbufs);
	    g_free (pixbufs);
	    g_warning ("Unable to load at least one of the following icons: " RB_STOCK_SET_STAR ", " RB_STOCK_UNSET_STAR " and " RB_STOCK_NO_STAR ". Displaying of the star rating will not work.\n");
	    return NULL;
	}
}

gboolean
rb_rating_render_stars (GtkWidget *widget,
			cairo_t *cairo_context,
			RBRatingPixbufs *pixbufs,
			gulong x,
			gulong y,
			gulong x_offset,
			gulong y_offset,
			gdouble rating,
			gboolean selected)
{
	int i, icon_width;
	gboolean rtl;

	g_return_val_if_fail (widget != NULL, FALSE);
	g_return_val_if_fail (cairo_context != NULL, FALSE);
	g_return_val_if_fail (pixbufs != NULL, FALSE);

	rtl = (gtk_widget_get_direction (widget) == GTK_TEXT_DIR_RTL);
	gtk_icon_size_lookup (GTK_ICON_SIZE_MENU, &icon_width, NULL);

	for (i = 0; i < RB_RATING_MAX_SCORE; i++) {
		GdkPixbuf *buf;
		GtkStateType state;
		gint star_offset;
		int offset;

		if (selected == TRUE) {
			offset = 0;
			if (gtk_widget_has_focus (widget))
				state = GTK_STATE_SELECTED;
			else
				state = GTK_STATE_ACTIVE;
		} else {
			offset = 120;
			if (!gtk_widget_get_sensitive (widget))
				state = GTK_STATE_INSENSITIVE;
			else
				state = GTK_STATE_NORMAL;
		}

		if (i < rating)
			buf = pixbufs->pix_star;
		else if (i >= rating && i < RB_RATING_MAX_SCORE)
			buf = pixbufs->pix_dot;
		else
			buf = pixbufs->pix_blank;

		if (buf == NULL) {
			return FALSE;
		}

		if (rtl) {
			star_offset = (RB_RATING_MAX_SCORE - i - 1) * icon_width;
		} else {
			star_offset = i * icon_width;
		}

		cairo_save (cairo_context);
		gdk_cairo_set_source_pixbuf (cairo_context, buf, x_offset + star_offset, y_offset);
		cairo_paint (cairo_context);
		cairo_restore (cairo_context);
	}

	return TRUE;
}

double
rb_rating_get_rating_from_widget (GtkWidget *widget,
				  gint widget_x,
				  gint widget_width,
				  double current_rating)
{
	int icon_width;
	double rating = -1.0;

	gtk_icon_size_lookup (GTK_ICON_SIZE_MENU, &icon_width, NULL);

	/* ensure the user clicks within the good cell */
	if (widget_x >= 0 && widget_x <= widget_width) {
		gboolean rtl;

		rating = (int) (widget_x / icon_width) + 1;

		rtl = (gtk_widget_get_direction (widget) == GTK_TEXT_DIR_RTL);
		if (rtl) {
			rating = RB_RATING_MAX_SCORE - rating + 1;
		}

		if (rating < 0)
			rating = 0;

		if (rating > RB_RATING_MAX_SCORE)
			rating = RB_RATING_MAX_SCORE;

		if (rating == current_rating) {
			/* Make it possible to give a 0 rating to a song */
			rating--;
		}
	}

	return rating;
}
