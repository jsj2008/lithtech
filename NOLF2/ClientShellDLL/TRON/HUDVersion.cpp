// ----------------------------------------------------------------------- //
//
// MODULE  : HUDVersion.cpp
//
// PURPOSE : HUDItem to display Jet's version number
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "TronHUDMgr.h"
#include "TRONPlayerStats.h"
#include "TronInterfaceMgr.h"

//******************************************************************************************
//**
//** HUD version number display
//**
//******************************************************************************************


CHUDVersion::CHUDVersion()
{
	LTIntPt kNullPt(0,0);

	m_UpdateFlags = kHUDVersion;
	m_pString = LTNULL;
	m_BasePos = kNullPt;
	m_fUpdateTime = 1.0f;
	m_vColor = LTVector(1.0f,1.0f,1.0f);	
}


LTBOOL CHUDVersion::Init()
{
	UpdateLayout();
	return LTTRUE;
}

void CHUDVersion::Term()
{
	if (m_pString)
	{
		g_pLTClient->GetFontManager()->DestroyPolyString(m_pString);
		m_pString = LTNULL;
	}
}

void CHUDVersion::Render()
{
	if (m_pString)
	{
		// If there was a recent change to the version number, this allows us to
		// set a time during which the version number flashes or something.
/*
		if (m_fTimeRemaining)
		{
			// Set the color here.
			m_pString->SetColor(SETRGB(m_vColor.x, m_vColor.y, m_vColor.z));
			// FIXME reduce m_fTimeRemaining by some amount
		}
		else
*/
		{
//			m_pString->SetColor(SETRGB(m_vColor.x, m_vColor.y, m_vColor.z));
		}

		// Finally...render the string
		m_pString->Render();
	}

}

void CHUDVersion::Update()
{
	// build the new string
	uint16 newVersion;
	g_pTronPlayerStats->GetJetVersion(newVersion);

	// Start some visual effect to denote that there was a change
	// Set the time for how long the visual effect lasts based upon which of the
	// three digits of the version number got updated.
	m_fTimeRemaining = m_fUpdateTime;

	// do the sprintf here!
	int major = newVersion / 100;
	newVersion -= 100 * major;
	int minor = newVersion / 10;
	newVersion -= 10 * minor;
	
	char szVersionStr[64];		// buffer to create the new string
	sprintf(szVersionStr,"Jet v%d.%d.%d",major,minor,newVersion);

	if (m_pString)
	{
		// String exists, just setText
		m_pString->SetText(szVersionStr);
	}
	else
	{
/*
	char *pTag = "ChatInput";
	m_BasePos = g_pLayoutMgr->GetPoint(pTag,"BasePos");
	uint8 nFont = (uint8)g_pLayoutMgr->GetInt(pTag,"Font");
	CUIFont* pFont	= g_pInterfaceResMgr->GetFont(nFont);
*/
		CUIFont * pFont = g_pInterfaceResMgr->GetFont(0);
		float x = (float)m_BasePos.x * g_pInterfaceResMgr->GetXRatio();
		float y = (float)m_BasePos.y * g_pInterfaceResMgr->GetYRatio();

		m_pString = g_pLTClient->GetFontManager()->CreatePolyString(pFont, szVersionStr, x, y);
		m_pString->SetColor(0xFF2020FF);
// FIXME make this additive blend work!
//		m_pString->SetBlendMode(DRAWPRIM_BLEND_ADD);
	}

}

void CHUDVersion::UpdateLayout()
{
	int nCurrentLayout = GetConsoleInt("HUDLayout",0);

	m_BasePos.x = 10;
	m_BasePos.y = 10;

	m_fUpdateTime = 1.0f;

	m_vColor.x = 255;
	m_vColor.y = 0;
	m_vColor.z = 0;

//	m_BasePos		= g_pLayoutMgr->GetJetVersionPos(nCurrentLayout);
//	m_fUpdateTime   = g_pLayoutMgr->GetVersionUpdateTime(nCurrentLayout);
//	m_vColor		= g_pLayoutMgr->GetVersionColor(nCurrentLayout);

/*
	char *pTag = "ChatInput";
	m_BasePos = g_pLayoutMgr->GetPoint(pTag,"BasePos");
	uint8 nFont = (uint8)g_pLayoutMgr->GetInt(pTag,"Font");
	CUIFont* pFont	= g_pInterfaceResMgr->GetFont(nFont);
*/
}

