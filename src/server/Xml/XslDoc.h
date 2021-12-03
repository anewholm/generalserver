//platform agnostic file
//interface that all XML libraries and XSL libraries should implement
//classes need to instanciate the concrete classes XslModule
#ifndef _XSLDOC_H
#define _XSLDOC_H

#include "IXml/IXmlBaseDoc.h"               //full definition for inheritance
#include "IXml/IXslDoc.h"                   //full definition for inheritance
#include "IXml/IXslTransformContext.h"      //full definition for inheritance
#include "IXml/IXslXPathFunctionContext.h"  //full definition for inheritance

namespace general_server {
  //---------------------------------------------------------------------------------------
  //---------------------------------------------------------------------------------------
  //---------------------------------------------------------------------------------------
  class XslTransformContext: virtual public MemoryLifetimeOwner, implements_interface_static IXslTransformContext {
  protected:
    const IXmlQueryEnvironment *m_pOwnerQE;
    XslTransformContext(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlQueryEnvironment *pOwnerQE);
    virtual ~XslTransformContext();
  public:
    void setQueryEnvironment(const IXmlQueryEnvironment *pOwnerQE);
    const IXmlQueryEnvironment *ownerQE() const;

    //error reporting
    const char *currentSourceNodeXPath()  const;
    const char *currentXSLTemplateXPath() const;
    const char *currentXSLCommandXPath()  const;
    
    void handleError(const char *sErrorMessage);
  };

  //---------------------------------------------------------------------------------------
  //---------------------------------------------------------------------------------------
  //---------------------------------------------------------------------------------------
  class XslXPathFunctionContext: virtual public MemoryLifetimeOwner, implements_interface_static IXslXPathFunctionContext {
  protected:
    const IXmlQueryEnvironment *m_pOwnerQE;
    XslXPathFunctionContext(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlQueryEnvironment *pOwnerQE);
    virtual ~XslXPathFunctionContext();
  public:
    void setQueryEnvironment(const IXmlQueryEnvironment *pOwnerQE);
    const IXmlQueryEnvironment *ownerQE() const;
    IXmlBaseNode *popInterpretNodeFromXPathFunctionCallStack(const bool bThrowOnNot1 = false);

    //error reporting
    const char *currentSourceNodeXPath()  const;
    const char *currentXSLTemplateXPath() const;
    const char *currentXSLCommandXPath()  const;
    
    void handleError(const char *sErrorMessage);

    const char *toString() const;
  };

  //---------------------------------------------------------------------------------------
  //---------------------------------------------------------------------------------------
  //---------------------------------------------------------------------------------------
  class XslDoc: implements_interface IXmlBaseDoc, implements_interface IXslDoc {
  protected:
    //cannot call virtual functions in constructors
    //thus *_construct(...) functions are called from derived classes to carry out super-work
    //with identical parameters
    IXslTransformContext *m_pCtxt;
    const IXmlBaseNode *m_pSourceNode;

    //(sub) constructors called only by inheriting classes during construction
    XslDoc(IXslTransformContext *pCtxt);
    XslDoc();
    //~XslDoc() {} //m_pCtxt is owned externally, explicitly delete it

  public:
    size_t translateIncludes(XmlNodeList<IXmlBaseNode> *pvIncludes, IXmlQueryEnvironment *pMasterDocQE, const IXmlBaseNode *pIncludeDirectoryNode);
    XmlNodeList<IXmlBaseNode> *includes();
    const char *name() const;
    const IXmlBaseNode *sourceNode() const;
    const IXmlBaseDoc  *sourceDoc()  const;
    const IXmlBaseNode *sourceNode(const IXmlBaseNode *pSourceNode);
    virtual const IXmlBaseDoc *queryInterface(const IXmlBaseDoc *p) const = 0;
  };
}

#endif
