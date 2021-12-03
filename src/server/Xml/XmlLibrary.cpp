//platform agnostic file
#include "Xml/XmlLibrary.h"

#include "Repository.h"
#include "Xml/XmlNamespace.h"
#include "IXml/IXslNode.h"
#include "IXml/IXmlGrammarContext.h"
#include "Debug.h"

#include "Utilities/strtools.h"
#include <string.h>
#include <sstream>
using namespace std;

//Abstract Factory Method: http://en.wikipedia.org/wiki/Abstract_factory_pattern
namespace general_server {
  XmlLibrary::XmlLibrary(const IMemoryLifetimeOwner *pMemoryLifetimeOwner): MemoryLifetimeOwner(pMemoryLifetimeOwner) {
    //http://man7.org/linux/man-pages/man3/strptime.3.html
    //DATE_FORMATS_COUNT = 6
    m_aDateFormats[0] = "%s";               //1430316435 (seconds since epoch time-stamp)
    m_aDateFormats[1] = DATE_FORMAT_XML;    //2015-09-28T12:34:54+00:00
    m_aDateFormats[2] = DATE_FORMAT_RFC822; //Sun, 06 Nov 1994 08:49:37 GMT (RFC 822, updated by RFC 1123)
    m_aDateFormats[3] = DATE_FORMAT_RFC850; //Sunday, 06-Nov-94 08:49:37 GMT (RFC 850, obsoleted by RFC 1036)
    m_aDateFormats[4] = DATE_FORMAT_ANSI;   //Sun Nov  6 08:49:37 1994 (ANSI C's asctime() format)
    m_aDateFormats[5] = "%A";               //Monday
  }

  XmlLibrary::~XmlLibrary() {
    XmlHasNamespaceDefinitions::freeStandardNamespaceDefinitions();
  }

  const char *XmlLibrary::toString() const {
    stringstream sOut;
    const char *sName = name();
    sOut << "XML Library [" << sName << "]:\n";
    sOut << "  whichSecurityImplementation: " << whichSecurityImplementation() << "\n";
    sOut << "  canHardLink: " << canHardLink() << "\n";
    sOut << "  canSoftLink: " << canSoftLink() << "\n";
    sOut << "  canTransform: " << canTransform() << "\n";
    sOut << "  supportsXmlID: " << supportsXmlID() << "\n";
    sOut << "  supportsXmlRefs: " << supportsXmlRefs() << "\n";
    sOut << "  isNativeXML: " << isNativeXML() << "\n";
    sOut << "  acceptsNodeParamsForTransform: " << acceptsNodeParamsForTransform() << "\n";
    sOut << "  canUpdateGlobalVariables: " << canUpdateGlobalVariables() << "\n";
    sOut << "  hasNameAxis: " << hasNameAxis() << "\n";
    sOut << "  hasTopAxis: " << hasTopAxis() << "\n";
    sOut << "  hasParentsAxis: " << hasParentsAxis() << "\n";
    sOut << "  hasAncestorsAxis: " << hasAncestorsAxis() << "\n";
    sOut << "  hasSecurity: " << hasSecurity() << "\n";

    //clear up
    //MMO_FREE(sName); //constant

    return MM_STRDUP(sOut.str().c_str());
  }

  void XmlLibrary::handleError(ExceptionBase& ex ATTRIBUTE_UNUSED) const {
    NOT_CURRENTLY_USED("");
  }

  const char *XmlLibrary::fileSystemPathToXPath(const IXmlQueryEnvironment *pQE, const char *sFileSystemPath, const char *sTargetType, const char *sDirectoryNamespacePrefix, const bool bUseIndexes) const {
    //fileSystemPathToXPath() DOES NOT interact with the database
    //  it is simple string handling
    //
    //relative paths:
    //  ../../shared/thingy.xsl => ../../repository:shared/name::thingy_xsl
    //if name axis is implemented:
    //  (non alpha-numeric chars ignored)
    //  ../../shared/thingy.xsl => ../../repository:shared/name::thingy_xsl
    //a note about query-strings (they are ignored):
    //  ../../shared/thingy.xsl?gs_include=1 => ../../repository:shared/name::thingy_xsl
    //xpath portions are left un-touched (anything with a colon)
    //  ../../repository:shared/thingy.xsl   => ../../repository:shared/name::thingy_xsl
    //absolute:
    //  /shared/index.html -> /<sRootXPath>/repository:shared/name::index_hmtl
    //class references:
    //  /~Person/name::view          => ~Person/name::view
    //  /test/eg/~Person/name::view  => ~Person/name::view
    //  ~Person/name::view           => ~Person/name::view
    //absolute paths
    //  caller MUST send through the DOC node or other base node for this to work
    //  /test/is/good with doc base node   => /repository:test/repository:is/name::good
    //  /test/is/good with base-node idx_8 => id('idx_8')/repository:test/repository:is/name::good
    //absolute function calls:
    //  /id('idx_3')/name::view          => id('idx_3')/name::view
    //  /test/eg/id('idx_3')/name::view  => id('idx_3')/name::view
    //  id('idx_3')/name::view           => id('idx_3')/name::view
    //index for empty directory
    //  directroy1/directroy2/           => repository:directroy1/repository:directroy2/name::index
    //  BUT: directroy1/directroy2       => repository:directroy1/name::directroy2
    //specials:
    //  ^Class__Response_response_loader/... => id('Class__Response_response_loader')/...
    string stOut;
    const char *sPos, *sLastPos, *sTerminatingChar,
               *sDUP   = 0;
    char c;
    size_t iLen, iInitialLen;
    int iBracketLevel  = 0; //processing only happens OUTSIDE brackets (...)
    bool bHasSplitters = false;

    //------------------- ignore common irrelevant bits
    iInitialLen      = strlen(sFileSystemPath);
    sTerminatingChar = sFileSystemPath + iInitialLen;
    sPos             = strchr(sFileSystemPath, '?');                   //query-string (first ?)
    if (!sPos) sPos  = sTerminatingChar;
    sLastPos = strnrchr(sFileSystemPath, '.', sPos - sFileSystemPath); //file extension (last .)
    if ( !sLastPos
      || strnchr(sLastPos, '\'', sPos - sLastPos) //inside a string literal: ' is not a valid extension char
      || strnchr(sLastPos, ')',  sPos - sLastPos) //inside a function      : ) is not a valid extension char
    ) sLastPos = sPos;
    //conditional STRDUP
    sPos = sFileSystemPath;
    if (sLastPos != sTerminatingChar) sPos = sDUP = _STRNDUP(sFileSystemPath, sLastPos - sFileSystemPath);
    sLastPos = sPos;
    stOut.reserve(iInitialLen * 2);

    //------------------- copy root slash (if present or completely blank string)
    if (Repository::issplitter(*sPos) || !*sPos) {
      stOut.append("/");
      bHasSplitters = true; //important so that name::index is appended
      if (*sPos) {
        sLastPos++;
        sPos++;
      }
    }

    //------------------- directories / xpath snippets (if any)
    //   name::<directory name>
    //OR repository:<directory name>
    while (sPos = Repository::strsplitter(sLastPos)) {
      if (iLen = sPos - sLastPos) {
        iBracketLevel += strcount(sLastPos, '(', iLen);
        if (iBracketLevel > 0) {
           //inside brackets, or part includes brackets, continue
           //TODO: can name:: work with brackets, e.g. /folder/document(thing)?
           stOut.append(sLastPos, iLen + 1);
        } else {
          iBracketLevel = 0; //over-negative brackets: ))) will be ignored
          if (maybeXPath(pQE, sLastPos, iLen)) {
            if (maybeAbsoluteXPath(pQE, sLastPos))  {
              //this portion appears to be absolute so restart the output from here
              stOut.assign(sLastPos, iLen + 1);
            } else stOut.append(sLastPos, iLen + 1);
          } else {
            IFDEBUG(
              if (_STRNEQUAL(sLastPos, "../", 3)) {
                NOT_COMPLETE(".. parent move found in the file-system-path, normally the HTTP caller will remove these");
              }
              else if (_STRNEQUAL(sLastPos, "./", 2)) {
                NOT_COMPLETE(". current directory found in the file-system-path, normally the HTTP caller will remove these");
              }
            )
              
            //override the normal name:: axis use
            //by specifying a namespace
            if (sDirectoryNamespacePrefix) {
              stOut.append(sDirectoryNamespacePrefix);
              stOut.append(":");
            }
            
            //local name
            stOut.append(sLastPos, iLen + 1); //include backslash
          }
        }
        iBracketLevel -= strcount(sLastPos, ')', iLen);
        bHasSplitters = true;
      } //else it is an irrelevant double slash, so ignore

      //advance to next character
      sPos++;
      sLastPos  = sPos;
    }

    //------------------- last part of the string which will be the filename
    //   name::<filename>
    //OR *[@repository:name = '<filename>']
    if (sLastPos && *sLastPos) {
      if (iBracketLevel) {
        //still in brackets
        stOut.append(sLastPos);
      } else {
        if (maybeXPath(pQE, sLastPos)) {
          if (maybeAbsoluteXPath(pQE, sLastPos)) {
            IFDEBUG(if (stOut.size()) Debug::report("last part of file-system-path was categorised absolute [%s]", sFileSystemPath, rtWarning, rlWorrying));
            stOut.assign(sLastPos);
          } else stOut.append(sLastPos);
        } else {
          //replace non-alpha-numeric
          //default to name:: axis
          while (c = *sLastPos++) {
            if (!isalnum(c)) c = '_';
            stOut.append(1,c);
          }
        }
      }
    } else if (bHasSplitters && bUseIndexes) {
      //no filename after last /
      //fileSystemPath ends in a /
      //so place an index search on the end
      if      (hasDefaultNameAxis()) stOut.append("index");
      else if (hasNameAxis())        stOut.append("name::index");
      else                           stOut.append(sTargetType).append("[@repository:name = 'index']");
    }

    //Debug::report("[%s] => [%s]", sFileSystemPath, stOut.c_str());

    //free up
    if (sDUP) MMO_FREE(sDUP);

    return MM_STRDUP(stOut.c_str());
  }

  bool XmlLibrary::maybeAbsoluteXPath(const IXmlQueryEnvironment *pQE, const char *sText) const {
    //does the path have absolute elements in it?
    //that would mean, for instance, that it would not need knowledge of its start root for absolute cases
    //  admin/interfaces/~Person/weeee
    //  HTTP/repository:interfaces/id('idx_3')/name::test
    bool bGPMaybeXPath = false;
    IXmlGrammarContext *pGP;
    
    if (pGP = pQE->grammarContext()) bGPMaybeXPath = pGP->maybeAbsoluteXPath(pQE, sText);
    
    return sText && (
        bGPMaybeXPath
     || (*sText == '$')             //variables
     || _STRNEQUAL(sText, "id(", 3) //the id() function
    );
  }

  const char *XmlLibrary::nodeTypeXPath(const iDOMNodeTypeRequest iNodeTypes) const {
    const char *sXPath = 0;

    switch (iNodeTypes) {
      //NOTE: this will not currently return CDATA nodes
      case node_type_element_or_text: {sXPath = "(*|text())"; break;} //default
      case node_type_element_only:    {sXPath = "*";          break;}
      case node_type_processing_instruction: {sXPath = "processing-instruction()"; break;}
      case node_type_any:             {sXPath = "node()";      NOT_CURRENTLY_USED(""); break;}
    }
    
    return sXPath;
  }

  bool XmlLibrary::maybeXPath(const IXmlQueryEnvironment *pQE, const char *sText, const size_t iLen) const {
    // need to check each part of a path also:
    //   admin/interfaces/add
    //   ~HTTP/repository:interfaces/name::test
    //TODO: this needs to be attached to the GrammarProcessor!
    //TODO: move this in to LibXmlLibrary
    bool bGPMaybeXPath = false;
    IXmlGrammarContext *pGP;
    
    if (pGP = pQE->grammarContext()) bGPMaybeXPath = pGP->maybeXPath(pQE, sText, iLen);
    
    return sText && (
         bGPMaybeXPath
      || (iLen ? strnchr(sText, '$', iLen) : strchr(sText, '$')) //variables
      || (iLen ? strnchr(sText, '@', iLen) : strchr(sText, '@')) //attributes
      || (iLen ? strnchr(sText, ':', iLen) : strchr(sText, ':')) //namespacing
      || (iLen ? strnchr(sText, '*', iLen) : strchr(sText, '*')) //anything
      || (_STRNEQUAL(sText, "id(", 3))                           //the id() function
    );
  }

  bool XmlLibrary::isAttributeXPath(const char *sText, const size_t iLen) const {
    //spots:
    //  repository:Thing/@name
    //  @meta:class
    //and attribute:: versions
    //NOTE: this function does not parse the XPath
    //therefore if a string was included: *[test='/@'] it would confuse this function
    return sText && (
         (iLen ? strnstr(sText, "/@", iLen) : strstr(sText, "/@"))
      || (iLen ? strnstr(sText, "/attribute::", iLen) : strstr(sText, "/attribute::"))
      || (*sText == '@')
      || (_STRNEQUAL(sText, "attribute::", 11))
    );
  }

  bool XmlLibrary::maybeNamespacedXPath(const char *sText, const size_t iLen) const {
    //ignore built-in xml namespace
    //TODO: this will FAIL in at least these edge cases:
    //  anything that can take a string argument, e.g. starts-with(':')
    //  sdjfhdf_xml:id because the back check only checks for "xml:"
    const char *sColon   = sText;
    bool bNameSpaceFound = false;
    
    if (sText) {
      while (!bNameSpaceFound 
        && (sColon = strchr(sColon, ':')) 
        && (!iLen || ((size_t) (sColon - sText) < iLen))
      ) {
        if (sColon - sText >= 3 && _STREQUAL(sColon-3, "xml:")) sColon++;
        else bNameSpaceFound = true;
      }
    }
    
    return bNameSpaceFound;
  }

  bool XmlLibrary::textEqualsTrue(const char *sText) const {
    //allowed boolean values
    //0x0 = false
    return sText && (
         !strcasecmp(sText, "yes") 
      || !strcasecmp(sText, "true") 
      || !strcasecmp(sText, "on") 
      || !strcasecmp(sText, "1")
      || !strcasecmp(sText, "enabled")
    );
  }

  const char *XmlLibrary::escapeForCDATA(const char *sString) const {
    const char *sOut = sString;
    
    if (sString) {
      while (strstr(sString, "]]>")) {
        NOT_COMPLETE("escapeForCDATA");
      }
    }
    
    return sOut;
  }

  const char *XmlLibrary::normalise(const char *sString) const {
    //caller frees non zero result
    //this just converts c-string style escaping to their characters \\n => \n
    //like the NORMALIZE flag on GRETA regex
    char *sOutput = 0;

    if (sString) {
      size_t iLen = strlen(sString);
      char *sOutputPosition = sOutput = MM_MALLOC_FOR_RETURN(iLen + 1);
      const char *sInputPosition = sString;
      char c;
      while (c = *sInputPosition++) {
        if (c == '\\') {
          if (c = *sInputPosition++) {
            if (isdigit(c)) {
              c = atoi(sInputPosition);
              while (isdigit(*sInputPosition)) sInputPosition++;
            } else {
              switch (c) {
                case 'r': {c = 13;break;}
                case 'n': {c = 10;break;}
                default: {c = '\\'; sInputPosition--;} //output the escapes normally
              }
            }
          } else c = '\\';
        }
        *sOutputPosition++ = c;
      }
      *sOutputPosition = 0;
    }
    return sOutput;
  }

  IXmlLibrary::xmlTraceFlag XmlLibrary::parseXMLTraceFlags(const char *sFlags) const {
    IXmlLibrary::xmlTraceFlag iFlags = IXmlLibrary::XML_DEBUG_NONE;

    if (sFlags) {
        if (strstr(sFlags, "XML_DEBUG_ALL")) iFlags =                 (IXmlLibrary::xmlTraceFlag) (iFlags | IXmlLibrary::XML_DEBUG_ALL);
        if (strstr(sFlags, "XML_DEBUG_NONE")) iFlags =                (IXmlLibrary::xmlTraceFlag) (iFlags | IXmlLibrary::XML_DEBUG_NONE);
        if (strstr(sFlags, "XML_DEBUG_XPATH_STEP")) iFlags =          (IXmlLibrary::xmlTraceFlag) (iFlags | IXmlLibrary::XML_DEBUG_XPATH_STEP);
        if (strstr(sFlags, "XML_DEBUG_XPATH_EXPR")) iFlags =          (IXmlLibrary::xmlTraceFlag) (iFlags | IXmlLibrary::XML_DEBUG_XPATH_EXPR);
        if (strstr(sFlags, "XML_DEBUG_XPATH_EVAL_COUNTS")) iFlags =   (IXmlLibrary::xmlTraceFlag) (iFlags | IXmlLibrary::XML_DEBUG_XPATH_EVAL_COUNTS);
        if (strstr(sFlags, "XML_DEBUG_PARENT_ROUTE")) iFlags =        (IXmlLibrary::xmlTraceFlag) (iFlags | IXmlLibrary::XML_DEBUG_PARENT_ROUTE);
    }

    return iFlags;
  }

  IXmlLibrary::xsltTraceFlag XmlLibrary::parseXSLTTraceFlags(const char *sFlags) const {
    IXmlLibrary::xsltTraceFlag iFlags = IXmlLibrary::XSLT_TRACE_NONE;

    if (sFlags) {
      if (strstr(sFlags, "XSLT_TRACE_ALL")) iFlags =                 (IXmlLibrary::xsltTraceFlag) (iFlags | IXmlLibrary::XSLT_TRACE_ALL);
      if (strstr(sFlags, "XSLT_TRACE_NONE")) iFlags =                (IXmlLibrary::xsltTraceFlag) (iFlags | IXmlLibrary::XSLT_TRACE_NONE);
      if (strstr(sFlags, "XSLT_TRACE_COPY_TEXT"))  iFlags =          (IXmlLibrary::xsltTraceFlag) (iFlags | IXmlLibrary::XSLT_TRACE_COPY_TEXT);
      if (strstr(sFlags, "XSLT_TRACE_PROCESS_NODE"))  iFlags =       (IXmlLibrary::xsltTraceFlag) (iFlags | IXmlLibrary::XSLT_TRACE_PROCESS_NODE);
      if (strstr(sFlags, "XSLT_TRACE_APPLY_TEMPLATE"))  iFlags =     (IXmlLibrary::xsltTraceFlag) (iFlags | IXmlLibrary::XSLT_TRACE_APPLY_TEMPLATE);
      if (strstr(sFlags, "XSLT_TRACE_COPY"))  iFlags =               (IXmlLibrary::xsltTraceFlag) (iFlags | IXmlLibrary::XSLT_TRACE_COPY);
      if (strstr(sFlags, "XSLT_TRACE_COMMENT"))  iFlags =            (IXmlLibrary::xsltTraceFlag) (iFlags | IXmlLibrary::XSLT_TRACE_COMMENT);
      if (strstr(sFlags, "XSLT_TRACE_PI"))  iFlags =                 (IXmlLibrary::xsltTraceFlag) (iFlags | IXmlLibrary::XSLT_TRACE_PI);
      if (strstr(sFlags, "XSLT_TRACE_COPY_OF"))  iFlags =            (IXmlLibrary::xsltTraceFlag) (iFlags | IXmlLibrary::XSLT_TRACE_COPY_OF);
      if (strstr(sFlags, "XSLT_TRACE_VALUE_OF"))  iFlags =           (IXmlLibrary::xsltTraceFlag) (iFlags | IXmlLibrary::XSLT_TRACE_VALUE_OF);
      if (strstr(sFlags, "XSLT_TRACE_CALL_TEMPLATE"))  iFlags =      (IXmlLibrary::xsltTraceFlag) (iFlags | IXmlLibrary::XSLT_TRACE_CALL_TEMPLATE);
      if (strstr(sFlags, "XSLT_TRACE_APPLY_TEMPLATES"))  iFlags =    (IXmlLibrary::xsltTraceFlag) (iFlags | IXmlLibrary::XSLT_TRACE_APPLY_TEMPLATES);
      if (strstr(sFlags, "XSLT_TRACE_CHOOSE"))  iFlags =             (IXmlLibrary::xsltTraceFlag) (iFlags | IXmlLibrary::XSLT_TRACE_CHOOSE);
      if (strstr(sFlags, "XSLT_TRACE_IF"))  iFlags =                 (IXmlLibrary::xsltTraceFlag) (iFlags | IXmlLibrary::XSLT_TRACE_IF);
      if (strstr(sFlags, "XSLT_TRACE_FOR_EACH"))  iFlags =           (IXmlLibrary::xsltTraceFlag) (iFlags | IXmlLibrary::XSLT_TRACE_FOR_EACH);
      if (strstr(sFlags, "XSLT_TRACE_STRIP_SPACES"))  iFlags =       (IXmlLibrary::xsltTraceFlag) (iFlags | IXmlLibrary::XSLT_TRACE_STRIP_SPACES);
      if (strstr(sFlags, "XSLT_TRACE_TEMPLATES"))  iFlags =          (IXmlLibrary::xsltTraceFlag) (iFlags | IXmlLibrary::XSLT_TRACE_TEMPLATES);
      if (strstr(sFlags, "XSLT_TRACE_KEYS"))  iFlags =               (IXmlLibrary::xsltTraceFlag) (iFlags | IXmlLibrary::XSLT_TRACE_KEYS);
      if (strstr(sFlags, "XSLT_TRACE_VARIABLES"))  iFlags =          (IXmlLibrary::xsltTraceFlag) (iFlags | IXmlLibrary::XSLT_TRACE_VARIABLES);

      if (strstr(sFlags, "XSLT_TRACE_FUNCTION"))  iFlags =           (IXmlLibrary::xsltTraceFlag) (iFlags | IXmlLibrary::XSLT_TRACE_FUNCTION);
      if (strstr(sFlags, "XSLT_TRACE_PARSING"))  iFlags =            (IXmlLibrary::xsltTraceFlag) (iFlags | IXmlLibrary::XSLT_TRACE_PARSING);
      if (strstr(sFlags, "XSLT_TRACE_BLANKS"))  iFlags =             (IXmlLibrary::xsltTraceFlag) (iFlags | IXmlLibrary::XSLT_TRACE_BLANKS);
      if (strstr(sFlags, "XSLT_TRACE_PROCESS"))  iFlags =            (IXmlLibrary::xsltTraceFlag) (iFlags | IXmlLibrary::XSLT_TRACE_PROCESS);
      if (strstr(sFlags, "XSLT_TRACE_EXTENSIONS"))  iFlags =         (IXmlLibrary::xsltTraceFlag) (iFlags | IXmlLibrary::XSLT_TRACE_EXTENSIONS);
      if (strstr(sFlags, "XSLT_TRACE_ATTRIBUTES"))  iFlags =         (IXmlLibrary::xsltTraceFlag) (iFlags | IXmlLibrary::XSLT_TRACE_ATTRIBUTES);
      if (strstr(sFlags, "XSLT_TRACE_EXTRA"))  iFlags =              (IXmlLibrary::xsltTraceFlag) (iFlags | IXmlLibrary::XSLT_TRACE_EXTRA);
      if (strstr(sFlags, "XSLT_TRACE_AVT"))  iFlags =                (IXmlLibrary::xsltTraceFlag) (iFlags | IXmlLibrary::XSLT_TRACE_AVT);
      if (strstr(sFlags, "XSLT_TRACE_PATTERN"))  iFlags =            (IXmlLibrary::xsltTraceFlag) (iFlags | IXmlLibrary::XSLT_TRACE_PATTERN);
      if (strstr(sFlags, "XSLT_TRACE_VARIABLE"))  iFlags =           (IXmlLibrary::xsltTraceFlag) (iFlags | IXmlLibrary::XSLT_TRACE_VARIABLE);

      /* pseudo */
      if (strstr(sFlags, "XSLT_TRACE_MOST")) iFlags =                (IXmlLibrary::xsltTraceFlag) (IXmlLibrary::XSLT_TRACE_ALL | ~IXmlLibrary::XSLT_TRACE_BLANKS);
    }

    return iFlags;
  }

  IXmlArea *XmlLibrary::factory_area(const IMemoryLifetimeOwner *pMemoryLifetimeOwner, const IXmlQueryEnvironment *pQE) const {
    return factory_area(pMemoryLifetimeOwner, (XmlNodeList<const IXmlBaseNode> *) 0, pQE, 0);
  }

  const char *XmlLibrary::translateDateFormat(const char *sFormat) const {
    //do not free the result
    //it is either static OR equal to the input
    const char *sResponseFormat = sFormat;

    //XML date standard
    //http://www.w3.org/TR/NOTE-datetime
    //YYYY-MM-DDThh:mm:ssTZD
    //1997-07-16T19:20:30+01:00
    if      (_STREQUAL(sFormat, "%XML")
          || _STREQUAL(sFormat, "%ISO8601")) sResponseFormat = DATE_FORMAT_XML;
    //RFC 822, updated by RFC 1123
    //Sun, 06 Nov 1994 08:49:37 GMT
    else if (_STREQUAL(sFormat, "%RFC822")
          || _STREQUAL(sFormat, "%HTTP"))    sResponseFormat = DATE_FORMAT_RFC822;
    //RFC 850, obsoleted by RFC 1036
    //Sunday, 06-Nov-94 08:49:37 GMT
    else if (_STREQUAL(sFormat, "%RFC850"))  sResponseFormat = DATE_FORMAT_RFC850;
    //ANSI C's asctime() format
    //Sun Nov  6 08:49:37 1994
    else if (_STREQUAL(sFormat, "%ANSI"))    sResponseFormat = DATE_FORMAT_ANSI;

    return sResponseFormat;
  }

  struct tm XmlLibrary::parseDate(const char *sValue) const {
    //m_aDateFormats populated at instanciation
    //this function mimicks getdate() but without the environmental dependency of DATEMSK
    struct tm tGMT;
    time_t tiTime;
    const char *sFailChar = 0;
    size_t i;

    //initilaise value because not all fields are completed
    memset(&tGMT, 0, sizeof(tm));

    //throw parse failure if no input
    if (sValue) {
      if (_STREQUAL(sValue, "now")) {
        time(&tiTime);
        gmtime_r(&tiTime, &tGMT); //reentrant
      } else {
        for (i = 0; i < DATE_FORMATS_COUNT && !sFailChar; i++) {
          sFailChar = strptime(sValue, m_aDateFormats[i], &tGMT);
        }
        if (!sFailChar) throw FailedToParseDate(this, sValue);
        else {
          //test it
          tiTime = mktime(&tGMT);
          if (tiTime == -1) throw FailedToParseDate(this, sValue);
        }
      }
    } else throw FailedToParseDate(this, sValue);

    return tGMT;
  }


  bool XmlLibrary::isNameChar(char c) const {
    //https://www.w3.org/TR/REC-xml/#NT-NameChar
    //NameStartChar | "-" | "." | [0-9] | #xB7 | [#x0300-#x036F] | [#x203F-#x2040]
    return isNameStartChar(c) || c == '-' || c == '.' || isdigit(c);
  }
  
  bool XmlLibrary::isNameStartChar(char c) const {
    //https://www.w3.org/TR/REC-xml/#NT-NameStartChar
    //":" | [A-Z] | "_" | [a-z] | [#xC0-#xD6] | [#xD8-#xF6] | [#xF8-#x2FF] | [#x370-#x37D] | [#x37F-#x1FFF] | [#x200C-#x200D] | [#x2070-#x218F] | [#x2C00-#x2FEF] | [#x3001-#xD7FF] | [#xF900-#xFDCF] | [#xFDF0-#xFFFD] | [#x10000-#xEFFFF]
    return c == ':' || isalpha(c) || c == '_';
  }

  const char *XmlLibrary::xml_element_name(const char *sInput, const bool bAllowNamespace, const bool bMakeLowerCase) const {
    //caller frees result if non-zero
    //by default take the last part of the string after /
    char *sElementName;
    char *position;
    bool bSeenColon            = false,
         bIsFirstCharacterMode = true,
         bLastCharacter        = false;

    if (!sInput) {
      sElementName = MM_STRDUP("gs:unassigned");
    } else {
      if (!*sInput) {
        sElementName = MM_STRDUP("gs:empty");
      } else {
        //ok, we have a valid non-empty sring
        //not changing the length of the string
        sElementName = MM_STRDUP(sInput);

        //make rest alpha numeric lowercase
        for (position = sElementName; *position; position++) {
          bLastCharacter = (position[1] == 0);
          if (bIsFirstCharacterMode) {
            //first character has different rules
            if (!isalpha(*position)) *position = 'X';
            else if (bMakeLowerCase && !islower(*position)) *position = tolower(*position);
            bIsFirstCharacterMode = false;
          } else {
            //rest of element name has other rules
            if (!bLastCharacter && *position == ':' && position[1] && bAllowNamespace && !bSeenColon) {
              bSeenColon = true;
              bIsFirstCharacterMode = true;
            } else {
              //encode non-valid characters
              if (!isalnum(*position) && !strchr("-_", *position)) *position = '_';
              //lower case valid characters
              else if (bMakeLowerCase && !islower(*position)) *position = tolower(*position);
            }
          }
        }

        //remove trailing _
        //decreases length of string but not to 0
        //first character will always not be an _
        while (--position > sElementName && *position == '_') *position = 0;
      }
    }

    //cout << "[" << sInput << "] => [" << sElementName << "]\n";

    return sElementName;
  }
}
