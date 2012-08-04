// ----------------------------------------------------------------------- //
//
// MODULE  : HUDTitleSafeAreaNTSC.h
//
// PURPOSE : Definitition of HUD's title-safe area on NTSC
//
// CREATED : 12/06/04
//
// (c) 1999-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUDTITLESAFEAREANTSC_H__
#define __HUDTITLESAFEAREANTSC_H__

#include "HUDItem.h"


//******************************************************************************************
//** HUD display of the title-safe area on NTSC displays
//******************************************************************************************
class CHUDTitleSafeAreaNTSC : public CHUDItem
{
public:
	CHUDTitleSafeAreaNTSC();

	virtual bool	Init();
	virtual void	Term();

	virtual void	Render();
	virtual void	Update();
	virtual void	UpdateLayout();
	virtual void	ScaleChanged();
};


#endif  // __HUDTITLESAFEAREANTSC_H__
