//-------------------------------------------------------------------
//
//   MODULE    : CUIWIDGETMANAGER.CPP
//
//   PURPOSE   : implements the CUIWidgetManager widget class
//
//   CREATED   : 1/01 
//
//   COPYRIGHT : (C) 2001 LithTech Inc
//
//-------------------------------------------------------------------


#ifndef __CUI_H__
#include "cui.h"
#endif

#ifndef __CUIDEBUG_H__
#include "cuidebug.h"
#endif

#ifndef __CUIWIDGETMANAGER_H__
#include "cuiwidgetmanager.h"
#endif

#ifndef __CUIMESSAGEQUEUE_H__
#include "cuimessagequeue.h"
#endif

#ifndef __CUIRENDERSTATE_H__
#include "cuirenderstate.h"
#endif

#ifndef __CUIBASE_IMPL_H__
#include "cuibase_impl.h"
#endif


// widgets I can create...

#ifndef __CUIWINDOW_H__
#include "cuiwindow.h"
#endif

#ifndef __CUIBUTTON_H__
#include "cuibutton.h"
#endif

#ifndef __CUISTATICTEXT_H__
#include "cuistatictext.h"
#endif

#ifndef __CUISTATICIMAGE_H__
#include "cuistaticimage.h"
#endif

#ifndef __CUISLIDER_H__
#include "cuislider.h"
#endif

#ifndef __CUIPROGRESS_H__
#include "cuiprogress.h"
#endif

#ifndef __CUICHECK_H__
#include "cuicheck.h"
#endif

#ifndef __CUIOPTION_H__
#include "cuioption.h"
#endif

#ifndef __CUILIST_H__
#include "cuilist.h"
#endif

#ifndef __CUIDROPDOWNLIST_H__
#include "cuidropdownlist.h"
#endif

// interface database
define_interface(CUIWidgetManager, ILTWidgetManager); 
					
//static CUIMessageQueue *pMessageQueue = NULL;
//define_holder(CUIMessageQueue, pMessageQueue);

static ILTDrawPrim *pDrawPrimInternal;
define_holder_to_instance(ILTDrawPrim, pDrawPrimInternal, Internal);


//  ---------------------------------------------------------------------------
CUIWidgetManager::CUIWidgetManager()
{
	// CUIWidgetManager::Init() must be called next!
	m_bInitted = false;
}


//  ---------------------------------------------------------------------------
CUIWidgetManager::~CUIWidgetManager()
{	
	// CUIWidgetManager::Shutdown() must be called first!
	if (m_bInitted) {
		CUI_ERR("WIdgetManager was destroyed without calling ::Term() !!\n");
	}
}


//  ---------------------------------------------------------------------------
void CUIWidgetManager::Init(uint16 scrWidth, uint16 scrHeight)
{
	CUIRECT rect;

	m_ScreenHeight = scrHeight;
	m_ScreenWidth  = scrWidth;
	
	// create the GUID storage area (RedBlack Tree)
	LT_MEM_TRACK_ALLOC(m_pGUIDTree = new CUIRedBlackTree(),LT_MEM_TYPE_UI);
	m_GUIDCount = 0;
	
	// create the destroylist
	LT_MEM_TRACK_ALLOC(m_pDestroyList = new CUILinkList(),LT_MEM_TYPE_UI);
	
	// create the messagequeue
	LT_MEM_TRACK_ALLOC(m_pMessageQueue = new CUIMessageQueue(),LT_MEM_TYPE_UI);

	// initialize the user-callback
	m_pUserCallback       = NULL;
	m_pUserCallbackObject = NULL;
	m_pUserCallbackData	  = NULL;

	// no default skin
	m_CurrentSkin = NULL;

	// create the desktop widget	
	rect.x = 0;
	rect.y = 0;
	rect.width  = scrWidth;
	rect.height = scrHeight;
	
	// a sneaky little shuffle-- do this here so we can create the root widget.
	m_bInitted = true;

	m_pRootWidget = this->CreateWidget(
		NULL, CUIW_WINDOW, &rect, NULL, CUIS_ENABLED | CUIS_TREE_VISIBLE);
		
	if (!m_pRootWidget) {
		m_bInitted = false;
		return;
	}

	m_pRootWidget->Hide(false);
	
	
}


//  ---------------------------------------------------------------------------
void CUIWidgetManager::Term()
{
	if (!m_bInitted) {
		CUI_ERR("WidgetManager was never Initialized!\n");
		return;
	}

	// ok, we need to kill all our known widgets (this should delete all the
	// widgets waiting in the destroy list, because they have not yet been
	// removed from the GUIDTree
	m_pGUIDTree->Postfix(KillGUIDTree, m_pGUIDTree->m_pRoot, 0);
	
	// delete the destroy list
	delete m_pDestroyList;
	m_pDestroyList = NULL;
	
	// delete the GUID Tree
	delete m_pGUIDTree;
	m_pGUIDTree = NULL;

	m_bInitted = false;
}


//	---------------------------------------------------------------------------
//	STATIC MEMBER FUNCTION
void CUIWidgetManager::KillGUIDTree(CUIRedBlackNode* node, int32 depth)
{
	if (!node) return;
	if (!node->m_pData) return;

	CUIBase* pWidget= (CUIBase*)node->m_pData;

	delete pWidget;
}


//  ---------------------------------------------------------------------------
uint16 CUIWidgetManager::GetScreenWidth()
{
	if (!m_bInitted) {
		CUI_ERR("WidgetManager was never Initialized!\n");
	}

	return m_ScreenWidth;
}
		

//  ---------------------------------------------------------------------------
uint16 CUIWidgetManager::GetScreenHeight()
{
	if (!m_bInitted) {
		CUI_ERR("WidgetManager was never Initialized!\n");
	}

	return m_ScreenHeight;
}
	

//  ---------------------------------------------------------------------------
void CUIWidgetManager::GetScreenDims(uint16* w, uint16* h)
{
	if (!m_bInitted) {
		CUI_ERR("WidgetManager was never Initialized!\n");
	}

	*w = m_ScreenWidth;
	*h = m_ScreenHeight;
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE	CUIWidgetManager::SetCurrentSkin(HTEXTURE hTex)
{
	m_CurrentSkin = hTex;	

	return CUIR_OK;
}


//  ---------------------------------------------------------------------------
void* 	CUIWidgetManager::LoadUISkin(const char* pFilename)
{
	return NULL;
}


//  ---------------------------------------------------------------------------
CUIBase* CUIWidgetManager::CreateWidget(
		CUIBase*		 pParent,
		CUI_WIDGETTYPE	 type, 
		CUIRECT*		 pRect, 
		CUIBase*		 pRelativeTo,
		uint32			 flags)
{
	
	CUIBase*	pWidget = NULL;
	CUIGUID		guid   = GenerateGUID();
	
	CUIRECT		rect;
	memcpy(&rect, pRect, sizeof(CUIRECT));

	if (!m_bInitted) {
		CUI_ERR("WidgetManager was never Initialized!\n");
		return NULL;
	}

	// what type of widget is the caller asking for?
	switch (type) {
		
		case CUIW_WINDOW:
			LT_MEM_TRACK_ALLOC(pWidget = new CUIWindow(guid, m_CurrentSkin, NULL),LT_MEM_TYPE_UI);
			break;
			
		case CUIW_STATICTEXT:
			LT_MEM_TRACK_ALLOC(pWidget = new CUIStaticText(guid),LT_MEM_TYPE_UI);
			break;
			
		case CUIW_STATICIMAGE:
			LT_MEM_TRACK_ALLOC(pWidget = new CUIStaticImage(guid),LT_MEM_TYPE_UI);
			break;
		
		case CUIW_BUTTON:	
			LT_MEM_TRACK_ALLOC(pWidget = new CUIButton(guid),LT_MEM_TYPE_UI);
			break;

		case CUIW_SLIDER:	
			LT_MEM_TRACK_ALLOC(pWidget = new CUISlider(guid),LT_MEM_TYPE_UI);
			break;

		case CUIW_PROGRESS:	
			LT_MEM_TRACK_ALLOC(pWidget = new CUIProgress(guid),LT_MEM_TYPE_UI);
			break;

		case CUIW_CHECK:	
			LT_MEM_TRACK_ALLOC(pWidget = new CUICheck(guid),LT_MEM_TYPE_UI);
			break;

		case CUIW_OPTION:	
			LT_MEM_TRACK_ALLOC(pWidget = new CUIOption(guid),LT_MEM_TYPE_UI);
			break;

		case CUIW_LIST:	
			LT_MEM_TRACK_ALLOC(pWidget = new CUIList(guid),LT_MEM_TYPE_UI);
			break;

		case CUIW_DROPDOWNLIST:	
			LT_MEM_TRACK_ALLOC(pWidget = new CUIDropDownList(guid),LT_MEM_TYPE_UI);
			break;
			
		default:
			CUI_ERR("CreateWidget got unknown CUI_WIDGETTYPE.\n");
	}
		
	if (!pWidget) {
		CUI_ERR("Could not allocate new widget\n");
		return NULL;
	}
	
		
	// SKINZ!!
	/*
	if (m_CurrentSkin) {
		
		
		switch (type) {

			case CUIW_WINDOW:
							
				break;

			case CUIW_STATICTEXT:
				
				break;

			case CUIW_CURSOR:
				
				break;

			case CUIW_BUTTON:	
				
				break;

		}
	}
	*/
	
	// set some common members
	
	// must set the parent first, because some members depend on it,
	// list position
	pWidget->SetParent(pParent);

	/*
	if (pParent) {
		// UI Widgets are initially positioned relatively to their parents
		CUIRECT ParentRect;
		pParent->GetRect(&ParentRect, CUIP_SCREEN);

		rect.x += ParentRect.x;
		rect.y += ParentRect.y;
	}
*/
	pWidget->SetRect(&rect, pRelativeTo, false); // shouldn't have children at this point!
	
	// add the widget to the GUID list
	m_pGUIDTree->Insert(guid, pWidget);

	// set the widget's flags
	pWidget->SetState(flags);

	// send the creation message
	CUIMessage* pMessage1;
	LT_MEM_TRACK_ALLOC(pMessage1 = new CUIMessage(CUIM_WIDGET_ON_CREATE, pWidget, pWidget, 0, 0),LT_MEM_TYPE_UI);
	this->EnqueueMessage(pMessage1);
	delete pMessage1;
	
	return pWidget;
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIWidgetManager::DestroyWidget(CUIBase* pWidget)
{
	// this function does several checks to prevent, if at all possible,
	// the calling of all these deletion routines on a possibly bogus
	// window pointer.  Once a window is deleted, its ID is removed
	// from the GUID tree and it can't be "deleted" again.  Furthermore,
	// no other window will ever have the same GUID and be accidentally
	// destroyed.
	
	CUIBase*	 widget_compare;
	CUIGUID		 guid;

	CUIBase*	 child_widget;
	CUILinkList* child_list;
	CUIListNode* node;	

	if (!m_bInitted) {
		CUI_ERR("WidgetManager was never Initialized!\n");
		return CUIR_ILT_WIDGETMANAGER_ERROR;
	}

	// is the pointer valid?
	if (!pWidget) {
		return CUIR_INVALID_POINTER;
	}

	// is the CUIGUID real?
	guid = pWidget->GetGUID();
	
	if (CUI_RB_KEY_NOT_FOUND == m_pGUIDTree->Find(guid, (void**)(&widget_compare))) {
		CUI_ERR("Tried to destroy nonexistent widget\n");
		return CUIR_NONEXISTENT_WIDGET;
	}
	
	if (pWidget != widget_compare) {
		CUI_ERR("attempting to destroy a widget that does not match the GUID tree\n");
		return CUIR_WIDGET_MISMATCH;
	}

	// destroy my children first, otherwise they will be left dangling
	child_list = pWidget->GetImpl()->GetChildList();
	if (child_list) {
	
		node = child_list->GetHead();
	
		while (node) {
			child_widget = (CUIBase*)child_list->GetData(node);
			this->DestroyWidget(child_widget);
			child_list->Remove(node);
			node = child_list->GetHead();
		}
	}


	// enqueue an ON_DESTROY message
	CUIMessage* pMessage1;
	LT_MEM_TRACK_ALLOC(pMessage1 = new CUIMessage(CUIM_WIDGET_ON_DESTROY, pWidget, pWidget, 0, 0),LT_MEM_TYPE_UI);
	this->EnqueueMessage(pMessage1);
	delete pMessage1;

	// enqueue an internal SCHEDULE_FOR_DESTROY
	CUIMessage* pMessage2;
	LT_MEM_TRACK_ALLOC(pMessage2 = new CUIMessage(CUIM_WIDGET_SCHEDULE_FOR_DESTROY, pWidget, pWidget, 0, 0),LT_MEM_TYPE_UI);
	this->EnqueueMessage(pMessage2);
	delete pMessage2;

	return CUIR_OK;	
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIWidgetManager::InternalDestroyWidget(CUIBase* pWidget)
{
	CUIBase*	 parent;
	CUILinkList* child_list;

	if (!pWidget) {
		return CUIR_INVALID_POINTER;
	}

	// remove the widget from the parent's child-list
	parent     = pWidget->GetParent();
	child_list = parent->GetImpl()->GetChildList();		
	child_list->Remove(child_list->Find(pWidget));


	// remove the widget's GUID from the GUID tree
	m_pGUIDTree->Remove(pWidget->GetGUID());
	
	// delete the widget's resources
	delete pWidget;
	
	return CUIR_OK;
}


//  ---------------------------------------------------------------------------
CUIBase* CUIWidgetManager::GetFocusWidget()
{
	if (!m_bInitted) {
		CUI_ERR("WidgetManager was never Initialized!\n");
		return NULL;
	}

	return m_pFocusWidget;
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIWidgetManager::SetFocusWidget(CUIBase* widget)
{
	if (!m_bInitted) {
		CUI_ERR("WidgetManager was never Initialized!\n");
		return CUIR_ILT_WIDGETMANAGER_ERROR;
	}

	// send a lost focus message
	CUIMessage* lost_focus_message;
	LT_MEM_TRACK_ALLOC(lost_focus_message = new CUIMessage(CUIM_LOST_FOCUS, m_pFocusWidget, m_pFocusWidget, (uint32)m_pFocusWidget, 0),LT_MEM_TYPE_UI);

	EnqueueMessage(lost_focus_message);
	delete lost_focus_message;

	// change the focus
	m_pFocusWidget = widget;
	
	// send a got focus message
	CUIMessage* got_focus_message;
	LT_MEM_TRACK_ALLOC(got_focus_message =	new CUIMessage(CUIM_GOT_FOCUS, widget, widget, (uint32)widget, 0),LT_MEM_TYPE_UI);

	EnqueueMessage(got_focus_message);
	delete got_focus_message;


	return CUIR_OK;
}


//  ---------------------------------------------------------------------------
bool CUIWidgetManager::IsWidgetValid(CUIBase* pWidget)
{
	CUIBase*	 widget_compare;
	CUIGUID		 guid;

	if (!m_bInitted) {
		CUI_ERR("WidgetManager was never Initialized!\n");
		return false;
	}
	
	// is the pointer valid?
	if (!pWidget) {
		return false;
	}

	// is the CUIGUID real?
	guid = pWidget->GetGUID();

	if (CUI_RB_OK == m_pGUIDTree->Find(guid, (void**)(&widget_compare))) {

		if (pWidget == widget_compare) {
			return true;
		}
	}

	return false;
}


//  ---------------------------------------------------------------------------
CUIBase* CUIWidgetManager::GetRootWidget()
{
	if (!m_bInitted) {
		CUI_ERR("WidgetManager was never Initialized!\n");
		return NULL;
	}

	return m_pRootWidget;
}


//  ---------------------------------------------------------------------------
CUIBase* CUIWidgetManager::GetWidgetAtCoordsHelper(CUIBase* pWidget, int16 x, int16 y)
{
	// this is a private recursive helper function to traverse the widget hierarchy

	CUIBase*		pReturn = NULL;
	CUILinkList*	pList;
	CUIListNode*	pNode;
	CUIRECT			r;

	if (!pWidget) {
		CUI_ERR("Tried to test coords of NULL widget\n");
		return NULL;
	}

	pList = pWidget->GetImpl()->GetChildList();

	// check my children
	if (pList) {
		pNode = pList->GetTail();
		while (pNode) {
			pReturn = GetWidgetAtCoordsHelper((CUIBase*)pNode->m_pData, x, y);
			if (pReturn) return pReturn;
			pNode = pNode->m_pPrev;
		}
	}

	// check me
	pWidget->GetRect(&r, NULL);

	if (x < r.x || x > r.x + r.width ||
		y < r.y || y > r.y + r.height) {

			return NULL;
	}

	// return the results
	return pWidget;
}


//  ---------------------------------------------------------------------------
CUIBase* CUIWidgetManager::GetWidgetAtCoords(int16 x, int16 y)
{
	if (!m_bInitted) {
		CUI_ERR("WidgetManager was never Initialized!\n");
		return NULL;
	}

	return GetWidgetAtCoordsHelper(m_pRootWidget, x, y);
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIWidgetManager::EnqueueMessage( CUIMessage* pMessage )
{
	if (!m_bInitted) {
		CUI_ERR("WidgetManager was never Initialized!\n");
		return CUIR_ILT_WIDGETMANAGER_ERROR;
	}

	// add a message to the tail of the queue	
	return m_pMessageQueue->EnqueueMessage(pMessage);
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIWidgetManager::DequeueMessage(CUIMessage* pMessage )
{
	if (!m_bInitted) {
		CUI_ERR("WidgetManager was never Initialized!\n");
		return CUIR_ILT_WIDGETMANAGER_ERROR;
	}

	// remove a message from the head of the queue
	return m_pMessageQueue->DequeueMessage(pMessage);
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIWidgetManager::PeekMessage(CUIMessage* pMessage )
{	
	if (!m_bInitted) {
		CUI_ERR("WidgetManager was never Initialized!\n");
		return CUIR_ILT_WIDGETMANAGER_ERROR;
	}

	// look at the message at the head of the queue
	// but don't remove it from the queue!
	return m_pMessageQueue->PeekMessage(pMessage);
}	

//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIWidgetManager::SetUserCallback(
	CUI_CALLBACKTYPE pUserCallback, 
	void* pUserCallbackObject,
	void* pUserCallbackData)
{
	if (!m_bInitted) {
		CUI_ERR("WidgetManager was never Initialized!\n");
		return CUIR_ILT_WIDGETMANAGER_ERROR;
	}

	m_pUserCallback       = pUserCallback;
	m_pUserCallbackObject = pUserCallbackObject;
	m_pUserCallbackData   = pUserCallbackData;

	return CUIR_OK;
}


//  ---------------------------------------------------------------------------
CUI_CALLBACKTYPE CUIWidgetManager::GetUserCallback(void** ppUserCallbackObject,
												   void** ppUserCallbackData)
{
	if (!m_bInitted) {
		CUI_ERR("WidgetManager was never Initialized!\n");
		return NULL;
	}

	if (ppUserCallbackObject)
		*ppUserCallbackObject = m_pUserCallbackObject;
	
	if (ppUserCallbackData)
		*ppUserCallbackData   = m_pUserCallbackData;

	return m_pUserCallback;
}


//  ---------------------------------------------------------------------------
void CUIWidgetManager::Render()
{
	if (!m_bInitted) {
		CUI_ERR("WidgetManager was never Initialized!\n");
		return;
	}

	CUIBase* widget = this->GetRootWidget();
	
	if (widget)	widget->Render();	
}
	
	
//  ---------------------------------------------------------------------------
//
//	This is a very simplistic GUID generation.  But the number of UI elements
//	needing GUIDs will be tiny compared to the size of MAX_UINT32.
 
CUIGUID CUIWidgetManager::GenerateGUID()
{
	void* data;
	CUIGUID  candidate = 0xFFFFFFFF - m_GUIDCount;
	
	while (CUI_RB_OK == m_pGUIDTree->Find(candidate, &data) ) {
		m_GUIDCount++;
		candidate = 0xFFFFFFFF - m_GUIDCount;
		
		if (m_GUIDCount == 0) {
			CUI_ERR("GUID Generator wrapped!\n");
		}
	}
	
	return candidate;
}
		

//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIWidgetManager::FocusNext()
{
	// sets the focus to the next widget in the local list
	CUILinkList	*pList = NULL;
	CUIListNode	*pNode;
	CUIBase		*pWidget, *pParent;
	//CUIGUID		guid;

	if (!m_bInitted) {
		CUI_ERR("WidgetManager was never Initialized!\n");
		return CUIR_ILT_WIDGETMANAGER_ERROR;
	}

	pWidget = this->GetFocusWidget();
	pParent = pWidget->GetParent();

	if (!pParent) {
		return CUIR_NONEXISTENT_WIDGET;
	}

	pList   = pParent->GetImpl()->GetChildList();
	//guid	= pWidget->GetGUID();

	pNode	= pList->Find(pWidget);
	pNode	= pNode->m_pNext;
	if (pNode == NULL) pNode = pList->GetHead();

	pWidget = (CUIBase*) pNode->m_pData;

	this->SetFocusWidget(pWidget);

	return CUIR_OK;
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIWidgetManager::FocusUp()
{
	// sets the focus to a widget's parent
	CUIBase		*pWidget, *pParent;

	if (!m_bInitted) {
		CUI_ERR("WidgetManager was never Initialized!\n");
		return CUIR_ILT_WIDGETMANAGER_ERROR;
	}

	pWidget = this->GetFocusWidget();
	pParent = pWidget->GetParent();

	if (!pParent) {
		return	CUIR_NONEXISTENT_WIDGET;
	}
		
	this->SetFocusWidget(pParent);
	
	return CUIR_OK;
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIWidgetManager::FocusDown()
{
	// sets the focus to a widget's first child 
	CUILinkList	*pList = NULL;
	CUIListNode	*pNode;
	CUIBase		*pWidget;

	if (!m_bInitted) {
		CUI_ERR("WidgetManager was never Initialized!\n");
		return CUIR_ILT_WIDGETMANAGER_ERROR;
	}

	pWidget = this->GetFocusWidget();
	pList   = pWidget->GetImpl()->GetChildList();

	pNode	=	pList->GetHead();

	if (!pNode) {
		return	CUIR_NONEXISTENT_WIDGET;
	}

	pWidget = (CUIBase*) pNode->m_pData;
	this->SetFocusWidget(pWidget);
	
	return CUIR_OK;
}


//  ---------------------------------------------------------------------------
void CUIWidgetManager::Update()
{
	CUIMessage	 message;
	
	CUILinkList	*pList = NULL;
	CUIListNode	*pNode;
	CUIBase		*pWidget, *pRootWidget;
	
	int32		result = CUIR_MESSAGE_NOT_HANDLED;

	if (!m_bInitted) {
		CUI_ERR("WidgetManager was never Initialized!\n");
		return;
	}

	// process messages 
	while (CUIR_OK == m_pMessageQueue->DequeueMessage(&message)) {		

		//CUIMessageQueue::DisplayMessage(NULL, &message);

		// handle widget-manager level messages here, and pass on
		// to topmost widgets.  We look for certain internal messages
		// before honoring the user-defined callback (if any)

				switch (message.m_Message) {
				
			case CUIM_WIDGET_SCHEDULE_FOR_DESTROY:
				m_pDestroyList->Add(message.m_pSender);
				break;

			default:
					
				// has the user requested callbacks from the WidgetManager?
				if (m_pUserCallback) {
					message.m_pUserObject = m_pUserCallbackObject;
					message.m_pUserData   = m_pUserCallbackData;

					result = m_pUserCallback(&message);
				}

				// if the user handled the message, I'm done
				if (result != CUIR_MESSAGE_NOT_HANDLED) continue;

				// messages that are "system" messages (CUIM_WIDGET_ON_DESTROY, etc.)
				// should not be sent to widgets.
				if (message.m_Message < CUIM_SYSTEM_THRESHOLD) continue;

				// else, send the message to my top-level widget, who will propagate it
				pRootWidget = this->GetRootWidget();
				
				if (!pRootWidget) {
					// uh-oh!  no desktop widget
					continue;
				}

				pRootWidget->HandleMessage(&message);
				
		}	// switch (message.m_Message)

	}
	
	// render the ui system
	this->Render();	

	this->DebugShowFocusWidget();
	
	// once all the messages have been processed, it's safe to
	// handle the destroy queue.
	pList = m_pDestroyList;
	pNode = pList->GetHead();
	
	while (pNode) {
		pWidget = (CUIBase*)pList->GetData(pNode);
		//CUI_DBG("Destroy widget: %x\n", pWidget);
		this->InternalDestroyWidget(pWidget);
		pList->Remove(pNode);
		pNode = pList->GetHead();
	}
}


//  ---------------------------------------------------------------------------
void CUIWidgetManager::DebugShowFocusWidget()
{
#ifdef _DEBUG
	
	// draws a red rectangle around the currently focused widget
	CUIBase* widget =  this->GetFocusWidget();

	if (widget) {
	
		LT_LINEF	pLines[4];
		CUIRECT		r;

		widget->GetRect(&r, NULL);

		for (uint32 i=0; i<4; i++) {
			pLines[i].rgba.r = 255;
			pLines[i].rgba.g = 0;
			pLines[i].rgba.b = 0;
			pLines[i].rgba.a = CUI_SYSTEM_OPAQUE >> 24;

			pLines[i].verts[0].z = 
				pLines[i].verts[1].z = SCREEN_NEAR_Z;
		}

		// top
		pLines[0].verts[0].x = r.x - 2;
		pLines[0].verts[0].y = r.y - 2;

		pLines[0].verts[1].x = r.x + r.width + 2;
		pLines[0].verts[1].y = r.y - 2;

		// bottom
		pLines[1].verts[0].x = r.x - 2;
		pLines[1].verts[0].y = r.y + r.height + 2;

		pLines[1].verts[1].x = r.x + r.width + 2;
		pLines[1].verts[1].y = r.y + r.height + 2;

		// left
		pLines[2].verts[0].x = r.x + r.width + 2;
		pLines[2].verts[0].y = r.y - 2;

		pLines[2].verts[1].x = r.x + r.width + 2;
		pLines[2].verts[1].y = r.y + r.height + 2;

		// right
		pLines[3].verts[0].x = r.x - 2;
		pLines[3].verts[0].y = r.y - 2;

		pLines[3].verts[1].x = r.x - 2;
		pLines[3].verts[1].y = r.y + r.height + 2;


		CUIRenderState::SetRenderState(NULL);

		pDrawPrimInternal->DrawPrim(pLines, 4);
	} // widget

#endif // _DEBUG
}

				
