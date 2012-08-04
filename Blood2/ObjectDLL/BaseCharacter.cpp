// ----------------------------------------------------------------------- //
//
// MODULE  : BaseCharacter.cpp
//
// PURPOSE : Base class for player and AI
//
// CREATED : 10/6/97
//
// ----------------------------------------------------------------------- //

#include "BaseCharacter.h"
#include "objectutilities.h"
#include "VolumeBrush.h"
#include "ClientServerShared.h"
#include "SParam.h"
#include "PlayerObj.h"
#include <stdio.h>


//#define FLAG_BASECHARACTER		1
//#define FLAG_PLAYER				2

#define LADDER_STOP_TIME			0.2f
#define SWIM_STOP_TIME				1.0f


BEGIN_CLASS(CBaseCharacter)
	ADD_DESTRUCTABLE_AGGREGATE()
END_CLASS_DEFAULT(CBaseCharacter, B2BaseClass, NULL, NULL)


CBaseCharacter::CBaseCharacter(DBYTE nType) : B2BaseClass(nType)
{
 	AddAggregate(&m_damage);
	AddAggregate(&m_InventoryMgr);
	AddAggregate(&m_MoveObj);

	m_eContainerCode			= CC_NOTHING;
	m_eLastContainerCode		= CC_NOTHING;
	m_bBodyInLiquid				= DFALSE;
	m_bBodyOnLadder				= DFALSE;
	m_bSwimmingOnSurface		= DFALSE;

	m_dwSurfType				= SURFTYPE_FLESH;

	m_fSwimVel					= DEFAULT_SWIM_VEL;
	m_fLadderVel				= DEFAULT_LADDER_VEL;

	m_nTrapped					= 0;
	m_bTrappedGrav				= DTRUE;
}



DDWORD CBaseCharacter::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	switch(messageID)
	{
		case MID_UPDATE:
			m_eLastContainerCode = m_eContainerCode;
			break;

		case MID_MODELSTRINGKEY:
		{
			OnStringKey((ArgList*)pData);
			break;
		}

		case MID_SAVEOBJECT:
			Save((HMESSAGEWRITE)pData, (DDWORD)fData);
			break;

		case MID_LOADOBJECT:
			Load((HMESSAGEREAD)pData, (DDWORD)fData);
			break;

		case MID_INITIALUPDATE:
			{
				if (fData != INITIALUPDATE_SAVEGAME)
				{
					// Set some flags that all basecharacters should have
					// Doing it here for now since I don't want to change every AI file - GK 3/18
					DDWORD dwFlags = g_pServerDE->GetObjectFlags(m_hObject);
					g_pServerDE->SetObjectFlags(m_hObject, dwFlags | FLAG_MODELGOURAUDSHADE | FLAG_ANIMTRANSITION);
					g_pServerDE->SetObjectUserFlags(m_hObject, USERFLG_NIGHTGOGGLESGLOW );

					dwFlags = g_pServerDE->GetObjectUserFlags(m_hObject);
					dwFlags |= ((m_dwSurfType << 24) + USRFLG_SAVEABLE | USRFLG_SINGULARITY_ATTRACT);
	 				g_pServerDE->SetObjectUserFlags(m_hObject, dwFlags);
				}
			}
			break;

		default : break;
	}

	return B2BaseClass::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseCharacter::OnStringKey()
//
//	PURPOSE:	Handle animation command
//
// ----------------------------------------------------------------------- //

void CBaseCharacter::OnStringKey(ArgList* pArgList)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !pArgList || !pArgList->argv || pArgList->argc == 0) return;

	char* pKey = pArgList->argv[0];
	if (!pKey) return;

	// get rotation
	DRotation rRot;
    DVector m_vUp, m_vRight, m_vForward;
    
	VEC_INIT(m_vUp);
	VEC_INIT(m_vRight);
	VEC_INIT(m_vForward);

	char szTemp[32];

	//check for direction change for velocity 
	if(Sparam_Get(szTemp,pKey,"RotY"))
	{
		DFLOAT fTurn = (DFLOAT)atof(szTemp);

		pServerDE->GetObjectRotation(m_hObject, &rRot);
		pServerDE->EulerRotateY(&rRot, fTurn);
		pServerDE->SetObjectRotation(m_hObject, &rRot);	
	}

	//check for forward/backward velocity
	if(Sparam_Get(szTemp,pKey,"ForwVel"))
	{
		pServerDE->GetObjectRotation(m_hObject, &rRot);
		pServerDE->GetRotationVectors(&rRot, &m_vUp, &m_vRight, &m_vForward);

		DVector vVel;
		VEC_MULSCALAR(vVel, m_vForward, (DFLOAT)atof(szTemp));

		pServerDE->SetVelocity(m_hObject, &vVel);
	}

	//check for right/left velocity
	if(Sparam_Get(szTemp,pKey,"RightVel"))
	{
		pServerDE->GetObjectRotation(m_hObject, &rRot);
		pServerDE->GetRotationVectors(&rRot, &m_vUp, &m_vRight, &m_vForward);

		DVector vVel;
		VEC_MULSCALAR(vVel, m_vRight, (DFLOAT)atof(szTemp));

		pServerDE->SetVelocity(m_hObject, &vVel);
	}


	//check for up/down velocity
	if(Sparam_Get(szTemp,pKey,"UpVel"))
	{
		pServerDE->GetObjectRotation(m_hObject, &rRot);
		pServerDE->GetRotationVectors(&rRot, &m_vUp, &m_vRight, &m_vForward);

		DVector vVel;
		VEC_MULSCALAR(vVel, m_vUp, (DFLOAT)atof(szTemp));

		pServerDE->SetVelocity(m_hObject, &vVel);
	}

	//check for position change
	if(Sparam_Get(szTemp,pKey,"AddX"))
	{
		DVector vPos,vNewPos;
		char szY[10],szZ[10];

		Sparam_Get(szY,pKey,"AddY");
		Sparam_Get(szZ,pKey,"AddZ");

		VEC_INIT(vPos);
		VEC_SET(vNewPos, atof(szTemp), atof(szY), atof(szZ));
		pServerDE->GetObjectPos(m_hObject, &vPos);
		VEC_ADD(vPos,vPos,vNewPos);

		//apply new dims
		pServerDE->MoveObject(m_hObject, &vPos);
	}

	//check for dims change
	if(Sparam_Get(szTemp,pKey,"DimsX"))
	{
		DVector vDims;
		char szY[10],szZ[10];

		Sparam_Get(szY,pKey,"DimsY");
		Sparam_Get(szZ,pKey,"DimsZ");

		VEC_INIT(vDims);
		VEC_SET(vDims, atof(szTemp), atof(szY), atof(szZ));
		
		//apply new dims
		pServerDE->SetObjectDims(m_hObject, &vDims);
	}

	return;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseCharacter::Trap
//
//	PURPOSE:	Don't allow a player to move (with or without gravity)
//
// ----------------------------------------------------------------------- //

DBOOL CBaseCharacter::Trap(char nTrap, DBOOL bGravity)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hObject) return DFALSE;

	// Modify the trapped counter...
	if(nTrap == -1)
	{
		m_nTrapped = 0;
		nTrap = 0;
	}
	else if(nTrap)
		m_nTrapped++;
	else
		m_nTrapped--;

	// Don't let the trapped counter drop below zero...
	if(m_nTrapped < 0)
		m_nTrapped = 0;

	// Zero out the current velocity
	if(nTrap)
	{
		DVector vTemp;
		VEC_INIT(vTemp);

		if(!bGravity)
		{
			pServerDE->GetVelocity(m_hObject, &vTemp);
			vTemp.x = vTemp.z = 0.0f;
		}

		pServerDE->SetVelocity(m_hObject, &vTemp);
		pServerDE->SetNextUpdate(m_hObject, 0.0001f);
	}

	// Trap the client if it's a player
	if(IsPlayer(m_hObject))
	{
		CPlayerObj *pHit = (CPlayerObj*)pServerDE->HandleToObject(m_hObject);

		HMESSAGEWRITE hMsg = pServerDE->StartMessage(pHit->GetClient(), SMSG_TRAPPED);
		pServerDE->WriteToMessageByte(hMsg, (DBYTE)m_nTrapped);
		pServerDE->WriteToMessageByte(hMsg, 1);
		pServerDE->EndMessage2(hMsg, MESSAGE_GUARANTEED | MESSAGE_NAGGLE);
	}

	// Save or restore the original gravity setting if needed
/*	if(bGravity)
	{
		DDWORD	dwFlags;
		dwFlags = pServerDE->GetObjectFlags(m_hObject);

		if(nTrap && (m_nTrapped == 1))
		{
			m_bTrappedGrav = (DBOOL)(dwFlags & FLAG_GRAVITY);
			pServerDE->SetObjectFlags(m_hObject, dwFlags & ~FLAG_GRAVITY);
		}
		else if(!nTrap && !m_nTrapped && m_bTrappedGrav)
		{
			pServerDE->SetObjectFlags(m_hObject, dwFlags | FLAG_GRAVITY);
		}
	}*/

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseCharacter::UpdateInLiquid
//
//	PURPOSE:	Update movement when in liquid
//
// ----------------------------------------------------------------------- //

void CBaseCharacter::UpdateInLiquid(VolumeBrush* pBrush, ContainerPhysics* pCPStruct)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !pBrush || !pCPStruct) return;

	m_bBodyInLiquid = DTRUE;

	DBOOL bHeadInLiquid = IsLiquid(m_eContainerCode);

	// Undo viscosity dampening done in brush (we'll do our own)...

	pBrush->UndoViscosityCalculation(pCPStruct);


	// Do REAL friction dampening (i.e., actually change our velocity)...

	DVector vVel;
	pServerDE->GetVelocity(m_hObject, &vVel);

	DFLOAT fTimeDelta = pServerDE->GetFrameTime();

	DVector vCurVel;
	VEC_COPY(vCurVel, vVel);

	if (VEC_MAG(vCurVel) > 1.0f)
	{
		DVector vDir;
		VEC_COPY(vDir, vCurVel);
		VEC_NORM(vDir);

		DFLOAT fAdjust = pServerDE->GetFrameTime()*(m_fSwimVel/SWIM_STOP_TIME);

		VEC_MULSCALAR(vVel, vDir, fAdjust);

		if (VEC_MAG(vVel) < VEC_MAG(vCurVel))
		{
			VEC_SUB(vVel, vCurVel, vVel);
		}
		else
		{
			VEC_INIT(vVel);
		}
	}


	// Handle floating around on the surface...

	if (m_bSwimmingOnSurface)
	{
		DBOOL bMoving = ((VEC_MAG(pCPStruct->m_Acceleration) > 0.01f) ||
						 (VEC_MAG(pCPStruct->m_Velocity) > 0.01f));
	
		pCPStruct->m_Flags &= ~FLAG_GRAVITY;  // Turn gravity off

		if (bMoving)  // Turn off y acceleration and velocity
		{
			if (vVel.y > 0.0f || pCPStruct->m_Acceleration.y > 0.0f)
			{
				vVel.y						= 0.0f;
				pCPStruct->m_Velocity.y		= 0.0f;
				pCPStruct->m_Acceleration.y = 0.0f;
			}
		}
		else // Pull us down if we're not moving (fast enough)
		{
			pCPStruct->m_Acceleration.y += pBrush->GetGravity();
		}
	}
	else if (IsLiquid(m_eContainerCode))
	{
		pCPStruct->m_Flags &= ~FLAG_GRAVITY;
		pCPStruct->m_Acceleration.y += pBrush->GetGravity();
	}

	// If we're trapped, don't move...
	if(m_nTrapped)
		{ VEC_INIT(vVel); }

	pServerDE->SetVelocity(m_hObject, &vVel);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseCharacter::UpdateOnLadder
//
//	PURPOSE:	Update movement when on a ladder
//
// ----------------------------------------------------------------------- //

void CBaseCharacter::UpdateOnLadder(VolumeBrush* pBrush, ContainerPhysics* pCPStruct)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !pBrush || !pCPStruct) return;

	m_bBodyOnLadder = DTRUE;

	// Do REAL friction dampening (i.e., actually change our velocity)...

	DVector vVel;
	pServerDE->GetVelocity(m_hObject, &vVel);

	DVector vCurVel;
	VEC_COPY(vCurVel, vVel);

	if (VEC_MAG(vCurVel) > 1.0f)
	{
		DVector vDir;
		VEC_COPY(vDir, vCurVel);
		VEC_NORM(vDir);

		DFLOAT fAdjust = pServerDE->GetFrameTime()*(m_fLadderVel/LADDER_STOP_TIME);

		VEC_MULSCALAR(vVel, vDir, fAdjust);

		if (VEC_MAG(vVel) < VEC_MAG(vCurVel))
		{
			VEC_SUB(vVel, vCurVel, vVel);
		}
		else
		{
			VEC_INIT(vVel);
		}

		// If we're trapped, don't move...
		if(m_nTrapped)
			{ VEC_INIT(vVel); }

		pServerDE->SetVelocity(m_hObject, &vVel);
	}

	pCPStruct->m_Flags &= ~FLAG_GRAVITY;  // Turn gravity off
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseCharacter::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CBaseCharacter::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hWrite) return;

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseCharacter::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CBaseCharacter::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hRead) return;

}

