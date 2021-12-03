#include <iostream>
#include "test.h"

using namespace std;

T_XMLLIBRARY_TEMPLATE_FUNC TIXmlBaseNode *LibXmlBaseDoc<T_XMLLIBRARY_TEMPLATE_PARAMS>::newNode() {
  return new TConcreteBaseNode(this);
}
IXmlNode *LibXmlDoc::newNode() {
  return new LibXmlNode(this);
}

int main() {
  IXmlDoc *d  = new LibXmlDoc();
  IXslDoc *x  = new LibXslDoc();
  IXmlNode *n = new LibXmlNode(d);
  IXslNode *s = new LibXslNode(x);
  IXmlNode *n2 = n->newNode();
  cout << n->whoami() << "\n";
  cout << n2->whoami() << "\n";
  cout << s->document()->whoami() << "\n";
}