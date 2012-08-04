// PopupNoteDlg.cpp : implementation file
//

#include "bdefs.h"
#include "..\dedit.h"
#include "popupnotedlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPopupNoteDlg dialog


CPopupNoteDlg::CPopupNoteDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPopupNoteDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPopupNoteDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	// Set the border size
	m_nBorderSize=12;

	// Create the font for displaying the text
	m_font.CreatePointFont( 90, "Courier New" );
}


void CPopupNoteDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPopupNoteDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

/************************************************************************/
// Creates the popup note dialog with the specified string as the text.
BOOL CPopupNoteDlg::CreateDlg(CString sNoteText, int nDialogWidth, CWnd *pParentWindow)
{
	// Create the dialog
	if (!Create(IDD))
	{
		return FALSE;
	}

	// Copy the note text
	m_sNoteText=sNoteText;

	// Copy the dialog width
	m_nDialogWidth=nDialogWidth;

	// Store the intial cursor position so it can be used to
	// move the window in the OnPaint message.
	GetCursorPos(&m_initialCursorPos);
	
	// Show the window
	ShowWindow(SW_SHOW);

	// Set the active window to this dialog
	SetActiveWindow();

	return TRUE;
}

BEGIN_MESSAGE_MAP(CPopupNoteDlg, CDialog)
	//{{AFX_MSG_MAP(CPopupNoteDlg)
	ON_WM_LBUTTONDOWN()
	ON_WM_MBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_KEYDOWN()
	ON_WM_KILLFOCUS()
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPopupNoteDlg message handlers

void CPopupNoteDlg::OnLButtonDown(UINT nFlags, CPoint point) 
{
	DestroyWindow();
	
	CDialog::OnLButtonDown(nFlags, point);
}

void CPopupNoteDlg::OnMButtonDown(UINT nFlags, CPoint point) 
{
	DestroyWindow();
	
	CDialog::OnMButtonDown(nFlags, point);
}

void CPopupNoteDlg::OnRButtonDown(UINT nFlags, CPoint point) 
{
	DestroyWindow();
	
	CDialog::OnRButtonDown(nFlags, point);
}

void CPopupNoteDlg::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	DestroyWindow();
	
	CDialog::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CPopupNoteDlg::OnKillFocus(CWnd* pNewWnd) 
{
	CDialog::OnKillFocus(pNewWnd);
	
	DestroyWindow();	
}

/************************************************************************/
// Updates the window position.  A DC is needed to calculate the size of the text
void CPopupNoteDlg::UpdateWindowPos(CDC *pDC)
{
	if (!pDC)
	{
		ASSERT(FALSE);
		return;
	}

	// Select the new font
	CFont *pOldFont=pDC->SelectObject(&m_font);

	// Calculate how big the window needs to be based on its width
	CRect rcTextRect(0,0, m_nDialogWidth, 0);
	pDC->DrawText(m_sNoteText, &rcTextRect, DT_CALCRECT | DT_LEFT | DT_WORDBREAK);

	// Make the window slightly larger than just the text
	int nBorderSize=m_nBorderSize;
	
	// Move the window up to the left a little bit
	CRect rcWindow(m_initialCursorPos.x, m_initialCursorPos.y, m_initialCursorPos.x+rcTextRect.Width()+(nBorderSize*2), m_initialCursorPos.y+rcTextRect.Height()+(nBorderSize*2));
	if (rcWindow.left > 25)
	{
		rcWindow.OffsetRect(-20, 0);
	}
	if (rcWindow.top > 25)
	{
		rcWindow.OffsetRect(0,-20);
	}

	// Make sure that the window isn't going off of the bottom of the main window
	CRect rcMainWindow;
	AfxGetMainWnd()->GetWindowRect(rcMainWindow);

	// Check to see if it extends past the bottom
	if (rcWindow.bottom > rcMainWindow.bottom)
	{
		// Move the window up
		rcWindow.OffsetRect(0, rcMainWindow.bottom-rcWindow.bottom-5);

		// Make sure we didn't go off of the top of the screen
		if (rcWindow.bottom < 0)
		{
			rcWindow.OffsetRect(0, (-1)*rcWindow.bottom);
		}
	}

	// Check to see if it extends past the right side of the screen
	if (rcWindow.right > rcMainWindow.right)
	{
		// Move the window to the left
		rcWindow.OffsetRect(rcMainWindow.right-rcWindow.right-5, 0);

		// Make sure we didn't go off of the left of the screen
		if (rcWindow.left < 0)
		{
			rcWindow.OffsetRect((-1)*rcWindow.left, 0);
		}
	}

	MoveWindow(rcWindow);

	// Select the old font
	pDC->SelectObject(pOldFont);
}

void CPopupNoteDlg::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// Set the draw mode to transparent
	dc.SetBkMode(TRANSPARENT);

	// Select the new font
	CFont *pOldFont=dc.SelectObject(&m_font);

	// Get the window size
	CRect rcClient;
	GetClientRect(rcClient);

	// Shrink the rectangle for the border
	rcClient.DeflateRect(m_nBorderSize/2, m_nBorderSize/2);

	// Draw the text
	dc.DrawText(m_sNoteText, rcClient, DT_LEFT | DT_WORDBREAK);	

	// Select the old font
	dc.SelectObject(pOldFont);
}

BOOL CPopupNoteDlg::OnEraseBkgnd(CDC* pDC) 
{
	// Update the window position
	UpdateWindowPos(pDC);

	// Get the client rectangle
	CRect rcClient;
	GetClientRect(&rcClient);

	// Draw a rectangle in yellow
	pDC->FillRect(rcClient, &CBrush(RGB(255,255,192)));	

	return TRUE;
}
