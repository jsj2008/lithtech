//-------------------------------------------------------------------
//
//   MODULE    : CUIOPTION.CPP
//
//   PURPOSE   : Implements the CUIOption bridge Class
//
//   CREATED   : 4/01 
//
//   COPYRIGHT : (C) 2001 LithTech Inc
//
//-------------------------------------------------------------------


#ifndef __CUIOPTION_H__
#include "cuioption.h"
#endif

#ifndef __CUIOPTION_IMPL_H__
#include "cuioption_impl.h"
#endif


//  ---------------------------------------------------------------------------
CUIOption::CUIOption(CUIGUID guid)
{
	LT_MEM_TRACK_ALLOC(m_pImpl = new CUIOption_Impl(this, guid),LT_MEM_TYPE_UI);
}


//  ---------------------------------------------------------------------------
CUIOption::~CUIOption()
{
	// delete resources
}


//  ---------------------------------------------------------------------------
//	The following functions are all bridge functions.  The widget does nothing
//	on its own.  This is to keep all implementation out of the SDK directory.
//  ---------------------------------------------------------------------------

