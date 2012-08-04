// ----------------------------------------------------------------------- //
//
// MODULE  : HUDBuildVersion.h
//
// PURPOSE : HUDItem to display player BuildVersion
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUD_BUILDVERSION_H
#define __HUD_BUILDVERSION_H

#include "HUDItem.h"

//******************************************************************************************
//** HUD BuildVersion display
//******************************************************************************************
class CHUDBuildVersion : public CHUDItem
{
public:
	CHUDBuildVersion();

    virtual bool	Init();
	virtual void	Term() {}

	virtual void	Update() {}
    virtual void	Render();

    virtual void	UpdateLayout();
};

#endif
