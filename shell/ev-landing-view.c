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
#include <libxapp/xapp-favorites.h>

#include "ev-application.h"
#include "ev-landing-view.h"
#include "ev-document-misc.h"
#include "ev-document-model.h"
#include "ev-jobs.h"
#include "ev-job-scheduler.h"
#include "ev-window.h"
#include "ev-utils.h"

struct _EvLandingViewPrivate {
    XAppFavorites    *favorites;
    GtkRecentManager *recent_manager;

    GSettings        *state_settings;

    GtkWidget        *view;
    GtkWidget        *stack_switcher;

    GtkWidget        *recents_sw;
    GtkWidget        *recent_grid_view;
    GtkWidget        *favorites_sw;
    GtkWidget        *favorites_grid_view;
    GtkWidget        *no_files_view;

    guint             favorites_changed_handler_id;
    guint             recent_manager_changed_handler_id;
};

typedef enum {
    EV_LANDING_VIEW_COLUMN_PIXBUF = 0,
    EV_LANDING_VIEW_COLUMN_NAME,
    EV_LANDING_VIEW_COLUMN_URI,
    NUM_COLUMNS
} EvLandingViewColumns;

enum {
    ITEM_ACTIVATED,
    NUM_SIGNALS
};

static guint signals[NUM_SIGNALS] = { 0, };

G_DEFINE_TYPE_WITH_PRIVATE (EvLandingView, ev_landing_view, GTK_TYPE_BOX)

#define THUMBNAIL_WIDTH 80
#define MAX_NBR_CARACTERS 30

#define STATE_SETTINGS "org.x.reader.state"
#define LAST_USED_STACK_PAGE_KEY "last-landing-page"

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
format_name (const gchar *str)
{
    gchar *escaped_str;

	if (g_utf8_strlen (str, MAX_NBR_CARACTERS*2+1) > MAX_NBR_CARACTERS) {
		gchar *truncated_str;
		gchar *str2;

		/* allocate number of bytes and not number of utf-8 chars */
		str2 = g_malloc ((MAX_NBR_CARACTERS-1) * 2 * sizeof(gchar));

		g_utf8_strncpy (str2, str, MAX_NBR_CARACTERS-2);
		truncated_str = g_strdup_printf ("%s..", str2);
		escaped_str = g_markup_escape_text (truncated_str, strlen (truncated_str));
		g_free (str2);
		g_free (truncated_str);
	} else {
		escaped_str = g_markup_escape_text (str, strlen (str));
	}

	return escaped_str;
}

static void
thumbnail_job_completed_callback (EvJobThumbnail *job,
                                  GtkImage       *image)
{
    if (!ev_job_is_failed (EV_JOB (job))) {
        GdkPixbuf *pixbuf = ev_document_misc_render_thumbnail_with_frame(GTK_WIDGET(image), job->thumbnail);
        gtk_image_set_from_pixbuf (image, pixbuf);
    }
}

static void
on_item_activated (GtkButton    *button,
                   EvLandingView *ev_landing_view)
{
    gchar *uri = (gchar *) g_object_get_data (G_OBJECT (button), "uri");
    g_signal_emit (ev_landing_view, signals[ITEM_ACTIVATED], 0, uri);
}

static void
select_view (EvLandingView *ev_landing_view,
             const gchar   *desired_view_name)
{
    EvLandingViewPrivate *priv = ev_landing_view->priv;
    GtkWidget *page;
    gchar *desired_title, *favorite_title, *recent_title;
    gchar *real_desired_view_name;
    const gchar *name_to_use;
    gboolean use_fallback;

    // If desired name is null, it's been called after a favorites or recents update.
    // We want to try and show the already visible page - if it's suddenly empty, this
    // will trigger a switch to another available one. Don't ever *try* to select the
    // empty view, however, so if that's the current page, try to select the last-used.

    real_desired_view_name = NULL;

    if (desired_view_name == NULL) {
        const gchar *current_active_name = gtk_stack_get_visible_child_name (GTK_STACK (priv->view));

        if (g_strcmp0 (current_active_name, "no_files") == 0) {
            real_desired_view_name = g_settings_get_string (priv->state_settings, LAST_USED_STACK_PAGE_KEY);
        } else {
            real_desired_view_name = g_strdup (current_active_name);
        }
    } else
    {
        real_desired_view_name = g_strdup (desired_view_name);
    }

    // Select the desired view only if it's titled (button is visible in the  switcher
    // If both content views are empty, switch to the no_files view.

    // Switch to the desired view if it is titled (visible)
    page = gtk_stack_get_child_by_name (GTK_STACK (priv->view), real_desired_view_name);
    gtk_container_child_get (GTK_CONTAINER (priv->view), page,
                             "title", &desired_title,
                             NULL);

    if (desired_title != NULL) {
        gtk_stack_set_visible_child_name (GTK_STACK (priv->view), real_desired_view_name);
        g_free (real_desired_view_name);
        g_free (desired_title);
        return;
    }

    // If it's not, find the next best thing (either the other content view, or no_files
    // if that's also empty.)
    page = gtk_stack_get_child_by_name (GTK_STACK (priv->view), "recent_files");
    gtk_container_child_get (GTK_CONTAINER (priv->view), page,
                             "title", &recent_title,
                             NULL);
    page = gtk_stack_get_child_by_name (GTK_STACK (priv->view), "favorite_files");
    gtk_container_child_get (GTK_CONTAINER (priv->view), page,
                             "title", &favorite_title,
                             NULL);

    name_to_use = NULL;

    // Can't use the desired view, use the other visible view or fall back to no_files
    if (g_strcmp0 (real_desired_view_name, "recent_files") == 0) {
        if (favorite_title != NULL) {
            name_to_use = "favorite_files";
        }
    }

    if (name_to_use == NULL && g_strcmp0 (real_desired_view_name, "favorite_files") == 0) {
        if (recent_title != NULL) {
            name_to_use = "recent_files";
        }
    }

    if (name_to_use == NULL) {
        name_to_use = "no_files";
    }

    gtk_stack_set_visible_child_name (GTK_STACK (priv->view), name_to_use);

    g_free (real_desired_view_name);
    g_free (favorite_title);
    g_free (recent_title);
}

static cairo_surface_t *
surface_for_mimetype (GtkIconTheme *theme,
                      const gchar  *mime_type,
                      gint          scale_factor)
{
    GIcon *gicon;
    GtkIconInfo *icon_info;
    GdkPixbuf *pixbuf;
    cairo_surface_t *surface;
    gchar *content_type;

    content_type = g_content_type_from_mime_type (mime_type);
    gicon = g_content_type_get_icon (content_type);
    g_free (content_type);

    icon_info = gtk_icon_theme_lookup_by_gicon_for_scale (theme,
                                                          gicon,
                                                          THUMBNAIL_WIDTH,
                                                          scale_factor,
                                                          GTK_ICON_LOOKUP_FORCE_SIZE);

    g_object_unref (gicon);

    if (icon_info != NULL) {

        pixbuf = gtk_icon_info_load_icon (icon_info, NULL);
        g_object_unref (icon_info);

        if (pixbuf) {
            surface = gdk_cairo_surface_create_from_pixbuf (pixbuf, scale_factor, NULL);
            g_object_unref (pixbuf);
        }

        return surface;
    }

    return NULL;
}

static void
ev_landing_view_refresh_favorites (EvLandingView *ev_landing_view)
{
    EvLandingViewPrivate *priv = ev_landing_view->priv;
    GtkIconTheme *theme;
    gint scale_factor;
    GList *items, *l;
    gboolean any_items = FALSE;

    gtk_container_foreach (GTK_CONTAINER (priv->favorites_grid_view),
                           (GtkCallback) destroy_child,
                           priv->favorites_grid_view);

    items = xapp_favorites_get_favorites (priv->favorites, (const gchar **) supported_mimetypes); // global from ev-application

    theme = gtk_icon_theme_get_default ();
    scale_factor = gtk_widget_get_scale_factor (priv->favorites_grid_view);

    for (l = items; l != NULL; l = l->next) {
        XAppFavoriteInfo *info;
        GIcon *gicon;
        GtkIconInfo *icon_info;
        GFile *file;
        gchar *name;
        const gchar *uri;
        gchar *content_type;
        cairo_surface_t *surface;
        GtkWidget *button;
        GtkWidget *image;
        GtkWidget *box;
        GtkWidget *label;
        EvDocument *document;

        info = (XAppFavoriteInfo *) l->data;

        file = g_file_new_for_uri (info->uri);

        if (g_file_is_native (file))
        {
            gchar *path = g_file_get_path (file);
            gboolean exists = FALSE;

            if (g_file_test (path, G_FILE_TEST_EXISTS))
            {
                exists = TRUE;
            }

            g_free (path);

            if (!exists)
            {
                g_object_unref (file);
                continue;
            }
        }

        g_object_unref (file);

        any_items = TRUE;

        name = format_name (info->display_name);

        button = gtk_button_new ();
        box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 6);
        gtk_container_add (GTK_CONTAINER (button), box);
        gtk_style_context_add_class (gtk_widget_get_style_context (button), GTK_STYLE_CLASS_FLAT);

        label = gtk_label_new (name);
        gtk_box_pack_end (GTK_BOX (box), label, FALSE, FALSE, 0);
        g_free (name);

        surface = surface_for_mimetype (theme, info->cached_mimetype, scale_factor);

        if (surface != NULL) {
            image = gtk_image_new_from_surface (surface);
            cairo_surface_destroy (surface);
        } else {
            image = gtk_image_new_from_icon_name ("unknown", GTK_ICON_SIZE_DND);
        }

        gtk_box_pack_start (GTK_BOX (box), image, TRUE, TRUE, 0);
        gtk_container_add (GTK_CONTAINER (priv->favorites_grid_view), button);
        gtk_widget_show_all (button);

        g_object_set_data_full (G_OBJECT (button),
                                "uri", (gpointer) g_strdup (info->uri),
                                (GDestroyNotify) g_free);
        g_signal_connect (button, "clicked", G_CALLBACK (on_item_activated), ev_landing_view);

        document = ev_document_factory_get_document (info->uri, NULL);

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
            g_signal_connect (job, "finished", G_CALLBACK (thumbnail_job_completed_callback), image);
            ev_job_scheduler_push_job (EV_JOB (job), EV_JOB_PRIORITY_HIGH);

            g_object_unref (job);
            g_object_unref (document);
        }
    }

    gtk_container_child_set (GTK_CONTAINER (priv->view),
                             priv->favorites_sw,
                             "title", any_items ? _("Favorites") : NULL,
                             NULL);

    g_list_free_full (items, (GDestroyNotify) xapp_favorite_info_free);
    select_view (ev_landing_view, NULL);
}

static void
ev_landing_view_refresh_recents (EvLandingView *ev_landing_view)
{
    GList *items, *l;
    const gchar *xreader = g_get_application_name ();
    EvLandingViewPrivate *priv = ev_landing_view->priv;
    GtkIconTheme *theme;
    gint scale_factor;
    gboolean any_items = FALSE;

    gtk_container_foreach (GTK_CONTAINER (priv->recent_grid_view),
                           (GtkCallback) destroy_child,
                           priv->recent_grid_view);

    items = gtk_recent_manager_get_items (priv->recent_manager);
    items = g_list_sort (items, (GCompareFunc) compare_recent_items);

    theme = gtk_icon_theme_get_default ();
    scale_factor = gtk_widget_get_scale_factor (priv->favorites_grid_view);

    for (l = items; l != NULL; l = l->next) {
        GtkRecentInfo *info;
        gchar *name;
        const gchar *uri;
        GdkPixbuf *pixbuf;
        GtkWidget *button;
        GtkWidget *image;
        GtkWidget *box;
        GtkWidget *label;
        cairo_surface_t *surface;
        EvDocument *document;
        gchar *content_type;

        info = (GtkRecentInfo *) l->data;
        uri = gtk_recent_info_get_uri (info);

        if (!gtk_recent_info_has_application (info, xreader))
            continue;

        if (gtk_recent_info_is_local (info) && !gtk_recent_info_exists (info))
            continue;

        any_items = TRUE;

        name = format_name (gtk_recent_info_get_display_name (info));

        button = gtk_button_new ();
        box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 6);
        gtk_container_add (GTK_CONTAINER (button), box);
        gtk_style_context_add_class (gtk_widget_get_style_context (button), GTK_STYLE_CLASS_FLAT);

        label = gtk_label_new (name);
        gtk_box_pack_end (GTK_BOX (box), label, FALSE, FALSE, 0);
        g_free (name);

        surface = surface_for_mimetype (theme, gtk_recent_info_get_mime_type (info), scale_factor);

        if (surface != NULL) {
            image = gtk_image_new_from_surface (surface);
            cairo_surface_destroy (surface);
        } else {
            image = gtk_image_new_from_icon_name ("unknown", GTK_ICON_SIZE_DND);
        }

        gtk_box_pack_start (GTK_BOX (box), image, TRUE, TRUE, 0);

        gtk_container_add (GTK_CONTAINER (priv->recent_grid_view), button);
        gtk_widget_show_all (button);

        g_object_set_data_full (G_OBJECT (button),
                                "uri", (gpointer) g_strdup (uri),
                                (GDestroyNotify) g_free);
        g_signal_connect (button, "clicked", G_CALLBACK (on_item_activated), ev_landing_view);

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
            g_signal_connect (job, "finished", G_CALLBACK (thumbnail_job_completed_callback), image);
            ev_job_scheduler_push_job (EV_JOB (job), EV_JOB_PRIORITY_HIGH);

            g_object_unref (job);
            g_object_unref (document);
        }
    }

    // This controls switcher button visibility.
    gtk_container_child_set (GTK_CONTAINER (priv->view),
                             priv->recents_sw,
                             "title", any_items ? _("Recent files") : NULL,
                             NULL);

    g_list_free_full (items, (GDestroyNotify) gtk_recent_info_unref);
    select_view (ev_landing_view, NULL);
}

static void
ev_landing_view_init (EvLandingView *ev_landing_view)
{
    ev_landing_view->priv = ev_landing_view_get_instance_private (ev_landing_view);

    ev_landing_view->priv->state_settings = g_settings_new (STATE_SETTINGS);

    gtk_widget_set_hexpand (GTK_WIDGET (ev_landing_view), TRUE);
    gtk_widget_set_vexpand (GTK_WIDGET (ev_landing_view), TRUE);
    gtk_orientable_set_orientation (GTK_ORIENTABLE (ev_landing_view), GTK_ORIENTATION_VERTICAL);
}

static GtkWidget *
add_grid_view_page (GtkStack     *stack,
                    const gchar  *page_name,
                    GtkWidget   **scrolled_window)
{
    GtkWidget *sw = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

    GtkWidget *flowbox = gtk_flow_box_new ();

    gtk_widget_set_valign (flowbox, GTK_ALIGN_START);
    gtk_flow_box_set_activate_on_single_click (GTK_FLOW_BOX (flowbox), TRUE);
    gtk_flow_box_set_selection_mode (GTK_FLOW_BOX (flowbox), GTK_SELECTION_NONE);
    gtk_flow_box_set_homogeneous (GTK_FLOW_BOX (flowbox), TRUE);
    gtk_widget_set_margin_start (flowbox, 6);
    gtk_widget_set_margin_end (flowbox, 6);
    gtk_widget_set_margin_top (flowbox, 6);
    gtk_widget_set_margin_bottom (flowbox, 6);

    gtk_container_add (GTK_CONTAINER (sw), flowbox);

    gtk_stack_add_named (stack, sw, page_name);

    gtk_widget_show_all (sw);

    *scrolled_window = sw;
    return flowbox;
}

static void
active_page_changed (EvLandingView *ev_landing_view)
{
    EvLandingViewPrivate *priv = ev_landing_view->priv;
    const gchar *visible_child_name;

    visible_child_name = gtk_stack_get_visible_child_name (GTK_STACK (priv->view));

    // Don't update last page if we've ended up with the no_files_view - the user can't
    // explicitly navigate to this.
    if (g_strcmp0 (visible_child_name, "no_files") == 0) {
        return;
    }

    g_settings_set_string (priv->state_settings,
                           LAST_USED_STACK_PAGE_KEY,
                           visible_child_name);
}

static void
ev_landing_view_constructed (GObject *object)
{
    EvLandingView *ev_landing_view = EV_LANDING_VIEW (object);
    EvLandingViewPrivate *priv = ev_landing_view->priv;
    gchar *saved_view;

    G_OBJECT_CLASS (ev_landing_view_parent_class)->constructed (object);

    priv->stack_switcher = gtk_stack_switcher_new ();
    gtk_widget_show (priv->stack_switcher);
    gtk_widget_set_halign (priv->stack_switcher, GTK_ALIGN_CENTER);
    gtk_widget_set_margin_top (priv->stack_switcher, 10);

    gtk_box_pack_start (GTK_BOX (ev_landing_view), priv->stack_switcher, FALSE, FALSE, 0);
    priv->view = gtk_stack_new();
    gtk_stack_set_transition_type (GTK_STACK (priv->view), GTK_STACK_TRANSITION_TYPE_CROSSFADE);
    gtk_box_pack_start (GTK_BOX (ev_landing_view), priv->view, TRUE, TRUE, 0);

    gtk_stack_switcher_set_stack (GTK_STACK_SWITCHER (priv->stack_switcher), GTK_STACK (priv->view));

    priv->favorites_grid_view = add_grid_view_page (GTK_STACK (priv->view), "favorite_files", &priv->favorites_sw);
    priv->recent_grid_view = add_grid_view_page (GTK_STACK (priv->view), "recent_files", &priv->recents_sw);

    priv->no_files_view = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);

    gtk_widget_set_can_focus(priv->no_files_view, FALSE);
    gtk_widget_set_valign(priv->no_files_view, GTK_ALIGN_CENTER);
    gtk_widget_set_halign(priv->no_files_view, GTK_ALIGN_FILL);

    GtkWidget *no_recents_image = gtk_image_new_from_icon_name("face-sad-symbolic", GTK_ICON_SIZE_DIALOG);
    gtk_image_set_pixel_size(GTK_IMAGE(no_recents_image), 96);
    gtk_box_pack_start(GTK_BOX(priv->no_files_view), no_recents_image, FALSE, TRUE, 0);
    gtk_widget_show(no_recents_image);

    GtkWidget *label = gtk_label_new(_("No recent or favorite documents"));
    PangoAttribute *attrib_size = pango_attr_scale_new(1.05);
    PangoAttribute *attrib_bold = pango_attr_weight_new(PANGO_WEIGHT_BOLD);
    PangoAttrList *attribs = pango_attr_list_new();
    pango_attr_list_insert(attribs, attrib_size);
    pango_attr_list_insert(attribs, attrib_bold);
    gtk_label_set_attributes(GTK_LABEL(label), attribs);
    pango_attr_list_unref (attribs);

    gtk_box_pack_start(GTK_BOX(priv->no_files_view), label, FALSE, FALSE, 5);
    gtk_widget_set_halign(label, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(label, GTK_ALIGN_START);
    gtk_widget_show(label);

    gtk_stack_add_named(GTK_STACK(priv->view), priv->no_files_view, "no_files");

    gtk_widget_show(priv->recent_grid_view);
    gtk_widget_show(priv->no_files_view);

    gtk_widget_show (priv->view);

    priv->favorites = xapp_favorites_get_default ();
    priv->favorites_changed_handler_id =
            g_signal_connect_swapped (priv->favorites,
                                      "changed",
                                      G_CALLBACK (ev_landing_view_refresh_favorites),
                                      ev_landing_view);
    ev_landing_view_refresh_favorites (ev_landing_view);

    priv->recent_manager = gtk_recent_manager_get_default ();
    priv->recent_manager_changed_handler_id =
            g_signal_connect_swapped (priv->recent_manager,
                                      "changed",
                                      G_CALLBACK (ev_landing_view_refresh_recents),
                                      ev_landing_view);

    ev_landing_view_refresh_recents (ev_landing_view);

    saved_view = g_settings_get_string (priv->state_settings, LAST_USED_STACK_PAGE_KEY);
    select_view (ev_landing_view, saved_view);
    g_free (saved_view);

    g_signal_connect_swapped (priv->view,
                              "notify::visible-child",
                              G_CALLBACK (active_page_changed),
                              ev_landing_view);
}

static void
ev_landing_view_dispose (GObject *obj)
{
    EvLandingView *ev_landing_view = EV_LANDING_VIEW (obj);
    EvLandingViewPrivate *priv = ev_landing_view->priv;

    if (priv->recent_manager_changed_handler_id) {
        g_signal_handler_disconnect (priv->recent_manager, priv->recent_manager_changed_handler_id);
        priv->recent_manager_changed_handler_id = 0;
    }

    if (priv->favorites_changed_handler_id) {
        g_signal_handler_disconnect (priv->favorites, priv->favorites_changed_handler_id);
        priv->favorites_changed_handler_id = 0;
    }

    g_clear_object (&priv->state_settings);

    priv->recent_manager = NULL;
    priv->favorites = NULL;

    G_OBJECT_CLASS (ev_landing_view_parent_class)->dispose (obj);
}

static void
ev_landing_view_class_init (EvLandingViewClass *klass)
{
    GObjectClass *g_object_class = G_OBJECT_CLASS (klass);
    g_object_class->constructed = ev_landing_view_constructed;
    g_object_class->dispose = ev_landing_view_dispose;

    signals[ITEM_ACTIVATED] =
            g_signal_new ("item-activated",
                          EV_TYPE_LANDING_VIEW,
                          G_SIGNAL_RUN_LAST,
                          0, NULL, NULL,
                          g_cclosure_marshal_generic,
                          G_TYPE_NONE, 1,
                          G_TYPE_STRING);
}


GtkWidget *
ev_landing_view_new (void)
{
    return GTK_WIDGET (g_object_new (EV_TYPE_LANDING_VIEW, NULL));
}
