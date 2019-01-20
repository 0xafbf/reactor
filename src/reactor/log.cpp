
#include <stdarg.h>
#include <memory>
#include "stdio.h"
#include "log.h"

#ifdef WIN32
#include <windows.h>
#include <debugapi.h>
#endif


void rLog(rLogLevel log_level, string& format, ... )
{
  
  string fmt = format;
	va_list args;
	va_start(args, &fmt);

    size_t size = vsnprintf( nullptr, 0, format.c_str(), args ) + 1; // Extra space for '\0'
    std::unique_ptr<char[]> buf( new char[ size ] ); 
    vsnprintf( buf.get(), size, format.c_str(), args );
	va_end(args);
    let output = string( buf.get(), buf.get() + size - 1 )+'\n'; // We don't want the '\0' inside
#ifdef WIN32
	OutputDebugStringA(output.c_str());
	if (log_level >= rLogLevel::error)
	{
		DebugBreak();
	}
	if (log_level == rLogLevel::fatal)
	{
		exit(1);
	}
#else
	assert(false); //"incomplete"
	//printf()
#endif // WIN32

}
