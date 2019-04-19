
#include <stdarg.h>
#include <memory>
#include "stdio.h"
#include "log.h"
#include "debug.h"

#ifdef WIN32
#include <windows.h>
#include <debugapi.h>
#endif


void rLog(rLogLevel log_level, const string format, ... )
{
	let format_c_str = format.c_str();
    va_list args;
	va_start(args, format);
	size_t size = vsnprintf( nullptr, 0, format_c_str, args ) + 1; // Extra space for '\0'
	va_end(args); // in gcc, calling vsnprintf without va_end+va_start crashes
	
	let buf = (char*)malloc(size);// buf( new char[ size ] ); 
	va_start(args, format);
	vsnprintf( buf, size, format_c_str, args );
    va_end(args);
    
	let output = string( buf, buf + size - 1 )+'\n'; // We don't want the '\0' inside
    free(buf);

#ifdef WIN32
	OutputDebugStringA(output.c_str());
#else
	printf(output.c_str());
#endif // WIN32

	if (log_level == rLogLevel::fatal)
	{
		rBreak();
		exit(1);
	}
}
