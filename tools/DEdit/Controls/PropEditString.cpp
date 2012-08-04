// PropEditString.cpp : implementation file
//

#include "bdefs.h"
#include "..\dedit.h"
#include "propeditstring.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPropEditString

// Constructor
CPropEditString::CPropEditString()
{
	// The button width
	m_nButtonWidth=12;
}

// Destructor
CPropEditString::~CPropEditString()
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
CButton *CPropEditString::AddButton(CString sButtonText, CWnd *pParent, int nID, CFont *pFont)
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
void CPropEditString::Position(CRect rcControl)
{
	// Shrink the control down to fit the buttons
	CRect rc=rcControl;
	rc.DeflateRect(0,0,m_nButtonWidth*m_buttonArray.GetSize(), 0);
			
	// Move the control
	MoveWindow( &rc );

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
}

BEGIN_MESSAGE_MAP(CPropEditString, CEdit)
	//{{AFX_MSG_MAP(CPropEditString)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPropEditString message handlers
