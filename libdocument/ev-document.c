/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; c-indent-level: 8 -*- */
/*
 *  Copyright (C) 2009 Carlos Garcia Campos
 *  Copyright (C) 2004 Marco Pesenti Gritti
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "config.h"

#include <stdlib.h>
#include <string.h>

#include "ev-document.h"
#include "synctex_parser.h"
#include "ev-file-helpers.h"

#define EV_DOCUMENT_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), EV_TYPE_DOCUMENT, EvDocumentPrivate))

typedef struct _EvPageSize
{
	gdouble width;
	gdouble height;
} EvPageSize;

struct _EvDocumentPrivate
{
	gchar          *uri;

	gint            n_pages;

	gboolean        uniform;
	gdouble         uniform_width;
	gdouble         uniform_height;

	gdouble         max_width;
	gdouble         max_height;
	gdouble         min_width;
	gdouble         min_height;
	gint            max_label;

	gchar         **page_labels;
	EvPageSize     *page_sizes;
	EvDocumentInfo *info;

	synctex_scanner_t synctex_scanner;
};

static gint            _ev_document_get_n_pages     (EvDocument *document);
static void            _ev_document_get_page_size   (EvDocument *document,
						     EvPage     *page,
						     double     *width,
						     double     *height);
static gchar          *_ev_document_get_page_label  (EvDocument *document,
						     EvPage     *page);
static EvDocumentInfo *_ev_document_get_info        (EvDocument *document);
static gboolean        _ev_document_support_synctex (EvDocument *document);

static GMutex ev_doc_mutex;
static GMutex ev_fc_mutex;

G_DEFINE_ABSTRACT_TYPE (EvDocument, ev_document, G_TYPE_OBJECT)

GQuark
ev_document_error_quark (void)
{
  static GQuark q = 0;
  if (q == 0)
    q = g_quark_from_static_string ("ev-document-error-quark");

  return q;
}

static EvPage *
ev_document_impl_get_page (EvDocument *document,
			   gint        index)
{
	return ev_page_new (index);
}

static EvDocumentInfo *
ev_document_impl_get_info (EvDocument *document)
{
	return g_new0 (EvDocumentInfo, 1);
}

static void
ev_document_finalize (GObject *object)
{
	EvDocument *document = EV_DOCUMENT (object);

	if (document->priv->uri) {
		g_free (document->priv->uri);
		document->priv->uri = NULL;
	}

	if (document->priv->page_sizes) {
		g_free (document->priv->page_sizes);
		document->priv->page_sizes = NULL;
	}

	if (document->priv->page_labels) {
		gint i;

		for (i = 0; i < document->priv->n_pages; i++) {
			g_free (document->priv->page_labels[i]);
		}
		g_free (document->priv->page_labels);
		document->priv->page_labels = NULL;
	}

	if (document->priv->info) {
		ev_document_info_free (document->priv->info);
		document->priv->info = NULL;
	}

	if (document->priv->synctex_scanner) {
		synctex_scanner_free (document->priv->synctex_scanner);
		document->priv->synctex_scanner = NULL;
	}

	G_OBJECT_CLASS (ev_document_parent_class)->finalize (object);
}

static void
ev_document_init (EvDocument *document)
{
	document->priv = EV_DOCUMENT_GET_PRIVATE (document);

	/* Assume all pages are the same size until proven otherwise */
	document->priv->uniform = TRUE;
	/* Assume that the document is not a web document*/
	document->iswebdocument = FALSE ;
}

static void
ev_document_class_init (EvDocumentClass *klass)
{
	GObjectClass *g_object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (g_object_class, sizeof (EvDocumentPrivate));

	klass->get_page = ev_document_impl_get_page;
	klass->get_info = ev_document_impl_get_info;
	klass->get_backend_info = NULL;

	g_object_class->finalize = ev_document_finalize;
}

void
ev_document_doc_mutex_lock (void)
{
	g_mutex_lock (&ev_doc_mutex);
}

void
ev_document_doc_mutex_unlock (void)
{
	g_mutex_unlock (&ev_doc_mutex);
}

gboolean
ev_document_doc_mutex_trylock (void)
{
	return g_mutex_trylock (&ev_doc_mutex);
}

void
ev_document_fc_mutex_lock (void)
{
	g_mutex_lock (&ev_fc_mutex);
}

void
ev_document_fc_mutex_unlock (void)
{
	g_mutex_unlock (&ev_fc_mutex);
}

gboolean
ev_document_fc_mutex_trylock (void)
{
	return g_mutex_trylock (&ev_fc_mutex);
}

/**
 * ev_document_load:
 * @document: a #EvDocument
 * @uri: the document's URI
 * @error: a #GError location to store an error, or %NULL
 *
 * Loads @document from @uri.
 * 
 * On failure, %FALSE is returned and @error is filled in.
 * If the document is encrypted, EV_DEFINE_ERROR_ENCRYPTED is returned.
 * If the backend cannot load the specific document, EV_DOCUMENT_ERROR_INVALID
 * is returned. Other errors are possible too, depending on the backend
 * used to load the document and the URI, e.g. #GIOError, #GFileError, and
 * #GConvertError.
 *
 * Returns: %TRUE on success, or %FALSE on failure.
 */
gboolean
ev_document_load (EvDocument  *document,
		  const char  *uri,
		  GError     **error)
{
	EvDocumentClass *klass = EV_DOCUMENT_GET_CLASS (document);
	gboolean retval;
	GError *err = NULL;

	/*
	 * Hardcoding a check for ePub documents, cause it needs a web document DOM
	 * and webkit, support for any other web document types can be added similarly.
	 */

	if ( !g_strcmp0 (ev_file_get_mime_type(uri,TRUE,&err),"application/epub+zip") )
		document->iswebdocument=TRUE ;
		
	retval = klass->load (document, uri, &err);
	if (!retval) {
		if (err) {
			g_propagate_error (error, err);
		} else {
			g_warning ("%s::EvDocument::load returned FALSE but did not fill in @error; fix the backend!\n",
				   G_OBJECT_TYPE_NAME (document));

			/* So upper layers don't crash */
			g_set_error_literal (error,
					     EV_DOCUMENT_ERROR,
					     EV_DOCUMENT_ERROR_INVALID,
					     "Internal error in backend");
		}
	} else {
		gint i;
		EvDocumentPrivate *priv = document->priv;

		/* Cache some info about the document to avoid
		 * going to the backends since it requires locks
		 */

		priv->uri = g_strdup (uri);

		priv->n_pages = _ev_document_get_n_pages (document);
		
		for (i = 0; i < priv->n_pages; i++) {

			/*
			 * Since there is no sense of paging in an ePub,it makes no sense to have pages sizes.
			 * We are however geeneralising the scenario by considering epub as a type of web document.
			 * FIXME: Labels, or bookmarks though, can be done.
			 */
			
			EvPage     *page = ev_document_get_page (document, i);
			gdouble     page_width = 0;
			gdouble     page_height = 0;
			EvPageSize *page_size;
			gchar      *page_label;
			
			if ( document->iswebdocument == FALSE ) {
				_ev_document_get_page_size (document, page, &page_width, &page_height);
			}
			else {
				//Fixed page sized to resolve the X-windowing system error.
				page_width = 800;
				page_height= 600;
			}
			
			if (i == 0) {
				priv->uniform_width = page_width;
				priv->uniform_height = page_height;
				priv->max_width = priv->uniform_width;
				priv->max_height = priv->uniform_height;
				priv->min_width = priv->uniform_width;
				priv->min_height = priv->uniform_height;
				if (document->iswebdocument == TRUE ) {
					priv->page_sizes = g_new0 (EvPageSize, 1);
					priv->page_sizes->width = priv->uniform_width;
					priv->page_sizes->height = priv->uniform_height;
					priv->uniform = TRUE ;
					break;
				}
			} else if (priv->uniform &&
				   (priv->uniform_width != page_width ||
				    priv->uniform_height != page_height)) {
				/* It's a different page size.  Backfill the array. */
				int j;
				
				priv->page_sizes = g_new0 (EvPageSize, priv->n_pages);
				
				for (j = 0; j < i; j++) {
					page_size = &(priv->page_sizes[j]);
					page_size->width = priv->uniform_width;
					page_size->height = priv->uniform_height;
				}
				priv->uniform = FALSE;
			}
			if (!priv->uniform) {
				page_size = &(priv->page_sizes[i]);

				page_size->width = page_width;
				page_size->height = page_height;

				if (page_width > priv->max_width)
					priv->max_width = page_width;
				if (page_width < priv->min_width)
					priv->min_width = page_width;

				if (page_height > priv->max_height)
					priv->max_height = page_height;
				if (page_height < priv->min_height)
					priv->min_height = page_height;
			}

			page_label = _ev_document_get_page_label (document, page);
			if (page_label) {
				if (!priv->page_labels)
					priv->page_labels = g_new0 (gchar *, priv->n_pages);

				priv->page_labels[i] = page_label;
				priv->max_label = MAX (priv->max_label,
						       g_utf8_strlen (page_label, 256));
			}

			g_object_unref (page);
		}

		priv->info = _ev_document_get_info (document);
		if (_ev_document_support_synctex (document)) {
			gchar *filename;

			filename = g_filename_from_uri (uri, NULL, NULL);
			if (filename != NULL) {
				priv->synctex_scanner =
					synctex_scanner_new_with_output_file (filename, NULL, 1);
				g_free (filename);
			}
		}
	}

	return retval;
}

/**
 * ev_document_save:
 * @document:
 * @uri: the target URI
 * @error: a #GError location to store an error, or %NULL
 *
 * Saves @document to @uri.
 * 
 * Returns: %TRUE on success, or %FALSE on error with @error filled in
 */
gboolean
ev_document_save (EvDocument  *document,
		  const char  *uri,
		  GError     **error)
{
	EvDocumentClass *klass = EV_DOCUMENT_GET_CLASS (document);

	return klass->save (document, uri, error);
}

EvPage *
ev_document_get_page (EvDocument *document,
		      gint        index)
{
	EvDocumentClass *klass = EV_DOCUMENT_GET_CLASS (document);

	return klass->get_page (document, index);
}

static gboolean
_ev_document_support_synctex (EvDocument *document)
{
	EvDocumentClass *klass = EV_DOCUMENT_GET_CLASS (document);

	return klass->support_synctex ? klass->support_synctex (document) : FALSE;
}

gboolean
ev_document_has_synctex (EvDocument *document)
{
	g_return_val_if_fail (EV_IS_DOCUMENT (document), FALSE);

	return document->priv->synctex_scanner != NULL;
}

/**
 * ev_document_synctex_backward_search:
 * @document:
 * @page: the target page
 * @x:
 * @y:
 *
 * Peforms a Synctex backward search to obtain the TeX input file, line and
 * (possibly) column  corresponding to the  position (@x,@y) (in 72dpi
 * coordinates) in the  @page of @document.
 *
 * Returns: A pointer to the EvSourceLink structure that holds the result. @NULL if synctex
 * is not enabled for the document or no result is found.
 * The EvSourceLink pointer should be freed with g_free after it is used.
 */
EvSourceLink *
ev_document_synctex_backward_search (EvDocument *document,
                                     gint        page_index,
                                     gfloat      x,
                                     gfloat      y)
{
        EvSourceLink *result = NULL;
        synctex_scanner_t scanner;

        g_return_val_if_fail (EV_IS_DOCUMENT (document), NULL);

        scanner = document->priv->synctex_scanner;
        if (!scanner)
                return NULL;

        if (synctex_edit_query (scanner, page_index + 1, x, y) > 0) {
                synctex_node_t node;

                /* We assume that a backward search returns either zero or one result_node */
                node = synctex_next_result (scanner);
                if (node != NULL) {
                        const gchar *filename;

			filename = synctex_scanner_get_name (scanner, synctex_node_tag (node));

			if (filename) {
				result = ev_source_link_new (filename,
							     synctex_node_line (node),
							     synctex_node_column (node));
			}
                }
        }

        return result;
}

/**
 * ev_document_synctex_forward_search:
 * @document:
 * @source_link:
 *
 * Peforms a Synctex forward search to obtain the area in the document
 * corresponding to the position @line and @column number in the source Tex file
 *
 * Returns: An EvMapping with the page number and area corresponfing to
 * the given line in the source file. It must be free with g_free when done
 */
EvMapping *
ev_document_synctex_forward_search (EvDocument   *document,
				    EvSourceLink *link)
{
        EvMapping        *result = NULL;
        synctex_scanner_t scanner;

        g_return_val_if_fail (EV_IS_DOCUMENT (document), NULL);

        scanner = document->priv->synctex_scanner;
        if (!scanner)
                return NULL;

        if (synctex_display_query (scanner, link->filename, link->line, link->col) > 0) {
                synctex_node_t node;
                gint           page;

                if ((node = synctex_next_result (scanner))) {
                        result = g_new (EvMapping, 1);

                        page = synctex_node_page (node) - 1;
                        result->data = GINT_TO_POINTER (page);

                        result->area.x1 = synctex_node_box_visible_h (node);
                        result->area.y1 = synctex_node_box_visible_v (node) -
                                synctex_node_box_visible_height (node);
                        result->area.x2 = synctex_node_box_visible_width (node) + result->area.x1;
                        result->area.y2 = synctex_node_box_visible_depth (node) +
                                synctex_node_box_visible_height (node) + result->area.y1;
                }
        }

        return result;
}

static gint
_ev_document_get_n_pages (EvDocument  *document)
{
	EvDocumentClass *klass = EV_DOCUMENT_GET_CLASS (document);

	return klass->get_n_pages (document);
}

gint
ev_document_get_n_pages (EvDocument  *document)
{
	g_return_val_if_fail (EV_IS_DOCUMENT (document), 0);

	return document->priv->n_pages;
}

static void
_ev_document_get_page_size (EvDocument *document,
			    EvPage     *page,
			    double     *width,
			    double     *height)
{
	EvDocumentClass *klass = EV_DOCUMENT_GET_CLASS (document);

	klass->get_page_size (document, page, width, height);
}

void
ev_document_get_page_size (EvDocument *document,
			   gint        page_index,
			   double     *width,
			   double     *height)
{
	g_return_if_fail (EV_IS_DOCUMENT (document));
	g_return_if_fail (page_index >= 0 || page_index < document->priv->n_pages);
	if (document->iswebdocument == TRUE ) {
		if (width)
			*width = document->priv->uniform_width;
		if (height)
			*height = document->priv->uniform_height;
	} else {
		if (width)
			*width = document->priv->uniform ?
				document->priv->uniform_width :
				document->priv->page_sizes[page_index].width;
		if (height)
			*height = document->priv->uniform ?
				document->priv->uniform_height :
				document->priv->page_sizes[page_index].height;
	}
}

static gchar *
_ev_document_get_page_label (EvDocument *document,
			     EvPage     *page)
{
	EvDocumentClass *klass = EV_DOCUMENT_GET_CLASS (document);

	return klass->get_page_label ?
		klass->get_page_label (document, page) : NULL;
}

gchar *
ev_document_get_page_label (EvDocument *document,
			    gint        page_index)
{
	g_return_val_if_fail (EV_IS_DOCUMENT (document), NULL);
	g_return_val_if_fail (page_index >= 0 || page_index < document->priv->n_pages, NULL);

	return (document->priv->page_labels && document->priv->page_labels[page_index]) ?
		g_strdup (document->priv->page_labels[page_index]) :
		g_strdup_printf ("%d", page_index + 1);
}

static EvDocumentInfo *
_ev_document_get_info (EvDocument *document)
{
	EvDocumentClass *klass = EV_DOCUMENT_GET_CLASS (document);

	return klass->get_info (document);
}

EvDocumentInfo *
ev_document_get_info (EvDocument *document)
{
	g_return_val_if_fail (EV_IS_DOCUMENT (document), NULL);

	return document->priv->info;
}

gboolean
ev_document_get_backend_info (EvDocument *document, EvDocumentBackendInfo *info)
{
	g_return_val_if_fail (EV_IS_DOCUMENT (document), FALSE);

	EvDocumentClass *klass = EV_DOCUMENT_GET_CLASS (document);
	if (klass->get_backend_info == NULL)
		return FALSE;

	return klass->get_backend_info (document, info);
}

cairo_surface_t *
ev_document_render (EvDocument      *document,
		    EvRenderContext *rc)
{
	EvDocumentClass *klass = EV_DOCUMENT_GET_CLASS (document);

	return klass->render (document, rc);
}

const gchar *
ev_document_get_uri (EvDocument *document)
{
	g_return_val_if_fail (EV_IS_DOCUMENT (document), NULL);

	return document->priv->uri;
}

const gchar *
ev_document_get_title (EvDocument *document)
{
	g_return_val_if_fail (EV_IS_DOCUMENT (document), NULL);

	return (document->priv->info->fields_mask & EV_DOCUMENT_INFO_TITLE) ?
		document->priv->info->title : NULL;
}

gboolean
ev_document_is_page_size_uniform (EvDocument *document)
{
	g_return_val_if_fail (EV_IS_DOCUMENT (document), TRUE);

	return document->priv->uniform;
}

void
ev_document_get_max_page_size (EvDocument *document,
			       gdouble    *width,
			       gdouble    *height)
{
	g_return_if_fail (EV_IS_DOCUMENT (document));

	if (width)
		*width = document->priv->max_width;
	if (height)
		*height = document->priv->max_height;
}

void
ev_document_get_min_page_size (EvDocument *document,
			       gdouble    *width,
			       gdouble    *height)
{
	g_return_if_fail (EV_IS_DOCUMENT (document));

	if (width)
		*width = document->priv->min_width;
	if (height)
		*height = document->priv->min_height;
}

gboolean
ev_document_check_dimensions (EvDocument *document)
{
	g_return_val_if_fail (EV_IS_DOCUMENT (document), FALSE);

	return (document->priv->max_width > 0 && document->priv->max_height > 0);
}

gint
ev_document_get_max_label_len (EvDocument *document)
{
	g_return_val_if_fail (EV_IS_DOCUMENT (document), -1);

	return document->priv->max_label;
}

gboolean
ev_document_has_text_page_labels (EvDocument *document)
{
	g_return_val_if_fail (EV_IS_DOCUMENT (document), FALSE);

	return document->priv->page_labels != NULL;
}

gboolean
ev_document_find_page_by_label (EvDocument  *document,
				const gchar *page_label,
				gint        *page_index)
{
	gint i, page;
	glong value;
	gchar *endptr = NULL;
	EvDocumentPrivate *priv = document->priv;

	g_return_val_if_fail (EV_IS_DOCUMENT (document), FALSE);
	g_return_val_if_fail (page_label != NULL, FALSE);
	g_return_val_if_fail (page_index != NULL, FALSE);

        /* First, look for a literal label match */
	for (i = 0; priv->page_labels && i < priv->n_pages; i ++) {
		if (priv->page_labels[i] != NULL &&
		    ! strcmp (page_label, priv->page_labels[i])) {
			*page_index = i;
			return TRUE;
		}
	}

	/* Second, look for a match with case insensitively */
	for (i = 0; priv->page_labels && i < priv->n_pages; i++) {
		if (priv->page_labels[i] != NULL &&
		    ! strcasecmp (page_label, priv->page_labels[i])) {
			*page_index = i;
			return TRUE;
		}
	}

	/* Next, parse the label, and see if the number fits */
	value = strtol (page_label, &endptr, 10);
	if (endptr[0] == '\0') {
		/* Page number is an integer */
		page = MIN (G_MAXINT, value);

		/* convert from a page label to a page offset */
		page --;
		if (page >= 0 && page < priv->n_pages) {
			*page_index = page;
			return TRUE;
		}
	}

	return FALSE;
}

/* EvSourceLink */
G_DEFINE_BOXED_TYPE (EvSourceLink, ev_source_link, ev_source_link_copy, ev_source_link_free)

EvSourceLink *
ev_source_link_new (const gchar *filename,
		    gint 	 line,
		    gint 	 col)
{
	EvSourceLink *link = g_slice_new (EvSourceLink);

	link->filename = g_strdup (filename);
	link->line = line;
	link->col = col;

	return link;
}

EvSourceLink *
ev_source_link_copy (EvSourceLink *link)
{
	EvSourceLink *copy;

	g_return_val_if_fail (link != NULL, NULL);

	copy = g_slice_new (EvSourceLink);

	*copy = *link;
	copy->filename = g_strdup (link->filename);

	return copy;
}

void
ev_source_link_free (EvSourceLink *link)
{
	if (link == NULL)
		return;

	g_free (link->filename);
	g_slice_free (EvSourceLink, link);
}

/* EvDocumentInfo */
G_DEFINE_BOXED_TYPE (EvDocumentInfo, ev_document_info, ev_document_info_copy, ev_document_info_free)

EvDocumentInfo *
ev_document_info_copy (EvDocumentInfo *info)
{
	EvDocumentInfo *copy;
	
	g_return_val_if_fail (info != NULL, NULL);

	copy = g_new0 (EvDocumentInfo, 1);
	copy->title = g_strdup (info->title);
	copy->format = g_strdup (info->format);
	copy->author = g_strdup (info->author);
	copy->subject = g_strdup (info->subject);
	copy->keywords = g_strdup (info->keywords);
	copy->security = g_strdup (info->security);
	copy->creator = g_strdup (info->creator);
	copy->producer = g_strdup (info->producer);
	copy->linearized = g_strdup (info->linearized);
	
	copy->creation_date = info->creation_date;
	copy->modified_date = info->modified_date;
	copy->layout = info->layout;
	copy->mode = info->mode;
	copy->ui_hints = info->ui_hints;
	copy->permissions = info->permissions;
	copy->n_pages = info->n_pages;
	copy->license = ev_document_license_copy (info->license);

	copy->fields_mask = info->fields_mask;

	return copy;
}

void
ev_document_info_free (EvDocumentInfo *info)
{
	if (info == NULL)
		return;

	g_free (info->title);
	g_free (info->format);
	g_free (info->author);
	g_free (info->subject);
	g_free (info->keywords);
	g_free (info->creator);
	g_free (info->producer);
	g_free (info->linearized);

	g_free (info->security);
	ev_document_license_free (info->license);

	g_free (info);
}

/* EvDocumentLicense */
G_DEFINE_BOXED_TYPE (EvDocumentLicense, ev_document_license, ev_document_license_copy, ev_document_license_free)

EvDocumentLicense *
ev_document_license_new (void)
{
	return g_new0 (EvDocumentLicense, 1);
}

EvDocumentLicense *
ev_document_license_copy (EvDocumentLicense *license)
{
	EvDocumentLicense *new_license;

	if (!license)
		return NULL;

	new_license = ev_document_license_new ();

	if (license->text)
		new_license->text = g_strdup (license->text);
	if (license->uri)
		new_license->uri = g_strdup (license->uri);
	if (license->web_statement)
		new_license->web_statement = g_strdup (license->web_statement);

	return new_license;
}

void
ev_document_license_free (EvDocumentLicense *license)
{
	if (!license)
		return;

	g_free (license->text);
	g_free (license->uri);
	g_free (license->web_statement);

	g_free (license);
}

const gchar *
ev_document_license_get_text (EvDocumentLicense *license)
{
	return license->text;
}

const gchar *
ev_document_license_get_uri (EvDocumentLicense *license)
{
	return license->uri;
}

const gchar *
ev_document_license_get_web_statement (EvDocumentLicense *license)
{
	return license->web_statement;
}

/* EvRectangle */
G_DEFINE_BOXED_TYPE (EvRectangle, ev_rectangle, ev_rectangle_copy, ev_rectangle_free)

EvRectangle *
ev_rectangle_new (void)
{
	return g_new0 (EvRectangle, 1);
}

EvRectangle *
ev_rectangle_copy (EvRectangle *rectangle)
{
	EvRectangle *new_rectangle;

	g_return_val_if_fail (rectangle != NULL, NULL);

	new_rectangle = g_new (EvRectangle, 1);
	*new_rectangle = *rectangle;

	return new_rectangle;
}

void
ev_rectangle_free (EvRectangle *rectangle)
{
	g_free (rectangle);
}

/* Compares two rects.  returns 0 if they're equal */
#define EPSILON 0.0000001

gint
ev_rect_cmp (EvRectangle *a,
	     EvRectangle *b)
{
	if (a == b)
		return 0;
	if (a == NULL || b == NULL)
		return 1;

	return ! ((ABS (a->x1 - b->x1) < EPSILON) &&
		  (ABS (a->y1 - b->y1) < EPSILON) &&
		  (ABS (a->x2 - b->x2) < EPSILON) &&
		  (ABS (a->y2 - b->y2) < EPSILON));
}

void
ev_document_toggle_night_mode(EvDocument *document,gboolean night)
{
	EvDocumentClass *klass = EV_DOCUMENT_GET_CLASS(document);
	return klass->toggle_night_mode(document,night) ;
}

void
ev_document_check_add_night_sheet(EvDocument *document)
{
	EvDocumentClass *klass = EV_DOCUMENT_GET_CLASS(document);
	return klass->check_add_night_sheet(document) ;
}
