//platform agnostic file
#ifndef _DATABASENODESERVEROBJECT_H
#define _DATABASENODESERVEROBJECT_H

#include "define.h"
#include "Xml/XmlAdminQueryEnvironment.h"

#include <vector>
using namespace std;

namespace general_server {
  class DatabaseNode;
  class Database;

  //---------------------------------------------------------------------------------------
  //---------------------------------------------------------------------------------------
  //---------------------------------------------------------------------------------------
  class DatabaseNodeServerObject: virtual public MemoryLifetimeOwner {
  protected:
    //assigned in constructor only
    //CAN BE ZERO in the case of Repository::factory_reader() loading the config xml doc string
    DatabaseNode *m_pNode;
    XmlAdminQueryEnvironment m_ibqe_nodeControl; //used for non-trigger admin control...

    //constructors: we need a node for this sort of construction
    DatabaseNodeServerObject(const IMemoryLifetimeOwner *pMemoryLifetimeOwner);
    DatabaseNodeServerObject(DatabaseNode *pNode, const bool bRegisterNode = true);
    DatabaseNodeServerObject(const DatabaseNode *pNode, const bool bRegisterNode = true); //object returned by factory should also be const
    ~DatabaseNodeServerObject();

  public:
    Database *db();
    const Database *db() const;
    const IXmlBaseDoc *document() const;
    const IXmlLibrary *xmlLibrary() const;
    DatabaseNode *dbNode() const;
    void hardlinkError(const IXmlQueryEnvironment *pQE, IXmlBaseNode *pErrorNode);

    static void compilationInformation(const IXmlQueryEnvironment *pQE ATTRIBUTE_UNUSED, IXmlBaseNode *pOutput ATTRIBUTE_UNUSED) {}
  };

  //---------------------------------------------------------------------------------------
  //---------------------------------------------------------------------------------------
  //---------------------------------------------------------------------------------------
  class GeneralServerDatabaseNodeServerObject: public DatabaseNodeServerObject {
  protected:
    GeneralServerDatabaseNodeServerObject(const IMemoryLifetimeOwner *pMemoryLifetimeOwner);
    GeneralServerDatabaseNodeServerObject(DatabaseNode *pNode, const bool bRegisterNode = true);
    GeneralServerDatabaseNodeServerObject(const DatabaseNode *pNode, const bool bRegisterNode = true); //object returned by factory should also be const
    ~GeneralServerDatabaseNodeServerObject();

    XmlAdminQueryEnvironment *m_pIBQE_serverStartup;

  public:
    void relinquishDatabaseNode();
    void setDatabaseNode(DatabaseNode *pNode); //used by Repository for top-level commits

    static void compilationInformation(const IXmlQueryEnvironment *pQE ATTRIBUTE_UNUSED, IXmlBaseNode *pOutput ATTRIBUTE_UNUSED) {}
  };
}

#endif
