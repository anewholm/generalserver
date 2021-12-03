//platform specific file (UNIX)
#include "LibXmlNode.h"

#include "LibXslNode.h"
#include "LibXmlDoc.h"
#include "Repository.h"

//ls /usr/include/libxml2/libxml/*.h, http://xmlsoft.org/, file:///usr/local/share/doc/libxml2-2.6.22/html/tutorial/apc.html
#include <libxslt/xslt.h>
#include <libxslt/xsltInternals.h>
#include <libxml/xpathInternals.h>
#include <libxml/xmlsave.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>

//http://www.opensource.apple.com/source/WebCore/WebCore-855.7/xml/XSLTExtensions.cpp
#include <libxslt/extensions.h>
#include <libxslt/extra.h>
#include <libxslt/templates.h>
//#include "XSLTExtensions.h"

#include "Utilities/clone.c"

using namespace std;

namespace general_server {
  LibXmlNode::LibXmlNode(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, xmlNodePtr oNode, xmlListPtr oParentRoute, IXmlBaseDoc *pDoc, const bool bRegister, const char *sLockingReason, const bool bDocOurs): 
    MemoryLifetimeOwner(pMemoryLifetimeOwner),
    LibXmlBaseNode(oNode, oParentRoute, pDoc, bRegister, sLockingReason, bDocOurs) 
  {}
  
  LibXmlNode::LibXmlNode(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, xmlNodePtr oNode, xmlListPtr oParentRoute, const IXmlBaseDoc *pDoc, const bool bRegister, const char *sLockingReason, const bool bDocOurs): 
    MemoryLifetimeOwner(pMemoryLifetimeOwner),
    LibXmlBaseNode(oNode, oParentRoute, pDoc, bRegister, sLockingReason, bDocOurs) 
  {}
  
  LibXmlNode::LibXmlNode(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const xmlNode *oNode, xmlListPtr oParentRoute, const IXmlBaseDoc *pDoc, const bool bRegister, const char *sLockingReason, const bool bDocOurs): 
    MemoryLifetimeOwner(pMemoryLifetimeOwner),
    LibXmlBaseNode((xmlNodePtr) oNode, oParentRoute, pDoc, bRegister, sLockingReason, bDocOurs) 
  {}
  
  IXmlBaseNode *LibXmlNode::clone_wrapper_only() const {return base_object_clone<LibXmlNode>(this);}
  IXmlBaseNode *LibXmlNode::clone_with_resources() const {return new LibXmlNode(*this);}
}
