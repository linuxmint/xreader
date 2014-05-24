#include "epub-document.h"

static xmlDocPtr  xmldocument ;
static xmlNodePtr xmlroot ;
static xmlChar*   xmlkey ;
static xmlChar*   retval ;

/*Open a XML document for reading */
gboolean 
openXmlDocument ( const gchar* filename )
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
checkRoot(xmlChar* rootname)
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
parseXMLchildren( xmlChar* parserfor,
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
        parseChildren( topchild , parserfor,rettype,attributename) ;

        topchild = topchild->next ;
	}

    return retval ;
}

void parseChildren(xmlNodePtr parent, 
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

        parseChildren(child,parserfor,rettype,attributename) ;
        child = child->next ;
    }
}

void xmlFreeAll()
{
    xmlFreeDoc(xmldocument);
    xmlFree(retval);
    xmlFree(xmlkey);
}