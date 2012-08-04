// EditVectorDlg.cpp : implementation file
//

#include "stdafx.h"
#include "spelled.h"
#include "EditVectorDlg.h"
#include "math.h"
#include "matrix.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEditVectorDlg dialog


CEditVectorDlg::CEditVectorDlg(float *pfVec, CWnd* pParent /*=NULL*/)
	: CDialog(CEditVectorDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEditVectorDlg)
	m_magnitude = 1.0f;
	//}}AFX_DATA_INIT

	m_vec = *((CFXVector *)pfVec);
	m_magnitude = m_vec.Mag();
	m_vec.Norm();

	m_fPitch = 0.0f;
	m_fYaw   = 0.0f;
}


void CEditVectorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditVectorDlg)
	DDX_Control(pDX, IDC_PITCH, m_pitch);
	DDX_Control(pDX, IDC_YAW, m_yaw);
	DDX_Text(pDX, IDC_MAGNITUDE, m_magnitude);
	DDV_MinMaxFloat(pDX, m_magnitude, 1.e-003f, 1000.f);
	//}}AFX_DATA_MAP

	if (pDX->m_bSaveAndValidate)
	{
		m_tran *= m_magnitude;
	}
}


BEGIN_MESSAGE_MAP(CEditVectorDlg, CDialog)
	//{{AFX_MSG_MAP(CEditVectorDlg)
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_CBN_SELCHANGE(IDC_PRESETS, OnSelchangePresets)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditVectorDlg message handlers

void CEditVectorDlg::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	// Render the two boxes

	CPen pnWhite(PS_SOLID, 1, RGB(255, 255, 255));
	CPen *pOldPen = dc.SelectObject(&pnWhite);

	CFXMatrix mRot;
	mRot.MakeRotation(m_fPitch, m_fYaw, 0.0f);

	m_tran = m_vec;
	{
		CRect rcPitch;
		GetDlgItem(IDC_PITCH)->GetWindowRect(&rcPitch);
		ScreenToClient(&rcPitch);

		int cx = rcPitch.left + (rcPitch.Width() >> 1);
		int cy = rcPitch.top + (rcPitch.Height() >> 1);

		float width  = (float)(rcPitch.Width() >> 1);
		float height = (float)(rcPitch.Height() >> 1);

		float mag = (width < height) ? width : height;

		float x = m_tran.z * mag;
		float y = m_tran.y * mag;

		float nx = -m_tran.y * 10.0f;
		float ny = m_tran.z * 10.0f;

		dc.FillSolidRect(&rcPitch, RGB(0, 0, 0));
		dc.MoveTo((int)cx - (int)nx, (int)cy + (int)ny);
		dc.LineTo((int)cx + (int)nx, (int)cy - (int)ny);
		dc.LineTo((int)cx + (int)x, (int)cy - (int)y);
		dc.LineTo((int)cx - (int)nx, (int)cy + (int)ny);

		dc.MoveTo((int)cx + (int)x - 2, (int)cy - (int)y);
		dc.LineTo((int)cx + (int)x + 3, (int)cy - (int)y);

		dc.MoveTo((int)cx + (int)x, (int)cy - (int)y - 2);
		dc.LineTo((int)cx + (int)x, (int)cy - (int)y + 3);
	}

	{
		CRect rcYaw;
		GetDlgItem(IDC_YAW)->GetWindowRect(&rcYaw);
		ScreenToClient(&rcYaw);

		dc.FillSolidRect(&rcYaw, RGB(0, 0, 0));

		int cx = rcYaw.left + (rcYaw.Width() >> 1);
		int cy = rcYaw.top + (rcYaw.Height() >> 1);

		float width  = (float)(rcYaw.Width() >> 1);
		float height = (float)(rcYaw.Height() >> 1);

		float mag = (width < height) ? width : height;

		float x = m_tran.x * mag;
		float y = m_tran.z * mag;

		float nx = -m_tran.z * 10.0f;
		float ny = m_tran.x * 10.0f;

		dc.FillSolidRect(&rcYaw, RGB(0, 0, 0));
		dc.MoveTo((int)cx - (int)nx, (int)cy + (int)ny);
		dc.LineTo((int)cx + (int)nx, (int)cy - (int)ny);
		dc.LineTo((int)cx + (int)x, (int)cy - (int)y);
		dc.LineTo((int)cx - (int)nx, (int)cy + (int)ny);

		dc.MoveTo((int)cx + (int)x - 2, (int)cy - (int)y);
		dc.LineTo((int)cx + (int)x + 3, (int)cy - (int)y);

		dc.MoveTo((int)cx + (int)x, (int)cy - (int)y - 2);
		dc.LineTo((int)cx + (int)x, (int)cy - (int)y + 3);
	}

	dc.SelectObject(pOldPen);
}

void CEditVectorDlg::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CRect rcPitch;
	CRect rcYaw;

	GetDlgItem(IDC_PITCH)->GetWindowRect(&rcPitch);
	GetDlgItem(IDC_YAW)->GetWindowRect(&rcYaw);

	ScreenToClient(&rcPitch);
	ScreenToClient(&rcYaw);

	if ((rcPitch.PtInRect(point)) || (rcYaw.PtInRect(point)))
	{
		BOOL bPitch = rcPitch.PtInRect(point) ? TRUE : FALSE;
		CRect rcOrd = bPitch ? rcPitch : rcYaw;
		
		int cx = rcOrd.Width() >> 1;
		int cy = rcOrd.Height() >> 1;

		CPoint ptLast;
		float oldx, oldy;

		{
			CPoint ptCursor;
			GetCursorPos(&ptCursor);
			
			ScreenToClient(&ptCursor);
			
			ptCursor.x -= rcOrd.left;
			ptCursor.y -= rcOrd.top;

			ptLast = ptCursor;

			oldx = (float)(ptCursor.x - cx);
			oldy = (float)-(ptCursor.y - cy);
		}

		while (GetAsyncKeyState(VK_LBUTTON) & 0x8000)
		{
			MSG msg;

			if (PeekMessage(&msg, this->GetSafeHwnd(), NULL, NULL, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}

			CPoint ptCursor;
			GetCursorPos(&ptCursor);
			
			ScreenToClient(&ptCursor);
			
			ptCursor.x -= rcOrd.left;
			ptCursor.y -= rcOrd.top;

			if (ptLast != ptCursor)
			{
				float x = (float)(ptCursor.x - cx);
				float y = (float)-(ptCursor.y - cy);

				if (bPitch)
				{
					float fDiff = (((float)atan2(x, y) - (float)atan2(oldx, oldy)) / 2.0f);

					CFXMatrix mRot;
					mRot.MakeRotation(CFXVector(1, 0, 0), fDiff);

					mRot.Apply(&m_vec);
					m_vec.Norm();
				}
				else
				{
					float fDiff = (((float)atan2(x, y) - (float)atan2(oldx, oldy)) / 2.0f);

					CFXMatrix mRot;
					mRot.MakeRotation(CFXVector(0, 1, 0), fDiff);

					mRot.Apply(&m_vec);
					m_vec.Norm();
				}

				ptLast = ptCursor;

				oldx = x;
				oldy = y;

				InvalidateRect(&rcPitch, FALSE);
				InvalidateRect(&rcYaw, FALSE);
			}
		}
	}
	
	CDialog::OnLButtonDown(nFlags, point);
}

void CEditVectorDlg::OnSelchangePresets() 
{
	CComboBox *pBox = (CComboBox *)GetDlgItem(IDC_PRESETS);
	if (!pBox) return;
	
	int nCurSel = pBox->GetCurSel();
	if (nCurSel == CB_ERR) return;
	
	switch (nCurSel)
	{
		case 0 : m_vec = CFXVector(0, 1, 0); break;
		case 1 : m_vec = CFXVector(0, -1, 0); break;
		case 2 : m_vec = CFXVector(-1, 0, 0); break;
		case 3 : m_vec = CFXVector(1, 0, 0); break;
		case 4 : m_vec = CFXVector(0, 0, 1); break;
		case 5 : m_vec = CFXVector(0, 0, -1); break;
	}

	InvalidateRect(NULL, TRUE);
	
	pBox->SetCurSel(-1);	
}
