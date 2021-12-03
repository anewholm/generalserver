//platform specific file (UNIX)
#ifndef _LIBXSLDOC_H
#define _LIBXSLDOC_H

//std library includes
#include <iostream>
#include <vector>
#include <map>
#include <pthread.h>
#include <stdarg.h>
using namespace std;

//needed by LibXmlBase.cpp templates
#include "LibXslTransformContext.h" //LibXmlBase::attributeValue()
#include "LibXslXPathFunctionContext.h"
#include "LibXmlBaseDoc.h"          //full definition for inheritance
#include "Xml/XslDoc.h"                 //full definition for inheritance

//ls /usr/include/libxml2/libxml/*.h, http://xmlsoft.org/, file:///usr/local/share/doc/libxml2-2.6.22/html/tutorial/apc.html
#include <libxslt/xslt.h>
#include <libxslt/xsltInternals.h>

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>

#include <libexslt/exslt.h>

#include "Utilities/clone.c"

namespace general_server {
  class LibXslDoc;
  class LibXslNode;
  class Repository; //full definition in CPP

  class LibXslDoc: public LibXmlBaseDoc, public XslDoc {
    friend class LibXmlLibrary; //only the library can instanciate stuff

    //only classes in this file should have access to the platform specific members
    xsltStylesheetPtr m_oXSLDoc2;

  protected:
    LibXslDoc(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlLibrary *pLib, const char *sAlias): MemoryLifetimeOwner(pMemoryLifetimeOwner), LibXmlBaseDoc(pMemoryLifetimeOwner, pLib, sAlias) {}
    //platform specific
    //LibXml object access
    LibXslDoc(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlLibrary *pLib, const char *sAlias, xmlDocPtr oDoc, const bool bOurs = OURS);
    LibXslDoc(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlLibrary *pLib, const char *sAlias, xmlDocPtr oDoc, IXslTransformContext *pCTxt, const bool bOurs = OURS);

  public:
    ~LibXslDoc();

    IXmlBaseDoc *clone_wrapper_only() const {return base_object_clone<LibXslDoc>(this);}
    IXmlBaseDoc *clone_with_resources() const {return new LibXslDoc(*this);}
    const xsltStylesheetPtr libXslStylesheetPtr() const {return m_oXSLDoc2;}

    void setParent(const IXmlBaseNode *pParent); //link the doc back to the world
    void clearParent();                          //unlink the doc from the world
    IXmlBaseNode *parentNode();       //get doc parent
    unsigned int incrementLockCount();           //native / application level lock counting
    unsigned int decrementLockCount();           //native / application level lock counting
    bool hasLocks();
    void parse(const CompilationEnvironment *pCE); //any parsing requirements

    //interface navigation: concrete classes MUST implement these to return the correct vtable pointer
    //IXslDoc requires this cast
    //they are copies of the prototypes from IXmlBaseNode
    IXslDoc              *queryInterface(IXslDoc *p)                 {return this;}
    const IXslDoc        *queryInterface(const IXslDoc *p) const     {return this;}
    IXmlBaseDoc          *queryInterface(IXmlBaseDoc *p)             {return this;}
    const IXmlBaseDoc    *queryInterface(const IXmlBaseDoc *p) const {return this;}
  };
}

#endif
