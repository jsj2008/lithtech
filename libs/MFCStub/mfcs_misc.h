// mfcs_misc.h - Misc. types and defines for the MFC stub

#ifndef __MFCS_MISC_H__
#define __MFCS_MISC_H__

// Note : Trace would be much better off with an actual implementation...
inline void MFCStubTrace(const char *pFormat, ...)
{
#ifdef OutputDebugString
	char szOut[500];
	va_list marker;

	va_start(marker, pFormat);
	_vsnprintf(szOut, 499, pFormat, marker);
	va_end(marker);

	OutputDebugString( szOut );
#endif // OutputDebugString
}

#ifdef _DEBUG
#define TRACE MFCStubTrace
#else
#define TRACE (void)0
#endif

#define DEBUG_NEW new

#endif // __MFCS_MISC_H__