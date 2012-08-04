// ----------------------------------------------------------------------- //
//
// MODULE  : ltguifillframe.cpp
//
// PURPOSE : Defines the CLTGUIFillFrame control class.  This class
//           creates a simple frame control that contains an outline
//           border and is filled with a specified color.
//
// CREATED : 06/30/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "ltguimgr.h"
#include "ltguifillframe.h"

// constructor
CLTGUIFillFrame::CLTGUIFillFrame() :
	m_nFrameWidth(0),
	m_nBorderFlags(eBorder_Left|eBorder_Top|eBorder_Right|eBorder_Bottom)
{
}

// destructor
CLTGUIFillFrame::~CLTGUIFillFrame()
{
	Destroy();
}

// creates a scrollbar control
bool CLTGUIFillFrame::Create( const CLTGUIFillFrame_create& cs )
{
	m_nSelectedColor = cs.nSelectedColor;
	m_nBackgroundColor = cs.nBackgroundColor;
	m_nNonSelectedColor = cs.nNonSelectedColor;

	CLTGUICtrl::Create( (CLTGUICtrl_create)cs );

	return true;
}

// destroys the scrollbar
void CLTGUIFillFrame::Destroy()
{
}

// updates the positions
void CLTGUIFillFrame::RecalcLayout()
{
	float frameW = ((float)m_nFrameWidth/* * m_vfScale.x*/);
	float frameH = ((float)m_nFrameWidth/* * m_vfScale.y*/);

	//top
	float fx = m_rfRect.Left();
	float fy = m_rfRect.Top();
	float fw = m_rfRect.GetWidth();
	float fh = frameH;
	DrawPrimSetXYWH(m_Frame[1],fx,fy,fw,fh);

	//right
	fx = m_rfRect.Right() - frameW;
	fy = m_rfRect.Top();
	fw = frameW;
	fh = m_rfRect.GetHeight();
	DrawPrimSetXYWH(m_Frame[2],fx,fy,fw,fh);

	//bottom
	fx = m_rfRect.Left();
	fy = m_rfRect.Bottom() - frameH;
	fw = m_rfRect.GetWidth();
	fh = frameH;
	DrawPrimSetXYWH(m_Frame[3],fx,fy,fw,fh);

	//left
	fx = m_rfRect.Left();
	fy = m_rfRect.Top();
	fw = frameW;
	fh = m_rfRect.GetHeight();
	DrawPrimSetXYWH(m_Frame[0],fx,fy,fw,fh);
}

// Render the control
void CLTGUIFillFrame::Render()
{
	if( !IsVisible() )
		return;

	// fill with a background color
	LT_POLYG4 polyBackground;
	DrawPrimSetRGBA( polyBackground, m_nBackgroundColor );
	DrawPrimSetXYWH( polyBackground, GetPos().x, GetPos().y, GetWidth(), GetHeight() );
	g_pDrawPrim->SetRenderMode(eLTDrawPrimRenderMode_Modulate_Translucent);
	g_pDrawPrim->DrawPrim( &polyBackground );

	// render the border
	if( m_nFrameWidth )
	{
		// set up the render state	
		g_pDrawPrim->SetRenderMode(eLTDrawPrimRenderMode_Modulate_NoBlend);

		for (int f = 0;f < LTARRAYSIZE(m_Frame); ++f)
			DrawPrimSetRGBA(m_Frame[f],GetCurrentColor());

		// draw our frames
		if( (m_nBorderFlags & eBorder_Left) )
			g_pDrawPrim->DrawPrim( &m_Frame[0], 1 );
		if( (m_nBorderFlags & eBorder_Top) )
			g_pDrawPrim->DrawPrim( &m_Frame[1], 1 );
		if( (m_nBorderFlags & eBorder_Right) )
			g_pDrawPrim->DrawPrim( &m_Frame[2], 1 );
		if( (m_nBorderFlags & eBorder_Bottom) )
			g_pDrawPrim->DrawPrim( &m_Frame[3], 1 );
	}
}

// Render the control
void CLTGUIFillFrame::RenderTransition(float fTrans)
{
}

void CLTGUIFillFrame::SetBasePos( const LTVector2n& pos )
{
	CLTGUICtrl::SetBasePos( pos );
	RecalcLayout();
}

void CLTGUIFillFrame::SetSize( const LTVector2n& sz )
{
	CLTGUICtrl::SetSize( sz );
	RecalcLayout();
}

void CLTGUIFillFrame::SetScale(const LTVector2& vfScale)
{
	CLTGUICtrl::SetScale( vfScale );
	RecalcLayout();
}

// sets the width of the frame
void CLTGUIFillFrame::SetFrameWidth( uint8 nFrameWidth )
{
	m_nFrameWidth = nFrameWidth;
	RecalcLayout();
}

void CLTGUIFillFrame::SetRenderBorder( uint32 nBorderFlags )
{
	m_nBorderFlags = nBorderFlags;
}