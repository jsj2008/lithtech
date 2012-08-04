
#include "estring.h"


// ---------------------------------------------------------------------- //
// EString
// ---------------------------------------------------------------------- //

void EString::Term()
{
	if(m_bAllocated)
	{
		delete m_pString;
	}

	m_pString = "";
	m_bAllocated = FALSE;
}


EString& EString::operator=(const char *pStr)
{
	Term();

	m_pString = new char[strlen(pStr)+1];
	strcpy(m_pString, pStr);
	return *this;
}

EString& EString::operator=(char *pStr)
{
	Term();

	m_pString = new char[strlen(pStr)+1];
	strcpy(m_pString, pStr);
	return *this;
}


EString& EString::operator=(EString &str)
{
	return (*this) = str.m_pString;
}


BOOL EString::operator==(const EString &str)
{
	return strcmp(str.m_pString, m_pString) == 0;
}


char* EString::Allocate()
{
	char *pRet;

	if(pRet = new char[strlen(m_pString)+1])
	{
		strcpy(pRet, m_pString);
	}

	return pRet;
}






