//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
 
/////////////////////////////////////////////////////////////////////
// ResizeDlg.h : implementation file for the dialog resizing handling classes
 
 
#include "bdefs.h"
#include "resizedlg.h"

// Helper functions for accessing rectangles by side based on index
static int GetRectSide(const CRect &cRect, DWORD uSide)
{
	switch (uSide)
	{
		case ANCHOR_LEFT :
			return cRect.left;
		case ANCHOR_RIGHT :
			return cRect.right;
		case ANCHOR_TOP : 
			return cRect.top;
		case ANCHOR_BOTTOM :
			return cRect.bottom;
	}
	return 0;
}

static void SetRectSide(CRect &cRect, DWORD uSide, int iValue)
{
	switch (uSide)
	{
		case ANCHOR_LEFT :
			cRect.left = iValue;
			break;
		case ANCHOR_RIGHT :
			cRect.right = iValue;
			break;
		case ANCHOR_TOP : 
			cRect.top = iValue;
			break;
		case ANCHOR_BOTTOM :
			cRect.bottom = iValue;
			break;
	}
}

CResizeAnchorOffset::CResizeAnchorOffset(int iDlgItem, EAnchorStyle eStyle) :
	CResizeAnchor(iDlgItem)
{
	ResetOffsets();
	SetStyle(eStyle);
}

void CResizeAnchorOffset::ResetOffsets(int iValue)
{
	for (DWORD uLoop = 0; uLoop < ANCHOR_NUMSIDES; uLoop++)
		SetOffset(uLoop, iValue);
}

void CResizeAnchorOffset::SetStyle(EAnchorStyle eStyle)
{
	m_eStyle = eStyle;
	switch (eStyle)
	{
		// Don't change when resizing (All to top+left)
		case eAnchorStatic : 
		{
			SetBinding(ANCHOR_LEFT, ANCHOR_LEFT);
			SetBinding(ANCHOR_RIGHT, ANCHOR_LEFT);
			SetBinding(ANCHOR_TOP, ANCHOR_TOP);
			SetBinding(ANCHOR_BOTTOM, ANCHOR_TOP);
			break;
		}
		// Move with resizing (All to bottom right)
		case eAnchorPosition :
		{
			SetBinding(ANCHOR_LEFT, ANCHOR_RIGHT);
			SetBinding(ANCHOR_RIGHT, ANCHOR_RIGHT);
			SetBinding(ANCHOR_TOP, ANCHOR_BOTTOM);
			SetBinding(ANCHOR_BOTTOM, ANCHOR_BOTTOM);
			break;
		}
		// Size with resizing (top+left to top+left, bottom+right to bottom+right)
		case eAnchorSize :
		{
			SetBinding(ANCHOR_LEFT, ANCHOR_LEFT);
			SetBinding(ANCHOR_RIGHT, ANCHOR_RIGHT);
			SetBinding(ANCHOR_TOP, ANCHOR_TOP);
			SetBinding(ANCHOR_BOTTOM, ANCHOR_BOTTOM);
			break;
		}
	}
}

void CResizeAnchorOffset::Lock(const CWnd *pParent, const CRect &cParentRect)
{
	// Get the child window rectangle
	CRect cChildRect;
	CWnd *pDlgWnd = pParent->GetDlgItem(GetDlgItem());
	if (!pDlgWnd)
		return;
	pDlgWnd->GetWindowRect(cChildRect);
	// Translate the rectangle to window coordinates
	pParent->ScreenToClient(cChildRect);

	// Calculate the offsets from the parent window
	for (DWORD uLoop = 0; uLoop < ANCHOR_NUMSIDES; uLoop++)
		SetOffset(uLoop, GetRectSide(cChildRect, uLoop) - GetRectSide(cParentRect, GetBinding(uLoop)));
}

void CResizeAnchorOffset::Resize(const CWnd *pParent, const CRect &cParentRect)
{
	// Resize the control
	CRect cChildRect;
	CWnd *pDlgWnd = pParent->GetDlgItem(GetDlgItem());
	if (!pDlgWnd)
		return;

	// Set up the new rectangle
	for (DWORD uLoop = 0; uLoop < ANCHOR_NUMSIDES; uLoop++)
		SetRectSide(cChildRect, uLoop, GetRectSide(cParentRect, GetBinding(uLoop)) + GetOffset(uLoop));

	// Move the child window
	pDlgWnd->MoveWindow(cChildRect, TRUE);
}


CDlgResizer::~CDlgResizer()
{
	for (DWORD uLoop = 0; uLoop < m_cAnchorList.GetSize(); uLoop++)
		delete m_cAnchorList[uLoop];
}

BOOL CDlgResizer::RemoveAnchor(const CResizeAnchor *pAnchor)
{
	// Remove the anchor
	for (DWORD uLoop = 0; uLoop < m_cAnchorList.GetSize(); uLoop++)
	{
		if (m_cAnchorList[uLoop] == pAnchor)
		{
			delete m_cAnchorList[uLoop];
			m_cAnchorList.Remove(uLoop);
			return TRUE;
		}
	}
	return FALSE;
}

CResizeAnchor *CDlgResizer::FindAnchor(int iDlgItem) const
{
	// Find the anchor
	for (DWORD uLoop = 0; uLoop < m_cAnchorList.GetSize(); uLoop++)
	{
		if (m_cAnchorList[uLoop]->GetDlgItem() == iDlgItem)
			return m_cAnchorList[uLoop];
	}

	// Didn't find anything
	return NULL;
}

void CDlgResizer::Lock(const CWnd *pDialog, const CRect &cRect)
{
	if (!pDialog)
		return;

	// Lock each of the controls
	for (DWORD uLoop = 0; uLoop < m_cAnchorList.GetSize(); uLoop++)
		m_cAnchorList[uLoop]->Lock(pDialog, cRect);
}

void CDlgResizer::Resize(const CWnd *pDialog, const CRect &cRect)
{
	if (!pDialog)
		return;

	// Resize each of the controls
	for (DWORD uLoop = 0; uLoop < m_cAnchorList.GetSize(); uLoop++)
		m_cAnchorList[uLoop]->Resize(pDialog, cRect);
}



