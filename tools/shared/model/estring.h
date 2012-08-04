
// This defines the EString class.

#ifndef __ESTRING_H__
#define __ESTRING_H__


	#include "bdefs.h"


	class EString
	{
	public:
					EString()
					{
						m_bAllocated = FALSE;
						m_pString = "";
					}

					~EString()
					{
						Term();
					}

		void		Term();

		EString& 	operator=(char *pStr);
		EString&	operator=(const char *pStr);
		EString& 	operator=(EString &str);

		BOOL		operator==(const EString &other);
		BOOL		operator!=(const EString &other)	{return !(*this == other);}

		char*		Allocate(); // Allocate a char* with the string in it.
					operator char*() {return m_pString;}

		char		*m_pString;
		BOOL		m_bAllocated;
	};


#endif

