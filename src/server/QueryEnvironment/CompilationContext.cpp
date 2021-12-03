//platform agnostic
#include "CompilationContext.h"

#include "IXml/IXmlBaseNode.h"
#include "IXml/IXmlBaseDoc.h"
#include "IXml/IXslDoc.h"
#include "Database.h"
#include "Xml/XmlAdminQueryEnvironment.h"
#include "DatabaseClass.h"

#include <pthread.h>
#include <algorithm>

#include "Utilities/container.c"

namespace general_server {
 /*
  * Generic low-level direct triggering
  * Generally used for:
  * These are mostly designed to push design up to the next layer
  * rather than hardcoded or parameterised implementation here
  * hooks:
  *   the security hook fires before attemps to browse a node, for any reason. OP is passed in and through
  *   read triggers fire before and receive the context and xpath context
  *     useful for [re]populating dynamic content
  *     useful examining the xpath query and pre-populating the result only with required contents (and child count)
  *     sub-functions return xmlNodeListPtr not xmlDocPtr
  *     <repository:employers       database:on-before-read="database:replace-children(., postgres:get-query-results())" />    <-- xpath context input
  *     <repository:namespace-cache database:on-before-read="database:replace-children(., server:serialise('namespace-cache'))" />
  *     <repository:cron            database:on-before-read="database:replace-children(., repository:rx('/etc/crontab', ../rx:cron-tab))" />
  *     xpath: repository:employers/postgres:employer[@eid = 9]
  *   write triggers fire before and after write attempts
  *     the before trigger can cancel the write attempt
  *     useful for releasing watches on content
  *     @Database:on-after-write="database:transform(., )"
  *     @Database:on-after-write="database:move-node(., ../repository:archived)"
  *     @Database:on-after-write="database:set-attribute(@count, @count + 1)"
  */
  CompilationContext::CompilationContext(IXslDoc *pMainXSLDoc):
    m_pOwnerQE(0),
    m_pMainXSLDoc(pMainXSLDoc)
  {}

  CompilationContext::CompilationContext(const CompilationContext& trig):
    m_pOwnerQE(trig.m_pOwnerQE),
    m_pMainXSLDoc(trig.m_pMainXSLDoc)
  {}

  void CompilationContext::setQueryEnvironment(const IXmlQueryEnvironment *pOwnerQE) {m_pOwnerQE = pOwnerQE;}
  IXmlBaseDoc *CompilationContext::singleWriteableDocumentContext() const {return m_pMainXSLDoc->queryInterface((IXmlBaseDoc*) 0);}

  const char *CompilationContext::toString() const {return MM_STRDUP("CompilationContext");}
  const char *CompilationContext::type() const {return "CompilationContext";}

  CompilationContext *CompilationContext::clone_with_resources() const {
    return new CompilationContext(*this);
  }

  IXmlQueryEnvironment::accessResult CompilationContext::beforeReadTriggerCallback(const IXmlQueryEnvironment *pCE ATTRIBUTE_UNUSED, IXmlBaseNode *pCur ATTRIBUTE_UNUSED) {
    //pCE = Compilation Environment
    assert(pCur);

    /* We do not do this anymore
     * because xpath now honours inheritance
     * 
    const char *sMatch = 0,
               *sCTE   = 0;
    const IXmlBaseNode *pClassNode = 0;
    DatabaseClass *pMainDatabaseClass = 0;
    
    if (pCur->isName(NAMESPACE_XSL, "template")) {
      //is this a class template?
      if (pClassNode = pCur->getSingleNode(pCE, "ancestor::class:*[1]")) {
        if (pMainDatabaseClass = DatabaseClass::classFromClassNode(pClassNode)) {
          if (sMatch = pCur->attributeValue(pCE, "match")) {
            //may return sCTE = sMatch
            if (sCTE = pMainDatabaseClass->derivedTemplateMatchClause(sMatch)) {
              if (!_STREQUAL(sCTE, sMatch)) {
                //Debug::report("auto-morphing class:%s xsl:template @match=%s => [%s]", pMainDatabaseClass->name(), sMatch, sCTE);
                pCur->setAttribute(pCE, "match", sCTE);
              }
            }
          }
        }
      }
    }
    
    //free up
    if (sCTE && sCTE != sMatch) MMO_FREE(sCTE);
    if (sMatch)     MMO_FREE(sMatch);
    if (pClassNode) delete pClassNode;
    */
    
    return IXmlQueryEnvironment::RESULT_INCLUDE;
  }
  
  IXmlQueryEnvironment::accessResult CompilationContext::afterReadTriggerCallback(const IXmlQueryEnvironment *pCE ATTRIBUTE_UNUSED, IXmlBaseNode *pCur ATTRIBUTE_UNUSED) {
    return IXmlQueryEnvironment::RESULT_INCLUDE;
  }
}
