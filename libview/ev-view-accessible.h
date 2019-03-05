/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; c-indent-level: 8 -*- */
/* this file is part of xreader, a mate document viewer
 *
 *  Copyright (C) 2004 Red Hat, Inc
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

#if !defined (XREADER_COMPILATION)
#error "This is a private header."
#endif

#ifndef __EV_VIEW_ACCESSIBLE_H__
#define __EV_VIEW_ACCESSIBLE_H__

#include <gtk/gtk-a11y.h>

#define EV_TYPE_VIEW_ACCESSIBLE      (ev_view_accessible_get_type ())
#define EV_VIEW_ACCESSIBLE(obj)      (G_TYPE_CHECK_INSTANCE_CAST ((obj), EV_TYPE_VIEW_ACCESSIBLE, EvViewAccessible))
#define EV_IS_VIEW_ACCESSIBLE(obj)   (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EV_TYPE_VIEW_ACCESSIBLE))

typedef struct _EvViewAccessible        EvViewAccessible;
typedef struct _EvViewAccessibleClass   EvViewAccessibleClass;
typedef struct _EvViewAccessiblePrivate EvViewAccessiblePrivate;

struct _EvViewAccessible
{
	GtkContainerAccessible parent;

	EvViewAccessiblePrivate *priv;
};

struct _EvViewAccessibleClass
{
	GtkContainerAccessibleClass parent_class;
};

GType      ev_view_accessible_get_type (void);
AtkObject *ev_view_accessible_new      (GtkWidget *widget);

#endif  /* __EV_VIEW_ACCESSIBLE_H__ */

