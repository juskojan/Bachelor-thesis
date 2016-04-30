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


#define WSA_ERROR 23
#define FIREFOX 25
#define CHROME 30
#define EXPLORER 35
#define DEFAULT_PROTO SOCK_STREAM

#define BUFLEN 128

//TestStates values

#define WRONG_REQUEST 90
#define FAILED_TO_ACK 91

#define ARGUMENT_ERROR 100
#define CONSTRUCTOR_ERROR 101
#define CONSTRUCTOR_SUCCESSFUL 102
#define INPUT_FILE_ERROR 103
#define MISSING_TEST_FILE 104

#define TEST_FAILED 0
#define TEST_SUCCESSFUL 1
#define INIT_RECEIVED 2
#define END_RECEIVED 4


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

		int Send_ACK(std::string response, SOCKET sock);


		std::string TestID;
		std::string Browser;
		std::string TestPath;
		int TestState;
		BOOL TestSuccess;
		HANDLE h;

		std::string MyDirectory;
};