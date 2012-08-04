// ----------------------------------------------------------------------- //
//
// MODULE  : UhlanA3.cpp
//
// PURPOSE : UhlanA3 - Implementation
//
// CREATED : 1/29/98
//
// ----------------------------------------------------------------------- //

#include "UhlanA3.h"

#define ANIM_ASSAULTRIFLE		"SHREDDER"
#define ANIM_ROCKETS			"ROCKETS"

#define ASSAULTRIFLE_PERCENTAGE	50

// These values are used as offsets from the model position for the indicated
// weapon...

#define NUM_ROCKET_POS	28
DVector s_vRocketPos[NUM_ROCKET_POS] = 
{   // F,     U,     R
	DVector(20.0f, 10.0f, 10.0f),
	DVector(20.0f, 10.0f, -10.0f),
	DVector(20.0f, -5.0f, 10.0f),
	DVector(20.0f, -5.0f, -10.0f),
	DVector(20.0f, -20.0f, 10.0f),
	DVector(20.0f, -20.0f, -10.0f),
	DVector(20.0f, 5.0f, 10.0f),
	DVector(20.0f, 5.0f, -10.0f),
	DVector(20.0f, 0.0f, 10.0f),
	DVector(20.0f, 0.0f, -10.0f),
	DVector(20.0f, -10.0f, 10.0f),
	DVector(20.0f, -10.0f, -10.0f),
	DVector(20.0f, -15.0f, 10.0f),
	DVector(20.0f, -15.0f, -10.0f),
	DVector(20.0f, 10.0f, 5.0f),
	DVector(20.0f, 10.0f, -5.0f),
	DVector(20.0f, -5.0f, 5.0f),
	DVector(20.0f, -5.0f, -5.0f),
	DVector(20.0f, -20.0f, 5.0f),
	DVector(20.0f, -20.0f, -5.0f),
	DVector(20.0f, 5.0f, 5.0f),
	DVector(20.0f, 5.0f, -5.0f),
	DVector(20.0f, 0.0f, 5.0f),
	DVector(20.0f, 0.0f, -5.0f),
	DVector(20.0f, -10.0f, 5.0f),
	DVector(20.0f, -10.0f, -5.0f),
	DVector(20.0f, -15.0f, 5.0f),
	DVector(20.0f, -15.0f, -5.0f)
};

#define NUM_ASSAULTRIFLE_POS 1
DVector s_vAssaultRiflePos[1] =
{	// F,     U,     R
	DVector(42.0f, 13.0f, 0.0f)
};


BEGIN_CLASS(UhlanA3)
	ADD_LONGINTPROP( State, Vehicle::DEFENSIVE )
	ADD_LONGINTPROP( WeaponId, GUN_ASSAULTRIFLE_ID )
	ADD_STRINGPROP_FLAG( Filename, GetModel(MI_AI_UHLANA3_ID), PF_DIMS | PF_HIDDEN )
END_CLASS_DEFAULT( UhlanA3, Vehicle, NULL, NULL )


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	UhlanA3::UhlanA3()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

UhlanA3::UhlanA3() : Vehicle()
{
	m_nModelId		= MI_AI_UHLANA3_ID;
	m_bIsMecha		= DTRUE;

	m_hShredderAni	= INVALID_ANI;
	m_hRocketsAni	= INVALID_ANI;

	m_pIdleSound = "Sounds\\Enemies\\Vehicle\\UhlanA3\\Idle.wav";
	m_pRunSound	 = "Sounds\\Enemies\\Vehicle\\UhlanA3\\Run.wav";

	m_bCreateHandHeldWeapon	= DFALSE;
	m_bChangeAnimation		= DFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	UhlanA3::SetAnimationIndexes()
//
//	PURPOSE:	Initialize model animation indexes
//
// ----------------------------------------------------------------------- //
	
void UhlanA3::SetAnimationIndexes()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hObject) return;

	Vehicle::SetAnimationIndexes();

	m_hShredderAni	= pServerDE->GetAnimIndex(m_hObject, ANIM_ASSAULTRIFLE);
	m_hRocketsAni	= pServerDE->GetAnimIndex(m_hObject, ANIM_ROCKETS);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	UhlanA3::UpdateWeapon()
//
//	PURPOSE:	Update the our weapon
//
// ----------------------------------------------------------------------- //

void UhlanA3::UpdateWeapon()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hObject) return;

	m_bChangeAnimation = DFALSE;

	// See if we are firing...

	if (IsFiring())
	{
		// If we just started firing, figure out what weapon/animation to use...

		if (!(m_dwLastAction & AI_AFLG_FIRE))
		{
			m_bChangeAnimation = DTRUE;
		}
	}

	Vehicle::UpdateWeapon();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	UhlanA3::UpdateAnimation()
//
//	PURPOSE:	Update the current animation
//
// ----------------------------------------------------------------------- //

void UhlanA3::UpdateAnimation()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hObject) return;


	// See if we should change our animation...

	if (m_bChangeAnimation)
	{
		m_bAllowMovement = DFALSE;

		ModelSize eSize				= MS_NORMAL;
		DFLOAT	fMinFireDuration	= 1.0f;
		DFLOAT	fMaxFireDuration	= 2.0f;
		DFLOAT	fMinFireRest		= 3.0f;
		DFLOAT	fMaxFireRest		= 6.0f;

		if (GetRandom(1, 100) <= ASSAULTRIFLE_PERCENTAGE)
		{
			m_nWeaponId = GUN_ASSAULTRIFLE_ID;
			SetAnimation(m_hShredderAni, DTRUE);
		}
		else
		{
			m_nWeaponId = GUN_TOW_ID;
			SetAnimation(m_hRocketsAni, DTRUE);
			eSize = MS_SMALL;
		}

		m_weapons.ObtainWeapon(m_nWeaponId);
		m_weapons.DeselectCurWeapon();
		m_weapons.ChangeWeapon(m_nWeaponId);
		m_weapons.AddAmmo(m_nWeaponId, GetWeaponMaxAmmo(m_nWeaponId));

		CWeapon* pWeapon = m_weapons.GetCurWeapon();
		if (pWeapon) 
		{
			pWeapon->SetSize(eSize);
			pWeapon->SetMinFireDuration(fMinFireDuration);
			pWeapon->SetMaxFireDuration(fMaxFireDuration);
			pWeapon->SetMinFireRest(fMinFireRest);	
			pWeapon->SetMaxFireRest(fMaxFireRest);
		}
	}
	else if (!IsFiring())
	{
		m_bAllowMovement = DTRUE;

		Vehicle::UpdateAnimation();
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	UhlanA3::GetFirePos()
//
//	PURPOSE:	Get the current weapon fire position
//
// ----------------------------------------------------------------------- //

DVector UhlanA3::GetFirePos(DVector* pvPos)
{
	DVector vPos;
	VEC_INIT(vPos);

	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hObject || !pvPos) return vPos;

	DVector vOffset;
	if (m_nWeaponId == GUN_ASSAULTRIFLE_ID)
	{
		int nIndex = GetRandom(0, NUM_ASSAULTRIFLE_POS-1);
		VEC_COPY(vOffset, s_vAssaultRiflePos[nIndex]);
	}
	else
	{
		int nIndex = GetRandom(0, NUM_ROCKET_POS-1);
		VEC_COPY(vOffset, s_vRocketPos[nIndex]);
	}

	// Adjust fire position...

	DRotation rRot;
	pServerDE->GetObjectRotation(m_hObject, &rRot);

	DVector vU, vF, vR;
	pServerDE->GetRotationVectors(&rRot, &vU, &vR, &vF);

	DVector vTemp;
	VEC_MULSCALAR(vTemp, vF, vOffset.x);
	VEC_ADD(vPos, *pvPos, vTemp);

	VEC_MULSCALAR(vTemp, vU, vOffset.y);
	VEC_ADD(vPos, *pvPos, vTemp);

	VEC_MULSCALAR(vTemp, vR, vOffset.z);
	VEC_ADD(vPos, *pvPos, vTemp);

	return vPos;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	UhlanA3::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD UhlanA3::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	switch(messageID)
	{
		case MID_SAVEOBJECT:
		{
			Save((HMESSAGEWRITE)pData, (DDWORD)fData);
		}
		break;

		case MID_LOADOBJECT:
		{
			Load((HMESSAGEREAD)pData, (DDWORD)fData);
		}
		break;

		default : break;
	}

	return Vehicle::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	UhlanA3::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void UhlanA3::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hWrite) return;

	pServerDE->WriteToMessageDWord(hWrite, m_hShredderAni);
	pServerDE->WriteToMessageDWord(hWrite, m_hRocketsAni);
	pServerDE->WriteToMessageByte(hWrite, m_bSaveAllowmovement);
	pServerDE->WriteToMessageByte(hWrite, m_bChangeAnimation);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	UhlanA3::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void UhlanA3::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hRead) return;

	m_hShredderAni			= pServerDE->ReadFromMessageDWord(hRead);
	m_hRocketsAni			= pServerDE->ReadFromMessageDWord(hRead);
	m_bSaveAllowmovement	= pServerDE->ReadFromMessageByte(hRead);
	m_bChangeAnimation		= pServerDE->ReadFromMessageByte(hRead);
}


BEGIN_CLASS(UCA_UhlanA3)
END_CLASS_DEFAULT(UCA_UhlanA3, UhlanA3, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	UCA_UhlanA3::UCA_UhlanA3()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

UCA_UhlanA3::UCA_UhlanA3() : UhlanA3()
{
	m_cc = UCA;
}


BEGIN_CLASS(CMC_UhlanA3)
END_CLASS_DEFAULT(CMC_UhlanA3, UhlanA3, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMC_UhlanA3::CMC_UhlanA3()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CMC_UhlanA3::CMC_UhlanA3() : UhlanA3()
{
	m_cc = CMC;
}


BEGIN_CLASS(FALLEN_UhlanA3)
END_CLASS_DEFAULT(FALLEN_UhlanA3, UhlanA3, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FALLEN_UhlanA3::FALLEN_UhlanA3()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

FALLEN_UhlanA3::FALLEN_UhlanA3() : UhlanA3()
{
	m_cc = FALLEN;
}


BEGIN_CLASS(CRONIAN_UhlanA3)
END_CLASS_DEFAULT(CRONIAN_UhlanA3, UhlanA3, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRONIAN_UhlanA3::CRONIAN_UhlanA3()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CRONIAN_UhlanA3::CRONIAN_UhlanA3() : UhlanA3()
{
	m_cc = CRONIAN;
}


BEGIN_CLASS(SHOGO_UhlanA3)
END_CLASS_DEFAULT(SHOGO_UhlanA3, UhlanA3, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SHOGO_UhlanA3::SHOGO_UhlanA3()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

SHOGO_UhlanA3::SHOGO_UhlanA3() : UhlanA3()
{
	m_cc = SHOGO;
}
