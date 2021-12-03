//platform independent
#include "Xml/XmlNamespace.h"

#include "MemoryLifetimeOwner.h"

namespace general_server {
  //--------------------------------------- XmlHasNamespaceDefinitions ---------------------------------------
  //--------------------------------------- XmlHasNamespaceDefinitions ---------------------------------------
  //--------------------------------------- XmlHasNamespaceDefinitions ---------------------------------------
  StringMap<const char*> XmlHasNamespaceDefinitions::m_mStandardNamespaceDefinitions;

  const StringMap<const char*> *XmlHasNamespaceDefinitions::standardNamespaceDefinitions() {
    const char *sXmlns = NAMESPACE_ALL, *sFirstQuote, *sSecondQuote;
    char *sPrefix, *sHref;
    size_t iLen;

    if (!m_mStandardNamespaceDefinitions.size()) {
      while (sXmlns = strstr(sXmlns, "xmlns:")) {
        if (
          (sFirstQuote = strchr(sXmlns, '"'))
          && (sSecondQuote = strchr(sFirstQuote + 1, '"'))
        ) {
          //prefix
          iLen = (sFirstQuote - 1) - (sXmlns + 6);
          sPrefix = MMO_MALLOC(iLen + 1);
          strncpy(sPrefix, sXmlns + 6, iLen);
          sPrefix[iLen] = 0;

          //href
          iLen = sSecondQuote - (sFirstQuote + 1);
          sHref = MMO_MALLOC(iLen + 1);
          strncpy(sHref, sFirstQuote + 1, iLen);
          sHref[iLen] = 0;

          //register
          m_mStandardNamespaceDefinitions.insert(sPrefix, sHref);
        }

        sXmlns++; //search for next
      }
    }

    return &m_mStandardNamespaceDefinitions;
  }
  void XmlHasNamespaceDefinitions::addAllStandardNamespaceDefinitions() {
    addNamespaceDefinitions(standardNamespaceDefinitions());
  }

  void XmlHasNamespaceDefinitions::freeStandardNamespaceDefinitions() {
    StringMap<const char*>::iterator iNamespace;
    for (iNamespace = m_mStandardNamespaceDefinitions.begin(); iNamespace != m_mStandardNamespaceDefinitions.end(); iNamespace++) {
      MMO_FREE(iNamespace->first);
      MMO_FREE(iNamespace->second);
    }
  }
  
  void XmlHasNamespaceDefinitions::addNamespaceDefinitions(const StringMap<const char*> *mDefinitions) {
    StringMap<const char*>::const_iterator iNamespace;
    for (iNamespace = mDefinitions->begin(); iNamespace != mDefinitions->end(); iNamespace++) {
      addNamespaceDefinition(iNamespace->first, iNamespace->second);
    }
  }

  IFDEBUG(const char *XmlHasNamespaceDefinitions::x() const {return toString();})
}
