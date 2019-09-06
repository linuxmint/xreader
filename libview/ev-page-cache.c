/* this file is part of xreader, a mate document viewer
 *
 *  Copyright (C) 2009 Carlos Garcia Campos
 *
 * Xreader is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Xreader is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <config.h>

#include <glib.h>
#include "ev-jobs.h"
#include "ev-job-scheduler.h"
#include "ev-mapping-list.h"
#include "ev-selection.h"
#include "ev-document-links.h"
#include "ev-document-forms.h"
#include "ev-document-images.h"
#include "ev-document-annotations.h"
#include "ev-document-text.h"
#include "ev-page-cache.h"

typedef struct _EvPageCacheData {
	EvJob             *job;
	gboolean           done : 1;
	gboolean           dirty : 1;
	EvJobPageDataFlags flags;

	EvMappingList     *link_mapping;
	EvMappingList     *image_mapping;
	EvMappingList     *form_field_mapping;
	EvMappingList     *annot_mapping;
	cairo_region_t    *text_mapping;
	EvRectangle       *text_layout;
	guint              text_layout_length;
	gchar             *text;
} EvPageCacheData;

struct _EvPageCache {
	GObject parent;

	EvDocument        *document;
	EvPageCacheData   *page_list;
	gint               n_pages;

	/* Current range */
	gint               start_page;
	gint               end_page;

	EvJobPageDataFlags flags;
};

struct _EvPageCacheClass {
	GObjectClass parent_class;
};

#define EV_PAGE_DATA_FLAGS_DEFAULT (        \
	EV_PAGE_DATA_INCLUDE_LINKS        | \
	EV_PAGE_DATA_INCLUDE_TEXT_MAPPING | \
	EV_PAGE_DATA_INCLUDE_IMAGES       | \
	EV_PAGE_DATA_INCLUDE_FORMS        | \
	EV_PAGE_DATA_INCLUDE_ANNOTS)


static void job_page_data_finished_cb (EvJob       *job,
				       EvPageCache *cache);
static void job_page_data_cancelled_cb (EvJob       *job,
					EvPageCacheData *data);

G_DEFINE_TYPE (EvPageCache, ev_page_cache, G_TYPE_OBJECT)

static void
ev_page_cache_data_free (EvPageCacheData *data)
{
	if (data->job) {
		g_object_unref (data->job);
		data->job = NULL;
	}

	if (data->link_mapping) {
		ev_mapping_list_unref (data->link_mapping);
		data->link_mapping = NULL;
	}

	if (data->image_mapping) {
		ev_mapping_list_unref (data->image_mapping);
		data->image_mapping = NULL;
	}

	if (data->form_field_mapping) {
		ev_mapping_list_unref (data->form_field_mapping);
		data->form_field_mapping = NULL;
	}

	if (data->annot_mapping) {
		ev_mapping_list_unref (data->annot_mapping);
		data->annot_mapping = NULL;
	}

	if (data->text_mapping) {
		cairo_region_destroy (data->text_mapping);
		data->text_mapping = NULL;
	}

	if (data->text_layout) {
		g_free (data->text_layout);
		data->text_layout = NULL;
		data->text_layout_length = 0;
	}

	if (data->text) {
		g_free (data->text);
		data->text = NULL;
	}
}

static void
ev_page_cache_finalize (GObject *object)
{
	EvPageCache *cache = EV_PAGE_CACHE (object);
	gint         i;

	if (cache->page_list) {
		for (i = 0; i < cache->n_pages; i++) {
			EvPageCacheData *data;

			data = &cache->page_list[i];

			if (data->job) {
				g_signal_handlers_disconnect_by_func (data->job,
								      G_CALLBACK (job_page_data_finished_cb),
								      cache);
				g_signal_handlers_disconnect_by_func (data->job,
								      G_CALLBACK (job_page_data_cancelled_cb),
								      data);
			}
			ev_page_cache_data_free (data);
		}

		g_free (cache->page_list);
		cache->page_list = NULL;
		cache->n_pages = 0;
	}

	if (cache->document) {
		g_object_unref (cache->document);
		cache->document = NULL;
	}

	G_OBJECT_CLASS (ev_page_cache_parent_class)->finalize (object);
}

static void
ev_page_cache_init (EvPageCache *cache)
{
}

static void
ev_page_cache_class_init (EvPageCacheClass *klass)
{
	GObjectClass *g_object_class = G_OBJECT_CLASS (klass);

	g_object_class->finalize = ev_page_cache_finalize;
}

static EvJobPageDataFlags
ev_page_cache_get_flags_for_data (EvPageCache     *cache,
				  EvPageCacheData *data)
{
	EvJobPageDataFlags flags = EV_PAGE_DATA_INCLUDE_NONE;

	if (data->flags == cache->flags && !data->dirty)
		return cache->flags;

	/* Flags changed or data is dirty */
	if (cache->flags & EV_PAGE_DATA_INCLUDE_LINKS) {
		flags = (data->link_mapping) ?
			flags & ~EV_PAGE_DATA_INCLUDE_LINKS :
			flags | EV_PAGE_DATA_INCLUDE_LINKS;
	}

	if (cache->flags & EV_PAGE_DATA_INCLUDE_IMAGES) {
		flags = (data->image_mapping) ?
			flags & ~EV_PAGE_DATA_INCLUDE_IMAGES :
			flags | EV_PAGE_DATA_INCLUDE_IMAGES;
	}

	if (cache->flags & EV_PAGE_DATA_INCLUDE_FORMS) {
		flags = (data->form_field_mapping) ?
			flags & ~EV_PAGE_DATA_INCLUDE_FORMS :
			flags | EV_PAGE_DATA_INCLUDE_FORMS;
	}

	if (cache->flags & EV_PAGE_DATA_INCLUDE_ANNOTS) {
		flags = (data->annot_mapping) ?
			flags & ~EV_PAGE_DATA_INCLUDE_ANNOTS :
			flags | EV_PAGE_DATA_INCLUDE_ANNOTS;
	}

	if (cache->flags & EV_PAGE_DATA_INCLUDE_TEXT_MAPPING) {
		flags = (data->text_mapping) ?
			flags & ~EV_PAGE_DATA_INCLUDE_TEXT_MAPPING :
			flags | EV_PAGE_DATA_INCLUDE_TEXT_MAPPING;
	}

	if (cache->flags & EV_PAGE_DATA_INCLUDE_TEXT) {
		flags = (data->text) ?
			flags & ~EV_PAGE_DATA_INCLUDE_TEXT :
			flags | EV_PAGE_DATA_INCLUDE_TEXT;
	}

	if (cache->flags & EV_PAGE_DATA_INCLUDE_TEXT_LAYOUT) {
		flags = (data->text_layout_length > 0) ?
			flags & ~EV_PAGE_DATA_INCLUDE_TEXT_LAYOUT :
			flags | EV_PAGE_DATA_INCLUDE_TEXT_LAYOUT;
	}

	return flags;
}

EvPageCache *
ev_page_cache_new (EvDocument *document)
{
	EvPageCache *cache;

	g_return_val_if_fail (EV_IS_DOCUMENT (document), NULL);

	cache = EV_PAGE_CACHE (g_object_new (EV_TYPE_PAGE_CACHE, NULL));
	cache->document = g_object_ref (document);
	cache->n_pages = ev_document_get_n_pages (document);
	cache->flags = EV_PAGE_DATA_FLAGS_DEFAULT;
	cache->page_list = g_new0 (EvPageCacheData, cache->n_pages);

	return cache;
}

static void
job_page_data_finished_cb (EvJob       *job,
			   EvPageCache *cache)
{
	EvJobPageData   *job_data = EV_JOB_PAGE_DATA (job);
	EvPageCacheData *data;

	data = &cache->page_list[job_data->page];

	if (job_data->flags & EV_PAGE_DATA_INCLUDE_LINKS)
		data->link_mapping = job_data->link_mapping;
	if (job_data->flags & EV_PAGE_DATA_INCLUDE_IMAGES)
		data->image_mapping = job_data->image_mapping;
	if (job_data->flags & EV_PAGE_DATA_INCLUDE_FORMS)
		data->form_field_mapping = job_data->form_field_mapping;
	if (job_data->flags & EV_PAGE_DATA_INCLUDE_ANNOTS)
		data->annot_mapping = job_data->annot_mapping;
	if (job_data->flags & EV_PAGE_DATA_INCLUDE_TEXT_MAPPING)
		data->text_mapping = job_data->text_mapping;
	if (job_data->flags & EV_PAGE_DATA_INCLUDE_TEXT_LAYOUT) {
		data->text_layout = job_data->text_layout;
		data->text_layout_length = job_data->text_layout_length;
	}
	if (job_data->flags & EV_PAGE_DATA_INCLUDE_TEXT)
		data->text = job_data->text;
	data->done = TRUE;
	data->dirty = FALSE;

	g_object_unref (data->job);
	data->job = NULL;
}

static void
job_page_data_cancelled_cb (EvJob           *job,
			    EvPageCacheData *data)
{
	g_object_unref (data->job);
	data->job = NULL;
}

void
ev_page_cache_set_page_range (EvPageCache *cache,
			      gint         start,
			      gint         end)
{
	gint i;

	if (cache->flags == EV_PAGE_DATA_INCLUDE_NONE)
		return;

	cache->start_page = start;
	cache->end_page = end;

	for (i = start; i <= end; i++) {
		EvPageCacheData   *data = &cache->page_list[i];
		EvJobPageDataFlags flags;

		if (data->flags == cache->flags && !data->dirty && (data->done || data->job))
			continue;

		if (data->job)
			ev_job_cancel (data->job);

		flags = ev_page_cache_get_flags_for_data (cache, data);

		data->flags = cache->flags;
		data->job = ev_job_page_data_new (cache->document, i, flags);
		g_signal_connect (data->job, "finished",
				  G_CALLBACK (job_page_data_finished_cb),
				  cache);
		g_signal_connect (data->job, "cancelled",
				  G_CALLBACK (job_page_data_cancelled_cb),
				  data);
		ev_job_scheduler_push_job (data->job, EV_JOB_PRIORITY_NONE);
	}
}

EvJobPageDataFlags
ev_page_cache_get_flags (EvPageCache *cache)
{
	return cache->flags;
}

void
ev_page_cache_set_flags (EvPageCache       *cache,
			 EvJobPageDataFlags flags)
{
	if (cache->flags == flags)
		return;

	cache->flags = flags;

	/* Update the current range for new flags */
	ev_page_cache_set_page_range (cache, cache->start_page, cache->end_page);
}

void
ev_page_cache_mark_dirty (EvPageCache       *cache,
                          gint               page,
                          EvJobPageDataFlags flags)
{
	EvPageCacheData *data;

	g_return_if_fail (EV_IS_PAGE_CACHE (cache));

	data = &cache->page_list[page];
	data->dirty = TRUE;

	if (flags & EV_PAGE_DATA_INCLUDE_LINKS)
		g_clear_pointer (&data->link_mapping, ev_mapping_list_unref);

	if (flags & EV_PAGE_DATA_INCLUDE_IMAGES)
		g_clear_pointer (&data->image_mapping, ev_mapping_list_unref);

	if (flags & EV_PAGE_DATA_INCLUDE_FORMS)
		g_clear_pointer (&data->form_field_mapping, ev_mapping_list_unref);

	if (flags & EV_PAGE_DATA_INCLUDE_ANNOTS)
		g_clear_pointer (&data->annot_mapping, ev_mapping_list_unref);

	if (flags & EV_PAGE_DATA_INCLUDE_TEXT_MAPPING)
		g_clear_pointer (&data->text_mapping, cairo_region_destroy);

	if (flags & EV_PAGE_DATA_INCLUDE_TEXT)
		g_clear_pointer (&data->text, g_free);

	if (flags & EV_PAGE_DATA_INCLUDE_TEXT_LAYOUT) {
		g_clear_pointer (&data->text_layout, g_free);
		data->text_layout_length = 0;
	}

	/* Update the current range */
	ev_page_cache_set_page_range (cache, cache->start_page, cache->end_page);
}

EvMappingList *
ev_page_cache_get_link_mapping (EvPageCache *cache,
				gint         page)
{
	EvPageCacheData *data;

	g_return_val_if_fail (EV_IS_PAGE_CACHE (cache), NULL);
	g_return_val_if_fail (page >= 0 && page < cache->n_pages, NULL);

	if (!(cache->flags & EV_PAGE_DATA_INCLUDE_LINKS))
		return NULL;

	data = &cache->page_list[page];
	if (data->done)
		return data->link_mapping;

	if (data->job)
		return EV_JOB_PAGE_DATA (data->job)->link_mapping;

	return data->link_mapping;
}

EvMappingList *
ev_page_cache_get_image_mapping (EvPageCache *cache,
				 gint         page)
{
	EvPageCacheData *data;

	g_return_val_if_fail (EV_IS_PAGE_CACHE (cache), NULL);
	g_return_val_if_fail (page >= 0 && page < cache->n_pages, NULL);

	if (!(cache->flags & EV_PAGE_DATA_INCLUDE_IMAGES))
		return NULL;

	data = &cache->page_list[page];
	if (data->done)
		return data->image_mapping;

	if (data->job)
		return EV_JOB_PAGE_DATA (data->job)->image_mapping;

	return data->image_mapping;
}

EvMappingList *
ev_page_cache_get_form_field_mapping (EvPageCache *cache,
				      gint         page)
{
	EvPageCacheData *data;

	g_return_val_if_fail (EV_IS_PAGE_CACHE (cache), NULL);
	g_return_val_if_fail (page >= 0 && page < cache->n_pages, NULL);

	if (!(cache->flags & EV_PAGE_DATA_INCLUDE_FORMS))
		return NULL;

	data = &cache->page_list[page];
	if (data->done)
		return data->form_field_mapping;

	if (data->job)
		return EV_JOB_PAGE_DATA (data->job)->form_field_mapping;

	return data->form_field_mapping;
}

EvMappingList *
ev_page_cache_get_annot_mapping (EvPageCache *cache,
				 gint         page)
{
	EvPageCacheData *data;

	g_return_val_if_fail (EV_IS_PAGE_CACHE (cache), NULL);
	g_return_val_if_fail (page >= 0 && page < cache->n_pages, NULL);

	if (!(cache->flags & EV_PAGE_DATA_INCLUDE_ANNOTS))
		return NULL;

	data = &cache->page_list[page];
	if (data->done)
		return data->annot_mapping;

	if (data->job)
		return EV_JOB_PAGE_DATA (data->job)->annot_mapping;

	return data->annot_mapping;
}

cairo_region_t *
ev_page_cache_get_text_mapping (EvPageCache *cache,
				gint         page)
{
	EvPageCacheData *data;

	g_return_val_if_fail (EV_IS_PAGE_CACHE (cache), NULL);
	g_return_val_if_fail (page >= 0 && page < cache->n_pages, NULL);

	if (!(cache->flags & EV_PAGE_DATA_INCLUDE_TEXT_MAPPING))
		return NULL;

	data = &cache->page_list[page];
	if (data->done)
		return data->text_mapping;

	if (data->job)
		return EV_JOB_PAGE_DATA (data->job)->text_mapping;

	return data->text_mapping;
}

const gchar *
ev_page_cache_get_text (EvPageCache *cache,
			     gint         page)
{
	EvPageCacheData *data;

	g_return_val_if_fail (EV_IS_PAGE_CACHE (cache), NULL);
	g_return_val_if_fail (page >= 0 && page < cache->n_pages, NULL);

	if (!(cache->flags & EV_PAGE_DATA_INCLUDE_TEXT))
		return NULL;

	data = &cache->page_list[page];
	if (data->done)
		return data->text;

	if (data->job)
		return EV_JOB_PAGE_DATA (data->job)->text;

	return data->text;
}

gboolean
ev_page_cache_get_text_layout (EvPageCache  *cache,
			       gint          page,
			       EvRectangle **areas,
			       guint        *n_areas)
{
	EvPageCacheData *data;

	g_return_val_if_fail (EV_IS_PAGE_CACHE (cache), FALSE);
	g_return_val_if_fail (page >= 0 && page < cache->n_pages, FALSE);

	if (!(cache->flags & EV_PAGE_DATA_INCLUDE_TEXT_LAYOUT))
		return FALSE;

	data = &cache->page_list[page];
	if (data->done)	{
		*areas = data->text_layout;
		*n_areas = data->text_layout_length;

		return TRUE;
	}

	if (data->job) {
		*areas = EV_JOB_PAGE_DATA (data->job)->text_layout;
		*n_areas = EV_JOB_PAGE_DATA (data->job)->text_layout_length;

		return TRUE;
	}

	return FALSE;
}
