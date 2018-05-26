/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; c-indent-level: 8 -*- */
/*
 * Copyright (C) 2005, Nickolay V. Shmyrev <nshmyrev@yandex.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "config.h"

#include <config.h>
#include "djvu-document.h"
#include "djvu-text-page.h"
#include "djvu-links.h"
#include "djvu-document-private.h"
#include "ev-document-thumbnails.h"
#include "ev-file-exporter.h"
#include "ev-document-misc.h"
#include "ev-document-find.h"
#include "ev-document-links.h"
#include "ev-document-annotations.h"
#include "ev-selection.h"
#include "ev-file-helpers.h"

#include <glib.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glib/gi18n-lib.h>
#include <string.h>

#define ANNOT_POPUP_WINDOW_DEFAULT_WIDTH  200
#define ANNOT_POPUP_WINDOW_DEFAULT_HEIGHT 150
#define ANNOTATION_ICON_SIZE 24

enum {
	PROP_0,
	PROP_TITLE
};

struct _DjvuDocumentClass
{
	EvDocumentClass parent_class;
};

typedef struct _DjvuDocumentClass DjvuDocumentClass;

static void djvu_document_document_thumbnails_iface_init (EvDocumentThumbnailsInterface *iface);
static void djvu_document_file_exporter_iface_init (EvFileExporterInterface *iface);
static void djvu_document_find_iface_init (EvDocumentFindInterface *iface);
static void djvu_document_document_links_iface_init  (EvDocumentLinksInterface *iface);
static void djvu_document_document_annotations_iface_init (EvDocumentAnnotationsInterface *iface);
static void djvu_selection_iface_init (EvSelectionInterface *iface);

static void djvu_document_annotations_load_annotations (DjvuDocument *djvu_document);
static void djvu_document_annotations_save_annotations (DjvuDocument *djvu_document);

EV_BACKEND_REGISTER_WITH_CODE (DjvuDocument, djvu_document,
    {
      EV_BACKEND_IMPLEMENT_INTERFACE (EV_TYPE_DOCUMENT_THUMBNAILS, djvu_document_document_thumbnails_iface_init);
      EV_BACKEND_IMPLEMENT_INTERFACE (EV_TYPE_FILE_EXPORTER, djvu_document_file_exporter_iface_init);
      EV_BACKEND_IMPLEMENT_INTERFACE (EV_TYPE_DOCUMENT_FIND, djvu_document_find_iface_init);
      EV_BACKEND_IMPLEMENT_INTERFACE (EV_TYPE_DOCUMENT_LINKS, djvu_document_document_links_iface_init);
      EV_BACKEND_IMPLEMENT_INTERFACE (EV_TYPE_DOCUMENT_ANNOTATIONS, djvu_document_document_annotations_iface_init);
      EV_BACKEND_IMPLEMENT_INTERFACE (EV_TYPE_SELECTION, djvu_selection_iface_init);
     });


#define EV_DJVU_ERROR ev_djvu_error_quark ()

static GQuark
ev_djvu_error_quark (void)
{
	static GQuark q = 0;
	if (q == 0)
		q = g_quark_from_string ("ev-djvu-quark");

	return q;
}

static void
handle_message (const ddjvu_message_t *msg, GError **error)
{
	switch (msg->m_any.tag) {
	        case DDJVU_ERROR: {
			gchar *error_str;

			if (msg->m_error.filename) {
				error_str = g_strdup_printf ("DjvuLibre error: %s:%d",
							     msg->m_error.filename,
							     msg->m_error.lineno);
			} else {
				error_str = g_strdup_printf ("DjvuLibre error: %s",
							     msg->m_error.message);
			}

			if (error) {
				g_set_error_literal (error, EV_DJVU_ERROR, 0, error_str);
			} else {
				g_warning ("%s", error_str);
			}

			g_free (error_str);
			return;
			}
			break;
	        default:
			break;
	}
}

void
djvu_handle_events (DjvuDocument *djvu_document, int wait, GError **error)
{
	ddjvu_context_t *ctx = djvu_document->d_context;
	const ddjvu_message_t *msg;

	if (!ctx)
		return;

	if (wait)
		ddjvu_message_wait (ctx);

	while ((msg = ddjvu_message_peek (ctx))) {
		handle_message (msg, error);
		ddjvu_message_pop (ctx);
		if (error && *error)
			return;
	}
}

static void
djvu_wait_for_message (DjvuDocument *djvu_document, ddjvu_message_tag_t message, GError **error)
{
	ddjvu_context_t *ctx = djvu_document->d_context;
	const ddjvu_message_t *msg;

	ddjvu_message_wait (ctx);
	while ((msg = ddjvu_message_peek (ctx)) && (msg->m_any.tag != message)) {
		handle_message (msg, error);
		ddjvu_message_pop (ctx);
		if (error && *error)
			return;
	}
	if (msg && msg->m_any.tag == message)
		ddjvu_message_pop (ctx);
}

static gboolean
djvu_document_load (EvDocument  *document,
		    const char  *uri,
		    GError     **error)
{
	DjvuDocument *djvu_document = DJVU_DOCUMENT (document);
	ddjvu_document_t *doc;
	gchar *filename;
	gboolean missing_files = FALSE;
	GError *djvu_error = NULL;

	/* FIXME: We could actually load uris  */
	filename = g_filename_from_uri (uri, NULL, error);
	if (!filename)
		return FALSE;

	doc = ddjvu_document_create_by_filename (djvu_document->d_context, filename, TRUE);

	if (!doc) {
		g_free (filename);
    		g_set_error_literal (error,
				     EV_DOCUMENT_ERROR,
				     EV_DOCUMENT_ERROR_INVALID,
				     _("DjVu document has incorrect format"));
		return FALSE;
	}

	if (djvu_document->d_document)
	    ddjvu_document_release (djvu_document->d_document);

	djvu_document->d_document = doc;

	djvu_wait_for_message (djvu_document, DDJVU_DOCINFO, &djvu_error);
	if (djvu_error) {
		g_set_error_literal (error,
				     EV_DOCUMENT_ERROR,
				     EV_DOCUMENT_ERROR_INVALID,
				     djvu_error->message);
		g_error_free (djvu_error);
		g_free (filename);
		ddjvu_document_release (djvu_document->d_document);
		djvu_document->d_document = NULL;

		return FALSE;
	}

	if (ddjvu_document_decoding_error (djvu_document->d_document))
		djvu_handle_events (djvu_document, TRUE, &djvu_error);

	if (djvu_error) {
		g_set_error_literal (error,
				     EV_DOCUMENT_ERROR,
				     EV_DOCUMENT_ERROR_INVALID,
				     djvu_error->message);
		g_error_free (djvu_error);
		g_free (filename);
		ddjvu_document_release (djvu_document->d_document);
		djvu_document->d_document = NULL;

		return FALSE;
	}

	g_free (djvu_document->uri);
	djvu_document->uri = g_strdup (uri);

	if (ddjvu_document_get_type (djvu_document->d_document) == DDJVU_DOCTYPE_INDIRECT) {
		gint n_files;
		gint i;
		gchar *base;

		base = g_path_get_dirname (filename);

		n_files = ddjvu_document_get_filenum (djvu_document->d_document);
		for (i = 0; i < n_files; i++) {
			struct ddjvu_fileinfo_s fileinfo;
			gchar *file;

			ddjvu_document_get_fileinfo (djvu_document->d_document,
						     i, &fileinfo);

			if (fileinfo.type != 'P')
				continue;

			file = g_build_filename (base, fileinfo.id, NULL);
			if (!g_file_test (file, G_FILE_TEST_EXISTS)) {
				missing_files = TRUE;
				g_free (file);

				break;
			}
			g_free (file);
		}
		g_free (base);
	}
	g_free (filename);

	if (missing_files) {
		g_set_error_literal (error,
                                     G_FILE_ERROR,
                                     G_FILE_ERROR_EXIST,
				     _("The document is composed of several files. "
                                       "One or more of these files cannot be accessed."));

		return FALSE;
	}

	return TRUE;
}


static gboolean
djvu_document_save (EvDocument  *document,
		    const char  *uri,
		    GError     **error)
{
	DjvuDocument *djvu_document = DJVU_DOCUMENT (document);

	if (g_file_test ("/usr/bin/djvused", G_FILE_TEST_EXISTS))
		djvu_document_annotations_save_annotations (DJVU_DOCUMENT (document));

	return ev_xfer_uri_simple (djvu_document->uri, uri, error);
}

int
djvu_document_get_n_pages (EvDocument  *document)
{
	DjvuDocument *djvu_document = DJVU_DOCUMENT (document);

	g_return_val_if_fail (djvu_document->d_document, 0);

	return ddjvu_document_get_pagenum (djvu_document->d_document);
}

static void
document_get_page_size (DjvuDocument *djvu_document,
			gint          page,
			double       *width,
			double       *height,
			double	     *dpi)
{
	ddjvu_pageinfo_t info;
	ddjvu_status_t r;

	while ((r = ddjvu_document_get_pageinfo(djvu_document->d_document, page, &info)) < DDJVU_JOB_OK)
		djvu_handle_events(djvu_document, TRUE, NULL);

	if (r >= DDJVU_JOB_FAILED)
		djvu_handle_events(djvu_document, TRUE, NULL);

	if (width)
		*width = info.width * 72.0 / info.dpi;
	if (height)
		*height = info.height * 72.0 / info.dpi;
	if (dpi)
		*dpi = info.dpi;
}

static void
djvu_document_get_page_size (EvDocument   *document,
			     EvPage       *page,
			     double       *width,
			     double       *height)
{
	DjvuDocument *djvu_document = DJVU_DOCUMENT (document);

	g_return_if_fail (djvu_document->d_document);

	document_get_page_size (djvu_document, page->index,
				width, height, NULL);
}

static cairo_surface_t *
djvu_document_render (EvDocument      *document,
		      EvRenderContext *rc)
{
	DjvuDocument *djvu_document = DJVU_DOCUMENT (document);
	cairo_surface_t *surface;
	gchar *pixels;
	gint   rowstride;
    	ddjvu_rect_t rrect;
	ddjvu_rect_t prect;
	ddjvu_page_t *d_page;
	ddjvu_page_rotation_t rotation;
	gint buffer_modified;
	double page_width, page_height, tmp;

	d_page = ddjvu_page_create_by_pageno (djvu_document->d_document, rc->page->index);

	while (!ddjvu_page_decoding_done (d_page))
		djvu_handle_events(djvu_document, TRUE, NULL);

	document_get_page_size (djvu_document, rc->page->index, &page_width, &page_height, NULL);

	page_width = page_width * rc->scale + 0.5;
	page_height = page_height * rc->scale + 0.5;

	switch (rc->rotation) {
	        case 90:
			rotation = DDJVU_ROTATE_90;
			tmp = page_height;
			page_height = page_width;
			page_width = tmp;

			break;
	        case 180:
			rotation = DDJVU_ROTATE_180;

			break;
	        case 270:
			rotation = DDJVU_ROTATE_270;
			tmp = page_height;
			page_height = page_width;
			page_width = tmp;

			break;
	        default:
			rotation = DDJVU_ROTATE_0;
	}

	surface = cairo_image_surface_create (CAIRO_FORMAT_RGB24,
					      page_width, page_height);

	rowstride = cairo_image_surface_get_stride (surface);
	pixels = (gchar *)cairo_image_surface_get_data (surface);

	prect.x = 0;
	prect.y = 0;
	prect.w = page_width;
	prect.h = page_height;
	rrect = prect;

	ddjvu_page_set_rotation (d_page, rotation);

	buffer_modified = ddjvu_page_render (d_page, DDJVU_RENDER_COLOR,
					     &prect,
					     &rrect,
					     djvu_document->d_format,
					     rowstride,
					     pixels);

	if (!buffer_modified) {
		cairo_t *cr = cairo_create (surface);

		cairo_set_source_rgb (cr, 1.0, 1.0, 1.0);
		cairo_paint (cr);
		cairo_destroy (cr);
	} else {
		cairo_surface_mark_dirty (surface);
	}

	/* Show annotation text */
	EvMappingList *annots;
	GList         *l;
	GdkPixbuf     *pixbuf = NULL;

	annots = (EvMappingList *)g_hash_table_lookup (djvu_document->annots,
								     GINT_TO_POINTER (rc->page->index));
	for (l = ev_mapping_list_get_list (annots); l && l->data; l = g_list_next (l)) {

		cairo_t       *cr;
		EvRectangle   rect;
		GdkColor      color;
		EvAnnotation *annot;

		annot = ((EvMapping *)(l->data))->data;

		if (EV_IS_ANNOTATION_MARKUP (annot)) {
			ev_annotation_get_area (annot, &rect);
			ev_annotation_get_color (annot, &color);

			cr = cairo_create (surface);
			rect.x1 = (int)(rect.x1*rc->scale);
			rect.y1 = (int)(rect.y1*rc->scale);
			rect.x2 = (int)(rect.x2*rc->scale);
			rect.y2 = (int)(rect.y2*rc->scale);

			if (EV_IS_ANNOTATION_TEXT (annot))
				gdk_cairo_set_source_color (cr, &color);
			else
				cairo_set_source_rgba (cr, color.red, color.green, color.blue, 0.7);

			cairo_rectangle(cr, rect.x1, rect.y1, rect.x2-rect.x1, rect.y2-rect.y1);
			cairo_fill (cr);
		}

		if (EV_IS_ANNOTATION_TEXT (annot)) {
			if (!pixbuf) {
				GtkIconTheme  *icon_theme;
				GdkScreen     *screen;

				screen = gdk_screen_get_default ();
				icon_theme = gtk_icon_theme_get_for_screen (screen);

				pixbuf = gtk_icon_theme_load_icon (icon_theme,
								   "document-new",
								   (int)(ANNOTATION_ICON_SIZE * rc->scale),
								   GTK_ICON_LOOKUP_FORCE_SIZE,
								   NULL);
			}

			gdk_cairo_set_source_pixbuf(cr, pixbuf, rect.x1, rect.y1);
			cairo_paint (cr);
			cairo_destroy (cr);
		}
	}


	return surface;
}

static void
djvu_document_finalize (GObject *object)
{
	DjvuDocument *djvu_document = DJVU_DOCUMENT (object);

	if (djvu_document->d_document)
	    ddjvu_document_release (djvu_document->d_document);

	if (djvu_document->opts)
	    g_string_free (djvu_document->opts, TRUE);

	if (djvu_document->ps_filename)
	    g_free (djvu_document->ps_filename);

	ddjvu_context_release (djvu_document->d_context);
	ddjvu_format_release (djvu_document->d_format);
	ddjvu_format_release (djvu_document->thumbs_format);
	g_free (djvu_document->uri);

	G_OBJECT_CLASS (djvu_document_parent_class)->finalize (object);
}

static void
djvu_document_dispose (GObject *object)
{
	DjvuDocument *djvu_document = DJVU_DOCUMENT(object);


	if (djvu_document->annots) {
		g_hash_table_destroy (djvu_document->annots);
		djvu_document->annots = NULL;
	}

	G_OBJECT_CLASS (djvu_document_parent_class)->dispose (object);
}

static void
djvu_document_class_init (DjvuDocumentClass *klass)
{
	GObjectClass    *gobject_class = G_OBJECT_CLASS (klass);
	EvDocumentClass *ev_document_class = EV_DOCUMENT_CLASS (klass);

	gobject_class->dispose = djvu_document_dispose;
	gobject_class->finalize = djvu_document_finalize;

	ev_document_class->load = djvu_document_load;
	ev_document_class->save = djvu_document_save;
	ev_document_class->get_n_pages = djvu_document_get_n_pages;
	ev_document_class->get_page_size = djvu_document_get_page_size;
	ev_document_class->render = djvu_document_render;
}

static gchar *
djvu_text_copy (DjvuDocument *djvu_document,
		gint           page,
		EvRectangle  *rectangle)
{
	miniexp_t page_text;
	gchar    *text = NULL;

	while ((page_text =
		ddjvu_document_get_pagetext (djvu_document->d_document,
					     page, "char")) == miniexp_dummy)
		djvu_handle_events (djvu_document, TRUE, NULL);

	if (page_text != miniexp_nil) {
		DjvuTextPage *page = djvu_text_page_new (page_text);

		text = djvu_text_page_copy (page, rectangle);
		djvu_text_page_free (page);
		ddjvu_miniexp_release (djvu_document->d_document, page_text);
	}

	return text;
}

static void
djvu_convert_to_doc_rect (EvRectangle *dest,
			  EvRectangle *source,
			  gdouble height,
			  gdouble dpi)
{
	dest->x1 = source->x1 * dpi / 72;
	dest->x2 = source->x2 * dpi / 72;
	dest->y1 = (height - source->y2) * dpi / 72;
	dest->y2 = (height - source->y1) * dpi / 72;
}

static GList *
djvu_selection_get_selection_rects (DjvuDocument    *djvu_document,
				    gint             page,
				    EvRectangle     *points,
				    gdouble          height,
				    gdouble          dpi)
{
	miniexp_t   page_text;
	EvRectangle rectangle;
	GList      *rects = NULL;

	djvu_convert_to_doc_rect (&rectangle, points, height, dpi);

	while ((page_text = ddjvu_document_get_pagetext (djvu_document->d_document,
							 page, "char")) == miniexp_dummy)
		djvu_handle_events (djvu_document, TRUE, NULL);

	if (page_text != miniexp_nil) {
		DjvuTextPage *tpage = djvu_text_page_new (page_text);

		rects = djvu_text_page_get_selection_region (tpage, &rectangle);
		djvu_text_page_free (tpage);
		ddjvu_miniexp_release (djvu_document->d_document, page_text);
	}

	return rects;
}

static cairo_region_t *
djvu_get_selection_region (DjvuDocument *djvu_document,
                           gint page,
                           gdouble scale,
                           EvRectangle *points)
{
	double          height, dpi;
	GList          *rects = NULL, *l;
	cairo_region_t *region;

	document_get_page_size (djvu_document, page, NULL, &height, &dpi);
	rects = djvu_selection_get_selection_rects (djvu_document, page, points,
						    height, dpi);
	region = cairo_region_create ();
	for (l = rects; l && l->data; l = g_list_next (l)) {
		cairo_rectangle_int_t rect;
		EvRectangle          *r = (EvRectangle *)l->data;
		gdouble               tmp;

		tmp = r->y1;
		r->x1 *= 72 / dpi;
		r->x2 *= 72 / dpi;
		r->y1 = height - r->y2 * 72 / dpi;
		r->y2 = height - tmp * 72 / dpi;

		rect.x = (gint) ((r->x1 * scale) + 0.5);
		rect.y = (gint) ((r->y1 * scale) + 0.5);
		rect.width = (gint) (((r->x2 - r->x1) * scale) + 0.5);
		rect.height = (gint) (((r->y2 - r->y1) * scale) + 0.5);
		cairo_region_union_rectangle (region, &rect);
		ev_rectangle_free (r);
	}
	g_list_free (l);

	return region;
}

static cairo_region_t *
djvu_selection_get_selection_region (EvSelection    *selection,
				     EvRenderContext *rc,
				     EvSelectionStyle style,
				     EvRectangle     *points)
{
	DjvuDocument *djvu_document = DJVU_DOCUMENT (selection);

	return djvu_get_selection_region (djvu_document, rc->page->index,
					  rc->scale, points);
}

static gchar *
djvu_selection_get_selected_text (EvSelection     *selection,
				  EvPage          *page,
				  EvSelectionStyle style,
				  EvRectangle     *points)
{
      	DjvuDocument *djvu_document = DJVU_DOCUMENT (selection);
	double height, dpi;
      	EvRectangle rectangle;
      	gchar *text;

	document_get_page_size (djvu_document, page->index, NULL, &height, &dpi);
	djvu_convert_to_doc_rect (&rectangle, points, height, dpi);
      	text = djvu_text_copy (djvu_document, page->index, &rectangle);

      	if (text == NULL)
		text = g_strdup ("");

    	return text;
}

static void
djvu_selection_iface_init (EvSelectionInterface *iface)
{
	iface->get_selected_text = djvu_selection_get_selected_text;
	iface->get_selection_region = djvu_selection_get_selection_region;
}

static void
djvu_document_thumbnails_get_dimensions (EvDocumentThumbnails *document,
					 EvRenderContext      *rc,
					 gint                 *width,
					 gint                 *height)
{
	DjvuDocument *djvu_document = DJVU_DOCUMENT (document);
	gdouble page_width, page_height;

	djvu_document_get_page_size (EV_DOCUMENT(djvu_document), rc->page,
				     &page_width, &page_height);

	if (rc->rotation == 90 || rc->rotation == 270) {
		*width = (gint) (page_height * rc->scale);
		*height = (gint) (page_width * rc->scale);
	} else {
		*width = (gint) (page_width * rc->scale);
		*height = (gint) (page_height * rc->scale);
	}
}

static GdkPixbuf *
djvu_document_thumbnails_get_thumbnail (EvDocumentThumbnails *document,
					EvRenderContext      *rc,
					gboolean 	      border)
{
	DjvuDocument *djvu_document = DJVU_DOCUMENT (document);
	GdkPixbuf *pixbuf, *rotated_pixbuf;
	gdouble page_width, page_height;
	gint thumb_width, thumb_height;
	guchar *pixels;

	g_return_val_if_fail (djvu_document->d_document, NULL);

	djvu_document_get_page_size (EV_DOCUMENT(djvu_document), rc->page,
				     &page_width, &page_height);

	thumb_width = (gint) (page_width * rc->scale);
	thumb_height = (gint) (page_height * rc->scale);

	pixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB, FALSE, 8,
				 thumb_width, thumb_height);
	gdk_pixbuf_fill (pixbuf, 0xffffffff);
	pixels = gdk_pixbuf_get_pixels (pixbuf);

	while (ddjvu_thumbnail_status (djvu_document->d_document, rc->page->index, 1) < DDJVU_JOB_OK)
		djvu_handle_events(djvu_document, TRUE, NULL);

	ddjvu_thumbnail_render (djvu_document->d_document, rc->page->index,
				&thumb_width, &thumb_height,
				djvu_document->thumbs_format,
				gdk_pixbuf_get_rowstride (pixbuf),
				(gchar *)pixels);

	rotated_pixbuf = gdk_pixbuf_rotate_simple (pixbuf, 360 - rc->rotation);
	g_object_unref (pixbuf);

        if (border) {
	      GdkPixbuf *tmp_pixbuf = rotated_pixbuf;

	      rotated_pixbuf = ev_document_misc_get_thumbnail_frame (-1, -1, tmp_pixbuf);
	      g_object_unref (tmp_pixbuf);
	}

	return rotated_pixbuf;
}

static void
djvu_document_document_thumbnails_iface_init (EvDocumentThumbnailsInterface *iface)
{
	iface->get_thumbnail = djvu_document_thumbnails_get_thumbnail;
	iface->get_dimensions = djvu_document_thumbnails_get_dimensions;
}

/* EvFileExporterIface */
static void
djvu_document_file_exporter_begin (EvFileExporter        *exporter,
				   EvFileExporterContext *fc)
{
	DjvuDocument *djvu_document = DJVU_DOCUMENT (exporter);

	if (djvu_document->ps_filename)
		g_free (djvu_document->ps_filename);
	djvu_document->ps_filename = g_strdup (fc->filename);

	g_string_assign (djvu_document->opts, "-page=");
}

static void
djvu_document_file_exporter_do_page (EvFileExporter  *exporter,
				     EvRenderContext *rc)
{
	DjvuDocument *djvu_document = DJVU_DOCUMENT (exporter);

	g_string_append_printf (djvu_document->opts, "%d,", (rc->page->index) + 1);
}

static void
djvu_document_file_exporter_end (EvFileExporter *exporter)
{
	int d_optc = 1;
	const char *d_optv[d_optc];

	DjvuDocument *djvu_document = DJVU_DOCUMENT (exporter);

	FILE *fn = fopen (djvu_document->ps_filename, "w");
	if (fn == NULL) {
		g_warning ("Cannot open file “%s”.", djvu_document->ps_filename);
		return;
	}

	d_optv[0] = djvu_document->opts->str;

	ddjvu_job_t * job = ddjvu_document_print(djvu_document->d_document, fn, d_optc, d_optv);
	while (!ddjvu_job_done(job)) {
		djvu_handle_events (djvu_document, TRUE, NULL);
	}

	fclose(fn);
}

static EvFileExporterCapabilities
djvu_document_file_exporter_get_capabilities (EvFileExporter *exporter)
{
	return  EV_FILE_EXPORTER_CAN_PAGE_SET |
		EV_FILE_EXPORTER_CAN_COPIES |
		EV_FILE_EXPORTER_CAN_COLLATE |
		EV_FILE_EXPORTER_CAN_REVERSE |
		EV_FILE_EXPORTER_CAN_GENERATE_PS;
}

static void
djvu_document_file_exporter_iface_init (EvFileExporterInterface *iface)
{
	iface->begin = djvu_document_file_exporter_begin;
	iface->do_page = djvu_document_file_exporter_do_page;
	iface->end = djvu_document_file_exporter_end;
	iface->get_capabilities = djvu_document_file_exporter_get_capabilities;
}

static void
djvu_document_init (DjvuDocument *djvu_document)
{
	guint masks[4] = { 0xff0000, 0xff00, 0xff, 0xff000000 };

	djvu_document->d_context = ddjvu_context_create ("Xreader");
	djvu_document->d_format = ddjvu_format_create (DDJVU_FORMAT_RGBMASK32, 4, masks);
	ddjvu_format_set_row_order (djvu_document->d_format, 1);

	djvu_document->thumbs_format = ddjvu_format_create (DDJVU_FORMAT_RGB24, 0, 0);
	ddjvu_format_set_row_order (djvu_document->thumbs_format, 1);

	djvu_document->ps_filename = NULL;
	djvu_document->opts = g_string_new ("");

	djvu_document->d_document = NULL;
}

static GList *
djvu_document_find_find_text (EvDocumentFind   *document,
			      EvPage           *page,
			      const char       *text,
			      gboolean          case_sensitive)
{
        DjvuDocument *djvu_document = DJVU_DOCUMENT (document);
	miniexp_t page_text;
	gdouble width, height, dpi;
	GList *matches = NULL, *l;

	g_return_val_if_fail (text != NULL, NULL);

	while ((page_text = ddjvu_document_get_pagetext (djvu_document->d_document,
							 page->index,
							 "char")) == miniexp_dummy)
		djvu_handle_events (djvu_document, TRUE, NULL);

	if (page_text != miniexp_nil) {
		DjvuTextPage *tpage = djvu_text_page_new (page_text);

		djvu_text_page_prepare_search (tpage, case_sensitive);
		if (tpage->links->len > 0) {
			djvu_text_page_search (tpage, text, case_sensitive);
			matches = tpage->results;
		}
		djvu_text_page_free (tpage);
		ddjvu_miniexp_release (djvu_document->d_document, page_text);
	}

	if (!matches)
		return NULL;

	document_get_page_size (djvu_document, page->index, &width, &height, &dpi);
	for (l = matches; l && l->data; l = g_list_next (l)) {
		EvRectangle *r = (EvRectangle *)l->data;
		gdouble tmp = r->y1;

		r->x1 *= 72.0 / dpi;
		r->x2 *= 72.0 / dpi;

		r->y1 = height - r->y2 * 72.0 / dpi;
		r->y2 = height - tmp * 72.0 / dpi;
	}


	return matches;
}

static void
djvu_document_find_iface_init (EvDocumentFindInterface *iface)
{
        iface->find_text = djvu_document_find_find_text;
}

static EvMappingList *
djvu_document_links_get_links (EvDocumentLinks *document_links,
			       EvPage          *page)
{
	gdouble dpi;

	document_get_page_size (DJVU_DOCUMENT (document_links), page->index, NULL, NULL, &dpi);
	return djvu_links_get_links (document_links, page->index, 72.0 / dpi);
}

static void
djvu_document_document_links_iface_init  (EvDocumentLinksInterface *iface)
{
	iface->has_document_links = djvu_links_has_document_links;
	iface->get_links_model = djvu_links_get_links_model;
	iface->get_links = djvu_document_links_get_links;
	iface->find_link_dest = djvu_links_find_link_dest;
	iface->find_link_page = djvu_links_find_link_page;
}

static void
annot_set_unique_name (EvAnnotation *annot)
{
	gchar *name;

	name = g_strdup_printf ("annot-%" G_GUINT64_FORMAT, g_get_real_time ());
	ev_annotation_set_name (annot, name);
	g_free (name);
}

static void
annot_area_changed_cb (EvAnnotation *annot,
                       GParamSpec   *spec,
                       EvMapping    *mapping)
{
	ev_annotation_get_area (annot, &mapping->area);
}

/* This function assumes that djvused exists */
static void
djvu_document_annotations_save_annotations (DjvuDocument *djvu_document)
{
	if (!djvu_document->annots)
		return;

	GHashTableIter iter;
	EvMappingList *mapping_list;
	gpointer       key, value;
	double         width, height, dpi;
	gchar         *command, *filename, *antfile, *content;

	/* Generate the djvused annotation file */
	content = g_strdup ("select; remove-ant\n");

	g_hash_table_iter_init (&iter, djvu_document->annots);
	while (g_hash_table_iter_next (&iter, &key, &value))
	{
		struct ddjvu_fileinfo_s fileinfo;
		EvMappingList *mapping_list;
		gint	       page_num;
		EvPage        *page;
		GList         *l;
		gchar         *t;

		page_num = (gint) key;
		mapping_list = (EvMappingList *) value;
		page = ev_document_get_page (EV_DOCUMENT (djvu_document), page_num-1);

		ddjvu_document_get_fileinfo (djvu_document->d_document,
					     page_num+1, &fileinfo);
		t = content;
		content = g_strdup_printf ("%sselect  \"%s\"\nset-ant\n", content, fileinfo.title);
		g_free (t);

		for (l = ev_mapping_list_get_list (mapping_list); l && l->data; l = g_list_next (l)) {

			EvAnnotation *annot;
			EvRectangle   rect;
			GdkColor      color;
			gchar        *content_annot;
			gdouble       height, dpi;

			annot = ((EvMapping *)(l->data))->data;
			ev_annotation_get_area (annot, &rect);
			ev_annotation_get_color (annot, &color);

			document_get_page_size (djvu_document, page_num, NULL, &height, &dpi);
			djvu_convert_to_doc_rect (&rect, &rect, height, dpi);

			if (EV_IS_ANNOTATION_TEXT (annot)) {
				t = content;
				content_annot = g_strescape (ev_annotation_get_contents (annot), NULL);
				content = g_strdup_printf ("%s(maparea \"\" \"%s\" (text %d %d %d %d) (pushpin) (backclr %s) )\n",
							   content,
							   content_annot,
							   (gint) rect.x1, (gint) rect.y1,
							   (gint) (100*dpi/72),
							   (gint) (50*dpi/72),
							   gdk_color_to_string (&color));
				g_free (content_annot);
				g_free (t);
			}
		}

		t = content;
		content = g_strdup_printf ("%s\n.\n", content);
		g_free (t);
	}


	antfile = g_strdup_printf ("/tmp/xreader_%" G_GUINT64_FORMAT, g_get_real_time ());
	if (!g_file_set_contents (antfile, content, -1, NULL)) {
		g_free (antfile);
		return;
	}

	filename = g_filename_from_uri (djvu_document->uri, NULL, NULL);
	command = g_strdup_printf ("djvused %s -f %s -s", filename, antfile);

	g_spawn_command_line_sync (command, NULL, NULL, NULL, NULL);
	g_remove (antfile);

	g_free (filename);
	g_free (antfile);
	g_free (command);
	djvu_document->annots_modified = FALSE;
}

/* This function assumes that djvused exists */
static void
djvu_document_annotations_load_annotations (DjvuDocument *djvu_document)
{
	if (djvu_document->annots)
		g_hash_table_destroy (djvu_document->annots);

	djvu_document->annots = g_hash_table_new_full (g_direct_hash,
						       g_direct_equal,
						       (GDestroyNotify)NULL,
						       (GDestroyNotify)ev_mapping_list_unref);

	EvMappingList *mapping_list;
	double width, height, dpi;
	gchar *command, *filename, *p_stdout, *p_stderr;
	gchar **lines, **line;
	GList *retval = NULL;
	int new_page, current_page = -1;

	/* Generate the djvused annotation file */
	filename = g_filename_from_uri (djvu_document->uri, NULL, NULL);
	command = g_strdup_printf ("djvused %s -e 'output-ant'", filename);
	g_free (filename);

	g_spawn_command_line_sync (command, &p_stdout, &p_stderr, NULL, NULL);
	g_free (command);

	lines = g_strsplit (p_stdout, "\n", -1);
	for (line = lines; *line; line++) {
		EvMapping    *annot_mapping = NULL;
		EvAnnotation *ev_annot = NULL;
		char         text[1000] = "", shape[10] = "";
		char         annot1[20] = "", annot2[20] = "", annot3[20] = "";
		double       x, y;

		/* Changing page of djvused file */
		if (sscanf (*line, "select \"%*[^\"]\" # page %d", &new_page) == 1) {

			if (retval) {
				mapping_list = ev_mapping_list_new (current_page-1,
								    g_list_reverse (retval),
								    (GDestroyNotify)g_object_unref);
				g_hash_table_insert (djvu_document->annots,
						     GINT_TO_POINTER (current_page-1),
						     ev_mapping_list_ref (mapping_list));

				retval = NULL;
			}

			current_page = new_page;
			continue;
		}

		if (current_page == -1)
			continue;

		if (sscanf(*line, "(maparea \"\" \"%[^\"]\" (%s %lf %lf %*lf %*lf) (%[^\\)]) (%[^\\)]) (%[^\\)]) )",
					text, shape, &x, &y, annot1, annot2, annot3) < 4)
			continue;

		gboolean is_annot_text = (g_strcmp0 (shape, "text") == 0
				      && (g_strcmp0 (annot1, "pushpin") == 0
				       || g_strcmp0 (annot2, "pushpin") == 0
				       || g_strcmp0 (annot3, "pushpin") == 0));
		if (is_annot_text) {
			document_get_page_size (djvu_document, current_page, &width, &height, &dpi);

			EvPage *page = ev_document_get_page (EV_DOCUMENT (djvu_document), current_page-1);

			ev_annot = ev_annotation_text_new (page);
			ev_annotation_text_set_is_open (EV_ANNOTATION_TEXT (ev_annot), FALSE);

			annot_mapping = g_new (EvMapping, 1);
			annot_mapping->area.x1 = x * 72.0 / dpi;
			annot_mapping->area.x2 = annot_mapping->area.x1 + ANNOTATION_ICON_SIZE;
			annot_mapping->area.y2 = height - (y * 72.0 / dpi);
			annot_mapping->area.y1 = MAX (0, annot_mapping->area.y2 - ANNOTATION_ICON_SIZE);

			/* Setting the color to annotation */
			char color_str[8] = "";
			sscanf (annot1, "backclr %s", color_str);
			sscanf (annot2, "backclr %s", color_str);
			sscanf (annot3, "backclr %s", color_str);

			GdkColor color = { 0, 65535, 65535, 0 };
			gdk_color_parse (color_str, &color);
			ev_annotation_set_color (ev_annot, &color);
		}

		if (EV_IS_ANNOTATION_MARKUP (ev_annot)) {

			EvRectangle popup_rect;

			popup_rect.x1 = annot_mapping->area.x2;
			popup_rect.x2 = popup_rect.x1 + ANNOT_POPUP_WINDOW_DEFAULT_WIDTH;
			popup_rect.y1 = annot_mapping->area.y2;
			popup_rect.y2 = popup_rect.y1 + ANNOT_POPUP_WINDOW_DEFAULT_HEIGHT;
			g_object_set (ev_annot,
					  "rectangle", &popup_rect,
					  "has_popup", TRUE,
					  "popup_is_open", FALSE,
					  "label", g_get_real_name (),
					  "opacity", 1.0,
					  NULL);
		}

		if (annot_mapping != NULL) {

			gchar *content_annot;

			content_annot = g_strcompress (text);
			ev_annotation_set_contents (ev_annot, content_annot);

			annot_mapping->data = ev_annot;
			annot_set_unique_name (ev_annot);
			ev_annotation_set_area (ev_annot, &annot_mapping->area);
			g_signal_connect (ev_annot, "notify::area",
					  G_CALLBACK (annot_area_changed_cb),
					  annot_mapping);


			retval = g_list_prepend (retval, annot_mapping);

			g_free (content_annot);
		}
	}

	if (retval) {
		mapping_list = ev_mapping_list_new (current_page-1,
						    g_list_reverse (retval),
						    (GDestroyNotify)g_object_unref);

		g_hash_table_insert (djvu_document->annots,
				     GINT_TO_POINTER (current_page-1),
				     ev_mapping_list_ref (mapping_list));
	}

	g_strfreev (lines);
	g_free (p_stdout);
	g_free (p_stderr);
}


static EvMappingList *
djvu_document_annotations_get_annotations (EvDocumentAnnotations *document_annotations,
                                           EvPage                *page)
{
	g_return_val_if_fail (g_file_test ("/usr/bin/djvused", G_FILE_TEST_EXISTS), NULL);

	DjvuDocument *djvu_document;
	EvMappingList *mapping_list;

	djvu_document = DJVU_DOCUMENT (document_annotations);

	if (!djvu_document->annots)
		djvu_document_annotations_load_annotations (djvu_document);

	mapping_list = (EvMappingList *)g_hash_table_lookup (djvu_document->annots,
							     GINT_TO_POINTER (page->index));

	return mapping_list ? ev_mapping_list_ref (mapping_list) : NULL;
}

static gboolean
djvu_document_annotations_document_is_modified (EvDocumentAnnotations *document_annotations)
{
	return DJVU_DOCUMENT (document_annotations)->annots_modified;
}

static void
djvu_document_annotations_add_annotation (EvDocumentAnnotations *document_annotations,
					  EvAnnotation          *annot,
					  EvRectangle           *rect)
{
	DjvuDocument    *djvu_document;
	EvMappingList   *mapping_list;
	EvMapping       *annot_mapping;
	GList           *list = NULL;
	EvPage          *page;

	djvu_document = DJVU_DOCUMENT (document_annotations);
	page = ev_annotation_get_page (annot);

	annot_mapping = g_new (EvMapping, 1);
	annot_mapping->area = *rect;
	annot_mapping->data = annot;
	g_signal_connect (annot, "notify::area",
			  G_CALLBACK (annot_area_changed_cb),
			  annot_mapping);

	if (!djvu_document->annots)
		djvu_document_annotations_load_annotations (djvu_document);


	mapping_list = (EvMappingList *)g_hash_table_lookup (djvu_document->annots,
							     GINT_TO_POINTER (page->index));

	annot_set_unique_name (annot);

	if (mapping_list) {
		list = ev_mapping_list_get_list (mapping_list);
		list = g_list_append (list, annot_mapping);
	} else {
		list = g_list_append (list, annot_mapping);
		mapping_list = ev_mapping_list_new (page->index, list, (GDestroyNotify)g_object_unref);
		g_hash_table_insert (djvu_document->annots,
				     GINT_TO_POINTER (page->index),
				     ev_mapping_list_ref (mapping_list));
	}

	djvu_document->annots_modified = TRUE;
}

static void
djvu_document_annotations_save_annotation (EvDocumentAnnotations *document_annotations,
					   EvAnnotation          *annot,
					   EvAnnotationsSaveMask  mask)
{
	DJVU_DOCUMENT (document_annotations)->annots_modified = TRUE;
}

static void
djvu_document_annotations_remove_annotation (EvDocumentAnnotations *document_annotations,
                                             EvAnnotation          *annot)
{
	DjvuDocument  *djvu_document;
	EvMappingList *mapping_list;
	EvMapping     *annot_mapping;
	EvPage        *page;

	djvu_document = DJVU_DOCUMENT (document_annotations);
	page = ev_annotation_get_page (annot);

	/* We don't check for pdf_document->annots, if it were NULL then something is really wrong */
	mapping_list = (EvMappingList *)g_hash_table_lookup (djvu_document->annots,
							     GINT_TO_POINTER (page->index));
	if (mapping_list) {
		annot_mapping = ev_mapping_list_find (mapping_list, annot);
		ev_mapping_list_remove (mapping_list, annot_mapping);

		if (ev_mapping_list_length (mapping_list) == 0)
			g_hash_table_remove (djvu_document->annots, GINT_TO_POINTER (page->index));
	}

	djvu_document->annots_modified = TRUE;
}

static EvDocumentAnnotationsAvailabilities
djvu_document_annotations_get_availabilities (EvDocumentAnnotations *document_annots)
{
	EvDocumentAnnotationsAvailabilities av;

	av.text = TRUE;
	av.markup_text = TRUE;
	av.circle = TRUE;
	av.line = FALSE;

	return av;
}

static void
djvu_document_document_annotations_iface_init (EvDocumentAnnotationsInterface *iface)
{
	iface->get_annotations = djvu_document_annotations_get_annotations;
	if (g_file_test ("/usr/bin/djvused", G_FILE_TEST_EXISTS)) {
		iface->document_is_modified = djvu_document_annotations_document_is_modified;
		iface->add_annotation = djvu_document_annotations_add_annotation;
		iface->save_annotation = djvu_document_annotations_save_annotation;
		iface->remove_annotation = djvu_document_annotations_remove_annotation;
		iface->get_availabilities = djvu_document_annotations_get_availabilities;
	}
}

