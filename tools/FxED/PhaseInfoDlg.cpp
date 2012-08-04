// PhaseInfoDlg.cpp : implementation file
//

#include "stdafx.h"
#include "spelled.h"
#include "PhaseInfoDlg.h"
#include "TrackWnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPhaseInfoDlg dialog


CPhaseInfoDlg::CPhaseInfoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPhaseInfoDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPhaseInfoDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CPhaseInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPhaseInfoDlg)
	DDX_Control(pDX, IDC_PHASELENGTH, m_phaseLength);
	DDX_Control(pDX, IDC_KEYREPEATS, m_keyRepeats);
	//}}AFX_DATA_MAP

	if (!pDX->m_bSaveAndValidate)
	{
		// We are loading the dialog

		m_msTotalTime = m_pTimeBar->GetTotalTime();
		m_phaseLength.SetValue("Choose phase length in Milliseconds", &m_msTotalTime);

		m_nKeyRepeat = 1;
		m_keyRepeats.SetValue("Choose Key Repeat", &m_nKeyRepeat);
		
		Refresh();
	}
}


BEGIN_MESSAGE_MAP(CPhaseInfoDlg, CDialog)
	//{{AFX_MSG_MAP(CPhaseInfoDlg)
//	ON_MESSAGE(WM_VALUEUPDATE, OnValueUpdate)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPhaseInfoDlg message handlers

//------------------------------------------------------------------
//
//   FUNCTION : OnValueUpdate()
//
//   PURPOSE  : Refresh everything
//
//------------------------------------------------------------------

void CPhaseInfoDlg::OnValueUpdate()
{
	CTrackWnd *pWnd = (CTrackWnd *)GetParent();
	if (!pWnd) return;

	// If we have a selected key, set the repeats

	if (pWnd->GetSelKey())
	{
		if (m_nKeyRepeat <= 0) m_nKeyRepeat = 1;

		pWnd->GetSelKey()->SetKeyRepeat((DWORD)m_nKeyRepeat);
	}

	// Set the total time

	m_pTimeBar->SetTotalTime(m_msTotalTime);
	pWnd->GetPhase()->SetPhaseLength(m_msTotalTime);
}

//------------------------------------------------------------------
//
//   FUNCTION : Refresh()
//
//   PURPOSE  : Updates and redraws all important information
//
//------------------------------------------------------------------

void CPhaseInfoDlg::Refresh()
{
	char sTmp[256];

	CTrackWnd *pWnd = (CTrackWnd *)GetParent();
	if (!pWnd) return;

	BOOL bMultiSelect = (pWnd->NumSelected() > 1) ? TRUE : FALSE;

	CKey *pKey = pWnd->GetSelKey();
	if (pKey)
	{
		CString sName;
		sName = "Key Info [";
		sName += pKey->GetFxRef()->m_sName;

		if( pKey->GetCustomName()[0] )
		{
			sName += " - ";
			sName += pKey->GetCustomName();
		}
		
		sName += "]";
		
		// Fill out the key info

		GetDlgItem(IDC_KEYINFO)->EnableWindow(TRUE);
		GetDlgItem(IDC_KEYINFO)->SetWindowText(sName);
		GetDlgItem(IDC_KEYINFO1)->EnableWindow(TRUE);
		GetDlgItem(IDC_KEYINFO2)->EnableWindow(TRUE);
		GetDlgItem(IDC_KEYINFO3)->EnableWindow(TRUE);
		GetDlgItem(IDC_KEYINFO4)->EnableWindow(TRUE);
		GetDlgItem(IDC_KEYREPEATS)->EnableWindow(FALSE);		

		m_nKeyRepeat = pKey->GetKeyRepeat();
		m_keyRepeats.SetValue("Choose Key Repeat", &m_nKeyRepeat);

		if (!bMultiSelect)
		{
			sprintf(sTmp, "%d ms", pKey->GetStartTime());
			GetDlgItem(IDC_STARTTIME)->SetWindowText(sTmp);

			sprintf(sTmp, "%d ms", pKey->GetEndTime());
			GetDlgItem(IDC_ENDTIME)->SetWindowText(sTmp);

			sprintf(sTmp, "%d ms", pKey->GetTotalTime());
			GetDlgItem(IDC_TOTALTIME)->SetWindowText(sTmp);
		}
		else
		{
			GetDlgItem(IDC_STARTTIME)->SetWindowText("---");
			GetDlgItem(IDC_ENDTIME)->SetWindowText("---");
			GetDlgItem(IDC_TOTALTIME)->SetWindowText("---");

			GetDlgItem(IDC_KEYINFO4)->EnableWindow(FALSE);
			GetDlgItem(IDC_KEYREPEATS)->EnableWindow(FALSE);
		}
	}
	else
	{
		GetDlgItem(IDC_KEYINFO)->EnableWindow(FALSE);
		GetDlgItem(IDC_KEYINFO)->SetWindowText("Key Info");
		GetDlgItem(IDC_KEYINFO1)->EnableWindow(FALSE);
		GetDlgItem(IDC_KEYINFO2)->EnableWindow(FALSE);
		GetDlgItem(IDC_KEYINFO3)->EnableWindow(FALSE);

		GetDlgItem(IDC_STARTTIME)->SetWindowText("");
		GetDlgItem(IDC_ENDTIME)->SetWindowText("");
		GetDlgItem(IDC_TOTALTIME)->SetWindowText("");

		GetDlgItem(IDC_KEYINFO4)->EnableWindow(FALSE);
		GetDlgItem(IDC_KEYREPEATS)->EnableWindow(FALSE);
	}

	// Count the number of tracks

	int nTracks = pWnd->GetPhase()->GetTracks()->GetSize();
	sprintf(sTmp, "%d", nTracks);
	GetDlgItem(IDC_TOTALTRACKS)->SetWindowText(sTmp);

	// Count the number of keys

	CLinkListNode<CTrack *> *pTrackNode = pWnd->GetPhase()->GetTracks()->GetHead();

	int nKeys = 0;
	
	while (pTrackNode)
	{
		nKeys += pTrackNode->m_Data->GetKeys()->GetSize();
		
		pTrackNode = pTrackNode->m_pNext;
	}
	sprintf(sTmp, "%d", nKeys);
	GetDlgItem(IDC_TOTALKEYS)->SetWindowText(sTmp);
}

LRESULT CPhaseInfoDlg::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	if (message == WM_VALUEUPDATE)
	{
		OnValueUpdate();
	}
	
	return CDialog::DefWindowProc(message, wParam, lParam);
}
