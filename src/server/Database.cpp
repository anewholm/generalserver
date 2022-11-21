//platform agnostic file
#include "Database.h"

//full definitions of the ahead declerations in Database.h
#include "types.h"
#include "DatabaseNode.h"
#include "IXml/IXmlSecurityContext.h"
#include "QueryEnvironment/QueryEnvironment.h"
#include "DatabaseClass.h"
#include "QueryEnvironment/BrowserQueryEnvironment.h"
#include "QueryEnvironment/TriggerContext.h"
#include "QueryEnvironment/MaskContext.h"
#include "IXml/IXmlNode.h"
#include "IXml/IXslDoc.h"
#include "IXml/IXslNode.h"
#include "IXml/IXmlBaseNode.h"
#include "IXml/IXmlBaseDoc.h"
#include "IXml/IXmlArea.h"
#include "TXml.h"
#include "RegularX.h"
#include "QueryEnvironment/DatabaseNodeUserStoreSecurityContext.h"
#include "Server.h"

#include "Utilities/strtools.h"
#include "Utilities/container.c"

#include "LibXmlBaseNode.h"
#include "LibXslDoc.h"

#include <time.h>
using namespace std;

#ifdef WITHOUT_XSL_EXTENSIONS
#define DISABLE_DATABASE_XSL_EXTENSIONS return 0
#else
#define DISABLE_DATABASE_XSL_EXTENSIONS
#endif

namespace general_server {
  StringMap<IXslModule::XslModuleCommandDetails>  Database::m_XSLCommands; //XSL extensions
  StringMap<IXslModule::XslModuleFunctionDetails> Database::m_XSLFunctions;

  pthread_mutex_t Database::m_mWriteListenerManagement = PTHREAD_MUTEX_INITIALIZER;
  void Database::writeListenerManagementLock()   {pthread_mutex_lock(  &m_mWriteListenerManagement);}
  void Database::writeListenerManagementUnlock() {pthread_mutex_unlock(&m_mWriteListenerManagement);}
  
  Database::Database(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlLibrary *pLib, const char *sAlias, const char *sRepositorySource, const char *sXPathToStore):
    MemoryLifetimeOwner(pMemoryLifetimeOwner),
    TXmlProcessor(pMemoryLifetimeOwner),
    m_debug(this, pLib),
    m_eXSLTRegExp(this),
    m_eXSLTStrings(this),
    m_xSLTCustomUtilities(this),
    m_xJavaScript(this),
    m_pLib(pLib),
    m_sAlias(MM_STRDUP(sAlias)),
    m_sRepositorySource(MM_STRDUP(sRepositorySource)),
    m_pRepositoryReader(0),
    m_pIBQE_dbAdministrator(0),
    m_vEvents(this),

    m_bStartingUp(true),
    m_pUserStore(0),
    m_pTransactionDisplayNode(0),
    m_bLookedForTransactionDisplayNode(false),
    m_pTmpNode(0),
    m_bLookedForTmpNode(false)
  {
    //caller frees their sRepositorySource and sAlias, we take a copy
    assert(m_pLib); //server is never optional
    assert(m_sRepositorySource);

    IXmlBaseNode *pRootNode    = 0;
    DatabaseNode *pRootDBBode  = 0,
                 *pClassesNode = 0;
    IXmlHasNamespaceDefinitions *pRootNodeType;
    size_t iNewIDs;
    XmlNodeList<IXmlBaseNode> vNewIDs(this);
    XmlNodeList<IXmlBaseNode>::iterator iNewID;
    IXmlBaseNode *pTmpNode    = 0;
    TXml::commitStatus cs;
    TXml::commitStyle  ct;

    //------------------------------------------ XslModuleManager extensions for TXml
    //important that these are the same during startup
    //built-in to the concept of Database
    if (!xmlLibrary()->hasEXSLTRegExp()) registerXslModule(eXSLTRegExp());
    registerXslModule(eXSLTStrings());
    registerXslModule(xJavaScript());
    registerXslModule(xSLTCustomUtilities());
    registerXslModule(this);
    registerXslModule(&m_debug);
    //registerXslModule(m_pRepositoryReader); //registered below after repository creation

    //------------------------------------------ create repository
    //does not read anything yet:
    m_pRepositoryReader     = Repository::factory_reader(this, xmlLibrary(), m_sRepositorySource);
    //load repository string representation in to IXmlDoc
    //note that this needs to work even with in-complete commits
    //  because ALL incomplete commit stages require an IXmlDoc to complete
    //streamLoadXML(Repository)
    //  cacheAllPrefixedNamespaceDefinitionsForXPath();
    //  recodeLineNumbers();  //line-number => xml:id
    //  translateStructure()  //xml:hardlink => hardlink
    m_pDoc = xmlLibrary()->factory_document(this, m_sAlias);
    m_pDoc->streamLoadXML(m_pRepositoryReader);
    m_pIBQE_dbAdministrator = new DatabaseAdminQueryEnvironment(this, m_pDoc, this);
    if (!m_pDoc || !m_pDoc->validate()) throw DocumentNotValid(this, MM_STRDUP(m_sAlias));
    pRootNode   = m_pDoc->rootNode(m_pIBQE_dbAdministrator);
    if (!pRootNode) throw NoRootNodeFound(this, MM_STRDUP(m_sAlias));
    pRootDBBode = factory_node(pRootNode);

#ifdef WITH_DATABASE_NAMESPACEROOT
    //move all prefixed namespaces to the root node
    //TODO: cannot do this currently until xmlNodePtr->other_parents are available to sync nsDef pointers
    if (pRootNode) {
      if (pRootNodeType = pRootNode->queryInterface((IXmlHasNamespaceDefinitions*) 0)) {
        Debug::report("setting namespace root");
        pRootNodeType->setNamespaceRoot();
      } else throw InterfaceNotSupported(this, MM_STRDUP("IXmlHasNamespaceDefinitions*"));
    }
#endif

    //setting the DatabaseNode of this readable repository allows direct commits
    m_pRepositoryReader->setDatabaseNode(rootNode(m_pIBQE_dbAdministrator));
    registerXslModule(m_pRepositoryReader);

    //repository:tmp
    createTmpArea(); 

    //------------------------------------------ SECURITY
    //default: /object:Server/repository:users
    if (!sXPathToStore || !*sXPathToStore) throw UserStoreNotFound(this, MM_STRDUP("<blank store xpath sent through>"));
    m_pUserStore = m_pDoc->getSingleNode(m_pIBQE_dbAdministrator, sXPathToStore);
    if (!m_pUserStore) throw UserStoreNotFound(this, MM_STRDUP(sXPathToStore));

    //------------------------------------------ classes
    //loading classes early because transactions (which include transforms) may rely on Class knowledge
    //class:* are restricted to /repository:classes area so that "installations" can happen
    //via hardlink / copy / hardlink from triggered external servers
    if (pClassesNode = pRootDBBode->getSingleNode(m_pIBQE_dbAdministrator, "repository:classes")) {
      DatabaseClass::loadClasses(m_pIBQE_dbAdministrator, pClassesNode);
    }

    //------------------------------------------ call the virtual transaction applier
    //this can only be done here when our derived TXmlProcessor has constructed its document(s) and repository(s)
#ifdef WITH_DATABASE_READONLY
    Debug::report("development mode: ignoring startup transactions");
#else
    applyStartupTransactions(m_pDoc); //virtual
#endif

    //------------------------------------------ check for previous commits and try to complete them
    //TODO: need to remember commit style also, i.e. an interrupted full commit will repeat as selective and do nothing
    m_pRepositoryReader->commitGetPersistentStatus(&cs, &ct);
    if (cs != TXml::commitFinished) {
      Debug::report("Previous unfinished commit at [%s] stage, re-complete...", getCommitStatusName(cs));
      commit(cs);
    }

    //------------------------------------------ check / update primary index
    //this only really happens here with new filesystem loaded data
    //normal operation will keep the xml:ids up-to-date
    if (pRootNode) {
      //xmlIDMainIndexDocument() does not create TXml
      //thus a selective TXml commit will not bring up anything naturally
      iNewIDs = xmlIDMainIndexDocument(pRootNode, vNewIDs);
      //TODO: need to do a selective TXml commit here if there are only a few...
      //  we would need to make TXml for the changes after we have spotted how many there are
      if (iNewIDs) Debug::report("[%s] new IDs assigned to elements", iNewIDs);
      if (iNewIDs <= 10) {
        for (iNewID = vNewIDs.begin(); iNewID != vNewIDs.end(); iNewID++) {
          Debug::report("  [%s]", (*iNewID)->uniqueXPathToNode(m_pIBQE_dbAdministrator, NO_BASE_NODE, WARN_IF_TRANSIENT));
        }
      }

      //commit the new IDs
      if (iNewIDs) {
        if (iNewIDs > 2000) {
          //probably a new database without IDs
          //re-write whole database at once
          commit(TXml::beginCommit, TXml::full);
        } else {
          //few new nodes, so selectively save some nodes
          //the transaction is simply useful here, rather than the only way of saving these changes
          pTmpNode = createTmpNode();
          for (iNewID = vNewIDs.begin(); iNewID != vNewIDs.end(); iNewID++) {
            TXml_TouchNode t(this, m_pIBQE_dbAdministrator, MM_STRDUP("saving an ID"), NO_TXML_BASENODE, *iNewID, TXml::saveElementOnly);
            applyTransaction(&t, pTmpNode, m_pIBQE_dbAdministrator);
          }
          //commit now, rather than at next startup
          commit(TXml::beginCommit, TXml::selective);
        }
      }
    }

    m_bStartingUp = false;

    //------------------------------------------ free up
    //if (pTmpNode)  delete pTmpNode; //temp node management is handled by the Database
    vector_element_destroy(vNewIDs);
    if (pRootDBBode)  delete pRootDBBode;
    if (pClassesNode) delete pClassesNode;
    //if (pRootNode) delete pRootNode; //released by pRootDBBode
  }

  Database::~Database() {
    if (m_sAlias)                MMO_FREE(m_sAlias);
    if (m_sRepositorySource)     MMO_FREE(m_sRepositorySource);
    if (m_pDoc)                  delete m_pDoc;
    if (m_pRepositoryReader)     delete m_pRepositoryReader;
    if (m_pIBQE_dbAdministrator) delete m_pIBQE_dbAdministrator;
    if (m_pUserStore)            delete m_pUserStore;
    if (m_pTransactionDisplayNode) delete m_pTransactionDisplayNode;
    if (m_pTmpNode)              delete m_pTmpNode;
    DatabaseClass::clearClasses(); //static
  }

  const char         *Database::name()                const {return m_sAlias;}
  const char         *Database::xsltModuleNamespace() const {return NAMESPACE_DATABASE;}
  const char         *Database::xsltModulePrefix()    const {return NAMESPACE_DATABASE_ALIAS;}
  const IXmlLibrary  *Database::xmlLibrary()          const {return m_pLib;}
  const Repository   *Database::repository()          const {return m_pRepositoryReader;}

  const StringMap<IXslModule::XslModuleCommandDetails> *Database::xslCommands() const {
    //Request is also an XslModule
    //static lazy cached name -> function lookups
    if (!m_XSLCommands.size()) {
      m_XSLCommands.insert("move-child",           XMC(Database::xslCommand_transaction));
      m_XSLCommands.insert("copy-child",           XMC(Database::xslCommand_transaction));
      m_XSLCommands.insert("merge-node",           XMC(Database::xslCommand_transaction));
      m_XSLCommands.insert("deviate-node",         XMC(Database::xslCommand_transaction));
      m_XSLCommands.insert("hardlink-child",       XMC(Database::xslCommand_transaction));
      m_XSLCommands.insert("softlink-child",       XMC(Database::xslCommand_transaction));
      m_XSLCommands.insert("softlink-children",    XMC(Database::xslCommand_transactions));
      m_XSLCommands.insert("replace-node",         XMC(Database::xslCommand_transaction));
      m_XSLCommands.insert("set-attribute",        XMC(Database::xslCommand_transaction));
      m_XSLCommands.insert("set-value",            XMC(Database::xslCommand_transaction));
      m_XSLCommands.insert("remove-node",          XMC(Database::xslCommand_transaction));
      m_XSLCommands.insert("create-child-element", XMC(Database::xslCommand_transaction));

      m_XSLCommands.insert("validity-check",       XMC(Database::xslCommand_validityCheck));

      //output stream commands (no transaction required obviously)
      m_XSLCommands.insert("query",                XMC(Database::xslCommand_query));
      m_XSLCommands.insert("transform",            XMC(Database::xslCommand_transform));
      m_XSLCommands.insert("with-node-mask",       XMC(Database::xslCommand_withNodeMask));
      m_XSLCommands.insert("without-node-mask",    XMC(Database::xslCommand_withoutNodeMask));
      m_XSLCommands.insert("hardlink-info",        XMC(Database::xslCommand_hardlinkInfo));
      m_XSLCommands.insert("deviation-info",       XMC(Database::xslCommand_deviationInfo));
      m_XSLCommands.insert("xpath-to-node",        XMC(Database::xslCommand_xpathToNode));
      m_XSLCommands.insert("differences",          XMC(Database::xslCommand_differences));

      m_XSLCommands.insert("wait-for-single-write-event", XMC(Database::xslCommand_waitForSingleWriteEvent));
      m_XSLCommands.insert("select-input-node",    XMC(Database::xslCommand_selectInputNode));

      m_XSLCommands.insert("set-security-context", XMC(Database::xslCommand_setSecurityContext));
      m_XSLCommands.insert("clear-security-context", XMC(Database::xslCommand_clearSecurityContext));
    }
    return &m_XSLCommands;
  }

  const StringMap<IXslModule::XslModuleFunctionDetails> *Database::xslFunctions() const {
    //Request is also an XslModule
    //static lazy cached name -> function lookups
    if (!m_XSLFunctions.size()) {
      m_XSLFunctions.insert("validity-check",        XMF(Database::xslFunction_validityCheck));
      m_XSLFunctions.insert("is-logged-in",          XMF(Database::xslFunction_isLoggedIn));

      //node informational functions
      m_XSLFunctions.insert("fully-qualified-name",  XMF(Database::xslFunction_fullyQualifiedName));
      m_XSLFunctions.insert("xpath-to-node",         XMF(Database::xslFunction_xpathToNode));
      m_XSLFunctions.insert("guid",                  XMF(Database::xslFunction_UUID));

      //set functions
      //these mirror EXSLT set because it is shit and banned
      //EXSLT set compares based on char value of each node!
      //because actually XSLT does with the "=" operation as well
      m_XSLFunctions.insert("distinct",              XMF(Database::xslFunction_distinct));
      m_XSLFunctions.insert("difference",            XMF(Database::xslFunction_difference));
      m_XSLFunctions.insert("has-same-node",         XMF(Database::xslFunction_hasSameNode));
      m_XSLFunctions.insert("intersection",          XMF(Database::xslFunction_intersection));
      m_XSLFunctions.insert("document-order",        XMF(Database::xslFunction_documentOrder));
      m_XSLFunctions.insert("similarity",            XMF(Database::xslFunction_similarity));

      //useful functions for database:triggers
      m_XSLFunctions.insert("replace-children-transient", XMF(Database::xslFunction_replaceChildrenTransient));
      m_XSLFunctions.insert("rx",                  XMF(Database::xslFunction_rx));
      m_XSLFunctions.insert("parse-xml",             XMF(Database::xslFunction_parseXML));
      m_XSLFunctions.insert("parse-html",            XMF(Database::xslFunction_parseHTML));
      m_XSLFunctions.insert("copy-of",               XMF(Database::xslFunction_copyOf));
      m_XSLFunctions.insert("set-namespace",         XMF(Database::xslFunction_setNamespace));
      m_XSLFunctions.insert("transform",             XMF(Database::xslFunction_transform));
      m_XSLFunctions.insert("node-mask",             XMF(Database::xslFunction_nodeMask));

      //direct TXml Database functions
      m_XSLFunctions.insert("move-child",            XMF(Database::xslFunction_transaction));
      m_XSLFunctions.insert("copy-child",            XMF(Database::xslFunction_transaction));
      m_XSLFunctions.insert("deviate-node",          XMF(Database::xslFunction_transaction));
      m_XSLFunctions.insert("hardlink-child",        XMF(Database::xslFunction_transaction));
      m_XSLFunctions.insert("softlink-child",        XMF(Database::xslFunction_transaction));
      m_XSLFunctions.insert("replace-node",          XMF(Database::xslFunction_transaction));
      m_XSLFunctions.insert("set-attribute",         XMF(Database::xslFunction_transaction));
      m_XSLFunctions.insert("set-value",             XMF(Database::xslFunction_transaction));
      m_XSLFunctions.insert("remove-node",           XMF(Database::xslFunction_transaction));
      m_XSLFunctions.insert("create-child-element",  XMF(Database::xslFunction_transaction));
      //TXml useful
      m_XSLFunctions.insert("extension-element-prefixes", XMF(Database::xslFunction_extensionElementPrefixes));

      m_XSLFunctions.insert("is-hard-linked",        XMF(Database::xslFunction_isHardlinked));
      m_XSLFunctions.insert("is-hard-link",          XMF(Database::xslFunction_isHardlink));
      m_XSLFunctions.insert("is-original-hard-link", XMF(Database::xslFunction_isOriginalHardlink));
      m_XSLFunctions.insert("is-soft-link",          XMF(Database::xslFunction_isSoftlink));
      m_XSLFunctions.insert("is-registered",         XMF(Database::xslFunction_isRegistered));
      m_XSLFunctions.insert("is-transient",          XMF(Database::xslFunction_isTransient));

      m_XSLFunctions.insert("is-deviant",            XMF(Database::xslFunction_isDeviant));
      m_XSLFunctions.insert("has-descendant-deviants", XMF(Database::xslFunction_hasDescendantDeviants));
      m_XSLFunctions.insert("has-deviants",          XMF(Database::xslFunction_hasDeviants));

      m_XSLFunctions.insert("is-node-element",       XMF(Database::xslFunction_isNodeElement));
      m_XSLFunctions.insert("is-node-document",      XMF(Database::xslFunction_isNodeDocument));
      m_XSLFunctions.insert("is-node-attribute",     XMF(Database::xslFunction_isNodeAttribute));
      m_XSLFunctions.insert("is-node-text",          XMF(Database::xslFunction_isNodeText));
      m_XSLFunctions.insert("is-processing-instruction", XMF(Database::xslFunction_isProcessingInstruction));
      m_XSLFunctions.insert("is-CDATA-section",      XMF(Database::xslFunction_isCDATASection));

      m_XSLFunctions.insert("is-undefined",          XMF(Database::xslFunction_isUndefined));
      m_XSLFunctions.insert("is-node-set",           XMF(Database::xslFunction_isNodeSet));
      m_XSLFunctions.insert("is-boolean",            XMF(Database::xslFunction_isBoolean));
      m_XSLFunctions.insert("is-number",             XMF(Database::xslFunction_isNumber));
      m_XSLFunctions.insert("is-string",             XMF(Database::xslFunction_isString));
      m_XSLFunctions.insert("is-point",              XMF(Database::xslFunction_isPoint));
      m_XSLFunctions.insert("is-range",              XMF(Database::xslFunction_isRange));
      m_XSLFunctions.insert("is-location-set",       XMF(Database::xslFunction_isLocationSet));
      m_XSLFunctions.insert("is-users",              XMF(Database::xslFunction_isUsers));
      m_XSLFunctions.insert("is-xslt-tree",          XMF(Database::xslFunction_isXSLTTree));
      m_XSLFunctions.insert("not-node-set",          XMF(Database::xslFunction_notNodeSet));

      //class and inheritance
      m_XSLFunctions.insert("class",                 XMF(Database::xslFunction_class));
      m_XSLFunctions.insert("classes",               XMF(Database::xslFunction_classes));
      m_XSLFunctions.insert("base-class-count",      XMF(Database::xslFunction_baseClassCount));
      m_XSLFunctions.insert("base-classes-all",      XMF(Database::xslFunction_baseClassesAll));
      m_XSLFunctions.insert("derived-classes",       XMF(Database::xslFunction_derivedClasses));
      m_XSLFunctions.insert("derived-template-match",  XMF(Database::xslFunction_derivedTemplateMatchClause));
      m_XSLFunctions.insert("class-template-match",  XMF(Database::xslFunction_classTemplateMatchClause));
      m_XSLFunctions.insert("class-xschema-node-mask", XMF(Database::xslFunction_classXSchemaNodeMask));
      m_XSLFunctions.insert("dynamic-stylesheet",    XMF(Database::xslFunction_dynamicStylesheet));
      m_XSLFunctions.insert("is-object",             XMF(Database::xslFunction_isObject));
      m_XSLFunctions.insert("container-object",      XMF(Database::xslFunction_containerObject));
      m_XSLFunctions.insert("descendant-objects",    XMF(Database::xslFunction_descendantObjects));

      //tech
      m_XSLFunctions.insert("class-xschema",         XMF(Database::xslFunction_classXSchema));
      m_XSLFunctions.insert("class-xstylesheet",     XMF(Database::xslFunction_classXStylesheet));
      //m_XSLFunctions.insert("class-xjavascript",     XMF(Database::xslFunction_classXStylesheet));
      //m_XSLFunctions.insert("class-xcss",            XMF(Database::xslFunction_classXStylesheet));
    }
    return &m_XSLFunctions;
  }
  
  IXmlBaseNode *Database::xslCommand_transactions(const IXmlQueryEnvironment *pQE, const IXslCommandNode *pCommandNode, const IXmlBaseNode *pSourceNode, IXmlBaseNode *pOutputNode) {
    //pseudo function that iterates through the node-set
    //creating a transaction for each
    TXml::transactionResult tr = TXml::transactionFailure;
    const IXmlBaseNode *pCommandNodeType = 0;
    IXmlBaseNode *pSerialisationNode;
    const XmlNodeList<const IXmlBaseNode> *pSelectNodes = 0;
    XmlNodeList<const IXmlBaseNode>::const_iterator iSelectNode;
    const IXmlBaseNode *pSelectNode;
    TXml *t;
    
    if (pCommandNodeType = dynamic_cast<const IXmlBaseNode*>(pCommandNode)) {
      if (pSelectNodes = pCommandNodeType->attributeValueNodes(pQE, "select")) {
        for (iSelectNode = pSelectNodes->begin(); iSelectNode != pSelectNodes->end(); iSelectNode++) {
          //we could use DatabaseNode here
          //as it automatically uses TXml
          //however, its easier to get TXml to read its own spec XML directly
          if (t = TXml::factory(
              this, pQE, this, pCommandNode->queryInterface((IXmlBaseNode*) 0), 
              *iSelectNode, 
              REQUIRE_PLURAL //NOT_CURRENTLY_USED("just to inform the TXml that it is part of a sequence")
          )) {
            if (pSerialisationNode = createTmpNode()) {
              tr = applyTransaction(t, pSerialisationNode, pQE);
              if (tr == TXml::transactionFailure) {
                //TODO: TXml::transactionFailure
              }
            }

            //free up
            delete t;
            //if (pSerialisationNode) delete pSerialisationNode; //tmp nodes are handled by the Database
          }
        }
      } else throw AttributeRequired(this, MM_STRDUP("select"), pCommandNodeType->localName());
    } //TODO: dynamic_cast = 0?
    
    //free up
    //if (pSelectNodes) pSelectNodes->vector_element_destroy(); //TXml will do this
    if (pSelectNodes) delete pSelectNodes;

    return 0;
  }

  IXmlBaseNode *Database::xslCommand_waitForSingleWriteEvent(const IXmlQueryEnvironment *pQE, const IXslCommandNode *pCommandNode, const IXmlBaseNode *pSourceNode, IXmlBaseNode *pOutputNode) {
    static const char *sElementSignature = "<database:wait-for-single-write-event select=\"node\" [node-mask=\"\" last-known-event-id=\"\"] />";
    const IXmlBaseNode *pCommandNodeType = 0;
    IXmlArea     *pArea             = 0;
    IXmlNodeMask *pNodeMask         = 0;
    IXmlBaseNode *pChangedNode      = 0;
    const XmlNodeList<const IXmlBaseNode> *pSelectNodes = 0;
    const IXmlBaseNode *pStylesheetNode = 0,
                 *pResultNode       = 0;
    const IXslStylesheetNode *pStylesheetNodeType = 0;
    const char *sWithMode           = 0;
    const char *sUniqueXPathToNode  = 0;
    size_t iEventID, iLastKnownEventID;
    StringMap<const size_t> mParamsInt;


    UNWIND_EXCEPTION_BEGIN {
      if (pCommandNodeType = dynamic_cast<const IXmlBaseNode*>(pCommandNode)) {
        if (pSelectNodes = pCommandNodeType->attributeValueNodes(pQE, "select")) {
          //optional
          pNodeMask         = pCommandNodeType->attributeValueNodeMask(pQE, "node-mask");           //can be zero
          iLastKnownEventID = pCommandNodeType->attributeValueInt(     pQE, "last-known-event-id"); //can be zero

          //--------------------- wait first for changes
          //if pChangedNode is zero, a timeout has occured (5 minutes)
          pArea = xmlLibrary()->factory_area(this, pSelectNodes, pQE, pNodeMask);
          do {
            iEventID = waitForSingleWriteEvent(pQE, pArea, iLastKnownEventID); //may return immediately
            //TODO: waitForSingleWriteEvent() timeout policy?
          } while (!iEventID); //1 based
          pChangedNode = m_vEvents[iEventID-1]; //1 based

          if (pStylesheetNode = pCommandNodeType->attributeValueNode(pQE, "transform")) {
            //--------------------- transform changed node to output
            sWithMode           = pCommandNodeType->attributeValueDynamic(pQE, "interface-mode"); //optional
            //will throw an Exception if not an xsl:stylesheet node
            pStylesheetNodeType = pStylesheetNode->queryInterface((const IXslStylesheetNode*) 0);
            mParamsInt.insert("gs_event_id", iEventID);
            pChangedNode->transform(
              pStylesheetNodeType,
              pQE,
              pOutputNode,
              &mParamsInt, NO_PARAMS_CHAR, NO_PARAMS_NODE,
              sWithMode
            );
          } else {
            //--------------------- copy element only to output
            pResultNode        = pOutputNode->copyChild(pQE, pChangedNode);
            sUniqueXPathToNode = pChangedNode->uniqueXPathToNode(pQE, NO_BASE_NODE, INCLUDE_TRANSIENT);
            pResultNode->setAttribute(pQE, "unique-xpath-to-node", sUniqueXPathToNode, NAMESPACE_META);
          }
        } else XPathReturnedEmptyResultSet(this, "required select nodeset missing [%s]", sElementSignature);
      }
    } UNWIND_EXCEPTION_END;

    //free up
    if (pArea)              delete pArea;
    if (pNodeMask)          delete pNodeMask;
    if (pSelectNodes)       delete pSelectNodes; //->element_destroy(); used by the Area
    if (sWithMode)          MMO_FREE(sWithMode);
    if (pStylesheetNode)    delete pStylesheetNode;
    if (pResultNode)        delete pResultNode;
    if (sUniqueXPathToNode) MMO_FREE(sUniqueXPathToNode);
    //if (pChangedNode)       delete pChangedNode; //pointer in to m_vEvents

    UNWIND_EXCEPTION_THROW;

    return 0;
  }

  IXmlBaseNode *Database::xslCommand_validityCheck(const IXmlQueryEnvironment *pQE, const IXslCommandNode *pCommandNode, const IXmlBaseNode *pSourceNode, IXmlBaseNode *pOutputNode) {
    IXmlBaseNode *pNewNode;
    const IXmlBaseNode *pCommandNodeType, *pSelectNode;
    int iErrors;

    if ( (pCommandNodeType = dynamic_cast<const IXmlBaseNode*>(pCommandNode))
      && (pSelectNode      = pCommandNodeType->attributeValueNode(pQE, "select"))
    ) {
      iErrors = pSelectNode->validityCheck("xslCommand_validityCheck");
      delete pSelectNode;
    } else {
      iErrors = m_pDoc->validityCheck("xslCommand_validityCheck");
    }

    if (iErrors) {
      pNewNode = pOutputNode->createChildElement(pQE, "validity-check");
      pNewNode->value(pQE, "validityCheck errors");
    }

    return 0;
  }

  const char *Database::xslFunction_isLoggedIn(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    //userNode() is a pointer, dont delete it
    static const char *sFunctionSignature = "database:is-logged-in()"; UNUSED(sFunctionSignature);
    return (pQE->securityContext()->isLoggedIn() != 0) ? MM_STRDUP("true") : 0;
  }

  IXmlBaseNode *Database::xslCommand_setSecurityContext(const IXmlQueryEnvironment *pQE, const IXslCommandNode *pCommandNode, const IXmlBaseNode *pSourceNode, IXmlBaseNode *pOutputNode) {
    static const char *sCommandSignature = "database:set-security-context @username @password"; UNUSED(sCommandSignature);
    assert(pQE->securityContext());
    const char *sPasswordHash            = 0;
    const IXmlBaseNode *pCommandNodeType = pCommandNode->queryInterface((IXmlBaseNode*) 0);
    const char *sUsername                = pCommandNodeType->attributeValueDynamic(pQE, "username");
    const char *sPassword                = pCommandNodeType->attributeValueDynamic(pQE, "password");

    UNWIND_EXCEPTION_BEGIN {
      sPasswordHash = sPassword;
      //may throw LoginFailed(this)
      pQE->securityContext()->login(sUsername, sPasswordHash);
    } UNWIND_EXCEPTION_END;

    //free up
    if (sUsername) MMO_FREE(sUsername);
    if (sPassword) MMO_FREE(sPassword);

    UNWIND_EXCEPTION_THROW;

    return 0;
  }

  IXmlBaseNode *Database::xslCommand_clearSecurityContext(const IXmlQueryEnvironment *pQE, const IXslCommandNode *pCommandNode, const IXmlBaseNode *pSourceNode, IXmlBaseNode *pOutputNode) {
    static const char *sCommandSignature = "database:clear-security-context"; UNUSED(sCommandSignature);
    assert(pQE->securityContext());
    
    pQE->securityContext()->logout();

    //TODO: the debug context will get used by pQE and is now pointing to nothing
    //if (pQE) pQE->debugContext(0);

    return 0;
  }

  IXmlBaseNode *Database::xslCommand_withoutNodeMask(const IXmlQueryEnvironment *pQE, const IXslCommandNode *pCommandNode, const IXmlBaseNode *pSourceNode, IXmlBaseNode *pOutputNode) {
    //a blank <database:without-node-mask>
    static const char *sCommandSignature = "database:without-node-mask [@trace @hardlink-policy]"; UNUSED(sCommandSignature);
    const IXmlBaseNode *pCommandNodeType = 0;
    IXslTransformContext *pTC            = 0;
    bool bTrace                          = false, 
         bOldEnabled                     = false;
    const char *sHardlinkPolicy          = 0;
    IXslTransformContext::iHardlinkPolicy iOldHardlinkPolicy = IXslTransformContext::includeAllHardlinks;

    UNWIND_EXCEPTION_BEGIN {
      if (pTC = pQE->transformContext()) {
        if (pCommandNodeType = pCommandNode->queryInterface((IXmlBaseNode*) 0)) {
          //all optional
          bTrace                   = pCommandNodeType->attributeValueBoolDynamicString(pQE, "trace");
          sHardlinkPolicy          = pCommandNodeType->attributeValueDynamic(pQE, "hardlink-policy");

          //we set the hardlink policy so here that the node-mask calcs are not in this new list
          if (sHardlinkPolicy && *sHardlinkPolicy) iOldHardlinkPolicy = pTC->hardlinkPolicy(sHardlinkPolicy);
          if (bTrace) pQE->setXSLTTraceFlags(IXmlLibrary::XSLT_TRACE_ALL);
          if (pQE->maskContext()) bOldEnabled = pQE->maskContext()->disable();
          { 
            //continue, synchronously, the sub-commands
            pTC->continueTransformation();
          }
          //restore previous state
          if (bOldEnabled && pQE->maskContext())   pQE->maskContext()->enable();
          if (bTrace)                              pQE->clearXSLTTraceFlags();
          if (sHardlinkPolicy && *sHardlinkPolicy) pTC->hardlinkPolicy(iOldHardlinkPolicy);
        } //will throw Interface not supported
      } else throw TransformContextRequired(this, MM_STRDUP("database:without-node-mask"));
    } UNWIND_EXCEPTION_END;

    //free up
    //if (pTC) delete pTC; //pointer in to pQE
    if (sHardlinkPolicy) MMO_FREE(sHardlinkPolicy);

    UNWIND_EXCEPTION_THROW;

    return 0;
  }

  IXmlBaseNode *Database::xslCommand_xpathToNode(const IXmlQueryEnvironment *pQE, const IXslCommandNode *pCommandNode, const IXmlBaseNode *pSourceNode, IXmlBaseNode *pOutputNode) {
    //caller frees result
    //database:xpath-to-node(node [, base-node, (bool) disable-ids])
    //  @node:
    //  @disable-ids: path, rather than id link
    //  @base-node:   start from this node (must be ancestor!)
    static const char *sCommandSignature = "database:xpath-to-node [@select, (bool) @enable-ids, @base-node]"; UNUSED(sCommandSignature);
    IXslTransformContext *pTC            = 0;
    const IXmlBaseNode *pCommandNodeType = 0;
    const char *sXPathToNode      = 0;
    const IXmlBaseNode *pNode     = 0,
                       *pBaseNode = 0;
    bool bEnableIds               = false,
         bForceBaseNode           = NO_FORCE_BASE_NODE;

    UNWIND_EXCEPTION_BEGIN {
      if (pTC = pQE->transformContext()) {
        if (pCommandNodeType = pCommandNode->queryInterface((IXmlBaseNode*) 0)) {
          //all optional
          pBaseNode         = pCommandNodeType->attributeValueNode(pQE, "base-node");
          bEnableIds        = pCommandNodeType->attributeValueBoolDynamicString(pQE, "enable-ids");
          bForceBaseNode    = pCommandNodeType->attributeValueBoolDynamicString(pQE, "force-base-node");
          pNode             = pCommandNodeType->attributeValueNode(pQE, "select");
          if (!pNode) pNode = pSourceNode;
          if (bEnableIds) Debug::warn("%s @enable-ids is true: this is not advised", MM_STRDUP(sCommandSignature));

          if (pNode)        sXPathToNode = pNode->uniqueXPathToNode(pQE, pBaseNode, INCLUDE_TRANSIENT, bForceBaseNode, bEnableIds);
          if (sXPathToNode) pOutputNode->createTextNode(pQE, sXPathToNode);
        }
      }
    } UNWIND_EXCEPTION_END;

    //free up
    if (pNode && pNode != pSourceNode) delete pNode;
    if (pBaseNode)    delete pBaseNode;
    if (sXPathToNode) MMO_FREE(sXPathToNode);

    UNWIND_EXCEPTION_THROW;

    return 0;
  }

  IXmlBaseNode *Database::xslCommand_withNodeMask(const IXmlQueryEnvironment *pQE, const IXslCommandNode *pCommandNode, const IXmlBaseNode *pSourceNode, IXmlBaseNode *pOutputNode) {
    static const char *sCommandSignature = "database:with-node-mask [@node-mask @select @trace @hardlink-policy]"; UNUSED(sCommandSignature);
    IXslTransformContext *pTC            = 0;
    const IXmlBaseNode *pCommandNodeType = 0;
    IXmlArea *pArea                      = 0;
    IXmlNodeMask *pNodeMask              = 0;
    XmlNodeList<const IXmlBaseNode> *pSelectNodes = 0;
    bool bTrace;
    const char *sHardlinkPolicy          = 0;
    IXslTransformContext::iHardlinkPolicy iOldHardlinkPolicy = IXslTransformContext::includeAllHardlinks;
    XmlNodeList<const IXmlBaseNode> *pvOldHardlinks = 0;
    bool bOldEnabled                     = false;

    UNWIND_EXCEPTION_BEGIN {
      if (pTC = pQE->transformContext()) {
        if (pCommandNodeType = pCommandNode->queryInterface((IXmlBaseNode*) 0)) {
          //all optional
          pNodeMask                = pCommandNodeType->attributeValueNodeMask(pQE, "node-mask");
          pSelectNodes             = (XmlNodeList<const IXmlBaseNode>*) pCommandNodeType->attributeValueNodes(pQE, "select");
          bTrace                   = pCommandNodeType->attributeValueBoolDynamicString(pQE, "trace");
          sHardlinkPolicy          = pCommandNodeType->attributeValueDynamic(pQE, "hardlink-policy");

          //set new state for continueTransformation...
          if (pQE->maskContext()) {
            if (pNodeMask) {
              //use the source node if we dont have a @select
              //note that the pNodeMask select will be calculated under the existing hardlinkPolicy traversal state
              if (!pSelectNodes) pSelectNodes = new XmlNodeList<const IXmlBaseNode>(pSourceNode);
              pArea = xmlLibrary()->factory_area(this, pSelectNodes, pQE, pNodeMask);
              pQE->maskContext()->push_back(pArea);
            }
            bOldEnabled = pQE->maskContext()->enable();
          }
          //we set the hardlink policy so here that the node-mask calcs are not in this new list
          if (sHardlinkPolicy && *sHardlinkPolicy) iOldHardlinkPolicy = pTC->hardlinkPolicy(sHardlinkPolicy);
          if (bTrace) {
            Debug::reportObject(pArea);
            pQE->setXSLTTraceFlags(IXmlLibrary::XSLT_TRACE_ALL);
          }
          { //continue, synchronously, the sub-commands
            pTC->continueTransformation();
          }
          //restore previous state
          if (bTrace)                              pQE->clearXSLTTraceFlags();
          if (sHardlinkPolicy && *sHardlinkPolicy) pTC->hardlinkPolicy(iOldHardlinkPolicy);
          if (pQE->maskContext() && pArea)         pQE->maskContext()->pop_back();
          if (!bOldEnabled && pQE->maskContext())  pQE->maskContext()->disable();
        }
      }
    } UNWIND_EXCEPTION_END;

    //free up
    //if (pTC) delete pTC; //pointer in to pQE
    if (pSelectNodes)    delete pSelectNodes;
    if (sHardlinkPolicy) MMO_FREE(sHardlinkPolicy);
    if (pvOldHardlinks)  delete pvOldHardlinks->element_destroy();

    UNWIND_EXCEPTION_THROW;

    return 0;
  }

  bool Database::shellCommand(const char *sCommand) const {
    return xmlLibrary()->shellCommand(sCommand, m_pDoc, m_pRepositoryReader);
  }

  bool Database::isObject(const IXmlBaseNode *pNode) const {
    const vector<DatabaseClass*> *pvClasses = DatabaseClass::classesFromElement(pNode);
    bool bIsObject = (pvClasses->size() != 0);
    delete pvClasses;
    return bIsObject;
  }

  const char *Database::xslFunction_isUndefined(  const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {return popIfType(pXCtxt, XPATH_UNDEFINED) ? MM_STRDUP("true") : 0;}
  const char *Database::xslFunction_isNodeSet(    const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {return popIfType(pXCtxt, XPATH_NODESET) ? MM_STRDUP("true") : 0;}
  const char *Database::xslFunction_isBoolean(    const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {return popIfType(pXCtxt, XPATH_BOOLEAN) ? MM_STRDUP("true") : 0;}
  const char *Database::xslFunction_isNumber(     const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {return popIfType(pXCtxt, XPATH_NUMBER) ? MM_STRDUP("true") : 0;}
  const char *Database::xslFunction_isString(     const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {return popIfType(pXCtxt, XPATH_STRING) ? MM_STRDUP("true") : 0;}
  const char *Database::xslFunction_isPoint(      const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {return popIfType(pXCtxt, XPATH_POINT) ? MM_STRDUP("true") : 0;}
  const char *Database::xslFunction_isRange(      const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {return popIfType(pXCtxt, XPATH_RANGE) ? MM_STRDUP("true") : 0;}
  const char *Database::xslFunction_isLocationSet(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {return popIfType(pXCtxt, XPATH_LOCATIONSET) ? MM_STRDUP("true") : 0;}
  const char *Database::xslFunction_isUsers(      const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {return popIfType(pXCtxt, XPATH_USERS) ? MM_STRDUP("true") : 0;}
  const char *Database::xslFunction_isXSLTTree(   const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {return popIfType(pXCtxt, XPATH_XSLT_TREE) ? MM_STRDUP("true") : 0;}
  const char *Database::xslFunction_notNodeSet(   const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes)  {return !popIfType(pXCtxt, XPATH_NODESET) ? MM_STRDUP("true") : 0;}

  bool Database::popIfType(IXslXPathFunctionContext *pXCtxt, iXPathParamType iTypeTest) const {
    assert(pXCtxt);
    bool isTypeTest = (pXCtxt->valueCount() ? pXCtxt->xtype() == iTypeTest : iTypeTest == XPATH_NODESET );
    if (pXCtxt->valueCount()) pXCtxt->popIgnoreFromXPathFunctionCallStack();
    return isTypeTest;
  }

  const char *Database::xslFunction_baseClassCount(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    //this is the count of base classes
    //  0 base classes comes first
    //  1 base class comes before
    //  3 base classes
    //thus classes with more bases come after those bases which therefore have less 
    //useful for <xsl:sort @select=database:base-class-count([classnode]) @data-type=number
    assert(pQE);
    assert(pXCtxt);
    static const char *sFunctionSignature = "database:base-class-count([classNode])"; UNUSED(sFunctionSignature);
    
    size_t iBaseClassesAllCount      = 0;
    const char *sBaseClassesAllCount = 0;
    DatabaseClass *pDatabaseClass    = 0;
    XmlNodeList<const IXmlBaseNode> *pvClassNodes = new XmlNodeList<const IXmlBaseNode>(this);
    IXmlBaseNode *pInputNode         = 0;

    //parameters (popped in reverse order)
    UNWIND_EXCEPTION_BEGIN {
      if (pXCtxt->valueCount()) {
        switch (pXCtxt->xtype()) {
          case XPATH_NODESET: {
            pInputNode = pXCtxt->popInterpretNodeFromXPathFunctionCallStack();
            break;
          }
          default: {
            throw XPathFunctionWrongArgumentType(this, MM_STRDUP("nodeset"), MM_STRDUP("unknown"));
          }
        }
      } else {
        //no parameters: use the context node as a class:* or other node
        pInputNode = pXCtxt->contextNode(pQE); //pQE as MM
      }

      //now return the base classes
      //we need to create a unique list here so that we do not infinite loop
      if (pInputNode) {
        if (pDatabaseClass = DatabaseClass::classFromClassNode(pInputNode)) {
          pDatabaseClass->allBasesUnique_recursive(pvClassNodes);
          iBaseClassesAllCount = pvClassNodes->size(); //can be zero base classes
          sBaseClassesAllCount = utoa(iBaseClassesAllCount);
        }
      }
    } UNWIND_EXCEPTION_END;
    
    //free up
    if (pvClassNodes) delete pvClassNodes; //no element_destroy() because pointers in to class
    if (pInputNode)  delete pInputNode;

    UNWIND_EXCEPTION_THROW;

    return sBaseClassesAllCount; 
  }
  
  const char *Database::xslFunction_baseClassesAll(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    //caller frees result
    //returns the base-classes class:nodes for the main class indicated by the parameter
    //  @className: this string will be used to lookup the main class
    //  @node (class:*)  this node will be used to evaluate the target main class
    //  no parameters: the context node will be used and treated as a node (class:*) definition
    static const char *sFunctionSignature = "database:base-classes-all([string className|element-node])";
    assert(pQE);
    assert(pXCtxt);

    vector<DatabaseClass*> *pDatabaseClasses = 0;
    vector<DatabaseClass*>::const_iterator iDatabaseClass;
    DatabaseClass *pDatabaseClass = 0;
    XmlNodeList<const IXmlBaseNode> *pvNewNodes = new XmlNodeList<const IXmlBaseNode>(this);
    XmlNodeList<const IXmlBaseNode> *pvNewClonedNodes = 0;
    IXmlBaseNode *pInputNode            = 0;
    const char *sClassName              = 0;

    //parameters (popped in reverse order)
    UNWIND_EXCEPTION_BEGIN {
      if (pXCtxt->valueCount()) {
        switch (pXCtxt->xtype()) {
          //------------- specific className string sent through: check map
          case XPATH_STRING: {
            sClassName = pXCtxt->popInterpretCharacterValueFromXPathFunctionCallStack();
            if (!sClassName || !*sClassName) throw XPathReturnedEmptyResultSet(this, MM_STRDUP("Input string empty or wrong type"), FUNCTION_SIGNATURE);
            else {
              pDatabaseClass = DatabaseClass::classFromName(sClassName);
              pDatabaseClasses = new vector<DatabaseClass*>();
              pDatabaseClasses->push_back(pDatabaseClass);
            }
            break;
          }
          //------------- node sent through: class:* or other node
          case XPATH_NODESET: {
            pInputNode     = pXCtxt->popInterpretNodeFromXPathFunctionCallStack();
            pDatabaseClasses = DatabaseClass::classesFromElement(pInputNode);
            break;
          }
          default: {
            throw XPathFunctionWrongArgumentType(this, MM_STRDUP("string|nodeset"), MM_STRDUP("unknown"));
          }
        }
      } else {
        //------------- no parameters: use the context node as a class:* or other node
        if (pInputNode = pXCtxt->contextNode(pQE)) { //pQE as MM
          pDatabaseClasses = DatabaseClass::classesFromElement(pInputNode);
        }
      }

      //now return the base classes
      //we need to create a unique list here so that we do not infinite loop
      if (pDatabaseClasses) {
        for (iDatabaseClass = pDatabaseClasses->begin(); iDatabaseClass != pDatabaseClasses->end(); iDatabaseClass++) {
          (*iDatabaseClass)->allBasesUnique_recursive(pvNewNodes);
        }
      } else Debug::report("DatabaseClass not found", rtWarning);
    } UNWIND_EXCEPTION_END;
    
    //we need to clone because these are pointers in to the DatabaseClass
    pvNewClonedNodes = pvNewNodes->clone_with_resources();

    //free up
    //if (pClassNode)  delete pClassNode; //server freed from map
    if (sClassName)        MMO_FREE(sClassName);
    if (pInputNode)        delete pInputNode;
    if (pDatabaseClasses)  delete pDatabaseClasses;
    if (pvNewNodes)        delete pvNewNodes; //no element_destroy() because pointers in to the DatabaseClass
    UNWIND_CONTAINER_DESTROY_AND_DELETE_IF_EXCEPTION(pvNewClonedNodes); //is clone_with_resources()

    UNWIND_EXCEPTION_THROW;

    *pNodes = pvNewClonedNodes;
    return 0;
  }

  const char *Database::xslFunction_isTransient(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    //caller frees result
    static const char *sFunctionSignature = "database:is-transient([node])";
    assert(pQE);
    assert(pXCtxt);

    const IXmlBaseNode *pSelectNode  = 0;
    bool bIsTransient = false;

    //parameters (popped in reverse order): node
    UNWIND_EXCEPTION_BEGIN {
      if (pXCtxt->valueCount()) pSelectNode = pXCtxt->popInterpretNodeFromXPathFunctionCallStack();
      else pSelectNode = pXCtxt->contextNode(pQE); //pQE as MM
      if (!pSelectNode) throw XPathReturnedEmptyResultSet(this, MM_STRDUP("Output node set empty"), FUNCTION_SIGNATURE);
      bIsTransient = pSelectNode->isTransient();
    } UNWIND_EXCEPTION_END;

    //free up
    if (pSelectNode) delete pSelectNode;

    UNWIND_EXCEPTION_THROW;

    return bIsTransient ? MM_STRDUP("true") : 0;    
  }
  
  const char *Database::xslFunction_isRegistered(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    //caller frees result
    static const char *sFunctionSignature = "database:is-registered([node])";
    assert(pQE);
    assert(pXCtxt);

    const IXmlBaseNode *pSelectNode  = 0;
    bool bIsRegsitered = false;

    //parameters (popped in reverse order): node
    UNWIND_EXCEPTION_BEGIN {
      if (pXCtxt->valueCount()) pSelectNode = pXCtxt->popInterpretNodeFromXPathFunctionCallStack();
      else pSelectNode = pXCtxt->contextNode(pQE); //pQE as MM
      if (!pSelectNode) throw XPathReturnedEmptyResultSet(this, MM_STRDUP("Output node set empty"), FUNCTION_SIGNATURE);
      bIsRegsitered = pSelectNode->isRegistered();
    } UNWIND_EXCEPTION_END;

    //free up
    if (pSelectNode) delete pSelectNode;

    UNWIND_EXCEPTION_THROW;

    return bIsRegsitered ? MM_STRDUP("true") : 0;
  }

  IXmlBaseDoc *Database::transform(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXslStylesheetNode *pXslStylesheetNode, const IXmlQueryEnvironment *pQE, const StringMap<const size_t> *pParamsInt, const StringMap<const char*> *pParamsChar, const StringMap<const XmlNodeList<const IXmlBaseNode>*> *pParamsNodeSet, const char *sWithMode) const {
    return m_pDoc->transform(pMemoryLifetimeOwner, pXslStylesheetNode, pQE, pParamsInt, pParamsChar, pParamsNodeSet, sWithMode);
  }

  const char *Database::xslFunction_classXSchemaNodeMask(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    static const char *sFunctionSignature = "database:class-xschema-node-mask([node, interface, class])";
    assert(pQE);
    assert(pXCtxt);

    DatabaseClass *pDatabaseClass = 0;
    vector<DatabaseClass*> *pDatabaseClasses   = 0;
    XmlNodeList<const IXmlBaseNode> *pNewNodes = 0;
    const char *sInterfaceMode = 0;
    IXmlBaseNode *pInputNode = 0,
                 *pNode      = 0;

    //parameters (popped in reverse order)
    UNWIND_EXCEPTION_BEGIN {
      if (pXCtxt->valueCount() > 2) {
        pDatabaseClass = DatabaseClass::popInterpretDatabaseClassFromXPathFunctionCallStack(pXCtxt, sFunctionSignature);
      }

      if (pXCtxt->valueCount() > 1) {
        sInterfaceMode = pXCtxt->popInterpretCharacterValueFromXPathFunctionCallStack();
      }

      if (pXCtxt->valueCount() > 0) {
        pInputNode = pXCtxt->popInterpretNodeFromXPathFunctionCallStack();
        if (!pInputNode) throw XPathReturnedEmptyResultSet(this, MM_STRDUP("input node"), FUNCTION_SIGNATURE);
      } else {
        pInputNode = pXCtxt->contextNode(pQE); //pQE as MM
        if (!pInputNode) throw XPathTooFewArguments(this, FUNCTION_SIGNATURE);
      }

      if (!pDatabaseClass && pInputNode->isNodeElement()) {
        pDatabaseClasses = DatabaseClass::classesFromElement(pInputNode);
        if (pDatabaseClasses->size() == 1) {
          pDatabaseClass = pDatabaseClasses->at(0);
        }
        else if (pDatabaseClasses->size() > 1) throw TooManyDatabaseClassOnElement(this, pInputNode->localName());
      }

      //collect the node-mask nodes in the QE
      if (pDatabaseClass) {
        pNode     = pDatabaseClass->xschemaNodeMask(pQE, pInputNode, sInterfaceMode);
        pNewNodes = pQE->nodes()->clone_with_resources();
      }
    } UNWIND_EXCEPTION_END;

    //free up
    //clear up the whole tmp node immediately: not being returned
    if (pNode)            {
      pNode->removeAndDestroyNode(m_pIBQE_dbAdministrator);
      delete pNode;
    }
    if (sInterfaceMode)     MMO_FREE(sInterfaceMode);
    if (pInputNode)       delete pInputNode;
    if (pDatabaseClasses) delete pDatabaseClasses;
    //if (pDatabaseClass)  delete pDatabaseClass; //direct pointer to the static map
    UNWIND_CONTAINER_DESTROY_AND_DELETE_IF_EXCEPTION(pNewNodes);

    UNWIND_EXCEPTION_THROW;
    
    *pNodes = pNewNodes;
    return 0;
  }

  const char *Database::xslFunction_classXSchema(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    static const char *sFunctionSignature = "database:class-xschema([string className|node class:*, xschema-name, stand-alone, compiled])";
    assert(pQE);
    assert(pXCtxt);

    XmlNodeList<const IXmlBaseNode> *pNewNodes = 0;
    IXmlBaseNode *pClassNode         = 0,
                 *pXSchemaNode       = 0;
    const DatabaseClass *pDatabaseClass = 0;
    vector<DatabaseClass*>::const_iterator iBaseClass;
    const char *sXSchemaName         = 0;
    bool bStandalone = true,
         bCompiled   = false;

    //parameters (popped in reverse order)
    UNWIND_EXCEPTION_BEGIN {
      pNewNodes = new XmlNodeList<const IXmlBaseNode>(this);

      if (pXCtxt->valueCount() > 3) {
        bCompiled = pXCtxt->popInterpretBooleanValueFromXPathFunctionCallStack();
      }

      if (pXCtxt->valueCount() > 2) {
        bStandalone = pXCtxt->popInterpretBooleanValueFromXPathFunctionCallStack();
      }

      if (pXCtxt->valueCount() > 1) {
        if (pXCtxt->xtype() == XPATH_BOOLEAN) Debug::report("boolean argument sent through for xschema name");
        else sXSchemaName = pXCtxt->popInterpretCharacterValueFromXPathFunctionCallStack();
      }

      if (pXCtxt->valueCount() > 0) {
        pDatabaseClass = DatabaseClass::popInterpretDatabaseClassFromXPathFunctionCallStack(pXCtxt, sFunctionSignature);
      } else {
        //------------- no parameters: use the context node as a class:* or other node
        if (pClassNode = pXCtxt->contextNode(pQE)) { //pQE as MM
          if (pClassNode->isNamespace(NAMESPACE_CLASS)) {
            pDatabaseClass = DatabaseClass::classFromClassNode(pClassNode);
          } else throw XPathFunctionWrongArgumentType(this, MM_STRDUP("node"), MM_STRDUP("node must be class:* namespace"));
        }
      }

      //compile the inherited xschema together
      if (pDatabaseClass) {
        //sorry, but i don't know you, so you have to use your own security profile:
        if (pXSchemaNode = pDatabaseClass->xschema(pQE, sXSchemaName, bStandalone, bCompiled))
          pNewNodes->push_back(pXSchemaNode);
      } else Debug::report("DatabaseClass not found", rtWarning);
    } UNWIND_EXCEPTION_END;

    //free up
    if (sXSchemaName) MMO_FREE(sXSchemaName);
    if (pClassNode)   delete pClassNode;
    //if (pDatabaseClass)  delete pDatabaseClass; //direct pointer to the static map
    UNWIND_CONTAINER_DESTROY_AND_DELETE_IF_EXCEPTION(pNewNodes);

    UNWIND_EXCEPTION_THROW;

    *pNodes   = pNewNodes;
    return 0;
  }

  const char *Database::xslFunction_extensionElementPrefixes(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    //get this from the underlying XSL provider
    //complement to LibXml2::register extension
    return pQE->emoContext()->extensionElementPrefixes();
  }

  const char *Database::xslFunction_dynamicStylesheet(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    //caller frees result
    //returns the DXSL dynamically generated stylesheet
    //pQE context used for variable resolution
    static const char *sFunctionSignature = "database:dynamic-stylesheet(xsl-stylesheet)"; UNUSED(sFunctionSignature);
    assert(pQE);
    assert(pXCtxt);

    const IXmlBaseNode *pDXSLStylesheetNode = 0;
    const IXmlBaseNode *pNewXSLStylesheet   = 0;
    XmlNodeList<const IXmlBaseNode> *pNewNodes = new XmlNodeList<const IXmlBaseNode>(this);

    UNWIND_EXCEPTION_BEGIN {
      //parameters (popped in reverse order)
      pDXSLStylesheetNode = pXCtxt->popInterpretNodeFromXPathFunctionCallStack();

      if (pDXSLStylesheetNode) {
        //TODO: move this down to the IXmlBaseNode layer?
        DatabaseNode dbDXSLStylesheetNode(this, this, pDXSLStylesheetNode);
        if (pNewXSLStylesheet = dbDXSLStylesheetNode.generateDynamicStylesheet(pQE))
          pNewNodes->push_back(pNewXSLStylesheet);
      }
    } UNWIND_EXCEPTION_END;

    //free up
    //if (pDXSLStylesheetNode) delete pDXSLStylesheetNode; //deleted by ~DatabaseNode
    UNWIND_CONTAINER_DESTROY_AND_DELETE_IF_EXCEPTION(pNewNodes);

    UNWIND_EXCEPTION_THROW;

    *pNodes = pNewNodes;
    return 0;
  }

  const char *Database::xslFunction_classXStylesheet(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    //caller frees result
    //returns the amalgamated inherited xschema deifinition for the main class indicated by the parameter
    //  @className: this string will be used to lookup the main class
    //  @node (class:*)  this node will be used to evaluate the target main class
    //  no parameters: the context node will be used and treated as a node (class:*) definition
    static const char *sFunctionSignature = "database:class-xstylesheet([string className|node class:*] [, stand-alone, client-side-only])";
    assert(pQE);
    assert(pXCtxt);

    XmlNodeList<const IXmlBaseNode> *pNewNodes = 0;
    IXmlBaseNode *pInputNode         = 0,
                 *pResultNode        = 0;
    const DatabaseClass *pDatabaseClass = 0;
    vector<DatabaseClass*>::const_iterator iBaseClass;
    const char *sClassName           = 0;
    bool bStandalone     = false,
         bClientSideOnly = true;

    //parameters (popped in reverse order)
    UNWIND_EXCEPTION_BEGIN {
      switch (pXCtxt->valueCount()) {
        //TODO: we re-factored this from if (pXCtxt->valueCount() > x-1) recently...
        case 3: bClientSideOnly = pXCtxt->popInterpretBooleanValueFromXPathFunctionCallStack(); ATTRIBUTE_FALLTHROUGH;
        case 2: bStandalone     = pXCtxt->popInterpretBooleanValueFromXPathFunctionCallStack(); ATTRIBUTE_FALLTHROUGH;
        case 1: {
          switch (pXCtxt->xtype()) {
            //------------- specific className string sent through: check map
            case XPATH_STRING: {
              sClassName = pXCtxt->popInterpretCharacterValueFromXPathFunctionCallStack();
              if (!sClassName || !*sClassName) throw XPathReturnedEmptyResultSet(this, MM_STRDUP("Input string empty or wrong type"), FUNCTION_SIGNATURE);
              else pDatabaseClass = DatabaseClass::classFromName(sClassName);
              break;
            }
            //------------- node sent through: class:* or other node
            case XPATH_NODESET: {
              pInputNode = pXCtxt->popInterpretNodeFromXPathFunctionCallStack();
              if (pInputNode->isNamespace(NAMESPACE_CLASS)) {
                pDatabaseClass = DatabaseClass::classFromClassNode(pInputNode);
              } else throw XPathFunctionWrongArgumentType(this, MM_STRDUP("node"), MM_STRDUP("node must be class:* namespace"));
              break;
            }
            default: {
              throw XPathFunctionWrongArgumentType(this, MM_STRDUP("string|nodeset"), MM_STRDUP("unknown"));
            }
          }
          break;
        }
        case 0: {
          //------------- no parameters: use the context node as a class:* or other node
          if (pInputNode = pXCtxt->contextNode(pQE)) { //pQE as MM
            if (pInputNode->isNamespace(NAMESPACE_CLASS)) {
              pDatabaseClass = DatabaseClass::classFromClassNode(pInputNode);
            } else throw XPathFunctionWrongArgumentType(this, MM_STRDUP("node"), MM_STRDUP("node must be class:* namespace"));
          }
          break;
        }
        default: throw XPathTooManyArguments(this, FUNCTION_SIGNATURE);
      }

      //compile the inherited xschema together
      if (pDatabaseClass) {
        //sorry, but i don't know you, so you have to use your own security profile:
        if (pResultNode = pDatabaseClass->xstylesheet(pQE, bStandalone, bClientSideOnly)) {
          pNewNodes = new XmlNodeList<const IXmlBaseNode>(this);
          pNewNodes->push_back(pResultNode);
        }
      } else Debug::report("DatabaseClass not found", rtWarning);
    } UNWIND_EXCEPTION_END;

    //free up
    //if (pClassNode)  delete pClassNode; //server freed from map
    if (sClassName)   MMO_FREE(sClassName);
    if (pInputNode)   delete pInputNode;
    //if (pDatabaseClass)  delete pDatabaseClass; //direct pointer to the static map
    UNWIND_CONTAINER_DESTROY_AND_DELETE_IF_EXCEPTION(pNewNodes);

    UNWIND_EXCEPTION_THROW;

    *pNodes = pNewNodes;
    return 0;
  }
  
  const char *Database::xslFunction_classTemplateMatchClause(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    static const char *sFunctionSignature = "database:class-template-match()"; UNUSED(sFunctionSignature);
    const char *sMatchClause             = 0;
    const IXmlBaseNode *pContextNode     = 0,
                       *pClassNode       = 0;
    DatabaseClass *pMainDatabaseClass    = 0;

    assert(pQE);
    assert(pXCtxt);

    UNWIND_EXCEPTION_BEGIN {
      //find the DatabaseClass
      if (pContextNode = pXCtxt->contextNode(pQE)) { //pQE as MM
        if (pClassNode = pContextNode->getSingleNode(m_pIBQE_dbAdministrator, "ancestor-or-self::class:*[1]")) {
          if (pMainDatabaseClass = DatabaseClass::classFromClassNode(pClassNode)) {
            sMatchClause = MM_STRDUP(pMainDatabaseClass->m_sTemplateMatchClause);
          } else throw UnregisteredDatabaseClass(this, pClassNode->localName());
        } else {
          throw XPathFunctionWrongArgumentType(this, MM_STRDUP("node"), MM_STRDUP("context node must have a valid class:*"));
        }
      } else throw XPathFunctionWrongArgumentType(this, MM_STRDUP("node"), MM_STRDUP("xsl:template context node required"));
    } UNWIND_EXCEPTION_END;

    //free up
    if (pContextNode)        delete pContextNode;
    if (pClassNode)          delete pClassNode;
    //if (pMainDatabaseClass)    delete pMainDatabaseClass;    //direct pointer in to database map

    UNWIND_EXCEPTION_THROW;
    
    return sMatchClause;
  }

  const char *Database::xslFunction_derivedTemplateMatchClause(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    //caller frees result
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
    //
    //takes in to account
    //  the inheritance
    //  TODO: partial xpath like object:User/@name
    //  TODO: conditions    like object:User[object:Group]
    //
    //hardlinked xsl:templates / xsl:stylesheets are not allowed. this is inheritance!
    //LibXSL also allows dynamic xsl:template/@match elements now
    //
    //GDB: p *((const LibXslXPathFunctionContext*) pXCtxt)->m_ctxt->context
    static const char *sFunctionSignature = "database:derived-template-match(['match' or @match])"; UNUSED(sFunctionSignature);
    const char *sMatch                   = 0,
               *sNewMatch                = 0;
    const IXmlBaseNode *pContextNode     = 0,
                       *pClassNode       = 0;
    DatabaseClass *pMainDatabaseClass    = 0;

    assert(pQE);
    assert(pXCtxt);

    UNWIND_EXCEPTION_BEGIN {
      //optional sub-match (defaults to class @elements)
      if (pXCtxt->valueCount()) {
        sMatch    = pXCtxt->popInterpretCharacterValueFromXPathFunctionCallStack();
        sNewMatch = sMatch;
      }

      //find the DatabaseClass
      if (pContextNode = pXCtxt->contextNode(pQE)) { //pQE as MM
        if (pClassNode = pContextNode->getSingleNode(m_pIBQE_dbAdministrator, "ancestor-or-self::class:*[1]")) {
          if (pMainDatabaseClass = DatabaseClass::classFromClassNode(pClassNode)) {
            //may return sNewMatch = sMatch
            sNewMatch = pMainDatabaseClass->derivedTemplateMatchClause(sMatch);
          } else throw UnregisteredDatabaseClass(this, pClassNode->localName());
        } else {
          //ancestor::class:*[1] missing, returns sMatch
          if (!sMatch || !*sMatch) {
            //@elements is blank @match was not sent through
            throw XPathFunctionWrongArgumentType(this, MM_STRDUP("node"), MM_STRDUP("xsl:template must have a valid class:*, or send through an explicit match string"));
          }
        }
      } else throw XPathFunctionWrongArgumentType(this, MM_STRDUP("node"), MM_STRDUP("xsl:template context node required"));
      //Debug::report("xsl:template @match morph [%s] => [%s]", sMatch, stXPathElements.c_str());
    } UNWIND_EXCEPTION_END;

    //free up
    if (pContextNode)        delete pContextNode;
    if (pClassNode)          delete pClassNode;
    if (sMatch && sMatch != sNewMatch) MMO_FREE(sMatch);
    //if (pMainDatabaseClass)    delete pMainDatabaseClass;    //direct pointer in to database map

    UNWIND_EXCEPTION_THROW;

    return sNewMatch;
  }

  const char *Database::xslFunction_derivedClasses(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    //caller frees result
    //NOTE: this returns nodes! do not put in to value-of!!
    //returns the derived-classes class:nodes for the main class indicated by the parameter
    //  @className: this string will be used to lookup the main class
    //  @node (class:*)  this node will be used to evaluate the target main class
    //  no parameters: the context node will be used and treated as a node (class:*) definition
    static const char *sFunctionSignature = "database:derived-classes([string className|node class:*])";
    assert(pQE);
    assert(pXCtxt);

    XmlNodeList<const IXmlBaseNode> *pNewNodes = new XmlNodeList<const IXmlBaseNode>(this);
    IXmlBaseNode *pInputNode         = 0;
    const DatabaseClass *pDatabaseClass = 0;
    vector<DatabaseClass*>::const_iterator iBaseClass;
    const char *sClassName           = 0;

    //parameters (popped in reverse order)
    UNWIND_EXCEPTION_BEGIN {
      if (pXCtxt->valueCount()) {
        switch (pXCtxt->xtype()) {
          //------------- specific className string sent through: check map
          case XPATH_STRING: {
            sClassName = pXCtxt->popInterpretCharacterValueFromXPathFunctionCallStack();
            if (!sClassName || !*sClassName) throw XPathReturnedEmptyResultSet(this, MM_STRDUP("Input string empty or wrong type"), FUNCTION_SIGNATURE);
            else pDatabaseClass = DatabaseClass::classFromName(sClassName);
            break;
          }
          //------------- node sent through: class:* or other node
          case XPATH_NODESET: {
            pInputNode = pXCtxt->popInterpretNodeFromXPathFunctionCallStack();
            if (pInputNode->isNamespace(NAMESPACE_CLASS)) {
              pDatabaseClass = DatabaseClass::classFromClassNode(pInputNode);
            } else throw XPathFunctionWrongArgumentType(this, MM_STRDUP("node"), MM_STRDUP("node must be class:* namespace"));
            break;
          }
          default: {
            throw XPathFunctionWrongArgumentType(this, MM_STRDUP("string|nodeset"), MM_STRDUP("unknown"));
          }
        }
      } else {
        //------------- no parameters: use the context node as a class:* or other node
        if (pInputNode = pXCtxt->contextNode(pQE)) { //pQE as MM
          if (pInputNode->isNamespace(NAMESPACE_CLASS)) {
            pDatabaseClass = DatabaseClass::classFromClassNode(pInputNode);
          } else throw XPathFunctionWrongArgumentType(this, MM_STRDUP("node"), MM_STRDUP("node must be class:* namespace"));
        }
      }

      //return the derived classes
      if (pDatabaseClass) {
        for (iBaseClass = pDatabaseClass->derivedClasses()->begin(); iBaseClass != pDatabaseClass->derivedClasses()->end(); iBaseClass++) {
          pNewNodes->push_back((*iBaseClass)->dbNode()->node_const()->clone_with_resources());
        }
      } else Debug::report("DatabaseClass not found", rtWarning);
    } UNWIND_EXCEPTION_END;

    //free up
    //if (pClassNode)  delete pClassNode; //server freed from map
    if (sClassName)  MMO_FREE(sClassName);
    if (pInputNode)  delete pInputNode;
    //if (pDatabaseClass)  delete pDatabaseClass; //direct pointer to the static map
    UNWIND_CONTAINER_DESTROY_AND_DELETE_IF_EXCEPTION(pNewNodes);

    UNWIND_EXCEPTION_THROW;

    *pNodes = pNewNodes;
    return 0;
  }

  const char *Database::xslFunction_classes(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    //caller frees result
    //  @element is the class @elements to search for, e.g. an object:User would yield the class:User definition
    //  without @element the context node will be used
    //  @and-bases = true() will cause the base classes of each class to be included, default: false()
    //results are always unique
    //note that there can be multiple class definitions for the same node
    //e.g. html:html supports HTMLWebpage, Togglesing, BodyInfoClasses
    static const char *sFunctionSignature = "database:classes([elements, and-bases, and-namespace])";
    //e.g. database:classes(//*) to get ALL the classes relevant to an entire document
    assert(pQE);
    assert(pXCtxt);

    XmlNodeList<const IXmlBaseNode> *pvNewNodes    = new XmlNodeList<const IXmlBaseNode>(this);
    XmlNodeList<IXmlBaseNode>       *pvSelectNodes = 0;
    XmlNodeList<IXmlBaseNode>::const_iterator iSelectNode;
                                    
    const vector<DatabaseClass*> *pvDatabaseClasses = 0;
    vector<DatabaseClass*>::const_iterator iDatabaseClass;
    const IXmlBaseNode *pClassNode = 0;
    bool bIncludeBases              = false,
         bIncludeAllElementsClasses = true;

    //parameters (popped in reverse order): node
    UNWIND_EXCEPTION_BEGIN {
      switch (pXCtxt->valueCount()) {
        case 3: bIncludeAllElementsClasses = pXCtxt->popInterpretBooleanValueFromXPathFunctionCallStack(); ATTRIBUTE_FALLTHROUGH;
        case 2: bIncludeBases              = pXCtxt->popInterpretBooleanValueFromXPathFunctionCallStack(); ATTRIBUTE_FALLTHROUGH;
        case 1: pvSelectNodes              = pXCtxt->popInterpretNodeListFromXPathFunctionCallStack();
          break;
        default: {
          pvSelectNodes = new XmlNodeList<IXmlBaseNode>(this);
          if (pXCtxt->contextNode(pQE)) pvSelectNodes->push_back(pXCtxt->contextNode(pQE)); //pQE as MM
          else throw XPathReturnedEmptyResultSet(this, MM_STRDUP("Input element node set empty"), FUNCTION_SIGNATURE);
        }
      }

      if (pvSelectNodes) {
        for (iSelectNode = pvSelectNodes->begin(); iSelectNode != pvSelectNodes->end(); iSelectNode++) {
          if (pvDatabaseClasses = DatabaseClass::classesFromElement(*iSelectNode, bIncludeBases, bIncludeAllElementsClasses)) {
            for (iDatabaseClass = pvDatabaseClasses->begin(); iDatabaseClass != pvDatabaseClasses->end(); iDatabaseClass++) {
              pClassNode = (*iDatabaseClass)->dbNode()->node_const();
              if (!pvNewNodes->contains(pClassNode)) pvNewNodes->push_back(pClassNode->clone_with_resources());
            }
            delete pvDatabaseClasses;
          }
        }
      }
    } UNWIND_EXCEPTION_END;

    //free up
    if (pvSelectNodes)     vector_element_destroy(pvSelectNodes);
    //if (pvDatabaseClasses) delete pvDatabaseClasses; //in-loop delete
    UNWIND_CONTAINER_DESTROY_AND_DELETE_IF_EXCEPTION(pvNewNodes);

    UNWIND_EXCEPTION_THROW;

    *pNodes = pvNewNodes;
    return 0;
  }

  const char *Database::xslFunction_class(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    //caller frees result
    //  @className: returns the class from the name
    //  without @className returns the ancestor::class:*[1] in which the context node lies (if any)
    //  with a blank or not found class-name returns an empty node-set
    static const char *sFunctionSignature = "database:class([className])"; UNUSED(sFunctionSignature);
    assert(pQE);
    assert(pXCtxt);

    XmlNodeList<const IXmlBaseNode> *pNewNodes = new XmlNodeList<const IXmlBaseNode>(this);
    const IXmlBaseNode *pInputNode   = 0,
                       *pClassNode   = 0;
    DatabaseClass *pDatabaseClass    = 0;
    const char *sClassName           = 0;

    //parameters (popped in reverse order): node
    UNWIND_EXCEPTION_BEGIN {
      if (pXCtxt->valueCount()) {
        //------------- specific className string sent through: check map
        sClassName = pXCtxt->popInterpretCharacterValueFromXPathFunctionCallStack();
        if (sClassName && sClassName[0]) {
          if (pDatabaseClass = DatabaseClass::classFromName(sClassName)) {
            pClassNode = pDatabaseClass->dbNode()->node_const();
          }
        }
      } else {
        //------------- no parameters: get the first ancestor::class:* definition
        if (pInputNode = pXCtxt->contextNode(pQE)) { //pQE as MM
          pClassNode = pInputNode->getSingleNode(pQE, "ancestor-or-self::class:*[1]");
        }
      }
      //add definition copy: pClassNode is in the server map and server freed
      if (pClassNode) pNewNodes->push_back(pClassNode->clone_with_resources());
    } UNWIND_EXCEPTION_END;

    //free up
    //if (pClassNode)  delete pClassNode; //server freed from map
    if (sClassName)  MMO_FREE(sClassName);
    if (pInputNode)  delete pInputNode;
    //if (pDatabaseClass) delete pDatabaseClass; //direct pointer in to map
    UNWIND_CONTAINER_DESTROY_AND_DELETE_IF_EXCEPTION(pNewNodes);

    UNWIND_EXCEPTION_THROW;

    *pNodes = pNewNodes;
    return 0;
  }

  const char *Database::xslFunction_isObject(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    //caller frees result
    static const char *sFunctionSignature = "database:is-object([node])";
    assert(pQE);
    assert(pXCtxt);

    const IXmlBaseNode *pSelectNode  = 0;
    bool bIsObject = false;

    //parameters (popped in reverse order): node
    UNWIND_EXCEPTION_BEGIN {
      if (pXCtxt->valueCount()) pSelectNode = pXCtxt->popInterpretNodeFromXPathFunctionCallStack();
      else pSelectNode = pXCtxt->contextNode(pQE); //pQE as MM
      if (!pSelectNode) throw XPathReturnedEmptyResultSet(this, MM_STRDUP("Output node set empty"), FUNCTION_SIGNATURE);
      bIsObject = isObject(pSelectNode);
    } UNWIND_EXCEPTION_END;

    //free up
    if (pSelectNode) delete pSelectNode;

    UNWIND_EXCEPTION_THROW;

    return bIsObject ? MM_STRDUP("true") : 0;
  }

  const char *Database::xslFunction_intersection(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    //caller frees result
    static const char *sFunctionSignature = "database:intersection(node-set, node-set [,hardlink-aware])"; UNUSED(sFunctionSignature);
    assert(pQE);
    assert(pXCtxt);

    NOT_CURRENTLY_USED("");
  }

  const char *Database::xslFunction_difference(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    //caller frees result
    static const char *sFunctionSignature = "database:difference(node-set, node-set [,hardlink-aware])"; UNUSED(sFunctionSignature);
    assert(pQE);
    assert(pXCtxt);

    NOT_CURRENTLY_USED("");
    
    XmlNodeList<const IXmlBaseNode> *pNewNodes = 0;
    const XmlNodeList<IXmlBaseNode> *pNodeSet1 = 0,
                                    *pNodeSet2 = 0;
    bool bHardlinkAware = HARDLINK_AWARE;
    XmlNodeList<const IXmlBaseNode> *pDistinctNodeSet = 0;
    IXmlBaseNode *pNode              = 0;
    XmlNodeList<IXmlBaseNode>::const_iterator iNode, iEnd2;

    //parameters (popped in reverse order): node
    UNWIND_EXCEPTION_BEGIN {
      switch (pXCtxt->valueCount()) {
        case 3: {
          bHardlinkAware = pXCtxt->popInterpretBooleanValueFromXPathFunctionCallStack();
          ATTRIBUTE_FALLTHROUGH;
        }
        case 2: {
          pNodeSet2 = pXCtxt->popInterpretNodeListFromXPathFunctionCallStack();
          pNodeSet1 = pXCtxt->popInterpretNodeListFromXPathFunctionCallStack();
          pNewNodes = pDistinctNodeSet = new XmlNodeList<const IXmlBaseNode>(this);

          iEnd2 = pNodeSet2->end();
          for (iNode = pNodeSet1->begin(); iNode != pNodeSet1->end(); iNode++) {
            pNode = *iNode;
            //the XmlNodeList::find() does an is() comparison
            if (pNodeSet2->find_const(pNode, bHardlinkAware) == iEnd2) pDistinctNodeSet->push_back(pNode);
          }
          break;
        }
        default: throw XPathArgumentCountWrong(this, FUNCTION_SIGNATURE);
      }
    } UNWIND_EXCEPTION_END;

    //free up
    if (pNodeSet1)             delete pNodeSet1;
    if (pNodeSet2)             delete pNodeSet2;
    UNWIND_CONTAINER_DESTROY_AND_DELETE_IF_EXCEPTION(pNewNodes);

    UNWIND_EXCEPTION_THROW;

    *pNodes = pNewNodes;
    return 0;
  }

  const char *Database::xslFunction_hasSameNode(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    //caller frees result
    static const char *sFunctionSignature = "database:has-same-node(node-set, node-set [,hardlink-aware])";
    assert(pQE);
    assert(pXCtxt);

    bool bFound = false;
    const XmlNodeList<IXmlBaseNode> *pNodeSet1 = 0,
                                    *pNodeSet2 = 0;
    bool bHardlinkAware = HARDLINK_AWARE;
    XmlNodeList<IXmlBaseNode>::const_iterator iNode, iEnd2;

    //parameters (popped in reverse order): node
    UNWIND_EXCEPTION_BEGIN {
      switch (pXCtxt->valueCount()) {
        case 3: bHardlinkAware = pXCtxt->popInterpretBooleanValueFromXPathFunctionCallStack(); ATTRIBUTE_FALLTHROUGH;
        case 2: {
          pNodeSet1 = pXCtxt->popInterpretNodeListFromXPathFunctionCallStack();
          pNodeSet2 = pXCtxt->popInterpretNodeListFromXPathFunctionCallStack();

          iEnd2 = pNodeSet2->end();
          for (iNode = pNodeSet1->begin(); iNode != pNodeSet1->end() && !bFound; iNode++) {
            //the XmlNodeList::find() does an is() comparison
            //which also checks hardlinks
            if (pNodeSet2->find_const(*iNode, bHardlinkAware) != iEnd2) bFound = true;
          }
          break;
        }
        default: throw XPathArgumentCountWrong(this, FUNCTION_SIGNATURE);
      }
    } UNWIND_EXCEPTION_END;

    //free up
    if (pNodeSet1)             delete pNodeSet1;
    if (pNodeSet2)             delete pNodeSet2;

    UNWIND_EXCEPTION_THROW;

    return (bFound ? MM_STRDUP("yes") : 0);
  }

  const char *Database::xslFunction_distinct(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    //caller frees result
    static const char *sFunctionSignature = "database:distinct(node-set [, xpath-distinct-clause ,hardlink-aware])";
    assert(pQE);
    assert(pXCtxt);

    XmlNodeList<const IXmlBaseNode> *pNewNodes = 0;
    const char *sXPathDistinctClause = 0,
               *sAttributeClause     = 0, //DO NOT free. it is a pointer in to other mallocd space
               *sDistinctValue;
    const XmlNodeList<IXmlBaseNode> *pNodeSet = 0;
    XmlNodeList<const IXmlBaseNode> *pDistinctNodeSet = 0;
    bool bHardlinkAware = HARDLINK_AWARE;
    IXmlBaseNode *pNode              = 0;
    XmlNodeList<IXmlBaseNode>::const_iterator iNode;
    StringMap<IXmlBaseNode*> mNodeSet;

    //parameters (popped in reverse order): node
    UNWIND_EXCEPTION_BEGIN {
      switch (pXCtxt->valueCount()) {
        case 3: {
          bHardlinkAware = pXCtxt->popInterpretBooleanValueFromXPathFunctionCallStack();
          ATTRIBUTE_FALLTHROUGH;
        }
        case 2: {
          sXPathDistinctClause = pXCtxt->popInterpretCharacterValueFromXPathFunctionCallStack();
          pNodeSet             = pXCtxt->popInterpretNodeListFromXPathFunctionCallStack();
          if (!pNodeSet) throw XPathReturnedEmptyResultSet(this, MM_STRDUP("Node set empty"), FUNCTION_SIGNATURE);
          pNewNodes = pDistinctNodeSet = new XmlNodeList<const IXmlBaseNode>(this);

          if (sXPathDistinctClause[0] == '@') {
            //assume an attribute value check
            sAttributeClause = sXPathDistinctClause + 1;
            for (iNode = pNodeSet->begin(); iNode != pNodeSet->end(); iNode++) {
              pNode          = *iNode;
              sDistinctValue = pNode->attributeValue(pQE, sAttributeClause);
              if (!sDistinctValue) sDistinctValue = MM_STRDUP("");
              if (mNodeSet.find(sDistinctValue) == mNodeSet.end()) {
                mNodeSet.insert(sDistinctValue, pNode);
                pDistinctNodeSet->push_back(pNode);
              } else {
                MMO_FREE(sDistinctValue);
              }
            }
          } else {
            NOT_CURRENTLY_USED("");
          }
          break;
        }
        case 1: {
          pNodeSet  = pXCtxt->popInterpretNodeListFromXPathFunctionCallStack();
          pNewNodes = pDistinctNodeSet = new XmlNodeList<const IXmlBaseNode>(this);
          for (iNode = pNodeSet->begin(); iNode != pNodeSet->end(); iNode++) {
            pNode = *iNode;
            //the XmlNodeList::find() does an is() comparison
            if (pDistinctNodeSet->find(pNode, bHardlinkAware) == pDistinctNodeSet->end()) pDistinctNodeSet->push_back(pNode);
          }
          break;
        }
        case 0:  throw XPathTooFewArguments(this, FUNCTION_SIGNATURE);
        default: throw XPathTooManyArguments(this, FUNCTION_SIGNATURE);
      }
    } UNWIND_EXCEPTION_END;

    //free up
    if (sXPathDistinctClause) MMO_FREE(sXPathDistinctClause);
    //if (sAttributeClause)     MMO_FREE(sAttributeClause); //DO NOT free. it is a pointer in to other mallocd space
    if (pNodeSet)             delete pNodeSet;
    mNodeSet.key_free();
    UNWIND_CONTAINER_DESTROY_AND_DELETE_IF_EXCEPTION(pNewNodes);

    UNWIND_EXCEPTION_THROW;

    *pNodes = pNewNodes;
    return 0;
  }

  size_t Database::descendantObjects(const IXmlQueryEnvironment *pQE, const IXmlBaseNode *pNode, XmlNodeList<const IXmlBaseNode> *pvDescendantObjects, const bool bIncludeIntermediateNodes) const {
    assert(pQE);
    assert(pNode);
    assert(pvDescendantObjects);

    XmlNodeList<const IXmlBaseNode> *pvChildren = pNode->children(pQE);
    XmlNodeList<const IXmlBaseNode>::iterator iChild;
    const IXmlBaseNode *pChildNode;

    for (iChild = pvChildren->begin(); iChild != pvChildren->end(); iChild++) {
      pChildNode = *iChild;
      if (isObject(pChildNode)) {
        pvDescendantObjects->push_back(pChildNode);
        //do not recurse below sub-objects
      } else {
        //recursive
        descendantObjects(pQE, pChildNode, pvDescendantObjects, bIncludeIntermediateNodes);
        //add intermediate nodes if asked
        //useful for node-masks
        if (bIncludeIntermediateNodes) pvDescendantObjects->push_back(pChildNode);
        else delete pChildNode;
      }
    }

    return pvDescendantObjects->size();
  }

  const char *Database::xslFunction_descendantObjects(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    //nearest or leaf sub-objects
    //starts from children this node
    static const char *sFunctionSignature = "database:descendant-objects(node)";
    assert(pQE);
    assert(pXCtxt);

    XmlNodeList<const IXmlBaseNode> *pNewNodes = 0;
    IXmlBaseNode *pParentNode        = 0;
    bool bIncludeIntermediateNodes   = false;

    //parameters (popped in reverse order): node
    UNWIND_EXCEPTION_BEGIN {
      if (pXCtxt->valueCount() > 1) bIncludeIntermediateNodes = pXCtxt->popInterpretBooleanValueFromXPathFunctionCallStack();
      pParentNode = pXCtxt->popInterpretNodeFromXPathFunctionCallStack();
      if (!pParentNode) throw XPathReturnedEmptyResultSet(this, MM_STRDUP("Output node set empty"), FUNCTION_SIGNATURE);

      //return vector
      pNewNodes   = new XmlNodeList<const IXmlBaseNode>(this);

      //recurse down tree looking for nearest objects
      descendantObjects(pQE, pParentNode, pNewNodes, bIncludeIntermediateNodes);

    } UNWIND_EXCEPTION_END;

    //free up
    UNWIND_CONTAINER_DESTROY_AND_DELETE_IF_EXCEPTION(pNewNodes);

    UNWIND_EXCEPTION_THROW;

    *pNodes = pNewNodes;
    return 0;
  }

  const char *Database::xslFunction_containerObject(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    //better performance for ancestor-or-self::*[isObject()][1]
    //starts from this node
    static const char *sFunctionSignature = "database:container-object(node)";
    assert(pQE);
    assert(pXCtxt);

    XmlNodeList<const IXmlBaseNode> *pNewNodes = 0;
    IXmlBaseNode *pParentNode        = 0,
                 *pOldParentNode     = 0;

    //parameters (popped in reverse order): node
    UNWIND_EXCEPTION_BEGIN {
      pParentNode = pXCtxt->popInterpretNodeFromXPathFunctionCallStack();
      if (!pParentNode) throw XPathReturnedEmptyResultSet(this, MM_STRDUP("Output node set empty"), FUNCTION_SIGNATURE);

      //return vector
      pNewNodes   = new XmlNodeList<const IXmlBaseNode>(this);

      //recurse up tree looking for nearest object
      while (pParentNode && !isObject(pParentNode)) {
        pOldParentNode = pParentNode;
        pParentNode    = pParentNode->parentNode(pQE);
        delete pOldParentNode;
      }
      if (pParentNode) pNewNodes->push_back(pParentNode);
    } UNWIND_EXCEPTION_END;

    //free up
    UNWIND_CONTAINER_DESTROY_AND_DELETE_IF_EXCEPTION(pNewNodes);

    UNWIND_EXCEPTION_THROW;

    *pNodes = pNewNodes;
    return 0;
  }

  const char *Database::xslFunction_nodeMask(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    //applies the current node-mask and returns the visible nodes
    //TODO: really?
    static const char *sFunctionSignature = "database:node-mask(node-set)"; UNUSED(sFunctionSignature);
    assert(pQE);
    assert(pXCtxt);

    XmlNodeList<const IXmlBaseNode> *pNewNodes = 0;
    
    //parameters (popped in reverse order): node
    UNWIND_EXCEPTION_BEGIN {
      pNewNodes = (XmlNodeList<const IXmlBaseNode> *) pXCtxt->popInterpretNodeListFromXPathFunctionCallStack();
    } UNWIND_EXCEPTION_END;

    //free up
    UNWIND_CONTAINER_DESTROY_AND_DELETE_IF_EXCEPTION(pNewNodes);

    UNWIND_EXCEPTION_THROW;

    *pNodes = pNewNodes;
    return 0;
  }

  size_t Database::xmlIDMainIndexDocument(IXmlBaseNode *pFromNode) {
    //caller frees pFromNode
    //vNewIDs is created and destroyed here
    XmlNodeList<IXmlBaseNode> vNewIDs(this);
    size_t iNewIDs;
    iNewIDs = xmlIDMainIndexDocument(pFromNode, vNewIDs);
    vector_element_destroy(vNewIDs);
    return iNewIDs;
  }

  size_t Database::xmlIDMainIndexDocument(IXmlBaseNode *pFromNode, XmlNodeList<IXmlBaseNode> &vNewIDs) {
    //public call to hide recursive one
    assert(pFromNode);

    dbIndexingStatus dbis;
    IXmlBaseNode *pMainIdxConfigNode = 0;

    //----------------------------------- settings
    //get / default index settings for THIS Database
    //indexing is rare, so live get settings every time
#ifdef WITH_DATABASE_IDMODE
    pMainIdxConfigNode = m_pDoc->getSingleNode(m_pIBQE_dbAdministrator, "/*/gs:main_idx");
    if (!pMainIdxConfigNode) {
      Debug::report("creating new primary indexing idx_* settings for Database [%s]", m_sAlias);
      pMainIdxConfigNode = m_pDoc->createChildElement(m_pIBQE_dbAdministrator, "main_idx", NULL, NAMESPACE_GS);
      pMainIdxConfigNode->setAttribute(m_pIBQE_dbAdministrator, "prefix", "idx_");
      pMainIdxConfigNode->setAttribute(m_pIBQE_dbAdministrator, "from",   "0");
      pMainIdxConfigNode->setAttribute(m_pIBQE_dbAdministrator, "ignore-namespaces", "http://general_server.org/xmlnamespaces/dummyxsl/2006 http://www.w3.org/1999/xhtml");
    }
    dbis.sMainIdxPrefix = pMainIdxConfigNode->attributeValue(m_pIBQE_dbAdministrator, "prefix");
    dbis.sMainIdxSuffix = pMainIdxConfigNode->attributeValue(m_pIBQE_dbAdministrator, "suffix");
    dbis.iUIDXFrom      = pMainIdxConfigNode->attributeValueInt(m_pIBQE_dbAdministrator, "from");
    dbis.sIgnoreNS      = pMainIdxConfigNode->attributeValue(m_pIBQE_dbAdministrator, "ignore-namespaces");
    if (dbis.sIgnoreNS && *dbis.sIgnoreNS) Debug::report("xmlIDMainIndexDocument::ignoring namespaces for auto-@xml:id\n  [%s]", dbis.sIgnoreNS);
    dbis.bInXSL         = false;

    //input cloning because it might be included in the output vector
    //and thus in danger of double free
    //this "clone_with_resources" is actually a wrapper clone
    //  because the IXmlBaseNode is only a wrapper anyway, it never OWNS anything, ALWAYS_NOT_OURS
    pFromNode = pFromNode->clone_with_resources();

    xmlIDMainIndexDocument_recursive(pFromNode, vNewIDs, dbis);

    //------------------------------------ registration of completion
    //finished
    //main index must not repeat deleted indexes
    pMainIdxConfigNode->setAttribute(m_pIBQE_dbAdministrator, "from", (size_t) dbis.iUIDXFrom);
    //free up
    if (pMainIdxConfigNode)  delete_wrapper pMainIdxConfigNode;
    if (dbis.sMainIdxPrefix) MMO_FREE(dbis.sMainIdxPrefix);
    if (dbis.sMainIdxSuffix) MMO_FREE(dbis.sMainIdxSuffix);
    if (dbis.sIgnoreNS)      MMO_FREE(dbis.sIgnoreNS);
#endif

    return vNewIDs.size();
  }

  size_t Database::xmlIDMainIndexDocument_recursive(IXmlBaseNode *pFromNode, XmlNodeList<IXmlBaseNode> &vNewIDs, dbIndexingStatus& dbis) {
    //caller frees pFromNode and vNewIDs
    //caller creates and destroys vNewIDs, usually with vector_element_destroy(vNewIDs)
    //passed-in top-level pFromNode is CLONED
    //  this is so that, if it is in the vNewIDs, it will not be double freed

    //assign a unique xml:id to every element
    //so that it can be linearly referenced
    //xml:id format = 'o123'
    XmlNodeList<IXmlBaseNode> *pvChildren = 0;
    XmlNodeList<IXmlBaseNode>::iterator iChild;
    IXmlBaseNode *pChildNode;
    const char *sXmlId = 0;
    bool bReturnedNode = false,
         bIsXSLTemplate;

    //dont follow hard or soft links for xml:id
    if (!pFromNode->isHardLink() && !pFromNode->isSoftLink()) {
      //ignore transient areas because:
      //  we can reference them with id('idx_56')/object:Session/...
      //  using serialised uniqueXPathToNode() for transient is dangerous because it will disappear
      //  transient created information, the xml:id is unknown to the user until after creation
      //recurse only in to xml:id valid areas, xml:ids won't be registered in "ignore" areas
      if (!pFromNode->isTransient()
        && pFromNode->xmlIDPolicy() == xmlid_area_normal
      ) {
        if (  pFromNode->attributeValueBoolDynamicString(m_pIBQE_dbAdministrator, "add-xml-id", NAMESPACE_XML, true)
          && !pFromNode->isNamespace(dbis.sIgnoreNS) //allows space delimited string of multiple HREFs
          && (!dbis.bInXSL || pFromNode->isNamespace(NAMESPACE_XSL)) //only xml:id XSL within xsl:templates
        ) {
          //----------------------------------- check current ID
          sXmlId = pFromNode->xmlID(m_pIBQE_dbAdministrator);
          if (!sXmlId || !*sXmlId) { //missing ID
            if (pFromNode->isNamespace(NAMESPACE_CLASS)) {
              //class:* have special xml:id
              pFromNode->setXmlID(m_pIBQE_dbAdministrator, "Class__", pFromNode->localName(NO_DUPLICATE));
            } else {
              //get next available, starting from last from
              //iUIDXFrom is not necessarily available, but is the minimum
              dbis.iUIDXFrom = m_pDoc->xmlIDNextAvailable(dbis.sMainIdxPrefix, dbis.iUIDXFrom, dbis.sMainIdxSuffix);

              //and assign it
              pFromNode->setXmlID(m_pIBQE_dbAdministrator, dbis.sMainIdxPrefix, dbis.iUIDXFrom, dbis.sMainIdxSuffix);
              dbis.iUIDXFrom++;
            }

            //report it, and don't delete it!
            vNewIDs.push_back(pFromNode);
            bReturnedNode = true;
          }
        }

        //------------------------------------ recursion
        //xsl:template sub-recursion
        //remeber that dbis is passed by reference for efficiency, not copied
        //so we need to reset THE, non-RENTRANT values after this procedural child loop
        bIsXSLTemplate = pFromNode->isName(NAMESPACE_XSL, "template");
        if (bIsXSLTemplate) dbis.bInXSL = true; {
          //we dont recurse passed xsl:template because it is code and should not be data object controlled
          //xsl:templates and stylesheets are controlled through the editor
          //everything can still be found through xpath
          pvChildren = pFromNode->children(m_pIBQE_dbAdministrator);
          for (iChild = pvChildren->begin(); iChild != pvChildren->end(); iChild++) {
            pChildNode = *iChild;
            xmlIDMainIndexDocument_recursive(pChildNode, vNewIDs, dbis);
            //delete pChildNode //xmlIDMainIndexDocument(...) will do this (unless added to the vector)
          }
        } if (bIsXSLTemplate) dbis.bInXSL = false;
      }
    }

    //free up
    if (!bReturnedNode) delete_wrapper pFromNode; //vector deleted below
    if (sXmlId)         MMO_FREE(sXmlId);
    if (pvChildren)     delete pvChildren;
    //vector_element_destroy(pvChildren)  //nodes are freed individually above

    return vNewIDs.size();
  }

  DatabaseNode *Database::factory_node(IXmlBaseNode *pNode, const bool bOurs) const {
    //TODO: we are manually removing const for Node creation
    //should use templates later on
    return pNode ? new DatabaseNode(this, this, pNode, bOurs) : 0;
  }

  const DatabaseNode *Database::factory_node(const IXmlBaseNode *pNode, const bool bOurs) const {
    //TODO: we are manually removing const for Node creation
    //should use templates later on
    return pNode ? new DatabaseNode(this, this, pNode, bOurs) : 0;
  }

  const IXmlBaseDoc *Database::document_const() const {return m_pDoc;}

  XmlNodeList<DatabaseNode> *Database::factory_nodes(XmlNodeList<IXmlBaseNode> *pvNodeList) const {
    return (XmlNodeList<DatabaseNode>*) factory_nodes((XmlNodeList<const IXmlBaseNode>*) pvNodeList);
  }

  XmlNodeList<const DatabaseNode> *Database::factory_nodes(XmlNodeList<const IXmlBaseNode> *pvNodeList) const {
    //deletes input!!!
    //caller handles return, which might be zero
    XmlNodeList<const DatabaseNode> *pvDatabaseNodeList = 0;
    XmlNodeList<const IXmlBaseNode>::const_iterator iNode;
    if (pvNodeList) {
      pvDatabaseNodeList = new XmlNodeList<const DatabaseNode>(this);
      for (iNode = pvNodeList->begin(); iNode != pvNodeList->end(); iNode++)
        pvDatabaseNodeList->push_back(factory_node(*iNode));
      delete pvNodeList;
    }


    return pvDatabaseNodeList;
  }

  const StringMap<const char*> *Database::allPrefixedNamespaceDefinitions() const {
    return m_pDoc->allPrefixedNamespaceDefinitions();
  }

  DatabaseNode *Database::documentNode(const char *sReason) {
    return factory_node(m_pDoc->documentNode(sReason));
  }

  DatabaseNode *Database::rootNode(const IXmlQueryEnvironment *pQE, const iDOMNodeTypeRequest iNodeTypes, const char *sName, const char *sLockingReason) {
    return factory_node(m_pDoc->rootNode(pQE, iNodeTypes, sName, sLockingReason));
  }

  const DatabaseNode *Database::dtdNode() const {
    return factory_node(m_pDoc->dtdNode());
  }

  DatabaseNode *Database::getSingleNode(const IXmlQueryEnvironment *pQE, const char *sXPath, const char *argchar, const int *argint) {
    return factory_node(m_pDoc->getSingleNode(pQE, sXPath, argchar, argint));
  }

  DatabaseNode *Database::nodeFromID(const IXmlQueryEnvironment *pQE, const char *sXMLID) {
    return factory_node(m_pDoc->nodeFromID(pQE, sXMLID));
  }

  XmlNodeList<DatabaseNode> *Database::getMultipleNodes(const IXmlQueryEnvironment *pQE, const char *sXPath, const char *argchar, const int *argint, const bool bRegister, const char *sLockingReason) {
    return factory_nodes(m_pDoc->getMultipleNodes(pQE, sXPath, argchar, argint, bRegister, sLockingReason));
  }

  int Database::validityCheck(const char *sReason) const {
    return m_pDoc->validityCheck(sReason);
  }

  const char *Database::toString() const {
    stringstream sOut;
    const char *sDoc     = m_pDoc->toString();
    const char *sRep     = m_pRepositoryReader->toString();
    const char *sEMO     = toStringModules();
    const char *sClasses = DatabaseClass::toStringAll();

    validityCheck("Database::toString");
    sOut << "Database [" << m_sAlias << "]\n";
#ifndef WITH_DATABASE_IDMODE
    sOut << "Database @xml:id mode is switched off: elements will not be universally assigned @xml:id's\n";
#endif
    
    sOut << sClasses;
    sOut << (sDoc ? sDoc : "no doc") << "\n"
         << (sRep ? sRep : "no repository") << "\n"
         << (sEMO ? sEMO : "no EMO");

    //clean up
    MMO_FREE(sDoc);
    MMO_FREE(sRep);
    MMO_FREE(sClasses);

    return MM_STRDUP(sOut.str().c_str());
  }

  TXml::transactionResult Database::rollback() {
    TXml::transactionResult tr;

    //delete in-memory transactions
    IXmlBaseNode *pTransactionDisplayNode;
    if (pTransactionDisplayNode = transactionDisplayNode()) pTransactionDisplayNode->removeAndDestroyChildren(m_pIBQE_dbAdministrator);
    //delete repository transactions
    tr = m_pRepositoryReader->rollback();

    return tr;
  }

  TXml::transactionResult Database::commit(TXml::commitStatus cs, TXml::commitStyle ct) {
    return TXmlProcessor::commit((const IXmlBaseDoc*) m_pDoc, (const IXmlBaseNode*) transactionDisplayNode(0), cs, ct);
  }
  TXml::transactionResult Database::commitSetPersistentStatus(TXml::commitStatus cs, TXml::commitStyle ct) {
    //TODO: Database::commitSetPersistentStatus(...) ...
    return TXml::transactionSuccess;
  }
  TXml::transactionResult Database::commitDeleteTransactions(const vector<TXml*> *pvTXmls) {
    transactionDisplayNode(0)->removeAndDestroyChildren(m_pIBQE_dbAdministrator);
    return TXml::transactionSuccess;
  }

  void Database::createTmpArea() {
    IXmlBaseNode *pServerNode = 0;

    m_pTmpNode = m_pDoc->getSingleNode(m_pIBQE_dbAdministrator, "/object:Server/repository:tmp");
    if (!m_pTmpNode) {
      Debug::report("creating [%s] tmp area");
      if (pServerNode = m_pDoc->getSingleNode(m_pIBQE_dbAdministrator, "/object:Server")) {
        m_pTmpNode = pServerNode->createChildElement(m_pIBQE_dbAdministrator, "tmp", NULL, NAMESPACE_REPOSITORY);
        m_pTmpNode->setAttribute(m_pIBQE_dbAdministrator, "id-policy-area",  "ignore",    NAMESPACE_XML);
        m_pTmpNode->setAttribute(m_pIBQE_dbAdministrator, "transient-area",  "yes",       NAMESPACE_GS);
        m_pTmpNode->setAttribute(m_pIBQE_dbAdministrator, "name",            "tmp",       NAMESPACE_REPOSITORY);
        m_pTmpNode->setAttribute(m_pIBQE_dbAdministrator, "type",            "Directory", NAMESPACE_REPOSITORY);
        xmlIDMainIndexDocument(m_pTmpNode); //add @xml:id idx_*

        //save
#ifndef WITH_DATABASE_READONLY
        TXml_TouchNode t(this, m_pIBQE_dbAdministrator, MM_STRDUP("creating the tmp area"), NO_TXML_BASENODE, m_pTmpNode);
        applyTransaction(&t, m_pTmpNode, m_pIBQE_dbAdministrator);
#endif
        
        delete pServerNode;
      } else throw NoTmpNodeFound(this, m_sAlias);
    }
  }

  IXmlBaseNode *Database::createTmpNode() const {
    //Database server keeps record of nodes we create in the tmp space
    //do not delete result! ~Database()
    IXmlBaseNode *pNewTmpNode = 0;

    //create new tmp node for return
    if (!m_pTmpNode) throw NoTmpNodeFound(this, m_sAlias);
    pNewTmpNode = m_pTmpNode->createChildElement(m_pIBQE_dbAdministrator, "tmp", NULL, NAMESPACE_GS);

    return pNewTmpNode;
  }

  const char *Database::xslFunction_validityCheck(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    m_pDoc->validityCheck("xslCommand_validityCheck");
    return 0;
  }

  const char *Database::xslFunction_replaceChildrenTransient(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    XmlNodeList<const IXmlBaseNode> *pNewNodes    = 0;
    
    cout << "Database::xslFunction_replaceChildrenTransient\n";
    NOT_COMPLETE(""); //Database::xslFunction_replaceChildrenTransient()
    /*
    XmlNodeList<IXmlBaseNode> *pvNewParentNodes = 0;
    XmlNodeList<IXmlBaseNode> *pTargetNodes = 0;
    IXslTransformContext *pTransformContext = 0;
    IXmlBaseNode *pTargetNode           = 0;
    IXmlBaseDoc  *pTargetDoc            = 0;

    pTransformContext = pCtxt->transformContext();
    if (pvNewParentNodes = pXCtxt->popInterpretNodeListFromXPathFunctionCallStack()) {
      pNewNodes = pvNewParentNodes->children();

      if (pXCtxt->valueCount() == 2) {
        pTargetNodes = pXCtxt->popInterpretNodeListFromXPathFunctionCallStack();
        if (pTargetNodes->size()) pTargetNode = pTargetNodes[0];
      } else {
        pTargetDoc  = pTransformContext->sourceDoc();
        pTargetNode = pTransformContext->sourceNode(pTargetDoc);
      }

      if (!pTargetNode) throw;
      if (!pTargetNode->isTransientAreaParent()) throw;

      pTargetNode->removeAndDestroyChildren();

      pvNewNodes->moveChildren(pTargetNode);
    }

    //free up
    //if (pvNewNodes)   vector_element_destroy(pvNewNodes); //being returned
    if (pTargetNodes) vector_element_destroy(pTargetNodes);
    delete pTransformContext;
    UNWIND_CONTAINER_DESTROY_AND_DELETE_IF_EXCEPTION(pNewNodes);
    */

    *pNodes = pNewNodes;
    return 0;
  }

  const char *Database::xslFunction_transform(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    //caller frees result
    static const char *sFunctionSignature = "database:transform(data, stylesheet [, mode, node-mask])";
    assert(pQE);
    assert(pXCtxt);

    XmlNodeList<const IXmlBaseNode> *pNewNodes = new XmlNodeList<const IXmlBaseNode>(this);
    XmlNodeList<const IXmlBaseNode> *pFromNodes = 0;
    XmlNodeList<const IXmlBaseNode>::iterator iFromNode;
    const IXmlBaseNode *pFromNode      = 0;
    IXmlBaseNode *pStylesheetNode      = 0,
                 *pOutputNode          = 0,
                 *pFirstChild          = 0;
    const char *sNodeMask              = 0;
    const char *sWithMode              = 0;
    IXmlArea *pArea                    = 0;
    IXmlNodeMask *pNodeMask            = 0;
    bool bOldEnabled                   = false;

    //parameters (popped in reverse order): data, stylesheet [, mode]
    UNWIND_EXCEPTION_BEGIN {
      if (pOutputNode = createTmpNode()) {
        //optional
        if (pXCtxt->valueCount() > 3) sNodeMask = pXCtxt->popInterpretCharacterValueFromXPathFunctionCallStack();
        if (pXCtxt->valueCount() > 2) sWithMode = pXCtxt->popInterpretCharacterValueFromXPathFunctionCallStack();

        //required
        if (pStylesheetNode = pXCtxt->popInterpretNodeFromXPathFunctionCallStack()) {
          if (pFromNodes = (XmlNodeList<const IXmlBaseNode> *) pXCtxt->popInterpretNodeListFromXPathFunctionCallStack()) {
            if (sNodeMask && *sNodeMask) {
              if (pNodeMask = xmlLibrary()->factory_nodeMask(this, sNodeMask)) {
                pArea = xmlLibrary()->factory_area(this, pFromNodes, pQE, pNodeMask);
                if (pQE->maskContext()) {
                  if (pArea) pQE->maskContext()->push_back(pArea);
                  bOldEnabled = pQE->maskContext()->enable();
                }
              }
            }

            //note if there are any:
            //  global <xsl:param @select=database:transform()
            //  URL: database:transform() => global <xsl:param @select=repository:filesystempath-to-nodes($gs_dynamic_url)
            //this will loop as the pQE re-calculates the global xsl:params each transform
            //pQE->inherit() will stop this but will loose the global xsl:params in the process
            for (iFromNode = pFromNodes->begin(); iFromNode != pFromNodes->end(); iFromNode++) {
              pFromNode = *iFromNode;

              pFromNode->transform(
                pStylesheetNode->queryInterface((const IXslStylesheetNode*) 0),
                pQE,
                pOutputNode,
                NO_PARAMS_INT, NO_PARAMS_CHAR, NO_PARAMS_NODE,
                sWithMode
              );
            }
            
            if (pQE->maskContext() && pArea)        pQE->maskContext()->pop_back();
            if (!bOldEnabled && pQE->maskContext()) pQE->maskContext()->disable();

            //WARNING: we are returning an IXmlBaseNode with an OURS document() pointing to the main Database!
            //because it is in the tmp area and createTmpNode() uses the main Database pointer
            //bad idea to free it
            if (pFirstChild = pOutputNode->firstChild(pQE)) pNewNodes->push_back(pFirstChild);
          } else throw XPathReturnedEmptyResultSet(this, MM_STRDUP("Data set empty"), FUNCTION_SIGNATURE);
        } else throw XPathReturnedEmptyResultSet(this, MM_STRDUP("Stylesheet parameter missing or not a node"), FUNCTION_SIGNATURE);
      } else throw CannotCreateInTmpNode(this, m_sAlias);
    } UNWIND_EXCEPTION_END;

    //clean up
    //the tmp node document() points to the main Database document_const() and does not require deletion
    //TODO: and tmp() nodes are cleaned up separately (at end of transform?)
    //if (pOutputNode) delete pOutputNode->document();
    //if (pOutputNode) delete pOutputNode;
    //these documents are returned by popInterpretNodeListFromXPathFunctionCallStack(...) and are sourceDoc() NOT_OURS
    //if (pDataNode)         delete pDataNode->document();       //shared document for these nodes
    //if (pStylesheetNode)   delete pStylesheetNode->document(); //shared document for these nodes
    //if (pTCtxt)            delete pTCtxt; //server free
    if (sWithMode)        MMO_FREE(sWithMode);
    if (sNodeMask)        MMO_FREE(sNodeMask);
    if (pArea)            delete pArea;
    if (pNodeMask)        delete pNodeMask;
    //UNWIND_DELETE_IF_EXCEPTION(pFirstChild); //being returned in pNewNodes
    UNWIND_CONTAINER_DESTROY_AND_DELETE_IF_EXCEPTION(pNewNodes);

    UNWIND_EXCEPTION_THROW;

    *pNodes = pNewNodes;
    return 0;
  }

  const char *Database::xslFunction_fullyQualifiedName(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    assert(pQE);
    assert(pXCtxt);
    static const char *sFunctionSignature = "database:fully-qualified-name(node)"; UNUSED(sFunctionSignature);

    const char *sFullyQualifiedName = 0;
    IXmlBaseNode *pStylesheetNode = 0;

    //parameters (popped in reverse order): data, stylesheet [, mode]
    UNWIND_EXCEPTION_BEGIN {
      if (pStylesheetNode = pXCtxt->popInterpretNodeFromXPathFunctionCallStack()) {
        sFullyQualifiedName = pStylesheetNode->fullyQualifiedName();
      }
    } UNWIND_EXCEPTION_END;

    //clear up
    //if (pNode) delete pNode->document();   //these docs are server freed by the context

    UNWIND_EXCEPTION_THROW;

    return sFullyQualifiedName;
  }

  const char *Database::xslFunction_hasDeviants(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    //true if the node has any deviants
    //includes the original node
    assert(pQE);
    assert(pXCtxt);
    static const char *sFunctionSignature = "database:has-deviants([node])"; UNUSED(sFunctionSignature);

    const char *sHas    = 0;
    IXmlBaseNode *pNode = 0;

    //parameters (popped in reverse order): data, stylesheet [, mode]
    UNWIND_EXCEPTION_BEGIN {
      if (pXCtxt->valueCount() > 0) pNode      = pXCtxt->popInterpretNodeFromXPathFunctionCallStack();
      else                          pNode      = pXCtxt->contextNode(pQE); //pQE as MM

      if (pNode && pNode->hasDeviants()) sHas = MM_STRDUP("true");
      if (!sHas)                         sHas = MM_STRDUP("");
    } UNWIND_EXCEPTION_END;

    //clear up
    if (pNode) delete pNode;   //these docs are server freed by the context

    UNWIND_EXCEPTION_THROW;

    return sHas;
  }

  const char *Database::xslFunction_isDeviant(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    //true if the node has any deviants
    //includes the original node
    assert(pQE);
    assert(pXCtxt);
    static const char *sFunctionSignature = "database:is-deviant([node])"; UNUSED(sFunctionSignature);

    const char *sIs = 0;
    IXmlBaseNode *pNode = 0;

    //parameters (popped in reverse order): data, stylesheet [, mode]
    UNWIND_EXCEPTION_BEGIN {
      if (pXCtxt->valueCount() > 0) pNode      = pXCtxt->popInterpretNodeFromXPathFunctionCallStack();
      else                          pNode      = pXCtxt->contextNode(pQE); //pQE as MM

      if (pNode && pNode->isDeviant()) sIs = MM_STRDUP("true");
      if (!sIs)                        sIs = MM_STRDUP("");
    } UNWIND_EXCEPTION_END;

    //clear up
    if (pNode) delete pNode;   //these docs are server freed by the context

    UNWIND_EXCEPTION_THROW;

    return sIs;
  }

  const char *Database::xslFunction_hasDescendantDeviants(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    //true if the node has any deviants
    //includes the original node
    assert(pQE);
    assert(pXCtxt);
    static const char *sFunctionSignature = "database:has-descendant-deviants([node])"; UNUSED(sFunctionSignature);

    const char *sHas    = 0;
    IXmlBaseNode *pNode = 0;

    //parameters (popped in reverse order): data, stylesheet [, mode]
    UNWIND_EXCEPTION_BEGIN {
      if (pXCtxt->valueCount() > 0) pNode      = pXCtxt->popInterpretNodeFromXPathFunctionCallStack();
      else                          pNode      = pXCtxt->contextNode(pQE); //pQE as MM

      if (pNode && pNode->hasDescendantDeviants()) sHas = MM_STRDUP("true");
      if (!sHas)                                   sHas = MM_STRDUP("");
    } UNWIND_EXCEPTION_END;

    //clear up
    if (pNode) delete pNode;   //these docs are server freed by the context

    UNWIND_EXCEPTION_THROW;

    return sHas;
  }

  const char *Database::xslFunction_isHardlinked(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    //true if the node has any hardlinks
    //includes the original node
    assert(pQE);
    assert(pXCtxt);
    static const char *sFunctionSignature = "database:is-hard-linked([node])"; UNUSED(sFunctionSignature);

    const char *sIsHardlinked = 0;
    IXmlBaseNode *pNode = 0;

    //parameters (popped in reverse order): data, stylesheet [, mode]
    UNWIND_EXCEPTION_BEGIN {
      if (pXCtxt->valueCount() > 0) pNode      = pXCtxt->popInterpretNodeFromXPathFunctionCallStack();
      else                          pNode      = pXCtxt->contextNode(pQE); //pQE as MM

      if (pNode && pNode->isHardLinked()) sIsHardlinked = MM_STRDUP("true");
      if (!sIsHardlinked)                 sIsHardlinked = MM_STRDUP("");
    } UNWIND_EXCEPTION_END;

    //clear up
    if (pNode) delete pNode;   //these docs are server freed by the context

    UNWIND_EXCEPTION_THROW;

    return sIsHardlinked;
  }

  const char *Database::xslFunction_isNodeElement(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    static const char *sFunctionSignature = "database:is-node-element([node])";
    bool bRet = false;
    IXmlBaseNode *pInputNode = 0;

    //parameters (popped in reverse order): data, stylesheet [, mode]
    UNWIND_EXCEPTION_BEGIN {
      switch (pXCtxt->valueCount()) {
        case 1: pInputNode = pXCtxt->popInterpretNodeFromXPathFunctionCallStack(); break;
        case 0: pInputNode = pXCtxt->contextNode(pQE); break; //pQE as MM
        default: throw XPathArgumentCountWrong(this, FUNCTION_SIGNATURE);
      }

      if (pInputNode) bRet = pInputNode->isNodeElement();
      else throw XPathContextNodeRequired(this, FUNCTION_SIGNATURE);
    } UNWIND_EXCEPTION_END;

    //clear up
    if (pInputNode) delete pInputNode;   //these docs are server freed by the context

    UNWIND_EXCEPTION_THROW;

    return (bRet ? MM_STRDUP("true") : 0);
  }

  const char *Database::xslFunction_isNodeDocument(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes){
    static const char *sFunctionSignature = "database:is-node-document()";
    bool bRet = false;
    IXmlBaseNode *pInputNode = pXCtxt->contextNode(pQE); //pQE as MM

    if (pInputNode) bRet = pInputNode->isNodeDocument();
    else throw XPathContextNodeRequired(this, FUNCTION_SIGNATURE);

    return (bRet ? MM_STRDUP("true") : 0);
  }

  const char *Database::xslFunction_isNodeAttribute(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes){
    static const char *sFunctionSignature = "database:is-node-attribute()";
    bool bRet = false;
    IXmlBaseNode *pInputNode = pXCtxt->contextNode(pQE); //pQE as MM

    if (pInputNode) bRet = pInputNode->isNodeAttribute();
    else throw XPathContextNodeRequired(this, FUNCTION_SIGNATURE);

    return (bRet ? MM_STRDUP("true") : 0);
  }

  const char *Database::xslFunction_isNodeText(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes){
    static const char *sFunctionSignature = "database:is-node-text()";
    bool bRet = false;
    IXmlBaseNode *pInputNode = pXCtxt->contextNode(pQE); //pQE as MM

    if (pInputNode) bRet = pInputNode->isNodeText();
    else throw XPathContextNodeRequired(this, FUNCTION_SIGNATURE);

    return (bRet ? MM_STRDUP("true") : 0);
  }

  const char *Database::xslFunction_isProcessingInstruction(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes){
    static const char *sFunctionSignature = "database:is-processing-instruction()";
    bool bRet = false;
    IXmlBaseNode *pInputNode = pXCtxt->contextNode(pQE); //pQE as MM

    if (pInputNode) bRet = pInputNode->isProcessingInstruction();
    else throw XPathContextNodeRequired(this, FUNCTION_SIGNATURE);

    return (bRet ? MM_STRDUP("true") : 0);
  }

  const char *Database::xslFunction_isCDATASection(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes){
    static const char *sFunctionSignature = "database:is-CDATA-section()";
    bool bRet = false;
    IXmlBaseNode *pInputNode = pXCtxt->contextNode(pQE); //pQE as MM

    if (pInputNode) bRet = pInputNode->isCDATASection();
    else throw XPathContextNodeRequired(this, FUNCTION_SIGNATURE);

    return (bRet ? MM_STRDUP("true") : 0);
  }

  const char *Database::xslFunction_isHardlink(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    //true ONLY if this is a hardlink of the original
    //returns false if it is the original
    assert(pQE);
    assert(pXCtxt);
    static const char *sFunctionSignature = "database:is-hard-link([node])"; UNUSED(sFunctionSignature);

    const char *sIsHardlink = 0;
    IXmlBaseNode *pNode = 0;

    //parameters (popped in reverse order): data, stylesheet [, mode]
    UNWIND_EXCEPTION_BEGIN {
      if (pXCtxt->valueCount() > 0) pNode      = pXCtxt->popInterpretNodeFromXPathFunctionCallStack();
      else                          pNode      = pXCtxt->contextNode(pQE); //pQE as MM

      if (pNode && pNode->isHardLink()) sIsHardlink = MM_STRDUP("true");
      if (!sIsHardlink)                 sIsHardlink = MM_STRDUP("");
    } UNWIND_EXCEPTION_END;

    //clear up
    if (pNode) delete pNode;   //these docs are server freed by the context

    UNWIND_EXCEPTION_THROW;

    return sIsHardlink;
  }

  const char *Database::xslFunction_isSoftlink(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    //true ONLY if this is a hardlink of the original
    //returns false if it is the original
    assert(pQE);
    assert(pXCtxt);
    static const char *sFunctionSignature = "database:is-soft-link([node])"; UNUSED(sFunctionSignature);

    const char *sIsSoftlink = 0;
    IXmlBaseNode *pNode = 0;

    //parameters (popped in reverse order): data, stylesheet [, mode]
    UNWIND_EXCEPTION_BEGIN {
      if (pXCtxt->valueCount() > 0) pNode      = pXCtxt->popInterpretNodeFromXPathFunctionCallStack();
      else                          pNode      = pXCtxt->contextNode(pQE); //pQE as MM

      if (pNode && pNode->isHardLink()) sIsSoftlink = MM_STRDUP("true");
      if (!sIsSoftlink)                 sIsSoftlink = MM_STRDUP("");
    } UNWIND_EXCEPTION_END;

    //clear up
    if (pNode) delete pNode;   //these docs are server freed by the context

    UNWIND_EXCEPTION_THROW;

    return sIsSoftlink;
  }

  const char *Database::xslFunction_isOriginalHardlink(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    assert(pQE);
    assert(pXCtxt);
    static const char *sFunctionSignature = "database:is-original-hard-link(node)"; UNUSED(sFunctionSignature);

    const char *sIsOriginalHardLink = 0;
    IXmlBaseNode *pNode = 0;

    //parameters (popped in reverse order): data, stylesheet [, mode]
    UNWIND_EXCEPTION_BEGIN {
      if (pNode = pXCtxt->popInterpretNodeFromXPathFunctionCallStack()) {
        if (pNode->isOriginalHardLink()) sIsOriginalHardLink = MM_STRDUP("true");
      }
      if (!sIsOriginalHardLink) sIsOriginalHardLink = MM_STRDUP("false");
    } UNWIND_EXCEPTION_END;

    //clear up
    //if (pNode) delete pNode->document();   //these docs are server freed by the context

    UNWIND_EXCEPTION_THROW;

    return sIsOriginalHardLink;
  }

  const char *Database::xslFunction_xpathToNode(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    //caller frees result
    static const char *sFunctionSignature = "database:xpath-to-node([node, base-node, force-base-node, enable-ids])"; UNUSED(sFunctionSignature);
    //  @node:
    //  @disable-ids: path, rather than id link
    //  @base-node:   start from this node (must be ancestor!)
    assert(pQE);
    assert(pXCtxt);

    const char *sXPathToNode = 0;
    IXmlBaseNode *pNode      = 0,
                 *pBaseNode  = 0;
    bool bEnableIds          = false,
         bForceBaseNode      = NO_FORCE_BASE_NODE;

    UNWIND_EXCEPTION_BEGIN {
      //all optional
      switch (pXCtxt->valueCount()) {
        case 4: bEnableIds     = pXCtxt->popInterpretBooleanValueFromXPathFunctionCallStack(); ATTRIBUTE_FALLTHROUGH;
        case 3: bForceBaseNode = pXCtxt->popInterpretBooleanValueFromXPathFunctionCallStack(); ATTRIBUTE_FALLTHROUGH;
        case 2: pBaseNode      = pXCtxt->popInterpretNodeFromXPathFunctionCallStack();         ATTRIBUTE_FALLTHROUGH;
        case 1: pNode          = pXCtxt->popInterpretNodeFromXPathFunctionCallStack(); break;
        case 0: pNode          = pXCtxt->contextNode(pQE); break; //pQE as MM
        default: throw XPathTooManyArguments(this, FUNCTION_SIGNATURE);
      }
      if (bEnableIds) Debug::warn("%s @enable-ids is true: this is not advised", MM_STRDUP(sFunctionSignature));

      if (pNode) sXPathToNode = pNode->uniqueXPathToNode(pQE, pBaseNode, INCLUDE_TRANSIENT, bForceBaseNode, bEnableIds);
      if (!sXPathToNode) sXPathToNode = MM_STRDUP("");
    } UNWIND_EXCEPTION_END;

    //clear up
    //if (pNode)      delete pNode->document();   //these docs are server freed by the context
    if (pNode)      delete pNode;
    if (pBaseNode)  delete pBaseNode;

    UNWIND_EXCEPTION_THROW;

    return sXPathToNode;
  }

  const char *Database::xslFunction_rx(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    //caller frees result
    //rx(@text-stream, @rx-node [,@output-node])
    //  @text-stream:     text to rx
    //  @rx-node:       transform of text stream
    //  @output-node:     optional direct output node, will return the nodes otherwise
    static const char *sFunctionSignature = "database:rx(text-stream, rx [, output])";
    assert(pQE);
    assert(pNodes);
    assert(pXCtxt);

    XmlNodeList<const IXmlBaseNode> *pNewNodes = *pNodes;
    const char *sTextStream   = 0;
    const IXmlBaseNode *pRegularXNode = 0;
    IXmlBaseNode *pOutputNode     = 0;

    //parameters (popped in reverse order): data, stylesheet [, mode]
    UNWIND_EXCEPTION_BEGIN {
      if (pXCtxt->valueCount() == 3) {
        //output directly in to the DOM
        //no return
        pOutputNode = pXCtxt->popInterpretNodeFromXPathFunctionCallStack();
        if (!pOutputNode) throw XPathReturnedEmptyResultSet(this, MM_STRDUP("Output node set empty"), FUNCTION_SIGNATURE);
      } else {
        //return nodes for the next function to work with
        NOT_COMPLETE(""); //Database::xslFunction_rx() return nodes for the next function to work with
        pNewNodes = new XmlNodeList<const IXmlBaseNode>(this);
      }

      if (pRegularXNode = pXCtxt->popInterpretNodeFromXPathFunctionCallStack()) {
        if (sTextStream = pXCtxt->popInterpretCharacterValueFromXPathFunctionCallStack()) {
          if (pOutputNode) pOutputNode->removeAndDestroyChildren(pQE);
          RegularX::rx(pQE, pRegularXNode, sTextStream, pOutputNode);
        } else throw XPathStringArgumentRequired(this, MM_STRDUP("Text Stream paramter required"));
      } else throw XPathReturnedEmptyResultSet(this, MM_STRDUP("RegularX node set empty"), FUNCTION_SIGNATURE);
    } UNWIND_EXCEPTION_END;

    //free up
    if (pRegularXNode)   delete pRegularXNode;
    if (pOutputNode)     delete pOutputNode;
    if (sTextStream)     MMO_FREE(sTextStream);
    UNWIND_DELETE_IF_EXCEPTION(pNewNodes);

    UNWIND_EXCEPTION_THROW;

    return 0;
  }

  const char *Database::xslFunction_parseHTML(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    //caller frees result
    static const char *sFunctionSignature = "database:parse-html(text-stream, rx [, output])";
    assert(pQE);
    assert(pXCtxt);

    XmlNodeList<const IXmlBaseNode> *pNewNodes = 0;
    IXmlBaseNode *pOutputNode = 0,
                 *pTmpNode    = 0,
                 *pNode       = 0;
    const char *sHTML          = 0;

    //parameters (popped in reverse order): data, stylesheet [, mode]
    UNWIND_EXCEPTION_BEGIN {
      if (pXCtxt->valueCount() == 2) {
        //output directly in to the DOM
        //no return
        pOutputNode = pXCtxt->popInterpretNodeFromXPathFunctionCallStack();
        if (!pOutputNode) throw XPathReturnedEmptyResultSet(this, MM_STRDUP("Output node set empty"), FUNCTION_SIGNATURE);
      } else {
        //return nodes for the next function to work with
        //seems to create them in the wrong doc alias?
        NOT_COMPLETE(""); //Database::xslFunction_parseHTML() return nodes for the next function to work with
        pNewNodes   = new XmlNodeList<const IXmlBaseNode>(this);
        pOutputNode = createTmpNode();
        pTmpNode    = pOutputNode;
      }

      //pop this string off the xpath ctxt stack of input variables...
      if (sHTML = pXCtxt->popInterpretCharacterValueFromXPathFunctionCallStack()) {
        pNode = pOutputNode->parseHTMLInContextNoAddChild(pQE, sHTML);
        if (pNewNodes) pNewNodes->push_back(pNode);
        else {
          //this is a replace
          pOutputNode->removeAndDestroyChildren(pQE);
          pOutputNode->moveChild(pQE, pNode);
          delete pNode;
        }
      } else throw XPathStringArgumentRequired(this, MM_STRDUP("xml"));
    } UNWIND_EXCEPTION_END;

    //free up
    if (sHTML)      MMO_FREE(sHTML);
    if (!pTmpNode) delete pOutputNode; //temp nodes are handled by the Database
    UNWIND_CONTAINER_DESTROY_AND_DELETE_IF_EXCEPTION(pNewNodes);

    UNWIND_EXCEPTION_THROW;

    *pNodes = pNewNodes;
    return 0;
  }

  const char *Database::xslFunction_setNamespace(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    //caller frees result
    //nodes are NOT copied. they are directly changed in-situ
    //no result
    static const char *sFunctionSignature = "database:set-namespace(node-set, new-output-namespace)";
    assert(pQE);
    assert(pXCtxt);

    XmlNodeList<IXmlBaseNode> *pInputNodes = 0;
    XmlNodeList<IXmlBaseNode>::iterator iNode;
    const char *sNewNamespace = 0;
    IXmlNamespaced *pNode     = 0;

    //parameters (popped in reverse order): data, stylesheet [, mode]
    UNWIND_EXCEPTION_BEGIN {
      switch (pXCtxt->valueCount()) {
        case 2: sNewNamespace = pXCtxt->popInterpretCharacterValueFromXPathFunctionCallStack(); ATTRIBUTE_FALLTHROUGH;
        case 1: pInputNodes   = pXCtxt->popInterpretNodeListFromXPathFunctionCallStack();
          break;
        default: throw XPathArgumentCountWrong(this, FUNCTION_SIGNATURE);
      }

      if (pInputNodes && sNewNamespace && *sNewNamespace) {
        for (iNode = pInputNodes->begin(); iNode != pInputNodes->end(); iNode++) {
          pNode = (*iNode)->queryInterface((IXmlNamespaced*) 0);
          pNode->setNamespace(sNewNamespace);
        }
      }

    } UNWIND_EXCEPTION_END;

    //free up
    if (sNewNamespace) MMO_FREE(sNewNamespace);
    UNWIND_CONTAINER_DESTROY_AND_DELETE_IF_EXCEPTION(pInputNodes);

    UNWIND_EXCEPTION_THROW;

    *pNodes = (XmlNodeList<const IXmlBaseNode>*) pInputNodes;
    return 0;
  }

  const char *Database::xslFunction_copyOf(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    //caller frees result
    //like xsl:copy-of but a function useful for direct xsl:variables
    //aimed at moving multiple @options around in to output
    //  @deep-clone does a recursive copy of the sub-trees
    //  @new-output-namespace optional will set the namespace of the output nodes. NOT affect the original nodes
    //  @evaluate-values will evaluate the singular {contents} in the case of only child::text() or @*
    //TODO: do we need to remove and destroy these after use?
    //  how does exsl:node-set do this?
    static const char *sFunctionSignature = "database:copy-of(node-set [, deep-clone, new-output-namespace, evaluate-values])";
    assert(pQE);
    assert(pXCtxt);

    XmlNodeList<IXmlBaseNode> *pNewNodes   = 0;
    XmlNodeList<IXmlBaseNode> *pInputNodes = 0;
    XmlNodeList<IXmlBaseNode>::iterator iNode;
    const char *sNewOutputNamespace = 0;
    IXmlNamespaced *pOutputNode = 0;
    bool bDeepClone = false;
    bool bEvaluateValues = false;

    //parameters (popped in reverse order): data, stylesheet [, mode]
    UNWIND_EXCEPTION_BEGIN {
      switch (pXCtxt->valueCount()) {
        case 4: bEvaluateValues     = pXCtxt->popInterpretBooleanValueFromXPathFunctionCallStack(); ATTRIBUTE_FALLTHROUGH;
        case 3: sNewOutputNamespace = pXCtxt->popInterpretCharacterValueFromXPathFunctionCallStack(); ATTRIBUTE_FALLTHROUGH;
        case 2: bDeepClone          = pXCtxt->popInterpretBooleanValueFromXPathFunctionCallStack(); ATTRIBUTE_FALLTHROUGH;
        case 1: pInputNodes         = pXCtxt->popInterpretNodeListFromXPathFunctionCallStack();
          break;
        default: throw XPathArgumentCountWrong(this, FUNCTION_SIGNATURE);
      }

      if (pInputNodes) {
        pNewNodes = pInputNodes->copyNodes(pQE, bDeepClone, bEvaluateValues);
        if (sNewOutputNamespace) {
          for (iNode = pNewNodes->begin(); iNode != pNewNodes->end(); iNode++) {
            pOutputNode = (*iNode)->queryInterface((IXmlNamespaced*) 0);
            pOutputNode->setNamespace(sNewOutputNamespace);
          }
        }
      }

    } UNWIND_EXCEPTION_END;

    //free up
    if (pInputNodes)         delete pInputNodes->element_destroy();
    if (sNewOutputNamespace) MMO_FREE(sNewOutputNamespace);
    UNWIND_CONTAINER_DESTROY_AND_DELETE_IF_EXCEPTION(pNewNodes);

    UNWIND_EXCEPTION_THROW;

    *pNodes = (XmlNodeList<const IXmlBaseNode>*) pNewNodes;
    return 0;
  }

  const char *Database::xslFunction_similarity(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    //caller frees result
    static const char *sFunctionSignature = "database:similarity(node [,node])";
    assert(pQE);
    assert(pXCtxt);

    percentage iSimilarity = 0;
    IXmlBaseNode *pFirstNode  = 0;
    IXmlBaseNode *pSecondNode = 0;

    //parameters (popped in reverse order): data, stylesheet [, mode]
    UNWIND_EXCEPTION_BEGIN {
      switch (pXCtxt->valueCount()) {
        case 2: {
          pSecondNode = pXCtxt->popInterpretNodeFromXPathFunctionCallStack();
          pFirstNode  = pXCtxt->popInterpretNodeFromXPathFunctionCallStack();
          break;
        }
        case 1: {
          pFirstNode  = pXCtxt->contextNode(pQE); //pQE as MM
          pSecondNode = pXCtxt->popInterpretNodeFromXPathFunctionCallStack();
          break;
        }
        default: throw XPathArgumentCountWrong(this, FUNCTION_SIGNATURE);
      }

      if (pFirstNode && pSecondNode) iSimilarity = pFirstNode->similarity(pQE, pSecondNode);
    } UNWIND_EXCEPTION_END;

    //free up
    if (pFirstNode)  delete pFirstNode;
    if (pSecondNode) delete pSecondNode;

    UNWIND_EXCEPTION_THROW;

    return utoa(iSimilarity);
  }

  IXmlBaseNode *Database::xslCommand_differences(const IXmlQueryEnvironment *pQE, const IXslCommandNode *pCommandNode, const IXmlBaseNode *pSourceNode, IXmlBaseNode *pOutputNode) {
    //caller frees result
    static const char *sCommandSignature = "database:differences new=\"node\" [existing=\"node\"]"; UNUSED(sCommandSignature);

    const IXmlBaseNode *pExisting = 0;
    const IXmlBaseNode *pNew      = 0;
    const IXmlBaseNode *pCommandNodeType = 0;
    const TXml *pTXml;
    vector<TXml*> *pvTXmls  = 0;
    vector<TXml*>::iterator iTXml;
    IXmlBaseNode *pTXmlSpecNode;
    
    DISABLE_DATABASE_XSL_EXTENSIONS;
    if (!pOutputNode) throw NoOutputRootNode(this, MM_STRDUP("request"));

    UNWIND_EXCEPTION_BEGIN {
      pCommandNodeType = pCommandNode->queryInterface((IXmlBaseNode*) 0);
      pNew = pCommandNodeType->attributeValueNode( pQE, "new", NAMESPACE_NONE, pSourceNode);
      if (!pNew) throw AttributeRequired(this, MM_STRDUP("new"), MM_STRDUP(sCommandSignature));
      
      //optional parameters
      pExisting = pCommandNodeType->attributeValueNode( pQE, "existing", NAMESPACE_NONE, pSourceNode);
      if (!pExisting) pExisting = pSourceNode;
      
      pvTXmls = pExisting->differences(pQE, pNew);
      for (iTXml = pvTXmls->begin(); iTXml != pvTXmls->end(); iTXml++) {
        pTXml         = *iTXml;
        pTXmlSpecNode = pTXml->toNode(pOutputNode);
        delete pTXmlSpecNode;
      }

    } UNWIND_EXCEPTION_END;

    //------------------------------------- free up
    if (pNew && pNew != pSourceNode) delete pNew;
    if (pExisting) delete pExisting;
    if (pvTXmls)   vector_element_destroy(pvTXmls);

    UNWIND_EXCEPTION_THROW;

    return 0;
  }
  
  const char *Database::xslFunction_UUID(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    return newUUID();
  }

  const char *Database::xslFunction_parseXML(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    //caller frees result
    static const char *sFunctionSignature = "database:parse-xml(text-stream, [, output, throw-on-fail])";
    assert(pQE);
    assert(pXCtxt);

    XmlNodeList<const IXmlBaseNode> *pNewNodes = 0;
    IXmlBaseNode *pOutputNode = 0,
                 *pTmpNode    = 0,
                 *pNewNode    = 0;
    const char *sXML          = 0;
    bool bDirectOutputToDOM   = false,
         bThrowOnError         = false;

    //parameters (popped in reverse order): data, stylesheet [, mode]
    UNWIND_EXCEPTION_BEGIN {
      switch (pXCtxt->valueCount()) {
        case 3:
          bThrowOnError = pXCtxt->popInterpretBooleanValueFromXPathFunctionCallStack();
          ATTRIBUTE_FALLTHROUGH;
        case 2:
          //output directly in to the DOM
          //no return nodes
          pOutputNode = pXCtxt->popInterpretNodeFromXPathFunctionCallStack();
          if (!pOutputNode) throw XPathReturnedEmptyResultSet(this, MM_STRDUP("Output node set empty"), FUNCTION_SIGNATURE);
          bDirectOutputToDOM = true;
          break;
        case 1:
          //1 = xml string
          //return nodes for the next function to work with
          //parse them in to the temp area
          //bDirectOutputToDOM = false
          pOutputNode = createTmpNode();
          break;
        default: throw XPathTooManyArguments(this, FUNCTION_SIGNATURE);
      }
      //XML string
      try {
        if (sXML = pXCtxt->popInterpretCharacterValueFromXPathFunctionCallStack()) {
          pNewNode = pOutputNode->parseXMLInContextNoAddChild(pQE, sXML);
        } else throw XPathStringArgumentRequired(this, MM_STRDUP("xml"));
      }
      catch (InvalidXML &eb) {
        IFDEBUG(if (!bThrowOnError) Debug::reportObject(&eb);)
        if (bThrowOnError) throw;
      }
      catch (ExceptionBase &eb) {
        IFDEBUG(if (!bThrowOnError) Debug::reportObject(&eb);)
        if (bThrowOnError) throw;
      }

      //output
      if (pNewNode) {
        if (bDirectOutputToDOM) {
          //this is a replace
          //xmlIDMainIndexDocument(pNewNode); //@xml:id is done by the moveChild() call
          //we translate the structure here because we want it in-place, in the same doc first
          pOutputNode->removeAndDestroyChildren(pQE);    //frees all the @xml:id
          pOutputNode->moveChild(pQE, pNewNode); //re-@xml:id the content
        } else {
          //returning the nodes instead
          //seems to create them in the wrong doc alias?
          NEEDS_TEST("database:parse-xml(text-stream)"); //Database::xslFunction_parseXML() return nodes for the next function to work with
          pOutputNode->moveChild(pQE, pNewNode); //re-@xml:id the content
          pNewNodes   = new XmlNodeList<const IXmlBaseNode>(this);
          pNewNodes->push_back(pNewNode);
          *pNodes     = pNewNodes;
        }
      }
    } UNWIND_EXCEPTION_END;

    //free up
    if (sXML)        MMO_FREE(sXML);
    if (bDirectOutputToDOM && pNewNode)    delete pNewNode;
    if (bDirectOutputToDOM && pOutputNode) delete pOutputNode; //TODO: ?? temporary nodes are garbage managed by the DB
    UNWIND_CONTAINER_DESTROY_AND_DELETE_IF_EXCEPTION(pNewNodes);

    UNWIND_EXCEPTION_THROW;

    return 0;
  }

  const char *Database::xslFunction_transaction(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    TXml::transactionResult tr;
    IXmlBaseNode *pSerialisationNode;
    XmlNodeList<const IXmlBaseNode> *pNewNodes = 0;

    //create the Transaction with all paths and variables serialised
    //and the security context serialised to the users guid
    //need context information from the current transform
    TXml *t = 0;

    UNWIND_EXCEPTION_BEGIN {
      t = TXml::factory(this, pQE, this, pXCtxt);
      Debug::reportObject(t);
      //createTmpNode() will throw up if it fails
      if (pSerialisationNode = createTmpNode()) {
        //apply the serialised transaction in the same way that it will be in any context
        //apart from the database to run it on of course
        //i.e. dont send through any objects or anything that holds state
        //TODO: speed up this user UUID lookup
        pNewNodes = new XmlNodeList<const IXmlBaseNode>(this);
        tr = applyTransaction(t, pSerialisationNode, pQE);
        if (tr == TXml::transactionFailure) {
          //TODO: TXml::transactionFailure
        }
        pNewNodes->push_back(pSerialisationNode);
      }
    } UNWIND_EXCEPTION_END;

    //free up
    if (t) delete t;
    //if (pSerialisationNode) delete pSerialisationNode; //tmp nodes are handled by the Database
    UNWIND_CONTAINER_DESTROY_AND_DELETE_IF_EXCEPTION(pNewNodes);
    
    UNWIND_EXCEPTION_THROW;

    *pNodes = pNewNodes;
    return 0;
  }

  const char *Database::xslFunction_documentOrder(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    //caller frees result
    //usage:
    //  <xsl:sort select="database:document-order()" data-type="number" />
    static const char *sFunctionSignature = "database:document-order([node])";
    assert(pQE);
    assert(pXCtxt);

    unsigned long ulDocumentOrder      = 0;
    const char   *sDocumentOrder       = 0;
    IXmlBaseNode *pInputNode           = 0;

    //parameters (popped in reverse order): data, stylesheet [, mode]
    UNWIND_EXCEPTION_BEGIN {
      //optional
      switch (pXCtxt->valueCount()) {
        case 1: {
          pInputNode = pXCtxt->popInterpretNodeFromXPathFunctionCallStack();
          break;
        }
        case 0: {
          pInputNode = pXCtxt->contextNode(pQE); //pQE as MM
          break;
        }
        default: throw XPathTooManyArguments(this, FUNCTION_SIGNATURE);
      }

      if      (!pInputNode)                  throw XPathFunctionWrongArgumentType(this, MM_STRDUP("context node"), MM_STRDUP("no context node"), FUNCTION_SIGNATURE);
      else if (!pInputNode->isNodeElement()) throw XPathFunctionWrongArgumentType(this, MM_STRDUP("context node"), MM_STRDUP("input node not an element"), FUNCTION_SIGNATURE);
      else {
        ulDocumentOrder      = pInputNode->documentOrder();
        sDocumentOrder       = ultoa(ulDocumentOrder);
      }
    } UNWIND_EXCEPTION_END;

    //free up
    if (pInputNode)     delete pInputNode;

    UNWIND_EXCEPTION_THROW;

    return sDocumentOrder ? sDocumentOrder : MM_STRDUP("0");
  }

  IXmlBaseNode *Database::xslCommand_query(const IXmlQueryEnvironment *pQE, const IXslCommandNode *pCommandNode, const IXmlBaseNode *pSourceNode, IXmlBaseNode *pOutputNode) {
    //server-side XSLT has atempted to process a database:query command
    //assume that the intention is to copy the database:query to the output stream
    //this would have the effect that the client would deal with it
    //which is probably an AJAX client side request
    const IXmlBaseNode *pCommandNodeType;

    Debug::report("database:query is a server-side thang");

    if (pCommandNodeType = pCommandNode->queryInterface((const IXmlBaseNode*) 0)) {
      pOutputNode->copyChild(pQE, pCommandNodeType, DEEP_CLONE);
    }

    return 0;
  }

  IXmlBaseNode *Database::xslCommand_selectInputNode(const IXmlQueryEnvironment *pQE, const IXslCommandNode *pCommandNode, const IXmlBaseNode *pSourceNode, IXmlBaseNode *pOutputNode) {
    const IXmlBaseNode *pSelectNode, *pCommandNodeType;

    if (pCommandNodeType = pCommandNode->queryInterface((const IXmlBaseNode*) 0)) {
      pSelectNode = pCommandNodeType->attributeValueNode(pQE, "select"); //(or child doc!)
      if (!pSelectNode) pSelectNode = pSourceNode->clone_with_resources();

      pQE->addNode(pSelectNode);
    }

    return 0;
  }

  IXmlBaseNode *Database::xslCommand_transaction(const IXmlQueryEnvironment *pQE, const IXslCommandNode *pCommandNode, const IXmlBaseNode *pSourceNode, IXmlBaseNode *pOutputNode) {
    TXml::transactionResult tr;
    IXmlBaseNode *pSerialisationNode;

    //create the Transaction with all paths and variables serialised
    //and the security context serialised to the users guid
    //need context information from the current transform
    TXml *t = TXml::factory(this, pQE, this, pCommandNode->queryInterface((IXmlBaseNode*) 0));
    //Debug::reportObject(t);

    //createTmpNode() will throw up if it fails
    if (pSerialisationNode = createTmpNode()) {
      //apply the serialised transaction in the same way that it will be in any context
      //apart from the database to run it on of course
      //i.e. dont send through any objects or anything that holds state
      //TODO: speed up this user UUID lookup
      tr = applyTransaction(t, pSerialisationNode, pQE);
      if (tr == TXml::transactionFailure) {
        //TODO: TXml::transactionFailure
      }
    }

    //free up
    delete t;
    //if (pSerialisationNode) delete pSerialisationNode; //tmp nodes are handled by the Database

    return 0;
  }

  IXmlBaseNode *Database::xslCommand_hardlinkInfo(const IXmlQueryEnvironment *pQE, const IXslCommandNode *pCommandNode, const IXmlBaseNode *pSourceNode, IXmlBaseNode *pOutputNode) {
    //caller frees result
    DISABLE_DATABASE_XSL_EXTENSIONS;
    if (!pOutputNode) throw NoOutputRootNode(this, MM_STRDUP("request"));
    else pSourceNode->createHardlinkPlaceholderIn(pQE, pOutputNode);
    return 0;
  }

  IXmlBaseNode *Database::xslCommand_deviationInfo(const IXmlQueryEnvironment *pQE, const IXslCommandNode *pCommandNode, const IXmlBaseNode *pSourceNode, IXmlBaseNode *pOutputNode) {
    //caller frees result
    DISABLE_DATABASE_XSL_EXTENSIONS;
    if (!pOutputNode) throw NoOutputRootNode(this, MM_STRDUP("request"));
    else pSourceNode->createDeviationPlaceholdersIn(pQE, pOutputNode);
    return 0;
  }

  IXmlBaseNode *Database::xslCommand_transform(const IXmlQueryEnvironment *pQE, const IXslCommandNode *pCommandNode, const IXmlBaseNode *pSourceNode, IXmlBaseNode *pOutputNode) {
    //caller frees result
    static const char *sCommandSignature = "database:transform stylesheet=\"node\" [node-mask=\"\" select=\"\" interface-mode=\"\" include-directory=\"\" use-parent-context=\"\" additional-params-node=\"\"] />"; UNUSED(sCommandSignature);

    DISABLE_DATABASE_XSL_EXTENSIONS;
    if (!pOutputNode) throw NoOutputRootNode(this, MM_STRDUP("request"));

    //transform something directly in to the output stream
    //if you want to transform something somewhere else then use move-child with @transform

    //inputs to this transform() call
    //pCommandNode: e.g. <Database:move-child>
    //pSourceNode:  e.g. <repository:index_xsl> if the transform is currently traversing the process_main_stylesheet
    //pOutputNode:  e.g. <root>
    //m_pNode:     the Database root node, often <repository:db_xml>

    //attributes for this transform() call
    //@stylesheet:     to the current source doc under @destination (required), usually the Database::m_pNode
    //@select:         what to transform (optional: defaults to .)
    //@interface-mode: transform mode
    //@node-mask:      flexible mask to additionally apply during the transform

    const char *sSelect                   = 0,
               *sWithMode                 = 0;

    const IXmlBaseNode *pCommandNodeType  = 0,
                       *pStylesheetNode   = 0,
                       *pFromNode         = 0,
                       *pIncludeDirectory = 0;
    XmlNodeList<const IXmlBaseNode> *pFromNodes = 0;
    XmlNodeList<const IXmlBaseNode>::iterator iFromNode;

    bool bUseParentContext;
    BrowserQueryEnvironment *pQEAnonymous = 0;

    const char *sNodeMask                 = 0;
    IXmlArea *pArea                       = 0;
    IXmlNodeMask *pNodeMask               = 0;

    const char *sAdditionalParamsXPath              = 0;
    const IXmlBaseNode *pParamsNode       = 0;
    XmlNodeList<const IXmlBaseNode> *pChildDirectives = 0;
    XmlNodeList<const IXmlBaseNode>::iterator iChildDirective;
    const IXmlBaseNode *pDirective        = 0;
    StringMap<XmlNodeList<IXmlBaseNode>*> mParamsNodeSet;
    StringMap<const char*>                mParamsChar;
    StringMap<const size_t>               mParamsInt;
    StringMap<XmlNodeList<IXmlBaseNode>*>::iterator iNodeSet;
    const char *sParamDashName = 0,
               *sParamName     = 0,
               *sParamValue    = 0;

    XmlNodeList<IXmlBaseNode> *pNodeList;
    char *sResult;
    int   iResult;
    bool  bResult;
    iXPathType iType;
    bool bOldEnabled = true;

    UNWIND_EXCEPTION_BEGIN {
      pCommandNodeType = pCommandNode->queryInterface((IXmlBaseNode*) 0);
      if (pStylesheetNode = pCommandNodeType->attributeValueNode(pQE, "stylesheet")) {
        //optional parameters
        sSelect                = pCommandNodeType->attributeValueDynamic(  pQE, "select"); //(or child doc!)
        sWithMode              = pCommandNodeType->attributeValueDynamic(  pQE, "interface-mode");
        pIncludeDirectory      = pCommandNodeType->attributeValueNode(     pQE, "include-directory");
        pNodeMask              = pCommandNodeType->attributeValueNodeMask( pQE, "node-mask");
        bUseParentContext      = pCommandNodeType->attributeValueBoolDynamicString(pQE, "use-parent-context", NAMESPACE_NONE, true);
        sAdditionalParamsXPath = pCommandNodeType->attributeValueDynamic(  pQE, "additional-params-node");

        //------------------------------------- optional @params-node child directives
        if (sAdditionalParamsXPath)  {
          if (_STREQUAL(sAdditionalParamsXPath, ".")) pParamsNode = pSourceNode; //common optimisation :)
          else pParamsNode = pSourceNode->getSingleNode(pQE, sAdditionalParamsXPath);
        }
        if (pParamsNode) {
          pChildDirectives     = pParamsNode->children(pQE);
          for (iChildDirective = pChildDirectives->begin(); iChildDirective != pChildDirectives->end(); iChildDirective++) {
            pDirective     = *iChildDirective;
            sParamDashName = pDirective->localName();
            if (pDirective->isNamespace(NAMESPACE_META)) {
              Debug::report("meta param ignored: [%s]", sParamDashName);
            } else {
              sParamValue = pDirective->value(pQE);
              sParamName  = strreplace(sParamDashName, '-', '_');
              if (sParamName != sParamDashName) MMO_FREE(sParamDashName);

              if (sParamName && sParamValue) {
                iType = TYPE_STRING;
                mParamsChar.insert(sParamName, sParamValue);
                Debug::report("param added: [%s] => (forced as string because from additional-params-node: %s) [%s]", sParamName, iType, sParamValue);
              } else {
                if (sParamValue) MMO_FREE(sParamValue);
                if (sParamName)  MMO_FREE(sParamName);
              }
            }
          }
          //free up
          vector_element_destroy(pChildDirectives);
        }

        //------------------------------------- optional <with-param> child fixed directives (if there is no child doc)
        //these take precedence over the @params-node settings
        //TODO: memory release during overwrite
        if (sSelect) {
          pChildDirectives     = pCommandNodeType->children(pQE);
          for (iChildDirective = pChildDirectives->begin(); iChildDirective != pChildDirectives->end(); iChildDirective++) {
            pDirective = *iChildDirective;
            if (_STREQUAL(pDirective->localName(NO_DUPLICATE), "with-param")) {
              sParamName  = pDirective->attributeValue(pQE, "name");
              sParamValue = pDirective->attributeValue(pQE, "select");
            }

            if (sParamName && sParamValue) {
              //not sure about the type being passed through!
              pDirective->getObject(pQE, sParamValue, &pNodeList, &sResult, &iResult, &bResult, &iType);
              switch (iType) {
                case TYPE_XSLT_TREE:
                case TYPE_NODESET: mParamsNodeSet.insert(sParamName, pNodeList); break;
                case TYPE_BOOLEAN: mParamsInt.insert(    sParamName, bResult);   break;
                case TYPE_NUMBER:  mParamsInt.insert(    sParamName, iResult);   break;
                case TYPE_STRING:  mParamsChar.insert(   sParamName, sResult);   break;
                default: throw XPathTypeUnhandled(this, iType);
              }
              //Debug::report("param added: [%s] => (%s) [%s]", sParamName, iType, sParamValue);
            }

            //free up
            if (sParamValue) MMO_FREE(sParamValue);
          }
          //free up
          vector_element_destroy(pChildDirectives);
        }

        //------------------------------------- start point
        if (sSelect) pFromNodes = pSourceNode->getMultipleNodes(pQE, sSelect);
        else         pFromNodes = new XmlNodeList<const IXmlBaseNode>(pSourceNode->clone_with_resources());
        if (pFromNodes->size() == 0) throw XPathReturnedEmptyResultSet(this, MM_STRDUP("Data not found or returned nothing"), sSelect);

        //QE
        //using a direct output node here so the result doc will be 0x0
        if (bUseParentContext) {
          //@use-parent-context="yes" (default)
          //QueryEnvironment security and values will be inherited from parent transform
        } else {
          //@use-parent-context="no"
          //new virgin transform with explicit security (e.g. server side transforms)
          //security = anonymous, will throw errors on attempted login
          //no EMO extensions
          pQE = pQEAnonymous = new BrowserQueryEnvironment(this, pSourceNode->document());

          //sanity checks
          //browser @use-parent-context="no" mimmic should not have parameters / modes
          if (mParamsNodeSet.size()) Debug::report("cannot use pParamsChar with @use-parent-context=no", rtWarning);
          if (sWithMode)   Debug::report("cannot use sWithMode with @use-parent-context=no",   rtWarning);
          if (pNodeMask)   Debug::report("cannot use pNodeMask with @use-parent-context=no",   rtWarning);
        }

        if (pNodeMask) {
          pArea = xmlLibrary()->factory_area(this, pFromNodes, pQE, pNodeMask);
          if (pQE->maskContext()) {
            if (pArea) pQE->maskContext()->push_back(pArea);
            bOldEnabled = pQE->maskContext()->enable();
          }
          else throw MaskContextNotAvailableForArea(this);
        }

        //------------------------------------- transform: each node in the list
        //TODO: in future we should re-code LibXML2 to handle a ctxt->initialContextNodeList
        //  with a XmlLibrary->canTransformMultipleRoots()?
        for (iFromNode = pFromNodes->begin(); iFromNode != pFromNodes->end(); iFromNode++) {
          pFromNode = *iFromNode;

          //main transform, usually direct to primary output document inherited from parent context
          pFromNode->transform(
            pStylesheetNode->queryInterface((const IXslStylesheetNode*) 0),
            pQE,
            pOutputNode,
            &mParamsInt, &mParamsChar, (StringMap<const XmlNodeList<const IXmlBaseNode>*>*) &mParamsNodeSet,
            sWithMode, pIncludeDirectory
          );
        }
        
        if (pQE->maskContext() && pArea)        pQE->maskContext()->pop_back();
        if (pQE->maskContext() && !bOldEnabled) pQE->maskContext()->disable();
      } else {
        throw XPathReturnedEmptyResultSet(this, 
          MM_STRDUP("Stylesheet not found: "), pCommandNodeType->attributeValue(pQE, "stylesheet")
        );
      }
    } UNWIND_EXCEPTION_END;

    //------------------------------------- free up
    for (iNodeSet = mParamsNodeSet.begin(); iNodeSet != mParamsNodeSet.end(); iNodeSet++) {
      iNodeSet->second->element_destroy();
      delete iNodeSet->second;
      MMO_FREE(iNodeSet->first);
    }
    mParamsChar.elements_free();
    mParamsInt.key_free();
    if (sSelect)          MMO_FREE(sSelect);
    if (sWithMode)        MMO_FREE(sWithMode);
    if (pIncludeDirectory) delete pIncludeDirectory;
    if (sNodeMask)        MMO_FREE(sNodeMask);
    if (pParamsNode != pSourceNode) delete pParamsNode;
    //if (pChildDirectives) delete pChildDirectives; //done by vector_element_destroy()
    if (sAdditionalParamsXPath) MMO_FREE(sAdditionalParamsXPath);
    if (pStylesheetNode)  delete pStylesheetNode;
    if (pFromNodes)       delete pFromNodes->element_destroy();
    if (pArea)            delete pArea;
    if (pNodeMask)        delete pNodeMask;
    if (pQEAnonymous)     delete pQEAnonymous;

    UNWIND_EXCEPTION_THROW;

    return 0;
  }

  ITDF_RETURN Database::moveChild(ITDF_SECURITY_PARAMS, ITDF_BASE_PARAMS, const IXmlBaseNode *pTSelectNode, const char *sDestinationXPath, const char *sBeforeXPath, const int iDestinationPosition, ITDF_TRANSFORM_PARAMS,
                                        ITDF_RESULT_PARAMS) {
    IXmlBaseNode
      *pBaseNode            = 0,
      *pSelectNode          = 0,
      *pDestinationNode     = 0,
      *pBeforeNode          = 0,
      *pResultNode          = 0,
      *pTransformNode       = 0,
      *pResultParentNode    = 0,
      *pTmpTXmlCopy         = 0;
    const IXmlBaseNode *pConstSourceNode = 0;
    StringMap<const char*> *pParamsChar = 0;
    const DatabaseNode *pUserStore = 0;

    //security
    IXmlQueryEnvironment *pQE = newQueryEnvironment();
    if (sReadOPUserUUIDForTransform) pQE->securityContext()->login(sReadOPUserUUIDForTransform);

    //base for xpath resolutions
    if (sBaseXPath) pBaseNode = m_pDoc->getSingleNode(pQE, sBaseXPath);
    else            pBaseNode = m_pDoc->documentNode(); //all xpath will be absolute anyway

    if (pBaseNode) {
      if (pDestinationNode = pBaseNode->getSingleNode(pQE, sDestinationXPath)) {
        //we have a mixed const and non-const situation
        //pTSelectNode will be const because we can't affect the TXml contents (need to copy it)
        //whereas sSelectXPath can be moved and is not const
        if (sSelectXPath) pSelectNode = pBaseNode->getSingleNode(pQE, sSelectXPath);
        if (pTSelectNode) {
          //pTSelectNode is in the TXml and its hardlinks will be out-of-context xml:hardlinks
          //we need to copy it to our tmp area so that the hardlinks will resolve in this document
          pTmpTXmlCopy     = createTmpNode();
          pConstSourceNode = pSelectNode = pTmpTXmlCopy->copyChild(pQE, pTSelectNode);
      } else pConstSourceNode = pSelectNode;

        if (pConstSourceNode) {
          //optional nodes
          if (sBeforeXPath) pBeforeNode = pBaseNode->getSingleNode(pQE, sBeforeXPath);

          if (sTransformWithXPath) {
            //optional transform (direct in to destination)
            pTransformNode = pBaseNode->getSingleNode(pQE, sTransformWithXPath); //usually absolute anyway
            if (sWithParam) {
              pParamsChar = new StringMap<const char*>();
              pParamsChar->insert("with_param", sWithParam);
            }
            //TODO: pBeforeNode and iDestinationPosition
            pConstSourceNode->transform(pTransformNode->queryInterface((const IXslStylesheetNode*) 0),
              pQE,
              pDestinationNode, //(output parent)
              NO_PARAMS_INT, pParamsChar, NO_PARAMS_NODE,
              sWithMode
            );
            if (pSelectNode) {
              //non-TXml
              pSelectNode->clearMyCachedStylesheets(pQE); //pQE as MM
              pSelectNode->removeAndDestroyNode(pQE); //its a move!
            }
            //TODO: multiple nodes?
            pResultNode = pDestinationNode->lastChild(pQE, node_type_element_only);
          } else {
            //no transform, direct act on source. if it is in TXml we copy it
            //returns pSelectNode
            if (pTSelectNode) pResultNode = pDestinationNode->copyChild(pQE, pTSelectNode, DEEP_CLONE, pBeforeNode, iDestinationPosition);
            else {
              pSelectNode->clearMyCachedStylesheets(pQE); //pQE as MM
              pResultNode = pDestinationNode->moveChild(pQE, pSelectNode, pBeforeNode, iDestinationPosition);
            }
          }
          if (pResultNode) pResultNode->clearMyCachedStylesheets(pQE); //pQE as MM

          //return new node details for serialisation
          if (pResultNode) {
            //maintain index, LibXML2 will have blanked the @xml:id attribute
            //the move may be between TXml and main document and thus have no xml:ids
            xmlIDMainIndexDocument(pResultNode);
            
            if (pResultParentNode  = pResultNode->parentNode(pQE)) *sSelectParentXPath = pResultParentNode->uniqueXPathToNode(pQE, NO_BASE_NODE, INCLUDE_TRANSIENT);
            *sSelectFQNodeName     = pResultNode->fullyQualifiedName();
            *sSelectXmlId          = pResultNode->xmlID(pQE);
            *sSelectRepositoryName = pResultNode->attributeValue(pQE, "repository:name");
          }
        } else throw TXmlRequiredNodeNotFound(this, MM_STRDUP("select"), m_sAlias);
      } else throw TXmlRequiredNodeNotFound(this, MM_STRDUP("destination"), m_sAlias);
    } else throw TXmlRequiredNodeNotFound(this, MM_STRDUP("base"), m_sAlias);

    //EMO report
    if (sExtensionElementPrefixes)      *sExtensionElementPrefixes      = extensionElementPrefixes();
    if (sNewResultXslModuleManagerName) *sNewResultXslModuleManagerName = xslModuleManagerName();

    //free up
    if (ppResultNode)     *ppResultNode = pResultNode;
    else if (pResultNode)  delete pResultNode;
    if (pBaseNode)         delete pBaseNode;
    if ( pSelectNode
      && pSelectNode != pTSelectNode
      && pSelectNode != pResultNode)      delete pSelectNode;
    if (pDestinationNode)  delete pDestinationNode;
    if (pBeforeNode)       delete pBeforeNode;
    if (pResultParentNode) delete pResultParentNode;
    if (pParamsChar)       delete pParamsChar; //transforms only
    if (pUserStore)        delete pUserStore;
    if (pTransformNode)    delete pTransformNode;

    return TXml::transactionSuccess;
  }

  ITDF_RETURN Database::copyChild(ITDF_SECURITY_PARAMS, ITDF_BASE_PARAMS, const IXmlBaseNode *pTSelectNode, const char *sDestinationXPath, const char *sBeforeXPath, const int iDestinationPosition, const bool bDeepClone, ITDF_TRANSFORM_PARAMS,
                                        ITDF_RESULT_PARAMS) {
    IXmlBaseNode
      *pBaseNode            = 0,
      *pSelectNode          = 0,
      *pDestinationNode     = 0,
      *pBeforeNode          = 0,
      *pTransformNode       = 0,
      *pResultNode          = 0,
      *pResultParentNode    = 0,
      *pTmpTXmlCopy         = 0;
    const IXmlBaseNode *pConstSourceNode = 0;
    StringMap<const char*>                            mParamsChar;
    StringMap<const XmlNodeList<const IXmlBaseNode>*> mParamsNodeSet;
    const DatabaseNode *pUserStore = 0;

    //security
    IXmlQueryEnvironment *pQE = newQueryEnvironment();
    if (sReadOPUserUUIDForTransform) pQE->securityContext()->login(sReadOPUserUUIDForTransform);

    //base for xpath resolutions
    if (sBaseXPath) pBaseNode = m_pDoc->getSingleNode(pQE, sBaseXPath);
    else            pBaseNode = m_pDoc->documentNode(); //all xpath will be absolute anyway

    if (pBaseNode) {
      if (pDestinationNode = pBaseNode->getSingleNode(pQE, sDestinationXPath)) {
        //we have a mixed const and non-const situation
        //pTSelectNode will be const because we can't affect the TXml contents (need to copy it)
        //whereas sSelectXPath can be moved and is not const
        if (sSelectXPath) pSelectNode = pBaseNode->getSingleNode(pQE, sSelectXPath);
        if (pTSelectNode) {
          //pTSelectNode is in the TXml and its hardlinks will be out-of-context xml:hardlinks
          //we need to copy it to our tmp area so that the hardlinks will resolve in this document
          pTmpTXmlCopy     = createTmpNode();
          pConstSourceNode = pSelectNode = pTmpTXmlCopy->copyChild(pQE, pTSelectNode);
        } else pConstSourceNode = pSelectNode;

        if (pConstSourceNode) {
          //optional nodes
          if (sBeforeXPath) pBeforeNode = pBaseNode->getSingleNode(pQE, sBeforeXPath);

          //optional transform (direct in to destination)
          if (sTransformWithXPath) {
            pTransformNode = pBaseNode->getSingleNode(pQE, sTransformWithXPath); //usually absolute anyway
            if (sWithParam) mParamsChar.insert("with_param", sWithParam);

            XmlNodeList<const IXmlBaseNode> vServer(    this); //, server()->dbNode()->node_const());
            XmlNodeList<const IXmlBaseNode> vRequest(   this); //, m_pNode->node_const());
            XmlNodeList<const IXmlBaseNode> vSession(   this); //, m_pConversation->sessionNode());
            XmlNodeList<const IXmlBaseNode> vUser(      this); //, m_pConversation->userNode(), IFNOTNULL);
            XmlNodeList<const IXmlBaseNode> vService(   this); //, m_pConversation->service()->dbNode()->node_const());
            XmlNodeList<const IXmlBaseNode> vPrimaryStylesheet(this); //, m_pMI->primaryOutputTransformation()->node_const());
            XmlNodeList<const IXmlBaseNode> vMI(        this); //, m_pMI->dbNode()->node_const());
            
            mParamsNodeSet.insert("gs_server",      &vServer);  // /object:Server
            mParamsNodeSet.insert("gs_service",     &vService);
            mParamsNodeSet.insert("gs_session",     &vSession);
            mParamsNodeSet.insert("gs_user",        &vUser);
            mParamsNodeSet.insert("gs_request",     &vRequest);
            mParamsNodeSet.insert("gs_message_interpretation", &vMI);
            mParamsNodeSet.insert("gs_primary_stylesheet",     &vPrimaryStylesheet);            
            
            //TODO: pBeforeNode and iDestinationPosition
            pConstSourceNode->transform(pTransformNode->queryInterface((const IXslStylesheetNode*) 0),
              pQE,
              pDestinationNode, //output parent
              NO_PARAMS_INT, &mParamsChar, &mParamsNodeSet,
              sWithMode
            );
            //TODO: multiple nodes?
            pResultNode = pDestinationNode->lastChild(pQE, node_type_element_only);
          } else {
            //returns the new node
            pResultNode = pDestinationNode->copyChild(pQE, pConstSourceNode, bDeepClone, pBeforeNode, iDestinationPosition);
          }
          if (pResultNode) pResultNode->clearMyCachedStylesheets(pQE); //pQE as MM

          if (pResultNode) {
            //maintain index, LibXML2 will have blanked the @xml:id attribute
            xmlIDMainIndexDocument(pResultNode);

            //return new node details for serialisation
            if (pResultParentNode = pResultNode->parentNode(pQE)) *sSelectParentXPath = pResultParentNode->uniqueXPathToNode(pQE, NO_BASE_NODE, INCLUDE_TRANSIENT);
            *sSelectFQNodeName     = pResultNode->fullyQualifiedName();
            *sSelectXmlId          = pResultNode->xmlID(pQE);
            *sSelectRepositoryName = pResultNode->attributeValue(pQE, "repository:name");
          }
        } else throw TXmlRequiredNodeNotFound(this, MM_STRDUP("select"), m_sAlias);
      } else throw TXmlRequiredNodeNotFound(this, MM_STRDUP("destination"), m_sAlias);
    } else throw TXmlRequiredNodeNotFound(this, MM_STRDUP("base"), m_sAlias);

    //EMO report
    if (sExtensionElementPrefixes)      *sExtensionElementPrefixes      = extensionElementPrefixes();
    if (sNewResultXslModuleManagerName) *sNewResultXslModuleManagerName = xslModuleManagerName();

    //free up
    if (ppResultNode)     *ppResultNode = pResultNode;
    else if (pResultNode && pResultNode != pDestinationNode)  delete pResultNode; //result of the copy
    if (pBaseNode)         delete pBaseNode;
    if (pSelectNode)       delete pSelectNode;
    if (pDestinationNode)  delete pDestinationNode;
    if (pBeforeNode)       delete pBeforeNode;
    if (pUserStore)        delete pUserStore;
    if (pResultParentNode) delete pResultParentNode;
    if (pTransformNode)    delete pTransformNode;

    return TXml::transactionSuccess;
  }

  ITDF_RETURN Database::deviateNode(ITDF_SECURITY_PARAMS, ITDF_BASE_PARAMS, const char *sHardlinkXPath, const char *sNewNodeXPath,
                                    ITDF_RESULT_PARAMS) {
    NOT_COMPLETE("");

    /*
    IXmlBaseNode
      *pBaseNode            = 0,
      *pSelectNode          = 0,
      *pDestinationNode     = 0,
      *pResultNode          = 0,
      *pBeforeNode          = 0;

    //security
    IXmlQueryEnvironment *pQE = newQueryEnvironment();
    if (sReadOPUserUUIDForTransform) pQE->securityContext()->login(sReadOPUserUUIDForTransform);

    //base for xpath resolutions
    if (sBaseXPath) pBaseNode = m_pDoc->getSingleNode(pQE, sBaseXPath);
    else            pBaseNode = m_pDoc->documentNode(); //all xpath will be absolute anyway

    if (pBaseNode) {
      if (pDestinationNode = pBaseNode->getSingleNode(pQE, sDestinationXPath)) {
        if (sSelectXPath)     pSelectNode = pBaseNode->getSingleNode(pQE, sSelectXPath);
        else throw TXmlRequiredNodeNotFound(this, MM_STRDUP("select"), m_sAlias);
        if (pSelectNode) {
          //optional nodes
          if (sBeforeXPath) pBeforeNode = pBaseNode->getSingleNode(pQE, sBeforeXPath);

          pResultNode = pSelectNode->deviateNode(pQE, pHardlink, pNewNode, iType);
          if (pDestinationNode) pDestinationNode->clearMyCachedStylesheets(pQE); //pQE as MM
        } else throw TXmlRequiredNodeNotFound(this, MM_STRDUP("select"), m_sAlias);
      } else throw TXmlRequiredNodeNotFound(this, MM_STRDUP("destination"), m_sAlias);
    } else throw TXmlRequiredNodeNotFound(this, MM_STRDUP("base"), m_sAlias);

    //EMO report
    if (sExtensionElementPrefixes)      *sExtensionElementPrefixes      = extensionElementPrefixes();
    if (sNewResultXslModuleManagerName) *sNewResultXslModuleManagerName = xslModuleManagerName();

    //free up
    if (ppResultNode)    *ppResultNode = pResultNode;
    else if (pResultNode != pDestinationNode) delete pResultNode;
    if (pBaseNode)        delete pBaseNode;
    if (pSelectNode)      delete pSelectNode;
    if (pDestinationNode) delete pDestinationNode;
    if (pBeforeNode)      delete pBeforeNode;
    */

    return TXml::transactionSuccess;
  }

  ITDF_RETURN Database::hardlinkChild(ITDF_SECURITY_PARAMS, ITDF_BASE_PARAMS, const char *sDestinationXPath, const char *sBeforeXPath, const int iDestinationPosition,
                                      ITDF_RESULT_PARAMS) {
    IXmlBaseNode
      *pBaseNode            = 0,
      *pSelectNode          = 0,
      *pDestinationNode     = 0,
      *pResultNode          = 0,
      *pBeforeNode          = 0;

    //security
    IXmlQueryEnvironment *pQE = newQueryEnvironment();
    if (sReadOPUserUUIDForTransform) pQE->securityContext()->login(sReadOPUserUUIDForTransform);

    //base for xpath resolutions
    if (sBaseXPath) pBaseNode = m_pDoc->getSingleNode(pQE, sBaseXPath);
    else            pBaseNode = m_pDoc->documentNode(); //all xpath will be absolute anyway

    if (pBaseNode) {
      if (pDestinationNode = pBaseNode->getSingleNode(pQE, sDestinationXPath)) {
        if (sSelectXPath)     pSelectNode = pBaseNode->getSingleNode(pQE, sSelectXPath);
        else throw TXmlRequiredNodeNotFound(this, MM_STRDUP("select"), m_sAlias);
        if (pSelectNode) {
          //optional nodes
          if (sBeforeXPath) pBeforeNode = pBaseNode->getSingleNode(pQE, sBeforeXPath);

          pResultNode = pDestinationNode->hardlinkChild(pQE, pSelectNode, pBeforeNode, iDestinationPosition);
          if (pDestinationNode) pDestinationNode->clearMyCachedStylesheets(pQE); //pQE as MM
        } else throw TXmlRequiredNodeNotFound(this, MM_STRDUP("select"), m_sAlias);
      } else throw TXmlRequiredNodeNotFound(this, MM_STRDUP("destination"), m_sAlias);
    } else throw TXmlRequiredNodeNotFound(this, MM_STRDUP("base"), m_sAlias);

    //EMO report
    if (sExtensionElementPrefixes)      *sExtensionElementPrefixes      = extensionElementPrefixes();
    if (sNewResultXslModuleManagerName) *sNewResultXslModuleManagerName = xslModuleManagerName();

    //free up
    if (ppResultNode)    *ppResultNode = pResultNode;
    else if (pResultNode != pDestinationNode) delete pResultNode;
    if (pBaseNode)        delete pBaseNode;
    if (pSelectNode)      delete pSelectNode;
    if (pDestinationNode) delete pDestinationNode;
    if (pBeforeNode)      delete pBeforeNode;

    return TXml::transactionSuccess;
  }

  ITDF_RETURN Database::softlinkChild(ITDF_SECURITY_PARAMS, ITDF_BASE_PARAMS, const char *sDestinationXPath, const char *sBeforeXPath, 
                                            ITDF_RESULT_PARAMS) {
    IXmlBaseNode
      *pBaseNode            = 0,
      *pSelectNode          = 0,
      *pDestinationNode     = 0,
      *pBeforeNode          = 0,
      *pResultNode          = 0,
      *pResultParentNode    = 0;
    const IXmlQueryEnvironment *pQE;

    //security
    if (pDirectPassedQE) pQE = pDirectPassedQE;
    else {
      pQE = newQueryEnvironment();
      if (sReadOPUserUUIDForTransform) pQE->securityContext()->login(sReadOPUserUUIDForTransform);
    }

    //base for xpath resolutions
    if (sBaseXPath) pBaseNode = m_pDoc->getSingleNode(pQE, sBaseXPath);
    else            pBaseNode = m_pDoc->documentNode(); //all xpath will be absolute anyway

    if (pBaseNode) {
      if (pDestinationNode = pBaseNode->getSingleNode(pQE, sDestinationXPath)) {
        if (sSelectXPath)     pSelectNode = pBaseNode->getSingleNode(pQE, sSelectXPath);
        else throw TXmlRequiredNodeNotFound(this, MM_STRDUP("select"), m_sAlias);
        if (pSelectNode) {
          if (sBeforeXPath) pBeforeNode = pBaseNode->getSingleNode(pQE, sBeforeXPath);
          
          pResultNode = pDestinationNode->softlinkChild(pQE, pSelectNode, pBeforeNode);
          if (pResultNode) pResultNode->clearMyCachedStylesheets(pQE); //pQE as MM

          //return new node details for serialisation
          if (pResultNode) {
            //maintain index, LibXML2 will have blanked the @xml:id attribute
            xmlIDMainIndexDocument(pResultNode);

            if (pResultParentNode = pResultNode->parentNode(pQE)) *sSelectParentXPath = pResultParentNode->uniqueXPathToNode(pQE, NO_BASE_NODE, INCLUDE_TRANSIENT);
            *sSelectFQNodeName     = pResultNode->fullyQualifiedName();
            *sSelectXmlId          = pResultNode->xmlID(pQE);
            *sSelectRepositoryName = pResultNode->attributeValue(pQE, "repository:name");
          }
        } else throw TXmlRequiredNodeNotFound(this, MM_STRDUP("select"), m_sAlias);
      } else throw TXmlRequiredNodeNotFound(this, MM_STRDUP("destination"), m_sAlias);
    } else throw TXmlRequiredNodeNotFound(this, MM_STRDUP("base"), m_sAlias);

    //EMO report
    if (sExtensionElementPrefixes)      *sExtensionElementPrefixes      = extensionElementPrefixes();
    if (sNewResultXslModuleManagerName) *sNewResultXslModuleManagerName = xslModuleManagerName();

    //free up
    if (ppResultNode)     *ppResultNode = pResultNode;
    else if (pResultNode != pDestinationNode)  delete pResultNode;
    if (pBaseNode)         delete pBaseNode;
    if (pSelectNode)       delete pSelectNode;
    if (pDestinationNode)  delete pDestinationNode;
    if (pResultParentNode) delete pResultParentNode;

    return TXml::transactionSuccess;
  }
  
  ITDF_RETURN Database::mergeNode(ITDF_SECURITY_PARAMS, ITDF_BASE_PARAMS, const IXmlBaseNode *pTSelectNode, const char *sDestinationXPath, ITDF_TRANSFORM_PARAMS,
                                              ITDF_RESULT_PARAMS) {
    //caller frees result and non-zero **in-params
    //USING the DatabaseNode::mergeNode which uses TXml, not xmlMergeHierarchy
    //we need to maintain ids especially during parse-xml in case things get moved around radically
    //  because we will always be able to identify what has happened, created, deleted, moved
    //  powerful, accurate compare possibilities: parse-xml(), compare-xml(), replace-xml()
    //TXml has already parsed the XML string in to a separate document
    const DatabaseNode *pSelectNode      = 0;
    DatabaseNode *pBaseNode              = 0,
                 *pDestinationNode       = 0,
                 *pResultNode            = 0,
                 *pResultParentNode      = 0;
    IXmlBaseDoc *pBackupDoc   = 0;
    const DatabaseNode *pUserStore  = 0;
    const IXmlQueryEnvironment *pQE;

    //security
    if (pDirectPassedQE) pQE = pDirectPassedQE;
    else {
      pQE = newQueryEnvironment();
      if (sReadOPUserUUIDForTransform) pQE->securityContext()->login(sReadOPUserUUIDForTransform);
    }

    //base for xpath resolutions
    if (sBaseXPath) pBaseNode = getSingleNode(pQE, sBaseXPath);
    else            pBaseNode = documentNode(); //all xpath will be absolute anyway

    if (pBaseNode) {
      if (pTSelectNode) {
        //copying from the TXml child doc -> destination -------------------------------------------------------------
        //TXml child Doc is foreign input and we need some checks
        pSelectNode = factory_node(pTSelectNode, NOT_OURS);
        if (pDestinationNode = pBaseNode->getSingleNode(pQE, sDestinationXPath)) {
          //--------------- manual high-level checks of TXml parsed XML (pTSelectNode)
          //TXml must ensure that main top level xml:id must be the same if that's what it wants
          //because TXml needs that for commiting the replaceNode (saveArea)

          //TODO: new, potentially conflicting xml:ids

          //TODO: @repository:* changes

          //TODO: write, delete, move notifications

          //TODO: deleting nodes and the existing node-mask pointers in pQE

          //--------------- copy existing node to a new backup document
          //TODO: pBackupDoc = xmlLibrary()->factory_document("replace-xml backup doc");
          //pBackupDocRootNode = pBackupDoc->createRootChildElement("root_namespace_holder", 0, m_pDoc);
          //delete pChildDocRootNode->copyChild(pDestinationNode);

          //--------------- replace the new one in the main document, checking for low-level errors
          //this is a CROSS-document replacement
          //which will xmlReconciliateNs(...)
          //returns this: pDestinationNode == pResultNode, but m_oNode is replaced with a copy of *const* pTSelectNode
          pResultNode = pDestinationNode->mergeNode(pQE, pSelectNode);
          if (pResultNode) pResultNode->m_pNode->clearMyCachedStylesheets(pQE); //pQE as MM

          //maintain index on any new elements, LibXML2 will have blanked the @xml:id attribute
          xmlIDMainIndexDocument(pResultNode->m_pNode);

          //return new node details for serialisation
          if (pResultNode) {
            if (pResultParentNode  = pResultNode->parentNode(pQE)) *sSelectParentXPath = pResultParentNode->uniqueXPathToNode(pQE, NO_BASE_NODE, INCLUDE_TRANSIENT);
            *sSelectFQNodeName     = pResultNode->fullyQualifiedName();
            *sSelectXmlId          = pResultNode->xmlID(pQE);
            *sSelectRepositoryName = pResultNode->attributeValue(pQE, "repository:name");
          }
        } else throw TXmlRequiredNodeNotFound(this, MM_STRDUP("destination"), m_sAlias);
      } else {
        //in-document transfer select -> destination -------------------------------------------------------------
        //selectNode -> destinationNode, no childDoc
        //no need for xml:id checks because all the nodes already exist in this context
        //  unless replacing from an ignore XMLIDPolicy area
        if (pSelectNode = pBaseNode->getSingleNode(pQE, sSelectXPath)) {
          if (pDestinationNode = pBaseNode->getSingleNode(pQE, sDestinationXPath)) {
            pResultNode = pDestinationNode->mergeNode(pQE, pSelectNode);
            //return new node details for serialisation
            if (pResultNode) {
              if (pResultParentNode = pResultNode->parentNode(pQE)) *sSelectParentXPath = pResultParentNode->uniqueXPathToNode(pQE, NO_BASE_NODE, INCLUDE_TRANSIENT);
              *sSelectFQNodeName     = pResultNode->fullyQualifiedName();
              *sSelectXmlId          = pResultNode->xmlID(pQE);
              *sSelectRepositoryName = pResultNode->attributeValue(pQE, "repository:name");
            }
          } else throw TXmlRequiredNodeNotFound(this, MM_STRDUP("destination"), m_sAlias);
        } else throw TXmlRequiredNodeNotFound(this, MM_STRDUP("select"), m_sAlias);
      }
    } else throw TXmlRequiredNodeNotFound(this, MM_STRDUP("base"), m_sAlias);

    //EMO report
    if (sExtensionElementPrefixes)      *sExtensionElementPrefixes      = extensionElementPrefixes();
    if (sNewResultXslModuleManagerName) *sNewResultXslModuleManagerName = xslModuleManagerName();
    if (ppResultNode && pResultNode)    *ppResultNode                   = pResultNode->m_pNode;

    //free up
    if (!ppResultNode && pResultNode && pResultNode != pDestinationNode) delete pResultNode;
    if (pBaseNode)         delete pBaseNode;
    if (pSelectNode)       delete pSelectNode;
    if (pDestinationNode)  delete pDestinationNode;
    if (pResultParentNode) delete pResultParentNode;
    if (pBackupDoc)        delete pBackupDoc;
    if (pUserStore)        delete pUserStore;


    return TXml::transactionSuccess;
  }

  ITDF_RETURN Database::replaceNodeCopy(ITDF_SECURITY_PARAMS, ITDF_BASE_PARAMS, const IXmlBaseNode *pTSelectNode, const char *sDestinationXPath, ITDF_TRANSFORM_PARAMS,
                                              ITDF_RESULT_PARAMS) {
    //caller frees result and non-zero **in-params
    //we need to maintain ids especially during parse-xml in case things get moved around radically
    //  because we will always be able to identify what has happened, created, deleted, moved
    //  powerful, accurate compare possibilities: parse-xml(), compare-xml(), replace-xml()
    //TXml has already parsed the XML string in to a separate document
    IXmlBaseNode
      *pBaseNode              = 0,
      *pSelectNode            = 0,
      *pDestinationNode       = 0,
      *pResultNode            = 0,
      *pResultParentNode      = 0;
    IXmlBaseDoc *pBackupDoc   = 0;
    const DatabaseNode *pUserStore  = 0;
    const IXmlQueryEnvironment *pQE;

    //security
    if (pDirectPassedQE) pQE = pDirectPassedQE;
    else {
      pQE = newQueryEnvironment();
      if (sReadOPUserUUIDForTransform) pQE->securityContext()->login(sReadOPUserUUIDForTransform);
    }

    //base for xpath resolutions
    if (sBaseXPath) pBaseNode = m_pDoc->getSingleNode(pQE, sBaseXPath);
    else            pBaseNode = m_pDoc->documentNode(); //all xpath will be absolute anyway

    if (pBaseNode) {
      if (pTSelectNode) {
        //copying from the TXml child doc -> destination -------------------------------------------------------------
        //TXml child Doc is foreign input and we need some checks
        if (pDestinationNode = pBaseNode->getSingleNode(pQE, sDestinationXPath)) {
          //--------------- manual high-level checks of TXml parsed XML (pTSelectNode)
          //TXml must ensure that main top level xml:id must be the same if that's what it wants
          //because TXml needs that for commiting the replaceNode (saveArea)

          //TODO: new, potentially conflicting xml:ids

          //TODO: @repository:* changes

          //TODO: write, delete, move notifications

          //TODO: deleting nodes and the existing node-mask pointers in pQE

          //--------------- copy existing node to a new backup document
          //TODO: pBackupDoc = xmlLibrary()->factory_document("replace-xml backup doc");
          //pBackupDocRootNode = pBackupDoc->createRootChildElement("root_namespace_holder", 0, m_pDoc);
          //delete pChildDocRootNode->copyChild(pDestinationNode);

          //--------------- replace the new one in the main document, checking for low-level errors
          //this is a CROSS-document replacement
          //which will xmlReconciliateNs(...)
          //returns this: pDestinationNode == pResultNode, but m_oNode is replaced with a copy of *const* pTSelectNode
          pResultNode = pDestinationNode->replaceNodeCopy(pQE, pTSelectNode);
          if (pResultNode) pResultNode->clearMyCachedStylesheets(pQE); //pQE as MM

          //maintain index on any new elements, LibXML2 will have blanked the @xml:id attribute
          xmlIDMainIndexDocument(pResultNode);

          //return new node details for serialisation
          if (pResultNode) {
            if (pResultParentNode  = pResultNode->parentNode(pQE)) *sSelectParentXPath = pResultParentNode->uniqueXPathToNode(pQE, NO_BASE_NODE, INCLUDE_TRANSIENT);
            *sSelectFQNodeName     = pResultNode->fullyQualifiedName();
            *sSelectXmlId          = pResultNode->xmlID(pQE);
            *sSelectRepositoryName = pResultNode->attributeValue(pQE, "repository:name");
          }
        } else throw TXmlRequiredNodeNotFound(this, MM_STRDUP("destination"), m_sAlias);
      } else {
        //in-document transfer select -> destination -------------------------------------------------------------
        //selectNode -> destinationNode, no childDoc
        //no need for xml:id checks because all the nodes already exist in this context
        //  unless replacing from an ignore XMLIDPolicy area
        if (pSelectNode = pBaseNode->getSingleNode(pQE, sSelectXPath)) {
          if (pDestinationNode = pBaseNode->getSingleNode(pQE, sDestinationXPath)) {
            pResultNode = pDestinationNode->replaceNodeCopy(pQE, pSelectNode);
            //return new node details for serialisation
            if (pResultNode) {
              if (pResultParentNode = pResultNode->parentNode(pQE)) *sSelectParentXPath = pResultParentNode->uniqueXPathToNode(pQE, NO_BASE_NODE, INCLUDE_TRANSIENT);
              *sSelectFQNodeName     = pResultNode->fullyQualifiedName();
              *sSelectXmlId          = pResultNode->xmlID(pQE);
              *sSelectRepositoryName = pResultNode->attributeValue(pQE, "repository:name");
            }
          } else throw TXmlRequiredNodeNotFound(this, MM_STRDUP("destination"), m_sAlias);
        } else throw TXmlRequiredNodeNotFound(this, MM_STRDUP("select"), m_sAlias);
      }
    } else throw TXmlRequiredNodeNotFound(this, MM_STRDUP("base"), m_sAlias);

    //EMO report
    if (sExtensionElementPrefixes)      *sExtensionElementPrefixes      = extensionElementPrefixes();
    if (sNewResultXslModuleManagerName) *sNewResultXslModuleManagerName = xslModuleManagerName();

    //free up
    if (ppResultNode)     *ppResultNode = pResultNode;
    else if (pResultNode && pResultNode != pDestinationNode) delete pResultNode;
    if (pBaseNode)         delete pBaseNode;
    if (pSelectNode)       delete pSelectNode;
    if (pDestinationNode)  delete pDestinationNode;
    if (pResultParentNode) delete pResultParentNode;
    if (pBackupDoc)        delete pBackupDoc;
    if (pUserStore)        delete pUserStore;


    return TXml::transactionSuccess;
  }

  ITDF_RETURN Database::touch(ITDF_SECURITY_PARAMS, ITDF_BASE_PARAMS, ITDF_RESULT_PARAMS) {
    static const char *sCommandSignature = "database:touch @select=[node-set]";
    IXmlBaseNode *pBaseNode              = 0,
                 *pSelectNode            = 0;
    XmlNodeList<IXmlBaseNode> *pvSelectNodes = 0;
    XmlNodeList<IXmlBaseNode>::iterator iSelectNode;
    const IXmlQueryEnvironment *pQE;

    //security
    if (pDirectPassedQE) pQE = pDirectPassedQE;
    else {
      pQE = newQueryEnvironment();
      if (sReadOPUserUUIDForTransform) pQE->securityContext()->login(sReadOPUserUUIDForTransform);
    }

    //base for xpath resolutions
    if (sBaseXPath) pBaseNode = m_pDoc->getSingleNode(pQE, sBaseXPath);
    else            pBaseNode = m_pDoc->documentNode(); //all xpath will be absolute anyway

    if (pBaseNode) {
      if (sSelectXPath) {
        if (pvSelectNodes = pBaseNode->getMultipleNodes(pQE, sSelectXPath)) {
          for (iSelectNode = pvSelectNodes->begin(); iSelectNode != pvSelectNodes->end(); iSelectNode++) {
            pSelectNode = *iSelectNode;
            *sSelectFQNodeName     = pSelectNode->fullyQualifiedName();
            *sSelectXmlId          = pSelectNode->xmlID(pQE);
            *sSelectRepositoryName = pSelectNode->attributeValue(pQE, "repository:name");

            pSelectNode->touch(pQE);
            pSelectNode->clearMyCachedStylesheets(pQE); //pQE as MM
          }
        } else throw TXmlRequiredNodeNotFound(this, MM_STRDUP("select"), m_sAlias);
      } else throw AttributeRequired(this, MM_STRDUP("select"), MM_STRDUP(sCommandSignature));
    } else throw TXmlRequiredNodeNotFound(this, MM_STRDUP("base"), m_sAlias);
    if (ppResultNode)    *ppResultNode = pSelectNode;

    //EMO report
    if (sExtensionElementPrefixes)      *sExtensionElementPrefixes      = extensionElementPrefixes();
    if (sNewResultXslModuleManagerName) *sNewResultXslModuleManagerName = xslModuleManagerName();

    //free up
    if (pBaseNode)        delete pBaseNode;
    if (pvSelectNodes)    vector_element_destroy(pvSelectNodes);

    return TXml::transactionSuccess;
  }

  ITDF_RETURN Database::setAttribute(ITDF_SECURITY_PARAMS, ITDF_BASE_PARAMS, const char *sName, const char *sValue, const char *sNamespace, ITDF_RESULT_PARAMS) {
    static const char *sCommandSignature = "database:set-attribute @select=[node-set] @name @value";
    IXmlBaseNode *pBaseNode              = 0,
                 *pSelectNode            = 0;
    XmlNodeList<IXmlBaseNode> *pvSelectNodes = 0;
    XmlNodeList<IXmlBaseNode>::iterator iSelectNode;
    const IXmlQueryEnvironment *pQE;

    //security
    if (pDirectPassedQE) pQE = pDirectPassedQE;
    else {
      pQE = newQueryEnvironment();
      if (sReadOPUserUUIDForTransform) pQE->securityContext()->login(sReadOPUserUUIDForTransform);
    }

    //base for xpath resolutions
    if (sBaseXPath) pBaseNode = m_pDoc->getSingleNode(pQE, sBaseXPath);
    else            pBaseNode = m_pDoc->documentNode(); //all xpath will be absolute anyway

    if (pBaseNode) {
      if (sSelectXPath) {
        if (pvSelectNodes = pBaseNode->getMultipleNodes(pQE, sSelectXPath)) {
          for (iSelectNode = pvSelectNodes->begin(); iSelectNode != pvSelectNodes->end(); iSelectNode++) {
            //GDB: p ((LibXmlBaseNode*) pSelectNode)->m_oNode
            pSelectNode = *iSelectNode;
            *sSelectFQNodeName     = pSelectNode->fullyQualifiedName();
            *sSelectXmlId          = pSelectNode->xmlID(pQE);
            *sSelectRepositoryName = pSelectNode->attributeValue(pQE, "repository:name");

            pSelectNode->setAttribute(pQE, sName, (sValue ? sValue : ""), sNamespace);
            pSelectNode->clearMyCachedStylesheets(pQE); //pQE as MM
          }
        } else throw TXmlRequiredNodeNotFound(this, MM_STRDUP("select"), m_sAlias);
      } else throw AttributeRequired(this, MM_STRDUP("select"), MM_STRDUP(sCommandSignature));
    } else throw TXmlRequiredNodeNotFound(this, MM_STRDUP("base"), m_sAlias);
    if (ppResultNode)    *ppResultNode = pSelectNode;

    //EMO report
    if (sExtensionElementPrefixes)      *sExtensionElementPrefixes      = extensionElementPrefixes();
    if (sNewResultXslModuleManagerName) *sNewResultXslModuleManagerName = xslModuleManagerName();

    //free up
    if (pBaseNode)        delete pBaseNode;
    if (pvSelectNodes)    vector_element_destroy(pvSelectNodes);

    return TXml::transactionSuccess;
  }

  ITDF_RETURN Database::setValue(ITDF_SECURITY_PARAMS, ITDF_BASE_PARAMS, const char *sValue, ITDF_RESULT_PARAMS) {
    static const char *sCommandSignature = "database:set-value @select=[node-set] @value";
    IXmlBaseNode
      *pBaseNode            = 0,
      *pSelectNode          = 0;
    const IXmlQueryEnvironment *pQE;

    //security
    if (pDirectPassedQE) pQE = pDirectPassedQE;
    else {
      pQE = newQueryEnvironment();
      if (sReadOPUserUUIDForTransform) pQE->securityContext()->login(sReadOPUserUUIDForTransform);
    }

    //base for xpath resolutions
    if (sBaseXPath) pBaseNode = m_pDoc->getSingleNode(pQE, sBaseXPath);
    else            pBaseNode = m_pDoc->documentNode(); //all xpath will be absolute anyway

    if (pBaseNode) {
      if (sSelectXPath) {
        if (pSelectNode = pBaseNode->getSingleNode(pQE, sSelectXPath)) {
          pSelectNode->value(pQE, (sValue ? sValue : ""));
          *sSelectFQNodeName     = pSelectNode->fullyQualifiedName();
          *sSelectXmlId          = pSelectNode->xmlID(pQE);
          *sSelectRepositoryName = pSelectNode->attributeValue(pQE, "repository:name");

          pSelectNode->clearMyCachedStylesheets(pQE); //pQE as MM
        } else throw TXmlRequiredNodeNotFound(this, MM_STRDUP("select"), m_sAlias);
      } else throw AttributeRequired(this, MM_STRDUP("select"), MM_STRDUP(sCommandSignature));
    } else throw TXmlRequiredNodeNotFound(this, MM_STRDUP("base"), m_sAlias);
    if (ppResultNode)    *ppResultNode = pSelectNode;

    //EMO report
    if (sExtensionElementPrefixes)      *sExtensionElementPrefixes      = extensionElementPrefixes();
    if (sNewResultXslModuleManagerName) *sNewResultXslModuleManagerName = xslModuleManagerName();

    //free up
    if (pBaseNode)        delete pBaseNode;
    if (pSelectNode)      delete pSelectNode;

    return TXml::transactionSuccess;
  }

  ITDF_RETURN Database::removeNode(ITDF_SECURITY_PARAMS, ITDF_BASE_PARAMS, ITDF_RESULT_PARAMS) {
    static const char *sCommandSignature = "database:remove-node @select=[node-set]";
    IXmlBaseNode
      *pBaseNode            = 0,
      *pSelectNode          = 0,
      *pResultParentNode    = 0;
    const IXmlQueryEnvironment *pQE;

    //security
    if (pDirectPassedQE) pQE = pDirectPassedQE;
    else {
      pQE = newQueryEnvironment();
      if (sReadOPUserUUIDForTransform) pQE->securityContext()->login(sReadOPUserUUIDForTransform);
    }

    //base for xpath resolutions
    if (sBaseXPath) pBaseNode = m_pDoc->getSingleNode(pQE, sBaseXPath);
    else            pBaseNode = m_pDoc->documentNode(); //all xpath will be absolute anyway

    if (pBaseNode) {
      if (sSelectXPath) {
        if (pSelectNode = pBaseNode->getSingleNode(pQE, sSelectXPath)) {
          //return new node details for serialisation
          if (pResultParentNode = pSelectNode->parentNode(pQE)) *sSelectParentXPath = pResultParentNode->uniqueXPathToNode(pQE, NO_BASE_NODE, INCLUDE_TRANSIENT);
          *sSelectFQNodeName     = pSelectNode->fullyQualifiedName();
          *sSelectXmlId          = pSelectNode->xmlID(pQE);
          *sSelectRepositoryName = pSelectNode->attributeValue(pQE, "repository:name");

          pSelectNode->clearMyCachedStylesheets(pQE); //pQE as MM
          pSelectNode->removeAndDestroyNode(pQE);
        } else throw TXmlRequiredNodeNotFound(this, MM_STRDUP("select"), m_sAlias);
      } else throw AttributeRequired(this, MM_STRDUP("select"), MM_STRDUP(sCommandSignature));
    } else throw TXmlRequiredNodeNotFound(this, MM_STRDUP("base"), m_sAlias);
    if (ppResultNode)    *ppResultNode = 0;

    //EMO report
    if (sExtensionElementPrefixes)      *sExtensionElementPrefixes      = extensionElementPrefixes();
    if (sNewResultXslModuleManagerName) *sNewResultXslModuleManagerName = xslModuleManagerName();

    //free up
    if (pBaseNode)         delete pBaseNode;
    if (pSelectNode)       delete pSelectNode;
    if (pResultParentNode) delete pResultParentNode;

    return TXml::transactionSuccess;
  }

  ITDF_RETURN Database::createChildElement(ITDF_SECURITY_PARAMS, ITDF_BASE_PARAMS, const char *sName, const char *sValue, const char *sNamespace, ITDF_RESULT_PARAMS) {
    static const char *sCommandSignature = "database:create-child-element @select=[node-set] @name @value";
    IXmlBaseNode
      *pBaseNode            = 0,
      *pResultNode          = 0,
      *pSelectNode          = 0;
    const IXmlQueryEnvironment *pQE;

    //security
    if (pDirectPassedQE) pQE = pDirectPassedQE;
    else {
      pQE = newQueryEnvironment();
      if (sReadOPUserUUIDForTransform) pQE->securityContext()->login(sReadOPUserUUIDForTransform);
    }

    //base for xpath resolutions
    if (sBaseXPath) pBaseNode = m_pDoc->getSingleNode(pQE, sBaseXPath);
    else            pBaseNode = m_pDoc->documentNode(); //all xpath will be absolute anyway

    if (pBaseNode) {
      if (sSelectXPath) {
        if (pSelectNode = pBaseNode->getSingleNode(pQE, sSelectXPath)) {
          pResultNode = pSelectNode->createChildElement(pQE, sName, sValue ? sValue : "", sNamespace);
          *sSelectFQNodeName     = pResultNode->fullyQualifiedName();
          *sSelectXmlId          = pResultNode->xmlID(pQE);
          *sSelectRepositoryName = pResultNode->attributeValue(pQE, "repository:name");

          pSelectNode->clearMyCachedStylesheets(pQE); //pQE as MM
        } else throw TXmlRequiredNodeNotFound(this, MM_STRDUP("select"), m_sAlias);
      } else throw AttributeRequired(this, MM_STRDUP("select"), MM_STRDUP(sCommandSignature));
    } else throw TXmlRequiredNodeNotFound(this, MM_STRDUP("base"), m_sAlias);

    //EMO report
    if (sExtensionElementPrefixes)      *sExtensionElementPrefixes      = extensionElementPrefixes();
    if (sNewResultXslModuleManagerName) *sNewResultXslModuleManagerName = xslModuleManagerName();

    //free up
    if (ppResultNode)    *ppResultNode = pResultNode;
    else if (pResultNode) delete pResultNode;
    if (pBaseNode)        delete pBaseNode;
    if (pSelectNode)      delete pSelectNode;

    return TXml::transactionSuccess;
  }

  ITDF_RETURN Database::changeName(ITDF_SECURITY_PARAMS, ITDF_BASE_PARAMS, const char *sName, ITDF_RESULT_PARAMS) {
    static const char *sCommandSignature = "database:change-name @select=[node-set] @name";
    IXmlBaseNode
      *pBaseNode            = 0,
      *pSelectNode          = 0;
    const IXmlQueryEnvironment *pQE;

    //security
    if (pDirectPassedQE) pQE = pDirectPassedQE;
    else {
      pQE = newQueryEnvironment();
      if (sReadOPUserUUIDForTransform) pQE->securityContext()->login(sReadOPUserUUIDForTransform);
    }

    //base for xpath resolutions
    if (sBaseXPath) pBaseNode = m_pDoc->getSingleNode(pQE, sBaseXPath);
    else            pBaseNode = m_pDoc->documentNode(); //all xpath will be absolute anyway

    if (pBaseNode) {
      if (sSelectXPath) {
        if (pSelectNode = pBaseNode->getSingleNode(pQE, sSelectXPath)) {
          pSelectNode->localName(pQE, sName);
          *sSelectFQNodeName     = pSelectNode->fullyQualifiedName();
          *sSelectXmlId          = pSelectNode->xmlID(pQE);
          *sSelectRepositoryName = pSelectNode->attributeValue(pQE, "repository:name");

          pSelectNode->clearMyCachedStylesheets(pQE); //pQE as MM
        } else throw TXmlRequiredNodeNotFound(this, MM_STRDUP("select"), m_sAlias);
      } else throw AttributeRequired(this, MM_STRDUP("select"), MM_STRDUP(sCommandSignature));
    } else throw TXmlRequiredNodeNotFound(this, MM_STRDUP("base"), m_sAlias);
    if (ppResultNode) *ppResultNode = pSelectNode;

    //EMO report
    if (sExtensionElementPrefixes)      *sExtensionElementPrefixes      = extensionElementPrefixes();
    if (sNewResultXslModuleManagerName) *sNewResultXslModuleManagerName = xslModuleManagerName();

    //free up
    if (pBaseNode)        delete pBaseNode;
    if (pSelectNode)      delete pSelectNode;

    return TXml::transactionSuccess;
  }
  const vector<TXmlProcessor*> *Database::linkedTXmlProcessors(IXmlBaseNode *pSerialisationNode) const {
    vector<TXmlProcessor*> *pLinkedTXmlProcessors = 0;
    if (m_pRepositoryReader) {
      pLinkedTXmlProcessors = new vector<TXmlProcessor*>();
      pLinkedTXmlProcessors->push_back(m_pRepositoryReader);
    }
    return pLinkedTXmlProcessors;
  }

  IXmlBaseNode *Database::transactionDisplayNode(IXmlBaseNode *pSerialisationNode) {
    //do not delete result!
    if (!m_bLookedForTransactionDisplayNode) {
      m_pTransactionDisplayNode = m_pDoc->getSingleNode(m_pIBQE_dbAdministrator, "/object:Server/repository:transactions");
      if (!m_pTransactionDisplayNode) {
        m_pTransactionDisplayNode = m_pDoc->createChildElement(m_pIBQE_dbAdministrator, "transactions", NULL, NAMESPACE_REPOSITORY);
      }
      m_bLookedForTransactionDisplayNode = true;
    }

    return m_pTransactionDisplayNode;
  }

  const char *Database::encoding() const {
    return m_pDoc->encoding();
  }

  void Database::serialise(const IXmlQueryEnvironment *pQE, IXmlBaseNode *pOutputNode) const {
    assert(pOutputNode);

    IXmlBaseNode *pCurrent;
    StringMap<IXmlBaseNode*>::const_iterator iClass;

    //--------------------------- attributes
    pCurrent = pOutputNode->createChildElement(pQE, "db");
    pCurrent->setAttribute(pQE, "alias", m_sAlias);
    pCurrent->setAttribute(pQE, "encoding", m_pDoc->encoding());
    pCurrent->setAttribute(pQE, "file", m_pRepositoryReader->source());
    pCurrent->setAttribute(pQE, "loadedAndParsed", m_pDoc->loadedAndParsed());
    pCurrent->setAttribute(pQE, "resolveExternals", m_pDoc->resolveExternals());
    pCurrent->setAttribute(pQE, "specified", m_pDoc->specified());
    pCurrent->setAttribute(pQE, "validateOnParse", m_pDoc->validateOnParse());

    //--------------------------- classes
    DatabaseClass::serialise(pQE, pCurrent);

    //--------------------------- base doc
    m_pDoc->serialise(pQE, pCurrent);

    //free up
    delete pCurrent;
  }

  IXmlQueryEnvironment *Database::newQueryEnvironment(IXmlProfiler *pProfiler, IXslModuleManager *pAlternateEMO, IXmlTriggerContext *pAlternativeTrigger) {
    IXmlTriggerContext   *pTrigger;
    IXmlMaskContext      *pMask;
    IXslModuleManager    *pEMO;
    IXmlQueryEnvironment *pNewQE;

    pTrigger = (pAlternativeTrigger ? pAlternativeTrigger->inherit() : new DATABASE_TRIGGER_CONTEXT(this, m_pDoc));
    pMask    = new DATABASE_MASK_CONTEXT();
    pEMO     = (pAlternateEMO ? pAlternateEMO : this); //the database is also an IXslModuleManager

    if (m_bStartingUp) pTrigger->disable();

    pNewQE = new QueryEnvironment(
      this,
      pProfiler,
      m_pDoc,
      pEMO,
      new DatabaseNodeUserStoreSecurityContext(this, m_pUserStore),
      pTrigger,
      pMask,
      0,
      &m_grammar
    );

    return pNewQE;
  }

  size_t Database::registerWriteEvent(const IXmlQueryEnvironment *pQE, IXmlBaseNode *pChangedNode) {
    //called from the Triggers system
    assert(pQE);
    assert(pChangedNode);
    
    size_t iEventID = 0; //1 based
    
    if (m_vEvents.size() == 1000000) NOT_COMPLETE("events recording management");
    else {
      writeListenerManagementLock(); {
        m_vEvents.push_back(pChangedNode->clone_with_resources()); //additional lock
        iEventID = m_vEvents.size(); //1 based
      } writeListenerManagementUnlock();
      
      releaseWriteListeners(pQE, iEventID);
    }
    
    return iEventID;
  }

  size_t Database::releaseWriteListeners(const IXmlQueryEnvironment *pQE, const size_t iEventID) {
    //called from the Triggers system
    assert(pQE);
  
    IXmlBaseNode *pChangedNode;
    list<ListenDetails>::iterator iWriteListener;
    const IXmlArea *pArea;
    sem_t *pSemWriteListener;

    pChangedNode = m_vEvents[iEventID - 1]; //1 based
    writeListenerManagementLock(); {
      for (iWriteListener = m_lWriteListeners.begin(); iWriteListener != m_lWriteListeners.end(); iWriteListener++) {
        pArea = iWriteListener->pArea;
        if (pArea->contains(pQE, pChangedNode)) {
          Debug::report("releasing area watch [%s] changed node [%s]", pArea->toString(), pChangedNode->uniqueXPathToNode(pQE, NO_BASE_NODE, INCLUDE_TRANSIENT));
          iWriteListener->iEventID = iEventID; //1 based
          pSemWriteListener = iWriteListener->pSemWriteListener;
          sem_post(pSemWriteListener); //causes element erasure, after writeListenerManagementUnlock()
        }
      }
    } writeListenerManagementUnlock();
    //released waiters will now delete themselves from the m_lWriteListeners list

    return iEventID;
  }

  size_t Database::waitForSingleWriteEvent(const IXmlQueryEnvironment *pQE, const IXmlArea *pArea, const size_t iLastKnownEventID) {
    //BLOCKING until any write happens
    //returns:
    //  0: a write happened. caller carries out analysis of the new pArea
    //  1: timedout (5 minutes). caller must re-initiate waiting call
    list<ListenDetails>::iterator iWriteListener;
    sem_t *pSemWriteListener; //locate on stack for visibility
    struct timespec abs_timeout;
    int s;
    IXmlBaseNode *pChangedNode;
    size_t iEventID = 0, iEvent;

    //check for existing events before listening
    //zero iLastKnownEventID will prevent a search for existing events
    if (iLastKnownEventID) {
      for (iEvent = iLastKnownEventID; !iEventID && iEvent < m_vEvents.size(); iEvent++) {
        pChangedNode = m_vEvents[iEvent];
        if (pArea->contains(pQE, pChangedNode)) {
          iEventID = iEvent + 1;
        }
      }
    }
    
    if (!iEventID) {
      //initialise stuff
      pSemWriteListener = new sem_t;
      sem_init(pSemWriteListener, NOT_FORK_SHARED, 0); //http://linux.die.net/man/3/sem_init
      clock_gettime(CLOCK_REALTIME, &abs_timeout);
      abs_timeout.tv_sec += 5 * 60; //5 minutes

      //insert
      //this causes an incremental ID to be assigned to the listener
      writeListenerManagementLock(); {
        m_lWriteListeners.push_front(ListenDetails(pArea, pSemWriteListener));
        iWriteListener = m_lWriteListeners.begin();
      } writeListenerManagementUnlock();

      //block
      while ((s = sem_timedwait(pSemWriteListener, &abs_timeout)) == -1 && SOCKETS_ERRNO == EINTR) continue;

      //remove waiter
      writeListenerManagementLock(); {
        iEventID = iWriteListener->iEventID;
        m_lWriteListeners.erase(iWriteListener);
      } writeListenerManagementUnlock();

      //analyse result
      UNWIND_EXCEPTION_BEGIN {
        if (s == -1) {
          if (SOCKETS_ERRNO != ETIMEDOUT) throw SemaphoreFailure(this, SOCKETS_ERRNO);
        }
        //else normal release
        if (iEventID) Debug::report("released area watch [%s] event id [%u]", pArea->toString(), iEventID);
      } UNWIND_EXCEPTION_END;

      //free up
      //if (pChangedNode) MMO_DELETE(pChangedNode); //pointer in to m_vEvents
      sem_destroy(pSemWriteListener);
      delete pSemWriteListener;

      UNWIND_EXCEPTION_THROW;
    }

    return iEventID;
  }
}
