#ifndef __PARAMLIST_H__
#define __PARAMLIST_H__

#include "ltinteger.h"

class CParamList
{
public:

	CParamList();
	CParamList(const char* pszString);
	CParamList(uint32 nArgc, const char** pszArgv);
	~CParamList();

	//given a string it will take it and break it up into a command list much like that
	//which is passed into applications at start time
	bool		Init(const char* pszString);

	//similar but takes the dos style command line parameters
	bool		Init(uint32 nArgc, const char** pszArgv);

	//frees all memory associated with this object
	void		Free();

	//given a string to match, it will pick out the matching
	//parameter and return the index of it, or -1 if not found
	int32		FindParam(const char* pszName) const;

	//gets the number of parameters
	uint32		GetNumParams() const;

	//gets a specific parameter
	const char*	GetParameter(uint32 nIndex) const;

	//retreives the list of strings. Useful when that is how you want the parsing to be done
	char**		GetParameterList() 			{return m_ppszArgV;}

private:

	uint32		m_nArgC;
	char**		m_ppszArgV;
};

#endif

