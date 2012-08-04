// KeyframeWnd.cpp : implementation file
//

#include "precompile.h"
#include "modeledit.h"
#include "modeleditdlg.h"
#include "keyframewnd.h"
#include "keyframetimedlg.h"
#include "ltbasetypes.h"
#include "keyframestringdlg.h"
#include "model_ops.h"
#include "timeoffsetdlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CKeyframeWnd

CKeyframeWnd::CKeyframeWnd()
{
	m_pModelAnim = NULL;
	m_nCurrentTime = 0;
	m_crKeyframe = RGB(255,0,0);
	m_crKeyframeString = RGB(0,255,64);
	m_crTaggedKeyframe = RGB(0,0,255);
	m_crTaggedKeyframeString = RGB(0,128,128);
	m_bTracking = FALSE;
	m_bSelectionBox = FALSE;
	m_bActive = FALSE;
	m_pModelAnim = NULL;
}

CKeyframeWnd::~CKeyframeWnd()
{
}


BEGIN_MESSAGE_MAP(CKeyframeWnd, CWnd)
	//{{AFX_MSG_MAP(CKeyframeWnd)
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_COMMAND(ID_CLIP_AFTER, OnClipAfter)
	ON_COMMAND(ID_CLIP_BEFORE, OnClipBefore)
	ON_COMMAND(ID_TAG_KEYFRAME, OnTagKeyframe)
	ON_COMMAND(ID_UNTAG_KEYFRAME, OnUntagKeyframe)
	ON_COMMAND(ID_EDIT_KEYFRAME_TIME, OnEditKeyframeTime)
	ON_COMMAND(ID_DELETE_KEYFRAME, OnDeleteKeyframe)
	ON_COMMAND(ID_DELETE_TAGGED_KEYFRAMES, OnDeleteTaggedKeyframes)
	ON_COMMAND(ID_EDITKEYFRAMESTRING, OnEditTaggedKeyframeString)
	ON_COMMAND(ID_TAGALL, OnTagAll )
	ON_COMMAND(ID_UNTAGALL, OnUnTagAll )
	ON_COMMAND(ID_INSERT_TIME_DELAY, OnInsertTimeDelay)
	ON_WM_LBUTTONDBLCLK()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CKeyframeWnd operations

BOOL CKeyframeWnd::InitTaggedArray()
{
	DWORD dwSize;

	dwSize = m_pModelAnim ? m_pModelAnim->NumKeyframes() : 0;
	return m_Tagged.SetSizeInit2(dwSize, FALSE);
}

void CKeyframeWnd::SetAnim (ModelAnim* pAnim)
{
	m_pModelAnim = pAnim;


	if(!m_hWnd)
		return;


	InitTaggedArray();
	
	m_nCurrentTime = 0;
	DWORD nKeyframes = m_pModelAnim ? m_pModelAnim->NumKeyFrames() : 0;

	// create new x value array

	m_XVal.SetSize(nKeyframes);

	// erase keyframe rect and markers

	m_dcOffscreen.Rectangle (m_rcKeyframes);
	m_dcOffscreen.FillRect (m_rcTopMarker, &m_brBackground);
	m_dcOffscreen.FillRect (m_rcBottomMarker, &m_brBackground);
	
	// reset the marker rects

	m_rcTopMarker.OffsetRect (-m_rcTopMarker.left, 0);
	m_rcBottomMarker.OffsetRect (-m_rcBottomMarker.left, 0);
	m_dcOffscreen.BitBlt (m_rcTopMarker.left, m_rcTopMarker.top, m_rcTopMarker.Width(), m_rcTopMarker.Height(), &m_dcTopMarker, 0, 0, SRCCOPY);
	m_dcOffscreen.BitBlt (m_rcBottomMarker.left, m_rcBottomMarker.top, m_rcBottomMarker.Width(), m_rcBottomMarker.Height(), &m_dcBottomMarker, 0, 0, SRCCOPY);

	// draw the keyframe markers

	CPen penKeyframe (PS_SOLID, 1, m_crKeyframe);
	CPen penKeyframeString (PS_SOLID, 1, m_crKeyframeString);
	CPen penTaggedKeyframe (PS_SOLID, 1, m_crTaggedKeyframe);
	CPen penTaggedKeyframeString (PS_SOLID, 1, m_crTaggedKeyframeString);
	CPen* pOldPen = m_dcOffscreen.SelectObject (&penKeyframe);

	DWORD nTotalTime = m_pModelAnim ? m_pModelAnim->GetAnimTime() : 0;

	//the last location we drew each type, we don't want to draw red on top of green, and
	//we don't want to draw green on top of blue
	uint32 nLastStringX = 0xFFFFFFFF;
	uint32 nLastSelX = 0xFFFFFFFF;

	if (nTotalTime > 0)
	{
		for (DWORD i = 0; i < nKeyframes; i++)
		{
			float x = ((float)m_pModelAnim->m_KeyFrames[i].m_Time * (float)(m_rcKeyframes.Width() - 3)) / (float)nTotalTime;
			uint32 nXVal = m_rcKeyframes.left + 1 + (int)x;
			m_XVal[i] = nXVal;

			//if we have already drawn a selection on this place, just skip
			if(nLastSelX == nXVal)
				continue;
			
			//draw a tag mark, unless we already have at this location
			if (m_Tagged[i])
			{
				if( m_pModelAnim->m_KeyFrames[i].m_pString[0] == '\0' )
					m_dcOffscreen.SelectObject (&penTaggedKeyframe);
				else
					m_dcOffscreen.SelectObject (&penTaggedKeyframeString);

				nLastSelX = nXVal;
			}
			//draw a normal selection marker, but only if we haven't already drawn a string
			else
			{				
				if( (m_pModelAnim->m_KeyFrames[i].m_pString[0] == '\0') && (nXVal != nLastStringX) )
				{
					m_dcOffscreen.SelectObject (&penKeyframe);
				}
				else
				{
					m_dcOffscreen.SelectObject (&penKeyframeString);
					nLastStringX = nXVal;
				}
			}

			m_dcOffscreen.MoveTo (nXVal, m_rcKeyframes.top + 1);
			m_dcOffscreen.LineTo (nXVal, m_rcKeyframes.bottom - 1);
		}
	}

	m_dcOffscreen.SelectObject (pOldPen);
	penKeyframe.DeleteObject();
	penKeyframeString.DeleteObject();
	penTaggedKeyframe.DeleteObject();
	penTaggedKeyframeString.DeleteObject();

	Invalidate (FALSE);
}

//this function will verify that this operation can be performed given the current animation
//settings, and will inform the user and return false if it cannot
bool CKeyframeWnd::CanPerformOperation(const char* pszDescription)
{
	//make sure that we have an animation
	if(!m_pModelAnim)
	{
		CString sMessage;
		sMessage.Format("Error performing operation:%s\nNo animation is currently selected in this track", pszDescription);
		MessageBox(sMessage, "Error performing operation", MB_OK | MB_ICONEXCLAMATION);
		return false;
	}

	if(((CModelEditDlg*)GetParent())->GetModel() != m_pModelAnim->GetModel())
	{
		CString sMessage;
		sMessage.Format("Error performing operation:%s\nYou cannot modify child model animations", pszDescription);
		MessageBox(sMessage, "Error performing operation", MB_OK | MB_ICONEXCLAMATION);
		return false;
	}

	//success
	return true;
}

void CKeyframeWnd::SetTime (DWORD nCurrentTime)
{
	m_nCurrentTime = nCurrentTime;

	if(!m_pModelAnim)
		return;

	// get new x location of marker
	
	DWORD nTotalTime = m_pModelAnim->GetAnimTime();
	float x = ((float)m_nCurrentTime * (float)(m_rcKeyframes.Width() - 3)) / (float)nTotalTime;
	int nNewTopX = (int)x;
	
	if (nNewTopX == m_rcTopMarker.left)
	{
		// same place - just return
		return;
	}

	int nNewBottomX = (int)x;

	// get the new rects

	CRect rcOldTop = m_rcTopMarker;
	CRect rcOldBottom = m_rcBottomMarker;
	m_rcTopMarker.OffsetRect (nNewTopX - m_rcTopMarker.left, 0);
	m_rcBottomMarker.OffsetRect (nNewTopX - m_rcBottomMarker.left, 0);

	// get the union of old and new rects

	CRect rcUnionTop;
	rcUnionTop.UnionRect (rcOldTop, m_rcTopMarker);
	CRect rcUnionBottom;
	rcUnionBottom.UnionRect (rcOldBottom, m_rcBottomMarker);

	// update keyframe bar

	m_dcOffscreen.FillRect (rcOldTop, &m_brBackground);
	m_dcOffscreen.FillRect (rcOldBottom, &m_brBackground);
	m_dcOffscreen.BitBlt (m_rcTopMarker.left, m_rcTopMarker.top, m_rcTopMarker.Width(), m_rcTopMarker.Height(), &m_dcTopMarker, 0, 0, SRCCOPY);
	m_dcOffscreen.BitBlt (m_rcBottomMarker.left, m_rcBottomMarker.top, m_rcBottomMarker.Width(), m_rcBottomMarker.Height(), &m_dcBottomMarker, 0, 0, SRCCOPY);

	CDC* pDC = GetDC();
	pDC->BitBlt (rcUnionTop.left, rcUnionTop.top, rcUnionTop.Width(), rcUnionTop.Height(), &m_dcOffscreen, rcUnionTop.left, rcUnionTop.top, SRCCOPY);
	pDC->BitBlt (rcUnionBottom.left, rcUnionBottom.top, rcUnionBottom.Width(), rcUnionBottom.Height(), &m_dcOffscreen, rcUnionBottom.left, rcUnionBottom.top, SRCCOPY);
	ReleaseDC (pDC);
}

void CKeyframeWnd::Redraw( )
{
	if( !m_pModelAnim )
		return;

	DWORD nKeyframes = m_pModelAnim->m_KeyFrames;

	// erase keyframe rect and markers

	m_dcOffscreen.Rectangle (m_rcKeyframes);
	m_dcOffscreen.FillRect (m_rcTopMarker, &m_brBackground);
	m_dcOffscreen.FillRect (m_rcBottomMarker, &m_brBackground);
	
	m_dcOffscreen.BitBlt (m_rcTopMarker.left, m_rcTopMarker.top, m_rcTopMarker.Width(), m_rcTopMarker.Height(), &m_dcTopMarker, 0, 0, SRCCOPY);
	m_dcOffscreen.BitBlt (m_rcBottomMarker.left, m_rcBottomMarker.top, m_rcBottomMarker.Width(), m_rcBottomMarker.Height(), &m_dcBottomMarker, 0, 0, SRCCOPY);

	// draw the keyframe markers

	CPen penKeyframe (PS_SOLID, 1, m_crKeyframe);
	CPen penKeyframeString (PS_SOLID, 1, m_crKeyframeString);
	CPen penTaggedKeyframe (PS_SOLID, 1, m_crTaggedKeyframe);
	CPen penTaggedKeyframeString (PS_SOLID, 1, m_crTaggedKeyframeString);
	CPen* pOldPen = m_dcOffscreen.SelectObject (&penKeyframe);

	DWORD nTotalTime = m_pModelAnim->GetAnimTime();
	if (nTotalTime > 0)
	{
		for (DWORD i = 0; i < nKeyframes; i++)
		{
			float x = ((float)m_pModelAnim->m_KeyFrames[i].m_Time * (float)(m_rcKeyframes.Width() - 3)) / (float)nTotalTime;
			m_XVal[i] = m_rcKeyframes.left + 1 + (int)x;
			
			if (m_Tagged[i])
			{
				if( m_pModelAnim->m_KeyFrames[i].m_pString[0] == '\0' )
					m_dcOffscreen.SelectObject (&penTaggedKeyframe);
				else
					m_dcOffscreen.SelectObject (&penTaggedKeyframeString);
			}
			else
			{
				if( m_pModelAnim->m_KeyFrames[i].m_pString[0] == '\0' )
					m_dcOffscreen.SelectObject (&penKeyframe);
				else
					m_dcOffscreen.SelectObject (&penKeyframeString);
			}

			m_dcOffscreen.MoveTo (m_XVal[i], m_rcKeyframes.top + 1);
			m_dcOffscreen.LineTo (m_XVal[i], m_rcKeyframes.bottom - 1);
		}
	}

	m_dcOffscreen.SelectObject (pOldPen);
	penKeyframe.DeleteObject();
	penKeyframeString.DeleteObject();
	penTaggedKeyframe.DeleteObject();
	penTaggedKeyframeString.DeleteObject();

	Invalidate (FALSE);
}

DWORD CKeyframeWnd::ForceNearestKeyframe()
{
	DWORD nClosestKeyframe = 0;

	if( !m_pModelAnim )
		return 0;

	if(m_pModelAnim->m_KeyFrames.GetSize() <= 1)
	{
		nClosestKeyframe = 0;
	}
	else
	{
		// see if we're already on a keyframe
		DWORD nKeyframes = m_pModelAnim->m_KeyFrames;
		int nXVal = m_rcTopMarker.left + (m_rcTopMarker.Width() >> 1);
		for (DWORD i = 0; i < nKeyframes; i++)
		{
			if (nXVal == m_XVal[i])
			{
				return i;
			}

			if (m_XVal[i] > nXVal)
			{
				break;
			}
		}

		// adjust the pointer and return the number of the nearest keyframe

		DWORD nKeyframe1 = i - 1;
		DWORD nKeyframe2 = i;
		if(i == 0)
		{
			nKeyframe1 = nKeyframe2 = 0;
		}

		nKeyframe1 = DMIN(nKeyframe1, m_pModelAnim->NumKeyFrames());
		nKeyframe2 = DMIN(nKeyframe2, m_pModelAnim->NumKeyFrames());
		
		float nPercent = (float)(m_nCurrentTime - m_pModelAnim->m_KeyFrames[nKeyframe1].m_Time) / 
						 (float)(m_pModelAnim->m_KeyFrames[nKeyframe2].m_Time - m_pModelAnim->m_KeyFrames[nKeyframe1].m_Time);

		if (nPercent >= 0.5f)
		{
			nClosestKeyframe = nKeyframe2;
		}
		else
		{
			nClosestKeyframe = nKeyframe1;
		}
	}

	m_nCurrentTime = m_pModelAnim->m_KeyFrames[nClosestKeyframe].m_Time;
	SetTime (m_nCurrentTime);
	((CModelEditDlg*)GetParent())->SetCurrentPosition(this, nClosestKeyframe, nClosestKeyframe, 0);

	return nClosestKeyframe;
}


DWORD CKeyframeWnd::GetNearestKeyframe( int x )
{
	if( !m_pModelAnim )
		return 0;

	// see if we're already on a keyframe
	DWORD nKeyframes = m_pModelAnim->m_KeyFrames;
	int nXVal = x;
	for (DWORD i = 0; i < nKeyframes; i++)
	{
		if (nXVal == m_XVal[i])
		{
			return i;
		}

		if (m_XVal[i] > nXVal)
		{
			break;
		}
	}

	// adjust the pointer and return the number of the nearest keyframe

	DWORD nClosestKeyframe = 0;
	DWORD nKeyframe1 = i - 1;
	DWORD nKeyframe2 = i;
	float fPercent = ((float)(nXVal - m_XVal[nKeyframe1])) / ((float)(m_XVal[nKeyframe2] - m_XVal[nKeyframe1]));

	if (fPercent >= 0.5f)
	{
		nClosestKeyframe = nKeyframe2;
	}
	else
	{
		nClosestKeyframe = nKeyframe1;
	}

	return nClosestKeyframe;
}


void CKeyframeWnd::RemoveKeyframe (DWORD nKeyframe)
{
	DWORD nKeyframes;

	if(!m_pModelAnim)
		return;

	nKeyframes = m_pModelAnim->NumKeyFrames();
	if(!m_pModelAnim->RemoveKeyFrame(nKeyframe))
		return;

	m_XVal.Remove(nKeyframe);

	// don't delete tag information from tagged array, just move all down one
	m_Tagged.Remove(nKeyframe);

	m_nCurrentTime = DCLAMP(m_nCurrentTime, (int)0, (int)m_pModelAnim->GetAnimTime());
}


void CKeyframeWnd::MoveKeyframe (DWORD nDst, DWORD nSrc)
{
	ModelAnim *pKeyFrame;
	DWORD nOldKeyFrames;


	nOldKeyFrames = m_pModelAnim->NumKeyFrames();

	// get adjusted destination index
	DWORD nAdjustedDst = nDst;
	if (nDst > nSrc)
	{
		nAdjustedDst--;
	}

	// Do the stuff in the model.
	pKeyFrame = m_pModelAnim->CutKeyFrame(nSrc);
	if(pKeyFrame)
	{
		m_pModelAnim->PasteKeyFrame(pKeyFrame, nAdjustedDst);
		delete pKeyFrame;
	}
	else
	{
		return;
	}	

	// move the values in the tagged array
	BOOL bTagged = m_Tagged[nSrc];

	if(nSrc < nOldKeyFrames - 1)
		memmove (&m_Tagged[nSrc], &m_Tagged[nSrc + 1], sizeof (BOOL) * (nOldKeyFrames - nSrc - 1));

	if(nAdjustedDst < nOldKeyFrames - 1)
		memmove (&m_Tagged[nAdjustedDst + 1], &m_Tagged[nAdjustedDst], sizeof (BOOL) * ((nOldKeyFrames - 1) - nAdjustedDst));

	m_Tagged[nAdjustedDst] = bTagged;
}


////////////////////////////////////////////////////////////////////////////
// CKeyframeWnd message handlers

int CKeyframeWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	CRect rcClient;
	GetClientRect (rcClient);

	CDC* pDC = GetDC();

	// set up the offscreen dc

	m_dcOffscreen.CreateCompatibleDC (pDC);
	m_bmpOffscreen.CreateCompatibleBitmap (pDC, rcClient.Width(), rcClient.Height());
	m_pOldBmpOffscreen = m_dcOffscreen.SelectObject (&m_bmpOffscreen);

	// set up the marker dcs

	m_dcTopMarker.CreateCompatibleDC (pDC);
	m_bmpTopMarker.LoadBitmap (IDB_MARKER_TOP);
	m_pOldBmpTopMarker = m_dcTopMarker.SelectObject (&m_bmpTopMarker);

	m_dcBottomMarker.CreateCompatibleDC (pDC);
	m_bmpBottomMarker.LoadBitmap (IDB_MARKER_BOTTOM);
	m_pOldBmpBottomMarker = m_dcBottomMarker.SelectObject (&m_bmpBottomMarker);
	
	// set up the marker rects

	BITMAP bm;
	m_bmpTopMarker.GetObject (sizeof (BITMAP), &bm);
	m_rcTopMarker.SetRect (0, 0, bm.bmWidth, bm.bmHeight);

	m_bmpBottomMarker.GetObject (sizeof (BITMAP), &bm);
	m_rcBottomMarker.SetRect (0, rcClient.bottom - bm.bmHeight, bm.bmWidth, rcClient.bottom);

	// create the background brush

	m_brBackground.CreateSolidBrush (RGB(192,192,192));

	// set up the keyframe rect
	
	m_rcKeyframes.SetRect ((m_rcTopMarker.Width() >> 1) - 1, m_rcTopMarker.Height(), rcClient.Width() - (m_rcTopMarker.Width() >> 1) + 1, m_rcBottomMarker.top);

	// draw the basic layout

	m_dcOffscreen.FillRect (rcClient, &m_brBackground);
	m_dcOffscreen.Rectangle (m_rcKeyframes);
	m_dcOffscreen.BitBlt (m_rcTopMarker.left, m_rcTopMarker.top, m_rcTopMarker.Width(), m_rcTopMarker.Height(), &m_dcTopMarker, 0, 0, SRCCOPY);
	m_dcOffscreen.BitBlt (m_rcBottomMarker.left, m_rcBottomMarker.top, m_rcBottomMarker.Width(), m_rcBottomMarker.Height(), &m_dcBottomMarker, 0, 0, SRCCOPY);

	ReleaseDC (pDC);

	Invalidate( FALSE );
	
	return 0;
}

void CKeyframeWnd::OnDestroy() 
{
	CWnd::OnDestroy();
	
	m_dcOffscreen.SelectObject (m_pOldBmpOffscreen);
	m_dcOffscreen.DeleteDC();
	m_bmpOffscreen.DeleteObject();

	m_dcTopMarker.SelectObject (m_pOldBmpTopMarker);
	m_dcTopMarker.DeleteDC();
	m_bmpTopMarker.DeleteObject();

	m_dcBottomMarker.SelectObject (m_pOldBmpBottomMarker);
	m_dcBottomMarker.DeleteDC();
	m_bmpBottomMarker.DeleteObject();

	m_brBackground.DeleteObject();

}

void CKeyframeWnd::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	CRect rcClient;
	GetClientRect (rcClient);

	dc.BitBlt (0, 0, rcClient.Width(), rcClient.Height(), &m_dcOffscreen, 0, 0, SRCCOPY);
}

void CKeyframeWnd::OnLButtonDown(UINT nFlags, CPoint point) 
{
	if( m_pModelAnim )
	{
		if (m_rcTopMarker.PtInRect (point) || m_rcBottomMarker.PtInRect (point))
		{
			// start dragging

			m_bTracking = TRUE;
			SetCapture();
			m_ptOld = point;

			GetModelEditDlg()->GetAnimInfo(m_iWnd)->StopPlayback();
		}
		else
		{

			if( GetAsyncKeyState( VK_SHIFT ))
			{
				m_bSelectionBox = TRUE;
				m_ptStartBox = point;
			}
			else
			{
				// just immediately move to new position
				// (do this by tricking mousemove handler)

				m_ptOld.x = m_rcTopMarker.left + (m_rcTopMarker.Width() >> 1);
				m_bTracking = TRUE;
				OnMouseMove (0, point);
				m_bTracking = FALSE;
			}
		}
	}
		
	CWnd::OnLButtonDown(nFlags, point);
}

void CKeyframeWnd::OnLButtonUp(UINT nFlags, CPoint point) 
{
	DWORD nStartKeyframe, nEndKeyframe;
	int nStart, nEnd;

	if (m_bTracking)
	{
		// end dragging

		ReleaseCapture();
		m_bTracking = FALSE;
	}
	else if( m_bSelectionBox )
	{
		// End selection box..

		ReleaseCapture();
		m_bSelectionBox = FALSE;

		// Make sure start is lower than end...
		if( m_ptStartBox.x <= point.x )
		{
			nStart = m_ptStartBox.x;
			nEnd = point.x;
		}
		else
		{
			nStart = point.x;
			nEnd = m_ptStartBox.x;
		}

		// Get the nearest keyframes...
		nStartKeyframe = GetNearestKeyframe( nStart );
		nEndKeyframe = GetNearestKeyframe( nEnd );

		// Choose the keyframes between start and finish...
		if( m_XVal[nStartKeyframe] < nStart )
			nStartKeyframe++;
		if( m_XVal[nEndKeyframe] > nEnd )
			nEndKeyframe--;

		// Redraw the keyframe window...
		Redraw( );

		// Tag away...
		DoTagKeyframes( nStartKeyframe, nEndKeyframe );
	}

	ForceNearestKeyframe();	
	CWnd::OnLButtonUp(nFlags, point);
}

void CKeyframeWnd::OnMouseMove(UINT nFlags, CPoint point) 
{
	if(!m_pModelAnim)
		return;	

	if (m_bTracking)
	{
		CRect rcClient;
		GetClientRect (rcClient);

		// get new positions of markers

		CRect rcOldTop = m_rcTopMarker;
		CRect rcOldBottom = m_rcBottomMarker;

		m_rcTopMarker.OffsetRect (point.x - m_ptOld.x, 0);
		m_rcBottomMarker.OffsetRect (point.x - m_ptOld.x, 0);

		if (m_rcTopMarker.left < 0)
		{
			m_rcTopMarker.OffsetRect (-m_rcTopMarker.left, 0);
		}
		if (m_rcTopMarker.right > rcClient.right - 1)
		{
			m_rcTopMarker.OffsetRect (rcClient.right - m_rcTopMarker.right, 0);
		}

		if (m_rcBottomMarker.left < 0)
		{
			m_rcBottomMarker.OffsetRect (-m_rcBottomMarker.left, 0);
		}
		if (m_rcBottomMarker.right > rcClient.right - 1)
		{
			m_rcBottomMarker.OffsetRect (rcClient.right - m_rcBottomMarker.right, 0);
		}

		if (rcOldTop == m_rcTopMarker && rcOldBottom == m_rcBottomMarker)
		{
			return;
		}

		// update keyframe bar

		CRect rcUnionTop;
		rcUnionTop.UnionRect (rcOldTop, m_rcTopMarker);
		CRect rcUnionBottom;
		rcUnionBottom.UnionRect (rcOldBottom, m_rcBottomMarker);

		m_dcOffscreen.FillRect (rcOldTop, &m_brBackground);
		m_dcOffscreen.FillRect (rcOldBottom, &m_brBackground);
		m_dcOffscreen.BitBlt (m_rcTopMarker.left, m_rcTopMarker.top, m_rcTopMarker.Width(), m_rcTopMarker.Height(), &m_dcTopMarker, 0, 0, SRCCOPY);
		m_dcOffscreen.BitBlt (m_rcBottomMarker.left, m_rcBottomMarker.top, m_rcBottomMarker.Width(), m_rcBottomMarker.Height(), &m_dcBottomMarker, 0, 0, SRCCOPY);

		CDC* pDC = GetDC();
		pDC->BitBlt (rcUnionTop.left, rcUnionTop.top, rcUnionTop.Width(), rcUnionTop.Height(), &m_dcOffscreen, rcUnionTop.left, rcUnionTop.top, SRCCOPY);
		pDC->BitBlt (rcUnionBottom.left, rcUnionBottom.top, rcUnionBottom.Width(), rcUnionBottom.Height(), &m_dcOffscreen, rcUnionBottom.left, rcUnionBottom.top, SRCCOPY);
		ReleaseDC (pDC);

		// get keyframes we're between

		int nXVal = m_rcTopMarker.left + (m_rcTopMarker.Width() >> 1);
		for (DWORD i = 0; i < m_pModelAnim->m_KeyFrames; i++)
		{
			if (m_XVal[i] == nXVal)
			{
				break;
			}
			if (m_XVal[i] > nXVal)
			{
				break;
			}
		}
		
		DWORD nKeyframe1 = 0;
		DWORD nKeyframe2 = 0;
		float nPercent = 0.0f;
		
		if (m_XVal[i] != nXVal)
		{
			// between two keyframes

			nKeyframe1 = i - 1;
			nKeyframe2 = i;
			nPercent = ((float)(nXVal - m_XVal[nKeyframe1])) / ((float)(m_XVal[nKeyframe2] - m_XVal[nKeyframe1]));
			m_nCurrentTime = m_pModelAnim->m_KeyFrames[nKeyframe1].m_Time + 
							 (DWORD)(nPercent * (float)(m_pModelAnim->m_KeyFrames[nKeyframe2].m_Time - m_pModelAnim->m_KeyFrames[nKeyframe1].m_Time));
		}
		else
		{
			// right on a keyframe
			nKeyframe1 = i;
			nKeyframe2 = i;
			nPercent = 0.0f;
			m_nCurrentTime = m_pModelAnim->m_KeyFrames[nKeyframe1].m_Time;
		}

		GetModelEditDlg()->SetCurrentPosition (this, nKeyframe1, nKeyframe2, nPercent);

		m_ptOld = point;
	}
	else if( m_bSelectionBox )
	{
		CRect rcSelectionBox;
		CPoint ptClientPoint;

		rcSelectionBox = m_rcKeyframes;
		rcSelectionBox.left = m_ptStartBox.x;
		rcSelectionBox.right = point.x;
		Redraw( );
		m_dcOffscreen.InvertRect( &rcSelectionBox );
		Invalidate( FALSE );
	}
	
	CWnd::OnMouseMove(nFlags, point);
}

void CKeyframeWnd::OnRButtonDown(UINT nFlags, CPoint point) 
{
	ClientToScreen (&point);

	CMenu menu;
	menu.LoadMenu (IDR_KEYFRAME_MENU);
	CMenu* pMenu = menu.GetSubMenu (0);
	pMenu->TrackPopupMenu (TPM_LEFTALIGN | TPM_LEFTBUTTON, point.x, point.y, this, NULL);
	
	CWnd::OnRButtonDown(nFlags, point);
}

void CKeyframeWnd::OnClipAfter() 
{
	if(!CanPerformOperation("Clip After Keyframe"))
		return;

	DWORD nKeyframes = m_pModelAnim->m_KeyFrames;
	DWORD nKeyframe = ForceNearestKeyframe();
	if (nKeyframe == nKeyframes) return;
	
	if (nKeyframe == nKeyframes - 1)
	{
		MessageBox ("Cannot clip past the last keyframe", "Clip Animation", MB_OK | MB_ICONINFORMATION);
		return;
	}

	int nResponse = MessageBox ("Are you sure you want to clip the animation past the current keyframe?", "Confirm Clip", MB_YESNO | MB_ICONQUESTION);
	if (nResponse == IDNO)
	{
		return;
	}

	// remove all keyframes before the current position

	for (long i = nKeyframes - 1; i > (long)nKeyframe; i--)
	{
		RemoveKeyframe ((DWORD)i);
	}

	// notify selves and parent dialog of the changes

	DWORD nTime = m_nCurrentTime;
	SetAnim (m_pModelAnim);
	SetTime (nTime);
	GetModelEditDlg()->SetCurrentPosition (this, nKeyframe, nKeyframe, 0.0f);
	GetModelEditDlg()->SetChangesMade();
}

void CKeyframeWnd::OnClipBefore() 
{
	if(!CanPerformOperation("Clip Before Keyframe"))
		return;

	DWORD nKeyframes = m_pModelAnim->m_KeyFrames;
	DWORD nKeyframe = ForceNearestKeyframe();
	if (nKeyframe == nKeyframes) return;
	
	if (nKeyframe == 0)
	{
		MessageBox ("Cannot clip before the first keyframe", "Clip Animation", MB_OK | MB_ICONINFORMATION);
		return;
	}
	
	int nResponse = MessageBox ("Are you sure you want to clip the animation before the current keyframe?", "Confirm Clip", MB_YESNO | MB_ICONQUESTION);
	if (nResponse == IDNO)
	{
		return;
	}

	// remove all keyframes before the current position

	for (long i = nKeyframe - 1; i >= 0; i--)
	{
		RemoveKeyframe ((DWORD)i);
	}

	// notify selves and parent dialog of the changes

	m_nCurrentTime = 0;
	DWORD nTime = m_nCurrentTime;
	SetAnim (m_pModelAnim);
	SetTime (nTime);
	GetModelEditDlg()->SetCurrentPosition (this, 0, 0, 0.0f);
	GetModelEditDlg()->SetChangesMade();
}

void CKeyframeWnd::OnTagKeyframe() 
{
	// attempt to get current keyframe

	DWORD nKeyframes = m_pModelAnim->m_KeyFrames;
	DWORD nKeyframe = ForceNearestKeyframe();
	if (nKeyframe == nKeyframes)
	{
		return;
	}

	// can't tag first or last keyframe

	if (nKeyframe == 0 || nKeyframe == nKeyframes - 1)
	{
		MessageBox ("Cannot tag the first nor the last keyframe in an animation", "Tag Keyframe", MB_OK | MB_ICONINFORMATION);
		return;
	}

	DoTagKeyframes( nKeyframe, nKeyframe );
}

// Tag keyframe range.  bForce forces value to bTag, rather than just toggle
BOOL CKeyframeWnd::DoTagKeyframes( DWORD nStartKeyframe, DWORD nEndKeyframe, BOOL bForce, BOOL bTag )
{
	if( !m_pModelAnim )
		return FALSE;

	// attempt to get current keyframe
	DWORD nKeyframe;
	BOOL bRet;
	DWORD nKeyframes = m_pModelAnim->m_KeyFrames;
	if (nEndKeyframe == nKeyframes)
	{
		return FALSE;
	}

	// can't tag first or last keyframe

	if (nStartKeyframe == 0 || nEndKeyframe >= nKeyframes - 1)
	{
		MessageBox ("Cannot tag the first nor the last keyframe in an animation", "Tag Keyframe", MB_OK | MB_ICONINFORMATION);
		return FALSE;
	}

	// get the tagged array
	// make sure there are at least 3 untagged keyframes
	int nUntagged = 0;
	for (DWORD i = 0; i < nKeyframes; i++)
	{
		if (!m_Tagged[i])
		{
			nUntagged++;
		}
	}

	bRet = TRUE;
	for( nKeyframe = nStartKeyframe; nKeyframe <= nEndKeyframe && bRet; nKeyframe++ )
	{
		// if the keyframe is already tagged, untag it...
		if (m_Tagged[nKeyframe])
		{			
			if( !bForce || ( bForce && bTag == FALSE ))
			{
				// draw the line in the normal color
				CPen pen;
				if( m_pModelAnim->m_KeyFrames[nKeyframe].m_pString[0] == '\0' )
					pen.CreatePen( PS_SOLID, 1, m_crKeyframe );
				else
					pen.CreatePen( PS_SOLID, 1, m_crKeyframeString );
				CPen* pOldPen = m_dcOffscreen.SelectObject (&pen);
				m_dcOffscreen.MoveTo (m_XVal[nKeyframe], m_rcKeyframes.top + 1);
				m_dcOffscreen.LineTo (m_XVal[nKeyframe], m_rcKeyframes.bottom - 1);
				m_dcOffscreen.SelectObject (pOldPen);
				pen.DeleteObject();

				CRect rcInvalid (m_XVal[nKeyframe], m_rcKeyframes.top + 1, m_XVal[nKeyframe] + 1, m_rcKeyframes.bottom - 1);
				InvalidateRect (rcInvalid, FALSE);
				
				// mark the keyframe as untagged
				m_Tagged[nKeyframe] = FALSE;
				nUntagged++;
			}
		}
		else
		{
			if( !bForce || ( bForce && bTag == TRUE ))
			{
				if (nUntagged < 3)
				{
					MessageBox ("Must have at least 2 untagged keyframes", "Tag Keyframe", MB_OK | MB_ICONINFORMATION);
					bRet = FALSE;
				}

				// draw the line in the tagged color
				CPen pen;
				if( m_pModelAnim->m_KeyFrames[nKeyframe].m_pString[0] == '\0' )
					pen.CreatePen( PS_SOLID, 1, m_crTaggedKeyframe );
				else
					pen.CreatePen( PS_SOLID, 1, m_crTaggedKeyframeString );
				CPen* pOldPen = m_dcOffscreen.SelectObject (&pen);
				m_dcOffscreen.MoveTo (m_XVal[nKeyframe], m_rcKeyframes.top + 1);
				m_dcOffscreen.LineTo (m_XVal[nKeyframe], m_rcKeyframes.bottom - 1);
				m_dcOffscreen.SelectObject (pOldPen);
				pen.DeleteObject();

				CRect rcInvalid (m_XVal[nKeyframe], m_rcKeyframes.top + 1, m_XVal[nKeyframe] + 1, m_rcKeyframes.bottom - 1);
				InvalidateRect (rcInvalid, FALSE);

				// mark the keyframe as tagged

				m_Tagged[nKeyframe] = TRUE;
				nUntagged--;
			}
		}
	}

	return bRet;
}

void CKeyframeWnd::OnUntagKeyframe() 
{
	// attempt to get current keyframe

	DWORD nKeyframe = ForceNearestKeyframe();
	if (nKeyframe == m_pModelAnim->m_KeyFrames)
	{
		return;
	}
	
	// get the tagged array

	// if the keyframe is already untagged, just return

	if (!m_Tagged[nKeyframe])
	{
		return;
	}

	// draw the line in the normal color

	CPen pen;
	if( m_pModelAnim->m_KeyFrames[nKeyframe].m_pString[0] == '\0' )
		pen.CreatePen( PS_SOLID, 1, m_crKeyframe );
	else
		pen.CreatePen( PS_SOLID, 1, m_crKeyframeString );
	CPen* pOldPen = m_dcOffscreen.SelectObject (&pen);
	m_dcOffscreen.MoveTo (m_XVal[nKeyframe], m_rcKeyframes.top + 1);
	m_dcOffscreen.LineTo (m_XVal[nKeyframe], m_rcKeyframes.bottom - 1);
	m_dcOffscreen.SelectObject (pOldPen);
	pen.DeleteObject();

	CRect rcInvalid (m_XVal[nKeyframe], m_rcKeyframes.top + 1, m_XVal[nKeyframe] + 1, m_rcKeyframes.bottom - 1);
	InvalidateRect (rcInvalid, FALSE);
	
	// mark the keyframe as untagged

	m_Tagged[nKeyframe] = FALSE;
}

void CKeyframeWnd::OnEditKeyframeTime() 
{
	// get nearest keyframe

	DWORD nKeyframes = m_pModelAnim->m_KeyFrames;
	DWORD nKeyframe = ForceNearestKeyframe();
	if (nKeyframe == nKeyframes)
	{
		return;
	}

	// cannot edit time of first keyframe

	if (nKeyframe == 0)
	{
		MessageBox ("Cannot edit keyframe time of first keyframe", "Edit Keyframe Time", MB_OK | MB_ICONINFORMATION);
		return;
	}

	// present the dialog to the user

	CKeyframeTimeDlg dlg;
	dlg.m_nCurrentTime = m_pModelAnim->m_KeyFrames[nKeyframe].m_Time;
	dlg.m_nNewTime = dlg.m_nCurrentTime;
	dlg.m_sCaption = "Edit Keyframe Time";
	if (dlg.DoModal() == IDCANCEL || dlg.m_nNewTime == dlg.m_nCurrentTime)
	{
		return;
	}

	DoEditKeyframeTime( nKeyframe, dlg.m_nNewTime );
}

void CKeyframeWnd::DoEditKeyframeTime( DWORD nKeyframe, DWORD dwNewKeyframeTime )
{
	// get nearest keyframe
	if( !m_pModelAnim )
		return;

	DWORD nKeyframes = m_pModelAnim->m_KeyFrames;
	if (nKeyframe == nKeyframes)
	{
		return;
	}

	// cannot edit time of first keyframe

	if (nKeyframe == 0)
	{
		MessageBox ("Cannot edit keyframe time of first keyframe", "Edit Keyframe Time", MB_OK | MB_ICONINFORMATION);
		return;
	}

	// present the dialog to the user

	if(dwNewKeyframeTime == m_pModelAnim->m_KeyFrames[nKeyframe].m_Time)
	{
		return;
	}

	// find where it's new position would be, and check for a duplicate time value

	for (DWORD i = 0; i < nKeyframes; i++)
	{
		if (m_pModelAnim->m_KeyFrames[i].m_Time == dwNewKeyframeTime && i != nKeyframe)
		{
			MessageBox ("A keyframe already exists with that time value.  Not changing keyframe time.", "Edit Keyframe Time", MB_OK | MB_ICONINFORMATION);
			return;
		}

		if (m_pModelAnim->m_KeyFrames[i].m_Time > dwNewKeyframeTime)
		{
			break;
		}
	}

	// set the new time value

	m_pModelAnim->m_KeyFrames[nKeyframe].m_Time = dwNewKeyframeTime;

	// if it's new position is the same, we don't need to do anything more...

	if (i - 1 != nKeyframe)
	{
		// new position different - take appropriate action

		MoveKeyframe (i, nKeyframe);
	}


	int32 nKeyToSelect = (nKeyframe < i) ? i - 1 : i;

	GetModelEditDlg()->SetCurrentPosition (this, nKeyToSelect, nKeyToSelect, 0);
	GetModelEditDlg()->SetChangesMade();

	SetAnim (m_pModelAnim);
	SetTime (dwNewKeyframeTime);
}

void CKeyframeWnd::OnDeleteKeyframe() 
{
	if(!CanPerformOperation("Delete Keyframe"))
		return;

	// get nearest keyframe

	DWORD nKeyframes = m_pModelAnim->m_KeyFrames;
	DWORD nKeyframe = ForceNearestKeyframe();
	if (nKeyframe == nKeyframes)
	{
		return;
	}
	
	// must be at least two keyframes in an animation

	if (nKeyframes <= 2)
	{
		MessageBox ("Cannot delete one of last two keyframes", "Delete Keyframe", MB_OK | MB_ICONINFORMATION);
		return;
	}

	// remove it

	RemoveKeyframe (nKeyframe);

	// let the dialog know what the current status is

	DWORD nSelKeyframe = LTMAX(0, LTMIN((int32)nKeyframes - 1, (int32)nKeyframe - 1));	
	GetModelEditDlg()->SetCurrentPosition (this, nSelKeyframe, nSelKeyframe, 0.0f);
	GetModelEditDlg()->SetChangesMade();
	
	SetAnim (m_pModelAnim);
	SetTime (m_pModelAnim->m_KeyFrames[nSelKeyframe].m_Time);
}

void CKeyframeWnd::OnDeleteTaggedKeyframes() 
{
	if(!CanPerformOperation("Delete Tagged Keyframes"))
		return;

	// go through and remove all tagged keyframes

	DWORD nKeyframes = m_pModelAnim->m_KeyFrames;

	for (long j = nKeyframes - 1; j >= 0; j--)
	{
		if (m_Tagged[j])
		{
			RemoveKeyframe ((DWORD)j);
		}
	}

	// now find out where we are on the bar and let the dialog know the status

	for (DWORD i = 0; i < m_pModelAnim->m_KeyFrames; i++)
	{
		if (m_pModelAnim->m_KeyFrames[i].m_Time >= m_nCurrentTime)
		{
			break;
		}
	}

	//make sure it is within range
	uint32 nSelKeyframe = LTMAX(0, LTMIN((int32)m_pModelAnim->m_KeyFrames - 1, (int32)i - 1));	

	GetModelEditDlg()->SetCurrentPosition (this, nSelKeyframe, nSelKeyframe, 0.0f);
	GetModelEditDlg()->SetChangesMade();
	
	SetAnim (m_pModelAnim);
	SetTime (m_pModelAnim->m_KeyFrames[nSelKeyframe].m_Time);
}


void CKeyframeWnd::OnEditTaggedKeyframeString()
{
	if(!CanPerformOperation("Edit Tagged Keyframes String"))
		return;

	DWORD nKeyframe;
	CKeyframeStringDlg dlg;
	Model *pModel;
	int nTaggedKeyframes = 0;

	pModel = ((CModelEditDlg*)GetParent())->GetModel();
	if(pModel && m_pModelAnim)
	{
		// Loop over the tagged keyframes and see if all the strings match...
		for( nKeyframe = 0; nKeyframe < m_pModelAnim->m_KeyFrames; nKeyframe++ )
		{
			// Keyframe tagged?
			if( m_Tagged[nKeyframe] )
			{
				// On the first one found, accept it's string as the master...
				if( nTaggedKeyframes == 0 )
					dlg.m_KeyframeString = m_pModelAnim->m_KeyFrames[nKeyframe].m_pString;
				else
				{
					// Check if the strings are the same...
					if( strcmp( dlg.m_KeyframeString, m_pModelAnim->m_KeyFrames[nKeyframe].m_pString ) != 0 )
					{
						// Strings are different, set master string to blank...
						dlg.m_KeyframeString = "";
						break;
					}
				}
				nTaggedKeyframes++;
			}
		}

		// No keyframes tagged, nothing to do...
		if( nTaggedKeyframes == 0 )
			return;

		if(dlg.DoModal() == IDOK)
		{
			// Loop over the tagged keyframes and set the strings.
			for( nKeyframe = 0; nKeyframe < m_pModelAnim->m_KeyFrames; nKeyframe++ )
			{
				// Keyframe tagged?
				if( m_Tagged[nKeyframe] )
				{
					DoEditKeyframeString( nKeyframe, dlg.m_KeyframeString );
				}
			}
		}

		((CModelEditDlg*)GetParent())->UpdateEditFrameString( );
	}
}

void CKeyframeWnd::DoEditKeyframeString( DWORD nKeyframe, CString &sFrameString )
{
	Model *pModel;

	pModel = ((CModelEditDlg*)GetParent())->GetModel();
	if(pModel && m_pModelAnim)
	{
		m_pModelAnim->m_KeyFrames[nKeyframe].m_pString = 
			pModel->AddString((char*)(LPCTSTR)sFrameString);
		Redraw();
		((CModelEditDlg*)GetParent())->SetChangesMade();
	}
}

void CKeyframeWnd::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	// just immediately move to new position
	// (do this by tricking mousemove handler)

	m_ptOld.x = m_rcTopMarker.left + (m_rcTopMarker.Width() >> 1);
	m_bTracking = TRUE;
	OnMouseMove (0, point);
	m_bTracking = FALSE;
	DWORD nKeyframe = ForceNearestKeyframe();
	DoTagKeyframes( nKeyframe, nKeyframe );
	
	CWnd::OnLButtonDblClk(nFlags, point);
}


void CKeyframeWnd::OnTagAll()
{
	DWORD nKeyframes = m_pModelAnim->m_KeyFrames;

	if( nKeyframes < 3 )
		return;

	DoTagKeyframes( 1, nKeyframes - 2, TRUE, TRUE );
}

void CKeyframeWnd::OnUnTagAll()
{
	DWORD nKeyframes = m_pModelAnim->m_KeyFrames;

	if( nKeyframes < 3 )
		return;

	DoTagKeyframes( 1, nKeyframes - 2, TRUE, FALSE );
}


void CKeyframeWnd::OnInsertTimeDelay()
{
	if(!CanPerformOperation("Insert Time Delay"))
		return;

	DWORD i, nKeyframe;
	TimeOffsetDlg dlg;

	if(!m_pModelAnim)
		return;
	
	if(dlg.DoModal() == IDOK)
	{
		nKeyframe = ForceNearestKeyframe();

		for(i=nKeyframe; i < m_pModelAnim->NumKeyFrames(); i++)
		{
			m_pModelAnim->m_KeyFrames[i].m_Time += dlg.m_TimeOffset;
		}

		Redraw();
		GetModelEditDlg()->SetChangesMade();
	}		
}
