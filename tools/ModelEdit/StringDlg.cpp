// StringDlg.cpp : implementation file
//

#include "precompile.h"
#include "stringdlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CStringDlg dialog


CStringDlg::CStringDlg(UINT id, CWnd* pParent /*=NULL*/)
	: CDialog(id, pParent)
{
	//{{AFX_DATA_INIT(CStringDlg)
	m_EnteredText = _T("");
	m_MsgText = _T("");
	//}}AFX_DATA_INIT

	m_bAllowLetters = m_bAllowNumbers = m_bAllowOthers = FALSE;
	m_bBeeping = FALSE;
	m_MaxStringLen = 255;
}


void CStringDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CStringDlg)
	DDX_Text(pDX, IDC_ENTEREDTEXT, m_EnteredText);
	DDX_Text(pDX, IDC_MSGTEXT, m_MsgText);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CStringDlg, CDialog)
	//{{AFX_MSG_MAP(CStringDlg)
	ON_EN_CHANGE(IDC_ENTEREDTEXT, OnChangeEnteredtext)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()



int CStringDlg::DoModal( UINT idCaption, UINT idText )
{
	m_idCaption = idCaption;
	m_idText = idText;

	return CDialog::DoModal();
}



/////////////////////////////////////////////////////////////////////////////
// CStringDlg message handlers


BOOL CStringDlg::OnInitDialog() 
{
	CString		captionString;


	CDialog::OnInitDialog();

//	GetDlgItem(IDOK)->EnableWindow(FALSE);
	
	captionString.LoadString( m_idCaption );
	m_MsgText.LoadString( m_idText );
	
	SetWindowText( captionString );
	UpdateData( FALSE );

	GetDlgItem(IDC_ENTEREDTEXT)->SetFocus();
	
	return TRUE;  
}




void CStringDlg::OnChangeEnteredtext() 
{
	CString		sText;
	CString		sNew;
	char		letter[2] = {0,0};
	int			i;

	CEdit		*pEdit = (CEdit*)GetDlgItem( IDC_ENTEREDTEXT );
	int			selStart, selEnd;


	// Validate the string.
	pEdit->GetWindowText( sText );

	for( i=0; i < sText.GetLength(); i++ )
	{
		if( m_bAllowLetters )
		{
			if( isalpha(sText[i]) )
			{
				letter[0] = sText[i];
				sNew += letter;
			}
		}
	
		if( m_bAllowNumbers )
		{
			if( isdigit(sText[i]) )
			{
				letter[0] = sText[i];
				sNew += letter;
			}
		}

		if( m_bAllowOthers )
		{
			if( !isdigit(sText[i]) && !isalpha(sText[i]) )
			{
				letter[0] = sText[i];
				sNew += letter;
			}
		}
	}

	if( (sNew.GetLength() != sText.GetLength()) || (sNew.GetLength() > m_MaxStringLen) )
	{
		if( sNew.GetLength() > m_MaxStringLen )
			sNew = sNew.Left(m_MaxStringLen);

		pEdit->GetSel( selStart, selEnd );
		pEdit->SetWindowText( sNew );
		pEdit->SetSel( selStart-1, selEnd-1 );
	
		if( m_bBeeping )
			MessageBeep(0);
	}

	GetDlgItem(IDOK)->EnableWindow(strlen(sNew) > 0 ? TRUE : FALSE);	
}









