//-------------------------------------------------------------------
//
//   MODULE    : CUIBASE.CPP
//
//   PURPOSE   : implements the CUIBase bridge Class
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


//  ---------------------------------------------------------------------------
CUIBase::~CUIBase()
{
	// release resources
	if (m_pImpl) delete m_pImpl;
}


//  ---------------------------------------------------------------------------
//	The following functions are all bridge functions.  The widget does nothing
//	on its own.  This is to keep all implementation out of the SDK directory.
//  ---------------------------------------------------------------------------


//  ---------------------------------------------------------------------------
void CUIBase::Destroy() 
{
	m_pImpl->Destroy();
}


//  ---------------------------------------------------------------------------
CUI_WIDGETTYPE CUIBase::GetType()
{
	return m_pImpl->GetType();
}


//  ---------------------------------------------------------------------------
const char*	CUIBase::GetClassName()
{
	return m_pImpl->GetClassName();
}


//  ---------------------------------------------------------------------------
HTEXTURE CUIBase::GetTexture(CUI_ELEMENTTYPE elm)
{
	return m_pImpl->GetTexture(elm);
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE	CUIBase::SetGutter(int16 left, int16 right, 
								   int16 top, int16 bottom)
{
	return m_pImpl->SetGutter(left, right, top, bottom);
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE	CUIBase::GetGutter(int16 *pLeft, int16 *pRight, 
								   int16 *pTop, int16 *pBottom)
{
	return m_pImpl->GetGutter(pLeft, pRight, pTop, pBottom);
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIBase::GetPosition(float *x, 
									float *y, 
									CUIBase* pRelativeTo)
{
	return m_pImpl->GetPosition(x, y, pRelativeTo);
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIBase::SetPosition(float x, 
									float y, 
									CUIBase* pRelativeTo,
									bool bMoveChildren)
{
	return m_pImpl->SetPosition(x, y, pRelativeTo, bMoveChildren);
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIBase::GetSize(float *w, float *h)
{
	return m_pImpl->GetSize(w, h);
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIBase::GetRect(CUIRECT* pRect, CUIBase* pRelativeTo)
{
	return m_pImpl->GetRect(pRect, pRelativeTo);
}


//  ---------------------------------------------------------------------------
CUIGUID	CUIBase::GetGUID()
{
	return m_pImpl->GetGUID();
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIBase::SetSize(float w, float h)
{
	return m_pImpl->SetSize(w, h);
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIBase::SetRect(CUIRECT* r, 
								CUIBase* pRelativeTo,
								bool bMoveChildren)
{
	return m_pImpl->SetRect(r, pRelativeTo, bMoveChildren);
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIBase::SetParent(CUIBase* parent) 
{
	return m_pImpl->SetParent(parent);
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIBase::Show(bool bShowChildren)   
{
	return m_pImpl->Show(bShowChildren);
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIBase::Hide(bool bHideChildren)   
{
	return m_pImpl->Hide(bHideChildren);
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIBase::SetState(uint32 flags)
{
	return m_pImpl->SetState(flags);
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIBase::UnsetState(uint32 flags)
{
	return m_pImpl->UnsetState(flags);
}


//  ---------------------------------------------------------------------------
uint32 CUIBase::GetState()
{
	return m_pImpl->GetState();
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIBase::BringToFront()
{
	return m_pImpl->BringToFront();
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIBase::SendToBack()
{
	return m_pImpl->SendToBack();
}


//  ---------------------------------------------------------------------------
CUIBase* CUIBase::GetParent()
{
	return m_pImpl->GetParent();
}


//  ---------------------------------------------------------------------------
CUIBase* CUIBase::GetFirstChild()
{
	return m_pImpl->GetFirstChild();
}


//  ---------------------------------------------------------------------------
CUIBase* CUIBase::GetNextChild()
{
	return m_pImpl->GetNextChild();
}


//	---------------------------------------------------------------------------
CUI_RESULTTYPE CUIBase::SetUserCallback(CUI_CALLBACKTYPE pUserCallback, 
										void* pUserCallbackObject,
										void* pUserCallbackData)
{
	return m_pImpl->SetUserCallback(pUserCallback, 
									pUserCallbackObject, 
									pUserCallbackData);
}


//	---------------------------------------------------------------------------
CUI_CALLBACKTYPE CUIBase::GetUserCallback(void** ppUserCallbackObject,
										  void** ppUserCallbackData)
{
	return m_pImpl->GetUserCallback(ppUserCallbackObject, 
									ppUserCallbackData);
}


//	---------------------------------------------------------------------------
void* CUIBase::GetUserCallbackObject()
{
	return m_pImpl->GetUserCallbackObject();
}


//	---------------------------------------------------------------------------
void* CUIBase::GetUserCallbackData()
{
	return m_pImpl->GetUserCallbackData();
}


//	---------------------------------------------------------------------------
CUI_RESULTTYPE CUIBase::SendMessageToChildren(CUIMessage*  pMessage)		
{
	return m_pImpl->SendMessageToChildren(pMessage);	
}


//	---------------------------------------------------------------------------
CUI_RESULTTYPE CUIBase::HandleMessage(CUIMessage*  pMessage)		
{
	return m_pImpl->HandleMessage(pMessage);	
}


//  ---------------------------------------------------------------------------
void CUIBase::Render()
{
	m_pImpl->Render();
}

