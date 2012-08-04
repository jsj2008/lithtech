// xls2rc.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "xls2rc.h"
#include <fstream.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// The one and only application object

CWinApp theApp;

bool ConvertXls2Rc( istream& streamIn, ostream& rcOut, ostream& hOut );

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	int nRetCode = 0;

	// initialize MFC and print and error on failure
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		// TODO: change error code to suit your needs
		cerr << _T("Fatal Error: MFC initialization failed") << endl;
		nRetCode = 1;
	}
	else
	{
		if( argc < 2 )
		{
			printf( "xls2rc\n\n" );
			printf( "Converts tab delimited text file with DIALOGUE into .rc file\n" );
			printf( "and .h file suitable for copying into VC++ rc and h files.\n" );
			printf( "Output is similar to:\n\n" );
			printf( "IDS_DIALOGUE_10482  \"Hey, everyone! It's Cate Archer! British superspy!\"\n\n" );
			printf( "usage:\n\n" );
			printf( "xls2rc <infilename.ext> [outfilename]\n" );
			printf( "\tinfilename.ext - tab delimited text filename with extension.\n" );
			printf( "\toutfilename - output filename name without extension.\n" );
			printf( "\t\t\tOptional  If omitted, then infilename is used.\n" );
			return 1;
		}

		bool bOk = true;

		ifstream* pStreamIn = new ifstream( argv[1], ios::in );
		if( !pStreamIn )
		{
			printf( "Could not open input file: %s\n", argv[1] );
			bOk = false;
		}

		CString sOutFileName;
		if( argc < 3 )
		{
			sOutFileName = argv[1];
		}
		else
		{
			sOutFileName = argv[2];
		}

		// Find the file name components so 2 individual files
		// can be created for output.
		char drive[_MAX_DRIVE];
		char dir[_MAX_DIR];
		char fname[_MAX_FNAME];
		char ext[_MAX_EXT];
		char szPath[_MAX_PATH*2];
		_splitpath( sOutFileName, drive, dir, fname, ext );

		// Create the .rc output file.
		ofstream* pRCOut = NULL;
		if( bOk )
		{
			_makepath( szPath, drive, dir, fname, ".rc" );
			pRCOut = new ofstream( szPath );
			if( !pRCOut )
			{
				printf( "Could not open output file: %s\n", szPath );
				bOk = false;
			}
		}

		// Create the .h output file.
		ofstream* pHOut = NULL;
		if( bOk )
		{
			_makepath( szPath, drive, dir, fname, ".h" );
			pHOut = new ofstream( szPath );
			if( !pHOut )
			{
				printf( "Could not open output file: %s\n", szPath );
				bOk = false;
			}
		}
		
		// Convert it.
		bOk = bOk && ConvertXls2Rc( *pStreamIn, *pRCOut, *pHOut );

		// Clean up.
		if( pStreamIn )
		{
			pStreamIn->close( );
			delete pStreamIn;
			pStreamIn = NULL;
		}

		if( pRCOut )
		{
			pRCOut->close( );
			delete pRCOut;
			pRCOut = NULL;
		}

		if( pHOut )
		{
			pHOut->close( );
			delete pHOut;
			pHOut = NULL;
		}

		if( !bOk )
		{
			nRetCode = 1;
		}
		else
		{
			nRetCode = 0;
		}
	}

	return nRetCode;
}


bool ConvertLine( const CString& sLineIn, CString& sRcLineOut, CString& sHLineOut )
{
	int i;

	// Start off clean.
	sRcLineOut.Empty( );
	sHLineOut.Empty( );

	// This will store the id number.
	CString sNumber = "";

	// Start at the beginning of the input string.
	i = 0;

	// Process the ID number.  Stop when tab found.
	for( ; i < sLineIn.GetLength( ); i++ )
	{
		TCHAR value = sLineIn[i];

		if( _istdigit( value ))
		{
			sNumber += value;
		}
		else if( value == _TCHAR( '\t' ))
		{
			i++;
			break;
		}
	}

	// Make sure we found an ID number.
	if( sNumber.IsEmpty( ))
		return false;

	// Format the id number into the outputs.
	sRcLineOut.Format( "\tIDS_DIALOGUE_%s\t\"", sNumber );
	sHLineOut.Format( "#define IDS_DIALOGUE_%s\t\t%s", sNumber, sNumber );

	// Loop through the input line and convert non-ascii values to "*".
	for( ; i < sLineIn.GetLength( ); i++ )
	{
		TCHAR value = sLineIn[i];

		// Replace non-ascii text with asterisk.
		if( !_istascii( value ))
		{
			sRcLineOut += "*";
		}
		// Ignore quote's.
		else if( value == _TCHAR( '\"' ))
		{
		}
		else
		{
			sRcLineOut += value;
		}
	}

	sRcLineOut += "\"";

	return true;
}


bool ConvertXls2Rc( istream& streamIn, ostream& strmRcOut, ostream& strmHOut )
{
	CString sLineIn;
	CString sRcLineOut;
	CString sHLineOut;

	while( true )
	{
		streamIn.getline( sLineIn.GetBuffer( 1024 ), 1024 );
		sLineIn.ReleaseBuffer( );
		if( !streamIn.good( ))
			break;

		if( ConvertLine( sLineIn, sRcLineOut, sHLineOut ))
		{
			strmRcOut << sRcLineOut << endl;
			strmHOut << sHLineOut << endl;
		}
	}

	return true;
}