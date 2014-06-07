#include "ev-file-helpers.h"
#include "epub-document.h"
#include "unzip.h"

#include <libxml/parser.h>
#include <libxml/xmlmemory.h>

#include <config.h>

#include <glib/gi18n.h>
#include <glib/gstdio.h>

/* A variable to hold the path where we extact our ePub */
static gchar* tmp_dir = NULL;
/* A variable to hold our epubDocument , for unzip purposes */
static unzFile epubDocument ;

typedef enum _xmlParseReturnType 
{
    xmlattribute = 0,
    xmlkeyword   = 1

}xmlParseReturnType;

struct _DocumentTreeNode {  
    gchar* key ;
    gchar* value ;
};

typedef  struct _DocumentTreeNode DocumentTreeNode;

/*Prototypes for some future functions*/
static gboolean
extract_one_file            (GError ** error);

static gboolean
check_mime_type             (const gchar* uri,
                             GError** error);

static gboolean 
extract_epub_from_container (const gchar* uri, 
                             GError ** error);

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

static GList*
setup_document_tree         (const gchar* content_uri, 
                             GError** error);

/*Global variables for XML parsing*/
static xmlDocPtr    xmldocument ;
static xmlNodePtr   xmlroot ;
static xmlNodePtr   retval ;

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
    xmlNodePtr topchild,children ;

    retval = NULL ;

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
                 retval = topchild;
                 return retval;     
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

    return retval ;
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
                 retval = child;
                 return ;
            }
            else 
            {
                /*No need to parse children node*/
                child = child->next ;
                continue ;
            }
        }

        /*return already if we have retval set*/
        if ( retval != NULL )
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
xml_get_data_from_node(xmlNodePtr node,xmlParseReturnType rettype,xmlChar* attributename)
{
    xmlChar* datastring ;
    if ( rettype == xmlattribute )
       datastring= xmlGetProp(node,attributename);
    else
       datastring= xmlNodeListGetString(xmldocument,node->xmlChildrenNode, 1);

    return datastring;
}

static gboolean
check_mime_type(const gchar* uri,GError** error)
{
    GError * err = NULL ;
    gchar* mimeFromFile = ev_file_get_mime_type(uri,FALSE,&err);
    
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
    else if ( g_strcmp0(mimeFromFile, "application/epub+zip") == 0  )
    {
        return TRUE ;
    }
    else
    {
        g_set_error_literal (error,
                     EV_DOCUMENT_ERROR,
                     EV_DOCUMENT_ERROR_INVALID,
                     _("Not an ePub document"));

        return FALSE;
    }
}

static gboolean 
extract_epub_from_container (const gchar* uri, GError ** error)
{
    GError* err = NULL ;
    GString * temporary_sub_directory ; 
    gchar* archivename = g_filename_from_uri(uri,NULL,error);
    
    if ( !archivename )
    {
         if (err)    {
            g_propagate_error (error, err);
        } 
        else    {
            g_set_error_literal (error,
                         EV_DOCUMENT_ERROR,
                         EV_DOCUMENT_ERROR_INVALID,
                         _("could not retrieve filename"));
        }
        return FALSE ;
    }

    tmp_dir = g_strrstr(archivename,"/");
    if ( *tmp_dir == '/' )
    tmp_dir++ ;

    temporary_sub_directory = g_string_new( tmp_dir );
    g_string_append(temporary_sub_directory,"XXXXXX") ;

    tmp_dir = ev_mkdtemp(temporary_sub_directory->str,error) ;

    if (!tmp_dir) {
        return FALSE ;
    }

    g_string_free(temporary_sub_directory,TRUE);

    epubDocument = unzOpen64(archivename);

    if ( epubDocument == NULL )
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
        return FALSE ;
    }
    if ( unzGoToFirstFile(epubDocument) != UNZ_OK )
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
        return FALSE ;
    }
    while ( TRUE )
    {
        if ( extract_one_file(&err) == FALSE )
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
        }   

        if ( unzGoToNextFile(epubDocument) == UNZ_END_OF_LIST_OF_FILE )
            break ;
    }

    unzClose(epubDocument);

    if ( err != NULL )
        g_error_free(err);

    g_free(archivename);

    return TRUE ;
}

static gboolean
extract_one_file(GError ** error)
{
    GFile * outfile ;
    gsize writesize = 0;
    GString * gfilepath ;
    unz_file_info64 info ;  
    gchar* directory;
    GFileOutputStream * outstream ;
    gpointer currentfilename = g_malloc0(512);
    gpointer buffer = g_malloc0(512);

    if ( unzOpenCurrentFile(epubDocument) != UNZ_OK )
    {
            return FALSE ;
    } 
        
    unzGetCurrentFileInfo64(epubDocument,&info,currentfilename,512,NULL,0,NULL,0) ;
    directory = g_strrstr(currentfilename,"/") ;

    if ( directory != NULL )
        directory++; 

    gfilepath = g_string_new(tmp_dir) ;
    g_string_append_printf(gfilepath,"/%s",(gchar*)currentfilename);

    /*if we encounter a directory, make a directory inside our temporary folder.*/
    if (directory != NULL && *directory == '\0')
    {
        g_mkdir(gfilepath->str,0777);
    }
    else
    {
        outfile = g_file_new_for_path(gfilepath->str);
        outstream = g_file_create(outfile,G_FILE_CREATE_PRIVATE,NULL,error);
        while ( (writesize = unzReadCurrentFile(epubDocument,buffer,512) ) != 0 )
        {
            if ( g_output_stream_write((GOutputStream*)outstream,buffer,writesize,NULL,error) == -1 )
            {
                return FALSE ;
            }
        }
        g_output_stream_close((GOutputStream*)outstream,NULL,error);
        g_object_unref(outfile) ;
        g_object_unref(outstream) ;
    }

    unzCloseCurrentFile (epubDocument) ;
    g_string_free(gfilepath,TRUE);
    g_free(currentfilename);
    g_free(buffer);
        
}

static gchar* get_uri_to_content(const gchar* uri,GError ** error)
{
    GError *   err = NULL ; 
    gchar*     containerpath = g_filename_from_uri(uri,NULL,&err);
    GString*   absolutepath = g_string_new(NULL);
    gchar*     content_uri ;
    xmlNodePtr rootfileNode ;
    xmlChar*   relativepath;
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

    if ( open_xml_document(containerpath) == FALSE )
    {
        g_set_error_literal(error,
                            EV_DOCUMENT_ERROR,
                            EV_DOCUMENT_ERROR_INVALID,
                            _("could not open container file"));
    
        return NULL ;
    }

    if ( set_xml_root_node("container") == FALSE)  {

        g_set_error_literal(error,
                            EV_DOCUMENT_ERROR,
                            EV_DOCUMENT_ERROR_INVALID,
                            _("container file is corrupt"));    
        return NULL ;
    }

    if ( (rootfileNode = xml_get_pointer_to_node("rootfile","media-type","application/oebps-package+xml")) == NULL)
    {
        g_set_error_literal(error,
                            EV_DOCUMENT_ERROR,
                            EV_DOCUMENT_ERROR_INVALID,
                            _("epub file is invalid or corrput"));
        return NULL ;
    }
    
    relativepath = xml_get_data_from_node(rootfileNode,xmlattribute,(xmlChar*)"full-path") ;
    
    if ( relativepath == NULL )
    {
        g_set_error_literal(error,
                            EV_DOCUMENT_ERROR,
                            EV_DOCUMENT_ERROR_INVALID,
                            _("epub file is corrupt,no container"));
        return NULL ;
    }
    g_string_printf(absolutepath,"%s/%s",tmp_dir,relativepath);

    content_uri = g_filename_to_uri(absolutepath->str,NULL,&err);
    if ( !content_uri )  {
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
    free(absolutepath);

    return content_uri ; 
}

static GList*
setup_document_tree(const gchar* content_uri, GError** error)
{
    GList* newlist = NULL ;
    GError *   err = NULL ; 
    gchar*     contentOpf="/home/rootavish/Downloads/zlib/progit/content.opf";
    xmlNodePtr manifest,spine,itemrefptr,itemptr ;
    gboolean errorflag = FALSE;

    gchar* relativepath ;
    GString* absolutepath = g_string_new(NULL);

    if ( open_xml_document(contentOpf) == FALSE )
    {
        g_set_error_literal(error,
                            EV_DOCUMENT_ERROR,
                            EV_DOCUMENT_ERROR_INVALID,
                            _("could not parse content manifest"));
    
        return FALSE ;
    }
    if ( set_xml_root_node("package") == FALSE)  {

        g_set_error_literal(error,
                            EV_DOCUMENT_ERROR,
                            EV_DOCUMENT_ERROR_INVALID,
                            _("content file is invalid"));    
        return FALSE ;
    }

    if ( ( spine = xml_get_pointer_to_node("spine",NULL,NULL) )== NULL )  
    {
         g_set_error_literal(error,
                            EV_DOCUMENT_ERROR,
                            EV_DOCUMENT_ERROR_INVALID,
                            _("epub file has no spine"));    
        return FALSE ;
    }
    
    if ( ( manifest = xml_get_pointer_to_node("manifest",NULL,NULL) )== NULL )  
    {
         g_set_error_literal(error,
                            EV_DOCUMENT_ERROR,
                            EV_DOCUMENT_ERROR_INVALID,
                            _("epub file has no manifest"));    
        return FALSE ;
    }

    retval = NULL ;

    /*Get first instance of itemref from the spine*/
    xml_parse_children_of_node(spine,"itemref",NULL,NULL);
    
    if ( retval != NULL )
        itemrefptr = retval ;
    else
    {
        errorflag=TRUE;
    }
    /*Parse the spine for remaining itemrefs*/
    do
    {
        /*for the first time that we enter the loop, if errorflag is set we break*/
        if ( errorflag )
        {
            break;
        }
        if ( xmlStrcmp(itemrefptr->name,"itemref") == 0)
        {    
            DocumentTreeNode* newnode = g_malloc0(sizeof(newnode));    
            newnode->key = xml_get_data_from_node(itemrefptr,xmlattribute,(xmlChar*)"idref");
            
            if ( newnode->key == NULL )
            {
                errorflag =TRUE;    
                break;
            }
            retval=NULL ;
            xml_parse_children_of_node(manifest,"item","id",newnode->key);
            
            if ( retval != NULL )
            {
                itemptr = retval ;
            }
            else
            {
                errorflag=TRUE;
                break;
            }
            relativepath = xml_get_data_from_node(itemptr,xmlattribute,(xmlChar*)"href");
            
            g_string_assign(absolutepath,tmp_dir);
            g_string_append_printf(absolutepath,"/%s",relativepath);
            newnode->value = g_filename_to_uri(absolutepath->str,NULL,&err);
            if ( newnode->value == NULL )
            {
                errorflag =TRUE;    
                break;
            }
            newlist = g_list_prepend(newlist,newnode);
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
        g_string_free(absolutepath,TRUE);
        g_list_free_full(newlist,(GDestroyNotify)free_tree_nodes);
        return NULL ;
    }

    g_string_free(absolutepath,TRUE);
    return newlist ;

}

static void
free_tree_nodes(gpointer data)
{
    DocumentTreeNode* dataptr = data ;
    g_free(dataptr->value);
    g_free(dataptr->key);
    g_free(dataptr);
}