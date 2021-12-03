//platform agnostic file
#include "Main.h"

#include "Server.h"
#include "Database.h"
#include "Tests.h"

#include "LibXmlLibrary.h" //full definition for usage
//platform specific project headers
#include "LibXslModule.h"
#include "LibXmlBaseDoc.h"

//TODO: wrap these to platform agnostic
#include "Utilities/gdb.c"
#include <stdexcept>
#include <signal.h>
#include <vector>
#include <iostream>
using namespace std;

namespace general_server {
  Main *Main::m_pMain = 0;

  Main::Main(const int argc, const char **argv):
    m_pServer(0),
    m_pConfigDatabase(0),
    m_pXmlLibrary(0)
  {
    //Main exists as a platform independent main(...)
    //called by main(...) after construction, with command line arguments
    //m_pServer is listening on new threads,
    //now we need to give it a reason to hang around, like console input and remote input
    //ALL the following calls are BLOCKING
    //argv[0] == "general_server"
    //argv[1] == <interaction mode> (default: process forked demon)
    bool bWaitOnCommandLine  = false;
    bool bRunServer          = true;
    bool bFork               = true;
    bool bBlockThisThread    = BLOCK;
    bool bTestAllThreadsExit = false;
    const char *sConfigDir  = "/var/www/generalserver/config/";
#ifdef WITH_DOCUMENT_ROOT
    sConfigDir = WITH_DOCUMENT_ROOT;
#endif

    int pid = 0;
    m_pMain = this;
    char pause[1];
    DatabaseNode *pServerNode = 0;

    //-------------------- command line arguments processing
    if (argc > 1) {
      if (!strcmp(argv[1], "demon"))       {
        //default, no args
        //used from start-stop-demon
        bWaitOnCommandLine = false;
        bFork              = true;
      } else if (!strcmp(argv[1], "single")) {
        //good for non-interactive debug environments
        bWaitOnCommandLine = false;
        bFork              = false;
      } else if (!strcmp(argv[1], "interactive")) {
        //forked process but with interaction
        bWaitOnCommandLine = true;
        bFork              = false;
        if (argc > 2) {
          if (!strcmp(argv[2], "pause")) {
            cout << "begin run? [Yn]\n";
            cin >> pause;
            if (strcmp(pause, "y")) exit(0);
          }
        }
      } else if (!strcmp(argv[1], "--help")) {
        cout << "general server command line help:\n";
        cout << "  usage: general_server <mode>\n";
        cout << "  modes:\n";
        cout << "    demon: no console interaction, run as new process (&)\n";
        cout << "    interactive: server console interaction, blocks main thread\n";
        bRunServer = false;
      } else {
        cout << "unknown mode [" << argv[1] << "]\n";
        cout << "modes are demon, interactive (default)\n";
        cout << "running server anyway...\n";
      }
    }

    //-------------------- create and run server
    //now run the server with blocking on *this* main thread
    if (bRunServer) {
      
      if (bFork) {
#ifdef HAVE_WORKING_FORK
	pid = fork();
#else
	cout << "cannot fork because it is not installed!";
	pid = 0;
#endif
      }
      
      if (pid == 0) {
        cout << "---------------------------------------------- general server startup sequence:\n";
        if (bFork) cout << "(forked)\n";
        else       cout << "(single process)\n";

        //-------------------- signals
        cout << "trapping SIGTERM, SIGINT, SIGPIPE\n";
        signal(SIGTERM,  Main::sigterm_static);
        signal(SIGINT,   Main::sigint_static);
        signal(SIGPIPE,  Main::sigpipe_static);

        //------------------------------------------------ native library validity checks
        m_pXmlLibrary = new SERVER_MAIN_XML_LIBRARY(this);
        m_pXmlLibrary->threadInit();
        Debug::reportObject(m_pXmlLibrary);
        assert(m_pXmlLibrary->isNativeXML() == CAPABILITY_XML_NATIVE);
        assert(m_pXmlLibrary->canTransform());

        //------------------------------------------------ load main Database (Repository + IXmlDoc)
        //that is the basis for the entire system
        //and the configuration for the Server objects that fire up everything else
        m_pConfigDatabase = mmo_new<Database>(new Database(this, m_pXmlLibrary, "config", sConfigDir));

        //-------------------- tests
        //any high level functionality overview test
        //do early to reduce xpath statements before this
        //avoid any HTTP testing at this point because the server is not up
        //the Server loads the Classes as well, asking the Database to install them
        Tests ts_pre(m_pConfigDatabase, "HTTP-tests,databaseclass-tests"); //BLOCKING
        IFDEBUG(Debug::reportObject(m_pConfigDatabase));

        //manual code test area
        /*
        Repository *pRepositoryTest = Repository::factory(m_pServer, "postgres:dbname=postgres");
        string stContent;
        pRepositoryTest->readValidXML(stContent);
        cout << "  Repository content: [\n" << stContent << "\n]\n";
        delete pRepositoryTest; 
        */

        //-------------------- recursive object attach, starting with the server
        //Server/Service/
        XmlAdminQueryEnvironment iBQE_serverStartup(this, m_pConfigDatabase->document_const());
        pServerNode = m_pConfigDatabase->getSingleNode(&iBQE_serverStartup, "/%s", Server::xsd_name());
        if (!pServerNode) throw NoServersFoundInMainDatabase(this, Server::xsd_name());
        m_pServer = mmo_new<Server>(new Server(this, pServerNode, bWaitOnCommandLine, bBlockThisThread));
        
        Tests ts_post(m_pConfigDatabase, NULL, "databaseclass-tests"); //BLOCKING
        
        //-------------------- start / stop
        m_pServer->runInCurrentThread(); //blocking if bBlockThisThread = BLOCK
        //server is blocked on main thread here
        //until:
        //  an exit command comes through a listening socket
        //  or it's CommandLineProcessor (bWaitOnCommandLine) returns an exit
        //server thread ->
        //  service thread ->
        //    requests thread
        m_pServer->blockingCancel();     //blocking wait for shutdown of services, and thus requests
        delete m_pServer;                //blocking shutdown sequence
        m_pServer = 0;
      }
    }

    //we are probably testing the immediate shutdown
    //so block until all threads have exited
    if (bTestAllThreadsExit) {
      cout << "exiting main thread to test that all other threads have gone.\n";
      cout << "if main freezes after this it means that there are still other threads waiting\n";
      pthread_exit(0);
    }
  }

  Main::~Main() {
    if (m_pServer)         delete m_pServer;
    if (m_pConfigDatabase) delete m_pConfigDatabase;
    if (m_pXmlLibrary)     m_pXmlLibrary->threadCleanup(); //TODO: encapsulate in the ~XmlLibrary?
    if (m_pXmlLibrary)     delete m_pXmlLibrary;
  }

  const char *Main::toString() const {return MM_STRDUP("Main");}
  
  void Main::sigint_static( int param) {Main::m_pMain->sigint( param);}
  void Main::sigterm_static(int param) {Main::m_pMain->sigterm(param);}
  void Main::sigpipe_static(int param) {Main::m_pMain->sigpipe(param);}

  void Main::sigint( int param) {
    NOT_CURRENTLY_USED("");
    cout << "recieved SIGINT\n";
  }
  void Main::sigterm(int param) {
    cout << "recieved SIGTERM\n";
    if (m_pServer) m_pServer->blockingCancel();
  }
  void Main::sigpipe(int param) {
    cout << "recieved SIGPIPE\n";
    if (m_pServer) m_pServer->blockingCancel();
  }
}
