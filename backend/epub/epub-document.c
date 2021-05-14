/* this file is part of xreader, a mate document viewer
 *
 *  Copyright (C) 2014 Avishkar Gupta
 *
 *  Author:
 *   Avishkar Gupta <avishkar.gupta.delhi@gmail.com>
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

#include "epub-document.h"
#include "ev-file-helpers.h"
#include "unzip.h"
#include "ev-document-thumbnails.h"
#include "ev-document-find.h"
#include "ev-backends-manager.h"
#include "ev-document-links.h"
#include "ev-document-misc.h"
#include <libxml/parser.h>
#include <libxml/xmlmemory.h>
#include <libxml/HTMLparser.h>
#include <config.h>

#include <glib/gi18n.h>
#include <glib/gstdio.h>

#include <gtk/gtk.h>

/*For strcasestr(),strstr()*/
#include <string.h>

typedef enum _xmlParseReturnType
{
    XML_ATTRIBUTE,
    XML_KEYWORD
}xmlParseReturnType;

typedef struct _contentListNode {
    gchar* key ;
    gchar* value ;
	gint index ;
}contentListNode;

typedef struct _linknode {
    gchar *pagelink;
	GList *children;
    gchar *linktext;
	guint page;
}linknode;

typedef struct _EpubDocumentClass EpubDocumentClass;

struct _EpubDocumentClass
{
    EvDocumentClass parent_class;
};

struct _EpubDocument
{
    EvDocument parent_instance;
	/*Stores the path to the source archive*/
    gchar* archivename ;
	/*Stores the path of the directory where we unzipped the epub*/
    gchar* tmp_archive_dir ;
	/*Stores the contentlist in a sorted manner*/
    GList* contentList ;
    /* A variable to hold our epubDocument for unzipping*/
    unzFile epubDocument ;
	/*The (sub)directory that actually houses the document*/
	gchar* documentdir;
	/*Stores the table of contents*/
	GList *index;
	/*Document title, for the sidebar links*/
	gchar *docTitle;
};

static void       epub_document_document_thumbnails_iface_init (EvDocumentThumbnailsInterface *iface);
static void       epub_document_document_find_iface_init       (EvDocumentFindInterface       *iface);
static void       epub_document_document_links_iface_init      (EvDocumentLinksInterface      *iface);

EV_BACKEND_REGISTER_WITH_CODE (EpubDocument, epub_document,
	{
		EV_BACKEND_IMPLEMENT_INTERFACE (EV_TYPE_DOCUMENT_THUMBNAILS,
						epub_document_document_thumbnails_iface_init);
		 EV_BACKEND_IMPLEMENT_INTERFACE (EV_TYPE_DOCUMENT_FIND,
								 epub_document_document_find_iface_init);
        EV_BACKEND_IMPLEMENT_INTERFACE (EV_TYPE_DOCUMENT_LINKS,
                                 epub_document_document_links_iface_init);

	} );

static void
epub_document_thumbnails_get_dimensions (EvDocumentThumbnails *document,
                                         EvRenderContext      *rc,
                                         gint                 *width,
                                         gint                 *height)
{
	gdouble page_width, page_height;

	page_width = 800;
	page_height = 1080;

	*width = MAX ((gint)(page_width * rc->scale + 0.5), 1);
	*height = MAX ((gint)(page_height * rc->scale + 0.5), 1);
}

static GdkPixbuf *
epub_document_thumbnails_get_thumbnail (EvDocumentThumbnails *document,
                                        EvRenderContext      *rc,
                                        gboolean              border)
{
	cairo_surface_t *webpage;
	GdkPixbuf *thumbnailpix = NULL ;
	gint width,height;
	epub_document_thumbnails_get_dimensions (document, rc, &width, &height);
	webpage = ev_document_misc_surface_rotate_and_scale (rc->page->backend_page,
	                                                     width, height, 0);
	thumbnailpix = ev_document_misc_pixbuf_from_surface (webpage);
	return thumbnailpix;
}

static gboolean
in_tag(const char* found)
{
    const char* bracket = found ;

    /* Since the dump started with the body tag, the '<' will be the first
     * character in the haystack.
     */
    while (*bracket != '<') {
        bracket--;
        if (*bracket == '>') {
            /*We encounted a close brace before an open*/
            return FALSE ;
        }
    }

    return TRUE;
}

static int
get_substr_count(const char * haystack,const char *needle,gboolean case_sensitive)
{
    const char* tmp = haystack ;
    char* (*string_compare_function)(const char*,const char*);
    int count=0;
    if (case_sensitive) {
        string_compare_function = strstr ;
    }
    else {
        string_compare_function = strcasestr;
    }

    while ((tmp=string_compare_function(tmp,needle))) {
        if (!in_tag(tmp)) {
            count++;
        }
        tmp = tmp + strlen(needle);
    }

    return count;
}

static guint
epub_document_check_hits(EvDocumentFind *document_find,
                         EvPage         *page,
                         const gchar    *text,
                         gboolean        case_sensitive)
{
	gchar *filepath = g_filename_from_uri((gchar*)page->backend_page,NULL,NULL);
	htmlDocPtr htmldoc =  xmlParseFile(filepath);
	
	if (htmldoc == NULL) {
		g_free (filepath);
		return 0;
	}
		
	htmlNodePtr htmltag = xmlDocGetRootElement(htmldoc);
	if(htmltag == NULL) {
		xmlFreeDoc(htmldoc);
		g_free (filepath);
		return 0;
	}
	
	int count=0;
	htmlNodePtr bodytag = htmltag->xmlChildrenNode;

	while ( xmlStrcmp(bodytag->name,(xmlChar*)"body") ) {
		bodytag = bodytag->next;
	}

	xmlBufferPtr bodybuffer = xmlBufferCreate();
	xmlNodeDump(bodybuffer,htmldoc,bodytag,0,1);

	count = get_substr_count((char*)bodybuffer->content,text,case_sensitive);

	xmlBufferFree(bodybuffer);
	xmlFreeDoc(htmldoc);
	g_free (filepath);

	return count;
}

static gboolean
epub_document_links_has_document_links(EvDocumentLinks *document_links)
{
    EpubDocument *epub_document = EPUB_DOCUMENT(document_links);

    g_return_val_if_fail(EPUB_IS_DOCUMENT(epub_document), FALSE);

    if (!epub_document->index)
        return FALSE;

    return TRUE;
}


typedef struct _LinksCBStruct {
	GtkTreeModel *model;
	GtkTreeIter  *parent;
}LinksCBStruct;

static void
epub_document_make_tree_entry(linknode* ListData,LinksCBStruct* UserData)
{
	GtkTreeIter tree_iter;
	EvLink *link = NULL;
	gboolean expand;
	char *title_markup;

	if (ListData->children) {
		expand=TRUE;
	}
	else {
		expand=FALSE;
	}

	EvLinkDest *ev_dest = NULL;
	EvLinkAction *ev_action;

	/* We shall use a EV_LINK_DEST_TYPE_PAGE for page links,
	 * and a EV_LINK_DEST_TYPE_HLINK(custom) for refs on a page of type url#label
	 * because we need both dest and page label for this.
	 */

	if (g_strrstr(ListData->pagelink,"#") == NULL) {
		ev_dest = ev_link_dest_new_page(ListData->page);
	}
	else {
		ev_dest = ev_link_dest_new_hlink((gchar*)ListData->pagelink,ListData->page);
	}

	ev_action = ev_link_action_new_dest (ev_dest);

	link = ev_link_new((gchar*)ListData->linktext,ev_action);

	gtk_tree_store_append (GTK_TREE_STORE (UserData->model), &tree_iter,(UserData->parent));
	title_markup = g_strdup((gchar*)ListData->linktext);

	gtk_tree_store_set (GTK_TREE_STORE (UserData->model), &tree_iter,
			    EV_DOCUMENT_LINKS_COLUMN_MARKUP, title_markup,
			    EV_DOCUMENT_LINKS_COLUMN_LINK, link,
			    EV_DOCUMENT_LINKS_COLUMN_EXPAND, expand,
			    -1);

	if (ListData->children) {
		LinksCBStruct cbstruct;
		cbstruct.parent = &tree_iter;
		cbstruct.model = UserData->model;
		g_list_foreach (ListData->children,(GFunc)epub_document_make_tree_entry,&cbstruct);
	}

	g_free (title_markup);
	g_object_unref (link);
}

static GtkTreeModel *
epub_document_links_get_links_model(EvDocumentLinks *document_links)
{
    GtkTreeModel *model = NULL;

	g_return_val_if_fail (EPUB_IS_DOCUMENT (document_links), NULL);

    EpubDocument *epub_document = EPUB_DOCUMENT(document_links);

    model = (GtkTreeModel*) gtk_tree_store_new (EV_DOCUMENT_LINKS_COLUMN_NUM_COLUMNS,
                                                G_TYPE_STRING,
                                                G_TYPE_OBJECT,
                                                G_TYPE_BOOLEAN,
                                                G_TYPE_STRING);

	LinksCBStruct linkStruct;
	linkStruct.model = model;
	EvLink *link = ev_link_new(epub_document->docTitle,
	                           ev_link_action_new_dest(ev_link_dest_new_page(0)));
	GtkTreeIter parent;

	linkStruct.parent = &parent;

	gtk_tree_store_append (GTK_TREE_STORE (model), &parent,NULL);

	gtk_tree_store_set (GTK_TREE_STORE (model), &parent,
			    EV_DOCUMENT_LINKS_COLUMN_MARKUP, epub_document->docTitle,
			    EV_DOCUMENT_LINKS_COLUMN_LINK, link,
			    EV_DOCUMENT_LINKS_COLUMN_EXPAND, TRUE,
			    -1);

	g_object_unref(link);

	if (epub_document->index) {
		g_list_foreach (epub_document->index,(GFunc)epub_document_make_tree_entry,&linkStruct);
	}

    return model;
}

static EvMappingList *
epub_document_links_get_links (EvDocumentLinks *document_links,
			       EvPage	       *page)
{
	/* TODO
	 * ev_mapping_list_new()
	 */
	return NULL;
}

static void
epub_document_document_thumbnails_iface_init (EvDocumentThumbnailsInterface *iface)
{
	iface->get_thumbnail = epub_document_thumbnails_get_thumbnail;
	iface->get_dimensions = epub_document_thumbnails_get_dimensions;
}

static void
epub_document_document_find_iface_init (EvDocumentFindInterface *iface)
{
	iface->check_for_hits = epub_document_check_hits;
}

static void
epub_document_document_links_iface_init(EvDocumentLinksInterface *iface)
{
    iface->has_document_links = epub_document_links_has_document_links;
    iface->get_links_model = epub_document_links_get_links_model;
    iface->get_links = epub_document_links_get_links;
}

static gboolean
epub_document_save (EvDocument *document,
                    const char *uri,
                    GError    **error)
{
    EpubDocument *epub_document = EPUB_DOCUMENT (document);

    gchar *source_uri = g_filename_to_uri (epub_document->archivename, NULL, error);
    if (source_uri == NULL)
        return FALSE;

    return ev_xfer_uri_simple (source_uri, uri, error);
}

static int
epub_document_get_n_pages (EvDocument *document)
{
    EpubDocument *epub_document = EPUB_DOCUMENT (document);

    if (epub_document-> contentList == NULL)
        return 0;

    return g_list_length(epub_document->contentList);
}

/**
 * epub_remove_temporary_dir : Removes a directory recursively.
 * This function is same as comics_remove_temporary_dir
 * Returns:
 *   	0 if it was successfully deleted,
 * 	-1 if an error occurred
 */
static int
epub_remove_temporary_dir (gchar *path_name)
{
	GDir  *content_dir;
	const gchar *filename;
	gchar *filename_with_path;

	if (g_file_test (path_name, G_FILE_TEST_IS_DIR)) {
		content_dir = g_dir_open  (path_name, 0, NULL);
		filename  = g_dir_read_name (content_dir);
		while (filename) {
			filename_with_path =
				g_build_filename (path_name,
						  filename, NULL);
			epub_remove_temporary_dir (filename_with_path);
			g_free (filename_with_path);
			filename = g_dir_read_name (content_dir);
		}
		g_dir_close (content_dir);
	}
	/* Note from g_remove() documentation: on Windows, it is in general not
	 * possible to remove a file that is open to some process, or mapped
	 * into memory.*/
	return (g_remove (path_name));
}


static gboolean
check_mime_type             (const gchar* uri,
                             GError** error);

static gboolean
open_xml_document           (const gchar* filename);

static gboolean
set_xml_root_node           (xmlChar* rootname);

static xmlNodePtr
xml_get_pointer_to_node     (xmlChar* parserfor,
                             xmlChar* attributename,
                             xmlChar* attributevalue);
static void
xml_parse_children_of_node  (xmlNodePtr parent,
                             xmlChar* parserfor,
                             xmlChar* attributename,
                             xmlChar* attributevalue);

static gboolean
xml_check_attribute_value   (xmlNode* node,
                             xmlChar * attributename,
                             xmlChar* attributevalue);

static xmlChar*
xml_get_data_from_node      (xmlNodePtr node,
                             xmlParseReturnType rettype,
                             xmlChar* attributename);

static void
xml_free_doc();

static void
free_tree_nodes             (gpointer data);

/*Global variables for XML parsing*/
static xmlDocPtr    xmldocument ;
static xmlNodePtr   xmlroot ;
static xmlNodePtr   xmlretval ;

/*
**Functions to parse the xml files.
**Open a XML document for reading
*/
static gboolean
open_xml_document ( const gchar* filename )
{
	xmldocument = xmlParseFile(filename);

	if ( xmldocument == NULL )
	{
		return FALSE ;
	}
	else
	{
		return TRUE ;
	}
}

/**
 *Check if the root value is same as rootname .
 *if supplied rootvalue = NULL ,just set root to rootnode .
**/
static gboolean
set_xml_root_node(xmlChar* rootname)
{
	xmlroot = xmlDocGetRootElement(xmldocument);

	if (xmlroot == NULL) {

		xmlFreeDoc(xmldocument);
		return FALSE;
	}

    if ( rootname == NULL )
    {
        return TRUE ;
    }

    if ( !xmlStrcmp(xmlroot->name,rootname))
    {
        return TRUE ;
    }
    else
    {
	   return FALSE;
    }
}

static xmlNodePtr
xml_get_pointer_to_node(xmlChar* parserfor,
                        xmlChar*  attributename,
                        xmlChar* attributevalue )
{
    xmlNodePtr topchild;

    xmlretval = NULL ;

    if ( !xmlStrcmp( xmlroot->name, parserfor) )
    {
        return xmlroot ;
    }

    topchild = xmlroot->xmlChildrenNode ;

    while ( topchild != NULL )
    {
        if ( !xmlStrcmp(topchild->name,parserfor) )
        {
            if ( xml_check_attribute_value(topchild,attributename,attributevalue) == TRUE )
            {
                 xmlretval = topchild;
                 return xmlretval;
            }
            else
            {
                /*No need to parse children node*/
                topchild = topchild->next ;
                continue ;
            }
        }

        xml_parse_children_of_node(topchild , parserfor, attributename, attributevalue) ;

        topchild = topchild->next ;
    }

    return xmlretval ;
}

static void
xml_parse_children_of_node(xmlNodePtr parent,
                           xmlChar* parserfor,
                           xmlChar* attributename,
                           xmlChar* attributevalue )
{
    xmlNodePtr child = parent->xmlChildrenNode ;

    while ( child != NULL )
    {
        if ( !xmlStrcmp(child->name,parserfor))
        {
            if ( xml_check_attribute_value(child,attributename,attributevalue) == TRUE )
            {
                 xmlretval = child;
                 return ;
            }
            else
            {
                /*No need to parse children node*/
                child = child->next ;
                continue ;
            }
        }

        /*return already if we have xmlretval set*/
        if ( xmlretval != NULL )
        {
            return ;
        }

        xml_parse_children_of_node(child,parserfor,attributename,attributevalue) ;
        child = child->next ;
    }
}

static void
xml_free_doc()
{
    xmlFreeDoc(xmldocument);
	xmldocument = NULL;
}

static gboolean
xml_check_attribute_value(xmlNode* node,
                          xmlChar * attributename,
                          xmlChar* attributevalue)
{
    xmlChar* attributefromfile ;
    if ( attributename == NULL || attributevalue == NULL )
    {
         return TRUE ;
    }
    else if ( !xmlStrcmp(( attributefromfile = xmlGetProp(node,attributename)),
                           attributevalue) )
    {
        xmlFree(attributefromfile);
        return TRUE ;
    }
    xmlFree(attributefromfile);
    return FALSE ;
}

static xmlChar*
xml_get_data_from_node(xmlNodePtr node,
                       xmlParseReturnType rettype,
                       xmlChar* attributename)
{
    xmlChar* datastring ;
    if ( rettype == XML_ATTRIBUTE )
       datastring= xmlGetProp(node,attributename);
    else
       datastring= xmlNodeListGetString(xmldocument,node->xmlChildrenNode, 1);

    return datastring;
}

static gboolean
check_mime_type(const gchar* uri,GError** error)
{
    GError * err = NULL ;
    const gchar* mimeFromFile = ev_file_get_mime_type(uri,FALSE,&err);

    gchar* mimetypes[] = {"application/epub+zip","application/x-booki+zip"};
    int typecount = 2;
    if ( !mimeFromFile )
    {
        if (err)    {
            g_propagate_error (error, err);
        }
        else    {
            g_set_error_literal (error,
                         EV_DOCUMENT_ERROR,
                         EV_DOCUMENT_ERROR_INVALID,
                         _("Unknown MIME Type"));
        }
        return FALSE;
    }
    else
    {
        int i=0;
        for (i=0; i < typecount ;i++) {
           if ( g_strcmp0(mimeFromFile, mimetypes[i]) == 0  ) {
                return TRUE;
           }
        }

        /*We didn't find a match*/
        g_set_error_literal (error,
                     EV_DOCUMENT_ERROR,
                     EV_DOCUMENT_ERROR_INVALID,
                     _("Not an ePub document"));

        return FALSE;
    }
}

static gboolean
extract_one_file(EpubDocument* epub_document,GError ** error)
{
    GFile * outfile ;
    gsize writesize = 0;
    GString * gfilepath ;
    unz_file_info64 info ;
    gchar* directory;
	GString* dir_create;
    GFileOutputStream * outstream ;

    if ( unzOpenCurrentFile(epub_document->epubDocument) != UNZ_OK )
    {
            return FALSE ;
    }

    gboolean result = TRUE;

    gpointer currentfilename = g_malloc0(512);
    unzGetCurrentFileInfo64(epub_document->epubDocument,&info,currentfilename,512,NULL,0,NULL,0) ;
    directory = g_strrstr(currentfilename,"/") ;

    if ( directory != NULL )
        directory++;

    gfilepath = g_string_new(epub_document->tmp_archive_dir) ;
    g_string_append_printf(gfilepath,"/%s",(gchar*)currentfilename);
    
    // handle the html extension (IssueID #266)
    if (g_strrstr(currentfilename, ".html") != NULL)
        g_string_insert_c (gfilepath, gfilepath->len-4, 'x');

    /*if we encounter a directory, make a directory inside our temporary folder.*/
    if (directory != NULL && *directory == '\0')
    {
        g_mkdir(gfilepath->str,0777);
        goto out;
    }
    else if (directory != NULL && *directory != '\0' ) {
        gchar* createdir = currentfilename;
        /*Since a substring can't be longer than the parent string, allocating space equal to the parent's size should suffice*/
        gchar *createdirname = g_malloc0(strlen(currentfilename));
        /* Add the name of the directory and subdirectories,if any to a buffer and then create it */
        gchar *createdirnametemp = createdirname;
        while ( createdir != directory ) {
            (*createdirnametemp) = (*createdir);
            createdirnametemp++;
            createdir++;
        }
        (*createdirnametemp) = '\0';

        dir_create = g_string_new(epub_document->tmp_archive_dir);
        g_string_append_printf(dir_create,"/%s",createdirname);
        g_free(createdirname);

        g_mkdir_with_parents(dir_create->str,0777);
		g_string_free(dir_create,TRUE);
    }

    outfile = g_file_new_for_path(gfilepath->str);
    outstream = g_file_create(outfile,G_FILE_CREATE_PRIVATE,NULL,error);
    gpointer buffer = g_malloc0(512);
    while ( (writesize = unzReadCurrentFile(epub_document->epubDocument,buffer,512) ) != 0 )
    {
        if ( g_output_stream_write((GOutputStream*)outstream,buffer,writesize,NULL,error) == -1 )
        {
            result = FALSE;
            break;
        }
    }
    g_free(buffer);
    g_output_stream_close((GOutputStream*)outstream,NULL,error);
    g_object_unref(outfile) ;
    g_object_unref(outstream) ;

out:
    unzCloseCurrentFile (epub_document->epubDocument) ;
    g_string_free(gfilepath,TRUE);
    g_free(currentfilename);
	return result;
}

static gboolean
extract_epub_from_container (const gchar* uri,
                             EpubDocument *epub_document,
                             GError ** error)
{
    GError *err = NULL;
    epub_document->archivename = g_filename_from_uri(uri,NULL,error);

    if ( !epub_document->archivename )
    {
        if (err) {
            g_propagate_error (error, err);
        }
        else {
            g_set_error_literal (error,
                         EV_DOCUMENT_ERROR,
                         EV_DOCUMENT_ERROR_INVALID,
                         _("could not retrieve filename"));
        }
        return FALSE;
    }

    gchar *epubfilename = g_strrstr(epub_document->archivename,"/");
    if ( *epubfilename == '/' )
        epubfilename++ ;

    GString *temporary_sub_directory = g_string_new(epubfilename);
    g_string_append(temporary_sub_directory,"XXXXXX") ;
    epub_document->tmp_archive_dir = ev_mkdtemp(temporary_sub_directory->str, error);
    g_string_free(temporary_sub_directory, TRUE);

    if (!epub_document->tmp_archive_dir) {
        return FALSE;
    }

    epub_document->epubDocument = unzOpen64(epub_document->archivename);
    if ( epub_document->epubDocument == NULL )
    {
        if (err)    {
            g_propagate_error (error, err);
        }
        else    {
            g_set_error_literal (error,
                         EV_DOCUMENT_ERROR,
                         EV_DOCUMENT_ERROR_INVALID,
                         _("could not open archive"));
        }
        return FALSE;
    }

    gboolean result = FALSE;

    if ( unzGoToFirstFile(epub_document->epubDocument) != UNZ_OK )
    {
        if (err) {
            g_propagate_error (error, err);
        }
        else    {
            g_set_error_literal (error,
                         EV_DOCUMENT_ERROR,
                         EV_DOCUMENT_ERROR_INVALID,
                         _("could not extract archive"));
        }
        goto out;
    }

    while ( TRUE )
    {
        if ( extract_one_file(epub_document,&err) == FALSE )
        {
            if (err) {
                g_propagate_error (error, err);
            }
            else    {
                g_set_error_literal (error,
                             EV_DOCUMENT_ERROR,
                             EV_DOCUMENT_ERROR_INVALID,
                             _("could not extract archive"));
            }
			goto out;
        }

        if ( unzGoToNextFile(epub_document->epubDocument) == UNZ_END_OF_LIST_OF_FILE ) {
            result = TRUE;
            break;
        }
    }

out:
    unzClose(epub_document->epubDocument);
    return result;
}

static gchar*
get_uri_to_content(const gchar* uri,GError ** error,EpubDocument *epub_document)
{
	gchar* tmp_archive_dir = epub_document->tmp_archive_dir;
    GError *err = NULL ;

    gchar *containerpath = g_filename_from_uri(uri,NULL,&err);
    if ( !containerpath )
    {
        if (err) {
            g_propagate_error (error,err);
        }
        else    {
            g_set_error_literal (error,
                                 EV_DOCUMENT_ERROR,
                                 EV_DOCUMENT_ERROR_INVALID,
                                 _("could not retrieve container file"));
        }
        return NULL ;
    }

    gboolean result = open_xml_document(containerpath);
    g_free (containerpath);
    if ( result == FALSE )
    {
        g_set_error_literal(error,
                            EV_DOCUMENT_ERROR,
                            EV_DOCUMENT_ERROR_INVALID,
                            _("could not open container file"));

        return NULL ;
    }

    if ( set_xml_root_node((xmlChar*)"container") == FALSE)  {

        g_set_error_literal(error,
                            EV_DOCUMENT_ERROR,
                            EV_DOCUMENT_ERROR_INVALID,
                            _("container file is corrupt"));
        return NULL ;
    }

    xmlNodePtr rootfileNode = xml_get_pointer_to_node((xmlChar*)"rootfile",(xmlChar*)"media-type",(xmlChar*)"application/oebps-package+xml");
    if ( rootfileNode == NULL)
    {
        g_set_error_literal(error,
                            EV_DOCUMENT_ERROR,
                            EV_DOCUMENT_ERROR_INVALID,
                            _("epub file is invalid or corrupt"));
        return NULL ;
    }

    xmlChar *relativepath = xml_get_data_from_node(rootfileNode,XML_ATTRIBUTE,(xmlChar*)"full-path") ;
    if ( relativepath == NULL )
    {
        g_set_error_literal(error,
                            EV_DOCUMENT_ERROR,
                            EV_DOCUMENT_ERROR_INVALID,
                            _("epub file is corrupt, no container"));
        return NULL ;
    }

	gchar* documentfolder = g_strrstr((gchar*)relativepath,"/");
	if (documentfolder != NULL) {
		gchar* copybuffer = (gchar*)relativepath ;
		gchar* directorybuffer = g_malloc0(sizeof(gchar*)*100);
		gchar* writer = directorybuffer;

		while(copybuffer != documentfolder) {
			(*writer) = (*copybuffer);
			writer++;copybuffer++;
		}
		*writer = '\0';

		GString *documentdir = g_string_new(tmp_archive_dir);
		g_string_append_printf(documentdir,"/%s",directorybuffer);
		g_free(directorybuffer);
		epub_document->documentdir = g_string_free(documentdir,FALSE);
	}
	else
	{
		epub_document->documentdir = g_strdup(tmp_archive_dir);
	}

	GString *absolutepath = g_string_new(tmp_archive_dir);
    g_string_append_printf(absolutepath,"/%s",relativepath);
    g_free (relativepath);

    gchar *content_uri = g_filename_to_uri(absolutepath->str,NULL,&err);
    g_string_free(absolutepath,TRUE);
    if ( !content_uri )  {
        if (err) {
            g_propagate_error (error,err);
        }
        else
        {
            g_set_error_literal (error,
                                 EV_DOCUMENT_ERROR,
                                 EV_DOCUMENT_ERROR_INVALID,
                                 _("could not retrieve container file"));
        }
        return NULL ;
    }
	xml_free_doc();
    return content_uri ;
}

static gboolean
link_present_on_page(const gchar* link,const gchar *page_uri)
{
	gchar *res;
	if ((res=g_strrstr(link, page_uri)) != NULL) {
		return TRUE;
	}
	else {
		return FALSE;
	}
}

static void
check_add_page_numbers(linknode *listdata, contentListNode *comparenode)
{
    if (link_present_on_page(listdata->pagelink, comparenode->value)) {
		listdata->page = comparenode->index - 1;
	}
    if (listdata->children != NULL) {
        g_list_foreach(listdata->children,(GFunc)check_add_page_numbers,comparenode);
    }
}

static GList*
setup_document_content_list(const gchar* content_uri, GError** error,gchar *documentdir)
{
    GError *err = NULL;
    gint indexcounter = 1;
    xmlNodePtr manifest,spine,itemrefptr,itemptr;
    gboolean errorflag = FALSE;

    if ( open_xml_document(content_uri) == FALSE )
    {
        g_set_error_literal(error,
                            EV_DOCUMENT_ERROR,
                            EV_DOCUMENT_ERROR_INVALID,
                            _("could not parse content manifest"));

        return FALSE ;
    }
    if ( set_xml_root_node((xmlChar*)"package") == FALSE)  {

        g_set_error_literal(error,
                            EV_DOCUMENT_ERROR,
                            EV_DOCUMENT_ERROR_INVALID,
                            _("content file is invalid"));
        return FALSE ;
    }

    if ( ( spine = xml_get_pointer_to_node((xmlChar*)"spine",NULL,NULL) )== NULL )
    {
         g_set_error_literal(error,
                            EV_DOCUMENT_ERROR,
                            EV_DOCUMENT_ERROR_INVALID,
                            _("epub file has no spine"));
        return FALSE ;
    }

    if ( ( manifest = xml_get_pointer_to_node((xmlChar*)"manifest",NULL,NULL) )== NULL )
    {
         g_set_error_literal(error,
                            EV_DOCUMENT_ERROR,
                            EV_DOCUMENT_ERROR_INVALID,
                            _("epub file has no manifest"));
        return FALSE ;
    }

    xmlretval = NULL ;

    /*Get first instance of itemref from the spine*/
    xml_parse_children_of_node(spine,(xmlChar*)"itemref",NULL,NULL);

    if ( xmlretval != NULL )
        itemrefptr = xmlretval ;
    else
    {
        errorflag=TRUE;
    }

    GList *newlist = NULL;

    /*Parse the spine for remaining itemrefs*/
    do
    {
        /*for the first time that we enter the loop, if errorflag is set we break*/
        if ( errorflag )
        {
            break;
        }
        if ( xmlStrcmp(itemrefptr->name,(xmlChar*)"itemref") == 0)
        {
            contentListNode *newnode = g_malloc0(sizeof(newnode));
            newnode->key = (gchar*)xml_get_data_from_node(itemrefptr,XML_ATTRIBUTE,(xmlChar*)"idref");
            if ( newnode->key == NULL )
            {
                g_free (newnode);
                errorflag = TRUE;
                break;
            }
            xmlretval=NULL ;
            xml_parse_children_of_node(manifest,(xmlChar*)"item",(xmlChar*)"id",(xmlChar*)newnode->key);

            if ( xmlretval != NULL )
            {
                itemptr = xmlretval ;
            }
            else
            {
                g_free (newnode->key);
                g_free (newnode);
                errorflag = TRUE;
                break;
            }


            GString* absolutepath = g_string_new(documentdir);
            gchar *relativepath = (gchar*)xml_get_data_from_node(itemptr,XML_ATTRIBUTE,(xmlChar*)"href");
            g_string_append_printf(absolutepath,"/%s",relativepath);

            // Handle the html extension (IssueID #266)
           if (g_strrstr(relativepath, ".html") != NULL)
                g_string_insert_c (absolutepath, absolutepath->len-4, 'x');
            g_free (relativepath);
            
            newnode->value = g_filename_to_uri(absolutepath->str,NULL,&err);
            g_string_free(absolutepath, TRUE);

            if ( newnode->value == NULL )
            {
                g_free (newnode->key);
                g_free (newnode);
                errorflag = TRUE;
                break;
            }

			newnode->index = indexcounter++ ;

            newlist = g_list_prepend(newlist, newnode);
        }
        itemrefptr = itemrefptr->next ;
    }
    while ( itemrefptr != NULL );

    if ( errorflag )
    {
        if ( err )
        {
            g_propagate_error(error,err);
        }
        else
        {
            g_set_error_literal(error,
                                EV_DOCUMENT_ERROR,
                                EV_DOCUMENT_ERROR_INVALID,
                                _("Could not set up document tree for loading, some files missing"));
        }
        /*free any nodes that were set up and return empty*/
        g_list_free_full(newlist, (GDestroyNotify)free_tree_nodes);
        return NULL;
    }

	newlist = g_list_reverse(newlist);
	xml_free_doc();
    return newlist;
}

/* Callback function to free the contentlist.*/
static void
free_tree_nodes(gpointer data)
{
    contentListNode* dataptr = data ;
    g_free(dataptr->value);
    g_free(dataptr->key);
    g_free(dataptr);
}

static void
free_link_nodes(gpointer data)
{
    linknode* dataptr = data ;
    g_free(dataptr->pagelink);
    g_free(dataptr->linktext);

	if (dataptr->children) {
		g_list_free_full(dataptr->children,(GDestroyNotify)free_link_nodes);
	}
    g_free(dataptr);
}

static gchar*
get_toc_file_name(gchar *containeruri)
{
	gchar *containerfilename = g_filename_from_uri(containeruri,NULL,NULL);
	open_xml_document(containerfilename);
	g_free (containerfilename);

	set_xml_root_node(NULL);

	xmlNodePtr manifest = xml_get_pointer_to_node((xmlChar*)"manifest",NULL,NULL);
	xmlNodePtr spine = xml_get_pointer_to_node((xmlChar*)"spine",NULL,NULL);

	xmlChar *ncx = xml_get_data_from_node(spine,XML_ATTRIBUTE,(xmlChar*)"toc");

    /*In an epub3, there is sometimes no toc, and we need to then use the nav file for this.*/
    if (ncx == NULL) {
        return NULL;
    }

	xmlretval = NULL;
	xml_parse_children_of_node(manifest,(xmlChar*)"item",(xmlChar*)"id",ncx);

	gchar* tocfilename = (gchar*)xml_get_data_from_node(xmlretval,XML_ATTRIBUTE,(xmlChar*)"href");
	xml_free_doc();

	return tocfilename;
}

static gchar*
epub_document_get_nav_file(gchar* containeruri)
{
    open_xml_document(containeruri);
    set_xml_root_node(NULL);
    xmlNodePtr manifest = xml_get_pointer_to_node((xmlChar*)"manifest",NULL,NULL);
    xmlretval = NULL;
    xml_parse_children_of_node(manifest,(xmlChar*)"item",(xmlChar*)"properties",(xmlChar*)"nav");

    gchar *uri = (gchar*)xml_get_data_from_node(xmlretval,XML_ATTRIBUTE, (xmlChar*)"href");

    xml_free_doc();
    return uri;
}

static GList*
get_child_list(xmlNodePtr ol,gchar* documentdir)
{
    GList *childlist = NULL;
    xmlNodePtr li = ol->xmlChildrenNode;

    while (li != NULL) {
		if (xmlStrcmp(li->name,(xmlChar*)"li")) {
			li = li->next;
			continue;
		}
        xmlNodePtr children = li->xmlChildrenNode;
        linknode *newlinknode = g_new0(linknode, 1);
        while (children != NULL) {
            if ( !xmlStrcmp(children->name,(xmlChar*)"a")) {
                newlinknode->linktext = (gchar*)xml_get_data_from_node(children,XML_KEYWORD,NULL);
                gchar* filename = (gchar*)xml_get_data_from_node(children,XML_ATTRIBUTE,(xmlChar*)"href");
				gchar *filepath = g_strdup_printf("%s/%s",documentdir,filename);
				newlinknode->pagelink = g_filename_to_uri(filepath,NULL,NULL);
				g_free(filename);
				g_free(filepath);
                newlinknode->children = NULL;
                childlist = g_list_prepend(childlist,newlinknode);
            }
            else if ( !xmlStrcmp(children->name,(xmlChar*)"ol")){
                newlinknode->children = get_child_list(children,documentdir);
            }

			children = children->next;
        }

        li = li->next;
    }

    return g_list_reverse(childlist);
}

/* For an epub3 style navfile */
static GList*
setup_index_from_navfile(gchar *tocpath)
{
    GList *index = NULL;
    open_xml_document(tocpath);
    set_xml_root_node(NULL);
    xmlNodePtr nav = xml_get_pointer_to_node((xmlChar*)"nav",(xmlChar*)"id",(xmlChar*)"toc");
    xmlretval=NULL;
    xml_parse_children_of_node(nav,(xmlChar*)"ol", NULL,NULL);
	gchar *navdirend = g_strrstr(tocpath,"/");
	gchar *navdir = g_malloc0(strlen(tocpath));
	gchar *reader = tocpath;
	gchar *writer = navdir;

	while (reader != navdirend) {
		(*writer) = (*reader) ;
		writer++;reader++;
	}
    index = get_child_list(xmlretval,navdir);
	g_free(navdir);
    xml_free_doc();
    return index;
}

static GList*
setup_document_children(EpubDocument *epub_document,xmlNodePtr node)
{
    GList *index = NULL;
    
    xmlretval = NULL;
    xml_parse_children_of_node(node,(xmlChar*)"navPoint",NULL,NULL);
    xmlNodePtr navPoint = xmlretval;
    
    while(navPoint != NULL) {
    
        if ( !xmlStrcmp(navPoint->name,(xmlChar*)"navPoint")) {
    		xmlretval = NULL;
    		xml_parse_children_of_node(navPoint,(xmlChar*)"navLabel",NULL,NULL);
    		xmlNodePtr navLabel = xmlretval;
    		xmlretval = NULL;
    		gchar *fragment=NULL,*end=NULL;
    		GString *uri = NULL;
            GString *pagelink = NULL;

    		xml_parse_children_of_node(navLabel,(xmlChar*)"text",NULL,NULL);
    		
            linknode *newnode = g_new0(linknode,1);
            newnode->linktext = NULL;
            while (newnode->linktext == NULL) {
	            newnode->linktext = (gchar*)xml_get_data_from_node(xmlretval,XML_KEYWORD,NULL);
	            xmlretval = xmlretval->next;
            }
           
            xmlretval = NULL;
            xml_parse_children_of_node(navPoint,(xmlChar*)"content",NULL,NULL);
            pagelink = g_string_new(epub_document->documentdir);
            newnode->pagelink = (gchar*)xml_get_data_from_node(xmlretval,XML_ATTRIBUTE,(xmlChar*)"src");
            g_string_append_printf(pagelink,"/%s",newnode->pagelink);

            xmlFree(newnode->pagelink);

            gchar *escaped = g_strdup(pagelink->str);

            //unescaping any special characters
            pagelink->str = g_uri_unescape_string (escaped,NULL);
            g_free(escaped);

            if ((end = g_strrstr(pagelink->str,"#")) != NULL) {
	            fragment = g_strdup(g_strrstr(pagelink->str,"#"));
	            *end = '\0';
            }
            
            uri = g_string_new(g_filename_to_uri(pagelink->str,NULL,NULL));
            
            // handle the html extension (IssueID #266)
            if (g_strrstr(uri->str, ".html") != NULL)
                g_string_insert_c (uri, uri->len-4, 'x');
                
            g_string_free(pagelink,TRUE);

            if (fragment) {
	            g_string_append(uri,fragment);
            }

            newnode->pagelink = g_strdup(uri->str);
            newnode->children = setup_document_children(epub_document, navPoint);
            g_string_free(uri,TRUE);
            index = g_list_prepend(index,newnode);
        } 

        navPoint = navPoint->next;
    }
    
    return g_list_reverse (index);
}

static GList*
setup_document_index(EpubDocument *epub_document,gchar *containeruri)
{
    GString *tocpath = g_string_new(epub_document->documentdir);
    gchar *tocfilename = get_toc_file_name(containeruri);
    GList *index = NULL;

    if (tocfilename == NULL) {
        tocfilename = epub_document_get_nav_file(containeruri);

        //Apparently, sometimes authors don't even care to add a TOC!! Guess standards are just guidelines.

        if (tocfilename == NULL) {
            //We didn't even find a nav file.The document has no TOC.
            g_string_free(tocpath,TRUE);
            return NULL;
        }

        g_string_append_printf (tocpath,"/%s",tocfilename);
        index = setup_index_from_navfile(tocpath->str);
        g_string_free(tocpath,TRUE);
        g_free (tocfilename);
        return index;
    }

    g_string_append_printf (tocpath,"/%s",tocfilename);
    g_free (tocfilename);

    open_xml_document(tocpath->str);
    g_string_free(tocpath,TRUE);
    set_xml_root_node((xmlChar*)"ncx");

	xmlNodePtr docTitle = xml_get_pointer_to_node((xmlChar*)"docTitle",NULL,NULL);
	xmlretval = NULL;
	xml_parse_children_of_node(docTitle,(xmlChar*)"text",NULL,NULL);

	while (epub_document->docTitle == NULL && xmlretval != NULL) {
		epub_document->docTitle = (gchar*)xml_get_data_from_node(xmlretval,XML_KEYWORD,NULL);
		xmlretval = xmlretval->next;
	}
    xmlNodePtr navMap = xml_get_pointer_to_node((xmlChar*)"navMap",NULL,NULL);
    index = setup_document_children (epub_document, navMap);
    
	xml_free_doc();
    return index;
}

static EvDocumentInfo*
epub_document_get_info(EvDocument *document)
{
	EpubDocument *epub_document = EPUB_DOCUMENT(document);
	GError *error = NULL ;
	gchar* infofile ;
	xmlNodePtr metanode ;
	GString* buffer ;

	GString* containerpath = g_string_new(epub_document->tmp_archive_dir);
	g_string_append_printf(containerpath,"/META-INF/container.xml");
	gchar* containeruri = g_filename_to_uri(containerpath->str,NULL,&error);
	g_string_free (containerpath, TRUE);
	if ( error )
	{
		return NULL ;
	}

	gchar* uri = get_uri_to_content (containeruri,&error,epub_document);
	g_free (containeruri);
	if ( error )
	{
		return NULL ;
	}

	EvDocumentInfo* epubinfo = g_new0 (EvDocumentInfo, 1);

	epubinfo->fields_mask = EV_DOCUMENT_INFO_TITLE |
			    EV_DOCUMENT_INFO_FORMAT |
			    EV_DOCUMENT_INFO_AUTHOR |
			    EV_DOCUMENT_INFO_SUBJECT |
			    EV_DOCUMENT_INFO_KEYWORDS |
			    EV_DOCUMENT_INFO_LAYOUT |
			    EV_DOCUMENT_INFO_CREATOR |
			    EV_DOCUMENT_INFO_LINEARIZED |
				EV_DOCUMENT_INFO_PERMISSIONS |
			    EV_DOCUMENT_INFO_N_PAGES ;

	infofile = g_filename_from_uri(uri,NULL,&error);
	g_free (uri);
	if ( error )
	{
		return epubinfo;
	}

	open_xml_document(infofile);
	g_free (infofile);

	set_xml_root_node((xmlChar*)"package");

	metanode = xml_get_pointer_to_node((xmlChar*)"title",NULL,NULL);
	if ( metanode == NULL )
	  epubinfo->title = NULL ;
	else
	  epubinfo->title = (char*)xml_get_data_from_node(metanode,XML_KEYWORD,NULL);

	metanode = xml_get_pointer_to_node((xmlChar*)"creator",NULL,NULL);
	if ( metanode == NULL )
	  epubinfo->author = g_strdup("unknown");
	else
	  epubinfo->author = (char*)xml_get_data_from_node(metanode,XML_KEYWORD,NULL);

	metanode = xml_get_pointer_to_node((xmlChar*)"subject",NULL,NULL);
	if ( metanode == NULL )
	   epubinfo->subject = g_strdup("unknown");
	else
	   epubinfo->subject = (char*)xml_get_data_from_node(metanode,XML_KEYWORD,NULL);

	buffer = g_string_new((gchar*)xml_get_data_from_node (xmlroot,XML_ATTRIBUTE,(xmlChar*)"version"));
	g_string_prepend(buffer,"epub ");
	epubinfo->format = g_string_free(buffer,FALSE);

	/*FIXME: Add more of these as you write the corresponding modules*/

	epubinfo->layout = EV_DOCUMENT_LAYOUT_SINGLE_PAGE;

	metanode = xml_get_pointer_to_node((xmlChar*)"publisher",NULL,NULL);
	if ( metanode == NULL )
	   epubinfo->creator = g_strdup("unknown");
	else
	   epubinfo->creator = (char*)xml_get_data_from_node(metanode,XML_KEYWORD,NULL);

	/* number of pages */
	epubinfo->n_pages = epub_document_get_n_pages(document);

	/*Copying*/
	epubinfo->permissions = EV_DOCUMENT_PERMISSIONS_OK_TO_COPY;
	/*TODO : Add a function to get date*/

	if (xmldocument)
		xml_free_doc();
	return epubinfo ;
}

static EvPage*
epub_document_get_page(EvDocument *document,
                       gint index)
{
	EpubDocument *epub_document = EPUB_DOCUMENT(document);
	EvPage* page = ev_page_new(index);
	contentListNode *listptr = g_list_nth_data (epub_document->contentList,index);
	page->backend_page = g_strdup(listptr->value);

	return page ;
}

static void
change_to_night_sheet(contentListNode *nodedata,gpointer user_data)
{
    gchar *filename = g_filename_from_uri(nodedata->value,NULL,NULL);
    open_xml_document(filename);
    set_xml_root_node(NULL);
    xmlNodePtr head =xml_get_pointer_to_node((xmlChar*)"head",NULL,NULL);
	gchar *class = NULL;
    xmlretval = NULL;
    xml_parse_children_of_node(head,(xmlChar*)"link",(xmlChar*)"rel",(xmlChar*)"stylesheet");

    xmlNodePtr day = xmlretval;
	if ( (class = (gchar*)xml_get_data_from_node(day,XML_ATTRIBUTE,(xmlChar*)"class")) == NULL) {
		xmlSetProp(day,(xmlChar*)"class",(xmlChar*)"day");
	}
	g_free(class);
    xmlSetProp(day,(xmlChar*)"rel",(xmlChar*)"alternate stylesheet");
    xmlretval = NULL;
    xml_parse_children_of_node(head,(xmlChar*)"link",(xmlChar*)"class",(xmlChar*)"night");
    xmlSetProp(xmlretval,(xmlChar*)"rel",(xmlChar*)"stylesheet");
    xmlSaveFormatFile (filename, xmldocument, 0);
    xml_free_doc();
    g_free(filename);
}

static void
change_to_day_sheet(contentListNode *nodedata,gpointer user_data)
{
    gchar *filename = g_filename_from_uri(nodedata->value,NULL,NULL);
    open_xml_document(filename);
    set_xml_root_node(NULL);
    xmlNodePtr head =xml_get_pointer_to_node((xmlChar*)"head",NULL,NULL);

    xmlretval = NULL;
    xml_parse_children_of_node(head,(xmlChar*)"link",(xmlChar*)"rel",(xmlChar*)"stylesheet");

    xmlNodePtr day = xmlretval;
    xmlSetProp(day,(xmlChar*)"rel",(xmlChar*)"alternate stylesheet");

    xmlretval = NULL;
    xml_parse_children_of_node(head,(xmlChar*)"link",(xmlChar*)"class",(xmlChar*)"day");
    xmlSetProp(xmlretval,(xmlChar*)"rel",(xmlChar*)"stylesheet");
    xmlSaveFormatFile (filename, xmldocument, 0);
    xml_free_doc();
    g_free(filename);
}

static gchar*
epub_document_get_alternate_stylesheet(gchar *docuri)
{
    gchar *filename = g_filename_from_uri(docuri,NULL,NULL);
    open_xml_document(filename);
    g_free(filename);

    set_xml_root_node(NULL);

    xmlNodePtr head= xml_get_pointer_to_node((xmlChar*)"head",NULL,NULL);

    xmlretval = NULL;

    xml_parse_children_of_node(head,(xmlChar*)"link",(xmlChar*)"class",(xmlChar*)"night");

    if (xmlretval != NULL) {
        return (gchar*)xml_get_data_from_node(xmlretval,XML_ATTRIBUTE,(xmlChar*)"href");
    }
    xml_free_doc();
    return NULL;
}

static void
add_night_sheet(contentListNode *listdata,gchar *sheet)
{
    gchar *sheeturi = g_filename_to_uri(sheet,NULL,NULL);
    open_xml_document(listdata->value);

    set_xml_root_node(NULL);

    xmlSaveFormatFile (listdata->value, xmldocument, 0);
    xml_free_doc();
    g_free(sheeturi);
}

static void
epub_document_check_add_night_sheet(EvDocument *document)
{
    EpubDocument *epub_document = EPUB_DOCUMENT(document);

    g_return_if_fail(EPUB_IS_DOCUMENT(epub_document));

    /*
     * We'll only check the first page for a supplied night mode stylesheet.
     * Odds are, if this one has it, all others have it too.
     */
	contentListNode *node = epub_document->contentList->data;
    gchar* stylesheetfilename = epub_document_get_alternate_stylesheet((gchar*)node->value) ;

    if (stylesheetfilename == NULL) {
        gchar *style = "body {color:rgb(255,255,255);\
                        background-color:rgb(0,0,0);\
                        text-align:justify;\
                        line-spacing:1.8;\
                        margin-top:0px;\
                        margin-bottom:4px;\
                        margin-right:50px;\
                        margin-left:50px;\
                        text-indent:3em;}\
                        h1, h2, h3, h4, h5, h6\
                        {color:white;\
                        text-align:center;\
                        font-style:italic;\
                        font-weight:bold;}";

        gchar *csspath = g_strdup_printf("%s/xreadernightstyle.css",epub_document->documentdir);


        GFile *styles = g_file_new_for_path (csspath);
        GOutputStream *outstream = (GOutputStream*)g_file_create(styles,G_FILE_CREATE_PRIVATE,NULL,NULL);
        if ( g_output_stream_write((GOutputStream*)outstream,style,strlen(style),NULL,NULL) == -1 )
        {
            return ;
        }
        g_output_stream_close((GOutputStream*)outstream,NULL,NULL);
        g_object_unref(styles) ;
        g_object_unref(outstream) ;
        //add this stylesheet to each document, for later.
        g_list_foreach(epub_document->contentList,(GFunc)add_night_sheet,csspath);
        g_free(csspath);
    }
    g_free(stylesheetfilename);
}

static void
epub_document_toggle_night_mode(EvDocument *document,gboolean night)
{
    EpubDocument *epub_document = EPUB_DOCUMENT(document);

    g_return_if_fail(EPUB_IS_DOCUMENT(epub_document));
    if (night)
        g_list_foreach(epub_document->contentList,(GFunc)change_to_night_sheet,NULL);
    else
        g_list_foreach(epub_document->contentList,(GFunc)change_to_day_sheet,NULL);
}

static gchar*
epub_document_set_document_title(gchar *containeruri)
{
	open_xml_document(containeruri);
	gchar *doctitle;
	set_xml_root_node(NULL);

	xmlNodePtr title = xml_get_pointer_to_node((xmlChar*)"title",NULL,NULL);

	doctitle = (gchar*)xml_get_data_from_node(title, XML_KEYWORD, NULL);
	xml_free_doc();

	return doctitle;
}

static void
page_set_function(linknode *Link, GList *contentList)
{
	GList *listiter = contentList;
	contentListNode *pagedata;

	guint flag=0;
	while (!flag && listiter) {
		pagedata = listiter->data;
		if (link_present_on_page(Link->pagelink, pagedata->value)) {
			flag=1;
			Link->page = pagedata->index - 1;
		}
		listiter = listiter->next;
	}

	if (Link->children) {
		g_list_foreach(Link->children,(GFunc)page_set_function,contentList);
	}
}

static void
epub_document_set_index_pages(GList *index,GList *contentList)
{
	g_return_if_fail (index != NULL);
	g_return_if_fail (contentList != NULL);

	g_list_foreach(index,(GFunc)page_set_function,contentList);
}


static void
add_mathjax_script_node_to_file(gchar *filename, gchar *data)
{
	xmlDocPtr mathdocument = xmlParseFile (filename);
	xmlNodePtr mathroot = xmlDocGetRootElement(mathdocument);

	if (mathroot == NULL)
		return;

	xmlNodePtr head = mathroot->children;

	while(head != NULL) {
		if (!xmlStrcmp(head->name,(xmlChar*)"head")) {
			break;
		}
		head = head->next;
	}

	if (xmlStrcmp(head->name,(xmlChar*)"head")) {
		return ;
	}

	xmlNodePtr script = xmlNewTextChild (head,NULL,(xmlChar*)"script",(xmlChar*)"");
	xmlNewProp(script,(xmlChar*)"type",(xmlChar*)"text/javascript");
	xmlNewProp(script,(xmlChar*)"src",(xmlChar*)data);

	xmlSaveFormatFile(filename, mathdocument, 0);
	xmlFreeDoc (mathdocument);
}

static void
epub_document_add_mathJax(gchar* containeruri,gchar* documentdir)
{
	gchar *containerfilename= g_filename_from_uri(containeruri,NULL,NULL);
	GString *mathjaxdir = g_string_new(MATHJAX_DIRECTORY);

	gchar *mathjaxref = g_filename_to_uri(mathjaxdir->str,NULL,NULL);
	gchar *nodedata = g_strdup_printf("%s/MathJax.js?config=TeX-AMS-MML_SVG",mathjaxref);

	open_xml_document(containerfilename);
	set_xml_root_node(NULL);
	xmlNodePtr manifest = xml_get_pointer_to_node((xmlChar*)"manifest",NULL,NULL);

	xmlNodePtr item = manifest->xmlChildrenNode;

	while (item != NULL) {
		if (xmlStrcmp(item->name,(xmlChar*)"item")) {
			item = item->next;
			continue;
		}

		xmlChar *mathml = xml_get_data_from_node(item,XML_ATTRIBUTE, (xmlChar*)"properties");

		if (mathml != NULL &&
		    !xmlStrcmp(mathml, (xmlChar*)"mathml") ) {
			gchar *href = (gchar*)xml_get_data_from_node(item, XML_ATTRIBUTE, (xmlChar*)"href");
			gchar *filename = g_strdup_printf("%s/%s",documentdir,href);

			add_mathjax_script_node_to_file(filename,nodedata);
			g_free(href);
			g_free(filename);
		}
		g_free(mathml);
		item = item->next;
	}
	xml_free_doc();
	g_free(mathjaxref);
	g_free(containerfilename);
	g_free(nodedata);
	g_string_free(mathjaxdir,TRUE);
}

static gboolean
epub_document_load (EvDocument* document,
                    const char* uri,
                    GError**    error)
{
	EpubDocument *epub_document = EPUB_DOCUMENT(document);
	GError *err = NULL;

	if ( check_mime_type (uri, &err) == FALSE )
	{
		/*Error would've been set by the function*/
		g_propagate_error(error,err);
		return FALSE;
	}

	extract_epub_from_container (uri,epub_document,&err);

	if ( err )
	{
		g_propagate_error( error,err );
		return FALSE;
	}

	/*FIXME : can this be different, ever?*/
	GString *containerpath = g_string_new(epub_document->tmp_archive_dir);
	g_string_append_printf(containerpath,"/META-INF/container.xml");
	gchar *containeruri = g_filename_to_uri(containerpath->str,NULL,&err);
	g_string_free (containerpath, TRUE);

	if ( err )
	{
		g_propagate_error(error,err);
		return FALSE;
	}

	gchar *contentOpfUri = get_uri_to_content (containeruri,&err,epub_document);
	g_free (containeruri);

	if ( contentOpfUri == NULL )
	{
		g_propagate_error(error,err);
		return FALSE;
	}

	epub_document->docTitle = epub_document_set_document_title(contentOpfUri);
	epub_document->index = setup_document_index(epub_document,contentOpfUri);

	epub_document->contentList = setup_document_content_list (contentOpfUri,&err,epub_document->documentdir);

    if (epub_document->index != NULL && epub_document->contentList != NULL)
	    epub_document_set_index_pages(epub_document->index, epub_document->contentList);

    epub_document_add_mathJax(contentOpfUri,epub_document->documentdir);
	g_free (contentOpfUri);

	if ( epub_document->contentList == NULL )
	{
		g_propagate_error(error,err);
		return FALSE;
	}

	return TRUE;
}

static void
epub_document_init (EpubDocument *epub_document)
{
    epub_document->archivename = NULL ;
    epub_document->tmp_archive_dir = NULL ;
    epub_document->contentList = NULL ;
	epub_document->documentdir = NULL;
	epub_document->index = NULL;
	epub_document->docTitle = NULL;
}


static void
epub_document_finalize (GObject *object)
{
	EpubDocument *epub_document = EPUB_DOCUMENT (object);

	if (epub_document->epubDocument != NULL) {
		if (epub_remove_temporary_dir (epub_document->tmp_archive_dir) == -1)
			g_warning (_("There was an error deleting “%s”."),
				   epub_document->tmp_archive_dir);
	}

	if ( epub_document->contentList ) {
            g_list_free_full(epub_document->contentList,(GDestroyNotify)free_tree_nodes);
			epub_document->contentList = NULL;
	}

	if (epub_document->index) {
		g_list_free_full(epub_document->index,(GDestroyNotify)free_link_nodes);
		epub_document->index = NULL;
	}

	if ( epub_document->tmp_archive_dir) {
		g_free (epub_document->tmp_archive_dir);
		epub_document->tmp_archive_dir = NULL;
	}

	if (epub_document->docTitle) {
		g_free(epub_document->docTitle);
		epub_document->docTitle = NULL;
	}
	if ( epub_document->archivename) {
		g_free (epub_document->archivename);
		epub_document->archivename = NULL;
	}
	if ( epub_document->documentdir) {
		g_free (epub_document->documentdir);
		epub_document->documentdir = NULL;
	}
	G_OBJECT_CLASS (epub_document_parent_class)->finalize (object);
}


static void
epub_document_class_init (EpubDocumentClass *klass)
{
	GObjectClass    *gobject_class = G_OBJECT_CLASS (klass);
	EvDocumentClass *ev_document_class = EV_DOCUMENT_CLASS (klass);

	gobject_class->finalize = epub_document_finalize;
	ev_document_class->load = epub_document_load;
	ev_document_class->save = epub_document_save;
	ev_document_class->get_n_pages = epub_document_get_n_pages;
	ev_document_class->get_info = epub_document_get_info;
	ev_document_class->get_page = epub_document_get_page;
	ev_document_class->toggle_night_mode = epub_document_toggle_night_mode;
    ev_document_class->check_add_night_sheet = epub_document_check_add_night_sheet;
}
