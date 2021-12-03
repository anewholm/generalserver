//platform agnostic file
#include "RXSL.h"
using namespace regex;

//platform specific project headers
#include "debug.h"
using namespace std;

namespace goofy {
	Rxsl::~Rxsl() {
		StringMap<match_results_c*>::iterator iDirective;
		for (iDirective=m_directives.begin();iDirective!=m_directives.end();iDirective++) delete iDirective->second;
		vector<char*>::iterator iScopeText;
		for (iScopeText=m_vScopeTexts.begin();iScopeText!=m_vScopeTexts.end();iScopeText++) free((void*) *iScopeText);
	}
	const XmlDoc *Rxsl::process(const XmlDoc *pRxsl, const char *sTextStream) {
		//caller frees the XmlDoc
		DEBUGPRINT("Rxsl::rxsl", DEBUG_FUNCTIONSTART);
		assert(sTextStream!=0&&pRxsl!=0);
		DEBUGPRINT1("input text stream:\n%s\n\n", DEBUG_LINE, sTextStream);
		DEBUGPRINT1("input rxsl doc:\n%s\n\n", DEBUG_LINE, pRxsl->xml());
		XmlDoc *pNewDoc=new XmlDoc();
		//recurse all the nodes looking for rxsl namespaces and text values
		XmlNode *pInputNode=pRxsl->rootNode_callerFree();
		XmlNode *pOutputNode=pNewDoc->appendChild_serverFree(pInputNode);
		recurseNode(pInputNode, pOutputNode, sTextStream, NULL);
		delete pInputNode;
#ifdef _DEBUG
		char *sXML=pNewDoc->xml();
		DEBUGPRINT1("pNewDoc:\n%s\n\n", DEBUG_LINE, sXML);
		free(sXML);
#endif
		DEBUGPRINT("Rxsl::rxsl", DEBUG_FUNCTIONEND);
		return pNewDoc;
	}
	void Rxsl::recurseNode(XmlNode *pInputNode, XmlNode *pOutputNode, const char *sTextStream, match_results_c *pmResults) {
		XmlNode *pChildNode=0, *pFirstChild=0, *pNewOutputChild=0;
		XmlNodeList *aChildNodes=pInputNode->getMultipleNodes("*"); //children::*
		XmlNodeList::iterator iNode;
		const char *sName=0, *sRegularExpression=0, *sDirective=0;
		char *sMatch=0;
		int iBackRefs;
		bool bIsRXSLNamspace;
		for (iNode=aChildNodes->begin();iNode!=aChildNodes->end();iNode++) {
			pChildNode=*iNode;
			sName=pChildNode->name();
			sRegularExpression=pChildNode->value(); //not always evaluated (freed afterwards)
			bIsRXSLNamspace=(strncmp(sName, "rxsl:", 5)==0);
			DEBUGPRINT2("found node [%s] with regex [%s]", DEBUG_BLOCKSTART, sName, sRegularExpression);
			if (bIsRXSLNamspace) {
//------------------------------------------------------- RXSL namespace
				sDirective=sName+5;
				DEBUGPRINT1("we have an rxsl directive [%s]", DEBUG_BLOCKSTART, sDirective);
				if (!strncmp(sDirective, "scope", 5)) {
					const char *sNewTextStream=sTextStream;
					//------------------ scope: limit the text stream to a subset for this area
					pFirstChild=pChildNode->firstChild_callerFree();
					sRegularExpression=pFirstChild->value(); //not always evaluated (freed afterwards)
					if (*sRegularExpression==0) DEBUGPRINT("no content", DEBUG_LINE);
					else if (*sRegularExpression=='$') {
						//shorthand for backreference to current default distributed
						if (pmResults) {
							const char *sBackRef=sRegularExpression+1;
							int iBackRef=atoi(sBackRef);
							const rpattern_c::backref_type br=pmResults->backref(iBackRef);
							sMatch=copyString(br.first, br.second);
							sNewTextStream=sMatch; //set the new text stream for this area
							DEBUGPRINT1("Backref: %s", DEBUG_LINE, sMatch);
							m_vScopeTexts.push_back(sMatch); //to free at destruct
						} else DEBUGPRINT("no current distributed!", DEBUG_LINE);
					} else {
						//assume new regex based on currentText (set by last scope)
						//using the typedefs *_c for TCHAR c strings
						rpattern_c rgx1(sRegularExpression, NOFLAGS, MODE_DEFAULT); //compile the regex
						match_results_c mResults;
						rgx1.match( sTextStream, mResults );
						if ((int) mResults.cbackrefs()==2) {
							const rpattern_c::backref_type br=mResults.backref(1);
							if (br.first&&br.second) {
								sMatch=copyString(br.first, br.second);
								sNewTextStream=sMatch;
								DEBUGPRINT1("Backref: %s", DEBUG_LINE, sMatch);
								m_vScopeTexts.push_back(sMatch); //to free
							}
						} else {
							DEBUGPRINT("no capture groups.", DEBUG_LINE);
						}
					}
					delete pFirstChild;
					//----------------- recurse child nodes
					recurseNode(pChildNode, pOutputNode, sNewTextStream, pmResults);
				} else if (!strncmp(sDirective, "distributed", 11)) {
					//------------------ distributed: named regex with submatches. Set it as the current default for this area
					const char *sDirectiveName=pChildNode->attributeValue("name");
					rpattern_c rgx1(sRegularExpression, NOFLAGS, MODE_DEFAULT); //compile the regex
					pmResults=new match_results_c;
					m_directives.insert(_PAIR_DIRECTIVE(sDirectiveName, pmResults));
					free((void*) sDirectiveName);
					rgx1.match( sTextStream, *pmResults );
#ifdef _DEBUG
					for (int i=1;i<(int) pmResults->cbackrefs();i++) {
						const rpattern_c::backref_type br=pmResults->backref(i);
						sMatch=copyString(br.first, br.second);
						DEBUGPRINT1("Backref: %s", DEBUG_LINE, sMatch);
						free(sMatch);
					}
#endif
				} else if (!strncmp(sDirective, "value-of", 8)) {
					//------------------ value-of:
				} else if (!strncmp(sDirective, "multi", 5)) {
					//------------------ multi: multiple matches with names for the tags
					char *sTageName=0;
					const char *sMultiName=pChildNode->attributeValue("name");
					XmlNode *pRegExNode=pChildNode->firstChild_callerFree();
					XmlNode *pNewNode=0;
					const char *sRegularExpression=pRegExNode->value();
					rpattern_c rgx1(sRegularExpression, regex::GLOBAL, MODE_DEFAULT); //compile the regex
					match_results_c mResults;
					rpattern_c::backref_type br;
					rgx1.match( sTextStream, mResults );
					while (mResults.cbackrefs()>1) {
						DEBUGPRINT1("backref count:%i", DEBUG_LINE, mResults.cbackrefs());
						if (mResults.cbackrefs()==3) {
							//we have a match with 2 backrefs, 1st is the tagname, 2nd is the value
							const rpattern_c::backref_type br1=mResults.backref(1);
							sTageName=copyString(br1.first, br1.second);
							br=mResults.backref(2);
							sMatch=copyString(br.first, br.second);
						} else {
							br=mResults.backref(1);
							sMatch=copyString(br.first, br.second);
							sTageName=_STRDUP(sMultiName);
						}
						//replace supurious characters in the tagname
						for (char *sP=sTageName;*sP;sP++) if (*sP<65||*sP>122||(*sP>90&&*sP<97)) *sP=97;
						DEBUGPRINT3("new tag: <%s>%s</%s>", DEBUG_LINE, sTageName, sMatch, sTageName);
						//create new node
						pNewNode=new XmlNode(sTageName, pOutputNode->m_pDoc);
						pNewNode->value(sMatch);
						pOutputNode->appendChild_serverFree(pNewNode, true, false); //no clone
						delete pNewNode;
						free(sMatch);
						free(sTageName);
						rgx1.match( br.second+1, mResults );
					}
					delete pRegExNode;
					free((void*) sRegularExpression);
					free((void*) sMultiName);
				}
				DEBUGPRINT("we have an rxsl directive", DEBUG_BLOCKEND);
			} else {

//------------------------------------------------------- default namespace
				DEBUGPRINT("Default namespace", DEBUG_LINE);
				pNewOutputChild=pOutputNode->appendChild_serverFree(pChildNode);
				pFirstChild=pChildNode->firstChild_callerFree();
				if (pFirstChild->isNodeText()) {
					sRegularExpression=pFirstChild->value();
					//----------------- standard regex or back reference
					if (*sRegularExpression==0) DEBUGPRINT("no content", DEBUG_LINE);
					else if (*sRegularExpression=='$') {
						//shorthand for backreference to current default distributed
						if (pmResults) {
							const char *sBackRef=sRegularExpression+1;
							int iBackRef=atoi(sBackRef);
							const rpattern_c::backref_type br=pmResults->backref(iBackRef);
							if (br.matched) {
								sMatch=copyString(br.first, br.second);
								DEBUGPRINT1("Backref: %s", DEBUG_LINE, sMatch);
								pNewOutputChild->value(sMatch);
								free(sMatch);
							} else DEBUGPRINT("no match", DEBUG_LINE);
						} else DEBUGPRINT("no current distributed!", DEBUG_LINE);
					} else {
						//assume new regex based on currentText (set by last scope)
						//using the typedefs *_c for TCHAR c strings
						rpattern_c rgx1(sRegularExpression, regex::GLOBAL|regex::MULTILINE, regex::MODE_DEFAULT); //compile the regex
						match_results_c mResults;
						rgx1.match( sTextStream, mResults );
						iBackRefs=(int) mResults.cbackrefs();
						DEBUGPRINT1("backref count:%i", DEBUG_LINE, iBackRefs);
						if (iBackRefs) {
							const rpattern_c::backref_type br=mResults.backref(1);
							if (br.matched) {
								sMatch=copyString(br.first, br.second);
								pNewOutputChild->value(sMatch);
								DEBUGPRINT1("Backref: %s", DEBUG_LINE, sMatch);
								free(sMatch);
							} else DEBUGPRINT("no match", DEBUG_LINE);
						} else {
							DEBUGPRINT("no capture groups.", DEBUG_LINE);
						}
					}
				}
				delete pFirstChild;
				//----------------- recurse child nodes
				recurseNode(pChildNode, pNewOutputChild, sTextStream, pmResults);
			}
			//free up
			free((void*) sName);
			free((void*) sRegularExpression);
			DEBUGPRINT("found node [%s] with text [%s]", DEBUG_BLOCKEND);
		}
		delete aChildNodes;
	}
}
