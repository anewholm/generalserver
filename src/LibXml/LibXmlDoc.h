//platform specific file (UNIX)
#ifndef _LIBXMLDOC_H
#define _LIBXMLDOC_H

//std library includes
#include <iostream>
#include <vector>
#include <stdarg.h>
using namespace std;

#include "define.h"
#include "LibXmlBaseDoc.h" //full for inheritance
#include "Xml/XmlDoc.h"        //full for inheritance

#include "Utilities/clone.c"

namespace general_server {
  class LibXmlNode;
  class Repository;
  interface_class IXmlBaseDoc;

  class LibXmlDoc: public LibXmlBaseDoc, public XmlDoc {
    friend class LibXmlLibrary;  //only the library can instanciate stuff
    friend class LibXmlBaseNode; //PERFORMANCE: instanciation exception: security performance creates on local stack

  protected:
    LibXmlDoc(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlLibrary *pLib, const char *sAlias): MemoryLifetimeOwner(pMemoryLifetimeOwner), LibXmlBaseDoc(pMemoryLifetimeOwner, pLib, sAlias) {}

    //platform specific
    //LibXml object access
    LibXmlDoc(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlLibrary *pLib, const char *sAlias, xmlDocPtr oDoc, const bool bOurs = OURS);

  public:
    IXmlBaseDoc *clone_wrapper_only() const {return base_object_clone<LibXmlDoc>(this);}
    IXmlBaseDoc *clone_with_resources() const {return new LibXmlDoc(*this);}

    //interface navigation: concrete classes MUST implement these to return the correct vtable pointer
    //IXslNode requires these casts
    //they are copies of the prototypes from IXmlBaseNode
    IXmlBaseDoc          *queryInterface(IXmlBaseDoc *p)             {return this;}
  };
}

#endif
