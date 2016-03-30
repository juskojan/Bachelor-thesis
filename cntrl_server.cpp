// cntrl_server.cpp : Defines the entry point for the console application.

#include "stdafx.h"
#include "cntrl_server.h"

/*
	CONSTRUCTOR
	- parses the command line argument
	- gets the ID of the test, finds out which browser will be used and searches for html file to open
	- should return successfully with filled class members (Browser, TestID, TestPath) and state code 1
	- in case of error state code is 99 and program ends (should not happen!)
*/
CNTR::CNTR(int argc, _TCHAR* argv[]){
	if (argc != 2){
		std::cout << "Argument not given!\n";
		this->TestState = ARGUMENT_ERROR;
		return;
	}

	// convert argument to int
	int n = _ttoi(argv[1]);
	// parse TestID and Browser ID
 	this->TestID = std::to_string((n % 1000)/100) + std::to_string((n % 100)/10) + std::to_string(n % 10);
	int Browser_ID = n / 1000;

	// find out which browser to use!!
	switch (Browser_ID){
		case 25:
			this->Browser = "firefox";
			break;
		case 30:
			this->Browser = "chrome";
			break;
		case 35:
			this->Browser = "iexplore";
			break;
		default:
			this->TestState = ARGUMENT_ERROR;
			return;
	}
	
	// search for html file path according to test ID
	std::ifstream fileInput;
	int offset = NULL;
	std::string line = "";
	std::string delimiter = "\t";
	// open file  to search
	fileInput.open("C:\\Users\\Jusko\\Desktop\\tests.txt");
	if (fileInput.is_open()) {
		while (getline(fileInput, line)) {
			if ((offset = line.find((this->TestID).c_str(), 0)) != std::string::npos) {
				this->TestPath = line.substr(line.find(delimiter) + 1, line.length()); 
			}
		}
		fileInput.close();
	}
	else {
		std::cout << "Unable to open file.";
		this->TestState = INPUT_FILE_ERROR;
		return;
	}
	// constructor successful!!!
	std::cout << "Constructor successful!\n";
	this->TestState = CONSTRUCTOR_SUCCESSFUL;
	return;
}

void CNTR::GetError(int errnum){

}

/*
	RESPONSES TO CLIENT
	- method called with arguments of string received from client and socket of client
	- parses the string and responds accordingly
	- if the received string is wrong/wrong seqnum/... returns with error code
*/
int CNTR::Parse_response(std::string response, SOCKET sock){
	
	//FIRST STRING that is supposed to be received
	if ((strcmp((response.substr(0, response.find('.'))).c_str(), "1") == 0)){
		if ((strcmp((response.substr(response.find('.') + 1, response.find(':') - 2)).c_str(), this->TestID.c_str()) == 0)){
			if ((strcmp((response.substr(response.find(':') + 1, response.length())).c_str(), "INIT") == 0)){
				std::cout << ">>> Test no. " << this->TestID << " initialized in browser " << this->Browser << ".\n";
				this->TestState = 2;
				// now respond
				int retval = send(sock, "1.ACK", sizeof("1.ACK"), 0);
				if (retval == SOCKET_ERROR)
				{
					fprintf(stderr, "Server: send() failed: error %d\n", WSAGetLastError());
				}
				else
					printf("Server: send() is OK.\n");
				// INIT OK
				return 2;
			}
			else{
				//wrong keyword
				this->TestState = 98;
				return 98;
			}
		}
		else{
			//wrong test ID
			this->TestState = 98;
			return 98;
		}
	}
	// SECOND STRING
	if ((strcmp((response.substr(0, response.find('.'))).c_str(), "2") == 0)){
		if ((strcmp((response.substr(response.find('.') + 1, response.find(':') - 2)).c_str(), this->TestID.c_str()) == 0)){
			if ((strcmp((response.substr(response.find(':') + 1, response.length())).c_str(), "SUCCESS") == 0)){
				std::cout << ">>> Test no. " << this->TestID << " is successful" << ".\n";

				int retval = send(sock, "2.ACK", sizeof("2.ACK"), 0);
				if (retval == SOCKET_ERROR)
				{
					fprintf(stderr, "Server: send() failed: error %d\n", WSAGetLastError());
				}
				else
					printf("Server: send() is OK.\n");

				this->TestState = 3;
				return 3;
				//TerminateProcess(this->h, 0);
			}
			else if ((strcmp((response.substr(response.find(':') + 1, response.length())).c_str(), "FAILURE") == 0)){
				std::cout << ">>> Test no. " << this->TestID << " has been prevented" << ".\n";

				int retval = send(sock, "2.ACK", sizeof("2.ACK"), 0);
				if (retval == SOCKET_ERROR)
				{
					fprintf(stderr, "Server: send() failed: error %d\n", WSAGetLastError());
				}
				else
					printf("Server: send() is OK.\n");

				this->TestState = 97;
				return 97;
			}
			else{
				//wrong keyword
				this->TestState = 97;
				return 97;
			}
		}
		else{
			//wrong test id
			this->TestState = 97;
			return 97;
		}
	}
	// THIRD STRING
	if ((strcmp((response.substr(0, response.find('.'))).c_str(), "3") == 0)){
		if ((strcmp((response.substr(response.find('.') + 1, response.find(':') - 2)).c_str(), this->TestID.c_str()) == 0)){
			if ((strcmp((response.substr(response.find(':') + 1, response.length())).c_str(), "END") == 0)){
				std::cout << ">>> Test no. " << this->TestID << " finished" << ".\n";

				int retval = send(sock, "3.ACK", sizeof("3.ACK"), 0);
				if (retval == SOCKET_ERROR)
				{
					fprintf(stderr, "Server: send() failed: error %d\n", WSAGetLastError());
				}
				else
					printf("Server: send() is OK.\n");

				this->TestState = 4;
				
				return 4;
				
			}
			else{
				//wrong keyword
				this->TestState = 96;
				return 96;
			}
		}
		else{
			//wrong ID
			this->TestState = 96;
			return 96;
		}
	}
	
	if (atoi((response.substr(0, response.find('.'))).c_str()) > 3){
		this->TestState = 95;
		return 95;
	}

	return 94;
}

/*
	SERVER
	- this method launches TCP server which listens for incoming communication from client(plugin)
	- communication is on localhost only(but future extension to INET easily possible)
	- method generates random free port given by WINDOWS which is used for communication
	- this PORT is the only thing client needs to know:
		- port is propagated to client via shared memory
	- accepted communication strings are processed by another method invoked from here
*/
int CNTR::Launch_Server() {
	/* Variables */
	char Buffer[128];
	int fromlen = NULL;
	int retval = NULL;
	struct sockaddr_in local, from;
	WSADATA wsaData;
	SOCKET listen_socket, msgsock;

	// Request Winsock version 2.2
	if ((retval = WSAStartup(0x202, &wsaData)) != 0)
	{
		fprintf(stderr, "Server: WSAStartup() failed with error %d\n", retval);
		WSACleanup();
		return WSA_ERROR;
	}
	else
		printf("Server: WSAStartup() is OK.\n");

	// Creating socket
	local.sin_family = AF_INET;
	local.sin_addr.s_addr = INADDR_ANY;
	// If the port is specified as zero, the service provider assigns a unique port to the application from the dynamic client port range.
	local.sin_port = 0;
	// TCP socket
	listen_socket = socket(AF_INET, SOCK_STREAM, 0);

	if (listen_socket == INVALID_SOCKET){
		fprintf(stderr, "Server: socket() failed with error %d\n", WSAGetLastError());
		WSACleanup();
		return WSA_ERROR;
	}
	else
		printf("Server: socket() is OK.\n");

	int ReceiveTimeout = 3000;
	int e = setsockopt(listen_socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&ReceiveTimeout, sizeof(int));


	// bind() associates a local address and port combination with the socket just created.
	int res = bind(listen_socket, (struct sockaddr*)&local, sizeof(local));
	if (res == SOCKET_ERROR)
	{
		fprintf(stderr, "Server: bind() failed with error %d\n", WSAGetLastError());
		WSACleanup();
		return WSA_ERROR;
	}
	else
		printf("Server: bind() is OK.\n");

	/* Get the port number that was randomly generated before */
	int Generated_Port = NULL;
	int addrlen = sizeof(local);
	if (!getsockname(listen_socket, (struct sockaddr *)&local, &addrlen))
	{
		Generated_Port = ntohs(local.sin_port);
		std::cout << "Generated port: " << Generated_Port << "\n";
	}
	else{
		fprintf(stderr, "Server: getsockname() failed with error %d\n", WSAGetLastError());
		WSACleanup();
		return WSA_ERROR;
	}

	//we obtained port number, lets write it into the shared memory
	CNTR::Propag_Port(std::to_string(Generated_Port));

	// listen()
	if (listen(listen_socket, 5) == SOCKET_ERROR)
	{
		fprintf(stderr, "Server: listen() failed with error %d\n", WSAGetLastError());
		WSACleanup();
		return -1;
	}
	else
		printf("Server: listen() is OK.\n");

	std::string received;
	
	// LOOP
	while (1)
	{
		fromlen = sizeof(from);
		msgsock = accept(listen_socket, (struct sockaddr*)&from, &fromlen);
		if (msgsock == INVALID_SOCKET)
		{
			fprintf(stderr, "Server: accept() error %d\n", WSAGetLastError());
			WSACleanup();
			return -1;
		}
		else
			printf("Server: accept() is OK.\n");
		printf("Server: accepted connection from %s \n", inet_ntoa(from.sin_addr));
		
		/*
		WSAOVERLAPPED ol;
		ol.hEvent = CreateEvent(...);
		WSARecv(,, &ol, NULL);
		HANDLE wait[] = { args.stopEvent, ol.hEvent };
		switch (WaitForMultipleObjects(_countof(wait), wait, FALSE, INFINITE))
		{
		case WAIT_OBJECT_0:
		// stopevent
		break;
		case WAIT_OBJECT_0+1:
		/// recv event
		}
		*/
		
		// In the case of SOCK_STREAM, the server can do recv() and send() on
		// the accepted socket and then close it.
		retval = recv(msgsock, Buffer, 128, 0);
		
		if (retval == SOCKET_ERROR)
		{
			fprintf(stderr, "Server: recv() failed: error %d\n", WSAGetLastError());
			closesocket(msgsock);
			continue;
		}
		else
			printf("Server: recv() is OK.\n");

		if (retval == 0)
		{
			printf("Server: Client closed connection.\n");
			closesocket(msgsock);
			continue;
		}

		Buffer[retval] = 0;
		printf("Server: Received %d bytes, data \"%s\" from client\n", retval, Buffer);

		received = std::string(Buffer);

		int ret = CNTR::Parse_response(received, msgsock);
		
		if (ret == 4){
			std::cout << "Communication successfully terminated.\n";
			closesocket(msgsock);
			WSACleanup();
			//TerminateProcess(this->h, 0);
			break;
		}
		else if (ret == 2 || ret == 3){
			continue;
		}
		else if (ret >= 94 && ret <= 98){
			std::cout << "Communication terminated with error.\n";
			closesocket(msgsock);
			WSACleanup();
			break;
		}
		
	}
	return 0;
}


/* Write ID:port string to the shared memory, client will know the path! */
int CNTR::Propag_Port(std::string Port){
	TCHAR szName[] = TEXT("MyFileMappingObject");
	TCHAR szPort[100];
	Port = this->TestID + ':' + Port;
	szPort[Port.length()] = 0;

	_tcscpy_s(szPort, CA2T(Port.c_str()));
	

	HANDLE hMapFile = CreateFileMapping(
		INVALID_HANDLE_VALUE,    // use paging file
		NULL,                    // default security
		PAGE_READWRITE,          // read/write access
		0,                       // maximum object size (high-order DWORD)
		256,	                 // maximum object size (low-order DWORD)
		szName);                 // name of mapping object

	if (hMapFile == NULL)
	{
		_tprintf(TEXT("Could not create file mapping object (%d).\n"),
			GetLastError());
		return 1;
	}
	LPCTSTR pBuf = (LPTSTR)MapViewOfFile(hMapFile,   // handle to map object
		FILE_MAP_ALL_ACCESS,				 // read/write permission
		0,
		0,
		256);

	if (pBuf == NULL)
	{
		_tprintf(TEXT("Could not map view of file (%d).\n"),
			GetLastError());

		CloseHandle(hMapFile);

		return 1;
	}
	
	CopyMemory((PVOID)pBuf, szPort, (_tcslen(szPort) * sizeof(TCHAR)));
	std::cout << "Written to memory: " << Port << "\n";
	
	//getchar();
	//UnmapViewOfFile(pBuf);

	//CloseHandle(hMapFile);

	return 0;
}

/*
	BROWSER LAUNCHER
		- starts new instance of desired web browser with html file
*/
HANDLE CNTR::Launch_Browser(){
	std::wstring tmp_str(this->TestPath.begin(), this->TestPath.end());
	LPCWSTR PATH = tmp_str.c_str();

	std::wstring tmp_str2(this->Browser.begin(), this->Browser.end());
	LPCWSTR BROWSER = tmp_str2.c_str();

	//ShellExecute(NULL, L"open", BROWSER, PATH, NULL, SW_SHOWNORMAL);


	SHELLEXECUTEINFO ExecuteInfo;

	memset(&ExecuteInfo, 0, sizeof(ExecuteInfo));

	ExecuteInfo.cbSize = sizeof(ExecuteInfo);
	ExecuteInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
	ExecuteInfo.hwnd = 0;
	ExecuteInfo.lpVerb = L"open";           // Operation to perform
	ExecuteInfo.lpFile = BROWSER;			// Application name
	ExecuteInfo.lpParameters = PATH;        // Additional parameters
	ExecuteInfo.lpDirectory = 0;            // Default directory
	ExecuteInfo.nShow = SW_SHOW;
	ExecuteInfo.hInstApp = 0;

	if (ShellExecuteEx(&ExecuteInfo) == FALSE){
		this->TestState = 90;
		return NULL;
	}

	//std::cout << ExecuteInfo.hProcess;
	//TerminateProcess(ExecuteInfo.hProcess, 0);
	//TerminateProcess(ExecuteInfo.hProcess, 1);
	// Could not start application -> call 'GetLastError()'

	return ExecuteInfo.hProcess;
}


int _tmain(int argc, _TCHAR* argv[])
{
	
	//get info from arg
	CNTR heh(argc, argv);

	if (heh.TestState != 1){
		heh.GetError(heh.TestState);
		return CONSTRUCTOR_ERROR;
	}

	std::thread t(&CNTR::Launch_Server, &heh);
	
	heh.h = heh.Launch_Browser();
	
	t.join();

	TerminateProcess(heh.h, 0);
	
	//DWORD id = GetCurrentProcessId();

	return 4;
}

