// InterfaceResMgr.h: interface for the CInterfaceResMgr class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_INTERFACERESMGR_H__44498A80_ECE2_11D2_B2DB_006097097C7B__INCLUDED_)
#define AFX_INTERFACERESMGR_H__44498A80_ECE2_11D2_B2DB_006097097C7B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "LTGUIMgr.h"
#include "InterfaceSurfMgr.h"

class CGameClientShell;
class CInterfaceResMgr;
extern CInterfaceResMgr* g_pInterfaceResMgr;


class CInterfaceResMgr
{
public:
	CInterfaceResMgr();
	virtual ~CInterfaceResMgr();

    LTBOOL               Init (ILTClient* pClientDE, CGameClientShell* pClientShell);
	void				Term();
	HSURFACE			GetSharedSurface(char *lpszPath)	{ return m_InterfaceSurfMgr.GetSurface(lpszPath); }
	void				FreeSharedSurface(char *lpszPath)	{ m_InterfaceSurfMgr.FreeSurface(lpszPath); }
	void				FreeSharedSurface(HSURFACE hSurf)	{ m_InterfaceSurfMgr.FreeSurface(hSurf); }

	CLTGUIFont			*GetTitleFont()						{return m_pTitleFont;}
	CLTGUIFont			*GetLargeFont()						{return m_pLargeFont;}
	CLTGUIFont			*GetSmallFont()						{return m_pSmallFont;}
	CLTGUIFont			*GetMediumFont()					{return m_pMediumFont;}
	CLTGUIFont			*GetHelpFont()						{return m_pHelpFont;}

	CLTGUIFont			*GetMsgForeFont()					{return m_pMsgForeFont;}
	CLTGUIFont			*GetHUDForeFont()					{return m_pHUDForeFont;}
	CLTGUIFont			*GetAirFont()						{return m_pAirFont;}
	CLTGUIFont			*GetChooserFont()					{return m_pChooserFont;}

	HSURFACE			GetSurfaceLoading();
	HSURFACE			GetSurfaceCursor();

	// Creates a surface just large enough for the string.
	// You can make the surface a little larger with extraPixelsX and extraPixelsY.
	// if a Width > 0 is specified, the text is wrapped to fit that width
    HSURFACE            CreateSurfaceFromString(CLTGUIFont *pFont, HSTRING hString, HLTCOLOR hBackColor,
												int extraPixelsX = 0, int extraPixelsY = 0, int nWidth = 0);
    HSURFACE            CreateSurfaceFromString(CLTGUIFont *pFont, int  nStringId, HLTCOLOR hBackColor,
												int extraPixelsX = 0, int extraPixelsY = 0, int nWidth = 0);
    HSURFACE            CreateSurfaceFromString(CLTGUIFont *pFont, char *lpszString, HLTCOLOR hBackColor,
												int extraPixelsX = 0, int extraPixelsY = 0, int nWidth = 0);


	const char			*GetSoundSelect();
	const char			*GetSoundChange();
	const char			*GetSoundPageChange();
	const char			*GetSoundUnselectable();
	const char			*GetSoundArrowUp();
	const char			*GetSoundArrowDown();
	const char			*GetSoundArrowLeft();
	const char			*GetSoundArrowRight();
	const char			*GetObjectiveAddedSound();
	const char			*GetObjectiveRemovedSound();
	const char			*GetObjectiveCompletedSound();

	HSURFACE			CreateSurfaceBlank();

	void				ScreenDimsChanged();


    LTBOOL               IsEnglish()                         { return m_bEnglish; }

	void				DrawFolder();
	void				DrawLoadScreen();

	int					GetXOffset()						{return m_Offset.x;}
	int					GetYOffset()						{return m_Offset.y;}
    LTFLOAT              GetXRatio()                         {return m_fXRatio;}
    LTFLOAT              GetYRatio()                         {return m_fYRatio;}

    uint32              GetScreenWidth();
    uint32              GetScreenHeight();

	void				DrawMessage(CLTGUIFont* pFont, int nMessageId);


	//call Setup() before entering a 2-d state (folder)
	//call Clean() before returning to the game
    LTBOOL               Setup();
	void				Clean();

protected:
	// More initialization

    LTBOOL               InitFonts();
    LTBOOL               InitEngineFont(CLTGUIFont *pFont, int nNameID, int nWidthID, int nHeightID, LTBOOL bBold);
    LTBOOL               InitEngineFont(CLTGUIFont *pFont, char *lpszName, int nWidth, int nHeight, LTBOOL bBold);
    LTBOOL               SetupFont(CLTGUIFont *pFont, LTBOOL bBlend = LTTRUE, uint32 dwFlags = LTF_INCLUDE_ALL);

protected:
    LTBOOL               m_bEnglish;             // True if the resource file has English as the specified language

	CLTGUIFont			*m_pTitleFont;			// Title font
	CLTGUIFont			*m_pLargeFont;			// Large fading font
	CLTGUIFont			*m_pMediumFont;			// Medium fading font
	CLTGUIFont			*m_pSmallFont;			// Small fading font
	CLTGUIFont			*m_pHelpFont;			// Help font
	CLTGUIFont			*m_pMsgForeFont;		// Foreground Font used in HUD text display
	CLTGUIFont			*m_pHUDForeFont;		// Foreground Font used in HUD numeric display
	CLTGUIFont			*m_pAirFont;			// Font used in HUD for air meter
	CLTGUIFont			*m_pChooserFont;		// Font used in HUD for choosing weapons/ammo

	HSURFACE			m_hSurfLoading;			// Loading level surface
	HSURFACE			m_hSurfUpArrow;			// The up arrow surface
	HSURFACE			m_hSurfDownArrow;		// The down arrow surface
	HSURFACE			m_hSurfLeftArrow;		// The left arrow surface
	HSURFACE			m_hSurfRightArrow;		// The right arrow surface

	HSURFACE			m_hSurfCursor;			// The software cursor surface

	int					m_nYesVKeyCode;			// The virtual key code for "yes" responses
	int					m_nNoVKeyCode;			// The virtual key code for "no" responses

	CInterfaceSurfMgr	m_InterfaceSurfMgr;	// Used to share title graphics

    HLTCOLOR            m_hTransColor;

    LTIntPt              m_Offset;
    LTFLOAT              m_fXRatio;
    LTFLOAT              m_fYRatio;
    uint32              m_dwScreenWidth;
    uint32              m_dwScreenHeight;

	CString				m_csSoundSelect;
	CString				m_csSoundChange;
	CString				m_csSoundPageChange;
	CString				m_csSoundUnselectable;
	CString				m_csSoundArrowUp;
	CString				m_csSoundArrowDown;
	CString				m_csSoundArrowLeft;
	CString				m_csSoundArrowRight;
	CString				m_csSoundObjAdd;
	CString				m_csSoundObjRemove;
	CString				m_csSoundObjComplete;

};

#define TERMSHAREDSURF(surf) if(surf) { g_pInterfaceResMgr->FreeSharedSurface(surf); surf = NULL; }

inline uint32 CInterfaceResMgr::GetScreenWidth()
{
	if (m_dwScreenWidth < 0)
		ScreenDimsChanged();
	return m_dwScreenWidth;
}
inline uint32 CInterfaceResMgr::GetScreenHeight()
{
	if (m_dwScreenHeight < 0)
		ScreenDimsChanged();
	return m_dwScreenHeight;
}


#endif // !defined(AFX_INTERFACERESMGR_H__44498A80_ECE2_11D2_B2DB_006097097C7B__INCLUDED_)