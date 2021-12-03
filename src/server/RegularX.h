#ifndef _REGULARX_H
#define _REGULARX_H

#include "define.h"

#include <vector>
using namespace std;

#include "Utilities/regexpr2.h"
using namespace regex;

#include "Utilities/StringMap.h"    //has-a local StringMap

//platform specific project headers
#include "LibXslModule.h"

namespace general_server {
  class IXmlBaseNode;
  class IXmlBaseDoc;
  class IXmlQueryEnvironment;

  /*
  enum REGEX_FLAGS
  {
      NOFLAGS       = 0x0000,
      NOCASE        = 0x0001, // ignore case
      GLOBAL        = 0x0002, // match everywhere in the string
      MULTILINE     = 0x0004, // ^ and $ can match internal line breaks
      SINGLELINE    = 0x0008, // . can match newline character
      RIGHTMOST     = 0x0010, // start matching at the right of the string
      NOBACKREFS    = 0x0020, // only meaningful when used with GLOBAL and substitute
      FIRSTBACKREFS = 0x0040, // only meaningful when used with GLOBAL
      ALLBACKREFS   = 0x0080, // only meaningful when used with GLOBAL
      NORMALIZE     = 0x0100, // Preprocess patterns: "\\n" => "\n", etc.
      EXTENDED      = 0x0200, // ignore whitespace in pattern
  };
  */

  class RegularX {
    enum decode {
      none, //default
      URL
    };
    static decode decodeType(const char *sString);

    StringMap<match_results_c*> m_directives; //rx:directive can set matches for a sub-tree

  public:
    //static one-time invocation only
    //pmResults can inform the $backrefs in the pRegularX spec
    //pQE must be valid for input and output nodes/docs
    //which means that input doc == output doc
    static IXmlBaseNode *rx(const IXmlQueryEnvironment *pQE, const IXmlBaseNode *pRegularX, const char *sTextStream, IXmlBaseNode *pDestinationNode, match_results_c *pmResults = NULL, const bool bThrowOnSingleOutputFail = false);
    ~RegularX();

    //also: for calling from DEBUG: match("([^\n]*)", sTextStream, SINGLELINE)
    //have used overrides so that simple DEBUG symbol matching works
    static const char *match(const char *sRegularExpression, const char *sInput);
    static const char *match(const char *sRegularExpression, const char *sInput, REGEX_FLAGS flags);
    static char *copyString(const char *pStart, const char *pFinish, const char *sDecode = 0);

    static void compilationInformation(const IXmlQueryEnvironment *pQE ATTRIBUTE_UNUSED, IXmlBaseNode *pOutput ATTRIBUTE_UNUSED) {}

  private:
    IXmlBaseNode *recurseNode(const IXmlQueryEnvironment *pQE, const IXmlBaseNode *pInputNode, IXmlBaseNode *pOutputNode, const char *sCurrentTextStream, match_results_c *pmResults = NULL, const bool bThrowOnSingleOutputFail = false);
  };
}

#endif

