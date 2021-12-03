#ifndef _XPATHSTRINGSTREAM
#define _XPATHSTRINGSTREAM

#include "Exceptions.h"

#include <sstream>
using namespace std;

class XPathStringStream: public stringstream {
public:
  bool m_bInStringMode;   //an odd number of ' causes stream to toggle value escape mode
  int  m_iPredicateDepth; //an [ without equal number of corresponding ] causes stream to toggle predicate mode

  XPathStringStream(): 
    m_bInStringMode(false), 
    m_iPredicateDepth(0) 
  {
    NOT_CURRENTLY_USED("problems with potential ('] and [') and ( << sDyn1 << sDyn2 ) not knowing if still m_bInStringMode");
  }
}; 

XPathStringStream& operator<<(XPathStringStream &xs, const char *sValue) {
  //encode stream inputs
  //an [ without equal number of corresponding ] causes stream to toggle predicate mode
  //an odd number of ' causes stream to toggle value escape mode
  //  e.g. << "html:div[@test='" << sThing << "']"
  size_t iLen, i;
  char c;
  static const char *sQuoteReplacement = "&quot;";
  static size_t iQuoteReplacementLen   = strlen(sQuoteReplacement);
  static const char *sAposReplacement = "&#39;";
  static size_t iAposReplacementLen   = strlen(sAposReplacement);
  
  iLen = strlen(sValue);
  if (xs.m_bInStringMode) { //STRING value
    for (i = 0; i < iLen; i++) {
      c = sValue[i];
      switch (c) {
        case '"':  xs.write(sQuoteReplacement, iQuoteReplacementLen); break;
        case '\'': xs.write(sAposReplacement,  iAposReplacementLen);  break;
        default: xs.put(c);
      }
    }
  }
  else { //XPATH
    xs.write(sValue, iLen); //important not to use << again for infinite recursion with this function
    
    //modifiers for this xpath section
    xs.m_iPredicateDepth += strcount(sValue, '[', 0) - strcount(sValue, ']', 0);
    if (strcount(sValue, '\'', 0) ^ 2 == 1) xs.m_bInStringMode = !xs.m_bInStringMode;
    if (strcount(sValue, '\"', 0) ^ 2 == 1) xs.m_bInStringMode = !xs.m_bInStringMode;
  }
  
  return xs;
}
#endif