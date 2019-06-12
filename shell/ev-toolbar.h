/* ev-toolbar.h
 *  this file is part of xreader, a document viewer
 */

#ifndef __EV_TOOLBAR_H__
#define __EV_TOOLBAR_H__

#include <gtk/gtk.h>
#include "ev-window.h"

G_BEGIN_DECLS

#define EV_TYPE_TOOLBAR            (ev_toolbar_get_type())
#define EV_TOOLBAR(object)         (G_TYPE_CHECK_INSTANCE_CAST((object), EV_TYPE_TOOLBAR, EvToolbar))
#define EV_IS_TOOLBAR(object)      (G_TYPE_CHECK_INSTANCE_TYPE((object), EV_TYPE_TOOLBAR))
#define EV_TOOLBAR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), EV_TYPE_TOOLBAR, EvToolbarClass))
#define EV_IS_TOOLBAR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), EV_TYPE_TOOLBAR))
#define EV_TOOLBAR_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), EV_TYPE_TOOLBAR, EvToolbarClass))

typedef struct _EvToolbar        EvToolbar;
typedef struct _EvToolbarClass   EvToolbarClass;
typedef struct _EvToolbarPrivate EvToolbarPrivate;

struct _EvToolbar
{
    GtkToolbar parent_object;

    EvToolbarPrivate *priv;
};

struct _EvToolbarClass
{
    GtkToolbarClass parent_class;
};

GType ev_toolbar_get_type (void);
GtkWidget *ev_toolbar_new (EvWindow *window);

void ev_toolbar_set_style (EvToolbar *ev_toolbar,
                           gboolean   is_fullscreen);

void ev_toolbar_set_preset_sensitivity (EvToolbar *ev_toolbar,
                                        gboolean   sensitive);

void ev_toolbar_activate_reader_view (EvToolbar *ev_toolbar);
void ev_toolbar_activate_page_view (EvToolbar *ev_toolbar);

gboolean ev_toolbar_zoom_action_get_focused (EvToolbar *ev_toolbar);
void ev_toolbar_zoom_action_select_all (EvToolbar *ev_toolbar);

G_END_DECLS

#endif /* __EV_TOOLBAR_H__ */
