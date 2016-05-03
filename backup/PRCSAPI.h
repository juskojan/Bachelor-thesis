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
		registerMethod("RunGeneralTest",      make_method(this, &PRCSAPI::RunGeneralTest));
		registerMethod("launched",  make_method(this, &PRCSAPI::launched));
		registerMethod("finalize",      make_method(this, &PRCSAPI::finalize));
		registerMethod("memory",    make_method(this, &PRCSAPI::memory));
		registerMethod("startprocess",    make_method(this, &PRCSAPI::startprocess));
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
	FB::variant RunGeneralTest(const FB::variant& executable);
	FB::variant finalize(void);
	FB::variant launched(void);
	FB::variant memory(void);
	FB::variant startprocess(void);


	bool PRCSAPI::doSomethingTimeConsuming( int num, FB::JSObjectPtr &callback );
	void PRCSAPI::doSomethingTimeConsuming_thread( int num, FB::JSObjectPtr &callback );
    
    // Event helpers
    FB_JSAPI_EVENT(test, 0, ());
    FB_JSAPI_EVENT(echo, 2, (const FB::variant&, const int));
	FB_JSAPI_EVENT(RunGeneralTest, 2, (const FB::variant&, const int));
	FB_JSAPI_EVENT(finalize, 2, (const FB::variant&, const int));
	FB_JSAPI_EVENT(launched, 2, (const FB::variant&, const int));
	FB_JSAPI_EVENT(memory, 2, (const FB::variant&, const int));
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
	
	HOST(std::string hostproc);				//constructor
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

