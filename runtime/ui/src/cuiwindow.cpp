//-------------------------------------------------------------------
//
//   MODULE    : CUIWINDOW.CPP
//
//   PURPOSE   : implements the CUIWindow bridge class
//
//   CREATED   : 1/01 
//
//   COPYRIGHT : (C) 2001 LithTech Inc
//
//-------------------------------------------------------------------


#ifndef __CUIWINDOW_H__
#include "cuiwindow.h"
#endif

#ifndef __CUIWINDOW_IMPL_H__
#include "cuiwindow_impl.h"
#endif


//  ---------------------------------------------------------------------------
CUIWindow::CUIWindow(CUIGUID guid, HTEXTURE skin, char* table)
{
	LT_MEM_TRACK_ALLOC(m_pImpl = new CUIWindow_Impl(this, guid, skin, table),LT_MEM_TYPE_UI);
}


//  ---------------------------------------------------------------------------
CUIWindow::~CUIWindow()
{

}


//  ---------------------------------------------------------------------------
//	The following functions are all bridge functions.  The widget does nothing
//	on its own.  This is to keep all implementation out of the SDK directory.
//  ---------------------------------------------------------------------------

