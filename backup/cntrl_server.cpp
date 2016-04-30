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

	this->h = NULL;
	this->TestID = "";
	this->Browser = "";
	this->TestPath = "";

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

	if (this->TestID == "" || this->Browser == ""){
		std::cout << "Argument could not be parsed!\n";
		this->TestState = ARGUMENT_ERROR;
		return;
	}
	
	/* search for html file path according to test ID */
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

	if (this->TestPath == ""){
		std::cout << "Test file was not found!\n";
		this->TestState = MISSING_TEST_FILE;
		return;
	}

	// constructor successful!!!
	std::cout << "Constructor successful, test no. " + this->TestID + " started!\n";
	this->TestState = CONSTRUCTOR_SUCCESSFUL;
	return;
}

/*
	PRINT ERROR MESSAGE
		-get internal state and print message accordingly
*/
void CNTR::GetError(){
	
}

/*Check if ID is valid in response from client*/
int CNTR::Check_ID(std::string response){
	if ((strcmp((response.substr(response.find('.') + 1, response.find(':') - 2)).c_str(), this->TestID.c_str()) == 0))
		return 1;

	return 0;
}
/*Return ack string to client*/
int CNTR::Send_ACK(std::string response, SOCKET sock){

	std::string ack = response + "-ACK";

	Sleep(1000);

	int retval = send(sock, ack.c_str(), sizeof(ack.c_str()), 0);
	if (retval == SOCKET_ERROR)
	{
		fprintf(stderr, "Server: send() failed: error %d\n", WSAGetLastError());
		return 0;
	}
	else
		printf("Server: send() is OK.\n");
	// ACK OK
	return 1;
}
/*Get current directory of executable*/
std::string GetExePath() {
	wchar_t buffer[MAX_PATH];
	GetModuleFileName(NULL, buffer, MAX_PATH);
	std::wstring ws(buffer);
	std::string path(ws.begin(), ws.end());
	std::string::size_type pos = path.find_last_of("\\/");
	
	return path.substr(0, pos+1);
}

/*
	RESPONSES TO CLIENT
	- method called with arguments of string received from client and socket of client
	- parses the string and responds accordingly
	- if the received string is wrong/wrong seqnum/... returns with error code
*/
int CNTR::Parse_response(std::string response, SOCKET sock){
	// check if test ID is right
	if (!CNTR::Check_ID(response)){
		std::cout << "Wrong ID.\n";
		this->TestState = 90;
		return 90;
	}

	//FIRST STRING that is supposed to be received is always INIT - return ack containing working directory
	if ((strcmp((response.substr(0, response.find('.'))).c_str(), "1") == 0)){
		if ((strcmp((response.substr(response.find(':') + 1, response.length())).c_str(), "INIT") == 0)){
			std::cout << ">>> Test no. " << this->TestID << " initialized in browser " << this->Browser << ".\n";
			// now respond with current working directory
			if (!CNTR::Send_ACK(response + "/" + GetExePath(), sock)){
				this->TestState = 91;
				return 91;
			}

			this->TestState = 2;
			return 2;
		}
		else{
			//wrong keyword
			this->TestState = 90;
			return 90;
		}
	}

	// Success/failure
	if ((strcmp((response.substr(response.find(':') + 1, response.length())).c_str(), "SUCCESS") == 0)){
		std::cout << ">>> Test no. " << this->TestID << " is successful" << ".\n";

		if (!CNTR::Send_ACK(response, sock)){
			this->TestState = 91;
			return 91;
		}
		//std::cout << ">>> Test no. " << this->TestID << " is successful ACK" << response <<".\n";
		this->TestState = 1;
		return 1;
	}
	else if ((strcmp((response.substr(response.find(':') + 1, response.length())).c_str(), "FAILURE") == 0)){
		std::cout << ">>> Test no. " << this->TestID << " has been prevented" << ".\n";

		if (!CNTR::Send_ACK(response, sock)){
			this->TestState = 91;
			return 91;
		}

		this->TestState = 0;
		return 0;
	}
	else if ((strcmp((response.substr(response.find(':') + 1, response.length())).c_str(), "END") == 0)){
		std::cout << ">>> Test no. " << this->TestID << " finished" << ".\n";

		if (!CNTR::Send_ACK(response, sock)){
			this->TestState = 91;
			return 91;
		}

		Sleep(1000);

		this->TestState = 4;
		return 4;	
	}
	//keylogger only
	else if (strcmp((response.substr(response.find(':') + 1, response.length())).c_str(), "KEYLOG_START") == 0){
		std::cout << ">>> Keylogger running.\n";
		//ack
		if (!CNTR::Send_ACK(response, sock)){
			this->TestState = 91;
			return 91;
		}
		
		//simulate keyboard input
		CNTR::Simulate_Keystrokes();

		//success?
		if (CNTR::Check_Log_File()){
			this->TestState = 1;
			return 1;
		}
		else{
			this->TestState = 0;
			return 0;
		}
	}

	return 92;
}

/*
	Simulate keystrokes for keylogger
*/
int CNTR::Simulate_Keystrokes(){
	INPUT ip;
	int i = 0;
	//	H E L L O \0
	unsigned char arr[6] = { 0x23, 0x12, 0x26, 0x26, 0x18, 0 };

	Sleep(5000);

	while (arr[i] != 0){
		//Set up the INPUT structure
		ip.type = INPUT_KEYBOARD;
		ip.ki.time = 0;
		ip.ki.wVk = 0; //We're doing scan codes instead
		ip.ki.dwExtraInfo = 0;

		//This let's you do a hardware scan instead of a virtual keypress
		ip.ki.dwFlags = KEYEVENTF_SCANCODE;
		ip.ki.wScan = arr[i++];  //Set a unicode character to use (A)

		//Send the press
		SendInput(1, &ip, sizeof(INPUT));

		//Prepare a keyup event
		ip.ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
		SendInput(1, &ip, sizeof(INPUT));
	}

	return 0;
}

/*
	Check if keylogger was successful
*/
int CNTR::Check_Log_File(){
	int length;
	std::ifstream filestr;

	filestr.open("C:\\Users\\Jusko\\Desktop\\log.txt", std::ios::binary); // open your file
	filestr.seekg(0, std::ios::end); // put the "cursor" at the end of the file
	length = filestr.tellg(); // find the position of the cursor
	filestr.close(); // close your file

	//std::remove("C:\\Users\\Jusko\\Desktop\\log.txt");

	if (length == 0){ 
		std::cout << "Keylogger not successful.\n";
		return 0;
	}
	else { 
		std::cout << "Keylogger successful.\n";
		return 1;
	}
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

	/*
	int ReceiveTimeout = 3000;
	int e = setsockopt(listen_socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&ReceiveTimeout, sizeof(int));
	*/

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
			break;
		}
		else if (ret >= 0 && ret <= 2){
			continue;
		}
		else if (ret >= 90 && ret <= 92){
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
	// fill variables and structure
	std::wstring tmp_str(this->TestPath.begin(), this->TestPath.end());
	LPCWSTR PATH = tmp_str.c_str();

	std::wstring tmp_str2(this->Browser.begin(), this->Browser.end());
	LPCWSTR BROWSER = tmp_str2.c_str();

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

/* M A I N */
int _tmain(int argc, _TCHAR* argv[])
{
	//get info from arg
	CNTR test(argc, argv);

	if (test.TestState != 1){
		test.GetError();
	}


	/*
	std::cout << "Browser: "	<< heh.Browser << "\n";
	std::cout << "TestID: "		<< heh.TestID << "\n";
	std::cout << "Path: "		<< heh.TestPath << "\n";
	std::cout << "State: "		<< heh.TestState << "\n";
	std::cout << "Handle: "		<< heh.h << "\n";
	*/


	std::thread t(&CNTR::Launch_Server, &test);
	
	test.h = test.Launch_Browser();
	
	t.join();

	// kill browser
	//TerminateProcess(heh.h, 0);


	return 4;
}

