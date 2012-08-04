// OptionsWarnings.h: interface for the COptionsWarnings class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_OPTIONSWARNINGS_H__6F82AB13_032F_11D3_BE1D_0060971BDC6D__INCLUDED_)
#define AFX_OPTIONSWARNINGS_H__6F82AB13_032F_11D3_BE1D_0060971BDC6D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "optionsbase.h"

class COptionsWarnings : public COptionsBase  
{
public:
	COptionsWarnings();
	virtual ~COptionsWarnings();

	// Saves/Loads the options
	BOOL	Load();
	BOOL	Save();
};

#endif // !defined(AFX_OPTIONSWARNINGS_H__6F82AB13_032F_11D3_BE1D_0060971BDC6D__INCLUDED_)
