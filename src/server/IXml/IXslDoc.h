//platform agnostic file
//interface that all XML libraries and XSL libraries should implement
//classes need to instanciate the concrete classes XslModule
#ifndef _IXSLDOC_H
#define _IXSLDOC_H

#include "define.h"
#include "MemoryLifetimeOwner.h" //direct inheritance
#include "IXmlBaseDoc.h"         //direct inheritance

namespace general_server {
  interface_class IXmlLibrary;
  interface_class IXslDoc;
  interface_class IXslCommandNode;
  interface_class IXslModuleManager;
  interface_class IXmlGrammarContext;
  class Server;
  class CompilationEnvironment;


  //-------------------------------------------------------------------------------------------
  //-------------------------------------------------------------------------------------------
  //-------------------------------------------------------------------------------------------
  interface_class IXslDoc: implements_interface IMemoryLifetimeOwner, implements_interface IXmlBaseDoc {
  public:
    virtual ~IXslDoc() {} //virtual public destructors allow proper destruction of derived object

    virtual size_t translateIncludes(XmlNodeList<IXmlBaseNode> *pvIncludes, IXmlQueryEnvironment *pMasterDocQE, const IXmlBaseNode *pIncludeDirectoryNode) = 0;
    virtual XmlNodeList<IXmlBaseNode> *includes()       = 0;
    virtual void setParent(const IXmlBaseNode *pParent) = 0; //link the doc back to the world
    virtual void clearParent() = 0;                          //unlink the doc from the world
    virtual IXmlBaseNode *parentNode() = 0;                  //get doc parent
    virtual const IXmlBaseNode *sourceNode() const = 0;      //where the XSL came from, potentially another doc
    virtual const IXmlBaseDoc  *sourceDoc()  const = 0;      //where the XSL came from
    virtual const IXmlBaseNode *sourceNode(const IXmlBaseNode *pSourceNode) = 0;
    virtual void parse(const CompilationEnvironment *pCE) = 0; //any parsing requirements
    virtual unsigned int incrementLockCount() = 0;           //native / application level lock counting
    virtual unsigned int decrementLockCount() = 0;
    virtual bool hasLocks() = 0;
    virtual const char *name() const = 0;                    //useful name of the stylesheet (usually the @repository:name)

    //interface navigation: concrete classes MUST implement these to return the correct vtable pointer
    //these pure virtual methods simply REQUIRE their concrete classes to implement these implicit casts
    //they are copies of the prototypes from IXmlBaseNode
    //this is ONLY to be used for query interfaces that are supported by IXml*,
    //so dont use it for queryInterface(LibXmlBaseNode*)
    virtual IXslDoc           *queryInterface(IXslDoc *p) = 0;
    virtual IXmlBaseDoc       *queryInterface(IXmlBaseDoc *p) = 0;
    virtual const IXmlBaseDoc *queryInterface(const IXmlBaseDoc *p) const = 0;
  };
}

#endif
