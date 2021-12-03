//platform agnostic file
#ifndef _DATABASECLASS_H
#define _DATABASECLASS_H

#include "DatabaseNodeServerObject.h" //direct inheritance
#include "IReportable.h"
#include "define.h"

#include "Utilities/StringMap.h"
#include "Utilities/StringMultiMap.h"
#include "Utilities/StringVector.h"
#include <vector>    //template return value
using namespace std;

#define USE_DB_ADMIN_QE 0

namespace general_server {
  class IXmlBaseNode;
  class Database;
  class DatabaseNode;
  class DatabaseAdminQueryEnvironment;
  interface_class IXslXPathFunctionContext;

  class DatabaseClass: public GeneralServerDatabaseNodeServerObject {
    friend class Database; //xslFunctions()

    //global static classes lookups
    static StringMap<DatabaseClass*> m_mClassesByName;

    //individual this Class members
    const char            *m_sName;
    const char            *m_sNamespacePrefix;
    StringVector           m_vElements;
    const char            *m_sPredicate;
    
    //calculated class members
    vector<DatabaseClass*> m_vBaseClasses;
    vector<DatabaseClass*> m_vDerivedClasses;
    const char            *m_sNodeTestXPath;
    const char            *m_sTemplateMatchClause;
    const char            *m_sNamespaceHREF;

    //instanciation
    DatabaseClass(const DatabaseAdminQueryEnvironment *pIBQE_dbAdministrator, DatabaseNode *pClassNode);
    void initDatabaseClassDependentProperties(const IMemoryLifetimeOwner *pMemoryLifetimeOwner);
    ~DatabaseClass();

  public:
    //class relationships workers
    vector<DatabaseClass*> *allBasesUnique(const bool bIncludeAllElementsClasses = true) const;
    void allBasesUnique_recursive(vector<DatabaseClass*> *pvAllBases, const bool bIncludeAllElementsClasses = true) const;
    void allBasesUnique_recursive(XmlNodeList<const IXmlBaseNode> *pvAllBases, const bool bIncludeAllElementsClasses = true) const;
    vector<DatabaseClass*> *allDerivedUnique(const bool bIncludeAllElementsClasses = true) const;
    void allDerivedUnique_recursive(vector<DatabaseClass*> *pvAllBases, const bool bIncludeAllElementsClasses = true) const;
    void allDerivedUnique_recursive(XmlNodeList<const IXmlBaseNode> *pvAllBases, const bool bIncludeAllElementsClasses = true) const;

    //instanciation
    static void loadClasses(DatabaseAdminQueryEnvironment *pIBQE_dbAdministrator, DatabaseNode *pFromNode);
    static void clearClasses(); //called by ~Database

    //utilities
    static char* classIDXPathFromName(const char *sClassName, size_t iNameLen = 0);

    //lookups
    //these return only the most derived classes, e.g. not XSL, when XSLStylesheet is also returned
    //DP; memento
    static vector<DatabaseClass*> *classesFromElement(const DatabaseNode *pNode, const bool bIncludeBases = false, const bool bIncludeAllElementsClasses = true);
    static vector<DatabaseClass*> *classesFromElement(const IXmlBaseNode *pNode, const bool bIncludeBases = false, const bool bIncludeAllElementsClasses = true);
    static vector<DatabaseClass*> *classesFromElement(const char *sLocalName, const char *sPrefix = NULL, const bool bIncludeBases = false, const bool bIncludeAllElementsClasses = true);
    static vector<DatabaseClass*> *classesFromLocalName(const char *sLocalName, const bool bIncludeBases = false, const bool bIncludeAllElementsClasses = true);
    static DatabaseClass *classFromClassNode(const DatabaseNode *pDBNode);
    static DatabaseClass *classFromClassNode(const IXmlBaseNode *pNode);
    static DatabaseClass *classFromName(const char *sClassName);
    bool insertInto(vector<DatabaseClass*> *pvDerivedClasses, const bool bIncludeBases = false, const bool bIncludeAllElementsClasses = true);
    const char *derivedTemplateMatchClause(const char *sMatch) const;
    static vector<DatabaseClass*> *all();
    static XmlNodeList<const IXmlBaseNode> *allClassNodes(const IMemoryLifetimeOwner *pMemoryLifetimeOwner);
    bool gimme(const IXmlBaseNode *pNode) const;
    bool gimme(const char *sLocalName) const;
    bool gimme(const char *sLocalName, const char *sPrefix) const;
    bool isBaseOf(const DatabaseClass *pSuper) const;
    bool isDerivedFrom(const DatabaseClass *pBase) const;

    IXmlBaseNode *xschema(const IXmlQueryEnvironment *pQE, const char *sXSchemaName = 0, const bool bStandalone = true, const bool bCompiled = false) const;
    IXmlBaseNode *xschemaNodeMask(const IXmlQueryEnvironment *pQE, const IXmlBaseNode *pInputNode, const char *sInterfaceMode = 0) const;
    IXmlBaseNode *xstylesheet(const IXmlQueryEnvironment *pQE, const bool bStandalone = false, const bool bClientSideOnly = true) const;

    static DatabaseClass *popInterpretDatabaseClassFromXPathFunctionCallStack(IXslXPathFunctionContext *pCtxt, const char *sFunctionSignature = 0);

    //accessors
    const char                   *name()           const {return m_sName;}
    const StringVector    *elements()       const {return &m_vElements;}
    
    bool                    hasElements()    const {return m_vElements.size();}
    bool                    isAllNamespaceElements()  const {return m_vElements.size() == 1 && _STREQUAL(m_vElements[0], "*") && !m_sPredicate;}
    bool                    isAllElements()  const {return m_sNamespacePrefix == NULL && m_vElements.size() == 1 && _STREQUAL(m_vElements[0], "*") && !m_sPredicate;}
    bool                    needsFullEval()  const {return m_sPredicate != NULL;}
    const vector<DatabaseClass*> *baseClasses()    const {return &m_vBaseClasses;}
    const vector<DatabaseClass*> *derivedClasses() const {return &m_vDerivedClasses;}

    //settings info
    const char *toString() const;
    static const char *toStringAll();
    static void serialise(const IXmlQueryEnvironment *pQE, IXmlBaseNode *pOutputNode);
  };
}

#endif
