//platform agnostic file
#include "Tests.h"

#include "Database.h"
#include "DatabaseNode.h"
#include "Server.h"
#include "Conversation.h"
#include "Request.h"
#include "LibXmlLibrary.h" //full definition for usage
#include "Xml/XmlAdminQueryEnvironment.h"
#include "Utilities/container.c"

namespace general_server {
  Test::Test(const MemoryLifetimeOwner *pMemoryLifetimeOwner, IXmlQueryEnvironment *pQE, const IXmlBaseNode *pTestSpec):
    MemoryLifetimeOwner(pMemoryLifetimeOwner),
    m_sGroup(pTestSpec->group(pQE, NO_DUPLICATE)), //local-name(..)
    m_sName(pTestSpec->attributeValue(pQE, "name")),
    m_sType(pTestSpec->localName(NO_DUPLICATE)),
    m_sValue(pTestSpec->value(pQE)),
    m_sFlags(pTestSpec->attributeValue(pQE, "flags")),
    m_bDebugOnly(pTestSpec->attributeValueBoolDynamicString(pQE, "debug-mode-only")),
    m_sDisableWarning(pTestSpec->attributeValue(pQE, "disable-warning")),
    m_bRunTest(pTestSpec->attributeValueBoolDynamicString(pQE, "run", NULL, true)),
    m_iResultCount(pTestSpec->attributeValueInt(pQE, "result-count", NAMESPACE_NONE, 1)),
    m_sResultXML(pTestSpec->attributeValue(pQE, "result-xml")),
    m_sResultThrow(pTestSpec->attributeValue(pQE, "result-throw"))
  {}
  
  Test::~Test() {
    if (m_sName)        MMO_FREE(m_sName);
    if (m_sValue)       MMO_FREE(m_sValue);
    if (m_sFlags)       MMO_FREE(m_sFlags);
    if (m_sDisableWarning) MMO_FREE(m_sDisableWarning);
    if (m_sResultXML)   MMO_FREE(m_sResultXML);
    if (m_sResultThrow) MMO_FREE(m_sResultThrow);
  }
  
  
  const char *Test::toString() const {
    string sString;
    const char *sResultCount = itoa(m_iResultCount);
    
    sString += "Test [";
    sString += (m_sName ? m_sName : m_sType);
    sString += "] [";
    sString += (m_sValue && *m_sValue != '\n' ? m_sValue : "");
    sString += "] = [";
    sString += (m_sResultXML ? m_sResultXML : "");
    sString += "/";
    sString += sResultCount;
    sString += "/";
    sString += (m_sResultThrow ? m_sResultThrow : "");
    sString += "]: ";
    
    //free up
    if (sResultCount) MMO_FREE(sResultCount);
    
    return MMO_STRDUP(sString.c_str());
  }
  
  Tests::Tests(Database *pTestDatabase, const char *sTestGroupsExclude, const char *sTestGroupsInclude): 
    MemoryLifetimeOwner(pTestDatabase),
    m_pTestDatabase(pTestDatabase),
    IFDEBUG(m_bDEBUG_MODE(true))
    IFNDEBUG(m_bDEBUG_MODE(false))
  {
    static const char *sTestsXPath = "/*/repository:tests/repository:*/*";
    static const char *sDataXPath  = "/*/repository:tests/gs:data";
    const XmlNodeList<DatabaseNode> *pvTestSpecs;
    XmlNodeList<DatabaseNode>::const_iterator iTestSpec;
    DatabaseNode *pTestSpec;
    const char *sGroup, *sName, *sType, *sValue, *sFlags;
    const char *sLastGroup = 0;
    bool bDebugOnly, bRunTest, bTestOk, bAllTestsOk = true;
    bool (Tests::*fTest)(DatabaseNode *pTestSpec, const char *sXPath, const size_t iResultCount, const char *sResultXML, const char *sResultThrow) const;

    size_t iResultCount;
    const char *sResultXML      = 0, 
               *sResultThrow    = 0, 
               *sDisableWarning = 0;

    IFDEBUG(
      //fake request and environment setup
      m_pQE = m_pTestDatabase->newQueryEnvironment(NULL, this);
      m_pQE->triggerContext()->disable();
      m_pTestDatabase->disableLinkedProcessors();
      DatabaseNode *pFakeRequestNode = m_pTestDatabase->getSingleNode(m_pQE, sDataXPath, "/object:Request");
      Request  oFakeRequest(this, pFakeRequestNode);
      registerXslModuleManager(m_pTestDatabase);
      registerXslModuleManager(&oFakeRequest);
      
      pvTestSpecs = m_pTestDatabase->getMultipleNodes(m_pQE, sTestsXPath);
      cout << "running [" << pvTestSpecs->size() << "] [" << (sTestGroupsInclude ? sTestGroupsInclude : "") << "] tests from [" << sTestsXPath << "]\n";
      for (iTestSpec = pvTestSpecs->begin(); iTestSpec != pvTestSpecs->end(); iTestSpec++) {
        pTestSpec    = *iTestSpec;
        sGroup       = pTestSpec->group(m_pQE, NO_DUPLICATE); //local-name(..)
        sName        = pTestSpec->attributeValue(m_pQE, "name");
        sType        = pTestSpec->localName(NO_DUPLICATE);
        sValue       = pTestSpec->value(m_pQE);
        sFlags       = pTestSpec->attributeValue(m_pQE, "flags");
        bDebugOnly   = pTestSpec->attributeValueBoolDynamicString(m_pQE, "debug-mode-only");
        sDisableWarning = pTestSpec->attributeValue(m_pQE, "disable-warning");
        bRunTest     = pTestSpec->attributeValueBoolDynamicString(m_pQE, "run", NULL, true);
        
        iResultCount = pTestSpec->attributeValueInt(m_pQE, "result-count", NAMESPACE_NONE, 1);
        sResultXML   = pTestSpec->attributeValue(m_pQE, "result-xml");
        sResultThrow = pTestSpec->attributeValue(m_pQE, "result-throw");
        bTestOk      = false; 
        fTest        = NULL;
        
        if ( (sTestGroupsExclude == NULL || strstr(sTestGroupsExclude, sGroup) == NULL)
          && (sTestGroupsInclude == NULL || strstr(sTestGroupsInclude, sGroup) != NULL)
        ) {
          if (_STRDIFFERENT(sLastGroup, sGroup)) {
            sLastGroup = sGroup;
            if (sLastGroup) cout << "\n------------------------------ " << sLastGroup << "\n";
          }
          
          if (bRunTest) {
            if (!bDebugOnly || m_bDEBUG_MODE) {
              cout << "  Running Test [" << (sName ? sName : sType) << "] [" << (sValue && *sValue != '\n' ? sValue : "") << "] = [" << (sResultXML ? sResultXML : "") << " / " << iResultCount << " / " << (sResultThrow ? sResultThrow : "") << "]: ";
              if      (_STREQUAL(sType, "xpath"))        fTest = &Tests::runXPathTest;
              else if (_STREQUAL(sType, "xpath-string")) fTest = &Tests::runXPathStringTest;
              else if (_STREQUAL(sType, "transform"))    fTest = &Tests::runTransformTest;
              else                                       cout << "UNKNOWN test type! ";
              if (fTest) {
                Debug::ignore(sDisableWarning);
                try {
                  bTestOk = (this->*fTest)(pTestSpec, sValue, iResultCount, sResultXML, sResultThrow);
                } 
                //---------------------------------- result-throw requests
                catch (ExceptionBase &eb) {
                  if (sResultThrow) {
                    if (eb.involvesType(sResultThrow)) {
                      cout << "Exception requested and caught [" << sResultThrow << "]\n";
                      bTestOk = true;
                    } else {
                      cout << "Wrong Exception caught [" << eb.type() << "]\n";
                    }
                    //MMO_FREE(sOutestType); //pointer in to constant
                  } else {
                    if (!sDisableWarning || !strstr(sDisableWarning, eb.type())) throw;
                  }
                } 
                catch (...) {
                  if (sResultThrow) {
                    cout << "Standard Exception requested and caught";
                    bTestOk = true;
                  } else throw;
                }
              }
                  
              if (bTestOk) cout << " OK\n";
              else {
                bAllTestsOk = false;
                if (sResultThrow) cout << "Throw requested and ";
                cout << "FAILED\n";
                
                //run the test again with debug things on
                //XML_DEBUG_ALL, XML_DEBUG_NONE, XML_DEBUG_XPATH_STEP, XML_DEBUG_XPATH_EXPR, XML_DEBUG_XPATH_EVAL_COUNTS, XML_DEBUG_PARENT_ROUTE
                //XSLT_TRACE_ALL, XSLT_TRACE_PROCESS_NODE, XSLT_TRACE_APPLY_TEMPLATE, XSLT_TRACE_APPLY_TEMPLATES, XSLT_TRACE_CALL_TEMPLATE, XSLT_TRACE_TEMPLATES, XSLT_TRACE_VARIABLES, XSLT_TRACE_VARIABLE
                //  XSLT_TRACE_COPY_TEXT, XSLT_TRACE_COPY, XSLT_TRACE_COMMENT, XSLT_TRACE_PI, XSLT_TRACE_COPY_OF, XSLT_TRACE_VALUE_OF, XSLT_TRACE_CHOOSE, XSLT_TRACE_IF, XSLT_TRACE_FOR_EACH, XSLT_TRACE_STRIP_SPACES, XSLT_TRACE_KEYS, XSLT_TRACE_FUNCTION, XSLT_TRACE_PARSING, XSLT_TRACE_BLANKS, XSLT_TRACE_PROCESS, XSLT_TRACE_EXTENSIONS, XSLT_TRACE_ATTRIBUTES, XSLT_TRACE_EXTRA, XSLT_TRACE_AVT, XSLT_TRACE_PATTERN, XSLT_TRACE_MOST, XSLT_TRACE_NONE
                /*
                m_pQE->setXMLTraceFlags( m_pTestDatabase->xmlLibrary()->parseXMLTraceFlags( sFlags ? sFlags : "XML_DEBUG_ALL" )); //actually XML Library level
                m_pQE->setXSLTTraceFlags(m_pTestDatabase->xmlLibrary()->parseXSLTTraceFlags(sFlags ? sFlags : "XSLT_TRACE_ALL")); //actually XML Library level
                (this->*fTest)(pTestSpec, sValue, iResultCount, sResultXML);
                m_pQE->clearXMLTraceFlags();
                m_pQE->clearXSLTTraceFlags();
                */
              }
            }
          } //else cout << (sName ? sName : sType) << " test @run=false\n";
        } //else cout << (sName ? sName : sType) << " test ignored\n";

        //loop free up
        if (sName)        MMO_FREE(sName);
        if (sValue)       MMO_FREE(sValue);
        if (sFlags)       MMO_FREE(sFlags);
        if (sDisableWarning) MMO_FREE(sDisableWarning);
        if (sResultXML)   MMO_FREE(sResultXML);
        if (sResultThrow) MMO_FREE(sResultThrow);
        //if (sType)      MMO_FREE(sType);  //NO_DUPLICATE
        //if (sGroup)     MMO_FREE(sGroup); //NO_DUPLICATE
      }
      m_pQE->triggerContext()->enable();
      m_pTestDatabase->enableLinkedProcessors();
      Debug::ignore();
        
      cout << "Tests complete: ";
      if (bAllTestsOk) cout << "OK";
      else throw TestsFailed(this);
      cout << "\n\n";
      
      //free up
      vector_const_element_destroy(pvTestSpecs);
      //if (pFakeRequestNode) delete pFakeRequestNode; //managed by Request
    );
  }

  const char *Tests::xslModuleManagerName() const {return "Test";}
  const IXmlLibrary *Tests::xmlLibrary() const {return m_pQE->xmlLibrary();}

  bool Tests::runTransformTest(DatabaseNode *pTestSpec, const char *sXPath, const size_t iResultCount, const char *sResultXML, const char *sResultThrow) const {
    DatabaseNode *pStyleSheetNode = 0;
    IXmlBaseDoc *pOutputDoc = 0;
    const char *sXML = 0;
    size_t iLen;
    bool bTestOk = false;              
    
    if (pStyleSheetNode = pTestSpec->firstChild(m_pQE))
      if (pOutputDoc = m_pTestDatabase->transform(this, pStyleSheetNode->node_const((IXslStylesheetNode*) 0), m_pQE))
        sXML = pOutputDoc->xml(m_pQE, false);
        if (sResultXML && !*sResultXML && !sXML) bTestOk = true;
        else if (sXML) {
          //ignore trailing \n
          //TODO: why is there a trailing \n?
          iLen = strlen(sXML);
          while (iLen && sXML[iLen-1] == '\n') iLen--;
          bTestOk = _STRNEQUAL(sXML, sResultXML, iLen);
        }
    
    if (!bTestOk) {
      cout << "\n";
      cout << "Test XML Result:   [" << (sXML ? sXML : "<no result>") << "]\n";
      cout << "Test XML Required: [" << (sResultXML ? sResultXML : "<no test>") << "]\n";
    }

    //free up
    if (pStyleSheetNode) delete(pStyleSheetNode);
    if (pOutputDoc)      delete(pOutputDoc);
    if (sXML)            MMO_FREE(sXML);

    return bTestOk;
  }
  
  bool Tests::runXPathTest(DatabaseNode *pTestSpec, const char *sXPath, const size_t iResultCount, const char *sResultXML, const char *sResultThrow) const {
    const XmlNodeList<DatabaseNode> *pvTestResults = 0;
    const char *sXML = 0;
    bool bTestOk = false;
    
    if (sXPath) {
      if (pvTestResults = m_pTestDatabase->getMultipleNodes(m_pQE, sXPath)) {
        if (sResultXML) {
          if (sXML = pvTestResults->xml(m_pQE)) {
            bTestOk = _STREQUAL(sXML, sResultXML);
            if (!bTestOk) cout << "\nTest XML Result: " << (sXML ? sXML : "<no result>") << "\n";
          }
        } else bTestOk = true;
        
        if (bTestOk) {
          bTestOk = (pvTestResults->size() == iResultCount);
          if (!bTestOk) cout << "\nTest @result-count: " << pvTestResults->size() << "\n";
        }        
      }
    }

    //free up
    if (pvTestResults) vector_const_element_destroy(pvTestResults);
    if (sXML)          MMO_FREE(sXML);
    
    return bTestOk;
  }
  
  bool Tests::runXPathStringTest(DatabaseNode *pTestSpec, const char *sXPath, const size_t iResultCount, const char *sResultXML, const char *sResultThrow) const {
    char *sString = 0;
    bool bTestOk = false;
    iXPathType iType;
    
    if (sXPath) {
      pTestSpec->node_const()->getObject(m_pQE, sXPath, NULL, &sString, NULL, NULL, &iType);
      if (iType == TYPE_STRING && sString && sResultXML) {
        bTestOk   = _STREQUAL(sString, sResultXML);
        if (!bTestOk) cout << "\nTest XML Result: " << (sString ? sString : "<no result>") << "\n";
      }
    }

    //free up
    if (sString != sXPath)   MMO_FREE(sString);
    
    return bTestOk;
  }
  
  const char *Tests::toString() const {return MMO_STRDUP("Tests");}
}
