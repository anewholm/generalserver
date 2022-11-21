//platform agnostic file
#ifndef _XMLNAMESPACE_H
#define _XMLNAMESPACE_H

#include "define.h"
#include "IXml/IXmlNamespace.h"
#include "Utilities/StringMap.h"

#include <iostream>
#include <string.h>
#include <map>
using namespace std;

namespace general_server {
  //see explanation in interface decleration

  //--------------------------------------- XmlNamespaced ---------------------------------------
  //--------------------------------------- XmlNamespaced ---------------------------------------
  //--------------------------------------- XmlNamespaced ---------------------------------------
  class XmlNamespaced: implements_interface IXmlNamespaced {
  };

  //--------------------------------------- XmlHasNamespaceDefinitions ---------------------------------------
  //--------------------------------------- XmlHasNamespaceDefinitions ---------------------------------------
  //--------------------------------------- XmlHasNamespaceDefinitions ---------------------------------------
  class XmlHasNamespaceDefinitions: implements_interface IXmlHasNamespaceDefinitions {
    static StringMap<const char*> m_mStandardNamespaceDefinitions;

  public:
    //these are used by createRootChildElement() from TXml to setup new documents with namespaces on root element
    //good also for incoming string XML with new / used prefixed namespaces on
    const StringMap<const char*> *standardNamespaceDefinitions();
    static void freeStandardNamespaceDefinitions(); //called by the ~XmlLibrary()
    void addAllStandardNamespaceDefinitions();
    void addNamespaceDefinitions(const StringMap<const char*> *mDefinitions);
    
    IFDEBUG(const char *x() const;)
  };
}

#endif
