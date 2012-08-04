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


class CInterfaceTimer
{
  public:

	CInterfaceTimer()
	{
		m_fTime		= 0.0f;
		m_bPause	= LTFALSE;
	}

	void		Draw(HSURFACE hScreen);
    void        SetTime(LTFLOAT fTime, LTBOOL bPause) { m_fTime = fTime; m_bPause = bPause; }
    LTFLOAT      GetTime() const { return m_fTime; }

  private:

    LTFLOAT     	m_fTime;
	LTBOOL			m_bPause;
};

#endif  // __INTERFACE_TIMER_H__