//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
 
/////////////////////////////////////////////////////////////////////
 
// ResizableDlgBar.cpp : implementation file
//
 
#include "bdefs.h"
#include "oldtypes.h"
#include "resizebar.h"
#include "mainfrm.h"

 
////////////////////////////////////////////////////////////////////
// CResizeBar Construction/Destruction

CResizeBar::CResizeBar()
{
#if 0 // REMOVED1
	m_fInit=FALSE;
	m_pFrame = NULL;
#endif // REMOVED1
	
	// Vertical stretch when docked.
	m_DockStretchMode = 0;
	m_bFitToFrame = TRUE;
}
 
BOOL CResizeBar::Create( CWnd* pParentWnd, UINT nIDTemplate,
                               UINT nStyle, UINT nID, BOOL bChange)
{
	if(!CDialogBar::Create(pParentWnd,nIDTemplate,nStyle,nID))
		return FALSE;
 
	m_bChangeDockedSize = bChange;
	m_sizeFloating = m_sizeDocked = m_sizeDefault;
	return TRUE;
}
 
BOOL CResizeBar::Create( CWnd* pParentWnd,
                               LPCTSTR lpszTemplateName, UINT nStyle,
                               UINT nID, BOOL bChange)
{
	if (!CDialogBar::Create( pParentWnd, lpszTemplateName,
                                              nStyle, nID))
	return FALSE;
 
	m_bChangeDockedSize = bChange;
	m_sizeFloating = m_sizeDocked = m_sizeDefault;
    return TRUE;
}


////////////////////////////////////////////////////////////////////
// Overloaded functions
 
CSize CResizeBar::CalcDynamicLayout(int nLength, DWORD nMode)
{
// The following section was removed because it didn't work well with the other
// toolbars.
#if 0 // REMOVED1
	SizeFixup();
	CMainFrame* pFrame = (CMainFrame*)m_pFrame;
	
	if (pFrame)
	{
		if( m_bFitToFrame )
		{
			CRect		rect;

			::GetClientRect( pFrame->m_hWndMDIClient, &rect );
			
			if ( !IsFloating() )
			{
				if( m_DockStretchMode == 0 )
				{
					m_sizeDocked.cy = GetFidgetHeight( pFrame );
					m_sizeDocked.cy+=4;

					m_sizeDocked.cy = rect.Height()+8;
				}
				else
				{
					m_sizeDocked.cx = GetFidgetWidth( pFrame ) + 2;
					m_sizeDocked.cy = m_sizeFloating.cy;

					//m_sizeDocked.cx = rect.Width();
					//m_sizeDocked.cy = m_sizeFloating.cy;
				}
			}
			
			if( (m_DockStretchMode == 0) && (1 == GetToolBarDocking(pFrame)) )
			{
				m_sizeDocked.cy+=2;
			}
		}
	}
#endif // REMOVED1

    // Return default if it is being docked or floated
    if ((nMode & LM_VERTDOCK) || (nMode & LM_HORZDOCK))
    {
		if (nMode & LM_STRETCH)
		{
			// if not docked stretch to fit
			return CSize((nMode & LM_HORZ) ? 32767 : m_sizeDocked.cx,
                         (nMode & LM_HORZ) ? m_sizeDocked.cy : 32767);
		}
		else
		{
			return m_sizeDocked;
		}
    }
    if (nMode & LM_MRUWIDTH)
        return m_sizeFloating;
    // In all other cases, accept the dynamic length
    if (nMode & LM_LENGTHY)
        return CSize(m_sizeFloating.cx,
			(m_bChangeDockedSize) ?
			m_sizeFloating.cy = m_sizeDocked.cy = nLength : m_sizeFloating.cy = nLength);
     else
        return CSize((m_bChangeDockedSize) ?
			m_sizeFloating.cx = m_sizeDocked.cx = nLength : m_sizeFloating.cx = nLength,
			m_sizeFloating.cy);
}

BEGIN_MESSAGE_MAP(CResizeBar, CDialogBar)
    //{{AFX_MSG_MAP(CResizeBar)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CResizeBar::OnSize(UINT nType, int cx, int cy) 
{
	CDialogBar::OnSize(nType, cx, cy);

#if 0 // REMOVED1
	Invalidate(FALSE);

	if( m_pFrame )
		m_pFrame->RecalcLayout();
#endif // REMOVED1
}


#if 0 // REMOVED1
void CResizeBar::SizeFixup()
{
	CWnd* pFrame=GetParent();
	if (IsWindow(m_hWnd) || pFrame!=NULL)
	{
		CRect rc;
		pFrame->GetClientRect(&rc);
		SendMessage(WM_SIZE, SIZE_RESTORED, MAKELPARAM(rc.Width(), rc.Height()) );
	}
}


int CResizeBar::GetFidgetWidth( CMainFrame *pFrame )
{
	CRect		rect;

	pFrame->GetClientRect( &rect );
	return rect.Width();
}


int CResizeBar::GetFidgetHeight( CMainFrame *pFrame )
{
	if( !::IsWindow(pFrame->m_hWnd) )
	{
		return 0;
	}

	CRect rc;
	pFrame->GetClientRect(&rc);

	// subtract the menu height
	rc.bottom -= GetSystemMetrics(SM_CYMENUSIZE);

	CRect	rcTool, rcStatus;
	
	pFrame->GetToolBar()->GetWindowRect( &rcTool );
	pFrame->GetStatusBar()->GetWindowRect( &rcStatus );

	// if the toolbar is not visible
	if (1 == GetToolBarDocking(pFrame))
	{
		return (rc.bottom - rcTool.Height()) - rcStatus.Height();
	}
	else
	{
		return rc.bottom;
	}
}


// 0 = Not docked
// 1 = TOP or BOTTOM
// 2 = LEFT or RIGHT
int CResizeBar::GetToolBarDocking( CMainFrame *pFrame )
{
	CRect rcTool;
	pFrame->GetToolBar()->GetWindowRect(&rcTool);
	
	if ( pFrame->GetToolBar()->IsFloating() || !pFrame->GetToolBar()->IsVisible() )
	{
		return 0;
	}

	if ( rcTool.Width() < rcTool.Height() )
	{
		// LEFT or RIGHT
		return 2;
	}
	
	return 1;
}

#endif // REMOVED1
