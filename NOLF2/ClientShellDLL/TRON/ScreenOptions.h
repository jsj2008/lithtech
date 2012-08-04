// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenOptions.h
//
// PURPOSE : Interface screen for navigation to various option setting screens
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef _SCREEN_OPTIONS_H_
#define _SCREEN_OPTIONS_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseScreen.h"

class CScreenOptions : public CBaseScreen
{
public:
	CScreenOptions();
	virtual ~CScreenOptions();

	// Build the screen
    LTBOOL   Build();

    void    OnFocus(LTBOOL bFocus);

protected:
    uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);

};

#endif // _SCREEN_OPTIONS_H_