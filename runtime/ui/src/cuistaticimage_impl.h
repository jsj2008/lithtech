//-------------------------------------------------------------------
//
//   MODULE    : CUISTATICIMAGE_IMPL.H
//
//   PURPOSE   : defines the CUIStaticImage_Impl widget class
//
//   CREATED   : 1/01 
//
//   COPYRIGHT : (C) 2001 LithTech Inc
//
//-------------------------------------------------------------------


#ifndef __CUISTATICIMAGE_IMPL_H__
#define __CUISTATICIMAGE_IMPL_H__


#ifndef __CUI_H__
#include "cui.h"
#endif

#ifndef __CUIWIDGET_IMPL_H__
#include "cuiwidget_impl.h"
#endif

class CUIStaticImage_Impl : public CUIWidget_Impl
{
	public:
			
		CUIStaticImage_Impl(CUIBase* abstract, CUIGUID guid);
		virtual	~CUIStaticImage_Impl();
		
		// get data		
		virtual CUI_WIDGETTYPE	GetType()	   { return CUIW_STATICIMAGE; }
		virtual const char*		GetClassName() { return "CUIStaticImage"; }	
		
	protected:	
						
};


#endif //__CUISTATICIMAGE_IMPL_H__
