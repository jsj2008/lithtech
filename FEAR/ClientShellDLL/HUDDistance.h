// ----------------------------------------------------------------------- //
//
// MODULE  : HUDDistance.h
//
// PURPOSE : HUDItem to display player distance to an area
//
// CREATED : 5/08/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUD_DISTANCE_H__
#define __HUD_DISTANCE_H__

//
// Includes...
//

	#include "HUDItem.h"
	#include "HUDBar.h"


class CHUDDistance : public CHUDItem
{
	public: // Methods...

		CHUDDistance();
		~CHUDDistance();

		virtual bool	Init();
		virtual void	Term();

		virtual void	Render();
		virtual void	Update();
		virtual void	ScaleChanged();		

		virtual void	UpdateLayout();

		void		Clear( );

	private: // Members...

		float		m_fBlinkSpeed;
		float		m_fAlpha;

		bool		m_bDraw;
		bool		m_bBlink;
		float		m_fFadeIncrement;
		int8		m_nFadeDir;
		bool		m_bFadeOut;
		float		m_fMaxAlpha;
		bool		m_bFirstUpdate;
		
		float		m_fDistPercent;

};

#endif // __HUD_DISTANCE_H__