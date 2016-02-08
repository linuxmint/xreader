#ifndef __EPUB_DOCUMENT_H__
#define __EPUB_DOCUMENT_H__

#define _GNU_SOURCE
#include "ev-document.h"

G_BEGIN_DECLS

#define EPUB_TYPE_DOCUMENT             (epub_document_get_type ())
#define EPUB_DOCUMENT(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), EPUB_TYPE_DOCUMENT, EpubDocument))
#define EPUB_IS_DOCUMENT(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EPUB_TYPE_DOCUMENT))

typedef struct _EpubDocument EpubDocument;

GType                 epub_document_get_type (void) G_GNUC_CONST;

G_MODULE_EXPORT GType register_xreader_backend  (GTypeModule *module); 
     
G_END_DECLS

#endif /* __EPUB_DOCUMENT_H__ */
