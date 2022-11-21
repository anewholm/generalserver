//platform specific file (UNIX)
//  the main code can be found on the laptop
//  windows compatibility discontinued ages ago cause its shit anyway but a good excerise for architecture
//External Component quicklinks:
//  LibXML2: http://www.xmlsoft.org/ (also on local machine)

/* Welcome to general server
GS is a generic server system desinged to be able to respond to any textual input stream arriving on any port
and transform that input to commands and processes. GS uses regex to transform the incoming
input stream to an XML document and then XSLT to further transofrm that in to a response. The base XML processing
library is extended to include write back commands allowing the XSLT to change its source document
and even embed foreign programming languages.

GS base capabilities include loading XML DOMs, XSD, XSLT + write back, regex, node read and write watching,
and communication with other servers. These base set of capabilities allow GS to operate as a Web Server and a
database server or anything else.

GS is defined and controlled by its single source XML file which, normally is actually a full directory structure
of XML files interpreted by Repository as a single XML file representation.
*/

#include "Main.h"
using namespace general_server;

int main(const int argc, const char **argv) {
  //pass directly to a Main for platform independent boot sequence
  Main bm(argc, argv); //BLOCKING

  return 0;
}

