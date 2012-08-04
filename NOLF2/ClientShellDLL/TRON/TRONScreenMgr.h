// ----------------------------------------------------------------------- //
//
// MODULE  : TronScreenMgr.h
//
// PURPOSE : Interface screen manager
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#if !defined(_TRON_SCREEN_MGR_H_)
#define _TRON_SCREEN_MGR_H_

#include "ScreenMgr.h"

class CTronScreenMgr : public CScreenMgr
{
public:
	CTronScreenMgr();
	virtual ~CTronScreenMgr();
    virtual LTBOOL			Init();
	virtual const char *	GetScreenName(eScreenID id);
	virtual uint16		GetScreenIDFromName(char * pName);
	virtual void		ExitScreens();
	
	// [kml] 3/11/02
	// This is a temporary solution until we
	// get a real direct music solution in... 
	// Which is already hooked up in interfacemgr
	// (SetMenuMusic())
	void			PlayMenuMusic();
	void			KillMenuMusic();


protected:

	void			AddScreen(eScreenID screenID);
	virtual void	SwitchToScreen(CBaseScreen *pNewScreen);

	// [kml] 3/11/02
	// This is a temporary solution until we
	// get a real direct music solution in... 
	// Which is already hooked up in interfacemgr
	// (SetMenuMusic())
	HLTSOUND		m_hMenuSnd;
};

#endif // _TRON_SCREEN_MGR_H_