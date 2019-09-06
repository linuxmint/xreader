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

#if !defined (__EV_XREADER_VIEW_H_INSIDE__) && !defined (XREADER_COMPILATION)
#error "Only <xreader-view.h> can be included directly."
#endif

#ifndef EV_PAGE_CACHE_H
#define EV_PAGE_CACHE_H

#include <glib-object.h>
#include <gdk/gdk.h>
#include <xreader-document.h>
#include <xreader-view.h>

G_BEGIN_DECLS

#define EV_TYPE_PAGE_CACHE    (ev_page_cache_get_type ())
#define EV_PAGE_CACHE(obj)    (G_TYPE_CHECK_INSTANCE_CAST ((obj), EV_TYPE_PAGE_CACHE, EvPageCache))
#define EV_IS_PAGE_CACHE(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EV_TYPE_PAGE_CACHE))

typedef struct _EvPageCache        EvPageCache;
typedef struct _EvPageCacheClass   EvPageCacheClass;

GType              ev_page_cache_get_type               (void) G_GNUC_CONST;
EvPageCache       *ev_page_cache_new                    (EvDocument        *document);

void               ev_page_cache_set_page_range         (EvPageCache       *cache,
							 gint               start,
							 gint               end);
EvJobPageDataFlags ev_page_cache_get_flags              (EvPageCache       *cache);
void               ev_page_cache_set_flags              (EvPageCache       *cache,
							 EvJobPageDataFlags flags);
void               ev_page_cache_mark_dirty             (EvPageCache       *cache,
							 gint               page,
                                                         EvJobPageDataFlags flags);
EvMappingList     *ev_page_cache_get_link_mapping       (EvPageCache       *cache,
							 gint               page);
EvMappingList     *ev_page_cache_get_image_mapping      (EvPageCache       *cache,
							 gint               page);
EvMappingList     *ev_page_cache_get_form_field_mapping (EvPageCache       *cache,
							 gint               page);
EvMappingList     *ev_page_cache_get_annot_mapping      (EvPageCache       *cache,
							 gint               page);
cairo_region_t    *ev_page_cache_get_text_mapping       (EvPageCache       *cache,
							 gint               page);
const gchar       *ev_page_cache_get_text               (EvPageCache       *cache,
							 gint               page);
gboolean           ev_page_cache_get_text_layout        (EvPageCache       *cache,
							 gint               page,
							 EvRectangle      **areas,
							 guint             *n_areas);

G_END_DECLS

#endif /* EV_PAGE_CACHE_H */
