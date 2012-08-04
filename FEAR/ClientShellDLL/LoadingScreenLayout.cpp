//
// MODULE  : LoadingScreenLayout.cpp
//
// PURPOSE : Layout data for loading screens
//
// CREATED : 2003
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "LoadingScreenLayout.h"

CLoadingScreenLayout g_DefaultLayout;
CLoadingScreenLayout g_CurrentLayout;


CLoadingScreenLayout::CLoadingScreenLayout()
{
	m_bReadLayout = false;
}


/*
bool CLoadingScreenLayout::Read(const char* pszTag)
{

	HRECORD hRecord = g_pLayoutDB->GetLoadScreenRecord(pszTag);
	if (!hRecord)
		return false;
	return Read(hRecord);
}
*/

bool CLoadingScreenLayout::Read(HRECORD hLayout)
{
	if (!hLayout)
		return false;

	//layout info for title string
	m_TitlePos = g_pLayoutDB->GetPosition(hLayout,"TitlePos");
	m_cTitleColor = g_pLayoutDB->GetColor(hLayout, "TitleColor");
	std::string sFontName = g_pLayoutDB->GetFont(hLayout,"TitleFont");
	uint32 nFontSize = g_pLayoutDB->GetInt32(hLayout,"TitleSize");
	m_sTitleFont = CFontInfo(sFontName.c_str(),nFontSize);


	//layout info for level string
	m_LevelPos = g_pLayoutDB->GetPosition(hLayout,"LevelPos");
	m_cLevelColor = g_pLayoutDB->GetColor(hLayout, "LevelColor");
	sFontName = g_pLayoutDB->GetFont(hLayout,"LevelFont");
	nFontSize = g_pLayoutDB->GetInt32(hLayout,"LevelSize");
	m_sLevelFont = CFontInfo(sFontName.c_str(),nFontSize);

	//layout info for briefing string
	m_BriefingRect = g_pLayoutDB->GetRect(hLayout,"BriefingRect");
	m_cBriefingColor = g_pLayoutDB->GetColor(hLayout, "BriefingColor");
	sFontName = g_pLayoutDB->GetFont(hLayout,"BriefingFont");
	nFontSize = g_pLayoutDB->GetInt32(hLayout,"BriefingSize");
	m_sBriefingFont = CFontInfo(sFontName.c_str(),nFontSize);

	sFontName = g_pLayoutDB->GetFont(hLayout,"BriefingHeaderFont");
	nFontSize = g_pLayoutDB->GetInt32(hLayout,"BriefingHeaderSize");
	m_sBriefingHeaderFont = CFontInfo(sFontName.c_str(),nFontSize);

	//layout info for server message string
	m_ServerMsgRect = g_pLayoutDB->GetRect(hLayout,"ServerMsgRect");
	m_cServerMsgColor = g_pLayoutDB->GetColor(hLayout, "ServerMsgColor");
	sFontName = g_pLayoutDB->GetFont(hLayout,"ServerMsgFont");
	nFontSize = g_pLayoutDB->GetInt32(hLayout,"ServerMsgSize");
	m_sServerMsgFont = CFontInfo(sFontName.c_str(),nFontSize);

	sFontName = g_pLayoutDB->GetFont(hLayout,"ServerMsgHeaderFont");
	nFontSize = g_pLayoutDB->GetInt32(hLayout,"ServerMsgHeaderSize");
	m_sServerMsgHeaderFont = CFontInfo(sFontName.c_str(),nFontSize);

	//layout info for help string
	m_HelpRect = g_pLayoutDB->GetRect(hLayout,"HelpRect");
	m_cHelpColor = g_pLayoutDB->GetColor(hLayout, "HelpColor");
	sFontName = g_pLayoutDB->GetFont(hLayout,"HelpFont");
	nFontSize = g_pLayoutDB->GetInt32(hLayout,"HelpSize");
	m_sHelpFont = CFontInfo(sFontName.c_str(),nFontSize);

	//layout info for Continue string
	m_ContinuePos = g_pLayoutDB->GetPosition(hLayout,"ContinuePos");
	m_cContinueColor = g_pLayoutDB->GetColor(hLayout, "ContinueColor");
	sFontName = g_pLayoutDB->GetFont(hLayout,"ContinueFont");
	nFontSize = g_pLayoutDB->GetInt32(hLayout,"ContinueSize");
	m_sContinueFont = CFontInfo(sFontName.c_str(),nFontSize);

	m_PhotoRect = g_pLayoutDB->GetRect(hLayout,"PhotoRect");

	m_sBackTexture = g_pLayoutDB->GetString(hLayout,"BackgroundTexture");;

	const char* pszBar = g_pLayoutDB->GetString(hLayout,"LoadingBarTexture");;
	m_hLoadProgressTexture.Load( pszBar);
	m_LoadProgressRect = g_pLayoutDB->GetRectF(hLayout,"LoadingBarRect");

	//layout info for Content download string and bar
	m_cContentColor = g_pLayoutDB->GetColor(hLayout, "ContentColor");
	sFontName = g_pLayoutDB->GetFont(hLayout,"ContentFont");
	nFontSize = g_pLayoutDB->GetInt32(hLayout,"ContentSize");
	m_sContentFont = CFontInfo(sFontName.c_str(),nFontSize);
	pszBar = g_pLayoutDB->GetString(hLayout,"ContentTransferBarTexture");;
	m_hContentTexture.Load(	pszBar);

	m_ContentFrameRect = g_pLayoutDB->GetRect(hLayout,"ContentFrameRect");
	m_hContentFrameTexture.Load(g_pLayoutDB->GetString(hLayout,"ContentFrameTexture"));


	m_CurrentFileNamePos = g_pLayoutDB->GetPosition(hLayout,"ContentPos",0);
	m_CurrentFileTimePos = g_pLayoutDB->GetPosition(hLayout,"ContentPos",1);
	m_FilesLeftPos = g_pLayoutDB->GetPosition(hLayout,"ContentPos",2);
	m_TotalTimePos = g_pLayoutDB->GetPosition(hLayout,"ContentPos",3);
	
	m_CurrentFileRect = g_pLayoutDB->GetRectF(hLayout,"ContentTransferBarRect",0);
	m_TotalRect = g_pLayoutDB->GetRectF(hLayout,"ContentTransferBarRect",1);


	m_bReadLayout = true;

	return true;


}



