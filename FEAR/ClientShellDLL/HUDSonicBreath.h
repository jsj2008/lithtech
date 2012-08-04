// ****************************************************************************************** //
//
// MODULE  : HUDSonicBreath.h
//
// PURPOSE : Definition of HUD Sonic Breath display
//
// CREATED : 07/16/04
//
// (c) 1999-2005 Monolith Productions, Inc.  All Rights Reserved
//
// ****************************************************************************************** //

#ifndef __HUDSONICBREATH_H__
#define __HUDSONICBREATH_H__

#ifndef __HUDITEM_H__
#include "HUDItem.h"
#endif//__HUDITEM_H__

// ****************************************************************************************** //

class CHUDSonicBreath : public CHUDItem
{
	public:

		// ------------------------------------------------------------------------------------------ //
		// Construction / destruction

		CHUDSonicBreath();


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

#endif//__HUDSONICBREATH_H__

