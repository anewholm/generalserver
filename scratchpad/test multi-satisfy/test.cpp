#include <iostream>
#include "test.h"

using namespace std;

IXmlBaseNode *LibXmlBaseNode::getSingleNode(const char *xpath) {
  return createNodeByNamespace(xpath);
}
vector<IXmlBaseNode*> *LibXmlBaseNode::getMultipleNodes(const char *xpath) {
  vector<IXmlBaseNode*> *v = new vector<IXmlBaseNode*>();
  v->push_back(new LibXmlNode());
  v->push_back(new LibXmlNode());
  v->push_back(new LibXmlNode());
  v->push_back(new LibXmlNode());
  return v;
}

IXmlBaseNode *XmlBaseNode::createNodeByNamespace(const char *sNamespace) {
  IXmlBaseNode *r = 0;
  
  //namespaces supported internally
  //xsl, xschema and xml provided by LibXml plugin
  //if (sNamespace[0] == 'x') r = factory()->factory_xslstylesheetnode(); //new LibXslStylesheetNode();
  switch(sNamespace[1]) {
    case 's': r = new LibXslStylesheetNode(); break; //xsl
    default:  r = new LibXmlNode();
  }

  return r;
}

  
//----------------------------------------------------------- main
int main() {
  IXmlBaseNode *b = new LibXslStylesheetNode();
  
  IXmlBaseDoc *dd = new LibXmlDoc(4);
  cout << "dd:" << dd->name() << "\n";
  
  //a direct cast is possible here because there is no virtual inheritance...
  //we are using a secondary interface rather than a concrete derived object
  //IXslStylesheetNode does not inherit from anything
  IXmlBaseNode *xx = b->getSingleNode("xsl:test");
  cout << "xx:" << xx->name() << "\n";
  
  IXslStylesheetNode *x45 = dynamic_cast<IXslStylesheetNode*>(xx);
  cout << "x45:" << x45->transform() << "\n";

  IXslStylesheetNode *x3 = xx->queryInterface((IXslStylesheetNode*) 0);
  cout << "x3:" << x3->transform() << "\n";

  IXmlBaseNode *xml = b->getSingleNode("xml:test");
  cout << "xml 1:" << xml->name() << "\n";

  IXmlNode *x46 = dynamic_cast<IXmlNode*>(xml);
  cout << "x46:" << x46->name() << "\n";

  IXmlNode *xml2 = b->getSingleNodeType<IXmlNode>("xml:test");
  cout << "xml2:" << xml2->name() << "\n";

  IXslStylesheetNode *xx2 = b->getSingleNodeType<IXslStylesheetNode>("xsl:test");
  cout << "xx2:" << xx2->name() << "\n";

  //IXslStylesheetNode *x = (IXslStylesheetNode*) xx; //NOTE: doesnt work!
  //cout << x->transform() << "\n";
  
  //-------------------------------- Server Objects
  cout << "-------------------------\n";
  Server s(xml);
  cout << "s:" << s.name() << "\n";
  s.runServerObject();

  Repository r(&s, xx);
  cout << "r:" << r.name() << "\n";
}