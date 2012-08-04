
#include "bdefs.h"
#include "lightmap_planes.h"


// Principal planes the lightmap planes come from.
LMPlane g_LMPlanes[NUM_LMPLANES] =
{
	LMPlane(LTVector(1.0f, 0.0f, 0.0f), LTVector(0.0f, 0.0f, -1.0f), LTVector(0.0f, 1.0f, 0.0f)),
	LMPlane(LTVector(1.0f, 0.0f, 0.0f), LTVector(0.0f, 0.0f, 1.0f), LTVector(0.0f, -1.0f, 0.0f)),
	LMPlane(LTVector(1.0f, 0.0f, 0.0f), LTVector(0.0f, 1.0f, 0.0f), LTVector(0.0f, 0.0f, 1.0f)),
	LMPlane(LTVector(1.0f, 0.0f, 0.0f), LTVector(0.0f, -1.0f, 0.0f), LTVector(0.0f, 0.0f, -1.0f)),
	LMPlane(LTVector(0.0f, 0.0f, 1.0f), LTVector(0.0f, -1.0f, 0.0f), LTVector(1.0f, 0.0f, 0.0f)),
	LMPlane(LTVector(0.0f, 0.0f, -1.0f), LTVector(0.0f, -1.0f, 0.0f), LTVector(-1.0f, 0.0f, 0.0f))
};



LMPlane::LMPlane(LTVector inP, LTVector inQ, LTVector inNormal)
{
	P = inP;
	Q = inQ;
	Normal = inNormal;
}


uint32 SelectLMPlaneVector(LTVector vNormal)
{
	uint32 i, iBestPlane;
	float fTest, fBestDot;

	// Pick a lightmap plane.
	iBestPlane = 0;
	fBestDot = -2.0f;
	for(i=0; i < NUM_LMPLANES; i++)
	{
		fTest = g_LMPlanes[i].Normal.Dot(vNormal);
		if(fTest > fBestDot)
		{
			fBestDot = fTest;
			iBestPlane = i;
		}
	}

	return iBestPlane;
}

void SetupLMPlaneVectors(uint32 iPlane, const LTVector &N, LTVector &P, LTVector &Q)
{
	ASSERT(iPlane < NUM_LMPLANES);

	// Find the right vector based on the plane's down vector and the normal
	P = N.Cross(g_LMPlanes[iPlane].Q);
	// Cross back to get the orthogonal down vector
	Q = P.Cross(N);
}
