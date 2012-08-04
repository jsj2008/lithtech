// ----------------------------------------------------------------------- //
//
// MODULE  : ltguiscrollbar.cpp
//
// PURPOSE : Defines the CLTGUIScrollBar class.  This class creates a
//           scrollbar control.
//
// CREATED : 06/20/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "ltguimgr.h"
#include "ltguiscrollbar.h"
#include "InterfaceSound.h"

#define WHEEL_DELTA      120      // Default value for rolling one notch

// constructor
CLTGUIScrollBar::CLTGUIScrollBar() :
	m_pMessageControl(NULL),
	m_pInputCaptureHandler(NULL),
	m_nPage(10),
	m_nMin(0),
	m_nMax(0),
	m_nPos(0),
	m_CaptureState(eCapture_None),
	m_fShaftScale(1.0f),
	m_fScrollSpeed(0.1f),
	m_fScrollDelay(0.5f),
	m_nFrameWidth(0)
{
	m_ScrollTimer.SetEngineTimer( RealTimeTimer::Instance() );
}

// destructor
CLTGUIScrollBar::~CLTGUIScrollBar()
{
	Destroy();
}

// creates a scrollbar control
bool CLTGUIScrollBar::Create( const CLTGUIScrollBar_create& cs )
{
	if( !cs.hBarTextureNormal || !cs.hBarTextureHot ) 
		return false;

	m_hBarNormal = cs.hBarTextureNormal;
	m_hBarHot = cs.hBarTextureHot;
	m_nSelectedColor = cs.nSelectedColor;
	m_nBackgroundColor = cs.nBackgroundColor;

	CLTGUICtrl::Create( (CLTGUICtrl_create)cs );

	InitBar();

	return true;
}

// destroys the scrollbar
void CLTGUIScrollBar::Destroy()
{
	m_ScrollTimer.Stop();
}

// initializes the control
void CLTGUIScrollBar::InitBar()
{
	RecalcLayout();
	SetupQuadUVs(m_Bar[eBarElement_Thumb], m_hBarNormal, 0.0f,0.5f,0.25f,0.5f);
	SetupQuadUVs(m_Bar[eBarElement_Track], m_hBarNormal, 0.0f,0.0f,0.25f,0.5f);
	SetupQuadUVs(m_Bar[eBarElement_ArrowUp], m_hBarNormal, 0.25f,0.0f,0.25f,0.5f);
	SetupQuadUVs(m_Bar[eBarElement_ArrowDown], m_hBarNormal, 0.25f,0.5f,0.25f,0.5f);
	SetupQuadUVs(m_Bar[eBarElement_ThumbTop], m_hBarNormal, 0.5f,0.0f,0.25f,0.25f);
	SetupQuadUVs(m_Bar[eBarElement_ThumbBottom], m_hBarNormal, 0.5f,0.25f,0.25f,0.25f);
	SetupQuadUVs(m_Bar[eBarElement_Grip], m_hBarNormal, 0.5f,0.5f,0.25f,0.5f);

	for(int nElement=0;nElement<eBarElementCount;nElement++) 
		DrawPrimSetRGBA(m_Bar[nElement], 0xFF, 0xFF, 0xFF, 0xFF);
}

// updates the positions of all scrollbar elements
void CLTGUIScrollBar::RecalcLayout()
{
	// eBarElement_Track
	LTRect2n rcTrack( (int32)m_rfRect.Left(), (int32)m_rfRect.Top(), (int32)m_rfRect.Right(), (int32)m_rfRect.Bottom() );
	DrawPrimSetXYWH( m_Bar[eBarElement_Track], (float)rcTrack.Left(), (float)rcTrack.Top(), (float)rcTrack.GetWidth(), (float)rcTrack.GetHeight() );

	// eBarElement_ArrowDown
	m_rcDownArrow.Init( (int32)m_rfRect.Left() + 1, (int32)m_rfRect.Bottom() - (int32)m_rfRect.GetWidth(), (int32)m_rfRect.Right(), (int32)m_rfRect.Bottom() );
	DrawPrimSetXYWH( m_Bar[eBarElement_ArrowDown], m_rfRect.Left(), m_rfRect.Bottom() - m_rfRect.GetWidth(), m_rfRect.GetWidth(), m_rfRect.GetWidth() );

	// eBarElement_ArrowUp
	m_rcUpArrow.Init( (int32)m_rfRect.Left() + 1, (int32)m_rfRect.Top(), (int32)m_rfRect.Right(), (int32)m_rfRect.Top() + (int32)m_rfRect.GetWidth());
	DrawPrimSetXYWH( m_Bar[eBarElement_ArrowUp], m_rfRect.Left(), m_rfRect.Top(), m_rfRect.GetWidth(), m_rfRect.GetWidth() );

	m_rcTrackWithoutArrows.Init( (int32)m_rcUpArrow.Left(), (int32)m_rcUpArrow.Bottom(), (int32)m_rcDownArrow.Right(), (int32)m_rcDownArrow.Top() );

	bool bForceHidden = false;
	if( m_pMessageControl && !m_pMessageControl->IsVisible() )
		bForceHidden = true;

	// calculate the position and size of the thumb
	int32 nShaftSize = (m_nMax - m_nMin);
	if( nShaftSize <= 0 )
	{
		m_fShaftScale = 1.0f;
		m_rcThumb.Init( (int32)m_rfRect.Left(), (int32)(m_rfRect.Top() + m_rfRect.GetWidth()), (int32)m_rfRect.Right(), (int32)(m_rfRect.Top() + m_rfRect.GetWidth() + m_rcTrackWithoutArrows.GetHeight()) );

		Show( false );
	}
	else if( m_nPos >= m_nMax)
	{
		Show( bForceHidden?false:true );

		// this case is just to make sure that the scrollbar reaches the bottom of the shaft
		// it may not always reach the bottom and be off by 1 pixel due to accuracy lose
		uint32 nThumbSize = 0;
		ComputeScaleAndThumb( nThumbSize );

		int32 nThumbBottom = (int32)(m_rfRect.Bottom() - m_rfRect.GetWidth());
		m_rcThumb.Init( (int32)m_rfRect.Left(), nThumbBottom - nThumbSize, (int32)m_rfRect.Right(), nThumbBottom );
	}
	else
	{
		Show( bForceHidden?false:true );

		uint32 nThumbSize = 0;
		ComputeScaleAndThumb( nThumbSize );

		int32 nThumbPos = LTMAX(m_nMin, LTMIN(m_nMax, m_nPos)) - m_nMin;

		int32 nThumbTop = (int32)((m_rfRect.Top() + m_rfRect.GetWidth()) + (nThumbPos * m_fShaftScale));
		m_rcThumb.Init( (int32)m_rfRect.Left(), nThumbTop, (int32)m_rfRect.Right(), nThumbTop + nThumbSize );
	}

	float fThumbCap = m_rfRect.GetWidth() * 0.5f;

	DrawPrimSetXYWH( m_Bar[eBarElement_ThumbTop], m_rfRect.Left(), (float)m_rcThumb.Top(), m_rfRect.GetWidth(), fThumbCap );
	DrawPrimSetXYWH( m_Bar[eBarElement_ThumbBottom], m_rfRect.Left(), (float)m_rcThumb.Bottom() - fThumbCap, m_rfRect.GetWidth(), fThumbCap );
	DrawPrimSetXYWH( m_Bar[eBarElement_Thumb], m_rfRect.Left(), (float)m_rcThumb.Top() + fThumbCap, m_rfRect.GetWidth(), (float)m_rcThumb.GetHeight() - (fThumbCap*2.0f) );
	DrawPrimSetXYWH( m_Bar[eBarElement_Grip], m_rfRect.Left(), (float)(int32)(m_rcThumb.Top() + (m_rcThumb.GetHeight() - m_rcThumb.GetWidth())*0.5f), m_rfRect.GetWidth(), m_rfRect.GetWidth() );

	if( m_Bar[eBarElement_ThumbTop].verts[3].pos.y > (float)m_rcThumb.Bottom() - fThumbCap )
	{
		DrawPrimSetXY(m_Bar[eBarElement_ThumbBottom], 0, m_rfRect.Left(), m_Bar[eBarElement_ThumbTop].verts[3].pos.y );
		DrawPrimSetXY(m_Bar[eBarElement_ThumbBottom], 1, m_rfRect.Right(), m_Bar[eBarElement_ThumbTop].verts[3].pos.y );

		DrawPrimSetXYWH( m_Bar[eBarElement_Thumb], m_rfRect.Left(), (float)m_rcThumb.Top() + fThumbCap, m_rfRect.GetWidth(), 0.0f );
	}

	float frameW = ((float)m_nFrameWidth/* * m_vfScale.x*/);
	float frameH = ((float)m_nFrameWidth/* * m_vfScale.y*/);

	//top
	float fx = m_rfRect.Left();
	float fy = m_rfRect.Top();
	float fw = m_rfRect.GetWidth();
	float fh = frameH;
	DrawPrimSetXYWH(m_Frame[0],fx,fy,fw,fh);

	//right
	fx = m_rfRect.Right() - frameW;
	fy = m_rfRect.Top();
	fw = frameW;
	fh = m_rfRect.GetHeight();
	DrawPrimSetXYWH(m_Frame[1],fx,fy,fw,fh);

	//bottom
	fx = m_rfRect.Left();
	fy = m_rfRect.Bottom() - frameH;
	fw = m_rfRect.GetWidth();
	fh = frameH;
	DrawPrimSetXYWH(m_Frame[2],fx,fy,fw,fh);

	//left
	fx = m_rfRect.Left();
	fy = m_rfRect.Top();
	fw = frameW;
	fh = m_rfRect.GetHeight();
	DrawPrimSetXYWH(m_Frame[3],fx,fy,fw,fh);

	//arrow top
	DrawPrimSetXYWH( m_Frame[4], m_rfRect.Left(), m_rfRect.Top() + m_rfRect.GetWidth(), m_rfRect.GetWidth(), frameH );

	//arrow bottom
	DrawPrimSetXYWH( m_Frame[5], m_rfRect.Left(), m_rfRect.Bottom() - m_rfRect.GetWidth() - frameH, m_rfRect.GetWidth(), frameH );

	//thumb top
	DrawPrimSetXYWH( m_Frame[6], m_rfRect.Left(), (float)m_rcThumb.Top(), m_rfRect.GetWidth(), frameH );

	//thumb bottom
	DrawPrimSetXYWH( m_Frame[7], m_rfRect.Left(), (float)m_rcThumb.Bottom() - frameH, m_rfRect.GetWidth(), frameH );
}

// computes the size of the thumb and the current scale
void CLTGUIScrollBar::ComputeScaleAndThumb( uint32& nThumbSize )
{
	const int32 nShaftSize = (m_nMax - m_nMin);
	const uint32 nMinThumbSize = GetBaseWidth();
	m_fShaftScale = (float)m_rcTrackWithoutArrows.GetHeight() / (float)(nShaftSize + m_nPage);
	nThumbSize = (uint32)LTMAX((uint32)(m_nPage * m_fShaftScale), nMinThumbSize);
	if( nThumbSize == nMinThumbSize )
	{
		float fPage = ((float)nShaftSize * (float)nThumbSize) / ((float)m_rcTrackWithoutArrows.GetHeight() - (float)nThumbSize);
		// update the scale
		m_fShaftScale = (float)m_rcTrackWithoutArrows.GetHeight() / ((float)nShaftSize + fPage);
	}
}

void CLTGUIScrollBar::SetRenderState()
{
	g_pDrawPrim->SetRenderMode(eLTDrawPrimRenderMode_Modulate_Translucent);
}

// Render the control
void CLTGUIScrollBar::Render()
{
	if( !IsVisible() )
		return;

	// check if our timer is up
	if( m_ScrollTimer.IsStarted() && m_ScrollTimer.IsTimedOut() )
	{
		float fAmount = (float)(m_ScrollTimer.GetElapseTime() / m_ScrollTimer.GetDuration());
		Scroll( fAmount );
	}

	// fill with a background color
	LT_POLYG4 polyBackground;
	DrawPrimSetRGBA( polyBackground, m_nBackgroundColor );
	DrawPrimSetXYWH( polyBackground, GetPos().x, GetPos().y, GetWidth(), GetHeight() );
	g_pDrawPrim->SetRenderMode(eLTDrawPrimRenderMode_Modulate_Translucent);
	g_pDrawPrim->DrawPrim( &polyBackground );

	TextureReference& hBar = IsSelected()?m_hBarHot:m_hBarNormal;

	g_pDrawPrim->SetTexture( hBar );
	g_pDrawPrim->DrawPrim( &m_Bar[eBarElement_Track] );

	g_pDrawPrim->SetTexture( hBar );
	g_pDrawPrim->DrawPrim( &m_Bar[eBarElement_ArrowDown] );

	g_pDrawPrim->SetTexture( hBar );
	g_pDrawPrim->DrawPrim( &m_Bar[eBarElement_ArrowUp] );

	g_pDrawPrim->SetTexture( hBar );
	g_pDrawPrim->DrawPrim( &m_Bar[eBarElement_ThumbTop] );

	g_pDrawPrim->SetTexture( hBar );
	g_pDrawPrim->DrawPrim( &m_Bar[eBarElement_ThumbBottom] );

	g_pDrawPrim->SetTexture( hBar );
	g_pDrawPrim->DrawPrim( &m_Bar[eBarElement_Thumb] );

	g_pDrawPrim->SetTexture( hBar );
	g_pDrawPrim->DrawPrim( &m_Bar[eBarElement_Grip] );

	// render the border
	if( m_nFrameWidth )
	{
		// set up the render state	
		g_pDrawPrim->SetRenderMode(eLTDrawPrimRenderMode_Modulate_NoBlend);

		for (int f = 0;f < LTARRAYSIZE(m_Frame); ++f)
			DrawPrimSetRGBA(m_Frame[f],GetCurrentColor());

		// draw our frames
		g_pDrawPrim->DrawPrim(m_Frame,6);

		if( m_nPos != 0 )
			g_pDrawPrim->DrawPrim( &m_Frame[6] );

		if( m_nPos < m_nMax )
			g_pDrawPrim->DrawPrim( &m_Frame[7] );
	}

	// set up the render state	
	SetRenderState();
}

// Render the control
void CLTGUIScrollBar::RenderTransition(float fTrans)
{
}

void CLTGUIScrollBar::SetBasePos( const LTVector2n& pos )
{
	CLTGUICtrl::SetBasePos( pos );
	RecalcLayout();
}

void CLTGUIScrollBar::SetSize( const LTVector2n& sz )
{
	CLTGUICtrl::SetSize( sz );
	RecalcLayout();
}

void CLTGUIScrollBar::SetScale(const LTVector2& vfScale)
{
	CLTGUICtrl::SetScale( vfScale );
	RecalcLayout();
}

// scrolls
void CLTGUIScrollBar::Scroll( float fScrollAmount )
{
	switch( m_CaptureState )
	{
	case eCapture_UpArrow:
		if( m_rcUpArrow.Contains(m_ptCursor) )
		{
			if( m_pMessageControl )
				m_pMessageControl->SendCommand( eScrollBarCmd_LineUp, 0, 0 );
		}
		m_ScrollTimer.Start( GetScrollSpeed() );
		break;
	case eCapture_DownArrow:
		if( m_rcDownArrow.Contains(m_ptCursor) )
		{
			if( m_pMessageControl )
				m_pMessageControl->SendCommand( eScrollBarCmd_LineDown, 0, 0 );
		}
		m_ScrollTimer.Start( GetScrollSpeed() );
		break;
	case eCapture_PageUp:
		if( m_ptThumbOffset.y < m_rcThumb.Top() )
		{
			if( m_pMessageControl )
				m_pMessageControl->SendCommand( eScrollBarCmd_PageUp, 0, 0 );
			m_ScrollTimer.Start( GetScrollSpeed() );
		}
		break;
	case eCapture_PageDown:
		if( m_ptThumbOffset.y > m_rcThumb.Bottom() )
		{
			if( m_pMessageControl )
				m_pMessageControl->SendCommand( eScrollBarCmd_PageDown, 0, 0 );
			m_ScrollTimer.Start( GetScrollSpeed() );
		}
		break;
	}
}

// called when the mouse is over the control
bool CLTGUIScrollBar::OnMouseMove(int x, int y)
{
	switch( m_CaptureState )
	{
	case eCapture_UpArrow:
	case eCapture_DownArrow:
		m_ptCursor.Init( x, y );
		break;

	case eCapture_Thumb:
		{
			m_ptCursor.Init( x, y );
			LTVector2n ptThumbPos = m_ptCursor - m_ptThumbOffset;
			int32 nPos = ptThumbPos.y - m_rcTrackWithoutArrows.Top();

			int32 nShaftSize = m_nMax - m_nMin;

			nPos = (int32)((float)nPos / m_fShaftScale);
			m_nPos = LTMAX(m_nMin, LTMIN(m_nMax, nPos));
			
			RecalcLayout();
			if( m_pMessageControl )
				m_pMessageControl->SendCommand( eScrollBarCmd_ThumbTrack, m_nPos, 0 );
		}
		break;
	}

	return false;
}

bool CLTGUIScrollBar::OnMouseWheel(int x, int y, int zDelta)
{
	if( m_pMessageControl )
		return m_pMessageControl->OnMouseWheel( x, y, zDelta );

	return false;
}

// called when the left mouse button is pressed over the scroll control
bool CLTGUIScrollBar::OnLButtonDown(int x, int y)
{
	if( m_pMessageControl )
	{
		LTVector2n vCursorPos(x, y);
		if( m_rcUpArrow.Contains(vCursorPos) )
		{
			if( m_pInputCaptureHandler )
			{
				m_pInputCaptureHandler->SendCommand( eGUICtrlCmd_SetCapture, (uint32)this, true );
				m_CaptureState = eCapture_UpArrow;
				m_ptCursor = vCursorPos;
				m_ScrollTimer.Start( GetScrollDelay() );
				m_pInputCaptureHandler->SendCommand( eGUICtrlCmd_PlaySound, IS_UP, 0 );
			}
			m_pMessageControl->SendCommand( eScrollBarCmd_LineUp, 0, 0 );
		}
		else if( m_rcDownArrow.Contains(vCursorPos) )
		{
			if( m_pInputCaptureHandler )
			{
				m_pInputCaptureHandler->SendCommand( eGUICtrlCmd_SetCapture, (uint32)this, true );
				m_CaptureState = eCapture_DownArrow;
				m_ptCursor = vCursorPos;
				m_ScrollTimer.Start( GetScrollDelay() );
				m_pInputCaptureHandler->SendCommand( eGUICtrlCmd_PlaySound, IS_DOWN, 0 );
			}
			m_pMessageControl->SendCommand( eScrollBarCmd_LineDown, 0, 0 );
		}
		else if( m_rcThumb.Contains(vCursorPos) )
		{
			if( m_pInputCaptureHandler )
			{
				m_pInputCaptureHandler->SendCommand( eGUICtrlCmd_SetCapture, (uint32)this, true );
				m_CaptureState = eCapture_Thumb;
			}
			m_ptCursor = vCursorPos;
			m_ptThumbOffset = m_ptCursor - m_rcThumb.GetTopLeft();
		}
		else if( m_rcTrackWithoutArrows.Contains(vCursorPos) )
		{
			if( m_pInputCaptureHandler )
			{
				m_pInputCaptureHandler->SendCommand( eGUICtrlCmd_SetCapture, (uint32)this, true );

				if( m_rcThumb.Bottom() < vCursorPos.y )
					m_CaptureState = eCapture_PageDown;
				else
					m_CaptureState = eCapture_PageUp;
				m_ScrollTimer.Start( GetScrollDelay() );
			}

			if( m_rcThumb.Bottom() < vCursorPos.y )
				m_pMessageControl->SendCommand( eScrollBarCmd_PageDown, 0, 0 );
			else
				m_pMessageControl->SendCommand( eScrollBarCmd_PageUp, 0, 0 );
			m_ptCursor = vCursorPos;
			m_ptThumbOffset = vCursorPos;
		}
	}

	return false;
}

// called when the left mouse button is released over the scroll control
bool CLTGUIScrollBar::OnLButtonUp(int x, int y)
{
	if( m_CaptureState != eCapture_None )
	{
		m_pInputCaptureHandler->SendCommand( eGUICtrlCmd_ReleaseCapture, 0, 0 );
		m_CaptureState = eCapture_None;
		m_ScrollTimer.Stop();
	}

	return false;
}

// sets the width of the frame
void CLTGUIScrollBar::SetFrameWidth( uint8 nFrameWidth )
{
	m_nFrameWidth = nFrameWidth;
	RecalcLayout();
}

// called to show the window
void CLTGUIScrollBar::Show( bool bShow )
{
	int32 nShaftSize = (m_nMax - m_nMin);
	if( nShaftSize <= 0 )
		bShow = false;

	CLTGUICtrl::Show( bShow );
}
