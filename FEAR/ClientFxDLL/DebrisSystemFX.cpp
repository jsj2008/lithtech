// ----------------------------------------------------------------------- //
//
// MODULE  : DebrisSystemFX.cpp
//
// PURPOSE : The DebrisSystemFX object
//
// CREATED : 4/10/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h" 
#include "ClientFX.h"
#include "DebrisSystemFX.h"
#include "PhysicsUtilities.h"
#include "iperformancemonitor.h"

//our object used for tracking performance for effect
static CTimedSystem g_tsClientFXDebrisSystem("ClientFX_DebrisSystem", "ClientFX");

//-------------------------------------------------------------------------------------------
// Utility Functions
//-------------------------------------------------------------------------------------------

//utility function that will generate a point on the specified plane
static LTVector GenerateDiskPoint(const LTVector& vAxis1, const LTVector& vAxis2, float fMinRadius, float fMaxRadius)
{
	//determine a random angle
	float fAngle = GetRandom(0.0f, MATH_CIRCLE);

	//create a directional vector
	LTVector vDirection = vAxis1 * cosf(fAngle) + vAxis2 * sinf(fAngle);

	//scale by distance
	vDirection *= GetRandom(fMinRadius, fMaxRadius);

	//success
	return vDirection;
}

//utility function that generates a random unit vector
static LTVector GenerateRandomUnitVector()
{
	//generate two random angles, one for XZ plane, one for Y 
	float fXZAngle = GetRandom(0.0f, MATH_TWOPI);
	float fYAngle = GetRandom(0.0f, MATH_PI);

	float fCosYAngle = LTCos(fYAngle);

	//and now generate our actual vector (XZ is just a ring using cos/sin scaled by the Y angle,
	//and Y is just the sine of the angle)
	return LTVector(fCosYAngle * LTCos(fXZAngle),
					LTSin(fYAngle),
					fCosYAngle * LTSin(fXZAngle));
}

//given a vector, this will return a rotation that uses the vector as the forward direction with a
//random twist
static LTRotation ConvertDirectionToOrientation(const LTVector& vUnitDir)
{
	//create a random rotation around the plane that we hit
	LTRotation rRot(vUnitDir, LTVector(0.0f, 1.0f, 0.0f));
	rRot.Rotate(vUnitDir, GetRandom(0.0f, MATH_TWOPI));

	return rRot;
}

//-------------------------------------------------------------------------------------------
// CDebrisSystemFX
//-------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CDebrisSystemFX::CDebrisSystemFX
//
//  PURPOSE:	Standard Constructor
//
// ----------------------------------------------------------------------- //

CDebrisSystemFX::CDebrisSystemFX() :
	CBaseFX(CBaseFX::eDebrisSystemFX)
{
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CDebrisSystemFX::~CDebrisSystemFX
//
//  PURPOSE:	Standard Destructor
//
// ----------------------------------------------------------------------- //

CDebrisSystemFX::~CDebrisSystemFX( void )
{
	Term();
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CDebrisSystemFX::Init
//
//  PURPOSE:	Creates and initialises the Debris system
//
// ----------------------------------------------------------------------- //

bool CDebrisSystemFX::Init(const FX_BASEDATA *pData, const CBaseFXProps *pProps)
{
	// Perform base class initialization
	if( !CBaseFX::Init(pData, pProps) ) 
		return false;


	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CDebrisSystemFX::Update
//
//  PURPOSE:	NONE
//
// ----------------------------------------------------------------------- //

bool CDebrisSystemFX::Update( float tmFrameTime )
{
	//track our performance
	CTimedSystemBlock TimingBlock(g_tsClientFXDebrisSystem);

	//allow the base effect to handle any updates
	BaseUpdate(tmFrameTime);

	//determine if this is our initial update and we need to create our debris. We do this
	//after the update since the newly created effects will already be in place
	if(IsInitialFrame())
	{
		CreateDebrisEmission();
	}	

	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CDebrisSystemFX::GenerateObjectSpaceDebrisVel
//
// this will randomly generate an object space position for the starting of a Debris based upon
// the current properties of this effect
//
// ----------------------------------------------------------------------- //

LTVector CDebrisSystemFX::GenerateObjectSpaceDebrisVel(const LTVector& vObjSpacePos, const LTVector& vMinVelocity, const LTVector& vMaxVelocity)
{
	LTVector vVel(0, 0, 0);

	// Randomize the velocity within our range
	switch(GetProps()->m_eVelocityType)
	{
	case eDebrisVelocity_Random:
		{
			vVel.x = GetRandom( vMinVelocity.x, vMaxVelocity.x );
			vVel.y = GetRandom( vMinVelocity.y, vMaxVelocity.y );
			vVel.z = GetRandom( vMinVelocity.z, vMaxVelocity.z );
		}
		break;
	case eDebrisVelocity_Center:
		{
			//velocity direction is based upon position from 0, 0, 0
			vVel = vObjSpacePos * (GetRandom(vMinVelocity.x, vMaxVelocity.x) / vObjSpacePos.Mag());
		}
		break;
	default:
		LTERROR( "Unknown Debris velocity type");
		break;
	}

	return vVel;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CDebrisSystemFX::GenerateObjectSpaceDebrisPos
//
// this will randomly generate an object space position for the starting of a Debris based upon
// the current properties of this effect
//
// ----------------------------------------------------------------------- //

LTVector CDebrisSystemFX::GenerateObjectSpaceDebrisPos(const LTVector& vEmissionOffset, const LTVector& vEmissionDims, float fMinRadius, float fMaxRadius)
{
	//the position we generate
	LTVector vPos(0, 0, 0);

	// What kind of emission do we have?
	switch( GetProps()->m_eEmissionType )
	{
		case eDebrisEmission_Sphere:
			{
				//in order to generate an evenly distributed point on a sphere, we need to roll it
				//first in the XZ plane, and then along the plane formed by that direction and the
				//Y axis
				float fXZAngle		= GetRandom(0.0f, MATH_CIRCLE);
				float fDirYAngle	= GetRandom(0.0f, MATH_CIRCLE);

				//find a direction in the plane
				LTVector vPlaneDir(cosf(fXZAngle), 0.0f, sinf(fXZAngle));

				//now go between that and the Y to find our final direction
				LTVector vFinalDir = vPlaneDir * cosf(fDirYAngle);
				vFinalDir.y += sinf(fDirYAngle);

				//scale based upon the allowed range
				vPos = vFinalDir * GetRandom( fMinRadius, fMaxRadius );
			}
			break;

		case eDebrisEmission_Point:
			{
				vPos.Init(0, 0, 0);
			}	
			break;

		case eDebrisEmission_Box:
			{
				vPos.x = GetRandom(-vEmissionDims.x, vEmissionDims.x);
				vPos.y = GetRandom(-vEmissionDims.y, vEmissionDims.y);
				vPos.z = GetRandom(-vEmissionDims.z, vEmissionDims.z);
			}
			break;

		case eDebrisEmission_Cone:
			{
				//the real trick here is evenly distributing the Debriss
				//inside of the cone since if we just blindly picked a height, the
				//Debriss would bunch up at the bottom. This is done by
				//using a square, since it bunches numbers more towards the lower
				//end of the spectrum, so if the lower end corresponds to the end
				//of the cone, it will tend to bunch it up more there
				float fRandom = GetRandom(0.0f, sqrtf(fMaxRadius));
				float fOffset = fMaxRadius - fRandom * fRandom;

				//find the radius of the cone at that point
				float fRadius = fMinRadius * fOffset / fMaxRadius;

				vPos = GenerateDiskPoint(GetProps()->m_vEmissionPerp1, GetProps()->m_vEmissionPerp2, 0.0f, fRadius);

				//now offset it appropriately
				vPos += (GetProps()->m_vEmissionDir * fOffset);
			}
			break;

		case eDebrisEmission_Cylinder:
			{
				//determine a disk point
				vPos = GenerateDiskPoint(GetProps()->m_vEmissionPerp1, GetProps()->m_vEmissionPerp2, fMinRadius,fMaxRadius);
				
				//offset it along the height
				vPos += GetProps()->m_vEmissionDir * GetRandom(0.0f, vEmissionDims.y);
			}
			break;

		default:
			LTERROR( "Unknown Debris emission type");
			break;

	}

	//apply the emission offst
	vPos += vEmissionOffset;

	//return that object space position
	return vPos;
}

//this will generate the orientation to use for a piece of debris
LTRotation CDebrisSystemFX::GenerateObjectSpaceDebrisRot(const LTVector& vObjSpacePos, const LTVector& vObjSpaceVel)
{
	switch(GetProps()->m_eOrientationType)
	{
	case eDebrisOrientation_Random:
		return ConvertDirectionToOrientation(GenerateRandomUnitVector());
		break;

	case eDebrisOrientation_Position:
		if(vObjSpacePos == LTVector::GetIdentity())
		{
			return LTRotation::GetIdentity();
		}
		else
		{
			return ConvertDirectionToOrientation(vObjSpacePos.GetUnit());
		}
		break;

	case eDebrisOrientation_Parent:
		return LTRotation::GetIdentity();
		break;

	case eDebrisOrientation_Velocity:
		if(vObjSpaceVel == LTVector::GetIdentity())
		{
			return LTRotation::GetIdentity();
		}
		else
		{
			return ConvertDirectionToOrientation(vObjSpaceVel.GetUnit());
		}
		break;

	default:
		LTERROR("Warning: Invalid debris system rotation type specified");
		return LTRotation::GetIdentity();
		break;
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CDebrisSystemFX::Term
//
//  PURPOSE:	NONE
//
// ----------------------------------------------------------------------- //

void CDebrisSystemFX::Term()
{
}


//called to create a single emission of debris
void CDebrisSystemFX::CreateDebrisEmission()
{
	//note that we must have a valid type in order to create a debris emission
	if(GetProps()->m_nNumTypes == 0)
		return;

	//determine the transform of this emitter
	LTRigidTransform tTransform;
	GetCurrentTransform(GetUnitLifetime(), tTransform.m_vPos, tTransform.m_rRot);

	//determine the number of pieces of debris that we will be creating
	uint32 nNumPieces = GetRandom((uint32)GetProps()->m_nMinDebris, (uint32)GetProps()->m_nMaxDebris);

	//determine the system group that all of this debris should be created within so
	//that it doesn't collide with other pieces of debris in this system
	uint32 nSystem = 0;
	g_pLTClient->PhysicsSim()->GetUniqueSystemGroup(nSystem);

	//now run through and create that number of debris
	for(uint32 nCurrPiece = 0; nCurrPiece < nNumPieces; nCurrPiece++)
	{
		//we need to determine which type to create for this piece (either random, or go sequentially)
		uint32 nType = 0;

		switch(GetProps()->m_eTypeSelection)
		{
		case eDebrisType_Random:
			//randomly select a piece from our potential pieces
			nType = GetRandom((uint32)0, GetProps()->m_nNumTypes);
			break;
		case eDebrisType_Sequential:
			//create each piece in turn in the list
			nType = nCurrPiece % GetProps()->m_nNumTypes;
			break;
		default:
			LTERROR( "Error: Unexpected type selection method specified");
			break;
		}

		CreateDebrisPiece(tTransform, nSystem, nType);
	}
}

//called to create a single piece of debris
bool CDebrisSystemFX::CreateDebrisPiece(const LTRigidTransform& tObjTransform, uint32 nSystem, uint32 nType)
{
	//we first need to determine the starting position and orientation of this piece of debris
	LTVector vObjPos = GenerateObjectSpaceDebrisPos(LTVector::GetIdentity(), GetProps()->m_vEmissionDims, GetProps()->m_fMinRadius, GetProps()->m_fMaxRadius);

	//determine the linear velocity of this rigid body in gu/s
	LTVector vObjVel = GenerateObjectSpaceDebrisVel(vObjPos, GetProps()->m_vMinLinearVelocity, GetProps()->m_vMaxLinearVelocity);

	//determine the initial orientation of the debris piece
	LTRotation rObjRot = GenerateObjectSpaceDebrisRot(vObjPos, vObjVel);

	//determine the transform in world space
	LTRigidTransform tWSTrans = tObjTransform * LTRigidTransform(vObjPos, rObjRot);

	//and also bring the velocity into world space
	LTVector vWSVel = tObjTransform.m_rRot.RotateVector(vObjVel);

	//determine the angular velocity of this rigid body in radians per second
	const LTVector& vAngMin = GetProps()->m_vMinAngularVelocity;
	const LTVector& vAngMax = GetProps()->m_vMaxAngularVelocity;
	LTVector vAngVel = LTVector(GetRandom(vAngMin.x, vAngMax.x), GetRandom(vAngMin.y, vAngMax.y), GetRandom(vAngMin.z, vAngMax.z));

	//get the information we need for the type that we are creating
	const CDebrisSystemProps::STypeInfo& TypeInfo = GetProps()->m_Types[nType];

	//and now create this rigid body
	HPHYSICSRIGIDBODY hRigidBody = g_pLTClient->PhysicsSim()->CreateRigidBody(	TypeInfo.m_hShape,
																				tWSTrans, false,
																				PhysicsUtilities::ePhysicsGroup_UserDebris,
																				nSystem,
																				TypeInfo.m_fFriction,
																				TypeInfo.m_fCOR);

	//verify that it worked
	if(hRigidBody == INVALID_PHYSICS_RIGID_BODY)
		return false;

	//we now need to setup our initial velocities on this rigid body
	g_pLTClient->PhysicsSim()->SetRigidBodyVelocity(hRigidBody, vWSVel);
	g_pLTClient->PhysicsSim()->SetRigidBodyAngularVelocity(hRigidBody, vAngVel);

	//we now have a rigid body, so we need to create an effect that will be attached to the rigid body
	CLIENTFX_CREATESTRUCT CreateStruct("", 0, hRigidBody, LTRigidTransform::GetIdentity());
	CreateNewFX(m_pFxMgr, TypeInfo.m_pszEffect, CreateStruct, true);

	//we can now release our reference to the rigid body now that the newly created effect is using
	//that as it's parent
	g_pLTClient->PhysicsSim()->ReleaseRigidBody(hRigidBody);

	//success
	return true;
}
