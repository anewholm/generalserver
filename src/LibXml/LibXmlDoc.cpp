//platform specific file (UNIX)
#include "LibXmlDoc.h"

using namespace std;

namespace general_server {
  LibXmlDoc::LibXmlDoc(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlLibrary *pLib, const char *sAlias, xmlDocPtr oDoc, const bool bOurs): 
    MemoryLifetimeOwner(pMemoryLifetimeOwner),
    LibXmlBaseDoc(pMemoryLifetimeOwner, pLib, sAlias, oDoc, bOurs)
  {}
}
