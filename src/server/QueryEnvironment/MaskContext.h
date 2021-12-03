//platform agnostic file
#ifndef _MASKCONTEXT_H
#define _MASKCONTEXT_H

#include "define.h"
#include "IXml/IXmlMaskContext.h" //direct inheritance
#include "IXml/IXmlArea.h"        //direct inheritance

#include <vector> //direct inheritance
using namespace std;

namespace general_server {
  interface_class IXmlBaseNode;
  interface_class IXmlLibrary;
  interface_class IXslModuleManager;
  interface_class IXmlQueryEnvironment;

  //---------------------------------------------------------------------
  //---------------------------------------------------------------------
  //---------------------------------------------------------------------
  class MaskContext: public IXmlMaskContext, public vector<const IXmlArea*> {
    bool m_bEnabled;
    const IXmlQueryEnvironment *m_pOwnerQE;

  public:
    MaskContext();
    MaskContext(const MaskContext& sec);
    void setQueryEnvironment(const IXmlQueryEnvironment *pOwnerQE);
    const IXmlQueryEnvironment *queryEnvironment() const {return m_pOwnerQE;}
    IXmlMaskContext *inherit() const;
    IXmlMaskContext *clone_with_resources() const;

    //info
    const char *type()     const {return "MaskContext";}
    const char *toString() const {return MM_STRDUP(type());}
    size_t      size()     const {return vector<const IXmlArea*>::size();}
    const IXmlArea *back() const {return vector<const IXmlArea*>::back();}

    //control
    bool enabled() const {return m_bEnabled;}
    bool enable()        {bool bOldEnabled = m_bEnabled; m_bEnabled = true;  return bOldEnabled;}
    bool disable()       {bool bOldEnabled = m_bEnabled; m_bEnabled = false; return bOldEnabled;}

    //mask queue
    void push_back(const IXmlArea *pArea) {return vector<const IXmlArea*>::push_back(pArea);}
    void pop_back() {return vector<const IXmlArea*>::pop_back();}
    IXmlQueryEnvironment::accessResult applyCurrentAreaMask(const IXmlBaseNode *pNode) const;
  };
}

#endif
