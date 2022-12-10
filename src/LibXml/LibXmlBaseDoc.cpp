//platform independent
#include "LibXmlBaseDoc.h"

#include "LibXmlLibrary.h"
#include "LibXmlBaseNode.h"
#include "LibXmlNode.h"
#include "LibXslNode.h"
#include "Repository.h"

#include "Utilities/strtools.h"

#include <libxml/debugXML.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
using namespace std;

namespace general_server {
#ifdef HAVE_LIBPTHREAD
  void LibXmlBaseDoc::stylesheetCacheLock(  const char *sReason) {pthread_mutex_lock(&m_mStylesheetCache);}
  void LibXmlBaseDoc::stylesheetCacheUnlock(const char *sReason) {pthread_mutex_unlock(&m_mStylesheetCache);}
#else
  void LibXmlBaseDoc::stylesheetCacheLock()   {}
  void LibXmlBaseDoc::stylesheetCacheUnlock() {}
#endif


  LibXmlBaseDoc::LibXmlBaseDoc(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlLibrary *pLib, const char *sAlias, xmlDocPtr oDoc, const bool bOurs): 
    MemoryLifetimeOwner(pMemoryLifetimeOwner),
    XmlBaseDoc(pMemoryLifetimeOwner, pLib, sAlias),
    m_bOurs(bOurs), 
    m_oDoc(oDoc),
    //protected: access it through the generalWriteLock()
    //  http://pubs.opengroup.org/onlinepubs/7908799/xsh/pthread_mutex_unlock.html
    m_mStylesheetCache(PTHREAD_MUTEX_INITIALIZER)
  {
    assert(m_oDoc);

    if (!m_oDoc->name) xmlNodeSetName((xmlNodePtr) m_oDoc, (const xmlChar*) m_sAlias); //DICT
    m_bLoadedAndParsed = true;
  }

  LibXmlBaseDoc::LibXmlBaseDoc(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlLibrary *pLib, const char *sAlias): 
    MemoryLifetimeOwner(pMemoryLifetimeOwner), 
    m_oDoc(0), 
    m_bOurs(true), 
    XmlBaseDoc(pMemoryLifetimeOwner, pLib, sAlias),
    //protected: access it through the generalWriteLock()
    //  http://pubs.opengroup.org/onlinepubs/7908799/xsh/pthread_mutex_unlock.html
    m_mStylesheetCache(PTHREAD_MUTEX_INITIALIZER)
  {
    createDOMDoc();
    m_bLoadedAndParsed = true;
  }

  LibXmlBaseDoc::LibXmlBaseDoc(const LibXmlBaseDoc &doc):
    m_oDoc(xmlCopyDoc(doc.m_oDoc, 1)),
    m_bOurs(doc.m_bOurs),
    m_bLoadedAndParsed(doc.m_bLoadedAndParsed),
    bResolveExternals(doc.bResolveExternals),
    bSpecified(doc.bSpecified),
    bValidateOnParse(doc.bValidateOnParse),
    MemoryLifetimeOwner(doc.mmParent()),  //addChild()
    XmlBaseDoc(doc), //MM_STRDUP m_sAlias
    //protected: access it through the generalWriteLock()
    //  http://pubs.opengroup.org/onlinepubs/7908799/xsh/pthread_mutex_unlock.html
    m_mStylesheetCache(PTHREAD_MUTEX_INITIALIZER)
  {
    NOT_CURRENTLY_USED("doc copy");
  }

  LibXmlBaseDoc::~LibXmlBaseDoc() {
    //Free up all the structures used by a document, tree included.
    if (m_oDoc && m_bOurs) xmlFreeDoc(m_oDoc);
  }

  bool LibXmlBaseDoc::createDOMDoc(const bool bWithDict) {
    //PRIVATE
    assert(m_sAlias);
    
    if (m_oDoc) xmlFreeDoc(m_oDoc);
    m_oDoc = xmlNewDoc((xmlChar*) "1.0");
    xmlNodeSetName((xmlNodePtr) m_oDoc, (const xmlChar*) m_sAlias);
    if (bWithDict) m_oDoc->dict = xmlDictCreate();
    m_bLoadedAndParsed = true;
    
    return m_bLoadedAndParsed;
  }

  bool LibXmlBaseDoc::is(const IXmlBaseDoc *pDoc2) const {
    const LibXmlBaseDoc *pDocType = dynamic_cast<const LibXmlBaseDoc*>(pDoc2);
    return pDocType && m_oDoc == pDocType->m_oDoc;
  }

  bool LibXmlBaseDoc::ours() const {
    //indicates if this document wrapper should free the underlying structure on destruction
    return m_bOurs;
  }

  const char *LibXmlBaseDoc::encoding() const {
    return (const char*) xmlGetCharEncodingName((xmlCharEncoding) m_oDoc->charset);
  }

  void LibXmlBaseDoc::streamLoadXML(Repository *pRepository) {
    assert(pRepository);

    //http://xmlsoft.org/html/libxml-xmlIO.html#xmlInputReadCallback
    if (m_oDoc) xmlFreeDoc(m_oDoc);

    //create via function to allow virtual overloading of streaming context creation
    Repository::StreamingContext *pStreamingContext = pRepository->newStreamingContext(pRepository);
      m_oDoc = xmlReadIO(Repository::static_inputReadCallback, Repository::static_inputCloseCallback, pStreamingContext, NULL, "UTF-8",
        XML_PARSE_NOENT | XML_PARSE_NONET | XML_PARSE_COMPACT
        //XML_PARSE_RECOVER | XML_PARSE_NSCLEAN
        //XML_PARSE_NOERROR | XML_PARSE_NOWARNING
      );
    xmlNodeSetName((xmlNodePtr) m_oDoc, (const xmlChar*) m_sAlias);
    assert(m_oDoc->name);
    assert(m_oDoc->dict);
    //translateStructure(); already done in xmlReadIO() => xmlDoRead();
    m_bLoadedAndParsed = true;
    if (pStreamingContext) delete pStreamingContext;

    cacheAllPrefixedNamespaceDefinitionsForXPath();
    recodeLineNumbers();  //line -> xml:id
  }

  void LibXmlBaseDoc::recodeLineNumbers(xmlNodePtr oNode ATTRIBUTE_UNUSED) {
#ifdef RECODELINENUMBERS //@xml:id is discouraged
    const char *sXmlId = 0, *sXmlIdNumber;

    //start at the top (if present)
    if (!oNode) oNode = m_oDoc->children;

    if (oNode 
      && !xmlIsHardLink(oNode)
      && !xmlIsSoftLink(oNode)
    ) {
      //recode the line number
      if (oNode->type == XML_ELEMENT_NODE) {
        sXmlId = (const char*) xmlGetNsProp(oNode, (const xmlChar*) "id", (const xmlChar*) NAMESPACE_XML);
        if (sXmlId) {
          //number must be at the end of the string for this recode to happen
          sXmlIdNumber = sXmlId;
          while (*sXmlIdNumber && !isdigit(*sXmlIdNumber)) sXmlIdNumber++;
          if (*sXmlIdNumber) oNode->line = atoi(sXmlIdNumber);
        }
        //recursive
        if (oNode->children) recodeLineNumbers(oNode->children);
      }

      //recursive
      if (oNode->next) recodeLineNumbers(oNode->next);
    }

    //free up
    if (sXmlId) xmlFree((void*) sXmlId);
#endif
  }

  void LibXmlBaseDoc::loadXml(const char *sXML) {
    assert(sXML);

    size_t iLen, iFirstTagLen, iNamespacesLen, iNewLen;
    const char *sFirst_tagFinish;
    char *sXML2 = 0;

    if (m_oDoc) xmlFreeDoc(m_oDoc);

    if (sXML && *sXML) {
      try {
        iLen             = strlen(sXML);
        m_bLoadedAndParsed = (m_oDoc = xmlParseMemory(sXML, iLen));
      } catch (XmlParserNamespaceNotDeclared& ex1) {
        //doc parsing failed due to missing namespace decleration
        //lets be nice and try again
        //we are going to check the root node only
        //if it has ANY namespaces declared then we assume it has done it correctly and independently
        //DANGER: trying to string handle insert all relevant namespaces here
        sFirst_tagFinish = strchr(sXML, '>');
        if (sFirst_tagFinish) {
          //if the document consists of only one tag, e.g. <object:Response/> then we need to insert before the /, not the >
          if (sFirst_tagFinish > sXML && *(sFirst_tagFinish-1) == '/') sFirst_tagFinish--;
          iFirstTagLen     = sFirst_tagFinish - sXML;
          iNamespacesLen   = strlen(NAMESPACE_ALL);
          iNewLen          = iLen + iNamespacesLen + 1;
          sXML2            = (char*) MM_MALLOC(iNewLen);

          //construct new input XML with extra namespaces in
          //this might re-iterate namespaces which will cause the server to throw up
          _SNPRINTF1(sXML2, iFirstTagLen + 1, "%s", sXML);
          _SNPRINTF2(sXML2 + iFirstTagLen, iNewLen - iFirstTagLen + 1, " %s%s", NAMESPACE_ALL, sXML + iFirstTagLen);

          //attempt re-parse
          try {
            m_bLoadedAndParsed = (m_oDoc = xmlParseMemory(sXML2, iNewLen));
          } catch (XmlParserNamespaceNotDeclared& ex2) {
            if (sXML2) MMO_FREE(sXML2);
            throw ex2; //let the exceptions flow this time
          }
        } else throw ex1;
      }
    } else {
      //blank doc
      createDOMDoc();
    }

    cacheAllPrefixedNamespaceDefinitionsForXPath();
  }

  void LibXmlBaseDoc::serialise(const IXmlQueryEnvironment *pQE, IXmlBaseNode *pOutputNode) const {
    assert(pOutputNode);

    IXmlBaseNode *pCurrent, *pStylesheets;
    StringMap<const char*>::const_iterator iNamespace;

    //--------------------------- stylesheets
    LibXslNode::serialise(pQE, pOutputNode);

    //--------------------------- namespace cache
    pStylesheets = pOutputNode->createChildElement(pQE, "namespace-cache");
    for (iNamespace = m_mNamespaceDefinitions.begin(); iNamespace != m_mNamespaceDefinitions.end(); iNamespace++) {
      pCurrent = pStylesheets->createChildElement(pQE, "namespace");
      pCurrent->setAttribute(pQE, "list-name", iNamespace->first, NAMESPACE_GS);
      pCurrent->setAttribute(pQE, "prefix", iNamespace->first);
      pCurrent->value(pQE, iNamespace->second);
      delete pCurrent;
    }

    //free up
    delete pStylesheets;
  }

  bool LibXmlBaseDoc::validate() {
    xmlValidCtxtPtr vctxt = xmlNewValidCtxt();
    
    //const bool bValidDoc  = xmlValidateDocument(vctxt, m_oDoc);
    const bool bHasDTD   = (m_oDoc->intSubset != NULL || m_oDoc->extSubset != NULL);
    const bool bValidDTD = !bHasDTD || xmlValidateDtdFinal(vctxt, m_oDoc);
    const bool bValidDoc = xmlValidateDocumentFinal(vctxt, m_oDoc);
    
    //free up
    xmlFreeValidCtxt(vctxt);

    return bValidDoc && bValidDTD;
  }
  
  int LibXmlBaseDoc::validityCheck(const char *sContext) const {
    IFDEBUG(
      static int iNumberOfValidityRequests = 0;
      iNumberOfValidityRequests++;
      if (iNumberOfValidityRequests > 20) {
        cout << "WARNING: more than 50 validityCheck()s can cause a slow down\n";
        iNumberOfValidityRequests = 0;
      }
    );
    IFDEBUG(assert(sContext);)

    int iErrors = 0;

    
    //[usually] does not throw LibXML2 errors, just pumps out to a FILE*
    //xmlLibrary()->clearErrorFunctions(); //prevent the overlayed throw architecture
    UNWIND_EXCEPTION_BEGIN {
#ifdef LIBXML_DEBUG_ENABLED
      IFDEBUG(
        try {
          xmlDebugCheckDocument(stderr, m_oDoc);
        } catch (exception &ex) {
          throw ValidityCheckFailure(StandardException(this, ex), MM_STRDUP(sContext)); //with note about stderr output
        }
      );
#endif
      
      //in case LibXML2 is not using setGenericErrorFunc() today
      //and thus not throwing XMLGeneric exceptions
      if (iErrors) {
        throw ValidityCheckFailure(this, MM_STRDUP(sContext));
      }
    } UNWIND_EXCEPTION_END;
    //xmlLibrary()->setErrorFunctions();


    UNWIND_EXCEPTION_THROW;

    return iErrors;
  }

  void LibXmlBaseDoc::assemblePrefixes(void * payload, void * data, const xmlChar * name, const xmlChar * name2, const xmlChar * name3) {
    const LibXmlBaseDoc *pDoc = (const LibXmlBaseDoc*) data;
    string *psExtensionElementPrefixes = (string*) data + 1;
    const char *sPrefix = pDoc->standardPrefix((const char*) name);
    if (sPrefix) {
      if (psExtensionElementPrefixes->size()) psExtensionElementPrefixes->append(" ");
      psExtensionElementPrefixes->append(sPrefix);
    }
  }

  const char *LibXmlBaseDoc::extensionElementPrefixes() const {
    NOT_COMPLETE("");
    string sExtensionElementPrefixes;
    const xmlHashTablePtr oHash = xsltExtensionHREFs();
    const void *data[2];

    //already unique entries need to be translated in to the prefixes for this document
    data[0] = this;
    data[1] = &sExtensionElementPrefixes;
    xmlHashScanFull(oHash, assemblePrefixes, &data);

    //free up
    if (oHash) xmlHashFree(oHash, NULL);

    return MM_STRDUP(sExtensionElementPrefixes.c_str());
  }

  const IXmlBaseNode *LibXmlBaseDoc::documentNode(const char *sLockingReason) const {
    //caller must delete the LibXmlNode
    //this is only used to append a root node generally
    const LibXmlBaseNode *pNewNode = LibXmlLibrary::factory_node(this, (xmlNodePtr) m_oDoc, this, NULL, sLockingReason);
    return pNewNode->queryInterface((const IXmlBaseNode*) 0);
  }

  IXmlBaseNode *LibXmlBaseDoc::documentNode(const char *sLockingReason) {
    //caller must delete the LibXmlNode
    //this is only used to append a root node generally
    LibXmlBaseNode *pNewNode = LibXmlLibrary::factory_node(this, (xmlNodePtr) m_oDoc, this, NULL, sLockingReason);
    return pNewNode->queryInterface((IXmlBaseNode*) 0);
  }

  IXmlBaseNode *LibXmlBaseDoc::dtdNode() const {
    //caller manages pNode
    IXmlBaseNode *pDTDNode = 0;
    xmlDtdPtr oDtd         = xmlGetIntSubset(m_oDoc);
    if (oDtd) pDTDNode = LibXmlLibrary::factory_node(this, (xmlNodePtr) oDtd, (LibXmlBaseDoc*) this);
    return pDTDNode;
  }

  XmlNodeList<IXmlBaseNode> *LibXmlBaseDoc::nodesWithXmlIDs(const IXmlQueryEnvironment *pQE) const {
    NOT_COMPLETE("nodesWithXmlIDs");
    return 0;
  }
  XmlNodeList<IXmlBaseNode> *LibXmlBaseDoc::nodesAreIDs(const IXmlQueryEnvironment *pQE) const {
    NOT_COMPLETE("nodesAreIDs");
    return 0;
  }
  XmlNodeList<IXmlBaseNode> *LibXmlBaseDoc::nodesAreIDRefs(const IXmlQueryEnvironment *pQE) const {
    //TODO: complete nodesAreIDRefs()
    return 0;
  }
  
  const IXmlBaseNode *LibXmlBaseDoc::nodeFromID(const IXmlQueryEnvironment *pQE, const char *sID, const char *sLockingReason) const {
    xmlAttrPtr     oAttrXmlID = 0;
    const LibXmlBaseNode *pElement  = 0;

    if (oAttrXmlID = xmlGetID(m_oDoc, (const xmlChar*) sID)) {
      if (oAttrXmlID->parent) pElement = LibXmlLibrary::factory_node(this, oAttrXmlID->parent, this, NULL, sLockingReason);
      //xmlFreeProp(oAttrXmlID); //not ours!
    }

    return pElement ? pElement->queryInterface((IXmlBaseNode*) 0) : NULL;
  }
  
  XmlNodeList<IXmlBaseNode> *LibXmlBaseDoc::nodeFromRef(const IXmlQueryEnvironment *pQE, const char *sRef) {
    xmlListPtr oAttrXmlRefs       = 0;
    xmlRefPtr oXmlRef             = 0;
    xmlAttrPtr oAttrPtr           = 0;
    LibXmlBaseNode *pElement      = 0;
    XmlNodeList<IXmlBaseNode> *pNodes = new XmlNodeList<IXmlBaseNode>(this);
    
    if (oAttrXmlRefs = xmlGetRefs(m_oDoc, (const xmlChar*) sRef)) {
      while ((oXmlRef  = (xmlRefPtr) xmlListPopBack(oAttrXmlRefs))
        &&   (oAttrPtr = oXmlRef->attr)
        &&   (oAttrPtr->parent)
      ) {
        pElement = LibXmlLibrary::factory_node(mmParent(), oAttrPtr->parent, this);
        pNodes->push_back(pElement->queryInterface((IXmlBaseNode*) 0));
      }
      xmlListDelete(oAttrXmlRefs);
    }

    return pNodes;
  }
}
