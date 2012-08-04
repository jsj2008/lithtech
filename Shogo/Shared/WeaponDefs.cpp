// ----------------------------------------------------------------------- //
//
// MODULE  : WeaponDefs.cpp
//
// PURPOSE : Definitions for weapon types
//
// CREATED : 11/07/97
//
// ----------------------------------------------------------------------- //

#include "WeaponDefs.h"
//#include "basedefs_de.h"

// Calculate the adjusted path and fire position for the particular weapon...

void CalculateWeaponPathAndFirePos(RiotWeaponId nId, LTVector & vPath, 
								   LTVector & vFirePos, LTVector & vU, LTVector & vR)
{
	LTVector vTemp;
	LTFLOAT fRPerturbe, fUPerturbe;
	int nPerturbe = GetWeaponPerturbe(nId, fRPerturbe, fUPerturbe);

	if (nPerturbe) 
	{
		VEC_MULSCALAR(vTemp, vR, fRPerturbe);
		VEC_ADD(vPath, vPath, vTemp);

		VEC_MULSCALAR(vTemp, vU, fUPerturbe);
		VEC_ADD(vPath, vPath, vTemp);
	}

	VEC_NORM(vPath);

	LTFLOAT fSpread = GetWeaponSpread(nId);

	static int s_nQuadrant = 1;

	if (fSpread > 0.0f) 
	{
		switch (s_nQuadrant) 
		{
			case 1:
				VEC_MULSCALAR(vTemp, vR, fSpread);
				VEC_ADD(vFirePos, vFirePos, vTemp);
			break;
			
			case 2:
				VEC_MULSCALAR(vTemp, vU, fSpread);
				VEC_ADD(vFirePos, vFirePos, vTemp);
			break;

			case 3:
				VEC_MULSCALAR(vTemp, vR, -fSpread);
				VEC_ADD(vFirePos, vFirePos, vTemp);
			break;

			case 4:
			default:
				VEC_MULSCALAR(vTemp, vU, -fSpread);
				VEC_ADD(vFirePos, vFirePos, vTemp);
			break;
		}

		if (++s_nQuadrant > 4) 
		{
			s_nQuadrant = 1;
		}
	}
}
