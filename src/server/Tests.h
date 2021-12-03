//platform agnostic file
#ifndef _TESTS_H
#define _TESTS_H

//platform generic includes
#include "define.h"
//platform specific project headers
#include "LibXslModule.h"
#include "MemoryLifetimeOwner.h" //direct inheritance
  
namespace general_server {
  class Database;
  class DatabaseNode;
  class Tests;
  interface_class IXmlBaseNode;
  
  //TODO: restructure in progress
  class Test: virtual public MemoryLifetimeOwner {
    friend class Tests;
    
    const char *m_sGroup;
    const char *m_sName;
    const char *m_sType;
    const char *m_sValue;
    const char *m_sFlags;
    const bool  m_bDebugOnly;
    const char *m_sDisableWarning;
    const bool  m_bRunTest;

    const int   m_iResultCount;
    const char *m_sResultXML;
    const char *m_sResultThrow;
    
    Test(const MemoryLifetimeOwner *pMemoryLifetimeOwner, IXmlQueryEnvironment *m_pQE, const IXmlBaseNode *pSpec);
    ~Test();

  public:
    const char *toString() const;
  };

  class Tests: virtual public MemoryLifetimeOwner, public XslModuleManager {
    Database *m_pTestDatabase;
    IXmlQueryEnvironment *m_pQE;
    const bool m_bDEBUG_MODE;
    
  public:
    Tests(Database *pTestDatabase, const char *sTestGroupsExclude = NULL, const char *sTestGroupsInclude = NULL);

    bool runXPathTest(       DatabaseNode *pTestSpec, const char *sXPath, const size_t iResultCount = 1, const char *sResultXML = 0, const char *sResultThrow = 0) const;
    bool runXPathStringTest( DatabaseNode *pTestSpec, const char *sXPath, const size_t iResultCount = 1, const char *sResultXML = 0, const char *sResultThrow = 0) const;
    bool runTransformTest(   DatabaseNode *pTestSpec, const char *sXPath, const size_t iResultCount = 1, const char *sResultXML = 0, const char *sResultThrow = 0) const;
    
    const char *toString() const;
    const char *xslModuleManagerName() const;
    const IXmlLibrary *xmlLibrary() const;

    static void compilationInformation(const IXmlQueryEnvironment *pQE ATTRIBUTE_UNUSED, IXmlBaseNode *pOutput ATTRIBUTE_UNUSED) {}
  };
}

#endif

