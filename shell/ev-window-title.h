/* this file is part of xreader, a mate document viewer
 *
 *  Copyright (C) 2005 Red Hat, Inc
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

#ifndef __EV_WINDOW_TITLE_H__
#define __EV_WINDOW_TITLE_H__

#include "ev-window.h"
#include "ev-document.h"

G_BEGIN_DECLS

typedef struct _EvWindowTitle EvWindowTitle;

typedef enum
{
	EV_WINDOW_TITLE_DOCUMENT,
	EV_WINDOW_TITLE_PASSWORD,
    EV_WINDOW_TITLE_RECENT
} EvWindowTitleType;

EvWindowTitle *ev_window_title_new	    (EvWindow *window);
void	       ev_window_title_set_type     (EvWindowTitle     *window_title,
					     EvWindowTitleType  type);
void           ev_window_title_set_document (EvWindowTitle     *window_title,
					     EvDocument        *document);
void	       ev_window_title_set_uri      (EvWindowTitle     *window_title,
			 		     const char        *uri);
void	       ev_window_title_free         (EvWindowTitle     *window_title);

G_END_DECLS

#endif /* __EV_WINDOW_TITLE_H__ */
