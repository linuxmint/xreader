/* ev-annotations-toolbar.c
 *  this file is part of evince, a gnome document viewer
 *
 * Copyright (C) 2015 Carlos Garcia Campos  <carlosgc@gnome.org>
 *
 * Evince is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Evince is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 */

#include "config.h"

#include "ev-annotations-toolbar.h"
#include <xreader-document.h>
#include <glib/gi18n.h>

enum {
        BEGIN_ADD_ANNOT,
        CANCEL_ADD_ANNOT,
        CHANGE_COLOR_ANNOT,
        N_SIGNALS
};

struct _EvAnnotationsToolbar {
        GtkFlowBox base_instance;

        GtkWidget *text_button;
        GtkWidget *highlight_button;
        GtkWidget *underline_button;
        GtkWidget *squiggly_button;
        GtkWidget *strike_out_button;
        
        GtkWidget *color_button;
};

struct _EvAnnotationsToolbarClass {
	GtkFlowBoxClass base_class;

};

static guint signals[N_SIGNALS];

G_DEFINE_TYPE (EvAnnotationsToolbar, ev_annotations_toolbar, GTK_TYPE_FLOW_BOX)

static void
ev_annotations_toolbar_annot_button_toggled (GtkWidget            *button,
                                             EvAnnotationsToolbar *toolbar)
{
    EvAnnotationInfo annot_info;

    if (!gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button))) {
            g_signal_emit (toolbar, signals[CANCEL_ADD_ANNOT], 0, NULL);
            return;
    }

    /* When an another button was activated before */
	if (toolbar->text_button != button && gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (toolbar->text_button)))
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (toolbar->text_button), FALSE);
	else if (toolbar->highlight_button != button && gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (toolbar->highlight_button)))
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (toolbar->highlight_button), FALSE);
	else if (toolbar->underline_button != button && gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (toolbar->underline_button)))
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (toolbar->underline_button), FALSE);
	else if (toolbar->squiggly_button != button && gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (toolbar->squiggly_button)))
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (toolbar->squiggly_button), FALSE);
	else if (toolbar->strike_out_button != button && gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (toolbar->strike_out_button)))
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (toolbar->strike_out_button), FALSE);

	if (button == toolbar->text_button) {
		annot_info.type = EV_ANNOTATION_TYPE_TEXT;
		annot_info.icon = EV_ANNOTATION_TEXT_ICON_NOTE;
	} else {
		annot_info.type = EV_ANNOTATION_TYPE_TEXT_MARKUP;

		if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (toolbar->highlight_button)))
			annot_info.markup_type = EV_ANNOTATION_TEXT_MARKUP_HIGHLIGHT;
		else if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (toolbar->strike_out_button)))
			annot_info.markup_type = EV_ANNOTATION_TEXT_MARKUP_STRIKE_OUT;
		else if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (toolbar->squiggly_button)))
			annot_info.markup_type = EV_ANNOTATION_TEXT_MARKUP_SQUIGGLY;
		else if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (toolbar->underline_button)))
			annot_info.markup_type = EV_ANNOTATION_TEXT_MARKUP_UNDERLINE;
		else 
			annot_info.type = EV_ANNOTATION_TYPE_UNKNOWN;
	}
	
	gtk_color_button_get_color (GTK_COLOR_BUTTON (toolbar->color_button), &annot_info.color);
	g_signal_emit (toolbar, signals[BEGIN_ADD_ANNOT], 0, &annot_info);
}

static void
ev_annotations_toolbar_color_changed (GtkWidget            *button,
                                      EvAnnotationsToolbar *toolbar)
{
	EvAnnotationInfo annot_info;

	g_signal_emit (toolbar, signals[CANCEL_ADD_ANNOT], 0, NULL);
	gtk_color_button_get_color (GTK_COLOR_BUTTON (toolbar->color_button), &annot_info.color);

	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (toolbar->highlight_button))) {
		annot_info.type = EV_ANNOTATION_TYPE_TEXT_MARKUP;
		annot_info.markup_type = EV_ANNOTATION_TEXT_MARKUP_HIGHLIGHT;
		g_signal_emit (toolbar, signals[BEGIN_ADD_ANNOT], 0, &annot_info);
	} else if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (toolbar->underline_button))) {
		annot_info.type = EV_ANNOTATION_TYPE_TEXT_MARKUP;
		annot_info.markup_type = EV_ANNOTATION_TEXT_MARKUP_UNDERLINE;
		g_signal_emit (toolbar, signals[BEGIN_ADD_ANNOT], 0, &annot_info);
	} else if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (toolbar->squiggly_button))) {
		annot_info.type = EV_ANNOTATION_TYPE_TEXT_MARKUP;
		annot_info.markup_type = EV_ANNOTATION_TEXT_MARKUP_SQUIGGLY;
		g_signal_emit (toolbar, signals[BEGIN_ADD_ANNOT], 0, &annot_info);
	} else if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (toolbar->strike_out_button))) {
		annot_info.type = EV_ANNOTATION_TYPE_TEXT_MARKUP;
		annot_info.markup_type = EV_ANNOTATION_TEXT_MARKUP_STRIKE_OUT;
		g_signal_emit (toolbar, signals[BEGIN_ADD_ANNOT], 0, &annot_info);
	} else if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (toolbar->text_button))) {
		annot_info.type = EV_ANNOTATION_TYPE_TEXT;
		annot_info.markup_type = EV_ANNOTATION_TEXT_ICON_NOTE;
		g_signal_emit (toolbar, signals[BEGIN_ADD_ANNOT], 0, &annot_info);
	}
	
	g_signal_emit (toolbar, signals[CHANGE_COLOR_ANNOT], 0, &annot_info.color);
}

static gboolean
ev_annotations_toolbar_toggle_button_if_active (EvAnnotationsToolbar *toolbar,
                                                GtkToggleButton      *button)
{
        if (!gtk_toggle_button_get_active (button))
                return FALSE;

        g_signal_handlers_block_by_func (button,
                                         ev_annotations_toolbar_annot_button_toggled,
                                         toolbar);
        gtk_toggle_button_set_active (button, FALSE);
        g_signal_handlers_unblock_by_func (button,
                                           ev_annotations_toolbar_annot_button_toggled,
                                           toolbar);

        return TRUE;
}

static GtkWidget *
ev_annotations_toolbar_create_toggle_button (EvAnnotationsToolbar *toolbar,
                                             const gchar          *icon_name,
                                             const gchar          *tooltip)
{
        GtkWidget *button = GTK_WIDGET (gtk_toggle_button_new ());
		gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);
        GtkWidget *image = gtk_image_new_from_icon_name (icon_name, GTK_ICON_SIZE_BUTTON);

        gtk_button_set_image (GTK_BUTTON (button), image);
        gtk_widget_set_tooltip_text (button, tooltip);
        
        /* For some reason adding text-button class to the GtkToogleButton makes the button smaller */
        gtk_style_context_add_class (gtk_widget_get_style_context (gtk_bin_get_child (GTK_BIN (button))), "text-button");
        g_signal_connect (button, "toggled",
                          G_CALLBACK (ev_annotations_toolbar_annot_button_toggled),
                          toolbar);

        return button;
}

static void
ev_annotations_toolbar_init (EvAnnotationsToolbar *toolbar)
{
        gtk_widget_set_valign (GTK_WIDGET (toolbar), GTK_ALIGN_START);
        gtk_flow_box_set_activate_on_single_click (GTK_FLOW_BOX (toolbar), TRUE);
        gtk_flow_box_set_selection_mode (GTK_FLOW_BOX (toolbar), GTK_SELECTION_SINGLE);
        gtk_flow_box_set_homogeneous (GTK_FLOW_BOX (toolbar), TRUE);

        toolbar->text_button = ev_annotations_toolbar_create_toggle_button (toolbar,
                                                                            "xapp-annotations-text-symbolic",
                                                                            _("Add text annotation"));
        gtk_container_add (GTK_CONTAINER(toolbar), toolbar->text_button);
        gtk_widget_show (toolbar->text_button);

        toolbar->highlight_button = ev_annotations_toolbar_create_toggle_button (toolbar,
                                                                                 "xapp-format-text-highlight-symbolic",
                                                                                 _("Add highlight annotation"));
        gtk_container_add (GTK_CONTAINER (toolbar), toolbar->highlight_button);
        gtk_widget_show (toolbar->highlight_button);
        
        
        toolbar->underline_button = ev_annotations_toolbar_create_toggle_button (toolbar,
                                                                            "gtk-underline",
                                                                            _("Underline the selection"));
        gtk_container_add (GTK_CONTAINER(toolbar), toolbar->underline_button);
        gtk_widget_show (toolbar->underline_button);
        
        toolbar->strike_out_button = ev_annotations_toolbar_create_toggle_button (toolbar,
                                                                            "gtk-strikethrough",
                                                                            _("Add text annotation"));
        gtk_container_add (GTK_CONTAINER(toolbar), toolbar->strike_out_button);
        gtk_widget_show (toolbar->strike_out_button);
        
        toolbar->squiggly_button = ev_annotations_toolbar_create_toggle_button (toolbar,
                                                                            "annotations-squiggly-symbolic",
                                                                            _("Squiggly the selected text"));
        gtk_container_add (GTK_CONTAINER(toolbar), toolbar->squiggly_button);
        gtk_widget_show (toolbar->squiggly_button);
        
        /* color button */
		toolbar->color_button = gtk_color_button_new ();
		gtk_button_set_relief (GTK_BUTTON (toolbar->color_button), GTK_RELIEF_NONE);

		gtk_container_add (GTK_CONTAINER(toolbar), toolbar->color_button);
		gtk_widget_set_tooltip_text (GTK_WIDGET (toolbar->color_button), _("Change color annotation"));
		g_signal_connect (toolbar->color_button, "color-set",
						  G_CALLBACK (ev_annotations_toolbar_color_changed),
						  toolbar);
		gtk_widget_show (GTK_WIDGET (toolbar->color_button));
}

static void
ev_annotations_toolbar_class_init (EvAnnotationsToolbarClass *klass)
{
        GObjectClass *g_object_class = G_OBJECT_CLASS (klass);

        signals[BEGIN_ADD_ANNOT] =
                g_signal_new ("begin-add-annot",
                              G_TYPE_FROM_CLASS (g_object_class),
                              G_SIGNAL_RUN_LAST,
                              0,
                              NULL, NULL,
                              g_cclosure_marshal_VOID__POINTER,
                              G_TYPE_NONE, 1,
                              G_TYPE_POINTER);

        signals[CANCEL_ADD_ANNOT] =
                g_signal_new ("cancel-add-annot",
                              G_TYPE_FROM_CLASS (g_object_class),
                              G_SIGNAL_RUN_LAST,
                              0,
                              NULL, NULL,
                              g_cclosure_marshal_VOID__VOID,
                              G_TYPE_NONE, 0,
                              G_TYPE_NONE);
		
        signals[CHANGE_COLOR_ANNOT] =
                g_signal_new ("change-color-annot",
                              G_TYPE_FROM_CLASS (g_object_class),
                              G_SIGNAL_RUN_LAST,
                              0,
                              NULL, NULL,
                              g_cclosure_marshal_VOID__POINTER,
                              G_TYPE_NONE, 1,
                              G_TYPE_POINTER);
}

GtkWidget *
ev_annotations_toolbar_new (void)
{
	return GTK_WIDGET (g_object_new (EV_TYPE_ANNOTATIONS_TOOLBAR, NULL));
}

void
ev_annotations_toolbar_add_annot_finished (EvAnnotationsToolbar *toolbar)
{
    g_return_if_fail (EV_IS_ANNOTATIONS_TOOLBAR (toolbar));

    if (ev_annotations_toolbar_toggle_button_if_active (toolbar, GTK_TOGGLE_BUTTON (toolbar->text_button)))
            return;

    if (ev_annotations_toolbar_toggle_button_if_active (toolbar, GTK_TOGGLE_BUTTON (toolbar->highlight_button)))
    	return;
    
    if (ev_annotations_toolbar_toggle_button_if_active (toolbar, GTK_TOGGLE_BUTTON (toolbar->underline_button)))
    	return;
    	
    if (ev_annotations_toolbar_toggle_button_if_active (toolbar, GTK_TOGGLE_BUTTON (toolbar->strike_out_button)))
    	return;
    
    ev_annotations_toolbar_toggle_button_if_active (toolbar, GTK_TOGGLE_BUTTON (toolbar->squiggly_button));
}

void
ev_annotations_toolbar_set_color (EvAnnotationsToolbar *toolbar, GdkColor *color)
{
	gtk_color_button_set_color (GTK_COLOR_BUTTON (toolbar->color_button), color);
}
													  
