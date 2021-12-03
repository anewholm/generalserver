//platform specific file (WINDOWS)
//The (UNIX) general_server platform specific file (with this documentation) can be found on the Unix server
/* Welcome to general_server
general_server is a generic server system desinged to be able to respond to any textual input stream arriving on any port
and generically transform that input to commands and processes. It can be instructed to listen on any port and
follow stabdard transformation rules for input streams on that port. general_server uses regex to transform the incoming
input stream to an XML document and then XSLT to further transofrm that to an XML general_server command list.

general_servers base capabilities include loading XML DOMs, XSD, XSLT, regex and communication with other servers
These base set of capabilities allow general_server to operate as a Web Server or a database server.

general_servers configuration file is general_server.config.xml in the same directory that the process is run in.
reads from configuration file general_server.config.xml and files in config\*.config.xml (includes are also allowed)

Query examples:
select xsl:<...> from id:343454 transformWith ref:post
select xpath:children::news from xpath://WebPage[@ID=10] transformWith xsl:PCS_news

protocols include:
id: a node id infered by ([0-9]+)
xsl: a loaded stylesheet (infered by (<.*))
xquery/xpath (including xpath): default
ref:(.*) a loaded stylesheet registered against the node under reference $1 infered by ([A-Za-z][A-Za-z0-9]+)
key:ID(5656) a node referenced in key ID with value 5656. Infered by [A-Za-z][A-Za-z0-9]+\((.+?)\)

note that the primary transform (select transform) must be securable (every element must have an ID)

Startup sequence:
read all config files
load all databases
load all stylesheets
register standard transforms
load standard web url interpretations
listen for commands

control commands have the same format as in the config files:
<configure file="..."/>
<loadDB file="..." readOnly="yes"/>

the <website name="EG" domain="www.eg.org"/> config option loads EG\DB.xml DATABASE, all EG\stylesheets\*.xsl and 
	registers default interpretations, and transformations according to the post and query attributes of WebPage(s)

-----------------------------------------------------------------------
Server is platform independent and provides the following functionality
	loading and interpretation of the configuration file
	BSD socket communications for listening and server communications
	parsing and interpretation of incoming messages into commands (main control is done through this, use telnet)
	XML DB management
	shutdown, startup, configuration-reread
general_server, Debug and XML are platform dependent.
	general_server starts the service (done by the ServiceControlManager on Windows)
	Debug writes to file (uses the event log on windows)
	XML uses libxml2 (uses MSXML5 on Windows)
	componentLoader (dlOpen / COM)
*/

//standard includes
#include "stdafx.h"
#include <iostream>
#include "Server.h"

using namespace general_server;
using namespace std;

//function prototypes
void stopService(DWORD status);
void WINAPI ServiceCtrlHandler(DWORD fdwControl);
void WINAPI ServiceStart(DWORD argc, LPTSTR *argv);
STDAPI Test();
STDAPI RunServer();
STDAPI DllRegisterServer(void); //Run As Administrator with "general_server -register"
STDAPI DllUnregisterServer(void);

//globalsre
SERVICE_STATUS          tServiceStatus; 
SERVICE_STATUS_HANDLE   hServiceStatusHandle; 
Server *pServer; //global Server instance

int main(int argc, _TCHAR* argv[])
{
	//Windows service code
	//EXE accepts command line as well as being a service
	switch (argc) {
		case 1: {
			//1 command option=general_server
			//no standard out-

			SERVICE_TABLE_ENTRY DispatchTable[] = 
			{
			  {TEXT("ServiceStart"), ServiceStart}, 
			  {NULL, NULL}
			};
			//ask the SCM to start the service by calling ServiceStart
			if (!StartServiceCtrlDispatcher(DispatchTable)) 000;
			break;
		}
		case 2: {
			//2 command options=general_server <something>
			//requires the system to have a standard out
			_TCHAR *sCommand=argv[1];
			cout << "command:" << sCommand << "\n";
			HRESULT hR = E_NOTIMPL;
			if (!_tcsncmp(sCommand, TEXT("-register"), 9)||!_tcsncmp(sCommand, TEXT("-r"), 2)) hR = DllRegisterServer();
			if (!_tcsncmp(sCommand, TEXT("-unregister"), 11)||!_tcsncmp(sCommand, TEXT("-u"), 2)) hR = DllUnregisterServer();
			if (!_tcsncmp(sCommand, TEXT("-debug"), 6)||!_tcsncmp(sCommand, TEXT("-d"), 2)) hR = RunServer();
			if (!_tcsncmp(sCommand, TEXT("-test"), 5)||!_tcsncmp(sCommand, TEXT("-t"), 2)) hR = Test();
			switch (hR) {
				case E_FAIL: {cout << "(failed)";break;}
				case E_NOTIMPL: {cout << "(not implemented)";break;}
				case S_OK: {cout << "(ok)";break;}
			}
			break;
		}
		default: {
			cout << "too many options\n";
		}
	}

	return 0;
}

STDAPI Test() {
	_THREADINIT;
	char sIn[1024];
	cin >> sIn;
	for (int i=0;i<10;i++) {
		IXMLDOMDocument3 *pDoc=0;
		HRESULT hResult=CoCreateInstance(__uuidof(FreeThreadedDOMDocument60), NULL, CLSCTX_INPROC_SERVER, IID_IXMLDOMDocument, (LPVOID*) &pDoc);
		pDoc->load("C:\\Users\\anewholm\\Documents\\Visual Studio 2005\\Projects\\general_server\\websites\\9carrots.org\\DBs\\test\\db.xml");
		pDoc->loadXML("");
		pDoc->Release();
	}
	cin >> sIn;
	for (int i=0;i<10;i++) {
		IXMLDOMDocument3 *pDoc=0;
		HRESULT hResult=CoCreateInstance(__uuidof(FreeThreadedDOMDocument60), NULL, CLSCTX_INPROC_SERVER, IID_IXMLDOMDocument, (LPVOID*) &pDoc);
		pDoc->load("C:\\Users\\anewholm\\Documents\\Visual Studio 2005\\Projects\\general_server\\websites\\9carrots.org\\DBs\\test\\db.xml");
		pDoc->loadXML("");
		pDoc->Release();
	}
	cin >> sIn;
	for (int i=0;i<10;i++) {
		IXMLDOMDocument3 *pDoc=0;
		HRESULT hResult=CoCreateInstance(__uuidof(FreeThreadedDOMDocument60), NULL, CLSCTX_INPROC_SERVER, IID_IXMLDOMDocument, (LPVOID*) &pDoc);
		pDoc->load("C:\\Users\\anewholm\\Documents\\Visual Studio 2005\\Projects\\general_server\\websites\\9carrots.org\\DBs\\test\\db.xml");
		pDoc->loadXML("");
		pDoc->Release();
	}
	cin >> sIn;
	_THREADKILL;
	return S_OK;
}

STDAPI RunServer() {
	_THREADINIT;
	pServer = new Server();
	char sCommand[1024];
	cin >> sCommand;
	delete pServer;
	return S_OK;
}

void stopService(DWORD status) {
	delete pServer;
	tServiceStatus.dwCurrentState       = SERVICE_STOPPED; 
	tServiceStatus.dwCheckPoint         = 0; 
	tServiceStatus.dwWaitHint           = 0; 
	tServiceStatus.dwWin32ExitCode      = status; 
	tServiceStatus.dwServiceSpecificExitCode = status; 
	SetServiceStatus(hServiceStatusHandle, &tServiceStatus); 
	_THREADKILL;
}

void WINAPI ServiceCtrlHandler(DWORD fdwControl) {
	//commands from the SCM (Service Control Manager)
	switch (fdwControl) {
		case SERVICE_CONTROL_STOP: {
			stopService(NULL);
			break;
	   }
	}
}

void WINAPI ServiceStart(DWORD argc, LPTSTR *argv) 
{
	//The SCM has called here to start the service (as requested by main)
	//get a handle for our service and give the control handler
	hServiceStatusHandle = RegisterServiceCtrlHandler(TEXT("general_server"), ServiceCtrlHandler); 
	if (hServiceStatusHandle == (SERVICE_STATUS_HANDLE)0) return; 

	//starting information
	tServiceStatus.dwServiceType        = SERVICE_WIN32; 
	tServiceStatus.dwCurrentState       = SERVICE_START_PENDING; 
	tServiceStatus.dwControlsAccepted   = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_PAUSE_CONTINUE; 
	tServiceStatus.dwWin32ExitCode      = 0; 
	tServiceStatus.dwServiceSpecificExitCode = 0; 
	tServiceStatus.dwCheckPoint         = 0; 
	tServiceStatus.dwWaitHint           = 0; 
	SetServiceStatus(hServiceStatusHandle, &tServiceStatus); 

	// Initialization code goes here. 
	_THREADINIT;
	//creates new threads for port listening
	//and returns after loaded all DBs and ready for requests
	pServer = new Server();

	// Initialization complete - report running status. 
	tServiceStatus.dwCurrentState       = SERVICE_RUNNING; 
	tServiceStatus.dwCheckPoint         = 0; 
	tServiceStatus.dwWaitHint           = 0; 
	SetServiceStatus(hServiceStatusHandle, &tServiceStatus);
	return; 
}

STDAPI DllRegisterServer(void) {
	//----------------------------------------------------------------
	//register the service with the Service Configuration Manager
	//regsvr32 general_server.exe
	cout << "registering service...\n";

	// Open a handle to the SC Manager database. 
	SC_HANDLE schSCManager = OpenSCManager(
		NULL,                    // local machine 
		NULL,                    // ServicesActive database 
		SC_MANAGER_ALL_ACCESS);  // full access rights 
	if (schSCManager==NULL) {
		//this could be because the command does not have admin privileges
		cout << "cannot get an OpenSCManager handle: do you have administrator rights?\n";
		return E_FAIL;
	}

	//get path to our EXE 
	TCHAR szPath[MAX_PATH]; 
	if (!GetModuleFileName(NULL, szPath, MAX_PATH)) return E_FAIL;
	
	//create service
	SC_HANDLE schService = CreateService( 
		schSCManager,              // SCManager database 
		TEXT("general_server"),		        // name of service 
		TEXT("general_server"),			     // service name to display 
		SERVICE_ALL_ACCESS,        // desired access 
		SERVICE_WIN32_OWN_PROCESS, // service type 
		SERVICE_DEMAND_START,      // start type 
		SERVICE_ERROR_NORMAL,      // error control type 
		TEXT("%SystemRoot%\\system32\\general_server.exe"),                    // path to service's binary 
		NULL,                      // no load ordering group 
		NULL,                      // no tag identifier 
		NULL,                      // no dependencies 
		NULL,                      // LocalSystem account 
		NULL);                     // no password 
	CloseServiceHandle(schService); 
	CloseServiceHandle(schSCManager);
	return S_OK;
}
STDAPI DllUnregisterServer(void) {
	cout << "un-registering service...\n";

	// Open a handle to the SC Manager database. 
	SC_HANDLE schSCManager = OpenSCManager( 
		NULL,                    // local machine 
		NULL,                    // ServicesActive database 
		SC_MANAGER_ALL_ACCESS);  // full access rights 
	if (schSCManager==NULL) return E_FAIL;
	SC_HANDLE schService=OpenService( 
		schSCManager,       // SCManager database 
		TEXT("general_server"), // name of service 
		DELETE);            // only need DELETE access 
	if (schService == NULL) return E_FAIL;
	if (!DeleteService(schService)) return E_FAIL;

	CloseServiceHandle(schService); 
	CloseServiceHandle(schSCManager);
	return S_OK;
}
