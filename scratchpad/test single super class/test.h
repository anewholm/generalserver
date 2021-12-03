//--------------------------------- aheads ---------------------
class IXslDoc;
class IXmlBaseNode;

//--------------------------------- misc ---------------------
class IXmlLibrary {
public:
  IXslDoc  *factory_stylesheet();
};

//--------------------------------- base interfaces ---------------------
//an XSL Node may be found within an XML document
//and an XSchema node can be found in an XML document
//it is up to the node creating procedure to assign the document
//getSingleNode() will return a concrete type based on the namespace, maintaining the same document
class IXmlBaseDoc {
  IXmlLibrary *fact;
  
public:
  IXmlBaseDoc() {fact = new IXmlLibrary();}
  IXmlLibrary *factory() const {return fact;}
  
  virtual const char *whoami() {return "IXmlBaseDoc";}
  virtual const bool isXslDoc() const      {return false;}
  virtual const bool isXmlDoc() const      {return false;}
  virtual const bool isXSchemalDoc() const {return false;}
  virtual IXmlBaseNode *createElement() = 0;
};

class IXmlBaseNode {
  //this is an interface pointer so we can hold it here
  //as the conrete classes dont want it apart from to access it

protected:
  //doc type does not have to comply with node type
  IXmlBaseDoc *m_pDoc;
  IXmlBaseNode(IXmlBaseDoc *pDoc): m_pDoc(pDoc) {}

public:
  IXmlLibrary *factory() const {return m_pDoc->factory();}
  
  virtual IXmlBaseDoc *document() {return m_pDoc;}
  virtual const char *whoami() {return "IXmlBaseNode";}
  virtual IXmlBaseNode *getSingleNode(const char *xpath) = 0;
};

//--------------------------------- doc interfaces ---------------------
class IXmlDoc: public IXmlBaseDoc {
public:
  const char *whoami() {return "IXmlDoc";}
  const bool isXmlDoc() const {return true;}
  IXmlBaseNode *createElement();
};
class IXslDoc: public IXmlBaseDoc {
public:
  const char *whoami() {return "IXslDoc";}
  const bool isXslDoc() const {return true;}
  IXmlBaseNode *createElement();
};

//--------------------------------- node interfaces ---------------------
class IXmlNode: public IXmlBaseNode {
protected:
  IXmlNode(IXmlBaseDoc *p): IXmlBaseNode(p) {}
};

class IXslNode: public IXmlBaseNode {
protected:
  IXslNode(IXmlBaseDoc *p): IXmlBaseNode(p) {}
public:
  virtual IXmlDoc *transform() = 0;
};



//---------------------------------------------------------------------
//--------------------------------- concrete docs ---------------------
//---------------------------------------------------------------------
class LibXmlNode;
class LibXslNode;

template<class TIXmlBaseDoc> class LibXmlBaseDoc: public TIXmlBaseDoc {
  //pure virtual interface: cannot be created
  //template controls only the inheritance to force implementation of the specific I*Node interface
  //but share general implementation
public:
};

class LibXmlDoc: public LibXmlBaseDoc<IXmlDoc> {
protected:
public:
  const char *whoami() {return "LibXmlDoc";}
};

class LibXslDoc: public LibXmlBaseDoc<IXslDoc> {
protected:
public:
  const char *whoami() {return "LibXslDoc";}
};

//--------------------------------- concrete nodes ---------------------
template<class TIXmlBaseNode> class LibXmlBaseNode: public TIXmlBaseNode {
  //pure virtual interface: cannot be created
  //template controls only the inheritance to force implementation of the specific I*Node interface
  //but share general implementation
protected:
  LibXmlBaseNode(IXmlBaseDoc *p): TIXmlBaseNode(p) {}
public:
  IXmlBaseNode *getSingleNode(const char *xpath);
};

class LibXmlNode: public LibXmlBaseNode<IXmlNode> {
protected:
public:
  LibXmlNode(IXmlBaseDoc *p): LibXmlBaseNode(p) {}
  const char *whoami() {return "LibXmlNode";}
};
class LibXslNode: public LibXmlBaseNode<IXslNode> {
protected:
public:
  LibXslNode(IXmlBaseDoc *p): LibXmlBaseNode(p) {}
  const char *whoami() {return "LibXslNode";}
  IXmlDoc *transform();
};
