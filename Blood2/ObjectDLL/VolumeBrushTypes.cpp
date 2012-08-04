// ----------------------------------------------------------------------- ////
// MODULE  : VolumeBrushTypes.cpp
//
// PURPOSE : VolumeBrushTypes implementation
//
// CREATED : 2/16/98
//
// ----------------------------------------------------------------------- //

#include <stdio.h>
#include "VolumeBrushTypes.h"
#include "BaseCharacter.h"
#include "PlayerObj.h"
#include "Commands.h"
#include "CameraObj.h"
#include "ObjectUtilities.h"
#include "SFXMsgIds.h"


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LiquidFilterFn()
//
//	PURPOSE:	Filter Liquid volume brushes out of CastRay and/or 
//				IntersectSegment calls (so vectors can go through liquid).
//
//	NOTE:		For now the assumption is if this function is called (via 
//				CastRay or IntersectSegment), FLAG_RAYHIT must be set on the 
//				object (if it is a volume brush).  So, only liquid volume
//				brushes have this flag set on them, so we can conclude that if 
//				this object is of type volume brush that it must be liquid...
//
// ----------------------------------------------------------------------- //

DBOOL LiquidFilterFn(HOBJECT hObj, void *pUserData)
{
	if (!hObj || !g_pServerDE) return DFALSE;

	HCLASS hVolumeBrush = g_pServerDE->GetClass("VolumeBrush");
	HCLASS hObjClass    = g_pServerDE->GetObjectClass(hObj);

	// Return DTRUE to keep this object (not liquid), or DFALSE to ignore
	// this object (is liquid)...
	// Maybe add a splash effect or something

	return !g_pServerDE->IsKindOf(hObjClass, hVolumeBrush);
}


// Water
BEGIN_CLASS(Water)
    ADD_STRINGPROP(SpriteSurfaceName, "Sprites\\WaterBlue.spr")   
	ADD_REALPROP_FLAG(Viscosity, 0.5f, PF_HIDDEN)
	ADD_LONGINTPROP_FLAG(Code, CC_WATER, PF_HIDDEN)
	ADD_REALPROP(Damage, 0.0f)
	ADD_LONGINTPROP(DamageType, DAMAGE_TYPE_SUFFOCATE)
	ADD_BOOLPROP_FLAG(Locked, DFALSE, PF_HIDDEN)
	ADD_BOOLPROP_FLAG(UnlockKeyRemove, DFALSE, PF_HIDDEN)
	ADD_STRINGPROP_FLAG(UnlockKeyName, "", PF_HIDDEN)
END_CLASS_DEFAULT(Water, VolumeBrush, NULL, NULL)


// Blood
BEGIN_CLASS(Blood)
    ADD_STRINGPROP(SpriteSurfaceName, "Sprites\\WaterBlood.spr")   
	ADD_REALPROP_FLAG(Viscosity, 0.8f, PF_HIDDEN)
	ADD_LONGINTPROP_FLAG(Code, CC_BLOOD, PF_HIDDEN)
	ADD_REALPROP_FLAG(Damage, 0.0f, PF_HIDDEN)
	ADD_LONGINTPROP_FLAG(DamageType, DAMAGE_TYPE_SUFFOCATE, PF_HIDDEN)
	ADD_BOOLPROP_FLAG(Locked, DFALSE, PF_HIDDEN)
	ADD_BOOLPROP_FLAG(UnlockKeyRemove, DFALSE, PF_HIDDEN)
	ADD_STRINGPROP_FLAG(UnlockKeyName, "", PF_HIDDEN)
END_CLASS_DEFAULT(Blood, VolumeBrush, NULL, NULL)


// Acid
BEGIN_CLASS(Acid)
    ADD_STRINGPROP(SpriteSurfaceName, "Sprites\\WaterAcid.spr")   
	ADD_REALPROP_FLAG(Viscosity, 0.5f, PF_HIDDEN)
	ADD_LONGINTPROP_FLAG(Code, CC_ACID, PF_HIDDEN)
	ADD_BOOLPROP_FLAG(Locked, DFALSE, PF_HIDDEN)
	ADD_BOOLPROP_FLAG(UnlockKeyRemove, DFALSE, PF_HIDDEN)
	ADD_STRINGPROP_FLAG(UnlockKeyName, "", PF_HIDDEN)
END_CLASS_DEFAULT(Acid, VolumeBrush, NULL, NULL)


// Ladders
BEGIN_CLASS(Ladder)
	ADD_BOOLPROP_FLAG(ShowSurface, DFALSE, PF_HIDDEN)
    ADD_STRINGPROP_FLAG(SpriteSurfaceName, "", PF_HIDDEN)   
	ADD_REALPROP_FLAG(SurfaceHeight, 0.0f, PF_HIDDEN)
    ADD_COLORPROP_FLAG(SurfaceColor1, 0.0f, 0.0f, 0.0f, PF_HIDDEN)   
    ADD_COLORPROP_FLAG(SurfaceColor2, 0.0f, 0.0f, 0.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(XScaleMin, 15.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(XScaleMax, 25.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(YScaleMin, 15.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(YScaleMax, 25.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(XScaleDuration, 10.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(YScaleDuration, 10.0f, PF_HIDDEN)   
	ADD_VECTORPROP_VAL_FLAG(Current, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Viscosity, 0.0f, PF_HIDDEN)
	ADD_LONGINTPROP_FLAG(Code, CC_LADDER, PF_HIDDEN)
	ADD_REALPROP_FLAG(Damage, 0.0f, PF_HIDDEN)
	ADD_LONGINTPROP_FLAG(DamageType, DAMAGE_TYPE_NORMAL, PF_HIDDEN)
	ADD_BOOLPROP_FLAG(Locked, DFALSE, PF_HIDDEN)
	ADD_BOOLPROP_FLAG(UnlockKeyRemove, DFALSE, PF_HIDDEN)
	ADD_STRINGPROP_FLAG(UnlockKeyName, "", PF_HIDDEN)
END_CLASS_DEFAULT(Ladder, VolumeBrush, NULL, NULL)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Ladder::UpdatePhysics()
//
//	PURPOSE:	Update the physics of the passed in object
//
// ----------------------------------------------------------------------- //

void Ladder::UpdatePhysics(ContainerPhysics* pCPStruct)
{
//	VolumeBrush::UpdatePhysics(pCPStruct);

	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	HCLASS hBaseCharClass = pServerDE->GetClass("CBaseCharacter");

	if (pServerDE->IsKindOf(pServerDE->GetObjectClass(pCPStruct->m_hObject), hBaseCharClass))
	{
		CBaseCharacter* pCharacter = (CBaseCharacter*)pServerDE->HandleToObject(pCPStruct->m_hObject);
		if (pCharacter)
		{
			pCharacter->UpdateOnLadder(this, pCPStruct);
		}
	}
}


// Conveyor
BEGIN_CLASS(Conveyor)
	ADD_BOOLPROP_FLAG(ShowSurface, DFALSE, PF_HIDDEN)
    ADD_STRINGPROP_FLAG(SpriteSurfaceName, "", PF_HIDDEN)   
	ADD_REALPROP_FLAG(SurfaceHeight, 0.0f, PF_HIDDEN)
    ADD_COLORPROP_FLAG(SurfaceColor1, 0.0f, 0.0f, 0.0f, PF_HIDDEN)   
    ADD_COLORPROP_FLAG(SurfaceColor2, 0.0f, 0.0f, 0.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(XScaleMin, 15.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(XScaleMax, 25.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(YScaleMin, 15.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(YScaleMax, 25.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(XScaleDuration, 10.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(YScaleDuration, 10.0f, PF_HIDDEN)   
	ADD_REALPROP_FLAG(Viscosity, 0.0f, PF_HIDDEN)
	ADD_LONGINTPROP_FLAG(Code, CC_CONVEYOR, PF_HIDDEN)
	ADD_REALPROP_FLAG(Damage, 0.0f, PF_HIDDEN)
	ADD_LONGINTPROP_FLAG(DamageType, DAMAGE_TYPE_NORMAL, PF_HIDDEN)
	ADD_BOOLPROP_FLAG(Locked, DFALSE, PF_HIDDEN)
	ADD_BOOLPROP_FLAG(UnlockKeyRemove, DFALSE, PF_HIDDEN)
	ADD_STRINGPROP_FLAG(UnlockKeyName, "", PF_HIDDEN)
END_CLASS_DEFAULT(Conveyor, VolumeBrush, NULL, NULL)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Conveyor::UpdatePhysics()
//
//	PURPOSE:	Update the physics of the passed in object
//
// ----------------------------------------------------------------------- //

void Conveyor::UpdatePhysics(ContainerPhysics* pCPStruct)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;
	CollisionInfo collisionInfo;
	DBOOL bOnGround;
	CPlayerObj *pPlayerObj;

	bOnGround = DFALSE;
	if( IsPlayer( pCPStruct->m_hObject ))
	{
		pPlayerObj = ( CPlayerObj * )pServerDE->HandleToObject( pCPStruct->m_hObject );
		bOnGround = pPlayerObj->IsOnGround( );
	}
	else if( IsBaseCharacter( pCPStruct->m_hObject ))
	{
		// See if we're standing on something
		pServerDE->GetStandingOn(pCPStruct->m_hObject, &collisionInfo);
		if( collisionInfo.m_hObject )
			bOnGround = DTRUE;
	}

	if( bOnGround )
	{
		DFLOAT fUpdateDelta = pServerDE->GetFrameTime();

		// If so, apply current to velocity
		DVector vCurrent;
		VEC_MULSCALAR(vCurrent, m_vCurrent, fUpdateDelta);

		VEC_ADD(pCPStruct->m_Velocity, pCPStruct->m_Velocity, vCurrent);
	}
}


// CraneControl
BEGIN_CLASS(CraneControl)
	ADD_BOOLPROP_FLAG(ShowSurface, DFALSE, PF_HIDDEN)
    ADD_STRINGPROP_FLAG(SpriteSurfaceName, "", PF_HIDDEN)   
	ADD_REALPROP_FLAG(SurfaceHeight, 0.0f, PF_HIDDEN)
    ADD_COLORPROP_FLAG(SurfaceColor1, 0.0f, 0.0f, 0.0f, PF_HIDDEN)   
    ADD_COLORPROP_FLAG(SurfaceColor2, 0.0f, 0.0f, 0.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(XScaleMin, 15.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(XScaleMax, 25.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(YScaleMin, 15.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(YScaleMax, 25.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(XScaleDuration, 10.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(YScaleDuration, 10.0f, PF_HIDDEN)   
	ADD_REALPROP_FLAG(Viscosity, 0.0f, PF_HIDDEN)
	ADD_LONGINTPROP(Code, CC_CRANECONTROL)
	ADD_REALPROP_FLAG(Damage, 0.0f, PF_HIDDEN)
	ADD_LONGINTPROP_FLAG(DamageType, DAMAGE_TYPE_NORMAL, PF_HIDDEN)
	ADD_BOOLPROP(Locked, DTRUE)
	ADD_BOOLPROP(UnlockKeyRemove, DTRUE)
	ADD_STRINGPROP(UnlockKeyName, "Lever")
END_CLASS_DEFAULT(CraneControl, VolumeBrush, NULL, NULL)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CraneControl::UpdatePhysics()
//
//	PURPOSE:	Update the physics of the passed in object
//
// ----------------------------------------------------------------------- //

#define MIN_CRANE_YAW		(-PI*0.6f)
#define MAX_CRANE_YAW		(PI*0.6f)
#define CRANE_ROTATE_SPEED 0.03f

void CraneControl::UpdatePhysics(ContainerPhysics* pCPStruct)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;
//	if (m_bLocked)
//	{
		// See if they have the key we need to unlock
//		HMESSAGEWRITE hMessage = pServerDE->StartMessageToObject((LPBASECLASS)this, pCPStruct->m_hObject, MID_KEYQUERY);
//		pServerDE->WriteToMessageHString(hMessage, m_hstrKeyName);
//		pServerDE->EndMessage(hMessage);
//		return;
//	}

	HCLASS hPlayerClass = pServerDE->GetClass("CPlayerObj");

	if (pServerDE->IsKindOf(pServerDE->GetObjectClass(pCPStruct->m_hObject), hPlayerClass))
	{
		CPlayerObj* pPlayer = (CPlayerObj*)pServerDE->HandleToObject(pCPStruct->m_hObject);
		if (pPlayer)
		{
			// Create a camera which points at the crane
			if (!m_hCameraObj)
			{
				ObjectCreateStruct theStruct;
				INIT_OBJECTCREATESTRUCT(theStruct);

				_mbscpy((unsigned char*)theStruct.m_Name, (const unsigned char*)"cranecam");
    
				DRotation rRot;
				ROT_INIT(rRot);

				DVector vPos;
				pServerDE->GetObjectPos(m_hObject, &vPos);

				ROT_COPY(theStruct.m_Rotation, rRot);
				VEC_COPY(theStruct.m_Pos, vPos);
    
				theStruct.m_NextUpdate = 0.1f;
    
				HCLASS pClass = pServerDE->GetClass("CameraObj");
    
				// Check to make sure this is a valid class.
				if (pClass)
				{    
    				LPBASECLASS pObject = pServerDE->CreateObject(pClass, &theStruct);
        
					if (pObject)
					{
						m_hCameraObj = pServerDE->ObjectToHandle(pObject);
					}
					pPlayer->m_hCameraObj = m_hCameraObj;
					pServerDE->SetObjectFlags(m_hCameraObj, 0);
				} 
				else
				{
					return;
				}
			}
			if (!pPlayer->m_hCameraObj) pPlayer->m_hCameraObj = m_hCameraObj;

			// Try to find the crane)
			if (!m_hCrane)
			{
    			ObjectList* pTargets = pServerDE->FindNamedObjects("crane");
        
				if (!pTargets || pTargets->m_nInList <= 0) 
				{
					return;
				}

				// Use the first object
				m_hCrane = pTargets->m_pFirstLink->m_hObject;

				// clean up	
				pServerDE->RelinquishList(pTargets);
			}

			// Try to find the wrecking ball
			if (!m_hBall)
			{
    			ObjectList* pTargets = pServerDE->FindNamedObjects("WreckingBall");
        
				if (!pTargets || pTargets->m_nInList <= 0) 
				{
					return;
				}

				// Use the first object
				m_hBall = pTargets->m_pFirstLink->m_hObject;

				// clean up	
				pServerDE->RelinquishList(pTargets);
			}

			// Point the camera at the ball if we have one.
			CameraObj* pCameraObj = (CameraObj*)pServerDE->HandleToObject(m_hCameraObj);
			pCameraObj->SetLinkObject(m_hBall ? m_hBall : m_hCrane);

			if (pPlayer->GetContainer() != CC_CRANECONTROL)
				return;


			if (pServerDE->IsCommandOn(pPlayer->GetClient(), COMMAND_LEFT) || pServerDE->IsCommandOn(pPlayer->GetClient(), COMMAND_STRAFELEFT))
			{
				m_fCamYaw -= CRANE_ROTATE_SPEED;
				if (m_fCamYaw < MIN_CRANE_YAW)
					m_fCamYaw = MIN_CRANE_YAW;
			}
			else if (pServerDE->IsCommandOn(pPlayer->GetClient(), COMMAND_RIGHT) || pServerDE->IsCommandOn(pPlayer->GetClient(), COMMAND_STRAFERIGHT))
			{
				m_fCamYaw += CRANE_ROTATE_SPEED;
				if (m_fCamYaw > MAX_CRANE_YAW)
					m_fCamYaw = MAX_CRANE_YAW;
			}
			DRotation rCrane;
			ROT_INIT(rCrane);
			pServerDE->SetupEuler(&rCrane, 0.0f, m_fCamYaw, 0.0f);
			pServerDE->SetObjectRotation(m_hCrane, &rCrane);
			pServerDE->SetObjectUserFlags(m_hBall, (DDWORD)m_fCamYaw);
		}
	}
}



// Damage brush
BEGIN_CLASS(Damage)
	ADD_BOOLPROP_FLAG(ShowSurface, DFALSE, PF_HIDDEN)
    ADD_STRINGPROP_FLAG(SpriteSurfaceName, "", PF_HIDDEN)   
	ADD_REALPROP_FLAG(SurfaceHeight, 0.0f, PF_HIDDEN)
    ADD_COLORPROP_FLAG(SurfaceColor1, 0.0f, 0.0f, 0.0f, PF_HIDDEN)   
    ADD_COLORPROP_FLAG(SurfaceColor2, 0.0f, 0.0f, 0.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(XScaleMin, 15.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(XScaleMax, 25.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(YScaleMin, 15.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(YScaleMax, 25.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(XScaleDuration, 10.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(YScaleDuration, 10.0f, PF_HIDDEN)   
	ADD_VECTORPROP_VAL_FLAG(Current, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Viscosity, 0.0f, PF_HIDDEN)
	ADD_LONGINTPROP_FLAG(Code, CC_DAMAGE, PF_HIDDEN)
	ADD_REALPROP(Damage, 100.0f)
	ADD_LONGINTPROP(DamageType, DAMAGE_TYPE_NORMAL)
	ADD_BOOLPROP_FLAG(Locked, DFALSE, PF_HIDDEN)
	ADD_BOOLPROP_FLAG(UnlockKeyRemove, DFALSE, PF_HIDDEN)
	ADD_STRINGPROP_FLAG(UnlockKeyName, "", PF_HIDDEN)
END_CLASS_DEFAULT(Damage, VolumeBrush, NULL, NULL)


// Minefield brush
BEGIN_CLASS(Minefield)
	ADD_BOOLPROP_FLAG(ShowSurface, DFALSE, PF_HIDDEN)
    ADD_STRINGPROP_FLAG(SpriteSurfaceName, "", PF_HIDDEN)   
	ADD_REALPROP_FLAG(SurfaceHeight, 0.0f, PF_HIDDEN)
    ADD_COLORPROP_FLAG(SurfaceColor1, 0.0f, 0.0f, 0.0f, PF_HIDDEN)   
    ADD_COLORPROP_FLAG(SurfaceColor2, 0.0f, 0.0f, 0.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(XScaleMin, 15.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(XScaleMax, 25.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(YScaleMin, 15.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(YScaleMax, 25.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(XScaleDuration, 10.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(YScaleDuration, 10.0f, PF_HIDDEN)   
	ADD_VECTORPROP_VAL_FLAG(Current, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Viscosity, 0.0f, PF_HIDDEN)
	ADD_LONGINTPROP_FLAG(Code, CC_DAMAGE, PF_HIDDEN)
	ADD_REALPROP_FLAG(Damage, 0.0f, PF_HIDDEN)
	ADD_LONGINTPROP_FLAG(DamageType, DAMAGE_TYPE_NORMAL, PF_HIDDEN)
	ADD_BOOLPROP_FLAG(Locked, DFALSE, PF_HIDDEN)
	ADD_BOOLPROP_FLAG(UnlockKeyRemove, DFALSE, PF_HIDDEN)
	ADD_STRINGPROP_FLAG(UnlockKeyName, "", PF_HIDDEN)
END_CLASS_DEFAULT(Minefield, VolumeBrush, NULL, NULL)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Minefield::UpdatePhysics()
//
//	PURPOSE:	Update the physics of the passed in object
//
// ----------------------------------------------------------------------- //

void Minefield::UpdatePhysics(ContainerPhysics* pCPStruct)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;
	CollisionInfo collisionInfo;
	DBOOL bOnGround;
	CPlayerObj *pPlayerObj;

	// See if we're standing on something
	pServerDE->GetStandingOn(pCPStruct->m_hObject, &collisionInfo);

	bOnGround = DFALSE;
	if( IsPlayer( pCPStruct->m_hObject ))
	{
		pPlayerObj = ( CPlayerObj * )pServerDE->HandleToObject( pCPStruct->m_hObject );
		bOnGround = DTRUE;
	}
	else if( IsBaseCharacter( pCPStruct->m_hObject ))
	{
		if( collisionInfo.m_hObject )
			bOnGround = DTRUE;
	}
	else
		return;

	if( bOnGround )
	{
		Damage( pCPStruct->m_hObject );
	}
}

void Minefield::Damage( HOBJECT hObj )
{
	if( !IsBaseCharacter( hObj ))
		return;

	CBaseCharacter *pBC = (CBaseCharacter*)g_pServerDE->HandleToObject( hObj );
	if( pBC && !pBC->IsDead( ))
	{
		// Boom.
		DVector vDir;
		DVector vPos;
		g_pServerDE->GetObjectPos( hObj, &vPos );
		VEC_SET(vDir, 0, 1, 0);

		// Do some damage, suicide-style
		DamageObject( hObj, 
					 g_pServerDE->HandleToObject( hObj ), 
					 hObj, 
					 500.0f, vDir, vPos, 
					 DAMAGE_TYPE_DEATH);
		AddExplosion(vPos);
	}
}

void Minefield::AddExplosion(DVector &vPos)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	DDWORD		nType = EXP_GRENADE;
	DVector		vUp;
	VEC_SET(vUp, 0.0f, 1.0f, 0.0f);

	HMESSAGEWRITE hMessage = pServerDE->StartInstantSpecialEffectMessage(&vPos);
	pServerDE->WriteToMessageByte(hMessage, SFX_EXPLOSIONFX_ID);

	pServerDE->WriteToMessageVector(hMessage, &vPos);
	pServerDE->WriteToMessageVector(hMessage, &vUp);
	pServerDE->WriteToMessageDWord(hMessage, nType);

	pServerDE->EndMessage(hMessage);

	PlaySoundFromPos(&vPos, "Sounds\\Weapons\\c4\\explosion_1.wav", 1000.0f, SOUNDPRIORITY_MISC_MEDIUM);
}

DDWORD Minefield::ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead)
{
	CServerDE* pServerDE = GetServerDE();
	switch (messageID)
	{
		case MID_INCONTAINER:
		{
			Damage( hSender );
			break;
		}
	}
	
	return VolumeBrush::ObjectMessageFn(hSender, messageID, hRead);
}




// freefall
BEGIN_CLASS(FreeFall)
	ADD_BOOLPROP_FLAG(ShowSurface, DFALSE, PF_HIDDEN)
    ADD_STRINGPROP_FLAG(SpriteSurfaceName, "", PF_HIDDEN)   
	ADD_REALPROP_FLAG(SurfaceHeight, 0.0f, PF_HIDDEN)
    ADD_COLORPROP_FLAG(SurfaceColor1, 0.0f, 0.0f, 0.0f, PF_HIDDEN)   
    ADD_COLORPROP_FLAG(SurfaceColor2, 0.0f, 0.0f, 0.0f, PF_HIDDEN)   
	ADD_VECTORPROP_VAL_FLAG(Current, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
    ADD_REALPROP_FLAG(XScaleMin, 15.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(XScaleMax, 25.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(YScaleMin, 15.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(YScaleMax, 25.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(XScaleDuration, 10.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(YScaleDuration, 10.0f, PF_HIDDEN)   
	ADD_REALPROP_FLAG(SurfaceHeight, 5.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(SurfaceAlpha, 0.7f, PF_HIDDEN)
	ADD_LONGINTPROP_FLAG(NumSurfacePolies, 160, PF_HIDDEN)
	ADD_REALPROP_FLAG(Viscosity, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Damage, 9999999.0f, PF_HIDDEN)
	ADD_LONGINTPROP_FLAG(DamageType, DAMAGE_TYPE_FREEFALL, PF_HIDDEN)
	ADD_LONGINTPROP_FLAG(Code, CC_FREEFALL, PF_HIDDEN)
	ADD_BOOLPROP_FLAG(Locked, DFALSE, PF_HIDDEN)
	ADD_BOOLPROP_FLAG(UnlockKeyRemove, DFALSE, PF_HIDDEN)
	ADD_STRINGPROP_FLAG(UnlockKeyName, "", PF_HIDDEN)
END_CLASS_DEFAULT(FreeFall, VolumeBrush, NULL, NULL)


