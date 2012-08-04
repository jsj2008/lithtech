//-------------------------------------------------------------------
//
//   MODULE    : CUISTATICIMAGE.CPP
//
//   PURPOSE   : implements the CUIStaticImage bridge class
//
//   CREATED   : 1/01 
//
//   COPYRIGHT : (C) 2001 LithTech Inc
//
//-------------------------------------------------------------------


#ifndef __CUISTATICIMAGE_H__
#include "cuistaticimage.h"
#endif

#ifndef __CUISTATICIMAGE_IMPL_H__
#include "cuistaticimage_impl.h"
#endif


//  ---------------------------------------------------------------------------
CUIStaticImage::CUIStaticImage() 
{
	// this is only for subclasses.  do not use.
}


//  ---------------------------------------------------------------------------
CUIStaticImage::CUIStaticImage(CUIGUID guid)
{
	LT_MEM_TRACK_ALLOC(m_pImpl = new CUIStaticImage_Impl(this, guid),LT_MEM_TYPE_UI);
}


//  ---------------------------------------------------------------------------
CUIStaticImage::~CUIStaticImage()
{
	
}


//  ---------------------------------------------------------------------------
//	The following functions are all bridge functions.  The widget does nothing
//	on its own.  This is to keep all implementation out of the SDK directory.
//  ---------------------------------------------------------------------------


