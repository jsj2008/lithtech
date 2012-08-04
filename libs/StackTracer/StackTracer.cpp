#include "stdafx.h"

#include "StackTracer.h"

#include <dbghelp.h>
#include <stdio.h>
#include <vector>
#include <string>
#include <io.h>
#include "stringtokenizer.h"

//////////////////////////////////////////////////////////////////////////
// If we're compiling on anything less than VC .NET 2003, then we need to 
// define this first. .NET 2003's version of ImageHlp.h has this addition.
#if _MSC_VER < 1310
	#define MAX_SYM_NAME 2000

	typedef struct _SYMBOL_INFO_PACKAGE {
		SYMBOL_INFO si;
		CHAR        name[MAX_SYM_NAME + 1];
	} SYMBOL_INFO_PACKAGE, *PSYMBOL_INFO_PACKAGE;
#endif

///////////////////////////////////////////////////////////////////////////////
// Directives 
//

#pragma comment( lib, "dbghelp.lib" )


struct CImageHlpLine64 : public IMAGEHLP_LINE64 
{
	CImageHlpLine64() 
	{
		SizeOfStruct = sizeof(IMAGEHLP_LINE64); 
	}
}; 

///////////////////////////////////////////////////////////////////////////////
// CSymbolInfoPackage class declaration 
// 
// Wrapper for SYMBOL_INFO_PACKAGE structure 
//

struct CSymbolInfoPackage : public SYMBOL_INFO_PACKAGE 
{
	CSymbolInfoPackage() 
	{
		si.SizeOfStruct = sizeof(SYMBOL_INFO); 
		si.MaxNameLen   = sizeof(name); 
	}
};

bool GetFileParams( const char* pFileName, DWORD64& BaseAddr, DWORD& FileSize );
bool GetFileSize( const char* pFileName, DWORD& FileSize );

///////////////////////////////////////////////////////////////////////////////
// CStackWalk class declaration 
//

class CStackWalk 
{
public: 

	// Helper types  

	struct CStackFrame
	{
		DWORD64 Ip;       // Instruction pointer (EIP on x86) 
		DWORD64 RetAddr;  // Return address 
		DWORD64 Bp;       // Stack base pointer (EBP on x86)

		CStackFrame( DWORD64 _Ip, DWORD64 _RetAddr, DWORD64 _Bp ) 
			: Ip( _Ip ), RetAddr( _RetAddr ), Bp( _Bp ) 
		{}
	};

	typedef std::vector<CStackFrame> FrameColl_t;


public: 

	// Constructors / destructor 

	CStackWalk( HANDLE hProcess = GetCurrentProcess(), HANDLE hThread = GetCurrentThread() ); 
	~CStackWalk(); 


	// Operations 

	// Walk the stack 
	bool Walk( CONTEXT* pContext = 0 ); 


	// Accessors 

	// Call stack 
	FrameColl_t CallStack() const 
	{ return m_CallStack; }

	// Last error code 
	DWORD LastError() const 
	{ return m_LastError; }

	// GetModuleBase function 
	void SetModuleBaseFunc( PGET_MODULE_BASE_ROUTINE64 pFunc ) 
	{ m_pfnGetModBase = pFunc; }


private: 

	// Copy protection 
	CStackWalk( const CStackWalk& );
	CStackWalk& operator=( const CStackWalk& );


private: 

	// Data members 

	// Process handle 
	HANDLE m_hProcess; 

	// Thread handle 
	HANDLE m_hThread; 

	// Call stack 
	FrameColl_t m_CallStack; 

	// Last error code 
	DWORD m_LastError; 

	// User-supplied GetModuleBase function (optional) 
	PGET_MODULE_BASE_ROUTINE64 m_pfnGetModBase; 

};


///////////////////////////////////////////////////////////////////////////////
// CStackWalk class implementation 
//

// Bring in _ReturnAddress intrinsic 

#ifdef __cplusplus
extern "C"
#endif
void* _ReturnAddress(void);

#pragma intrinsic(_ReturnAddress)


// GetCallerAddress() helper function 

static void* GetCallerAddress(); 

#pragma optimize ( "", off )
void* GetCallerAddress()
{
	return _ReturnAddress();
}
#pragma optimize ( "", on )


// Constructor 

CStackWalk::CStackWalk( HANDLE hProcess, HANDLE hThread ) 
: m_hProcess( hProcess ), m_hThread( hThread ), m_LastError( 0 ), 
m_pfnGetModBase( 0 ) 
{
}


// Destructor 

CStackWalk::~CStackWalk() 
{
	// no actions
}


// Stack walker function 

// Turn off optimizations to make sure that frame pointer is present 
#pragma optimize ( "", off )

bool CStackWalk::Walk( CONTEXT* pContext ) 
{
	// Obtain an address in the address range of this function 
	// _after_ its stack frame has been constructed. 
	// (We cannot just use the function's address, because it is before 
	// the stack frame construction) 

	DWORD64 MyAddress = (DWORD64)GetCallerAddress(); 


	// Cleanup 

	m_LastError = 0; 

	m_CallStack.clear(); 

	SetLastError( 0 ); 


	// Collect the data needed by StackWalk64 

	// Machine type 

	DWORD MachineType = 0; 

	// Stack frame 

	STACKFRAME64 StackFrame; 
	memset( &StackFrame, 0, sizeof(StackFrame) );

	// Architecture-specific initialization 

#ifdef _M_IX86

	// Machine type 

	MachineType = IMAGE_FILE_MACHINE_I386;

	// STACKFRAME64 structure 

	if( pContext != 0 ) 
	{
		StackFrame.AddrPC.Offset      = pContext->Eip;
		StackFrame.AddrPC.Mode        = AddrModeFlat;
		StackFrame.AddrStack.Offset   = pContext->Esp;
		StackFrame.AddrStack.Mode     = AddrModeFlat;
		StackFrame.AddrFrame.Offset   = pContext->Ebp;
		StackFrame.AddrFrame.Mode     = AddrModeFlat;
	}
	else 
	{
		// Initialize the stack frame structure so that StackWalk64 
		// attempts to walk the stack above the current function only, 
		// excluding this function (Walk) itself. 
		// 
		// This is to avoid modifying the stack frame of the current function 
		// between subsequent calls to StackWalk64, which IMO can affect 
		// the possibility to walk the stack successfully. 
		// 

		unsigned long StackPtr;
		unsigned long BasePtr; 

		__asm mov [StackPtr], esp
			__asm mov [BasePtr], ebp

			StackFrame.AddrPC.Offset      = MyAddress; 
		StackFrame.AddrPC.Mode        = AddrModeFlat;
		StackFrame.AddrStack.Offset   = StackPtr; 
		StackFrame.AddrStack.Mode     = AddrModeFlat;
		StackFrame.AddrFrame.Offset   = BasePtr; 
		StackFrame.AddrFrame.Mode     = AddrModeFlat;
	}

#else

#error This architecture is not supported.

#endif //_M_IX86

	// GetModuleBase function 

	PGET_MODULE_BASE_ROUTINE64 pfnGetModBase = m_pfnGetModBase; 

	if( pfnGetModBase == 0 ) 
		pfnGetModBase = SymGetModuleBase64; 

	// If we obtained the context ourselves, we have to skip 
	// the first frame (the current function - Walk) 

	bool bSkipFirst = ( pContext == 0 ); 


	// Walk the stack 

	while( 1 ) 
	{
		// Reset last error code 

		SetLastError( 0 ); 


		// Call StackWalk64 

		if( !StackWalk64( 
			MachineType,  // Machine architecture type 
			m_hProcess,   // Process handle 
			m_hThread,    // Thread handle 
			&StackFrame,  // Stack frame 
			0,            // Thread context (not needed for x86)
			0,            // Read memory function - not used 
			SymFunctionTableAccess64,  // Function table access function (FPO access on x86) 
			pfnGetModBase, //SymGetModuleBase64, // Function that can determine module base for the given address 
			0             // Address translation function - not user 
			) )
		{
			// StackWalk64 failed 
			m_LastError = GetLastError(); 
			break; 
		}


		// Check the stack frame 

		if( StackFrame.AddrFrame.Offset == 0 ) 
		{
			// Invalid frame 
			break; 
		}

		bool bSaveFrame = true; 

		if( StackFrame.AddrPC.Offset == 0 ) 
		{
			// Do not save it 
			bSaveFrame = false; 
		}

		if( StackFrame.AddrPC.Offset == StackFrame.AddrReturn.Offset ) 
		{
			// Do not save it 
			bSaveFrame = false; 
		}

		if( bSkipFirst ) 
		{
			// Do not save it 
			bSaveFrame = false; 
			bSkipFirst = false; 
		}


		// Save the stack frame 

		if( bSaveFrame ) 
		{
			CStackFrame NewFrame( StackFrame.AddrPC.Offset, 
				StackFrame.AddrReturn.Offset, 
				StackFrame.AddrFrame.Offset 
				); 

			m_CallStack.push_back( NewFrame );

		}


		// Proceed to the next frame 

	}


	// Complete 

	return ( m_CallStack.size() > 0 );

}

#pragma optimize ( "", on )

bool GetFileParams( const char* pFileName, DWORD64& BaseAddr, DWORD& FileSize ) 
{
	// Check parameters 

	if( pFileName == 0 ) 
	{
		return false; 
	}


	// Determine the extension of the file 

	TCHAR szFileExt[_MAX_EXT] = {0}; 

	_tsplitpath( pFileName, NULL, NULL, NULL, szFileExt ); 


	// Is it .PDB file ? 

	if( _tcsicmp( szFileExt, _T(".PDB") ) == 0 ) 
	{
		// Yes, it is a .PDB file 

		// Determine its size, and use a dummy base address 

		BaseAddr = 0x10000000; // it can be any non-zero value, but if we load symbols 
		// from more than one file, memory regions specified 
		// for different files should not overlap 
		// (region is "base address + file size") 

		if( !GetFileSize( pFileName, FileSize ) ) 
		{
			return false; 
		}

	}
	else 
	{
		// It is not a .PDB file 

		// Base address and file size can be 0 

		BaseAddr = 0; 
		FileSize = 0; 
	}


	// Complete 

	return true; 
}

bool GetFileSize( const TCHAR* pFileName, DWORD& FileSize )
{
	// Check parameters 

	if( pFileName == 0 ) 
	{
		return false; 
	}


	// Open the file 

	HANDLE hFile = ::CreateFile( pFileName, GENERIC_READ, FILE_SHARE_READ, 
		NULL, OPEN_EXISTING, 0, NULL ); 

	if( hFile == INVALID_HANDLE_VALUE ) 
	{
		//_tprintf( _T("CreateFile() failed. Error: %u \n"), ::GetLastError() ); 
		return false; 
	}


	// Obtain the size of the file 

	FileSize = ::GetFileSize( hFile, NULL ); 

	if( FileSize == INVALID_FILE_SIZE ) 
	{
		//_tprintf( _T("GetFileSize() failed. Error: %u \n"), ::GetLastError() ); 
		// and continue ... 
	}


	// Close the file 

	if( !::CloseHandle( hFile ) ) 
	{
		//_tprintf( _T("CloseHandle() failed. Error: %u \n"), ::GetLastError() ); 
		// and continue ... 
	}


	// Complete 

	return ( FileSize != INVALID_FILE_SIZE ); 

}


//
//
//

StackTracer::StackTracer():
m_bInitialized(false)
{
	Init();
}

StackTracer::~StackTracer()
{
	Term();
}

void StackTracer::Init()
{
	BOOL bRet = FALSE; 

	// Set options 

	DWORD Options = SymGetOptions(); 

	// SYMOPT_DEBUG option asks DbgHelp to print additional troubleshooting 
	// messages to debug output - use the debugger's Debug Output window 
	// to view the messages 

	Options |= SYMOPT_DEBUG | SYMOPT_LOAD_LINES; 

	::SymSetOptions( Options ); 


	// Initialize DbgHelp and load symbols for all modules of the current process 
	bRet = ::SymInitialize ( 
		GetCurrentProcess(),  // Process handle of the current process 
		NULL,                 // No user-defined search path -> use default 
		FALSE                  // Load symbols for all modules in the current process 
		); 

	if( !bRet ) 
	{
		_tprintf( _T("Error: SymInitialize() failed. Error code: %u \n"), ::GetLastError() );
		return; 
	}

	m_bInitialized = true;
}

void StackTracer::Term()
{
	m_bInitialized = false;

	// Deinitialize DbgHelp 
	BOOL bRet = FALSE; 

	bRet = ::SymCleanup( GetCurrentProcess() ); 

	if( !bRet ) 
	{
		_tprintf(_T("Error: SymCleanup() failed. Error code: %u \n"), ::GetLastError());
	}
}

void StackTracer::DoStackTrace(StackTrace& trace)
{
	if(!m_bInitialized)
	{
		return;
	}

	// Obtain the call stack 
	CStackWalk StackWalk; 

	if( StackWalk.Walk() ) 
	{
		// Display the call stack 
		CStackWalk::FrameColl_t Frames( StackWalk.CallStack() ); 

		for( size_t i = 0; i < Frames.size(); i++ ) 
		{
			_tStackTraceEntry entry;
			entry.ebp		= Frames[i].Bp;
			entry.retAddr	= Frames[i].RetAddr;
			entry.ip		= Frames[i].Ip;
			trace.push_back(entry);
		}
	}
	else 
	{
		//ERROR!
		//_tprintf( _T("Stack walk failed. Error: %u \n"), StackWalk.LastError() ); 
	}
}


void StackTracer::LoadSymbols(const char* pFilename)
{
	DWORD64   BaseAddr  = 0; 
	DWORD     FileSize  = 0; 

	if( !GetFileParams( pFilename, BaseAddr, FileSize ) ) 
	{
		_tprintf( _T("Error: Cannot obtain file parameters (internal error).\n") ); 
		return; 
	}



	DWORD64 ModBase = ::SymLoadModule64 ( 
		GetCurrentProcess(), // Process handle of the current process 
		NULL,                // Handle to the module's image file (not needed)
		(PSTR)pFilename,           // Path/name of the file 
		NULL,                // User-defined short name of the module (it can be NULL) 
		BaseAddr,            // Base address of the module (cannot be NULL if .PDB file is used, otherwise it can be NULL) 
		FileSize             // Size of the file (cannot be NULL if .PDB file is used, otherwise it can be NULL) 
		); 

	if(ModBase != 0)
	{
		m_vecModBase.push_back(ModBase);
	}
}

void StackTracer::LoadSymbolsInDir(const char* pDirectory, const char* pExtensions)
{
	//parse the extensions
	std::vector<std::string> vecExtensions;	
	StringTokenizer tokenizer((char*)pExtensions, ",");

	while(tokenizer.hasMoreTokens())
	{
		char* pTok = tokenizer.nextToken();
		
		if(pTok[0] != '\0')
		{
			vecExtensions.push_back(pTok);
		}
	}

	struct _finddata_t file;
	long hFile;

	//loop through the extensions for a match
	int nVecSize = (int)vecExtensions.size();
	for(int i = 0; i < nVecSize; ++i)
	{
		char fulldirpath[4096];
		sprintf(fulldirpath, "%s\\*.%s", pDirectory, vecExtensions[i].c_str());

		if( (hFile = (long)_findfirst( fulldirpath, &file )) == -1L )
		{
			printf( "ERROR: No files in current directory! (%s)\n" , fulldirpath);
		}
		else
		{
			char fullname[4096];
			sprintf(fullname, "%s\\%s", pDirectory, file.name);

			LoadSymbols(fullname);
			/* Find the rest of thefiles */

			while( _findnext( hFile, &file ) == 0 )
			{
				sprintf(fullname, "%s\\%s", pDirectory, file.name);
				LoadSymbols(fullname);
			}
			_findclose( hFile );
		}		
	}

	vecExtensions.clear();
}

void StackTracer::GetSymbolByAddr(SymbolDetails &symDetails, DWORD64 addr)
{
	symDetails.ErrorCode = 0;

	CSymbolInfoPackage sip; // it contains SYMBOL_INFO structure plus additional 
							// space for the name of the symbol 

	DWORD64 Displacement = 0; 

	BOOL bRet = ::SymFromAddr( 
		GetCurrentProcess(), // Process handle of the current process 
		addr,             // Symbol address 
		&Displacement,       // Address of the variable that will receive the displacement 
		&sip.si              // Address of the SYMBOL_INFO structure (inside "sip" object) 
		);

	if( !bRet ) 
	{
		symDetails.ErrorCode = 1;
		//_tprintf( _T("Error: SymFromAddr() failed. Error code: %u \n"), ::GetLastError() ); 
	}
	else 
	{
		// Get information about the symbol 
		{	
			symDetails.Address	= sip.si.Address;
			ZeroMemory(symDetails.Name, 512);
			strncpy(symDetails.Name, sip.si.Name, 511);
			symDetails.Size		= sip.si.Size;
		}

		// Obtain file name and line number for the address 

		CImageHlpLine64 LineInfo; 
		DWORD LineDisplacement = 0; // Displacement from the beginning of the line 

		bRet = SymGetLineFromAddr64( 
			GetCurrentProcess(), // Process handle of the current process 
			addr,             // Address 
			&LineDisplacement, // Displacement will be stored here by the function 
			&LineInfo          // File name / line information will be stored here 
			); 

		if( !bRet ) 
		{
			symDetails.ErrorCode = 1;
			//_tprintf( _T("Line information not found. Error code: %u \n"), ::GetLastError() ); 
		}
		else 
		{
			// Get file name / line number information 
			ZeroMemory(symDetails.Filename, 512);
			strncpy(symDetails.Filename, LineInfo.FileName, 511);
			symDetails.LineNumber = LineInfo.LineNumber;					
		}
	}
}

void StackTracer::UnloadSymbols()
{
	// Unload symbols for the module 
	int nNumSymbolTables = (int)m_vecModBase.size();
	for(int i = 0; i < nNumSymbolTables; ++i)
	{		
		BOOL bRet = ::SymUnloadModule64( GetCurrentProcess(), m_vecModBase[i] ); 
	}
	m_vecModBase.clear();
}

