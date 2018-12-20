
#include <stdarg.h>
#include <memory>
#include "log.h"

#ifdef WIN32
#include <windows.h>
#include <debugapi.h>
#endif

void rLog(rLogLevel log_level, const string& format, ... )
{
	va_list args;
	va_start(args, &format);

    size_t size = std::snprintf( nullptr, 0, format.c_str(), args ) + 1; // Extra space for '\0'
    std::unique_ptr<char[]> buf( new char[ size ] ); 
    std::snprintf( buf.get(), size, format.c_str(), args );
    let output = string( buf.get(), buf.get() + size - 1 )+'\n'; // We don't want the '\0' inside
#ifdef WIN32
	OutputDebugStringA(output.c_str());
#else
	assert(false); //"incomplete"
	//printf()
#endif // WIN32

}
