//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
// VectorEdit.cpp : implementation file
//

#include "bdefs.h"
#include "dedit.h"
#include "vectoredit.h"
#include "optionsmisc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CVectorEdit dialog


CVectorEdit::CVectorEdit(CWnd* pParent /*=NULL*/)
	: CDialog(CVectorEdit::IDD, pParent)
{
	//{{AFX_DATA_INIT(CVectorEdit)
	//}}AFX_DATA_INIT

	m_UserCallback = NULL;
	m_pUserCallbackData = NULL;

	m_vVector.Init();
	m_fIncrement = 0.1f;

	//load the icons for the buttons
	m_hCopyIcon  = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_COPY));
	m_hPasteIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_PASTE));
}

void CVectorEdit::SetCallback(TVectorEditUpdateCallback pCallback, void* pUserData)
{
	m_UserCallback = pCallback;
	m_pUserCallbackData = pUserData;
}

void CVectorEdit::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CVectorEdit)	
	DDX_Text(pDX, IDC_VECTOR_EDITX, m_sVecX);
	DDX_Text(pDX, IDC_VECTOR_EDITY, m_sVecY);
	DDX_Text(pDX, IDC_VECTOR_EDITZ, m_sVecZ);
	DDX_Text(pDX, IDC_VECTOR_EDIT_INC, m_sIncrement);
	DDX_Check(pDX, IDC_CHECK_AUTOAPPLY, m_bAutoApply);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CVectorEdit, CDialog)
	//{{AFX_MSG_MAP(CVectorEdit)
	ON_EN_CHANGE(IDC_VECTOR_EDITX, OnChangeVectorEdit)
	ON_EN_CHANGE(IDC_VECTOR_EDITY, OnChangeVectorEdit)
	ON_EN_CHANGE(IDC_VECTOR_EDITZ, OnChangeVectorEdit)
	ON_EN_CHANGE(IDC_VECTOR_EDIT_INC, OnChangeIncrement)
	ON_BN_CLICKED(IDC_BUTTON_COPY, OnCopy)
	ON_BN_CLICKED(IDC_BUTTON_PASTE, OnPaste)
	ON_BN_CLICKED(IDC_CHECK_AUTOAPPLY, OnAutoApply)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_VECTORX, OnSpinX)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_VECTORY, OnSpinY)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_VECTORZ, OnSpinZ)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CVectorEdit message handlers

/************************************************************************/
// The dialog is initializing
BOOL CVectorEdit::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// Create the vector string
	m_sVecX.Format("%.6f", m_vVector.x);
	TrimZeros(m_sVecX);
	
	m_sVecY.Format("%.6f", m_vVector.y);
	TrimZeros(m_sVecY);

	m_sVecZ.Format("%.6f", m_vVector.z);
	TrimZeros(m_sVecZ);

	//setup the icons on the buttons
	((CButton*)GetDlgItem(IDC_BUTTON_COPY))->SetIcon(m_hCopyIcon);
	((CButton*)GetDlgItem(IDC_BUTTON_PASTE))->SetIcon(m_hPasteIcon);	

	//load up the increment
	m_fIncrement = GetApp()->GetOptions().GetMiscOptions()->GetVectorEditIncrement();
	m_sIncrement.Format("%.6f", m_fIncrement);
	TrimZeros(m_sIncrement);

	//load up the auto apply
	m_bAutoApply = GetApp()->GetOptions().GetMiscOptions()->IsVectorEditAutoApply();

	//if there is no callback, don't show the auto apply check
	if(!m_UserCallback)
		GetDlgItem(IDC_CHECK_AUTOAPPLY)->ShowWindow(SW_HIDE);

	// Update the data
	UpdateData(FALSE);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

/************************************************************************/
// This is called to trim the trailing zeros from a floating point number
void CVectorEdit::TrimZeros(CString &sNumber)
{
	// Start at the end of the string
	int i=sNumber.GetLength()-1;

	// Decrement the index while the character is a zero and
	// the index isn't at the start of the string
	while (sNumber[i] == '0' && i > 1)
	{
		i--;
	}

	// Clip off the right side of the string removing the zeros
	sNumber=sNumber.Left(i+1);
	
	// If the last character is a '.' then add a zero
	int nLength=sNumber.GetLength();
	if (nLength > 0 && sNumber[nLength-1] == '.')
	{
		sNumber+="0";
	}
}

/************************************************************************/
// The vector string has changed
void CVectorEdit::OnChangeVectorEdit() 
{
	// Update the data
	UpdateData();

	//read in the strings and make a vector
	m_vVector.x = atof(m_sVecX);
	m_vVector.y = atof(m_sVecY);
	m_vVector.z = atof(m_sVecZ);

	//do the callback
	if(m_bAutoApply && m_UserCallback)
	{
		m_UserCallback(m_vVector, m_pUserCallbackData);
	}

}

/************************************************************************/
// The increment string has changed
void CVectorEdit::OnChangeIncrement() 
{
	// Update the data
	UpdateData();

	//read in the new increment
	CString sIncrement;
	GetDlgItem(IDC_VECTOR_EDIT_INC)->GetWindowText(sIncrement);

	m_fIncrement = atof(sIncrement);

	//now also save it
	GetApp()->GetOptions().GetMiscOptions()->SetVectorEditIncrement(m_fIncrement);
}


/************************************************************************/
// for copying and pasting of the vector

void CVectorEdit::OnCopy()
{
	//what we need to do here is fill this text into the dummy edit control we have set up and then
	//copy it to the clipboard (saves us the very ugly work of having to manage the clipboard ourselves
	CString sVal;

	//now get the strings...
	CString sEditVal;
	GetDlgItem(IDC_VECTOR_EDITX)->GetWindowText(sEditVal);
	sVal += sEditVal + ", ";
	GetDlgItem(IDC_VECTOR_EDITY)->GetWindowText(sEditVal);
	sVal += sEditVal + ", ";
	GetDlgItem(IDC_VECTOR_EDITZ)->GetWindowText(sEditVal);
	sVal += sEditVal;

	CEdit* pHelperEdit = (CEdit*)GetDlgItem(IDC_VECTOR_EDITZ);

	CString sOldVal;
	pHelperEdit->GetWindowText(sOldVal);
	pHelperEdit->SetWindowText(sVal);
	pHelperEdit->SetSel(0, -1);
	pHelperEdit->Cut();
	pHelperEdit->SetWindowText(sOldVal);

	//horay, it is now on the clipboard
}

static CString Tokenize(CString& sStr)
{
	const char* pszSpacingChars = " \n\r\t,";

	sStr.TrimLeft(pszSpacingChars);

	int nPos = sStr.FindOneOf(pszSpacingChars);

	if(nPos == -1)
		return sStr;

	CString sRV = sStr.Left(nPos);
	sStr = sStr.Mid(nPos + 1);

	return sRV;
}

void CVectorEdit::OnPaste()
{
	//have our invisible helper control paste the contents
	CEdit* pHelperEdit = (CEdit*)GetDlgItem(IDC_VECTOR_EDITX);
	CString sOldVal;
	pHelperEdit->GetWindowText(sOldVal);
	pHelperEdit->SetWindowText("");
	pHelperEdit->SetSel(0, -1);
	pHelperEdit->Paste();

	//ok, now read the string out and restore the contents
	CString sVal;
	pHelperEdit->GetWindowText(sVal);
	pHelperEdit->SetWindowText(sOldVal);

	//now set these into the appropriate fields
	GetDlgItem(IDC_VECTOR_EDITX)->SetWindowText(Tokenize(sVal));
	GetDlgItem(IDC_VECTOR_EDITY)->SetWindowText(Tokenize(sVal));
	GetDlgItem(IDC_VECTOR_EDITZ)->SetWindowText(Tokenize(sVal));
}

/************************************************************************/
// for handling the dialog's spin controls

void CVectorEdit::OnSpinX(NMHDR * pNotifyStruct, LRESULT * result)
{
	HandleSpin(IDC_VECTOR_EDITX, pNotifyStruct);
}

void CVectorEdit::OnSpinY(NMHDR * pNotifyStruct, LRESULT * result)
{
	HandleSpin(IDC_VECTOR_EDITY, pNotifyStruct);
}

void CVectorEdit::OnSpinZ(NMHDR * pNotifyStruct, LRESULT * result)
{
	HandleSpin(IDC_VECTOR_EDITZ, pNotifyStruct);
}


//handles the spin controls
void CVectorEdit::HandleSpin(DWORD nField, NMHDR * pNotifyStruct)
{
	//get the original value
	CString sWinText;
	GetDlgItem(nField)->GetWindowText(sWinText);

	//convert it to a value
	float fVal = atof(sWinText);

	//figure out which way we are going
	NMUPDOWN* pInfo = (NMUPDOWN*)pNotifyStruct;

	//note the minus. Yes this is correct. The spin control's history comes from a scroll bar
	//where up is negative.
	fVal -= pInfo->iDelta * m_fIncrement;

	//write it back out
	sWinText.Format("%.6f", fVal);
	TrimZeros(sWinText);
	GetDlgItem(nField)->SetWindowText(sWinText);
}

//Handle Auto apply
void CVectorEdit::OnAutoApply()
{
	UpdateData();

	//do the callback
	if(m_bAutoApply && m_UserCallback)
	{
		m_UserCallback(m_vVector, m_pUserCallbackData);
	}

	//save the value
	GetApp()->GetOptions().GetMiscOptions()->SetVectorEditAutoApply(m_bAutoApply);
}