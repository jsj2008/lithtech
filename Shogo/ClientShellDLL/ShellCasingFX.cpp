//----------------------------------------------------------
//
// MODULE  : SHELLCASINGFX.CPP
//
// PURPOSE : defines classes for ejected shells
//
// CREATED : 5/1/98
//
//----------------------------------------------------------

// Includes....

#include "ShellCasingFX.h"
#include "WeaponDefs.h"
#include "clientheaders.h"
#include "ltlink.h"
#include "ClientUtilities.h"
#include "ltobjectcreate.h"

extern PhysicsState g_normalPhysicsState;
extern PhysicsState g_waterPhysicsState;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CShellCasingFX::CShellCasingFX
//
//	PURPOSE:	Initialize
//
// ----------------------------------------------------------------------- //

CShellCasingFX::CShellCasingFX()
{
	m_rRot.Init();
	VEC_INIT(m_vStartPos);
	m_nWeaponId = GUN_NONE;
		
	m_fExpireTime	= 0.0f;
    m_bInVisible	= LTTRUE;
	m_bResting		= LTFALSE;
	m_nBounceCount	= 2;	// Set maximum bounces
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CShellCasingFX::Init
//
//	PURPOSE:	Create the shell casing
//
// ----------------------------------------------------------------------- //

LTBOOL CShellCasingFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!psfxCreateStruct) return LTFALSE;

	CSpecialFX::Init(psfxCreateStruct);

	SHELLCREATESTRUCT* pShell = (SHELLCREATESTRUCT*)psfxCreateStruct;

	m_rRot = pShell->rRot;
	VEC_COPY(m_vStartPos, pShell->vStartPos);
	m_nWeaponId = pShell->nWeaponId;

	VEC_SET(m_vScale, 1.0f, 1.0f, 1.0f);

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CShellCasingFX::CreateObject
//
//	PURPOSE:	Create the model associated with the shell
//
// ----------------------------------------------------------------------- //

LTBOOL CShellCasingFX::CreateObject(ILTClient *pClientDE)
{
	if (!CSpecialFX::CreateObject(pClientDE)) return LTFALSE;

	char* pModelName = GetModelName();
	char* pSkinName  = GetSkinName();

	if (!pModelName || !pSkinName) return LTFALSE;

	// Setup the shell...

	ObjectCreateStruct createStruct;
	INIT_OBJECTCREATESTRUCT(createStruct);

	createStruct.m_ObjectType = OT_MODEL;
	createStruct.m_Flags = FLAG_VISIBLE | FLAG_NOLIGHT;
	SAFE_STRCPY(createStruct.m_Filename, pModelName);
	SAFE_STRCPY(createStruct.m_SkinName, pSkinName);
	VEC_COPY(createStruct.m_Pos, m_vStartPos);
	createStruct.m_Rotation = m_rRot;

	m_hObject = pClientDE->CreateObject(&createStruct);
	if (!m_hObject) return LTFALSE;


	m_pClientDE->SetObjectScale(m_hObject, &m_vScale);

	LTVector vU, vR, vF;
	m_pClientDE->Common()->GetRotationVectors(m_rRot, vU, vR, vF);

	VEC_SET(vU, 0.0f, 1.0f, 0.0f);
	m_pClientDE->Math()->AlignRotation(m_rRot, vF, vU);
	m_pClientDE->Common()->GetRotationVectors(m_rRot, vU, vR, vF);

	VEC_MULSCALAR(vU, vU, GetRandom(30.0f, 60.0f));
	VEC_MULSCALAR(vR, vR, GetRandom(80.0f, 110.0f));

	LTVector vStartVel;
	VEC_ADD(vStartVel, vU, vR);

	InitMovingObject(&m_movingObj, &m_vStartPos, &vStartVel);;

	m_fDieTime = pClientDE->GetTime() + GetRandom(10.0f, 15.0f);

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CShellCasingFX::Update
//
//	PURPOSE:	Update the shell
//
// ----------------------------------------------------------------------- //

LTBOOL CShellCasingFX::Update()
{
	if (!m_hObject || !m_pClientDE) return LTFALSE;

	if (m_pClientDE->GetTime() > m_fDieTime) return LTFALSE;

	if (m_bResting) return LTTRUE;

	LTRotation rRot;

	// If velocity slows enough, and we're on the ground, just stop bouncing and just wait to expire.

	if (m_movingObj.m_PhysicsFlags & MO_RESTING)
	{
		m_bResting = LTTRUE;

		// Stop the spinning...

		m_pClientDE->Common()->SetupEuler(rRot, 0, m_fYaw, 0);
		m_pClientDE->SetObjectRotation(m_hObject, &rRot);	
		
		// Shell is at rest, we can add a check here to see if we really want
		// to keep it around depending on detail settings...

		HLOCALOBJ hObjs[1];
		uint32 nNumFound, nBogus;
		m_pClientDE->FindObjectsInSphere(&m_movingObj.m_Pos, 64.0f, hObjs, 1, &nBogus, &nNumFound);

		// Remove thyself...
	
		if (nNumFound > 15) return LTFALSE;
	}
	else
	{
		if (m_fPitchVel != 0 || m_fYawVel != 0)
		{
			LTFLOAT fDeltaTime = m_pClientDE->GetFrameTime();

			m_fPitch += m_fPitchVel * fDeltaTime;
			m_fYaw   += m_fYawVel * fDeltaTime;

			m_pClientDE->Common()->SetupEuler(rRot, m_fPitch, m_fYaw, 0.0f);
			m_pClientDE->SetObjectRotation(m_hObject, &rRot);	
		}
	}


	LTVector vNewPos;
	if (UpdateMovingObject(LTNULL, &m_movingObj, &vNewPos))
	{
		ClientIntersectInfo info;
		if (BounceMovingObject(LTNULL, &m_movingObj, &vNewPos, &info))
		{
			if (m_nBounceCount == 2)
			{
				if (!(m_movingObj.m_PhysicsFlags & MO_LIQUID))
				{
					char* pSounds[] = 
					{ 
						"Sounds\\Weapons\\shell1.wav",
						"Sounds\\Weapons\\shell2.wav",
						"Sounds\\Weapons\\shell3.wav",
						"Sounds\\Weapons\\shell4.wav",
						"Sounds\\Weapons\\shell5.wav",
						"Sounds\\Weapons\\shell6.wav"
					};

					// Play appropriate sound...

					PlaySoundFromPos(&vNewPos, pSounds[GetRandom(0,5)], 200.0f,
									 SOUNDPRIORITY_MISC_LOW);
				}
			}

			// Adjust the bouncing..

			m_fPitchVel = GetRandom(-MATH_CIRCLE * 2, MATH_CIRCLE * 2);
			m_fYawVel	= GetRandom(-MATH_CIRCLE * 2, MATH_CIRCLE * 2);

			m_nBounceCount--;

			if (m_nBounceCount <= 0)
			{
				m_movingObj.m_PhysicsFlags |= MO_RESTING;
			}
		}

		VEC_COPY(m_movingObj.m_Pos, vNewPos);

		if (m_pClientDE->Common()->GetPointStatus(&vNewPos) == LT_OUTSIDE)
		{
			return LTFALSE;
		}

		m_pClientDE->SetObjectPos(m_hObject, &vNewPos);
	}

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CShellCasingFX::GetModelName
//
//	PURPOSE:	Get the name of the shell model
//
// ----------------------------------------------------------------------- //

char* CShellCasingFX::GetModelName() 
{ 
	char* pName = "Models\\PV_Weapons\\shell.abc"; 

	VEC_SET(m_vScale, 0.5f, 0.5f, 0.5f);

	switch ((RiotWeaponId)m_nWeaponId)
	{
		case GUN_SNIPERRIFLE_ID :
		{
			pName = "Models\\PV_Weapons\\sniper_shell.abc"; 
		}
		break;

		case GUN_COLT45_ID :
		{
			pName = "Models\\PV_Weapons\\colt_shell.abc"; 
		}
		break;

		case GUN_ASSAULTRIFLE_ID :
		{
			pName = "Models\\PV_Weapons\\assault_shell.abc"; 
		}
		break;
		
		case GUN_MAC10_ID :
		{
			pName = "Models\\PV_Weapons\\machinegun_shell.abc"; 
		}
		break;

		case GUN_SHOTGUN_ID	:
		{
			pName = "Models\\PV_Weapons\\shotgunshell.abc"; 
			VEC_SET(m_vScale, 4.0f, 4.0f, 4.0f);
		}
		break;

		case GUN_SHREDDER_ID :
		{
			pName = "Models\\PV_Weapons\\shredder_shell.abc"; 
			VEC_SET(m_vScale, 1.0f, 1.0f, 1.0f);
		}
		break;

		case GUN_JUGGERNAUT_ID :
		{
			pName = "Models\\PV_Weapons\\juggernaut_shell.abc"; 
			VEC_SET(m_vScale, 2.0f, 2.0f, 2.0f);
		}
		break;

		default : return LTNULL;
	}

	return pName;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CShellCasingFX::GetSkinName
//
//	PURPOSE:	Get the name of the shell skin
//
// ----------------------------------------------------------------------- //

char* CShellCasingFX::GetSkinName()  
{ 
	char* pName = "Skins\\Weapons\\shell.dtx"; 

	switch ((RiotWeaponId)m_nWeaponId)
	{
		case GUN_SNIPERRIFLE_ID :
		{
			pName = "Skins\\Weapons\\sniper_shell.dtx";
		}
		break;

		case GUN_COLT45_ID :
		{
			pName = "Skins\\Weapons\\colt_shell.dtx";
		}
		break;

		case GUN_ASSAULTRIFLE_ID :
		{
			pName = "Skins\\Weapons\\assault_shell.dtx";
		}
		break;

		case GUN_MAC10_ID :
		{
			pName = "Skins\\Weapons\\machinegun_shell.dtx";
		}
		break;

		case GUN_SHOTGUN_ID	:
		{
			pName = "Skins\\Weapons\\shotgunshell.dtx";
		}
		break;

		case GUN_SHREDDER_ID :
		{
			pName = "Skins\\Weapons\\shredder_shell.dtx";
		}
		break;

		case GUN_JUGGERNAUT_ID :
		{
			pName = "Skins\\Weapons\\juggernaut_shell.dtx"; 
		}
		break;

		default : return LTNULL;
	}

	return pName;
}
