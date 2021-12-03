//platform specific file (WINDOWS)
#ifndef _XML_H
#define _XML_H

//std library includes
#include <iostream>
#include <vector>
#include <map>
#include <pthread.h>
using namespace std;

//platform agnostic includes
#include "IXml.h"

//windows XML library pointed to by IXMLDOMDocument3Ptr requires the #import to load all the lib types included in stdafx.h included in XML.h
#include "stdafx.h" //windows precompiled header includes #import of MSXML
using namespace MSXML2;
#include "COM.h"

namespace general_server {
	class XmlNode;
	class XmlNodeList;
	class Repository;

	//------------------------------------------------
	//--------------------------------------- XmlDoc
	//------------------------------------------------
	class XmlDoc: public IXmlDoc, private COM {
		//only classes in this file should have access to the platform specific members
		//_com_error: http://msdn.microsoft.com/en-us/library/0ye3k36s%28v=VS.80%29.aspx
		friend class XmlNode;
		friend class XmlNodeList;
		friend class IXmlDoc;

	protected:
		//Visitor pattern
		//http://en.wikipedia.org/wiki/Visitor_pattern
		//inherited overridden in every base to pass back a concrete pointer
		const bool acceptClone(const IXmlDocVisitor *pVisitor) {return pVisitor->visitClone(this);}
		const bool visitClone(XmlDoc *pFromDoc) const;
		IXmlNode *acceptCreateElement(const char *sTagName, IXmlNode *pVisitor = 0) {return pVisitor->visitCreateElement(sTagName, this);}
		IXmlNode *visitCreateElement(const char *sTagName, XmlNode *pVisitor = 0);

	private:
		IXMLDOMDocument3Ptr m_oDoc; //pointer to base WINDOWS XML library
		const bool createDOMDoc();


	public:
		//constructors cannot be inherited in C++
		//virtual functions cannot be called from constructors
		XmlDoc(): IXmlDoc() {IXmlDoc_construct();}
		XmlDoc(const char *sAlias): IXmlDoc(sAlias) {IXmlDoc_construct(sAlias);}
		XmlDoc(const char *sAlias, const char *sXML): IXmlDoc(sAlias, sXML) {IXmlDoc_construct(sAlias, sXML);}
		XmlDoc(const char *sAlias, Repository *repository): IXmlDoc(sAlias, repository) {IXmlDoc_construct(sAlias, repository);}
		XmlDoc(const char *sAlias, const char *sFilePath, const bool diff): IXmlDoc(sAlias, sFilePath, diff) {IXmlDoc_construct(sAlias, sFilePath, diff);}
		XmlDoc(const char *sAlias, const XmlNodeList *pSourceNodeList, const char *sRootTagName = "root", const bool bDeepClone = DEEP_CLONE): IXmlDoc(sAlias, pSourceNodeList, sRootTagName, bDeepClone) {IXmlDoc_construct(sAlias, pSourceNodeList, sRootTagName, bDeepClone);}
		XmlDoc(const char *sAlias, const vector<const IXmlDoc *> *pDocs, const char *sRootTagName = "root"): IXmlDoc(sAlias, pDocs, sRootTagName) {IXmlDoc_construct(sAlias, pDocs, sRootTagName);}
		XmlDoc(const char *sAlias, const vector<const IXmlDoc *> *pDocs, const IXmlNode *pRootNode): IXmlDoc(sAlias, pDocs, pRootNode) {IXmlDoc_construct(sAlias, pDocs, pRootNode);}

		//creation from sourceDoc or node with clone option (and variable const)
		//const = clone from source, bAlwaysClone has no effect except to select this function
		XmlDoc(const char *sAlias, const IXmlNode *pSourceNode, const bool bDeepClone): IXmlDoc(sAlias, pSourceNode, bDeepClone) {IXmlDoc_construct(sAlias, pSourceNode, bDeepClone);} //const causes clone, but deep clone?
		XmlDoc(const char *sAlias, IXmlNode *pSourceNode): IXmlDoc(sAlias, pSourceNode) {IXmlDoc_construct(sAlias, pSourceNode);}
		XmlDoc(const char *sAlias, const IXmlDoc *pSourceDoc, const bool bDeepClone): IXmlDoc(sAlias, pSourceDoc, bDeepClone) {IXmlDoc_construct(sAlias, pSourceDoc, bDeepClone);} //const causes clone, but deep clone?
		XmlDoc(const char *sAlias, IXmlDoc *pSourceDoc): IXmlDoc(sAlias, pSourceDoc) {IXmlDoc_construct(sAlias, pSourceDoc);}

		~XmlDoc();
		IXmlDoc *factory(const char *sAlias) const; //create a new Doc from this one (of the same type)
		void loadXml(const char *sXML);

		//information
		const bool loadedAndParsed() const {return (m_oDoc->parsed == VARIANT_TRUE);}
		const bool resolveExternals() const {return (m_oDoc->resolveExternals == VARIANT_TRUE);}
		const bool specified() const {return (m_oDoc->specified == VARIANT_TRUE);}
		const bool validateOnParse() const {return (m_oDoc->validateOnParse == VARIANT_TRUE);}

		IXmlNode *documentNode_callerFree() const; //the top slash, i.e. xpath:/
		IXmlNode *nodeFromID_callerFree(const char *sID) const;
		IXmlNode *createElement(const char *sTagName, IXmlNode *pInNode) {return acceptCreateElement(sTagName, pInNode);}
		const bool clone(IXmlDoc *pToDoc) const {return pToDoc->acceptClone(this);} //Visitor pattern using IXmlDocVisitor class

		//update: all these fucntions are implemented in IXmlDoc and defer processing to IXmlNode
		//appendChild_*() -> XmlNode::appendChild_*()
	};



	//------------------------------------------------
	//--------------------------------------- XmlNode
	//------------------------------------------------
	class XmlNode: public IXmlNode, private COM {
		//only classes in this file should have access to the platform specific members
		//no concept of child XmlNodes exists here
		//transforms and XPath should be used to query
		//_com_error: http://msdn.microsoft.com/en-us/library/0ye3k36s%28v=VS.80%29.aspx
		friend class XmlDoc;
		friend class IXmlDoc;

		static map<const void*, IXSLTemplatePtr> vCachedTemplates;
		static pthread_mutex_t m_mAtomicSetAttribute; //platform specific (WINDOWS)

	protected:
		//Visitor pattern conrete type resolution functions
		IXmlNode *acceptClone(const bool bDeep, const IXmlNodeVisitor *pVisitor) {return pVisitor->visitClone(bDeep, this);}
		IXmlNode *visitClone(const bool bDeep, XmlNode *pToNode) const;
		IXmlNode *acceptAppendChild(IXmlNodeVisitor *pVisitor, const bool bDeepClone, const bool bClone) {return pVisitor->visitAppendChild(this, bDeepClone, bClone);}
		IXmlNode *visitAppendChild(XmlNode *pXmlNode, const bool bDeepClone, const bool bClone);
		IXmlDoc *acceptTransform(const IXmlNode *pVisitor, const map<const char*, const size_t> *pParamsInt = 0, const map<const char*, const char*> *pParamsChar = 0, const char *sWorkingDirectory = 0) const {return pVisitor->visitTransform(this, pParamsInt, pParamsChar, sWorkingDirectory);}
		IXmlDoc *visitTransform(const XmlNode *pXSLDoc, const map<const char*, const size_t> *pParamsInt = 0, const map<const char*, const char*> *pParamsChar = 0, const char *sWorkingDirectory = 0) const;
		const bool acceptRemoveChild(IXmlNode *pVisitor) {return pVisitor->visitRemoveChild(this);}
		const bool visitRemoveChild(XmlNode *pChildNodeToRemove);
		IXmlNode *visitCreateElement(const char *sTagName, IXmlDoc *pDoc) {return pDoc->visitCreateElement(sTagName, this);}

	private:
		//platform specific
		IXMLDOMNodePtr m_oNode;
		XmlNode(IXMLDOMNodePtr oNode, IXmlDoc *m_pDoc);

		//platform agnostic
		XmlNode() {} //only used in cloning. we need so we can indicate the type to clone into
		const iDOMNodeType nodeType() const;

	public:
		//constructors cannot be inherited in C++
		//virtual functions cannot be called from constructors
		XmlNode(const char *sTagName, IXmlDoc *m_pDoc);
		XmlNode(const IXmlNode *pFromNode); //assumes this = same type pFromNode
		~XmlNode();

		//information
		XmlNodeList *getMultipleNodes(const char *sXPath) const;
		IXmlNode *firstChild_callerFree() const;
		IXmlNode *parentNode_callerFree() const;
		const char *attributeValue(const char *sName) const;
		IXmlNode *setAttribute(const char *sName, const char *sValue) const;
		const char *value() const;
		const char *name() const;
		const size_t position() const;
		char *xml() const;

		//update
		IXmlNode *appendChild_callerFree(IXmlNode *pNode, const bool bDeepClone = SHALLOW_CLONE, const bool bClone = true) {return pNode->acceptAppendChild(this, bDeepClone, bClone);}
		const bool removeChild(IXmlNode *pChild) {return pChild->acceptRemoveChild(this);}
		IXmlNode *clone(const bool bDeep, IXmlNode *pToNode = 0) const;
		const char *value(const char *sValue);
		const char *name(const char *sName);

		//query
		IXmlDoc *transform(const IXmlNode *pXSLDoc, const map<const char*, const size_t> *pParamsInt = 0, const map<const char*, const char*> *pParamsChar = 0, const char *sWorkingDirectory = 0) const {return pXSLDoc->acceptTransform(this, pParamsInt, pParamsChar, sWorkingDirectory);}
	};
}
#endif
