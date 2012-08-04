// ----------------------------------------------------------------------- //
//
// MODULE  : HUDBar.h
//
// PURPOSE : Definition of "Bar" HUD component
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "HUDMgr.h"
#include "InterfaceMgr.h"
#include "HUDBar.h"


CHUDBar::CHUDBar()
{
	m_bInitted = false;
	m_Bar = NULL;
}


void CHUDBar::Init(HTEXTURE hBar)
{
	ASSERT(hBar);
	if (!hBar) return;
	m_Bar = hBar;


	float u[4] = {0.0f, 0.25f, 0.75f, 1.0f};
	float v[3] = {0.0f, 0.5f, 1.0f};


	for (int row = 0; row < 2; row++)
	{
		for (int col = 0; col < 3; col++)
		{
			int i = row*3+col;
			g_pDrawPrim->SetRGBA(&m_Poly[i],0xFFFFFFFF);
			SetupQuadUVs(m_Poly[i], hBar, u[col],v[row], u[col+1] - u[col],v[row+1] - v[row]);
		}
	}

	m_bInitted = true;

}

void CHUDBar::Render()
{
	if (!m_bInitted) return;
	
	g_pDrawPrim->SetTexture(m_Bar);
	g_pDrawPrim->DrawPrim(m_Poly,6);
}


void CHUDBar::Update(float x,float y, float fillW, float maxW, float h)
{
	float capW = h/2.0f;
	float barW = maxW - (2.0f * capW);

	float barLeft = x + capW;
	float barRight = barLeft + barW;	

	if (fillW < capW)
	{
		//width of bar is lass than endcap width

		//draw partial left endcap
		float capUw = (fillW/capW) / 4.0f;
		g_pDrawPrim->SetXYWH(&m_Poly[0],x,y,fillW,h);
		SetupQuadUVs(m_Poly[0], m_Bar, 0.0f,0.0f,capUw,0.5f);
		g_pDrawPrim->SetXYWH(&m_Poly[3],x+fillW,y,(capW-fillW),h);
		SetupQuadUVs(m_Poly[3], m_Bar, capUw,0.5f,0.25f-capUw,0.5f);

		//hide full armor bars and right endcap
		g_pDrawPrim->SetXYWH(&m_Poly[1],-1.0f,-1.0f,0.0f,0.0f);
		g_pDrawPrim->SetXYWH(&m_Poly[2],-1.0f,-1.0f,0.0f,0.0f);

		//draw empty armor bars and right endcap
		g_pDrawPrim->SetXYWH(&m_Poly[4],barLeft,y, barW,h);
		g_pDrawPrim->SetXYWH(&m_Poly[5],barRight,y, capW,h);
		SetupQuadUVs(m_Poly[5], m_Bar, 0.75f,0.5f,0.25f,0.5f);
	}
	else
	{
		//draw full left endcap
		g_pDrawPrim->SetXYWH(&m_Poly[0],x,y,capW,h);
		SetupQuadUVs(m_Poly[0], m_Bar, 0.0f,0.0f,0.25f,0.5f);
		g_pDrawPrim->SetXYWH(&m_Poly[3],-1.0f,-1.0f,0.0f,0.0f);
		SetupQuadUVs(m_Poly[3], m_Bar, 0.0f,0.5f,0.25f,0.5f);

		if (fillW < (capW + barW) )
		{
			x += fillW;
			
			//draw partial bar
			g_pDrawPrim->SetXYWH(&m_Poly[1],barLeft,y,x-barLeft,h);
			g_pDrawPrim->SetXYWH(&m_Poly[4],x,y,barRight-x,h);

			//draw empty right endcap
			g_pDrawPrim->SetXYWH(&m_Poly[2],-1.0f,-1.0f,0.0f,0.0f);
			g_pDrawPrim->SetXYWH(&m_Poly[5],barRight,y, capW,h);
			SetupQuadUVs(m_Poly[5], m_Bar, 0.75f,0.5f,0.25f,0.5f);
		}
		else
		{
			//draw full bar
			g_pDrawPrim->SetXYWH(&m_Poly[1],barLeft,y, barW,h);
			g_pDrawPrim->SetXYWH(&m_Poly[4],-1.0f,-1.0f,0.0f,0.0f);

			if (fillW < maxW)
			{
				//draw partial right endcap
				float partW = maxW - fillW;
				float capUw = (partW/capW) / 4.0f;
				g_pDrawPrim->SetXYWH(&m_Poly[2],barRight,y, capW-partW,h);
				SetupQuadUVs(m_Poly[2], m_Bar, 0.75f,0.0f,0.25f-capUw,0.5f);
				g_pDrawPrim->SetXYWH(&m_Poly[5],x+fillW,y,partW,h);
				SetupQuadUVs(m_Poly[5], m_Bar, 1.0f-capUw,0.5f,capUw,0.5f);
			}
			else
			{
				//draw full right endcap
				g_pDrawPrim->SetXYWH(&m_Poly[2],barRight,y, capW,h);
				SetupQuadUVs(m_Poly[2], m_Bar, 0.75f,0.0f,0.25f,0.5f);
				g_pDrawPrim->SetXYWH(&m_Poly[5],-1.0f,-1.0f,0.0f,0.0f);
			}

		}
	}
}
