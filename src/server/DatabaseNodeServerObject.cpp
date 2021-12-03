//platform agnostic file
#include "DatabaseNodeServerObject.h"
#include "Database.h"
#include "DatabaseNode.h"

namespace general_server {
  //---------------------------------------------------------------------------------------
  //---------------------------------------------------------------------------------------
  //---------------------------------------------------------------------------------------
  DatabaseNodeServerObject::DatabaseNodeServerObject(DatabaseNode *pNode, const bool bRegisterNode):
    MemoryLifetimeOwner(pNode),
    m_pNode(pNode),
    m_ibqe_nodeControl(this, pNode->db()->document_const())
  {
    assert(m_pNode);
    if (bRegisterNode && m_pNode->isNodeElement()) {
      m_pNode->registerNode();
      m_pNode->setTransientAttribute(&m_ibqe_nodeControl, "cpp-component", true, NAMESPACE_OBJECT);
    }
  }

  DatabaseNodeServerObject::DatabaseNodeServerObject(const DatabaseNode *pNode, const bool bRegisterNode):
    MemoryLifetimeOwner(pNode),
    m_pNode((DatabaseNode*) pNode),
    m_ibqe_nodeControl(this, pNode->db()->document_const())
  {
    //object returned by factory should also be const
    assert(m_pNode);
    if (bRegisterNode && m_pNode->isNodeElement()) {
      m_pNode->registerNode();
      m_pNode->setTransientAttribute(&m_ibqe_nodeControl, "cpp-component", true, NAMESPACE_OBJECT);
    }
  }

  DatabaseNodeServerObject::DatabaseNodeServerObject(const IMemoryLifetimeOwner *pMemoryLifetimeOwner):
    MemoryLifetimeOwner(pMemoryLifetimeOwner),
    m_pNode(0),
    m_ibqe_nodeControl(this, (IXmlBaseDoc*) 1)
  {
    //TODO: get rid of this DatabaseNodeServerObject()!!!!
    //only used by Repository now...
  }

  DatabaseNodeServerObject::~DatabaseNodeServerObject() {
    if (m_pNode) delete m_pNode;
  }
  Database       *DatabaseNodeServerObject::db()       {return m_pNode ? m_pNode->db() : 0;}
  const Database *DatabaseNodeServerObject::db() const {return m_pNode ? m_pNode->db() : 0;}
  DatabaseNode *DatabaseNodeServerObject::dbNode() const {return m_pNode;}
  const IXmlBaseDoc *DatabaseNodeServerObject::document() const {return m_pNode->document_const();}
  const IXmlLibrary *DatabaseNodeServerObject::xmlLibrary() const {return db()->xmlLibrary();}

  void DatabaseNodeServerObject::hardlinkError(const IXmlQueryEnvironment *pQE, IXmlBaseNode *pErrorNode) {
    assert(pQE);
    IXmlBaseNode *pErrorRepositoryNode = 0,
                 *pErrorNodeLink       = 0;

    if (pErrorRepositoryNode = m_pNode->getOrCreateTransientChildElement(&m_ibqe_nodeControl, "errors", NULL, NAMESPACE_REPOSITORY)) {
      pErrorNodeLink = pErrorRepositoryNode->hardlinkChild(&m_ibqe_nodeControl, pErrorNode);
      if (!pErrorNodeLink) Debug::report("Failed to add error to repository:errors on object");
    } else Debug::report("Failed to add repository:errors to object");

    //touch the parent here because the error collections are transient
    //trandsient changes to not trigger listeners
    m_pNode->touch(pQE, "transient error nodes added to the DatabaseNodeServerObject");

    //free up
    if (pErrorNodeLink)       delete pErrorNodeLink;
    if (pErrorRepositoryNode) delete pErrorRepositoryNode;
  }


  //---------------------------------------------------------------------------------------
  //---------------------------------------------------------------------------------------
  //---------------------------------------------------------------------------------------
  GeneralServerDatabaseNodeServerObject::GeneralServerDatabaseNodeServerObject(DatabaseNode *pNode, const bool bRegisterNode):
    MemoryLifetimeOwner(pNode),
    DatabaseNodeServerObject(pNode, bRegisterNode),
    m_pIBQE_serverStartup(new XmlAdminQueryEnvironment(this, pNode->document_const()))
  {}

  GeneralServerDatabaseNodeServerObject::GeneralServerDatabaseNodeServerObject(const DatabaseNode *pNode, const bool bRegisterNode):
    MemoryLifetimeOwner(pNode),
    DatabaseNodeServerObject(pNode, bRegisterNode),
    m_pIBQE_serverStartup(new XmlAdminQueryEnvironment(this, pNode->document_const()))
  {
    //object returned by factory should also be const
  }

  GeneralServerDatabaseNodeServerObject::GeneralServerDatabaseNodeServerObject(const IMemoryLifetimeOwner *pMemoryLifetimeOwner): 
    MemoryLifetimeOwner(pMemoryLifetimeOwner),
    DatabaseNodeServerObject(pMemoryLifetimeOwner), 
    m_pIBQE_serverStartup(0) 
  {}

  GeneralServerDatabaseNodeServerObject::~GeneralServerDatabaseNodeServerObject() {
    if (m_pIBQE_serverStartup) delete m_pIBQE_serverStartup;
  }

  void GeneralServerDatabaseNodeServerObject::setDatabaseNode(DatabaseNode *pNode) {
    //defered setup by the Repository object
    assert(!m_pNode);
    m_pNode = pNode;
    m_pNode->registerNode();
    m_pIBQE_serverStartup = new XmlAdminQueryEnvironment(this, pNode->document_const());
  }
  void GeneralServerDatabaseNodeServerObject::relinquishDatabaseNode() {
    assert(m_pNode);
    m_pNode = 0;
  }
}

