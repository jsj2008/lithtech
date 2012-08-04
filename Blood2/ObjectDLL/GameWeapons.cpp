//----------------------------------------------------------
//
// MODULE  : GAMEWEAPONS.CPP
//
// PURPOSE : Blood2 weapon classes
//
// CREATED : 9/20/97
//
//----------------------------------------------------------

// Includes....
#include <stdio.h>
#include "InventoryMgr.h"
#include "BaseCharacter.h"
#include "generic_msg_de.h"
#include "GameProjectiles.h"
#include "PlayerObj.h"
#include "SFXMsgIds.h"
#include "ClientTracer.h"
#include "GameWeapons.h"
#include "ObjectUtilities.h"
#include "ClientLaserFX.h"
#include "ViewWeaponModel.h"
#include "ClientServerShared.h"
#include "VolumeBrushTypes.h" // For LiquidFilterFn
#include "destructable.h"

#include "gib.h"


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CShikariClaw::Fire()
//
//	PURPOSE:	Fires the weapon
//
// ----------------------------------------------------------------------- //
DDWORD CShikariClaw::Fire()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE)
		return 0;

	DBOOL bAltFire = (m_eState == WS_ALT_FIRING);

	int sound = pServerDE->IntRandom(0, 4);

	// Choose a random firing sound
	if (bAltFire)
	{
		if (sound < 1)		m_szFireSound = "Sounds\\Slash1.wav";
		else if (sound < 2) m_szFireSound = "Sounds\\Slash2.wav";
		else if (sound < 3)	m_szFireSound = "Sounds\\Slash3.wav";
		else				m_szFireSound = "Sounds\\Slash4.wav";
	}
	else
	{
		if (sound < 1)		m_szAltFireSound = "Sounds\\Slash1.wav";
		else if (sound < 2) m_szAltFireSound = "Sounds\\Slash2.wav";
		else if (sound < 3)	m_szAltFireSound = "Sounds\\Slash3.wav";
		else				m_szAltFireSound = "Sounds\\Slash4.wav";
	}

	return CWeapon::Fire();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSoulCrowbar::Fire()
//
//	PURPOSE:	Fires the weapon
//
// ----------------------------------------------------------------------- //
DDWORD CSoulCrowbar::Fire()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE)
		return 0;

	DBOOL bAltFire = (m_eState == WS_ALT_FIRING);

	int sound = pServerDE->IntRandom(0, 4);

	if (sound < 1)		m_szAltFireSound = "Sounds\\Slash1.wav";
	else if (sound < 2) m_szAltFireSound = "Sounds\\Slash2.wav";
	else if (sound < 3)	m_szAltFireSound = "Sounds\\Slash3.wav";
	else				m_szAltFireSound = "Sounds\\Slash4.wav";

	return CWeapon::Fire();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSoulAxe::Fire()
//
//	PURPOSE:	Fires the weapon
//
// ----------------------------------------------------------------------- //
DDWORD CSoulAxe::Fire()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE)
		return 0;

	DBOOL bAltFire = (m_eState == WS_ALT_FIRING);

	int sound = pServerDE->IntRandom(0, 4);

	if (sound < 1)		m_szAltFireSound = "Sounds\\Slash1.wav";
	else if (sound < 2) m_szAltFireSound = "Sounds\\Slash2.wav";
	else if (sound < 3)	m_szAltFireSound = "Sounds\\Slash3.wav";
	else				m_szAltFireSound = "Sounds\\Slash4.wav";

	return CWeapon::Fire();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSoulPipe::Fire()
//
//	PURPOSE:	Fires the weapon
//
// ----------------------------------------------------------------------- //
DDWORD CSoulPipe::Fire()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE)
		return 0;

	DBOOL bAltFire = (m_eState == WS_ALT_FIRING);

	int sound = pServerDE->IntRandom(0, 4);

	if (sound < 1)		m_szAltFireSound = "Sounds\\Slash1.wav";
	else if (sound < 2) m_szAltFireSound = "Sounds\\Slash2.wav";
	else if (sound < 3)	m_szAltFireSound = "Sounds\\Slash3.wav";
	else				m_szAltFireSound = "Sounds\\Slash4.wav";

	return CWeapon::Fire();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSoulHook::Fire()
//
//	PURPOSE:	Fires the weapon
//
// ----------------------------------------------------------------------- //

DDWORD CSoulHook::Fire()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE)
		return 0;

	int sound = pServerDE->IntRandom(0, 4);

	if (sound < 1)		m_szFireSound = "Sounds\\Slash1.wav";
	else if (sound < 2) m_szFireSound = "Sounds\\Slash2.wav";
	else if (sound < 3)	m_szFireSound = "Sounds\\Slash3.wav";
	else				m_szFireSound = "Sounds\\Slash4.wav";

	return CWeapon::Fire();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBehemothClaw::Fire()
//
//	PURPOSE:	Fires the weapon
//
// ----------------------------------------------------------------------- //
DDWORD CBehemothClaw::Fire()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE)
		return 0;

	int sound = pServerDE->IntRandom(0, 4);

	if (sound < 1)		m_szFireSound = "Sounds\\Slash1.wav";
	else if (sound < 2) m_szFireSound = "Sounds\\Slash2.wav";
	else if (sound < 3)	m_szFireSound = "Sounds\\Slash3.wav";
	else				m_szFireSound = "Sounds\\Slash4.wav";

	return CWeapon::Fire();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNightmareBite::Fire()
//
//	PURPOSE:	Fires the weapon
//
// ----------------------------------------------------------------------- //
DDWORD CNightmareBite::Fire()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE)
		return 0;

	int sound = pServerDE->IntRandom(0, 4);

	if (sound < 1)		m_szFireSound = "Sounds\\Slash1.wav";
	else if (sound < 2) m_szFireSound = "Sounds\\Slash2.wav";
	else if (sound < 3)	m_szFireSound = "Sounds\\Slash3.wav";
	else				m_szFireSound = "Sounds\\Slash4.wav";

	return CWeapon::Fire();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CThiefSuck::Fire()
//
//	PURPOSE:	Fires the weapon
//
// ----------------------------------------------------------------------- //
DDWORD CThiefSuck::Fire()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE)
		return 0;

	DBOOL bAltFire = (m_eState == WS_ALT_FIRING);

	int sound = pServerDE->IntRandom(0, 4);

	if (sound < 1)		m_szFireSound = "Sounds\\Slash1.wav";
	else if (sound < 2) m_szFireSound = "Sounds\\Slash2.wav";
	else if (sound < 3)	m_szFireSound = "Sounds\\Slash3.wav";
	else				m_szFireSound = "Sounds\\Slash4.wav";

	return CWeapon::Fire();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBoneleechSuck::Fire()
//
//	PURPOSE:	Fires the weapon
//
// ----------------------------------------------------------------------- //
DDWORD CBoneleechSuck::Fire()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE)
		return 0;

	int sound = pServerDE->IntRandom(0, 4);

	if (sound < 1)		m_szFireSound = "Sounds\\Slash1.wav";
	else if (sound < 2) m_szFireSound = "Sounds\\Slash2.wav";
	else if (sound < 3)	m_szFireSound = "Sounds\\Slash3.wav";
	else				m_szFireSound = "Sounds\\Slash4.wav";

	return CWeapon::Fire();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CEnergyBlast::Fire()
//
//	PURPOSE:	Fires the weapon
//
// ----------------------------------------------------------------------- //
DDWORD CEnergyBlast::Fire()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE)
		return 0;

	HCLASS hObjClass = pServerDE->GetObjectClass(m_hOwner);

	if(pServerDE->IsKindOf(hObjClass, pServerDE->GetClass("Gideon")))
		m_szFireSound = "sounds\\enemies\\gideon\\distfire.wav";

	DBOOL bAltFire = (m_eState == WS_ALT_FIRING);

	return CWeapon::Fire();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CEnergyBlast::FireProjectile()
//
//	PURPOSE:	Fires the weapon
//
// ----------------------------------------------------------------------- //
CProjectile *CEnergyBlast::FireProjectile(DVector *vFire, DFLOAT dist, DBOOL bAltFire)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	CProjectile *pProject = CWeapon::FireProjectile(vFire, dist, bAltFire);

	DDWORD dwFlags = pServerDE->GetObjectFlags(pProject->m_hObject);
	dwFlags &= ~FLAG_GRAVITY;
	pProject->SetFlags(dwFlags);

	return pProject;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGroundStrike::Fire()
//
//	PURPOSE:	Fires the weapon
//
// ----------------------------------------------------------------------- //
DDWORD CGroundStrike::Fire()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE)
		return 0;

	DBOOL bAltFire = (m_eState == WS_ALT_FIRING);

	return CWeapon::Fire();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGroundStrikeBlast::FireProjectile()
//
//	PURPOSE:	Fires the weapon
//
// ----------------------------------------------------------------------- //
CProjectile *CGroundStrike::FireProjectile(DVector *vFire, DFLOAT dist, DBOOL bAltFire)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	CProjectile *pProject = CWeapon::FireProjectile(vFire, dist, bAltFire);

	DDWORD dwFlags = pServerDE->GetObjectFlags(pProject->m_hObject);
	dwFlags &= ~FLAG_GRAVITY;
	pProject->SetFlags(dwFlags);

	return pProject;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFireball::Fire()
//
//	PURPOSE:	Fires the weapon
//
// ----------------------------------------------------------------------- //
DDWORD CFireball::Fire()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE)
		return 0;

	DBOOL bAltFire = (m_eState == WS_ALT_FIRING);

	return CWeapon::Fire();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFireball::FireProjectile()
//
//	PURPOSE:	Fires the weapon
//
// ----------------------------------------------------------------------- //
CProjectile *CFireball::FireProjectile(DVector *vFire, DFLOAT dist, DBOOL bAltFire)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	CProjectile *pProject = CWeapon::FireProjectile(vFire, dist, bAltFire);

	DDWORD dwFlags = pServerDE->GetObjectFlags(pProject->m_hObject);
	dwFlags &= ~FLAG_GRAVITY;
	pProject->SetFlags(dwFlags);

	return pProject;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CShockwave::Fire()
//
//	PURPOSE:	Fires the weapon
//
// ----------------------------------------------------------------------- //
DDWORD CShockwave::Fire()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE)
		return 0;

	DBOOL bAltFire = (m_eState == WS_ALT_FIRING);

	return CWeapon::Fire();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CShockwave::FireProjectile()
//
//	PURPOSE:	Fires the weapon
//
// ----------------------------------------------------------------------- //
CProjectile *CShockwave::FireProjectile(DVector *vFire, DFLOAT dist, DBOOL bAltFire)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	CProjectile *pProject = CWeapon::FireProjectile(vFire, dist, bAltFire);

	DDWORD dwFlags = pServerDE->GetObjectFlags(pProject->m_hObject);
	dwFlags &= ~FLAG_GRAVITY;
	pProject->SetFlags(dwFlags);

	return pProject;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBehemothShockwave::Fire()
//
//	PURPOSE:	Fires the weapon
//
// ----------------------------------------------------------------------- //
DDWORD CBehemothShockwave::Fire()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE)
		return 0;

	DBOOL bAltFire = (m_eState == WS_ALT_FIRING);

	return CWeapon::Fire();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBehemothShockwave::FireProjectile()
//
//	PURPOSE:	Fires the weapon
//
// ----------------------------------------------------------------------- //
CProjectile *CBehemothShockwave::FireProjectile(DVector *vFire, DFLOAT dist, DBOOL bAltFire)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	CProjectile *pProject = CWeapon::FireProjectile(vFire, dist, bAltFire);

	DDWORD dwFlags = pServerDE->GetObjectFlags(pProject->m_hObject);
	dwFlags &= ~FLAG_GRAVITY;
	pProject->SetFlags(dwFlags);

	return pProject;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CHandSqueeze::Fire()
//
//	PURPOSE:	Fires the weapon
//
// ----------------------------------------------------------------------- //
DDWORD CHandSqueeze::Fire()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE)
		return 0;

	DBOOL bAltFire = (m_eState == WS_ALT_FIRING);

	int sound = pServerDE->IntRandom(0, 4);

	if (sound < 1)		m_szFireSound = "Sounds\\Slash1.wav";
	else if (sound < 2) m_szFireSound = "Sounds\\Slash2.wav";
	else if (sound < 3)	m_szFireSound = "Sounds\\Slash3.wav";
	else				m_szFireSound = "Sounds\\Slash4.wav";

	return CWeapon::Fire();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFlareGun::FireSpread()
//
//	PURPOSE:	Spread the flare shots
//
// ----------------------------------------------------------------------- //

void CFlareGun::FireSpread(Spread *spread, DDWORD shots, DFLOAT range, DBOOL bAltFire, DVector *rDir)
{
	//***** Make sure the server and inventory systems are valid *****//
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !m_pInventoryMgr)		return;

	//***** Variables to calculate the spread of the weapon fire *****//
	DFLOAT		hOffset, vOffset;
	DVector		vTmp, vFire;
	DVector		vU, vR, vF;
	DBOOL		bTracer = DFALSE;
	pServerDE->GetRotationVectors(&m_rRotation, &vU, &vR, &vF); 

	// Check to see if the player is standing too close to something, and we need
	// to move the firing position back in order to hit it
	if(IsPlayer(m_hOwner) && FiringTooClose(&vF, 60.0f, &vTmp))
		{ VEC_COPY(m_vPosition, vTmp); }

	for(int i = (shots - 1); i >= 0; i--)
	{
		VEC_COPY(vFire, vF)
        
		if(shots - 1)
			switch(i)
			{
				case	7:	hOffset = 0.03f; vOffset = 0.03f;	break;
				case	6:	hOffset = -0.03f; vOffset = -0.03f;	break;
				case	5:	hOffset = -0.03f; vOffset = 0.03f;	break;
				case	4:	hOffset = 0.03f; vOffset = -0.03f;	break;
				case	3:	hOffset = 0.0f; vOffset = 0.08f;	break;
				case	2:	hOffset = 0.0f; vOffset = -0.08f;	break;
				case	1:	hOffset = -0.08f; vOffset = 0.0f;	break;
				case	0:	hOffset = 0.08f; vOffset = 0.0f;	break;
				default:	hOffset = 0.0f; vOffset = 0.0f;		break;
			}
		else
		{
			hOffset = pServerDE->Random(-spread->h, spread->h) / 2000;
			vOffset = pServerDE->Random(-spread->v, spread->v) / 2000;
		}

		VEC_MULSCALAR(vTmp, vR, hOffset)
		VEC_ADD(vFire, vFire, vTmp)

		VEC_MULSCALAR(vTmp, vU, vOffset)
		VEC_ADD(vFire, vFire, vTmp)

		VEC_NORM(vFire)

		if(m_bAccuracyCheck && IsPlayer(m_hOwner))
			AlignFireVector(&vFire, range);

		FireProjectile(&vFire, range, (m_eState == WS_ALT_FIRING));
	}

	VEC_COPY(*rDir, vR);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVoodooDoll::FireVector()
//
//	PURPOSE:	Fires a vector
//
// ----------------------------------------------------------------------- //

DBOOL CVoodooDoll::FireVector(DVector *vFire, DFLOAT dist, DBOOL bTracer)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if(!pServerDE)	return DFALSE;

	switch(m_nHitFX)
	{
		case	0:		m_nHitType = DAMAGE_TYPE_NORMAL;		break;
		case	1:		m_nHitType = DAMAGE_TYPE_NORMAL;		break;
		case	2:		m_nHitType = DAMAGE_TYPE_BLIND;			break;
		case	3:		m_nHitType = DAMAGE_TYPE_DROPWEAPON;	break;
		case	4:		m_nHitType = DAMAGE_TYPE_SLOW;			break;
		default:		m_nHitType = DAMAGE_TYPE_NORMAL;		break;
	}

	if(m_eState == WS_FIRING)
	{
		DamageClosestInFOV();
	}
	else if(m_eState == WS_ALT_FIRING)
	{
		m_nHitFX = 5;
		m_nHitType = DAMAGE_TYPE_VISION;	// Wonkie vision (violet)

		m_fDamage = pServerDE->Random(m_fMinAltDamage, m_fMaxAltDamage);

		if (!WonkyThemAll(WONKY_VISION_DEFAULT_TIME))
		{
			// If you miss everyone with the alt fire, you just explode cause you suck
			BaseClass *ffObj = pServerDE->HandleToObject(m_hOwner);
			DVector		vFire, vPos;

			VEC_SET(vFire, 0.0f, -1.0f, 0.0f);
			pServerDE->GetObjectPos(m_hOwner, &vPos);

			HMESSAGEWRITE hMsg = pServerDE->StartMessageToObject(ffObj, m_hOwner, MID_DAMAGE);
			pServerDE->WriteToMessageVector(hMsg, &vFire);
			pServerDE->WriteToMessageFloat(hMsg, 100.0f);
			pServerDE->WriteToMessageByte(hMsg, DAMAGE_TYPE_EXPLODE);
			pServerDE->WriteToMessageObject(hMsg, m_hOwner);
			pServerDE->WriteToMessageVector(hMsg, &vPos);
			pServerDE->EndMessage2(hMsg, MESSAGE_GUARANTEED | MESSAGE_NAGGLEFAST);
		}
	}

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVoodooDoll::SendDamageMsg()
//
//	PURPOSE:    VoodooDoll Damage Message
//
// ----------------------------------------------------------------------- //

DBOOL CVoodooDoll::SendDamageMsg(HOBJECT firedTo, DVector vPoint, int nNode, DVector *vFire, DFLOAT m_nDamage)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();

	if (!pServerDE)
		return DFALSE;

	BaseClass *ffObj = pServerDE->HandleToObject(m_hOwner);

	// Send a damage message to the object
	HMESSAGEWRITE hMsg = pServerDE->StartMessageToObject(ffObj, firedTo, MID_DAMAGE);
	pServerDE->WriteToMessageVector(hMsg, vFire);
    pServerDE->WriteToMessageFloat(hMsg, (float)m_nDamage * GetDamageMultiplier());
    pServerDE->WriteToMessageByte(hMsg, (char)m_nHitType);
	pServerDE->WriteToMessageObject(hMsg, m_hOwner);
	pServerDE->WriteToMessageVector(hMsg, &vPoint);
	pServerDE->EndMessage2(hMsg, MESSAGE_GUARANTEED | MESSAGE_NAGGLEFAST);

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVoodooDoll::DamageClosestInFOV()
//
//	PURPOSE:    Search for the closest person to damage
//
// ----------------------------------------------------------------------- //

DBOOL CVoodooDoll::DamageClosestInFOV()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE)	return DNULL;

	// Get the head of the list of destructable objects
	DLink*	pLink = CDestructable::m_DestructableHead.m_pNext;
	if(!pLink || (pLink == &CDestructable::m_DestructableHead))	return DNULL;

	// Work with the squares of the distances so as to not have to get a square root.
	HOBJECT		hObj;
	HOBJECT		hDamageObj = DNULL;
	DFLOAT		fLastDist = 3000.0f;
	DVector		vPoint, vDirection;

	HCLASS		hCharacter	= pServerDE->GetClass("CBaseCharacter");
	HCLASS		hPlayer		= pServerDE->GetClass("CPlayerObj");
	HCLASS		hObjClass   = DNULL;

	DBOOL		bHitSomething = DFALSE;

	// Get a forward vector and position of the player who shot the voodoo doll
   	CPlayerObj	*pObj = (CPlayerObj*)pServerDE->HandleToObject(m_hOwner);
	DVector		vOwnerF, vOwnerPos;
	pObj->GetPlayerForward(&vOwnerF);
	pServerDE->GetObjectPos(m_hOwner, &vOwnerPos);
	VEC_NORM(vOwnerF);

	CPlayerObj* pOwnerPlayer = pObj;

	// Go through the list of destructable objects and look for people within your FOV
	while(pLink != &CDestructable::m_DestructableHead)
	{
		hObj = ((CDestructable*)pLink->m_pData)->GetObject();
		if(!hObj)	break;
		hObjClass = pServerDE->GetObjectClass(hObj);

		if(pServerDE->IsKindOf(hObjClass, hCharacter))
		{
			DVector		vObjPos, vDir;
			DFLOAT		fDist, fAngle;

			// If it's us... skip to the next in the list
			if(hObj == m_hOwner)
				{ pLink = pLink->m_pNext; continue; }

			// Test the angle between this object and us
			pServerDE->GetObjectPos(hObj, &vObjPos);
			VEC_SUB(vDir, vObjPos, vOwnerPos);
			fDist = VEC_MAG(vDir);

			// If the target is too far away... skip it
			if(fDist > VOODOO_WONKY_DIST)
				{ pLink = pLink->m_pNext; continue; }

			VEC_NORM(vDir);
			fAngle = VEC_DOT(vDir, vOwnerF);

			// If the target isn't within our square FOV, then skip it
			if(fAngle < VOODOO_PRIMARY_FOV)
				{ pLink = pLink->m_pNext; continue; }

			// Make sure there's nothing blocking our view of the character
			IntersectQuery iq;
			IntersectInfo  ii;
    
			VEC_COPY(iq.m_From, vOwnerPos);
			VEC_COPY(iq.m_To, vObjPos);
			VEC_MULSCALAR(vDir, vDir, 15.0f);
			VEC_ADD(iq.m_To, iq.m_To, vDir);
    
			iq.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID;
			iq.m_FilterFn = LiquidFilterFn;
			iq.m_pUserData = NULL;	

			if(pServerDE->IntersectSegment(&iq, &ii))
			{
				if(ii.m_hObject == hObj)
				{
					if(fDist < fLastDist)
					{
						fLastDist = fDist;
						hDamageObj = hObj;
						bHitSomething = DTRUE;
						VEC_COPY(vPoint, ii.m_Point);
						VEC_COPY(vDirection, vDir);
					}
				}
			}
		}

		pLink = pLink->m_pNext;
	}

	// Handle the object damage and effect

	if(hDamageObj)
	{
		hObjClass = pServerDE->GetObjectClass(hDamageObj);

		if(pServerDE->IsKindOf(hObjClass, hPlayer))
		{
    		CPlayerObj *pObj = (CPlayerObj*)pServerDE->HandleToObject(hDamageObj);

			if (!pOwnerPlayer->IsPlayerDamageOk(pObj))	// [blg] check for team-play and friendly-fire
			{
				return(DTRUE);
			}

			if(m_nHitFX == 2)
			{
				HMESSAGEWRITE hMsg = pServerDE->StartMessage(pObj->GetClient(), SMSG_BLIND);
				pServerDE->WriteToMessageFloat(hMsg, 5.0f);
				pServerDE->EndMessage2(hMsg, MESSAGE_GUARANTEED | MESSAGE_NAGGLEFAST);
			}
			if(m_nHitFX == 3)
			{
				// Make the character holster his weapon
			}
			if(m_nHitFX == 4)
			{
				pObj->Slow(5.0f);
			}
		}

		//******************************************************************
		if(m_nHitFX == 1)
			SendDamageMsg(hDamageObj, vPoint, 999, &vDirection, m_fDamage * 2.0f);
		else
			SendDamageMsg(hDamageObj, vPoint, 999, &vDirection, m_fDamage);

		//******************************************************************
		// Create a particle effect around the hit character
		DVector		offset;
		DDWORD		fxType = OBJFX_VOODOO_1 + m_nHitFX;
		VEC_SET(offset, 0.0f, 0.0f, 0.0f);

		HMESSAGEWRITE hMessage = pServerDE->StartInstantSpecialEffectMessage(&offset);
		pServerDE->WriteToMessageByte(hMessage, SFX_OBJECTFX_ID);

		pServerDE->WriteToMessageObject(hMessage, hDamageObj);
		pServerDE->WriteToMessageVector(hMessage, &offset);
		pServerDE->WriteToMessageFloat(hMessage, 0.0f);
		pServerDE->WriteToMessageDWord(hMessage, 0);
		pServerDE->WriteToMessageDWord(hMessage, fxType);
		pServerDE->WriteToMessageDWord(hMessage, 0);

		pServerDE->EndMessage2(hMessage, MESSAGE_GUARANTEED | MESSAGE_NAGGLEFAST);
		//******************************************************************

		if(m_nHitFX == 4)
			pServerDE->SetVelocity(hDamageObj, &offset);
	}
	else
	{
		SendDamageMsg(m_hOwner, vOwnerPos, 999, &vDirection, m_fDamage / 3.0f);
	}

	return bHitSomething;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVoodooDoll::WonkyThemAll()
//
//	PURPOSE:    Search for people to wonky
//
// ----------------------------------------------------------------------- //

DBOOL CVoodooDoll::WonkyThemAll(DFLOAT fWonkyTime)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE)	return DNULL;

	// Get the head of the list of destructable objects
	DLink*	pLink = CDestructable::m_DestructableHead.m_pNext;
	if(!pLink || (pLink == &CDestructable::m_DestructableHead))	return DNULL;

	// Work with the squares of the distances so as to not have to get a square root.
	HOBJECT		hObj;

	HCLASS		hCharacter	= pServerDE->GetClass("CBaseCharacter");
	HCLASS		hPlayer		= pServerDE->GetClass("CPlayerObj");
	HCLASS		hObjClass   = DNULL;

	DBOOL		bHitSomething = DFALSE;

	// Get a forward vector and position of the player who shot the voodoo doll
   	CPlayerObj	*pObj = (CPlayerObj*)pServerDE->HandleToObject(m_hOwner);
	DVector		vOwnerF, vOwnerPos;
	pObj->GetPlayerForward(&vOwnerF);
	pServerDE->GetObjectPos(m_hOwner, &vOwnerPos);
	VEC_NORM(vOwnerF);

	CPlayerObj* pOwnerPlayer = pObj;

	// Go through the list of destructable objects and look for people within your FOV
	while(pLink != &CDestructable::m_DestructableHead)
	{
		hObj = ((CDestructable*)pLink->m_pData)->GetObject();
		if(!hObj)	break;
		hObjClass = pServerDE->GetObjectClass(hObj);

		if(pServerDE->IsKindOf(hObjClass, hCharacter))
		{
			DVector		vObjPos, vDir;
			DFLOAT		fDist, fAngle;

			// If it's us... skip to the next in the list
			if(hObj == m_hOwner)
				{ pLink = pLink->m_pNext; continue; }

			// Test the angle between this object and us
			pServerDE->GetObjectPos(hObj, &vObjPos);
			VEC_SUB(vDir, vObjPos, vOwnerPos);
			fDist = VEC_MAG(vDir);

			// If the target is too far away... skip it
			if(fDist > VOODOO_WONKY_DIST)
				{ pLink = pLink->m_pNext; continue; }

			VEC_NORM(vDir);
			fAngle = VEC_DOT(vDir, vOwnerF);

			// If the target isn't within our square FOV, then skip it
			if(fAngle < VOODOO_WONKY_FOV)
				{ pLink = pLink->m_pNext; continue; }

			// If it's a team-mate and there's no friendly-fire, then skip it
			if (!pOwnerPlayer->IsPlayerDamageOk(hObj))
			{
				pLink = pLink->m_pNext;
				bHitSomething = DTRUE;
				continue;
			}

			// Make sure there's nothing blocking our view of the character
			IntersectQuery iq;
			IntersectInfo  ii;
    
			VEC_COPY(iq.m_From, vOwnerPos);
			VEC_COPY(iq.m_To, vObjPos);
			VEC_MULSCALAR(vDir, vDir, 15.0f);
			VEC_ADD(iq.m_To, iq.m_To, vDir);
    
			iq.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID;
			iq.m_FilterFn = LiquidFilterFn;
			iq.m_pUserData = NULL;	

			if(pServerDE->IntersectSegment(&iq, &ii))
			{
				if(ii.m_hObject == hObj)
				{
					bHitSomething = DTRUE;
					VEC_NORM(vDir);

					SendDamageMsg(ii.m_hObject, ii.m_Point, 999, &vDir, m_fDamage);

					//******************************************************************
					// Create a particle effect around the hit character
					DVector		offset;
					VEC_SET(offset, 0.0f, 0.0f, 0.0f);

					HMESSAGEWRITE hMessage = pServerDE->StartInstantSpecialEffectMessage(&offset);
					pServerDE->WriteToMessageByte(hMessage, SFX_OBJECTFX_ID);

					pServerDE->WriteToMessageObject(hMessage, ii.m_hObject);
					pServerDE->WriteToMessageVector(hMessage, &offset);
					pServerDE->WriteToMessageFloat(hMessage, 0.0f);
					pServerDE->WriteToMessageDWord(hMessage, 0);
					pServerDE->WriteToMessageDWord(hMessage, OBJFX_VOODOO_6);
					pServerDE->WriteToMessageDWord(hMessage, 0);

					pServerDE->EndMessage2(hMessage, MESSAGE_GUARANTEED | MESSAGE_NAGGLEFAST);
					//******************************************************************

					if(pServerDE->IsKindOf(hObjClass, hPlayer))
					{
		    			CPlayerObj *pHit = (CPlayerObj*)pServerDE->HandleToObject(ii.m_hObject);

						HMESSAGEWRITE hMsg = pServerDE->StartMessage(pHit->GetClient(), SMSG_WONKYVISION);
						pServerDE->WriteToMessageFloat(hMsg, fWonkyTime);
						pServerDE->WriteToMessageByte(hMsg, 0);	// bNoMove = DFALSE
						pServerDE->EndMessage2(hMsg, MESSAGE_GUARANTEED | MESSAGE_NAGGLE);
					}
				}
			}
		}

		pLink = pLink->m_pNext;
	}

	return bHitSomething;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CShotgun::Fire()
//
//	PURPOSE:	Fires the shotgun
//
// ----------------------------------------------------------------------- //

DDWORD CShotgun::Fire()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE)
		return DFALSE;

	if (m_eState == WS_ALT_FIRING)
		m_bEjectShell = DTRUE;
	else
		m_bEjectShell = DFALSE;

	return CWeapon::Fire();
/*
	DVector vU, vR, vF;

	pServerDE->GetRotationVectors(&m_rRotation, &vU, &vR, &vF); 

	m_bPlayImpactSound = DFALSE;

	m_bEjectShell = DFALSE;

	// Fire two barrels
	if ((m_eState == WS_ALT_FIRING) && m_dwCurBarrel == BARREL_1)
	{
		DFLOAT newLastShotTime;
		DFLOAT oldLastShotTime = m_fLastShotTime;

		DDWORD dwRet = CWeapon::Fire();
		if (dwRet)
		{
			newLastShotTime = m_fLastShotTime;
			m_fLastShotTime = oldLastShotTime;

			m_bEjectShell = DTRUE;
			// Fire the 2nd barrel.
			dwRet = CWeapon::Fire();
			if (!dwRet)  // 2nd shot didn't go for some reason
			{
				m_fLastShotTime = newLastShotTime;
			}
			else
			{
				VEC_ADD(vR, vU, vR);
			}

			dwRet = 2;
			m_bLastFireAlt = DTRUE;
			m_dwCurBarrel = BARREL_1;
		}
		return dwRet;
	}
	else // Fire only one barrel
	{
		if (m_dwCurBarrel == BARREL_2)
			m_bEjectShell = DTRUE;

		DDWORD dwRet = CWeapon::Fire();
		if (dwRet)
		{
			if (m_dwCurBarrel == BARREL_2)
			{
				m_bLastFireAlt = DTRUE;
				m_dwCurBarrel = BARREL_1;

				VEC_ADD(vR, vU, vR);
			}
			else
			{
				m_bLastFireAlt = DFALSE;
				m_dwCurBarrel = BARREL_2;
			}
			return 1;
		}
		else
			return 0;
	}
*/
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRGLauncher::FireProjectile()
//
//	PURPOSE:	Fires a projectile for the RGLauncher
//
// ----------------------------------------------------------------------- //

CProjectile* CNapalmCannon::FireProjectile(DVector *vFire, DFLOAT dist, DBOOL bAltFire)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	CProjectile *pProject = CWeapon::FireProjectile(vFire, dist, bAltFire);

	// Primary fire has rocket projectiles, so turn off gravity
	if (!bAltFire)
	{
		DDWORD dwFlags = pServerDE->GetObjectFlags(pProject->m_hObject);
		dwFlags &= ~FLAG_GRAVITY;
		pProject->SetFlags(dwFlags);
	}
	return pProject;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDeathRay::CDeathRay()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CDeathRay::CDeathRay() : CWeapon(WEAP_DEATHRAY)
{
	m_nFlags = FLAG_ENVIRONMENTMAP;
	m_fChromeValue = 0.05f;
	m_fLastDamageTime = 0.0f;
	m_pRotModel = DNULL;

	ObjectCreateStruct ocStruct;
	INIT_OBJECTCREATESTRUCT(ocStruct);

	ocStruct.m_ObjectType = OT_NORMAL; 
	ocStruct.m_Flags = FLAG_FORCECLIENTUPDATE;

	HCLASS hClass = g_pServerDE->GetClass("BaseClass");
	m_pRotModel = (BaseClass*)g_pServerDE->CreateObject(hClass, &ocStruct);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDeathRay::~CDeathRay()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CDeathRay::~CDeathRay()
{
	if(m_pRotModel)
	{
		delete m_pRotModel;
		m_pRotModel = DNULL;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDeathRay::Fire()
//
//	PURPOSE:	Fires the rings o' death
//
// ----------------------------------------------------------------------- //

DDWORD CDeathRay::Fire()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if(!pServerDE)	return 0;

	if(m_eState == WS_ALT_FIRING)
	{
		DVector		vPos;
		DRotation	rRot;

		pServerDE->GetObjectPos(m_hOwner, &vPos);

		if(IsOwnerAPlayer())
		{
   			CPlayerObj	*pObj = (CPlayerObj*)pServerDE->HandleToObject(m_hOwner);
			DFLOAT		fEyeLevel = pObj->GetEyeLevel();
			vPos.y += fEyeLevel;
			pObj->GetPlayerRotation(&rRot);
		}
		else
			pServerDE->GetObjectRotation(m_hOwner, &rRot);

		pServerDE->SetObjectPos(m_pRotModel->m_hObject, &vPos);
		pServerDE->SetObjectRotation(m_pRotModel->m_hObject, &rRot);

		// Create a client side effect around the projectile
		DVector		offset;
		VEC_SET(offset, m_vFlash.y, m_vFlash.x, m_vFlash.z);

		HMESSAGEWRITE hMessage = pServerDE->StartInstantSpecialEffectMessage(&offset);
		pServerDE->WriteToMessageByte(hMessage, SFX_OBJECTFX_ID);

		pServerDE->WriteToMessageObject(hMessage, m_pRotModel->m_hObject);
		pServerDE->WriteToMessageVector(hMessage, &offset);
		pServerDE->WriteToMessageFloat(hMessage, 0.0f);
		pServerDE->WriteToMessageDWord(hMessage, 0);
		pServerDE->WriteToMessageDWord(hMessage, OBJFX_DEATHRAY_RING_1);
		pServerDE->WriteToMessageDWord(hMessage, 0);

		pServerDE->EndMessage2(hMessage, MESSAGE_GUARANTEED | MESSAGE_NAGGLEFAST);

		hMessage = pServerDE->StartInstantSpecialEffectMessage(&offset);
		pServerDE->WriteToMessageByte(hMessage, SFX_OBJECTFX_ID);

		pServerDE->WriteToMessageObject(hMessage, m_pRotModel->m_hObject);
		pServerDE->WriteToMessageVector(hMessage, &offset);
		pServerDE->WriteToMessageFloat(hMessage, 0.0f);
		pServerDE->WriteToMessageDWord(hMessage, 0);
		pServerDE->WriteToMessageDWord(hMessage, OBJFX_DEATHRAY_RING_2);
		pServerDE->WriteToMessageDWord(hMessage, 0);

		pServerDE->EndMessage2(hMessage, MESSAGE_GUARANTEED | MESSAGE_NAGGLEFAST);

		DamageObjectsInFOV(DEATHRAY_BEAM_DIST, DEATHRAY_BEAM_FOV);
	}

	return CWeapon::Fire();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDeathRay::DamageObjectsInFOV()
//
//	PURPOSE:	Damages every destructable object in the FOV at a short range
//
// ----------------------------------------------------------------------- //

void CDeathRay::DamageObjectsInFOV(DFLOAT fRange, DFLOAT fFOV)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE)	return;

	// Get the head of the list of destructable objects
	DLink*	pLink = CDestructable::m_DestructableHead.m_pNext;
	if(!pLink || (pLink == &CDestructable::m_DestructableHead))	return;

	HOBJECT		hObj;

	// Get a forward vector and position of the player who shot the death ray
   	CPlayerObj	*pObj = (CPlayerObj*)pServerDE->HandleToObject(m_hOwner);
	DVector		vOwnerF, vOwnerPos;
	pObj->GetPlayerForward(&vOwnerF);
	pServerDE->GetObjectPos(m_hOwner, &vOwnerPos);
	VEC_NORM(vOwnerF);

	// Go through the list of destructable objects and look for people within your FOV
	while(pLink != &CDestructable::m_DestructableHead)
	{
		hObj = ((CDestructable*)pLink->m_pData)->GetObject();
		if(!hObj)	break;

		DVector		vObjPos, vDir;
		DFLOAT		fDist, fAngle;

		// If it's us... skip to the next in the list
		if(hObj == m_hOwner)
			{ pLink = pLink->m_pNext; continue; }

		// Test the angle between this object and us
		pServerDE->GetObjectPos(hObj, &vObjPos);
		VEC_SUB(vDir, vObjPos, vOwnerPos);
		fDist = VEC_MAG(vDir);

		// If the target is too far away... skip it
		if(fDist > fRange)
			{ pLink = pLink->m_pNext; continue; }

		VEC_NORM(vDir);
		fAngle = VEC_DOT(vDir, vOwnerF);

		// If the target isn't within our square FOV, then skip it
		if(fAngle < fFOV)
			{ pLink = pLink->m_pNext; continue; }

		// Make sure there's nothing blocking our view of the character
		IntersectQuery iq;
		IntersectInfo  ii;

		VEC_COPY(iq.m_From, vOwnerPos);
		VEC_COPY(iq.m_To, vObjPos);
		VEC_MULSCALAR(vDir, vDir, 15.0f);
		VEC_ADD(iq.m_To, iq.m_To, vDir);

		iq.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID;
		iq.m_FilterFn = LiquidFilterFn;
		iq.m_pUserData = NULL;	

		if(pServerDE->IntersectSegment(&iq, &ii))
		{
			if(ii.m_hObject == hObj)
			{
				VEC_NORM(vDir);

				m_nDamageType = DAMAGE_TYPE_ELECTRIC;
				SendDamageMsg(ii.m_hObject, ii.m_Point, &vDir, m_fDamage);

				//******************************************************************
				// Create a particle effect around the hit character
				DVector		offset;
				VEC_SET(offset, 0.0f, 0.0f, 0.0f);

				HMESSAGEWRITE hMessage = pServerDE->StartInstantSpecialEffectMessage(&offset);
				pServerDE->WriteToMessageByte(hMessage, SFX_OBJECTFX_ID);

				pServerDE->WriteToMessageObject(hMessage, ii.m_hObject);
				pServerDE->WriteToMessageVector(hMessage, &offset);
				pServerDE->WriteToMessageFloat(hMessage, 0.0f);
				pServerDE->WriteToMessageDWord(hMessage, 0);
				pServerDE->WriteToMessageDWord(hMessage, OBJFX_ELECTRIC_1);
				pServerDE->WriteToMessageDWord(hMessage, 0);

				pServerDE->EndMessage2(hMessage, MESSAGE_GUARANTEED | MESSAGE_NAGGLEFAST);
				//******************************************************************
			}
		}

		pLink = pLink->m_pNext;
	}
}

#ifdef _ADD_ON

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFlayer::FireProjectile()
//
//	PURPOSE:	Fires the weapon
//
// ----------------------------------------------------------------------- //

CProjectile* CFlayer::FireProjectile(DVector *vFire, DFLOAT dist, DBOOL bAltFire)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE)		return 0;

/*	DVector		vNormal;
	DDWORD		dwExpType = EXP_FLAYER_PRIMARY;

	VEC_SET(vNormal, 0.0f, 1.0f, 0.0f);

	if(bAltFire)
		dwExpType = EXP_FLAYER_ALT;

	// Create a little explosion effect at the source
	HMESSAGEWRITE hMessage = pServerDE->StartInstantSpecialEffectMessage(&m_vPosition);
	pServerDE->WriteToMessageByte(hMessage, SFX_EXPLOSIONFX_ID);

	pServerDE->WriteToMessageVector(hMessage, &m_vPosition);
	pServerDE->WriteToMessageVector(hMessage, &vNormal);
	pServerDE->WriteToMessageDWord(hMessage, dwExpType);

	pServerDE->EndMessage2(hMessage, MESSAGE_GUARANTEED | MESSAGE_NAGGLEFAST);
*/
	return CWeapon::FireProjectile(vFire, dist, bAltFire);
}


#endif

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTeslaCannon::Fire()
//
//	PURPOSE:	Fires the weapon
//
// ----------------------------------------------------------------------- //

DDWORD CTeslaCannon::Fire()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE)		return 0;

	if(m_eState == WS_ALT_FIRING)
	{
		numAltFires++;

		if(numAltFires == 2)
			m_szAltProjectileClass = "CTeslaBallProjectile";
		else
			m_szAltProjectileClass = "CTeslaBoltProjectile";
	}

	return CWeapon::Fire();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTeslaCannon::FireProjectile()
//
//	PURPOSE:	Fires the weapon
//
// ----------------------------------------------------------------------- //

CProjectile* CTeslaCannon::FireProjectile(DVector *vFire, DFLOAT dist, DBOOL bAltFire)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE)		return 0;

	if(bAltFire)
	{
		IntersectQuery	iq;
		IntersectInfo	ii;
		DVector			vDist, vDest;

		iq.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID;
		iq.m_FilterFn = DNULL;
		iq.m_pUserData = DNULL;

		VEC_COPY(iq.m_From, m_vPosition);
		VEC_COPY(iq.m_To, m_vPosition);
		VEC_MULSCALAR(vDist, *vFire, dist)
		VEC_ADD(iq.m_To, iq.m_To, vDist);

		if(pServerDE->IntersectSegment(&iq, &ii))
			{ VEC_COPY(vDest, ii.m_Point); }
		else
			{ VEC_COPY(vDest, iq.m_To); }

		// Create the client side lightning effect
		HMESSAGEWRITE hMessage = pServerDE->StartInstantSpecialEffectMessage(&(iq.m_From));
		pServerDE->WriteToMessageByte(hMessage, SFX_LIGHTNING_ID);

		pServerDE->WriteToMessageVector(hMessage, &(iq.m_From));
		pServerDE->WriteToMessageVector(hMessage, &vDest);
		pServerDE->WriteToMessageByte(hMessage, LIGHTNING_SHAPE_BOXED);
		pServerDE->WriteToMessageByte(hMessage, LIGHTNING_FORM_THIN2THIN);
		pServerDE->WriteToMessageByte(hMessage, LIGHTNING_TYPE_HIGHSEG);

		pServerDE->EndMessage2(hMessage, MESSAGE_GUARANTEED | MESSAGE_NAGGLEFAST);
	}

	return CWeapon::FireProjectile(vFire, dist, bAltFire);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSingularity::FireProjectile()
//
//	PURPOSE:	Fires the weapon
//
// ----------------------------------------------------------------------- //
CProjectile* CSingularity::FireProjectile(DVector *vFire, DFLOAT dist, DBOOL bAltFire)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE)		return 0;

	IntersectQuery	iq;
	IntersectInfo	ii;
	DVector			vDist, vDest;

	iq.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID;
	iq.m_FilterFn = DNULL;
	iq.m_pUserData = DNULL;

	VEC_COPY(iq.m_From, m_vPosition);
	VEC_COPY(iq.m_To, m_vPosition);
	VEC_MULSCALAR(vDist, *vFire, dist)
	VEC_ADD(iq.m_To, iq.m_To, vDist);

	if(pServerDE->IntersectSegment(&iq, &ii))
		{ VEC_COPY(vDest, ii.m_Point); }
	else
		{ VEC_COPY(vDest, iq.m_To); }

	// Create the client side laser effect
	HMESSAGEWRITE hMessage = pServerDE->StartInstantSpecialEffectMessage(&(iq.m_From));
	pServerDE->WriteToMessageByte(hMessage, SFX_LASERBEAM_ID);

	pServerDE->WriteToMessageVector(hMessage, &m_vPosition);
	pServerDE->WriteToMessageVector(hMessage, &vDest);
	pServerDE->WriteToMessageByte(hMessage, LASER_BLUE_LARGE);

	pServerDE->EndMessage2(hMessage, MESSAGE_GUARANTEED | MESSAGE_NAGGLEFAST);

	return CWeapon::FireProjectile(vFire, dist, bAltFire);
}

#ifndef _DEMO

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapProximityBomb::FireProjectile()
//
//	PURPOSE:	Fires a projectile
//
// ----------------------------------------------------------------------- //

CProjectile* CWeapProximityBomb::FireProjectile(DVector *vFire, DFLOAT dist, DBOOL bAltFire)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE)	return DNULL;

	CProjectile *pProject = 0;

	// If we held onto the bomb too long and it should explode in our hand
	if(!m_fProjVelocity)
	{
		DVector vPos, vNormal;
		VEC_SET(vNormal, 0.0f, 1.0f, 0.0f);
		pServerDE->GetObjectPos(m_hOwner, &vPos);

		// Show a little explosion
		HMESSAGEWRITE hMessage = pServerDE->StartInstantSpecialEffectMessage(&vPos);
		pServerDE->WriteToMessageByte(hMessage, SFX_EXPLOSIONFX_ID);

		pServerDE->WriteToMessageVector(hMessage, &vPos);
		pServerDE->WriteToMessageVector(hMessage, &vNormal);
		pServerDE->WriteToMessageDWord(hMessage, EXP_DEFAULT_SMALL);

		pServerDE->EndMessage2(hMessage, MESSAGE_GUARANTEED | MESSAGE_NAGGLEFAST);

		// Play an explosion sound
		PlaySoundFromPos(&vPos, "Sounds\\Weapons\\time\\explode.wav", 1000.0f, SOUNDPRIORITY_MISC_MEDIUM);

		// Damage yourself
		SendDamageMsg(m_hOwner, vPos, &vNormal, m_fDamage);
		return DNULL;
	}

	if( bAltFire )
	{
		pServerDE->GetObjectPos(m_hOwner, &m_vPosition);

		vFire->x = 0.0f;
		vFire->y = -1.0f;
		vFire->z = 0.0f;
		pProject = CWeapon::FireProjectile(vFire, dist, DTRUE);
	}
	else
		pProject = CWeapon::FireProjectile(vFire, dist, DFALSE);

	if(m_pInventoryMgr && !m_pInventoryMgr->GetAmmoCount(GetAmmoType(bAltFire)))
		m_pInventoryMgr->SelectNextWeapon();

	return pProject;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapRemoteBomb::FireProjectile()
//
//	PURPOSE:	Fires a projectile
//
// ----------------------------------------------------------------------- //

CProjectile* CWeapRemoteBomb::FireProjectile(DVector *vFire, DFLOAT dist, DBOOL bAltFire)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	CProjectile *pProject = 0;

	// If we held onto the bomb too long and it should explode in our hand
/*	if(!m_fProjVelocity)
	{
		DVector vPos, vNormal;
		VEC_SET(vNormal, 0.0f, 1.0f, 0.0f);
		pServerDE->GetObjectPos(m_hOwner, &vPos);

		// Show a little explosion
		HMESSAGEWRITE hMessage = pServerDE->StartInstantSpecialEffectMessage(&vPos);
		pServerDE->WriteToMessageByte(hMessage, SFX_EXPLOSIONFX_ID);

		pServerDE->WriteToMessageVector(hMessage, &vPos);
		pServerDE->WriteToMessageVector(hMessage, &vNormal);
		pServerDE->WriteToMessageDWord(hMessage, EXP_DEFAULT_SMALL);

		pServerDE->EndMessage2(hMessage, MESSAGE_GUARANTEED | MESSAGE_NAGGLEFAST);

		// Play an explosion sound
		PlaySoundFromPos(&vPos, "Sounds\\Weapons\\time\\explode.wav", 1000.0f, SOUNDPRIORITY_MISC_MEDIUM);

		// Damage yourself
		SendDamageMsg(m_hOwner, vPos, &vNormal, m_fDamage);
		return DNULL;
	}
*/
	if( bAltFire )
	{
		pServerDE->GetObjectPos(m_hOwner, &m_vPosition);

		vFire->x = 0.0f;
		vFire->y = -1.0f;
		vFire->z = 0.0f;
		pProject = CWeapon::FireProjectile(vFire, dist, DTRUE);
	}
	else
		pProject = CWeapon::FireProjectile(vFire, dist, DFALSE);

	if(IsOwnerAPlayer())
	{
        CPlayerObj *pObj = (CPlayerObj*)pServerDE->HandleToObject(m_hOwner);
		if(pObj)
			pObj->AddRemoteBomb(pProject);
	}

	if(m_pInventoryMgr && !m_pInventoryMgr->GetAmmoCount(GetAmmoType(bAltFire)))
		m_pInventoryMgr->SelectNextWeapon();

	return pProject;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapTimeBomb::FireProjectile()
//
//	PURPOSE:	Fires a projectile
//
// ----------------------------------------------------------------------- //

CProjectile* CWeapTimeBomb::FireProjectile(DVector *vFire, DFLOAT dist, DBOOL bAltFire)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE)	return DNULL;

	CProjectile *pProject = 0;

	// If we held onto the bomb too long and it should explode in our hand
	if(!m_fProjVelocity)
	{
		DVector vPos, vNormal;
		VEC_SET(vNormal, 0.0f, 1.0f, 0.0f);
		pServerDE->GetObjectPos(m_hOwner, &vPos);

		// Show a little explosion
		HMESSAGEWRITE hMessage = pServerDE->StartInstantSpecialEffectMessage(&vPos);
		pServerDE->WriteToMessageByte(hMessage, SFX_EXPLOSIONFX_ID);

		pServerDE->WriteToMessageVector(hMessage, &vPos);
		pServerDE->WriteToMessageVector(hMessage, &vNormal);
		pServerDE->WriteToMessageDWord(hMessage, EXP_DEFAULT_SMALL);

		pServerDE->EndMessage2(hMessage, MESSAGE_GUARANTEED | MESSAGE_NAGGLEFAST);

		// Play an explosion sound
		PlaySoundFromPos(&vPos, "Sounds\\Weapons\\time\\explode.wav", 1000.0f, SOUNDPRIORITY_MISC_MEDIUM);

		// Damage yourself
		SendDamageMsg(m_hOwner, vPos, &vNormal, m_fDamage);
		return DNULL;
	}

	if( bAltFire )
	{
		pServerDE->GetObjectPos(m_hOwner, &m_vPosition);

		vFire->x = 0.0f;
		vFire->y = -1.0f;
		vFire->z = 0.0f;
		pProject = CWeapon::FireProjectile(vFire, dist, DTRUE);
	}
	else
		pProject = CWeapon::FireProjectile(vFire, dist, DFALSE);

	if(m_pInventoryMgr && !m_pInventoryMgr->GetAmmoCount(GetAmmoType(bAltFire)))
		m_pInventoryMgr->SelectNextWeapon();

	return pProject;
}

#endif // _DEMO

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDrudgeLightning::FireVector()
//
//	PURPOSE:	Fires a vector
//
// ----------------------------------------------------------------------- //

DBOOL CDrudgeLightning::FireVector(DVector *vFire, DFLOAT dist, DBOOL bTracer)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE)		return DFALSE;

	IntersectQuery	iq;
	IntersectInfo	ii;
	DVector			vDist, vDest;

	iq.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID;
	iq.m_FilterFn = DNULL;
	iq.m_pUserData = DNULL;

	VEC_COPY(iq.m_From, m_vPosition);
	VEC_COPY(iq.m_To, m_vPosition);
	VEC_MULSCALAR(vDist, *vFire, dist)
	VEC_ADD(iq.m_To, iq.m_To, vDist);

	if(pServerDE->IntersectSegment(&iq, &ii))
		{ VEC_COPY(vDest, ii.m_Point); }
	else
		{ VEC_COPY(vDest, iq.m_To); }

	// Create the client side lightning effect
	HMESSAGEWRITE hMessage = pServerDE->StartInstantSpecialEffectMessage(&(iq.m_From));
	pServerDE->WriteToMessageByte(hMessage, SFX_LIGHTNING_ID);

	pServerDE->WriteToMessageVector(hMessage, &(iq.m_From));
	pServerDE->WriteToMessageVector(hMessage, &vDest);
	pServerDE->WriteToMessageByte(hMessage, LIGHTNING_SHAPE_BOXED);
	pServerDE->WriteToMessageByte(hMessage, LIGHTNING_FORM_THIN2THIN);
	pServerDE->WriteToMessageByte(hMessage, LIGHTNING_TYPE_HIGHSEG);

	pServerDE->EndMessage2(hMessage, MESSAGE_GUARANTEED | MESSAGE_NAGGLEFAST);

	return CWeapon::FireVector(vFire, dist, bTracer);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDeathShroudZap::CDeathShroudZap()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CDeathShroudZap::CDeathShroudZap() : CWeapon(WEAP_DEATHSHROUD_ZAP)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE)		return;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDeathShroudZap::Fire()
//
//	PURPOSE:	Fires the Death Shroud's nasty 'zap'
//
// ----------------------------------------------------------------------- //

DDWORD CDeathShroudZap::Fire()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE)		return 0;

	return CWeapon::Fire();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDeathShroudZap::FireVector()
//
//	PURPOSE:	Fires a vector
//
// ----------------------------------------------------------------------- //

DBOOL CDeathShroudZap::FireVector(DVector *vFire, DFLOAT dist, DBOOL bTracer)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE)		return DFALSE;

	IntersectQuery	iq;
	IntersectInfo	ii;
	DVector			vDist;

	iq.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID;
	iq.m_FilterFn = DNULL;
	iq.m_pUserData = DNULL;

	VEC_COPY(iq.m_From, m_vPosition);
	VEC_COPY(iq.m_To, m_vPosition);
	VEC_MULSCALAR(vDist, *vFire, dist);
	VEC_ADD(iq.m_To, iq.m_To, vDist);

	HCLASS	hCharacter	= pServerDE->GetClass("CBaseCharacter");
	HCLASS	hPlayer		= pServerDE->GetClass("CPlayerObj");
	HCLASS	hCorpse		= pServerDE->GetClass("CCorpse");
	HCLASS	hObjClass   = DNULL;

	// Did we hit something?
	if(pServerDE->IntersectSegment(&iq, &ii)) 
	{
		// Was it an object?
		if (ii.m_hObject)
		{
			hObjClass = pServerDE->GetObjectClass(ii.m_hObject);

			// Was it something this spell affects?
			if(pServerDE->IsKindOf(hObjClass, hCharacter) || pServerDE->IsKindOf(hObjClass, hPlayer) || pServerDE->IsKindOf(hObjClass, hCorpse))
			{
				DVector vPos, vNormal;

				pServerDE->GetObjectPos(ii.m_hObject,&vPos);
				VEC_COPY(vNormal, ii.m_Plane.m_Normal);

				// Show a little 'flame' explosion
				HMESSAGEWRITE hMessage = pServerDE->StartInstantSpecialEffectMessage(&vPos);
				pServerDE->WriteToMessageByte(hMessage, SFX_EXPLOSIONFX_ID);

				pServerDE->WriteToMessageVector(hMessage, &vPos);
				pServerDE->WriteToMessageVector(hMessage, &vNormal);
				pServerDE->WriteToMessageDWord(hMessage, EXP_FLAME_SMALL);

				pServerDE->EndMessage2(hMessage, MESSAGE_GUARANTEED | MESSAGE_NAGGLEFAST);
			}
		}
	}

	// Pass down to normal vector damage-calculation
	return CWeapon::FireVector(vFire, dist, bTracer);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CZealotHeal::CZealotHeal()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CZealotHeal::CZealotHeal() : CWeapon(WEAP_ZEALOT_HEAL)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE)		return;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CZealotHeal::Fire()
//
//	PURPOSE:	Fires the Zealot's Heal Spell
//
// ----------------------------------------------------------------------- //

DDWORD CZealotHeal::Fire()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE)		return 0;

	return CWeapon::Fire();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CZealotHeal::FireVector()
//
//	PURPOSE:	Fires a vector
//
// ----------------------------------------------------------------------- //

DBOOL CZealotHeal::FireVector(DVector *vFire, DFLOAT dist, DBOOL bTracer)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE)		return DFALSE;

	DVector		offset;
	VEC_SET(offset, 0.0f, 0.0f, 0.0f);

	// Display the 'heal' effect.
	HMESSAGEWRITE hMessage = pServerDE->StartInstantSpecialEffectMessage(&offset);
	pServerDE->WriteToMessageByte(hMessage, SFX_OBJECTFX_ID);

	pServerDE->WriteToMessageObject(hMessage, m_hOwner);
	pServerDE->WriteToMessageVector(hMessage, &offset);
	pServerDE->WriteToMessageFloat(hMessage, 0.0f);
	pServerDE->WriteToMessageDWord(hMessage, 0);
	pServerDE->WriteToMessageDWord(hMessage, OBJFX_HEAL_1);
	pServerDE->WriteToMessageDWord(hMessage, 0);

	pServerDE->EndMessage2(hMessage, MESSAGE_GUARANTEED | MESSAGE_NAGGLEFAST);

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CZealotShield::CZealotShield()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CZealotShield::CZealotShield() : CWeapon(WEAP_ZEALOT_SHIELD)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE)		return;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CZealotShield::Fire()
//
//	PURPOSE:	Fires the Zealot's Heal Spell
//
// ----------------------------------------------------------------------- //

DDWORD CZealotShield::Fire()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE)		return 0;

	return CWeapon::Fire();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CZealotShield::FireVector()
//
//	PURPOSE:	Fires a vector
//
// ----------------------------------------------------------------------- //


DBOOL CZealotShield::FireVector(DVector *vFire, DFLOAT dist, DBOOL bTracer)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE)		return DFALSE;

	IntersectQuery	iq;
	IntersectInfo	ii;
	DVector			vDist;

	iq.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID;
	iq.m_FilterFn = DNULL;
	iq.m_pUserData = DNULL;

	VEC_COPY(iq.m_From, m_vPosition);
	VEC_COPY(iq.m_To, m_vPosition);
	VEC_MULSCALAR(vDist, *vFire, dist);
	VEC_ADD(iq.m_To, iq.m_To, vDist);

	HCLASS	hCharacter	= pServerDE->GetClass("CBaseCharacter");
	HCLASS	hPlayer		= pServerDE->GetClass("CPlayerObj");
	HCLASS	hObjClass   = DNULL;

	// Check intersection
	if(pServerDE->IntersectSegment(&iq, &ii)) 
	{
		// Did we hit an object?
		if (ii.m_hObject)
		{
			// What is the object?
			hObjClass = pServerDE->GetObjectClass(ii.m_hObject);

			// Is it a player or character?
			if(pServerDE->IsKindOf(hObjClass, hCharacter) || pServerDE->IsKindOf(hObjClass, hPlayer))
			{
				DVector		offset;
				VEC_SET(offset, 0.0f, 0.0f, 0.0f);

				// Display the 'heal' effect.
				HMESSAGEWRITE hMessage = pServerDE->StartInstantSpecialEffectMessage(&offset);
				pServerDE->WriteToMessageByte(hMessage, SFX_OBJECTFX_ID);

				pServerDE->WriteToMessageObject(hMessage, ii.m_hObject);
				pServerDE->WriteToMessageVector(hMessage, &offset);
				pServerDE->WriteToMessageFloat(hMessage, 0.0f);
				pServerDE->WriteToMessageDWord(hMessage, 0);
				pServerDE->WriteToMessageDWord(hMessage, OBJFX_SHIELD_1);
				pServerDE->WriteToMessageDWord(hMessage, 0);

				pServerDE->EndMessage2(hMessage, MESSAGE_GUARANTEED | MESSAGE_NAGGLEFAST);
			}

			return DTRUE;
		}
	}

	return DFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CShikariSpit::Fire()
//
//	PURPOSE:	Fires the weapon
//
// ----------------------------------------------------------------------- //
DDWORD CShikariSpit::Fire()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE)
		return 0;

	return CWeapon::Fire();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CShikariSpit::FireProjectile()
//
//	PURPOSE:	Fires the weapon
//
// ----------------------------------------------------------------------- //
CProjectile *CShikariSpit::FireProjectile(DVector *vFire, DFLOAT dist, DBOOL bAltFire)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	CProjectile *pProject = CWeapon::FireProjectile(vFire, dist, bAltFire);

	DDWORD dwFlags = pServerDE->GetObjectFlags(pProject->m_hObject);
	dwFlags &= ~FLAG_GRAVITY;
	pProject->SetFlags(dwFlags);

	return pProject;
}

DBOOL IgnoreTypeFilterFn(HOBJECT hObj, void *pUserData)
{
	if (!hObj || !g_pServerDE) return DFALSE;

	DBOOL  bKeep = DTRUE;
	HCLASS hTest = (*((HCLASS *)(pUserData)));
	HCLASS hClass = g_pServerDE->GetObjectClass(hObj);

	if (g_pServerDE->IsKindOf(hClass,hTest))
		bKeep = DFALSE;

	return bKeep;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNagaEyes::FireVector()
//
//	PURPOSE:	Fires the weapon
//
// ----------------------------------------------------------------------- //

DBOOL CNagaEyes::FireVector(DVector *vFire, DFLOAT dist, DBOOL bTracer)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE)		return 0;

	IntersectQuery	iq;
	IntersectInfo	ii;
	DVector			vDist, vDest;
	HCLASS			hClass;

	hClass = pServerDE->GetObjectClass(m_hOwner);

	iq.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID;
	iq.m_FilterFn = IgnoreTypeFilterFn;
	iq.m_pUserData = (void *)&hClass;

	VEC_COPY(iq.m_From, m_vPosition);
	VEC_COPY(iq.m_To, m_vPosition);
	VEC_MULSCALAR(vDist, *vFire, dist)
	VEC_ADD(iq.m_To, iq.m_To, vDist);

	if(pServerDE->IntersectSegment(&iq, &ii))
		{ VEC_COPY(vDest, ii.m_Point); }
	else
		{ VEC_COPY(vDest, iq.m_To); }

	// Create the client side laser effect
	HMESSAGEWRITE hMessage = pServerDE->StartInstantSpecialEffectMessage(&(iq.m_From));
	pServerDE->WriteToMessageByte(hMessage, SFX_LASERBEAM_ID);

	pServerDE->WriteToMessageVector(hMessage, &m_vPosition);
	pServerDE->WriteToMessageVector(hMessage, &vDest);
	pServerDE->WriteToMessageByte(hMessage, LASER_RED_SMALL);

	pServerDE->EndMessage2(hMessage, MESSAGE_GUARANTEED | MESSAGE_NAGGLEFAST);

	// Create the impact effect
	DDWORD		nType = EXP_NAGA_EYE_BEAM;

	hMessage = pServerDE->StartInstantSpecialEffectMessage(&vDest);
	pServerDE->WriteToMessageByte(hMessage, SFX_EXPLOSIONFX_ID);

	pServerDE->WriteToMessageVector(hMessage, &vDest);
	pServerDE->WriteToMessageVector(hMessage, &vDest);
	pServerDE->WriteToMessageDWord(hMessage, nType);

	pServerDE->EndMessage2(hMessage, MESSAGE_GUARANTEED | MESSAGE_NAGGLEFAST);

	return CWeapon::FireVector(vFire, dist, bTracer);
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNagaSpike::Fire()
//
//	PURPOSE:	Fires the weapon
//
// ----------------------------------------------------------------------- //
DDWORD CNagaSpike::Fire()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE)
		return 0;

	HCLASS hObjClass = pServerDE->GetObjectClass(m_hOwner);

	if(pServerDE->IsKindOf(hObjClass, pServerDE->GetClass("Gideon")))
		m_szFireSound = "sounds\\enemies\\gideon\\eyelaser.wav";

	return CWeapon::Fire();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNagaSpike::FireProjectile()
//
//	PURPOSE:	Fires the weapon
//
// ----------------------------------------------------------------------- //
CProjectile *CNagaSpike::FireProjectile(DVector *vFire, DFLOAT dist, DBOOL bAltFire)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	CProjectile *pProject = CWeapon::FireProjectile(vFire, dist, bAltFire);

	DDWORD dwFlags = pServerDE->GetObjectFlags(pProject->m_hObject);
	dwFlags &= ~FLAG_GRAVITY;
	pProject->SetFlags(dwFlags);

	return pProject;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNagaDebris::Fire()
//
//	PURPOSE:	Fires the weapon
//
// ----------------------------------------------------------------------- //
DDWORD CNagaDebris::Fire()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE)
		return 0;

	return CWeapon::Fire();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNagaDebris::FireProjectile()
//
//	PURPOSE:	Fires the weapon
//
// ----------------------------------------------------------------------- //
CProjectile *CNagaDebris::FireProjectile(DVector *vFire, DFLOAT dist, DBOOL bAltFire)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	CProjectile *pProject = CWeapon::FireProjectile(vFire, dist, bAltFire);

	DDWORD dwFlags = pServerDE->GetObjectFlags(pProject->m_hObject);
	dwFlags &= ~FLAG_GRAVITY;
	pProject->SetFlags(dwFlags);

	return pProject;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGideonShield::CGideonShield()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CGideonShield::CGideonShield() : CWeapon(WEAP_GIDEON_SHIELD)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE)		return;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGideonShield::Fire()
//
//	PURPOSE:	Fires the Zealot's Heal Spell
//
// ----------------------------------------------------------------------- //

DDWORD CGideonShield::Fire()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE)		return 0;

	return CWeapon::Fire();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGideonShield::FireVector()
//
//	PURPOSE:	Fires a vector
//
// ----------------------------------------------------------------------- //

DBOOL CGideonShield::FireVector(DVector *vFire, DFLOAT dist, DBOOL bTracer)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE)		return DFALSE;

	DVector		offset;
	VEC_SET(offset, 0.0f, 0.0f, 0.0f);

	// Display the 'heal' effect.
	HMESSAGEWRITE hMessage = pServerDE->StartInstantSpecialEffectMessage(&offset);
	pServerDE->WriteToMessageByte(hMessage, SFX_OBJECTFX_ID);

	pServerDE->WriteToMessageObject(hMessage, m_hOwner);
	pServerDE->WriteToMessageVector(hMessage, &offset);
	pServerDE->WriteToMessageFloat(hMessage, 0.0f);
	pServerDE->WriteToMessageDWord(hMessage, 0);
	pServerDE->WriteToMessageDWord(hMessage, OBJFX_SHIELD_1);
	pServerDE->WriteToMessageDWord(hMessage, 0);

	pServerDE->EndMessage2(hMessage, MESSAGE_GUARANTEED | MESSAGE_NAGGLEFAST);

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGideonWind::CGideonWind()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CGideonWind::CGideonWind() : CWeapon(WEAP_GIDEON_WIND)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE)		return;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGideonWind::Fire()
//
//	PURPOSE:	Fires the Zealot's Heal Spell
//
// ----------------------------------------------------------------------- //

DDWORD CGideonWind::Fire()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE)		return 0;

	return CWeapon::Fire();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGideonWind::FireVector()
//
//	PURPOSE:	Fires a vector
//
// ----------------------------------------------------------------------- //

DBOOL CGideonWind::FireVector(DVector *vFire, DFLOAT dist, DBOOL bTracer)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE)		return DFALSE;

	DVector vPos;

	pServerDE->GetObjectPos(m_hOwner,&vPos);

	ObjectList *ol = pServerDE->FindObjectsTouchingSphere(&vPos, 1200.0f);

	if(ol)
	{
		DVector		vObjPos, vObjVel, vUp;
		ObjectLink* pLink = ol->m_pFirstLink;
		DFLOAT		mag;
		HOBJECT		hObj;
		short		objType;
		DDWORD		dwUserFlags;

		VEC_SET(vUp, 0.0f, 1.0f, 0.0f);

		while(pLink)
		{
			hObj = pLink->m_hObject;
			pLink = pLink->m_pNext;

			objType = pServerDE->GetObjectType(hObj);
			dwUserFlags = pServerDE->GetObjectUserFlags(hObj);

			if(hObj == m_hOwner) continue;
			if((objType != OT_MODEL) || !(dwUserFlags & USRFLG_SINGULARITY_ATTRACT)) continue;
//			if(!pServerDE->IsKindOf(pServerDE->GetObjectClass(hObj), pServerDE->GetClass("CBaseCharacter"))) continue;

			// Get the information about the obj
			pServerDE->GetObjectPos(hObj, &vObjPos);
			pServerDE->GetVelocity(hObj, &vObjVel);

			VEC_SUB(vObjPos, vObjPos, vPos);
			mag = 1200.0f - VEC_MAG(vObjPos);
			if(mag < 0.0f)	mag = 0.0f;
			VEC_NORM(vObjPos);
			VEC_MULSCALAR(vObjPos, vObjPos, mag * 7.5f);
			pServerDE->GetGlobalForce(&vUp);
			VEC_MULSCALAR(vUp, vUp, -0.25f);
			VEC_ADD(vObjVel, vObjVel, vUp);
			VEC_ADD(vObjVel, vObjVel, vObjPos);
			pServerDE->SetVelocity(hObj, &vObjVel);
		}

		pServerDE->RelinquishList(ol);
	}

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVomit::Fire()
//
//	PURPOSE:	Fires the weapon
//
// ----------------------------------------------------------------------- //
DDWORD CVomit::Fire()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE)
		return 0;

	return CWeapon::Fire();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVomit::FireProjectile()
//
//	PURPOSE:	Fires the weapon
//
// ----------------------------------------------------------------------- //
CProjectile *CVomit::FireProjectile(DVector *vFire, DFLOAT dist, DBOOL bAltFire)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	CProjectile *pProject = CWeapon::FireProjectile(vFire, dist, bAltFire);

	DDWORD dwFlags = pServerDE->GetObjectFlags(pProject->m_hObject);
	dwFlags &= ~FLAG_GRAVITY;
	pProject->SetFlags(dwFlags);

	return pProject;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGooSpit::Fire()
//
//	PURPOSE:	Fires the weapon
//
// ----------------------------------------------------------------------- //
DDWORD CGooSpit::Fire()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE)
		return 0;

	return CWeapon::Fire();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGooSpit::FireProjectile()
//
//	PURPOSE:	Fires the weapon
//
// ----------------------------------------------------------------------- //
CProjectile *CGooSpit::FireProjectile(DVector *vFire, DFLOAT dist, DBOOL bAltFire)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	CProjectile *pProject = CWeapon::FireProjectile(vFire, dist, bAltFire);

	DDWORD dwFlags = pServerDE->GetObjectFlags(pProject->m_hObject);
	dwFlags = dwFlags | FLAG_GRAVITY;
	pProject->SetFlags(dwFlags);

	return pProject;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGideonSpear::FireVector()
//
//	PURPOSE:	Fires the weapon
//
// ----------------------------------------------------------------------- //

DBOOL CGideonSpear::FireVector(DVector *vFire, DFLOAT dist, DBOOL bTracer)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE)		return 0;

	IntersectQuery	iq;
	IntersectInfo	ii;
	DVector			vDist;

	iq.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID;
	iq.m_FilterFn = DNULL;
	iq.m_pUserData = DNULL;

	VEC_COPY(iq.m_From, m_vPosition);
	VEC_COPY(iq.m_To, m_vPosition);
	VEC_MULSCALAR(vDist, *vFire, dist)
	VEC_ADD(iq.m_To, iq.m_To, vDist);

	if(pServerDE->IntersectSegment(&iq, &ii))
	{ 
		HOBJECT hObj = ii.m_hObject;

		if (hObj)
		{
			HCLASS hPlayer = pServerDE->GetClass("CPlayerObj");
			HCLASS hClass = pServerDE->GetObjectClass(hObj);

			LPBASECLASS pObj = pServerDE->HandleToObject(m_hOwner);

			if (pServerDE->IsKindOf(hClass,hPlayer))
			{
				HMESSAGEWRITE hMessage = pServerDE->StartMessageToObject(pObj, hObj, MID_IMMOBILIZE);
				pServerDE->WriteToMessageFloat(hMessage,4.0f);
				pServerDE->EndMessage2(hMessage, MESSAGE_GUARANTEED | MESSAGE_NAGGLEFAST);
			}
		}
	}

	return CWeapon::FireVector(vFire, dist, bTracer);
}


DBOOL AncientOneFilterFn(HOBJECT hObj, void *pUserData)
{
	if (!hObj || !g_pServerDE) return DFALSE;

	HCLASS hObjClass	= g_pServerDE->GetObjectClass(hObj);
	DBOOL bKeep = DTRUE;

	BaseClass* pObj = (BaseClass*)g_pServerDE->HandleToObject(hObj);			

	HCLASS hCharacter = g_pServerDE->GetClass("AncientOneTentacle");

	if(g_pServerDE->IsKindOf(hObjClass, hCharacter))
		bKeep = DFALSE;

	return bKeep;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAncientOneBeam::FireVector()
//
//	PURPOSE:	Fires the weapon
//
// ----------------------------------------------------------------------- //

DBOOL CAncientOneBeam::FireVector(DVector *vFire, DFLOAT dist, DBOOL bTracer)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE)		return 0;

	IntersectQuery	iq;
	IntersectInfo	ii;
	DVector			vDist, vDest;

	iq.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID;
	iq.m_FilterFn = AncientOneFilterFn;
	iq.m_pUserData = DNULL;

	VEC_COPY(iq.m_From, m_vPosition);
	VEC_COPY(iq.m_To, m_vPosition);
	VEC_MULSCALAR(vDist, *vFire, dist)
	VEC_ADD(iq.m_To, iq.m_To, vDist);

	if(pServerDE->IntersectSegment(&iq, &ii))
		{ VEC_COPY(vDest, ii.m_Point); }
	else
		{ VEC_COPY(vDest, iq.m_To); }

	// Create the client side laser effect
	HMESSAGEWRITE hMessage = pServerDE->StartInstantSpecialEffectMessage(&(iq.m_From));
	pServerDE->WriteToMessageByte(hMessage, SFX_LASERBEAM_ID);

	pServerDE->WriteToMessageVector(hMessage, &m_vPosition);
	pServerDE->WriteToMessageVector(hMessage, &vDest);
	pServerDE->WriteToMessageByte(hMessage, LASER_RED_LARGE);

	pServerDE->EndMessage2(hMessage, MESSAGE_GUARANTEED | MESSAGE_NAGGLEFAST);

	DVector			vFrom, vTo;
	DDWORD			nFX = 0;
	int				nNode = 999;

	// Setup the origin and destination points
	VEC_COPY(vFrom, m_vPosition);
	VEC_MULSCALAR(vDist, *vFire, dist)
	VEC_ADD(vTo, m_vPosition, vDist);

	// Set the intersection query values
	iq.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID | INTERSECT_HPOLY;
	iq.m_FilterFn = AncientOneFilterFn;
	iq.m_pUserData = DNULL;

	HCLASS	hVolume		= g_pServerDE->GetClass("VolumeBrush");
	HCLASS	hObjClass   = DNULL;

	VEC_COPY(iq.m_From, vFrom);
	VEC_COPY(iq.m_To, vTo);

	while(1)
	{
		if(pServerDE->IntersectSegment(&iq, &ii))
		{
			if (m_hOwner != ii.m_hObject)	// don't hit myself.
			{
				hObjClass   = g_pServerDE->GetObjectClass(ii.m_hObject);

				SendDamageMsg(ii.m_hObject, ii.m_Point, vFire, m_fDamage);

				DVector vNormal;
				VEC_SET(vNormal, 0.0f, 1.0f, 0.0f);

				DDWORD		nType = EXP_NAPALM_PRIMARY;

				hMessage = pServerDE->StartInstantSpecialEffectMessage(&(ii.m_Point));
				pServerDE->WriteToMessageByte(hMessage, SFX_EXPLOSIONFX_ID);

				pServerDE->WriteToMessageVector(hMessage, &ii.m_Point);
				pServerDE->WriteToMessageVector(hMessage, &vNormal);
				pServerDE->WriteToMessageDWord(hMessage, nType);

				pServerDE->EndMessage2(hMessage, MESSAGE_GUARANTEED | MESSAGE_NAGGLEFAST);

				return DTRUE;
			}

			// Add a bit to the vector and carry the shot through
			VEC_ADD(iq.m_From, ii.m_Point, *vFire);
		}
		else
			return DFALSE;		//did not hit an object in max range
	}

	PlaySoundFromPos(&(iq.m_From), "Sounds\\Weapons\\c4\\explosion_1.wav", 1000.0f, SOUNDPRIORITY_MISC_MEDIUM);

	return DTRUE;

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAncientOneTentacle::Fire()
//
//	PURPOSE:	Fires the weapon
//
// ----------------------------------------------------------------------- //
DDWORD CAncientOneTentacle::Fire()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE)
		return 0;

	DBOOL bAltFire = (m_eState == WS_ALT_FIRING);

	int sound = pServerDE->IntRandom(0, 4);

	// Choose a random firing sound
	if (bAltFire)
	{
		if (sound < 1)		m_szFireSound = "Sounds\\Slash1.wav";
		else if (sound < 2) m_szFireSound = "Sounds\\Slash2.wav";
		else if (sound < 3)	m_szFireSound = "Sounds\\Slash3.wav";
		else				m_szFireSound = "Sounds\\Slash4.wav";
	}
	else
	{
		if (sound < 1)		m_szAltFireSound = "Sounds\\Slash1.wav";
		else if (sound < 2) m_szAltFireSound = "Sounds\\Slash2.wav";
		else if (sound < 3)	m_szAltFireSound = "Sounds\\Slash3.wav";
		else				m_szAltFireSound = "Sounds\\Slash4.wav";
	}

	return CWeapon::Fire();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSkull::UpdateFiringState()
//
//	PURPOSE:	Special update stuff for skull
//
// ----------------------------------------------------------------------- //

void CSkull::UpdateFiringState(DVector *firedPos, DRotation *rotP, DBOOL bFiring, DBOOL bAltFiring)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE)	return;

	CWeapon::UpdateFiringState(firedPos, rotP, bFiring, bAltFiring);

	if(m_eState == WS_REST)
		pServerDE->SetModelLooping(m_pViewModel->m_hObject, DTRUE);
}

#ifdef _ADD_ON

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGremlinRock::FireProjectile()
//
//	PURPOSE:	Fires the weapon
//
// ----------------------------------------------------------------------- //
CProjectile *CGremlinRock::FireProjectile(DVector *vFire, DFLOAT dist, DBOOL bAltFire)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();

	m_fProjVelocity = pServerDE->Random(500.0f, 1000.0f);
	m_fAltProjVelocity = m_fProjVelocity;

	DVector	vUp;
	VEC_SET(vUp, 0.0f, 1.0f, 0.0f);
	VEC_MULSCALAR(vUp, vUp, 1.0f - ((m_fProjVelocity - 500.0f) / 1000.0f));
	VEC_ADD(*vFire, *vFire, vUp);
	VEC_NORM(*vFire);

	return CWeapon::FireProjectile(vFire, dist, bAltFire);
}

#endif