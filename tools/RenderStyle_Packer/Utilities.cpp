// Utilities.cpp

#include "stdafx.h"
#include "Utilities.h"

void OutputMsg(LPSTR fmt, ...)
{
    char buff[256]; va_list va;

    va_start( va, fmt ); wvsprintf( buff, fmt, va );
    va_end( va ); lstrcat(buff, "\r\n");
    MessageBox(NULL,buff,"Message Box",MB_OK);
}

void dprintf(LPSTR fmt, ...)
{
    char buff[256]; va_list va;

    va_start( va, fmt ); wvsprintf( buff, fmt, va );
    va_end( va ); lstrcat(buff, "\r\n");
    OutputDebugString(buff);
}

void DisplayLastWndError()
{
	LPVOID lpMsgBuf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,NULL,GetLastError(),MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),(LPTSTR)&lpMsgBuf,0,NULL);
	MessageBox(NULL,(LPCTSTR)lpMsgBuf,"Error",MB_OK|MB_ICONINFORMATION);
	LocalFree(lpMsgBuf);
}

