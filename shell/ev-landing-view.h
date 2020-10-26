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

#ifndef __EV_LANDING_VIEW_H__
#define __EV_LANDING_VIEW_H__

#include <glib-object.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef struct _EvLandingView        EvLandingView;
typedef struct _EvLandingViewClass   EvLandingViewClass;
typedef struct _EvLandingViewPrivate EvLandingViewPrivate;

#define EV_TYPE_LANDING_VIEW              (ev_landing_view_get_type ())
#define EV_LANDING_VIEW(obj)              (G_TYPE_CHECK_INSTANCE_CAST ((obj), EV_TYPE_LANDING_VIEW, EvLandingView))
#define EV_IS_LANDING_VIEW(obj)           (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EV_TYPE_LANDING_VIEW))
#define EV_LANDING_VIEW_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), EV_TYPE_LANDING_VIEW, EvLandingViewClass))
#define EV_IS_LANDING_VIEW_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE((klass), EV_TYPE_LANDING_VIEW))
#define EV_LANDING_VIEW_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), EV_TYPE_LANDING_VIEW, EvLandingViewClass))

struct _EvLandingView
{
        GtkBox parent;

        EvLandingViewPrivate *priv;
};

struct _EvLandingViewClass
{
        GtkBoxClass parent_class;
};

GType      ev_landing_view_get_type (void) G_GNUC_CONST;
GtkWidget *ev_landing_view_new      (void);

G_END_DECLS

#endif /* __EV_LANDING_VIEW_H__ */
