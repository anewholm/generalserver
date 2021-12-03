//platform agnostic file
//interface that all XML libraries and XSL libraries should implement
//classes need to instanciate the concrete classes XslModule
#ifndef _IXSLNODE_H
#define _IXSLNODE_H

#include "define.h"
#include "IXmlBaseNode.h" //iErrorPolicy enum

namespace general_server {
  //aheads
  interface_class IXslDoc;

  //---------------------------------------------------------------------------------------
  //---------------------------------------------------------------------------------------
  //---------------------------------------------------------------------------------------
  interface_class IXslCommandNode {
    //external command nodes
    //NOTE: <database:*> <server:*> <request:*> etc.
  public:
    virtual ~IXslCommandNode() {} //virtual public destructors allow proper destruction of derived object

    //suggested for returning if the namespace is correct for this command node
    //static virtual bool isCommandNamespace(const char *sHREF) const = 0;

    //the stylesheet might not be a stylesheet
    //use a dynamic cast and check the results in the implementation
    virtual const IXslDoc *stylesheet() const = 0;
    
    virtual IXmlBaseNode *clone_with_resources() const = 0;

    //interface navigation: concrete classes MUST implement these to return the correct vtable pointer
    //these pure virtual methods simply REQUIRE their concrete classes to implement these implicit casts
    //they are copies of the prototypes from IXmlBaseNode
    //this is ONLY to be used for query interfaces that are supported by IXml*,
    //so dont use it for queryInterface(LibXmlBaseNode*)
    virtual IXmlBaseNode       *queryInterface(IXmlBaseNode *p)             = 0;
    virtual const IXmlBaseNode *queryInterface(const IXmlBaseNode *p) const = 0;
    virtual const IXmlBaseNode *queryInterfaceIXmlBaseNode() const          = 0;
  };

  //---------------------------------------------------------------------------------------
  //---------------------------------------------------------------------------------------
  //---------------------------------------------------------------------------------------
  interface_class IXslNode: implements_interface IXslCommandNode {
    //<xsl:*>
  public:
    virtual void clearRedundantStylesheetNodeCacheItems(const IXmlQueryEnvironment *pQE) const = 0;
    virtual IXslDoc *checkStylesheetNodeCache(const IXmlQueryEnvironment *pQE) const = 0;
    virtual void cacheStylesheetNode(const IXmlQueryEnvironment *pQE, IXslDoc *pSS) const = 0;

    //interface navigation: concrete classes MUST implement these to return the correct vtable pointer
    //these pure virtual methods simply REQUIRE their concrete classes to implement these implicit casts
    //they are copies of the prototypes from IXmlBaseNode
    //this is ONLY to be used for query interfaces that are supported by IXml*,
    //so dont use it for queryInterface(LibXmlBaseNode*)
    virtual IXmlBaseNode       *queryInterface(IXmlBaseNode *p)             = 0;
    virtual const IXmlBaseNode *queryInterface(const IXmlBaseNode *p) const = 0;
    virtual const IXmlBaseNode *queryInterfaceIXmlBaseNode() const          = 0;
  };

  //---------------------------------------------------------------------------------------
  //---------------------------------------------------------------------------------------
  //---------------------------------------------------------------------------------------
  interface_class IXslTemplateNode: implements_interface IXslNode, implements_interface IReportable {
    //<xsl:template> only
  public:
    //interface navigation: concrete classes MUST implement these to return the correct vtable pointer
    //these pure virtual methods simply REQUIRE their concrete classes to implement these implicit casts
    //they are copies of the prototypes from IXmlBaseNode
    //this is ONLY to be used for query interfaces that are supported by IXml*,
    //so dont use it for queryInterface(LibXmlBaseNode*)
    virtual IXmlBaseNode       *queryInterface(IXmlBaseNode *p)             = 0;
    virtual const IXmlBaseNode *queryInterface(const IXmlBaseNode *p) const = 0;
    virtual const IXmlBaseNode *queryInterfaceIXmlBaseNode() const          = 0;
  };

  //---------------------------------------------------------------------------------------
  //---------------------------------------------------------------------------------------
  //---------------------------------------------------------------------------------------
  interface_class IXslStylesheetNode: implements_interface IXslNode {
    //<xsl:stylesheet> only
  public:
    virtual IXslDoc *preprocessToStylesheet(const IXmlQueryEnvironment *pQE, const IXmlBaseNode *pIncludeDirectory = 0) const = 0;

    //interface navigation: concrete classes MUST implement these to return the correct vtable pointer
    //these pure virtual methods simply REQUIRE their concrete classes to implement these implicit casts
    //they are copies of the prototypes from IXmlBaseNode
    //this is ONLY to be used for query interfaces that are supported by IXml*,
    //so dont use it for queryInterface(LibXmlBaseNode*)
    virtual IXmlBaseNode       *queryInterface(IXmlBaseNode *p)             = 0;
    virtual const IXmlBaseNode *queryInterface(const IXmlBaseNode *p) const = 0;
    virtual const IXmlBaseNode *queryInterfaceIXmlBaseNode() const          = 0;
  };
}

#endif
