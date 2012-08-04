// ----------------------------------------------------------------------- //
//
// MODULE  : InterfaceResMgr.h
//
// PURPOSE : Manager for resources associated with the interface
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#if !defined(_INTERFACERESMGR_H_)
#define _INTERFACERESMGR_H_

#include "LTGUIMgr.h"
#include "TextureReference.h"

class CGameClientShell;
class CInterfaceResMgr;
extern CInterfaceResMgr* g_pInterfaceResMgr;


const int kMaxSoundFilename = 64;

class CInterfaceResMgr
{
public:
	CInterfaceResMgr();
	virtual ~CInterfaceResMgr();

    bool            Init();
	void			Term();

	const char*		GetSoundSelect();
	const char*		GetSoundChange();
	const char*		GetSoundPageChange();
	const char*		GetSoundUnselectable();
	const char*		GetSoundArrowUp();
	const char*		GetSoundArrowDown();
	const char*		GetSoundArrowLeft();
	const char*		GetSoundArrowRight();

	void			ScreenDimsChanged();


    bool              IsEnglish()                         { return m_bEnglish; }

	void				DrawScreen();


	void				ConvertScreenRect(LTRect2n &rect);
	const LTVector2		ConvertScreenPos(LTVector2n pos) const {return ConvertScreenPos(pos.x,pos.y);}
	const LTVector2		ConvertScreenPos(int x, int y) const;
	void				ScaleScreenPos(LTVector2n &pos);

    const float			GetXRatio() const       { return m_vfScale.x; }
    const float			GetYRatio() const		{ return m_vfScale.y; }
	const LTVector2		GetScreenScale() const	{ return m_vfScale; }
    uint32              GetScreenWidth();
    uint32              GetScreenHeight();
	bool				IsWidescreen() const	{ return m_bWidescreen; }

	void				DrawMessage(const char* szMessageId, uint32 nFontSize = 0);
	void				DrawMessage(const wchar_t *pString, uint32 nFontSize = 0);

	//given a Y FOV in radians and an aspect ratio scale, this will determine the FOV angles
	//in radians that should be used for the camera's FOV in the X and Y directions based
	//upon the current aspect ratio of the device
	LTVector2			GetScreenFOV(float fYFOVRadians, float fAspectRatioScale);

	//call Setup() before entering a 2-d state (screen)
	//call Clean() before returning to the game
    bool               Setup();
	void				Clean();

protected:
	// More initialization

    bool              InitFonts();

protected:
    bool				m_bEnglish;             // True if the resource file has English as the specified language

	int					m_nYesVKeyCode;			// The virtual key code for "yes" responses
	int					m_nNoVKeyCode;			// The virtual key code for "no" responses

	LTVector2			m_vfScale;
    uint32              m_dwScreenWidth;
    uint32              m_dwScreenHeight;
	bool				m_bWidescreen;

	CFontInfo			m_DlgFont;
	CLTGUIWindow	    m_MsgDlg;
	CLTGUITextCtrl*		m_pMsgText;

	typedef std::vector<HCUSTOMFONTFILE, LTAllocator<HCUSTOMFONTFILE, LT_MEM_TYPE_CLIENTSHELL> > FontArray;
	FontArray m_FontArray;
};

#define TERMSHAREDSURF(surf) if(surf) { g_pInterfaceResMgr->FreeSharedSurface(surf); surf = NULL; }

inline uint32 CInterfaceResMgr::GetScreenWidth()
{
	if (m_dwScreenWidth == 0)
		ScreenDimsChanged();
	return m_dwScreenWidth;
};
inline uint32 CInterfaceResMgr::GetScreenHeight()
{
	if (m_dwScreenHeight == 0)
		ScreenDimsChanged();
	return m_dwScreenHeight;
};


#endif // !defined(_INTERFACERESMGR_H_)