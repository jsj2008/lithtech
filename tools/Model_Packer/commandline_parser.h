// Helper class for parsing the command line...

#include <string.h>
#include <vector>
#include <string>
#include "lta2ltb_d3d.h"

// Command line parsing...
struct CommandPair		{ string szParam; string szValue; };

class CommandLineParser {
public:
	CommandLineParser() { 
		m_bOnCmd = true; 
		m_bInPlace = false;
		m_bGeom = true  ;
		for (uint32 i = 0; i < 4; ++i) {
			m_bOnStream[i] = false;
			m_SteamFlags[i] = NULL; } 
		m_SteamFlags[0] = VERTDATATYPE_POSITION | VERTDATATYPE_NORMAL | VERTDATATYPE_UVSETS_1; 
		m_AnimCompression = eRelevant ;
		m_bVerbose = false ;
		m_bGeomOverride=false;
		m_bACOverride = false;
	}	
	
	void Clear()
	{
		m_bOnCmd = true; 
		m_bInPlace = false;
		for (uint32 i = 0; i < 4; ++i) {
			m_bOnStream[i] = false;
			m_SteamFlags[i] = NULL; } 
		m_SteamFlags[0] = VERTDATATYPE_POSITION | VERTDATATYPE_NORMAL | VERTDATATYPE_UVSETS_1;
		m_CmdLine.clear();
	}

	void SetCommandLine(int iArgCount, char* szCmdLine[]) 
	{
		// Go through all the params and parse them...
		if (iArgCount > 1) m_ProcessCmd = szCmdLine[1];
		for (int i=2;i<iArgCount;++i) {	
			ParseParam(szCmdLine[i]); 
		} 
	}

	void SetCommandLine( std::vector< std::string > & cmds )
	{
		// Go through all the params and parse them...
		if( cmds.size() < 1 ) return ;

		m_ProcessCmd = cmds[0];
		for (unsigned int i=1;i<cmds.size();++i) {	
			ParseParam(cmds[i].c_str()); 
		} 
	}
	

	// Get the value from a param name (if it was entered)...
	const char* GetParamVal(const char* szParam) 
	{
		vector<CommandPair>::iterator it = m_CmdLine.begin();
		while (it != m_CmdLine.end()) 
		{
			if (stricmp(it->szParam.c_str(),szParam)==0) return it->szValue.c_str();
			++it; 
		} 
		return NULL; 
	}

	

	const char* GetInFile()					{ if(m_bInPlace) return GetParamVal("inplace"); else return GetParamVal("input"); }
	const char* GetOutFile()				{ return GetParamVal("output"); }
	
	const char* GetProcessCmd()				{ return m_ProcessCmd.c_str(); }
	uint32		GetSteamFlags(uint32 i)		{ return m_SteamFlags[i]; }

	int hasVerbose()    
	{ 
		return m_bVerbose ; 
	}

	int hasStall()    
	{ 
		const char *val = "stall";
		vector<CommandPair>::iterator it = m_CmdLine.begin();
		while (it != m_CmdLine.end()) 
		{
			if (stricmp(it->szParam.c_str(),val)==0) return 1;
			++it; 
		} 
		return 0;
	}

	bool InPlace() { return m_bInPlace ; } 

	bool ExportGeom() { return m_bGeom ; }
	bool ExportGeomOverride() { return m_bGeomOverride ; }

	EANIMCOMPRESSIONTYPE GetAnimCompressionType() { return m_AnimCompression ; } 
	bool GetAnimCompressionTypeOverride() { return m_bACOverride ; }

private:
	
	EANIMCOMPRESSIONTYPE	m_AnimCompression ;
	bool				m_bACOverride ;
	bool				m_bGeomOverride;
	bool				m_bVerbose ;
	bool				m_bInPlace ;
	bool				m_bOnCmd;
	bool				m_bOnStream[4];
	bool				m_bGeom ;
	uint32				m_SteamFlags[4];
	string				m_ProcessCmd;
	vector<CommandPair> m_CmdLine;

	// ------------------------------------------------------------------------
	// Parse the command line parameters.
	// ------------------------------------------------------------------------
	void ParseParam(const char* szParam) 
	{
		if (m_bOnCmd) 
		{		// Parsing a command now...
			CommandPair CmdPair;
			if (szParam && szParam[0]=='-' && szParam[1]) 
			{
				m_bOnCmd = false;
				CmdPair.szParam = &szParam[1]; 

				// Check for stream params (this is for d3d vert data)...
				if (stricmp(&szParam[1], "stream0") == 0) {
					m_SteamFlags[0] = NULL;
					m_bOnStream[0] = true; }
				else if (stricmp(&szParam[1], "stream1") == 0) {
					m_bOnStream[1] = true; }
				else if (stricmp(&szParam[1], "stream2") == 0) {
					m_bOnStream[2] = true; }
				else if (stricmp(&szParam[1], "stream3") == 0) {
					m_bOnStream[3] = true; }
			
				// huh, if switch has no params, don't ask to expect any.
				else if( stricmp( &szParam[1], "verbose" ) == 0 )
				{
					m_bOnCmd = true ;
					m_bVerbose = true ;
				}

				// huh, if switch has no params, don't ask to expect any.
				else if( stricmp( &szParam[1], "stall" ) == 0 )
					m_bOnCmd = true ;

				else if( stricmp( &szParam[1], "wire" ) == 0 )
					m_bOnCmd = true ;

				else if( stricmp( &szParam[1], "notex" ) == 0 )
					m_bOnCmd = true ;
		
				else if( stricmp( &szParam[1], "inplace" ) == 0 )
					m_bInPlace = true ;
				
				else if( stricmp( &szParam[1], "ac0" ) == 0 )
				{
					m_bACOverride=true;
					m_bOnCmd = true;
					m_AnimCompression = eNone ;
				}
				else if( stricmp( &szParam[1], "ac1" ) == 0 )
				{
					m_bACOverride=true;
					m_bOnCmd = true ;
					m_AnimCompression = eRelevant;
				}
				else if( stricmp( &szParam[1], "ac2" ) == 0 )
				{
					m_bACOverride=true;
					m_bOnCmd = true ;
					m_AnimCompression = eRelevant16bit;
				}
				else if( stricmp( &szParam[1], "acpv" ) == 0 )
				{
					m_bACOverride=true;
					m_bOnCmd = true ; 
					m_AnimCompression = eRelevant16bit_PlayerView ;
				}
				else if( stricmp( &szParam[1], "nogeom" ) == 0 )
				{
					m_bGeomOverride=true;
					m_bOnCmd = true ;
					m_bGeom = false ;
				}

				m_CmdLine.push_back(CmdPair); 
			} 
		}
		else 
		{				// Every other one is a param...
			for (uint32 i = 0; i < 4; ++i) {
				if (m_bOnStream[i]) 
				{	
					// we're done with this stream - still need to handle this param...
					if (szParam && szParam[0] == '-') 
					{	
						m_bOnStream[i] = false; m_bOnCmd = true; 
						ParseParam(szParam); return; 
					}

					if (stricmp(szParam,"position") == 0) {
						m_SteamFlags[i] |= VERTDATATYPE_POSITION; }
					else if (stricmp(szParam,"normal") == 0) {
						m_SteamFlags[i] |= VERTDATATYPE_NORMAL; }
					else if (stricmp(szParam,"color") == 0) {
						m_SteamFlags[i] |= VERTDATATYPE_COLOR; }
					else if (stricmp(szParam,"uv1") == 0) {
						m_SteamFlags[i] |= VERTDATATYPE_UVSETS_1; }
					else if (stricmp(szParam,"uv2") == 0) {
						m_SteamFlags[i] |= VERTDATATYPE_UVSETS_2; }
					else if (stricmp(szParam,"uv3") == 0) {
						m_SteamFlags[i] |= VERTDATATYPE_UVSETS_3; }
					else if (stricmp(szParam,"uv4") == 0) {
						m_SteamFlags[i] |= VERTDATATYPE_UVSETS_4; }
					else if (stricmp(szParam,"basisvectors") == 0) {
						m_SteamFlags[i] |= VERTDATATYPE_BASISVECTORS; }
					return; } }

			if (m_CmdLine.size() && szParam) {
				m_bOnCmd = true;
				m_CmdLine[m_CmdLine.size()-1].szValue = szParam; 
			} 
		} 
	}
};


