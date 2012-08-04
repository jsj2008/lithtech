//----------------------------------------------------------
//
// MODULE  : WRECKINGBALL.CPP
//
// PURPOSE : A Wrecking ball for the construction level.
//
// CREATED : 3/28/98
//
//----------------------------------------------------------

// Includes....
#include <stdio.h>
#include <string.h>
#include "serverobj_de.h"
#include "WreckingBall.h"
#include "cpp_server_de.h"
#include "ClientCastLineSFX.h"
#include "ObjectUtilities.h"
#include <mbstring.h>

void BPrint(char*);

BEGIN_CLASS(WreckingBall)
END_CLASS_DEFAULT_FLAGS(WreckingBall, B2BaseClass, NULL, NULL, CF_ALWAYSLOAD)


#define CHAIN_LENGTH	580
#define BEAM_LENGTH		180
#define CRAIN_HEIGHT	330
#define CHAIN_FRICTION	0.2f

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WreckingBall::WreckingBall
//
//	PURPOSE:	Initialize
//
// ----------------------------------------------------------------------- //

WreckingBall::WreckingBall() : B2BaseClass(OT_MODEL)
{
 	AddAggregate(&m_damage);	// Just so it'll move when you shoot it.
	m_hCrane = DNULL;
	m_hChain = DNULL;
	m_bFirstUpdate	= DTRUE;
	VEC_INIT(m_vLastAttachPoint);
	VEC_INIT(m_vMomentum);
	m_fDamage = 20000.0f;		// That ought to do it.
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WreckingBall::~WreckingBall
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

WreckingBall::~WreckingBall()
{
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WreckingBall::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD WreckingBall::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return 0;

	switch(messageID)
	{
		case MID_UPDATE:
		{
			if (!Update((DVector *)pData))
			{
				pServerDE->RemoveObject(m_hObject);
			}
			break;
		}

		case MID_PRECREATE:
		{
			DDWORD dwRet = B2BaseClass::EngineMessageFn(messageID, pData, fData);

			ObjectCreateStruct* pStruct = (ObjectCreateStruct*)pData;
			if (pStruct)
			{
				_mbscpy((unsigned char*)pStruct->m_Filename, (const unsigned char*)"models\\worldobjects\\wreckingball.abc");
				_mbscpy((unsigned char*)pStruct->m_SkinName, (const unsigned char*)"skins\\worldobjects\\wreckingball.dtx");
				_mbscpy((unsigned char*)pStruct->m_Name, (const unsigned char*)"WreckingBall");
				pStruct->m_NextUpdate = 0.01f;
				pStruct->m_Flags = FLAG_VISIBLE | FLAG_SOLID | FLAG_SHADOW | FLAG_TOUCH_NOTIFY | FLAG_MODELGOURAUDSHADE;
			}			
			return dwRet;
			break;
		}

		case MID_INITIALUPDATE:
		{
			InitialUpdate((DVector *)pData);
			break;
		}

		case MID_TOUCHNOTIFY:
		{
			HOBJECT hObj = (HOBJECT)pData;
			DVector vVel, vPos;
			pServerDE->GetVelocity(m_hObject, &vVel);
			pServerDE->GetObjectPos(m_hObject, &vPos);

			DFLOAT fRadius = 300.0f;
			ObjectList *ol = pServerDE->FindObjectsTouchingSphere(&vPos, fRadius);

			if (ol)
			{
				ObjectLink* pLink = ol->m_pFirstLink;
				while(pLink)
				{
					HOBJECT hObj = pLink->m_hObject;
					short nType = pServerDE->GetObjectType(hObj);

					if (nType == OT_WORLDMODEL)
					{
						DamageObject(m_hObject, this, hObj, m_fDamage, vVel, vPos, DAMAGE_TYPE_NORMAL); 
//						PlaySoundFromPos(&vPos, "sounds\\exp_tnt.wav", 2000, SOUNDTYPE_MISC, SOUNDPRIORITY_HIGH );
					}
				
					pLink = pLink->m_pNext;
				}
				
				pServerDE->RelinquishList(ol);
			}

			// Damage anything we touch
//			DamageObjectsInRadius(m_hObject, this, vPos, 400.0f, m_fDamage, DAMAGE_TYPE_NORMAL);
//			DamageObject(m_hObject, this, hObj, m_fDamage, vVel, vPos, DAMAGE_TYPE_NORMAL); 
			break;
		}

		default : break;
	}


	return B2BaseClass::EngineMessageFn(messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WreckingBall::InitialUpdate
//
//	PURPOSE:	Do initial updating
//
// ----------------------------------------------------------------------- //

DBOOL WreckingBall::InitialUpdate(DVector*)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !m_hObject) return DFALSE;

	DVector vDims;
	m_damage.Init(m_hObject);
	m_damage.SetMass(5000);
	
	pServerDE->GetModelAnimUserDims(m_hObject, &vDims, 0);
	pServerDE->SetObjectDims(m_hObject, &vDims);

	pServerDE->SetForceIgnoreLimit(m_hObject, 0.0f);
	pServerDE->SetBlockingPriority(m_hObject, 50);

	pServerDE->SetNextUpdate(m_hObject, (DFLOAT)0.01);
	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WreckingBall::FirstUpdate
//
//	PURPOSE:	Do First updating
//
// ----------------------------------------------------------------------- //

void WreckingBall::FirstUpdate()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	// Try to find a crane
    ObjectList* pTargets = pServerDE->FindNamedObjects("crane");

	if (!pTargets || pTargets->m_nInList <= 0) 
	{
		return;
	}

	// Use the first object
	if (!(m_hCrane = pTargets->m_pFirstLink->m_hObject)) return;

	// clean up	
	pServerDE->RelinquishList(pTargets);

	// Get initial attach point
	DVector vAttachPoint, vPos;
	ComputeAttachPoint(&vAttachPoint);
	VEC_COPY(vPos, vAttachPoint);
	vPos.y -= CHAIN_LENGTH;
	pServerDE->TeleportObject(m_hObject, &vPos);

	// Make a chain
	DrawChain(&vAttachPoint);

	VEC_COPY(m_vLastAttachPoint, vAttachPoint);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WreckingBall::Update
//
//	PURPOSE:	Update the impact
//
// ----------------------------------------------------------------------- //

DBOOL WreckingBall::Update(DVector* pMovement)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !m_hObject) return DFALSE;

	pServerDE->SetNextUpdate(m_hObject, (DFLOAT)0.01);

	if (m_bFirstUpdate)
	{
		FirstUpdate();
		m_bFirstUpdate = DFALSE;
	}

	// If there is no crane, we have no business existing.
	if (!m_hCrane) return DFALSE;

	DVector vAttachPoint, vPos;
	ComputeAttachPoint(&vAttachPoint);

	pServerDE->GetObjectPos(m_hObject, &vPos);
	
	// Move the ball

	// Fake the y height based on X and Z distances so chain is always the same length
	DFLOAT fY, fX, fZ;
	fX = (vPos.x - vAttachPoint.x) * (vPos.x - vAttachPoint.x);
	fZ = (vPos.z - vAttachPoint.z) * (vPos.z - vAttachPoint.z);

	fY = (DFLOAT)sqrt(CHAIN_LENGTH*CHAIN_LENGTH - fX - fZ);
	vPos.y = vAttachPoint.y - fY;
	pServerDE->MoveObject(m_hObject, &vPos);

	// Calculate new X & Z acceleration
	DVector vAccel;

	// Acceleration towards the center based on how far away the ball is
	VEC_SUB(vAccel, vPos, vAttachPoint);
	vAccel.y = 0.0f;
	VEC_NEGATE(vAccel, vAccel);

	pServerDE->SetAcceleration(m_hObject, &vAccel);

	// Apply friction if the crane isn't moving
	if (VEC_DISTSQR(m_vLastAttachPoint, vAttachPoint) < 1.0f)
	{
		DVector vVel, vDamp;

		DFLOAT fFriction = -pServerDE->GetFrameTime() * CHAIN_FRICTION;
		pServerDE->GetVelocity(m_hObject, &vVel);
		VEC_COPY(vDamp, vVel);
		VEC_MULSCALAR(vDamp, vDamp, fFriction);

		VEC_ADD(vVel, vVel, vDamp);
		pServerDE->SetVelocity(m_hObject, &vVel);
	}

	// Update the chain position
	if (m_hChain)
	{
		DVector vF, vU;
		DRotation rRot;
		VEC_SET(vU, 0, 1, 0);
		VEC_SUB(vF, vPos, vAttachPoint);
		pServerDE->AlignRotation(&rRot, &vF, &vU);
		pServerDE->SetObjectRotation(m_hChain, &rRot);
		pServerDE->TeleportObject(m_hChain, &vAttachPoint);
	}

	VEC_COPY(m_vLastAttachPoint, vAttachPoint);

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WreckingBall::ComputeAttachPoint
//
//	PURPOSE:	determines where the attach point on the crane is.
//
// ----------------------------------------------------------------------- //

void WreckingBall::ComputeAttachPoint(DVector* pvAttachPoint)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !m_hObject) return;

	DRotation rRot;
	DVector vF, vR, vU, vPos;

	pServerDE->GetObjectRotation(m_hCrane, &rRot);
	pServerDE->GetObjectPos(m_hCrane, &vPos);
    pServerDE->GetRotationVectors(&rRot, &vU, &vR, &vF);
	vF.x = -vF.x;

	// Compute how far along the beam (vF) and how hight (vU) the attach point is.
	VEC_MULSCALAR(vF, vF, BEAM_LENGTH);
	VEC_MULSCALAR(vU, vU, CRAIN_HEIGHT);
	VEC_ADD(*pvAttachPoint, vPos, vF);
	VEC_ADD(*pvAttachPoint, *pvAttachPoint, vU);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WreckingBall::DrawChain
//
//	PURPOSE:	Cast a line from the attach point to the ball
//
// ----------------------------------------------------------------------- //
DBOOL WreckingBall::DrawChain(DVector *pvAttachPoint)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return DFALSE;
    
    // Cast a Ray Forward From the camera (DebugOnly)
//    if (m_hChain) pServerDE->RemoveObject(m_hChain);
    
	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

	DVector vPos;
	DRotation rRot;
    
	pServerDE->GetObjectPos(m_hObject, &vPos);
	pServerDE->GetObjectRotation(m_hObject, &rRot);
    
	VEC_COPY(theStruct.m_Pos, *pvAttachPoint);
	ROT_COPY(theStruct.m_Rotation, rRot);

	HCLASS hClass = pServerDE->GetClass("CClientCastLineSFX");
	CClientCastLineSFX* pLine = DNULL;

	if (hClass)
	{
		pLine = (CClientCastLineSFX*)pServerDE->CreateObject(hClass, &theStruct);
		if (!pLine) return DFALSE;
	}

	DVector vColor;
	VEC_SET(vColor, 0.0f, 0.0f, 0.0f);          // Black line
	pLine->Setup(vColor, vColor, 1.0f, 1.0f, m_hObject);
    
	m_hChain = pLine->m_hObject;
    
    return DTRUE;
}

