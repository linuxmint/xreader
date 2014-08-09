/* this file is part of atril, a mate document viewer
 *
 *  Copyright (C) 2014 Avishkar Gupta
 *  Based on ev-view.h, also a part of atril, a mate document viewer
 *
 * Atril is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Atril is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#if !defined (__EV_ATRIL_VIEW_H_INSIDE__) && !defined (ATRIL_COMPILATION)
#error "Only <atril-web_view.h> can be included directly."
#endif

#ifndef __EV_WEB_VIEW_H__
#define __EV_WEB_VIEW_H__

#include <gtk/gtk.h>

#include <atril-document.h>
#include "ev-jobs.h"
#include "ev-document-model.h"
#include <glib-object.h>
G_BEGIN_DECLS


typedef struct _EvWebView       EvWebView;
typedef struct _EvWebViewClass  EvWebViewClass;

#define EV_TYPE_WEB_VIEW         (ev_web_view_get_type ())
#define EV_WEB_VIEW(obj)         (G_TYPE_CHECK_INSTANCE_CAST ((obj), EV_TYPE_WEB_VIEW, EvWebView))
#define EV_IS_WEB_VIEW(obj)      (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EV_TYPE_WEB_VIEW))

GType		ev_web_view_get_type			(void) G_GNUC_CONST;

GtkWidget*	ev_web_view_new					(void);

void		ev_web_view_set_model	        (EvWebView          *webview,
										     EvDocumentModel *model);

void       ev_web_view_reload               (EvWebView          *webview);

void       ev_web_view_reload_page			(EvWebView         *webview,
  		    								 gint               page);
	
/* Navigation */
gboolean       ev_web_view_next_page		  (EvWebView         *webview);
gboolean       ev_web_view_previous_page	  (EvWebView         *webview);

/* Sidebar links */
void       ev_web_view_handle_link (EvWebView *webview, EvLink* link);

/* Searching */
void     ev_web_view_find_next                 (EvWebView *webview);
void     ev_web_view_find_previous             (EvWebView *webview);

void     ev_web_view_find_changed              (EvWebView *webview,
                                                guint *results,
                                                gchar *text, 
                                                gboolean case_sensitive);

void     ev_web_view_find_search_changed       (EvWebView *webview);
void     ev_web_view_find_cancel               (EvWebView *webview);
void     ev_web_view_find_set_highlight_search (EvWebView *webview,gboolean visible);
void     ev_web_view_set_handler               (EvWebView *webview,gboolean visible);
/* Selection */
gboolean	ev_web_view_get_has_selection      (EvWebView *webview);
void		ev_web_view_select_all			   (EvWebView  *webview);
void		ev_web_view_copy				   (EvWebView  *webview);

/* Zoom control */
gboolean     ev_web_view_zoom_in               (EvWebView *webview);
gboolean     ev_web_view_zoom_out              (EvWebView *webview);

/*For safe replacement by an EvView*/
void       ev_web_view_disconnect_handlers    (EvWebView  *webview);
G_END_DECLS

#endif /* __EV_WEB_VIEW_H__ */
