//-------------------------------------------------------------------
//
//   MODULE    : CUIPROGRESS_IMPL.H
//
//   PURPOSE   : defines the CUIProgressImage_Impl widget class
//
//   CREATED   : 3/01 
//
//   COPYRIGHT : (C) 2001 LithTech Inc
//
//-------------------------------------------------------------------


#ifndef __CUIPROGRESS_IMPL_H__
#define __CUIPROGRESS_IMPL_H__


#ifndef __CUI_H__
#include "cui.h"
#endif

#ifndef __CUIINTERVAL_IMPL_H__
#include "cuiinterval_impl.h"
#endif


class CUIProgress_Impl : public CUIInterval_Impl
{
	public:
			
		CUIProgress_Impl(CUIBase* abstract, CUIGUID guid);
		virtual	~CUIProgress_Impl();
		
		// get data		
		virtual CUI_WIDGETTYPE	GetType()	   { return CUIW_PROGRESS; }
		virtual const char*		GetClassName() { return "CUIProgress"; }	

		virtual CUI_RESULTTYPE	SetGutter(int16 left, int16 right, int16 top, int16 bottom);

		virtual CUI_RESULTTYPE	SetStretchMode(CUI_STRETCHMODE stretch);
		virtual CUI_STRETCHMODE	GetStretchMode() { return m_Stretch; }
		
		virtual CUI_RESULTTYPE	SetFillMode(CUI_FILLMODE fill);
		virtual CUI_FILLMODE	GetFillMode() { return m_Fill; }


		virtual CUI_RESULTTYPE	QueryPoint(int16 x, int16 y);


	protected:	

		virtual void RepositionInterval();


	protected:
		
		CUI_STRETCHMODE		m_Stretch;
		CUI_FILLMODE		m_Fill;

};


#endif //__CUIPROGRESS_IMPL_H__
