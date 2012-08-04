// ----------------------------------------------------------------------- //
//
// MODULE  : FullScreenTint.h
//
// PURPOSE : Definition of FullScreenTint class
//
// CREATED : 6/22/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __FULL_SCREEN_TINT_H__
#define __FULL_SCREEN_TINT_H__

#include "ltbasedefs.h"

class CFullScreenTint
{
  public:

	CFullScreenTint();
	~CFullScreenTint();

	void		Init();
	void		Term();

	void		Draw();
    void        TurnOn(bool bOn=true) { m_bOn = bOn; }
    uint8       IsOn() { return m_bOn; }

    void        SetAlpha(float fAlpha);

  private:

	float		m_fAlpha;
    bool		m_bOn;
};

#endif  // __FULL_SCREEN_TINT_H__