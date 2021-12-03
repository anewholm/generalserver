//platform agnostic file
#ifndef _RELATIONALDATABASES_H
#define _RELATIONALDATABASES_H

#include "Repository.h"

#include <stdio.h>
#include <stdlib.h>
//apt-get install postgresql
//apt-get install libpq-dev
#include <postgresql/libpq-fe.h>

using namespace std;

//http://www.postgresql.org/docs/6.5/static/libpqplusplus.htm

namespace general_server {
  class IXmlQueryEnvironment;

  class RelationalDatabase: public Repository {
  protected:
    RelationalDatabase(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlLibrary *pLib, const char *sFullPath, const Repository *pParentRepository = NULL, const char *sArg = NULL): 
      MemoryLifetimeOwner(pMemoryLifetimeOwner),
      Repository(pMemoryLifetimeOwner, pLib, sFullPath, pParentRepository)
    {}
    RelationalDatabase(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlLibrary *pLib, const DatabaseNode *pNode, const Repository *pRootRepository): 
      MemoryLifetimeOwner(pMemoryLifetimeOwner),
      Repository(pMemoryLifetimeOwner, pLib, pNode, pRootRepository)
    {}

    virtual void tables(StringMap<size_t> &mTables) const {throw CapabilityNotSupported(this, MM_STRDUP("tables list"));}

  public:
    void readToNodes(const IXmlQueryEnvironment *pQE, IXmlBaseNode *pOutput, const char *sXPathQuery, const char *sXPathMask = 0) const;

    static void compilationInformation(const IXmlQueryEnvironment *pQE ATTRIBUTE_UNUSED, IXmlBaseNode *pOutput ATTRIBUTE_UNUSED) {}
  };

  //----------------------------------------------------------------------------------------------------------
  //------------------------------------------- Repository types ---------------------------------------------
  //----------------------------------------------------------------------------------------------------------
  class Postgres: public RelationalDatabase {
    PGconn     *m_pConn;
    const char *m_sPostgresConnectionString;

  protected:
    static const Repository::repositoryType m_iRepositoryType;
    void tables(StringMap<size_t> &mTables) const;

  public:
    Postgres(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlLibrary *pLib, const char *sConnectionString, const Repository *pParent = NULL, const char *sArg = NULL);
    Postgres(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlLibrary *pLib, const DatabaseNode *pNode, const Repository *pRootRepository);
    ~Postgres();

    //Repository interface
    static const char *m_sTypeName;
    static bool gimme(const char *sFilePath);
    Repository::repositoryType type() const {return m_iRepositoryType;}
  };

  class MySQL: public RelationalDatabase {
  protected:
    static const Repository::repositoryType m_iRepositoryType;

  public:
    MySQL(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlLibrary *pLib, const char *sConnectionString, const Repository *pParent = NULL, const char *sArg = NULL);
    MySQL(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlLibrary *pLib, const DatabaseNode *pNode, const Repository *pRootRepository);

    //Repository interface
    static const char *m_sTypeName;
    static bool gimme(const char *sFilePath);
    Repository::repositoryType type() const {return m_iRepositoryType;}
  };
}

#endif
