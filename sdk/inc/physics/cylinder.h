#ifndef __CYLINDER_H__
#define __CYLINDER_H__


#include "coordinate_frame.h"


//---------------------------------------------------------------------------//
//Check if the swept sphere hit the swept cylinder.  Unit normal n points from
//cylinder to sphere.
bool SphereCylinderSweep
(
	float&				u,
	LTVector3f&			n,
	LTVector3f&			a,
	LTVector3f&			b,
	const LTVector3f&	A0,
	const LTVector3f&	A1,
	const float			ra,
	const LTCoordinateFrameQ& B0,
	const LTCoordinateFrameQ& B1,
	const float			rb,
	const float			hh
);

//---------------------------------------------------------------------------//
//Check if the swept sphere hit the swept cylinder
bool CylinderBoxSweep
(
	float&				u,
	LTVector3f&			n,
	LTVector3f&			a,
	LTVector3f&			b,
	const LTCoordinateFrameQ& A0,
	const LTCoordinateFrameQ& A1,
	const float			r,
	const float			hh,
	const LTCoordinateFrameQ& B0,
	const LTCoordinateFrameQ& B1,
	const LTVector3f&	dim
);

//---------------------------------------------------------------------------//
//Check if the swept sphere hit the swept cylinder
bool CylinderSweep
(
	float&				u,
	LTVector3f&			n,
	LTVector3f&			a,
	LTVector3f&			b,
	const LTCoordinateFrameQ& A0,
	const LTCoordinateFrameQ& A1,
	const float			ra,
	const float			ha,
	const LTCoordinateFrameQ& B0,
	const LTCoordinateFrameQ& B1,
	const float			rb,
	const float			hb
);


#endif
//EOF
