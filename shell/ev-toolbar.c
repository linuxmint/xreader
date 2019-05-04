/* ev-toolbar.c
 *  this file is part of xreader, a document viewer
 */

#include "config.h"

#include <glib/gi18n.h>

#include "ev-toolbar.h"
#include "ev-document-model.h"
#include "ev-zoom-action.h"

enum
{
    PROP_0,
    PROP_WINDOW
};

struct _EvToolbarPrivate
{
    GtkStyleContext *style_context;

    GtkWidget *fullscreen_group;
    GtkWidget *preset_group;
    GtkWidget *expand_window_button;
    GtkWidget *zoom_action;
    GtkWidget *page_preset_button;
    GtkWidget *reader_preset_button;
    GtkWidget *history_group;

    GSettings *settings;

    EvWindow *window;
    EvDocumentModel *model;
};

G_DEFINE_TYPE (EvToolbar, ev_toolbar, GTK_TYPE_TOOLBAR)

static void
ev_toolbar_set_property (GObject      *object,
                         guint         prop_id,
                         const GValue *value,
                         GParamSpec   *pspec)
{
    EvToolbar *ev_toolbar = EV_TOOLBAR (object);

    switch (prop_id)
    {
        case PROP_WINDOW:
            ev_toolbar->priv->window = g_value_get_object (value);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
ev_toolbar_document_model_changed_cb (EvDocumentModel *model,
                                      GParamSpec      *pspec,
                                      EvToolbar       *ev_toolbar)
{
    EvSizingMode sizing_mode;
    gboolean continuous;
    gboolean best_fit;
    gboolean page_width;

    sizing_mode = ev_document_model_get_sizing_mode (model);
    continuous = ev_document_model_get_continuous (model);

    switch (sizing_mode)
    {
        case EV_SIZING_BEST_FIT:
            best_fit = TRUE;
            page_width = FALSE;
            break;
        case EV_SIZING_FIT_WIDTH:
            best_fit = FALSE;
            page_width = TRUE;
            break;
        default:
            best_fit = page_width = FALSE;
            break;
    }

    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ev_toolbar->priv->page_preset_button),
                                  !continuous && best_fit);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ev_toolbar->priv->reader_preset_button),
                                  continuous && page_width);
}

static void
on_page_preset_toggled (GtkToggleButton *button,
                        gpointer         user_data)
{
    EvToolbar *ev_toolbar = EV_TOOLBAR (user_data);

    if (gtk_toggle_button_get_active (button))
    {
        ev_document_model_set_continuous (ev_toolbar->priv->model, FALSE);
        ev_document_model_set_sizing_mode (ev_toolbar->priv->model, EV_SIZING_BEST_FIT);
    }
}

static void
on_reader_preset_toggled (GtkToggleButton *button,
                          gpointer         user_data)
{
    EvToolbar *ev_toolbar = EV_TOOLBAR (user_data);

    if (gtk_toggle_button_get_active (button))
    {
        ev_document_model_set_continuous (ev_toolbar->priv->model, TRUE);
        ev_document_model_set_sizing_mode (ev_toolbar->priv->model, EV_SIZING_FIT_WIDTH);
    }
}

static GtkWidget *
setup_preset_buttons (EvToolbar *ev_toolbar)
{
    GtkWidget *box;
    GtkWidget *image;

    box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);

    /* Page preset button */
    ev_toolbar->priv->page_preset_button = gtk_toggle_button_new ();
    gtk_widget_set_valign (ev_toolbar->priv->page_preset_button, GTK_ALIGN_CENTER);
    image = gtk_image_new_from_icon_name ("view-paged-symbolic", GTK_ICON_SIZE_MENU);
    gtk_style_context_add_class (gtk_widget_get_style_context (ev_toolbar->priv->page_preset_button), "flat");
    gtk_button_set_focus_on_click (GTK_BUTTON (ev_toolbar->priv->page_preset_button), FALSE);

    gtk_button_set_image (GTK_BUTTON (ev_toolbar->priv->page_preset_button), image);
    gtk_widget_set_tooltip_text (ev_toolbar->priv->page_preset_button,
                                 _("Page View\nNon-Continuous + Best Fit\nCtrl+2"));
    gtk_box_pack_end (GTK_BOX (box), ev_toolbar->priv->page_preset_button, FALSE, FALSE, 0);
    g_signal_connect (ev_toolbar->priv->page_preset_button, "toggled",
                      G_CALLBACK (on_page_preset_toggled), ev_toolbar);

    /* Reader preset button */
    ev_toolbar->priv->reader_preset_button = gtk_toggle_button_new ();
    gtk_widget_set_valign (ev_toolbar->priv->reader_preset_button, GTK_ALIGN_CENTER);
    image = gtk_image_new_from_icon_name ("view-continuous-symbolic", GTK_ICON_SIZE_MENU);
    gtk_style_context_add_class (gtk_widget_get_style_context (ev_toolbar->priv->reader_preset_button), "flat");
    gtk_button_set_focus_on_click (GTK_BUTTON (ev_toolbar->priv->reader_preset_button), FALSE);

    gtk_button_set_image (GTK_BUTTON (ev_toolbar->priv->reader_preset_button), image);
    gtk_widget_set_tooltip_text (ev_toolbar->priv->reader_preset_button,
                                 _("Reader View\nContinuous + Fit Page Width\nCtrl+1"));
    gtk_box_pack_end (GTK_BOX (box), ev_toolbar->priv->reader_preset_button, FALSE, FALSE, 0);
    g_signal_connect (ev_toolbar->priv->reader_preset_button, "toggled",
                      G_CALLBACK (on_reader_preset_toggled), ev_toolbar);

    return box;
}

static GtkWidget *
create_button (GtkAction *action)
{
    GtkWidget *button;
    GtkWidget *image;

    button = gtk_button_new ();
    gtk_widget_set_valign (button, GTK_ALIGN_CENTER);
    image = gtk_image_new ();

    gtk_button_set_image (GTK_BUTTON (button), image);
    gtk_style_context_add_class (gtk_widget_get_style_context (button), "flat");
    gtk_activatable_set_related_action (GTK_ACTIVATABLE (button), action);
    gtk_button_set_label (GTK_BUTTON (button), NULL);
    gtk_widget_set_tooltip_text (button, gtk_action_get_tooltip (action));
    gtk_button_set_focus_on_click (GTK_BUTTON (button), FALSE);

    return button;
}

gboolean
ev_toolbar_zoom_action_get_focused (EvToolbar *ev_toolbar)
{
    return ev_zoom_action_get_focused (EV_ZOOM_ACTION (ev_toolbar->priv->zoom_action));
}

void
ev_toolbar_zoom_action_select_all (EvToolbar *ev_toolbar)
{
    ev_zoom_action_select_all (EV_ZOOM_ACTION (ev_toolbar->priv->zoom_action));
}

static void
ev_toolbar_constructed (GObject *object)
{
    EvToolbar *ev_toolbar = EV_TOOLBAR (object);
    GtkActionGroup *action_group;
    GtkAction *action;
    GtkToolItem *tool_item;
    GtkWidget *box;
    GtkWidget *button;
    GtkWidget *separator;

    G_OBJECT_CLASS (ev_toolbar_parent_class)->constructed (object);

    ev_toolbar->priv->model = ev_window_get_document_model (ev_toolbar->priv->window);

    ev_toolbar->priv->style_context = gtk_widget_get_style_context (GTK_WIDGET (ev_toolbar));
    gtk_style_context_add_class (ev_toolbar->priv->style_context, "primary-toolbar");

    action_group = ev_window_get_main_action_group (ev_toolbar->priv->window);

    /* Go Previous/Next page */
    tool_item = gtk_tool_item_new ();
    gtk_container_add (GTK_CONTAINER (ev_toolbar), GTK_WIDGET (tool_item));
    gtk_widget_show (GTK_WIDGET (tool_item));

    box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_container_add (GTK_CONTAINER (tool_item), box);
    gtk_widget_show (GTK_WIDGET (box));

    action = gtk_action_group_get_action (action_group, "GoPreviousPage");
    button = create_button (action);
    gtk_box_pack_start (GTK_BOX (box), button, FALSE, FALSE, 0);
    gtk_widget_show (GTK_WIDGET (button));

    action = gtk_action_group_get_action (action_group, "GoNextPage");
    button = create_button (action);
    gtk_box_pack_start (GTK_BOX (box), button, FALSE, FALSE, 0);
    gtk_widget_show (GTK_WIDGET (button));

    action = gtk_action_group_get_action (action_group, "PageSelector");
    tool_item = GTK_TOOL_ITEM (gtk_action_create_tool_item (action));
    gtk_container_add (GTK_CONTAINER (ev_toolbar), GTK_WIDGET (tool_item));
    gtk_widget_show (GTK_WIDGET (tool_item));

    /* History Navigation */
    tool_item = gtk_tool_item_new ();
    gtk_container_add (GTK_CONTAINER (ev_toolbar), GTK_WIDGET (tool_item));
    gtk_widget_show (GTK_WIDGET (tool_item));

    ev_toolbar->priv->history_group = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_container_add (GTK_CONTAINER (tool_item), ev_toolbar->priv->history_group);
    gtk_widget_show (GTK_WIDGET (ev_toolbar->priv->history_group));

    action = gtk_action_group_get_action (action_group, "GoPreviousHistory");
    button = create_button (action);
    gtk_box_pack_start (GTK_BOX (ev_toolbar->priv->history_group), button, FALSE, FALSE, 0);
    gtk_widget_show (GTK_WIDGET (button));

    action = gtk_action_group_get_action (action_group, "GoNextHistory");
    button = create_button (action);
    gtk_box_pack_start (GTK_BOX (ev_toolbar->priv->history_group), button, FALSE, FALSE, 0);
    gtk_widget_show (GTK_WIDGET (button));

    /* Zoom */
    tool_item = gtk_tool_item_new ();
    gtk_tool_item_set_expand (tool_item, TRUE);
    gtk_container_add (GTK_CONTAINER (ev_toolbar), GTK_WIDGET (tool_item));
    gtk_widget_show (GTK_WIDGET (tool_item));

    box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_container_add (GTK_CONTAINER (tool_item), box);
    gtk_widget_show (GTK_WIDGET (box));

    separator = gtk_separator_new (GTK_ORIENTATION_VERTICAL);
    gtk_widget_set_margin_start (separator, 6);
    gtk_widget_set_margin_end (separator, 6);
    gtk_box_pack_end (GTK_BOX (box), separator, FALSE, FALSE, 0);
    gtk_widget_show (GTK_WIDGET (separator));

    ev_toolbar->priv->zoom_action = ev_zoom_action_new
        (ev_window_get_document_model (ev_toolbar->priv->window), g_menu_new());
    gtk_widget_set_tooltip_text (ev_toolbar->priv->zoom_action,
                                 _("Select or set the zoom level of the document"));
    gtk_widget_set_margin_start(ev_toolbar->priv->zoom_action, 2);
    gtk_box_pack_end (GTK_BOX (box), ev_toolbar->priv->zoom_action, FALSE, FALSE, 0);

    action = gtk_action_group_get_action (action_group, "ViewExpandWindow");
    ev_toolbar->priv->expand_window_button = create_button (action);
    gtk_box_pack_end (GTK_BOX (box), ev_toolbar->priv->expand_window_button, FALSE, FALSE, 0);

    action = gtk_action_group_get_action (action_group, "ViewZoomReset");
    button = create_button (action);
    gtk_box_pack_end (GTK_BOX (box), button, FALSE, FALSE, 0);
    gtk_widget_show (GTK_WIDGET (button));

    action = gtk_action_group_get_action (action_group, "ViewZoomIn");
    button = create_button (action);
    gtk_box_pack_end (GTK_BOX (box), button, FALSE, FALSE, 0);
    gtk_widget_show (GTK_WIDGET (button));

    action = gtk_action_group_get_action (action_group, "ViewZoomOut");
    button = create_button (action);
    gtk_box_pack_end (GTK_BOX (box), button, FALSE, FALSE, 0);
    gtk_widget_show (GTK_WIDGET (button));

    /* View preset button */
    tool_item = gtk_tool_item_new ();
    gtk_container_add (GTK_CONTAINER (ev_toolbar), GTK_WIDGET (tool_item));
    ev_toolbar->priv->preset_group = setup_preset_buttons (ev_toolbar);
    gtk_container_add (GTK_CONTAINER (tool_item), ev_toolbar->priv->preset_group);
    gtk_widget_show_all (GTK_WIDGET (tool_item));

    /* Setup the buttons we only want to show when fullscreened */
    tool_item = gtk_tool_item_new ();
    gtk_container_add (GTK_CONTAINER (ev_toolbar), GTK_WIDGET (tool_item));
    gtk_widget_show (GTK_WIDGET (tool_item));
    ev_toolbar->priv->fullscreen_group = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_container_add (GTK_CONTAINER (tool_item), ev_toolbar->priv->fullscreen_group);

    action = gtk_action_group_get_action (action_group, "LeaveFullscreen");
    button = create_button (action);
    gtk_box_pack_end (GTK_BOX (ev_toolbar->priv->fullscreen_group), button, FALSE, FALSE, 0);

    separator = gtk_separator_new (GTK_ORIENTATION_VERTICAL);
    gtk_widget_set_margin_start (separator, 6);
    gtk_widget_set_margin_end (separator, 6);
    gtk_box_pack_end (GTK_BOX (ev_toolbar->priv->fullscreen_group), separator, FALSE, FALSE, 0);

    action = gtk_action_group_get_action (action_group, "StartPresentation");
    button = create_button (action);
    gtk_box_pack_end (GTK_BOX (ev_toolbar->priv->fullscreen_group), button, FALSE, FALSE, 0);

    g_signal_connect (ev_toolbar->priv->model, "notify::continuous",
                      G_CALLBACK (ev_toolbar_document_model_changed_cb), ev_toolbar);
    g_signal_connect (ev_toolbar->priv->model, "notify::sizing-mode",
                      G_CALLBACK (ev_toolbar_document_model_changed_cb), ev_toolbar);

    /* Toolbar buttons visibility bindings */
    g_settings_bind (ev_toolbar->priv->settings, GS_SHOW_EXPAND_WINDOW,
                     ev_toolbar->priv->expand_window_button, "visible",
                     G_SETTINGS_BIND_DEFAULT);
    g_settings_bind (ev_toolbar->priv->settings, GS_SHOW_ZOOM_ACTION,
                     ev_toolbar->priv->zoom_action, "visible",
                     G_SETTINGS_BIND_DEFAULT);
    g_settings_bind (ev_toolbar->priv->settings, GS_SHOW_HISTORY_BUTTONS,
                     ev_toolbar->priv->history_group, "visible",
                     G_SETTINGS_BIND_DEFAULT);

    ev_toolbar_document_model_changed_cb (ev_toolbar->priv->model, NULL, ev_toolbar);
}

static void
ev_toolbar_class_init (EvToolbarClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->set_property = ev_toolbar_set_property;
    object_class->constructed = ev_toolbar_constructed;

    g_object_class_install_property (object_class,
                                     PROP_WINDOW,
                                     g_param_spec_object ("window",
                                                          "Window",
                                                          "The parent xreader window",
                                                          EV_TYPE_WINDOW,
                                                          G_PARAM_WRITABLE |
                                                          G_PARAM_CONSTRUCT_ONLY |
                                                          G_PARAM_STATIC_STRINGS));

    g_type_class_add_private (object_class, sizeof (EvToolbarPrivate));
}

static void
ev_toolbar_init (EvToolbar *ev_toolbar)
{
    ev_toolbar->priv = G_TYPE_INSTANCE_GET_PRIVATE (ev_toolbar, EV_TYPE_TOOLBAR, EvToolbarPrivate);
    ev_toolbar->priv->settings = g_settings_new (GS_SCHEMA_NAME_TOOLBAR);
}

GtkWidget *
ev_toolbar_new (EvWindow *window)
{
    g_return_val_if_fail (EV_IS_WINDOW (window), NULL);

    return GTK_WIDGET (g_object_new (EV_TYPE_TOOLBAR, "window", window, NULL));
}

void
ev_toolbar_set_style (EvToolbar *ev_toolbar,
                      gboolean   is_fullscreen)
{
    g_return_if_fail (EV_IS_TOOLBAR (ev_toolbar));

    if (is_fullscreen && gtk_style_context_has_class (ev_toolbar->priv->style_context, "primary-toolbar"))
    {
        gtk_style_context_remove_class (ev_toolbar->priv->style_context, "primary-toolbar");
        gtk_widget_show_all (ev_toolbar->priv->fullscreen_group);
        gtk_widget_hide (ev_toolbar->priv->expand_window_button);
    }
    else if (!is_fullscreen && !gtk_style_context_has_class (ev_toolbar->priv->style_context, "primary-toolbar"))
    {
        gtk_style_context_add_class (ev_toolbar->priv->style_context, "primary-toolbar");
        gtk_widget_hide (ev_toolbar->priv->fullscreen_group);
        gtk_widget_show_all (ev_toolbar->priv->expand_window_button);
    }
}

void
ev_toolbar_set_preset_sensitivity (EvToolbar *ev_toolbar,
                                   gboolean   sensitive)
{
    g_return_if_fail (EV_IS_TOOLBAR (ev_toolbar));

    gtk_widget_set_sensitive (ev_toolbar->priv->preset_group, sensitive);
}

void
ev_toolbar_activate_reader_view (EvToolbar *ev_toolbar)
{
    g_return_if_fail (EV_IS_TOOLBAR (ev_toolbar));

    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ev_toolbar->priv->reader_preset_button), TRUE);
}

void
ev_toolbar_activate_page_view (EvToolbar *ev_toolbar)
{
    g_return_if_fail (EV_IS_TOOLBAR (ev_toolbar));

    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ev_toolbar->priv->page_preset_button), TRUE);
}
