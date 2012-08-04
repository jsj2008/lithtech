//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
// RotationEdit.cpp : implementation file
//

#include "bdefs.h"
#include "..\dedit.h"
#include "rotationedit.h"
#include "optionsmisc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRotationEdit dialog
			

CRotationEdit::CRotationEdit(CWnd* pParent /*=NULL*/)
	: CDialog(CRotationEdit::IDD, pParent)
{
	//{{AFX_DATA_INIT(CRotationEdit)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_bIgnoreEditChanges = false;
	m_bAutoApply = false;

	m_UserCallback		= NULL;
	m_pUserData			= NULL;


	//load the icons for the buttons
	m_hCopyIcon  = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_COPY));
	m_hPasteIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_PASTE));

}

void CRotationEdit::SetUserCallback(TRotationEditUpdateCallback CallbackFn, void* pUserData)
{
	m_UserCallback		= CallbackFn;
	m_pUserData			= pUserData;
}

void CRotationEdit::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRotationEdit)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CRotationEdit, CDialog)
	//{{AFX_MSG_MAP(CRotationEdit)
	ON_WM_HSCROLL()
	ON_WM_PAINT()
	ON_EN_CHANGE(IDC_EDIT_ROTATION_YAW, OnEditAngleChanged)
	ON_EN_CHANGE(IDC_EDIT_ROTATION_PITCH, OnEditAngleChanged)
	ON_EN_CHANGE(IDC_EDIT_ROTATION_ROLL, OnEditAngleChanged)
	ON_BN_CLICKED(IDC_BUTTON_COPY, OnCopy)
	ON_BN_CLICKED(IDC_BUTTON_PASTE, OnPaste)
	ON_BN_CLICKED(IDC_CHECK_AUTOAPPLY, OnAutoApply)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//given any float, it will map it to [0...360)
static float MapAngleToRange(float fAngle)
{
	//first take care of negatives
	if(fAngle < 0)
	{
		return 360.0f - fmod(-fAngle, 360.0f);
	}
	else
	{
		return fmod(fAngle, 360.0f);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CRotationEdit message handlers

BOOL CRotationEdit::OnInitDialog() 
{
	SCROLLINFO info;
	float fMul;

	CDialog::OnInitDialog();

	//setup the icons on the buttons
	((CButton*)GetDlgItem(IDC_BUTTON_COPY))->SetIcon(m_hCopyIcon);
	((CButton*)GetDlgItem(IDC_BUTTON_PASTE))->SetIcon(m_hPasteIcon);	
	
	m_pYawBar = (CScrollBar*)GetDlgItem(IDC_YAWSCROLLBAR);
	m_pPitchBar = (CScrollBar*)GetDlgItem(IDC_PITCHSCROLLBAR);
	m_pRollBar = (CScrollBar*)GetDlgItem(IDC_ROLLSCROLLBAR);
	if(!m_pYawBar || !m_pPitchBar || !m_pRollBar)
		return FALSE;

	// Convert from radians to degrees on input..
	fMul = 360.0f / MATH_CIRCLE;
	m_EulerAngles *= fMul;

	//we also need to wrap the angles around so that they are all [0...360)
	m_EulerAngles.x = MapAngleToRange(m_EulerAngles.x);
	m_EulerAngles.y = MapAngleToRange(m_EulerAngles.y);
	m_EulerAngles.z = MapAngleToRange(m_EulerAngles.z);


	info.cbSize = sizeof(info);
	info.fMask = SIF_ALL;
	info.nMin = 0;
	info.nMax = 3600;
	info.nPage = 10;

	info.nPos = info.nTrackPos = (int)(m_EulerAngles.y * 10.0f);
	m_pYawBar->SetScrollInfo(&info);

	info.nPos = info.nTrackPos = (int)(m_EulerAngles.x * 10.0f);
	m_pPitchBar->SetScrollInfo(&info);

	info.nPos = info.nTrackPos = (int)(m_EulerAngles.z * 10.0f);
	m_pRollBar->SetScrollInfo(&info);

	m_bIgnoreEditChanges = false;

	UpdateScrollTexts();
	DrawGraphical();

	//load up the auto apply value
	m_bAutoApply = GetApp()->GetOptions().GetMiscOptions()->IsRotationEditAutoApply();
	((CButton*)GetDlgItem(IDC_CHECK_AUTOAPPLY))->SetCheck(m_bAutoApply ? 1 : 0);

	//hide the auto apply checkbox if there is no callback
	if(!m_UserCallback)
	{
		GetDlgItem(IDC_CHECK_AUTOAPPLY)->ShowWindow(SW_HIDE);
	}


	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CRotationEdit::DoCallback()
{
	if(m_bAutoApply && m_UserCallback)
	{
		m_UserCallback(m_EulerAngles * MATH_CIRCLE / 360.0f, m_pUserData);
	}
}


void CRotationEdit::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	if(nSBCode != SB_ENDSCROLL)
	{
		if(nSBCode == SB_PAGELEFT)
		{
			nPos = CLAMP(pScrollBar->GetScrollPos() - 100, 0, 3600);
		}
		else if(nSBCode == SB_PAGERIGHT)
		{
			nPos = CLAMP(pScrollBar->GetScrollPos() + 100, 0, 3600);
		}
		else if(nSBCode == SB_LINELEFT)
		{
			nPos = CLAMP(pScrollBar->GetScrollPos() - 10, 0, 3600);
		}
		else if(nSBCode == SB_LINERIGHT)
		{
			nPos = CLAMP(pScrollBar->GetScrollPos() + 10, 0, 3600);
		}

		if(pScrollBar)
		{
			pScrollBar->SetScrollPos(nPos);
			UpdateScrollTexts();
			DrawGraphical();
			DoCallback();
		}
	}

	//CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CRotationEdit::OnOK() 
{
	float fMul;

	// Convert from degrees to radians...
	fMul = MATH_CIRCLE / 360.0f;
	m_EulerAngles *= fMul;
	
	CDialog::OnOK();
}


void CRotationEdit::UpdateScrollTexts()
{
	CString theText;
	CWnd *pWnd;

	//tell the edit boxes to ignore the changes we will be sending
	m_bIgnoreEditChanges = true;

	m_EulerAngles.z = (float)m_pRollBar->GetScrollPos() / 10.0f;
	theText.Format("%.1f", m_EulerAngles.z);
	if(pWnd = GetDlgItem(IDC_EDIT_ROTATION_ROLL))
		pWnd->SetWindowText(theText);

	m_EulerAngles.x = (float)m_pPitchBar->GetScrollPos() / 10.0f;
	theText.Format("%.1f", m_EulerAngles.x);
	if(pWnd = GetDlgItem(IDC_EDIT_ROTATION_PITCH))
		pWnd->SetWindowText(theText);

	m_EulerAngles.y = (float)m_pYawBar->GetScrollPos() / 10.0f;
	theText.Format("%.1f", m_EulerAngles.y);
	if(pWnd = GetDlgItem(IDC_EDIT_ROTATION_YAW))
		pWnd->SetWindowText(theText);

	//let the edit boxes take control once again
	m_bIgnoreEditChanges = false;
}


void CRotationEdit::DrawGraphical()
{
	CWnd *pWnd;
	DVector radianRotation;
	float fMul;


	fMul = MATH_CIRCLE / 360.0f;
	radianRotation = m_EulerAngles * fMul;

	pWnd = GetDlgItem(IDC_YAWVIEW);
	DrawAngleToWnd(pWnd, -radianRotation.y + MATH_HALFPI);

	pWnd = GetDlgItem(IDC_PITCHVIEW);
	DrawAngleToWnd(pWnd, -radianRotation.x);
}


void CRotationEdit::DrawAngleToWnd(CWnd *pWnd, float radians)
{
	CRect rect;
	POINT center, outward;
	CBrush theBrush(RGB(0,0,0));
	CBrush *pOldBrush;
	CPen pen(PS_SOLID, 1, RGB(255,255,255));
	CPen *pOldPen;
	CDC *pDC;

	if(pWnd)
	{
		pWnd->GetClientRect(&rect);
		center.x = rect.Width() / 2;
		center.y = rect.Height() / 2;
		outward.x = center.x + (int)(cos(radians) * (float)(rect.Width() / 2));
		outward.y = center.y + (int)(-sin(radians) * (float)(rect.Height() / 2));
		
		pDC = pWnd->GetDC();
		if(pDC)
		{
			pOldBrush = pDC->SelectObject(&theBrush);
//			pDC->Rectangle(&rect);
			pDC->Ellipse(&rect);
			pDC->SelectObject(pOldBrush);

			pOldPen = pDC->SelectObject(&pen);
			pDC->MoveTo(center);
			pDC->LineTo(outward);
			pDC->SelectObject(pOldPen);
			
			pWnd->ReleaseDC(pDC);
		}
	}
}




void CRotationEdit::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	DrawGraphical();	
	// Do not call CDialog::OnPaint() for painting messages
}

//given the ID of an edit control, it will get its floating point value
float CRotationEdit::GetEditValue(DWORD nID)
{
	//get the text
	CString sText;
	GetDlgItem(nID)->GetWindowText(sText);

	//convert it
	return (float)atof(sText);
}

void CRotationEdit::OnEditAngleChanged()
{
	//sync the scrollbars to the specified value
	if(!m_bIgnoreEditChanges)
	{
		m_EulerAngles.x = GetEditValue(IDC_EDIT_ROTATION_PITCH);
		m_pPitchBar->SetScrollPos((int)(m_EulerAngles.x * 10.0f));

		m_EulerAngles.y = GetEditValue(IDC_EDIT_ROTATION_YAW);
		m_pYawBar->SetScrollPos((int)(m_EulerAngles.y * 10.0f));

		m_EulerAngles.z = GetEditValue(IDC_EDIT_ROTATION_ROLL);
		m_pRollBar->SetScrollPos((int)(m_EulerAngles.z * 10.0f));

		DrawGraphical();

		DoCallback();
	}
}

#define CLIPBOARD_INTRO_SEQ			"&-*"
#define CLIPBOARD_INTRO_SEQ_LEN		3


void CRotationEdit::OnCopy()
{
	//what we need to do here is fill this text into the dummy edit control we have set up and then
	//copy it to the clipboard (saves us the very ugly work of having to manage the clipboard ourselves

	//start the string off with some wheird character sequence so we can verify it in paste
	CString sVal = CLIPBOARD_INTRO_SEQ;

	//now get the strings...
	CString sEditVal;
	GetDlgItem(IDC_EDIT_ROTATION_YAW)->GetWindowText(sEditVal);
	sVal += sEditVal + ",";
	GetDlgItem(IDC_EDIT_ROTATION_PITCH)->GetWindowText(sEditVal);
	sVal += sEditVal + ",";
	GetDlgItem(IDC_EDIT_ROTATION_ROLL)->GetWindowText(sEditVal);
	sVal += sEditVal;

	//now set this into the invisible control and move it to the clipboard
	CEdit* pHelperEdit = (CEdit*)GetDlgItem(IDC_EDIT_COPYPASTE);
	pHelperEdit->SetWindowText(sVal);
	pHelperEdit->SetSel(0, -1);
	pHelperEdit->Cut();

	//horay, it is now on the clipboard
}

static CString Tokenize(CString& sStr, char ch)
{
	int nPos = sStr.Find(ch);

	if(nPos == -1)
		return CString("");

	CString sRV = sStr.Left(nPos);
	sStr = sStr.Mid(nPos + 1);

	return sRV;
}

void CRotationEdit::OnPaste()
{
	//have our invisible helper control paste the contents
	CEdit* pHelperEdit = (CEdit*)GetDlgItem(IDC_EDIT_COPYPASTE);
	pHelperEdit->SetWindowText("");
	pHelperEdit->SetSel(0, -1);
	pHelperEdit->Paste();

	//ok, now read the string out
	CString sVal;
	pHelperEdit->GetWindowText(sVal);

	//now we can see if the string is of a valid format
	if((sVal.GetLength() < CLIPBOARD_INTRO_SEQ_LEN) || (sVal.Left(CLIPBOARD_INTRO_SEQ_LEN) != CLIPBOARD_INTRO_SEQ))
	{
		//this isn't a valid string, we can't paste it
		return;
	}

	//ok, this is valid, take off the header
	sVal = sVal.Mid(CLIPBOARD_INTRO_SEQ_LEN);

	//now set these into the appropriate fields
	GetDlgItem(IDC_EDIT_ROTATION_YAW)->SetWindowText(Tokenize(sVal, ','));
	GetDlgItem(IDC_EDIT_ROTATION_PITCH)->SetWindowText(Tokenize(sVal, ','));
	GetDlgItem(IDC_EDIT_ROTATION_ROLL)->SetWindowText(sVal);
}

void CRotationEdit::OnAutoApply()
{
	//read in the new auto apply value
	m_bAutoApply = ((CButton*)GetDlgItem(IDC_CHECK_AUTOAPPLY))->GetCheck() ? true : false;
	GetApp()->GetOptions().GetMiscOptions()->SetRotationEditAutoApply(m_bAutoApply);

	DoCallback();
}