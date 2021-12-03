//platform specific file (UNIX)
#ifndef _LIBXMLBASEDOC_H
#define _LIBXMLBASEDOC_H

//std library includes
#include <iostream>
#include <vector>
#include <map>
#include <pthread.h>
#include <stdarg.h>
using namespace std;

//ls /usr/include/libxml2/libxml/*.h, http://xmlsoft.org/, file:///usr/local/share/doc/libxml2-2.6.22/html/tutorial/apc.html
#include <libxml/xpathInternals.h>
#include <libxml/xmlsave.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>

//http://www.opensource.apple.com/source/WebCore/WebCore-855.7/xml/XSLTExtensions.cpp
#include <libxslt/xslt.h>
#include <libxslt/xsltInternals.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>
#include <libxslt/extensions.h>
#include <libxslt/extra.h>
#include <libxslt/templates.h>
//#include "XSLTExtensions.h"

#include <libexslt/exslt.h>

#include "Xml/XmlBaseDoc.h" //full definition for inheritance

namespace general_server {
  class LibXmlBaseDoc: public XmlBaseDoc {
  private:
    static void assemblePrefixes(void * payload, void * data, const xmlChar * name, const xmlChar * name2, const xmlChar * name3);

  protected:
    pthread_mutex_t m_mStylesheetCache;
    xmlDocPtr m_oDoc; //LibXML pointer
    bool m_bOurs;     //all constructors should initiate this one. although its not const
    bool m_bLoadedAndParsed;
    bool bResolveExternals;
    bool bSpecified;
    bool bValidateOnParse;

    void recodeLineNumbers(xmlNodePtr oNode = 0); //depreciated
    bool createDOMDoc(const bool bWithDict = true);
    LibXmlBaseDoc(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlLibrary *pLib, const char *sAlias, xmlDocPtr oDoc, const bool bOurs = true);
    LibXmlBaseDoc(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlLibrary *pLib, const char *sAlias);
    LibXmlBaseDoc(const LibXmlBaseDoc &doc);

  public:
    ~LibXmlBaseDoc();

    void loadXml(const char *sXML);
    void streamLoadXML(Repository *pRepository);
    bool validate();             //standard XML validity
    int  validityCheck(const char *sContext = 0) const; //recurse LibXML structures looking for consistency errors
    bool ours() const; //indicates if this document wrapper should free the underlying structure on destruction
    bool is(const IXmlBaseDoc *pDoc2) const;

    //information
    bool loadedAndParsed() const {return m_bLoadedAndParsed;}
    bool resolveExternals() const {return xmlLoadExtDtdDefaultValue == 1;}
    bool specified() const {return false;} //not implemented
    bool validateOnParse() const {return false;} //not implemented
    void serialise(const IXmlQueryEnvironment *pQE, IXmlBaseNode *pOutputNode) const;
    const char *encoding() const;
    const char *extensionElementPrefixes() const;

    //in-built indexing and other Xml linking standards
    XmlNodeList<IXmlBaseNode> *nodesWithXmlIDs(const IXmlQueryEnvironment *pQE) const;
    XmlNodeList<IXmlBaseNode> *nodesAreIDs(    const IXmlQueryEnvironment *pQE) const;
    XmlNodeList<IXmlBaseNode> *nodesAreIDRefs( const IXmlQueryEnvironment *pQE) const;
    const IXmlBaseNode        *nodeFromID(     const IXmlQueryEnvironment *pQE, const char *sID, const char *sLockingReason = 0) const;
    XmlNodeList<IXmlBaseNode> *nodeFromRef(    const IXmlQueryEnvironment *pQE, const char *sRef);

    const IXmlBaseNode *documentNode(const char *sLockingReason = "document node") const; //the top slash, i.e. xpath:/
    IXmlBaseNode *documentNode(const char *sLockingReason = "document node"); //the top slash, i.e. xpath:/
    IXmlBaseNode *dtdNode() const;      //override the standard one

    //update: all these fucntions are implemented in IXmlBaseDoc and defer processing to IXmlNode
    //appendChild_*() -> LibXmlNode::appendChild_*()

    //platform specific
    //concurrency
    void stylesheetCacheLock(  const char *sReason = 0);
    void stylesheetCacheUnlock(const char *sReason = 0);
    //LibXml object access
    const xmlDocPtr libXmlDocPtr() const     {return (const xmlDocPtr) m_oDoc;}
    void relinquishOwnershipOfLibXmlDocPtr() {m_bOurs = false;}
  };
}

#endif
