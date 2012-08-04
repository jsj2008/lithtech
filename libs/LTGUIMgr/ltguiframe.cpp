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

	m_hFrame   = LTNULL;

	m_nWidth			= 0;
	m_nHeight			= 0;
	m_nBaseWidth		= 0;
	m_nBaseHeight		= 0;

	m_bEnabled = LTFALSE;

	m_bSimpleStretch = LTFALSE;

	m_fTextureScale = 1.0f;

	m_argbColor = 0xFFFFFFFF;

	memset(m_Poly,0,sizeof(m_Poly));
	memset(m_Border,0,sizeof(m_Border));

	m_nBorderWidth = 0;
	m_BorderColor = 0xFF000000;

}

CLTGUIFrame::~CLTGUIFrame()
{
	Destroy();
}

// Create the control
LTBOOL CLTGUIFrame::Create (	HTEXTURE hFrame, uint16 nWidth, uint16 nHeight, LTBOOL bSimpleStretch)
{
	if (!hFrame || !nHeight || !nWidth) return LTFALSE;

	m_hFrame	= hFrame;

	m_nBaseWidth = nWidth;
	m_nBaseHeight = nHeight;
	m_nWidth = nWidth;
	m_nHeight = nHeight;

	m_bSimpleStretch = bSimpleStretch;
	m_argbColor = 0xFFFFFFFF;

	InitPolies();
    m_bCreated=LTTRUE;

    return LTTRUE;
}

LTBOOL CLTGUIFrame::Create (uint32 argbColor, uint16 nWidth, uint16 nHeight)
{
	if (!nHeight || !nWidth) return LTFALSE;

	m_hFrame	= LTNULL;

	m_nBaseWidth = nWidth;
	m_nBaseHeight = nHeight;
	m_nWidth = nWidth;
	m_nHeight = nHeight;

	m_bSimpleStretch = LTTRUE;

	m_argbColor = argbColor;


	InitPolies();
    m_bCreated=LTTRUE;

    return LTTRUE;

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
			g_pDrawPrim->SetRGBA(&m_Border[f],m_BorderColor);

		// draw our frames
		g_pDrawPrim->DrawPrim(m_Border,4);
	}


}

void CLTGUIFrame::SetBasePos ( LTIntPt pos )
{ 
	CLTGUICtrl::SetBasePos(pos);
	ScalePolies();
}

void CLTGUIFrame::SetScale(float fScale)
{
	CLTGUICtrl::SetScale(fScale);
	m_nWidth = (uint16)((float)m_nBaseWidth * m_fScale);
	m_nHeight = (uint16)((float)m_nBaseHeight * m_fScale);
	ScalePolies();

}

void CLTGUIFrame::SetTextureScale(float fScale)
{
	m_fTextureScale = fScale;
	ScalePolies();

}

void CLTGUIFrame::InitPolies()
{
	ScalePolies();

	if (m_bSimpleStretch)
	{
		SetupQuadUVs(m_Poly[0], m_hFrame, 0.0f, 0.0f, 1.0f, 1.0f);
		g_pDrawPrim->SetRGBA(&m_Poly[0],m_argbColor);
	}
	else
	{
		float uv[4] = {0.0f, 0.25f, 0.75f, 1.0f};


		for (int r = 0; r < 3; r++)
		{
			for (int c = 0; c < 3; c++)
			{
				int i = r*3+c;
				g_pDrawPrim->SetRGBA(&m_Poly[i],0xFFFFFFFF);
				SetupQuadUVs(m_Poly[i], m_hFrame, uv[c], uv[r], uv[c+1] - uv[c], uv[r+1] - uv[r]);
			}
		}
	}


}

void CLTGUIFrame::ScalePolies()
{
	uint32 tw,th;
	float fx = (float)m_pos.x;
	float fy = (float)m_pos.y;

	if (m_bSimpleStretch)
	{
		float fw = (float)m_nWidth;
		float fh = (float)m_nHeight;
		g_pDrawPrim->SetXYWH(&m_Poly[0],fx,fy,fw,fh);
	}
	else
	{
		if (!m_hFrame) return;
		g_pTexInterface->GetTextureDims(m_hFrame,tw,th);
		float fw = (float)tw;
		float fh = (float)th;

		float x[4] = {fx, fx+fw*0.25f, (fx+m_nWidth) - fw*0.25f, (fx+m_nWidth) };
		float y[4] = {fy, fy+fh*0.25f, (fy+m_nHeight) - fh*0.25f, (fy+m_nHeight) };


		for (int r = 0; r < 3; r++)
		{
			for (int c = 0; c < 3; c++)
			{
				int i = r*3+c;
				g_pDrawPrim->SetXY4(&m_Poly[i],x[c],y[r],x[c+1],y[r],x[c+1],y[r+1],x[c],y[r+1]);
			}
		}
	}

	float BorderW = ((float)m_nBorderWidth * m_fScale);

	//top
	fx = (float)m_pos.x - BorderW;
	fy = (float)m_pos.y - BorderW;
	float fw = (float)m_nWidth + BorderW;
	float fh = BorderW;
	g_pDrawPrim->SetXYWH(&m_Border[0],fx,fy,fw,fh);

	//right
	fx = (float)(m_pos.x + m_nWidth);
	fy = (float)m_pos.y - BorderW;
	fw = (float)BorderW;
	fh = (float)GetHeight() + BorderW;
	g_pDrawPrim->SetXYWH(&m_Border[1],fx,fy,fw,fh);

	//bottom
	fx = (float)m_pos.x;
	fy = (float)(m_pos.y + GetHeight());
	fw = (float)m_nWidth + BorderW;
	fh = BorderW;
	g_pDrawPrim->SetXYWH(&m_Border[2],fx,fy,fw,fh);

	//left
	fx = (float)m_pos.x - BorderW;
	fy = (float)m_pos.y;
	fw = (float)BorderW;
	fh = (float)GetHeight() + BorderW;
	g_pDrawPrim->SetXYWH(&m_Border[3],fx,fy,fw,fh);


}

void CLTGUIFrame::SetRenderState()
{
	g_pDrawPrim->SetTransformType(DRAWPRIM_TRANSFORM_SCREEN);
	g_pDrawPrim->SetZBufferMode(DRAWPRIM_NOZ); 
	g_pDrawPrim->SetClipMode(DRAWPRIM_NOCLIP);
	g_pDrawPrim->SetFillMode(DRAWPRIM_FILL);
	g_pDrawPrim->SetColorOp(DRAWPRIM_MODULATE);
	g_pDrawPrim->SetAlphaTestMode(DRAWPRIM_NOALPHATEST);
	g_pDrawPrim->SetAlphaBlendMode(DRAWPRIM_BLEND_MOD_SRCALPHA);
		
}


void CLTGUIFrame::SetSize(uint16 nWidth, uint16 nHeight)
{
	m_nBaseWidth = nWidth;
	m_nBaseHeight = nHeight;
	m_nWidth = (uint16)((float)m_nBaseWidth * m_fScale);
	m_nHeight = (uint16)((float)m_nBaseHeight * m_fScale);
	ScalePolies();
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
	m_hFrame	= LTNULL;
	m_argbColor = argbColor;
	g_pDrawPrim->SetRGBA(&m_Poly[0],m_argbColor);
	ScalePolies();
}

// Sets the width of the frames's border, set to 0 to not show the frame
void CLTGUIFrame::SetBorder(uint8 nBorderWidth, uint32 nBorderColor)
{
	m_nBorderWidth = nBorderWidth;
	m_BorderColor = nBorderColor;
	ScalePolies();
}
