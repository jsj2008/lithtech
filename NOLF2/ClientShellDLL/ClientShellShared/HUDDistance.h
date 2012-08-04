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

		LTBOOL      Init();
		void        Render();
		void        Update();
		void        UpdateLayout();

	private: // Members...

		LTIntPt		m_BasePos;
		float		m_fBlinkSpeed;
		float		m_fAlpha;
		float		m_fIconSize;

		bool		m_bDraw;
		bool		m_bBlink;
		float		m_fFadeIncrement;
		int8		m_nFadeDir;
		bool		m_bFadeOut;
		float		m_fFadeOutSpeed;
		float		m_fMaxAlpha;
		
		float		m_fDistPercent;

		LTPoly_GT4	m_Poly;
		HTEXTURE	m_hIcon;			//  icon
};

#endif // __HUD_DISTANCE_H__