// ----------------------------------------------------------------------- //
//
// MODULE  : LoadingScreenLayout.h
//
// PURPOSE : Layout data for loading screens
//
// CREATED : 2003
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __LOADINGSCREENLAYOUT_H__
#define __LOADINGSCREENLAYOUT_H__


class CLoadingScreenLayout
{
	// External functions
public:
	CLoadingScreenLayout();
	~CLoadingScreenLayout() {}

	bool Read(HRECORD hLayout);
//	bool Read(const char* pszTag);

	LTVector2n	m_TitlePos;
	CFontInfo	m_sTitleFont;
	uint32		m_cTitleColor;

	LTVector2n	m_LevelPos;
	CFontInfo	m_sLevelFont;
	uint32		m_cLevelColor;

	LTRect2n	m_BriefingRect;
	CFontInfo	m_sBriefingFont;
	uint32		m_cBriefingColor;
	CFontInfo	m_sBriefingHeaderFont;

	LTRect2n	m_ServerMsgRect;
	CFontInfo	m_sServerMsgFont;
	uint32		m_cServerMsgColor;
	CFontInfo	m_sServerMsgHeaderFont;

	LTRect2n	m_HelpRect;
	CFontInfo	m_sHelpFont;
	uint32		m_cHelpColor;

	LTVector2n	m_ContinuePos;
	CFontInfo	m_sContinueFont;
	uint32		m_cContinueColor;

	LTRect2n	m_PhotoRect;

	std::string		m_sBackTexture;

	TextureReference	m_hLoadProgressTexture;
	LTRect2f			m_LoadProgressRect;

	CFontInfo			m_sContentFont;
	uint32				m_cContentColor;
	TextureReference	m_hContentTexture;
	LTRect2n			m_ContentFrameRect;
	TextureReference	m_hContentFrameTexture;

	LTVector2n			m_CurrentFileNamePos;
	LTVector2n			m_CurrentFileTimePos;
	LTVector2n			m_FilesLeftPos;
	LTVector2n			m_TotalTimePos;
	
	LTRect2f			m_CurrentFileRect;
	LTRect2f			m_TotalRect;


	bool		m_bReadLayout;

};


extern CLoadingScreenLayout g_DefaultLayout;
extern CLoadingScreenLayout g_CurrentLayout;

#endif __LOADINGSCREENLAYOUT_H__
