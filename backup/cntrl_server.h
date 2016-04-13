#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <windows.h>
// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
#include <string>
#include <thread>
#include <conio.h>
#include <tchar.h>

#include <fstream>

#include <atlstr.h>

#define WSA_ERROR 23;
#define FIREFOX 25;
#define CHROME 30;
#define EXPLORER 35;
#define DEFAULT_PROTO SOCK_STREAM


//TestStates values
#define CONSTRUCTOR_SUCCESSFUL 11;
#define CONSTRUCTOR_ERROR 101;
#define ARGUMENT_ERROR 100;
#define INPUT_FILE_ERROR 102;
#define MISSING_TEST_FILE 103;
#define TEST_SUCCESSFUL 1;
#define TEST_FAILED 0;

class CNTR {
	public:
		CNTR(int argc, _TCHAR* argv[]);
		int Launch_Server();
		int Propag_Port(std::string Port);
		HANDLE Launch_Browser();
		int Parse_response(std::string response, SOCKET sock);
		void GetError();
		int Simulate_Keystrokes();
		int Check_Log_File();

		int Check_ID(std::string response);
		int Send_ACK(std::string response, SOCKET sock);


		std::string TestID;
		std::string Browser;
		std::string TestPath;
		int TestState;
		HANDLE h;
};