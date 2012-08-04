// Debug.cpp - debug utilities..

#include <stdio.h>
#include <windows.h>

void DebugTraceFn(const char *st, ...)
#ifdef _DEBUG
{
	va_list args;
	va_start(args, st);
	char buffer[512];
	vsprintf(buffer, st, args);
	OutputDebugString(buffer);
	va_end(args);
}
#else
{
}
#endif

