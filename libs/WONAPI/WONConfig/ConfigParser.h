#ifndef __WON_CONFIGPARSER_H__
#define __WON_CONFIGPARSER_H__

#include "WONCommon/StringUtil.h"
#include <map>

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
struct ConfigParserState
{
	long mFilePos;
	std::string mFileNameAndPath;
	int mLineNum;

	std::string mBackStr;
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class ConfigParser
{
private:
	ConfigParser(const ConfigParser& theCopy);
	ConfigParser& operator=(const ConfigParser& theAssign);

protected:
	typedef std::map<std::string,std::string,WONAPI::StringLessNoCase> DefineMap;
	DefineMap mDefineMap;
	
	ConfigParser *mParent;

	FILE* mFile;
	std::string mBackStr;
	std::string mAbortReason;
	std::string mWarnings;
	int mLineNum;

	std::string mFilePath;
	std::string mFileNameAndPath;

	bool mIgnorePathOnIncludeFile;
	bool mStopped;

	bool EndOfFile();
	int GetChar();
	int PeekChar();
	void UngetChar(int theChar);

public:
	ConfigParser();
	virtual ~ConfigParser();

	void SetParent(ConfigParser *theParent);

	const std::string& GetFilePath() const { return mFilePath; }
	
	bool GetIgnorePathOnIncludeFile() { return mIgnorePathOnIncludeFile; }
	void SetIgnorePathOnIncludeFile(bool ignore) { mIgnorePathOnIncludeFile = ignore; }
	
	void Reset(); 
	bool OpenFile(const std::string& theFilePath);
	void CloseFile();

	void SkipWhitespace();
	void SkipLine();
	void SkipBlockComment();

	void AbortRead(const std::string &theReason);
	void AddWarning(const std::string &theWarning);
	void CopyWarnings(ConfigParser &theCopyFrom);

	void StopRead() { mStopped = true; }
	bool IsStopped() { return mStopped; }

	bool IsAborted() const { return !mAbortReason.empty(); }
	const std::string& GetAbortReason();

	bool HasWarnings() const { return !mWarnings.empty(); }
	const std::string& GetWarnings() const { return mWarnings; } 

	bool ConfigParser::AddDefine(const std::string &theName, const std::string &theVal);
	bool ConfigParser::GetDefine(const std::string &theName, std::string &theValR);

	bool ReadToken(std::string &theToken, bool toEndLine = false, bool skip = false);

	void GetState(ConfigParserState &theState);
	bool SetState(const ConfigParserState &theState);
};

}; // namespace WONAPI


#endif
