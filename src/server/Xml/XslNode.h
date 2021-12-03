//platform agnostic file
//interface that all XML libraries and XSL libraries should implement
//classes need to instanciate the concrete classes XslModule
#ifndef _XSLNODE_H
#define _XSLNODE_H

#include "define.h"

#include "IXml/IXmlBaseNode.h" //complete include for inheritance
#include "IXml/IXslNode.h"     //complete include for inheritance

namespace general_server {
  //derived classes must also implement IXmlBaseDoc which does require constructors and parameters
  //e.g.
  //  class LibXmlDoc: public XmlDoc, public XmlBaseDoc {
  //     X(int i): XmlBaseDoc(i), XmlDoc() {this->XmlBaseDoc::document();}
  //  }
  //XmlDoc implements IXmlBaseDoc so all members() can be accessed (after instanciation)

  //---------------------------------------------------------------------------------------
  //---------------------------------------------------------------------------------------
  //---------------------------------------------------------------------------------------
  class XslCommandNode: implements_interface IXmlBaseNode, implements_interface IXslCommandNode {
  protected:
    XslCommandNode() {} //only created by derived class constructors
  public:
    //note that IXmlBaseNode holds an IXmlBaseDoc, which may or may not be an XslDoc
    static bool isCommandNamespace(const char *sHREF);
    const IXslDoc *stylesheet() const;
  };


  //---------------------------------------------------------------------------------------
  //---------------------------------------------------------------------------------------
  //---------------------------------------------------------------------------------------
  class XslNode: implements_interface IXmlBaseNode, implements_interface IXslNode {
  protected:
    XslNode() {} //only created by derived class constructors
  };


  //---------------------------------------------------------------------------------------
  //---------------------------------------------------------------------------------------
  //---------------------------------------------------------------------------------------
  class XslTemplateNode: implements_interface IXmlBaseNode, implements_interface IXslTemplateNode {
  protected:
    XslTemplateNode() {} //only created by derived class constructors
  };


  //---------------------------------------------------------------------------------------
  //---------------------------------------------------------------------------------------
  //---------------------------------------------------------------------------------------
  class XslStylesheetNode: implements_interface IXmlBaseNode, implements_interface IXslStylesheetNode {
  protected:
    XslStylesheetNode() {} //only created by derived class constructors

  public:
    IXslDoc *preprocessToStylesheet(const IXmlQueryEnvironment *pQE, const IXmlBaseNode *pIncludeDirectory = 0) const;
  };
}

#endif
