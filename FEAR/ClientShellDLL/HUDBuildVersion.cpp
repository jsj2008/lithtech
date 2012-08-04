// ----------------------------------------------------------------------- //
//
// MODULE  : HUDAmmo.cpp
//
// PURPOSE : HUDItem to display player ammo
//
// (c) 2001-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "HUDBuildVersion.h"
#include "HUDMgr.h"

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CHUDBuildVersion::CHUDBuildVersion
//
//  PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

CHUDBuildVersion::CHUDBuildVersion()
{
	m_eLevel = kHUDRenderText;
	m_UpdateFlags = kHUDFrame;
}

bool CHUDBuildVersion::Init()
{
	// Set up the layout to bind textures, etc.

	UpdateLayout();
	ScaleChanged();

	// Set the string (this won't change during a run).

	wchar_t szVersion[256];
	g_pVersionMgr->GetDisplayVersion( szVersion, LTARRAYSIZE(szVersion) ); 
	m_Text.SetText( szVersion );

	return true;
}

void CHUDBuildVersion::Render()
{
	// Only render this element in non-final builds.  This element really doesn't
	// even need to be created for final builds.
#ifndef _FINAL 
	m_Text.Render();
#endif _FINAL 
}

void CHUDBuildVersion::UpdateLayout()
{
	//if we haven't initialized our layout info

	if (!m_hLayout)
	{
		m_hLayout = g_pLayoutDB->GetHUDRecord("HUDBuildVersion");
	}

	CHUDItem::UpdateLayout();
}
