// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#include "stdafx.h"
#include "AISteering.h"

CSteerable::CSteerable()
{
	Steerable_m_fMass = 1.0f;
	Steerable_m_fMaxForce = 1.0f;
	Steerable_m_fMaxSpeed = 250.0f;

	VEC_INIT(Steerable_m_vPosition);
	VEC_INIT(Steerable_m_vVelocity);

	VEC_INIT(Steerable_m_vUp);
	VEC_INIT(Steerable_m_vForward);
	VEC_INIT(Steerable_m_vRight);

	VEC_INIT(Steerable_m_vLastForce);
}


LTBOOL CSteerable::Steerable_Update(const LTVector& vSteeringDirection)
{
	// Now do the step update outlined in [Reynolds 99]

    LTVector vSteeringForce;
    LTVector vAcceleration;
    LTVector vVelocity;
    LTVector vPosition;

	// steering_force = truncate(steering_direction, max_force)

	vSteeringForce = vSteeringDirection;
	if ( VEC_MAG(vSteeringForce) > Steerable_GetMaxForce() )
	{
		vSteeringForce.Norm();
		vSteeringForce *= Steerable_GetMaxForce();
	}

	// acceleration = steering_force / mass

	vAcceleration = vSteeringForce/Steerable_GetMass();

	// velocity = truncate(velocity+acceleration, max_speed)

    vVelocity = Steerable_GetVelocity()+vAcceleration;//*g_pLTServer->GetFrameTime();
	if ( VEC_MAG(vVelocity) > Steerable_GetMaxSpeed() )
	{
		vVelocity.Norm();
		vVelocity *= Steerable_GetMaxSpeed();
	}

	// position = position + velocity

    vPosition = Steerable_GetPosition() + vVelocity*g_pLTServer->GetFrameTime();

	// now do our basis vectors - this is an approximation

    LTVector vForward;
    LTVector vRight;

	// new_forward = normalize(velocity);

	vForward = vVelocity;
	vForward.y = 0.0f;
	vForward.Norm();

	// new_right = cross(new_forward, up);

//	vRight = vForward.Cross(Steerable_GetUpVector());

	// update our member variables

	Steerable_SetPosition(vPosition);
	Steerable_SetVelocity(vVelocity);
	Steerable_SetForwardVector(vForward);

//	Steerable_SetRightVector(vRight);

	Steerable_m_vLastForce = vSteeringForce;

    return LTTRUE;
}


CSteeringMgr::CSteeringMgr()
{
    m_pSteerable = LTNULL;
}

void CSteeringMgr::Term()
{
    m_pSteerable = LTNULL;
}

LTBOOL CSteeringMgr::Init(CSteerable* pSteerable)
{
	_ASSERT(pSteerable);
    if ( !pSteerable ) return LTFALSE;

	m_pSteerable = pSteerable;

	if ( !m_SteeringSeek.Init(m_pSteerable) ||
		 !m_SteeringArrival.Init(m_pSteerable) )
	{
        return LTFALSE;
	}

    return LTTRUE;
}

void CSteeringMgr::Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags)
{
	m_SteeringSeek.Save(hWrite, dwSaveFlags);
	m_SteeringArrival.Save(hWrite, dwSaveFlags);
}

void CSteeringMgr::Load(HMESSAGEREAD hRead, uint32 dwLoadFlags)
{
	m_SteeringSeek.Load(hRead, dwLoadFlags);
	m_SteeringArrival.Load(hRead, dwLoadFlags);
}

void CSteeringMgr::EnableAllSteering()
{
	m_SteeringSeek.Enable();
	m_SteeringArrival.Enable();
}

void CSteeringMgr::DisableAllSteering()
{
	m_SteeringSeek.Disable();
	m_SteeringArrival.Disable();
}

void CSteeringMgr::EnableSteering(CSteering::SteeringType eSteering)
{
	switch ( eSteering )
	{
		case CSteering::eSteeringSeek:					m_SteeringSeek.Enable();				break;
		case CSteering::eSteeringArrival:				m_SteeringArrival.Enable();				break;
		default : _ASSERT(FALSE); break;
	}
}

void CSteeringMgr::DisableSteering(CSteering::SteeringType eSteering)
{
	switch ( eSteering )
	{
		case CSteering::eSteeringSeek:					m_SteeringSeek.Disable();				break;
		case CSteering::eSteeringArrival:				m_SteeringArrival.Disable();			break;
		default : _ASSERT(FALSE); break;
	}
}

CSteering* CSteeringMgr::GetSteering(CSteering::SteeringType eSteering)
{
	switch ( eSteering )
	{
		case CSteering::eSteeringSeek:					return &m_SteeringSeek;					break;
		case CSteering::eSteeringArrival:				return &m_SteeringArrival;				break;
        default : _ASSERT(FALSE);                       return LTNULL;                           break;
	}
}

LTBOOL CSteeringMgr::Update()
{
	// Pre update the steerable

	if ( !m_pSteerable->Steerable_PreUpdate() )
	{
        return LTFALSE;
	}

	// Use all the steering behaviors to determine a steering force

    LTVector vSteering(0,0,0);

	// Get all our steering directions

	if ( m_SteeringSeek.IsEnabled() )				vSteering += m_SteeringSeek.Update();
	if ( m_SteeringArrival.IsEnabled() )			vSteering += m_SteeringArrival.Update();

	// Update the steerable

	if ( !m_pSteerable->Steerable_Update(vSteering) )
	{
        return LTFALSE;
	}

	// Post update the steerable

	if ( !m_pSteerable->Steerable_PostUpdate() )
	{
        return LTFALSE;
	}

    return LTTRUE;
}

CSteering::CSteering()
{
	m_fPriority = 1.0f;
    m_pSteerable = LTNULL;
    m_bEnabled = LTFALSE;
}

LTBOOL CSteering::Init(CSteerable *pSteerable)
{
	_ASSERT(pSteerable);
    if ( !pSteerable ) return LTFALSE;

	m_pSteerable = pSteerable;

    return LTTRUE;
}

void CSteering::Term()
{
}

void CSteering::Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags)
{
	SAVE_FLOAT(m_fPriority);
	SAVE_BOOL(m_bEnabled);
}

void CSteering::Load(HMESSAGEREAD hRead, uint32 dwLoadFlags)
{
	LOAD_FLOAT(m_fPriority);
	LOAD_BOOL(m_bEnabled);
}

CSteeringSeek::CSteeringSeek() : CSteering()
{
	VEC_INIT(m_vTarget);
}

void CSteeringSeek::Term()
{
	CSteering::Term();

	VEC_INIT(m_vTarget);
}

LTVector CSteeringSeek::Update()
{
    LTVector vSteering;

	// desired_velocity = normalize(position-target)*max_speed;

	vSteering = m_vTarget - GetSteerable()->Steerable_GetPosition();
	vSteering.Norm();
	vSteering *= GetSteerable()->Steerable_GetMaxSpeed();

	// steering = desired_velocity - velocity

	vSteering -= GetSteerable()->Steerable_GetVelocity();

	return vSteering;
}

LTVector CSteeringArrival::Update()
{
    LTVector vSteering;

	// target offset = target - position

	vSteering = m_vTarget - GetSteerable()->Steerable_GetPosition();

	// distance = length(target_offset)

    LTFLOAT fDistance = VEC_MAG(vSteering);

    LTFLOAT fSpeed = VEC_MAG(GetSteerable()->Steerable_GetVelocity());

	// ramped speed = max speed * (distance / slowing distance)
	// we only used 90% arrival distance in order to guarantee that we will get to the point eventually

    LTFLOAT fRampedSpeed =  GetSteerable()->Steerable_GetMaxSpeed() * (fDistance/(.90f*m_fArrivalDistance));

	// clipped speed = minimum ( ramped speed , max speed )

    LTFLOAT fClippedSpeed = Min<float>(fRampedSpeed, GetSteerable()->Steerable_GetMaxSpeed());

	// desired velocity = ( clipped speed / distance ) * target offset

    LTVector vDesiredVelocity = vSteering*(fClippedSpeed/fDistance);

	// steering = desired_velocity - velocity

	vSteering = vDesiredVelocity - GetSteerable()->Steerable_GetVelocity();

	return vSteering;
}