#include <gtk/gtk.h>
#include <glib.h>
#include <libxml/parser.h>
#include <libxml/xmlmemory.h>

typedef enum  
{
	xmlattribute,
	xmlkeyword
}XMLparsereturntype;

gboolean  openXmlDocument ( const gchar* filename );

gboolean  checkRoot       (xmlChar* rootname);

void      parseChildren   (xmlNodePtr parent, 
                	       xmlChar* parserfor,
           		           XMLparsereturntype rettype,
                   		   xmlChar* attributename );

xmlChar*  parseXMLchildren (xmlChar* parserfor,
				            XMLparsereturntype rettype,
                            xmlChar* attributename );

void      xmlFreeAll();