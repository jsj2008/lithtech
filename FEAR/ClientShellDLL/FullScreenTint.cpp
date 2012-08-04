// ----------------------------------------------------------------------- //
//
// MODULE  : FullScreenTint.cpp
//
// PURPOSE : Implementation of FullScreenTint class
//
// CREATED : 6/22/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "FullScreenTint.h"
#include "iltrenderer.h"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFullScreenTint::CFullScreenTint()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

CFullScreenTint::CFullScreenTint()
{
	m_fAlpha = 1.0f;
	m_bOn = false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFullScreenTint::~CFullScreenTint()
//
//	PURPOSE:	Handle object destruction
//
// ----------------------------------------------------------------------- //

CFullScreenTint::~CFullScreenTint()
{
	Term();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFullScreenTint::SetAlpha()
//
//	PURPOSE:	Set the surface's alpha value
//
// ----------------------------------------------------------------------- //

void CFullScreenTint::SetAlpha(float fAlpha)
{
	m_fAlpha = (fAlpha < 0.0f ? 0.0f : (fAlpha > 1.0 ? 1.0f : fAlpha));
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFullScreenTint::Init()
//
//	PURPOSE:	Create the tint surface - black only (for now)
//
// ----------------------------------------------------------------------- //

void CFullScreenTint::Init()
{
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFullScreenTint::Term()
//
//	PURPOSE:	clean up 
//
// ----------------------------------------------------------------------- //

void CFullScreenTint::Term()
{
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFullScreenTint::Draw()
//
//	PURPOSE:	Handle drawing the tint
//
// ----------------------------------------------------------------------- //

void CFullScreenTint::Draw()
{
	if (!m_bOn || (m_fAlpha < 0.01f)) 
		return;

    uint32 dwWidth = 640, dwHeight = 480;
    g_pLTClient->GetRenderer()->GetCurrentRenderTargetDims(dwWidth, dwHeight);
    
	//setup a black quad and render that on the screen
	LT_POLYG4 Quad;
	DrawPrimSetXYWH(Quad, -0.5f, -0.5f, (float)dwWidth, (float)dwHeight);
	DrawPrimSetRGBA(Quad, 0x00, 0x00, 0x00, (uint8)(m_fAlpha * 255.0f));

	g_pLTClient->GetDrawPrim()->SetRenderMode(eLTDrawPrimRenderMode_Modulate_Translucent);
	
	g_pLTClient->GetDrawPrim()->DrawPrim(&Quad, 1);
}


