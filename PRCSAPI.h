/**********************************************************\

  Auto-generated PRCSAPI.h

\**********************************************************/

#include <string>
#include <sstream>
#include <boost/weak_ptr.hpp>
#include "JSAPIAuto.h"
#include "BrowserHost.h"
#include "PRCS.h"

#include <windows.h> 
#include <tchar.h>
#include <stdio.h> 
#include <strsafe.h>
#include <string>
#include <iostream>
#include <fstream>
#include <atlstr.h>


#ifndef H_PRCSAPI
#define H_PRCSAPI

class PRCSAPI : public FB::JSAPIAuto
{
public:
    ////////////////////////////////////////////////////////////////////////////
    /// @fn PRCSAPI::PRCSAPI(const PRCSPtr& plugin, const FB::BrowserHostPtr host)
    ///
    /// @brief  Constructor for your JSAPI object.
    ///         You should register your methods, properties, and events
    ///         that should be accessible to Javascript from here.
    ///
    /// @see FB::JSAPIAuto::registerMethod
    /// @see FB::JSAPIAuto::registerProperty
    /// @see FB::JSAPIAuto::registerEvent
    ////////////////////////////////////////////////////////////////////////////
    PRCSAPI(const PRCSPtr& plugin, const FB::BrowserHostPtr& host) :
        m_plugin(plugin), m_host(host)
    {
        registerMethod("echo",      make_method(this, &PRCSAPI::echo));
		registerMethod("launched",  make_method(this, &PRCSAPI::launched));
		registerMethod("finalize",      make_method(this, &PRCSAPI::finalize));
		registerMethod("memory",    make_method(this, &PRCSAPI::memory));
		registerMethod("keylogger",    make_method(this, &PRCSAPI::keylogger));
        registerMethod("testEvent", make_method(this, &PRCSAPI::testEvent));
		registerMethod("doSomethingTimeConsuming", make_method(this, &PRCSAPI::doSomethingTimeConsuming));
		
        
        // Read-write property
        registerProperty("testString",
                         make_property(this,
                                       &PRCSAPI::get_testString,
                                       &PRCSAPI::set_testString));
        
        // Read-only property
        registerProperty("version",
                         make_property(this,
                                       &PRCSAPI::get_version));
    }



    ///////////////////////////////////////////////////////////////////////////////
    /// @fn PRCSAPI::~PRCSAPI()
    ///
    /// @brief  Destructor.  Remember that this object will not be released until
    ///         the browser is done with it; this will almost definitely be after
    ///         the plugin is released.
    ///////////////////////////////////////////////////////////////////////////////
    virtual ~PRCSAPI() {};

    PRCSPtr getPlugin();

    // Read/Write property ${PROPERTY.ident}
    std::string get_testString();
    void set_testString(const std::string& val);

    // Read-only property ${PROPERTY.ident}
    std::string get_version();

    // Method echo
    FB::variant echo(const FB::variant& msg);
	FB::variant finalize(const FB::variant& msg);
	FB::variant launched(void);
	FB::variant memory(void);
	FB::variant keylogger(void);


	bool PRCSAPI::doSomethingTimeConsuming( int num, FB::JSObjectPtr &callback );
	void PRCSAPI::doSomethingTimeConsuming_thread( int num, FB::JSObjectPtr &callback );
    
    // Event helpers
    FB_JSAPI_EVENT(test, 0, ());
    FB_JSAPI_EVENT(echo, 2, (const FB::variant&, const int));
	FB_JSAPI_EVENT(finalize, 2, (const FB::variant&, const int));
	FB_JSAPI_EVENT(launched, 2, (const FB::variant&, const int));
	FB_JSAPI_EVENT(memory, 2, (const FB::variant&, const int));
	FB_JSAPI_EVENT(keylogger, 2, (const FB::variant&, const int));

    // Method test-event
    void testEvent();

private:
    PRCSWeakPtr m_plugin;
    FB::BrowserHostPtr m_host;

    std::string m_testString;
};

/*
H
O
S
T
*/

/*
	CLASS HOST
	- launching child processes and communicating with them
*/
class HOST{
public:

	/*
	CONSTRUCTOR:
	- create pipes for communication with child
	- child IN/OUT
	*/
	HOST(std::string hostproc){
		std::cout << "Constructor:\n";

		//assign exe name
		hostname = hostproc;

		g_hChildStd_IN_Rd = NULL;
		g_hChildStd_IN_Wr = NULL;
		g_hChildStd_OUT_Rd = NULL;
		g_hChildStd_OUT_Wr = NULL;

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


	void ErrorExit(PTSTR lpszFunction){
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
	void CreateChild(){
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
			CloseHandle(piProcInfo.hProcess);
			CloseHandle(piProcInfo.hThread);
		}
	}


	std::string ReadFromPipe(){
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


	void WriteToPipe(std::string what){
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

	std::string hostname;
	HANDLE g_hChildStd_IN_Rd;
	HANDLE g_hChildStd_IN_Wr;
	HANDLE g_hChildStd_OUT_Rd;
	HANDLE g_hChildStd_OUT_Wr;
};



#endif // H_PRCSAPI

