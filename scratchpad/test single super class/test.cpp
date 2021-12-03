#include <iostream>
#include "test.h"

using namespace std;

IXmlDoc *LibXslNode::transform() {return new LibXmlDoc();}
template<class TIXmlBaseNode> IXmlBaseNode* LibXmlBaseNode<TIXmlBaseNode>::getSingleNode(const char *xpath) {
  IXmlBaseNode *n = 0;
  switch (xpath[1]) {
    case 's': {n = new LibXslNode(this->document()); break;}
    case 'm':
    default:  {n = new LibXmlNode(this->document()); break;}
  }
  return n;
}
IXslDoc *IXmlLibrary::factory_stylesheet() {return new IXslDoc();}
IXmlBaseNode *IXslDoc::createElement() {return new LibXslNode(this);}
IXmlBaseNode *IXmlDoc::createElement() {return new LibXmlNode(this);}


//----------------------------------------------------------- main
int main() {
  IXmlDoc *d  = new LibXmlDoc();
  IXslDoc *x  = new LibXslDoc();
  IXmlNode *n = new LibXmlNode(d);
  IXslNode *s = new LibXslNode(x);
  cout << n->whoami() << "\n";
  cout << s->document()->whoami() << "\n";

  //alignment with up-classing
  IXslNode *x1 = (IXslNode*) s->getSingleNode("xsl:test");
  IXmlNode *x2 = (IXmlNode*) s->getSingleNode("xml:test");
  cout << x1->whoami() << "\n";
  cout << x2->whoami() << "\n";
  IXmlDoc *d2 = x1->transform();
  cout << d2->whoami() << "\n";

  //introspection
  if (x && x->isXslDoc()) cout << "it is\n";
  
  //factories
  IXslDoc *y = x->factory()->factory_stylesheet();
  cout << y->whoami() << "\n";
  IXslNode *t = (IXslNode*) y->createElement();
  cout << t->whoami() << "\n";
}