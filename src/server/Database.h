//platform agnostic file
//XML Database class wraps the XmlDoc
//late bound namespace XSL style commands
//  to expose DOM Level 3 to the XSL process
//  mostly to make changes to the source document during a transformation
//Indexes and other external high level functions for a DOM document
#ifndef _DATABASE_H
#define _DATABASE_H

#include "Utilities/regexpr2.h"
using namespace regex;

#include "Utilities/StringMap.h"
#include "Utilities/StringMultiMap.h"
#include <map>
#include <list>
#include "semaphore.h"
using namespace std;

#include "IXml/IXmlBaseNode.h"               //enums
#include "IXml/IXslXPathFunctionContext.h"   //enums
#include "QueryEnvironment/TriggerContext.h" //Database has-a Database_TRIGGER_CONTEXT
#include "QueryEnvironment/GrammarContext.h" //Database has-a GrammarContext
#include "XJavaScript.h"        //member
#include "QueryEnvironment/DatabaseAdminQueryEnvironment.h" //Database has-a embedded sub-class XmlAdminQueryEnvironment
#include "TXmlProcessor.h"      //direct inheritance
#include "IReportable.h"        //direct inheritance
#include "XSLFunctions.h"       //member
#include "Debug.h"              //member
#include "define.h"

#include "Xml/XmlNodeList.h"        //member variable

//platform specific project headers
#include "LibXslModule.h"

namespace general_server {
  class Database;
  class DatabaseNode;
  class DatabaseClass;
  class Server;
  class SecurityContext;

  //these are included in the Database.cpp
  interface_class IXmlDoc;
  interface_class IXmlBaseDoc;
  interface_class IXmlBaseNode;
  interface_class IXmlArea;
  interface_class IXslCommandNode;     //XSL extensions

  /** \brief the main Database dude
    *
    * wrapper around the in-memory IXmlBaseDoc and associated permanent Repository(s) deals with
    *   storage, transactions and commits
    *   XSL database: functional namespace
    *   Classes, with the DatabaseClass class
    */
  class Database: public TXmlProcessor, public SERVER_MAIN_XSL_MODULE, public XslModuleManager, implements_interface IMemoryLifetimeOwner, implements_interface IReportable, implements_interface ITXmlDirectFunctions {
    friend class DatabaseNode;
    const IXmlLibrary  *m_pLib;
    EXSLTRegExp         m_eXSLTRegExp;          /**< EXSLTRegExp  in Database for XSL commands */
    EXSLTStrings        m_eXSLTStrings;         /**< EXSLTStrings in Database for XSL commands */
    XSLTCustomUtilities m_xSLTCustomUtilities;  /**< XSLTCustomUtilities in Database for XSL commands */
    XJavaScript         m_xJavaScript;          /**< XJavaScript  in Database for XSL commands */

    //indexing, commits and other direct ROOT control by-pass actions
    //triggers and filtering are disabled
    //see Database::newQueryEnvironment() for general use Environments
    //m_pIBQE_dbAdministrator is a pointer because the Database does not initialise with the document
    DatabaseAdminQueryEnvironment *m_pIBQE_dbAdministrator;
    bool m_bStartingUp; //causes disabled triggers

    struct ListenDetails {
      const IXmlArea *pArea;             //input
      sem_t          *pSemWriteListener; //process semaphore
      size_t          iEventID;          //output
      
      ListenDetails(const IXmlArea *_pArea, sem_t *_pSemWriteListener):
        pArea(_pArea),
        pSemWriteListener(_pSemWriteListener),
        iEventID(0)
      {}
    };
    list<ListenDetails>    m_lWriteListeners;
    static pthread_mutex_t m_mWriteListenerManagement;

    const char   *m_sAlias;
    IXmlBaseDoc  *m_pDoc;                 //fast in-memory repository
    Repository   *m_pRepositoryReader;    //slow permanent store
    Debug         m_debug;                //XSL reporting to console
    const char   *m_sRepositorySource;
    IXmlBaseNode *m_pTransactionDisplayNode;
    bool m_bLookedForTransactionDisplayNode;
    IXmlBaseNode *m_pTmpNode;
    bool m_bLookedForTmpNode;
    GrammarContext m_grammar; //fixed Grammar handed out for this DB: stateless!

    static StringMap<IXslModule::XslModuleCommandDetails>  m_XSLCommands; //XSL extensions
    static StringMap<IXslModule::XslModuleFunctionDetails> m_XSLFunctions;

    struct dbIndexingStatus {
      const char  *sMainIdxPrefix;
      const char  *sMainIdxSuffix;
      unsigned int iUIDXFrom;
      const char  *sIgnoreNS;
      bool         bInXSL; //if there is an ancestor xsl:template
    };
    size_t xmlIDMainIndexDocument_recursive(IXmlBaseNode *pFromNode, XmlNodeList<IXmlBaseNode> &vNewIDs, dbIndexingStatus& pDatabaseis);
    void createTmpArea();

    //UserStore with userids must match the userids used throughout the Database
    //  certainly with UnixSecurityStrategy anyway
    //  therefore the knowledge of the userStore location is with the Database
    //the Database needs to apply TXml with SecurityContext::userStore knowledge for read transforms
    //  at startup
    //  so TXml stores the knowledge of the userStore with it
    //  which is currently pointing in to the same Database
    IXmlBaseNode *m_pUserStore;

  public:
    Database(
       const IMemoryLifetimeOwner *pLifetimeOwner, 
       const IXmlLibrary *pLib, 
       const char *sAlias, 
       const char *sRepositorySource,
       const char *sXPathToStore = "/object:Server/repository:users"
    );
    ~Database();

    //indexing
    size_t xmlIDMainIndexDocument(IXmlBaseNode *pFromNode, XmlNodeList<IXmlBaseNode> &vNewIDs);
    size_t xmlIDMainIndexDocument(IXmlBaseNode *pFromNode); //don't care about new IDs

    //object support (xschema node areas)
    size_t descendantObjects(const IXmlQueryEnvironment *pQE, const IXmlBaseNode *pNode, XmlNodeList<const IXmlBaseNode> *pvDescendantObjects, const bool bIncludeIntermediateNodes = false) const;
    bool   isObject(const IXmlBaseNode *pNode) const; //class(IXmlBaseNode *pNode) != 0
    bool popIfType(IXslXPathFunctionContext *pXCtxt, iXPathParamType iTypeTest) const;

    //worker functions
    //non-static because require passing the Database through, which is this
    DatabaseNode *factory_node(IXmlBaseNode *pNode, const bool bOurs = OURS) const;
    const DatabaseNode *factory_node(const IXmlBaseNode *pNode, const bool bOurs = OURS) const;
    XmlNodeList<DatabaseNode>       *factory_nodes(XmlNodeList<IXmlBaseNode> *pvNodeList) const;
    XmlNodeList<const DatabaseNode> *factory_nodes(XmlNodeList<const IXmlBaseNode> *pvNodeList) const;
    IXmlBaseNode *createTmpNode() const; //tmp area for rendering transient data like in-context XML parsing
    IXmlBaseNode *runSystemTransform(const char *sName, IXmlBaseNode *pOutputNode);

    //accessors and reporting
    void serialise(const IXmlQueryEnvironment *pQE, IXmlBaseNode *pOutput) const;
    static void compilationInformation(const IXmlQueryEnvironment *pQE ATTRIBUTE_UNUSED, IXmlBaseNode *pOutput ATTRIBUTE_UNUSED) {}
    int   validityCheck(const char *sReason = 0) const;
    bool  shellCommand(const char *sCommand) const; //native library shell commands
    const char *toString() const; //IReportable
    const char *name() const;
    const char *encoding() const;
    const char *xsltModuleNamespace() const;
    const char *xsltModulePrefix()    const;
    const IXmlLibrary *xmlLibrary() const;
    //override the TXmlProcessor document_const() => 0
    //this also provides protected const access to the document for commit() process
    //  affectedNodes(...) and isTransient(...)
    //this also classes Database as an in-memory doc TXmlProcessor
    //enabling various activities when saving transactions and commiting
    const IXmlBaseDoc *document_const() const;
    const Repository  *repository() const;

    //XslModuleManager interface
    Database *db() const {return (Database*) this;}
    const char *xslModuleManagerName() const {return "Database";}
    const StringMap<IXslModule::XslModuleCommandDetails>  *xslCommands() const;
    const StringMap<IXslModule::XslModuleFunctionDetails> *xslFunctions() const;
    //permanent, re-entrant sub-XSL modules to complement the XSL library
    SERVER_MAIN_XSL_MODULE *eXSLTRegExp()         {return &m_eXSLTRegExp;}
    SERVER_MAIN_XSL_MODULE *eXSLTStrings()        {return &m_eXSLTStrings;}
    SERVER_MAIN_XSL_MODULE *xSLTCustomUtilities() {return &m_xSLTCustomUtilities;}
    SERVER_MAIN_XSL_MODULE *xJavaScript()         {return &m_xJavaScript;}

    //QueryEnvironment for query calls
    //only Database can create a QE (friend class)
    //the pProfiler is used for memory management also
    IXmlQueryEnvironment *newQueryEnvironment(
      IXmlProfiler *pProfiler = 0, 
      IXslModuleManager *pEMO = 0, 
      IXmlTriggerContext *pTrigger = 0
    );

    TXml::transactionResult commit(TXml::commitStatus cs = TXml::beginCommit, TXml::commitStyle ct = TXml::selective);
    TXml::transactionResult commitSetPersistentStatus(TXml::commitStatus cs, TXml::commitStyle ct);
    TXml::transactionResult commitDeleteTransactions(const vector<TXml*> *pvTXmls = 0);
    TXml::transactionResult rollback();

    //----------------------------------------------------- read-only section
    DatabaseNode          *documentNode(const char *sReason = 0);
    DatabaseNode          *rootNode(const IXmlQueryEnvironment *pQE, const iDOMNodeTypeRequest iNodeTypes = node_type_element_only, const char *sName = 0, const char *sLockingReason = 0);
    const DatabaseNode    *dtdNode() const;
    DatabaseNode          *getSingleNode(const IXmlQueryEnvironment *pQE, const char *sXPath, const char *argchar = 0, const int *argint = 0);
    XmlNodeList<DatabaseNode> *getMultipleNodes(const IXmlQueryEnvironment *pQE, const char *sXPath, const char *argchar = 0, const int *argint = 0, const bool bRegister = false, const char *sLockingReason = 0);
    DatabaseNode          *nodeFromID(const IXmlQueryEnvironment *pQE, const char *sXMLID);
    const StringMap<const char*> *allPrefixedNamespaceDefinitions() const; //-> IXmlBaseDoc
    IXmlBaseDoc *transform(
      const IMemoryLifetimeOwner *pMemoryLifetimeOwner, 
      const IXslStylesheetNode *pXslStylesheetNode,
      const IXmlQueryEnvironment *pQE,
      const StringMap<const size_t> *pParamsInt = 0, const StringMap<const char*> *pParamsChar = 0, const StringMap<const XmlNodeList<const IXmlBaseNode>*> *pParamsNodeSet = 0,
      const char *sWithMode = 0
    ) const;

    //XslModule commands and functions
    /** \addtogroup XSLModule-Commands
     * @{
     */
    IXmlBaseNode *xslCommand_query(const IXmlQueryEnvironment *pQE, const IXslCommandNode *pCommandNode, const IXmlBaseNode *pSourceNode, IXmlBaseNode *pOutputNode);
    IXmlBaseNode *xslCommand_transform(const IXmlQueryEnvironment *pQE, const IXslCommandNode *pCommandNode, const IXmlBaseNode *pSourceNode, IXmlBaseNode *pOutputNode);
    IXmlBaseNode *xslCommand_withNodeMask(const IXmlQueryEnvironment *pQE, const IXslCommandNode *pCommandNode, const IXmlBaseNode *pSourceNode, IXmlBaseNode *pOutputNode);
    IXmlBaseNode *xslCommand_withoutNodeMask(const IXmlQueryEnvironment *pQE, const IXslCommandNode *pCommandNode, const IXmlBaseNode *pSourceNode, IXmlBaseNode *pOutputNode);
    IXmlBaseNode *xslCommand_hardlinkInfo(const IXmlQueryEnvironment *pQE, const IXslCommandNode *pCommandNode, const IXmlBaseNode *pSourceNode, IXmlBaseNode *pOutputNode);
    IXmlBaseNode *xslCommand_deviationInfo(const IXmlQueryEnvironment *pQE, const IXslCommandNode *pCommandNode, const IXmlBaseNode *pSourceNode, IXmlBaseNode *pOutputNode);
    IXmlBaseNode *xslCommand_validityCheck(const IXmlQueryEnvironment *pQE, const IXslCommandNode *pCommandNode, const IXmlBaseNode *pSourceNode, IXmlBaseNode *pOutputNode);
    IXmlBaseNode *xslCommand_waitForSingleWriteEvent(const IXmlQueryEnvironment *pQE, const IXslCommandNode *pCommandNode, const IXmlBaseNode *pSourceNode, IXmlBaseNode *pOutputNode);
    IXmlBaseNode *xslCommand_selectInputNode(const IXmlQueryEnvironment *pQE, const IXslCommandNode *pCommandNode, const IXmlBaseNode *pSourceNode, IXmlBaseNode *pOutputNode);
    IXmlBaseNode *xslCommand_xpathToNode(const IXmlQueryEnvironment *pQE, const IXslCommandNode *pCommandNode, const IXmlBaseNode *pSourceNode, IXmlBaseNode *pOutputNode);
    IXmlBaseNode *xslCommand_differences(const IXmlQueryEnvironment *pQE, const IXslCommandNode *pCommandNode, const IXmlBaseNode *pSourceNode, IXmlBaseNode *pOutputNode);

    IXmlBaseNode *xslCommand_setSecurityContext(const IXmlQueryEnvironment *pQE, const IXslCommandNode *pCommandNode, const IXmlBaseNode *pSourceNode, IXmlBaseNode *pOutputNode);
    IXmlBaseNode *xslCommand_clearSecurityContext(const IXmlQueryEnvironment *pQE, const IXslCommandNode *pCommandNode, const IXmlBaseNode *pSourceNode, IXmlBaseNode *pOutputNode);
    /** @} */

    /** \addtogroup XSLModule-Functions
     * @{
     */
    const char *xslFunction_parseXML(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_parseHTML(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_copyOf(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_setNamespace(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_transform(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_replaceChildrenTransient(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_validityCheck(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_fullyQualifiedName(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_xpathToNode(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_rx(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_UUID(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_nodeMask(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_distinct(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_difference(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_hasSameNode(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_intersection(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_documentOrder(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_similarity(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);

    const char *xslFunction_isHardlinked(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_isHardlink(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_isSoftlink(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_isOriginalHardlink(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);

    const char *xslFunction_isDeviant(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_hasDeviants(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_hasDescendantDeviants(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);

    const char *xslFunction_isNodeElement(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_isNodeDocument(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_isNodeAttribute(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_isNodeText(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_isProcessingInstruction(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_isCDATASection(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);

    const char *xslFunction_isUndefined(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_isNodeSet(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_isBoolean(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_isNumber(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_isString(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_isPoint(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_isRange(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_isLocationSet(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_isUsers(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_isXSLTTree(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_notNodeSet(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);

    const char *xslFunction_isObject(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_isRegistered(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_isTransient(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_containerObject(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_class(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_classes(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_baseClassCount(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_baseClassesAll(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_derivedClasses(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_classTemplateMatchClause(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_derivedTemplateMatchClause(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);

    const char *xslFunction_isLoggedIn(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);

    /** \brief database:class-xschema([string className|node class:*, xschema-name, stand-alone, compiled]).
     * generate the inherited tech version of the xschema for the given Class
     *
     * caller frees result.
     * generates the amalgamated inherited xschema deifinition for the main class indicated by the parameter.
     * no parameters: the context node will be used and treated as a node (class:*) definition
     *
     * @param pQE
     * @param pXCtxt
     * @param [out] pNodes
     *
     * @param @className this string will be used to lookup the main class
     *     OR @node (class:*)  this node will be used to evaluate the target main class
     * @param @xschema-name optional name of the xschema name to return
     * @param @stand-alone all dependent types included (inheritance and element types)
     * @param @compiled all inherited fields compiled in to one linear xsd:complexType
     *
     * @see system_transforms/class_xschema.xsl
     * @return the generated xschema stylesheet
     */
    const char *xslFunction_classXSchema(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_classXSchemaNodeMask(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_classXStylesheet(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_dynamicStylesheet(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_descendantObjects(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_extensionElementPrefixes(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    /** @} */

    //----------------------------------------------------- write section
    //using TXml -> TXmlProcessor directly
    /**
     * \ingroup XSLModule-Commands
     */
    IXmlBaseNode *xslCommand_transaction( const IXmlQueryEnvironment *pQE, const IXslCommandNode *pCommandNode, const IXmlBaseNode *pSourceNode, IXmlBaseNode *pOutputNode);
    IXmlBaseNode *xslCommand_transactions(const IXmlQueryEnvironment *pQE, const IXslCommandNode *pCommandNode, const IXmlBaseNode *pSourceNode, IXmlBaseNode *pOutputNode);

    /**
     * \ingroup XSLModule-Functions
     */
    const char   *xslFunction_transaction(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);

    //write listening
    XmlNodeList<IXmlBaseNode> m_vEvents;
    size_t registerWriteEvent(const IXmlQueryEnvironment *pQE, IXmlBaseNode *pChangedNode);
    size_t releaseWriteListeners(const IXmlQueryEnvironment *pQE, const size_t iEventID);
    size_t waitForSingleWriteEvent(const IXmlQueryEnvironment *pQE, const IXmlArea *pArea, const size_t iLastKnownEventID = 0);
  private:
    static void writeListenerManagementLock();
    static void writeListenerManagementUnlock();

  private: //called internally only by our own inherited TXmlProcessor::applyTransaction()
    /**
     * @name ITXmlDirectFunctions
     * #define ITDF_RETURN           TXml::transactionResult
     * #define ITDF_SECURITY_PARAMS  const char *sUserId, const char *sUserStoreLocation, const char *sReadOPUserUUIDForTransform
     * #define ITDF_BASE_PARAMS      const char *sBaseXPath, const char *sSelectXPath
     * #define ITDF_TRANSFORM_PARAMS const char *sTransformWithXPath, const char *sWithParam, const char *sWithMode
     * #define ITDF_RESULT_PARAMS    IXmlBaseNode **ppResultNode, const char **sSelectFQNodeName, const char **sSelectXmlId, const char **sSelectRepositoryName, const char **sSelectParentXPath, const char **sExtensionElementPrefixes, const char **sNewResultXslModuleManagerName
     *
     * @{
     */
    ITDF_RETURN moveChild(         ITDF_SECURITY_PARAMS, ITDF_BASE_PARAMS, const IXmlBaseNode *pTSelectNode, const char *sDestinationXPath, const char *sBeforeXPath, const int iDestinationPosition, ITDF_TRANSFORM_PARAMS, ITDF_RESULT_PARAMS);
    ITDF_RETURN copyChild(         ITDF_SECURITY_PARAMS, ITDF_BASE_PARAMS, const IXmlBaseNode *pTSelectNode, const char *sDestinationXPath, const char *sBeforeXPath, const int iDestinationPosition, const bool bDeepClone, ITDF_TRANSFORM_PARAMS, ITDF_RESULT_PARAMS);
    ITDF_RETURN deviateNode(       ITDF_SECURITY_PARAMS, ITDF_BASE_PARAMS, const char *sHardlinkXPath, const char *sNewNodeXPath, ITDF_RESULT_PARAMS);
    ITDF_RETURN hardlinkChild(     ITDF_SECURITY_PARAMS, ITDF_BASE_PARAMS, const char *sDestinationXPath, const char *sBeforeXPath, const int iDestinationPosition, ITDF_RESULT_PARAMS);
    ITDF_RETURN softlinkChild(     ITDF_SECURITY_PARAMS, ITDF_BASE_PARAMS, const char *sDestinationXPath, const char *sBeforeXPath, ITDF_RESULT_PARAMS);
    ITDF_RETURN replaceNodeCopy(   ITDF_SECURITY_PARAMS, ITDF_BASE_PARAMS, const IXmlBaseNode *pTSelectNode, const char *sDestinationXPath, ITDF_TRANSFORM_PARAMS, ITDF_RESULT_PARAMS);
    ITDF_RETURN mergeNode(         ITDF_SECURITY_PARAMS, ITDF_BASE_PARAMS, const IXmlBaseNode *pTSelectNode, const char *sDestinationXPath, ITDF_TRANSFORM_PARAMS, ITDF_RESULT_PARAMS);
    ITDF_RETURN setAttribute(      ITDF_SECURITY_PARAMS, ITDF_BASE_PARAMS, const char *sName, const char *sValue, const char *sNamespace, ITDF_RESULT_PARAMS);
    ITDF_RETURN setValue(          ITDF_SECURITY_PARAMS, ITDF_BASE_PARAMS, const char *sValue, ITDF_RESULT_PARAMS);
    ITDF_RETURN removeNode(        ITDF_SECURITY_PARAMS, ITDF_BASE_PARAMS, ITDF_RESULT_PARAMS);
    ITDF_RETURN createChildElement(ITDF_SECURITY_PARAMS, ITDF_BASE_PARAMS, const char *sName, const char *sValue, const char *sNamespace, ITDF_RESULT_PARAMS);
    ITDF_RETURN changeName(        ITDF_SECURITY_PARAMS, ITDF_BASE_PARAMS, const char *sName, ITDF_RESULT_PARAMS);
    ITDF_RETURN touch(             ITDF_SECURITY_PARAMS, ITDF_BASE_PARAMS, ITDF_RESULT_PARAMS);
    /** @} */

    //Transaction (temporary) display
    IXmlBaseNode *transactionDisplayNode(IXmlBaseNode *pSerialisationNode = 0);
    const vector<TXmlProcessor*> *linkedTXmlProcessors(IXmlBaseNode *pSerialisationNode = 0) const;

    //---------------------------------------------------- interface navigation
  public:
    ITXmlDirectFunctions *queryInterface(ITXmlDirectFunctions *p ATTRIBUTE_UNUSED) {return this;}
  };
}
#endif
