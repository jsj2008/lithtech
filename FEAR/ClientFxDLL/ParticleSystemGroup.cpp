#include "stdafx.h" 
#include "ParticleSystemGroup.h"
#include "ParticleSystemProps.h"
#include "iltcustomrender.h"
#include "iltrenderer.h"
#include "clientfx.h"
#include "ClientFXVertexDeclMgr.h"
#include "VarTrack.h"
#include "iperformancemonitor.h"
#include "GameRenderLayers.h"

//our object used for tracking performance for effect
static CTimedSystem g_tsClientFXParticles("ClientFX_Particles", "ClientFX");

//-------------------------------------------------------------------------------------------
// Particle Structures
//-------------------------------------------------------------------------------------------

#define PARTICLE_BOUNCE				(1<<28)
#define PARTICLE_SPLAT				(1<<29)
#define PARTICLE_BATCH_MARKER		(1<<30)
#define PARTICLE_DEFAULT_BATCH		(1<<31)
#define PARTICLE_IMAGE_MASK			(0xF)

//represents the data for a single particle
struct SParticle
{
	LTVector    m_Pos;				//Current position of the particle
	LTVector    m_Velocity;			//Current velocity of the particle

	float       m_fLifetime;         //Current lifetime left
	float       m_fTotalLifetime;    //Total lifetime (i.e. initial value)

	float		m_fAngle;			//Angle in radians of this particle
	float		m_fAngularVelocity;	//Velocity of the angular change in radians per second

	uint32		m_nUserData;		//the top two bits are the above flags, bottom 4 bits are the image index
};

//console variable that controls the scale of the amount of particles that will bounce
VarTrack	g_vtParticleBounceScale;

//-------------------------------------------------------------------------------------------
// Utility Functions
//-------------------------------------------------------------------------------------------

//utility function that will generate a point on the specified plane
static LTVector GenerateDiskPoint(const LTVector& vAxis1, const LTVector& vAxis2, float fMinRadius, float fMaxRadius)
{
	//determine a random angle
	float fAngle = GetRandom(0.0f, MATH_CIRCLE);

	//create a directional vector
	LTVector vDirection = vAxis1 * LTCos(fAngle) + vAxis2 * LTSin(fAngle);

	//scale by distance
	vDirection *= GetRandom(fMinRadius, fMaxRadius);

	//success
	return vDirection;
}

//given a particle position, a limit on the velocities, and the acceleration type, this will generate the 
//appropriate velocity
static LTVector GenerateObjectSpaceParticleVel(ePSVelocityType eType, const LTVector& vObjSpacePos, 
											   const LTVector& vMinVelocity, const LTVector& vMaxVelocity)
{
	LTVector vVel(0, 0, 0);

	// Randomize the velocity within our range
	switch(eType)
	{
	case PSV_eRandom:
		{
			vVel.x = GetRandom( vMinVelocity.x, vMaxVelocity.x );
			vVel.y = GetRandom( vMinVelocity.y, vMaxVelocity.y );
			vVel.z = GetRandom( vMinVelocity.z, vMaxVelocity.z );
		}
		break;
	case PSV_eCenter:
		{
			//velocity direction is based upon position from 0, 0, 0
			float fMag = LTMAX(vObjSpacePos.Mag(), 0.01f);
			vVel = vObjSpacePos * (GetRandom(vMinVelocity.x, vMaxVelocity.x) / fMag);
		}
		break;
	default:
		LTERROR( "Unknown particle velocity type");
		break;
	}

	return vVel;
}

//utility function for setting up a particle's vertices given four positions
static void SetupParticle(STexTangentSpaceVert* pVerts, 
						  const LTVector& vPos0, const LTVector& vPos1,
						  const LTVector& vPos2, const LTVector& vPos3,
						  uint32 nColor, const LTVector& vNormal,
						  const LTVector& vTangent,
						  const LTVector& vBinormal,
						  uint32 nParticleImage, float fUImageWidth)
{
	//determine the U range
	float fUMin = (float)nParticleImage * fUImageWidth;
	float fUMax = fUMin + fUImageWidth;

	pVerts[0].m_vPos = vPos0;
	pVerts[0].m_vUV.Init(fUMin, 0.0f);
	pVerts[0].m_nPackedColor = nColor;
	pVerts[0].m_vNormal = vNormal;
	pVerts[0].m_vTangent = vTangent;
	pVerts[0].m_vBinormal = vBinormal;

	pVerts[1].m_vPos = vPos1;
	pVerts[1].m_vUV.Init(fUMax, 0.0f);
	pVerts[1].m_nPackedColor = nColor;
	pVerts[1].m_vNormal = vNormal;
	pVerts[1].m_vTangent = vTangent;
	pVerts[1].m_vBinormal = vBinormal;

	pVerts[2].m_vPos = vPos2;
	pVerts[2].m_vUV.Init(fUMax, 1.0f);
	pVerts[2].m_nPackedColor = nColor;
	pVerts[2].m_vNormal = vNormal;
	pVerts[2].m_vTangent = vTangent;
	pVerts[2].m_vBinormal = vBinormal;

	pVerts[3].m_vPos = vPos3;
	pVerts[3].m_vUV.Init(fUMin, 1.0f);
	pVerts[3].m_nPackedColor = nColor;
	pVerts[3].m_vNormal = vNormal;
	pVerts[3].m_vTangent = vTangent;
	pVerts[3].m_vBinormal = vBinormal;
}

//-------------------------------------------------------------------------------------------
// CParticleSystemGroup
//-------------------------------------------------------------------------------------------

CParticleSystemGroup::CParticleSystemGroup() :
	m_pNextGroup(NULL),
	m_nNumRayTestParticles(0),
	m_hCustomRender(NULL),
	m_pProps(NULL),
	m_pFxMgr(NULL),
	m_pVisibleFlag(NULL)
{
	//setup our particle stride
	m_Particles.SetStride(sizeof(SParticle));
}

CParticleSystemGroup::~CParticleSystemGroup()
{
	Term();
}

//called to initialize a group and create the appropriate objects
bool CParticleSystemGroup::Init(IClientFXMgr* pFxMgr, HMATERIAL hMaterial, const LTRigidTransform& tObjTrans, 
								bool* pVisibleFlag, const CParticleSystemProps* pProps)
{
	//make sure our console variable is initialized
	if(!g_vtParticleBounceScale.IsInitted())
		g_vtParticleBounceScale.Init(g_pLTClient, "ParticleBounceScale", NULL, 1.0f);

	//make sure we are in a valid state
	Term();

	ObjectCreateStruct	ocs;

	// Create Particle System
	ocs.m_ObjectType	= OT_CUSTOMRENDER;
	ocs.m_Flags			|= FLAG_VISIBLE;

	if(!pProps->m_bSolid)
		ocs.m_Flags2 |= FLAG2_FORCETRANSLUCENT;

	if(!pProps->m_bTranslucentLight)
		ocs.m_Flags |= FLAG_NOLIGHT;

	//setup whether or not it is in the sky
	ocs.m_Flags2 |= GetSkyFlags(pProps->m_eInSky);

	ocs.m_Pos = tObjTrans.m_vPos;
	ocs.m_Rotation = tObjTrans.m_rRot;

	m_hCustomRender = g_pLTClient->CreateObject( &ocs );
	if( !m_hCustomRender )
		return false;

	//setup our rendering layer
	if(pProps->m_bPlayerView)
		g_pLTClient->GetRenderer()->SetObjectDepthBiasTableIndex(m_hCustomRender, eRenderLayer_Player);

	LTVector vTempBBox(1.0f, 1.0f, 1.0f);

	g_pLTClient->GetCustomRender()->SetRenderingSpace(m_hCustomRender, pProps->m_bObjectSpace ? eRenderSpace_Object : eRenderSpace_World);
	g_pLTClient->GetCustomRender()->SetVisBoundingBox(m_hCustomRender, -vTempBBox, vTempBBox);
	g_pLTClient->GetCustomRender()->SetRenderCallback(m_hCustomRender, CustomRenderCallback);
	g_pLTClient->GetCustomRender()->SetCallbackUserData(m_hCustomRender, this);

	//assign the material to the object
	g_pLTClient->GetCustomRender()->SetMaterial(m_hCustomRender, hMaterial);

	//and save our other values passed in
	m_pProps		= pProps;
	m_pFxMgr		= pFxMgr;
	m_pVisibleFlag	= pVisibleFlag;

	//success
	return true;
}

//called to terminate this object and place it into an invalid state
void CParticleSystemGroup::Term()
{
	m_nNumRayTestParticles = 0;
	m_Particles.FreeAllParticles();

	if(m_hCustomRender)
	{
		g_pLTClient->RemoveObject(m_hCustomRender);
		m_hCustomRender = NULL;
	}
}

//called to add a particle batch marker onto our listing of particles
void CParticleSystemGroup::AddParticleBatchMarker(float fUpdateTime, bool bDefault)
{
	SParticle* pParticle = (SParticle*)m_Particles.AllocateParticle();
	if(!pParticle)
	{
		//we are out of memory
		return;
	}

	//designate this particle as a batch marker
	pParticle->m_nUserData = PARTICLE_BATCH_MARKER;
	if(bDefault)
		pParticle->m_nUserData |= PARTICLE_DEFAULT_BATCH;

	//store our batch time in the total lifetime
	pParticle->m_fTotalLifetime = fUpdateTime;

	//we only check for batches in the dead particle branch to avoid branches, so the lifetime must
	//be expired
	pParticle->m_fLifetime		= -1.0f;
}

//called to emit a batch of particles given the properties 
void CParticleSystemGroup::EmitParticleBatch(float fUnitLifetime, float fUpdateTime, const LTRigidTransform& tObjTrans)
{
	LTASSERT(m_pProps, "Error: Called EmitParticleBatch on an uninitialized particle group");

	//determine the number of particles that we are going to emit (and bail if we aren't going to emit any)
	uint32 nParticlesToEmit = m_pProps->m_nfcParticlesPerEmission.GetValue(fUnitLifetime);
	if(nParticlesToEmit == 0)
		return;

	LTMatrix3x4 mMat;
	tObjTrans.ToMatrix(mMat);

	float fPercentToBounce		= m_pProps->m_ffcPercentToBounce.GetValue(fUnitLifetime);
	float fPercentToSplat		= m_pProps->m_ffcPercentToSplat.GetValue(fUnitLifetime);

	float fMinLifetime			= m_pProps->m_ffcMinLifetime.GetValue(fUnitLifetime); 
	float fMaxLifetime			= m_pProps->m_ffcMinLifetime.GetValue(fUnitLifetime);

	float fMinRadius			= m_pProps->m_ffcMinRadius.GetValue(fUnitLifetime);
	float fMaxRadius			= m_pProps->m_ffcMaxRadius.GetValue(fUnitLifetime);

	LTVector vEmissionOffset	= m_pProps->m_vfcEmissionOffset.GetValue(fUnitLifetime);
	LTVector vEmissionDims		= m_pProps->m_vfcEmissionDims.GetValue(fUnitLifetime);

	LTVector vMinVelocity		= m_pProps->m_vfcMinVelocity.GetValue(fUnitLifetime);
	LTVector vMaxVelocity		= m_pProps->m_vfcMaxVelocity.GetValue(fUnitLifetime);

	//apply the global particle scale if appropriate
	if(m_pProps->m_bEnableBounceScale)
	{
		fPercentToBounce	*= g_vtParticleBounceScale.GetFloat(1.0f);
		fPercentToSplat		*= g_vtParticleBounceScale.GetFloat(1.0f);
	}

	//determine if we will be bouncing or splatting (avoids some expensive fcmps)
	bool bBounceParticles		= (fPercentToBounce > 0.001f);
	bool bSplatParticles		= (fPercentToSplat > 0.001f) && !LTStrEmpty(m_pProps->m_pszSplatEffect);

	//run through and add all of the particles
	for( uint32 nCurrParticle = 0; nCurrParticle < nParticlesToEmit; nCurrParticle++ )
	{
		SParticle* pParticle = (SParticle*)m_Particles.AllocateParticle();
		if(!pParticle)
		{
			//we are out of memory
			break;
		}

		LTVector vPos = GenerateObjectSpaceParticlePos(vEmissionOffset, vEmissionDims, fMinRadius, fMaxRadius);
		LTVector vVel = GenerateObjectSpaceParticleVel(m_pProps->m_eVelocityType, vPos, vMinVelocity, vMaxVelocity);

		//convert velocity and position into world space
		if (!m_pProps->m_bObjectSpace)
		{
			vPos = mMat * vPos;
			vVel = mMat.Transform3x3(vVel);
		}

		float fParticleLifespan = GetRandom( fMinLifetime, fMaxLifetime );

		// Try and add the new particle to the system
		pParticle->m_Pos				= vPos;
		pParticle->m_Velocity			= vVel;
		pParticle->m_fLifetime			= fParticleLifespan;
		pParticle->m_fTotalLifetime		= fParticleLifespan;
		pParticle->m_nUserData			= rand() % m_pProps->m_nNumImages;

		// Randomize the angle information if needed
		if(m_pProps->m_bRotate)
		{
			pParticle->m_fAngle				= GetRandom(0.0f, MATH_CIRCLE);
			pParticle->m_fAngularVelocity	= GetRandom(m_pProps->m_fMinAngularVelocity, m_pProps->m_fMaxAngularVelocity);
		}
		else
		{
			pParticle->m_fAngle				= 0.0f;
			pParticle->m_fAngularVelocity	= 0.0f;
		}

		//determine if we want this particle to bounce
		if(bBounceParticles && (GetRandom(0.0f, 100.0f) < fPercentToBounce))
		{
			//this particle should bounce
			pParticle->m_nUserData |= PARTICLE_BOUNCE;
			m_nNumRayTestParticles++;
		}

		//determine if we want this particle to splat
		if(bSplatParticles && (GetRandom(0.0f, 100.0f) < fPercentToSplat))
		{
			//this particle should bounce
			pParticle->m_nUserData |= PARTICLE_SPLAT;
			m_nNumRayTestParticles++;
		}			
	}

	//add a batch marker for this group
	AddParticleBatchMarker(fUpdateTime, false);
}

//called to remove a particle using the particle iterator
CParticleReverseIterator CParticleSystemGroup::RemoveParticle(CParticleReverseIterator& Iterator)
{
	SParticle* pParticle = (SParticle*)Iterator.GetParticle();

	if(pParticle->m_nUserData & PARTICLE_BOUNCE)
	{
		m_nNumRayTestParticles--;
	}
	if(pParticle->m_nUserData & PARTICLE_SPLAT)
	{
		m_nNumRayTestParticles--;
	}

	return m_Particles.RemoveParticle(Iterator);
}

//called to handle updating of a batch of particles given the appropriate properties
void CParticleSystemGroup::UpdateParticles(float tmFrame, const LTVector& vGravity, float fFrictionCoef, const LTRigidTransform& tObjTrans)
{
	LTASSERT(m_pProps, "Error: Called UpdateParticles on an uninitialized particle group");

	//track our performance
	CTimedSystemBlock TimingBlock(g_tsClientFXParticles);

	//get an iterator to our list of particles
	CParticleReverseIterator itParticles = m_Particles.GetReverseIterator();

	//bail if we have no particles
	if(itParticles.IsDone())
		return;

	//find the coefficient of restitution to use for these particles in case they bounce
	float fCOR = m_pProps->m_fBounceStrength;

	//do our particles have infinite lifetime?
	bool bInfiniteLife = m_pProps->m_bInfiniteLife;

	//initialize our particle bounding box to extreme extents
	static const float kfInfinity = FLT_MAX;
	LTVector vMin = LTVector(kfInfinity, kfInfinity, kfInfinity);
	LTVector vMax = LTVector(-kfInfinity, -kfInfinity, -kfInfinity);

	LTVector vDefaultGravity	= vGravity * tmFrame;
	float fDefaultFriction		= powf(fFrictionCoef, tmFrame);

	//the current gravity and friction for us to use
	LTVector vCurrGravity	= vDefaultGravity;
	float fCurrFriction		= fDefaultFriction;
	float fCurrUpdateTime	= tmFrame;

	//we now need to handle updating the particles. For performance reasons, this is broken apart
	//into two update loops, one that handles bouncing/splat, another that doesn't
	if(m_nNumRayTestParticles == 0)
	{
		//this is the non-bouncing update loop
		while(!itParticles.IsDone())
		{
			SParticle* pParticle = (SParticle*)itParticles.GetParticle();

			//update the lifetime
			pParticle->m_fLifetime -= fCurrUpdateTime;

			// Check for expiration
			if( pParticle->m_fLifetime <= 0.0f )
			{
				if(pParticle->m_nUserData & PARTICLE_BATCH_MARKER)
				{
					//restore our defaults
					if(pParticle->m_nUserData & PARTICLE_DEFAULT_BATCH)
					{
						//restore our defaults
						vCurrGravity	= vDefaultGravity;
						fCurrFriction	= fDefaultFriction;
						fCurrUpdateTime = tmFrame;
					}
					else
					{
						//compute new values for us to use
						vCurrGravity	= vGravity * pParticle->m_fTotalLifetime;
						fCurrFriction	= powf(fFrictionCoef, pParticle->m_fTotalLifetime);
						fCurrUpdateTime	= pParticle->m_fTotalLifetime;
					}						

					//do the direct remove (we know batch markers don't have bounce or splat)
					itParticles = m_Particles.RemoveParticle(itParticles);
					continue;
				}
				else if(bInfiniteLife)
				{
					//this particle has died, but resurrect it since it lives forever
					pParticle->m_fLifetime = pParticle->m_fTotalLifetime - fmodf(-pParticle->m_fLifetime, pParticle->m_fTotalLifetime); 				
				}
				else
				{
					//remove the dead particle (can do direct version since we know we don't have splat or bounce)
					itParticles = m_Particles.RemoveParticle(itParticles);
					continue;
				}
			}

			// Give the particle an update

			//update the velocity, applying gravity and friction
			pParticle->m_Velocity = pParticle->m_Velocity * fCurrFriction + vCurrGravity;
			pParticle->m_Pos	 += pParticle->m_Velocity * fCurrUpdateTime;

			// Update the angle if appropriate
			pParticle->m_fAngle	+= pParticle->m_fAngularVelocity * fCurrUpdateTime;

			//extend the bounding box
			vMin.Min(pParticle->m_Pos);
			vMax.Max(pParticle->m_Pos);

			if(m_pProps->m_bStreak)
			{
				LTVector vStreakPt = pParticle->m_Pos - pParticle->m_Velocity * m_pProps->m_fStreakScale;
				vMin.Min(vStreakPt);
				vMax.Max(vStreakPt);
			}

			//and move onto the next particle
			itParticles.Prev();
		}
	}
	else
	{
		//this is the bouncing/splat update loop
		IntersectQuery		iQuery;
		IntersectInfo		iInfo;

		//get the main world that we are going to test against
		HOBJECT hMainWorld = g_pLTClient->GetMainWorldModel();

		//cache the inverse object transform
		LTRigidTransform tInvObjTrans = tObjTrans.GetInverse();

		while(!itParticles.IsDone())
		{
			SParticle* pParticle = (SParticle*)itParticles.GetParticle();

			//update the lifetime
			pParticle->m_fLifetime -= fCurrUpdateTime;

			// Check for expiration
			if( pParticle->m_fLifetime <= 0.0f )
			{
				if(pParticle->m_nUserData & PARTICLE_BATCH_MARKER)
				{
					//restore our defaults
					if(pParticle->m_nUserData & PARTICLE_DEFAULT_BATCH)
					{
						//restore our defaults
						vCurrGravity	= vDefaultGravity;
						fCurrFriction	= fDefaultFriction;
						fCurrUpdateTime = tmFrame;
					}
					else
					{
						//compute new values for us to use
						vCurrGravity	= vGravity * pParticle->m_fTotalLifetime;
						fCurrFriction	= powf(fFrictionCoef, pParticle->m_fTotalLifetime);
						fCurrUpdateTime	= pParticle->m_fTotalLifetime;
					}						

					//do the direct remove (we know batch markers don't have bounce or splat)
					itParticles = m_Particles.RemoveParticle(itParticles);
					continue;
				}
				else if(bInfiniteLife)
				{
					//this particle has died, but resurrect it since it lives forever
					pParticle->m_fLifetime = pParticle->m_fTotalLifetime - fmodf(-pParticle->m_fLifetime, pParticle->m_fTotalLifetime); 				
				}
				else
				{
					//remove the dead particle (can't do direct version since it might have splat or bounce)
					itParticles = RemoveParticle(itParticles);
					continue;
				}
			}

			// Give the particle an update

			//update the velocity, applying gravity and friction
			pParticle->m_Velocity = pParticle->m_Velocity * fCurrFriction + vCurrGravity;

			// Update the angle if appropriate
			pParticle->m_fAngle	+= pParticle->m_fAngularVelocity * fCurrUpdateTime;

			//determine where the particle should be moving to
			LTVector vDestPos = pParticle->m_Pos + pParticle->m_Velocity * fCurrUpdateTime;

			//we now need to compute the new position of the particle
			if(pParticle->m_nUserData & (PARTICLE_BOUNCE | PARTICLE_SPLAT))
			{
				LTVector vParticlePos = pParticle->m_Pos;
				LTVector vParticleDest = vDestPos;

				//do all intersections in world space
				if(m_pProps->m_bObjectSpace)
				{
					tObjTrans.Transform(pParticle->m_Pos, vParticlePos);
					tObjTrans.Transform(vDestPos, vParticleDest);
				}

				iQuery.m_From	= vParticlePos;
				iQuery.m_To		= vParticleDest;

				if( g_pLTClient->IntersectSegmentAgainst( iQuery, &iInfo, hMainWorld ) )
				{
					//handle bounce
					if(pParticle->m_nUserData & PARTICLE_BOUNCE)
					{
						//move our particle to the position of the intersection, but offset based upon
						//the normal slightly to avoid tunnelling
						vDestPos = iInfo.m_Point + iInfo.m_Plane.m_Normal * 0.1f;

						//and handle transforming back into object space if appropriate
						if(m_pProps->m_bObjectSpace)
						{
							vDestPos = tInvObjTrans * vDestPos;
						}

						LTVector& vVel	 = pParticle->m_Velocity;
						LTVector vNormal = iInfo.m_Plane.m_Normal;

						if(m_pProps->m_bObjectSpace)
						{
							vNormal = tInvObjTrans.m_rRot.RotateVector(vNormal);
						}

						//reflect the velocity over the normal
						vVel -= vNormal * (2.0f * vVel.Dot(vNormal));

						//apply the coefficient of restitution
						vVel *= fCOR;
					}

					//handle splat
					if(pParticle->m_nUserData & PARTICLE_SPLAT)
					{
						//alright, we now need to create a splat effect

						//create a random rotation around the plane that we hit
						LTRotation rSplatRot(iInfo.m_Plane.m_Normal, LTVector(0.0f, 1.0f, 0.0f));
						rSplatRot.Rotate(iInfo.m_Plane.m_Normal, GetRandom(0.0f, MATH_TWOPI));

						LTRigidTransform tSplatTrans(iInfo.m_Point, rSplatRot);

						//now handle if we hit an object, we need to convert spaces and set that as our parent
						if(iInfo.m_hObject)
						{
							//convert the transform into a relative object space transform
							LTRigidTransform tHitObjTrans;
							g_pLTClient->GetObjectTransform(iInfo.m_hObject, &tHitObjTrans);
							tSplatTrans = tHitObjTrans.GetInverse() * tSplatTrans;
						}

						//and create the actual new object
						CLIENTFX_CREATESTRUCT CreateStruct("", 0, iInfo.m_hObject, tSplatTrans);
						CreateNewFX(m_pFxMgr, m_pProps->m_pszSplatEffect, CreateStruct, true);					

						//we need to kill the particle
						itParticles = RemoveParticle(itParticles);
						continue;
					}
				}
			}			

			//move the particle to the destination position that we calculated
			pParticle->m_Pos = vDestPos;			

			//and update our extents box to match accordingly
			vMin.Min(pParticle->m_Pos);
			vMax.Max(pParticle->m_Pos);

			if(m_pProps->m_bStreak)
			{
				LTVector vStreakPt = pParticle->m_Pos - pParticle->m_Velocity * m_pProps->m_fStreakScale;
				vMin.Min(vStreakPt);
				vMax.Max(vStreakPt);
			}

			//and move onto our next particle
			itParticles.Prev();
		}
	}

	//handle the case where we didn't hit any particles and therefore need to clear out our min and
	//max (note we only check one component for speed)
	if(vMin.x == kfInfinity)
	{
		vMin = tObjTrans.m_vPos;
		vMax = tObjTrans.m_vPos;
	}	

	//expand the bounding box out by the largest particle size times the square root of two
	//to handle rotating particles that are at 45 degrees
	float fExpandAmount = m_pProps->m_fMaxParticlePadding;

	vMin -= LTVector(fExpandAmount, fExpandAmount, fExpandAmount);
	vMax += LTVector(fExpandAmount, fExpandAmount, fExpandAmount);

	//handle the case of when the particles are in object space, in such a case, we need to convert
	//the AABB from object space to an AABB in world space
	if(m_pProps->m_bObjectSpace)
	{
		//transform the object-space AABB to a world space AABB by projecting the dims onto the object basis vectors
		LTVector vRight, vUp, vForward;
		tObjTrans.m_rRot.GetVectors(vRight, vUp, vForward);

		LTVector vObjHalfDims = (vMax - vMin) * 0.5f;
		LTVector vWorldHalfDims;
		vWorldHalfDims.x = vObjHalfDims.Dot(LTVector(fabsf(vRight.x), fabsf(vUp.x), fabsf(vForward.x)));
		vWorldHalfDims.y = vObjHalfDims.Dot(LTVector(fabsf(vRight.y), fabsf(vUp.y), fabsf(vForward.y)));
		vWorldHalfDims.z = vObjHalfDims.Dot(LTVector(fabsf(vRight.z), fabsf(vUp.z), fabsf(vForward.z)));

		LTVector vObjCenter = (vMin + vMax) * 0.5f;
		LTVector vWorldCenter;
		tObjTrans.Transform(vObjCenter, vWorldCenter);

		//save the transformed results
		vMin = vWorldCenter - vWorldHalfDims;
		vMax = vWorldCenter + vWorldHalfDims;
	}

	//update the visibility box of the object
	g_pLTClient->GetCustomRender()->SetVisBoundingBox(m_hCustomRender, vMin - tObjTrans.m_vPos, vMax - tObjTrans.m_vPos);

	//we also need to update the transform of our object to follow
	g_pLTClient->SetObjectTransform(m_hCustomRender, tObjTrans);
}


//this will randomly generate an object space position for the starting of a particle based upon
//the current properties of this effect
LTVector CParticleSystemGroup::GenerateObjectSpaceParticlePos(	const LTVector& vEmissionOffset, const LTVector& vEmissionDims,
															  float fMinRadius, float fMaxRadius)
{
	LTASSERT(m_pProps, "Error: Called GenerateObjectSpaceParticlePos on an uninitialized particle group");

	//the position we generate
	LTVector vPos(0, 0, 0);

	// What kind of emission do we have?
	switch( m_pProps->m_eEmissionType )
	{
	case PS_eSphere:
		{
			//in order to generate an evenly distributed point on a sphere, we need to roll it
			//first in the XZ plane, and then along the plane formed by that direction and the
			//Y axis
			float fXZAngle		= GetRandom(0.0f, MATH_CIRCLE);
			float fDirYAngle	= GetRandom(0.0f, MATH_CIRCLE);

			//find a direction in the plane
			LTVector vPlaneDir(LTCos(fXZAngle), 0.0f, LTSin(fXZAngle));

			//now go between that and the Y to find our final direction
			LTVector vFinalDir = vPlaneDir * LTCos(fDirYAngle);
			vFinalDir.y += LTSin(fDirYAngle);

			//scale based upon the allowed range
			vPos = vFinalDir * GetRandom( fMinRadius, fMaxRadius );
		}
		break;

	case PS_ePoint:
		{
			vPos.Init(0, 0, 0);
		}	
		break;

	case PS_eBox:
		{
			vPos.x = GetRandom(-vEmissionDims.x, vEmissionDims.x);
			vPos.y = GetRandom(-vEmissionDims.y, vEmissionDims.y);
			vPos.z = GetRandom(-vEmissionDims.z, vEmissionDims.z);
		}
		break;

	case PS_eCone:
		{
			//the real trick here is evenly distributing the particles
			//inside of the cone since if we just blindly picked a height, the
			//particles would bunch up at the bottom. This is done by
			//using a square, since it bunches numbers more towards the lower
			//end of the spectrum, so if the lower end corresponds to the end
			//of the cone, it will tend to bunch it up more there
			float fRandom = GetRandom(0.0f, sqrtf(fMaxRadius));
			float fOffset = fMaxRadius - fRandom * fRandom;

			//find the radius of the cone at that point
			float fRadius = fMinRadius * fOffset / fMaxRadius;

			vPos = GenerateDiskPoint(m_pProps->m_vEmissionPerp1, m_pProps->m_vEmissionPerp2, 0.0f, fRadius);

			//now offset it appropriately
			vPos += (m_pProps->m_vEmissionDir * fOffset);
		}
		break;

	case PS_eCylinder:
		{
			//determine a disk point
			vPos = GenerateDiskPoint(m_pProps->m_vEmissionPerp1, m_pProps->m_vEmissionPerp2, fMinRadius,fMaxRadius);

			//offset it along the height
			vPos += m_pProps->m_vEmissionDir * GetRandom(0.0f, vEmissionDims.y);
		}
		break;

	default:
		LTERROR( "Unknown particle emission type");
		break;

	}

	//apply the emission offst
	vPos += vEmissionOffset;

	//return that object space position
	return vPos;
}

//---------------------------------------
// Particle Rendering

//given a particle, this will determine the size and color of that particle
void CParticleSystemGroup::GetParticleSizeAndColor(SParticle* pParticle, uint32& nColor, float& fScale)
{
	//determine the lifetime of this particle
	float fLifetime = (1.0f - pParticle->m_fLifetime / pParticle->m_fTotalLifetime);

	// Color it and scale it
	fScale = m_pProps->m_ffcParticleScale.GetValue(fLifetime);

	nColor = CFxProp_Color4f::ToColor(m_pProps->m_cfcParticleColor.GetValue(fLifetime));
}

//hook for the custom render object, this will just call into the render function
void CParticleSystemGroup::CustomRenderCallback(ILTCustomRenderCallback* pInterface, const LTRigidTransform& tCamera, void* pUser)
{
	((CParticleSystemGroup*)pUser)->RenderParticleSystem(pInterface, tCamera);
}

//function that handles the custom rendering
void CParticleSystemGroup::RenderParticleSystem(ILTCustomRenderCallback* pInterface, const LTRigidTransform& tCamera)
{
	//track our performance
	CTimedSystemBlock TimingBlock(g_tsClientFXParticles);

	//setup our vertex declaration
	if(pInterface->SetVertexDeclaration(g_ClientFXVertexDecl.GetTexTangentSpaceDecl()) != LT_OK)
		return;

	//bind a quad index stream
	if(pInterface->BindQuadIndexStream() != LT_OK)
		return;

	//set the fact that we were visible
	*m_pVisibleFlag = true;

	//now determine the largest number of particles that we can render at any time
	uint32 nMaxParticlesPerBatch = QUAD_RENDER_INDEX_STREAM_SIZE / 6;
	nMaxParticlesPerBatch = LTMIN(nMaxParticlesPerBatch, DYNAMIC_RENDER_VERTEX_STREAM_SIZE / (sizeof(STexTangentSpaceVert) * 4));

	//determine the screen orientation
	LTRotation rCamera = tCamera.m_rRot;

	if (m_pProps->m_bObjectSpace)
	{
		LTRotation rObjectRotation;
		g_pLTClient->GetObjectRotation(m_hCustomRender, &rObjectRotation);
		rCamera = rObjectRotation.Conjugate() * rCamera;
	}

	LTVector vUp = rCamera.Up();
	LTVector vRight = rCamera.Right();

	//create some vectors to offset to each corner (avoids adding for displacement in the inner loop)
	//Each one can just be scaled by the size of the particle to get the final offset
	static const float kfHalfRoot2 = 0.5f * MATH_SQRT2;

	//premultiplied versions of up and right scaled by half the square root of two
	LTVector vUpHalfRoot2 = vUp * kfHalfRoot2;
	LTVector vRightHalfRoot2 = vRight * kfHalfRoot2;

	//precalculate the diagonals for non-rotating particles since these are constant
	LTVector vDiagonals[4];
	vDiagonals[0] =  vUpHalfRoot2 - vRightHalfRoot2; 
	vDiagonals[1] =  vUpHalfRoot2 + vRightHalfRoot2; 
	vDiagonals[2] = -vUpHalfRoot2 + vRightHalfRoot2; 
	vDiagonals[3] = -vUpHalfRoot2 - vRightHalfRoot2; 

	uint32 nNumParticlesLeft = m_Particles.GetNumParticles();

	//precalculate some data for the basis space of the particles
	LTVector	vNormal		= -rCamera.Forward();
	LTVector	vTangent	= vRight;
	LTVector	vBinormal	= -vUp;

	//the U scale for particle images
	float		fUImageWidth = 1.0f / (float)m_pProps->m_nNumImages;

	//variables used within the inner loop
	float		fSize;
	uint32		nColor;

	//now run through all the particles and render
	CParticleIterator itParticles = m_Particles.GetIterator();
	while(nNumParticlesLeft > 0)
	{
		//determine our batch size
		uint32 nBatchSize = LTMIN(nNumParticlesLeft, nMaxParticlesPerBatch);

		//lock down our buffer for rendering
		SDynamicVertexBufferLockRequest LockRequest;
		if(pInterface->LockDynamicVertexBuffer(nBatchSize * 4, LockRequest) != LT_OK)
			return;

		//fill in a batch of particles
		STexTangentSpaceVert* pCurrOut = (STexTangentSpaceVert*)LockRequest.m_pData;

		if(m_pProps->m_bRotate)
		{
			//we need to render the particles rotated
			for(uint32 nBatchParticle = 0; nBatchParticle < nBatchSize; nBatchParticle++)
			{
				//sanity check
				LTASSERT(!itParticles.IsDone(), "Error: Particle count and iterator mismatch");

				//get the particle from the iterator
				SParticle* pParticle = (SParticle*)itParticles.GetParticle();

				GetParticleSizeAndColor(pParticle, nColor, fSize);

				//determine the sin and cosine of this particle angle
				float fAngle = pParticle->m_fAngle;
				float fSinAngle = LTSin(fAngle);
				float fCosAngle = LTCos(fAngle);

				LTVector vRotRight = (vRightHalfRoot2 * fCosAngle + vUpHalfRoot2 * fSinAngle) * fSize;
				LTVector vRotUp    = vNormal.Cross(vRotRight);

				LTVector vRotTangent  = vTangent * fCosAngle + vBinormal * fSinAngle;
				LTVector vRotBinormal = vTangent * fSinAngle + vBinormal * fCosAngle;

				SetupParticle(	pCurrOut, 
					pParticle->m_Pos + vRotUp - vRotRight,
					pParticle->m_Pos + vRotUp + vRotRight,
					pParticle->m_Pos - vRotUp + vRotRight,
					pParticle->m_Pos - vRotUp - vRotRight,
					nColor, vNormal, vRotTangent, vRotBinormal, 
					pParticle->m_nUserData & PARTICLE_IMAGE_MASK, fUImageWidth);

				//move onto the next set of particles
				pCurrOut += 4;

				//move onto the next particle for processing
				itParticles.Next();
			}
		}
		else if (m_pProps->m_bStreak)
		{
			//the particles are non-rotated but streaked along their velocity
			for(uint32 nBatchParticle = 0; nBatchParticle < nBatchSize; nBatchParticle++)
			{
				//sanity check
				LTASSERT(!itParticles.IsDone(), "Error: Particle count and iterator mismatch");

				//get the particle from the iterator
				SParticle* pParticle = (SParticle*)itParticles.GetParticle();

				GetParticleSizeAndColor(pParticle, nColor, fSize);

				//in order to render the streak, we determine a line that passes through
				//the particle and runs in the direction of the velocity of the particle

				//we need to project the velocity onto the screen
				LTVector2 vScreen;
				vScreen.x = -(pParticle->m_Velocity.Dot(vRight));
				vScreen.y = -(pParticle->m_Velocity.Dot(vUp));

				//we know that the up and right vectors are normalized, so we can save some work by
				//just doing a 2d normalization
				float fMag = vScreen.Mag();
				if(fMag == 0.0f)
					vScreen.Init(fSize, 0.0f);
				else
					vScreen *= fSize / fMag;

				//determine our actual screen space velocity
				LTVector vScreenRight = vUp * vScreen.y + vRight * vScreen.x;
				LTVector vScreenUp = vUp * -vScreen.x + vRight * vScreen.y;

				//and now compute the endpoint of the streak
				LTVector vEndPos = pParticle->m_Pos - pParticle->m_Velocity * m_pProps->m_fStreakScale;

				SetupParticle(	pCurrOut, 
					pParticle->m_Pos	- vScreenRight - vScreenUp,
					vEndPos				+ vScreenRight - vScreenUp,
					vEndPos				+ vScreenRight + vScreenUp,
					pParticle->m_Pos	- vScreenRight + vScreenUp,
					nColor, vNormal, vScreenRight, vScreenUp, 
					pParticle->m_nUserData & PARTICLE_IMAGE_MASK, fUImageWidth);

				//move onto the next set of particles
				pCurrOut += 4;

				//move onto the next particle for processing
				itParticles.Next();
			}
		}
		else
		{
			//the particles are non-rotated
			for(uint32 nBatchParticle = 0; nBatchParticle < nBatchSize; nBatchParticle++)
			{
				//sanity check
				LTASSERT(!itParticles.IsDone(), "Error: Particle count and iterator mismatch");

				//get the particle from the iterator
				SParticle* pParticle = (SParticle*)itParticles.GetParticle();

				GetParticleSizeAndColor(pParticle, nColor, fSize);

				SetupParticle(	pCurrOut, 
					pParticle->m_Pos + vDiagonals[0] * fSize,
					pParticle->m_Pos + vDiagonals[1] * fSize,
					pParticle->m_Pos + vDiagonals[2] * fSize,
					pParticle->m_Pos + vDiagonals[3] * fSize,
					nColor, vNormal, vTangent, vBinormal, 
					pParticle->m_nUserData & PARTICLE_IMAGE_MASK, fUImageWidth);

				//move onto the next set of particles
				pCurrOut += 4;

				//move onto the next particle for processing
				itParticles.Next();
			}
		}

		//unlock and render the batch
		pInterface->UnlockAndBindDynamicVertexBuffer(LockRequest);
		pInterface->RenderIndexed(	eCustomRenderPrimType_TriangleList, 
			0, nBatchSize * 6, LockRequest.m_nStartIndex, 
			0, nBatchSize * 4);

		nNumParticlesLeft -= nBatchSize;
	}
}
