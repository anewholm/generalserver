//platform agnostic file
#include "Repository.h"

#include <cctype>

#include "FileSystem.h"
#include "RelationalDatabases.h"
#include "Xml/XmlAdminQueryEnvironment.h"
#include "QueryEnvironment/RepositorySaveSecurityContext.h"
#include "IXml/IXslNode.h"
#include "IXml/IXslDoc.h"
#include "IXml/IXslTransformContext.h"
#include "IXml/IXslXPathFunctionContext.h"
#include "IXml/IXmlBaseNode.h"
#include "RegularX.h"
#include "Server.h"
#include "Database.h"
#include "DatabaseNode.h"

#include "Utilities/strtools.h"
#include "Utilities/container.c"

#include <algorithm>
using namespace std;

namespace general_server {
  StringMap<IXslModule::XslModuleCommandDetails>  Repository::m_XSLCommands;
  StringMap<IXslModule::XslModuleFunctionDetails> Repository::m_XSLFunctions;

  size_t Repository::m_iRepXMLNSLen   = strlen(NAMESPACE_REPOSITORY_XMLNS);
  size_t Repository::m_iRepNSAliasLen = strlen(NAMESPACE_REPOSITORY_ALIAS);
  size_t Repository::m_iRootNameLen   = strlen(REPOSITORY_ROOT_NAME);
  size_t Repository::m_iNSDefaultLen  = strlen(NAMESPACE_DEFAULT);
  size_t Repository::m_iNSAllLen      = strlen(NAMESPACE_ALL);

  Repository *Repository::factory_writable(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlLibrary *pLib, const DatabaseNode *pNode, const Repository *pRootRepository, repositoryType rt) {
    //Administration function used by:
    //  commit process -> FileSystem::createSaveTreePlaceholder()
    //  commit process -> Repository::affectedSubRepositories() -> Repository::nearestSubRepository()
    const XmlAdminQueryEnvironment ibqe(pMemoryLifetimeOwner, pNode->document_const(), new RepositorySaveSecurityContext()); //commit process: Repository::factory(DatabaseNode) reading node
    Repository *pRepository     = 0;
    const char *sRepositoryType = 0;

    //Repository with an associated IXmlBaseNode
    //we DO NOT use m_sFullPath directly here, it is dynamically calculated
    //writeable repositories (from node) this means that it can write back directly from the xml(...)
    //this node might have moved so we want to figure out it's position from the @repository:name hierarchy
    assert(pNode);
    //root relative repository has to be a base read one that has a m_sFullPath
    assert(pRootRepository && pRootRepository->hasBasePath());
    assert(!pNode->isTransient()); //the QE, RepositorySaveSecurityContext, should prevent discovery of transient nodes
      
    //will throw an error if this is not a @repository:name node
    //we still want to generate the correct type of Repository from the primary factory pattern
    if (rt == tAutoDetect) { //default
      if      (pNode->isHardLink()) rt = tHardlink;
      else if (sRepositoryType = pNode->attributeValueDirect(&ibqe, "type", NAMESPACE_REPOSITORY)) {
        rt = parseRepositoryType(sRepositoryType);
      } else  throw RepositoryTypeMissing(pMemoryLifetimeOwner);
    }

    switch (rt) {
      case tCommitFileTemporary: {pRepository = new CommitFileTemporary(pMemoryLifetimeOwner, pLib, pNode, pRootRepository);break;}
      case tInternetLocation:    {pRepository = new InternetLocation(   pMemoryLifetimeOwner, pLib, pNode, pRootRepository);break;}
      case tResourcesStore:      {pRepository = new ResourcesStore(     pMemoryLifetimeOwner, pLib, pNode, pRootRepository);break;}
      case tDirectory:           {pRepository = new Directory(          pMemoryLifetimeOwner, pLib, pNode, pRootRepository);break;}
      case tDirectoryDetails:    {pRepository = new DirectoryDetails(   pMemoryLifetimeOwner, pLib, pNode, pRootRepository);break;}
      case tDOCTypeDefinition:   {pRepository = new DOCTypeDefinition(  pMemoryLifetimeOwner, pLib, pNode, pRootRepository);break;}
      case tMetaFile:            {pRepository = new MetaFile(           pMemoryLifetimeOwner, pLib, pNode, pRootRepository);break;}
      case tEditorBackupFile:    {pRepository = new EditorBackupFile(   pMemoryLifetimeOwner, pLib, pNode, pRootRepository);break;}
      case tDatabaseStatusFile:  {pRepository = new DatabaseStatusFile( pMemoryLifetimeOwner, pLib, pNode, pRootRepository);break;}
      case tHardlink:            {pRepository = new Hardlink(           pMemoryLifetimeOwner, pLib, pNode, pRootRepository);break;}
      case tPostgres:            {pRepository = new Postgres(           pMemoryLifetimeOwner, pLib, pNode, pRootRepository);break;}
      case tMySQL:               {pRepository = new MySQL(              pMemoryLifetimeOwner, pLib, pNode, pRootRepository);break;}
      case tFile:                {pRepository = new File(               pMemoryLifetimeOwner, pLib, pNode, pRootRepository);break;}
      case tDirectoryLayout:     {pRepository = new DirectoryLayout(    pMemoryLifetimeOwner, pLib, pNode, pRootRepository);break;}
      case tAutoDetect: ATTRIBUTE_FALLTHROUGH;
      case tUnknownRepositoryType: throw RepositoryTypeUnknown(pMemoryLifetimeOwner, sRepositoryType);
    }
    if (!pRepository) throw RepositoryTypeUnknown(pMemoryLifetimeOwner, sRepositoryType);

    //virtual post-construction call
    if (pRepository) pRepository->setDynamicFullPath();

    //free up
    //if (sRepositoryType) MM_FREE(sRepositoryType); //attributeValueDirect()

    return pRepository;
  }

  Repository *Repository::factory_reader(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlLibrary *pLib, const char *sFullPath, const Repository *pParentRepository, const char *sArg) {
    //caller frees sFullPath and pParentRepository
    //readData-only repositories
    Repository *pRepository = 0;

    //order is important here
    //this is all hard coded rather than Derived classes registering themselves
    //because the functionality must be built in anyway at compile time
    if      (InternetLocation::gimme(sFullPath))    pRepository = new InternetLocation(   pMemoryLifetimeOwner, pLib, sFullPath, pParentRepository, sArg); //URL, e.g. http://google.com
    else if (Postgres::gimme(sFullPath))            pRepository = new Postgres(           pMemoryLifetimeOwner, pLib, sFullPath, pParentRepository, sArg); //postgres:...
    else if (MySQL::gimme(sFullPath))               pRepository = new MySQL(              pMemoryLifetimeOwner, pLib, sFullPath, pParentRepository, sArg); //mysql:...
    else if (EditorBackupFile::gimme(sFullPath))    pRepository = new EditorBackupFile(   pMemoryLifetimeOwner, pLib, sFullPath, pParentRepository, sArg); //css.xsl~
    else if (CommitFileTemporary::gimme(sFullPath)) pRepository = new CommitFileTemporary(pMemoryLifetimeOwner, pLib, sFullPath, pParentRepository, sArg); //0_commit_placeholder_*
    else if (ResourcesStore::gimme(sFullPath))      pRepository = new ResourcesStore(     pMemoryLifetimeOwner, pLib, sFullPath, pParentRepository, sArg); //resources
    else if (Directory::gimme(sFullPath))           pRepository = new Directory(          pMemoryLifetimeOwner, pLib, sFullPath, pParentRepository, sArg); //directory, e.g. c:/windows/
    else if (DirectoryDetails::gimme(sFullPath))    pRepository = new DirectoryDetails(   pMemoryLifetimeOwner, pLib, sFullPath, pParentRepository, sArg); //directory.metaxml
    else if (DirectoryLayout::gimme(sFullPath))     pRepository = new DirectoryLayout(    pMemoryLifetimeOwner, pLib, sFullPath, pParentRepository, sArg); //.directory
    else if (DOCTypeDefinition::gimme(sFullPath))   pRepository = new DOCTypeDefinition(  pMemoryLifetimeOwner, pLib, sFullPath, pParentRepository, sArg); //DOCTYPE.dtd
    else if (MetaFile::gimme(sFullPath))            pRepository = new MetaFile(           pMemoryLifetimeOwner, pLib, sFullPath, pParentRepository, sArg); //hidden file, e.g. /root/.directory.xml
    else if (DatabaseStatusFile::gimme(sFullPath))  pRepository = new DatabaseStatusFile( pMemoryLifetimeOwner, pLib, sFullPath, pParentRepository, sArg); //dbstatus.metaxml
    else if (Hardlink::gimme(sFullPath))            pRepository = new Hardlink(           pMemoryLifetimeOwner, pLib, sFullPath, pParentRepository, sArg); //hardlink, e.g. shared.hardlink
    else if (File::gimme(sFullPath))                pRepository = new File(               pMemoryLifetimeOwner, pLib, sFullPath, pParentRepository, sArg); //file, e.g. c:\windows\test.xml
    else throw RepositoryTypeUnknown(pMemoryLifetimeOwner, sFullPath);

    //virtual post-construction call
    if (pRepository) pRepository->populateChildren();

    return pRepository;
  }

  Repository::Repository(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlLibrary *pLib, const char *sFullPath, const Repository *pParent):
    MemoryLifetimeOwner(pMemoryLifetimeOwner),
    GeneralServerDatabaseNodeServerObject(pMemoryLifetimeOwner),
    TXmlProcessor(pMemoryLifetimeOwner),
    m_pLib(pLib),
    m_pParent(pParent),
    m_sFullPath(0),
    m_sProtocol(0),
    m_sName(0),
    m_sExtension(0),
    m_xml_filename(0)
  {
    //caller frees sFullPath and pParent
    //we copy sFullPath
    //readData-only repository instanciation
    setFullPath(MM_STRDUP(sFullPath));

    //call the virtual populateChildren() after concrete construction
    //if sub-repository traversal is required

    assert(m_pLib);
    assert(m_sFullPath);
    assert(m_sName);
  }
  
  Repository::Repository(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlLibrary *pLib, const DatabaseNode *pNode, const Repository *pRootRepository, const bool bRegisterNode):
    MemoryLifetimeOwner(pMemoryLifetimeOwner),
    GeneralServerDatabaseNodeServerObject(pNode, bRegisterNode),
    TXmlProcessor(pMemoryLifetimeOwner),
    m_pLib(pLib),
    m_pParent(pRootRepository),
    m_sFullPath(0),
    m_sProtocol(0),
    m_sName(0),
    m_sExtension(0),
    m_xml_filename(0)
  {
    //this frees pNode
    //caller frees pParent
    //writeData-only repository instanciation

    //call the virtual dynamicFileSystemPath(...) to generate setFullPath(...)
    //from the associated GeneralServerDatabaseNodeServerObject::m_pNode if saving is required

    //write repository instanciation
    assert(m_pLib);
    assert(m_pNode);
    assert(m_pParent);
  }

  Repository::~Repository() {
    if (m_sFullPath)    MMO_FREE(m_sFullPath);
    if (m_sProtocol)    MMO_FREE(m_sProtocol);
    if (m_sName)        MMO_FREE(m_sName);
    if (m_xml_filename) MMO_FREE(m_xml_filename);
    //if (m_pNode)        delete m_pNode;             //GeneralServerDatabaseNodeServerObject holds and deletes this
    //if (m_sExtension)   MMO_FREE(m_sExtension); //points to the extension IN m_sFullPath
  }

  const char *Repository::nodeXML(const DatabaseNode *pNode, const int iMaxDepth) const {
    //commit process: Repository::nodeXML() to write the full contents
    const XmlAdminQueryEnvironment ibqe(this, pNode->document_const(), new RepositorySaveSecurityContext());
    const char *sXML = pNode->node_const()->xml(&ibqe, iMaxDepth);
    //delete rssc;     //~XmlAdminQueryEnvironment always owns m_pSec
    return sXML;
  }

  void Repository::setDynamicFullPath() {
    assert(m_pNode);

    m_sFullPath    = dynamicFileSystemPath(m_pNode, m_pParent); //virtual call
    m_sProtocol    = protocol(m_sFullPath);
    m_sName        = name(m_sFullPath);
    m_sExtension   = extension(m_sFullPath);
    m_xml_filename = xml_filename(m_sFullPath);
  }

  void Repository::setFullPath(const char *sFullPath) {
    assert(sFullPath);

    m_sFullPath    = sFullPath;
    m_sProtocol    = protocol(m_sFullPath);
    m_sName        = name(m_sFullPath);
    m_sExtension   = extension(m_sFullPath);
    m_xml_filename = xml_filename(m_sFullPath);
  }
  const char *Repository::getRepositoryTypeName() const {
    const char *sRepositoryTypeName = "Unknown";
    switch (type()) {
      case tResourcesStore:      {sRepositoryTypeName = ResourcesStore::m_sTypeName; break;}
      case tInternetLocation:    {sRepositoryTypeName = InternetLocation::m_sTypeName; break;}
      case tDirectory:           {sRepositoryTypeName = Directory::m_sTypeName; break;}
      case tDirectoryDetails:    {sRepositoryTypeName = DirectoryDetails::m_sTypeName; break;}
      case tDirectoryLayout:     {sRepositoryTypeName = DirectoryLayout::m_sTypeName; break;}
      case tDOCTypeDefinition:   {sRepositoryTypeName = DOCTypeDefinition::m_sTypeName; break;}
      case tCommitFileTemporary: {sRepositoryTypeName = CommitFileTemporary::m_sTypeName; break;}
      case tMetaFile:            {sRepositoryTypeName = MetaFile::m_sTypeName; break;}
      case tEditorBackupFile:    {sRepositoryTypeName = EditorBackupFile::m_sTypeName; break;}
      case tDatabaseStatusFile:  {sRepositoryTypeName = DatabaseStatusFile::m_sTypeName; break;}
      case tHardlink:            {sRepositoryTypeName = Hardlink::m_sTypeName; break;}
      case tPostgres:            {sRepositoryTypeName = Postgres::m_sTypeName; break;}
      case tMySQL:               {sRepositoryTypeName = MySQL::m_sTypeName; break;}
      case tFile:                {sRepositoryTypeName = File::m_sTypeName; break;}
      case tAutoDetect:            break;
      case tUnknownRepositoryType: break;
    }
    return sRepositoryTypeName;
  }

  Repository::repositoryType Repository::parseRepositoryType(const char *sRepositoryTypeName) {
    repositoryType iRepositoryType = tUnknownRepositoryType;
    if (sRepositoryTypeName) {
      if      (!strcmp(sRepositoryTypeName, InternetLocation::m_sTypeName))    iRepositoryType = tInternetLocation;
      else if (!strcmp(sRepositoryTypeName, ResourcesStore::m_sTypeName))      iRepositoryType = tResourcesStore;
      else if (!strcmp(sRepositoryTypeName, Directory::m_sTypeName))           iRepositoryType = tDirectory;
      else if (!strcmp(sRepositoryTypeName, DirectoryDetails::m_sTypeName))    iRepositoryType = tDirectoryDetails;
      else if (!strcmp(sRepositoryTypeName, DirectoryLayout::m_sTypeName))     iRepositoryType = tDirectoryLayout;
      else if (!strcmp(sRepositoryTypeName, DOCTypeDefinition::m_sTypeName))   iRepositoryType = tDOCTypeDefinition;
      else if (!strcmp(sRepositoryTypeName, CommitFileTemporary::m_sTypeName)) iRepositoryType = tCommitFileTemporary;
      else if (!strcmp(sRepositoryTypeName, MetaFile::m_sTypeName))            iRepositoryType = tMetaFile;
      else if (!strcmp(sRepositoryTypeName, EditorBackupFile::m_sTypeName))    iRepositoryType = tEditorBackupFile;
      else if (!strcmp(sRepositoryTypeName, DatabaseStatusFile::m_sTypeName))  iRepositoryType = tDatabaseStatusFile;
      else if (!strcmp(sRepositoryTypeName, Hardlink::m_sTypeName))            iRepositoryType = tHardlink;
      else if (!strcmp(sRepositoryTypeName, File::m_sTypeName))                iRepositoryType = tFile;
      else if (!strcmp(sRepositoryTypeName, Postgres::m_sTypeName))            iRepositoryType = tPostgres;
      else if (!strcmp(sRepositoryTypeName, MySQL::m_sTypeName))               iRepositoryType = tMySQL;
      else if (!strcmp(sRepositoryTypeName, "AutoDetect"))                     iRepositoryType = tAutoDetect;
      else if (!strcmp(sRepositoryTypeName, "Unknown"))                        iRepositoryType = tUnknownRepositoryType;
    }
    return iRepositoryType;
  }

  const char *Repository::dynamicFileSystemPath(DatabaseNode *pSubRepositoryNode, const Repository *pRootRepository) const {
    return MM_STRDUP(baseFileSystemPath(pSubRepositoryNode, pRootRepository).c_str());
  }

  string Repository::baseFileSystemPath(const DatabaseNode *pSubRepositoryNode, const Repository *pRootRepository) const {
    //compiled base dynamic filesystem path of @repository:name's from pSubRepositoryNode to pRootRepository m_sFullPath
    //e.g. /media/Data/general_server/config/users (for the users Directory node)

    //used by:
    //  commit process -> Repository::setDynamicFullPath() -> derived Repositorys::dynamicFileSystemPath()s -> Repository::baseFileSystemPath()
    //  [shouldn't] be used by other processes: essentially to calculate the whereabouts of the permanent filesystem store file for commit only
    const XmlAdminQueryEnvironment ibqe(this, pSubRepositoryNode->document_const()); //commit process: Repository::baseFileSystemPath() to find the permanent store

    //flexible method allowing an (optional) root, a current this, and an (optional) sub-node
    //  with no root the document root will be used
    //  with a (placholder) root, usually with an altered m_sFullPath placeholder, the path can be changed at any level
    //  with a child pSubRepositoryNode, and a root of thisxmlNodeSetPtr seq to children can be efficiently calculated
    //use Derived::dynamicFileSystemPath() to add a slash to it, or /directorydetails.metaxml or whatever
    //saveAs(...) can be used in conjunction with an already calculated placeholder path
    assert(m_pNode);

    //optional arguments
    if (!pSubRepositoryNode) pSubRepositoryNode = m_pNode;
    if (!pRootRepository)    pRootRepository    = m_pNode->db()->repository();

    //requirements
    assert(pSubRepositoryNode);
    assert(pRootRepository);
    assert(pRootRepository->m_sFullPath && pRootRepository->m_pNode);

    const char *sRepositoryName;
    string sDynamicRepositoryPath;
    XmlNodeList<const IXmlBaseNode> *pvAncestors;
    XmlNodeList<const IXmlBaseNode>::reverse_iterator iAncestor;
    const IXmlBaseNode *pAncestor;
    bool bStartFound = false;

    //init
    sDynamicRepositoryPath = pRootRepository->m_sFullPath;
    pvAncestors            = pSubRepositoryNode->node_const()->ancestors(&ibqe);

    //work out current location backwards to top
    //ancestors includes /object:Server/...
    //m_sFullPath finishes with name and a slash (.../config/), which is erased here
    assert(sDynamicRepositoryPath.size());
    sDynamicRepositoryPath.resize(sDynamicRepositoryPath.size()-1);
    for (iAncestor = pvAncestors->rbegin(); iAncestor != pvAncestors->rend(); ++iAncestor) {
      pAncestor = *iAncestor;
      if (bStartFound) {
        //only include nodes that are Repositories
        //which should be all of them because only the first one can be a file, the rest are Directories
        if (sRepositoryName = pAncestor->attributeValueDirect(&ibqe, "name", NAMESPACE_REPOSITORY)) {
          sDynamicRepositoryPath += '/';
          sDynamicRepositoryPath += sRepositoryName;
          MMO_FREE(sRepositoryName);
        } else throw DynamicPathToNonRepositoryNode(this, pAncestor->uniqueXPathToNode(&ibqe, NO_BASE_NODE, INCLUDE_TRANSIENT));
      }
      //ignore the first one because we already included its m_sFullPath
      if (pAncestor->is(pRootRepository->m_pNode->node_const(), HARDLINK_AWARE)) bStartFound = true;
      delete pAncestor;
    }
    //add in self
    //note that the Directory override of dynamicFileSystemPath() will add a '/' on the end
    if (bStartFound) {
      sRepositoryName = pSubRepositoryNode->node_const()->attributeValueDirect(&ibqe, "name", NAMESPACE_REPOSITORY);
      if (!sRepositoryName) throw DynamicPathToNonRepositoryNode(this, pSubRepositoryNode->uniqueXPathToNode(&ibqe, NO_BASE_NODE, INCLUDE_TRANSIENT));
      sDynamicRepositoryPath += '/';
      sDynamicRepositoryPath += sRepositoryName;
    }

    //free up
    delete pvAncestors;

    return sDynamicRepositoryPath;
  }

  void Repository::populateChildren(const bool bForce)  {} //startup populate hierarchy call
  void Repository::applyDirectives(Repository *pTarget) {} //e.g. ordering

  const Repository *Repository::parent() const {
    return m_pParent;
  }
  const Repository *Repository::firstChild(const Repository::repositoryType iRepositoryType) const {
    const_iterator iChildRepository;
    const Repository *pChildRepository = 0;

    for (iChildRepository = begin(); iChildRepository != end(); iChildRepository++) {
      pChildRepository = (*iChildRepository);
      if (iRepositoryType == tUnknownRepositoryType || pChildRepository->type() == iRepositoryType) break;
      else pChildRepository = 0;
    }
    return pChildRepository;
  }
  Repository::const_iterator Repository::find(const Repository *pSubRepository) const {
    assert(pSubRepository);
    return std::find(begin(), end(), pSubRepository);
  }
  const Repository *Repository::nextSibling(const Repository *pRepository) const {
    const Repository *pParentRepository;
    Repository::const_iterator iChildRepository;

    if (pParentRepository = pRepository->parent()) { //might be zero if at top
      iChildRepository = pParentRepository->find(pRepository);
      if (iChildRepository++ == pParentRepository->end()) {
        pRepository = 0;
      } else {
        pRepository = *iChildRepository;
      }
    } else pRepository = 0;

    return pRepository;
  }

  const Repository *Repository::walkHierarchy(const Repository *pRepository) const {
    //returns zero at top
    //non-recursive walking (for event based streaming):
    //  down tree: first child
    //  across tree: next sibling
    //  up tree: parent next sibling
    //  exit

    if (pRepository->size()) {
      //down tree: first child
      //always happens if this is the top Repository of the tree
      pRepository = *pRepository->begin();
    } else {
      //across tree: next sibling
      //only happens after going down from top, so parent always valid at this point
      while (pRepository && !(pRepository = nextSibling(pRepository))) {
        //up tree: parent next sibling
        //returns zero at the top
        pRepository = pRepository->parent();
      }
    }

    return pRepository;
  }
  bool Repository::isRepositoryNode(const DatabaseNode *pNode) const {
    //used by:
    //  commit process: createSaveTreePlaceholder()
    const XmlAdminQueryEnvironment ibqe(this, pNode->document_const()); //commit process: Repository::isRepositoryNode() is node to be written
    return pNode && pNode->attributeExists(&ibqe, "repository:name");
  }

  bool Repository::includeFileAsNode() const {return false;}
  bool Repository::in(const vector<Repository*> *pVec) const {
    vector<Repository*>::const_iterator iNode;
    for (iNode = pVec->begin(); iNode != pVec->end(); iNode++) if (is(*iNode)) return true;
    return false;
  }
  bool Repository::is(const Repository *pRepository) const {
    assert(m_sFullPath);
    return _STREQUAL(m_sFullPath, pRepository->m_sFullPath);
  }

  const StringMap<IXslModule::XslModuleCommandDetails> *Repository::xslCommands() const {
    if (!m_XSLCommands.size()) {
    }
    return &m_XSLCommands;
  }

  const StringMap<IXslModule::XslModuleFunctionDetails> *Repository::xslFunctions() const {
    if (!m_XSLFunctions.size()) {
      m_XSLFunctions.insert("filesystempath-to-nodes", XMF(Repository::xslFunction_fileSystemPathToNodeList));
      m_XSLFunctions.insert("filesystempath-to-XPath", XMF(Repository::xslFunction_fileSystemPathToXPath));
      m_XSLFunctions.insert("rx",                      XMF(Repository::xslFunction_rx));
      m_XSLFunctions.insert("read-valid-xml",          XMF(Repository::xslFunction_readValidXML));
      m_XSLFunctions.insert("read-raw",                XMF(Repository::xslFunction_readRaw));
      m_XSLFunctions.insert("read-to-nodes",           XMF(Repository::xslFunction_readToNodes));
      m_XSLFunctions.insert("node",                    XMF(Repository::xslFunction_node));
    }
    return &m_XSLFunctions;
  }

  Repository::repositoryType Repository::type() const {
    //can happen with pure Repository objects
    //because this is not an interface class only
    assert(false);
    return tUnknownRepositoryType;
  }

  const char *Repository::source() const {
    //DO NOT free return value: it is a constant
    return getRepositoryTypeName();
  }
  const char *Repository::toString() const {
    stringstream sOut;
    sOut << "Repository [" << m_sName << "]";
    return MM_STRDUP(sOut.str().c_str());
  }

  const IXmlLibrary *Repository::xmlLibrary() const {
    return m_pLib;
  }

  bool Repository::in(const vector<AffectedRepositoryDetails> *pVec) const {
    bool bFound = false;
    vector<AffectedRepositoryDetails>::const_iterator iARDetail;

    for (iARDetail = pVec->begin(); !bFound && iARDetail != pVec->end(); iARDetail++) {
      if ((*iARDetail).pRelatedRepository->is(this)) bFound = true;
    }

    return bFound;
  }

  Repository *Repository::nearestSubRepository(const IXmlBaseNode *pAffectedNode) const {
    //used only by:
    //  commit process: TXmlProcessor::commit(...) -> commitCreatePlaceholders(...) -> affectedSubRepositories(...) -> nearestSubRepository(...)
    const XmlAdminQueryEnvironment ibqe(this, pAffectedNode->document()); //commit process: Repository::nearestSubRepository() find repository related to this changed node
    IFDEBUG(assert(m_pNode));

    const IXmlBaseNode *pNearestRepositoryNode;
    Repository *pRepository = 0;

    if (pNearestRepositoryNode = pAffectedNode->getSingleNode(&ibqe, "ancestor-or-self::*[@repository:name][1]")) {
      //throws RepositoryTypeMissing
      pRepository = Repository::factory_writable(mmParent(), m_pLib, m_pNode->factory_node(pNearestRepositoryNode), this);
    }

    return pRepository;
  }

  void Repository::affectedSubRepositories(vector<AffectedRepositoryDetails> *pvAffectSubRepositories, const vector<TXml::AffectedNodeDetails> *pvAffectedNodes) const {
    //used only by:
    //  commit process: TXmlProcessor::commit(...) -> commitCreatePlaceholders(...) -> affectedSubRepositories(...)

    vector<TXml::AffectedNodeDetails>::const_iterator iAffectedNode;
    TXml::AffectedNodeDetails andetails;
    const IXmlBaseNode *pAffectedNode;
    Repository *pNearestRepository;
    bool bSameNode;
    TXml::affectedScope iAffectedScope;

    for (iAffectedNode = pvAffectedNodes->begin(); iAffectedNode != pvAffectedNodes->end(); iAffectedNode++) {
      andetails     = *iAffectedNode;
      pAffectedNode = andetails.pRelatedNode;
      const XmlAdminQueryEnvironment ibqe(this, pAffectedNode->document()); //commit process: Repository::affectedSubRepositories()

      //can return 0x0 if not found
      if (pNearestRepository = nearestSubRepository(pAffectedNode)) {
        //if the nearest Repository is higher up then we need to saveTree on the content
        //this basically means that the change happened (syncImmediateChildren, saveElementOnly or saveTree) within a File Repository
        //bSameNode will always be true for Directory changes for example
        bSameNode      = (pNearestRepository->m_pNode->is(pAffectedNode, EXACT_MATCH));
        iAffectedScope = andetails.iAffectedScope;
        if (!bSameNode) iAffectedScope = TXml::saveTree;
        //saveElementOnly can only happen on same node Directory, not File
        //currently
        if (iAffectedScope == TXml::saveElementOnly && !pNearestRepository->canSaveElementOnly()) iAffectedScope = TXml::saveTree;

        Debug::report("      [%s/%s] nearest repository for [%s/%s] (%s:%s) %s",
          pNearestRepository->m_pNode->uniqueXPathToNode(&ibqe),
          pNearestRepository->name(),
          pAffectedNode->uniqueXPathToNode(&ibqe),
          pAffectedNode->fullyQualifiedName(),
          pNearestRepository->getRepositoryTypeName(),
          TXml::getAffectedScopeName(iAffectedScope),
          bSameNode ? "(same node)" : 0
        );
        AffectedRepositoryDetails ardetails = {iAffectedScope, pNearestRepository};
        pvAffectSubRepositories->push_back(ardetails);
      } else {
        //could return *this* instead
        throw SubRepositoryNotFoundForNode(this, pAffectedNode->uniqueXPathToNode(&ibqe, NO_BASE_NODE, INCLUDE_TRANSIENT));
      }
    }
  }

  bool Repository::rationaliseAffectedRepositoriesVector(vector<AffectedRepositoryDetails> *pvAffectedRepositories) {
    //create a distinct list of areas of an IXmlBaseDoc from a vector of AffectedNodeDetails
    //this means that if a area effect node is an ancestor of other nodes in the array then they should be removed
    vector<AffectedRepositoryDetails>::iterator iAncestorNode, iDecendentNode;
    AffectedRepositoryDetails andAncestorNode, andDecendentNode;
    Repository *pAncestorNode, *pDecendentNode;
    size_t iInitialSize = pvAffectedRepositories->size();

    for (iAncestorNode = pvAffectedRepositories->begin(); iAncestorNode != pvAffectedRepositories->end(); iAncestorNode++) {
      andAncestorNode = *iAncestorNode;
      pAncestorNode   = andAncestorNode.pRelatedRepository;

      switch (andAncestorNode.iAffectedScope) {
        //these cause sym-links, they don't change the area underneath
        //so don't replace descendants
        //saveElementOnly, syncImmediateChildren, saveTree, linkChild
        case TXml::linkChild:

        //remove node will only delete the indicated child it will not save the entire parent node pRelatedNode tree
        //so don't remove the descendants of the pRelatedNode because it is just the parent and won't save all its children
        //these are caused by moveChild and removeNode
        //removeChild cannot remove a saveTree
        case TXml::syncImmediateChildren:

        //these are additional if ancestors so keep their descendant AffectedNodeDetails
        //because they affect a different axis, e.g. a Directory name, not the Files in it
        case TXml::saveElementOnly: {
          iDecendentNode = pvAffectedRepositories->end();
          while (iDecendentNode != pvAffectedRepositories->begin()) {
            iDecendentNode--;
            if (iAncestorNode != iDecendentNode) {
              andDecendentNode = *iDecendentNode;
              pDecendentNode   = andDecendentNode.pRelatedRepository;

              //erase only identical requests
              if (andDecendentNode.iAffectedScope == andAncestorNode.iAffectedScope && pAncestorNode->is(pDecendentNode)) {
                pvAffectedRepositories->erase(iDecendentNode);
                //erased one the before iAncestorNode?
                if (iDecendentNode < iAncestorNode) iAncestorNode--;
              }
            }
          }
          break;
        }

        //this re-writes the node and area underneath so we need to remove ALL descendant AffectedNodeDetails
        //and also the current node, with name and attributes and all removeNode's
        //these are caused by moveChild, copyChild and saveTree also
        case TXml::saveTree: {
          //traverse backwards so that erasing after elements does not cause navigation issues
          iDecendentNode = pvAffectedRepositories->end();
          while (iDecendentNode != pvAffectedRepositories->begin()) {
            iDecendentNode--;
            if (iAncestorNode != iDecendentNode) {
              andDecendentNode = *iDecendentNode;
              pDecendentNode   = andDecendentNode.pRelatedRepository;

              //erase all other requests on this node, or its descendants
              if (pAncestorNode->is(pDecendentNode)) {
                //the ancestor saveTree node is equal to or above this descendant node, so remove it
                pvAffectedRepositories->erase(iDecendentNode);
                //we are erasing vector elements whilst traversing in the outside loop
                //iterators are simple pointers that increment by sizeof(whatever)
                //so comparison and movement are simple
                //erased one the before iAncestorNode?
                if (iDecendentNode < iAncestorNode) iAncestorNode--;
              }
            }
          }
        }
      }
    }

    return iInitialSize != pvAffectedRepositories->size();
  }

  const char *Repository::xsltModuleNamespace() const {return NAMESPACE_REPOSITORY;}
  const char *Repository::xsltModulePrefix()    const {return NAMESPACE_REPOSITORY_ALIAS;}

  const char *Repository::xml_filename(const char *sFullPath) const {
    //caller frees result if non-zero
    //by default take the last part of the string after /
    const char *xml_filename;
    const char *position; //not OUR MEMORY!
    size_t iLen;

    if (!sFullPath) {
      xml_filename = MM_STRDUP("unassigned");
    } else {
      iLen = strlen(sFullPath);
      if (!iLen) {
        xml_filename = MM_STRDUP("empty");
      } else {
        //ok, we have a valid non-empty sring
        //find the first character after the last seperator
        position = (char*) sFullPath + iLen - 1; //points to the last character
        if (iLen != 1 && issplitter(*position)) position--;
        while (position > sFullPath && !issplitter(*position)) position--;
        if (issplitter(*position) && position[1]) position++;

        xml_filename = xmlLibrary()->xml_element_name(position);
      }
    }

    return xml_filename;
  }

  bool Repository::isProtocol(const char *sFilePath, const char *sProtocol) {
    assert(sProtocol);

    const char *sFilePathProtocol = protocolStandard(sFilePath);
    bool bIsProtocol = false;

    if (sFilePathProtocol) {
      bIsProtocol = _STREQUAL(sFilePathProtocol, sProtocol);
      MM_FREE(sFilePathProtocol);
    }

    return bIsProtocol;
  }

  const char *Repository::protocolStandard(const char *sFullPath) {
    //caller frees non-zero result
    const char *sProtocol = 0;
    const char *sPos = sFullPath;
    size_t iLen;

    while (isalnum(*sPos)) sPos++;
    if ((iLen = sPos - sFullPath) && *sPos == ':') sProtocol = strndup(sFullPath, iLen);

    return sProtocol;
  }

  const char *Repository::extensionStandard(const char *sFullPath) {
    const char *sExtension = 0;
    if (sFullPath) {
      sExtension = strrchr(sFullPath, '.');
      if (sExtension) sExtension++;
    }
    return sExtension;
  }

  const char *Repository::nameStandard(const char *sFullPath) {
    //caller frees result if non-zero
    //by default take the last part of the string after /
    char *name;
    char *sPosition; //not OUR MEMORY!
    size_t iLen;

    if (!sFullPath) {
      name = MM_STRDUP("unassigned");
    } else {
      iLen = strlen(sFullPath);
      if (!iLen) {
        name = MM_STRDUP("empty");
      } else {
        //ok, we have a valid non-empty sring
        //find the first character after the last seperator
        sPosition = (char*) sFullPath + iLen - 1; //points to the last character
        if (iLen != 1 && issplitter(*sPosition)) sPosition--;
        while (sPosition > sFullPath && !issplitter(*sPosition)) sPosition--;
        if (issplitter(*sPosition) && sPosition[1]) sPosition++;

        //copy and make changes (so as not to violate memory)
        name = MM_STRDUP(sPosition);
        sPosition = name + strlen(name) - 1;
        if (*sPosition == '/') *sPosition = 0; //zero trailing slash
      }
    }

    return name;
  }
  const char *Repository::strsplitter(const char *sInput) {
    //returns NULL is not found
    const char *sPosition = sInput;
    while (*sPosition && !issplitter(*sPosition)) sPosition++;
    return (issplitter(*sPosition) ? sPosition : 0);
  }

  const char *Repository::strrsplitter(const char *sInput) {
    //returns NULL is not found
    const char *sPosition = sInput + strlen(sInput); //points to zero terminator
    while (sPosition >= sInput && !issplitter(*sPosition)) sPosition--;
    return (sPosition >= sInput ? sPosition : 0);
  }
  const char *Repository::canonicalise(const char *sRawPath) {
    //caller frees result
    //always reductive
    char c, *sPosition, *sNewPath = MM_STRDUP(sRawPath), *sPrevious, *sParent, sParentPath[5];

    //parent path /../
    sParentPath[0] = splitter();
    sParentPath[1] = '.';
    sParentPath[2] = '.';
    sParentPath[3] = splitter();
    sParentPath[4] = 0;

    //change all splitters to the platform standard
    sPosition = sNewPath;
    while (c = *sPosition) {
      if (issplitter(c)) *sPosition = splitter();
      sPosition++;
    }

    //parent directory references
    while ((sParent = strstr(sNewPath, sParentPath)) && sParent > sNewPath) {
      sPrevious = sParent-1;
      while (sPrevious > sNewPath && !issplitter(*sPrevious)) sPrevious--;
      memmove(sPrevious + 1, sParent + 4, strlen(sParent + 4) + 1); //includes terminator
    }
    return sNewPath;
  }

  const char *Repository::tagStart(const char *sTagStart) {
    char c;
    while ((c = *sTagStart) && c != '<') sTagStart++;
    return sTagStart;
  }
  const char *Repository::tagNameEnd(const char *sTagStart) {
    char c;
    while ((c = *sTagStart) && c != ' ' && c != '>') sTagStart++;
    return sTagStart;
  }
  const char *Repository::tagEnd(const char *sTagStart) {
    char c;
    //< in attribute values will be escaped to &lt; so we don't need to check when we are in attribute values
    while ((c = *sTagStart) && c != '>') sTagStart++;
    return sTagStart;
  }
  const char *Repository::tagEndBack(const char *sTagStart) {
    char c;
    //< in attribute values will be escaped to &lt; so we don't need to check when we are in attribute values
    while ((c = *sTagStart) && c != '>') sTagStart--;
    return sTagStart;
  }
  const char *Repository::tagAttributeStart(const char *sTagStart, const char *sAttributeName) {
    assert(sAttributeName && *sAttributeName);

    char *sFullAttributeSearch;
    const char *sAttributeStartSpace = 0;
    const char *sTagEnd              = tagEnd(sTagStart);
    size_t iLen                      = strlen(sAttributeName) + 4;
    if (sTagEnd) {
      sFullAttributeSearch = MMO_MALLOC(iLen);
      _SNPRINTF1(sFullAttributeSearch, iLen, " %s=\"", sAttributeName);
      sAttributeStartSpace = strnstr(sTagStart, sFullAttributeSearch, sTagEnd - sTagStart);
      MMO_FREE(sFullAttributeSearch);
    }

    return sAttributeStartSpace && sAttributeStartSpace < sTagEnd ? sAttributeStartSpace + 1 : 0;
  }
  const char *Repository::tagAttributeValueStart(const char *sTagStart, const char *sAttributeName) {
    const char *sTagAttributeStart = tagAttributeStart(sTagStart, sAttributeName);
    return sTagAttributeStart ? sTagAttributeStart + strlen(sAttributeName) + 2 : 0;
  }
  const char *Repository::tagAttributeValue(const char *sTagStart, const char *sAttributeName) {
    char *sAttributeValue = 0;
    const char *sTagAttributeValueEnd;
    const char *sTagAttributeValueStart = tagAttributeValueStart(sTagStart, sAttributeName);
    if (sTagAttributeValueStart) {
      if (sTagAttributeValueEnd = strchr(sTagAttributeValueStart, '"')) {
        //strndup() from string
        sAttributeValue = strndup(sTagAttributeValueStart, sTagAttributeValueEnd - sTagAttributeValueStart);
      }
    }
    return sAttributeValue;
  }

  int Repository::static_inputReadCallback(void *context, char * buffer, int len) {
    StreamingContext *pStreamingContext = (StreamingContext*) context;
    return pStreamingContext->m_pRootRepository->inputReadCallback(pStreamingContext, buffer, len);
  }
  int Repository::static_inputCloseCallback(void *context) {
    StreamingContext *pStreamingContext = (StreamingContext*) context;
    return pStreamingContext->m_pRootRepository->inputCloseCallback(pStreamingContext);
  }

  const char *Repository::xslFunction_rx(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    //caller frees result
    //rx(@repository-path, @rx-node [,@output-node])
    //  @repository-path: passed to Repository::factory()
    //  @rx-node:       transform of text stream
    //  @output-node:     optional direct output node, will return the nodes otherwise
    static const char *sFunctionSignature = "repository:rx(repository-path, rx-node [, output-node])";
    assert(pQE);
    assert(pNodes);

    XmlNodeList<const IXmlBaseNode> *pNewNodes = 0;
    const char *sFileSystemPath   = 0;
    string stContent;
    const IXmlBaseNode *pRegularXNode = 0;
    IXmlBaseNode *pOutputNode     = 0;
    Repository *pSource           = 0;

    UNWIND_EXCEPTION_BEGIN {
      switch (pXCtxt->valueCount()) {
        case 3:
          //output directly in to the DOM
          //no return
          pOutputNode = pXCtxt->popInterpretNodeFromXPathFunctionCallStack();
          if (!pOutputNode) throw XPathReturnedEmptyResultSet(this, MM_STRDUP("Output node set empty"), sFunctionSignature);
          break;
        case 2:
          //return nodes for the next function to work with
          NOT_COMPLETE("repository:rx: node return"); //Repository::xslFunction_rx() return nodes for the next function to work with
          //pOutputNode = createTempNode();
          pNewNodes   = new XmlNodeList<const IXmlBaseNode>(this, pOutputNode);
          *pNodes     = pNewNodes;
          break;
        case 1: ATTRIBUTE_FALLTHROUGH;
        case 0:
          throw XPathTooFewArguments(this, sFunctionSignature);
          break;
        default: throw XPathTooManyArguments(this, sFunctionSignature);
      }

      if (pRegularXNode = pXCtxt->popInterpretNodeFromXPathFunctionCallStack()) {
        if (sFileSystemPath = pXCtxt->popInterpretCharacterValueFromXPathFunctionCallStack()) {
          if (pSource = Repository::factory_reader(mmParent(), m_pLib, sFileSystemPath)) {
            pSource->readValidXML(stContent);

            if (pOutputNode) pOutputNode->removeAndDestroyChildren(pQE);
            RegularX::rx(pQE, pRegularXNode, stContent.c_str(), pOutputNode);

          } else throw XPathStringArgumentRequired(this, MM_STRDUP("Cannot create repository"));
        } else throw XPathStringArgumentRequired(this, MM_STRDUP("Path empty"));
      } else throw XPathReturnedEmptyResultSet(this, MM_STRDUP("RegularX node set empty"), sFunctionSignature);
    } UNWIND_EXCEPTION_END;

    //free up
    if (pRegularXNode)   delete pRegularXNode;
    if (pOutputNode)     delete pOutputNode;
    if (sFileSystemPath) MMO_FREE(sFileSystemPath);
    if (pSource)         delete pSource;
    UNWIND_CONTAINER_DESTROY_AND_DELETE_IF_EXCEPTION(pNewNodes);

    UNWIND_EXCEPTION_THROW;

    return 0;
  }

  const char *Repository::xslFunction_readRaw(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    //caller frees result
    //read-raw(@repository-path)
    //  @repository-path: passed to Repository::factory()
    static const char *sFunctionSignature = "repository:read-raw(repository-path [, fail-on-error])"; UNUSED(sFunctionSignature);
    assert(pQE);

    const char *sFileSystemPath   = 0;
    Repository *pSource           = 0;
    const char *sContent          = 0;
    bool bThrowOnError = false;

    UNWIND_EXCEPTION_BEGIN {
      switch (pXCtxt->valueCount()) {
        case 2:
          bThrowOnError = pXCtxt->popInterpretBooleanValueFromXPathFunctionCallStack();
          ATTRIBUTE_FALLTHROUGH;
        case 1:
          sFileSystemPath = pXCtxt->popInterpretCharacterValueFromXPathFunctionCallStack();
          break;
        case 0:
          throw XPathTooFewArguments(this, sFunctionSignature);
          break;
        default:
          throw XPathTooManyArguments(this, sFunctionSignature);
      }
          
      if (sFileSystemPath) {
        if (pSource = Repository::factory_reader(this, m_pLib, sFileSystemPath)) {
          try {
            sContent = pSource->readRaw();
          }
          catch (ExceptionBase &eb) {
            IFDEBUG(if (!bThrowOnError) Debug::reportObject(&eb));
            if (bThrowOnError) throw;
          }
        } else throw XPathStringArgumentRequired(this, MM_STRDUP("Cannot create repository"));
      } else throw XPathStringArgumentRequired(this, MM_STRDUP("File system path"));
    } UNWIND_EXCEPTION_END;

    //free up
    if (sFileSystemPath) MM_FREE(sFileSystemPath);
    if (pSource)         MMO_DELETE(pSource);
    
    UNWIND_EXCEPTION_THROW;
    
    return (sContent ? sContent : MM_STRDUP(""));
  }

  const char *Repository::xslFunction_readValidXML(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    //caller frees result
    //read-valid-xml(@repository-path)
    //  @repository-path: passed to Repository::factory_reader()
    static const char *sFunctionSignature = "repository:read-valid-xml(repository-path [, fail-on-error])"; UNUSED(sFunctionSignature);
    assert(pQE);

    const char *sFileSystemPath   = 0;
    Repository *pSource           = 0;
    string stContent;
    bool bThrowOnError = false;

    UNWIND_EXCEPTION_BEGIN {
      switch (pXCtxt->valueCount()) {
        case 2:
          bThrowOnError = pXCtxt->popInterpretBooleanValueFromXPathFunctionCallStack();
          ATTRIBUTE_FALLTHROUGH;
        case 1:
          sFileSystemPath = pXCtxt->popInterpretCharacterValueFromXPathFunctionCallStack();
          break;
        case 0:
          throw XPathTooFewArguments(this, sFunctionSignature);
          break;
        default: throw XPathTooManyArguments(this, sFunctionSignature);
      }

      if (sFileSystemPath) {
        if (pSource = Repository::factory_reader(mmParent(), m_pLib, sFileSystemPath)) {
          try {
            pSource->readValidXML(stContent);
          }
          catch (CannotConnectSocket &ccs) {
            Debug::reportObject(&ccs);
            if (bThrowOnError) throw;
          }
          catch (ExceptionBase &eb) {
            if (bThrowOnError) throw;
          }
        } else throw XPathStringArgumentRequired(this, MM_STRDUP("Cannot create repository"));
      } else throw XPathStringArgumentRequired(this, MM_STRDUP("File system path"));
    } UNWIND_EXCEPTION_END;

    //free up
    if (sFileSystemPath) MM_FREE(sFileSystemPath);
    if (pSource)         MMO_DELETE(pSource);

    UNWIND_EXCEPTION_THROW;

    return MM_STRDUP(stContent.c_str());
  }

  const char *Repository::xslFunction_node(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    //caller frees result
    static const char *sFunctionSignature = "repository:node()"; UNUSED(sFunctionSignature);
    *pNodes = new XmlNodeList<const IXmlBaseNode>(m_pNode->node_const()); //pQE as MM
    return 0;
  }
  
  const char *Repository::xslFunction_readToNodes(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    //caller frees result
    //read-to-nodes(@repository-path, @output-node, [, @xpath, @mask, @transform])
    //  @repository-path: passed to Repository::factory_reader()
    //  @output-node:     direct output node
    //  @xpath:           what to read. note that some systems will examine the context xpath to selectively read
    //  @transform:       optional transform the nodes before addition to @output-node
    static const char *sFunctionSignature = "repository:read-to-nodes(path, output [, xpath, mask, transform, throw-on-fail])";
    assert(pQE);

    IXmlBaseNode *pTransformNode  = 0;
    const char *sFileSystemPath   = 0;
    const char *sXPath            = 0,
               *sXPathArgMalloc   = 0,
               *sXPathMask        = 0;
    IXmlBaseNode *pOutputNode     = 0;
    Repository *pSource           = 0;
    bool bThrowOnError            = false;

    UNWIND_EXCEPTION_BEGIN {
      switch (pXCtxt->valueCount()) {
        case 6:
          bThrowOnError = pXCtxt->popInterpretBooleanValueFromXPathFunctionCallStack();
          ATTRIBUTE_FALLTHROUGH;
        case 5:
          NOT_COMPLETE("repository:read-to-nodes(..., transform)");
          pTransformNode = pXCtxt->popInterpretNodeFromXPathFunctionCallStack();
          if (!pTransformNode) throw XPathReturnedEmptyResultSet(this, MM_STRDUP("Transform node set empty"), sFunctionSignature);
          ATTRIBUTE_FALLTHROUGH;
        case 4:
          sXPathMask = pXCtxt->popInterpretCharacterValueFromXPathFunctionCallStack();
          if (!sXPathMask) throw XPathStringArgumentRequired(this, MM_STRDUP("Explicit xpath mask is empty"));
          ATTRIBUTE_FALLTHROUGH;
        case 3:
          sXPathArgMalloc = sXPath = pXCtxt->popInterpretCharacterValueFromXPathFunctionCallStack();
          if (!sXPath) throw XPathStringArgumentRequired(this, MM_STRDUP("Explicit xpath query is empty"));
          ATTRIBUTE_FALLTHROUGH;
        case 2:
          sXPath = pXCtxt->xpathCurrent(); //current part of xpath being evaluated, e.g. tables/tEmployer
          break;
        case 1: ATTRIBUTE_FALLTHROUGH;
        case 0:
          throw XPathTooFewArguments(this, sFunctionSignature);
          break;
        default:
          throw XPathTooManyArguments(this, sFunctionSignature);
      }

      if (pOutputNode = pXCtxt->popInterpretNodeFromXPathFunctionCallStack()) {
        if (sFileSystemPath = pXCtxt->popInterpretCharacterValueFromXPathFunctionCallStack()) {
          //attempt to connect to and read the repository
          //which may be a database server, or cloud thing or a file, etc.
          try {
            pSource = Repository::factory_reader(pQE, m_pLib, sFileSystemPath); //pQE as MM
          }
          catch (ExceptionBase &eb) {
            IFDEBUG(if (!bThrowOnError) Debug::reportObject(&eb));
            if (bThrowOnError) throw;
          }
          
          if (pSource) {
            pOutputNode->removeAndDestroyChildren(pQE);
            pSource->readToNodes(pQE, pOutputNode, sXPath, sXPathMask);
          } else throw XPathStringArgumentRequired(this, MM_STRDUP("Cannot create repository"));
        } else throw XPathStringArgumentRequired(this, MM_STRDUP("Path empty"));
      } else throw XPathReturnedEmptyResultSet(this, MM_STRDUP("Output node set empty"), sFunctionSignature);
    } UNWIND_EXCEPTION_END;
      
    //free up
    if (pTransformNode)  delete pTransformNode;
    if (pOutputNode)     delete pOutputNode;
    if (sFileSystemPath) MMO_FREE(sFileSystemPath);
    if (sXPathArgMalloc) MMO_FREE(sXPathArgMalloc);
    if (sXPathMask)      MMO_FREE(sXPathMask);
    if (pSource)         delete pSource;
    
    UNWIND_EXCEPTION_THROW;

    return 0;
  }

  const char *Repository::xslFunction_fileSystemPathToNodeList(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    //caller frees result
    static const char *sFunctionSignature = "repository:filesystempath-to-nodes(string path[, base node, not found node, use-indexes, throw-on-write-attempt, auto-target])";
    assert(pQE);

    XmlNodeList<const IXmlBaseNode> *pNewNodes = 0;
    const IXmlBaseNode *pNotFoundNode = 0,
                       *pBaseNode     = 0;
    const char *sFileSystemPath = 0;
    bool bUseIndexes          = true,
         bThrowOnWriteAttempt = false,
         bAutoTarget          = true;

    UNWIND_EXCEPTION_BEGIN {
      //parameters (popped in reverse order): fileSystemPath
      switch (pXCtxt->valueCount()) {
        case 6:
          bAutoTarget = pXCtxt->popInterpretBooleanValueFromXPathFunctionCallStack(); 
          ATTRIBUTE_FALLTHROUGH;
        case 5:
          bThrowOnWriteAttempt = pXCtxt->popInterpretBooleanValueFromXPathFunctionCallStack(); 
          ATTRIBUTE_FALLTHROUGH;
        case 4:
          bUseIndexes     = pXCtxt->popInterpretBooleanValueFromXPathFunctionCallStack(); 
          ATTRIBUTE_FALLTHROUGH;
        case 3:
          pNotFoundNode   = pXCtxt->popInterpretNodeFromXPathFunctionCallStack(); 
          ATTRIBUTE_FALLTHROUGH;
        case 2:
          pBaseNode       = pXCtxt->popInterpretNodeFromXPathFunctionCallStack();
          sFileSystemPath = pXCtxt->popInterpretCharacterValueFromXPathFunctionCallStack();
          break;
        case 1:
          pBaseNode       = pXCtxt->contextNode(pQE); //pQE as MM
          sFileSystemPath = pXCtxt->popInterpretCharacterValueFromXPathFunctionCallStack();
          break;
        case 0:
          //we should take the string value of the current node
          NOT_COMPLETE("repository:filesystempath-to-nodes 0 params");
          break;
        default: throw XPathTooManyArguments(this, sFunctionSignature);
      }

      //default to / document node, not object:Server
      if (!pBaseNode) pBaseNode = pQE->singularDocumentContext()->documentNode();

      if (sFileSystemPath) {
        //bAutoTarget is useful when we need to reject this call with a parameter
        //e.g. a global xsl:param calculation, Get Request API
        if (bAutoTarget) {
          try {
            if (*sFileSystemPath)
              pNewNodes = pBaseNode->fileSystemPathToNodeList(pQE, sFileSystemPath, ALLOW_ABSOLUTE, "*", NULL, bUseIndexes);
          } catch (NodeWriteableAccessDenied &eb) {
            //filesystem path contained something like database:set-attribute(...)
            if (bThrowOnWriteAttempt) throw;
          }
        }
        
        if (!pNewNodes || !pNewNodes->size()) {
          if (pNotFoundNode) {
            pNewNodes     = new XmlNodeList<const IXmlBaseNode>(pQE, pNotFoundNode); //pQE as MM
            pNotFoundNode = 0; //prevent deletion
          } else {
            pNewNodes     = new XmlNodeList<const IXmlBaseNode>(pQE); //pQE as MM
          }
        }
      } else throw XPathStringArgumentRequired(this, MM_STRDUP("File system path"));
    } UNWIND_EXCEPTION_END;

    //free up
    if (sFileSystemPath) MMO_FREE(sFileSystemPath);
    if (pBaseNode)       delete pBaseNode;
    if (pNotFoundNode)   delete pNotFoundNode;
    UNWIND_CONTAINER_DESTROY_AND_DELETE_IF_EXCEPTION(pNewNodes);

    UNWIND_EXCEPTION_THROW;

    *pNodes = pNewNodes;
    return 0;
  }

  const char *Repository::xslFunction_fileSystemPathToXPath(const IXmlQueryEnvironment *pQE, IXslXPathFunctionContext *pXCtxt, XmlNodeList<const IXmlBaseNode> **pNodes) {
    //caller frees result
    static const char *sFunctionSignature = "repository:filesystempath-to-XPath([string path, use-indexes])";
    assert(pQE);

    const char *sFileSystemPath = 0;
    const char *sXPath          = 0;
    bool bUseIndexes = true;

    UNWIND_EXCEPTION_BEGIN {
      //parameters (popped in reverse order): fileSystemPath
      switch (pXCtxt->valueCount()) {
        case 2:
          bUseIndexes     = pXCtxt->popInterpretBooleanValueFromXPathFunctionCallStack();
          ATTRIBUTE_FALLTHROUGH;
        case 1:
          sFileSystemPath = pXCtxt->popInterpretCharacterValueFromXPathFunctionCallStack();
          break;
        case 0:
          //we should take the string value of the current node
          NOT_COMPLETE("repository:filesystempath-to-XPath: 0 params");
          break;
        default: throw XPathTooManyArguments(this, sFunctionSignature);
      }
      if (sFileSystemPath) sXPath = m_pLib->fileSystemPathToXPath(pQE, sFileSystemPath, "*", NULL, bUseIndexes);
      else throw XPathStringArgumentRequired(this, MM_STRDUP("File system path"));
    } UNWIND_EXCEPTION_END;

    //free up
    if (sFileSystemPath) MMO_FREE(sFileSystemPath);

    UNWIND_EXCEPTION_THROW;

    return sXPath;
  }
}
