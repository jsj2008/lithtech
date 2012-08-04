// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenMgr.h
//
// PURPOSE : Interface screen manager
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#if !defined(_TO2_SCREEN_MGR_H_)
#define _TO2_SCREEN_MGR_H_

#include "ScreenMgr.h"

class CTO2ScreenMgr : public CScreenMgr
{
public:
	CTO2ScreenMgr();
	virtual ~CTO2ScreenMgr();
    virtual LTBOOL			Init();
	virtual const char *	GetScreenName(eScreenID id);
	virtual uint16		GetScreenIDFromName(char * pName);


protected:

	void			AddScreen(eScreenID screenID);
	virtual void	SwitchToScreen(CBaseScreen *pNewScreen);

};

#endif // _SCREENMGR_H_