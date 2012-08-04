// ----------------------------------------------------------------------- //
//
// MODULE  : InterfaceTimer.h
//
// PURPOSE : Definition of InterfaceTimer class
//
// CREATED : 10/18/99
//
// (c) 1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __INTERFACE_TIMER_H__
#define __INTERFACE_TIMER_H__

#include "ltbasedefs.h"
#include "iltfontmanager.h"
#include "TeamMgr.h"

extern ILTFontManager*	g_pFontManager;

class CInterfaceTimer
{
	public:

		CInterfaceTimer();
		~CInterfaceTimer();

		void		SetTeamId( uint8 nTeamId ) { m_nTeamId = nTeamId; }
		uint8		GetTeamId( ) const { return m_nTeamId; }

		void		Draw();

		void        SetTime(LTFLOAT fTime, bool bPause) { m_fTime = fTime; m_bPause = bPause; }
		float		GetTime() const { return m_fTime; }

	private:

		// Server game time when timer runs out.
		float					m_fTime;
		bool					m_bPause;
		uint8					m_nTeamId;

		CUIFormattedPolyString	*m_pTimeStr;

		LTIntPt			m_BasePos;
		uint8			m_nBaseSize;
		uint32			m_nColor;
};

#endif  // __INTERFACE_TIMER_H__