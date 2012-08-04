// ----------------------------------------------------------------------- //
//
// MODULE  : ServerMeleeCollisionController.cpp
//
// PURPOSE : Server-side controller for managing rigidbody collisions
//
// CREATED : 01/20/05
//
// (c) 2003-2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "ServerMeleeCollisionController.h"
#include "Character.h"
#include "Arsenal.h"
#include "Weapon.h"
#include "WeaponFireInfo.h"
#include "AI.h"

// ----------------------------------------------------------------------- //

CMDMGR_BEGIN_REGISTER_CLASS( CServerMeleeCollisionController )
CMDMGR_END_REGISTER_CLASS( CServerMeleeCollisionController, IAggregate )

// ----------------------------------------------------------------------- //

CServerMeleeCollisionController::CServerMeleeCollisionController()
: IAggregate( "CServerMeleeCollisionController" )
{
	m_hObject = NULL;
}

// ----------------------------------------------------------------------- //

CServerMeleeCollisionController::~CServerMeleeCollisionController()
{
}

// ----------------------------------------------------------------------- //

uint32 CServerMeleeCollisionController::EngineMessageFn(LPBASECLASS pObject, uint32 messageID, void *pData, float fData)
{
	switch(messageID)
	{
	case MID_SAVEOBJECT:
		{
			Save((ILTMessage_Write*)pData, (uint8)fData);
		}
		break;

	case MID_LOADOBJECT:
		{
			Load((ILTMessage_Read*)pData, (uint8)fData);
		}
		break;
	}

	return IAggregate::EngineMessageFn(pObject, messageID, pData, fData);
}

// ----------------------------------------------------------------------- //

uint32 CServerMeleeCollisionController::ObjectMessageFn(LPBASECLASS pObject, HOBJECT hSender, ILTMessage_Read *pMsg)
{
	pMsg->SeekTo(0);
	uint32 messageID = pMsg->Readuint32();
	switch(messageID)
	{
	case MID_MELEEATTACK:
		{
			// Just do a normal weapon fire.  This avoids skipping years worth of code/features that we likely want for melee.
			// (The projectile should detect that it's already inside the object and not do the trace.)

			HOBJECT hTarget = pMsg->ReadObject();
			HMODELNODE hNodeHit = pMsg->Readuint32();
			LTVector vPos = pMsg->ReadLTVector();
			LTVector vDir = pMsg->ReadLTVector();
			int32 nTimestamp = pMsg->Readint32();

			//!!ARL: We should really be filtering these out on the client using IsServerObject (which isn't exposed to the game currently).  Then maybe change this into an assert.
			if (!hTarget)	// non-server object!
				break;

			CCharacter* pCharacter = CCharacter::DynamicCast(m_hObject);
			CWeapon* pWeapon = pCharacter ? pCharacter->GetArsenal()->GetCurWeapon() : NULL;
			if (pWeapon == NULL)
			{
				// This is valid; the character may have dropped or changes 
				// weapons since the last frame.  We handle this latency by 
				// throwing out any messages which don't need to be handled here.

				return IAggregate::ObjectMessageFn(pObject, hSender, pMsg);
			}

			// cause damage to the weapon for character impacts
			CCharacter* pCharTarget = CCharacter::DynamicCast(hTarget);
			if (pCharTarget && pCharTarget->IsAlive())
			{
				pWeapon->DamageWeapon();
			}

			WeaponFireInfo weaponFireInfo;

			weaponFireInfo.hFiredFrom  = m_hObject;
			weaponFireInfo.vPath       = vDir;
			weaponFireInfo.vFirePos    = vPos;
			weaponFireInfo.vFlashPos   = vPos;
			weaponFireInfo.hTestObj    = hTarget;
			weaponFireInfo.hNodeHit    = hNodeHit;
			weaponFireInfo.hFiringWeapon = pWeapon->GetModelObject();
			weaponFireInfo.fPerturb    = 0.f;
			weaponFireInfo.nFireTimestamp = nTimestamp;
			weaponFireInfo.bGuaranteedHit = true;

			pWeapon->UpdateWeapon(weaponFireInfo, true);
		}
		break;

	case MID_MELEEBLOCK:
		{
			HOBJECT hTarget = pMsg->ReadObject();

			// Player handling is performed on client.

			CCharacter* pAttacker = CCharacter::DynamicCast(m_hObject);
			CCharacter* pTarget = CCharacter::DynamicCast(hTarget);

			if (pAttacker)
				pAttacker->HandleMeleeBlocked();

			if (pTarget)
				pTarget->HandleMeleeBlockedAttacker();
		}
		break;

		default : break;
	}

	return IAggregate::ObjectMessageFn(pObject, hSender, pMsg);
}

// ----------------------------------------------------------------------- //

void CServerMeleeCollisionController::Init(HOBJECT hParent)
{
	m_hObject = hParent;
}

// ----------------------------------------------------------------------- //

void CServerMeleeCollisionController::Save(ILTMessage_Write *pMsg, uint8 nType)
{
	if (!pMsg) return;

	SAVE_HOBJECT(m_hObject);
}

// ----------------------------------------------------------------------- //

void CServerMeleeCollisionController::Load(ILTMessage_Read *pMsg, uint8 nType)
{
	if (!pMsg) return;

	LOAD_HOBJECT(m_hObject);
}

