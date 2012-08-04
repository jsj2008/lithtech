
#include "precompile.h"

#include "aabb.h"

EAABBCorner GetAABBPlaneCorner(const LTVector &vNormal)
{
	static LTVector aCornerDir[8] = {
		LTVector(-1.0f, 1.0f, -1.0f), // Near top-left
		LTVector(1.0f, 1.0f, -1.0f), // Near top-right
		LTVector(-1.0f, -1.0f, -1.0f), // Near bottom-left
		LTVector(1.0f, -1.0f, -1.0f), // Near bottom-right
		LTVector(-1.0f, 1.0f, 1.0f), // Far top-left
		LTVector(1.0f, 1.0f, 1.0f), // Far top-right
		LTVector(-1.0f, -1.0f, 1.0f), // Far bottom-left
		LTVector(1.0f, -1.0f, 1.0f), // Far bottom-right
	};

	uint32 nBestCorner = 0;
	float fBestCornerDot = -1.0f;
	for (uint32 nCornerLoop = 0; nCornerLoop < 8; ++nCornerLoop)
	{
		float fCornerDot = vNormal.Dot(aCornerDir[nCornerLoop]);
		// We're looking for the most positive corner dot product..
		if (fCornerDot > fBestCornerDot)
		{
			nBestCorner = nCornerLoop;
			fBestCornerDot = fCornerDot;
		}
	}

	return (EAABBCorner)nBestCorner;
}

