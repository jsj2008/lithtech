// PropEditStringList.cpp : implementation file
//

#include "bdefs.h"
#include "..\dedit.h"
#include "propeditstringlist.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPropEditStringList

CPropEditStringList::CPropEditStringList()
{
	// The button width
	m_nButtonWidth=12;
}

CPropEditStringList::~CPropEditStringList()
{
	// Delete the buttons
	int i;
	for (i=0; i < m_buttonArray.GetSize(); i++)
	{
		if (m_buttonArray[i])
		{
			delete m_buttonArray[i];
		}
	}

	// Clear the button array
	m_buttonArray.RemoveAll();
}

/************************************************************************/
// Add a button.  This returns the created button.  Note that it is
// deleted by the control in the constructor.
CButton *CPropEditStringList::AddButton(CString sButtonText, CWnd *pParent, int nID, CFont *pFont)
{
	// Create the button
	CButton *pButton = new CButton;
	if (pButton->Create( sButtonText, WS_CHILD | WS_VISIBLE | WS_TABSTOP, CRect(0,0,10,10), pParent, nID ))
	{
		// Set the font if necessary
		if (pFont)
		{
			pButton->SetFont(pFont);
		}

		m_buttonArray.Add(pButton);
		return pButton;
	}
	else
	{
		delete pButton;
		return NULL;
	}	
}

/************************************************************************/
// Position the control and its buttons
void CPropEditStringList::Position(CRect rcControl)
{
	// Shrink the control down to fit the buttons
	CRect rc=rcControl;
	rc.DeflateRect(0,0,m_nButtonWidth*m_buttonArray.GetSize(), 0);
	rc.bottom += 200;
	
	// I record and later restore the string because MoveWindow, for whatever reason,
	// causes the combobox to execute a FindString, and if the edit box partially 
	// matches anything in the listbox, it will alter the edit box contents to that
	// matched string
	CString csHack;
	GetWindowText(csHack);

	// Move the control
	MoveWindow( &rc );
	SetHorizontalExtent(1024);
	SetDroppedWidth(rcControl.right - rcControl.left);

	// Place the buttons
	int i;
	for (i=0; i < m_buttonArray.GetSize(); i++)
	{
		// Start the left edge of the button on the right edge of the control
		int nLeft=rcControl.right;

		// Move the left edge of the button to the left based on our current index
		nLeft-=(m_buttonArray.GetSize()-i)*m_nButtonWidth;

		CRect rcButton(nLeft, rcControl.top, nLeft+m_nButtonWidth, rcControl.bottom);
		m_buttonArray[i]->MoveWindow(&rcButton);
	}

	SetWindowText(csHack);
}

BEGIN_MESSAGE_MAP(CPropEditStringList, CComboBox)
	//{{AFX_MSG_MAP(CPropEditStringList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPropEditStringList message handlers
