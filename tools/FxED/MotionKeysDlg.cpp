// MotionKeysDlg.cpp : implementation file
//

#include "stdafx.h"
#include "spelled.h"
#include "MotionKeysDlg.h"
#include "clippers.h"
#include "ChooseMotionAnimDlg.h"
#include "StringDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMotionKeysDlg dialog


CMotionKeysDlg::CMotionKeysDlg(CKey *pKey, CWnd* pParent /*=NULL*/)
	: CDialog(CMotionKeysDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMotionKeysDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_pKey = pKey;
	
	m_pMemDC     = NULL;
	m_pBitmap    = NULL;
	m_pOldBitmap = NULL;
	m_nViewType	 = VT_BACK;

	m_xRot		 = 0.0f;
	m_yRot		 = 0.0f;
	m_fzDist	 = 200.0f;

	m_bShowPaths   = TRUE;
	m_bShowMarkers = TRUE;
	m_bShowNumbers = TRUE;

	m_nCurMode	   = CM_CREATE;

	m_bUsePreset   = FALSE;
	m_nPresetAnim  = 0;
	m_nSize		   = 0;
	m_nReps		   = 0;

	m_bShowTrack   = FALSE;
}


void CMotionKeysDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMotionKeysDlg)
	//}}AFX_DATA_MAP

	if (!pDX->m_bSaveAndValidate)
	{
		// Load up the dialog

		CLinkListNode<MOVEKEY> *pNode = m_pKey->GetMoveKeys()->GetHead();

		while (pNode)
		{
			m_collKeys.AddTail(pNode->m_Data);
			
			pNode = pNode->m_pNext;
		}

		// Load the presets dialog

		CSpellEdApp *pApp = (CSpellEdApp *)AfxGetApp();

		if (!pApp->GetMoveFavourites()->GetSize()) GetDlgItem(IDC_CHOOSEMOVEFAVOURITE)->EnableWindow(FALSE);
	}
	else
	{
		// Load the key

		m_pKey->GetMoveKeys()->RemoveAll();

		m_pKey->SetUsePresetAnim(m_bUsePreset);

		if (m_bUsePreset)
		{
			m_nPresetAnim = ((CComboBox *)GetDlgItem(IDC_PRESET))->GetCurSel();
			
			m_pKey->SetPresetAnim(m_nPresetAnim);
			m_pKey->SetRepsAnim(m_nReps);
		}

		if (!m_collKeys.GetSize())
		{
			return;
		}

		// Compute the times for the keys

		CLinkListNode<MOVEKEY> *pNode = m_collKeys.GetHead();

		float fTotalDist = 0.0f;
		
		while (pNode)
		{
			fTotalDist += pNode->m_Data.m_pos.Mag();
			
			pNode = pNode->m_pNext;
		}

		// fTotalDist contains the total length of the path

		float fCurTime = 0.0f;
		float fCurDist = 0.0f;

		pNode = m_collKeys.GetHead();

		while (pNode)
		{
			float fLen = pNode->m_Data.m_pos.Mag();
			float fTime = fLen / fTotalDist;

			MOVEKEY mk;
			mk.m_pos   = pNode->m_Data.m_pos;
			
			if (pNode->m_pNext)
			{
				mk.m_tmKey = fCurTime;
			}
			else
			{
				mk.m_tmKey = 1.0f;
			}

			m_pKey->GetMoveKeys()->AddTail(mk);
			
			fCurTime += fTime;
			
			pNode = pNode->m_pNext;
		}
	}
}


BEGIN_MESSAGE_MAP(CMotionKeysDlg, CDialog)
	//{{AFX_MSG_MAP(CMotionKeysDlg)
	ON_WM_LBUTTONDOWN()
	ON_WM_PAINT()
	ON_WM_RBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_BN_CLICKED(IDC_SHOWMARKERS, OnShowmarkers)
	ON_BN_CLICKED(IDC_SHOWNUMBERS, OnShownumbers)
	ON_BN_CLICKED(IDC_SHOWPATHS, OnShowpaths)
	ON_BN_CLICKED(IDC_DELETEALL, OnDeleteall)
	ON_BN_CLICKED(IDC_MIRRORX, OnMirrorx)
	ON_BN_CLICKED(IDC_MIRRORY, OnMirrory)
	ON_BN_CLICKED(IDC_MIRRORZ, OnMirrorz)
	ON_BN_CLICKED(IDC_CREATEKEY, OnCreateKey)
	ON_BN_CLICKED(IDC_DELETESINGLEKEY, OnDeleteSingleKey)
	ON_BN_CLICKED(IDC_MOVEKEY, OnMoveKey)
	ON_BN_CLICKED(IDC_SELECTKEY, OnSelectKey)
	ON_BN_CLICKED(IDC_SELECTALL, OnSelectAll)
	ON_BN_CLICKED(IDC_UNSELECTALL, OnUnselectAll)
	ON_BN_CLICKED(IDC_USEPRESET, OnUsePreset)
	ON_BN_CLICKED(IDC_ADDTOMOVEFAVOURITES, OnAddToMoveFavourites)
	ON_BN_CLICKED(IDC_CHOOSEMOVEFAVOURITE, OnChooseMoveFavourite)
	ON_BN_CLICKED(IDC_SPLIT, OnSplit)
	ON_BN_CLICKED(IDC_ROTATE, OnRotate)
	ON_BN_CLICKED(IDC_SCALE, OnScale)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMotionKeysDlg message handlers

//------------------------------------------------------------------
//
//   FUNCTION : OnLButtonDown()
//
//   PURPOSE  : Handles WM_LBUTTONDOWN
//
//------------------------------------------------------------------

void CMotionKeysDlg::OnLButtonDown(UINT nFlags, CPoint point) 
{
	if ((m_nCurMode != CM_SELECT) && (GetAsyncKeyState(VK_CONTROL) & 0x8000))
	{
		SetCurrentMode(CM_SELECT);
	}

	CFXMatrix mInv = m_mView;
	mInv.Inverse();

	CRect rcView;
	GetDlgItem(IDC_MOTION)->GetWindowRect(&rcView);
	ScreenToClient(&rcView);

	int sx = rcView.Width() / 2;
	int sy = rcView.Height() / 2;

	CPoint ptClick = point;
	ptClick.x -= rcView.left;
	ptClick.y -= rcView.top;

	ptClick.x = ptClick.x - sx;
	ptClick.y = -(ptClick.y - sy);

	if ((m_nViewType != VT_PERSPECTIVE) && (rcView.PtInRect(point)))
	{
		switch (m_nCurMode)
		{
			case CM_SPLIT :
			{
				CLinkListNode<MOVEKEY> *pKey = GetKeyByPos(ptClick);

				if (pKey)
				{
					if (pKey->m_pPrev)
					{
						CFXVector vMid = (pKey->m_Data.m_pos + pKey->m_pPrev->m_Data.m_pos) / 2;

						MOVEKEY mk;
						mk.m_bSelected = FALSE;
						mk.m_pos	   = vMid;
						mk.m_tmKey	   = 0.0f;

						m_collKeys.InsertBefore(pKey, mk);
					}

					if (pKey->m_pNext)
					{
						CFXVector vMid = (pKey->m_Data.m_pos + pKey->m_pNext->m_Data.m_pos) / 2;

						MOVEKEY mk;
						mk.m_bSelected = FALSE;
						mk.m_pos	   = vMid;
						mk.m_tmKey	   = 0.0f;

						m_collKeys.InsertAfter(pKey, mk);
					}

					DrawView();
				}
			}
			break;

			case CM_CREATE :
			{
				// Create a new point
				
				CFXVector vPt((float)ptClick.x, (float)ptClick.y, 0.0f);
				mInv.Apply(&vPt);

				MOVEKEY mk;
				mk.m_tmKey = 0.0f;
				mk.m_pos   = vPt;

				m_collKeys.AddTail(mk);
			}
			break;

			case CM_SELECT :
			{
				CLinkListNode<MOVEKEY> *pKey = GetKeyByPos(ptClick);

				if (pKey)
				{
					pKey->m_Data.m_bSelected = !pKey->m_Data.m_bSelected;

					DrawView();
				}
				else
				{
					// Do a rectangle track

					CPoint ptAnchor = point;
					ptAnchor.x -= rcView.left;
					ptAnchor.y -= rcView.top;

					m_bShowTrack = TRUE;

					while (GetAsyncKeyState(VK_LBUTTON) & 0x8000)
					{
						CPoint ptCur;
						GetCursorPos(&ptCur);
						ScreenToClient(&ptCur);

						ptCur.x -= rcView.left;
						ptCur.y -= rcView.top;

						m_rcTrack.left   = ptAnchor.x;
						m_rcTrack.top    = ptAnchor.y;
						m_rcTrack.right  = ptCur.x;
						m_rcTrack.bottom = ptCur.y;

						DrawView();
					}

					m_bShowTrack = FALSE;

					m_rcTrack.NormalizeRect();

					// Select all nodes inside the rectangle

					CLinkListNode<MOVEKEY> *pNode = m_collKeys.GetHead();

					while (pNode)
					{
						CFXVector vKey = pNode->m_Data.m_pos;
						m_mView.Apply(&vKey);

						CPoint ptKey(sx + (int)vKey.x, sy - (int)vKey.y);

						if (m_rcTrack.PtInRect(ptKey))
						{
							pNode->m_Data.m_bSelected = !pNode->m_Data.m_bSelected;
						}
						
						pNode = pNode->m_pNext;
					}

					DrawView();
				}
			}
			break;
			
			case CM_MOVE :
			{
				CPoint ptAnchor = point;

				CFXVector vAnchor((float)ptClick.x, (float)ptClick.y, 0.0f);
				mInv.Apply(&vAnchor);

				CLinkListNode<MOVEKEY> *pSelNode = GetKeyByPos(ptClick);
				MOVEKEY *pSelKey = NULL;
				
				BOOL bPreSelected = FALSE;
				if (pSelNode) pSelKey = &pSelNode->m_Data;
				if (pSelKey)
				{
					if (pSelKey->m_bSelected) bPreSelected = TRUE;
					pSelKey->m_bSelected = TRUE;
				}

				CLinkListNode<MOVEKEY> *pNode = m_collKeys.GetHead();

				while (pNode)
				{
					pNode->m_Data.m_anchorPos = pNode->m_Data.m_pos;

					pNode = pNode->m_pNext;
				}

				while (GetAsyncKeyState(VK_LBUTTON) & 0x8000)
				{
					CPoint ptCur;
					GetCursorPos(&ptCur);
					ScreenToClient(&ptCur);

					ptCur.x -= rcView.left;
					ptCur.y -= rcView.top;

					CFXVector vWorld((float)ptCur.x - sx, (float)-(ptCur.y - sy), 0.0f);
					mInv.Apply(&vWorld);

					vWorld -= vAnchor;
					
					pNode = m_collKeys.GetHead();

					while (pNode)
					{
						if (pNode->m_Data.m_bSelected)
						{									
							pNode->m_Data.m_pos = pNode->m_Data.m_anchorPos + vWorld;
						}
						
						pNode = pNode->m_pNext;
					}
					
					DrawView();
				}

				if ((!bPreSelected) && (pSelKey)) pSelKey->m_bSelected = FALSE;

				DrawView();
			}
			break;

			case CM_SCALE :
			{
				CPoint ptAnchor = point;

				CFXVector vAnchor((float)ptClick.x, (float)ptClick.y, 0.0f);
				mInv.Apply(&vAnchor);

				CLinkListNode<MOVEKEY> *pSelNode = GetKeyByPos(ptClick);
				MOVEKEY *pSelKey = NULL;
				
				BOOL bPreSelected = FALSE;
				if (pSelNode) pSelKey = &pSelNode->m_Data;
				if (pSelKey)
				{
					if (pSelKey->m_bSelected) bPreSelected = TRUE;
					pSelKey->m_bSelected = TRUE;
				}

				CLinkListNode<MOVEKEY> *pNode = m_collKeys.GetHead();

				while (pNode)
				{
					pNode->m_Data.m_anchorPos = pNode->m_Data.m_pos;

					pNode = pNode->m_pNext;
				}

				GetCursorPos(&ptAnchor);
				ScreenToClient(&ptAnchor);
				
				ptAnchor.x -= rcView.left;
				ptAnchor.y -= rcView.top;					

				while (GetAsyncKeyState(VK_LBUTTON) & 0x8000)
				{
					CPoint ptCur;
					GetCursorPos(&ptCur);
					ScreenToClient(&ptCur);

					ptCur.x -= rcView.left;
					ptCur.y -= rcView.top;					
					
					pNode = m_collKeys.GetHead();

					float fScale = 1.0f + ((float)(ptCur.x - ptAnchor.x) / 100.0f);

					while (pNode)
					{
						if (pNode->m_Data.m_bSelected)
						{									
							pNode->m_Data.m_pos = (pNode->m_Data.m_anchorPos * fScale);
						}
						
						pNode = pNode->m_pNext;
					}
					
					DrawView();
				}

				if ((!bPreSelected) && (pSelKey)) pSelKey->m_bSelected = FALSE;

				DrawView();
			}
			break;

			case CM_DELETE :
			{
				CLinkListNode<MOVEKEY> *pKey = GetKeyByPos(ptClick);

				if (pKey)
				{
					m_collKeys.Remove(pKey);

					DrawView();
				}
			}
			break;

			case CM_ROTATE :
			{
				CPoint ptAnchor = point;

				CFXVector vAnchor((float)(point.x - rcView.left) - sx, (float)-((point.y - rcView.top) - sy), 0.0f);
				mInv.Apply(&vAnchor);

				CLinkListNode<MOVEKEY> *pSelNode = GetKeyByPos(ptClick);
				MOVEKEY *pSelKey = NULL;
				
				BOOL bPreSelected = FALSE;
				if (pSelNode) pSelKey = &pSelNode->m_Data;
				if (pSelKey)
				{
					if (pSelKey->m_bSelected) bPreSelected = TRUE;
					pSelKey->m_bSelected = TRUE;
				}

				CLinkListNode<MOVEKEY> *pNode = m_collKeys.GetHead();

				while (pNode)
				{
					pNode->m_Data.m_anchorPos = pNode->m_Data.m_pos;

					pNode = pNode->m_pNext;
				}

				while (GetAsyncKeyState(VK_LBUTTON) & 0x8000)
				{
					CPoint ptCur;
					GetCursorPos(&ptCur);
					ScreenToClient(&ptCur);

					ptCur.x -= rcView.left;
					ptCur.y -= rcView.top;

					float rot = (float)(ptCur.x - (ptAnchor.x - rcView.left)) / 100.0f;

					CFXMatrix mRot;
					mRot.MakeRotation(0.0f, 0.0f, -rot);

					CFXMatrix mTmpInv = mInv;

					mTmpInv = mInv * mRot * m_mView;
				
					pNode = m_collKeys.GetHead();

					while (pNode)
					{
						if (pNode->m_Data.m_bSelected)
						{									
							CFXVector vTmp = pNode->m_Data.m_anchorPos;
							mTmpInv.Apply(&vTmp);

							pNode->m_Data.m_pos = vTmp;
						}
						
						pNode = pNode->m_pNext;
					}
					
					DrawView();
				}

				if ((!bPreSelected) && (pSelKey)) pSelKey->m_bSelected = FALSE;

				DrawView();
			}
			break;
		}

		DrawView();
	}
	
	CDialog::OnLButtonDown(nFlags, point);
}

//------------------------------------------------------------------
//
//   FUNCTION : OnInitDialog()
//
//   PURPOSE  : Handles WM_INITDIALOG
//
//------------------------------------------------------------------

BOOL CMotionKeysDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	CRect rcView;
	GetDlgItem(IDC_MOTION)->GetClientRect(&rcView);

	m_sx = rcView.Width();
	m_sy = rcView.Height();
	
	// Create memory dc

	CDC *pDC = GetDC();

	m_pMemDC = new CDC;
	m_pBitmap = new CBitmap;

	m_pMemDC->CreateCompatibleDC(pDC);
	m_pBitmap->CreateCompatibleBitmap(pDC, m_sx, m_sy);
	m_pOldBitmap = m_pMemDC->SelectObject(m_pBitmap);
	m_pMemDC->SetBkMode(TRANSPARENT);

	ReleaseDC(pDC);

	// Set the view type

	SetViewType(VT_BACK);

	((CButton *)GetDlgItem(IDC_SHOWMARKERS))->SetCheck(1);
	((CButton *)GetDlgItem(IDC_SHOWNUMBERS))->SetCheck(1);
	((CButton *)GetDlgItem(IDC_SHOWPATHS))->SetCheck(1);

	SetCurrentMode(CM_CREATE);
	
	// Success !!
	
	return TRUE;  
}

//------------------------------------------------------------------
//
//   FUNCTION : OnPaint()
//
//   PURPOSE  : Handles WM_PAINT
//
//------------------------------------------------------------------

void CMotionKeysDlg::OnPaint() 
{
	CPaintDC dc(this);

	DrawView();
}

//------------------------------------------------------------------
//
//   FUNCTION : DrawView()
//
//   PURPOSE  : Draws the 3D motion control and all the keys
//
//------------------------------------------------------------------

void CMotionKeysDlg::DrawView()
{
	if (!m_pMemDC) return;

	CDC *pDC = GetDC();
	CRect rcMotion;
	GetDlgItem(IDC_MOTION)->GetWindowRect(&rcMotion);
	ScreenToClient(&rcMotion);

	int sx = rcMotion.Width() / 2;
	int sy = rcMotion.Height() / 2;

	// Clear the offscreen

	m_pMemDC->FillSolidRect(0, 0, m_sx, m_sy, RGB(0, 0, 0));

	// Draw the key points

	CLinkListNode<MOVEKEY> *pNode = m_collKeys.GetHead();

	CPen pnRed(PS_SOLID, 1, RGB(255, 0, 0));
	CPen pnGreen(PS_SOLID, 1, RGB(0, 255, 0));
	CPen pnBlue(PS_SOLID, 1, RGB(0, 0, 255));
	CPen pnGray(PS_SOLID, 1, RGB(200, 200, 200));
	CPen pnDot(PS_DOT, 1, RGB(200, 200, 200));
	CPen pnDash(PS_DOT, 1, RGB(128, 128, 128));

	CPen *ppnOld = m_pMemDC->SelectObject(&pnRed);

	// Draw the axis

	int nAxisLen = 40;

	switch (m_nViewType)
	{
		case VT_BACK :
		{
			m_pMemDC->SelectObject(&pnDot);
			m_pMemDC->MoveTo(sx, 0);
			m_pMemDC->LineTo(sx, sy * 2);
			m_pMemDC->MoveTo(0, sy);
			m_pMemDC->LineTo(sx * 2, sy);
			
			m_pMemDC->SelectObject(&pnRed);
			m_pMemDC->MoveTo(sx, sy);
			m_pMemDC->LineTo(sx + nAxisLen, sy);
			m_pMemDC->SetTextColor(RGB(255, 0, 0));
			m_pMemDC->TextOut(sx + nAxisLen + 2, sy - 6, "X");

			m_pMemDC->SelectObject(&pnGreen);
			m_pMemDC->MoveTo(sx, sy);
			m_pMemDC->LineTo(sx, sy - nAxisLen);
			m_pMemDC->SetTextColor(RGB(0, 255, 0));
			m_pMemDC->TextOut(sx - 4, sy - nAxisLen - 10, "Y");
		}
		break;

		case VT_RIGHT :
		{
			m_pMemDC->SelectObject(&pnDot);
			m_pMemDC->MoveTo(sx, 0);
			m_pMemDC->LineTo(sx, sy * 2);
			m_pMemDC->MoveTo(0, sy);
			m_pMemDC->LineTo(sx * 2, sy);

			m_pMemDC->SelectObject(&pnBlue);
			m_pMemDC->MoveTo(sx, sy);
			m_pMemDC->LineTo(sx + nAxisLen, sy);
			m_pMemDC->SetTextColor(RGB(0, 0, 255));
			m_pMemDC->TextOut(sx + nAxisLen + 2, sy - 6, "Z");

			m_pMemDC->SelectObject(&pnGreen);
			m_pMemDC->MoveTo(sx, sy);
			m_pMemDC->LineTo(sx, sy - nAxisLen);
			m_pMemDC->SetTextColor(RGB(0, 255, 0));
			m_pMemDC->TextOut(sx - 4, sy - nAxisLen - 10, "Y");
		}
		break;

		case VT_TOP :
		{
			m_pMemDC->SelectObject(&pnDot);
			m_pMemDC->MoveTo(sx, 0);
			m_pMemDC->LineTo(sx, sy * 2);
			m_pMemDC->MoveTo(0, sy);
			m_pMemDC->LineTo(sx * 2, sy);

			m_pMemDC->SelectObject(&pnRed);
			m_pMemDC->MoveTo(sx, sy);
			m_pMemDC->LineTo(sx + nAxisLen, sy);
			m_pMemDC->SetTextColor(RGB(255, 0, 0));
			m_pMemDC->TextOut(sx + nAxisLen + 2, sy - 6, "X");

			m_pMemDC->SelectObject(&pnBlue);
			m_pMemDC->MoveTo(sx, sy);
			m_pMemDC->LineTo(sx, sy - nAxisLen);
			m_pMemDC->SetTextColor(RGB(0, 0, 255));
			m_pMemDC->TextOut(sx - 4, sy - nAxisLen - 10, "Z");
		}
		break;

		case VT_PERSPECTIVE :
		{
			// Draw the grid

			CFXMatrix mRot;
			mRot.MakeRotation(m_xRot, m_yRot, 0.0f);

			CFXMatrix mTrans;
			mTrans.Identity();
			mTrans.m_Elem[11] = m_fzDist;

			CFXMatrix mGrid = mTrans * mRot;

			for (int i = 0; i < 11; i ++)
			{
				CFXVector vStart(-200 + (float)(i * 40), 0.0f, -200);
				CFXVector vEnd(-200 + (float)(i * 40), 0.0f, 200);

				mGrid.Apply(&vStart);
				mGrid.Apply(&vEnd);

				if (ClipFront(vStart, vEnd, 10.0f))
				{
					Project(&vStart);
					Project(&vEnd);

					if (ClipLeft(vStart, vEnd, 0.0f))
					{
						if (ClipRight(vStart, vEnd, (float)rcMotion.Width()))
						{
							if (ClipTop(vStart, vEnd, 0.0f))
							{
								if (ClipBottom(vStart, vEnd, (float)rcMotion.Height()))
								{
									m_pMemDC->MoveTo((int)vStart.x, (int)vStart.y);
									m_pMemDC->LineTo((int)vEnd.x, (int)vEnd.y);
								}
							}
						}
					}
				}
			}

			for (i = 0; i < 11; i ++)
			{
				CFXVector vStart(-200, 0.0f, -200 + (float)(i * 40));
				CFXVector vEnd(200, 0.0f, -200 + (float)(i * 40));

				mGrid.Apply(&vStart);
				mGrid.Apply(&vEnd);

				if (ClipFront(vStart, vEnd, 10.0f))
				{
					Project(&vStart);
					Project(&vEnd);

					if (ClipLeft(vStart, vEnd, 0.0f))
					{
						if (ClipRight(vStart, vEnd, (float)rcMotion.Width()))
						{
							if (ClipTop(vStart, vEnd, 0.0f))
							{
								if (ClipBottom(vStart, vEnd, (float)rcMotion.Height()))
								{
									m_pMemDC->MoveTo((int)vStart.x, (int)vStart.y);
									m_pMemDC->LineTo((int)vEnd.x, (int)vEnd.y);
								}
							}
						}
					}
				}
			}
		}
		break;
	}
	
	int xLast;
	int yLast;

	int xPos;
	int yPos;

	int nCount = 0;
	
	while (pNode)
	{
		// Get the key vector
		
		CFXVector vFinal = pNode->m_Data.m_pos;
		
		// Transform it
		
		m_mView.Apply(&vFinal);

		if (pNode->m_Data.m_bSelected)
		{
			m_pMemDC->SelectObject(&pnRed);
		}
		else
		{
			m_pMemDC->SelectObject(&pnGray);
		}

		if (m_nViewType == VT_PERSPECTIVE)
		{
			// Draw with perspective projection

			if (vFinal.z > 1.0f)
			{
				Project(&vFinal);

				xPos = (int)vFinal.x;
				yPos = (int)vFinal.y;

				if (m_bShowMarkers)
				{
					m_pMemDC->MoveTo(xPos, yPos - 5);
					m_pMemDC->LineTo(xPos, yPos + 5);

					m_pMemDC->MoveTo(xPos - 5, yPos);
					m_pMemDC->LineTo(xPos + 5, yPos);
				}
			}
		}
		else
		{			
			// Draw with parallel projection

			xPos = sx + (int)vFinal.x;
			yPos = sy - (int)vFinal.y;

			if (m_bShowMarkers)
			{
				m_pMemDC->MoveTo(xPos, yPos - 4);
				m_pMemDC->LineTo(xPos, yPos + 5);

				m_pMemDC->MoveTo(xPos - 4, yPos);
				m_pMemDC->LineTo(xPos + 5, yPos);
			}
		}

		if ((m_bShowPaths) && (pNode->m_pPrev))
		{
			// Draw a line back

			m_pMemDC->SelectObject(&pnDash);
			m_pMemDC->MoveTo(xPos, yPos);
			m_pMemDC->LineTo(xLast, yLast);
		}

		if (m_bShowNumbers)
		{
			char sTmp[256];
			sprintf(sTmp, "%d", nCount ++);
			if (pNode->m_Data.m_bSelected)
			{
				m_pMemDC->SetTextColor(RGB(255, 0, 0));
			}
			else
			{
				m_pMemDC->SetTextColor(RGB(200, 200, 200));
			}
			m_pMemDC->TextOut(xPos, yPos, sTmp);
		}
						
		xLast = xPos;
		yLast = yPos;

		pNode = pNode->m_pNext;
	}

	// Draw the view type

	m_pMemDC->SetTextColor(RGB(255, 255, 0));
	m_pMemDC->TextOut(5, 5, m_sViewType);

	// Draw the track rectangle

	if (m_bShowTrack)
	{
		m_pMemDC->SelectObject(&pnDot);
		m_pMemDC->MoveTo(m_rcTrack.left, m_rcTrack.top);
		m_pMemDC->LineTo(m_rcTrack.right, m_rcTrack.top);
		m_pMemDC->LineTo(m_rcTrack.right, m_rcTrack.bottom);
		m_pMemDC->LineTo(m_rcTrack.left, m_rcTrack.bottom);
		m_pMemDC->LineTo(m_rcTrack.left, m_rcTrack.top);
	}

	m_pMemDC->SelectObject(ppnOld);

	// Blit the offscreen

	pDC->BitBlt(rcMotion.left, rcMotion.top, m_sx, m_sy, m_pMemDC, 0, 0, SRCCOPY);	

	// Release the motion control's dc

	ReleaseDC(pDC);
}

//------------------------------------------------------------------
//
//   FUNCTION : SetViewType()
//
//   PURPOSE  : Sets the view type
//
//------------------------------------------------------------------

void CMotionKeysDlg::SetViewType(int nViewType)
{
	CFXVector vRight;
	CFXVector vUp;
	CFXVector vForward;

	// Identity please

	m_mView.Identity();

	switch (nViewType)
	{	
		case VT_BACK :
		{
			strcpy(m_sViewType, "View X / Y [Front]");
			
			vRight	 = CFXVector(1.0f, 0.0f, 0.0f);
			vUp		 = CFXVector(0.0f, 1.0f, 0.0f);
			vForward = CFXVector(0.0f, 0.0f, 1.0f);
		}
		break;

		case VT_TOP :
		{
			strcpy(m_sViewType, "View X / Z [Top]");

			vRight	 = CFXVector(1.0f, 0.0f, 0.0f);
			vUp		 = CFXVector(0.0f, 0.0f, 1.0f);
			vForward = CFXVector(0.0f, -1.0f, 0.0f);
		}
		break;

		case VT_RIGHT :
		{
			strcpy(m_sViewType, "View Y / Z [Right]");

			vRight	 = CFXVector(0.0f, 0.0f, 1.0f);
			vUp		 = CFXVector(0.0f, 1.0f, 0.0f);
			vForward = CFXVector(-1.0f, 0.0f, 0.0f);
		}
		break;

		case VT_PERSPECTIVE :
		{
			strcpy(m_sViewType, "View Y / Z [Persp]");

			m_mView.MakeRotation(m_xRot, m_yRot, 0.0f);
			m_mView.m_Elem[11] = m_fzDist;
		}
		break;
	}

	if (nViewType == VT_PERSPECTIVE)
	{
		GetDlgItem(IDC_MOVEKEY)->EnableWindow(FALSE);
		GetDlgItem(IDC_CREATEKEY)->EnableWindow(FALSE);
		GetDlgItem(IDC_DELETESINGLEKEY)->EnableWindow(FALSE);
		GetDlgItem(IDC_SELECTKEY)->EnableWindow(FALSE);
		GetDlgItem(IDC_SPLIT)->EnableWindow(FALSE);
		GetDlgItem(IDC_ROTATE)->EnableWindow(FALSE);
	}
	else
	{
		GetDlgItem(IDC_MOVEKEY)->EnableWindow(TRUE);
		GetDlgItem(IDC_CREATEKEY)->EnableWindow(TRUE);
		GetDlgItem(IDC_DELETESINGLEKEY)->EnableWindow(TRUE);
		GetDlgItem(IDC_SELECTKEY)->EnableWindow(TRUE);
		GetDlgItem(IDC_SPLIT)->EnableWindow(TRUE);
		GetDlgItem(IDC_ROTATE)->EnableWindow(TRUE);
	}

	m_nViewType = nViewType;

	// Set up the transform through the axis vectors

	if (m_nViewType != VT_PERSPECTIVE) m_mView.SetVectors(vRight, vUp, vForward);

	DrawView();
}

//------------------------------------------------------------------
//
//   FUNCTION : OnRButtonDown()
//
//   PURPOSE  : Handles WM_RBUTTONDOWN
//
//------------------------------------------------------------------

void CMotionKeysDlg::OnRButtonDown(UINT nFlags, CPoint point) 
{
	CRect rcMotion;
	GetDlgItem(IDC_MOTION)->GetWindowRect(&rcMotion);
	ScreenToClient(&rcMotion);

	// See if we right clicked inside the motion view

	CSize szText = m_pMemDC->GetTextExtent(m_sViewType);
	szText.cx += 5;
	szText.cy += 5;
	CRect rcView(rcMotion.left, rcMotion.top, rcMotion.left + szText.cx, rcMotion.top + szText.cy);
	
	if (rcView.PtInRect(point))
	{
		ClientToScreen(&point);
		CMenu rbMenu;
		rbMenu.CreatePopupMenu();

		rbMenu.AppendMenu(MF_STRING, 10000, "&View Perspective");		
		rbMenu.AppendMenu(MF_STRING, 10001, "&View X / Y [Front]");
		rbMenu.AppendMenu(MF_STRING, 10002, "&View X / Z [Right]");
		rbMenu.AppendMenu(MF_STRING, 10003, "&View Z / Y [Top]");

		rbMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);
	}
	
	CDialog::OnRButtonDown(nFlags, point);
}

//------------------------------------------------------------------
//
//   FUNCTION : OnCommand()
//
//   PURPOSE  : Handles WM_COMMAND
//
//------------------------------------------------------------------

BOOL CMotionKeysDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	WORD wID = LOWORD(wParam);

	switch (wID)
	{
		case 10000 : SetViewType(VT_PERSPECTIVE); break;
		case 10001 : SetViewType(VT_BACK); break;
		case 10002 : SetViewType(VT_RIGHT); break;
		case 10003 : SetViewType(VT_TOP); break;
	}
		
	return CDialog::OnCommand(wParam, lParam);
}

//------------------------------------------------------------------
//
//   FUNCTION : Project()
//
//   PURPOSE  : Projects a point
//
//------------------------------------------------------------------

void CMotionKeysDlg::Project(CFXVector *pVec)
{
	CRect rcMotion;
	GetDlgItem(IDC_MOTION)->GetClientRect(&rcMotion);

	float sx = (float)(rcMotion.Width() / 2);
	float sy = (float)(rcMotion.Height() / 2);
	
	pVec->x = sx + ((pVec->x * sx) / pVec->z);
	pVec->y = sy - ((pVec->y * sx) / pVec->z);
}

//------------------------------------------------------------------
//
//   FUNCTION : OnMouseMove()
//
//   PURPOSE  : Handles WM_MOUSEMOVE
//
//------------------------------------------------------------------

void CMotionKeysDlg::OnMouseMove(UINT nFlags, CPoint point) 
{
	CRect rcMotion;
	GetDlgItem(IDC_MOTION)->GetWindowRect(&rcMotion);
	ScreenToClient(&rcMotion);

	int sx = rcMotion.Width() / 2;
	int sy = rcMotion.Height() / 2;

	BOOL bRightMouse = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) ? TRUE : FALSE;
	BOOL bLeftMouse  = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) ? TRUE : FALSE;
	
	if ((bRightMouse) && (rcMotion.PtInRect(point)) && (m_nViewType == VT_PERSPECTIVE))
	{
		TrackPerspectiveView(point);
	}	
	
	CDialog::OnMouseMove(nFlags, point);
}

//------------------------------------------------------------------
//
//   FUNCTION : TrackPerspectiveView()
//
//   PURPOSE  : Handles tracking on the perspective view
//
//------------------------------------------------------------------

void CMotionKeysDlg::TrackPerspectiveView(CPoint ptAnchor)
{
	CRect rcMotion;
	GetDlgItem(IDC_MOTION)->GetWindowRect(&rcMotion);
	ScreenToClient(&rcMotion);

	int sx = rcMotion.Width() / 2;
	int sy = rcMotion.Height() / 2;

	while (GetAsyncKeyState(VK_RBUTTON) & 0x8000)
	{
		CPoint ptRot;
		GetCursorPos(&ptRot);
		ScreenToClient(&ptRot);
		
		ptRot.x -= rcMotion.left;
		ptRot.y -= rcMotion.top;

		m_xRot += (float)(ptRot.y - sy) / 200000.0f;
		m_yRot += (float)(-(ptRot.x - sx)) / 200000.0f;

		m_mView.MakeRotation(m_xRot, m_yRot, 0.0f);
		m_mView.m_Elem[11] = m_fzDist;

		// Redraw the view
		
		DrawView();
	}
}

//------------------------------------------------------------------
//
//   FUNCTION : OnShowmarkers()
//
//   PURPOSE  : Toggles markers
//
//------------------------------------------------------------------

void CMotionKeysDlg::OnShowmarkers() 
{
	m_bShowMarkers = !m_bShowMarkers;
	
	DrawView();	
}

//------------------------------------------------------------------
//
//   FUNCTION : OnShownumbers()
//
//   PURPOSE  : Toggles numbers
//
//------------------------------------------------------------------

void CMotionKeysDlg::OnShownumbers() 
{
	m_bShowNumbers = !m_bShowNumbers;
	
	DrawView();		
}

//------------------------------------------------------------------
//
//   FUNCTION : OnShowpaths()
//
//   PURPOSE  : Toggles paths
//
//------------------------------------------------------------------

void CMotionKeysDlg::OnShowpaths() 
{
	m_bShowPaths = !m_bShowPaths;
	
	DrawView();	
}

//------------------------------------------------------------------
//
//   FUNCTION : OnDeleteAll()
//
//   PURPOSE  : Deletes all keys frames
//
//------------------------------------------------------------------

void CMotionKeysDlg::OnDeleteall() 
{
	int ret = AfxMessageBox("Are you sure ?", MB_YESNO);
	
	if (ret == IDYES)
	{
		m_collKeys.RemoveAll();

		DrawView();
	}	
}

//------------------------------------------------------------------
//
//   FUNCTION : OnMirrorx()
//
//   PURPOSE  : Mirrors all points around the x axis
//
//------------------------------------------------------------------

void CMotionKeysDlg::OnMirrorx() 
{
	CLinkListNode<MOVEKEY> *pNode = m_collKeys.GetHead();

	while (pNode)
	{
		float x = pNode->m_Data.m_pos.x;
		float y = pNode->m_Data.m_pos.y;
		float z = pNode->m_Data.m_pos.z;

		pNode->m_Data.m_pos.y = -z;
		pNode->m_Data.m_pos.z = y;
		
		pNode = pNode->m_pNext;
	}

	DrawView();
}

//------------------------------------------------------------------
//
//   FUNCTION : OnMirrory()
//
//   PURPOSE  : Mirrors all points around the y axis
//
//------------------------------------------------------------------

void CMotionKeysDlg::OnMirrory() 
{
	CLinkListNode<MOVEKEY> *pNode = m_collKeys.GetHead();

	while (pNode)
	{
		float x = pNode->m_Data.m_pos.x;
		float y = pNode->m_Data.m_pos.y;
		float z = pNode->m_Data.m_pos.z;

		pNode->m_Data.m_pos.x = -z;
		pNode->m_Data.m_pos.z = x;
		
		pNode = pNode->m_pNext;
	}

	DrawView();
}

//------------------------------------------------------------------
//
//   FUNCTION : OnMirrorz()
//
//   PURPOSE  : Mirrors all points around the z axis
//
//------------------------------------------------------------------

void CMotionKeysDlg::OnMirrorz() 
{
	CLinkListNode<MOVEKEY> *pNode = m_collKeys.GetHead();

	while (pNode)
	{
		float x = pNode->m_Data.m_pos.x;
		float y = pNode->m_Data.m_pos.y;
		float z = pNode->m_Data.m_pos.z;

		pNode->m_Data.m_pos.x = -y;
		pNode->m_Data.m_pos.y = x;
		
		pNode = pNode->m_pNext;
	}

	DrawView();
}

//------------------------------------------------------------------
//
//   FUNCTION : GetKeyByPos()
//
//   PURPOSE  : Returns the key under the clicked point (if any)
//
//------------------------------------------------------------------

CLinkListNode<MOVEKEY>* CMotionKeysDlg::GetKeyByPos(CPoint ptClick)
{
	CLinkListNode<MOVEKEY> *pNode = m_collKeys.GetHead();

	while (pNode)
	{
		CFXVector vPt = pNode->m_Data.m_pos;

		m_mView.Apply(&vPt);

		CRect rcCheck((int)vPt.x - 4, (int)vPt.y - 4, (int)vPt.x + 4, (int)vPt.y + 4);

		if (rcCheck.PtInRect(ptClick)) return pNode;
		
		pNode = pNode->m_pNext;
	}

	// Failure !!!

	return NULL;
}

//------------------------------------------------------------------
//
//   FUNCTION : OnCreateKey()
//
//   PURPOSE  : Sets current mode to key creation
//
//------------------------------------------------------------------

void CMotionKeysDlg::OnCreateKey() 
{
	SetCurrentMode(CM_CREATE);
}

//------------------------------------------------------------------
//
//   FUNCTION : OnDeleteSingleKey()
//
//   PURPOSE  : Sets current mode to key deletion
//
//------------------------------------------------------------------

void CMotionKeysDlg::OnDeleteSingleKey() 
{
	SetCurrentMode(CM_DELETE);
}

//------------------------------------------------------------------
//
//   FUNCTION : OnMoveKey()
//
//   PURPOSE  : Sets current mode to key movement
//
//------------------------------------------------------------------

void CMotionKeysDlg::OnMoveKey() 
{
	SetCurrentMode(CM_MOVE);
}

//------------------------------------------------------------------
//
//   FUNCTION : OnSelectKey()
//
//   PURPOSE  : Sets current mode to key selection
//
//------------------------------------------------------------------

void CMotionKeysDlg::OnSelectKey() 
{
	SetCurrentMode(CM_SELECT);
}

//------------------------------------------------------------------
//
//   FUNCTION : OnSplit()
//
//   PURPOSE  : Sets current mode to splitting
//
//------------------------------------------------------------------

void CMotionKeysDlg::OnSplit() 
{
	SetCurrentMode(CM_SPLIT);
}

//------------------------------------------------------------------
//
//   FUNCTION : OnRotate()
//
//   PURPOSE  : Sets mode to rotation
//
//------------------------------------------------------------------

void CMotionKeysDlg::OnRotate() 
{
	SetCurrentMode(CM_ROTATE);
}

//------------------------------------------------------------------
//
//   FUNCTION : OnScale()
//
//   PURPOSE  : Sets mode to scale
//
//------------------------------------------------------------------

void CMotionKeysDlg::OnScale() 
{
	SetCurrentMode(CM_SCALE);	
}

//------------------------------------------------------------------
//
//   FUNCTION : SetCurrentMode()
//
//   PURPOSE  : Sets the current mode
//
//------------------------------------------------------------------

void CMotionKeysDlg::SetCurrentMode(int nNewMode)
{
	m_nCurMode = nNewMode;

	((CButton *)GetDlgItem(IDC_CREATEKEY))->SetState(FALSE);
	((CButton *)GetDlgItem(IDC_DELETESINGLEKEY))->SetState(FALSE);
	((CButton *)GetDlgItem(IDC_MOVEKEY))->SetState(FALSE);
	((CButton *)GetDlgItem(IDC_SELECTKEY))->SetState(FALSE);
	((CButton *)GetDlgItem(IDC_SPLIT))->SetState(FALSE);
	((CButton *)GetDlgItem(IDC_ROTATE))->SetState(FALSE);
	((CButton *)GetDlgItem(IDC_SCALE))->SetState(FALSE);

	switch (m_nCurMode)
	{
		case CM_CREATE : ((CButton *)GetDlgItem(IDC_CREATEKEY))->SetState(TRUE); break;
		case CM_DELETE : ((CButton *)GetDlgItem(IDC_DELETESINGLEKEY))->SetState(TRUE); break;
		case CM_MOVE   : ((CButton *)GetDlgItem(IDC_MOVEKEY))->SetState(TRUE); break;
		case CM_SELECT : ((CButton *)GetDlgItem(IDC_SELECTKEY))->SetState(TRUE); break;
		case CM_SPLIT  : ((CButton *)GetDlgItem(IDC_SPLIT))->SetState(TRUE); break;
		case CM_ROTATE : ((CButton *)GetDlgItem(IDC_ROTATE))->SetState(TRUE); break;
		case CM_SCALE  : ((CButton *)GetDlgItem(IDC_SCALE))->SetState(TRUE); break;
	}

	SetFocus();
}

//------------------------------------------------------------------
//
//   FUNCTION : OnSelectAll()
//
//   PURPOSE  : Selects all keys
//
//------------------------------------------------------------------

void CMotionKeysDlg::OnSelectAll() 
{
	CLinkListNode<MOVEKEY> *pNode = m_collKeys.GetHead();

	while (pNode)
	{
		pNode->m_Data.m_bSelected = TRUE;
		
		pNode = pNode->m_pNext;
	}

	DrawView();
}

//------------------------------------------------------------------
//
//   FUNCTION : OnUnselectAll()
//
//   PURPOSE  : Unselects all keys
//
//------------------------------------------------------------------

void CMotionKeysDlg::OnUnselectAll() 
{
	CLinkListNode<MOVEKEY> *pNode = m_collKeys.GetHead();

	while (pNode)
	{
		pNode->m_Data.m_bSelected = FALSE;
		
		pNode = pNode->m_pNext;
	}

	DrawView();
}

//------------------------------------------------------------------
//
//   FUNCTION : OnUsePreset()
//
//   PURPOSE  : Toggles the use of preset animations
//
//------------------------------------------------------------------

void CMotionKeysDlg::OnUsePreset() 
{
	m_bUsePreset = ((CButton *)GetDlgItem(IDC_USEPRESET))->GetCheck();
	
	GetDlgItem(IDC_PRESET)->EnableWindow(m_bUsePreset);
	GetDlgItem(IDC_REPS)->EnableWindow(m_bUsePreset);
}

//------------------------------------------------------------------
//
//   FUNCTION : OnAddToMoveFavourites()
//
//   PURPOSE  : Adds the current keys to the motion favourites
//
//------------------------------------------------------------------

void CMotionKeysDlg::OnAddToMoveFavourites() 
{
	CSpellEdApp *pApp = (CSpellEdApp *)AfxGetApp();

	CStringDlg dlg("Choose Name");

	if (dlg.DoModal() == IDOK)
	{
		MK_FAVOURITE *pNewFavourite = new MK_FAVOURITE;

		pNewFavourite->m_sName = dlg.m_sText;

		pNewFavourite->m_collKeys.CopyList(&m_collKeys);

		pApp->GetMoveFavourites()->AddTail(pNewFavourite);
	}

	GetDlgItem(IDC_CHOOSEMOVEFAVOURITE)->EnableWindow(pApp->GetMoveFavourites()->GetSize() ? TRUE : FALSE);
}

//------------------------------------------------------------------
//
//   FUNCTION : OnChooseMoveFavourite()
//
//   PURPOSE  : Chooses a favourite motion animation
//
//------------------------------------------------------------------

void CMotionKeysDlg::OnChooseMoveFavourite() 
{
	CChooseMotionAnimDlg dlg;

	if ((dlg.DoModal() == IDOK) && (dlg.m_pFav))
	{
		m_collKeys.RemoveAll();

		MK_FAVOURITE *pFav = dlg.m_pFav;

		CLinkListNode<MOVEKEY> *pNode = pFav->m_collKeys.GetHead();

		while (pNode)
		{
			m_collKeys.AddTail(pNode->m_Data);

			pNode = pNode->m_pNext;
		}

		DrawView();
	}
}
