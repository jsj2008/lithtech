// ----------------------------------------------------------------------- //
//
// MODULE  : LTGUIFrame.cpp
//
// PURPOSE : Simple resizeable frame control
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "ltguimgr.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLTGUIFrame::CLTGUIFrame()
{

	m_hFrame   = NULL;
	m_bEnabled = false;
	m_bSimpleStretch = false;
	m_argbColor = 0xFFFFFFFF;

	m_nBorderWidth = 0;
	m_BorderColor = 0xFF000000;
}

CLTGUIFrame::~CLTGUIFrame()
{
	Destroy();
}

// Create the control
bool CLTGUIFrame::Create(HTEXTURE hFrame,  const CLTGUICtrl_create& cs, bool bSimpleStretch)
{
	if (!hFrame) return false;

	m_hFrame	= hFrame;
	m_bSimpleStretch = bSimpleStretch;
	m_argbColor = 0xFFFFFFFF;

	InitPolies();

	CLTGUICtrl::Create(cs);

    return true;
}

bool CLTGUIFrame::Create(uint32 argbColor, const CLTGUICtrl_create& cs)
{

	m_hFrame	= NULL;

	m_bSimpleStretch = true;

	m_argbColor = argbColor;

	InitPolies();
    
	CLTGUICtrl::Create(cs);

    return true;

}

// Render the control
void CLTGUIFrame::Render()
{
	// Sanity checks...
	if (!IsVisible()) return;

	g_pDrawPrim->SetTexture(m_hFrame);

	// set up the render state	
	SetRenderState();

	// draw our button
	if (m_bSimpleStretch)
	{
		g_pDrawPrim->DrawPrim(m_Poly);
	}
	else
	{
		g_pDrawPrim->DrawPrim(m_Poly,9);
	}

	if (m_nBorderWidth)
	{
		for (int f = 0;f < 4; ++f)
			DrawPrimSetRGBA(m_Border[f],m_BorderColor);

		// draw our frames
		g_pDrawPrim->DrawPrim(m_Border,4);
	}


}


// Render the control
void CLTGUIFrame::RenderTransition(float fTrans)
{
	// Sanity checks...
	if (!IsVisible()) return;

	g_pDrawPrim->SetTexture(m_hFrame);

	// set up the render state	
	SetRenderState();

	// draw our button
	if (m_bSimpleStretch)
	{
		DrawPrimSetRGBA(m_Poly[0],FadeARGB(m_argbColor,fTrans));
		g_pDrawPrim->DrawPrim(m_Poly);
		DrawPrimSetRGBA(m_Poly[0],m_argbColor);
	}
}


void CLTGUIFrame::SetBasePos(const LTVector2n& pos)
{ 
	CLTGUICtrl::SetBasePos(pos);
	ScalePolies();
}

void CLTGUIFrame::SetSize(const LTVector2n& sz)
{ 
	CLTGUICtrl::SetSize(sz);
	ScalePolies();
}

void CLTGUIFrame::SetScale(const LTVector2& vfScale)
{
	CLTGUICtrl::SetScale(vfScale);
	ScalePolies();
}

void CLTGUIFrame::InitPolies()
{
	ScalePolies();

	if (m_bSimpleStretch)
	{
		SetupQuadUVs(m_Poly[0], m_hFrame, 0.0f, 0.0f, 1.0f, 1.0f);
		DrawPrimSetRGBA(m_Poly[0],m_argbColor);
	}
	else
	{
		float uv[4] = {0.0f, 0.25f, 0.75f, 1.0f};


		for (int r = 0; r < 3; r++)
		{
			for (int c = 0; c < 3; c++)
			{
				int i = r*3+c;
				DrawPrimSetRGBA(m_Poly[i], 0xFF, 0xFF, 0xFF, 0xFF);
				SetupQuadUVs(m_Poly[i], m_hFrame, uv[c], uv[r], uv[c+1] - uv[c], uv[r+1] - uv[r]);
			}
		}
	}


}

// This prevents an internal compiler error under the VC 7.1 compiler.  It doesn't like DrawPrimSetXYWH.
#pragma optimize("g", off)

void CLTGUIFrame::ScalePolies()
{
	float fx = m_rfRect.Left();
	float fy = m_rfRect.Top();
	float fWidth = m_rfRect.GetWidth();
	float fHeight = m_rfRect.GetHeight();

	if (m_bSimpleStretch)
	{
		DrawPrimSetXYWH(m_Poly[0],fx,fy,fWidth,fHeight);
	}
	else
	{

		if (!m_hFrame) return;
		uint32 tw,th;
		g_pILTTextureMgr->GetTextureDims(m_hFrame,tw,th);
		float fw = (float)tw;
		float fh = (float)th;

		float x[4] = {fx, fx+fw*0.25f, (fx+fWidth) - fw*0.25f, (fx+fWidth) };
		float y[4] = {fy, fy+fh*0.25f, (fy+fHeight) - fh*0.25f, (fy+fHeight) };


		for (int r = 0; r < 3; r++)
		{
			for (int c = 0; c < 3; c++)
			{
				int i = r*3+c;
				DrawPrimSetXY(m_Poly[i], 0, x[c], y[r]);
				DrawPrimSetXY(m_Poly[i], 1, x[c+1], y[r]);
				DrawPrimSetXY(m_Poly[i], 2, x[c+1], y[r+1]);
				DrawPrimSetXY(m_Poly[i], 3, x[c], y[r+1]);
			}
		}
	}

	float fBorderW = ((float)m_nBorderWidth * m_vfScale.x);
	float fBorderH = ((float)m_nBorderWidth * m_vfScale.y);

	//top
	fx = m_rfRect.Left() - fBorderW;
	fy = m_rfRect.Top() - fBorderH;
	float fw = fWidth + fBorderW;
	float fh = fBorderH;
	DrawPrimSetXYWH(m_Border[0],fx,fy,fw,fh);

	//right
	fx = m_rfRect.Left() + fWidth;
	fy = m_rfRect.Top() - fBorderH;
	fw = fBorderW;
	fh = fHeight + fBorderH;
	DrawPrimSetXYWH(m_Border[1],fx,fy,fw,fh);

	//bottom
	fx = m_rfRect.Left();
	fy = m_rfRect.Top() + fHeight;
	fw = fWidth + fBorderW;
	fh = fBorderH;
	DrawPrimSetXYWH(m_Border[2],fx,fy,fw,fh);

	//left
	fx = m_rfRect.Left() - fBorderW;
	fy = m_rfRect.Top();
	fw = fBorderW;
	fh = fHeight + fBorderH;
	DrawPrimSetXYWH(m_Border[3],fx,fy,fw,fh);
}
// See comment above function.
#pragma optimize("g", on)

void CLTGUIFrame::SetRenderState()
{
	g_pDrawPrim->SetRenderMode(eLTDrawPrimRenderMode_Modulate_Translucent);;
}



void CLTGUIFrame::SetFrame(HTEXTURE hFrame)
{
	m_hFrame	= hFrame;
	m_argbColor = 0xFFFFFFFF;
	InitPolies();
	ScalePolies();
}

void CLTGUIFrame::SetColor(uint32 argbColor)
{
	m_hFrame	= NULL;
	m_argbColor = argbColor;
	DrawPrimSetRGBA(m_Poly[0],m_argbColor);
	ScalePolies();
}

// Sets the width of the frames's border, set to 0 to not show the frame
void CLTGUIFrame::SetBorder(uint8 nBorderWidth, uint32 nBorderColor)
{
	m_nBorderWidth = nBorderWidth;
	m_BorderColor = nBorderColor;
	ScalePolies();
}
