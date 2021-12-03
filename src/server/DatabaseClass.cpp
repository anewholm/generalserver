//platform agnostic file
#include "DatabaseClass.h"

#include "Database.h"
#include "DatabaseNode.h"
#include "Debug.h"
#include "IXml/IXslXPathFunctionContext.h"
#include "IXml/IXmlMaskContext.h"
#include "IXml/IXmlXPathProcessingContext.h"
#include "IXml/IXslTransformContext.h"

#include "Utilities/container.c"
#include "Utilities/strtools.h"
#include <algorithm>
using namespace std;

namespace general_server {
  StringMap<DatabaseClass*> DatabaseClass::m_mClassesByName;

  void DatabaseClass::loadClasses(DatabaseAdminQueryEnvironment *pIBQE_dbAdministrator, DatabaseNode *pFromNode) {
    XmlNodeList<DatabaseNode>::iterator iClassNode;
    XmlNodeList<DatabaseNode> *pvNewClassNodes;
    vector<DatabaseClass*>::iterator iDatabaseClass;
    vector<DatabaseClass*> vNewDatabaseClasses;
    DatabaseNode *pNode;
    DatabaseClass *pClass;

    //instanciate top level classes first
    //avoid transient areas like the Transactions list
    //no inheritance hardlinked classes
    //DatabaseClass constructor will add itself to the appropriate static maps
    //DatabaseClass constructor can throw DatabaseClassAlreadyExists
    //descendant-natural ignores hardlinks
    pvNewClassNodes = pFromNode->getMultipleNodes(pIBQE_dbAdministrator, "descendant-natural::class:*"); 
    pvNewClassNodes->removeDuplicates();
    for (iClassNode = pvNewClassNodes->begin(); iClassNode != pvNewClassNodes->end(); iClassNode++) {
      pNode = *iClassNode;
      if (!pNode->isTransient()) {
        try {
          pClass = 0;
          pClass = new DatabaseClass(pIBQE_dbAdministrator, pNode);
        } catch (ExceptionBase &eb) {}
        if (pClass) vNewDatabaseClasses.push_back(pClass);
      }
    }

    //initDatabaseClassDependentProperties like inheritance
    //that requires all the Class pointers available already
    for (iDatabaseClass = vNewDatabaseClasses.begin(); iDatabaseClass != vNewDatabaseClasses.end(); iDatabaseClass++) {
      try {
        (*iDatabaseClass)->initDatabaseClassDependentProperties(pFromNode->db());
      } catch (UnregisteredDatabaseClass& udbc) {
        Debug::reportObject(&udbc);
      }
    }

    //free up
    delete pvNewClassNodes;
    //delete vNewDatabaseClasses; //local variable
  }

  DatabaseClass::DatabaseClass(const DatabaseAdminQueryEnvironment *pIBQE_dbAdministrator, DatabaseNode *pClassNode): 
    MemoryLifetimeOwner(pClassNode->db()),
    GeneralServerDatabaseNodeServerObject(pClassNode),
    m_sNamespacePrefix(0),
    m_sNodeTestXPath(0),
    m_sTemplateMatchClause(0),
    m_sNamespaceHREF(0)
  {
    //initialise non-inter-dependent-DatabaseClass dependent properties
    //note that inheritance must be carried out AFTER all DatabaseClasses are instanciated
    //using initDatabaseClassDependentProperties() below
    //class names must be unique, elements not
    //multimap: duplicates are allowed!
    //e.g. html:script, javascript:code
    StringVector::iterator iElement;
    const char *sElements         = 0,
               *sElementName      = 0,
               *sElementNameStart = 0,
               *sElementNameEnd   = 0,
               *sXmlID            = 0,
               *sNameAttribute    = 0;
    string stNodeTestXPath, stTemplateMatchClause;
    const DatabaseNode *pTestNode = 0;

    UNWIND_EXCEPTION_BEGIN {
      //------------------ required attaributes
      //@xml:id = Class__<name>
      //@name   = <name>
      m_sName            = pClassNode->localName();
      sXmlID             = pClassNode->xmlID(pIBQE_dbAdministrator);
      sNameAttribute     = pClassNode->attributeValue(pIBQE_dbAdministrator, "name");
      
      //------------------ optional attributes
      //namespace or predicate requires elements
      sElements          = pClassNode->attributeValue(pIBQE_dbAdministrator, "elements"); //default: *
      m_sNamespacePrefix = pClassNode->attributeValue(pIBQE_dbAdministrator, "namespace-prefix");
      m_sPredicate       = pClassNode->attributeValue(pIBQE_dbAdministrator, "predicate");
      
      if (!m_sName || !*m_sName) throw DatabaseClassNameRequired(this);
      if (!sXmlID || !_STRNEQUAL(sXmlID, "Class__", 7) || !_STREQUAL(sXmlID + 7, m_sName)) throw DatabaseClassNameWrong(this, m_sName);
      if (!sNameAttribute || !_STREQUAL(sNameAttribute, m_sName)) throw DatabaseClassNameXmlIDWrong(this, m_sName);
      if (!sElements && (m_sPredicate || m_sNamespacePrefix))     throw DatabaseMatchFailed(this, m_sName, MM_STRDUP("@predicate or @namespace-prefix requires @elements"));
      
      //------------------ calculated attributes
      //blank or missing @elements indicates that the class does not have an element presence
      //@namespace-prefix and missing @elements = a namespace class, e.g. class:Repository = repository:*
      //only @elements=* and no @namespace-prefix is a special case of handling ALL elements
      //  this should only occur once in the system
      m_sNamespaceHREF = NULL; //TODO: m_sNamespaceHREF
      
      if (sElements && *sElements) {
        //---------------------------- elements with optional namespace and predicate
        //=self::<namespace?>:element1[<predicate>]|self::<namespace?>:element2[<predicate>]|...
        //elements can include *
        //namespace and predicate are optional
        sElementNameEnd = sElements - 1;
        do {
          sElementNameStart = sElementNameEnd + 1; //maybe pointing to EOL or another space
          sElementNameEnd   = sElementNameStart;
          while (*sElementNameEnd != ' ' && *sElementNameEnd != '\0') sElementNameEnd++;
          if (sElementNameEnd != sElementNameStart) {
            sElementName = _STRNDUP(sElementNameStart, sElementNameEnd - sElementNameStart);
            
            //construct the xpath
            //it runs on the self axis so it can be run directly against a node
            //leaving the default children axis would cause it to evaluate the children of course
            //e.g. bIsThisClass = pNodeToTest->getSingleNode(&ibqe, self::object:User)
            if (stNodeTestXPath.size())       stNodeTestXPath += '|';
            if (stTemplateMatchClause.size()) stTemplateMatchClause += '|';
            stNodeTestXPath += "self::";
            if (m_sNamespacePrefix && *m_sNamespacePrefix) {
              stNodeTestXPath       += m_sNamespacePrefix;
              stNodeTestXPath       += ':';
              stTemplateMatchClause += m_sNamespacePrefix; 
              stTemplateMatchClause += ':';
            }
            stNodeTestXPath       += sElementName; //maybe *
            stTemplateMatchClause += sElementName; //maybe *
            if (m_sPredicate && *m_sPredicate) {
              stNodeTestXPath       += '[';
              stNodeTestXPath       += m_sPredicate;
              stNodeTestXPath       += ']';
              stTemplateMatchClause += '[';
              stTemplateMatchClause += m_sPredicate;
              stTemplateMatchClause += ']';
              
            }
            
            m_vElements.push_back(sElementName);
          }
        } while (*sElementNameEnd == ' ');
        
        //------------------ store and check xpath
        //will throw an Exception if it cannot compile it...
        //namespace prefix:element[predicate]
        //m_sNodeTestXPath is also the indicator of an active element class
        m_sNodeTestXPath       = MM_STRDUP(stNodeTestXPath.c_str());
        m_sTemplateMatchClause = MM_STRDUP(stTemplateMatchClause.c_str());
        if (xmlLibrary()->isAttributeXPath(m_sNodeTestXPath)) throw ClassElementsMustBeElement(this, m_sNodeTestXPath);
        pTestNode = db()->getSingleNode(pIBQE_dbAdministrator, m_sNodeTestXPath);
      }

      //------------------ names lookup
      //single map, duplicates are NOT allowed
      //e.g. CSSStylesheet, CarSalesCSSStylesheet etc.
      if (m_mClassesByName.find(m_sName) != m_mClassesByName.end()) throw DuplicateDatabaseClass(this, m_sName);
      m_mClassesByName.insert(m_sName, this);
    } UNWIND_EXCEPTION_END;

    //free up
    if (sElements)      MMO_FREE(sElements);
    if (sXmlID)         MMO_FREE(sXmlID);
    if (sNameAttribute) MMO_FREE(sNameAttribute);
    if (pTestNode)      delete pTestNode;

    UNWIND_EXCEPTION_THROW;
  }

  DatabaseClass::~DatabaseClass() {
    if (m_sName)            MMO_FREE(m_sName);
    if (m_sNamespacePrefix) MMO_FREE(m_sNamespacePrefix);
    if (m_sPredicate)       MMO_FREE(m_sPredicate);
    if (m_sNamespaceHREF)   MMO_FREE(m_sNamespaceHREF);
    if (m_sNodeTestXPath)    MMO_FREE(m_sNodeTestXPath);
    vector_element_free(m_vElements);
  }

  void DatabaseClass::clearClasses() {
    //delete all DatabaseClasses
    //~DatabaseClass() is private so we manually delete them
    //map_elements_delete(m_mClassesByName);
    StringMap<DatabaseClass*>::iterator iMap;
    for (iMap = m_mClassesByName.begin(); iMap != m_mClassesByName.end(); iMap++) {
      MM_FREE(iMap->first);
      delete iMap->second;
    }
  }

  char *DatabaseClass::classIDXPathFromName(const char *sClassName, size_t iNameLen) {
    char *sClassIDXPath;
    
    if (!iNameLen) iNameLen = strlen(sClassName);
    sClassIDXPath = (char*) MM_MALLOC_FOR_RETURN(iNameLen + 14); 
    memcpy(sClassIDXPath, "id('Class__", 11);
    memcpy(sClassIDXPath + 11, sClassName, iNameLen);
    memcpy(sClassIDXPath + 11 + iNameLen, "')", 3);
    
    return sClassIDXPath;
  }
  
  DatabaseClass *DatabaseClass::popInterpretDatabaseClassFromXPathFunctionCallStack(IXslXPathFunctionContext *pXCtxt, const char *sFunctionSignature) {
    //xpath stack pop can be:
    //  string name of class
    //  class:* node
    DatabaseClass *pDatabaseClass = 0;
    const IXmlBaseNode *pInputNode = 0;
    const char *sClassName = 0;

    UNWIND_EXCEPTION_BEGIN_STATIC(pXCtxt) {
      if (pXCtxt->valueCount() > 0) {
        switch (pXCtxt->xtype()) {
          //------------- specific className string sent through: check map
          case XPATH_STRING: {
            sClassName = pXCtxt->popInterpretCharacterValueFromXPathFunctionCallStack();
            if (sClassName && *sClassName) pDatabaseClass = DatabaseClass::classFromName(sClassName);
            else throw XPathReturnedEmptyResultSet(pXCtxt, MM_STRDUP("Input string empty or wrong type"), sFunctionSignature);
            break;
          }
          //------------- node sent through: class:* or other node
          case XPATH_NODESET: {
            pInputNode = pXCtxt->popInterpretNodeFromXPathFunctionCallStack();
            if (pInputNode) {
              if (pInputNode->isNamespace(NAMESPACE_CLASS)) {
                pDatabaseClass = DatabaseClass::classFromClassNode(pInputNode);
              } else throw XPathFunctionWrongArgumentType(pXCtxt, MM_STRDUP("node"), MM_STRDUP("node must be class:* namespace"), sFunctionSignature);
            } else throw XPathFunctionWrongArgumentType(pXCtxt, MM_STRDUP("node"), MM_STRDUP("nodeset empty"), sFunctionSignature);
            break;
          }
          default: {
            throw XPathFunctionWrongArgumentType(pXCtxt, MM_STRDUP("string|nodeset"), MM_STRDUP("unknown"), sFunctionSignature);
          }
        }
      } else throw XPathTooFewArguments(pXCtxt, sFunctionSignature);
    } UNWIND_EXCEPTION_END_STATIC(pXCtxt);

    //free up
    if (sClassName) MM_FREE(sClassName);
    if (pInputNode) delete pInputNode;

    UNWIND_EXCEPTION_THROW;

    return pDatabaseClass;
  }

  const char *DatabaseClass::derivedTemplateMatchClause(const char *sMatch) const {
    //caller ONLY frees the result if it is not equal to the input
    //Class__XSLTemplate/name::inheritance XSL to client:
    //  @match = str:dynamic(database:derived-template-match(@match))
    //e.g. for class:User @namespace-prefix=object @elements=User
    //  object:User       => object:User|object:Person|object:Manager
    //  (object:User)     => (object:User)
    //  object:User[@a=1] => object:User[@a=1]|object:Person[@a=1]|object:Manager[@a=1]
    //returns:
    //  xpath string containing the self and inherited classes @elements separated by xpath OR (|)
    //  the @match MUST start with the associated Class template match string otherwise it will be returned as-is
    //  if ANY of the inherited class @elements are * then just * will be returned
    //  TODO: if ANY of the inherited class @elements are @* or text() then just that will be returned
    //TODO: cacheing of derived @elements?
    string stXPathElements;
    const char *sTrailingMatch           = 0,
               *sNewMatch                = 0;
    DatabaseClass *pDerivedDatabaseClass = 0;
    vector<DatabaseClass*>::const_iterator iDerivedClass;

    if (!sMatch || !sMatch[0]) {
      //if no match was sent through then we return the inherited @elements
      if (!m_sTemplateMatchClause || !m_sTemplateMatchClause[0])
        throw XPathFunctionWrongArgumentType(this, MM_STRDUP("node"), MM_STRDUP("xsl:template must have a valid class:*/@elements, or send through an explicit match string"));
      sMatch = MM_STRDUP(m_sTemplateMatchClause);
    }
    
    if (sMatch[0] == '*' && sMatch[1] != '[') {
      //we have a match on all elements, without a predicate
      //but potentially with more axis selectors, e.g. */@meta:class
      //only the DatabaseElement should have catch all xsl:templates
      //therefore no need to compile other @elements
      sNewMatch = MM_STRDUP(sMatch);
    } else {
      //we have a valid match string to process that:
      //  might == @elements
      //  might contain more <@elements>/xpath
      //check @elements is not blank, return the @match if it is
      stXPathElements = sMatch;
      if (m_sTemplateMatchClause && *m_sTemplateMatchClause) {
        //we have a valid non-zero @elements matcher, e.g. Resource
        //see if @match is starting with @elements
        //  e.g. @match=interface:HTMLObject and @namespace-prefix=interface, @elements=HTMLObject
        if (_STRNEQUAL(sMatch, m_sTemplateMatchClause, strlen(m_sTemplateMatchClause))) {
          //@match starts with @elements
          //BUT not necessarily equal to...
          //work out the extra xpath conditions after @elements
          //e.g. object:User/@meta:classes = object:User + /@meta:classes
          sTrailingMatch  = sMatch + strlen(m_sTemplateMatchClause); //potentially zero length

          //------------------------------------ derived classes
          //@elements + sTrailingMatch separated by |
          //if this class applies to all elements in this namespace then no need to compile derived match
          if (!isAllNamespaceElements()) {
            for (iDerivedClass = m_vDerivedClasses.begin(); iDerivedClass != m_vDerivedClasses.end(); iDerivedClass++) {
              pDerivedDatabaseClass = *iDerivedClass;
              if (pDerivedDatabaseClass->hasElements()) {
                if (pDerivedDatabaseClass->isAllNamespaceElements()) {
                  //this derived class applies to all elements
                  //which is strange because the base does not
                  //no need to include any previous or other derived classes
                  stXPathElements = pDerivedDatabaseClass->m_sTemplateMatchClause;
                  stXPathElements += sTrailingMatch;
                  break;
                }
                if (stXPathElements.size()) stXPathElements += '|';
                stXPathElements += pDerivedDatabaseClass->m_sTemplateMatchClause;
                stXPathElements += sTrailingMatch;
              }
            }
          }
        } //if starts with m_sTemplateMatchClause, i.e. is subject to inheritance
      }
      sNewMatch = MM_STRDUP(stXPathElements.c_str());
    } //else  
    
    //free up
    //if (sTrailingMatch)        MMO_FREE(sTrailingMatch); //pointer in to sMatch

    return sNewMatch;
  }
  
  IXmlBaseNode *DatabaseClass::xschemaNodeMask(const IXmlQueryEnvironment *pQE, const IXmlBaseNode *pInputNode, const char *sInterfaceMode) const {
    //caller frees non zero response
    //send through an Admin QE if the true xschema is needed
    IXmlBaseNode *pTmpNode  = 0,
                 *pNode     = 0;
    StringMap<const char*> mParamsChar;
    XmlNodeList<const IXmlBaseNode> lParamsNode(this);
    StringMap<const XmlNodeList<const IXmlBaseNode>*> mParamsNode;

    assert(pInputNode);
    
    NOT_CURRENTLY_USED("");

    UNWIND_EXCEPTION_BEGIN {
      if (pTmpNode = db()->createTmpNode()) { //auto-transient
        if (sInterfaceMode) mParamsChar.insert("gs_interface_mode", sInterfaceMode);
        lParamsNode.push_back(pInputNode);
        mParamsNode.insert("gs_input_node", &lParamsNode);
        m_pNode->inheritanceTransform("XSchema", pQE, pTmpNode,
          NO_PARAMS_INT, &mParamsChar, &mParamsNode,
          "gs_xsd_node_mask");
        pNode = pTmpNode->firstChild(&m_ibqe_nodeControl);
        delete pTmpNode;
      } else throw CannotCreateInTmpNode(this, db()->name());
    } UNWIND_EXCEPTION_END;

    //free up

    UNWIND_EXCEPTION_THROW;

    return pNode;
  }

  IXmlBaseNode *DatabaseClass::xschema(const IXmlQueryEnvironment *pQE, const char *sXSchemaName, const bool bStandalone, const bool bCompiled) const {
    //caller frees non zero response
    //send through an Admin QE if the true xschema is needed
    //TODO: some sort of warning if there are xschema nodes that are not visible to this QE?
    //  for sure the Database will use a full Admin QE and see a different view if there is security...
    IXmlBaseNode *pTmpNode         = 0,
                 *pXSchemaNode     = 0;
    StringMap<const char*> paramsChar;
    StringMap<const size_t> paramsInt;
    const char *sMode;

    NOT_CURRENTLY_USED("");
    
    UNWIND_EXCEPTION_BEGIN {
      if (pTmpNode = db()->createTmpNode()) { //auto-transient
        sMode = (bStandalone ? "gs_inheritance_render_xschema_standalone" : "gs_inheritance_render_xschema_complement");
        //TODO: these params need to be passed to the template, not global
        if (sXSchemaName) paramsChar.insert("gs_xschema_name", sXSchemaName);
        paramsInt.insert("gs_inheritance_render_xschema_compiled",     bCompiled);
        m_pNode->inheritanceTransform("XSchema", pQE, pTmpNode, &paramsInt, &paramsChar, NO_PARAMS_NODE, sMode);
        pXSchemaNode = pTmpNode->firstChild(&m_ibqe_nodeControl);
        delete pTmpNode;
      } else throw CannotCreateInTmpNode(this, db()->name());
    } UNWIND_EXCEPTION_END;

    //free up
    //if (sMode) MMO_FREE(sMode); //static

    UNWIND_EXCEPTION_THROW;

    return pXSchemaNode;
  }

  IXmlBaseNode *DatabaseClass::xstylesheet(const IXmlQueryEnvironment *pQE, const bool bStandalone, const bool bClientSideOnly) const {
    //caller frees non zero response
    IXmlBaseNode *pTmpNode         = 0,
                 *pXStylesheetNode = 0;
    StringMap<const size_t> mParamsInt;
    const char *sMode;

    NOT_CURRENTLY_USED("");
    
    UNWIND_EXCEPTION_BEGIN {
      if (pTmpNode = db()->createTmpNode()) { //auto-transient
        sMode = (bStandalone ? "gs_inheritance_render_xsl_standalone" : "gs_inheritance_render_xsl_complement");
        mParamsInt.insert("gs_client_side_only", bClientSideOnly);
        m_pNode->inheritanceTransform("XSLStylesheet", pQE, pTmpNode, &mParamsInt, NO_PARAMS_CHAR, NO_PARAMS_NODE, sMode);
        pXStylesheetNode = pTmpNode->firstChild(&m_ibqe_nodeControl);
        delete pTmpNode;
      } else throw CannotCreateInTmpNode(this, db()->name());
    } UNWIND_EXCEPTION_END;

    //free up
    //if (sMode) MMO_FREE(sMode); //static

    UNWIND_EXCEPTION_THROW;

    return pXStylesheetNode;
  }

  void DatabaseClass::initDatabaseClassDependentProperties(const IMemoryLifetimeOwner *pMemoryLifetimeOwner) {
    XmlNodeList<DatabaseNode>::iterator iClassNode;
    XmlNodeList<DatabaseNode> *pvClassNodes;
    DatabaseClass *pClass;

    //class:* node-mask
    XmlNodeList<const IXmlBaseNode> *pAllClassNodes = allClassNodes(pMemoryLifetimeOwner);
    IXmlArea *pAreaClassNodesOnly = xmlLibrary()->factory_area(this, pAllClassNodes);

    //base classes
    pvClassNodes = m_pNode->getMultipleNodes(&m_ibqe_nodeControl, "class:*");
    for (iClassNode = pvClassNodes->begin(); iClassNode != pvClassNodes->end(); iClassNode++) {
      if (pClass = classFromClassNode(*iClassNode)) {
        m_vBaseClasses.push_back(pClass);
      } else throw UnregisteredDatabaseClass(this, (*iClassNode)->localName());
    }

    //derived classes
    //note that this harlink to the parents classes will not get noticed if the parents axis has not been implemented
    if (m_pNode->xmlLibrary()->hasAncestorsAxis()) {
      //NOTE: we are using a node-mask to prevent traversal through non-class:* nodes when using ancestors::class:*
      m_ibqe_nodeControl.maskContext()->push_back(pAreaClassNodesOnly); {
        pvClassNodes = m_pNode->getMultipleNodes(&m_ibqe_nodeControl, "ancestors::class:*");
      } m_ibqe_nodeControl.maskContext()->pop_back();
    } else {
      NOT_CURRENTLY_USED("");
      Debug::report("Database Doc does not support the parents axis. Class inheritance limited to direct node", rtWarning);
      pvClassNodes = m_pNode->getMultipleNodes(&m_ibqe_nodeControl, "parent::class:*");
    }
    for (iClassNode = pvClassNodes->begin(); iClassNode != pvClassNodes->end(); iClassNode++) {
      if (pClass = classFromClassNode(*iClassNode)) m_vDerivedClasses.push_back(pClass);
      else throw UnregisteredDatabaseClass(this, (*iClassNode)->localName());
    }
  }

  const char *DatabaseClass::toString() const {
    stringstream sOut;
    vector<DatabaseClass*>::const_iterator iDatabaseClass;
    const DatabaseClass *pDatabaseClass;

    sOut << "class:" << m_sName << " ";
    if (m_sTemplateMatchClause) sOut << "<= [" << m_sTemplateMatchClause;
    if (m_sTemplateMatchClause) sOut << "] ";
    if (m_vBaseClasses.size()) {
      sOut << "inherits (";
      for (iDatabaseClass = m_vBaseClasses.begin(); iDatabaseClass != m_vBaseClasses.end(); iDatabaseClass++) {
        pDatabaseClass = *iDatabaseClass;
        if (iDatabaseClass != m_vBaseClasses.begin()) sOut << ",";
        if (pDatabaseClass && pDatabaseClass->name()) sOut << pDatabaseClass->name();
        else sOut << "<0x0 class or name!>";
      }
      sOut << ")";
    }
    if (m_vDerivedClasses.size()) {
      sOut << " " << m_vDerivedClasses.size() << " derived classes";
    }

    return MM_STRDUP(sOut.str().c_str());
  }

  XmlNodeList<const IXmlBaseNode> *DatabaseClass::allClassNodes(const IMemoryLifetimeOwner *pMemoryLifetimeOwner) {
    XmlNodeList<const IXmlBaseNode> *pClassNodes = new XmlNodeList<const IXmlBaseNode>(pMemoryLifetimeOwner);
    vector<DatabaseClass*> *pClasses = all();

    vector<DatabaseClass*>::const_iterator iDatabaseClass;

    for (iDatabaseClass = pClasses->begin(); iDatabaseClass != pClasses->end(); iDatabaseClass++) {
      pClassNodes->push_back((*iDatabaseClass)->dbNode()->node_const());
    }

    //free up
    delete pClasses;

    return pClassNodes;
  }

  vector<DatabaseClass*> *DatabaseClass::all() {
    vector<DatabaseClass*> *pClasses = new vector<DatabaseClass*>();
    StringMap<DatabaseClass*>::const_iterator iDatabaseClass;

    for (iDatabaseClass = m_mClassesByName.begin(); iDatabaseClass != m_mClassesByName.end(); iDatabaseClass++) {
      pClasses->push_back(iDatabaseClass->second);
    }

    return pClasses;
  }

  const char *DatabaseClass::toStringAll() {
    stringstream sOut;
    const char *sDatabaseClass;
    StringMap<DatabaseClass*>::const_iterator iDatabaseClass;

    sOut << "  [" << m_mClassesByName.size() << "] classes registered\n";
    for (iDatabaseClass = m_mClassesByName.begin(); iDatabaseClass != m_mClassesByName.end(); iDatabaseClass++) {
      sDatabaseClass = iDatabaseClass->second->toString();
      sOut << "    " << sDatabaseClass << "\n";
      MM_FREE(sDatabaseClass);
    }
    sOut << "\n";
    return MM_STRDUP(sOut.str().c_str());
  }

  void DatabaseClass::serialise(const IXmlQueryEnvironment *pQE, IXmlBaseNode *pOutputNode) {
    IXmlBaseNode *pClasses, *pClass;
    StringMap<DatabaseClass*>::const_iterator iDatabaseClass;
    DatabaseClass *pDatabaseClass;

    pClasses = pOutputNode->createChildElement(pQE, "classes");
    for (iDatabaseClass = m_mClassesByName.begin(); iDatabaseClass != m_mClassesByName.end(); iDatabaseClass++) {
      pDatabaseClass  = iDatabaseClass->second;
      if (pClass = pClasses->createChildElement(pQE, pDatabaseClass->m_sName)) {
        pClass->setAttribute(pQE, "elements",      pDatabaseClass->m_sNodeTestXPath);
        pClass->setAttribute(pQE, "class-name",    pDatabaseClass->m_sName);
        delete pClass;
      }
    }

    //free up
    if (pClasses) delete pClasses;
  }

  vector<DatabaseClass*> *DatabaseClass::allDerivedUnique(const bool bIncludeAllElementsClasses) const {
    vector<DatabaseClass*> *pvAllDerived = new vector<DatabaseClass*>();
    allDerivedUnique_recursive(pvAllDerived, bIncludeAllElementsClasses);
    return pvAllDerived;
  }

  vector<DatabaseClass*> *DatabaseClass::allBasesUnique(const bool bIncludeAllElementsClasses) const {
    vector<DatabaseClass*> *pvAllBases = new vector<DatabaseClass*>();
    allBasesUnique_recursive(pvAllBases, bIncludeAllElementsClasses);
    return pvAllBases;
  }

  void DatabaseClass::allDerivedUnique_recursive(vector<DatabaseClass*> *pvAllDerived, const bool bIncludeAllElementsClasses) const {
    vector<DatabaseClass*>::const_iterator iClass;
    DatabaseClass *pDatabaseClass;

    for (iClass = m_vDerivedClasses.begin(); iClass != m_vDerivedClasses.end(); iClass++) {
      pDatabaseClass = *iClass;
      if (bIncludeAllElementsClasses || !pDatabaseClass->isAllNamespaceElements()) {
        if (find(pvAllDerived->begin(), pvAllDerived->end(), pDatabaseClass) == pvAllDerived->end()) {
          pvAllDerived->push_back(pDatabaseClass);
          pDatabaseClass->allDerivedUnique_recursive(pvAllDerived);
        }
      }
    }
  }

  void DatabaseClass::allDerivedUnique_recursive(XmlNodeList<const IXmlBaseNode> *pvAllDerived, const bool bIncludeAllElementsClasses) const {
    vector<DatabaseClass*> vAllDerived;
    vector<DatabaseClass*>::const_iterator iClass;

    allDerivedUnique_recursive(&vAllDerived, bIncludeAllElementsClasses);
    for (iClass = vAllDerived.begin(); iClass != vAllDerived.end(); iClass++) {
      pvAllDerived->push_back((*iClass)->dbNode()->node_const());
    }
  }
  
  void DatabaseClass::allBasesUnique_recursive(vector<DatabaseClass*> *pvAllBases, const bool bIncludeAllElementsClasses) const {
    vector<DatabaseClass*>::const_iterator iClass;
    DatabaseClass *pDatabaseClass;

    for (iClass = m_vBaseClasses.begin(); iClass != m_vBaseClasses.end(); iClass++) {
      pDatabaseClass = *iClass;
      if (bIncludeAllElementsClasses || !pDatabaseClass->isAllNamespaceElements()) {
        if (find(pvAllBases->begin(), pvAllBases->end(), pDatabaseClass) == pvAllBases->end()) {
          pvAllBases->push_back(pDatabaseClass);
          pDatabaseClass->allBasesUnique_recursive(pvAllBases);
        }
      }
    }
  }

  void DatabaseClass::allBasesUnique_recursive(XmlNodeList<const IXmlBaseNode> *pvAllBases, const bool bIncludeAllElementsClasses) const {
    vector<DatabaseClass*> vAllBases;
    vector<DatabaseClass*>::const_iterator iClass;

    allBasesUnique_recursive(&vAllBases, bIncludeAllElementsClasses);
    for (iClass = vAllBases.begin(); iClass != vAllBases.end(); iClass++) {
      pvAllBases->push_back((*iClass)->dbNode()->node_const());
    }
  }

  bool DatabaseClass::isBaseOf(const DatabaseClass *pSuper) const {
    vector<DatabaseClass*> *pvAllBases = pSuper->allBasesUnique();
    bool bIsBaseOf = (find(pvAllBases->begin(), pvAllBases->end(), this) != pvAllBases->end());
    delete pvAllBases;
    return bIsBaseOf;
  }

  bool DatabaseClass::isDerivedFrom(const DatabaseClass *pBase) const {
    return pBase->isBaseOf(this);
  }

  vector<DatabaseClass*> *DatabaseClass::classesFromLocalName(const char *sLocalName, const bool bIncludeBases, const bool bIncludeAllElementsClasses) {
    assert(sLocalName);

    vector<DatabaseClass*> *pvDerivedClasses = new vector<DatabaseClass*>();
    StringMap<DatabaseClass*>::const_iterator iNewClass;
    DatabaseClass *pNewClass;

    for (iNewClass = m_mClassesByName.begin(); iNewClass != m_mClassesByName.end(); iNewClass++) {
      pNewClass = iNewClass->second;
      if (pNewClass->gimme(sLocalName)) pNewClass->insertInto(pvDerivedClasses, bIncludeBases, bIncludeAllElementsClasses);
    }

    return pvDerivedClasses;    
  }
  
  vector<DatabaseClass*> *DatabaseClass::classesFromElement(const char *sLocalName, const char *sPrefix, const bool bIncludeBases, const bool bIncludeAllElementsClasses) {
    //different style of class lookup that:
    //CANNOT run xpath on the element name
    //so complex @elements will fail to resolve
    //server frees result DO NOT free! returns direct pointer in to server map
    //this returns only leaf derived classes that want the element
    //  e.g. XSLStylesheet, but NOT XSL (because it is a base of XSLStylesheet)
    assert(sLocalName);

    vector<DatabaseClass*> *pvDerivedClasses = new vector<DatabaseClass*>();
    StringMap<DatabaseClass*>::const_iterator iNewClass;
    DatabaseClass *pNewClass;

    for (iNewClass = m_mClassesByName.begin(); iNewClass != m_mClassesByName.end(); iNewClass++) {
      pNewClass = iNewClass->second;
      if (pNewClass->gimme(sLocalName, sPrefix)) pNewClass->insertInto(pvDerivedClasses, bIncludeBases, bIncludeAllElementsClasses);
    }

    return pvDerivedClasses;    
  }

  vector<DatabaseClass*> *DatabaseClass::classesFromElement(const DatabaseNode *pNode, const bool bIncludeBases, const bool bIncludeAllElementsClasses) {
    //STATIC
    assert(pNode);
    return classesFromElement(pNode->node_const(), bIncludeBases, bIncludeAllElementsClasses);
  }

  vector<DatabaseClass*> *DatabaseClass::classesFromElement(const IXmlBaseNode *pSelectNode, const bool bIncludeBases, const bool bIncludeAllElementsClasses) {
    //STATIC
    //@bIncludeAllElementsClasses = true: include * isAllNamespaceElements() classes, e.g. repository:*
   assert(pSelectNode);

    vector<DatabaseClass*> *pvDerivedClasses = new vector<DatabaseClass*>();
    StringMap<DatabaseClass*>::const_iterator iNewClass;
    DatabaseClass *pNewClass;

    if (pSelectNode->isNodeElement()) {
      //classes on this element, e.g. object:User
      for (iNewClass = m_mClassesByName.begin(); iNewClass != m_mClassesByName.end(); iNewClass++) {
        pNewClass = iNewClass->second;
        if (pNewClass->gimme(pSelectNode)) pNewClass->insertInto(pvDerivedClasses, bIncludeBases, bIncludeAllElementsClasses);
      }
    }

    return pvDerivedClasses;
  }
  
  bool DatabaseClass::insertInto(vector<DatabaseClass*> *pvDerivedClasses, const bool bIncludeBases, const bool bIncludeAllElementsClasses) {
    bool bInsert = true;
    vector<DatabaseClass*>::iterator iExistingClass;
    vector<DatabaseClass*>::const_iterator iBaseClass;
    vector<DatabaseClass*> *pAllBaseClasses = 0;
    DatabaseClass *pExistingClass;
    
    if (!bIncludeAllElementsClasses && isAllNamespaceElements()) bInsert = false;
    else {
      if (bIncludeBases) {
        //-------------------- insert classes AND all their bases
        allBasesUnique_recursive(pvDerivedClasses, bIncludeAllElementsClasses);
        if (find(pvDerivedClasses->begin(), pvDerivedClasses->end(), this) != pvDerivedClasses->end()) bInsert = false;
      } else {
        //-------------------- insert / replace with only the most derived classes
        //e.g. replace all Class__Overlay and Class__HTML with one Class__Frame
        //pExistingClass is in the return
        //pNewClass wants this element
        //  e.g. object:User => Class__User
        iExistingClass = pvDerivedClasses->end();
        while (iExistingClass != pvDerivedClasses->begin()) {
          pExistingClass = *--iExistingClass;
          
          //if the new class is a base of an already returning class then ignore the new class
          //by returing a "found" iterator and thus preventing addition
          if (isBaseOf(pExistingClass)) {
            bInsert = false;
            break;
          }
          
          //if the new Class is a derived of an existing then replace it
          //and continue looking for other bases
          if (isDerivedFrom(pExistingClass)) {
            if (bInsert) {
              *iExistingClass = this; //REPLACEMENT
              bInsert = false;
            } else {
              //remove! because already replaced
              pvDerivedClasses->erase(iExistingClass);
            }
          }
        }
      }
    
      //not found, so insert
      if (bInsert) pvDerivedClasses->push_back(this);
    }
    
    //free up
    if (pAllBaseClasses) delete pAllBaseClasses;
    
    return bInsert;
  }
  
  bool DatabaseClass::gimme(const char *sLocalName) const {
    //match only on local-name
    //does not include all-elements * classes
    bool bMatch = false;
    StringVector::const_iterator iMatchLocalName;
    const char *sMatchLocalName;
    
    for (iMatchLocalName = m_vElements.begin(); !bMatch && iMatchLocalName != m_vElements.end(); iMatchLocalName++) {
      sMatchLocalName = *iMatchLocalName;
      bMatch = _STREQUAL(sMatchLocalName, "*") || _STREQUAL(sMatchLocalName, sLocalName);
    }

    //free up
    //if (sMatchLocalName) MMO_FREE(sMatchLocalName); //pointer in to vector
    
    return bMatch;
  }
  
  bool DatabaseClass::gimme(const char *sLocalName, const char *sPrefix) const {
    //simple element QNames only
    //TODO: enable namespace matches, e.g. class:*
    bool bMatch = false;
    StringVector::const_iterator iMatchLocalName;
    const char *sMatchLocalName;
    
    //m_vElements can be empty for abstract classes
    //_STREQUAL returns true if both inputs are NULL
    if (_STREQUAL(sPrefix, m_sNamespacePrefix)) {
      for (iMatchLocalName = m_vElements.begin(); !bMatch && iMatchLocalName != m_vElements.end(); iMatchLocalName++) {
        sMatchLocalName = *iMatchLocalName;
        bMatch          = _STREQUAL(sMatchLocalName, "*") || _STREQUAL(sMatchLocalName, sLocalName);
      }
    }
    
    //free up
    //if (sMatchLocalName) MMO_FREE(sMatchLocalName); //pointer in to vector
    
    return bMatch;
  }
    
  bool DatabaseClass::gimme(const IXmlBaseNode *pNode) const {
    //general element => governing class
    bool bMatch = false;
    const IXmlBaseNode *pNodeTest   = 0;

    if (hasElements()) {     //m_vElements  >  0
      if (needsFullEval()) { //m_sPredicate != NULL
        //------------------ predicate complex xpath match
        //runs on the self:: axis
        m_ibqe_nodeControl.xpathProcessingContext()->disable();
        pNodeTest = pNode->getSingleNode(&m_ibqe_nodeControl, m_sNodeTestXPath);
        m_ibqe_nodeControl.xpathProcessingContext()->enable();
        bMatch = (pNodeTest != NULL);
      } else {
        //------------------ simple element QNames
        //xsd:attribute|xsd:element|...
        //css:* will match all css namespace elements
        bMatch = gimme(pNode->localName(NO_DUPLICATE), pNode->standardPrefix());
      }
    }

    //free up
    if (pNodeTest)  delete pNodeTest;

    return bMatch;
  }

  DatabaseClass *DatabaseClass::classFromClassNode(const DatabaseNode *pDBClassNode) {
    //class:* => Class
    return classFromClassNode(pDBClassNode->node_const());
  }

  DatabaseClass *DatabaseClass::classFromClassNode(const IXmlBaseNode *pClassNode) {
    //class:* => Class
    return classFromName(pClassNode->localName(NO_DUPLICATE));
  }

  DatabaseClass *DatabaseClass::classFromName(const char *sClassName) {
    //(~|Class__|CSS__)?<name> => Class
    StringMap<DatabaseClass*>::const_iterator iClass;
    DatabaseClass *pDatabaseClass = 0;

    if (sClassName) {
      if      (sClassName[0] == '~')                 sClassName += 1;
      else if (_STRNEQUAL(sClassName, "Class__", 7)) sClassName += 7;
      else if (_STRNEQUAL(sClassName, "CSS__", 5))   sClassName += 5;
      else if (_STRNEQUAL(sClassName, "class:", 6))  sClassName += 6;
      iClass = m_mClassesByName.find(sClassName);
      if (iClass != m_mClassesByName.end()) pDatabaseClass = iClass->second;
    }

    return pDatabaseClass;
  }
}
