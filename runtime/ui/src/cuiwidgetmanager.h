
//-------------------------------------------------------------------
//
//   MODULE    : CUIWIDGETMANAGER.H
//
//   PURPOSE   : defines the CUIWidgetManager class
//
//   CREATED   : 1/01 
//
//   COPYRIGHT : (C) 2001 LithTech Inc
//
//-------------------------------------------------------------------


#ifndef __CUIWIDGETMANAGER_H__
#define __CUIWIDGETMANAGER_H__


#ifndef __ILTWIDGETMANAGER_H__
#include "iltwidgetmanager.h"
#endif

#ifndef __CUIMESSAGEQUEUE_H__
#include "cuimessagequeue.h"
#endif

#ifndef __CUIREDBLACK_H__
#include "cuiredblack.h"
#endif

#ifndef __CUILINKLIST_H__
#include "cuilinklist.h"
#endif


class CUIWidgetManager : public ILTWidgetManager
{
	public:
		
		// interface database	
		declare_interface(CUIWidgetManager);

		virtual const char* GetClassName() { return "CUIWidgetManager"; }

	public:
		
		CUIWidgetManager();
		~CUIWidgetManager();
					
		uint16			GetScreenWidth();		
		uint16			GetScreenHeight();	
		void			GetScreenDims(uint16* w, uint16* h);	
		
		void			Init(uint16 scrWidth, uint16 scrHeight);
		void			Term();
		bool			IsInitted()		{ return m_bInitted; }

		void* 			LoadUISkin(const char* pFilename);
		CUI_RESULTTYPE	SetCurrentSkin(HTEXTURE hTex);
		HTEXTURE		GetCurrentSkin() { return m_CurrentSkin; }
		
		void			Update();
	
		CUIBase*		CreateWidget(CUIBase*		  pParent,
									 CUI_WIDGETTYPE	  type, 
						  			 CUIRECT*		  pRect,
									 CUIBase*		  pRelativeTo,
		   				  			 uint32			  flags);
		
		CUI_RESULTTYPE	DestroyWidget(CUIBase* pWidget);
		CUI_RESULTTYPE	InternalDestroyWidget(CUIBase* pWidget);
		
		CUIBase*		GetRootWidget();
		CUIBase*		GetWidgetAtCoords(int16 x, int16 y);
		
		CUI_RESULTTYPE	SetFocusWidget(CUIBase* pWidget);
		CUIBase*		GetFocusWidget();


		bool			IsWidgetValid(CUIBase* pWidget);

		
		// add a message to the tail of the queue	
		CUI_RESULTTYPE 	EnqueueMessage(CUIMessage* pMessage);
							
		// remove a message from the head of the queue
		CUI_RESULTTYPE	DequeueMessage(CUIMessage* pMessage);		
						
		// look at the message at the head of the queue
		// but don't remove it!
		CUI_RESULTTYPE	PeekMessage(CUIMessage* pMessage);	

		CUI_RESULTTYPE	SetUserCallback(CUI_CALLBACKTYPE pUserCallback, 
										void* pUserCallbackObject,
										void* pUserCallbackData);

		CUI_CALLBACKTYPE GetUserCallback(void** ppUserCallbackObject,
										 void** ppUserCallbackData);

		void*			GetUserCallbackObject() { return m_pUserCallbackObject; }
		void*			GetUserCallbackData()   { return m_pUserCallbackData; }


		CUI_RESULTTYPE	FocusNext();
		CUI_RESULTTYPE	FocusUp();
		CUI_RESULTTYPE	FocusDown();

	private:
			
		static void		KillGUIDTree(CUIRedBlackNode* node, int32 depth);

		void			DebugShowFocusWidget();
		CUIGUID			GenerateGUID();
		void	 		Render();		
		CUIBase*		GetWidgetAtCoordsHelper(CUIBase* pWidget, int16 x, int16 y);
		
				
	private:

		uint16				m_ScreenWidth;
		uint16				m_ScreenHeight;
		
		bool				m_bInitted;

		HTEXTURE			m_CurrentSkin;
			
		CUIBase*  			m_pRootWidget;
		CUIBase*			m_pFocusWidget;	

		CUIMessageQueue*	m_pMessageQueue;
		CUIRedBlackTree*	m_pGUIDTree;
		uint32				m_GUIDCount;			
		CUILinkList*		m_pDestroyList;

		// message system callback
		CUI_CALLBACKTYPE	m_pUserCallback;
		void*				m_pUserCallbackObject;
		void*				m_pUserCallbackData;

};


#endif //__CUIWIDGETMANAGER_H__
