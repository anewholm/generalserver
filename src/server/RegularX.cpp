//platform agnostic file
#include "RegularX.h"

#include "IXml/IXmlBaseDoc.h"
#include "IXml/IXmlBaseNode.h"
#include "IXml/IXmlNamespace.h"
#include "IXml/IXmlQueryEnvironment.h"
#include "Xml/XmlAdminQueryEnvironment.h"

#include "Xml/XmlNodeList.h"
#include "Utilities/container.c"

//GRETA: http://easyethical.org/opensource/spider/regexp%20c++/greta2.htm
using namespace regex;
using namespace std;

namespace general_server {
  IXmlBaseNode *RegularX::rx(const IXmlQueryEnvironment *pQE, const IXmlBaseNode *pInputNode, const char *sTextStream, IXmlBaseNode *pDestinationNode, match_results_c *pmResults, const bool bThrowOnSingleOutputFail) {
    //caller frees result
    //pDestinationNode e.g. <repository:requests>
    //  it is IMPORTANT to get the created node, NOT the firstChild of pDestinationNode
    assert(pInputNode);

    RegularX rx;
    IFDEBUG_RX(
      const char *sName = pInputNode->attributeValue(pQE, "name");
      cout << "\n-------------------------------------------- new RegularX transform [" << (sName ? sName : "no @name") << "]\n";
      cout << "input: [" << sTextStream << "]\n";
      MM_FREE(sName);
    );
    IXmlBaseNode *pFirstOutputChild = rx.recurseNode(pQE, pInputNode, pDestinationNode, sTextStream, pmResults, bThrowOnSingleOutputFail);
    IFDEBUG_RX(if (pFirstOutputChild) {
      const XmlAdminQueryEnvironment rdo(this, pFirstOutputChild->document());
      const char *sXML = pFirstOutputChild->xml(pQE);
      cout << "RegularX result: \n" << sXML << "\n\n";
      MM_FREE(sXML);
    });
    return pFirstOutputChild;
  }

  RegularX::~RegularX() {
    m_directives.clear();
  }

  IXmlBaseNode *RegularX::recurseNode(const IXmlQueryEnvironment *pQE, const IXmlBaseNode *pInputNode, IXmlBaseNode *pOutputNode, const char *sTextStream, match_results_c *pmResults, const bool bThrowOnSingleOutputFail) {
    //caller frees result
    //pInputNode:  current node to copy and process under the current pOutputNode
    //pOutputNode: destination for processed pInputNode
    //sTextStream: incoming string
    //pmResults:   current results match
    //returns:     the first and singular newly created output node

    const bool bWriteBackEnable = false,
               bIsAttribute = pInputNode->isNodeAttribute();
    bool bAttributeMultiType, bAllowNamespace;
    const IXmlBaseNode *pInputSettingsNode   = 0, //for settings in the case of an attribute
                       *pInputNodeFirstChild = 0; //just to see what's underneath
    IXmlBaseNode *pNewOutputChild            = 0, //only if newly created
                 *pCurrentOutputChild        = 0, //can be the current output node, or the newly created one
                 *pNewChildrenOutputNode     = 0, //possible pNewOutputChild returned from EACH recursion
                 *pFirstNewChildrenOutputNode = 0, //first pNewOutputChild returned from the recursion
                 *pReturnNode                = 0; //first newly created node from somewhere in the recursion
    const IXmlNamespaced *pInputNodeType;
    XmlNodeList<const IXmlBaseNode> *aChildNodes      = 0; //children + attributes of input node
    XmlNodeList<const IXmlBaseNode>::iterator iNode;
    const char *sLocalName                   = 0,
               *sRegularExpression           = 0,
               *sRegularExpressionAttribute  = 0,
               *sRegularExpressionNotAttribute = 0,
               *sMultiType                   = 0,
               *sDecode                      = 0,
               *sDirectiveNameAttribute      = 0,
               *sBackRef                     = 0, //pointer in to another string: not freed
               *sNewTextStream               = 0, //can be a new malloc used by rx:scope
               *sEndPosition                 = 0, //end of sTextStream
               *sPosition                    = 0, //used by rx:multi as it traverses the matches
               *sTagName                     = 0, //used only and freed in the multi loop
               *sLeftSide                    = 0; //used only and freed in the multi loop
    char       *sAttributeValueStartName     = 0;
    const char *sMatch                       = 0; //general use and freed in the multi loop
    size_t iTagNameLen;
    bool bSINGLELINE = false, bMULTILINE = true, bNORMALIZE = true;    //default flags
    REGEX_FLAGS iFlags;

    match_results_c mResults, mNotResults;
    int iBackRefs, iBackRef;
    rpattern_c *pRgxNot;
    rpattern_c::backref_type br;
    rpattern_c::backref_type brO;
    rpattern_c::backref_type brNotMatch;

    //common ALL attributes
    sLocalName     = pInputNode->localName(NO_DUPLICATE);
    sEndPosition   = sTextStream + strlen(sTextStream);
    sNewTextStream = sTextStream;
    if (bIsAttribute) pInputSettingsNode = pInputNode->parentNode(pQE, "regularXSL");
    else              pInputSettingsNode = pInputNode;
    sDecode        = pInputSettingsNode->attributeValue(pQE, "decode", NAMESPACE_RX);
    bSINGLELINE    = pInputSettingsNode->attributeValueBoolDynamicString(pQE, "singleline", NAMESPACE_RX, false);
    bMULTILINE     = pInputSettingsNode->attributeValueBoolDynamicString(pQE, "multiline",  NAMESPACE_RX, true);
    bNORMALIZE     = pInputSettingsNode->attributeValueBoolDynamicString(pQE, "normalize",  NAMESPACE_RX, true);
    iFlags         = (bSINGLELINE ? SINGLELINE : NOFLAGS) |
                     (bMULTILINE  ? MULTILINE  : NOFLAGS) |
                     (bNORMALIZE  ? NORMALIZE  : NOFLAGS);

    //const namespace HREF comparison, no reconcile default needed
    pInputNodeType  = pInputNode->queryInterface((const IXmlNamespaced*) 0);
    //IFDEBUG_RX(cout << "------ pInputNode: " << pInputNode->toString() << "\n";)

    //process by namespace of input node
    if (pInputNodeType->isNamespace(NAMESPACE_RX)) { //const namespace HREF comparison, no reconcile default needed
      //------------------------------------------------------- RegularX namespace
      //these DO NOT copy the input node,
      //  they do things to the context
      //  or create custom nodes based on the rx:directive
      //common RegularX attributes
      sRegularExpressionAttribute    = pInputNode->attributeValue(pQE, "regex",     NAMESPACE_RX);     //not always evaluated (freed afterwards)
      sRegularExpressionNotAttribute = pInputNode->attributeValue(pQE, "regex-not", NAMESPACE_RX); //not always evaluated (freed afterwards)
      sDirectiveNameAttribute        = pInputNode->attributeValue(pQE, "name",      NAMESPACE_RX);
      pRgxNot = 0;

      if (!strncmp(sLocalName, "scope", 5)) {
        //------------------ scope: limit the text stream to a subset for this area
        sNewTextStream = 0; //will cause the "" blank string sNewTextStream below by default
        if (!sRegularExpressionAttribute || !*sRegularExpressionAttribute) {
          IFDEBUG_RX(cout << "warning: missing or blank @rx:regex on rx:scope\n";)
        } else if (*sRegularExpressionAttribute == '$') {
          //shorthand for backreference to current default distributed
          if (pmResults) {
            sBackRef = sRegularExpressionAttribute+1;
            iBackRef = atoi(sBackRef);
            br       = pmResults->backref(iBackRef);
            //set the new text stream for this area
            sNewTextStream = copyString(br.first, br.second);
          }
        } else {
          //assume new regex based on currentText (set by last scope)
          //using the typedefs *_c for TCHAR c strings
          rpattern_c rgx1(sRegularExpressionAttribute, iFlags, MODE_DEFAULT); //compile the regex
          rgx1.match( sTextStream, mResults );
          if ((int) mResults.cbackrefs() == 2) {
            br = mResults.backref(1);
            //set the new text stream for this area
            if (br.first && br.second) sNewTextStream = copyString(br.first, br.second);
          } else {
            IFDEBUG_RX(cout << "warning: multiple backrefs from rx:scope\n";)
          }
        }
        //if scope fails for some reason then run children on no input
        if (!sNewTextStream) sNewTextStream = MM_STRDUP("");
        IFDEBUG_RX(
          cout << "------ rx:scope [" << (sRegularExpressionAttribute ? sRegularExpressionAttribute : "(missing @rx:regex)") << "]\n";
          cout << "new stream: [" << sNewTextStream << "]\n";
        );

        //recurse child nodes with new current text stream
        //continue outputting to same place by default
        pCurrentOutputChild = pOutputNode;


      } else if (!strncmp(sLocalName, "attribute", 9)) {
        //------------------ attribute:
        sTagName = pInputNode->attributeValue(pQE, "name");
        NOT_COMPLETE("rx:attribute");
        pOutputNode->setAttribute(pQE, sTagName, "not complete");


      } else if (!strncmp(sLocalName, "distributed", 11) && sRegularExpressionAttribute) {
        //------------------ distributed: named regex with submatches. Set it as the current default for this area
        rpattern_c rgx1(sRegularExpressionAttribute, iFlags, MODE_DEFAULT); //compile the regex
        pmResults = new match_results_c;
        m_directives.insert(MM_STRDUP(sDirectiveNameAttribute), pmResults);
        rgx1.match( sTextStream, *pmResults );



      } else if (!strncmp(sLocalName, "value-of", 8)) {
        //------------------ value-of:
        NOT_COMPLETE("");



      } else if (!strncmp(sLocalName, "multi", 5) && sRegularExpressionAttribute) {
        //------------------ multi: multiple matches with names for the tags
        IFDEBUG_RX(cout << "------ rx:multi [" << sRegularExpressionAttribute << "]\n";)
        sMultiType          = pInputNode->attributeValue(pQE, "type", NAMESPACE_RX);
        bAttributeMultiType = _STREQUAL(sMultiType, "attribute");
        bAllowNamespace     = pInputNode->attributeValueBoolInterpret(pQE, "allow-namespace", NAMESPACE_RX, ALLOW_NAMESPACE);
        sPosition           = sTextStream;
        //compile regexs
        rpattern_c rgx1(sRegularExpressionAttribute, iFlags, MODE_DEFAULT);    
        if (sRegularExpressionNotAttribute) pRgxNot = new rpattern_c(sRegularExpressionNotAttribute, iFlags, MODE_DEFAULT); //compile the regex

        //loop through results
        brO = rgx1.match(sPosition, mResults);
        while (sPosition < sEndPosition && brO.matched && mResults.cbackrefs() > 1) {
          if (pRgxNot) brNotMatch = pRgxNot->match(brO.first, mNotResults);
          if (pRgxNot && brNotMatch.matched) {
            //rx:regex-not matched, so skip the result
            br = brO;
          } else {
            if (mResults.cbackrefs() == 3) {
              //we have a match with 2 backrefs, 1st is the tagname, 2nd is the value
              br        = mResults.backref(1);
              sLeftSide = copyString(br.first, br.second, sDecode);
              br        = mResults.backref(2);
              sMatch    = copyString(br.first, br.second, sDecode);
            } else {
              //1 backref only: @name is the tagname, 1st is the value
              br        = mResults.backref(1);
              sLeftSide = MM_STRDUP(sDirectiveNameAttribute);
              sMatch    = copyString(br.first, br.second, sDecode);
            }

            //replace supurious characters in the tagname and add node
            //allow namespaces
            sTagName = pQE->xmlLibrary()->xml_element_name(sLeftSide, bAllowNamespace); //borrowing the worker function here for compatibility
            try {
              if (sTagName) {
                //create new node for each multi
                if (bAttributeMultiType) {
                  if (pOutputNode->attributeExists(pQE, sTagName)) {
                    if (bWriteBackEnable) {IFDEBUG_RX(cout << "  rx:multi tag attribute @" << sTagName << " already exists, appending value, write back will not currently work\n")}
                    pOutputNode->appendAttribute(pQE, sTagName, sMatch);
                  } else {
                    pOutputNode->setAttribute(pQE, sTagName, sMatch);
                    if (bWriteBackEnable) {
                      iTagNameLen              = 24 + strlen(sTagName) + 1;
                      sAttributeValueStartName = MMO_MALLOC(iTagNameLen);
                      snprintf(sAttributeValueStartName, iTagNameLen, "rx:relative-value-%s-start", sTagName);
                      pOutputNode->setAttribute(pQE, sAttributeValueStartName, (size_t) (br.first  - sNewTextStream));
                      snprintf(sAttributeValueStartName, iTagNameLen, "rx:relative-value-%s-end", sTagName);
                      pOutputNode->setAttribute(pQE, sAttributeValueStartName, (size_t) (br.second - sNewTextStream));
                    }
                  }
                  IFDEBUG_RX(cout << "  rx:multi tag @" << sTagName << "=" << sMatch << "\n";)
                } else {
                  pNewOutputChild = pOutputNode->createChildElement(pQE, sTagName, NULL, NULL, false, "RegularXSL");
                  pNewOutputChild->value(pQE, sMatch);
                  if (bWriteBackEnable) pNewOutputChild->setAttribute(pQE, "relative-value-start", (size_t) (br.first  - sNewTextStream), NAMESPACE_RX);
                  if (bWriteBackEnable) pNewOutputChild->setAttribute(pQE, "relative-value-end",   (size_t) (br.second - sNewTextStream), NAMESPACE_RX);
                  IFDEBUG_RX(cout << "  rx:multi tag <" << sTagName << ">" << sMatch << "</" << sTagName << ">\n";)
                }
              }
            } 
            catch (CannotReconcileNamespacePrefix &eb) {
              IFDEBUG_RX(cout << "  rx:multi tag <" << sTagName << ">" << sMatch << "</" << sTagName << "> FAILED because CannotReconcileNamespacePrefix()\n";)
              IFDEBUG(if (!bThrowOnSingleOutputFail) Debug::reportObject(&eb);)
              if (bThrowOnSingleOutputFail) throw;
            }
            catch (ExceptionBase &eb) {
              IFDEBUG_RX(cout << "  rx:multi tag <" << sTagName << ">" << sMatch << "</" << sTagName << "> FAILED\n");
              IFDEBUG(if (!bThrowOnSingleOutputFail) Debug::reportObject(&eb);)
              if (bThrowOnSingleOutputFail) throw;
            }

            //free in loop for re-use
            //NOTE: rx:multi CANNOT create the root node for return
            if (pNewOutputChild) {delete pNewOutputChild; pNewOutputChild = 0;}
            if (sMatch)          {MMO_FREE(sMatch);     sMatch          = 0;}
            if (sAttributeValueStartName) {MMO_FREE(sAttributeValueStartName); sAttributeValueStartName = 0;}
            if (sTagName)        {MMO_FREE(sTagName);   sTagName        = 0;}
            if (sLeftSide)       {MMO_FREE(sLeftSide);  sLeftSide       = 0;}
          }

          //next
          sPosition = br.second;
          if (sPosition < sEndPosition) brO = rgx1.match(sPosition, mResults);
        }
        if (pRgxNot) delete pRgxNot;
      }
    } else if (pInputNodeType->isNamespace(NAMESPACE_REPOSITORY)) { //const namespace HREF comparison, no reconcile default needed
      //ignore these
    } else if (pInputNodeType->isNamespace(NAMESPACE_XML)) {
      //ignore xml:id
    } else {



      //------------------------------------------------------- default / other namespace
      //other namespace: copy current input node and then set it's value(s)
      //pNewOutputChild will be the new attribute in the new document also if appended
      //attribute aware appending...
      pInputNodeFirstChild = pInputNode->firstChild(pQE, node_type_element_only, NULL, "RegularXSL"); //see if we have one regular expression style sub-node

      if (!pInputNodeFirstChild) {
        //no XML_NODE_ELEMENT so let's check the text content
        if (sRegularExpression) MM_FREE(sRegularExpression);
        if (sRegularExpression = pInputNode->value(pQE)) {
          if (*sRegularExpression == '$') {
            //----------------- standard regex or back reference
            //shorthand for backreference to current default distributed
            if (pmResults) {
              sBackRef = sRegularExpression+1;
              iBackRef = atoi(sBackRef);
              br       = pmResults->backref(iBackRef);
              if (br.matched) sMatch = copyString(br.first, br.second, sDecode);
            }
          } else if (*sRegularExpression == '=') {
            //----------------- direct text copy (after =)
            sMatch = MM_STRDUP(sRegularExpression + 1);
          } else if (strisspace(sRegularExpression)) {
            //----------------- ignore whitespace text nodes
          } else if (*sRegularExpression != 0) {
            //----------------- new TEXT regex
            //assume new regex based on currentText (set by last scope)
            //using the typedefs *_c for TCHAR c strings
            rpattern_c rgx1(sRegularExpression, iFlags, MODE_DEFAULT); //compile the regex
            rgx1.match( sTextStream, mResults );
            iBackRefs = (int) mResults.cbackrefs();

            if (iBackRefs > 1) {
              br = mResults.backref(1);
              if (br.matched) sMatch = copyString(br.first, br.second, sDecode);
            }
          }
        }
      }

      //write non-whitespace TEXT nodes and elements
      if (pInputNode->isNodeElement() || sMatch) {
        pNewOutputChild      = pOutputNode->copyChild(pQE, pInputNode, SHALLOW_CLONE);
        pCurrentOutputChild  = pNewOutputChild;
        if (sMatch) {
          pNewOutputChild->value(pQE, sMatch);
          if (bWriteBackEnable) pNewOutputChild->setAttribute(pQE, "relative-value-start", (size_t) (br.first  - sNewTextStream), NAMESPACE_RX);
          if (bWriteBackEnable) pNewOutputChild->setAttribute(pQE, "relative-value-end",   (size_t) (br.second - sNewTextStream), NAMESPACE_RX);
        }
      }

      //free ALL DEFAULT namespace attributes
      if (sRegularExpression) {MM_FREE(sRegularExpression);sRegularExpression = 0;}
    }

    //IFDEBUG_RX(if (pNewOutputChild) cout << "------ pNewOutputChild: " << pNewOutputChild->toString() << "\n";)

    //navigate down to the next RegularX command (pInputNode) or node to copy output
    if (pCurrentOutputChild && pInputNode->isNodeElement()) {
      //recursing all children::* and attribute::*
      aChildNodes = pInputNode->children(pQE, "RegularXSL");
      for (iNode = aChildNodes->begin(); iNode != aChildNodes->end(); iNode++) {
        //this returns the first created child output node,
        //for consideration as the document root
        //because the top RegularX command might be a scope and thus not create an immediate root node
        //all the other siblings must be deleted if non-zero
        if (pNewChildrenOutputNode = recurseNode(pQE, *iNode, pCurrentOutputChild, sNewTextStream, pmResults, bThrowOnSingleOutputFail)) {
          if (pFirstNewChildrenOutputNode) delete pNewChildrenOutputNode;
          else                             pFirstNewChildrenOutputNode = pNewChildrenOutputNode;
        }
      }
      
      //attributes if not rx:*
      if (!pInputNodeType->isNamespace(NAMESPACE_RX)) {
        vector_element_destroy(aChildNodes);
        aChildNodes = pInputNode->attributes(pQE);
        for (iNode = aChildNodes->begin(); iNode != aChildNodes->end(); iNode++) {
          if (pNewChildrenOutputNode = recurseNode(pQE, *iNode, pCurrentOutputChild, sNewTextStream, pmResults, bThrowOnSingleOutputFail))
            delete pNewChildrenOutputNode;
        }
      }
    }

    //we want to return the first created root output node
    //BUT: the first input node might be an rx:scope and thus not have created an output node
    //return the newly created node, or the newly created node from the recursion
    //is potentially zero if none of the recursive output created a new output node
    pReturnNode = (pNewOutputChild ? pNewOutputChild : pFirstNewChildrenOutputNode);

    //clear up
    //if (pInputNode)             delete pInputNode;             //do not delete the callers input
    //if (pOutputNode)            delete pOutputNode;            //do not delete the callers input
    if (pInputNodeFirstChild)   delete pInputNodeFirstChild;     //firstChild()
    if (pInputSettingsNode != pInputNode) delete pInputSettingsNode;
    if (pNewOutputChild != pReturnNode)   delete pNewOutputChild;  //potentially returned
    //if (pCurrentOutputChild)    delete pCurrentOutputChild;    //alias for which node to pass to children recursion
    //if (pNewChildrenOutputNode) delete pNewChildrenOutputNode; //already deleted
    if (pFirstNewChildrenOutputNode != pReturnNode) delete pFirstNewChildrenOutputNode; //potentially returned
    //if (pReturnNode)            delete pReturnNode;            //alias for return node
    if (aChildNodes)        vector_element_destroy(aChildNodes);
    //if (sLocalName)         MM_FREE(sLocalName); //NO_DUPLICATE
    if (sRegularExpression) MM_FREE(sRegularExpression);
    if (sRegularExpressionAttribute) MM_FREE(sRegularExpressionAttribute);
    if (sRegularExpressionNotAttribute) MM_FREE(sRegularExpressionNotAttribute);
    if (sMultiType)         MM_FREE(sMultiType);
    if (sDecode)            MM_FREE(sDecode);
    if (sDirectiveNameAttribute)     MM_FREE(sDirectiveNameAttribute);
    //if (sBackRef)           MM_FREE(sBackRef);  //pointer in to another string: not freed
    if (sNewTextStream != sTextStream) MM_FREE(sNewTextStream);
    //if (sEndPosition)       MM_FREE(sEndPosition);  //pointer in to another string: not freed
    //if (sPosition)          MM_FREE(sPosition);  //pointer in to another string: not freed
    if (sTagName)           MM_FREE(sTagName);    //used only and freed in the multi loop
    if (sLeftSide)          MM_FREE(sLeftSide);   //used only and freed in the multi loop
    if (sMatch)             MM_FREE(sMatch);

    return pReturnNode;
  }

  RegularX::decode RegularX::decodeType(const char *sString) {
    if (sString && !strncmp(sString, "URL", 3)) return URL;
    return none;
  }

  char *RegularX::copyString(const char *pStart, const char *pFinish, const char *sDecode) {
    //caller freed copy of string
    //inline
    char *sMatch = 0;
    decode iDecode = decodeType(sDecode);
    if (pStart && pFinish) {
      //allocate same amount of space: decoding must be equal or reductive
      const char *pPosSource = pStart;
      char *pPosDest = sMatch = MMO_MALLOC((int) (pFinish - pStart) + 1);
      char c;
      unsigned int ii;
      while (pPosSource < pFinish) {
        c = *pPosSource++;
        switch (iDecode) {
          case URL: {
            switch (c) {
              case '%': {
                if (*pPosSource) {
                  sscanf(pPosSource, "%2x", &ii);
                  c = static_cast<char>(ii);
                  pPosSource += 2;
                }
                break;
              }
              case '+': {c = ' ';break;}
            }
            break;
          }
          default: {}
        }
        *pPosDest++ = c;
      }
      *pPosDest = 0;
    } else sMatch = MM_STRDUP(""); //silent fail
    return sMatch;
  }

  const char *RegularX::match(const char *sRegularExpression, const char *sInput) {
    return match(sRegularExpression, sInput, MULTILINE | NORMALIZE);
  }

  const char *RegularX::match(const char *sRegularExpression, const char *sInput, REGEX_FLAGS flags) {
    //caller frees results
    const char *sMatch = 0;
    rpattern_c rgx1(sRegularExpression, flags, regex::MODE_DEFAULT);
    match_results_c mResults;
    rpattern_c::backref_type br = rgx1.match(sInput, mResults);
    if (br.matched) {
      if (mResults.cbackrefs() == 2) {
        br = mResults.backref(1);
        if (br.first && br.second) sMatch = copyString(br.first, br.second);
      } else sMatch = MM_STRDUP(sInput);
    }
    return sMatch;
  }
}
