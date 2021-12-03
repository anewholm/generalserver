//platform agnostic file
//interface that all XML libraries and XSL libraries should implement
//classes need to instanciate the concrete classes XslModule
#ifndef _XMLNODE_H
#define _XMLNODE_H

#include "define.h"

#include "IXml/IXmlBaseNode.h" //complete include for inheritance
#include "IXml/IXmlNode.h"     //complete include for inheritance

using namespace std;

namespace general_server {
  class XmlNode: implements_interface IXmlBaseNode, implements_interface IXmlNode {
    //this concrete helper class that is only created by derived class constructors
    //has no constructors because it has no members to do anything with
    //derived classes must also implement IXmlBaseDoc which does require constructors and parameters
    //e.g.
    //  class LibXmlDoc: public XmlDoc, public XmlBaseDoc {
    //     X(int i): XmlBaseDoc(i), XmlDoc() {this->XmlBaseDoc::document();}
    //  }
    //XmlDoc implements IXmlBaseDoc so all members() can be accessed (after instanciation)
  };
}

#endif
