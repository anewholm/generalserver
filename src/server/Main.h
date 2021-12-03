//platform agnostic file
#ifndef _MAIN_H
#define _MAIN_H

//platform generic includes
#include "define.h"
#include "MemoryLifetimeOwner.h" //direct inheritance

namespace general_server {
  class Server;
  class Database;
  class SERVER_MAIN_XML_LIBRARY;

  class Main: virtual public MemoryLifetimeOwner {
    //Main exists as a platform independent main(...)
    //using pointers to objects because we might create a dynamic number
    //of server watching things and stuff
    Server *m_pServer;
    static Main *m_pMain;
    SERVER_MAIN_XML_LIBRARY *m_pXmlLibrary;
    Database *m_pConfigDatabase;

  public:
    Main(const int argc, const char **argv);
    ~Main();

    static void sigint_static( int param);
    static void sigterm_static(int param);
    static void sigpipe_static(int param);

    void sigint( int param);
    void sigterm(int param);
    void sigpipe(int param);

    const char *toString() const;
    static void compilationInformation(const IXmlQueryEnvironment *pQE ATTRIBUTE_UNUSED, IXmlBaseNode *pOutput ATTRIBUTE_UNUSED) {}
  };
}

#endif

