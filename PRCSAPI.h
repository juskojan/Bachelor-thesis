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
    /// @see FB::JSAPIAuto::registerEvent  terminateProcess
    ////////////////////////////////////////////////////////////////////////////
    PRCSAPI(const PRCSPtr& plugin, const FB::BrowserHostPtr& host) :
        m_plugin(plugin), m_host(host)
    {
        registerMethod("CMDproc",      make_method(this, &PRCSAPI::CMDproc));
		registerMethod("RunGeneralTest",      make_method(this, &PRCSAPI::RunGeneralTest));
		registerMethod("launched",  make_method(this, &PRCSAPI::launched));
		registerMethod("finalize",      make_method(this, &PRCSAPI::finalize));
		registerMethod("memory",    make_method(this, &PRCSAPI::memory));
		registerMethod("WriteMemory",    make_method(this, &PRCSAPI::WriteMemory));
		registerMethod("terminateProcess",    make_method(this, &PRCSAPI::terminateProcess));
		registerMethod("startprocess",    make_method(this, &PRCSAPI::startprocess));
        registerMethod("testEvent", make_method(this, &PRCSAPI::testEvent));
		registerMethod("keylogger", make_method(this, &PRCSAPI::Keylogger));
		
        
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
    FB::variant CMDproc(void);
	FB::variant RunGeneralTest(std::string executable);
	FB::variant finalize(void);
	FB::variant launched(void);
	FB::variant memory(void);
	FB::variant WriteMemory(void);
	FB::variant terminateProcess(std::string TestExe);
	FB::variant startprocess(void);


	bool PRCSAPI::Keylogger( int num, FB::JSObjectPtr &callback );
	void PRCSAPI::Keylogger_thread( int num, FB::JSObjectPtr &callback );
    
    // Event helpers
    FB_JSAPI_EVENT(test, 0, ());
    FB_JSAPI_EVENT(CMDproc, 2, (const FB::variant&, const int));
	FB_JSAPI_EVENT(RunGeneralTest, 2, (const FB::variant&, const int));
	FB_JSAPI_EVENT(finalize, 2, (const FB::variant&, const int));
	FB_JSAPI_EVENT(launched, 2, (const FB::variant&, const int));
	FB_JSAPI_EVENT(memory, 2, (const FB::variant&, const int));
	FB_JSAPI_EVENT(WriteMemory, 2, (const FB::variant&, const int));
	FB_JSAPI_EVENT(terminateProcess, 2, (const FB::variant&, const int));
	FB_JSAPI_EVENT(startprocess, 2, (const FB::variant&, const int));

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
T	CLASS
*/

/*
	CLASS HOST
	- launching child processes and communicating with them
*/
class HOST{
public:
	
	HOST(std::string hostproc, BOOL value);	//constructor
	~HOST();								//destructor
	void CreateChild();						//create child process
	std::string ReadFromPipe();				//read from child STDOUT
	BOOL WriteToPipe(std::string what);		//write to child's STDIN
	int GetExitCode();						//fetch return value
	
	std::string hostname;					//path to host process's executable
	HANDLE hChildStdIN_Rd;					//handles to pipes
	HANDLE hChildStdIN_Wr;
	HANDLE hChildStdOUT_Rd;
	HANDLE hChildStdOUT_Wr;

	int value;
	HANDLE host_process;					//handle for host process
	HANDLE host_thread;						//handle for host thread
	int State;
};


class COMM{
public:
	COMM(std::string Buff);
	int Communicate(void);

	int Port_number;
	std::string Buffer;
	std::string Received;
};



#endif // H_PRCSAPI

