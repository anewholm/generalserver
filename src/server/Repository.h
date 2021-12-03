//platform agnostic file
#ifndef _REPOSITORY_H
#define _REPOSITORY_H

//standard library includes
#include <vector>
using namespace std;

//aheads
namespace general_server {class Repository;}

#include "define.h"
#include "Utilities/StringMap.h"

#include "TXml.h"                //enums
#include "TXmlProcessor.h"       //direct inheritance
#include "DatabaseNodeServerObject.h" //direct inheritance GeneralServerDatabaseNodeServerObject

//platform specific project headers
#include "LibXslModule.h"

//for injection in to XML read streams
#define NAMESPACE_REPOSITORY_XMLNS "xmlns:" NAMESPACE_REPOSITORY_ALIAS "=\"" NAMESPACE_REPOSITORY "\""
#define REPOSITORY_ROOT_NAME "root"
#define EOL_SLOT 1U
#define ELEMENT_ONLY 1
#define FULL_TREE 0
#define NO_PLACEHOLDER false
#define TRANSACTION_FILE "transactions/transactions.xml"
#define NOT_TOP false
#define NO_REGISTER false
#define NO_PARENT_REPOSITORY NULL

namespace general_server {
  class Repository;
  class QueryEnvironment;

  class Repository: public SERVER_MAIN_XSL_MODULE, public vector<const Repository*>, public TXmlProcessor, public GeneralServerDatabaseNodeServerObject, implements_interface IReportable {
    IFDEBUG(friend class Main;) //TODO: friend class Main is just to allow Main experiments

    //------------------------------- Permanent Store of information
    //  Repository: A permanent, slow, *non-queryable* structure XML store
    //  IXmlDoc:    A temporary, high-speed, queryable           XML store
    //Usually a wrapper for the FileSystem, but could be a wrapper for another store
    //has a DUAL PURPOSE: can also be used as an XslModule
    //Database: Repositories need an associated IXmlDoc in order to sync changes
    //has parent and children Repository available

    //------------------------------- Reading can happen in many ways:
    //  1) steaming in to the xmlLibrary parser (main read of Directory using LibXML parser -> doc)
    //  2) read raw content                     (-> RegularX -> nodes)
    //  3) create nodes in to output node       (FileSystem browsing in tree -> nodes)
    //  4) read valid XML                       (InternetLocation reads of foreign websites -> parseXML(..., output node))

    //------------------------------- Construction
    //Factory pattern: http://en.wikipedia.org/wiki/Factory_method_pattern#Definition
    //1) create a derived class based on the format of the sFilePath
    //  calls populateChildren(...) to get all sub-repositories first before reading all XML
    //  it is the top level, e.g. a Directory with sub-repositories
    //  used for XML string stream
    //2) create from an existing DatabaseNode to allow saving to its source Repository
    //  enables write-back
    //  called only by nearestSubRepository(const IXmlBaseNode *pAffectedNode)
    //  uses the type 1 m_pParentRepository to store the root one (for absolute filesystem path root)
    //  fills out its m_sFullPath dynamically from the ancestor::*[@preository:name] path attributes

  protected:
    //helper function for xml(...)
    //used in DatabaseNode mode (2) during saves
    const char *nodeXML(const DatabaseNode *pNode, const int iMaxDepth = FULL_TREE) const;

    class FilenamePred {
      const char *m_sFilename; //caller manages this
    public:
      FilenamePred(const char *sFilename): m_sFilename(sFilename) {}
      bool operator()(const Repository* pRep) const {return _STREQUAL(pRep->name(), m_sFilename);}
    };

  private:
    //SERVER_MAIN_XSL_MODULE only
    static StringMap<IXslModule::XslModuleCommandDetails> m_XSLCommands;
    static StringMap<IXslModule::XslModuleFunctionDetails> m_XSLFunctions;

  protected:
    //known derived classes
    enum repositoryType {
      tUnknownRepositoryType,
      tInternetLocation,
      tResourcesStore,
      tDirectory,
      tDirectoryDetails,
      tDirectoryLayout,
      tDOCTypeDefinition,
      tCommitFileTemporary,
      tMetaFile,
      tEditorBackupFile,
      tDatabaseStatusFile,
      tFile,
      tHardlink,
      tMySQL,
      tPostgres,
      tAutoDetect //calculate from  the associated sFullpath
    };

    struct AffectedRepositoryDetails {
      TXml::affectedScope iAffectedScope; //saveElementOnly, syncImmediateChildren, saveTree, linkChild
      Repository *pRelatedRepository;
    };

    //mode (1) called from sub-classes like InternetLocation, File, Directory
    //  calls populateChildren(...)
    Repository(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlLibrary *pLib, const char *sFullPath, const Repository *pParentRepository = NULL);
    //mode (2) savingzis
    Repository(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlLibrary *pLib, const DatabaseNode *pNode, const Repository *pRootRepository, const bool bRegisterNode = true);

    //repository sub-tree knowledge
    virtual void populateChildren(const bool bForce = false); //startup populate hierarchy call
    Repository *nearestSubRepository(const IXmlBaseNode *pAffectedNode) const;
    void affectedSubRepositories(vector<AffectedRepositoryDetails> *pvAffectSubRepositories, const vector<TXml::AffectedNodeDetails> *pvAffectedNodes) const;
    static bool rationaliseAffectedRepositoriesVector(vector<AffectedRepositoryDetails> *pvAffectedRepositories);

    const IXmlLibrary *m_pLib;   //comes from different sources depending on instanciation type
    const Repository *m_pParent; //hierarchy only
    const char *m_sFullPath;     //location info, from instanciation
    const char *m_sProtocol;     //alpha before first colon :, e.g. postgres:...
    const char *m_sName;         //this position is important because m_sName relies on m_sFullPath in initialisation lists
    const char *m_sExtension;    //optional type info pointer in to m_sName, so dont free!
    const char *m_xml_filename;  //completed on creation

    //fullpath names and parts
    virtual const char *xml_filename(const char *sFullPath) const;
    virtual const char *protocol( const char *sFullPath) const {return protocolStandard( sFullPath);}
    virtual const char *name(     const char *sFullPath) const {return nameStandard(     sFullPath);}
    virtual const char *extension(const char *sFullPath) const {return extensionStandard(sFullPath);}
    static bool isProtocol(const char *sFilePath, const char *sProtocol);

    virtual bool includeFileAsNode() const; //extra file node around contents when reading XML
    string baseFileSystemPath(const DatabaseNode *pSubRepositoryNode = 0, const Repository *pRootRepository = 0) const;

    //static define.h XML component lengths
    static size_t m_iRepXMLNSLen;
    static size_t m_iRepNSAliasLen;
    static size_t m_iRootNameLen;
    static size_t m_iNSDefaultLen;
    static size_t m_iNSAllLen;


  public:
    //------------------------------------- construction (3 types)
    //Factory pattern: http://en.wikipedia.org/wiki/Factory_method_pattern#Definition
    //  factory works on the type of path provided, e.g. http://...
    //1) create a derived class based on the format of the sFilePath
    //  calls populateChildren(...) to get all sub-repositories first before reading all XML
    //  it is the top level, e.g. a Directory with sub-repositories
    //  used for XML string stream
    static Repository *factory_reader(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlLibrary *pLib, const char *sFullPath, const Repository *pParentRepository = NULL, const char *sArg = NULL);

    //2) create from an existing DatabaseNode to allow saving to its source Repository
    //  enables write-back
    //  called only by nearestSubRepository(const IXmlBaseNode *pAffectedNode)
    //  uses the type 1 m_pParentRepository to store the root one (for absolute filesystem path root)
    //  fills out its m_sFullPath dynamically from the ancestor::*[@preository:name] path attributes
    static Repository *factory_writable(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlLibrary *pLib, const DatabaseNode *pNode, const Repository *pRootRepository, repositoryType rt = tAutoDetect);

    virtual ~Repository();

    //Repository modes
    bool hasBasePath() const {return m_sFullPath != 0;}
    bool isReadable()  const {return m_sFullPath != 0;}
    bool isWriteable() const {return m_pNode != 0;}
    bool isRepositoryNode(const DatabaseNode *pNode) const;

    static repositoryType parseRepositoryType(const char *sRepositoryTypeName);
    const char *getRepositoryTypeName() const;

    //DatabaseNode mode
    //filesystem location
    void setFullPath(const char *sFullPath);
    void setDynamicFullPath();
    virtual void applyDirectives(Repository *pTarget = 0); //e.g. ordering
    virtual const char *dynamicFileSystemPath(DatabaseNode *pNode = 0, const Repository *pRootRepository = 0) const;
    //direct writes to device
    virtual void saveFromDatabaseNode(const char *sFullPath ATTRIBUTE_UNUSED = 0) const {throw CapabilityNotSupported(this, MM_STRDUP("save"));} //can be binary data
    virtual const char *xmlFromDatabaseNode()                    const {throw CapabilityNotSupported(this, MM_STRDUP("xmlFromDatabaseNode"));} //can be binary data

    //accessors, utilities and type info
    virtual const char *toString() const;
    bool in(const vector<Repository*> *pVec) const;
    bool in(const vector<AffectedRepositoryDetails> *pVec) const;
    bool is(const Repository *pRepository) const;
    virtual bool canSaveElementOnly() const {return false;}
    virtual bool shouldIncludeSubRepository(const Repository *pSubRepository ATTRIBUTE_UNUSED) const {return false;}
    virtual bool removeDuringSync() const {return false;}
    virtual bool includeInXML() const {return true;}

    static bool gimme(const char *sFilePath ATTRIBUTE_UNUSED) {return false;}
    virtual repositoryType type() const; //tDirectory, tFile, tInternetLocation
    const char *source() const;                    //Directory, File, InternetLocation

    //path details
    const char *fullPath()  const {return m_sFullPath;}
    const char *protocol()  const {return m_sProtocol;}
    const char *name()      const {return m_sName;}
    const char *extension() const {return m_sExtension;}
    static const char *protocolStandard( const char *sFullPath);
    static const char *nameStandard(     const char *sFullPath);
    static const char *extensionStandard(const char *sFullPath);

    //------------------------------------- capabilities
    bool supportsStreaming() const {return CAPABILITY_IMPLEMENTED;}
    bool supportsTXML()      const {return CAPABILITY_IMPLEMENTED;}
    bool isHierarchy()       const {return CAPABILITY_IMPLEMENTED;}

    //------------------------------------- read
    //direct read from device
    //these are available through the XSL Module interface also
    virtual void readValidXML( string &sCurrentXML ATTRIBUTE_UNUSED)  const {throw CapabilityNotSupported(this, MM_STRDUP("read to valid XML"));}
    virtual const char   *readRaw()                  const {throw CapabilityNotSupported(this, MM_STRDUP("read raw"));}
    virtual vector<char> *readBinary()               const {throw CapabilityNotSupported(this, MM_STRDUP("read raw binary"));}
    virtual void readToNodes(const IXmlQueryEnvironment *pQE ATTRIBUTE_UNUSED, IXmlBaseNode *pOutput ATTRIBUTE_UNUSED, const char *sXPathQuery ATTRIBUTE_UNUSED, const char *sXPathMask ATTRIBUTE_UNUSED = 0)
                                                     const {throw CapabilityNotSupported(this, MM_STRDUP("read to output node"));}
    //string XML -> IXmlDoc streaming
    //live, uncached from filesystem with context and file pointers passed around
    //so no member variables, hence the const Repositories
    class StreamingContext {
    public:
      const Repository *m_pRootRepository;
      const Repository *m_pCurrentRepository;
      string      m_sCurrentXML;
      size_t      m_iCurrentXMLPosition;
      StreamingContext(Repository *pRepository): m_pRootRepository(pRepository), m_pCurrentRepository(pRepository), m_sCurrentXML(""), m_iCurrentXMLPosition(0) {}
    };
    virtual StreamingContext *newStreamingContext(Repository *pRepository) {return new StreamingContext(pRepository);}
    static int static_inputReadCallback(void * context, char * buffer, int len);
    static int static_inputCloseCallback(void * context);
    virtual int inputReadCallback( StreamingContext *pStreamingContext ATTRIBUTE_UNUSED, char * buffer ATTRIBUTE_UNUSED, int len ATTRIBUTE_UNUSED) const {return 0;} //required if implemented
    virtual int inputCloseCallback(StreamingContext *pStreamingContext ATTRIBUTE_UNUSED) const {return 0;} //optional
    virtual void appendXmlTo_preCursor(string &sCurrentXML ATTRIBUTE_UNUSED, const bool bIsTop ATTRIBUTE_UNUSED = false) const {} //e.g. DTD
    virtual void appendXmlTo_startTag( string &sCurrentXML ATTRIBUTE_UNUSED, const bool bIsTop ATTRIBUTE_UNUSED = false) const {} //e.g. Directory starts / File node starts
    virtual void appendXmlTo_content(  string &sCurrentXML ATTRIBUTE_UNUSED, const bool bIsTop ATTRIBUTE_UNUSED = false) const {} //e.g. File contents
    virtual void appendXmlTo_endTag(   string &sCurrentXML ATTRIBUTE_UNUSED, const bool bIsTop ATTRIBUTE_UNUSED = false) const {} //e.g. Directory closes

    //DOM style isHierarchy() navigation
    const Repository *firstChild(const Repository::repositoryType iRepositoryType = tUnknownRepositoryType) const;
    const Repository *parent() const;
    const_iterator    find(const Repository *pSubRepository) const;
    const Repository *nextSibling(const Repository *pRepository) const;
    const Repository *walkHierarchy(const Repository *pRepository) const;

    //for URLs, UNIX and Windows
    static int   issplitter(char c) {return c == '/' || c == '\\';}
    static const char *strrsplitter(const char *sInput);
    static const char *strsplitter(const char *sInput);
    static char  splitter() {
#ifdef PATH_SPLITTER
      return PATH_SPLITTER;
#else
      return '/';
#endif
    }
    static const char *canonicalise(const char *sRawPath);

    //string xml navigation
    //for quick easy non-parsed XML structures
    static const char *tagStart(  const char *sTagStart);
    static const char *tagNameEnd(const char *sTagStart);
    static const char *tagEnd(    const char *sTagStart);
    static const char *tagEndBack(const char *sTagStart);
    static const char *tagAttributeStart(     const char *sTagStart, const char *sAttributeName);
    static const char *tagAttributeValueStart(const char *sTagStart, const char *sAttributeName);
    static const char *tagAttributeValue(     const char *sTagStart, const char *sAttributeName);

    //XSLModule interface
    const IXmlLibrary *xmlLibrary()   const;
    const char *xsltModuleNamespace() const;
    const char *xsltModulePrefix()    const;
    const StringMap<IXslModule::XslModuleCommandDetails>  *xslCommands()  const;
    const StringMap<IXslModule::XslModuleFunctionDetails> *xslFunctions() const;

    /** \addtogroup XSLModule-Functions
     * @{
     */
    const char *xslFunction_fileSystemPathToNodeList(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_fileSystemPathToXPath(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_rx(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_readValidXML(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_readRaw(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_readToNodes(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    const char *xslFunction_node(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes);
    /** @} */

    //------------------------------------- write (commits)
    virtual TXml::transactionResult applyTransaction(TXml *t ATTRIBUTE_UNUSED, IXmlBaseNode *pSerialisationNode ATTRIBUTE_UNUSED = 0, const IXmlQueryEnvironment *pQE ATTRIBUTE_UNUSED = 0) {throw CapabilityNotSupported(this, MM_STRDUP("applyTransaction")); return TXml::transactionFailure;}
    virtual TXml::transactionResult createSaveTreePlaceholder(const bool bAddPlaceholderPrefix ATTRIBUTE_UNUSED = true) {throw CapabilityNotSupported(this, MM_STRDUP("createSaveTreePlaceholder")); return TXml::transactionFailure;}
    virtual TXml::transactionResult createSaveTreePlaceholderAdditions() {return TXml::transactionFailure;}
    virtual TXml::transactionResult swapInSaveTreePlaceholder()    {throw CapabilityNotSupported(this, MM_STRDUP("swapInPlaceholder")); return TXml::transactionFailure;}
    virtual TXml::transactionResult createElementOnlyPlaceholder() {throw CapabilityNotSupported(this, MM_STRDUP("createElementOnlyPlaceholder")); return TXml::transactionFailure;}
    virtual TXml::transactionResult swapInElementOnlyPlaceholder() {throw CapabilityNotSupported(this, MM_STRDUP("swapInElementOnlyPlaceholder")); return TXml::transactionFailure;}
    virtual TXml::transactionResult syncImmediateChildren()        {throw CapabilityNotSupported(this, MM_STRDUP("syncImmediateChildren")); return TXml::transactionFailure;}
    virtual TXml::transactionResult rollback()                     {throw CapabilityNotSupported(this, MM_STRDUP("rollback")); return TXml::transactionFailure;}

    static void compilationInformation(const IXmlQueryEnvironment *pQE ATTRIBUTE_UNUSED, IXmlBaseNode *pOutput ATTRIBUTE_UNUSED) {}
  };
}

#endif
