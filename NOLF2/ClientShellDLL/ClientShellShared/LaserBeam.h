 // ----------------------------------------------------------------------- //
//
// MODULE  : LaserBeam.h
//
// PURPOSE : LaserBeam class - Definition
//
// CREATED : 11/16/99
//
// (c) 1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __LASER_BEAM_H__
#define __LASER_BEAM_H__

#include "ltbasedefs.h"
#include "PolyLineFX.h"

class CLaserBeam
{
	public :

		CLaserBeam();
		~CLaserBeam();

		void	Toggle()	{ (m_bOn ? TurnOff() : TurnOn());}
		void	TurnOn();
		void	TurnOff();
        void    Update(LTVector &vBeamStartPos, const LTRotation* pRDirRot,
            LTBOOL b3rdPerson, LTBOOL bDetect=LTFALSE);

	private :

        LTBOOL           m_bOn;
		CPolyLineFX		m_LightBeam;

		void	Init();
};

#endif // __LASER_BEAM_H__