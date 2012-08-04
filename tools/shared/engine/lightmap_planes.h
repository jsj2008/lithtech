
// This module holds the table that is used to generate the lightmap texture
// vectors and has a function to find the appropriate vector for a plane.

#ifndef __LIGHTMAP_PLANES_H__
#define __LIGHTMAP_PLANES_H__


	class LMPlane
	{
	public:
				LMPlane(LTVector inP, LTVector inQ, LTVector inNormal);
		
		LTVector	P, Q, Normal;
	};
	
	
	#define NUM_LMPLANES	6
	extern LMPlane g_LMPlanes[NUM_LMPLANES];
	

	// Returns a number between 0 and NUM_LMPLANES telling the best lightmap plane
	// to use for the specified plane.
	uint32 SelectLMPlaneVector(LTVector vNormal);

	// Sets up P and Q given the plane index and a normal (the normal is so it can
	// clamp P and Q to the plane, so the lightmap follows it..)
	void SetupLMPlaneVectors(uint32 iPlane, const LTVector &N, LTVector &P, LTVector &Q);


#endif




