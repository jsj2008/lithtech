// ----------------------------------------------------------------------- //
//
// MODULE  : Explosion.cpp
//
// PURPOSE : Explosion - Definition
//
// CREATED : 11/25/97
//
// (c) 1997-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "Explosion.h"
#include "iltserver.h"
#include "ServerUtilities.h"
#include "WeaponFXTypes.h"
#include "ObjectMsgs.h"
#include "ParsedMsg.h"
#include "DamageTypes.h"
#include "SharedFXStructs.h"
#include "SFXMsgIds.h"
#include "Character.h"
#include "VarTrack.h"
#include "FXDB.h"
#include "AIUtils.h"
#include "PhysicsUtilities.h"
#include "CharacterHitBox.h"
#include "PlayerObj.h"
#include "ServerConnectionMgr.h"
#include "CharacterDB.h"

static VarTrack s_vtExplosionForce;
static VarTrack s_vtExplosionForceDirMinY;

#define MIN_RADIUS_PERCENT				0.25f
#define DEFAULT_EXPOLSION_FORCE			5000.0f

LINKFROM_MODULE( Explosion );

BEGIN_CLASS(Explosion)

	ADD_STRINGPROP_FLAG(ImpactFXName, "", PF_STATICLIST, "Select from a list of ImpactFX listed in GameDatabase under FX\\ImpactFX.")
	ADD_STRINGPROP_FLAG(DamageType, "EXPLODE", PF_STATICLIST, "This is a pull down used to identify what kind of damage will be inflicted upon anyone caught within the DamageRadius of the Explosion object when it is triggered.")
	ADD_REALPROP_FLAG(DamageRadiusMin, 0.0f, PF_RADIUS, "This the minimum radius measured in WorldEdit units in which an object will take 100% of the MaxDamage damage from the explosion.  The damage amount is linearly scaled from MaxDamage to 0 between the DamageRadiusMin and DamageRadius.  NOTE: If set to 0, 25% of the DamageRadius will be used for the DamageRadiusMin.")
	ADD_REALPROP_FLAG(DamageRadius, 200.0f, PF_RADIUS, "This the radius measured in WorldEdit units in which an object will take damage from the explosion.")
	ADD_REALPROP_FLAG(MaxDamage, 200.0f, 0, "This field defines the maximum amount of damage that will be inflicted upon an object caught within the DamageRadius of the explosion when it is triggered.")
    ADD_BOOLPROP_FLAG(RemoveWhenDone, true, 0, "This flag toggles whether or not the explosion is removed after being triggered. Sometimes you will want to trigger an Explosion object repeatedly. In which case you would set this flag to false.")
	ADD_REALPROP_FLAG(ImpulseForce, DEFAULT_EXPOLSION_FORCE, 0, "The amount of force the explosion will apply to objects inside the DamageRadiusMin.  The force is linearly scaled between ImpulseForce and 0 between DamageRadiusMin and DamageRadius.  The value is measured in game unit newton seconds.")

END_CLASS_FLAGS_PLUGIN(Explosion, GameBase, 0, CExplosionPlugin, "Explosion objects are used to create a dynamic explosion with a ClientFX, damage type and physics impulse force." )


CMDMGR_BEGIN_REGISTER_CLASS( Explosion )

	ADD_MESSAGE( ON,	1,	NULL,	MSG_HANDLER( Explosion, HandleOnMsg ),	"ON", "Turns the explosion on.", "msg Explosion ON" )

CMDMGR_END_REGISTER_CLASS( Explosion, GameBase )


bool ExplosionFilterFn(HOBJECT hObj, void* /*pUserData*/)
{
	uint32 dwFlags;
	g_pCommonLT->GetObjectFlags(hObj, OFT_Flags, dwFlags);
	if (!(dwFlags & FLAG_SOLID))
	{
		return false;
	}
	else if (IsMainWorld(hObj) || (OT_WORLDMODEL == GetObjectType(hObj)))
	{
		return true;
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Explosion::Explosion()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

Explosion::Explosion() : GameBase()
{
	m_fImpulse				= DEFAULT_EXPOLSION_FORCE;
	m_fDamageRadiusMin		= 0.0f;
	m_fDamageRadius			= 200.0f;
	m_fMaxDamage			= 200.0f;
	m_fPenetration			= 0.0f;
	m_eDamageType			= DT_UNSPECIFIED;

	// For now these aren't used with WorldEdit created Explosions...

	m_fProgDamage			= 0.0f;
	m_fProgDamageDuration	= 0.0f;
	m_fProgDamageRadius		= 0.0f;
	m_fProgDamageLifetime	= 0.0f;
	m_eProgDamageType		= DT_UNSPECIFIED;
	m_hFiredFrom			= NULL;
	m_hAmmo					= NULL;

    m_bRemoveWhenDone       = true;

	m_vPos.Init();

	m_nImpactFXId			= INVALID_GAME_DATABASE_INDEX;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Explosion::Setup()
//
//	PURPOSE:	Setup the Explosion
//
// ----------------------------------------------------------------------- //

void Explosion::Setup( HOBJECT hFiredFrom, HAMMO hAmmo, bool bUseAIAmmo )
{
	if ( !hAmmo )
		return;

    m_bRemoveWhenDone = true;
	m_hAmmo	= hAmmo;

	HAMMODATA hAmmoData = g_pWeaponDB->GetAmmoData(hAmmo,bUseAIAmmo);
	m_fImpulse				= g_pWeaponDB->GetFloat( hAmmoData, WDB_AMMO_fAreaDamageImpulseForce );
	m_fDamageRadius			= g_pWeaponDB->GetFloat( hAmmoData, WDB_AMMO_fAreaDamageRadius );
	m_fMaxDamage			= g_pWeaponDB->GetFloat( hAmmoData, WDB_AMMO_fAreaDamage );
	m_fPenetration			= g_pWeaponDB->GetFloat( hAmmoData, WDB_AMMO_fAreaPenetration );
	m_eDamageType			= g_pWeaponDB->GetAmmoAreaDamageType( hAmmo );
	m_fDamageRadiusMin		= g_pWeaponDB->GetFloat( hAmmoData, WDB_AMMO_fAreaDamageRadiusMin );

	// For backwards compatibility before this value was in the database...
	if (m_fDamageRadiusMin <= 0.0f)
	{
		m_fDamageRadiusMin = m_fDamageRadius * MIN_RADIUS_PERCENT;
	}

	m_fProgDamageRadius		= g_pWeaponDB->GetFloat( hAmmoData, WDB_AMMO_fProgDamageRadius );
	m_fProgDamageLifetime	= g_pWeaponDB->GetFloat( hAmmoData, WDB_AMMO_fProgDamageLifetime );
	m_fProgDamage			= g_pWeaponDB->GetFloat( hAmmoData, WDB_AMMO_fProgDamage );
	m_fProgDamageDuration	= g_pWeaponDB->GetFloat( hAmmoData, WDB_AMMO_fProgDamageDuration );
	m_eProgDamageType		= g_pWeaponDB->GetAmmoProgDamageType( hAmmo );

	if (!s_vtExplosionForce.IsInitted())
	{
		s_vtExplosionForce.Init(g_pLTServer, "PhysicsExplosionForce", NULL, DEFAULT_EXPOLSION_FORCE);
	}
	if(!s_vtExplosionForceDirMinY.IsInitted())
	{
		s_vtExplosionForceDirMinY.Init(g_pLTServer, "ExplosionForceDirMinY", NULL, 0.75f);
	}

	Start(hFiredFrom);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Explosion::Start()
//
//	PURPOSE:	Start the Explosion
//
// ----------------------------------------------------------------------- //

void Explosion::Start(HOBJECT hFiredFrom)
{
	if (!m_hObject) return;

	g_pLTServer->GetObjectPos(m_hObject, &m_vPos);

	// Do special fx (on the client)...
    LTRotation rRot;
	g_pLTServer->GetObjectRotation(m_hObject, &rRot);

	EXPLOSIONCREATESTRUCT cs;
	cs.nImpactFX		= m_nImpactFXId;
	cs.rRot				= rRot;
	cs.vPos				= m_vPos;
	cs.hAmmo			= m_hAmmo;
	cs.fRadius			= m_fDamageRadius;
	cs.fMinDamageRadius	= m_fDamageRadiusMin;
	cs.fImpulse			= m_fImpulse;

	CAutoMessage cMsg;
	cMsg.Writeuint8(SFX_EXPLOSION_ID);
    cs.Write(cMsg);
	g_pLTServer->SendSFXMessage(cMsg.Read(), 0);


	SetFiredFrom( hFiredFrom );

	// Do Area damage to the objects caught in the blast...

	if (m_fDamageRadius > 0.0f && m_fMaxDamage > 0.0f)
	{
		AreaDamageObjectsInSphere();
	}

	// Progressively damage the objects caught in the blast...

	if (m_fProgDamageRadius > 0.0f)
	{
		ProgDamageObjectsInSphere();
	}

	if (m_fProgDamageLifetime > 0.0f && m_fProgDamageRadius > 0.0f)
	{
		// Process the progressive damage every frame...

		m_ProgDamageTimer.Start(m_fProgDamageLifetime);
        SetNextUpdate(UPDATE_NEXT_FRAME);
	}
	else
	{
		if (m_bRemoveWhenDone)
		{
            g_pLTServer->RemoveObject(m_hObject);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Explosion::DoDamage()
//
//	PURPOSE:	Do the damage...
//
// ----------------------------------------------------------------------- //

void Explosion::Update()
{
	// Do progressive damage to the objects caught in the blast...

	ProgDamageObjectsInSphere();

	if (m_ProgDamageTimer.IsTimedOut())
	{
		if (m_bRemoveWhenDone)
		{
            g_pLTServer->RemoveObject(m_hObject);
		}

		SetFiredFrom( NULL );
	}
	else
	{
        SetNextUpdate(UPDATE_NEXT_FRAME);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Explosion::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 Explosion::EngineMessageFn(uint32 messageID, void *pData, float fData)
{
	switch(messageID)
	{
		case MID_UPDATE :
		{
			Update();
		}
		break;

		case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
				m_ProgDamageTimer.SetEngineTimer( SimulationTimer::Instance( ));
				SetNextUpdate(UPDATE_NEVER);
			}
		}
		break;

		case MID_PRECREATE :
		{
			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				ReadProp(&((ObjectCreateStruct*)pData)->m_cProperties);
			}
		}
		break;

		case MID_SAVEOBJECT:
		{
            Save((ILTMessage_Write*)pData, (uint32)fData);
		}
		break;

		case MID_LOADOBJECT:
		{
            Load((ILTMessage_Read*)pData, (uint32)fData);
		}
		break;

		default : break;
	}

	return GameBase::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Explosion::HandleOnMsg
//
//	PURPOSE:	Handle a ON message...
//
// ----------------------------------------------------------------------- //

void Explosion::HandleOnMsg( HOBJECT hSender, const CParsedMsg& /*crParsedMsg*/ )
{
	Start( hSender );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Explosion::ReadProp
//
//	PURPOSE:	Read object properties
//
// ----------------------------------------------------------------------- //

void Explosion::ReadProp(const GenericPropList *pProps)
{
	m_fImpulse			= pProps->GetReal( "ImpulseForce", m_fImpulse );
	m_fDamageRadiusMin	= pProps->GetReal( "DamageRadiusMin", m_fDamageRadiusMin );
	m_fDamageRadius		= pProps->GetReal( "DamageRadius", m_fDamageRadius );
	m_fMaxDamage		= pProps->GetReal( "MaxDamage", m_fMaxDamage );
	m_bRemoveWhenDone	= pProps->GetBool( "RemoveWhenDone", m_bRemoveWhenDone );

	g_pFXDB->ReadImpactFXProp(pProps, "ImpactFXName", m_nImpactFXId);

   	const char *pszDamageType = pProps->GetString( "DamageType", "" );
	if( pszDamageType && pszDamageType[0] )
	{
		m_eDamageType = StringToDamageType( pszDamageType );
	}

	// For backwards compatibility use 25% of the damage radius if the default value
	// is set for the min damage radius...
	m_fDamageRadiusMin = (m_fDamageRadiusMin <= 0 ? m_fDamageRadius * MIN_RADIUS_PERCENT : m_fDamageRadiusMin);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Explosion::AreaDamageObject()
//
//	PURPOSE:	Damage the object...
//
// ----------------------------------------------------------------------- //

void Explosion::AreaDamageObject(HOBJECT hObj)
{
	if (!hObj) return;

    LTVector vObjPos;
	g_pLTServer->GetObjectPos(hObj, &vObjPos);

    LTVector vDir = vObjPos - m_vPos;
    float fDist = vDir.Mag();

	if (fDist > m_fDamageRadius)
		return;

	// Test for character early out conditions...
	if (CharacterEarlyOut(hObj))
		return;

	// Scale damage if necessary...

    float fMultiplier = 1.0f;
	if (fDist > m_fDamageRadiusMin && m_fDamageRadiusMin < m_fDamageRadius)
	{
        float fPercent = (fDist - m_fDamageRadiusMin) / (m_fDamageRadius - m_fDamageRadiusMin);
		fPercent = fPercent > 1.0f ? 1.0f : (fPercent < 0.0f ? 0.0f : fPercent);

		fMultiplier = (1.0f - fPercent);
	}

	float fForce = m_fImpulse;

	//Apply a physical impulse force to the object that was damaged...
	if (vDir != LTVector::GetIdentity())
	{
		vDir.Normalize();

#ifndef _FINAL
		// See if someone is tweaking the forces...if not, use our impulse force.
		if (s_vtExplosionForce.GetFloat(DEFAULT_EXPOLSION_FORCE) != DEFAULT_EXPOLSION_FORCE)
		{
			fForce = s_vtExplosionForce.GetFloat();
		}
#endif // _FINAL

		// Adjust the force based on the distance away from the center...
		fForce *= fMultiplier;

		// If we hit a character hit box, actually apply the force to it's model object...
		HOBJECT hApplyObj = hObj;
		if (IsCharacterHitBox(hObj))
		{
			CCharacterHitBox* pHitBox = (CCharacterHitBox*)g_pLTServer->HandleToObject(hObj);
			if (pHitBox)
			{
				hApplyObj = pHitBox->GetModelObject();
			}
		}

		// Make sure there is some "up" force applied
		if (vDir.y < s_vtExplosionForceDirMinY.GetFloat())
		{
			vDir.y = s_vtExplosionForceDirMinY.GetFloat();
		}

		PhysicsUtilities::ApplyPhysicsImpulseForce(hApplyObj, fForce, vDir, LTVector(0, 0, 0), true);
	}

	if (IsCharacterHitBox(hObj))
	{
		CCharacterHitBox* pHitBox = (CCharacterHitBox*)g_pLTServer->HandleToObject(hObj);
		if (pHitBox)
		{
			pHitBox->SetModelNodeLastHit(NULL);
		}
	}


	DamageStruct damage;
	damage.hDamager		 = (m_hFiredFrom ? (HOBJECT)m_hFiredFrom : m_hObject);
	damage.eType		 = m_eDamageType;
	damage.fDamage		 = m_fMaxDamage * fMultiplier;
	damage.fPenetration  = m_fPenetration;
	damage.fImpulseForce = fForce;
	damage.hContainer	 = m_hObject;
	damage.hAmmo		 = m_hAmmo;
	damage.SetPositionalInfo(vObjPos, vDir);

	damage.DoDamage(m_hObject, hObj);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Explosion::ProgDamageObject()
//
//	PURPOSE:	Damage the object...
//
// ----------------------------------------------------------------------- //

void Explosion::ProgDamageObject(HOBJECT hObj)
{
	if (!hObj) return;

    LTVector vObjPos;
	g_pLTServer->GetObjectPos(hObj, &vObjPos);

    LTVector vDir = vObjPos - m_vPos;
    float fDist = vDir.Mag();

	if (fDist > m_fProgDamageRadius)
		return;

	// Test for character early out conditions...
	if (CharacterEarlyOut(hObj))
		return;

	DamageStruct damage;
	damage.hDamager		= (m_hFiredFrom ? (HOBJECT)m_hFiredFrom : m_hObject);
	damage.eType		= m_eProgDamageType;
	damage.fDuration	= m_fProgDamageDuration;
	damage.fDamage		= m_fProgDamage;
	damage.hContainer	= m_hObject;
	damage.SetPositionalInfo(vObjPos, vDir);

	damage.DoDamage(m_hObject, hObj);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Explosion::CharacterEarlyOut()
//
//	PURPOSE:	Determine if we should early out and not damage this object
//				because of special Character considerations
//
// ----------------------------------------------------------------------- //

bool Explosion::CharacterEarlyOut(HOBJECT hObj)
{
	// Filter out characters and only apply damage to CharacterHitBox's.  If we look 
	// for both Character and CharacterHitbox's, the object may get damaged twice.
	if (IsCharacter(hObj))
		return true;

	LTVector vObjPos;
	g_pLTServer->GetObjectPos(hObj, &vObjPos);

	LTVector vDir = vObjPos - m_vPos;
	float fDist = vDir.Mag();

	// Make sure that Characters don't take damage if another object
	// is blocking them from the explosion...

	if (IsCharacterHitBox(hObj))
	{
		// Don't allow AI to blow up their friends.

		if( IsAI( m_hFiredFrom ) )
		{
			CAI* pAI = (CAI*)g_pLTServer->HandleToObject( m_hFiredFrom );
			CCharacterHitBox* pHitBox = (CCharacterHitBox*)g_pLTServer->HandleToObject( hObj );
			if( pAI && pHitBox && IsCharacter( pHitBox->GetModelObject() ) )
			{
				CCharacter* pChar = (CCharacter*)g_pLTServer->HandleToObject( pHitBox->GetModelObject() );
				if( pChar && g_pCharacterDB->GetStance( pAI->GetAlignment(), pChar->GetAlignment() ) == kCharStance_Like )
				{
					return true;
				}
			}
		}


		// To do this test, do an intersect segment both directions
		// (from the object to the explosion and from the explosion
		// to the object).  This will ensure that neither point
		// is inside a wall and that nothing is blocking the damage...

		IntersectInfo iInfo;
		IntersectQuery qInfo;

		qInfo.m_Flags	  = INTERSECT_HPOLY | INTERSECT_OBJECTS | IGNORE_NONSOLID;
		qInfo.m_FilterFn  = ExplosionFilterFn;

		qInfo.m_From = m_vPos + vDir/fDist;
		qInfo.m_To   = vObjPos;

		bool bIntersect1 = g_pLTServer->IntersectSegment(qInfo, &iInfo);
		if( bIntersect1 )
			return true;

		qInfo.m_From = vObjPos;
		qInfo.m_To   = m_vPos + vDir/fDist;

		bool bIntersect2 = g_pLTServer->IntersectSegment(qInfo, &iInfo);
		if (bIntersect2)
			return true;
	}

	// Go ahead and damage this object..
	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Explosion::ProgDamageObjectsInSphere()
//
//	PURPOSE:	Progressively damage all the objects in our radius
//
// ----------------------------------------------------------------------- //

void Explosion::ProgDamageObjectsInSphere()
{
    ObjectList* pList = g_pLTServer->FindObjectsTouchingSphere(&m_vPos, m_fProgDamageRadius);
	if (!pList) return;

	ObjectLink* pLink = pList->m_pFirstLink;
	while (pLink)
	{
		ProgDamageObject(pLink->m_hObject);
		pLink = pLink->m_pNext;
	}

    g_pLTServer->RelinquishList(pList);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Explosion::AreaDamageObjectsInSphere()
//
//	PURPOSE:	Area damage all the objects in our radius
//
// ----------------------------------------------------------------------- //

void Explosion::AreaDamageObjectsInSphere()
{
    ObjectList* pList = g_pLTServer->FindObjectsTouchingSphere(&m_vPos, m_fDamageRadius);
	if (!pList) return;

	ObjectLink* pLink = pList->m_pFirstLink;
	while (pLink)
	{
		AreaDamageObject(pLink->m_hObject);
		pLink = pLink->m_pNext;
	}

    g_pLTServer->RelinquishList(pList);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Explosion::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void Explosion::Save(ILTMessage_Write *pMsg, uint32 /*dwSaveFlags*/)
{
	if (!pMsg) return;

    SAVE_HOBJECT(m_hFiredFrom);
	SAVE_VECTOR(m_vPos);
	SAVE_FLOAT(m_fDamageRadius);
	SAVE_FLOAT(m_fDamageRadiusMin);
    SAVE_FLOAT(m_fMaxDamage);
    SAVE_BYTE(( uint8 )m_eDamageType);
    SAVE_FLOAT(m_fProgDamage);
    SAVE_FLOAT(m_fProgDamageRadius);
    SAVE_FLOAT(m_fProgDamageDuration);
    SAVE_FLOAT(m_fProgDamageLifetime);
    SAVE_BYTE(( uint8 )m_eProgDamageType);
    SAVE_BOOL(m_bRemoveWhenDone);
    SAVE_DWORD(m_nImpactFXId);
	SAVE_FLOAT(m_fImpulse);

	m_ProgDamageTimer.Save(*pMsg);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Explosion::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void Explosion::Load(ILTMessage_Read *pMsg, uint32 /*dwLoadFlags*/)
{
	if (!pMsg) return;

	HOBJECT hFiredFrom;
	LOAD_HOBJECT(hFiredFrom);
	SetFiredFrom( hFiredFrom );

	LOAD_VECTOR(m_vPos);
	LOAD_FLOAT(m_fDamageRadius);
	LOAD_FLOAT(m_fDamageRadiusMin);
    LOAD_FLOAT(m_fMaxDamage);
    LOAD_BYTE_CAST(m_eDamageType, DamageType);
    LOAD_FLOAT(m_fProgDamage);
    LOAD_FLOAT(m_fProgDamageRadius);
    LOAD_FLOAT(m_fProgDamageDuration);
    LOAD_FLOAT(m_fProgDamageLifetime);
    LOAD_BYTE_CAST(m_eProgDamageType, DamageType);
    LOAD_BOOL(m_bRemoveWhenDone);
    LOAD_DWORD(m_nImpactFXId);
	LOAD_FLOAT(m_fImpulse);

	m_ProgDamageTimer.Load( *pMsg );
}

void Explosion::SetFiredFrom( HOBJECT hFiredFrom )
{
	m_hFiredFrom = hFiredFrom;
	if( !m_hFiredFrom )
	{
		m_delegateRemoveClient.Detach();
		m_delegatePlayerSwitched.Detach();
		return;
	}

	// If the firing object is a player, then record the gameclientdata object so we
	// can properly give the client the kill.
	CPlayerObj* pPlayerObj = dynamic_cast< CPlayerObj* >( g_pLTServer->HandleToObject( m_hFiredFrom ));
	if( pPlayerObj )
	{
		GameClientData* pGameClientData = ServerConnectionMgr::Instance().GetGameClientData( pPlayerObj->GetClient( ));
		if( pGameClientData )
		{
			m_delegateRemoveClient.Attach( this, pGameClientData, pGameClientData->RemoveClient );
			m_delegatePlayerSwitched.Attach( this, pGameClientData, pGameClientData->PlayerSwitched );
		}
	}
}

void Explosion::OnPlayerSwitched( Explosion* pExplosion, GameClientData* pGameClientData, EventCaster::NotifyParams& notifyParams )
{
	// Transfer our firedfrom to the client's new player.
	pExplosion->m_hFiredFrom = pGameClientData->GetPlayer( );
}

void Explosion::OnRemoveClient( Explosion* pExplosion, GameClientData* pGameClientData, EventCaster::NotifyParams& notifyParams )
{
	// Clear out our fired from, since our client is gone.
	pExplosion->m_hFiredFrom = NULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CExplosionPlugin::PreHook_EditStringList
//
//	PURPOSE:	Requests a state change
//
// ----------------------------------------------------------------------- //

LTRESULT CExplosionPlugin::PreHook_EditStringList(const char* szRezPath,
												 const char* szPropName,
												 char** aszStrings,
                                                 uint32* pcStrings,
                                                 const uint32 cMaxStrings,
                                                 const uint32 cMaxStringLength)
{
	// See if we can handle the property...

	if( LTStrIEquals( "ImpactFXName", szPropName ))
	{
		CFXDBPlugin::Instance().PreHook_EditStringList(szRezPath, szPropName,
			aszStrings, pcStrings, cMaxStrings, cMaxStringLength);

		if (!CFXDBPlugin::Instance().PopulateStringList(aszStrings, pcStrings,
			 cMaxStrings, cMaxStringLength)) return LT_UNSUPPORTED;

		return LT_OK;
	}
	else if( LTStrIEquals( "DamageType", szPropName ))
	{
		LTASSERT( aszStrings && pcStrings, "Invalid string list or string count!" );
		if( !aszStrings || !pcStrings )
			return LT_UNSUPPORTED;

		// Add an entry for each supported damage type

		for (int i=0; i < kNumDamageTypes; i++)
		{
			LTASSERT( cMaxStrings > (*pcStrings) + 1, "Max limit of strings reached!" );

			DamageType eDT = static_cast<DamageType>(i);
			uint32 dwNameLen = LTStrLen( DamageTypeToString(eDT) );

			if (dwNameLen < cMaxStringLength &&
				((*pcStrings) + 1) < cMaxStrings)
			{
				LTStrCpy( aszStrings[(*pcStrings)++], DamageTypeToString(eDT), cMaxStringLength );
			}
			
		}

		return LT_OK;
	}

	return LT_UNSUPPORTED;
}
