// ----------------------------------------------------------------------- //
//
// MODULE  : InterfaceMeter.h
//
// PURPOSE : Definition of InterfaceMeter class
//
// CREATED : 10/18/99
//
// (c) 1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __INTERFACE_METER_H__
#define __INTERFACE_METER_H__

#include "ltbasedefs.h"


class CInterfaceMeter
{
  public:

	CInterfaceMeter();
	~CInterfaceMeter();

	void		Init();
	void		Term();

	void		Draw(HSURFACE hScreen);
    void        SetValue(uint8 nValue) { m_nValue = nValue; }
    uint8       GetValue() { return m_nValue; }

  private:

    uint8		m_nValue;
	LTRect		m_rcRect;
	HSURFACE	m_hEmptySurf;
	HSURFACE	m_hFullSurf;
};

#endif  // __INTERFACE_METER_H__