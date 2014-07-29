/* this file is part of atril, a mate document viewer
 *
 *  Copyright (C) 2014 Avishkar Gupta
 *  Based on ev-view.c, also a part of atril, a mate document viewer.
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

#include "config.h"

#include <glib/gi18n-lib.h>
#include <gtk/gtk.h>

#if GTK_CHECK_VERSION (3, 0, 0)
#include <webkit2/webkit2.h>
#else
#include <webkit/webkit.h>
#endif

#include "ev-web-view.h"
#include "ev-document-model.h"
#include "ev-jobs.h"

#define EV_WEB_VIEW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), EV_TYPE_WEB_VIEW, EvWebViewClass))
#define EV_IS_WEB_VIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), EV_TYPE_WEB_VIEW))
#define EV_WEB_VIEW_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), EV_TYPE_WEB_VIEW, EvWebViewClass))

typedef struct _SearchParams {
	gboolean case_sensitive;
	guint    page_current;
	gboolean search_jump;
	gchar*   search_string;
	guint    on_result;
	guint    n_results;
}SearchParams;

struct _EvWebView
{
	WebKitWebView web_view;
	EvDocument *document;
	EvDocumentModel *model;
	gint current_page;
	gboolean inverted_colors ;
	gboolean fullscreen;
	SearchParams *search;
};

struct _EvWebViewClass 
{
	WebKitWebViewClass base_class;
};

/*** Callbacks ***/
static void       ev_web_view_change_page                   (EvWebView          *webview,
						          							  gint                new_page);

static void       ev_web_view_page_changed_cb               (EvDocumentModel     *model,
														      gint                 old_page,
															  gint                 new_page,
														      EvWebView           *webview);
/*** GObject ***/
static void       ev_web_view_dispose                           (GObject             *object);

static void       ev_web_view_finalize						 (GObject             *object);
static void       ev_web_view_class_init                    (EvWebViewClass      *klass);
static void       ev_web_view_init                          (EvWebView           *webview);

G_DEFINE_TYPE (EvWebView, ev_web_view, WEBKIT_TYPE_WEB_VIEW)
static void
web_view_update_range_and_current_page (EvWebView *webview)
{
	g_return_if_fail(EV_IS_WEB_VIEW(webview));
	
	if (ev_document_get_n_pages (webview->document) <= 0)
		return;

	ev_document_model_set_page(webview->model, 0);
	webview->current_page = 0;
	EvPage *webpage = ev_document_get_page(webview->document,0);
	webkit_web_view_load_uri(WEBKIT_WEB_VIEW(webview),(gchar*)webpage->backend_page);
}

static void
ev_web_view_dispose (GObject *object)
{
	EvWebView *webview = EV_WEB_VIEW (object);

	if (webview->document) {
		g_object_unref(webview->document);
		webview->document = NULL ;
	}

	if (webview->model) {
		g_object_unref(webview->model);
		webview->model = NULL;
	};
	

	G_OBJECT_CLASS (ev_web_view_parent_class)->dispose (object);
}

static void
ev_web_view_class_init (EvWebViewClass *klass)
{
	G_OBJECT_CLASS(klass)->finalize = ev_web_view_finalize;
	G_OBJECT_CLASS(klass)->dispose = ev_web_view_dispose;
}

static void
ev_web_view_init (EvWebView *webview)
{
	gtk_widget_set_can_focus (GTK_WIDGET (webview), TRUE);

	gtk_widget_set_has_window (GTK_WIDGET (webview), TRUE);
	
	webview->current_page = 0;

	webview->search = g_new0(SearchParams, 1);
	
	webview->search->search_jump = TRUE ;

	webview->fullscreen = FALSE;
}

static void
ev_web_view_finalize (GObject *object)
{
	G_OBJECT_CLASS(ev_web_view_parent_class)->finalize(object);
}

/*** Callbacks ***/

static void
ev_web_view_change_page (EvWebView *webview,
		         gint    new_page)
{
	g_return_if_fail(EV_IS_WEB_VIEW(webview));
	
	EvDocumentClass *klass = EV_DOCUMENT_GET_CLASS(webview->document);

	EvPage *page = klass->get_page(webview->document,new_page);

	webview->current_page = new_page;
	ev_document_model_set_page(webview->model,new_page);
	webkit_web_view_load_uri(WEBKIT_WEB_VIEW(webview),(gchar*)page->backend_page);
}

static void
ev_web_view_page_changed_cb (EvDocumentModel *model,
			     gint             old_page,
			     gint     	      new_page,
			     EvWebView       *webview)
{
	if (!webview->document)
		return;

	if (webview->current_page != new_page) {
		ev_web_view_change_page (webview, new_page);
	} else {
		webkit_web_view_reload (WEBKIT_WEB_VIEW(webview));
	}
}

GtkWidget*
ev_web_view_new (void)
{
	GtkWidget *webview;

	webview = g_object_new (EV_TYPE_WEB_VIEW, NULL);

	return webview;
}

static void
ev_web_view_document_changed_cb (EvDocumentModel *model,
  			         GParamSpec      *pspec,
			         EvWebView       *webview)
{
	g_return_if_fail(EV_IS_WEB_VIEW(webview));
	
	EvDocument *document = ev_document_model_get_document (model);

	if (document != webview->document) {

		if (webview->document )
			g_object_unref(webview->document);

		webview->document = document ;

		if(webview->document) {
			g_object_ref(webview->document);
		}

		gint current_page = ev_document_model_get_page(model);
		
		ev_web_view_change_page (webview, current_page);
		
	}
}       

static void
ev_web_view_inverted_colors_changed_cb (EvDocumentModel *model,
				        GParamSpec      *pspec,
				        EvWebView       *webview)
{
	guint inverted_colors = ev_document_model_get_inverted_colors (model);
	inverted_colors = !inverted_colors;
	/*TODO*/
}

static void
ev_web_view_fullscreen_changed_cb (EvDocumentModel *model,
			           GParamSpec      *pspec,
			           EvWebView       *webview)
{
	gboolean fullscreen = ev_document_model_get_fullscreen (model);

	webview->fullscreen = fullscreen;
#if GTK_CHECK_VERSION (3, 0, 0)
	WebKitWindowProperties *window_properties = 
		webkit_web_view_get_window_properties (WEBKIT_WEB_VIEW(webview));

	webkit_window_properties_get_fullscreen(window_properties);
	/*TODO*/
#else
	webkit_web_view_set_view_mode(WEBKIT_WEB_VIEW(webview), WEBKIT_WEB_VIEW_VIEW_MODE_FULLSCREEN);
#endif
}
void
ev_web_view_set_model (EvWebView          *webview,
		       EvDocumentModel    *model)
{
	g_return_if_fail (EV_IS_WEB_VIEW (webview));
	g_return_if_fail (EV_IS_DOCUMENT_MODEL (model));

	if (model == webview->model)
		return;

	if (webview->model) {
		g_signal_handlers_disconnect_by_func (webview->model,
						      ev_web_view_document_changed_cb,
						      webview);
		g_signal_handlers_disconnect_by_func (webview->model,
						      ev_web_view_page_changed_cb,
						      webview);
		g_object_unref (webview->model);
	}
	webview->model = g_object_ref (model);

	/* Initialize webview from model */
	webview->fullscreen = ev_document_model_get_fullscreen (webview->model);
	webview->document = ev_document_model_get_document(webview->model);
	webview->inverted_colors = ev_document_model_get_inverted_colors(webview->model);
	ev_web_view_document_changed_cb (webview->model, NULL, webview);

	g_signal_connect (webview->model, "notify::document",
			  G_CALLBACK (ev_web_view_document_changed_cb),
			  webview);
	g_signal_connect (webview->model, "notify::inverted-colors",
			  G_CALLBACK (ev_web_view_inverted_colors_changed_cb),
			  webview);
	g_signal_connect (webview->model,"page-changed",
			  G_CALLBACK(ev_web_view_page_changed_cb),
			  webview);
}

void
ev_web_view_reload_page (EvWebView         *webview,
                         gint               page)
{
	webkit_web_view_reload (WEBKIT_WEB_VIEW(webview));
}

void
ev_web_view_reload (EvWebView *webview)
{
	web_view_update_range_and_current_page (webview);
}


gboolean
ev_web_view_next_page (EvWebView *webview)
{
	int page, n_pages;

	g_return_val_if_fail (EV_IS_WEB_VIEW (webview), FALSE);
	
	if (!webview->document)
		return FALSE;

	page = ev_document_model_get_page (webview->model);
	n_pages = ev_document_get_n_pages (webview->document);

	page = page + 1;

	if (page < n_pages) {
		ev_document_model_set_page (webview->model, page);
		EvPage *webpage = ev_document_get_page(webview->document,page);
		webview->current_page = page ;
		webkit_web_view_load_uri(WEBKIT_WEB_VIEW(webview),(gchar*)webpage->backend_page);
		return TRUE;
	} else if (page == n_pages) {
		ev_document_model_set_page (webview->model, page - 1);
		EvPage *webpage = ev_document_get_page(webview->document,page);
		webkit_web_view_load_uri(WEBKIT_WEB_VIEW(webview),(gchar*)webpage->backend_page);
		return TRUE;
	} else {
		return FALSE;
	}
}

gboolean
ev_web_view_previous_page (EvWebView *webview)
{
	int page;

	g_return_val_if_fail (EV_IS_WEB_VIEW (webview), FALSE);

	if (!webview->document)
		return FALSE;

	page = ev_document_model_get_page (webview->model);

	page = page - 1 ;

	if (page >= 0) {
		ev_document_model_set_page (webview->model, page);
		EvPage *webpage = ev_document_get_page(webview->document,page);
		webkit_web_view_load_uri(WEBKIT_WEB_VIEW(webview),(gchar*)webpage->backend_page);
		return TRUE;
	} else if (page == -1) {
		ev_document_model_set_page (webview->model, 0);
		EvPage *webpage = ev_document_get_page(webview->document,page);
		webkit_web_view_load_uri(WEBKIT_WEB_VIEW(webview),(gchar*)webpage->backend_page);
		return TRUE;
	} else {	
		return FALSE;
	}
}
		
void 
ev_web_view_handle_link(EvWebView *webview,EvLink *link) 
{

}

/* Searching */
void 
ev_web_view_find_next(EvWebView *webview)
{
	/*First search for the next item on the current page*/
	 webkit_web_view_search_text (WEBKIT_WEB_VIEW(webview),
                                  webview->search->search_string,
                                  webview->search->case_sensitive,
                                  TRUE,
                                  FALSE);
}

void 
ev_web_view_find_previous(EvWebView *webview)
{
	webkit_web_view_search_text (WEBKIT_WEB_VIEW(webview),
                          		webview->search->search_string,
                          		webview->search->case_sensitive,
                          		FALSE,
                          		TRUE);
}

void
ev_web_view_find_set_highlight_search(EvWebView *webview, gboolean visible)
{
	webkit_web_view_set_highlight_text_matches(WEBKIT_WEB_VIEW(webview),visible);
}

typedef struct _FindCBStruct {
	EvJobFind *job;
	gint page;
}FindCBStruct;

static void
find_page_change_cb(WebKitWebView  *webview,
                    WebKitWebFrame *webframe,
                    FindCBStruct   *findcbs)
{
	findcbs->job->results[findcbs->page] = webkit_web_view_mark_text_matches(WEBKIT_WEB_VIEW(webview),
	                                                                         findcbs->job->text,
	                                                                         findcbs->job->case_sensitive,
	                                                                         0);
	ev_web_view_find_set_highlight_search(webview, TRUE);
	
	webkit_web_view_search_text (WEBKIT_WEB_VIEW(webview),
	                             findcbs->job->text,
	                             findcbs->job->case_sensitive,
                                 TRUE,
                                 FALSE);
}
void
ev_web_view_find_changed(EvWebView *webview, gint page_found_on,EvJobFind *job)
{
	if (job->has_results == FALSE)
		return;
	
	if (webview->search->search_jump == TRUE) {

		webview->search->on_result = 1;
		webview->search->case_sensitive = job->case_sensitive;
		webview->search->search_string = g_strdup(job->text);
		webview->search->search_jump = FALSE;
		
		if (page_found_on != webview->current_page) {
			ev_web_view_change_page(webview, page_found_on);

			FindCBStruct *findstruct = g_new0 (FindCBStruct, 1);
			findstruct->job = job;
			findstruct->page = page_found_on;
			
			g_signal_connect(WEBKIT_WEB_VIEW(webview),"document-load-finished",G_CALLBACK(find_page_change_cb),findstruct);
		}
		else {
			job->results[webview->current_page] = webkit_web_view_mark_text_matches(WEBKIT_WEB_VIEW(webview),
			                                                                        job->text,
			                                                                        job->case_sensitive,
			                                                                        0);

				ev_web_view_find_set_highlight_search(webview, TRUE);
		}
	}
}

void
ev_web_view_find_search_changed(EvWebView *webview)
{
	ev_web_view_find_set_highlight_search(webview,FALSE);
	webkit_web_view_unmark_text_matches (WEBKIT_WEB_VIEW(webview));
	webview->search->search_jump = TRUE;
}

void
ev_web_view_find_cancel(EvWebView *webview)
{
	ev_web_view_find_set_highlight_search(webview,FALSE);
	webkit_web_view_unmark_text_matches (WEBKIT_WEB_VIEW(webview));
}

void
ev_web_view_empty_search(EvWebView *webview)
{
	SearchParams *search = webview->search ;
	search->case_sensitive = FALSE;
	if (search->search_string) 
		g_free(search->search_string);
	search->search_string = NULL;
	search->search_jump = TRUE ;
}

/* Selection */
gboolean 
ev_web_view_get_has_selection(EvWebView *webview)
{
	return webkit_web_view_has_selection(WEBKIT_WEB_VIEW(webview));
}

void
ev_web_view_select_all(EvWebView *webview)
{
	webkit_web_view_select_all(WEBKIT_WEB_VIEW(webview));
}

void
ev_web_view_copy(EvWebView *webview)
{
		/* If for some reason we don't have a selection any longer,best to be safe*/
		if (ev_web_view_get_has_selection(webview) == FALSE)
			return;
		if (webkit_web_view_can_copy_clipboard(WEBKIT_WEB_VIEW(webview))) {
			webkit_web_view_copy_clipboard(WEBKIT_WEB_VIEW(webview));
		}
	
}

gboolean
ev_web_view_zoom_in(EvWebView *webview)
{
	webkit_web_view_zoom_in(WEBKIT_WEB_VIEW(webview));
	return TRUE;
}

gboolean
ev_web_view_zoom_out(EvWebView *webview)
{
	webkit_web_view_zoom_out(WEBKIT_WEB_VIEW(webview));
	return TRUE;
}