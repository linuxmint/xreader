#include "ev-file-helpers.h"
#include "epub-document.h"
#include "unzip.h"
#include <libxml/parser.h>
#include <libxml/xmlmemory.h>
#include <config.h>
#include <glib/gi18n.h>

/* A variable to hold the path where we extact our ePub */
static gchar* tmp_dir ;
/* A variable to hold our epubDocument , for unzip purposes */
static unzFile epubDocument ;

/*Global variables for XML parsing*/
static xmlDocPtr  xmldocument ;
static xmlNodePtr xmlroot ;
static xmlChar*   xmlkey ;
static xmlChar*   retval ;

/*
**Functions to parse the xml files.
**Open a XML document for reading 
*/
gboolean 
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
gboolean 
check_xml_root_node(xmlChar* rootname)
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

xmlChar*
parse_xml_children(xmlChar* parserfor,
				   XMLparsereturntype rettype,
                   xmlChar* attributename )
{
	xmlNodePtr topchild,children ;

    retval = NULL ;
    topchild = xmlroot->xmlChildrenNode ;

	while ( topchild != NULL )
	{
        if ( !xmlStrcmp(topchild->name,parserfor) )
        {
            if ( rettype == xmlattribute )
            {
                 retval = xmlGetProp(children,attributename);
                 return retval;     
            }
            else
            {
                retval = xmlNodeListGetString(xmldocument,topchild->xmlChildrenNode, 1);
                return retval ;
            }
        }
        parse_children( topchild , parserfor,rettype,attributename) ;

        topchild = topchild->next ;
	}

    return retval ;
}

static void 
parse_children(xmlNodePtr parent, 
               xmlChar* parserfor,
               XMLparsereturntype rettype,
               xmlChar* attributename )
{
    xmlNodePtr child = parent->xmlChildrenNode ;
    
    while ( child != NULL )
    {
        if ( !xmlStrcmp(child->name,parserfor))
        {
             if ( rettype == xmlattribute )
            {
                 retval = xmlGetProp(child,attributename);   
            }
            else
            {
                retval = xmlNodeListGetString(xmldocument,child->xmlChildrenNode, 1);
            }
            return ;
        }

        /*return already if we have retval set*/
        if ( retval != NULL )
        {
            return ;
        }

        parse_children(child,parserfor,rettype,attributename) ;
        child = child->next ;
    }
}

static void 
xml_free_all()
{
    xmlFreeDoc(xmldocument);
    xmlFree(retval);
    xmlFree(xmlkey);
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
        if (err)    {
            g_propagate_error (error, err);
        } 
        else    {
            g_set_error_literal (error,
                         EV_DOCUMENT_ERROR,
                         EV_DOCUMENT_ERROR_INVALID,
                         _("Not an ePub document"));
        }
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

gboolean
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
    g_string_append(gfilepath,"/");
    g_string_append(gfilepath,currentfilename);

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
    g_string_free(gfilepath);
    g_free(currentfilename);
    g_free(buffer);
        
}
