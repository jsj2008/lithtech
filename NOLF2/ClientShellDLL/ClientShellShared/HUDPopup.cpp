// ----------------------------------------------------------------------- //
//
// MODULE  : HUDPopup.cpp
//
// PURPOSE : Implementation of CHUDPopup to display popups
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//

	#include "stdafx.h"
	#include "HUDMgr.h"
	#include "InterfaceResMgr.h"
	#include "PopupMgr.h"
	#include "HUDPopup.h"



// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CHUDPopup::CHUDPopup
//
//  PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CHUDPopup::CHUDPopup()
:	CHUDItem		( )
{
	m_UpdateFlags	= kHUDFrame;
	m_bVisible		= LTFALSE;	
	m_bColorOverride = LTFALSE;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CHUDPopup::Init
//
//  PURPOSE:	Initialize the popup...
//
// ----------------------------------------------------------------------- //

LTBOOL CHUDPopup::Init()
{
	m_Text.Create(" ",0,0,g_pInterfaceResMgr->GetFont(0),8,LTNULL);
	m_Frame.Create(g_pInterfaceResMgr->GetTexture("interface\\menu\\sprtex\\frame.dtx"),200,320,LTTRUE);

	return LTTRUE;

}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CHUDPopup::Term
//
//  PURPOSE:	Destroy thyself...
//
// ----------------------------------------------------------------------- //

void CHUDPopup::Term()
{
	m_Frame.Destroy();
	m_Text.Destroy();
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CHUDPopup::Hide
//
//  PURPOSE:	NONE
//
// ----------------------------------------------------------------------- //

void CHUDPopup::Hide( )
{
	m_bVisible = LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CHUDPopup::Render
//
//  PURPOSE:	Draw the popup...
//
// ----------------------------------------------------------------------- //

void CHUDPopup::Render()
{
	if (!m_bVisible) return;
	m_Frame.Render();
	m_Text.Render();
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CHUDPopup::Update
//
//  PURPOSE:	Update the popup...
//
// ----------------------------------------------------------------------- //

void CHUDPopup::Update()
{
	// Sanity checks...
	if( !IsVisible() ) return;

	if( m_fScale != g_pInterfaceResMgr->GetXRatio() )
		SetScale( g_pInterfaceResMgr->GetXRatio() );

}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CHUDPopup::SetScale
//
//  PURPOSE:	Set the items scale...
//
// ----------------------------------------------------------------------- //

void CHUDPopup::SetScale(float fScale)
{
	m_Frame.SetScale( fScale );
	m_Text.SetScale( fScale );

	m_fScale = fScale;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CHUDPopup::Show
//
//  PURPOSE:	Display the popup with the passed in text...
//
// ----------------------------------------------------------------------- //

void CHUDPopup::Show( uint8 nPopupID, const char *pText )
{
	POPUP* pPopup = g_pPopupMgr->GetPopup(nPopupID);
	if (!pPopup) return;

	CUIFont *pFont = g_pInterfaceResMgr->GetFont(pPopup->nFont);
	LTIntPt pos( (640 - pPopup->sSize.x) / 2, (480 - pPopup->sSize.y) / 2 );

	m_Frame.SetFrame(g_pInterfaceResMgr->GetTexture(pPopup->szFrame));
	m_Frame.SetSize(pPopup->sSize.x,pPopup->sSize.y);
	m_Frame.SetBasePos(pos);
	m_Frame.SetScale(g_pInterfaceResMgr->GetXRatio());

	pos.x += pPopup->sTextOffset.x;
	pos.y += pPopup->sTextOffset.y;

	m_Text.SetScale(1.0f);

	m_Text.SetString( (pText ? pText : "") );

	m_Text.SetFont(pFont,pPopup->nFontSize);

	if( !m_bColorOverride )
		m_Text.SetColors(pPopup->argbTextColor,pPopup->argbTextColor,pPopup->argbTextColor);

	m_bColorOverride = LTFALSE;
	
	m_Text.SetFixedWidth(pPopup->nTextWidth);
	m_Text.SetBasePos(pos);
	m_Text.SetScale(g_pInterfaceResMgr->GetXRatio());

	m_bVisible = LTTRUE;
}