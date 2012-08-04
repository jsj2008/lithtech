//-------------------------------------------------------------------
//
//   MODULE    : CUIBASE_IMPL.CPP
//
//   PURPOSE   : implements the CUIBase_Impl Class (common superclass
//				 to all CUI Widgets.
//
//   CREATED   : 1/01 
//
//   COPYRIGHT : (C) 2001 LithTech Inc
//
//-------------------------------------------------------------------


#ifndef __CUIBASE_H__
#include "cuibase.h"
#endif

#ifndef __CUIBASE_IMPL_H__
#include "cuibase_impl.h"
#endif

#ifndef __CUIMESSAGEQUEUE_H__
#include "cuimessagequeue.h"
#endif

#ifndef __CUIDEBUG_H__
#include "cuidebug.h"
#endif

//#ifndef __CUIBASE_H__
//#include "sysdebugging.h"
//#endif

#ifndef __ILTWIDGETMANAGER_H__
#include "iltwidgetmanager.h"
#endif

// use the one and only widget-manager from the interface database
static ILTWidgetManager*	pWidgetManager;
define_holder(ILTWidgetManager, pWidgetManager);


//  ---------------------------------------------------------------------------
CUIBase_Impl::CUIBase_Impl(CUIBase* pAbstract, CUIGUID guid)
{
	// allocate necessary memory
	LT_MEM_TRACK_ALLOC(m_pChildren = new CUILinkList(),LT_MEM_TYPE_UI);
	
	// fill in members
	m_Rect.x 		= 0;
	m_Rect.y 		= 0;
	m_Rect.width	= 0;
	m_Rect.height	= 0;

	m_GutterLeft	= 2;
	m_GutterRight	= 2;
	m_GutterTop		= 2;
	m_GutterBottom	= 2;
		
	m_pParent		= NULL;

	m_pListIterator = NULL;
				
	// default state flags!
	m_StateFlags	= 0;//CUIS_VISIBLE | CUIS_ENABLED | CUIS_TREE_VISIBLE;
	
	m_pUserCallback		  = NULL;
	m_pUserCallbackObject = NULL;

	// once this is set, it must not be changed
	m_GUID			= guid;	
	m_pAbstract		= pAbstract;
}


//  ---------------------------------------------------------------------------
CUIBase_Impl::~CUIBase_Impl()
{
	// release resources
	if (m_pChildren) delete m_pChildren;
}


//  ---------------------------------------------------------------------------
void CUIBase_Impl::Destroy() 
{
	// add myself to the destroy queue
	pWidgetManager->DestroyWidget(m_pAbstract);
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIBase_Impl::GetPosition(float *x, float *y, CUIBase* pRelativeTo)
{
	float rX, rY;

	// what are (x,y) relative to?
	if (pRelativeTo) {
		pRelativeTo->GetPosition(&rX, &rY, NULL);
		*x = m_Rect.x - rX;
		*y = m_Rect.y - rY;
	}
	else {
		// relative to screen
		*x = m_Rect.x;
		*y = m_Rect.y;
	}

	return CUIR_OK;
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIBase_Impl::SetPosition(float x, 
										 float y, 
										 CUIBase* pRelativeTo,
										 bool bMoveChildren)
{
	CUIBase*	 pWidget;
	CUILinkList* pChildren;
	CUIListNode* pChild;
	
	float cX, cY, rX, rY;
	
	// newx, newy are screen relative
	float newX, newY;

	// this move is relative to what?
	if (pRelativeTo) {
		pRelativeTo->GetPosition(&rX, &rY, NULL);
		newX = rX + x;
		newY = rY + y;
	}
	else {
		newX = x;
		newY = y;
	}
	
	// move the parent
	Move(newX,newY);

	// move the children ?
	if (bMoveChildren) {

		pChildren = GetChildList();
		pChild    = pChildren->GetHead();

		while (pChild) {
			pWidget = (CUIBase*)pChildren->GetData(pChild);
			
			// set child's relative position
			pWidget->GetPosition(&cX, &cY, NULL);
			pWidget->SetPosition( newX - m_Rect.x + cX, newY - m_Rect.y + cY, NULL, bMoveChildren);
			
			pChild = pChildren->GetNext(pChild);
		}		
	}

	// send the on_move message
	if (pWidgetManager) {
		// pWidgetManager can be NULL during the root widget creation
		CUIMessage* pMessage1;
		LT_MEM_TRACK_ALLOC(pMessage1 = new CUIMessage(CUIM_ON_MOVE, m_pAbstract, m_pAbstract, (int16)m_Rect.x | ((int16)m_Rect.y) << 16, 0),LT_MEM_TYPE_MISC);
		
		pWidgetManager->EnqueueMessage(pMessage1);
		delete pMessage1;
	}
	return CUIR_OK;
}

//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIBase_Impl::GetSize(float *w, float *h)
{
	*h = m_Rect.height;
	*w = m_Rect.width;
	
	return CUIR_OK;
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIBase_Impl::GetRect(CUIRECT* pRect, CUIBase* pRelativeTo)
{
	float rX, rY;

	// x,y are relative to where?
	if (pRelativeTo) {
		pRelativeTo->GetPosition(&rX, &rY, NULL);
		pRect->x = m_Rect.x - rX;
		pRect->y = m_Rect.y - rY;
	}
	else {
		pRect->x = m_Rect.x;
		pRect->y = m_Rect.y;
	}

	pRect->width  = m_Rect.width;
	pRect->height = m_Rect.height;
	
	return CUIR_OK;
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIBase_Impl::SetSize(float w, float h)
{	
	// only resize me if the dims are really different
	if (m_Rect.width != w || m_Rect.height != h) {
		Resize(w,h);
	}

	// send the on_move message
	if (pWidgetManager) {
		// pWidgetManager can be NULL during the root widget creation
		CUIMessage* pMessage1;
		LT_MEM_TRACK_ALLOC(pMessage1 = new CUIMessage(CUIM_ON_RESIZE, m_pAbstract, m_pAbstract, (int16)w | ((int16)h) << 16, 0),LT_MEM_TYPE_UI);
		pWidgetManager->EnqueueMessage(pMessage1);
		delete pMessage1;
	}
	return CUIR_OK;
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIBase_Impl::SetRect(CUIRECT* r, CUIBase* pRelativeTo, bool bMoveChildren) 
{
	if (!r) return CUIR_INVALID_POINTER;

	SetSize(r->width, r->height);	
	SetPosition(r->x, r->y, pRelativeTo, bMoveChildren);

	// messages will be sent by the positioning funcs.

	return CUIR_OK;	
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE	CUIBase_Impl::SetGutter(int16 left, int16 right, 
										int16 top, int16 bottom)
{
	if (left <= right) {
		m_GutterLeft   = left;
		m_GutterRight  = right;
	}
	if (top <= bottom) {
		m_GutterTop    = top;
		m_GutterBottom = bottom;
	}

	return CUIR_OK;
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE	CUIBase_Impl::GetGutter(int16 *pLeft, int16 *pRight, 
										int16 *pTop, int16 *pBottom)
{
	if (pLeft)	 *pLeft		= m_GutterLeft;
	if (pRight)  *pRight	= m_GutterRight;
	if (pTop)	 *pTop		= m_GutterTop;
	if (pBottom) *pBottom	= m_GutterBottom;

	return CUIR_OK;
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIBase_Impl::SetParent(CUIBase* p) 
{
	CUILinkList* pList;
	CUIListNode* pNode;
		
	CUIBase* pParent = (CUIBase*)p;
		
	// do I currently have a parent?
	if (m_pParent) {	
		// remove myself from my old parent's list of children
		pList = m_pParent->GetImpl()->GetChildList();
		if (pList) {
			pNode = pList->Find(m_pAbstract);
			if (pNode) pList->Remove(pNode);
		}
	}
	
	// set my new parent 
	m_pParent = pParent;
	
	// add myself to my new parent's list of children
	if (m_pParent) {
		pList = m_pParent->GetImpl()->GetChildList();
		if (pList) {
			pList->Add(m_pAbstract);
		}
	}
	
	return CUIR_OK;	
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIBase_Impl::Show(bool bShowChildren)
{
	CUIBase*	 pWidget;
	CUILinkList* pChildren = this->GetChildList();
	CUIListNode* pChild    = NULL;

	m_StateFlags |= CUIS_VISIBLE;

	if (bShowChildren) {		
    
		if (pChildren) pChild = pChildren->GetHead();
		
		// render my children
		while (pChild) {
			pWidget = (CUIBase*)pChildren->GetData(pChild);
			pWidget->Hide(bShowChildren);
			pChild = pChildren->GetNext(pChild);
		}		
	}

	return CUIR_OK;
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIBase_Impl::Hide(bool bHideChildren)
{
	CUIBase*	 pWidget;
	CUILinkList* pChildren = this->GetChildList();
	CUIListNode* pChild    = NULL;

	m_StateFlags &= ~CUIS_VISIBLE;

	if (bHideChildren) {		
    
		if (pChildren) pChild = pChildren->GetHead();
		
		// render my children
		while (pChild) {
			pWidget = (CUIBase*)pChildren->GetData(pChild);
			pWidget->Hide(bHideChildren);
			pChild = pChildren->GetNext(pChild);
		}		
	}

	return CUIR_OK;
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIBase_Impl::SetState(uint32 flags)
{
	m_StateFlags |= flags;

	return CUIR_OK;
}


//	--------------------------------------------------------------------------
CUI_RESULTTYPE CUIBase_Impl::UnsetState(uint32 flags)
{
	m_StateFlags &= (~flags);

	return CUIR_OK;
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIBase_Impl::BringToFront()
{
	// bring me to the end of my parent's draw list, so I will draw last
	// and be on top of everything
	
	CUILinkList*	pChildren;
	CUIListNode*	pChild;
	
	if (!m_pParent) return CUIR_ERROR;
	
	pChildren = m_pParent->GetImpl()->GetChildList();
	
	pChild = pChildren->Find(m_pAbstract);
	
	if (!pChild) return CUIR_ERROR;

	// remove destroys the CUILinkList Node
	pChildren->Remove(pChild);
	
	// adding to a list always adds to the tail
	pChildren->Add(m_pAbstract);
	
	return CUIR_OK;	
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIBase_Impl::SendToBack()
{
	// bring me to the end of my parent's draw list, so I will draw last
	// and be on top of everything
	
	CUILinkList*	pChildren;
	CUIListNode*	pChild;
	
	if (!m_pParent) return CUIR_ERROR;
	
	pChildren = m_pParent->GetImpl()->GetChildList();
	
	pChild = pChildren->Find(m_pAbstract);
	
	if (!pChild) return CUIR_ERROR;

	// remove destroys the CUILinkList Node
	pChildren->Remove(pChild);
	
	// add before the head
	if (!pChildren->InsertBefore(pChildren->GetHead(), m_pAbstract)) {
		// if this fails, the list is empty, and therefore an add is fine
		pChildren->Add(m_pAbstract);
	}
	
	return CUIR_OK;	
}


//  ---------------------------------------------------------------------------
CUIBase* CUIBase_Impl::GetParent()
{
	return m_pParent;
}


//  ---------------------------------------------------------------------------
CUILinkList* CUIBase_Impl::GetChildList()
{
	return m_pChildren;	
}


//  ---------------------------------------------------------------------------
CUIBase* CUIBase_Impl::GetFirstChild()
{
	if (!m_pChildren) return NULL;
	
	CUIListNode* pNode = m_pChildren->GetHead();

	m_pListIterator = (CUIBase*)m_pChildren->GetData(pNode);
	
	return m_pListIterator;
}


//  ---------------------------------------------------------------------------
CUIBase* CUIBase_Impl::GetNextChild()
{
	if (!m_pChildren)     return NULL;
	if (!m_pListIterator) return NULL;
	
	CUIListNode* pNode = m_pChildren->GetHead();
	CUIBase*	 pChild;

	// I use a CUIBase* rather than just keeping a CUIListNode* so I don't
	// access a bad node if the user is sloppy in using this function.

	while (pNode) {

		pChild = (CUIBase*)m_pChildren->GetData(pNode);

		// get the next node.  I'll either return the data or continue searching
		pNode = m_pChildren->GetNext(pNode);

		if (pChild == m_pListIterator) {

			if (pNode) m_pListIterator = (CUIBase*)m_pChildren->GetData(pNode);
			else	   m_pListIterator = NULL;

			return m_pListIterator;
		}

	}

	m_pListIterator = NULL;

	return NULL;
}


//	---------------------------------------------------------------------------
CUI_RESULTTYPE CUIBase_Impl::SetUserCallback(CUI_CALLBACKTYPE pUserCallback, 
											 void* pUserCallbackObject,
											 void* pUserCallbackData)
{
	m_pUserCallback			= pUserCallback;
	m_pUserCallbackObject	= pUserCallbackObject;
	m_pUserCallbackData		= pUserCallbackData;

	return CUIR_OK;
}


//  ---------------------------------------------------------------------------
CUI_CALLBACKTYPE CUIBase_Impl::GetUserCallback(void** ppUserCallbackObject,
											   void** ppUserCallbackData)
{
	if (ppUserCallbackObject)
		*ppUserCallbackObject = m_pUserCallbackObject;
	
	if (ppUserCallbackData)
		*ppUserCallbackData   = m_pUserCallbackData;

	return m_pUserCallback;
}


//	---------------------------------------------------------------------------
CUI_RESULTTYPE CUIBase_Impl::SendMessageToChildren(CUIMessage* pMessage)		
{
	CUILinkList* pList;
	CUIListNode* pNode;
	CUIBase*	 pChild;

	CUI_RESULTTYPE	 result = CUIR_MESSAGE_NOT_HANDLED;
	
	pList = GetChildList();
		
	// iterate through my list of children, sending the message to each
	if (pList) {
		pNode = pList->GetTail();
		
		while (pNode) {
			pChild = (CUIBase*)pList->GetData(pNode);
			
			pMessage->m_pRecipient = pChild;
			
			result = pChild->HandleMessage(pMessage);
			 
			if (result == CUIR_MESSAGE_NOT_HANDLED)  
			{				
				pNode = pList->GetPrev(pNode);
			}
			else {
				pNode = NULL;
			}
		}		
	}	
	
	return result;	
}


//	---------------------------------------------------------------------------
CUI_RESULTTYPE CUIBase_Impl::HandleMessage(CUIMessage* pMessage)
{
	CUI_RESULTTYPE result = CUIR_MESSAGE_NOT_HANDLED;

	// children get the first crack at it.
	result = SendMessageToChildren(pMessage);

	// if the message was not handled, does the user wish to handle callbacks?
	if (result == CUIR_MESSAGE_NOT_HANDLED && m_pUserCallback) {
		pMessage->m_pRecipient  = m_pAbstract;
		pMessage->m_pUserObject = m_pUserCallbackObject;
		pMessage->m_pUserData   = m_pUserCallbackData;
		result = m_pUserCallback(pMessage);
	}

	return result;
}


//  ---------------------------------------------------------------------------
void CUIBase_Impl::Render()
{
	CUIBase*	 pWidget;
	CUILinkList* pChildren = GetChildList();
	CUIListNode* pChild    = NULL;
    
	if (pChildren) pChild = pChildren->GetHead();
	
	// render myself, if visible
	if ( m_StateFlags & CUIS_TREE_VISIBLE ) {

		if ( m_StateFlags & CUIS_VISIBLE ) {
			Draw();
		}

		// render my children, if tree-visible
		while (pChild) {
			pWidget = (CUIBase*)pChildren->GetData(pChild);
			pWidget->Render();
			pChild = pChildren->GetNext(pChild);
		}	
	}
}

