//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
// ColorSelectDlg.cpp : implementation file
//

#include "bdefs.h"
#include "dedit.h"
#include "colorselectdlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



#define CS_SHIFT	25
#define CS_ONE		(1<<25)

#define VECTOR_TO_COLORREF(vec)	( (DWORD)(vec.x*255.0f) | (((DWORD)(vec.y*255.0f))<<8) | (((DWORD)(vec.z*255.0f))<<16) )
#define COLORREF_TO_VECTOR(color, vec)\
	{\
		(vec).x = GetRValue(color) / 255.0f;\
		(vec).y = GetGValue(color) / 255.0f;\
		(vec).z = GetBValue(color) / 255.0f;\
	}	



/////////////////////////////////////////////////////////////////////////////
// CColorSelectDlg dialog


CColorSelectDlg::CColorSelectDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CColorSelectDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CColorSelectDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_pDib			= NULL;
	m_bLButtonDown	= FALSE;
	m_SelBoxRadius	= 10;
	m_bInitialized	= FALSE;
}


CColorSelectDlg::~CColorSelectDlg()
{
	if( m_pDib )
		m_DibMgr.RemoveDib( m_pDib );
}


void CColorSelectDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CColorSelectDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CColorSelectDlg, CDialog)
	//{{AFX_MSG_MAP(CColorSelectDlg)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_PAINT()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()




/////////////////////////////////////////////////////////////////////////////
// CColorSelectDlg custom functions

void CColorSelectDlg::SetCurColor(COLORREF inColor, BOOL bNotify)
{
	DVector vColor, vWhite, vHueColor, vTest;
	float hue, fPercent, fPercentInc, fClosest, fTest;
	int x, y, width, height, halfWidth;
	int xClosest, yClosest;

	// Setup our internal color.
	COLORREF_TO_VECTOR(inColor, vColor);
	m_CurColor = inColor;

	
	// Figure out where the selection box should go.
	width = m_DrawRect.right - m_DrawRect.left;
	height = m_DrawRect.bottom - m_DrawRect.top;
	halfWidth = width >> 1;

	fClosest = 1000000.0f;
	xClosest = yClosest = 0;
	for( y=0; y < height; y++ )
	{
		hue = ((CReal)y / height) * 360.0f;
		HSV_To_RGB(hue, 1.0f, 1.0f, vHueColor);

		// Black to the color.
		fPercent = 0.0f;
		fPercentInc = 1.0f / halfWidth;
		for(x=0; x < halfWidth; x++)
		{
			vTest = vHueColor * fPercent;
			fTest = (vTest - vColor).Mag();
			if(fTest < fClosest)
			{
				xClosest = x;
				yClosest = y;
				fClosest = fTest;
			}

			fPercent += fPercentInc;
		}

		// The color to white.
		fPercent = 0.0f;
		fPercentInc = 1.0f / halfWidth;
		for(x=halfWidth; x < width; x++)
		{
			vTest = vHueColor + (vWhite - vHueColor) * fPercent;
			fTest = (vTest - vColor).Mag();
			if(fTest < fClosest)
			{
				xClosest = x;
				yClosest = y;
				fClosest = fTest;
			}

			fPercent += fPercentInc;
		}
	}


	m_CurSelPt.x = (float)xClosest;
	m_CurSelPt.y = (float)yClosest;

	DrawColors();

	// Do notifications..
	if(bNotify)
	{
		NotifyCallbacks();
	}
}


BOOL CColorSelectDlg::AddCallback(ColorChangeCB cb, void *pUser)
{
	ColorChangeNotify notify;

	notify.m_CB = cb;
	notify.m_pUser = pUser;
	return m_Callbacks.Append(notify);
}


void CColorSelectDlg::RemoveCallback(ColorChangeCB cb, void *pUser)
{
	ColorChangeNotify notify;
	DWORD index;

	notify.m_CB = cb;
	notify.m_pUser = pUser;
	index = m_Callbacks.FindElement(notify);
	if(index < m_Callbacks.GetSize())
		m_Callbacks.Remove(index);
}


void CColorSelectDlg::SetColorSel()
{
	CPoint ret;

	GetCursorPos( &ret );
	ScreenToClient( &ret );

	m_CurSelPt.x = (CReal)CLAMP( ret.x, m_DrawRect.left, m_DrawRect.right-1 );
	m_CurSelPt.y = (CReal)CLAMP( ret.y, m_DrawRect.top, m_DrawRect.bottom-1 );

	m_CurColor = GetColorFromPoint( m_CurSelPt );

	NotifyCallbacks();
}


void CColorSelectDlg::NotifyCallbacks()
{
	DWORD i;	

	for(i=0; i < m_Callbacks; i++)
	{
		m_Callbacks[i].m_CB(m_CurColor, m_Callbacks[i].m_pUser);
	}
}


COLORREF CColorSelectDlg::GetColorFromPoint( CVector point )
{
	CReal		hue;
	CVector		color, white;
	int			halfWidth;
	
	if( !m_pDib )
		return 0;

	hue = ((CReal)point.y / m_pDib->GetHeight()) * 360.0f;
	HSV_To_RGB( hue, 1.0f, 1.0f, color );

	halfWidth = m_DrawRect.Width() / 2;
	if( point.x < halfWidth )
	{
		color *= (CReal)point.x / halfWidth;
	}
	else
	{
		white.Init( 1.0f, 1.0f, 1.0f );
		color += (white - color) * ((CReal)(point.x - halfWidth) / halfWidth);
	}

	return VECTOR_TO_COLORREF(color);
}


void CColorSelectDlg::DrawColors()
{
	if( !m_pDib )
		return;

	CDC			*pDC;
	CDib		*pDib = m_pDib;
	
	DWORD		y;
	DWORD		pitch = pDib->GetPitch();
	DWORD		dwHeight = (DWORD)pDib->GetHeight();
	WORD		*pBuf = pDib->GetBuf16();

	CVector		color;
	CReal		hue;

	CBrush		brush, *pOldBrush;
								

	// Clear out.
	for( y=0; y < dwHeight; y++ )
		memset( &pBuf[y*pitch], 0, pitch*2 );

	// Draw the colors.
	for( y=0; y < dwHeight; y++ )
	{
		hue = ((CReal)y / pDib->GetHeight()) * 360.0f;
		HSV_To_RGB( hue, 1.0f, 1.0f, color );

		DrawColorStrip( color.x, color.y, color.z, &pBuf[((dwHeight-y)-1)*pitch], pDib->GetWidth()/2 );
	}

	
	// Blit.
	pDC = GetDC();
	pDC->SetStretchBltMode( COLORONCOLOR );
	
	pDib->Blt( pDC->m_hDC, m_DrawRect.left, m_DrawRect.top, pDib->GetWidth(), pDib->GetHeight() );
	
	brush.CreateSolidBrush( m_CurColor );
	pOldBrush = pDC->SelectObject( &brush );
	pDC->Rectangle( (int)(m_CurSelPt.x-m_SelBoxRadius), (int)(m_CurSelPt.y+m_SelBoxRadius), 
					(int)(m_CurSelPt.x+m_SelBoxRadius), (int)(m_CurSelPt.y-m_SelBoxRadius) );
	pDC->SelectObject( pOldBrush );

	ReleaseDC( pDC );
}



void CColorSelectDlg::DrawColorStrip( CReal inR, CReal inG, CReal inB, WORD *pBuf, DWORD halfWidth )
{
	DWORD		i;
	
	SDWORD		dwInR = (SDWORD)(inR * (CReal)CS_ONE), dwInG = (SDWORD)(inG * (CReal)CS_ONE), dwInB = (SDWORD)(inB * (CReal)CS_ONE);
	DWORD		r, g, b, rInc, gInc, bInc;

	r = g = b = 0;
	rInc = (dwInR - r) / halfWidth;
	gInc = (dwInG - g) / halfWidth;
	bInc = (dwInB - b) / halfWidth;

	for( i=0; i < halfWidth; i++ )
	{
		*pBuf++ = (WORD)( ((r>>20)<<10) | ((g>>20)<<5) | (b>>20) );
		r += rInc;
		g += gInc;
		b += bInc;
	}
		
	rInc = (CS_ONE - r) / halfWidth;
	gInc = (CS_ONE - g) / halfWidth;
	bInc = (CS_ONE - b) / halfWidth;

	for( i=0; i < halfWidth; i++ )
	{
		*pBuf++ = (WORD)( ((r>>20)<<10) | ((g>>20)<<5) | (b>>20) );
		r += rInc;
		g += gInc;
		b += bInc;
	}
}


void CColorSelectDlg::HSV_To_RGB( CReal h, CReal s, CReal v, CVector &color )
{
	h /= 60.0f;
	CReal	i = (CReal)floor( h );
	CReal	f = h - i;
	CReal	p = v * (1.0f - s);
	CReal	q = v * (1.0f - (s * f));
	CReal	t = v * (1.0f - (s * (1.0f - f)));
	DWORD	dwI = (DWORD)i;

	if( i == 0 )
		color.Init( v, t, p );
	else if( i == 1 )
		color.Init( q, v, p );
	else if( i == 2 )
		color.Init( p, v, t );
	else if( i == 3 )
		color.Init( p, q, v );
	else if( i == 4 )
		color.Init( t, p, v );
	else if( i == 5 )
		color.Init( v, p, q );
}


void CColorSelectDlg::RGB_To_HSV(DVector color, float &h, float &s, float &v)
{
	float fMin, fMax, fDelta;

	
	fMin = DMIN(color.x, DMIN(color.y, color.z));
	fMax = DMAX(color.x, DMAX(color.y, color.z));

	v = fMax;
	
	// Determine saturation.
	s = 0.0f;
	if(fabs(fMax) > 0.00001f)
	{
		s = (fMax - fMin) / fMax;
	}

	if(fabs(s) < 0.00001f)
	{
		h = 0.0f; // It's black...
	}
	else
	{
		fDelta = fMax - fMin;
		if(color.x == fMax)
		{
			h = (color.y - color.z) / fDelta;
		}
		else if(color.y == fMax)
		{
			h = 2.0f + (color.z - color.x) / fDelta;
		}
		else
		{
			h = 4.0f + (color.x - color.y) / fDelta;
		}

		h *= 60.0f;
		if(h < 0)
			h += 360.0f;
	}
}



/////////////////////////////////////////////////////////////////////////////
// CColorSelectDlg message handlers

BOOL CColorSelectDlg::OnInitDialog() 
{
	int		width;
	CPoint	point;
	
	CDialog::OnInitDialog();
	
	GetClientRect( &m_DrawRect );

	width = m_DrawRect.Width() + (4 - (m_DrawRect.Width() % 4));
	m_DibMgr.Init( AfxGetInstanceHandle(), m_hWnd );
	m_pDib = m_DibMgr.AddDib( width, m_DrawRect.Height(), 16 );

	m_CurSelPt.x = (CReal)m_DrawRect.Width() / 2.0f;
	m_CurSelPt.y = (CReal)m_DrawRect.Height() / 2.0f;
	m_CurColor = GetColorFromPoint( m_CurSelPt );

	m_bInitialized = TRUE;

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}





void CColorSelectDlg::OnLButtonDown(UINT nFlags, CPoint point) 
{
	m_bLButtonDown = TRUE;

	SetColorSel();
	InvalidateRect( NULL, FALSE );
	SetCapture();
	
	CDialog::OnLButtonDown(nFlags, point);
}

void CColorSelectDlg::OnLButtonUp(UINT nFlags, CPoint point) 
{
	m_bLButtonDown = FALSE;
	ReleaseCapture();
	
	CDialog::OnLButtonUp(nFlags, point);
}

void CColorSelectDlg::OnMouseMove(UINT nFlags, CPoint point) 
{
	if( m_bLButtonDown )
	{
		SetColorSel();
		InvalidateRect( NULL, FALSE );
		m_LastPt = point;
	}
	
	CDialog::OnMouseMove(nFlags, point);
}

void CColorSelectDlg::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	DrawColors();
}

void CColorSelectDlg::OnSize(UINT nType, int cx, int cy) 
{
	//only let windows resize it if it is a valid size, otherwise don't let it
	if((cx <= 10) || (cy <= 10))
		return;
	
	CReal		scaleX, scaleY;
	int			width;

	CDialog::OnSize(nType, cx, cy);
	
	if(m_bInitialized)
	{
		scaleX = (CReal)cx / m_DrawRect.Width();
		scaleY = (CReal)cy / m_DrawRect.Height();

		m_CurSelPt.x *= scaleX;
		m_CurSelPt.y *= scaleY;

		m_DrawRect.right = cx;
		m_DrawRect.bottom = cy;

		width = m_DrawRect.Width() + (4 - (m_DrawRect.Width() % 4));

		if(m_pDib)
		{
			m_DibMgr.RemoveDib( m_pDib );
		}

		m_pDib = m_DibMgr.AddDib( width, m_DrawRect.Height(), 16 );
		InvalidateRect( NULL, FALSE );
	}
}


