//platform agnostic file
#include "RelationalDatabases.h"
#include "Server.h"
#include "define.h"
#include "IXml/IXmlBaseNode.h"
#include "Debug.h"
#include <string>
#include "Utilities/strtools.h"

namespace general_server {
  //----------------------------------------------------------------------------------------------------------
  //------------------------------------------- RelationalDatabase ---------------------------------------------
  //----------------------------------------------------------------------------------------------------------
  void RelationalDatabase::readToNodes(const IXmlQueryEnvironment *pQE, IXmlBaseNode *pOutput, const char *sXPathQuery, const char *sXPathMask) const {
    //optional sXPathQuery examples:
    //  /tables/tEmployer/*[@id = 4]                        => all rows from tEmployer table with an id of 4
    //  /views/vFrozenEmployer/*[@date = now()]             => all rows from vFrozenEmployer for today
    //  /stored-procedures/uspEmp[@eg1 = 4 and @ert = 'eg'] => run stored procedure uspEmp(4, 'example')
    //tabled query data is used to create nodes
    //string responses, from sprocs, are XML parsed in to nodes
    const char *sStepEnd = strchrs(sXPathQuery, "[/");
    if (!sStepEnd) sStepEnd = sXPathQuery + strlen(sXPathQuery);
    const char *sStepFirst = strndup(sXPathQuery, sStepEnd - sXPathQuery - 1);

    //------------------------------------------- children, e.g. tree request
    if        (strcmp(sStepFirst, "*")) {
      //groups
      delete pOutput->getOrCreateChildElement(pQE, "users");
      delete pOutput->getOrCreateChildElement(pQE, "tables");
      delete pOutput->getOrCreateChildElement(pQE, "queries");
      delete pOutput->getOrCreateChildElement(pQE, "stored-procedures");

    //------------------------------------------- specific table or view query
    } else if (strcmp(sStepFirst, "views")
           ||  strcmp(sStepFirst, "tables")
    ) {
      //list tables
      IXmlBaseNode *pTableNode;
      IXmlBaseNode *pTablesNode;
      StringMap<size_t> mTables;
      StringMap<size_t>::const_iterator iTable;

      //navigate to the tables collection
      pTablesNode = pOutput->getOrCreateChildElement(pQE, "tables");

      //populate it
      tables(mTables);
      for (iTable = mTables.begin(); iTable != mTables.end(); iTable++) {
        pTableNode = pTablesNode->createChildElement(pQE, iTable->first);
        pTableNode->setAttribute(pQE, "count", iTable->second, NAMESPACE_GS);
      }
      //TODO: postgres query:  move to next step and dynamically specifically populate

      //free up
      if (pTablesNode) delete pTablesNode;

    //------------------------------------------- custom query
    //queries/myQuery[@sql='select * from tEmployer']
    } else if (strcmp(sStepFirst, "queries")) {
      NOT_COMPLETE(""); //RelationalDatabase::readToNodes() queries

    //------------------------------------------- stored procedure
    //stored-procedures/uspExample[@arg1 = 2 and @arg2 = 'test']
    } else if (strcmp(sStepFirst, "stored-procedures")) {
      NOT_COMPLETE(""); //RelationalDatabase::readToNodes() stored-procedures
    }
  }

  //----------------------------------------------------------------------------------------------------------
  //------------------------------------------- Postgres ---------------------------------------------
  //----------------------------------------------------------------------------------------------------------
  const Repository::repositoryType Postgres::m_iRepositoryType = tPostgres;
  const char *Postgres::m_sTypeName = "Postgres";

  Postgres::Postgres(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlLibrary *pLib, const char *sConnectionString, const Repository *pParent, const char *sArg): 
    MemoryLifetimeOwner(pMemoryLifetimeOwner),
    RelationalDatabase(pMemoryLifetimeOwner, pLib, sConnectionString, pParent), 
    m_sPostgresConnectionString(0),
    m_pConn(0)
  {
    const char *sErrorMessage = 0;
    
    //e.g. sConnectionString "postgres:dbname = test"
    //http://www.postgresql.org/docs/9.1/static/libpq-example.html
    m_sPostgresConnectionString = strchr(sConnectionString, ':');
    if (m_sPostgresConnectionString) {
      m_sPostgresConnectionString++;
      m_pConn = PQconnectdb(m_sPostgresConnectionString);
      if (PQstatus(m_pConn) == CONNECTION_OK) Debug::report("successfully connected to Postgres Database [%s]", m_sPostgresConnectionString);
      else {
        sErrorMessage = MM_STRDUP(PQerrorMessage(m_pConn));
        PQfinish(m_pConn); m_pConn = 0;
        throw RelationalDatabaseConnectionFailed(this, MM_STRDUP(m_sTypeName), sErrorMessage);
      }
    }
  }
  
  Postgres::Postgres(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlLibrary *pLib, const DatabaseNode *pNode, const Repository *pRootRepository): 
    MemoryLifetimeOwner(pMemoryLifetimeOwner),
    RelationalDatabase(pMemoryLifetimeOwner, pLib, pNode, pRootRepository), 
    m_sPostgresConnectionString(0),
    m_pConn(0)
  {}

  Postgres::~Postgres() {
    if (m_pConn) PQfinish(m_pConn);
  }
  
  bool Postgres::gimme(const char *sFilePath) {return isProtocol(sFilePath, "postgres");}

  void Postgres::tables(StringMap<size_t> &mTables) const {
    mTables.insert("users", 7);
  }

  //----------------------------------------------------------------------------------------------------------
  //------------------------------------------- MySQL ---------------------------------------------
  //----------------------------------------------------------------------------------------------------------
  const Repository::repositoryType MySQL::m_iRepositoryType = tMySQL;
  const char *MySQL::m_sTypeName = "MySQL";

  MySQL::MySQL(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlLibrary *pLib, const char *sConnectionString, const Repository *pParent, const char *sArg): 
    MemoryLifetimeOwner(pMemoryLifetimeOwner),
    RelationalDatabase(pMemoryLifetimeOwner, pLib, sConnectionString, pParent) 
  {}
  
  MySQL::MySQL(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlLibrary *pLib, const DatabaseNode *pNode, const Repository *pRootRepository): 
    MemoryLifetimeOwner(pMemoryLifetimeOwner),
    RelationalDatabase(pMemoryLifetimeOwner, pLib, pNode, pRootRepository) 
  {}

  bool MySQL::gimme(const char *sFilePath) {return isProtocol(sFilePath, "mysql");}
}
