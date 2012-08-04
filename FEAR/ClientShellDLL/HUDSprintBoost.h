// ****************************************************************************************** //
//
// MODULE  : CHUDSprintBoost.h
//
// PURPOSE : Definition of HUD speed boost display
//
// CREATED : 10/12/04
//
// (c) 1999-2005 Monolith Productions, Inc.  All Rights Reserved
//
// ****************************************************************************************** //

#ifndef __HUDSPRINTBOOST_H__
#define __HUDSPRINTBOOST_H__

#ifndef __HUDITEM_H__
#include "HUDItem.h"
#endif//__HUDITEM_H__

// ****************************************************************************************** //

class CHUDSprintBoost : public CHUDItem
{
public:

	// ------------------------------------------------------------------------------------------ //
	// Construction / destruction

	CHUDSprintBoost();


	// ------------------------------------------------------------------------------------------ //
	// Initialization and termination

	virtual bool		Init();
	virtual void		Term();


	// ------------------------------------------------------------------------------------------ //
	// General updating

	virtual void        Render();
	virtual void        Update();

	virtual void        UpdateLayout();
};

// ****************************************************************************************** //

#endif//__HUDSPRINTBOOST_H__

