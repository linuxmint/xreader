/* this file is part of xreader, a mate document viewer
 *
 *  Copyright (C) 2017 Mickael Albertus
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
#include "config.h"

#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include "ev-recent-view.h"
#include "ev-document-misc.h"
#include "ev-document-model.h"
#include "ev-jobs.h"
#include "ev-job-scheduler.h"

struct _EvRecentViewPrivate {
        GtkIconView      *view;
        GtkListStore     *model;
        GtkTreePath      *pressed_item_tree_path;
        guint             recent_manager_changed_handler_id;
};

typedef enum {
		EV_RECENT_VIEW_COLUMN_PIXBUF = 0,
		EV_RECENT_VIEW_COLUMN_NAME,
        EV_RECENT_VIEW_COLUMN_URI,
        NUM_COLUMNS
} EvRecentViewColumns;

enum {
        ITEM_ACTIVATED,
        NUM_SIGNALS
};

static guint signals[NUM_SIGNALS] = { 0, };

G_DEFINE_TYPE (EvRecentView, ev_recent_view, GTK_TYPE_SCROLLED_WINDOW)

#define THUMBNAIL_WIDTH 80
#define MAX_RECENT_VIEW_ITEMS 20

static void
ev_recent_view_clear_model (EvRecentView *ev_recent_view)
{
        EvRecentViewPrivate *priv = ev_recent_view->priv;
        gtk_list_store_clear (priv->model);
}

static gint
compare_recent_items (GtkRecentInfo *a,
                      GtkRecentInfo *b)
{
        gboolean     has_ev_a, has_ev_b;
        const gchar *evince = g_get_application_name ();

        has_ev_a = gtk_recent_info_has_application (a, evince);
        has_ev_b = gtk_recent_info_has_application (b, evince);

        if (has_ev_a && has_ev_b) {
                time_t time_a, time_b;

                time_a = gtk_recent_info_get_modified (a);
                time_b = gtk_recent_info_get_modified (b);

                return (time_b - time_a);
        } else if (has_ev_a) {
                return -1;
        } else if (has_ev_b) {
                return 1;
        }

        return 0;
}

static void
thumbnail_job_completed_callback (EvJobThumbnail *job,
				  				  EvRecentView *ev_recent_view)
{
	GtkTreeIter 		*iter = (GtkTreeIter *) g_object_get_data (G_OBJECT (job), "tree_iter");
	EvRecentViewPrivate *priv = ev_recent_view->priv;
	
	gtk_list_store_set (priv->model,
			    		iter,
			    		0, job->thumbnail,
			    		-1);
}

static void
on_icon_view_item_activated (GtkIconView  *iconview,
                             GtkTreePath  *path,
                             EvRecentView *ev_recent_view)
{
        EvRecentViewPrivate *priv = ev_recent_view->priv;
        GtkTreeIter          iter;
        gchar               *uri;

        if (!gtk_tree_model_get_iter (GTK_TREE_MODEL (priv->model), &iter, path))
                return;

        gtk_tree_model_get (GTK_TREE_MODEL (priv->model), &iter,
                            EV_RECENT_VIEW_COLUMN_URI, &uri,
                            -1);
        g_signal_emit (ev_recent_view, signals[ITEM_ACTIVATED], 0, uri);
        g_free (uri);
}

static void
ev_recent_view_refresh (EvRecentView *ev_recent_view)
{
	GList *items, *l;
	GtkRecentManager 	*recent_manager = gtk_recent_manager_get_default ();
	const gchar 	 	*xreader = g_get_application_name ();
	EvRecentViewPrivate *priv = ev_recent_view->priv;

	
	items = gtk_recent_manager_get_items (recent_manager);
	items = g_list_sort (items, (GCompareFunc) compare_recent_items);
	
	for (l = items; l && l->data; l = g_list_next (l)) {
		GtkRecentInfo            *info;
		const gchar              *name, *uri;
		GdkPixbuf                *pixbuf;
		GtkTreeIter               iter;
		
		info = (GtkRecentInfo *) l->data;

		if (!gtk_recent_info_has_application (info, xreader))
		        continue;

		if (gtk_recent_info_is_local (info) && !gtk_recent_info_exists (info))
		        continue;

		name = gtk_recent_info_get_display_name (info);
		uri = gtk_recent_info_get_uri (info);
		pixbuf = gtk_recent_info_get_icon (info, THUMBNAIL_WIDTH);
		gtk_list_store_append (priv->model, &iter);
		gtk_list_store_set (priv->model, &iter, 
							EV_RECENT_VIEW_COLUMN_PIXBUF, pixbuf, 
							EV_RECENT_VIEW_COLUMN_NAME, name, 
							EV_RECENT_VIEW_COLUMN_URI, uri,
							-1);
		
		
		EvDocument *document = ev_document_factory_get_document (uri, NULL);
		if (document) {
			gdouble width;
			if (document->iswebdocument == TRUE ) {
				/* Hardcoded for epub documents*/
				width = 800;
			} else {
				ev_document_get_page_size (document, 0, &width, NULL);
			}
			
			width = (gdouble)THUMBNAIL_WIDTH / width;
			EvJobThumbnail *job = ev_job_thumbnail_new (document, 0, 0, width);
			g_object_set_data_full (G_OBJECT (job), "tree_iter",
						gtk_tree_iter_copy (&iter),
						(GDestroyNotify) gtk_tree_iter_free);
			g_signal_connect (job, "finished",
					  G_CALLBACK (thumbnail_job_completed_callback),
					  ev_recent_view);
			ev_job_scheduler_push_job (EV_JOB (job), EV_JOB_PRIORITY_HIGH);
			

			/* The queue and the list own a ref to the job now */
			g_object_unref (job);
		}
	}
}


static void
ev_recent_view_init (EvRecentView *ev_recent_view)
{
    EvRecentViewPrivate *priv;
    ev_recent_view->priv = G_TYPE_INSTANCE_GET_PRIVATE (ev_recent_view, EV_TYPE_RECENT_VIEW, EvRecentViewPrivate);
    
    priv = ev_recent_view->priv;
    priv->model = gtk_list_store_new (NUM_COLUMNS, 
    								  GDK_TYPE_PIXBUF, 
    								  G_TYPE_STRING,
    								  G_TYPE_STRING);
}

static void
ev_recent_view_constructed (GObject *object)
{
	EvRecentView        *ev_recent_view = EV_RECENT_VIEW (object);
	EvRecentViewPrivate *priv = ev_recent_view->priv;
	
	G_OBJECT_CLASS (ev_recent_view_parent_class)->constructed (object);
	
    priv->view = gtk_icon_view_new_with_model (GTK_TREE_MODEL (priv->model));

    gtk_widget_set_hexpand (GTK_WIDGET (ev_recent_view), TRUE);
    gtk_widget_set_vexpand (GTK_WIDGET (ev_recent_view), TRUE);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (ev_recent_view),
                                    GTK_POLICY_NEVER,
                                    GTK_POLICY_AUTOMATIC);
                                    
	gtk_icon_view_set_item_orientation (priv->view, GTK_ORIENTATION_HORIZONTAL);
	gtk_icon_view_set_activate_on_single_click (priv->view, TRUE);
	gtk_icon_view_set_pixbuf_column (priv->view, 0);
	gtk_icon_view_set_text_column (priv->view, 1);
	
	g_signal_connect (priv->view, "item-activated",
                          G_CALLBACK (on_icon_view_item_activated),
                          ev_recent_view);
	
	gtk_container_add (GTK_CONTAINER (ev_recent_view), priv->view);
	gtk_widget_show (priv->view);
	
	ev_recent_view_refresh (ev_recent_view);
}

static void
ev_recent_view_dispose (GObject *obj)
{
        EvRecentView        *ev_recent_view = EV_RECENT_VIEW (obj);
        EvRecentViewPrivate *priv = ev_recent_view->priv;

        if (priv->model) {
                ev_recent_view_clear_model (ev_recent_view);
                g_object_unref (priv->model);
                priv->model = NULL;
        }

        G_OBJECT_CLASS (ev_recent_view_parent_class)->dispose (obj);
}

static void
ev_recent_view_class_init (EvRecentViewClass *klass)
{
        GObjectClass *g_object_class = G_OBJECT_CLASS (klass);
        g_object_class->constructed = ev_recent_view_constructed;
        g_object_class->dispose = ev_recent_view_dispose;
        
        signals[ITEM_ACTIVATED] =
                  g_signal_new ("item-activated",
                                EV_TYPE_RECENT_VIEW,
                                G_SIGNAL_RUN_LAST,
                                0, NULL, NULL,
                                g_cclosure_marshal_generic,
                                G_TYPE_NONE, 1,
                                G_TYPE_STRING);
                                
        g_type_class_add_private (klass, sizeof (EvRecentViewPrivate));
}


GtkWidget *
ev_recent_view_new (void)
{
        return GTK_WIDGET (g_object_new (EV_TYPE_RECENT_VIEW, NULL));
}
