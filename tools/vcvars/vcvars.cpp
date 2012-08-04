//------------------------------------------------------------------
//
//	FILE	: VCVARS
//
//	PURPOSE	: Runs vcvars32.bat/vsvars32.bat command line settings 
//	depending on a given dev studio version ex 6 2002 2003 
//
//
//	usage should be from a batch file like so 
//
//	$(GAME_TOOLS)vcvars 2003 > vcvars.bat   // writes batch to call vcvars32 enviro
//  call vcvars.bat							// sets the command line enviro
//
//	CREATED	: 12/10/04
//
//------------------------------------------------------------------
#include <windows.h>
#include <stdio.h>

// Reg keys for the flavors 
static const char szSubKey6[] = "SOFTWARE\\Microsoft\\VisualStudio\\6.0\\Setup\\Microsoft Visual C++";
static const char szSubKey2002[] = "SOFTWARE\\Microsoft\\VisualStudio\\7.0\\Setup\\VS";
static const char szSubKey2003[] = "SOFTWARE\\Microsoft\\VisualStudio\\7.1\\Setup\\VS";

// Reg key values for each flavor
static const char szValue6[] = "ProductDir";
static const char szValue2002[] = "VS7CommonBinDir";
static const char szValue2003[] = "VS7CommonBinDir";

#define REGSTRINGSIZE 2048

static char szDir[REGSTRINGSIZE];

HANDLE hStd;


// forward delcarations 
bool GetDirectory ( LPCTSTR lpSubKey, LPCTSTR lpValue, char * pDir );
void displayHelp( void );
void OutputToConsole(const char* szMsg, ...);


main ( int argc, char ** argv )
{

	// Get the console handle 
	hStd = GetStdHandle( STD_OUTPUT_HANDLE );

	if ( hStd == INVALID_HANDLE_VALUE )
	{
		return 1;
	}

	// make sure we have parameters 
	if ( argc != 2 )
	{
		displayHelp();
		return 1;
	}

	// Check for valid options 
	if ( !strcmp ( "6" , argv[1] ) )
	{
		if ( GetDirectory ( szSubKey6, szValue6, szDir ) )
		{
			strcat ( szDir, "\\Bin\\vcvars32.bat" );
		}
	}
	else if ( !strcmp ( "2002" , argv[1] ) )
	{
		if ( GetDirectory ( szSubKey2002, szValue2002, szDir ) )
		{
			strcat ( szDir, "vsvars32.bat" );
		}
	}
	else if ( !strcmp ( "2003" , argv[1] ) )
	{
		if ( GetDirectory ( szSubKey2003, szValue2003, szDir ) )
		{
			strcat ( szDir, "vsvars32.bat" );
		}
	}
	else
	{
		displayHelp();
		return 1;
	}

	// Output the command line command 
	OutputToConsole("call \"%s\"",szDir);

	return 0;

}


bool GetDirectory ( LPCTSTR lpSubKey, LPCTSTR lpValue, char * pDir )
{
	LONG lError;
	DWORD nType;
	DWORD nSize = REGSTRINGSIZE;
	HKEY hKey;

	if ( RegOpenKeyEx(  HKEY_LOCAL_MACHINE, lpSubKey, 0, KEY_QUERY_VALUE, &hKey ) == ERROR_SUCCESS )
	{
		lError = RegQueryValueEx( hKey, lpValue, NULL, &nType,(unsigned char*)pDir,( DWORD * )&nSize);

		RegCloseKey ( hKey );

		if ( lError == ERROR_SUCCESS )
		{
			return true;
		}
	}

	return false;
}


void displayHelp( void )
{
	OutputToConsole ("\n vcvars (6 | 2002 | 2003) - 6 run vcvars32.bat for VC 6\n" );
	  OutputToConsole ("                          - 2002 run vcvars32.bat for .Net 2002\n" );
	  OutputToConsole ("                          - 2003 run vcvars32.bat for .Net 2003\n\n" );
	  OutputToConsole ("The appropriate registry key must exist and have read access rights\n" );

}

void OutputToConsole(const char* szMsg, ...)
{
	va_list v;
	va_start(v, szMsg);
	char aMsgBuffer[2048];
	_vsnprintf(aMsgBuffer, sizeof(aMsgBuffer), szMsg, v);

	DWORD bytesWritten;

	WriteFile( hStd,              // output handle 
				aMsgBuffer,          // prompt string 
				lstrlen(aMsgBuffer), // string length 
				&bytesWritten,            // bytes written 
				NULL);               // not overlapped 

}
