//platform specific file (UNIX)
#ifndef _LIBXSLNODE_H
#define _LIBXSLNODE_H

//std library includes
#include <iostream>
#include <vector>
#include <map>
#include <pthread.h>
#include <stdarg.h>

using namespace std;

//platform agnostic includes
#include "Xml/XslDoc.h"
#include "Xml/XslNode.h"

//needed by LibXmlBase.cpp templates
#include "LibXslTransformContext.h" //LibXmlBase::attributeValue()
#include "LibXmlBaseNode.h"         //full definition for inheritance

//ls /usr/include/libxml2/libxml/*.h, http://xmlsoft.org/, file:///usr/local/share/doc/libxml2-2.6.22/html/tutorial/apc.html
#include <libxslt/xslt.h>
#include <libxslt/xsltInternals.h>

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>

#include <libexslt/exslt.h>

#include "Utilities/clone.c"

namespace general_server {
  class LibXslDoc;
  class Repository; //full definition in CPP

  //----------------------------------------------------------------------------------------------------------
  //----------------------------------------------------------------------------------------------------------
  //----------------------------------------------------------------------------------------------------------
  class LibXslCommandNode: public LibXmlBaseNode, public XslCommandNode {
    friend class LibXmlLibrary; //only the library can instanciate stuff

  protected:
    LibXslCommandNode(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, xmlNodePtr oNode, xmlListPtr oParentRoute, IXmlBaseDoc *pDoc, const bool bRegister = NO_REGISTRATION, const char *sLockingReason = 0, const bool pDocOurs = NOT_OURS): 
      MemoryLifetimeOwner(pMemoryLifetimeOwner),
      LibXmlBaseNode(oNode, oParentRoute, pDoc, bRegister, sLockingReason, pDocOurs) 
    {}

  public:
    static bool gimme(const xmlNodePtr oNode);

    //interface navigation: concrete classes MUST implement these to return the correct vtable pointer
    //IXslNode requires these casts
    //they are copies of the prototypes from IXmlBaseNode
    IXslCommandNode             *queryInterface(IXslCommandNode *p)             {return this;}
    const IXslCommandNode       *queryInterface(const IXslCommandNode *p) const {return this;}
    IXmlBaseNode                *queryInterface(IXmlBaseNode *p)                {return this;}
    const IXmlBaseNode          *queryInterface(const IXmlBaseNode *p) const    {return this;}
    IXmlNamespaced              *queryInterface(IXmlNamespaced *p)              {return this;}
    const IXmlNamespaced        *queryInterface(const IXmlNamespaced *p) const  {return this;}
    IXmlHasNamespaceDefinitions *queryInterface(IXmlHasNamespaceDefinitions *p) {return this;}
    const IXmlHasNamespaceDefinitions *queryInterface(const IXmlHasNamespaceDefinitions *p) const {return this;}
    const IXmlBaseNode          *queryInterfaceIXmlBaseNode() const             {return this;}

  protected:
    IXmlBaseNode *clone_wrapper_only() const;
    IXmlBaseNode *clone_with_resources() const;
  };

  //----------------------------------------------------------------------------------------------------------
  //----------------------------------------------------------------------------------------------------------
  //----------------------------------------------------------------------------------------------------------
  class LibXslNode: public LibXslCommandNode, public XslNode {
    friend class LibXmlLibrary; //only the library can instanciate stuff
  protected:
    //implements the IXmlBaseNode and thus IXmlBaseNode<IXmlDoc> interfaces by
    //  using the LibXmlBaseNode<IXmlDoc> virtual base class

    //no concept of child LibXmlNodes exists here
    //transforms and XPath should be used to query
    static StringMap<IXslDoc*> ms_mCachedStylesheets;
    static vector<IXslDoc*>    ms_vRedundantStylesheets;

    LibXslNode(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, xmlNodePtr oNode, xmlListPtr oParentRoute, IXmlBaseDoc *pDoc, const bool bRegister = NO_REGISTRATION, const char *sLockingReason = 0, const bool bDocOurs = NOT_OURS): 
      MemoryLifetimeOwner(pMemoryLifetimeOwner),
      LibXslCommandNode(pMemoryLifetimeOwner, oNode, oParentRoute, pDoc, bRegister, sLockingReason, bDocOurs) 
    {}
    
    static bool clearMyCachedStylesheetsRecursive(const IXmlQueryEnvironment *pQE, const IXslStylesheetNode *pXSLStylesheetNode, const IXmlBaseNode *pNode);

  public:
    static bool gimme(const xmlNodePtr oNode);

    //node -> stylesheet cacheing
    void clearRedundantStylesheetNodeCacheItems(const IXmlQueryEnvironment *pQE) const;
    IXslDoc *checkStylesheetNodeCache(const IXmlQueryEnvironment *pQE) const;
    void cacheStylesheetNode(const IXmlQueryEnvironment *pQE, IXslDoc *pSS) const;
    static void freeParsedTemplates();
    static bool clearMyCachedStylesheets(const IXmlQueryEnvironment *pQE, const IXmlBaseNode *pNode);
    static void serialise(const IXmlQueryEnvironment *pQE, IXmlBaseNode *pOutputNode);

    //interface navigation: concrete classes MUST implement these to return the correct vtable pointer
    //IXslNode requires these casts
    //they are copies of the prototypes from IXmlBaseNode
    IXmlBaseNode                *queryInterface(IXmlBaseNode *p)                {return this;}
    const IXmlBaseNode          *queryInterface(const IXmlBaseNode *p) const    {return this;}
    const IXslNode              *queryInterface(const IXslNode *p) const        {return this;}
    IXmlNamespaced              *queryInterface(IXmlNamespaced *p)              {return this;}
    const IXmlNamespaced        *queryInterface(const IXmlNamespaced *p) const  {return this;}
    IXmlHasNamespaceDefinitions *queryInterface(IXmlHasNamespaceDefinitions *p) {return this;}
    const IXmlHasNamespaceDefinitions *queryInterface(const IXmlHasNamespaceDefinitions *p) const {return this;}
    const IXmlBaseNode          *queryInterfaceIXmlBaseNode() const             {return this;}

  protected:
    IXmlBaseNode *clone_wrapper_only() const;
    IXmlBaseNode *clone_with_resources() const;
  };


  //----------------------------------------------------------------------------------------------------------
  //----------------------------------------------------------------------------------------------------------
  //----------------------------------------------------------------------------------------------------------
  class LibXslTemplateNode: public LibXslNode, public XslTemplateNode {
    friend class LibXmlLibrary;

  protected:
    //implements the IXmlBaseNode and thus IXmlBaseNode<IXmlDoc> interfaces by
    //  using the LibXmlBaseNode<IXmlDoc> virtual base class
    LibXslTemplateNode(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, xmlNodePtr oNode, xmlListPtr oParentRoute, IXmlBaseDoc *pDoc, const bool bRegister = NO_REGISTRATION, const char *sLockingReason = 0, const bool bDocOurs = NOT_OURS): 
      MemoryLifetimeOwner(pMemoryLifetimeOwner),
      LibXslNode(pMemoryLifetimeOwner, oNode, oParentRoute, pDoc, bRegister, sLockingReason, bDocOurs) 
    {}

  public:
    static bool gimme(const xmlNodePtr oNode);

    const char *toString() const;

    //interface navigation: concrete classes MUST implement these to return the correct vtable pointer
    //IXslNode requires these casts
    //they are copies of the prototypes from IXmlBaseNode
    IXmlBaseNode                *queryInterface(IXmlBaseNode *p)                   {return this;}
    const IXmlBaseNode          *queryInterface(const IXmlBaseNode *p) const       {return this;}
    const IXslTemplateNode      *queryInterface(const IXslTemplateNode *p) const   {return this;}
    const IXmlBaseNode          *queryInterfaceIXmlBaseNode() const                {return this;}
    //IXmlNamespaced          *queryInterface(IXmlNamespaced *p)               {return this;}
    //const IXmlNamespaced    *queryInterface(const IXmlNamespaced *p) const   {return this;}
  };


  //----------------------------------------------------------------------------------------------------------
  //----------------------------------------------------------------------------------------------------------
  //----------------------------------------------------------------------------------------------------------
  class LibXslStylesheetNode: public LibXslNode, public XslStylesheetNode {
    friend class LibXmlLibrary;

  protected:
    //implements the IXmlBaseNode and thus IXmlBaseNode<IXmlDoc> interfaces by
    //  using the LibXmlBaseNode<IXmlDoc> virtual base class
    LibXslStylesheetNode(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, xmlNodePtr oNode, xmlListPtr oParentRoute, IXmlBaseDoc *pDoc, const bool bRegister = NO_REGISTRATION, const char *sLockingReason = 0, const bool bDocOurs = NOT_OURS): 
      MemoryLifetimeOwner(pMemoryLifetimeOwner),
      LibXslNode(pMemoryLifetimeOwner, oNode, oParentRoute, pDoc, bRegister, sLockingReason, bDocOurs) 
    {}

  public:
    static bool gimme(const xmlNodePtr oNode);

    //interface navigation: concrete classes MUST implement these to return the correct vtable pointer
    //IXslNode requires these casts
    //they are copies of the prototypes from IXmlBaseNode
    IXmlBaseNode                *queryInterface(IXmlBaseNode *p)                   {return this;}
    const IXmlBaseNode          *queryInterface(const IXmlBaseNode *p) const       {return this;}
    const IXslStylesheetNode    *queryInterface(const IXslStylesheetNode *p) const {return this;}
    IXmlBaseNode                *queryInterfaceIXmlBaseNode()                      {return this;}
    const IXmlBaseNode          *queryInterfaceIXmlBaseNode() const                {return this;}
    //IXmlNamespaced          *queryInterface(IXmlNamespaced *p)               {return this;}
    //const IXmlNamespaced    *queryInterface(const IXmlNamespaced *p) const   {return this;}
  };
}

#endif
