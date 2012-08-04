#ifndef __WORLD_INTERFACE_H__
#define __WORLD_INTERFACE_H__

#ifndef __LTMODULE_H__
#include "ltmodule.h"
#endif

//------------------------------------------------------------
//Base class for world interfaces.
//------------------------------------------------------------


struct IntersectInfo;
class IntersectQuery;

class CompWorldPos;

class IWorld : public IBase {
public:
    interface_version(IWorld, 0);

    //
    //World-dependent messaging functions.
    //

	// encode/decode world position directly to a variable (no packets involved)
	virtual void EncodeCompressWorldPosition(CompWorldPos *pPos, const LTVector *pVal) = 0;
	virtual void DecodeCompressWorldPosition(LTVector *pVal, const CompWorldPos *pPos) = 0;

    //
    //
    //Physics stuff will most likely go here, since it could work
    //with client and server data structures in the same manner.
    //Because there are IWorldClient and IWorldServer that are
    //both implemented, there will be 2 IWorld implementations.
    //This allows for physics, or whatever, interfaces here to
    //have common parameters and return types, but separate implementations
    //for server and client data structures.
    //
    //

    //generic intersect function.
    virtual bool IntersectSegment(IntersectQuery *pQuery, IntersectInfo *pInfo) = 0;

	//function to handle sweeping a sphere from start to end and determining where exactly it will intersect
	virtual bool IntersectSweptSphere(const LTVector& vStart, const LTVector& vEnd, float fRadius, LTVector& vPos, LTVector& vNormal) = 0;
};


#endif

