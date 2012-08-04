//////////////////////////////////////////////////////////////////////////////
// Polygons that block particle movement.

#ifndef __WORLD_PARTICLE_BLOCKER_DATA_H__
#define __WORLD_PARTICLE_BLOCKER_DATA_H__

#include "ltmodule.h"
#include "loadstatus.h"
#include <vector>


class IWorldParticleBlockerData : public IBase
{
public:
    interface_version(IWorldParticleBlockerData, 0);

	virtual ~IWorldParticleBlockerData() {};

	virtual void Term() = 0;
	
    virtual ELoadWorldStatus Load(ILTStream *pStream) = 0;

	virtual bool GetBlockersInAABB( const LTVector& pos, const LTVector& dims, std::vector<uint32>& indices ) = 0;
	virtual bool GetBlockerEdgesXZ( uint32 index, LTPlane& blockerPlane, uint32& numEdges, LTPlane*& edgePlanes ) = 0;
};


#endif //__WORLD_PARTICLE_BLOCKER_DATA_H__
