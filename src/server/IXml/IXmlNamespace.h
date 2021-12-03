//platform agnostic file
#ifndef _IXMLNAMESPACE_H
#define _IXMLNAMESPACE_H

#include "define.h"
#include "Utilities/StringMap.h"

#define RECONCILE_DEFAULT  true
#define REQUIRE_NAMESPACES true

namespace general_server {
  interface_class IXmlBaseNode;

  /* XML namespaces:
   *   parent->child relationship for @xmlns:* http://www.w3.org/TR/xpath/#namespace-nodes
   *
   * these are now ALWAYS connected to a node
   * although LibXml has a seperate object, the interface (C) is always node specific
   *   for the namespace, and the definitions
   * so they are inherited and part of a node rather than being seperate entities
   * so IXmlNamespace DOES NOT appear by itself, only as part of a host node
   * this is a WRAPPER, not a library: we do not store a copy of prefix and URL in our own objects
   * although we might reference the node->ns we do not own it, and the xmlFreeNode() will destroy it
   *   however, we do store the reconciled one, which we will free.
   *
   * IXmlNamespaceDefinition also MUST be attached to a node, so has node, prefix and href
   *
   */

  //these interfaces standardise our access to underlying XML stores
  interface_class IXmlNamespaced {
    //class for querying the underlying library for namespace info
    //does not hold any info itself but instead queries the library each time
    virtual const char *constructName(const char *sPrefix, const char *sLocalName) const = 0;

  protected:
    //this recommendation should be used to lazy resolve default (no) prefix
    //ALL NODES in a namespace will have a valid cur->ns, but it might not have a prefix associated
    //by searching self-or-ancestor:: for a similar namespace definition that has a prefix to use
    //So: if the NS for a node is valid but with blank prefix, search up the tree for a namespace definition WITH a prefix
    //this is useful for more exact xpath node definition, gs:node instead of node
    //TXml uses this with uniqueXPathToNode() -> fullyQualifiedName()
    virtual void checkNamespaceValidity() const {} //base namespace belongs to correct document?

    virtual bool hasNamespace() const = 0; //it is ILLEGAL for this to return false! always gs

  public:
    //query
    virtual const char *namespaceHREF(const bool bDuplicate = true) const = 0; //always on node
    virtual bool isNamespace(const char *sHREF) const = 0; //always on node, must be a string compare
    //prefix might be blank, and thus the default, so reconcileNamespace() can look for a valid prefix up the tree
    virtual const char *namespacePrefix(const bool bReconcileDefault = RECONCILE_DEFAULT, const bool bDuplicate = true) const = 0; //expensive one-off no cache

    //write
    virtual void setNamespace(const char *sHREF) = 0;
  };

  //this class is mostly split out to highlight the difference between own namespace and potential associated definitions
  interface_class IXmlHasNamespaceDefinitions: implements_interface IReportable {
    //the underlying library handles ANY mulition concepts and stores the actual namespaces
    //LibXML documentation does not require that the return xmlNsPtr is freed from any of these function calls
    //this library has no direct access so this is a functional interface, not a concrete one
    //addition functions are transient and thus cannot keep or affect the passed object
  public:
    //definitions control
    virtual const StringMap<const char*> *namespaceDefinitions() const = 0;
    virtual void addNamespaceDefinitions(const StringMap<const char*> *mDefinitions) = 0;
    virtual void addNamespaceDefinition(const char *sPrefix, const char *sHREF) = 0;
    virtual void addDefaultNamespaceDefinition(              const char *sHREF) = 0;
    virtual bool setNamespaceRoot(IXmlBaseNode *pFromNode = NULL) = 0;

    //standard definitions (static lazy cached)
    virtual const StringMap<const char*> *standardNamespaceDefinitions() = 0;
    //static void freeStandardNamespaceDefinitions() = 0; //global map, not specific to a node
    virtual void addAllStandardNamespaceDefinitions() = 0;
    
    virtual const char *toString() const = 0;
    IFDEBUG(virtual const char *x() const = 0);

    //interface navigation: concrete classes MUST implement these to return the correct vtable pointer
    //these pure virtual methods simply REQUIRE their concrete classes to implement these implicit casts
    //they are copies of the prototypes from IXmlBaseNode
    //this is ONLY to be used for query interfaces that are supported by IXml*,
    //so dont use it for queryInterface(LibXmlBaseNode*)
    virtual IXmlBaseNode       *queryInterface(IXmlBaseNode *p)             = 0;
    virtual const IXmlBaseNode *queryInterface(const IXmlBaseNode *p) const = 0;
    virtual const IXmlBaseNode *queryInterfaceIXmlBaseNode() const          = 0;
  };
}

#endif
