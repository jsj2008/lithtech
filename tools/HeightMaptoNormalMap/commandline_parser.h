// Helper class for parsing the command line...

#include <string.h>
#include <vector>
#include <string>

// Command line parsing...
struct CommandPair							{ string szParam; string szValue; };

class CommandLineParser : public CCommandLineInfo {
public:
	CommandLineParser()						{ m_bOnCmd = true;  }	// Default it...
	
	// Get the value from a param name (if it was entered)...
	const char* GetParamVal(const char* szParam) {
		vector<CommandPair>::iterator it = m_CmdLine.begin();
		while (it != m_CmdLine.end()) {
			if (stricmp(it->szParam.c_str(),szParam)==0) return it->szValue.c_str(); ++it; } 
		return NULL; }

	// hasSwitch("verbose")
	bool hasSwitch(const char * szParam) {
		vector<CommandPair>::iterator it = m_CmdLine.begin();
		while (it != m_CmdLine.end()) {
			if (stricmp(it->szParam.c_str(),szParam)==0) return 1; ++it; } 
		return 0; }

	const char* GetInFile()					{ return GetParamVal("input"); }
	const char* GetOutFile()				{ return GetParamVal("output"); }
	
	bool hasVerbose() { 
		const char *val = "verbose";
		vector<CommandPair>::iterator it = m_CmdLine.begin();
		while (it != m_CmdLine.end()) {
			if (stricmp(it->szParam.c_str(),val)==0) return true; ++it; } 
		return false; }

	bool hasHelp() { 
		const char *val = "help";
		vector<CommandPair>::iterator it = m_CmdLine.begin();
		while (it != m_CmdLine.end()) {
			if (stricmp(it->szParam.c_str(),val)==0) return true; ++it; } 
		return false; }

	bool hasWin() { 
		const char *val = "win";
		vector<CommandPair>::iterator it = m_CmdLine.begin();
		while (it != m_CmdLine.end()) {
			if (stricmp(it->szParam.c_str(),val)==0) return true; ++it; } 
		return false; }

private:
	bool				m_bOnCmd;
	vector<CommandPair> m_CmdLine;

	virtual void ParseParam(LPCTSTR szParam, BOOL bFlag, BOOL bLast) {
		if (bFlag && m_bOnCmd) {			// Parsing a command now...
			CommandPair CmdPair;
			if (szParam) {
				m_bOnCmd = false; CmdPair.szParam = szParam; 

				if (stricmp(szParam, "verbose") == 0)	m_bOnCmd = true;	// huh, if switch has no params, don't ask to expect any.
				if (stricmp(szParam, "help") == 0)		m_bOnCmd = true;
				if (stricmp(szParam, "win") == 0)		m_bOnCmd = true;

				m_CmdLine.push_back(CmdPair); } }
		else {								// Every other one is a param...
			if (m_CmdLine.size() && szParam) {
				m_bOnCmd = true;
				m_CmdLine[m_CmdLine.size()-1].szValue = szParam; } } }
};


