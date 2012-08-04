//-------------------------------------------------------------------
//
//   MODULE    : CUILIST_IMPL.CPP
//
//   PURPOSE   : implements the CUIList_Impl widget class
//
//   CREATED   : 4/01 
//
//   COPYRIGHT : (C) 2001 LithTech Inc
//
//-------------------------------------------------------------------


#ifndef __CUILIST_IMPL_H__
#include "cuilist_impl.h"
#endif

#ifndef __ILTFONTMANAGER_H__
#include "iltfontmanager.h"
#endif


// use the ILTFontManager for polystring allocation
static ILTFontManager *pLTFontManager;
define_holder(ILTFontManager, pLTFontManager);


//  ---------------------------------------------------------------------------
CUIList_Impl::CUIList_Impl(CUIBase* abstract, CUIGUID guid) :
	CUIWidget_Impl(abstract, guid)
{
	m_pFont = NULL;
	LT_MEM_TRACK_ALLOC(m_pList = new CUILinkList(),LT_MEM_TYPE_UI);

	m_pTextColors[0] = 
		m_pTextColors[1] = 
			m_pTextColors[2] = 
				m_pTextColors[3] = CUI_DEFAULT_FONT_COLOR | CUI_SYSTEM_OPAQUE;

	m_pSelectedColors[0] = 
		m_pSelectedColors[1] = 
			m_pSelectedColors[2] = 
				m_pSelectedColors[3] = (~CUI_DEFAULT_FONT_COLOR) | CUI_SYSTEM_OPAQUE;

	m_CharHeight	= 0;

	m_ItemCount = 0;
	m_Selection = -1;
	m_WindowStart = 0;
}


//  ---------------------------------------------------------------------------
CUIList_Impl::~CUIList_Impl()
{
	// free any allocated resources
	if (m_pList) delete m_pList;
}

	
//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIList_Impl::SetFont(CUIFont* pFont)
{
	CUIListNode*	pNode;
	CUIPolyString*	pStr;

	// set the font
	m_pFont = pFont;

	if (pFont) {
		if (!m_CharHeight) m_CharHeight = pFont->GetDefCharScreenHeight();
	}
	else {
		return CUIR_NO_FONT;
	}

	// adjust all the polystrings
	if (m_pList) {

		pNode = m_pList->GetHead();

		while (pNode) {			
			pStr = (CUIPolyString*) pNode->m_pData;
			if (pStr) {
				pStr->SetFont(pFont);
			}
			pNode = pNode->m_pNext;
		}
	}
	
	return CUIR_OK;
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIList_Impl::SetCharHeight(uint8 height)
{
	CUIListNode*	pNode;
	CUIPolyString*	pStr;

	// set the height memeber
	m_CharHeight = height;

	// adjust all the polystrings
	if (m_pList) {

		pNode = m_pList->GetHead();

		while (pNode) {			
			pStr = (CUIPolyString*) pNode->m_pData;
			if (pStr) {
				pStr->SetCharScreenHeight(m_CharHeight);
			}
			pNode = pNode->m_pNext;
		}
	}

	return CUIR_OK;
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIList_Impl::AddItem(const char* text, int32 index)
{
	CUIPolyString*  pStr;
	CUIListNode*	pNode;
	int32			count = 0;

	// we need a font
	if (!m_pFont) return CUIR_NO_FONT;

	// allocate a new string
	LT_MEM_TRACK_ALLOC(pStr = new CUIPolyString(m_pFont, text),LT_MEM_TYPE_UI);
	if (!pStr) return CUIR_OUT_OF_MEMORY;

	// set it's dims
	pStr->SetCharScreenHeight(m_CharHeight);
	pStr->SetColors(m_pTextColors[0], m_pTextColors[1], m_pTextColors[2], m_pTextColors[3]);

	if (m_pList) {
		if (index < 0 || index >= m_ItemCount) {	
			// add it to the end of the list
			m_pList->Add(pStr);
		}
		else {
			// user wants the item at a specific location
			pNode = m_pList->GetHead();

			// insert before the head?
			if (index == 0) {
				m_pList->InsertBefore(pNode, pStr);
			}
			else {

				while (pNode) {			
					count++;
					if (count == index) {
						m_pList->InsertAfter(pNode, pStr);
						break;
					}
					pNode = pNode->m_pNext;
				}
			}
		}
	}
	else {
		delete pStr;
		return CUIR_OUT_OF_MEMORY;
	}

	// keep the same item selected
	if (m_Selection >=0 && index >=0 && index <= m_Selection) {
		this->SetSelection(m_Selection+1);
	}

	// increment the total item count
	m_ItemCount++;

	// align text
	this->AlignTextInWidget();

	return CUIR_OK;
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIList_Impl::RemoveItem(int32 index)
{

	CUIListNode*	pNode;
	int32			count = -1;

	if (m_pList) {

		pNode = m_pList->GetHead();

		while (pNode) {			
			count++;
			if (count == index) {
				// remove this node
				m_pList->Remove(pNode);
				break;
			}
			pNode = pNode->m_pNext;
		}
	}
	else {
		return CUIR_INVALID_POINTER;
	}

	if (count == index) {
		// did i remove something before the index, or the index?
		if (m_Selection >=0 && index >=0) {
			if (index < m_Selection) {
				this->SetSelection(m_Selection-1);
			}
			if (index == m_Selection) {
				this->SetSelection(m_Selection);
			}
		}	

		// increment the total item count
		m_ItemCount--;

		// align text
		this->AlignTextInWidget();

		return CUIR_OK;
	}

	return CUIR_ERROR;
}


//  ---------------------------------------------------------------------------
int32 CUIList_Impl::FindItem(const char* text)
{
	CUIListNode*	pNode;
	CUIPolyString*	pStr;
	int32			index = -1;

	if (m_pList) {

		pNode = m_pList->GetHead();

		while (pNode) {			
			index++;
			pStr = (CUIPolyString*) pNode->m_pData;
			if (strcmp(pStr->GetText(), text) == 0) {
				break;
			}
			pNode = pNode->m_pNext;
		}
	}

	return index;
}


//  ---------------------------------------------------------------------------
const char* CUIList_Impl::GetItem(int32 index)
{
	CUIListNode*	pNode;
	CUIPolyString*	pStr;
	int32			count = -1;

	if (m_pList) {

		pNode = m_pList->GetHead();

		while (pNode) {			
			count++;
			if (count == index) {
				pStr = (CUIPolyString*) pNode->m_pData;
				return pStr->GetText();
			}
			pNode = pNode->m_pNext;
		}
	}

	return NULL;
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIList_Impl::SetSelection(int32 selindex)
{
	CUIPolyString *pOld = NULL;
	CUIPolyString *pNew = NULL;
	CUIListNode   *pNode;
	int32		   curindex = 0;

	if (selindex < -1 || selindex > m_ItemCount-1) {
		return CUIR_NONEXISTENT_INDEX;
	}

	if (m_pList) {
		pNode = m_pList->GetHead();

		while (pNode) {
			if (m_Selection == curindex) {
				pOld = (CUIPolyString*) pNode->m_pData;
			}
			if (selindex == curindex) {
				pNew = (CUIPolyString*) pNode->m_pData;
			}
			if (pNew && pOld) break;

			pNode = pNode->m_pNext;
			curindex++;
		}
	}

	// un-color the old selection
	if (pOld) {
		pOld->SetColors(m_pTextColors[0], 
						m_pTextColors[1], 
						m_pTextColors[2], 
						m_pTextColors[3]);
	}

	// color the new selection
	if (pNew) {
		pNew->SetColors(m_pSelectedColors[0], 
						m_pSelectedColors[1], 
						m_pSelectedColors[2], 
						m_pSelectedColors[3]);
	}

	m_Selection = selindex;

	return CUIR_OK;
}



//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIList_Impl::SetColors(CUI_ELEMENTTYPE elm, 
									   uint32 argb0,
									   uint32 argb1,
									   uint32 argb2,
									   uint32 argb3)
{
	CUIListNode* pNode;
	CUIPolyString* pStr;
	int32 index = 0;

	switch (elm) {

		case CUIE_TEXT:
			m_pTextColors[0] = argb0;
			m_pTextColors[1] = argb1;
			m_pTextColors[2] = argb2;
			m_pTextColors[3] = argb3;

			if (m_pList) {
				pNode = m_pList->GetHead();
				
				while (pNode) {
					if (index != m_Selection) {
						pStr = (CUIPolyString*) pNode->m_pData;
						if (pStr) {
							pStr->SetColors(argb0, argb1, argb2, argb3);
						}
					}
					pNode = pNode->m_pNext;
					index++;
				}
			}
			break;

		case CUIE_SELECTEDTEXT:
			m_pSelectedColors[0] = argb0;
			m_pSelectedColors[1] = argb1;
			m_pSelectedColors[2] = argb2;
			m_pSelectedColors[3] = argb3;

			if (m_pList) {
				pNode = m_pList->GetHead();
				
				while (pNode) {
					if (index == m_Selection) {
						pStr = (CUIPolyString*) pNode->m_pData;
						if (pStr) {
							pStr->SetColors(argb0, argb1, argb2, argb3);
						}
						break;
					}
					pNode = pNode->m_pNext;
					index++;
				}
			}
		
			break;
			

		default:
			return CUIWidget_Impl::SetColors(elm, argb0, argb1, argb2, argb3);			
	}
	
	return CUIR_OK;
}


//  ---------------------------------------------------------------------------
int32 CUIList_Impl::QueryPoint(float x, float y)
{
	CUIListNode*	pNode;
	CUIPolyString*	pStr;
	CUIRECT			r;
	int32			curindex = 0;

	// get the list of strings
	if (m_pList) {
		pNode = m_pList->GetHead();
		
		// and test them one by one
		while (pNode) {
			pStr = (CUIPolyString*) pNode->m_pData;

			if (pStr) {
				// ignore items that are not shown
				if (curindex >= m_WindowStart) {
					pStr->GetRect(&r);

					if (x > r.x && x < r.x + r.width) 
						if (y > r.y && y < r.y + r.height)
							return curindex;
				}
			}

			pNode = pNode->m_pNext;
			curindex++;
		}
	}

	return -1;
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIList_Impl::Scroll(int32 number)
{
	m_WindowStart += number;

	m_WindowStart = LTMAX(0, LTMIN(m_ItemCount-1, m_WindowStart));

	this->AlignTextInWidget();

	return CUIR_OK;
}


//  ---------------------------------------------------------------------------
void CUIList_Impl::Draw()
{
	CUIListNode*	pNode;
	CUIPolyString*	pStr;
	int32			curindex = 0;

	CUIRECT			clip (m_Rect.x + m_GutterLeft, m_Rect.y + m_GutterTop,
						  m_Rect.width - m_GutterLeft - m_GutterRight,
						  m_Rect.height - m_GutterTop - m_GutterBottom);

	// draw the bg image
	CUIWidget_Impl::Draw();

	// get the list of strings
	if (m_pList) {
		pNode = m_pList->GetHead();
		
		// and draw them one by one
		while (pNode) {
			if (curindex >= m_WindowStart) {
				pStr = (CUIPolyString*) pNode->m_pData;
				if (pStr) pStr->RenderClipped(&clip);
			}
			pNode = pNode->m_pNext;
			curindex++;
		}
	}
}


//  ---------------------------------------------------------------------------
void CUIList_Impl::Move(float x, float y)
{
	CUIWidget_Impl::Move(x,y);

	this->AlignTextInWidget();
}


//  ---------------------------------------------------------------------------
void CUIList_Impl::Resize(float w, float h)
{
	CUIWidget_Impl::Resize(w,h);
}


//  ---------------------------------------------------------------------------
void CUIList_Impl::AlignTextInWidget()
{
	CUIListNode*	pNode;
	CUIPolyString*	pStr;
	float			x,y;
	int32			curindex = 0;

	x = m_Rect.x + m_GutterLeft;
	y = m_Rect.y + m_GutterTop;

	// get the list of strings
	if (m_pList) {
		pNode = m_pList->GetHead();
		
		// and setpos() them one by one
		while (pNode) {
			if (curindex >= m_WindowStart) {
				pStr = (CUIPolyString*) pNode->m_pData;
				if (pStr) pStr->SetPosition(x,y);
				y += m_CharHeight + m_pFont->GetDefSpacingV();
			}
			pNode = pNode->m_pNext;
			curindex++;
		}
	}	
}
