//platform specific file (WINDOWS)
#include "XML.h"
#include "Repository.h"
#include <exception>

using namespace std;

/* --------------------------------------- A word about SMART pointers
SMART pointers exist for auto-garbage collection
They are local objects, not pointers, and are deleted once they go out of scope
It is the pointer itself that monitors references to itself, not the underlying COM Object
  So: two SMART pointers to the same COM Object would simply be 2 pointers with 1 ref on each
  If anything tried to copy one or run a function that requested / used another interface, 1+ ref would happen
  Because they are not passing pointers, but SMART pointers by value and thus running copy constructors
  Releasing the SMART pointer to 0 refs does not delete the COM Object, just the SMART pointer
Their local stack ~Destructors() run IUnknown->Release()
Their copy constructors run AddRef()
Any IInterface requests run AddRef()
There is never a need to delete them, they will Release() and delete when their holding Class ~Destructor() runs
*/

#include <comutil.h> //code in comsuppw.lib (dependency). VARIANT types utilities
using namespace MSXML2;
using namespace _com_util; //VARIANT types utilities

namespace general_server {
	//-----------------------------------------------
	//--------------------------------------- XmlDoc
	//-----------------------------------------------
	XmlDoc::~XmlDoc() {
		//release the actual document
		//which will also release all the COM nodes
#ifdef _DEBUG
		assert(refCount()); 
#endif

		//m_oDoc.Release(); //it is a smart pointer so it will release itself
	}

	const bool XmlDoc::createDOMDoc() {
		bool bOk = false;

		//DOMDocument
		m_oDoc.CreateInstance(__uuidof(FreeThreadedDOMDocument60)); //(1 ref)
		m_oDoc->preserveWhiteSpace = VARIANT_TRUE;
		m_oDoc->validateOnParse = VARIANT_FALSE;
		m_oDoc->async = VARIANT_FALSE; //synchronous loading off
		m_oDoc->resolveExternals = VARIANT_TRUE;

		//NameSpaceManager
		m_oDoc->setProperty("SelectionNamespaces", NAMESPACE_ALL);

		//COM and result
		setUnknown(m_oDoc); //manage our COM component IUnknown (+0 ref)
		bOk = (m_oDoc != 0);
		return bOk;
	}
	IXmlDoc *XmlDoc::factory(const char *sAlias) const {
		//create a new Doc from this one (of the same type)
		return new XmlDoc(sAlias);
	}
	void XmlDoc::loadXml(const char *sXML) {
		//caller frees the sXML
		//createDoc required
		assert(sXML && m_oDoc);
		BSTR bstrXML = ConvertStringToBSTR(sXML);
		m_oDoc->loadXML(bstrXML);

		SysFreeString(bstrXML);
	}

	IXmlNode *XmlDoc::documentNode_callerFree() const {
		//caller must delete the XmlNode
		IXMLDOMNodePtr oNode = m_oDoc;
		return new XmlNode(oNode, (XmlDoc*) this);
	}
	IXmlNode *XmlDoc::nodeFromID_callerFree(const char *sID) const {
		BSTR bstrID=ConvertStringToBSTR(sID);
		IXmlNode *pNode=new XmlNode(m_oDoc->nodeFromID(bstrID), (XmlDoc*) this);
		SysFreeString(bstrID);
		return pNode;
	}
	const bool XmlDoc::visitClone(XmlDoc *pToDoc) const {
		//using Visitor pattern
		//this = pSourceDoc = const = XmlDoc
		//pToDoc = not const = XmlDoc
		//always clone the entire doc
		pToDoc->m_oDoc = m_oDoc->cloneNode(VARIANT_TRUE); //deep clone
		pToDoc->setUnknown(pToDoc->m_oDoc);
		return true;
	}
	IXmlNode *XmlDoc::visitCreateElement(const char *sTagName, XmlNode *pInNode) {
		//caller frees result
		//elements (Nodes) should always be attached to a document
		BSTR bstrTagName = ConvertStringToBSTR(sTagName);
		IXMLDOMNodePtr oNode = m_oDoc->createElement(bstrTagName); //(1 ref)
		SysFreeString(bstrTagName);
		if (pInNode) pInNode->m_oNode = oNode; //just assign it
		else pInNode = new XmlNode(oNode, this); //create the thing in the appropriate address
		//COM Smart pointer frees itself
		return pInNode;
	}

	
	//-----------------------------------------------
	//--------------------------------------- XmlNode
	//-----------------------------------------------
	map<const void*, IXSLTemplatePtr> XmlNode::vCachedTemplates;
	pthread_mutex_t XmlNode::m_mAtomicSetAttribute = PTHREAD_MUTEX_INITIALIZER;

	const iDOMNodeType XmlNode::nodeType() const {
		//the two type enums are equal
		return (iDOMNodeType) m_oNode->nodeType;
	}
	XmlNode::XmlNode(const IXmlNode *pFromNode): IXmlNode(pFromNode) {
		//assumes this = same type pFromNode
		m_oNode = ((XmlNode*) pFromNode)->m_oNode;
	}
	XmlNode::XmlNode(IXMLDOMNodePtr oNode, IXmlDoc *pDoc): IXmlNode("uninitialised", pDoc), COM(oNode), m_oNode(oNode) {
		//all IXMLDOMNodePtr should immediately be couched in this. Thus it represents (1 ref)
		assert(m_oNode);
		m_sTagName = name();
		assert(m_sTagName);
	}
	XmlNode::XmlNode(const char *sTagName, IXmlDoc *pDoc): IXmlNode(sTagName, pDoc) {
		//have to create an IXmlDoc first to attch this to
		assert(sTagName && pDoc);
		m_pDoc->acceptCreateElement(sTagName, this); //(1 ref)
		setUnknown(m_oNode); //manage our COM component IUnknown 
	}
	const char *XmlNode::name() const {
		//caller frees sValue if non-zero
		//this is the full qualified name of the node e.g. xsl:template
		//IXML functions split out the prefix and basename
		char *sValue = 0;
		if (m_oNode) {
			BSTR bstrValue = m_oNode->nodeName; 
			if (bstrValue) {
				sValue = ConvertBSTRToString(bstrValue);
				SysFreeString(bstrValue);
			}
		}
		return sValue;
	}
	const size_t XmlNode::position() const {
		return 0;
	}
	const char *XmlNode::name(const char *sName) {
		//DOMDocument 2.0 specification does not allow node name changes, DOMDocument 3.0 specification does
		//so we will implement it
		if (m_oNode) {
			//create same element with new name
			XmlNode *pNewNode = new XmlNode(sName, m_pDoc);
			//move children
			XmlNodeList *pKids = getMultipleNodes("*|@*|text()");
			for (XmlNodeList::iterator iKid = pKids->begin(); iKid != pKids->end(); iKid++) {
				delete pNewNode->appendChild_callerFree(*iKid);
			}
			//append new
			IXmlNode *pParentNode = parentNode_callerFree();
			pParentNode->appendChild_callerFree(pNewNode, MOVE_NODE, MOVE_NODE);
			//delete old
			remove();
			//clever swap of pointers to new
			m_oNode = pNewNode->m_oNode;
			//free
			delete pNewNode;
			delete pKids;
		}
		return sName;
	}
	XmlNode::~XmlNode() {
#ifdef _DEBUG
		//releasing a 0 ref count isnt very healthy!
		//has to be done here because the ~COM() is after the ~XmlNode() Release()
		assert(refCount()); 
#endif
		//m_oNode.Release(); //it is a smart pointer so it will release itself on destroy
	}
	XmlNodeList *XmlNode::getMultipleNodes(const char *sXPath) const {
		//these new nodes are added to the NodeList vector, not the document vector
		//they are not freed by the document, but by the NodeList object in its destructor
		//the caller must, however delete the XmlNodeList*
		//NAMESPACE AWARE: xpath requires the namespace prefixes registered and correct xpath namespace prefixes
		XmlNodeList *aNodeList = new XmlNodeList();
		IXmlNode *oNode;


		MSXML2::IXMLDOMNodeListPtr aNodeset;
		try {
			//silently ignore badly formatted xpath statements
			aNodeset = m_oNode->selectNodes(sXPath); //(1 ref)
		} catch(_com_error &e) {
			long lCode;
			const char *sDesc = interpretCOMError(e, &lCode);
			return 0;
		}
		MSXML2::IXMLDOMNodePtr oLibXmlNode = aNodeset->nextNode(); //(1 ref)
		while (oLibXmlNode) {
			//couch the IXMLDOMNodePtr in its user class representing (1 ref)
			oNode = new XmlNode(oLibXmlNode, m_pDoc); //(+1 ref from copy constructor)
			aNodeList->push_back(oNode);
			oLibXmlNode = aNodeset->nextNode(); //(temporary +1 ref on referenced node, and -1 on previous)
		}
		return aNodeList;
		//aNodeset->Release(); //(-1 ref and destruction smart pointer releases itself)
	}
	const char *XmlNode::attributeValue(const char *sName) const {
		//caller frees result
		//NAMESPACE AWARE: name may include namespace prefix, e.g. rxsl:regex or not e.g. regex
		const char *sValue = 0;
		MSXML2::IXMLDOMNodePtr oLibXmlNode;
		MSXML2::IXMLDOMNamedNodeMapPtr oLibXmlNamedNodeMap;
		BSTR bstrValue;

		if (m_oNode) {
			if (oLibXmlNamedNodeMap = m_oNode->attributes) {
				if (oLibXmlNode = oLibXmlNamedNodeMap->getNamedItem(sName)) {
					bstrValue = oLibXmlNode->text;
					sValue = ConvertBSTRToString(bstrValue);
					SysFreeString(bstrValue);
				}
			}
		}

		//XML specification requires that newlines are only \n and newlines will be normalised by this function
		//so we incorporate a special attribute "normalise" to allow c-string style escaping \r\n
		//note that HTTP newlines and Windows newlines are \r\n or &#13;&#10;
		//defaults to off
		if (sValue && *sValue && strstr(sName, "_normalise")) {
			const char *sValueNormalised = normalise(sValue);
			//swap them
			free((void*) sValue);
			sValue = sValueNormalised;
		}

		return sValue;
	}
	IXmlNode *XmlNode::setAttribute(const char *sName, const char *sValue) const {
		//caller frees result which is always new
		//sets it even if it isnt there
		//NAMESPACE AWARE: name may include namespace prefix, e.g. rxsl:regex or not e.g. regex
		XmlNode *pNewAttr = 0;

		BSTR bstrName = ConvertStringToBSTR(sName);
		BSTR bstrValue = ConvertStringToBSTR(sValue);

		//check to see if node exists
		MSXML2::IXMLDOMNamedNodeMapPtr oLibXmlNamedNodeMap;
		MSXML2::IXMLDOMAttributePtr oAttributeNode;

		if (m_oNode) {
			if (oLibXmlNamedNodeMap = m_oNode->attributes) {
				//this part needs to be atmoic
				pthread_mutex_lock(&m_mAtomicSetAttribute);
				if (oAttributeNode = oLibXmlNamedNodeMap->getNamedItem(sName)) {
					//set value on existing
					oAttributeNode->put_text(bstrValue);
				} else {
					//create new attribute node
					oAttributeNode = ((XmlDoc*) m_pDoc)->m_oDoc->createAttribute(bstrName); //(1 ref)
					oAttributeNode->put_text(bstrValue);
					try {
						//this will crash out if the attribute node is already there
						oLibXmlNamedNodeMap->setNamedItem(oAttributeNode);
					} catch(_com_error &e) {
						long lCode;
						const char *sDesc = interpretCOMError(e, &lCode);
					}
				}
				pthread_mutex_unlock(&m_mAtomicSetAttribute);
			}
			pNewAttr = new XmlNode(oAttributeNode, m_pDoc);
		}

		//free
		SysFreeString(bstrName);
		SysFreeString(bstrValue);

		return pNewAttr;
	}

	const char *XmlNode::value() const {
		//caller frees result if non-zero
		//get value
		BSTR bstrValue = m_oNode->text;
		const char *sValue = ConvertBSTRToString(bstrValue);
		SysFreeString(bstrValue);

		//XML specification requires that newlines are only \n and newlines will be normalised by this function
		//so we incorporate a special attribute "normalise" to allow c-string style escaping \r\n
		//note that HTTP newlines and Windows newlines are \r\n or &#13;&#10;
		//defaults to off
		if (sValue && *sValue && isNodeElement()) {
			const char *sNormalise = attributeValue("normalise");
			if (sNormalise) {
				if (!strncmp(sNormalise, "yes", 3)) {
					const char *sValueNormalised = normalise(sValue);
					//swap them
					free((void*) sValue);
					sValue = sValueNormalised;
				}
				free((void*) sNormalise);
			}
		}
		return sValue;
	}
	IXmlNode *XmlNode::visitClone(const bool bDeep, XmlNode *pToNode) const {
		MSXML2::IXMLDOMNodePtr oNode = m_oNode->cloneNode(bDeep ? VARIANT_TRUE : VARIANT_FALSE);
		if (pToNode) {
			pToNode->m_oNode = oNode;
			pToNode->m_pDoc = m_pDoc;
			pToNode->m_sTagName = _STRDUP(m_sTagName);
		} else pToNode = new XmlNode(oNode, m_pDoc);
		return pToNode;
	}
	const char *XmlNode::value(const char *sValue) {
		//caller frees sValue
		BSTR bstrValue = ConvertStringToBSTR(sValue);
		m_oNode->put_text(bstrValue);
		SysFreeString(bstrValue);
		return sValue;
	}
	char *XmlNode::xml() const {
		//caller freed char*
		char *sXML=ConvertBSTRToString(m_oNode->xml);
		return sXML;
	}
	IXmlNode *XmlNode::parentNode_callerFree() const {
		//caller free (if not 0)
		//return 0 if no parent
		MSXML2::IXMLDOMNodePtr oParentNode = m_oNode->parentNode; //(1 ref)
		return oParentNode ? new XmlNode(oParentNode, m_pDoc) : 0; //(+1 ref in copy constructor)
	}
	IXmlNode *XmlNode::firstChild_callerFree() const {
		//caller free (if not 0)
		//return 0 if no first child
		//includes processing instructions by default
		MSXML2::IXMLDOMNodePtr oFirstChild = m_oNode->firstChild; 
		return oFirstChild ? new XmlNode(oFirstChild, m_pDoc) : 0;
	}
	IXmlNode *XmlNode::clone(const bool bDeep, IXmlNode *pToNode) const {
		return pToNode ? pToNode->acceptClone(bDeep, this) : (new XmlNode())->acceptClone(bDeep, this);
	}
	IXmlNode *XmlNode::visitAppendChild(XmlNode *pNode, const bool bDeepClone, const bool bClone) {
		//caller manages pNode
		//caller additonally frees result *if cloned*
		//if the node is moved from another document and not cloned then the COM ref count stays the same
		XmlNode *pNewNode = 0, *pFirstChild = 0;
		if (pNode) {
			//sometimes the node might not be the right type
			if (pNode->isNodeDocument()) pNode = pFirstChild = (XmlNode*) pNode->firstChild_callerFree();
			if (pNode) {
				//clone?
				if (bClone) pNewNode = (XmlNode*) pNode->clone(bDeepClone); //(1 ref)
				else pNewNode = pNode; //(+0 ref)
				//append attributes differently
				//MSXML does not allow appendChild() with attributes
				if (pNewNode->isNodeAttribute()) {
					pNewNode = (XmlNode*) IXmlNode::setAttribute(pNewNode); //return the created new attribute
				} else {
					try {
						m_oNode->appendChild(pNewNode->m_oNode); //(+0 ref)
					} catch(_com_error &e) {
						long lCode;
						const char *sDesc = interpretCOMError(e, &lCode);
						return 0;
					}
				}
				pNewNode->m_pDoc = m_pDoc;
			}
			if (pFirstChild && bClone) {delete pFirstChild; pFirstChild = 0;}
		}
		return pNewNode;
	}
	const bool XmlNode::visitRemoveChild(XmlNode *pChild) {
		try {
			m_oNode->removeChild(pChild->m_oNode);
		} catch(_com_error &e) {
			long lCode;
			const char *sDesc = interpretCOMError(e, &lCode);
			return 0;
		}
		return true;
	}

	IXmlDoc *XmlNode::visitTransform(const XmlNode *pXSLNode, const map<const char*, const size_t> *pParamsInt, const map<const char*, const char*> *pParamsChar, const char *sWorkingDirectory) const {
		//Note that the Visitor function is assuming that the pParams have the same type of concrete class (XmlNode) as pXSLDoc
		//pXSLNode can be the File repository above or the direct xsl:stylesheet node
		//using templates for compilation cacheing and parameters
		XmlDoc *pNewDoc = 0, *pXSLDoc2 = 0;

		//parameters
		XmlNodeList::const_iterator iNodeParam;
		map<const char*, const size_t>::const_iterator iParamInt;
		map<const char*, const char* >::const_iterator iParamChar;
		const char *sName, *sValue;
		size_t uValue;

		//compiled template, processor
		IXSLTemplatePtr oXSLTemplate;
		IXSLProcessorPtr oXSLProcessor;
		VARIANT_BOOL bResult;

		if (pXSLNode) { //quietly ignore missing XSL input
			pNewDoc = new XmlDoc("transform");

			//processor compilation cacheing
			oXSLTemplate = vCachedTemplates[pXSLNode];
			if (!oXSLTemplate) {
				//no template in cache, create it and cache it
				//request pre-processing of the stylesheet which includes
				//  translate xsl:includes to absolute paths (translateIncludes)
				//  complete clone (to AVOID read-only compiled MSXML conflicts)
				//  processing of repository info -> xsl:stylesheet node
				//created document using the factory method on pXSLNodes document pointer
				pXSLDoc2 = (XmlDoc*) pXSLNode->preprocessToStylesheet(&sWorkingDirectory); 

				//create the template
				//http://msdn.microsoft.com/en-us/library/windows/desktop/ms762312%28v=vs.85%29.aspx
				oXSLTemplate.CreateInstance(__uuidof(XSLTemplate60));
				try {
					//externals may not resolve, etc.
					oXSLTemplate->stylesheet = pXSLDoc2->m_oDoc.GetInterfacePtr();
				} catch (_com_error &e) {
					long lCode;
					const char *sDesc = interpretCOMError(e, &lCode);
					return 0;
				}

				//cache the template
				vCachedTemplates[pXSLNode] = oXSLTemplate;
			}


			//assign inputs and outputs
			oXSLProcessor = oXSLTemplate->createProcessor();
			oXSLProcessor->input = m_oNode.GetInterfacePtr();
			oXSLProcessor->output = pNewDoc->m_oDoc.GetInterfacePtr();
			//oXSLProcessor->clearParameters(); //NOT available!
			if (pParamsInt) {
				for (iParamInt = pParamsInt->begin(); iParamInt != pParamsInt->end(); iParamInt++) {
					sName = iParamInt->first;
					uValue = iParamInt->second;
					oXSLProcessor->addParameter(sName, (unsigned int) uValue, "");
				}
			}
			if (pParamsChar) {
				for (iParamChar = pParamsChar->begin(); iParamChar != pParamsChar->end(); iParamChar++) {
					sName = iParamChar->first;
					sValue = iParamChar->second;
					oXSLProcessor->addParameter(sName, sValue, "");
				}
			}

			//transform
			try {
				bResult = oXSLProcessor->transform();
			} catch (_com_error &e) {
				long lCode;
				const char *sDesc = interpretCOMError(e, &lCode);
				return 0;
			}

			//remove the top level XML decleration otherwise it wont merge with other documents
			pNewDoc->removeProcessingInstructions();
		}

		//free
		if (pXSLDoc2) delete pXSLDoc2;

		return pNewDoc;
	}
}
