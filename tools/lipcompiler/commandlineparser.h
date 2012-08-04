//-------------------------------------------------------------------
// CommandLineParser.h
//
// Provides the definition for CCommandLineParser, which provides
// an easy way to take a command line string and get the options and
// parameters, based upon a specified delimiter. The rules for parsing
// are: Any string beginning with the delimiter character denotes an
// option. Any other string is a parameter, and is related to the
// previous option. If there is no previous option, it is a global
// parameter and those can be retreived separately. Note: Designators
// for options CANNOT be whitespace characters.
//
// Created: 1/27/01
// Author: John O'Rorke
// Modification History:
//
//-------------------------------------------------------------------
#ifndef __COMMANDLINEPARSER_H__
#define __COMMANDLINEPARSER_H__

#include "ltbasedefs.h"

class CCommandLineParser
{
public:

	CCommandLineParser();
	~CCommandLineParser();

	//initializes the command line parser given an entire string
	//(windows style)
	bool Init(const char* pszCommandLine, char nDesignator);

	//initializes the command line parser given a series of multiple
	//strings
	//(dos style)
	bool Init(uint32 nArgCount, char** ppszArgStrings, char nDesignator);

	//determines if a specific option is set
	bool IsOptionSet(const char* pszOption) const;

	//gets the number of parameters that follow the specified option
	uint32 GetNumParameters(const char* pszOption) const;

	//sets the number of parameters for an option. If the existing number
	//of parameters exceeds that number, it will move the extra parameters
	//to the global listing. Returns the number of parameters moved to the
	//global list
	uint32 SetNumParameters(const char* pszOption, uint32 nNumParams);

	//gets the total number of command line options
	uint32 GetNumOptions() const;

	//gets the specified parameter from the specified option. NULL if invalid
	const char* GetParameter(const char* pszOption, uint32 nParameter) const;

	//gets the number of global parameters
	uint32 GetNumGlobalParameters() const;

	//gets a global parameter. NULL if invalid
	const char* GetGlobalParameter(uint32 nIndex) const;

	//gets an option name given its index into the list of options
	const char* GetOptionName(uint32 nIndex) const;

	//maximums
	enum	{	MAX_OPTIONS		= 64,
				MAX_PARAMETERS	= 128
			};

private:


	//the structure for each option
	struct COption
	{
		//constructor
		COption() : m_nPos(0), m_nNumParameters(0) {}

		//position in the command line string
		uint32	m_nPos;

		//number of parameters
		uint32	m_nNumParameters;

		//the offset of each parameter
		uint32	m_nParamOffset[MAX_PARAMETERS];
	};


	//frees all memory associated with the object and resets it to a clean state
	void Free();

	//handles internal initialization of the command line parser
	bool InternalInit(char cDesignator);

	//finds an option given the name, and will return a pointer to it. Will
	//return NULL if no options match
	const COption* GetOption(const char* pszName) const;

	//a copy of the command line option string
	char*		m_pszCommandLine;

	//number of options found in the command line
	uint32		m_nNumOptions;


	//the actual options
	COption		m_Options[MAX_OPTIONS];

	//the global parameters (not bound to any option)
	COption		m_GlobalParams;

};

#endif
