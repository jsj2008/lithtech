//----------------------------------------------------------
//
// MODULE  : SHELLCASINGFX.CPP
//
// PURPOSE : defines classes for ejected shells
//
// CREATED : 9/8/98
//
//----------------------------------------------------------

// Includes....

#include "ShellCasingFX.h"
#include "cpp_client_de.h"
#include "ClientUtilities.h"
#include "SharedDefs.h"
#include "ClientUtilities.h"
#include <stdio.h>
#include <mbstring.h>
#include "SoundTypes.h"

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
	ROT_INIT(m_rRot);
	VEC_INIT(m_vStartPos);
		
	m_fExpireTime	= 0.0f;
    m_bInVisible	= DTRUE;
	m_bResting		= DFALSE;
	m_nBounceCount	= 2;	// Set maximum bounces
	m_bLeftHanded	= DFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CShellCasingFX::Init
//
//	PURPOSE:	Create the shell
//
// ----------------------------------------------------------------------- //

DBOOL CShellCasingFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!psfxCreateStruct) return DFALSE;

	CSpecialFX::Init(psfxCreateStruct);

	SHELLCREATESTRUCT* pShell = (SHELLCREATESTRUCT*)psfxCreateStruct;

	ROT_COPY(m_rRot, pShell->rRot);
	VEC_COPY(m_vStartPos, pShell->vStartPos);
	m_nAmmoType		= pShell->nAmmoType;
	m_bLeftHanded	= pShell->bLeftHanded;

	if(m_nAmmoType == AMMO_SHELL)
		{ VEC_SET(m_vScale, 2.25f, 2.25f, 2.25f); }
	else
		{ VEC_SET(m_vScale, 1.5f, 1.5f, 1.5f); }

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CShellCasingFX::CreateObject
//
//	PURPOSE:	Create the model associated with the shell
//
// ----------------------------------------------------------------------- //

DBOOL CShellCasingFX::CreateObject(CClientDE *pClientDE)
{
	if (!CSpecialFX::CreateObject(pClientDE)) return DFALSE;

	char* pModelName = DNULL;
	char* pSkinName = DNULL;
	if (!GetFileNames(&pModelName, &pSkinName))
		return DFALSE;

	if (!pModelName || !pSkinName) return DFALSE;

	// Setup the shell...

	ObjectCreateStruct createStruct;
	INIT_OBJECTCREATESTRUCT(createStruct);

	createStruct.m_ObjectType = OT_MODEL;
	createStruct.m_Flags = 0;
	_mbscpy((unsigned char*)createStruct.m_Filename, (const unsigned char*)pModelName);
	_mbscpy((unsigned char*)createStruct.m_SkinName, (const unsigned char*)pSkinName);
	VEC_COPY(createStruct.m_Pos, m_vStartPos);
	ROT_COPY(createStruct.m_Rotation, m_rRot);

	m_hObject = pClientDE->CreateObject(&createStruct);
	if (!m_hObject) return DFALSE;


	m_pClientDE->SetObjectScale(m_hObject, &m_vScale);

	DVector vU, vR, vF;
	pClientDE->GetRotationVectors(&m_rRot, &vU, &vR, &vF);

	DVector vVel;

	if(m_bLeftHanded)
		VEC_NEGATE(vR, vR);

	DFLOAT fUpVel = GetRandom(60.0f, 90.0f);
	VEC_MULSCALAR(vU, vU, fUpVel);
	DFLOAT fRightVel = GetRandom(50.0f, 70.0f);
	VEC_MULSCALAR(vR, vR, fRightVel);
	DFLOAT fForwardVel = GetRandom(10.0f, 25.0f);
	VEC_MULSCALAR(vF, vF, fForwardVel);

	VEC_ADD(vVel, vU, vR);
	VEC_ADD(vVel, vVel, vF);

	InitMovingObject(&m_movingObj, &m_vStartPos, &vVel);;
	m_movingObj.m_PhysicsFlags |= MO_HALFGRAVITY;

	m_fExpireTime = 20.0f + m_pClientDE->GetTime();

	// Set the pitch velocity
	m_fPitchVel = GetRandom(-MATH_CIRCLE * 2, MATH_CIRCLE * 2);
	m_fYawVel = GetRandom(-MATH_CIRCLE * 2, MATH_CIRCLE * 2);
	m_fPitch	= m_fYaw = 0.0f;

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CShellCasingFX::Update
//
//	PURPOSE:	Update the shell
//
// ----------------------------------------------------------------------- //

DBOOL CShellCasingFX::Update()
{
	if (!m_hObject || !m_pClientDE) 
		return DFALSE;

	if (m_pClientDE->GetTime() > m_fExpireTime) 
		return DFALSE;

	if (m_bInVisible)
	{
		m_bInVisible = DFALSE;
		m_pClientDE->SetObjectFlags(m_hObject, FLAG_VISIBLE);
	}

	if (m_bResting) return DTRUE;

	DRotation rRot;

	// If velocity slows enough, and we're on the ground, just stop bouncing and just wait to expire.

	if (m_movingObj.m_PhysicsFlags & MO_RESTING)
	{
		m_bResting = DTRUE;

		// Stop the spinning...

		m_pClientDE->SetupEuler(&rRot, 0, m_fYaw, 0);
		m_pClientDE->SetObjectRotation(m_hObject, &rRot);	
/*		
		// Shell is at rest, we can add a check here to see if we really want
		// to keep it around depending on detail settings...

		HLOCALOBJ hObjs[1];
		DDWORD nNumFound, nBogus;
		m_pClientDE->FindObjectsInSphere(&m_movingObj.m_Pos, 64.0f, hObjs, 1, &nBogus, &nNumFound);

		// Remove thyself...
	
		if (nNumFound > 15) return DFALSE;
*/
	}
	else
	{
		if (m_fPitchVel != 0 || m_fYawVel != 0)
		{
			DFLOAT fDeltaTime = m_pClientDE->GetFrameTime();

			m_fPitch += m_fPitchVel * fDeltaTime;
			m_fYaw   += m_fYawVel * fDeltaTime;

			m_pClientDE->SetupEuler(&rRot, m_fPitch, m_fYaw, 0.0f);
			m_pClientDE->SetObjectRotation(m_hObject, &rRot);	
		}
	}


	DVector vNewPos;
	if (UpdateMovingObject(DNULL, &m_movingObj, &vNewPos))
	{
		ClientIntersectInfo info;
		SurfaceType eType = SURFTYPE_UNKNOWN;
		if (BounceMovingObject(DNULL, &m_movingObj, &vNewPos, &info, &eType))
		{
			if (m_nBounceCount > 0)
			{
				char sType[10];
				char sFile[MAX_CS_FILENAME_LEN];
				switch(eType)
				{
//					case SURFTYPE_FLESH:	_mbscpy((unsigned char*)sType, (const unsigned char*)"Flesh"); break;
					case SURFTYPE_GLASS:	_mbscpy((unsigned char*)sType, (const unsigned char*)"Glass"); break;
					case SURFTYPE_METAL:	_mbscpy((unsigned char*)sType, (const unsigned char*)"Metal"); break;
					case SURFTYPE_PLASTIC:	_mbscpy((unsigned char*)sType, (const unsigned char*)"Plastic"); break;
					case SURFTYPE_TERRAIN:	_mbscpy((unsigned char*)sType, (const unsigned char*)"Terrain"); break;
					case SURFTYPE_LIQUID:	_mbscpy((unsigned char*)sType, (const unsigned char*)"Water"); break;
					case SURFTYPE_WOOD:		_mbscpy((unsigned char*)sType, (const unsigned char*)"Wood"); break;
					case SURFTYPE_STONE: 
					default:				_mbscpy((unsigned char*)sType, (const unsigned char*)"Stone"); break;
				}

				sprintf(sFile, "Sounds\\Weapons\\ShellDrops\\%s\\Shell%d.wav", sType, GetRandom(1, 2));

				PlaySoundFromPos(&vNewPos, sFile, 150.0f, SOUNDPRIORITY_MISC_LOW);
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

		if (m_pClientDE->GetPointStatus(&vNewPos) == DE_OUTSIDE)
		{
 			return DFALSE;
		}

		m_pClientDE->SetObjectPos(m_hObject, &vNewPos);
	}

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CShellCasingFX::GetModelName
//
//	PURPOSE:	Get the name of the shell model
//
// ----------------------------------------------------------------------- //

DBOOL CShellCasingFX::GetFileNames(char **pModelName, char **pSkinName) 
{ 
	if (!pModelName || !pSkinName)
		return DFALSE;

	if (m_nAmmoType == AMMO_BULLET)
	{
		*pModelName	= "models/ammo/bshell.abc";
		*pSkinName	= "skins/ammo/bshell.dtx";
	}
	else if (m_nAmmoType == AMMO_SHELL)
	{
		*pModelName	= "models/ammo/sshell.abc";
		*pSkinName	= "skins/ammo/sshell.dtx";
	}
	else
	{
		*pModelName	= NULL;
		*pSkinName	= NULL;
		return DFALSE;
	}

	return DTRUE;
}

