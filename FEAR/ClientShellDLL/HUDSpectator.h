// ----------------------------------------------------------------------- //
//
// MODULE  : HUDSpectator.h
//
// PURPOSE : HUDItem to spectator information
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUDSPECTATOR_H__
#define __HUDSPECTATOR_H__

//
// Includes...
//
	
#include "HUDItem.h"

class CHUDSpectator : public CHUDItem
{
	public:  // Methods...

		CHUDSpectator();
		virtual ~CHUDSpectator();

		virtual bool	Init();
		virtual void	Term() {};
		virtual	void	Render();
		virtual void	Update();
		virtual void	ScaleChanged();

		// Force any implementation of this hud item to use its own layout data...
		virtual void	UpdateLayout();

	protected: // Members...

		LTVector2		m_pos;
		SpectatorMode	m_eLastSpectatorMode;
		LTObjRef		m_hLastTarget;
		CCharacterFX*	m_pLastCharFx;
		bool			m_bLastCanRespawn;
};

#endif // __HUDSPECTATOR_H__