/*
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
#include "ev-window.h"

struct _EvRecentViewPrivate {
    GtkWidget        *view;
    GtkRecentManager *recent_manager;
    guint             recent_manager_changed_handler_id;
    GtkWidget        *grid_view;
    GtkWidget        *no_recents_view;
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

G_DEFINE_TYPE_WITH_PRIVATE (EvRecentView, ev_recent_view, GTK_TYPE_SCROLLED_WINDOW)

#define THUMBNAIL_WIDTH 80

static gint
compare_recent_items (GtkRecentInfo *a,
                      GtkRecentInfo *b)
{
    gboolean has_ev_a, has_ev_b;
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
destroy_child (GtkWidget *child,
               gpointer   data)
{
    gtk_container_remove (GTK_CONTAINER (data), child);
}

static gchar *
format_name (const gchar *name)
{
    GString *str = g_string_new (name);
    guint length = strlen (name);

    if (length <= 32)
        return (gchar *) name;

    g_string_erase (str, 20, length-32);
    g_string_insert (str, 20, "...");
    return g_string_free (str, FALSE);
}

static void
thumbnail_job_completed_callback (EvJobThumbnail *job,
                                  GtkButton      *button)
{
    if (!ev_job_is_failed (EV_JOB (job))) {
        GdkPixbuf *pixbuf = ev_document_misc_render_thumbnail_with_frame(GTK_WIDGET(button), job->thumbnail);
        gtk_button_set_image (button, gtk_image_new_from_pixbuf(pixbuf));
    }
}

static void
on_item_activated (GtkButton    *button,
                   EvRecentView *ev_recent_view)
{
    gchar *uri = (gchar *) g_object_get_data (G_OBJECT (button), "uri");
    g_signal_emit (ev_recent_view, signals[ITEM_ACTIVATED], 0, uri);
}

static void
ev_recent_view_clear (EvRecentView *ev_recent_view)
{
    EvRecentViewPrivate *priv = ev_recent_view->priv;

    gtk_container_foreach (GTK_CONTAINER (priv->grid_view), (GtkCallback) destroy_child, priv->grid_view);
}

static void
ev_recent_view_refresh (EvRecentView *ev_recent_view)
{
    GList *items, *l;
    const gchar *xreader = g_get_application_name ();
    EvRecentViewPrivate *priv = ev_recent_view->priv;

    ev_recent_view_clear (ev_recent_view);

    items = gtk_recent_manager_get_items (priv->recent_manager);
    items = g_list_sort (items, (GCompareFunc) compare_recent_items);

    if (g_list_length(items) == 0) {
        gtk_stack_set_visible_child_name(GTK_STACK(priv->view), "no_recent_files");
        return;
    }

    for (l = items; l && l->data; l = g_list_next (l)) {
        GtkRecentInfo *info;
        gchar *name;
        const gchar *uri;
        GdkPixbuf *pixbuf;
        GtkWidget *button;
        EvDocument *document;

        info = (GtkRecentInfo *) l->data;

        if (!gtk_recent_info_has_application (info, xreader))
            continue;

        if (gtk_recent_info_is_local (info) && !gtk_recent_info_exists (info))
            continue;

        name = format_name (gtk_recent_info_get_display_name (info));
        uri = gtk_recent_info_get_uri (info);
        pixbuf = gtk_recent_info_get_icon (info, THUMBNAIL_WIDTH);

        button = gtk_button_new ();
        gtk_style_context_add_class (gtk_widget_get_style_context (button), GTK_STYLE_CLASS_FLAT);

        gtk_button_set_label (GTK_BUTTON (button), name);
        gtk_button_set_always_show_image (GTK_BUTTON (button), TRUE);
        gtk_button_set_image (GTK_BUTTON (button), gtk_image_new_from_pixbuf (pixbuf));
        gtk_button_set_image_position (GTK_BUTTON (button), GTK_POS_TOP);
        g_free (name);

        gtk_container_add (GTK_CONTAINER (priv->grid_view), button);
        gtk_widget_show (button);

        g_object_set_data (G_OBJECT (button), "uri", (gpointer) uri);
        g_signal_connect (button, "clicked", G_CALLBACK (on_item_activated), ev_recent_view);

        document = ev_document_factory_get_document (uri, NULL);

        if (document) {
            gdouble width;
            EvJob *job;

            if (document->iswebdocument == TRUE ) {
                width = 800;
            } else {
                ev_document_get_page_size (document, 0, &width, NULL);
            }

            width = (gdouble)THUMBNAIL_WIDTH / width;
            job = ev_job_thumbnail_new (document, 0, 0, width);
            if (document->iswebdocument) {
                ev_job_set_run_mode(EV_JOB(job), EV_JOB_RUN_MAIN_LOOP);
            }
            g_signal_connect (job, "finished", G_CALLBACK (thumbnail_job_completed_callback), button);
            ev_job_scheduler_push_job (EV_JOB (job), EV_JOB_PRIORITY_HIGH);

            g_object_unref (job);
        }
    }
    gtk_stack_set_visible_child_name(GTK_STACK(priv->view), "recent_files");
}


static void
ev_recent_view_init (EvRecentView *ev_recent_view)
{
    ev_recent_view->priv = ev_recent_view_get_instance_private (ev_recent_view);

    gtk_widget_set_hexpand (GTK_WIDGET (ev_recent_view), TRUE);
    gtk_widget_set_vexpand (GTK_WIDGET (ev_recent_view), TRUE);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (ev_recent_view), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
}


static void
ev_recent_view_constructed (GObject *object)
{
    EvRecentView *ev_recent_view = EV_RECENT_VIEW (object);
    EvRecentViewPrivate *priv = ev_recent_view->priv;

    G_OBJECT_CLASS (ev_recent_view_parent_class)->constructed (object);

    priv->view = gtk_stack_new();

    priv->grid_view = gtk_flow_box_new();
    gtk_widget_set_valign (priv->grid_view, GTK_ALIGN_START);
    gtk_flow_box_set_activate_on_single_click (GTK_FLOW_BOX (priv->grid_view), TRUE);
    gtk_flow_box_set_selection_mode (GTK_FLOW_BOX (priv->grid_view), GTK_SELECTION_NONE);
    gtk_flow_box_set_homogeneous (GTK_FLOW_BOX (priv->grid_view), TRUE);
    gtk_widget_set_margin_start (priv->grid_view, 6);
    gtk_widget_set_margin_end (priv->grid_view, 6);
    gtk_widget_set_margin_top (priv->grid_view, 6);
    gtk_widget_set_margin_bottom (priv->grid_view, 6);

    priv->no_recents_view = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);

    gtk_widget_set_can_focus(priv->no_recents_view, FALSE);
    gtk_widget_set_valign(priv->no_recents_view, GTK_ALIGN_CENTER);
    gtk_widget_set_halign(priv->no_recents_view, GTK_ALIGN_FILL);

    GtkWidget *no_recents_image = gtk_image_new_from_icon_name("document-open-recent", GTK_ICON_SIZE_DIALOG);
    gtk_image_set_pixel_size(GTK_IMAGE(no_recents_image), 96);
    gtk_box_pack_start(GTK_BOX(priv->no_recents_view), no_recents_image, FALSE, TRUE, 0);
    gtk_widget_show(no_recents_image);

    GtkWidget *label = gtk_label_new(_("No recent documents"));
    PangoAttribute *attrib_size = pango_attr_scale_new(1.05);
    PangoAttribute *attrib_bold = pango_attr_weight_new(PANGO_WEIGHT_BOLD);
    PangoAttrList *attribs = pango_attr_list_new();
    pango_attr_list_insert(attribs, attrib_size);
    pango_attr_list_insert(attribs, attrib_bold);
    gtk_label_set_attributes(GTK_LABEL(label), attribs);
    gtk_box_pack_start(GTK_BOX(priv->no_recents_view), label, FALSE, FALSE, 5);
    gtk_widget_set_halign(label, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(label, GTK_ALIGN_START);
    gtk_widget_show(label);

    gtk_stack_add_named(GTK_STACK(priv->view), priv->no_recents_view, "no_recent_files");
    gtk_stack_add_named(GTK_STACK(priv->view), priv->grid_view, "recent_files");

    gtk_stack_set_visible_child_name(GTK_STACK(priv->view), "no_recent_files");

    gtk_widget_show(priv->grid_view);
    gtk_widget_show(priv->no_recents_view);

    gtk_container_add (GTK_CONTAINER (ev_recent_view), priv->view);
    gtk_widget_show (priv->view);

    priv->recent_manager = gtk_recent_manager_get_default ();
    priv->recent_manager_changed_handler_id =
            g_signal_connect_swapped (priv->recent_manager,
                                      "changed",
                                      G_CALLBACK (ev_recent_view_refresh),
                                      ev_recent_view);

    ev_recent_view_refresh (ev_recent_view);

}

static void
ev_recent_view_dispose (GObject *obj)
{
    EvRecentView *ev_recent_view = EV_RECENT_VIEW (obj);
    EvRecentViewPrivate *priv = ev_recent_view->priv;

    if (priv->recent_manager_changed_handler_id) {
        g_signal_handler_disconnect (priv->recent_manager, priv->recent_manager_changed_handler_id);
        priv->recent_manager_changed_handler_id = 0;
    }

    priv->recent_manager = NULL;

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
}


GtkWidget *
ev_recent_view_new (void)
{
    return GTK_WIDGET (g_object_new (EV_TYPE_RECENT_VIEW, NULL));
}
