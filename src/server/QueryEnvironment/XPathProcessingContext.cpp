//platform agnostic
#include "XPathProcessingContext.h"

#include "IXml/IXmlBaseNode.h"
#include "IXml/IXmlBaseDoc.h"
#include "IXml/IXslDoc.h"
#include "Database.h"
#include "DatabaseClass.h"
#include "Xml/XmlAdminQueryEnvironment.h"
#include "Debug.h"
#include "Utilities/StringVector.h"

namespace general_server {
  XPathProcessingContext::XPathProcessingContext():
    m_bEnabled(true),
    m_pOwnerQE(0)
  {
  }

  XPathProcessingContext::XPathProcessingContext(const XPathProcessingContext& xprocessor):
    m_bEnabled(xprocessor.m_bEnabled),
    m_pOwnerQE(0)
  {}

  void XPathProcessingContext::setQueryEnvironment(const IXmlQueryEnvironment *pOwnerQE) {m_pOwnerQE = pOwnerQE;}

  const char *XPathProcessingContext::toString() const {
    return MM_STRDUP("XPathProcessingContext");
  }
  const char *XPathProcessingContext::type() const {return "XPathProcessingContext";}

  XPathProcessingContext *XPathProcessingContext::clone_with_resources() const {
    return new XPathProcessingContext(*this);
  }

  IXmlXPathProcessingContext *XPathProcessingContext::inherit() const {
    return clone_with_resources();
  }

  IXmlQueryEnvironment::xprocessorResult XPathProcessingContext::checkName(const IXmlBaseNode *pCur, const char *sXPathPrefix, const char *sXPathLocalName) const {
    //EXECUTION phase:
    //during xpath evaluation, including compiled xsl:template/@match evaluation
    //we include matches for base classes of the sClause

    //only called if xpath native XP_TEST has so far XP_TEST_HIT_NO
    //sPrefix/sLocalName = the current name test from the xpath clause / match
    //pCur               = the cur node tested
    
    //e.g. 
    //  looking for .../object:User[...]/...
    //  found <object:Person>
    //we want that to match so that Person can fullfill anything that wanted a User
    IXmlQueryEnvironment::xprocessorResult ret = IXmlQueryEnvironment::XP_TEST_HIT_NO;
    const vector<DatabaseClass*> *pvClasses    = 0;
    vector<DatabaseClass*> vDerivedClasses;
    vector<DatabaseClass*>::const_iterator iDerivedClass, iClass;
    DatabaseClass *pClass, *pDerivedClass;
    
    //inheritance
    //string check immediately to see if we have an @elements match on the saught xpath clause / match
    //ignore namespace:* and * isAllNamespaceElements() classes
    if (pvClasses = DatabaseClass::classesFromElement(sXPathLocalName, sXPathPrefix, false, false)) { //false = no namespace classes, e.g. javascript:*
      for (iClass = pvClasses->begin(); iClass != pvClasses->end() && ret == IXmlQueryEnvironment::XP_TEST_HIT_NO; iClass++) {
        pClass = *iClass;
        
        //if any of the xpath [/object:User] derived classes [object:Person] have this pCur node test then return a XP_TEST_HIT
        //e.g. <object:Person> class:Person/@elements=object:Person
        pClass->allDerivedUnique_recursive(&vDerivedClasses);
        for (iDerivedClass = vDerivedClasses.begin(); iDerivedClass != vDerivedClasses.end() && ret == IXmlQueryEnvironment::XP_TEST_HIT_NO; iDerivedClass++) {
          pDerivedClass = *iDerivedClass;
          if (pDerivedClass->gimme(pCur)) ret = IXmlQueryEnvironment::XP_TEST_HIT;
        }
        vDerivedClasses.empty();
      }
    }

    //free up
    //if (pvDerivedClasses) delete pvDerivedClasses; //pointer in to DatabaseClass
    if (pvClasses) delete pvClasses;
    
    return ret;
  }

  StringVector *XPathProcessingContext::templateSuggestionsForLocalName(const char *sLocalName) const {
    //COMPILATION phase:
    //fast lookup grouping of compiled templates by base @match local-name
    //e.g. if @match=object:Person, xsl:template is placed under "Person" key
    //all relevant templates will then filtered by a full @match xsl xsltCompTest() evaluation (not xpath)
    //
    //include inheritance:
    //we include all the xsl:template[@match=object:User] for base name "Person" here as well
    //so if sLocalName == User, we include derived Person and Manager groups
    StringVector *pvNameLookupGroups = 0;
    const vector<DatabaseClass*> *pvClasses = 0;
    vector<DatabaseClass*> vDerivedClasses;
    vector<DatabaseClass*>::const_iterator iDerivedClass, iClass;
    DatabaseClass *pClass, *pDerivedClass;
    
    //inheritance
    //string check immediately to see if we have an @elements match on the clause / match
    //e.g. object:User
    //ignore object:* and * isAllNamespaceElements() classes
    if (pvClasses = DatabaseClass::classesFromLocalName(sLocalName, false, false)) {
      for (iClass = pvClasses->begin(); iClass != pvClasses->end(); iClass++) {
        pClass = *iClass;
        //if any of the derived classes have this node test then return a XP_TEST_HIT
        //e.g. class:Person/@elements=object:Person
        pClass->allDerivedUnique_recursive(&vDerivedClasses);
        for (iDerivedClass = vDerivedClasses.begin(); iDerivedClass != vDerivedClasses.end(); iDerivedClass++) {
          pDerivedClass = *iDerivedClass;
          if (!pvNameLookupGroups) pvNameLookupGroups = new StringVector();
          pvNameLookupGroups->insert_unique(pDerivedClass->elements()->begin(), pDerivedClass->elements()->end());
        }
        vDerivedClasses.empty();
      }
    }

    /*
    IFDEBUG(
      const char *sNameLookupGroups;
      if (pvNameLookupGroups) {
        if (sNameLookupGroups = pvNameLookupGroups->toString()) {
          Debug::report("xsl:template compilation lookups [%s] => [%s]", sLocalName, sNameLookupGroups);
          MMO_FREE(sNameLookupGroups);
        }
      }
    );
    */
    
    //free up
    //if (pvDerivedClasses) delete pvDerivedClasses; //pointer in to DatabaseClass
    if (pvClasses) delete pvClasses;
    
    return pvNameLookupGroups;
  }
}
