#define T_XMLLIBRARY_TEMPLATE_LIST   class TIXmlBaseDoc, class TIXmlBaseNode, class TConcreteBaseNode
#define T_XMLLIBRARY_TEMPLATE_PARAMS TIXmlBaseDoc, TIXmlBaseNode, TConcreteBaseNode
#define T_XMLLIBRARY_TEMPLATE_CLASS  template<T_XMLLIBRARY_TEMPLATE_LIST = IOptionalTemplate>
#define T_XMLLIBRARY_TEMPLATE_FUNC   template<T_XMLLIBRARY_TEMPLATE_LIST>

#define T_XMLLIBRARY_TEMPLATE_XML     IXmlDoc, IXmlNode
#define T_XMLLIBRARY_TEMPLATE_XSL     IXslDoc, IXslNode
#define T_XMLLIBRARY_TEMPLATE_XSCHEMA IXSchemaDoc, IXSchemaNode

class IXmlNode;
class IXslNode;
class IOptionalTemplate;

//--------------------------------- base interfaces ---------------------
T_XMLLIBRARY_TEMPLATE_CLASS class IXmlBaseDoc {
public:
  virtual TIXmlBaseNode *newNode() = 0;
  virtual const char *whoami() {return "IXmlBaseDoc";}
};

T_XMLLIBRARY_TEMPLATE_CLASS class IXmlBaseNode {
  //this is an interface pointer so we can hold it here
  //as the conrete classes dont want it apart from to access it
  TIXmlBaseDoc *m_pDOc;

protected:
  IXmlBaseNode(TIXmlBaseDoc *pDoc): m_pDOc(pDoc) {}

public:
  virtual TIXmlBaseDoc *document() {return m_pDOc;}
  virtual const char *whoami() {return "IXmlBaseNode";}
  virtual TIXmlBaseNode *newNode() {return document()->newNode();}
};

//--------------------------------- doc interfaces ---------------------
class IXmlDoc: public IXmlBaseDoc<T_XMLLIBRARY_TEMPLATE_XML> {
};
class IXslDoc: public IXmlBaseDoc<T_XMLLIBRARY_TEMPLATE_XSL> {
};

//--------------------------------- node interfaces ---------------------
class IXmlNode: public IXmlBaseNode<T_XMLLIBRARY_TEMPLATE_XML> {
protected:
  IXmlNode(IXmlDoc *p): IXmlBaseNode(p) {}
  
};

class IXslNode: public IXmlBaseNode<T_XMLLIBRARY_TEMPLATE_XSL> {
protected:
  IXslNode(IXslDoc *p): IXmlBaseNode(p) {}
};



//---------------------------------------------------------------------
//--------------------------------- concrete docs ---------------------
//---------------------------------------------------------------------
class LibXmlNode;
class LibXslNode;

T_XMLLIBRARY_TEMPLATE_CLASS class LibXmlBaseDoc: public TIXmlBaseDoc {
  //pure virtual interface: cannot be created
public:
  virtual TIXmlBaseNode *newNode();
};

class LibXmlDoc: public LibXmlBaseDoc<T_XMLLIBRARY_TEMPLATE_XML, LibXmlNode> {
protected:
public:
  const char *whoami() {return "LibXmlDoc";}
  IXmlNode *newNode();
};

class LibXslDoc: public LibXmlBaseDoc<T_XMLLIBRARY_TEMPLATE_XSL, LibXslNode> {
protected:
public:
  const char *whoami() {return "LibXslDoc";}
};

//--------------------------------- concrete nodes ---------------------
T_XMLLIBRARY_TEMPLATE_CLASS class LibXmlBaseNode: public TIXmlBaseNode {
  //pure virtual interface
  //cannot be created
protected:
  LibXmlBaseNode(TIXmlBaseDoc *p): TIXmlBaseNode(p) {}
};

class LibXmlNode: public LibXmlBaseNode<T_XMLLIBRARY_TEMPLATE_XML, LibXmlNode> {
protected:
public:
  LibXmlNode(IXmlDoc *p): LibXmlBaseNode(p) {}
  const char *whoami() {return "LibXmlNode";}
  
};
class LibXslNode: public LibXmlBaseNode<T_XMLLIBRARY_TEMPLATE_XSL, LibXslNode> {
protected:
public:
  LibXslNode(IXslDoc *p): LibXmlBaseNode(p) {}
  const char *whoami() {return "LibXslNode";}
};
