// ----------------------------------------------------------------------- //
//
// MODULE  : HUDInventory.cpp
//
// PURPOSE : generic HUD display for an inventory item
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "HUDInventory.h"
#include "HUDMgr.h"

// ----------------------------------------------------------------------- //
// Contructor
// ----------------------------------------------------------------------- //
CHUDInventory::CHUDInventory() : 
	m_bDraw(false),
	m_nCount(0),
	m_nThreshold(0)
{
	m_Text.SetDropShadow(2);

}

// ----------------------------------------------------------------------- //
// Draw the item to the screen
// ----------------------------------------------------------------------- //
void CHUDInventory::Render()
{
	if (!m_bDraw) return;


	SetRenderState();

	if (m_hIconTexture)
	{
		g_pDrawPrim->SetTexture(m_hIconTexture);
		g_pDrawPrim->DrawPrim(&m_IconPoly,1);
	}
	m_Text.Render();

}

// ----------------------------------------------------------------------- //
// Position the item on screen
// ----------------------------------------------------------------------- //
void CHUDInventory::SetBasePos(const LTVector2n& pos)
{
	m_vBasePos = pos;

	LTVector2 vPos;
	vPos.x = (float)(m_vBasePos.x + m_vTextOffset.x);
	vPos.y = (float)(m_vBasePos.y + m_vTextOffset.y);
	m_Text.SetPos(vPos);

	float x = (float)(m_vBasePos.x + m_vIconOffset.x);
	float y = (float)(m_vBasePos.y + m_vIconOffset.y);
	float w = (float)m_vIconSize.x;
	float h = (float)m_vIconSize.y;
	DrawPrimSetXYWH(m_IconPoly,x,y,w,h);
}
