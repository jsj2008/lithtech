//////////////////////////////////////////////////////////////////////////////
// ViewParams class implementation

#include "precompile.h"

#include "d3d_viewparams.h"
#include "d3d_draw.h"

bool ViewParams::ViewAABBIntersect(const LTVector &vBoxMin, const LTVector &vBoxMax) const
{
	// Check the AABB
	if ((vBoxMin.x > m_ViewAABBMax.x) || 
		(vBoxMin.y > m_ViewAABBMax.y) || 
		(vBoxMin.z > m_ViewAABBMax.z) || 
		(vBoxMax.x < m_ViewAABBMin.x) || 
		(vBoxMax.y < m_ViewAABBMin.y) || 
		(vBoxMax.z < m_ViewAABBMin.z))
	{
		return false;
	}

	for (uint32 nPlaneLoop = 0; nPlaneLoop < NUM_CLIPPLANES; ++nPlaneLoop)
	{
		// If the near vertex is on the outside of this plane, we've found a seperating axis
		if (GetAABBPlaneSideBack(m_AABBPlaneCorner[nPlaneLoop], m_ClipPlanes[nPlaneLoop], vBoxMin, vBoxMax))
			return false;
	}

	// It was on the inside of at least one plane, so we must be visible
	return true;
}
