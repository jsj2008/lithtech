// ----------------------------------------------------------------------- //
//
// MODULE  : ParticleSystemFX.cpp
//
// PURPOSE : The ParticleSystemFX object
//
// CREATED : 4/10/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h" 
#include "ParticleSystemFX.h"
#include "ParticleSystemProps.h"
#include "ClientFX.h"
#include <float.h>
#include "iltcustomrender.h"
#include "iltrenderer.h"

//-------------------------------------------------------------------------------------------
// Particle Group Management
//-------------------------------------------------------------------------------------------

//the bank for creating particle system groups
static CBankedList<CParticleSystemGroup>	g_ParticleGroupBank;

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CParticleSystemFX::CParticleSystemFX
//
//  PURPOSE:	Standard Constructor
//
// ----------------------------------------------------------------------- //

CParticleSystemFX::CParticleSystemFX( void ) :	
	CBaseFX(CBaseFX::eParticleSystemFX),
	m_tmElapsedEmission(0.0f),
	m_tmElapsedGroupCreation(0.0f),
	m_hMaterial(NULL),
	m_bWasVisible(false),
	m_fElapsedUpdate(0.0f)
{
	m_pActiveGroup = &m_GroupHead;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CParticleSystemFX::~CParticleSystemFX
//
//  PURPOSE:	Standard Destructor
//
// ----------------------------------------------------------------------- //

CParticleSystemFX::~CParticleSystemFX( void )
{
	Term();
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CParticleSystemFX::Init
//
//  PURPOSE:	Creates and initialises the Particle system
//
// ----------------------------------------------------------------------- //

bool CParticleSystemFX::Init(const FX_BASEDATA *pData, const CBaseFXProps *pProps)
{
	// Perform base class initialization
	if( !CBaseFX::Init(pData, pProps) ) 
		return false;

	//load up the material for this particle system, and assign it to the object
	m_hMaterial = g_pLTClient->GetRenderer()->CreateMaterialInstance(GetProps()->m_pszMaterial);
	if(!m_hMaterial)
	{
		Term();
		return false;
	}
	
	LTRigidTransform tObjTrans;
	GetCurrentTransform(0.0f, tObjTrans.m_vPos, tObjTrans.m_rRot);

	//make sure to initialize our main group object
	if(!m_GroupHead.Init(m_pFxMgr, m_hMaterial, tObjTrans, &m_bWasVisible, GetProps()))
	{
		Term();
		return false;
	}

	//assume that we were visible initially in order to catch the initial update
	m_bWasVisible = true;

	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CParticleSystemFX::Term
//
//  PURPOSE:	NONE
//
// ----------------------------------------------------------------------- //

void CParticleSystemFX::Term( void )
{
	//run through and terminate all of our groups
	CParticleSystemGroup* pCurrGroup = &m_GroupHead;
	do
	{
		//cache our next group so we can clean up this object
		CParticleSystemGroup* pNextGroup = pCurrGroup->m_pNextGroup;

		//clean up our object
		pCurrGroup->Term();
		pCurrGroup->m_pNextGroup = NULL;

		//free the object
		if(pCurrGroup != &m_GroupHead)
			g_ParticleGroupBank.Delete(pCurrGroup);

		//and move to the next object to cleanup
		pCurrGroup = pNextGroup;
	}
	while(pCurrGroup);

	//and reset our active group back to the group head
	m_pActiveGroup = &m_GroupHead;

	if(m_hMaterial)
	{
		g_pLTClient->GetRenderer()->ReleaseMaterialInstance(m_hMaterial);
		m_hMaterial = NULL;
	}
}

//called to allocate a new particle group that should be considered the active particle group.
//this will setup the group as the active group.
bool CParticleSystemFX::AllocateActiveGroup(const LTRigidTransform& tObjTrans)
{
	//run through the list of the objects that we already have and see if we can just reuse one of
	//them
	CParticleSystemGroup* pCurrGroup = &m_GroupHead;
	CParticleSystemGroup* pLastGroup = NULL;
	do
	{
		//see if this object can be reused
		if(pCurrGroup->GetNumParticles() == 0)
		{
			//we can just reuse this one
			m_pActiveGroup = pCurrGroup;
			return true;
		}

		//onto the next group
		pLastGroup = pCurrGroup;
		pCurrGroup = pCurrGroup->m_pNextGroup;
	}
	while(pCurrGroup);

	//at this point, we now have the last group and we need to allocate a new group and set this
	//as our active group
	CParticleSystemGroup* pNewGroup = g_ParticleGroupBank.New();
	if(!pNewGroup)
		return false;

	//attempt to initialize this object
	if(!pNewGroup->Init(m_pFxMgr, m_hMaterial, tObjTrans, &m_bWasVisible, GetProps()))
	{
		g_ParticleGroupBank.Delete(pNewGroup);
		return false;
	}

	//add this to our list
	pLastGroup->m_pNextGroup = pNewGroup;
	m_pActiveGroup = pNewGroup;

	//success
	return true;
}

//called to free any groups that are no longer needed back to the pool of groups
void CParticleSystemFX::FreeInactiveGroups()
{
	//what we need to do is start at the list of the head and just free particle groups
	//that have no particles and are not the active group. However, if the active group
	//has no particles, and the group head has no particles, then we can just reassign the
	//active group to be the group head as an optimization

	if((m_pActiveGroup->GetNumParticles() == 0) && (m_GroupHead.GetNumParticles() == 0))
	{
		//the group has no particles, reassign it to be the active group
		m_pActiveGroup = &m_GroupHead;
	}

	//now we can run through and free any groups (other than the head group) that is not
	//the active group
	CParticleSystemGroup* pCurrGroup = m_GroupHead.m_pNextGroup;
	CParticleSystemGroup* pPrevGroup = &m_GroupHead;

	while(pCurrGroup)
	{
		if((pCurrGroup->GetNumParticles() == 0) && (m_pActiveGroup != pCurrGroup))
		{
			//update our list to remove the group
			pPrevGroup->m_pNextGroup = pCurrGroup->m_pNextGroup;

			g_ParticleGroupBank.Delete(pCurrGroup);

			//and move along in our list
			pCurrGroup = pPrevGroup->m_pNextGroup;
		}
		else
		{
			//valid group, just skip over it
			pPrevGroup = pCurrGroup;
			pCurrGroup = pCurrGroup->m_pNextGroup;
		}
	}
}

//given an update time, this will handle accumulating the time into the elapsed update time
//and returning whether or not the update should be handled
bool CParticleSystemFX::HandleUpdateTime(float fIntervalTime, float& fUpdateTime)
{
	m_fElapsedUpdate += fIntervalTime;

	//the update rate for invisible effects
	static const float kfInvisibleUpdateRate = 0.3f;

	//we can avoid this update if we weren't visible and this isn't the initial update (which
	//means we need to handle to emit our first batch)
	if(!m_bWasVisible && !IsInitialFrame())
	{
		//we weren't visible, see if it is time for our invisible update
		if(m_fElapsedUpdate < kfInvisibleUpdateRate)
			return false;
	}

	fUpdateTime = m_fElapsedUpdate;

	//and reset our state so that way it will be correct next iteration
	m_fElapsedUpdate = 0.0f;
	m_bWasVisible = false;

	return true;
}


bool CParticleSystemFX::SuspendedUpdate( float tmFrameTime )
{
	if(!CBaseFX::SuspendedUpdate(tmFrameTime))
		return false;

	float fUpdateTime;
	if(!HandleUpdateTime(tmFrameTime, fUpdateTime))
		return true;

	//precalculate a couple of properties for use with updating
	float fUnitLifetime = GetUnitLifetime();

	LTRigidTransform tObjTrans;
	GetCurrentTransform(fUnitLifetime, tObjTrans.m_vPos, tObjTrans.m_rRot);

	LTVector vGravity = GetParticleGravity(fUnitLifetime, tObjTrans);

	//and now update the particles for this interval
	UpdateParticles(fUpdateTime, fUnitLifetime, vGravity, tObjTrans);

	//and make sure to remove inactive groups so that we can terminate properly
	FreeInactiveGroups();

	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CParticleSystemFX::Update
//
//  PURPOSE:	NONE
//
// ----------------------------------------------------------------------- //

bool CParticleSystemFX::Update( float tmFrameTime )
{
	//allow the base effect to handle any updates
	BaseUpdate(tmFrameTime);

	float fUpdateTime;
	if(!HandleUpdateTime(tmFrameTime, fUpdateTime))
		return true;

	//precalculate a couple of properties for use with updating
	float fUnitLifetime = GetUnitLifetime();

	LTRigidTransform tObjTrans;
	GetCurrentTransform(fUnitLifetime, tObjTrans.m_vPos, tObjTrans.m_rRot);

	LTVector vGravity = GetParticleGravity(fUnitLifetime, tObjTrans);

	//determine whether or not we should actually perform group creation
	float fGroupCreationTime = GetProps()->m_fGroupCreationInterval;
	bool bCreateGroups = (fGroupCreationTime > 0.001f);

	//as long as we aren't shutting down, we need to handle the possibility of
	//emitting particles
	if(!IsShuttingDown())
	{
		bool bPlacedDefaultBatch = false;

		//if this is the first frame, we need to handle emitting particles
		if(IsInitialFrame())
		{
			//create a new group for this lifetime if this particle system is broken
			//into groups
			if(bCreateGroups)
			{
				if(!AllocateActiveGroup(tObjTrans))
					return false;
			}

			//place our default batch marker so that our base particles update correctly
			if(!bPlacedDefaultBatch)
			{
				m_pActiveGroup->AddParticleBatchMarker(0.0f, true);
				bPlacedDefaultBatch = true;
			}

			//always emit the initial batch of the particles at time zero
			m_pActiveGroup->EmitParticleBatch(0.0f, GetElapsed(), tObjTrans);

			//and reset our counters for these operations so they will be done at correct
			//timings
			m_tmElapsedEmission	= 0.0f;
			m_tmElapsedGroupCreation = 0.0f;
		}		
		else
		{		
		//see if we want to emit any more particles
		float fEmissionInterval = GetProps()->m_fEmissionInterval;
		if(fEmissionInterval > 0.0f)
		{
			//the maximum lifespan of the particle for this given update
			float fMaxLifespan = GetProps()->m_ffcMaxLifetime.GetValue(fUnitLifetime);

			//the time offset to our next emission
			float fFirstEmission = fEmissionInterval - m_tmElapsedEmission;

			//determine if we even need to emit
			if(fUpdateTime < fFirstEmission)
			{
				//don't need to emit, just update our counters
				m_tmElapsedEmission += fUpdateTime;
				m_tmElapsedGroupCreation += fUpdateTime;
			}
			else
			{
				//this is the amount of time that we have processed so far
				float fIntervalLeft = fUpdateTime - fFirstEmission;

				//and we need to process the full range of the time step to handle emitting particles and groups
				while(1)
				{
					//determine if we can complete the step, and those particles will live long enough to bother
					//to create
						if(fIntervalLeft < fMaxLifespan)
					{
						//advance forward our group creation, and see if we need to create a new group
						m_tmElapsedGroupCreation += fEmissionInterval;

						if(bCreateGroups && (m_tmElapsedGroupCreation >= fGroupCreationTime))
						{
							//allocate a new group
							if(!AllocateActiveGroup(tObjTrans))
								return false;
							
							//no need to update the new active group to cover the elapsed time since it is
							//empty

							//no need to place a batch group at this point since we have no particles
							bPlacedDefaultBatch = true;

							//reset our elapsed group time
							m_tmElapsedGroupCreation = fmodf(m_tmElapsedGroupCreation, fGroupCreationTime);
						}
						
						//place our default batch marker so that our base particles update correctly
						if(!bPlacedDefaultBatch)
						{
								m_pActiveGroup->AddParticleBatchMarker(fIntervalLeft, true);
							bPlacedDefaultBatch = true;
						}

						//and now that we have properly stepped our active group, we can add our new particles
						//to the current group
						m_pActiveGroup->EmitParticleBatch(fUnitLifetime, fIntervalLeft, tObjTrans);

						//and reset our emission counter since we just emitted
						m_tmElapsedEmission = 0.0f;

						//and shrink our interval by the time step
						fIntervalLeft -= fEmissionInterval;
					}
					//we can complete this step, but the particles will just die before we can see them
					//so don't bother creating them
					else
					{
						//pretend we created an emission at this point
						m_tmElapsedEmission = 0.0f;
						m_tmElapsedGroupCreation += fEmissionInterval;
						fIntervalLeft -= fEmissionInterval;
					}

						//determine if we can't complete the full time step, i.e. our last step of this update
						if(fEmissionInterval > fIntervalLeft)
						{
							//we cannot fit in the remainder of this time, so just accumulate it into our counters
							m_tmElapsedGroupCreation += fIntervalLeft;
							m_tmElapsedEmission += fIntervalLeft;
							break;
						}
					}
				}
			}
		}
	}

	//we are shutting down, so just update all of the particles for the frame time
	UpdateParticles(fUpdateTime, fUnitLifetime, vGravity, tObjTrans );

	//at this point we can free any unused groups. We don't free in the middle of the update
	//so that way removed and added groups can use the same memory
	FreeInactiveGroups();

	//see if we want to shut down (can only do this if we have no particles and are not going to emit
	//again)
	if( ((GetElapsed() + GetProps()->m_fEmissionInterval >= GetLifetime()) || GetProps()->m_fEmissionInterval <= 0.0f) &&
		!HasAnyParticles() && !GetProps()->m_bContinuous)
		return false;

	return true;
}

//called to enumerate through each of the objects and will call into the provided function for each
void CParticleSystemFX::EnumerateObjects(TEnumerateObjectsFn pfnObjectCB, void* pUserData)
{
	CParticleSystemGroup* pCurrGroup = &m_GroupHead;
	do 
	{
		//see if this group has an object
		HOBJECT hObj = pCurrGroup->GetObject();
		if(hObj)
		{
			pfnObjectCB(this, hObj, pUserData);
		}

		//onto the next group
		pCurrGroup = pCurrGroup->m_pNextGroup;

	} while(pCurrGroup);
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CParticleSystemFX::AddParticles
//
//  PURPOSE:	Emit Particles
//
// ----------------------------------------------------------------------- //

//given the unit lifetime and the object space transform, this will determine the gravity vector
//that should be used by the particles
LTVector CParticleSystemFX::GetParticleGravity(float fUnitLifetime, const LTRigidTransform& tObjTrans) const
{
	LTVector vWorldGravity;
	g_pLTClient->Physics()->GetGlobalForce(vWorldGravity);

	//transform gravity into object space if necessary
	if(GetProps()->m_bObjectSpace)
	{
		vWorldGravity = tObjTrans.m_rRot.Conjugate().RotateVector(vWorldGravity);
	}

	float fGravityScale		= GetProps()->m_ffcGravityScale.GetValue(fUnitLifetime);
	LTVector vAcceleration	= GetProps()->m_vfcAcceleration.GetValue(fUnitLifetime);

	LTVector vGravity = (vAcceleration + vWorldGravity * fGravityScale);
	return vGravity;
}

//handles updating the particle system for the given time step. This will create particle groups and
//coordinate updating the groups
void CParticleSystemFX::UpdateParticles(float tmDelta, float fUnitLifetime, const LTVector& vGravity, const LTRigidTransform& tObjTrans)
{
	//ignore updates of too small of a delta
	static const float kfMinParticleUpdate = 0.001f;
	if(tmDelta < kfMinParticleUpdate)
		return;

	//find the frictional coefficient (this is what we multiply the velocity vector by).
	//this needs to be scaled based upon the time delta so that the friction amount is
	//reduced by the amount specified per second
	float fFrictionCoef = 1.0f - GetProps()->m_ffcDrag.GetValue(fUnitLifetime);

	//now run through the listing of our system groups and update each one in turn
	CParticleSystemGroup* pGroup = &m_GroupHead;
	do 
	{
		//determine if our current group is the active one
		pGroup->UpdateParticles(tmDelta, vGravity, fFrictionCoef, tObjTrans);
		pGroup = pGroup->m_pNextGroup;
	} 
	while(pGroup);
}

//called to determine if there are any more particles in this effect
bool CParticleSystemFX::HasAnyParticles() const
{
	//run through the list and return success if we find any groups with particles
	//(note that this looks like a linear operation, but since this is compacted after every
	//update, this is closer to constant time since either the first or second element should
	//always have something or the list have no following elements)
	const CParticleSystemGroup* pGroup = &m_GroupHead;
	do 
	{
		if(pGroup->GetNumParticles() > 0)
			return true;

		pGroup = pGroup->m_pNextGroup;
	} 
	while(pGroup);

	return false;
}






