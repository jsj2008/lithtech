// ----------------------------------------------------------------------- //
//
// MODULE  : ParticleSystemFX.h
//
// PURPOSE : The ParticleSystemFX object
//
// CREATED : 4/10/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __PARTICLESYSTEMFX_H__
#define __PARTICLESYSTEMFX_H__

#ifndef __BASEFX_H__
#	include "basefx.h"
#endif

#ifndef __PARTICLESYSTEMGROUP_H__
#	include "ParticleSystemGroup.h"
#endif

//----------------------------------------------------------------------------------
// CParticleSystemFX
//
//a particle group represents a collection of particles and maintains a single custom render object
//for visibility and rendering of the group
class CParticleSystemFX : 
	public CBaseFX
{
public:
	
	CParticleSystemFX();
	virtual ~CParticleSystemFX(); 

	virtual bool	Init(const FX_BASEDATA *pData, const CBaseFXProps *pProps);
	virtual bool	Update( float tmFrameTime );
	virtual bool	SuspendedUpdate( float tmFrameTime );
	virtual void	Term( void );
	virtual void	EnumerateObjects(TEnumerateObjectsFn pfnObjectCB, void* pUserData);

	virtual bool	IsVisibleWhileSuspended()	{ return HasAnyParticles(); }
	virtual bool	IsFinishedShuttingDown()	{ return !HasAnyParticles(); }

	
protected:

	//---------------------------------------
	// Particle Lifetime

	//called to determine if there are any more particles in this effect
	bool				HasAnyParticles() const;


	//called to allocate a new particle group that should be considered the active particle group.
	//this will setup the group as the active group.
	bool				AllocateActiveGroup(const LTRigidTransform& tObjTrans);

	//called to free any groups that are no longer needed back to the pool of groups
	void				FreeInactiveGroups();	

	//handles updating the particle system for the given time step. This will create particle groups and
	//coordinate updating the groups
	void				UpdateParticles(float tmDelta, float fUnitLifetime, const LTVector& vGravity, const LTRigidTransform& tObjTrans);

	//given the unit lifetime and the object space transform, this will determine the gravity vector
	//that should be used by the particles
	LTVector			GetParticleGravity(float fUnitLifetime, const LTRigidTransform& tObjTrans) const;

	const CParticleSystemProps*	GetProps() const	{ return (const CParticleSystemProps*)m_pProps; }

	//given an update time, this will handle accumulating the time into the elapsed update time
	//and returning whether or not the update should be handled
	bool				HandleUpdateTime(float fIntervalTime, float& fUpdateTime);

	//all particle simulations have at least one group. This group is created at initialization
	//and will always be used if possible. This setup is to avoid frequently having to allocate
	//groups, and avoids all allocations for the typical case of particle systems with only a single
	//group
	CParticleSystemGroup	m_GroupHead;

	//a pointer to the currently active group. This should never be NULL since if all groups are gone,
	//the active group should be the group head
	CParticleSystemGroup*	m_pActiveGroup;

	//the amount of time that has elapsed since the last emission
	float					m_tmElapsedEmission;			

	//the amount of time that has elapsed since the last group creation
	float					m_tmElapsedGroupCreation;

	//the material that the particles are rendered with
	HMATERIAL				m_hMaterial;

	//flag keeping track of whether or not this particle system was rendered this update
	bool					m_bWasVisible;

	//the amount of time that we want to wait until the next update
	float					m_fElapsedUpdate;
};

#endif // __PARTICLESYSTEMFX_H__
