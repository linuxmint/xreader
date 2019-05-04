/* ev-zoom-action.h
 *  this file is part of xreader, a generic document viewer
 *
 * Copyright (C) 2012 Carlos Garcia Campos  <carlosgc@gnome.org>
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

#ifndef EV_ZOOM_ACTION_H
#define EV_ZOOM_ACTION_H

#include <gtk/gtk.h>
#include <xreader-document.h>
#include <xreader-view.h>

G_BEGIN_DECLS

#define EV_TYPE_ZOOM_ACTION            (ev_zoom_action_get_type ())
#define EV_ZOOM_ACTION(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), EV_TYPE_ZOOM_ACTION, EvZoomAction))
#define EV_IS_ZOOM_ACTION(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EV_TYPE_ZOOM_ACTION))
#define EV_ZOOM_ACTION_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), EV_TYPE_ZOOM_ACTION, EvZoomActionClass))
#define EV_IS_ZOOM_ACTION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((obj), EV_TYPE_ZOOM_ACTION))
#define EV_ZOOM_ACTION_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), EV_TYPE_ZOOM_ACTION, EvZoomActionClass))

typedef struct _EvZoomAction        EvZoomAction;
typedef struct _EvZoomActionClass   EvZoomActionClass;

struct _EvZoomAction {
        GtkBox parent;
};

struct _EvZoomActionClass {
        GtkBoxClass parent_class;
};

GType      ev_zoom_action_get_type        (void);

GtkWidget *ev_zoom_action_new             (EvDocumentModel *model,
                                           GMenu           *menu);
gboolean   ev_zoom_action_get_popup_shown (EvZoomAction    *action);
gboolean   ev_zoom_action_get_focused     (EvZoomAction    *zoom_action);
void       ev_zoom_action_select_all      (EvZoomAction    *zoom_action);

G_END_DECLS

#endif
