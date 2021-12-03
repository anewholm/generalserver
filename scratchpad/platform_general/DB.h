//platform agnostic file
#ifndef _DB_H
#define _DB_H

#include "xml.h" //platform specific wrapper for XML Library (compile different on in for different platforms)
#include "StringMap.h"
#include "index.h" //external Database indexes
#include <map>

#define _PAIR_STYLESHEET pair<const char*, XmlDoc*>
#define _PAIR_COMMANDFILE pair<const char*, XmlDoc*>
#define _PAIR_NODETRANSFORMS pair<const char*, StringMap<XmlDoc*>* >
#define _PAIR_STAGETRANSFORMS pair<const char*, XmlDoc*>

namespace goofy {
	class DB {
		//XML Database class wraps the XmlDoc
		//provides the SQL style select language to access the XmlDoc
		//and external indexes
		//and associated SPROCs (named XSL associations)
	private:
		const bool m_bReleaseXmlDoc;
		bool m_bReadOnly;
		const char *m_sAlias;
		XmlDoc *m_pDoc; //platform specific wrapper for the XML library

		StringMap<XmlDoc*> m_mStyleSheets;
		StringMap<XmlDoc*> m_mCommandFiles; 
		StringMap<StringMap<XmlDoc*>* > m_mTransforms; //SPROCs
		StringMap<Index*> m_mIndexes; //external indexes access to nodes, see index.h

	public:
		DB(const char *sAlias, XmlDoc *pDoc, const bool bReleaseXmlDoc = true);
		DB(const char *sAlias, const XmlDoc *pDoc, const bool bReleaseXmlDoc = true);
		~DB();

		const bool loadedAndParsed() const {return m_pDoc->loadedAndParsed();}
		void addStyleSheet(_PAIR_STYLESHEET pPair) {m_mStyleSheets.insert(pPair);}
		void addCommandFile(_PAIR_COMMANDFILE pPair) {m_mCommandFiles.insert(pPair);}
		Index *createIndex(const char *sIXAlias, const char *sType, const char *sXPath="");
		const int associateTransformsToNodes(const char *sRef, const char *sXPathToNodes, const char *sSSAlias);
		XmlDoc *findXSLDoc(const char *sSSAlias) const;
		XmlDoc *findCommandFile(const char *sCFAlias) const;
		const string *serialise() const;

		//commands
		HRESULT select(const char *sContext, const char *sWhat, const XmlDoc **ppResultDoc, const bool bDeepCloneXPath=false) const;
		HRESULT remove(const char *sContext, const char *sWhat, const XmlDoc **ppResultDoc);
		HRESULT insert(const char *sContext, const char *sWhat, const XmlDoc **ppResultDoc);
		HRESULT update(const char *sContext, const char *sWhat, const char *sTo, const XmlDoc **ppResultDoc);
		//HRESULT addStyleSheet(const char *sSSAlias, const char *sFilepath);
		//HRESULT addIndex(const char *sIDXAlias, const char *sType, const char *sXPath="");
	};
}

#endif
