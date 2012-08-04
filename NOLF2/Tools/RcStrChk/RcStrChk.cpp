// ----------------------------------------------------------------------- //
//
// MODULE  : RcStrChk.cpp
//
// PURPOSE : RC String Check - Compares a version 1 local rc file to version
//				0 local rc file and version 0 foreign rc file to find
//				what stringtable entries need translating.
//
// CREATED : 2/7/03
//
// (c) 1997-2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "rcstrchk.h"
#include <fstream.h>
#pragma warning( disable : 4786 )

#include <set>
#include <vector>
#include <string>
#include <hash_map>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// ----------------------------------------------------------------------- //
//
//  CLASS:		StringTableEntry
//
//  PURPOSE:	Contains a single string table entry of label and value.
//
// ----------------------------------------------------------------------- //
class StringTableEntry
{
	public:

		// Constructors.
		StringTableEntry( ) { };
		StringTableEntry( StringTableEntry const& other )
		{
			Copy( other );
		}

		// Assignment operator.
		StringTableEntry& operator=( StringTableEntry const& other )
		{
			return Copy( other );
		}

		// Copies data from other object.
		StringTableEntry& Copy( StringTableEntry const& other )
		{
			if( &other == this )
				return *this;

			m_sLabel = other.m_sLabel;
			m_sValue = other.m_sValue;

			return *this;
		}

		// Comparison operator for compatibility with stl containers.
		bool operator< ( StringTableEntry const& other ) const
		{
			return ( strcmp( m_sLabel.c_str( ), other.m_sLabel.c_str( )) < 0 );
		}

		// Entry label and value.
		std::string m_sLabel;
		std::string m_sValue;
};

// ----------------------------------------------------------------------- //
//
//  CLASS:		StringTableEntry_Exact
//
//  PURPOSE:	Exact comparison functor for StringTableEntry used for set_difference.
//
// ----------------------------------------------------------------------- //
struct StringTableEntry_Exact
{
	bool operator()( StringTableEntry const& ste1, StringTableEntry const& ste2 ) const
	{
		int nRes = strcmp( ste1.m_sLabel.c_str( ), ste2.m_sLabel.c_str( ));
		if( nRes < 0 )
			return true;
		if( nRes == 0 )
		{
			if( strcmp( ste1.m_sValue.c_str( ), ste2.m_sValue.c_str( )) < 0 )
				return true;
		}

		return false;
	}
};


// ----------------------------------------------------------------------- //
//
//  CLASS:		StringTableState
//
//  PURPOSE:	State of stringtable entry parsing.  Since parsing
//				label and value can be multiline, this contains the
//				state of the parsing between lines.
//
// ----------------------------------------------------------------------- //
class StringTableState
{
	public:

		// Constructor.
		StringTableState( )
		{
			Clear( );
		}

		// Clears the object to initial state.
		void Clear( )
		{
			m_StringTableEntry.m_sLabel.erase( );
			m_StringTableEntry.m_sValue.erase( );

			m_bStringTableHeader = false;
			m_bBegin = false;
			m_bEnd = false;
			m_bHaveLabel = false;
			m_bHaveValue = false;
		}

		// Entry currently being read.
		StringTableEntry	m_StringTableEntry;

		// Stringtable header has been read.
		bool m_bStringTableHeader;

		// Begin statement has been read.
		bool m_bBegin;

		// End statement has been read.
		bool m_bEnd;

		// Label has been read.
		bool m_bHaveLabel;

		// Value has been read.
		bool m_bHaveValue;
};

// Set of stringtable entries.
typedef std::set< StringTableEntry > StringTableEntries;

// Convenience macro determining the size of an array.
#define ARRAY_LEN( x ) ( sizeof(( x )) / sizeof(( x )[0] ))

// ----------------------------------------------------------------------- //
//
//  FUNCTION:	ParseRcLine
//
//  PURPOSE:	Parses a single line of an rc file for stringtable information.
//
//  PARAMETERS:	pszLine - the line to parse.
//				stringTableState - current state of parsing for stringtable entries.
//
//	RETURN:		true if successful parse, false on failure.
//
// ----------------------------------------------------------------------- //
static bool ParseRcLine( char const* pszLine, StringTableState& stringTableState )
{
	if( !pszLine )
		return false;

	// Copy the line so we can strtok it.
	char szFinger[1024];
	strncpy( szFinger, pszLine, 1024 );
	szFinger[1023] = 0;

	static char const* pszStringHeader = "STRINGTABLE";
	static char const* pszTableBegin = "BEGIN";
	static char const* pszTableEnd = "END";
	static char const* pszSpaceDelims = " \t\n";

	char const* pszToken = NULL;
	while( !stringTableState.m_bHaveLabel )
	{
		// Setup for first time through or not.
		char* pszParse = ( pszToken ? NULL : szFinger );
		pszToken = strtok( pszParse, pszSpaceDelims );
		if( !pszToken )
			break;

		// Didn't find stringtable header yet.
		if( !stringTableState.m_bStringTableHeader )
		{
			// Check if this isn't the stringtable header.
			if( strcmp( pszToken, pszStringHeader ) != 0 )
			{
				continue;
			}

			// Got the header.  Get the next token.
			stringTableState.Clear( );
			stringTableState.m_bStringTableHeader = true;
			continue;
		}

		// Didn't find begin of table yet.
		if( !stringTableState.m_bBegin )
		{
			// Check if this isn't the beginning of the table.
			if( strncmp( pszToken, pszTableBegin, ARRAY_LEN( pszTableBegin )) != 0 )
			{
				continue;
			}

			// Got the begin. Get the next token.
			stringTableState.m_bBegin = true;
			stringTableState.m_bEnd = false;
			stringTableState.m_bHaveLabel = false;
			stringTableState.m_bHaveValue = false;
			continue;
		}

		// Have begin.
		if( stringTableState.m_bBegin )
		{
			// Check if this is the end of the table.
			if( strncmp( pszLine, pszTableEnd, ARRAY_LEN( pszTableEnd )) == 0 )
			{
				stringTableState.Clear( );
				stringTableState.m_bEnd = true;
				break;
			}
		}

		// This last token will be the label.
		char szLabel[1024];
		strncpy( szLabel, pszToken, ARRAY_LEN( szLabel ));
		stringTableState.m_StringTableEntry.m_sLabel = szLabel;
		stringTableState.m_bHaveLabel = true;
		break;
	}

	// Check if we never found a label.
	if( !stringTableState.m_bHaveLabel )
		return true;

	char const* pszFirstQuote = strchr( pszLine, '\"' );
	if( !pszFirstQuote )
		return true;
	char const* pszSecondQuote = strchr( pszFirstQuote + 1, '\"' );
	if( !pszSecondQuote )
		return true;

	// Now we have the value.
	char szValue[1024];
	int nLen = __min( ARRAY_LEN( szValue ) - 1, pszSecondQuote - pszFirstQuote + 1 );
	strncpy( szValue, pszFirstQuote, nLen );
	szValue[ nLen ] = 0;
	stringTableState.m_StringTableEntry.m_sValue = szValue;
	stringTableState.m_bHaveValue = true;

	return true;
}


// ----------------------------------------------------------------------- //
//
//  FUNCTION:	ParseRcFile
//
//  PURPOSE:	Parses an rc file for for stringtable information.
//
//  PARAMETERS:	pszRcFile - the file to parse.
//				tableEntries - the set of stringtable entries to add to.
//
//	RETURN:		true if successful parse, false on failure.
//
// ----------------------------------------------------------------------- //
bool ParseRcFile( char const* pszRcFile, StringTableEntries& tableEntries )
{
	if( !pszRcFile )
		return false;

	// Clear out the output.
	tableEntries.clear( );

	// Open the file to parse.
	ifstream* pStreamIn = new ifstream( pszRcFile, ios::in );
	if( !pStreamIn )
	{
		printf( "Could not open input file: %s\n", pszRcFile );
		return false;
	}

	StringTableState stringTableState;

	// Parse through the file line by line, adding it to the set.
	char szLine[1024];
	while( true )
	{
		// Get the line and make sure we haven't reached the eof.
		pStreamIn->getline( szLine, 1024 );
		if( !pStreamIn->good( ))
			break;

		// Parse the line into a tableentry.
		if( !ParseRcLine( szLine, stringTableState ))
			return false;

		// If we have a label and a value, we can store them.
		if( stringTableState.m_bHaveLabel && stringTableState.m_bHaveValue )
		{
			// Add the table entry.
			tableEntries.insert( stringTableState.m_StringTableEntry );
			stringTableState.m_bHaveLabel = false;
			stringTableState.m_bHaveValue = false;
		}
	}

	// Done with the file.
	pStreamIn->close( );
	delete pStreamIn;

	return true;
}

// ----------------------------------------------------------------------- //
//
//  FUNCTION:	FormatLabelValue
//
//  PURPOSE:	Formats a label and value to fit rc format.
//
//  PARAMETERS:	sLabel - Label to use.
//				sValue - value to use.
//				pszLine - Output line.
//
//	RETURN:		void.
//
// ----------------------------------------------------------------------- //
static void FormatLabelValue( std::string const& sLabel, std::string const& sValue, char* pszLine )
{
	// Write out label and value in RC format.
	int nLen = 4 + sLabel.length( );
	nLen = 28 - nLen;
	nLen = __max( nLen, 1 );
	sprintf( pszLine, "    %s%*c%s", sLabel.c_str( ), nLen, ' ', sValue.c_str( ));
}

// ----------------------------------------------------------------------- //
//
//  FUNCTION:	GenerateNewForeignFile
//
//  PURPOSE:	Creates a new foreign file with strings that don't need
//				translating and strings that do need translating.  Strings
//				that do need translating are marked with "// *** NEEDS TRANSLATION ***".
//
//  PARAMETERS:	pszOldLocal - version 0 of local file.
//				pszNewLocal - version 1 of local file.
//				pszOldForeign - version 0 of foreign file.
//				pszNewForeign - version 1 of foreign file.
//
//	RETURN:		true if successful parse, false on failure.
//
// ----------------------------------------------------------------------- //
bool GenerateNewForeignFile( char const* pszOldLocal, char const* pszNewLocal, 
						 char const* pszOldForeign, char const* pszNewForeign )
{
	if( !pszOldLocal || !pszNewLocal || !pszOldForeign || !pszNewForeign )
		return false;

	// Read the new local file.
	StringTableEntries newLocalTableEntries;
	if( !ParseRcFile( pszNewLocal, newLocalTableEntries ))
		return false;

	// Read the old local file.
	StringTableEntries oldLocalTableEntries;
	if( !ParseRcFile( pszOldLocal, oldLocalTableEntries ))
		return false;

	// Read the old foreign file.
	StringTableEntries oldForeignTableEntries;
	if( !ParseRcFile( pszOldForeign, oldForeignTableEntries ))
		return false;

	// Open the newlocal file to use as template for newforeign.
	ifstream* pStreamIn = new ifstream( pszNewLocal, ios::in );
	if( !pStreamIn )
	{
		printf( "Could not open input file: %s\n", pszNewLocal );
		return false;
	}

	// Create the new foreign output file.
	ofstream* pStreamOut = new ofstream( pszNewForeign );
	if( !pStreamOut )
	{
		printf( "Could not open output file: %s\n", pszNewForeign );
		return false;
	}

	// Find the strings that have been changed.
	StringTableEntries localChanged;
	std::set_difference( oldLocalTableEntries.begin(), oldLocalTableEntries.end(), 
		newLocalTableEntries.begin(), newLocalTableEntries.end(), 
		std::inserter( localChanged, localChanged.begin()),
		StringTableEntry_Exact( ));



	// Parse through the file line by line, adding it to the set.
	StringTableState stringTableState;
	char szLine[1024];
	while( true )
	{
		pStreamIn->getline( szLine, 1024 );
		if( !pStreamIn->good( ))
			break;

		// Parse the line into a tableentry.
		if( !ParseRcLine( szLine, stringTableState ))
			return false;

		// Check if we just got the end of a table.
		if( stringTableState.m_bEnd )
		{
			stringTableState.m_bEnd = false;
			*pStreamOut << szLine << endl << endl;
			continue;
		}

		// Only write out stringtables.
		if( !stringTableState.m_bStringTableHeader )
		{
			continue;
		}

		// If we don't have a label or a value, just write out the line.
		if( !stringTableState.m_bHaveLabel && !stringTableState.m_bHaveValue )
		{
			*pStreamOut << szLine << endl;
			continue;
		}

		// If we have a label and a value, we're ready to process.
		if( stringTableState.m_bHaveLabel && stringTableState.m_bHaveValue )
		{
			char szLine[1024];

			// Check if this is one of the changed entries.
			if( localChanged.find( stringTableState.m_StringTableEntry ) != localChanged.end( ))
			{
				// Write out the local version so it can be retranslated.
				FormatLabelValue( stringTableState.m_StringTableEntry.m_sLabel,
					stringTableState.m_StringTableEntry.m_sValue, szLine );
				*pStreamOut << szLine << "\t\t // *** NEEDS TRANSLATION ***" << endl;
			}
			else 
			{
				// Check if this exists in the foreign version.
				StringTableEntries::iterator iter = oldForeignTableEntries.find( stringTableState.m_StringTableEntry );
				if( iter != oldForeignTableEntries.end( ))
				{
					// Write out the foreign version.
					FormatLabelValue( iter->m_sLabel, iter->m_sValue, szLine );
					*pStreamOut << szLine << endl;
				}
				else
				{
					// Write out the local version so it can be retranslated.
					FormatLabelValue( stringTableState.m_StringTableEntry.m_sLabel,
						stringTableState.m_StringTableEntry.m_sValue, szLine );
					*pStreamOut << szLine << "\t\t // *** NEEDS TRANSLATION ***" << endl;
				}
			}

			stringTableState.m_bHaveLabel = false;
			stringTableState.m_bHaveValue = false;
		}
	}

	// Close the output file.
	pStreamOut->close( );
	delete pStreamOut;
	pStreamOut = NULL;

	// Close the input file.
	pStreamIn->close( );
	delete pStreamIn;
	pStreamIn = NULL;

	return true;
}

/////////////////////////////////////////////////////////////////////////////
// The one and only application object

CWinApp theApp;

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	// initialize MFC and print and error on failure
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		// TODO: change error code to suit your needs
		cerr << _T("Fatal Error: MFC initialization failed") << endl;
		return 1;
	}
	else
	{
		if( argc < 5 )
		{
			printf( "rcstrchk\n\n" );
			printf( "Finds changes to the stringtable of the latest local\n" );
			printf( "rc file compared to an old local rc file.\n" );
			printf( "Then it checks to see what changes are necessary\n" );
			printf( "when compared to an old foreign file.  The resulting\n" );
			printf( "string tables are put into an output file.\n" );
			printf( "usage:\n\n" );
			printf( "rcstrchk <oldlocal.rc> <oldlocal.rc> <oldforiegn.rc> <out.rc>\n" );
			printf( "\toldlocal.rc - Old version of local .rc file.\n" );
			printf( "\tnewocal.rc - New version of local .rc file.\n" );
			printf( "\toldforeign.rc - Old version of foreign .rc file.\n" );
			printf( "\tout.rc - Output containing new foreign string tables.\n" );
			return 1;
		}

		if( !GenerateNewForeignFile( argv[1], argv[2], argv[3], argv[4] ))
			return 1;

	}

	return 0;
}
