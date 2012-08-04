//////////////////////////////////////////////////////////////////////////////
// Blocker polygon handling specifically for players

#ifndef __WORLD_BLOCKER_DATA_H__
#define __WORLD_BLOCKER_DATA_H__

#include <vector>

#include "ltmodule.h"
#include "loadstatus.h"

class IWorldBlockerData : public IBase
{
public:
    interface_version(IWorldBlockerData, 0);

	virtual ~IWorldBlockerData() {};

	virtual void Term() = 0;
	
    virtual ELoadWorldStatus Load(ILTStream *pStream) = 0;

	typedef std::vector<LTVector> TNormalList;

	virtual bool CollidePlayer(
		const LTVector &vStartPt, // Starting point
		const LTVector &vEndPt, // Ending point
		const LTVector &vDims, // Dims of the player
		const TNormalList *pRestrictNormals, // Optional restriction normal list
		LTVector *pNewEndPt // New ending point result
		) = 0;

	virtual bool Intersect(
		const LTVector &vStartPt, // Starting point
		const LTVector &vEndPt, // Ending point
		const LTVector &vDims, // Dims of the player
		float *pTime, // Time of first intersection
		LTVector *pNormal // Normal of first intersection
		) = 0;
};

#endif //__WORLD_BLOCKER_DATA_H__
