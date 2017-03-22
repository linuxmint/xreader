/* ev-toolbar.c
 *  this file is part of xreader, a document viewer
 */

#include "config.h"

#include "ev-toolbar.h"

enum
{
    PROP_0,
    PROP_WINDOW
};

struct _EvToolbarPrivate
{
    GtkStyleContext *style_context;

    GtkWidget *fullscreen_group;

    EvWindow *window;
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

static GtkWidget *
create_button (GtkAction *action)
{
    GtkWidget *button;
    GtkWidget *image;
    GtkWidget *label;
    GtkWidget *box;

    button = gtk_button_new ();
    image = gtk_image_new_from_icon_name (gtk_action_get_icon_name (action), GTK_ICON_SIZE_MENU);
    label = gtk_label_new (gtk_action_get_short_label (action));
    box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 4);

    gtk_container_add (GTK_CONTAINER (button), box);
    gtk_box_pack_start (GTK_BOX (box), image, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (box), label, FALSE, FALSE, 0);
    gtk_style_context_add_class (gtk_widget_get_style_context (button), "flat");
    gtk_activatable_set_related_action (GTK_ACTIVATABLE (button), action);
    gtk_widget_set_tooltip_text (button, gtk_action_get_tooltip (action));

    return button;
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

    G_OBJECT_CLASS (ev_toolbar_parent_class)->constructed (object);

    ev_toolbar->priv->style_context = gtk_widget_get_style_context (GTK_WIDGET (ev_toolbar));
    gtk_style_context_add_class (ev_toolbar->priv->style_context, "primary-toolbar");

    action_group = ev_window_get_main_action_group (ev_toolbar->priv->window);

    tool_item = gtk_tool_item_new ();
    gtk_toolbar_insert (GTK_TOOLBAR (ev_toolbar), tool_item, 0);

    box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_container_add (GTK_CONTAINER (tool_item), box);

    action = gtk_action_group_get_action (action_group, "GoPreviousPage");
    button = create_button (action);
    gtk_box_pack_start (GTK_BOX (box), button, FALSE, FALSE, 0);

    action = gtk_action_group_get_action (action_group, "GoNextPage");
    button = create_button (action);
    gtk_box_pack_start (GTK_BOX (box), button, FALSE, FALSE, 0);

    gtk_widget_show_all (GTK_WIDGET (tool_item));

    action = gtk_action_group_get_action (action_group, "PageSelector");
    tool_item = GTK_TOOL_ITEM (gtk_action_create_tool_item (action));
    gtk_container_add (GTK_CONTAINER (ev_toolbar), GTK_WIDGET (tool_item));
    gtk_widget_show (GTK_WIDGET (tool_item));

    action = gtk_action_group_get_action (action_group, "ViewZoom");
    tool_item = GTK_TOOL_ITEM (gtk_action_create_tool_item (action));
    gtk_container_add (GTK_CONTAINER (ev_toolbar), GTK_WIDGET (tool_item));
    gtk_widget_show (GTK_WIDGET (tool_item));

    /* Setup the buttons we only want to show when fullscreened */
    tool_item = gtk_tool_item_new ();
    gtk_tool_item_set_expand (tool_item, TRUE);
    gtk_container_add (GTK_CONTAINER (ev_toolbar), GTK_WIDGET (tool_item));
    gtk_widget_show (GTK_WIDGET (tool_item));
    ev_toolbar->priv->fullscreen_group = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_container_add (GTK_CONTAINER (tool_item), ev_toolbar->priv->fullscreen_group);

    action = gtk_action_group_get_action (action_group, "LeaveFullscreen");
    button = create_button (action);
    gtk_box_pack_end (GTK_BOX (ev_toolbar->priv->fullscreen_group), button, FALSE, FALSE, 0);

    action = gtk_action_group_get_action (action_group, "StartPresentation");
    button = create_button (action);
    gtk_box_pack_end (GTK_BOX (ev_toolbar->priv->fullscreen_group), button, FALSE, FALSE, 0);
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
    }
    else if (!is_fullscreen && !gtk_style_context_has_class (ev_toolbar->priv->style_context, "primary-toolbar"))
    {
        gtk_style_context_add_class (ev_toolbar->priv->style_context, "primary-toolbar");
        gtk_widget_hide (ev_toolbar->priv->fullscreen_group);
    }
}