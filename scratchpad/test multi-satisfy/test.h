/* --------------------------------- external library COM style
 * http://en.wikipedia.org/wiki/Software_design_pattern#Domain-specific_patterns
 * this is an area of extreme expansion and importance as the project goes forward
 * we need a plugin Xml library (LibXML2 / MSXML6) for the XML, XSL and XSchema COM style plugin system
 *   to allow different styles of XML engine to *co-exist*:
 *   ) LibXML2
 *   ) Connector PostGRES -> IXMLNode
 *   ) Disk page loaded XML documents for large repositories
 *   ) Platform independence
 *   ) Additional External server capability
 * we need a general IXML* interface for those XML libraries to declare their interface
 *   which can be used generically for all:
 *   ) XML tasks
 *   ) XSL tasks
 *   ) XSchema tasks
 * we want partial helpful implementation XML*
 * we also want to talk in interfaces, not concrete classes, 
 *   deferring concrete class creation to the plugins abstract factory
 * we dont want templates
 * For the Server:
 *   we want to return a Node type based on namespace AND local-name() from outside of the LibXML2 / libXml* library
 *     because we are making our own non-library specific node types, e.g. Repository, Server, MessageInterpretation
 * We NEED to dynamic_cast<> for polymorphic situations!!!
 * 
 * 
 * should a class be tied to its XML schema representation? 
 *   class XmlNodeServerObject (for xsl:stylesheet)
 *   class GeneralServerXmlNodeServerObject: public XmlNodeServerObject
 *   e.g. Server* holds a private IXMLNode that describes services
 * an XSLStylehseet[Node] that understands its vector<XSLTemplate[Node]> members
 *   and can process / loads / edit them
 * static Server::processMainConfig(IXmlDoc *doc) -> Service(IXmlNode *m_node) -> carries out remaining tasks internally
 * Repository(IXmlNode *m_node) rather than Repository: public LibXmlNode
 * XSchema(IXmlNode *m_node)->parse(IXmlNode *struct)
 * Service->save()
 * Repository->save() / reload()
 * vector<Service*> = Server->getMultipleNodesAsXmlNodeServerObjects('./gs:service') ?
 * general_server namespace that should create these extra objects:
 *   gs:server         -> Server (load) (vector<Service*> or singletons (Services*) getMultipleNodes('//gs:service'))
 *   gs:repository     -> Repository (save, load) (lastUpdated)
 *   gs:service        -> Service (start, stop)
 *   gs:db             -> DB (start, stop, save, load, rollback) (defaultNamespace)
 *   gs:conversation/* -> MessageInterpretation
 *   gs:request        -> Request ?
 * 
 * IXml* interface sub-node types:
 *   xsl:stylesheet    -> XslStylesheetNode (transform, transformationContext)
 *   xsl:template, xsl:if, xsl:when, xsl:value-of -> XslNode (stylehseet)
 *   xschema:*         -> XSchemaNode
 * Server->getMultipleNodesAsGeneralServerNamespaceObjects('//repository')->saveAll(); ?
 * so XML schema, actions and knowledge is encapsualted in each "Server Object"
 * which *can* generate itself from a node, or create a node from itself, or sync
 *   e.g. new Service(port 81, tcp, vector<MIs>)->appendToNode(IXMLNode *server)
 * So there is a difference between XmlNodeServerObject::StyleSheet and IXml::IXslStylesheetNode
 * 
 * Therefore XML:
 * provide separate dedicated pure interfaces (with any templated dynamic_casts unfortunately)
 *   that can therefore be virtually inherited multiple times in the inheritance tree
 * provide separate partial concrete implementations
 *   that have useful concrete members
 *   that dont conflict
 *   that only implement their own incremental functional parts and members
 * provide access to the members of other interfaces via virtual functions and virtual interfaces
 *   that can be extra-inherited as they are pure virtual
 * also combine an abstract factory concept even though we are also building in direct knowledge via the inheritance
 * 
 * XmlNodeServerObject: provide access to other implemented interfaces and hidden runtime dynamic_cast<>
 *   that wont compile unless available
 * ALL Server Objects should be defined by the XML and ONLY createable via __cstr(IXmlNode *definition)
 *   unless creating a new one, in which case the parent node should be sent through
 *   __cstr(IXmlNode *parent, ...)
 */

#include <exception>
#include <iostream>
#include <string.h>
#include <vector>
#include <algorithm>
using namespace std;

#define implements_interface virtual public
#define interface_class class

#define RUN_SERVEROBJECT true

//aheads
class IXslStylesheetNode;
class Repository;
class Server;
class Service;

//--------------------------------- exceptions ---------------------
class InterfaceNotSupported: public exception {
  virtual const char* what() const throw() {return "Interface Not Supported";}  
} ins;
class NodeNotFound: public exception {
  virtual const char* what() const throw() {return "Node not found";}  
} nnf;

//--------------------------------- Xml interfaces (strict) ---------------------
//cannot be created directly, all virtual
interface_class IXmlBaseDoc {
public:
  virtual const char *name() = 0;
  virtual const char *meDoc() = 0;
};

interface_class IXmlDoc {
public:
  virtual const char *wibble() = 0;
};

interface_class IXmlBaseNode {
public:
  virtual const char *name() = 0;
  virtual const char *def_f() = 0;
  virtual const char *nameSpace() {return "";}
  virtual IXmlBaseDoc  *document() = 0; //BaseNode implementations should hold the document pointer and provide access to it
  template<class Derived> Derived *getSingleNodeType(const char *xpath) {
    //template cannot be virtual
    //or on XmlBaseNode (Server uses IXml interface)
    //  and all functions return IXml interface anyway
    //it is only for dynamic_cast of the actual virtual so we can live with it
    Derived *d = dynamic_cast<Derived*>(getSingleNode(xpath));
    if (!d) throw ins;
    return d;
  }
  //this one must be virtual
  virtual IXmlBaseNode *getSingleNode(const char *xpath) = 0;
  virtual vector<IXmlBaseNode*> *getMultipleNodes(const char *xpath) = 0;
  
  virtual IXslStylesheetNode *queryInterface(IXslStylesheetNode *p) {throw ins; return 0;}
};

interface_class IXmlNode {
public:
  virtual const char *name() {return "IXmlNode";}
}; //: public IXmlBaseNode
interface_class IXslNode {}; //: public IXmlBaseNode
interface_class IXslStylesheetNode: implements_interface IXmlBaseNode { //: public IXslNode, public IXmlBaseNode
public:
  virtual const char *transform() = 0;

  //other required interfaces
  //return type causes the conversion
  //argument selects the return type
  virtual IXslNode           *queryInterface(IXslNode *p) = 0;
};

//-------------------------------------- XmlNode concretes
//cannot be created directly still
//non-library-specific implementations of some functions
//not required to use this but cuts down implementation work
//and provides some base structure to the node (m_pDoc)
//all pure interfaces are virtual to avoid conflicts
class XmlBaseDoc: implements_interface IXmlBaseDoc {
  int m_i;
public:
  virtual const char *name()  {return "XmlBaseDoc";}
  virtual const char *meDoc() {return "meDoc";}
  
  XmlBaseDoc(int i): m_i(i) {}
};
class XmlDoc: implements_interface IXmlDoc, implements_interface IXmlBaseDoc {
public:
  virtual const char *wibble() {return meDoc();}
  virtual const char *name() {return "XmlDoc";}
};
class XmlBaseNode: implements_interface IXmlBaseNode {
protected:
  IXmlBaseDoc *m_pDoc; //access through the document()
public:
  virtual const char   *def_f() {return "XmlBaseNode";}
  virtual IXmlBaseDoc  *document() {return m_pDoc;} //provide accessors to these members via IXmlBaseNode interface
  virtual IXmlBaseNode *createNodeByNamespace(const char *sNamespace);
};
class XmlNode: implements_interface IXmlNode {};
class XslNode: implements_interface IXslNode, implements_interface IXmlBaseNode {};
class XslStylesheetNode: implements_interface IXslStylesheetNode, implements_interface IXslNode, implements_interface IXmlBaseNode {
  //additional virtual inheritance of IXmlBaseNode forces requirements for:
  // name() = 0;
  // def_() = 0;
  // document() = 0;
public:
  virtual const char *transform() {return this->document()->name();}
};

//-------------------------------------- LibXml-Xsl
//this IS *an* external library (LibXML2)
//create directly
//each concrete classes inherits ONLY ONE other concrete LibXml class and multiple XmlNode concretes
class LibXmlBaseDoc: public XmlBaseDoc {
protected:
  LibXmlBaseDoc(int i): XmlBaseDoc(i) {}
};
class LibXmlDoc: public LibXmlBaseDoc, public XmlDoc {
    const char *name() {return "LibXmlDoc";}
    
public:
    LibXmlDoc(int i): LibXmlBaseDoc(i) {}
};
class LibXmlBaseNode: public XmlBaseNode {
public:
  LibXmlBaseNode()   {m_pDoc = new LibXmlDoc(2);}
  ~LibXmlBaseNode()  {delete m_pDoc;}
  virtual const char *name() {return "LibXmlBaseNode";}
  virtual const char *nameSpace() {return "repository";}
  virtual IXmlBaseNode *getSingleNode(const char *xpath);
  virtual vector<IXmlBaseNode*> *getMultipleNodes(const char *xpath);
};

class LibXmlNode: public LibXmlBaseNode, public XmlNode {
    const char *name() {return "LibXmlNode";}
};
class LibXslNode: public LibXmlBaseNode, public XslNode {};

class LibXslStylesheetNode: public XslStylesheetNode, public LibXslNode {
  //XslStylesheetNode requirements (name, def_f) are satisfied by 
  //  LibXslNode -> LibXmlBaseNode::name()
  //  LibXslNode -> LibXmlBaseNode -> XmlBaseNode::def_f()
  int u;
  
public:
  LibXslStylesheetNode() {u=80;}
  //virtual int transform();
  virtual const char *name() {return "LibXslStylesheetNode";}

  //other interfaces required
  //must be declared here because this is where the switching between inherited vtables occurs
  //so we need access to both, and for the conversion to be valid and correct
  IXslStylesheetNode   *queryInterface(IXslStylesheetNode *p) {return this;}
  IXslNode             *queryInterface(IXslNode *p)           {return this;}
};

//-------------------------------------- native objects
class XmlNodeServerObject {
protected:
  IXmlBaseNode *m_pNode; //assigned in constructor only
  XmlNodeServerObject *m_xParent;
  
  template<class DerivedServerObject> vector<DerivedServerObject*> *getMultipleServerObjects(const char *xpath, const bool bRunServerObject = false) {
    vector<IXmlBaseNode*>        *vBaseNodes     = m_pNode->getMultipleNodes(xpath);
    vector<DerivedServerObject*> *vServerObjects = new vector<DerivedServerObject*>();
    //TODO: size(vBaseNodes->size());
    vector<IXmlBaseNode*>::iterator iBaseNode;
    for (iBaseNode = vBaseNodes->begin(); iBaseNode != vBaseNodes->end(); iBaseNode++) {
      DerivedServerObject *d = new DerivedServerObject(m_xParent, *iBaseNode);
      vServerObjects->push_back(d);
      if (bRunServerObject) d->runServerObject();
    }
    return vServerObjects;
  }
  
  virtual void runServerObject() {} //only when obvious: i.e. startup! so: Server, Service and Response, but not Repository
  
  //constructors: we need a node for this sort of construction
  XmlNodeServerObject(XmlNodeServerObject *pParent, IXmlBaseNode *pNode): m_xParent(pParent), m_pNode(pNode) {}
  XmlNodeServerObject(XmlNodeServerObject *pParent, const char *xpath):   m_xParent(pParent), m_pNode(pParent->m_pNode->getSingleNode(xpath)) {}
};

class GeneralServerXmlNodeServerObject: public XmlNodeServerObject {
protected:
  GeneralServerXmlNodeServerObject(XmlNodeServerObject *pParent, IXmlBaseNode *pNode): XmlNodeServerObject(pParent, pNode) {}
  GeneralServerXmlNodeServerObject(XmlNodeServerObject *pParent, const char *xpath):   XmlNodeServerObject(pParent, xpath) {}
};

class Repository: public GeneralServerXmlNodeServerObject {
public:
  Repository(XmlNodeServerObject *pServer, IXmlBaseNode *pNode): GeneralServerXmlNodeServerObject(pServer, pNode) {}
  Repository(XmlNodeServerObject *pServer, const char *xpath): GeneralServerXmlNodeServerObject(pServer, xpath) {}
  const char *name() {return m_pNode->name();}
  void save() {}
  void load() {}
  void unlink() {}
};

class Service: public GeneralServerXmlNodeServerObject {
public:
  Service(XmlNodeServerObject *pServer, IXmlBaseNode *pNode): GeneralServerXmlNodeServerObject(pServer, pNode) {}
  
  void runServerObject() {cout << "starting\n";}
};

class Server: public GeneralServerXmlNodeServerObject {
  const char *sName;
  vector<Service*> *m_services; //this owner sent through in constructor
  Repository *m_r;
  
  void loadAndRunServices() {
    m_r = new Repository(this, m_pNode->getSingleNode("//repsitory:*[0]"));
    m_r = new Repository(this, "//repsitory:*[0]");
    //get and start all services
    m_services = getMultipleServerObjects<Service>("//gs:service", RUN_SERVEROBJECT);
  }
  
public:
  Server(IXmlBaseNode *pNode): GeneralServerXmlNodeServerObject(0, pNode) {}
  const char *name() {return m_pNode->name();}
  void runServerObject() {loadAndRunServices();}
  void stop() {}
  vector<Service*> *services() {return m_services;}
};

