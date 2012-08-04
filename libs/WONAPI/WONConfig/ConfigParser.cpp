#include "ConfigParser.h"

using namespace std;
using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ConfigParser::ConfigParser()
{
	mFile = NULL;
	mLineNum = 0;
	mParent = NULL;
	mStopped = false;
}
	
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ConfigParser::~ConfigParser()
{
	CloseFile();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ConfigParser::SetParent(ConfigParser *theParent)
{
	mParent = theParent;
	if(mParent!=NULL)
	{
		mIgnorePathOnIncludeFile = theParent->GetIgnorePathOnIncludeFile();
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ConfigParser::AbortRead(const string &theReason)
{
	if(mAbortReason.empty())
	{
		char aBuf[1024];
		sprintf(aBuf," (%s:%d)",mFileNameAndPath.c_str(),mLineNum);
		mAbortReason =  theReason + aBuf;
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ConfigParser::AddWarning(const std::string &theWarning)
{
	char aBuf[1024];
	sprintf(aBuf,"Warning (%s: %d) - %s\n",mFileNameAndPath.c_str(),mLineNum,theWarning.c_str());
	mWarnings += aBuf;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ConfigParser::CopyWarnings(ConfigParser &theCopyFrom)
{
	mWarnings += theCopyFrom.GetWarnings();
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
const std::string& ConfigParser::GetAbortReason()
{
	if(mAbortReason.empty())
		AbortRead("Parse aborted.");

	return mAbortReason;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ConfigParser::Reset(void)
{
	mDefineMap.clear();
	mAbortReason.erase();
	mWarnings.erase();
	mBackStr.erase();

	mLineNum = 1;
	mStopped = false;
	if(mFile)
		rewind(mFile);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ConfigParser::AddDefine(const std::string &theName, const std::string &theVal)
{
	if(theName.find_first_of(' ')!=string::npos)
	{
		AddWarning("Not allowed to have spaces in define names.");
		return true;
	}

	if(mDefineMap.insert(DefineMap::value_type(theName,theVal)).second==false)
	{
		AddWarning("Redefintion of " + theName);
		return true;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ConfigParser::GetDefine(const std::string &theName, std::string &theValR)
{
	DefineMap::iterator anItr = mDefineMap.find(theName);
	if(anItr!=mDefineMap.end())
	{
		theValR = anItr->second;
		return true;
	}
	else if(mParent==NULL)
		return false;
	else 
		return mParent->GetDefine(theName,theValR);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ConfigParser::EndOfFile()
{	
	if(!feof(mFile)) 
		return false; 
	else return 
		mBackStr.empty(); 
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
int ConfigParser::GetChar()
{
	int aChar = 0;
	if(mBackStr.empty())
		aChar = fgetc(mFile);
	else
	{
		aChar = mBackStr[mBackStr.length()-1];
		mBackStr.resize(mBackStr.length()-1);
	}

	if(aChar=='\n')
		mLineNum++;

	return aChar;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
int ConfigParser::PeekChar()
{
	if(mFile && !EndOfFile())
	{
		int aChar = GetChar();
		UngetChar(aChar);
		return aChar;
	}
	else
		return -1;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ConfigParser::UngetChar(int theChar)
{ 
	if(theChar==EOF)
		return;

	if(theChar=='\n')
		mLineNum--;

	mBackStr+=theChar;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ConfigParser::OpenFile(const std::string& theFilePath)
{
	CloseFile();
	mLineNum = 1;

	mBackStr.erase();
	mFileNameAndPath = theFilePath;

	int aPos = mFileNameAndPath.find_last_of('/');
	if(aPos == string::npos)
		aPos = mFileNameAndPath.find_last_of('\\');

	if(aPos == string::npos)
		mFilePath = "";
	else 
		mFilePath = mFileNameAndPath.substr(0, aPos+1);

	mFile = fopen(theFilePath.c_str(), "r");
	return mFile!=NULL;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ConfigParser::CloseFile()
{
	if(mFile!=NULL)
	{
		fclose(mFile);
		mFile = NULL;
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ConfigParser::GetState(ConfigParserState &theState)
{
	theState.mBackStr = mBackStr;
	theState.mFileNameAndPath = mFileNameAndPath;
	theState.mFilePos = ftell(mFile);
	theState.mLineNum = mLineNum;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ConfigParser::SetState(const ConfigParserState &theState)
{
	if(mFile==NULL || theState.mFileNameAndPath!=mFileNameAndPath)
	{
		if(!OpenFile(theState.mFileNameAndPath))
			return false;
	}

	fseek(mFile,theState.mFilePos,SEEK_SET);
	mLineNum = theState.mLineNum;
	mBackStr = theState.mBackStr;
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ConfigParser::SkipWhitespace()
{
	while(!EndOfFile())
	{
		int aChar = GetChar();
		if(!isspace(aChar))
		{
			if(aChar=='#')
			{
				SkipLine();
				continue;
			}
			else if(aChar=='/')
			{
				int anotherChar = GetChar();
				if(anotherChar=='/')
				{
					SkipLine();
					continue;
				}
				else if(anotherChar=='*')
				{
					SkipBlockComment();
					continue;
				}
				else
					UngetChar(anotherChar);
			}

			UngetChar(aChar);
			return;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ConfigParser::SkipLine()
{
	while(!EndOfFile())
	{
		int aChar = GetChar();
		if(aChar=='\n')
			return;
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ConfigParser::SkipBlockComment()
{
	bool gotStar = false;
	while(!EndOfFile())
	{
		int aChar = GetChar();
		if(gotStar && aChar=='/')
			break;
		
		gotStar = aChar=='*';
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ConfigParser::ReadToken(std::string &theToken, bool toEndLine, bool skip)
{
	if(EndOfFile())
		return false;

	theToken.erase();

	SkipWhitespace();
	bool inQuote = false, lastWasBackslash = false, lastWasSpace = true;
	int aChar = 0;
	while(!EndOfFile())
	{
		lastWasBackslash = aChar=='\\';
		aChar = GetChar();
		if(aChar==EOF)
			break;

		if(isspace(aChar) && !inQuote)
		{
			if(!toEndLine)
				break;
			else if(aChar=='\n' && !lastWasBackslash)
			{
				UngetChar(aChar); // unget last '\n' in order to have correct line number on errors and warnings
				break;
			}
			else if(!skip)
				theToken += ' ';

			lastWasSpace = true;

			int aStartLine = mLineNum;
			SkipWhitespace();
			if(aStartLine!=mLineNum)
				break;
			else
				continue;
		}
		else if(aChar=='"')
		{
			if(inQuote && !lastWasBackslash)
				inQuote = false;
			else if(!inQuote && lastWasSpace)
				inQuote = true;
		}
		else if(aChar=='\\')
		{
			int anotherChar = GetChar();
			if(anotherChar=='\n')
				continue;
			else
				UngetChar(anotherChar);
		}
		else if(aChar=='$')
		{
			if(!skip && (!inQuote || !lastWasBackslash))
			{
				// do substitution
				string aDefine, aVal;
				while(!EndOfFile())
				{
					int aChar = GetChar();
					if(isspace(aChar))
					{
						UngetChar(aChar);
						break;
					}
					aDefine += aChar;
				}
				
				if(!GetDefine(aDefine,aVal))
				{
					AbortRead("Define not found: " + aDefine);
					return false;
				}

				theToken += aVal;
				continue;
			}
		}
		
		if(!skip)
			theToken += aChar;
	}

	return skip || !theToken.empty();
}

