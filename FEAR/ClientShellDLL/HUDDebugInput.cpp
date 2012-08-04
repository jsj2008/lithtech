// ----------------------------------------------------------------------- //
//
// MODULE  : HUDDebugInput.cpp
//
// PURPOSE : Enter Debug input
//
// CREATED : 01/06/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "stdafx.h"
#include "HUDDebugInput.h"
#include "HUDMessageQueue.h"
#include "HUDMgr.h"
#include "HUDDebug.h"
#include "InterfaceMgr.h"
#include "GameClientShell.h"
#include "MsgIds.h"
#include "ClientSoundMgr.h"
#include "MissionMgr.h"
#include "BindMgr.h"
#include "ltfilewrite.h"
#include "iltfilemgr.h"
#include "ltfileoperations.h"
#include "WinUtil.h"
#include "CheatMgr.h"

#include "sys/win/mpstrconv.h"
#include <time.h>

extern VarTrack g_vtApplyWorldOffset;


//******************************************************************************************
//**
//** HUD Debug display
//**
//******************************************************************************************

CHUDDebugInput::CHUDDebugInput()
{
	m_UpdateFlags = kHUDNone;
	m_bVisible = false;
	m_eLevel = kHUDRenderText;

	for( uint32 nElement = 0; nElement < kMaxDebugLength; ++nElement )
		m_szDebugStr[nElement] = '\0';
}


bool CHUDDebugInput::Init()
{

	UpdateLayout();
	ScaleChanged();

	wchar_t szTemp[64];
	LTSNPrintF(szTemp,LTARRAYSIZE(szTemp),L"%s: ",LoadString("IDS_DEBUGMSG"));
	m_Text.SetText(szTemp);

	return true;
}

void CHUDDebugInput::Term()
{
	m_EditCtrl.Destroy();

}

void CHUDDebugInput::Render()
{

	if (!m_bVisible) return;
//	SetRenderState();

	m_Text.Render();
	m_EditCtrl.Render();


}

void CHUDDebugInput::Update()
{
}

void CHUDDebugInput::ScaleChanged()
{
	CHUDItem::ScaleChanged();

	LTVector2n nPos = m_vBasePos;
	g_pInterfaceResMgr->ScaleScreenPos(nPos);
	m_EditCtrl.SetBasePos(nPos);
	
	LTVector2n sz(m_nInputWidth,m_sTextFont.m_nHeight);
	sz.x = int32(g_pInterfaceResMgr->GetXRatio() * float(sz.x));
	m_EditCtrl.SetSize(sz);
}

void CHUDDebugInput::OnExitWorld() 
{
	if (m_bVisible)
	{
		m_bVisible = false;

		m_EditCtrl.Show(false);
		m_EditCtrl.UpdateData(true);

		g_pGameClientShell->SetInputState(true);
		CBindMgr::GetSingleton().ClearAllCommands();
	}
}

void CHUDDebugInput::Show(bool bShow) 
{
	m_bVisible = bShow;

	m_EditCtrl.Show(bShow);
	g_pGameClientShell->SetInputState(!bShow);
	CBindMgr::GetSingleton().ClearAllCommands();

	if (bShow)
	{
		m_szDebugStr[0] = 0;
	
		Update();
		if (g_pHUDDebug)
		{
			g_pHUDDebug->ShowPlayerPos(true);
		}

	}
	else
	{
		if (g_pHUDDebug && !g_pCheatMgr->GetCheatInfo(CHEAT_POS).bActive )
		{
			g_pHUDDebug->ShowPlayerPos(false);
		}
	}
	
	// Pause the server in single player while Debugting... 

	if( !IsMultiplayerGameClient() )
	{
		g_pGameClientShell->PauseGame( !!bShow, true );
	}

	m_EditCtrl.UpdateData(!bShow);

}

void CHUDDebugInput::UpdateLayout()
{
	//if we haven't initialized our layout info
	if (!m_hLayout)
	{
		m_hLayout = g_pLayoutDB->GetHUDRecord("HUDChatInput");
	}

	CHUDItem::UpdateLayout();

	m_nInputWidth = (uint32)g_pLayoutDB->GetInt32(m_hLayout,LDB_HUDAddInt,0);


	CLTGUIEditCtrl_create ecs;
	ecs.bUseCaret = true;
	ecs.nMaxLength = kMaxDebugLength;
	ecs.rnBaseRect.Right() = m_nInputWidth;
	ecs.rnBaseRect.Bottom() = m_sTextFont.m_nHeight;
	ecs.argbCaretColor = m_cTextColor;
	ecs.pszValue = m_szDebugStr;

	m_EditCtrl.Create(g_pLTClient,m_sTextFont,ecs);
	m_EditCtrl.SetColor(m_cTextColor);

}

bool CHUDDebugInput::HandleKeyDown(int key, int rep)
{
// XENON: Currently disabled in Xenon builds
#if !defined(PLATFORM_XENON)

	switch (key)
	{
	case VK_ESCAPE:
	{
		Show(false);
		return true;
	} break;

	case VK_RETURN:
	{
		Send();
		return true;
	} break;

	default:
		return m_EditCtrl.HandleKeyDown(key,rep);
	}

#else // PLATFORM_XENON
	return false;
#endif // PLATFORM_XENON

}

bool CHUDDebugInput::HandleChar(wchar_t c)
{
	return m_EditCtrl.HandleChar(c);
}

void CHUDDebugInput::Send()
{
	char szPath[MAX_PATH*2];
	char szTemp[MAX_PATH];
	char szShotname[MAX_PATH + 1];

	LTFileOperations::GetUserDirectory(szPath, LTARRAYSIZE(szPath));
	LTStrCat(szPath,"DebugLog",LTARRAYSIZE(szPath));
	LTStrCat(szPath,FILE_PATH_SEPARATOR,LTARRAYSIZE(szPath));

	LTStrCpy(szTemp,g_pMissionMgr->GetCurrentWorldName(),LTARRAYSIZE(szTemp));

	char* szWorld = strrchr(szTemp,FILE_PATH_SEPARATOR[0]);
	if (szWorld)
	{
		szWorld++;
	}
	else
	{
		szWorld = szTemp;
	}

	LTStrCat(szPath,szWorld,LTARRAYSIZE(szPath));

	if (!CWinUtil::DirExist(szPath))
	{
		CWinUtil::CreateDir(szPath);
	}

	LTStrCat(szPath,FILE_PATH_SEPARATOR,LTARRAYSIZE(szPath));


	//determine a filename for this. This should start with the screenshot console variable and be
	//numbered sequentially

	//an upper cap to make sure that we don't attempt to create screenshots indefinitely, which could
	//potentially be caused by some file access issues
	static const uint32 knMaxScreenshotAttempts = 1000;

	for(uint32 nCurrScreenshotIndex = 0; nCurrScreenshotIndex < knMaxScreenshotAttempts; nCurrScreenshotIndex++)
	{
		//build up the filename to use for this screenshot
		
		LTSNPrintF(szShotname, LTARRAYSIZE(szShotname), "Screenshot%03d.jpg", nCurrScreenshotIndex);

		char pszFilename[MAX_PATH];
		LTSNPrintF(pszFilename, LTARRAYSIZE(pszFilename), "DebugLog%s%s%s%s",  FILE_PATH_SEPARATOR,szWorld,FILE_PATH_SEPARATOR,szShotname);

		// get the user path to the screenshot file, as we need this to check if the file exists.
		char pszUserFilename[MAX_PATH];
		g_pLTClient->FileMgr()->GetAbsoluteUserFileName( pszFilename, pszUserFilename, LTARRAYSIZE( pszUserFilename ) );

		//see if this file already exists - if it does, continue searching for an unused file
		if ( !LTFileOperations::FileExists( pszUserFilename ) )
		{
			//the filename doesn't exist, so go ahead and try to make the screenshot
			if(g_pLTClient->GetRenderer()->MakeScreenShot(pszUserFilename) == LT_OK)
			{
				g_pLTClient->CPrint("Successfully created screenshot %s", pszUserFilename);
			}
			else
			{
				g_pLTClient->CPrint("Failed to create screenshot %s", pszUserFilename);
			}
			break;
		}
	}

	//report overflow
	if(nCurrScreenshotIndex >= knMaxScreenshotAttempts)
	{
		g_pLTClient->CPrint("Unable to create screenshot. Please make sure that the directory is readable and that there are less than %d screenshots", knMaxScreenshotAttempts);
	}


	Show(false);

	// Ignore empty messages.
	if( !m_szDebugStr[0] )
		return;

	char szMsg[256];

	static bool bUseTime = false;

	if (bUseTime)
	{
		time_t tmSys;
		time( &tmSys );

		struct tm* pTimeDate = NULL;
		pTimeDate = localtime (&tmSys);
		if (pTimeDate)
		{
			LTSNPrintF(szMsg, LTARRAYSIZE( szMsg ), "<tr><td>%02d/%02d/%02d %02d:%02d:%02d</td>", pTimeDate->tm_mon + 1, pTimeDate->tm_mday, (pTimeDate->tm_year + 1900) % 100, pTimeDate->tm_hour, pTimeDate->tm_min, pTimeDate->tm_sec);
		}
	}
	else
	{
		LTSNPrintF(szMsg, LTARRAYSIZE( szMsg ), "<tr>");
	}


	//determine the offset we should use on the positions
	LTVector vOffset(0, 0, 0);
	g_pLTClient->GetSourceWorldOffset(vOffset);
	HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();
	if (hPlayerObj)
	{
		char buf[256];
		LTVector vPos;
		g_pLTClient->GetObjectPos(hPlayerObj, &vPos);

		//handle the shift from the current world to the source world
		vPos += vOffset;

		LTSNPrintF(buf, LTARRAYSIZE(buf), "<td>(pos %0.0f %0.0f %0.0f)</td>", vPos.x, vPos.y, vPos.z);
		LTStrCat(szMsg,buf,LTARRAYSIZE(szMsg));

	}

	LTSNPrintF(szTemp,LTARRAYSIZE(szTemp),"<td><a href=\"%s\">%s</a></td>",szShotname,MPW2A(m_szDebugStr).c_str());

	LTStrCat(szMsg,szTemp,LTARRAYSIZE(szMsg));

	LTStrCat(szMsg,"</tr>\r\n",LTARRAYSIZE(szMsg));


	CLTFileWrite cFileWrite;

	LTStrCat( szPath, "debuglog.htm", LTARRAYSIZE( szPath ));

	bool bNewFile = !CWinUtil::FileExist(szPath);

	if (cFileWrite.Open(szPath,true))
	{
		if (bNewFile)
		{
			char szTmp[128] = "<HTML><BODY>\r\n<table>\r\n";
			cFileWrite.Write(szTmp,LTStrLen(szTmp));
		}
		
		cFileWrite.Write(szMsg,LTStrLen(szMsg));
		cFileWrite.Close();
	}

}
