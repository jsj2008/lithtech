#include "commandlineparser.h"
#include <ctype.h>

CCommandLineParser::CCommandLineParser() :
	m_pszCommandLine(NULL),
	m_nNumOptions(0)
{
}

CCommandLineParser::~CCommandLineParser()
{
	Free();
}


//initializes the command line parser given an entire string
//(windows style)
bool CCommandLineParser::Init(const char* pszCommandLine, char nDesignator)
{
	//clean up the old stuff
	Free();

	//first off, we need to create our own internal copy of the string
	uint32 nStrLen = strlen(pszCommandLine);

	//allocate the memory
	m_pszCommandLine = new char[nStrLen + 1];

	//make sure the memory was allocated ok
	if(m_pszCommandLine == NULL)
	{
		return false;
	}

	//copy the string over
	strcpy(m_pszCommandLine, pszCommandLine);

	//now pass along to our internal initializer
	return InternalInit(nDesignator);	
}


//initializes the command line parser given a series of multiple
//strings
//(dos style)
bool CCommandLineParser::Init(uint32 nArgCount, char** ppszArgStrings, char nDesignator)
{
	//clean up old stuff
	Free();

	//we need to run through all the strings, and build up the size 
	//of the total string we will need
	uint32 nTotalSize = 0;
	uint32 nCurrStr	  = 0;

	for(; nCurrStr < nArgCount; nCurrStr++)
	{
		//add an extra character to insert a space on the end
		nTotalSize += strlen(ppszArgStrings[nCurrStr]) + 1;
	}

	//allocate the string
	m_pszCommandLine = new char[nTotalSize + 1];

	//make sure the memory was allocated ok
	if(m_pszCommandLine == NULL)
	{
		return false;
	}

	//init the string to be empty
	m_pszCommandLine[0] = '\0';

	//now copy over all the strings
	uint32 nCurrPos = 0;
	for(nCurrStr = 0; nCurrStr < nArgCount; nCurrStr++)
	{
		strcat(m_pszCommandLine, ppszArgStrings[nCurrStr]);
		strcat(m_pszCommandLine, " ");
	}

	//now pass it on to the internal constructor
	return InternalInit(nDesignator);
}


//determines if a specific option is set
bool CCommandLineParser::IsOptionSet(const char* pszOption) const
{
	return GetOption(pszOption) ? true : false;
}


//gets the number of parameters that follow the specified option
uint32 CCommandLineParser::GetNumParameters(const char* pszOption) const
{
	//get the option
	const COption* pOpt = GetOption(pszOption);

	//see if we got one
	if(pOpt)
	{
		return pOpt->m_nNumParameters;
	}

	//no option found
	return 0;
}

//sets the number of parameters for an option. If the existing number
//of parameters exceeds that number, it will move the extra parameters
//to the global listing. Returns the number of parameters moved to the
//global list
uint32 CCommandLineParser::SetNumParameters(const char* pszOption, uint32 nNumParams)
{
	//try and find the option
	COption* pOpt = (COption*)GetOption(pszOption);

	//see if we found it
	if(pOpt == NULL)
	{
		return 0;
	}

	//now see if we exceed the limit
	if(pOpt->m_nNumParameters <= nNumParams)
	{
		return 0;
	}

	//we need to now move the extra number of parameters over to the global ones
	uint32 nNumMoved = 0;

	for(uint32 nCurrParam = nNumParams; nCurrParam < pOpt->m_nNumParameters; nCurrParam++)
	{
		//try and add it to the global listing
		if(m_GlobalParams.m_nNumParameters >= MAX_PARAMETERS)
		{
			//can't move any more
			break;
		}

		//move the offset
		m_GlobalParams.m_nParamOffset[m_GlobalParams.m_nNumParameters] =
			pOpt->m_nParamOffset[nCurrParam];

		//update the counds
		m_GlobalParams.m_nNumParameters++;
		nNumMoved++;
	}

	//update the option to hold the correct amount
	pOpt->m_nNumParameters = nNumParams;

	return nNumMoved;
}


//gets the total number of command line options
uint32 CCommandLineParser::GetNumOptions() const
{
	return m_nNumOptions;
}


//gets the specified parameter from the specified option
const char* CCommandLineParser::GetParameter(const char* pszOption, uint32 nParameter) const
{
	//get the option
	const COption* pOpt = GetOption(pszOption);

	//see if we got one
	if(pOpt)
	{
		//determine if the index is valid
		if(nParameter < pOpt->m_nNumParameters)
		{
			//valid
			return &m_pszCommandLine[pOpt->m_nParamOffset[nParameter]];
		}
	}

	//invalid index or name
	return NULL;
}

//gets the number of global parameters
uint32 CCommandLineParser::GetNumGlobalParameters() const
{
	return m_GlobalParams.m_nNumParameters;
}

//gets a global parameter. NULL if invalid
const char* CCommandLineParser::GetGlobalParameter(uint32 nIndex) const
{
	if(nIndex < GetNumGlobalParameters())
	{
		return &(m_pszCommandLine[m_GlobalParams.m_nParamOffset[nIndex]]);
	}

	//out of bounds
	return NULL;
}

//frees all memory associated with the object and resets it to a clean state
void CCommandLineParser::Free()
{
	//clean up the command line string
	delete [] m_pszCommandLine;
	m_pszCommandLine = NULL;

	m_nNumOptions = 0;
}

//finds an option given the name, and will return a pointer to it. Will
//return NULL if no options match
const CCommandLineParser::COption* CCommandLineParser::GetOption(const char* pszName) const
{
	for(uint32 nCurrOpt = 0; nCurrOpt < GetNumOptions(); nCurrOpt++)
	{
		if(stricmp(&m_pszCommandLine[m_Options[nCurrOpt].m_nPos], pszName) == 0)
		{
			//match found
			return &(m_Options[nCurrOpt]);
		}
	}

	//no match found
	return NULL;
}

//gets an option name given its index into the list of options
const char* CCommandLineParser::GetOptionName(uint32 nIndex) const
{
	//make sure it is in range
	if(nIndex < GetNumOptions())
	{
		//get the option's name
		return &m_pszCommandLine[m_Options[nIndex].m_nPos];
	}

	//out of bounds
	return NULL;
}


//handles internal initialization of the command line parser
bool CCommandLineParser::InternalInit(char cDesignator)
{
	//sanity check, since designators CANNOT be space characters
	ASSERT(!isspace(cDesignator));

	//first replace all spaces with end of string
	//this allows for returning subsets of the string without having
	//to do all sorts of extra code
	uint32 nStrLen = strlen(m_pszCommandLine);

	uint32 nCurrChar;

	for(nCurrChar = 0; nCurrChar < nStrLen; nCurrChar++)
	{
		if(isspace(m_pszCommandLine[nCurrChar]))
		{
			m_pszCommandLine[nCurrChar] = '\0';
		}
	}

	//now we run through and look for the start of each option
	for(nCurrChar = 0; nCurrChar < nStrLen; nCurrChar++)
	{
		//see if we have a designator preceded by a \0 (or if it is the first
		//character)
		if(	(m_pszCommandLine[nCurrChar] == cDesignator) && 
			((nCurrChar == 0) || (m_pszCommandLine[nCurrChar - 1] == '\0')))
		{
			//found a designator

			//make sure that we aren't adding too many options
			if(m_nNumOptions >= MAX_OPTIONS)
			{
				//too many options
				return false;
			}

			//set up a new option. The +1 skips over the designator
			m_Options[m_nNumOptions].m_nPos = nCurrChar + 1;

			//init the option
			m_Options[m_nNumOptions].m_nNumParameters = 0;

			//move onto the next one
			m_nNumOptions++;
		}
		else if( (m_pszCommandLine[nCurrChar] != '\0') && 
				 ((nCurrChar == 0) || (m_pszCommandLine[nCurrChar - 1] == '\0')))
		{
			//we have a parameter, add it to the latest option, or, if there are
			//no options, just add it to the global parameter list

			COption* pOption = &m_GlobalParams;

			if(m_nNumOptions > 0)
			{
				pOption = &(m_Options[m_nNumOptions - 1]);				
			}
			

			//make sure we have room
			if(pOption->m_nNumParameters >= MAX_PARAMETERS)
			{
				//too many parameters
				return false;
			}

			//add it to the list
			pOption->m_nParamOffset[pOption->m_nNumParameters] = nCurrChar;
			pOption->m_nNumParameters++;
		}
				
	}

	return true;
}


