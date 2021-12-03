//platform agnostic file
#include "LibXslNode.h"

//#include "LibXmlBase.cpp" is at the bottom
#include "LibXmlDoc.h"
#include "LibXmlNode.h"
#include "Repository.h"
#include "Debug.h"

#include "Utilities/container.c"

//ls /usr/include/libxml2/libxml/*.h, http://xmlsoft.org/, file:///usr/local/share/doc/libxml2-2.6.22/html/tutorial/apc.html
#include <libxslt/xslt.h>
#include <libxslt/xsltInternals.h>
#include <libxml/xpathInternals.h>
#include <libxml/xmlsave.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>

//http://www.opensource.apple.com/source/WebCore/WebCore-855.7/xml/XSLTExtensions.cpp
#include <libxslt/extensions.h>
#include <libxslt/extra.h>
#include <libxslt/templates.h>
//#include "XSLTExtensions.h"

using namespace std;

namespace general_server {
  //----------------------------------------------------------------------------------------------------------
  //------------------------------------------- LibXslCommandNode --------------------------------------------
  //----------------------------------------------------------------------------------------------------------
  bool LibXslCommandNode::gimme(const xmlNodePtr oNode) {
    assert(oNode);

    return (oNode->ns
      && oNode->type == XML_ELEMENT_NODE
      && isCommandNamespace((const char*) oNode->ns->href)
    );
  }

  IXmlBaseNode *LibXslCommandNode::clone_wrapper_only() const {return base_object_clone<LibXslCommandNode>(this);}
  IXmlBaseNode *LibXslCommandNode::clone_with_resources() const {return new LibXslCommandNode(*this);}

  //----------------------------------------------------------------------------------------------------------
  //------------------------------------------- LibXslNode ---------------------------------------------------
  //----------------------------------------------------------------------------------------------------------
  StringMap<IXslDoc*> LibXslNode::ms_mCachedStylesheets;
  vector<IXslDoc*>    LibXslNode::ms_vRedundantStylesheets;

  bool LibXslNode::gimme(const xmlNodePtr oNode) {
    assert(oNode);

    return (oNode->type == XML_ELEMENT_NODE
      && oNode->ns
      && _STREQUAL((const char*) oNode->ns->href, NAMESPACE_XSL)
    );
  }

  void LibXslNode::clearRedundantStylesheetNodeCacheItems(const IXmlQueryEnvironment *pQE) const {
    //xsltFreeStylesheet() returns 1 if the stylesheet has usage_locks still
    //if the stylesheet pointer is in this array then it cannot recieve anymore cache hits
    //so keep trying to release them (quick)
    IXslDoc *pStylesheet;
    vector<IXslDoc*>::iterator iRedundantStylesheet;

    iRedundantStylesheet = ms_vRedundantStylesheets.begin();
    while (iRedundantStylesheet != ms_vRedundantStylesheets.end()) {
      pStylesheet = *iRedundantStylesheet;
      if (pStylesheet->hasLocks()) {
        //Debug::report("stylesheet in use still");
        iRedundantStylesheet++;
      } else {
        //Debug::report("stylesheet release");
        delete pStylesheet; //xsltFreeStylesheet(oSS)
        ms_vRedundantStylesheets.erase(iRedundantStylesheet);
      }
    }
  }

  IXslDoc *LibXslNode::checkStylesheetNodeCache(const IXmlQueryEnvironment *pQE) const {
    //search for cached stylesheet by unique xpath to stylesheet root (usually xml:id)
    //ms_mCachedStylesheets uses a string xpath because we don't want to use temporary pointers
    //however, the primary MessageInterpretations actually maintain their pointers so it would have worked for them
    //this is more future proof and has little performance loss
    //Stylesheets can be transient if they are client-side transforms in the tmp area
    IXslDoc *pCacheHit = 0;
    StringMap<IXslDoc*>::iterator iCachedStylesheet;
    const char *sUniqueStylesheetIdentifier;

    if (!isTransient()) {
      sUniqueStylesheetIdentifier = uniqueXPathToNode(pQE);

      iCachedStylesheet = ms_mCachedStylesheets.find(sUniqueStylesheetIdentifier);
      if (iCachedStylesheet != ms_mCachedStylesheets.end()) {
        //cache hit
        pCacheHit = iCachedStylesheet->second;
        if (REPORT_CACHE) Debug::report("cache hit!");
      }
    }

    return pCacheHit;
  }

  bool LibXslNode::clearMyCachedStylesheetsRecursive(const IXmlQueryEnvironment *pQE, const IXslStylesheetNode *pXSLStylesheetNode, const IXmlBaseNode *pNode) {
    //PRIVATE
    //returns true if this cached stylesheet (pXSLStylesheetNode) NEEDS to be cleared
    //  pXSLStylesheetNode = the cached stylesheet top level node
    //  pNode    = the altered node
    //this cached stylesheet also has xsl:includes
    //those need to also be checked to see if pNode is part of them
    //location of xsl:include same as:
    //  C++ clearMyCachedStylesheetsRecursive()
    //  C++ translateIncludes() @href => @xpath
    //  LibXml2: xsl:include processing (of @xpath) during server side compilation
    //  map_url_to_stylesheet_model.xsl REST based client xsl:include requests
    //  AJAX client_stylesheet_transform.xsl xsl:include include directly in main xsl:stylesheet request
    bool bClearThis = false;
    const IXmlBaseNode *pXSLIncludeNode           = 0,
                       *pIncludedNode             = 0,
                       *pParentOfIncludeDirectory = 0,
                       *pXSLStylesheetNodeType    = 0;
    const IXslStylesheetNode *pIncludedStylesheet = 0;
    const char *sHREF      = 0,
               *sXPath     = 0,
               *sXPathHREF = 0,
               *sPathUsed  = 0;
    XmlNodeList<const IXmlBaseNode> *pIncludes = 0;
    typename XmlNodeList<const IXmlBaseNode>::iterator iInclude;

    UNWIND_EXCEPTION_BEGIN_STATIC(pQE) { //pQE as MM
      //TODO: clearMyCachedStylesheetsRecursive() prevent infinite recursion
      pXSLStylesheetNodeType = pXSLStylesheetNode->queryInterface((const IXmlBaseNode*) 0);
      if (pXSLStylesheetNodeType->is(pNode, HARDLINK_AWARE) || pXSLStylesheetNodeType->isAncestorOf(pQE, pNode)) {
        //pNode is directly part of this pXSLStylesheetNode stylesheet
        //main stylesheet edit: return clear it
        bClearThis = true;
      } else {
        //check the xsl:includes of this pXSLStylesheetNode stylesheet for pNode
        //<xsl:include href="<xpath or file-system-path or mixed>" [xpath="<xpath>"] />
        //  @xpath takes preference
        //  @href may be mixed
        //@href: all xsl:include location resolution must use fileSystemPathToXPath(@href)
        if (pIncludes = pXSLStylesheetNodeType->getMultipleNodes(pQE, "xsl:include")) {
          if (pParentOfIncludeDirectory = pXSLStylesheetNodeType->parentNode(pQE)) {
            for (iInclude = pIncludes->begin(); iInclude != pIncludes->end() && !bClearThis; iInclude++) {
              //get the xpath to the actual included stylesheet
              pXSLIncludeNode = (*iInclude);

              //--------- @xpath (xpath only) takes precedence
              if (sXPath = pXSLIncludeNode->attributeValue(pQE, "xpath")) {
                //server side only direct xpath resolution
                //xpath MUST be correct otherwise the xsl:include xpath resolution will fail (obviously)
                //LibXML2 has been altered to accept xsl:include @xpath attributes during compilation
                //obviously client-side software will not do this
                sPathUsed = sXPath;
                if (pIncludedNode = pParentOfIncludeDirectory->getSingleNode(pQE, sXPath))
                  pIncludedStylesheet = pIncludedNode->queryInterface((const IXslStylesheetNode*) 0);
                else Debug::report("xsl:include @xpath [%s] does not resolve", sXPath, rtWarning);
              }

              //--------- @href (xpath) path can be resolved maybe
              //file-system: /system/
              //to xpath:    /repository:system[1]/
              //@href SHOULD be in file-system format
              //@href can also be in relative xpath format, sent through from the browser!
              //@href can also be MIXED file-system and xpath with file-system xsl:includes from xpath specified stylesheets
              //  this happens also when client-side stylesheets include each other based on xpath
              //@href can also include ~HTTP class references and things
              //fileSystemPathToXPath(string) translates only the file-system parts
              //using maybeXPath(string) to decide what to translate
              if (!pIncludedStylesheet) {
                if (sHREF = pXSLIncludeNode->attributeValue(pQE, "href")) {
                  if (pXSLStylesheetNodeType->xmlLibrary()->maybeXPath(pQE, sHREF)) {
                    //maybe un-necessary step because fileSystemPathToXPath(string) will handle this
                    if (pIncludedNode = pParentOfIncludeDirectory->getSingleNode(pQE, sHREF))
                      pIncludedStylesheet = pIncludedNode->queryInterface((const IXslStylesheetNode*) 0);
                    else Debug::report("xsl:include @href [%s] looks like an xpath expression (contains :) but does not resolve", sHREF, rtWarning);
                    sPathUsed = sHREF;
                  }

                  //--------- @href (file-system / MIXED) path can be resolved maybe
                  if (!pIncludedStylesheet) {
                    pIncludedStylesheet = pParentOfIncludeDirectory->relativeXSLIncludeFileSystemPathToXSLStylesheetNode(pQE, sHREF);
                    if (!pIncludedStylesheet) Debug::report("xsl:include @href [%s] does not resolve", sHREF, rtInformation, rlWorrying);
                  }
                }
              }

              //--------- recurse looking for our changed node
              //if we have a valid include node
              if (pIncludedStylesheet) bClearThis = clearMyCachedStylesheetsRecursive(pQE, pIncludedStylesheet, pNode);
              else if (!sPathUsed)     throw AttributeRequired(pQE, MM_STRDUP("@xpath or @href"), MM_STRDUP("xsl:include")); //pQE as MM
              else                     throw FailedToResolveXSLIncludePath(pQE, MM_STRDUP(sPathUsed)); //pQE as MM

              //free up
              if (pIncludedStylesheet) {delete pIncludedStylesheet; pIncludedStylesheet = 0;}
              if (sHREF)               {MM_FREE(sHREF);        sHREF  = 0;}
              if (sXPath)              {MM_FREE(sXPath);       sXPath = 0;}
              sPathUsed = 0;
            }
          }
        }
      }
    } UNWIND_EXCEPTION_END_STATIC(pQE); //pQE as MM

    //free up
    if (pIncludedStylesheet)       delete pIncludedStylesheet;
    if (sHREF)                     MM_FREE(sHREF);
    if (sXPath)                    MM_FREE(sXPath);
    if (sXPathHREF)                MM_FREE(sXPathHREF);
    if (pIncludes)                 vector_element_destroy(pIncludes);
    if (pParentOfIncludeDirectory) delete pParentOfIncludeDirectory;

    UNWIND_EXCEPTION_THROW;

    return bClearThis;
  }

  bool LibXslNode::clearMyCachedStylesheets(const IXmlQueryEnvironment *pQE, const IXmlBaseNode *pNode) {
    //this node can be in the xsl:include's also
    //because the compiled parent stylesheet is cached, not each individual include
    bool bClearThis, bSomeCleared = false;
    StringMap<IXslDoc*>::iterator iCachedStylesheet;
    IXslDoc *pStyleSheet;
    const char *sUniqueXPathToStylesheet;
    const IXmlBaseNode *pStylesheetNode;

    //test each stored stylesheet to see if it contains the altererd node
    //or any of its xsl:includes contain the node
    //Database:transforms will re-access the cacheing system so will also have entries in this map cache
    if (!pNode->isTransient()) {
      iCachedStylesheet = ms_mCachedStylesheets.begin();
      while (iCachedStylesheet != ms_mCachedStylesheets.end()) {
        bClearThis = false;
        sUniqueXPathToStylesheet = iCachedStylesheet->first;
        if (pStylesheetNode = pNode->document()->getSingleNode(pQE, sUniqueXPathToStylesheet)) {
          bClearThis = clearMyCachedStylesheetsRecursive(pQE, pStylesheetNode->queryInterface((const IXslStylesheetNode*) 0), pNode);
          delete pStylesheetNode;
        } else bClearThis = true;

        if (bClearThis) {
          if (REPORT_CACHE) Debug::report("cache remove [%s]", sUniqueXPathToStylesheet);
          //here we move the cache entry in to the removal vector
          //  but do not free the compiled stylesheet
          //  because these compiled XSLTs edit the IXmlDoc thus running this cache removal as part of the XSLT
          //  which would free a running stylesheet
          //we are removing entries from a map that we are traversing
          //  this following statement is subtle, it increments the iterator and then passes a copy of the old iterator to the function
          pStyleSheet = iCachedStylesheet->second;
          ms_mCachedStylesheets.erase(iCachedStylesheet++);   //prevent further cache hits
          ms_vRedundantStylesheets.push_back(pStyleSheet);    //mark the compiled stylesheet for removal next transform
          MM_FREE(sUniqueXPathToStylesheet);             //free up the old key
        } else ++iCachedStylesheet;
      }
    }
    
    return bSomeCleared;
  }

  void LibXslNode::freeParsedTemplates() {
    //freeing cached templates
    //called by our XmlLibrary
    StringMap<IXslDoc*>::iterator iCachedTemplate;
    for (iCachedTemplate = ms_mCachedStylesheets.begin(); iCachedTemplate != ms_mCachedStylesheets.end(); iCachedTemplate++) {
      MM_FREE(iCachedTemplate->first);
      delete iCachedTemplate->second;
    }
  }

  void LibXslNode::cacheStylesheetNode(const IXmlQueryEnvironment *pQE, IXslDoc *pSS) const {
    const char *sUniqueStylesheetIdentifier = uniqueXPathToNode(pQE); //TODO: cache the identifier
    ms_mCachedStylesheets.insert(MM_STRDUP(sUniqueStylesheetIdentifier), pSS);
    if (REPORT_CACHE) Debug::report("cached [%s]", sUniqueStylesheetIdentifier);
  }

  void LibXslNode::serialise(const IXmlQueryEnvironment *pQE, IXmlBaseNode *pOutputNode) {
    //static
    IXmlBaseNode *pCurrent, *pStylesheets, *pRootNode;
    StringMap<IXslDoc*>::const_iterator iStylesheet;
    vector<IXslDoc*>::const_iterator iRedundantStylesheet;
    IXslDoc *pStylesheet;
    IXmlBaseDoc *pStylesheetType;
    const char *sRepositoryName;

    pStylesheets = pOutputNode->createChildElement(pQE, "cached-stylesheets");
    for (iStylesheet = ms_mCachedStylesheets.begin(); iStylesheet != ms_mCachedStylesheets.end(); iStylesheet++) {
      pCurrent        = pStylesheets->createChildElement(pQE, "stylesheet", NULL, NAMESPACE_XSL);
      pStylesheet     = iStylesheet->second;
      pStylesheetType = pStylesheet->queryInterface((IXmlBaseDoc*) 0);
      if (pRootNode = pStylesheetType->rootNode(pQE)) {
        sRepositoryName = pRootNode->attributeValue(pQE, "name", NAMESPACE_REPOSITORY);
        pCurrent->setAttribute(pQE, "name", sRepositoryName, NAMESPACE_REPOSITORY);
        delete pRootNode;
        MM_FREE(sRepositoryName);
      }
      delete pCurrent;
    }
    delete pStylesheets;

    pStylesheets = pOutputNode->createChildElement(pQE, "redundant-stylesheets");
    for (iRedundantStylesheet = ms_vRedundantStylesheets.begin(); iRedundantStylesheet != ms_vRedundantStylesheets.end(); iRedundantStylesheet++) {
      pCurrent        = pStylesheets->createChildElement(pQE, "stylesheet", NULL, NAMESPACE_XSL);
      pStylesheet     = iStylesheet->second;
      pStylesheetType = pStylesheet->queryInterface((IXmlBaseDoc*) 0);
      if (pRootNode = pStylesheetType->rootNode(pQE)) {
        sRepositoryName = pRootNode->attributeValue(pQE, "name", NAMESPACE_REPOSITORY);
        pCurrent->setAttribute(pQE, "name",  sRepositoryName, NAMESPACE_REPOSITORY);
        MM_FREE(sRepositoryName);
      }
      delete pCurrent;
    }
    delete pStylesheets;
  }

  IXmlBaseNode *LibXslNode::clone_wrapper_only() const {return base_object_clone<LibXslNode>(this);}
  IXmlBaseNode *LibXslNode::clone_with_resources() const {return new LibXslNode(*this);}

  //----------------------------------------------------------------------------------------------------------
  //------------------------------------------- --------------------------------------------------------------
  //----------------------------------------------------------------------------------------------------------
  bool LibXslTemplateNode::gimme(const xmlNodePtr oNode) {
    return (oNode->type == XML_ELEMENT_NODE
      && oNode->ns
      && oNode->name
      && _STREQUAL((const char*) oNode->ns->href, NAMESPACE_XSL)
      && _STREQUAL((const char*) oNode->name, "template")
    );
  }

  const char *LibXslTemplateNode::toString() const {
    stringstream sOut;

    const char *sXmlID = (const char*) xmlGetNsProp(m_oNode,   (const xmlChar*) "id", (const xmlChar*) NAMESPACE_XML);
    const char *sMatch = (const char*) xmlGetNoNsProp(m_oNode, (const xmlChar*) "match");
    const char *sMode  = (const char*) xmlGetNoNsProp(m_oNode, (const xmlChar*) "mode");

    sOut << "xsl:template"
      << " [" << (sXmlID ? sXmlID : "<no xml:id>") << "]"
      << " [" << (sMatch ? sMatch : "<no match>")  << "]"
      << " [" << (sMode  ? sMode  : "<no mode>")   << "]";

    //TODO: parameters for the xsl:template debug output

    //free up
    if (sXmlID) MMO_FREE(sXmlID);
    if (sMatch) MMO_FREE(sMatch);
    if (sMode)  MMO_FREE(sMode);

    return MM_STRDUP(sOut.str().c_str());
  }

  //----------------------------------------------------------------------------------------------------------
  //------------------------------------------- --------------------------------------------------------------
  //----------------------------------------------------------------------------------------------------------
  bool LibXslStylesheetNode::gimme(const xmlNodePtr oNode) {
    return (oNode->type == XML_ELEMENT_NODE
      && oNode->ns
      && oNode->name
      && _STREQUAL((const char*) oNode->ns->href, NAMESPACE_XSL)
      && _STREQUAL((const char*) oNode->name, "stylesheet")
    );
  }
}
