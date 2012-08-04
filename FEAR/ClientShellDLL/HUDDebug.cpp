// ----------------------------------------------------------------------- //
//
// MODULE  : HUDDebug.cpp
//
// PURPOSE : HUDItem to display debug information
//
// (c) 2001-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "HUDDebug.h"
#include "HUDMgr.h"
#include "sys/win/mpstrconv.h"
#include "PlayerCamera.h"
#include "LTEulerAngles.h"
#include "PerformanceTest.h"
#include "EngineTimer.h"

// TEMP
extern VarTrack g_vtApplyWorldOffset;
// END TEMP


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CHUDDebug::CHUDDebug
//
//  PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

CHUDDebug::CHUDDebug()
{
	m_eLevel = kHUDRenderText;
	m_UpdateFlags = kHUDFrame;
	m_bShowPlayerPos = false;
	m_bShowCamPosRot = false;
}
	

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CHUDDebug::Init()
//
//  PURPOSE:	
//
// ----------------------------------------------------------------------- //

bool CHUDDebug::Init()
{

	UpdateLayout();
	ScaleChanged();
	EnableFade(false);
	
	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CHUDDebug::Term()
//
//  PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CHUDDebug::Term()
{
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CHUDDebug::Render()
//
//  PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CHUDDebug::Render()
{
	if( !m_Text.IsEmpty() )
	{
		m_Text.Render();
	}

	RenderCameraInformationDebugStrings();
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CHUDDebug::Update()
//
//  PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CHUDDebug::Update()
{
	UpdateCameraInformationDebugInfo();
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CHUDDebug::ScaleChanged()
//
//  PURPOSE:	Update display to fit new screen resolution
//
// ----------------------------------------------------------------------- //

void CHUDDebug::ScaleChanged()
{
	CHUDItem::ScaleChanged();
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CHUDDebug::UpdateLayout()
//
//  PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CHUDDebug::UpdateLayout()
{
	//if we haven't initialized our layout info
	if (!m_hLayout)
	{
		m_hLayout = g_pLayoutDB->GetHUDRecord("HUDDebug");
	}

	CHUDItem::UpdateLayout();

	m_nTextWidth = ( int16 )g_pLayoutDB->GetInt32(m_hLayout,LDB_HUDAddInt,0);

	CFontInfo Font(g_pLayoutDB->GetHelpFont(),g_pLayoutDB->GetHelpSize());

	// Clear the strings.
	for (int i=0; i < kMaxDebugStrings; i++)
	{
		m_LeftDebugString[i].SetColor(m_cTextColor);
		m_RightDebugString[i].SetColor(m_cTextColor);
		m_LeftDebugString[i].SetFont(Font);
		m_RightDebugString[i].SetFont(Font);
	}

}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CHUDDebug::UpdateCameraInformationDebugInfo()
//
//  PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CHUDDebug::UpdateCameraInformationDebugInfo()
{
	char buf[100];

	int nMaxLeft = -1, nMaxRight = -1;

	//determine the offset we should use on the positions
	LTVector vOffset(0, 0, 0);
	const char* pszOffsetDescription = "(Actual)";

	if((uint32)g_vtApplyWorldOffset.GetFloat(1.0f))
	{
		g_pLTClient->GetSourceWorldOffset(vOffset);
		pszOffsetDescription = "(Level)";
	}

	// Check to see if we should show the player position...

    HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();
	if (m_bShowPlayerPos && hPlayerObj)
	{
        LTVector vPos;
		g_pLTClient->GetObjectPos(hPlayerObj, &vPos);

		//handle the shift from the current world to the source world
		vPos += vOffset;

		LTSNPrintF(buf, LTARRAYSIZE(buf), "Player Position %s: %6.0f, %6.0f, %6.0f  ", pszOffsetDescription, vPos.x, vPos.y, vPos.z);

		SetCameraInformationDebugString(buf, eDSBottomRight, 0);
		nMaxRight = LTMAX(nMaxRight, 0);
	}

	if (m_bShowCamPosRot)
	{
		LTVector vPos = g_pPlayerMgr->GetPlayerCamera()->GetCameraPos( );

		//handle the shift from the current world to the source world
		vPos += vOffset;

		// Convert pitch and yaw to the same units used by WorldEdit...
		EulerAngles EA = Eul_FromQuat( g_pPlayerMgr->GetPlayerCamera( )->GetCameraRotation( ), EulOrdYXZr );
		float fYaw		= EA.x;
		float fPitch	= EA.y;
		float fRoll		= EA.z;

		float fYawDeg = fmodf( RAD2DEG(fYaw), 360.0f );
		float fPitchDeg = fmodf( RAD2DEG(fPitch), 360.0f );
		float fRollDeg = fmodf( RAD2DEG(fRoll), 360.0f );

		LTSNPrintF(buf, LTARRAYSIZE(buf), "Camera Position %s: %6.0f %6.0f %6.0f  ", pszOffsetDescription, vPos.x, vPos.y, vPos.z);
		SetCameraInformationDebugString(buf, eDSBottomRight, 0);

		LTSNPrintF(buf, LTARRAYSIZE(buf), "Camera Yaw: %6.0f  ", fYawDeg);
		SetCameraInformationDebugString(buf, eDSBottomRight, 2);

		LTSNPrintF(buf, LTARRAYSIZE(buf), "Camera Pitch: %6.0f  ", fPitchDeg);
		SetCameraInformationDebugString(buf, eDSBottomRight, 1);

		LTSNPrintF(buf, LTARRAYSIZE(buf), "Camera Roll: %6.0f  ", fRoll);
		SetCameraInformationDebugString(buf, eDSBottomRight, 1);
		nMaxRight = LTMAX(nMaxRight, 2);
	}


	// Check to see if we are in spectator or invisible mode, both only available for debugging.
	if( g_pCheatMgr->GetCheatInfo(CHEAT_INVISIBLE).bActive || g_pCheatMgr->GetCheatInfo(CHEAT_CLIP).bActive )
	{
		SetCameraInformationDebugString(g_pCheatMgr->GetCheatInfo(CHEAT_CLIP).bActive ? 
			"SPECTATOR MODE" : "GHOST MODE", eDSBottomLeft, 0);
		nMaxLeft = LTMAX(nMaxLeft, 0);

#ifndef _DEMO
		LTVector vPos = g_pPlayerMgr->GetPlayerCamera()->GetCameraPos( );

		//handle the shift from the current world to the source world
		vPos += vOffset;

		LTSNPrintF(buf, LTARRAYSIZE(buf), "Camera Position %s: %6.0f %6.0f %6.0f", pszOffsetDescription, vPos.x, vPos.y, vPos.z);
		SetCameraInformationDebugString(buf, eDSBottomRight, 0);
		nMaxRight = LTMAX(nMaxRight, 0);
#endif // _DEMO
	}

	// Check to see if we are in performance test mode...

	CPerformanceTest* pTest = g_pGameClientShell->GetLastPerformanceTest();
	if (g_pGameClientShell->IsRunningPerformanceTest() && pTest)
	{
		pTest->Update(RealTimeTimer::Instance().GetTimerElapsedS());
	}

	// Clear the strings we didn't set
	// Note : If we clear all of them, it will reset the text, and re-create the associated texture.  So this
	// only clears the ones we didn't touch.
	for (int i=0; i < kMaxDebugStrings; i++)
	{
		if (i > nMaxLeft)
			m_LeftDebugString[i].SetText(NULL);
		if (i > nMaxRight)
			m_RightDebugString[i].SetText(NULL);
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CHUDDebug::SetCameraInformationDebugString()
//
//	PURPOSE:	Set the debug string (create it if necessary).
//
// --------------------------------------------------------------------------- //

void CHUDDebug::SetCameraInformationDebugString(char* strMessage, DSSL eLoc, uint8 nLine)
{
	if (!strMessage || strMessage[0] == '\0') return;
	if (nLine < 0 || nLine >= kMaxDebugStrings) return;

	
	CLTGUIString* pString = NULL;

	switch (eLoc)
	{
		case eDSBottomLeft :
		{
			pString = &m_LeftDebugString[nLine];
		}
		break;

		case eDSBottomRight :
		default :
		{
			pString = &m_RightDebugString[nLine];
			m_RightDebugString[nLine].SetAlignment(kRight);
		}
		break;
	}

	if (pString)
	{
		// Only reset the font and text if we've changed the message
		MPA2W wMsg(strMessage);
		if (LTStrCmp(pString->GetText(), wMsg.c_str()) != 0)
		{
			pString->SetText( wMsg.c_str() );
		}
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CHUDDebug::RenderCameraInformationDebugStrings()
//
//	PURPOSE:	Render all the debug strings
//
// --------------------------------------------------------------------------- //

void CHUDDebug::RenderCameraInformationDebugStrings()
{
    uint32 nScreenWidth = 0, nScreenHeight = 0;
    g_pLTClient->GetRenderer()->GetCurrentRenderTargetDims(nScreenWidth, nScreenHeight);

	float fy = 0.0f;
	float fYLeftOffset = 0.0f, fYRightOffset = 0.0f;
	float fScreenWidth  = float(nScreenWidth);
	float fScreenHeight = float(nScreenHeight);

	for (int i=0; i < kMaxDebugStrings; i++)
	{
		LTRect2n rExt;
		if (!m_LeftDebugString[i].IsEmpty())
		{
			if (!m_LeftDebugString[i].IsValid())
				m_LeftDebugString[i].CreateTexture();
			m_LeftDebugString[i].GetExtents(rExt);
			fYLeftOffset += (float)rExt.GetHeight();

			fy = fScreenHeight - fYLeftOffset;
			m_LeftDebugString[i].SetPos(LTVector2(0.0f,fy));
			m_LeftDebugString[i].Render();
		}

		if (!m_RightDebugString[i].IsEmpty())
		{
			if (!m_RightDebugString[i].IsValid())
				m_RightDebugString[i].CreateTexture();
			m_RightDebugString[i].GetExtents(rExt);
			fYRightOffset += (float)rExt.GetHeight();

			fy = fScreenHeight - fYRightOffset;
			m_RightDebugString[i].SetPos(LTVector2(fScreenWidth,fy));
			m_RightDebugString[i].Render() ;
		}
	}
}


void CHUDDebug::SetPerturbDebugString(const wchar_t* const pszDebugString)
{
	m_LeftDebugString[1].SetText(pszDebugString);
}
