//-------------------------------------------------------------------
//
//   MODULE    : CUIINTERVAL_IMPL.H
//
//   PURPOSE   : defines the CUIInterval_Impl widget class
//
//   CREATED   : 3/01 
//
//   COPYRIGHT : (C) 2001 LithTech Inc
//
//-------------------------------------------------------------------


#ifndef __CUIINTERVAL_IMPL_H__
#define __CUIINTERVAL_IMPL_H__


#ifndef __CUI_H__
#include "cui.h"
#endif

#ifndef __CUISTATICIMAGE_IMPL_H__
#include "cuistaticimage_impl.h"
#endif


class CUIInterval_Impl : public CUIStaticImage_Impl
{
	public:
			
		CUIInterval_Impl(CUIBase* abstract, CUIGUID guid);
		virtual	~CUIInterval_Impl();
		
		// get data		
		virtual CUI_WIDGETTYPE	GetType() = 0;//	   { return CUIW_INTERVAL; }
		virtual const char*		GetClassName() = 0;//  { return "CUIInterval"; }	

		virtual CUI_RESULTTYPE	GetRange(int32* pMin, int32* pMax);

		virtual int32			GetMaxValue()  { return m_MaxValue; }
		virtual int32			GetMinValue()  { return m_MinValue; }
		virtual int32			GetCurrentValue() { return m_CurrentValue; }

		virtual CUI_ORIENTATIONTYPE	GetOrientation() { return m_Orientation; }


		// set data
		virtual CUI_RESULTTYPE	SetColors(CUI_ELEMENTTYPE elm, 
										  uint32 argb0,
										  uint32 argb1,
										  uint32 argb2,
										  uint32 argb3);

		virtual CUI_RESULTTYPE	SetTexture(CUI_ELEMENTTYPE elm, 
										   HTEXTURE hTex, 
										   CUIRECT* pRect);

		virtual CUI_RESULTTYPE	SetGutter(int16 left, int16 right, int16 top, int16 bottom);


		virtual CUI_RESULTTYPE	SetRange(int32 min, int32 max);

		virtual CUI_RESULTTYPE	SetMaxValue(int32 max);
		virtual CUI_RESULTTYPE	SetMinValue(int32 min);
		virtual CUI_RESULTTYPE	SetCurrentValue(int32 val);

		virtual CUI_RESULTTYPE	SetOrientation(CUI_ORIENTATIONTYPE orient);

		// actions
		virtual CUI_RESULTTYPE	Increment(int32 inc);
		virtual CUI_RESULTTYPE	IncrementPercent(int32 percent);

		virtual CUI_RESULTTYPE	QueryPoint(int16 x, int16 y) = 0;


	protected:	

		virtual void Move(float x, float y);
		virtual void Resize(float w, float h);
		virtual void Draw();

		// must be defined by subclasses
		virtual void RepositionInterval() = 0;

	
	protected:
		
		CUI_ORIENTATIONTYPE	m_Orientation;

		CUIPolyTex			m_DrawInterval;

		int32				m_MinValue;
		int32				m_MaxValue;
		int32				m_CurrentValue;

		// necessary because we sometimes draw a partial texture
		CUIRECT				m_TexRect;

};


#endif //__CUIINTERVAL_IMPL_H__
