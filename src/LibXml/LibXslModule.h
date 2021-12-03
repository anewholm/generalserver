//platform specific file (UNIX)
#ifndef _LIBXSLMODULE_H
#define _LIBXSLMODULE_H

//std library includes
#include <iostream>
using namespace std;

//platform agnostic includes
#include "Xml/XslModule.h" //direct inheritance

//ls /usr/include/libxml2/libxml/*.h, http://xmlsoft.org/, file:///usr/local/share/doc/libxml2-2.6.22/html/tutorial/apc.html
#include <libxslt/xslt.h>
#include <libxslt/xsltInternals.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libexslt/exslt.h>

namespace general_server {
  class LibXslModule: implements_interface_static XslModule {
    //using implements_interface_static because we want to pass member pointers
    //for registration of member()s of an IXslModule compatible class
    //as oppossed to actual IXslModule::member()s
    //private LibXML specific
  public: //TODO: remove public
    static void xslFunctionCall(xmlXPathParserContextPtr ctxt, int nargs);
    static void xslCall(xsltTransformContextPtr ctxt, xmlNodePtr node, xmlNodePtr inst, xsltElemPreCompPtr comp);
  protected:
    //LibXML platform specific
    HRESULT xmlLibraryRegisterXslFunction(const char *xsltModuleNamespace, const char *sFunctionName) const;
    HRESULT xmlLibraryRegisterXslCommand( const char *xsltModuleNamespace, const char *sCommandName)  const; //virtual required = 0
  };
}
#endif
