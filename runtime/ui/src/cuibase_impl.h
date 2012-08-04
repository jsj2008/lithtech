//-------------------------------------------------------------------
//
//   MODULE    : CUIBASE_IMPL.H
//
//   PURPOSE   : defines the CUIBase_Impl base widget class
//
//   CREATED   : 1/01 
//
//   COPYRIGHT : (C) 2001 LithTech Inc
//
//-------------------------------------------------------------------


//	implements platform-independent portions of the UI base Widget Class
//	this class provides messaging & positioning functions, but none of the
//	visible UI functions are actually defined.

//	this class has pure virtual functions and cannot be instantiated


#ifndef __CUIBASE_IMPL_H__
#define __CUIBASE_IMPL_H__


#ifndef __CUI_H__
#include "cui.h"
#endif

#ifndef __CUIBASE_H__
#include "cuibase.h"
#endif

#ifndef __CUILINKLIST_H__
#include "cuilinklist.h"
#endif


class CUIBase_Impl
{
	public:
	
		CUIBase_Impl(CUIBase* abstract, CUIGUID guid);
		virtual ~CUIBase_Impl();
		
		// get data
		virtual CUI_WIDGETTYPE	GetType()		= 0;
		virtual const char*		GetClassName()	= 0;
		
		CUIBase*				GetParent();
		CUIBase*				GetFirstChild();
		CUIBase*				GetNextChild();
		CUILinkList*			GetChildList();		
		
		CUI_RESULTTYPE 			GetPosition(float *x, float *y, CUIBase* pRelativeTo);

		CUI_RESULTTYPE 			GetSize(float *w, float *h);
		CUI_RESULTTYPE			GetRect(CUIRECT* pRect, CUIBase* pRelativeTo);
		uint32					GetState() { return m_StateFlags; }

		CUIGUID					GetGUID() { return m_GUID; }
		
		
		virtual HTEXTURE		GetTexture(CUI_ELEMENTTYPE elm) = 0;
		
		virtual CUI_RESULTTYPE	GetGutter(int16 *pLeft, int16 *pRight, int16 *pTop, int16 *pBottom);
		
		CUI_CALLBACKTYPE		GetUserCallback(void** ppUserCallbackObject,
												void** ppUserCallbackData);

		void*					GetUserCallbackObject() { return m_pUserCallbackObject; }
		void*					GetUserCallbackData()   { return m_pUserCallbackData; }


		// set data
		CUI_RESULTTYPE			SetPosition(float x, 
											float y, 
											CUIBase* pRelativeTo,
											bool bMoveChildren);

		CUI_RESULTTYPE			SetSize(float w, float h);
		
		CUI_RESULTTYPE			SetRect(CUIRECT* r, 
										CUIBase* pRelativeTo,
										bool bMoveChildren);
		
		CUI_RESULTTYPE			SetUserCallback(CUI_CALLBACKTYPE pUserCallback, 
												void* pUserCallbackObject,
												void* pUserCallbackData);

		CUI_RESULTTYPE			SetParent(CUIBase* parent);


		virtual CUI_RESULTTYPE	SetState(uint32 flags);
		virtual CUI_RESULTTYPE	UnsetState(uint32 flags);
		
		virtual CUI_RESULTTYPE 	SetColor(CUI_ELEMENTTYPE elm, uint32 argb) = 0;
	
		virtual CUI_RESULTTYPE 	SetColors(CUI_ELEMENTTYPE elm, 
										  uint32 argb0,
										  uint32 argb1,
										  uint32 argb2,
										  uint32 argb3) = 0;
		
		virtual CUI_RESULTTYPE	SetTexture(CUI_ELEMENTTYPE elm, 
							   		   HTEXTURE hTex, 
							   		   bool tile) = 0;

		virtual CUI_RESULTTYPE	SetTexture(CUI_ELEMENTTYPE elm, 
							   		   HTEXTURE hTex, 
							   		   CUIRECT* pRect) = 0;

		virtual CUI_RESULTTYPE	SetGutter(int16 left, int16 right, int16 top, int16 bottom);


		// actions
		CUI_RESULTTYPE			BringToFront();
		CUI_RESULTTYPE	  		SendToBack();	
		virtual void  			Render();
		virtual void 			Destroy(); 

		CUI_RESULTTYPE			Show(bool bShowChildren);
		CUI_RESULTTYPE			Hide(bool bHideChildren);
		
		virtual CUI_RESULTTYPE	HandleMessage(CUIMessage* pMessage);
		
		CUI_RESULTTYPE			SendMessageToChildren(CUIMessage* pMessage);
								  							  
	protected:

		// override these to do your own thang...
		virtual void			Draw() = 0;
		virtual void			Move(float x, float y) = 0;
		virtual void			Resize(float w, float h) = 0;
		
	protected:
	
		// message system callback
		CUI_CALLBACKTYPE	m_pUserCallback;
		void*				m_pUserCallbackObject;
		void*				m_pUserCallbackData;

		// window housekeeping
		CUIBase*			m_pParent;
		CUILinkList*		m_pChildren;
		CUIBase*			m_pAbstract;
		CUIBase*			m_pListIterator;
		uint32	   			m_StateFlags;
		uint32				m_BGColor;
		CUIGUID				m_GUID;

		// dimensions
		int16				m_GutterRight;
		int16				m_GutterLeft;
		int16				m_GutterTop;
		int16				m_GutterBottom;

		CUIRECT				m_Rect;
};

#endif //__CUIBASE_IMPL_H__
