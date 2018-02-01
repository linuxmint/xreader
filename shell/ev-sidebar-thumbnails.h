/* ev-sidebar-thumbnails.h
 *  this file is part of xreader, a mate document viewer
 *
 * Copyright (C) 2004 Red Hat, Inc.
 *
 * Author:
 *   Jonathan Blandford <jrb@alum.mit.edu>
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

#ifndef __EV_SIDEBAR_THUMBNAILS_H__
#define __EV_SIDEBAR_THUMBNAILS_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef struct _EvSidebarThumbnails EvSidebarThumbnails;
typedef struct _EvSidebarThumbnailsClass EvSidebarThumbnailsClass;
typedef struct _EvSidebarThumbnailsPrivate EvSidebarThumbnailsPrivate;

#define EV_TYPE_SIDEBAR_THUMBNAILS		(ev_sidebar_thumbnails_get_type())
#define EV_SIDEBAR_THUMBNAILS(object)		(G_TYPE_CHECK_INSTANCE_CAST((object), EV_TYPE_SIDEBAR_THUMBNAILS, EvSidebarThumbnails))
#define EV_SIDEBAR_THUMBNAILS_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST((klass), EV_TYPE_SIDEBAR_THUMBNAILS, EvSidebarThumbnailsClass))
#define EV_IS_SIDEBAR_THUMBNAILS(object)	(G_TYPE_CHECK_INSTANCE_TYPE((object), EV_TYPE_SIDEBAR_THUMBNAILS))
#define EV_IS_SIDEBAR_THUMBNAILS_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), EV_TYPE_SIDEBAR_THUMBNAILS))
#define EV_SIDEBAR_THUMBNAILS_GET_CLASS(object)	(G_TYPE_INSTANCE_GET_CLASS((object), EV_TYPE_SIDEBAR_THUMBNAILS, EvSidebarThumbnailsClass))

struct _EvSidebarThumbnails {
	GtkBox base_instance;

	EvSidebarThumbnailsPrivate *priv;
};

struct _EvSidebarThumbnailsClass {
	GtkBoxClass base_class;

	void    (* size_changed)     (EvSidebarThumbnails *sidebar_thumbnails,
	                              gint                 new_size);
};

GType      ev_sidebar_thumbnails_get_type     (void) G_GNUC_CONST;
GtkWidget *ev_sidebar_thumbnails_new          (void);
gboolean   ev_sidebar_thumbnails_can_zoom_in  (EvSidebarThumbnails *sidebar_thumbnails);
gboolean   ev_sidebar_thumbnails_can_zoom_out (EvSidebarThumbnails *sidebar_thumbnails);
void       ev_sidebar_thumbnails_zoom_in      (EvSidebarThumbnails *sidebar_thumbnails);
void       ev_sidebar_thumbnails_zoom_out     (EvSidebarThumbnails *sidebar_thumbnails);
void       ev_sidebar_thumbnails_zoom_reset   (EvSidebarThumbnails *sidebar_thumbnails);
void       ev_sidebar_thumbnails_set_size     (EvSidebarThumbnails *sidebar_thumbnails, gint size);

G_END_DECLS

#endif /* __EV_SIDEBAR_THUMBNAILS_H__ */

