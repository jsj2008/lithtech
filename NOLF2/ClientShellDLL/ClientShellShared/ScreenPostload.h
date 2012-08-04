// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenPostload.h
//
// PURPOSE : Interface screen to be displayed after loading a level but before
//				starting to play it.
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#if !defined(_SCREEN_POSTLOAD_H_)
#define _SCREEN_POSTLOAD_H_

#include "BaseScreen.h"

class CScreenPostload : public CBaseScreen
{
public:
	CScreenPostload();
	virtual ~CScreenPostload();

	// Build the screen
    virtual LTBOOL	Build();
    virtual void	OnFocus(LTBOOL bFocus);
	virtual void	Escape();

	virtual bool	UpdateInterfaceSFX();

    virtual LTBOOL   HandleKeyDown(int key, int rep) { Escape(); return LTTRUE; }
    virtual LTBOOL   OnLButtonDown(int x, int y) { Escape(); return LTTRUE;}

    virtual LTBOOL   Render(HSURFACE hDestSurf);

	// used by Load Screen to synchronize layout info
	void SetLayout(const char *pszLayout) { m_layout = pszLayout;}


protected:

	virtual void	CreateInterfaceSFX();

	std::string		m_missionname;
	std::string		m_levelname;
	std::string		m_layout;
	std::string		m_briefing;
	std::string		m_help;

	CUIFormattedPolyString 	*m_pMissionNameStr;
	CUIFormattedPolyString 	*m_pLevelNameStr;
	CUIFormattedPolyString 	*m_pBriefingStr;
	CUIFormattedPolyString 	*m_pHelpStr;
	CUIFormattedPolyString 	*m_pContinueStr;


	//layout info
	bool	m_bReadLayout;
	LTIntPt m_DefaultTitlePos;
	uint8	m_nDefaultTitleFont;
	uint8	m_nDefaultTitleFontSize;
	uint32	m_nDefaultTitleColor;
	LTIntPt m_DefaultLevelPos;
	uint8	m_nDefaultLevelFont;
	uint8	m_nDefaultLevelFontSize;
	uint32	m_nDefaultLevelColor;
	LTIntPt m_DefaultBriefingPos;
	uint16	m_nDefaultBriefingWidth;
	uint8	m_nDefaultBriefingFont;
	uint8	m_nDefaultBriefingFontSize;
	uint32	m_nDefaultBriefingColor;
	LTIntPt m_DefaultHelpPos;
	uint16	m_nDefaultHelpWidth;
	uint8	m_nDefaultHelpFont;
	uint8	m_nDefaultHelpFontSize;
	uint32	m_nDefaultHelpColor;
	LTIntPt m_DefaultContinuePos;
	uint8	m_nDefaultContinueFont;
	uint8	m_nDefaultContinueFontSize;
	uint32	m_nDefaultContinueColor;

	uint32	m_nContinueColor;

	bool	m_bPressAnyKey;


};

#endif // !defined(_SCREEN_PRELOAD_H_)