#include "paramlist.h"
#include <string.h>
#include <assert.h>
#include <ctype.h>

//maximum number of args that can be pulled out of a string
#define MAX_ARGS 1024

CParamList::CParamList() :
	m_ppszArgV(NULL),
	m_nArgC(0)
{
}

CParamList::CParamList(const char* pszString) :
	m_ppszArgV(NULL),
	m_nArgC(0)
{
	Init(pszString);
}

CParamList::CParamList(uint32 nArgc, const char** pszArgv) :
	m_ppszArgV(NULL),
	m_nArgC(0)
{
	Init(nArgc, pszArgv);
}

CParamList::~CParamList()
{
	Free();
}

//utility function that will add a character to a buffer, make sure there is enough room,
//and append a trailing end string on the end
static bool AppendCharacter(char* pszBuffer, char ch, uint32 nBufferLen)
{
	if(nBufferLen < 2)
		return false;

	uint32 nCurrLen = strlen(pszBuffer);

	if(nCurrLen >= nBufferLen - 2)
		return false;

	//there is room
	pszBuffer[nCurrLen]		= ch;
	pszBuffer[nCurrLen + 1] = '\0';

	return true;
}

static bool AppendSlashes(char* pszBuffer, uint32 nSlashCount, uint32 nBufferLen)
{
	//see if we need to add our slashes in
	uint32 nSlashToWrite = (nSlashCount % 2) ? nSlashCount : nSlashCount / 2;

	for(uint32 nCurrSlash = 0; nCurrSlash < nSlashToWrite; nCurrSlash++)
		AppendCharacter(pszBuffer, '\\', nBufferLen);

	return true;
}

//given a string it will take it and break it up into a command list much like that
//which is passed into applications at start time
bool CParamList::Init(const char* pszString)
{
	//for the holding of strings as they are created
	char* pszStrBuffer[MAX_ARGS];
	uint32 nBufferIndex = 0;

	uint32 nStrLen = strlen(pszString);

	//inside quotes or not
	bool	bInQuotes = false;
	uint32	nSlashCount = 0;

	//a working buffer for the current parameter
	static const uint32 knCurrParamLength = 512;
	char	pszCurrParam[knCurrParamLength] = "";

	//run through and count up the number of arguments, and build up strings
	for(uint32 nCurrChar = 0; nCurrChar <= nStrLen; nCurrChar++)
	{
		char cCurr = pszString[nCurrChar];

		//flag to indicate if we should add this string
		bool bAddString = false;

		//see if the current character is a space and we aren't in quotes (quotes ignore
		//spaces, and treat them normally
		if((cCurr == '\0') || (isspace(cCurr) && !bInQuotes))
		{
			if(strlen(pszCurrParam) > 0)
			{
				bAddString = true;
			}			
		}
		else
		{
			//if this is a slash, add it to our slash count
			if(cCurr == '\\')
			{
				//we can't add it to our string since the number of slashes to output is dependant
				//upon the number of slashes that are in a sequence
				nSlashCount++;
			}
			else
			{
				//not a space, see if it is quotes
				if((cCurr == '\"') && ((nSlashCount % 2) == 0))
				{
					//see if we need to add our slashes in
					if(nSlashCount)
					{
						AppendSlashes(pszCurrParam, nSlashCount - 1, knCurrParamLength);
						nSlashCount = 0;
					}

					bInQuotes = !bInQuotes;

					//see if we are leaving the quote
					if(!bInQuotes)
					{
						//we need to add this parameter
						bAddString = true;
					}
				}
				else
				{
					//see if we need to add our slashes in
					AppendSlashes(pszCurrParam, nSlashCount, knCurrParamLength);
					nSlashCount = 0;

					//just another character, add it to our string
					AppendCharacter(pszCurrParam, cCurr, knCurrParamLength);
				}
			}
		}

		//see if we were building up a string
		if(bAddString)
		{
			//we need to allocate this string, copy it over, and add it to the list
			if(nBufferIndex < MAX_ARGS)
			{
				//copy over our working string and clear it out
				pszStrBuffer[nBufferIndex] = new char[strlen(pszCurrParam) + 1];
				strcpy(pszStrBuffer[nBufferIndex], pszCurrParam);
				pszCurrParam[0] = '\0';

				//move onto the next buffer
				nBufferIndex++;
			}
		}
	}

	//okay, now all we need to do is make the final list
	m_ppszArgV = new char* [nBufferIndex];

	for(uint32 nCurrStr = 0; nCurrStr < nBufferIndex; nCurrStr++)
	{
		m_ppszArgV[nCurrStr] = pszStrBuffer[nCurrStr];
	}

	//save the count
	m_nArgC = nBufferIndex;

	//success
	return true;
}

//similar but takes the dos style command line parameters
bool CParamList::Init(uint32 nArgc, const char** pszArgv)
{
	Free();

	//just duplicate the strings
	m_ppszArgV = new char* [nArgc];

	//memory check
	if(m_ppszArgV == NULL)
		return false;

	for(uint32 nCurrStr = 0; nCurrStr < nArgc; nCurrStr++)
	{
		m_ppszArgV[nCurrStr] = new char[strlen(pszArgv[nCurrStr]) + 1];
		if(m_ppszArgV[nCurrStr])
			strcpy(m_ppszArgV[nCurrStr], pszArgv[nCurrStr]);
	}

	//update the number
	m_nArgC = nArgc;
	
	return true;
}

//frees all memory associated with this object
void CParamList::Free()
{
	for(uint32 nCurrArg = 0; nCurrArg < GetNumParams(); nCurrArg++)
	{
		delete [] m_ppszArgV[nCurrArg];
	}
	delete [] m_ppszArgV;
	m_ppszArgV = NULL;

	m_nArgC = 0;
}


//given a string to match, it will pick out the matching
//parameter and return the index of it, or -1 if not found
int32 CParamList::FindParam(const char* pszName) const
{
	for(uint32 nCurrArg = 0; nCurrArg < GetNumParams(); nCurrArg++)
	{
		if(stricmp(pszName, GetParameter(nCurrArg)) == 0)
		{
			return nCurrArg;
		}
	}
	return -1;
}

//gets the number of parameters
uint32 CParamList::GetNumParams() const
{
	return m_nArgC;
}

//gets a specific parameter
const char* CParamList::GetParameter(uint32 nIndex) const
{
	if(nIndex < GetNumParams())
	{
		return m_ppszArgV[nIndex];
	}
	assert(false);
	return NULL;
}
