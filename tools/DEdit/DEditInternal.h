//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
// DEditInternal.h: Definition of DEditInternal class
//
//////////////////////////////////////////////////////////////////////

#ifndef _DEDITINTERNAL_H_
#define _DEDITINTERNAL_H_

class DEditInternal
{
	public :
	
		DEditInternal()
		{
			m_aszStrings = NULL;
			m_cStrings = 0;
		};

	public :

		unsigned int m_cStrings;
		char** m_aszStrings;
};

#endif
