// ----------------------------------------------------------------------- //
//
// MODULE  : stacktrace.cpp
//
// PURPOSE : This entire file was taken from the Microsoft Systems Journal
//           Bugslayer column.  It is used to do a stack trace in debug
//			 builds only.
//
// CREATED : 6/27/01
//
// (c) 2001-2002 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include <stdafx.h>

// Only used in debug builds of the engine...

#ifdef _DEBUG

#include "stacktrace.h"
#include <vector>

// The address typedef.
typedef std::vector<ULONG> ADDRVECTOR ;

//====================================================================================
//
// Everything below this line was taken from John Robbins' 
// Microsoft Systems Journal BugSlayer column:
// http://www.microsoft.com/msj/defaulttop.asp?page=/msj/0299/bugslayer/bugslayer0299top.htm
//
//====================================================================================

// The symbol engine.
static CSymbolEngine g_cSym ;

// If TRUE, the symbol engine has been initialized.
static BOOL g_bSymIsInit = FALSE ;

#ifdef _M_IX86
static DWORD __stdcall GetModBase ( HANDLE hProcess , DWORD dwAddr )
#else
static DWORD64 __stdcall GetModBase ( HANDLE hProcess , DWORD64 dwAddr )
#endif
{
    // Check in the symbol engine first.
    IMAGEHLP_MODULE stIHM ;

    // This is what the MFC stack trace routines forgot to do so their
    //  code will not get the info out of the symbol engine.
    stIHM.SizeOfStruct = sizeof ( IMAGEHLP_MODULE ) ;

    if ( g_cSym.SymGetModuleInfo ( dwAddr , &stIHM ) )
    {
        return ( stIHM.BaseOfImage ) ;
    }
    else
    {
        // Let's go fishing.
        MEMORY_BASIC_INFORMATION stMBI ;

        if ( 0 != VirtualQueryEx ( hProcess         ,
                                   (LPCVOID)dwAddr  ,
                                   &stMBI           ,
                                   sizeof ( stMBI )  ) )
        {
            // Try and load it.
            DWORD dwNameLen = 0 ;
            TCHAR szFile[ MAX_PATH ] ;

            dwNameLen = GetModuleFileName ( (HINSTANCE)
                                                stMBI.AllocationBase ,
                                            szFile                   ,
                                            MAX_PATH                  );

            HANDLE hFile = NULL ;

            if ( 0 != dwNameLen )
            {
                hFile = CreateFile ( szFile       ,
                                     GENERIC_READ    ,
                                     FILE_SHARE_READ ,
                                     NULL            ,
                                     OPEN_EXISTING   ,
                                     0               ,
                                     0                ) ;
            }

            g_cSym.SymLoadModule ( hFile                            ,
                                   ( dwNameLen ? szFile : NULL )    ,
                                   NULL                             ,
                                   (DWORD)stMBI.AllocationBase      ,
                                   0                                 ) ;
            return ( (DWORD)stMBI.AllocationBase ) ;
        }
    }

    return ( 0 ) ;
}

static DWORD ConvertAddress ( DWORD dwAddr , LPTSTR szOutBuff )
{
    char szTemp [ MAX_PATH + sizeof ( IMAGEHLP_SYMBOL ) ] ;

    PIMAGEHLP_SYMBOL pIHS = (PIMAGEHLP_SYMBOL)&szTemp ;

    IMAGEHLP_MODULE stIHM ;

    LPTSTR pCurrPos = szOutBuff ;

    ZeroMemory ( pIHS , MAX_PATH + sizeof ( IMAGEHLP_SYMBOL ) ) ;
    ZeroMemory ( &stIHM , sizeof ( IMAGEHLP_MODULE ) ) ;

    pIHS->SizeOfStruct = sizeof ( IMAGEHLP_SYMBOL ) ;
    pIHS->Address = dwAddr ;
    pIHS->MaxNameLength = MAX_PATH ;

    stIHM.SizeOfStruct = sizeof ( IMAGEHLP_MODULE ) ;

    // Get the module name.
    if ( 0 != g_cSym.SymGetModuleInfo ( dwAddr , &stIHM ) )
    {
        // Strip off the path.
        LPTSTR szName = _tcsrchr ( stIHM.ImageName , _T ( '\\' ) ) ;
        if ( NULL != szName )
        {
            szName++ ;
        }
        else
        {
            szName = stIHM.ImageName ;
        }
        pCurrPos += wsprintf ( pCurrPos , _T ( "%s: " ) , szName ) ;
    }
    else
    {
        pCurrPos += wsprintf ( pCurrPos , _T ( "<unknown module>: " ) );
    }

    // Get the function.
#ifdef _M_IX86
    DWORD dwDisp ;
#else
	DWORD64 dwDisp ;
#endif
    if ( 0 != g_cSym.SymGetSymFromAddr ( dwAddr , &dwDisp , pIHS ) )
    {
        pCurrPos += wsprintf ( pCurrPos , _T ( "%s() " ) , pIHS->Name);

		// If I got a symbol, give the source and line a whirl.
        IMAGEHLP_LINE stIHL ;

        ZeroMemory ( &stIHL , sizeof ( IMAGEHLP_LINE ) ) ;

        stIHL.SizeOfStruct = sizeof ( IMAGEHLP_LINE ) ;

        if ( 0 != g_cSym.SymGetLineFromAddr ( dwAddr  ,
                                              (PDWORD)&dwDisp ,
                                              &stIHL   ) )
        {
            // Put this on the next line and indented a bit.
            pCurrPos += wsprintf ( pCurrPos                  ,
                                  _T ( "%s, Line %d" ) ,
                                  stIHL.FileName             ,
                                  stIHL.LineNumber            ) ;
        }
    }
    else
    {
        pCurrPos += wsprintf ( pCurrPos , _T ( "<unknown symbol>" ) ) ;
    }

    // Tack on a CRLF.
    pCurrPos += wsprintf ( pCurrPos , _T ( "\n" ) ) ;

    return ( pCurrPos - szOutBuff ) ;
}


// [KLS 4/5/02] Turn off optimizations on DoStackTrace.  This is necessary because there is 
// a compiler optimization bug that causes the szString passed in to be filled with garbage 
// if this function is called from a debug version of the game code but was compiled into a
// release version of the engine (which of course happens all the time ;).  See the comment
// inside the fuction for the bit of code that is having "issues"...
#pragma optimize("", off)
#pragma optimize("q", off)

void DoStackTrace ( LPTSTR szString  ,
                    DWORD  dwSize    ,
                    DWORD  dwNumSkip  )
{
    HANDLE hProcess = GetCurrentProcess ( ) ;

    // If the symbol engine is not initialized, do it now.
    if ( FALSE == g_bSymIsInit )
    {
        DWORD dwOpts = SymGetOptions ( ) ;

        // Turn on load lines.
        SymSetOptions ( dwOpts                |
                        SYMOPT_LOAD_LINES      ) ;

        if ( FALSE == g_cSym.SymInitialize ( hProcess ,
                                             NULL     ,
                                             FALSE     ) )
        {
            //dsi_PrintToConsole("DoStackTrace : Unable to initialize the symbol engine!!!");
        }
        else
        {
            g_bSymIsInit = TRUE ;
        }
    }

    // The symbol engine is initialized so do the stack walk.

    // The array of addresses.
    ADDRVECTOR vAddrs ;

    // The thread information.
    CONTEXT    stCtx  ;

    stCtx.ContextFlags = CONTEXT_FULL ;

    if ( GetThreadContext ( GetCurrentThread ( ) , &stCtx ) )
    {
        STACKFRAME stFrame ;
        DWORD      dwMachine ;

        ZeroMemory ( &stFrame , sizeof ( STACKFRAME ) ) ;

        stFrame.AddrPC.Mode = AddrModeFlat ;

#ifdef _M_IX86
        dwMachine                = IMAGE_FILE_MACHINE_I386 ;
        stFrame.AddrPC.Offset    = stCtx.Eip    ;
        stFrame.AddrStack.Offset = stCtx.Esp    ;
#else
		dwMachine				 = IMAGE_FILE_MACHINE_AMD64 ;
		stFrame.AddrPC.Offset	 = stCtx.Rip	;
		stFrame.AddrStack.Offset = stCtx.Rsp	;
#endif
        stFrame.AddrStack.Mode   = AddrModeFlat ;
#ifdef _M_IX86
        stFrame.AddrFrame.Offset = stCtx.Ebp    ;
#else
		stFrame.AddrFrame.Offset = stCtx.Rbp	;
#endif
        stFrame.AddrFrame.Mode   = AddrModeFlat ;

        // Loop for the first 512 stack elements.
        for ( DWORD i = 0 ; i < 512 ; i++ )
        {
            if ( FALSE == StackWalk ( dwMachine              ,
                                      hProcess               ,
                                      hProcess               ,
                                      &stFrame               ,
                                      &stCtx                 ,
                                      NULL                   ,
                                      SymFunctionTableAccess ,
                                      GetModBase             ,
                                      NULL                    ) )
            {
				// [KLS 4/5/02] The optimization bug doesn't occur if you touch the 
				// variable "i" here (e.g., call dsi_PrintToConsole("i = %d", i); )
				// ...I love compiler bugs ;)

				break ;
            }
            if ( i > dwNumSkip )
            {
                // Also check that the address is not zero.  Sometimes
                //  StackWalk returns TRUE with a frame of zero.
                if ( 0 != stFrame.AddrPC.Offset )
                {
                    vAddrs.push_back ( stFrame.AddrPC.Offset ) ;
                }
            }
        }

        // Now start converting the addresses.
        DWORD dwSizeLeft = dwSize ;
        DWORD dwSymSize ;

        TCHAR szSym [ MAX_PATH * 2 ] ;
        LPTSTR szCurrPos = szString ;

        ADDRVECTOR::iterator loop ;
        for ( loop =  vAddrs.begin ( ) ;
              loop != vAddrs.end ( )   ;
              loop++                     )
        {

            dwSymSize = ConvertAddress ( *loop , szSym ) ;
            if ( dwSizeLeft < dwSymSize )
            {
                break ;
            }
            _tcscpy ( szCurrPos , szSym ) ;
            szCurrPos += dwSymSize ;
            dwSizeLeft -= dwSymSize ;
        }
    }
}

// [KLS 4/5/02] Make sure we turn optimizations back on! ;)
#pragma optimize("", on)
#pragma optimize("q", on)

#else

void DoStackTrace ( LPTSTR szString  ,
                    DWORD  dwSize    ,
                    DWORD  dwNumSkip  )
{
	sprintf(szString, "DoStackTrace not supported in release builds");
}

#endif // _DEBUG
