#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <glib/gi18n.h>
#include <glib/gstdio.h>
#include <gtk/gtk.h>

#include "ev-preferences-dialog.h"

enum
{
    PROP_0,
    PROP_WINDOW,
};

/*
 * The preferences dialog is a singleton since we don't want two dialogs showing an inconsistent state of the
 * preferences.
 * When ev_preferences_dialog_show is called and there is already a prefs dialog dialog open, it is reparented
 * and shown.
 */
static GtkWidget *preferences_dialog = NULL;

#define EV_TYPE_PREFERENCES_DIALOG    (ev_preferences_dialog_get_type())

G_DECLARE_FINAL_TYPE(EvPreferencesDialog, ev_preferences_dialog, EV, PREFERENCES_DIALOG, XAppPreferencesWindow)

struct _EvPreferencesDialog
{
    XAppPreferencesWindow  parent_instance;

    GSettings             *toolbar_settings;

    /* Main pages */
    GtkWidget             *toolbar_page;

    /* Toolbar page */
    GtkWidget             *show_history_buttons_switch;
    GtkWidget             *show_expand_window_button_switch;
};

G_DEFINE_TYPE(EvPreferencesDialog, ev_preferences_dialog, XAPP_TYPE_PREFERENCES_WINDOW)

static void
ev_preferences_dialog_dispose(GObject *object)
{
    EvPreferencesDialog *dlg = EV_PREFERENCES_DIALOG(object);

    g_clear_object(&dlg->toolbar_settings);

    G_OBJECT_CLASS(ev_preferences_dialog_parent_class)->dispose(object);
}

static void
ev_preferences_dialog_class_init(EvPreferencesDialogClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

    object_class->dispose = ev_preferences_dialog_dispose;

    gtk_widget_class_set_template_from_resource(widget_class, "/org/x/reader/shell/ui/ev-preferences-dialog.ui");

    /* Pages */
    gtk_widget_class_bind_template_child(widget_class, EvPreferencesDialog, toolbar_page);

    /* Editor Page widgets */
    gtk_widget_class_bind_template_child(widget_class, EvPreferencesDialog, show_history_buttons_switch);
    gtk_widget_class_bind_template_child(widget_class, EvPreferencesDialog, show_expand_window_button_switch);
}

static void
close_button_clicked(GtkButton *button,
                     gpointer   data)
{
    EvPreferencesDialog *dlg = EV_PREFERENCES_DIALOG(data);

    gtk_widget_destroy(GTK_WIDGET(dlg));
}

static void
help_button_clicked(GtkButton *button,
                    gpointer   data)
{
    EvPreferencesDialog *dlg = EV_PREFERENCES_DIALOG(data);
    ev_window_show_help(GTK_WINDOW(dlg), NULL);
}

static void
setup_editor_page(EvPreferencesDialog *dlg)
{
    g_settings_bind(dlg->toolbar_settings,
                     GS_SHOW_HISTORY_BUTTONS,
                     dlg->show_history_buttons_switch,
                     "active",
                     G_SETTINGS_BIND_GET | G_SETTINGS_BIND_SET);

    g_settings_bind(dlg->toolbar_settings,
                     GS_SHOW_EXPAND_WINDOW,
                     dlg->show_expand_window_button_switch,
                     "active",
                     G_SETTINGS_BIND_GET | G_SETTINGS_BIND_SET);

    xapp_preferences_window_add_page(XAPP_PREFERENCES_WINDOW(dlg), dlg->toolbar_page, "toolbar", _("Toolbar"));
}

static void
setup_buttons(EvPreferencesDialog *dlg)
{
    GtkWidget *button;

    button = gtk_button_new_with_label(_("Help"));
    xapp_preferences_window_add_button(XAPP_PREFERENCES_WINDOW(dlg), button, GTK_PACK_START);
    g_signal_connect(button, "clicked", G_CALLBACK(help_button_clicked), dlg);

    button = gtk_button_new_with_label(_("Close"));
    xapp_preferences_window_add_button(XAPP_PREFERENCES_WINDOW(dlg), button, GTK_PACK_END);
    g_signal_connect(button, "clicked", G_CALLBACK(close_button_clicked), dlg);
}

static void
ev_preferences_dialog_init(EvPreferencesDialog *dlg)
{
    dlg->toolbar_settings = g_settings_new(GS_SCHEMA_NAME_TOOLBAR);

    gtk_window_set_title(GTK_WINDOW(dlg), _("Preferences"));

    gtk_widget_init_template(GTK_WIDGET(dlg));

    setup_buttons(dlg);
    setup_editor_page(dlg);
    gtk_widget_show_all(GTK_WIDGET(dlg));
}

GtkWidget *
ev_preferences_dialog_new(EvWindow *parent)
{
    g_return_val_if_fail(EV_IS_WINDOW(parent), NULL);

    return GTK_WIDGET(g_object_new(EV_TYPE_PREFERENCES_DIALOG,
                                   "transient-for", parent,
                                   NULL));
}

void
ev_preferences_dialog_show(EvWindow *parent)
{
    g_return_if_fail(EV_IS_WINDOW(parent));

    if (preferences_dialog == NULL) {
        preferences_dialog = ev_preferences_dialog_new(parent);
        g_signal_connect(preferences_dialog, "destroy", G_CALLBACK(gtk_widget_destroyed), &preferences_dialog);
    }

    if (GTK_WINDOW(parent) != gtk_window_get_transient_for(GTK_WINDOW(preferences_dialog))) {
        gtk_window_set_transient_for(GTK_WINDOW(preferences_dialog), GTK_WINDOW(parent));
    }

    gtk_window_present(GTK_WINDOW(preferences_dialog));
}

