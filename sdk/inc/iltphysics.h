#ifndef __ILTPHYSICS_H__
#define __ILTPHYSICS_H__

#ifndef __LTCODES_H__
#include "ltcodes.h"
#endif

#ifndef __LTBASEDEFS_H__
#include "ltbasedefs.h"
#endif

#ifndef __COLLISION_OBJECT_H__
#include "physics/collision_object.h"
#endif

#ifndef __COORDINATE_FRAME_H__
#include "physics/coordinate_frame.h"
#endif

#ifndef __LTMODULE_H__
#include "ltmodule.h"
#endif

#define SETDIMS_PUSHOBJECTS (1<<0)
#define MOVEOBJECT_TELEPORT (1<<0)

struct MoveInfo
{
    HOBJECT     m_hObject;
    float       m_dt;
    LTVector    m_Offset;
};


class ILTPhysics : public IBase
{
protected:

    ClientServerType m_ClientServerType;

public:
    interface_version(ILTPhysics, 0);

    virtual LTRESULT IsWorldObject(HOBJECT hObj);
    virtual LTRESULT GetStairHeight(float &fHeight) = 0;
    virtual LTRESULT SetStairHeight(float fHeight) = 0;
    virtual LTRESULT GetMass(HOBJECT hObj, float *m);
    virtual LTRESULT SetMass(HOBJECT hObj, float m);
    virtual LTRESULT GetFrictionCoefficient(HOBJECT hObj, float* u);
    virtual LTRESULT SetFrictionCoefficient(HOBJECT hObj, float u);
    virtual LTRESULT GetObjectDims(HOBJECT hObj, LTVector *d);
    virtual LTRESULT SetObjectDims(HOBJECT hObj, LTVector *d, uint32 flag) = 0;
    virtual LTRESULT GetVelocity(HOBJECT hObj, LTVector *v);
    virtual LTRESULT SetVelocity(HOBJECT hObj, const LTVector *v) = 0;
    virtual LTRESULT GetForceIgnoreLimit(HOBJECT hObj, float &f);
    virtual LTRESULT SetForceIgnoreLimit(HOBJECT hObj, float f);
    virtual LTRESULT GetAcceleration(HOBJECT hObj, LTVector *a);
    virtual LTRESULT SetAcceleration(HOBJECT hObj, const LTVector *a) = 0;
    virtual LTRESULT MoveObject(HOBJECT hObj, const LTVector *p, uint32 flag) = 0;
    virtual LTRESULT GetStandingOn(HOBJECT hObj, CollisionInfo *info);
    virtual LTRESULT GetGlobalForce(LTVector &a) = 0;
    virtual LTRESULT SetGlobalForce(const LTVector &a) = 0;
};

class ILTClientPhysics : public ILTPhysics
{
public:

    virtual LTRESULT UpdateMovement(MoveInfo*   pInfo) = 0;
    virtual LTRESULT MovePushObjects(HOBJECT hToMove,const LTVector& p,HOBJECT* hPushObjects,uint32 nPushObjects) = 0;
    virtual LTRESULT RotatePushObjects(HOBJECT hToMove,const LTRotation& q,HOBJECT* hPushObjects,uint32 nPushObjects) = 0;
};


#endif
//EOF
