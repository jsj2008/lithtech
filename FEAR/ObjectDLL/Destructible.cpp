// ----------------------------------------------------------------------- //
//
// MODULE  : Destructible.cpp
//
// PURPOSE : Destructible class - Implementation
//
// CREATED : 9/23/97
//
// (c) 1997-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "Destructible.h"
#include "MsgIDs.h"
#include "ServerUtilities.h"
#include "PlayerObj.h"
#include "ObjectMsgs.h"
#include "Camera.h"
#include "AIUtils.h"
#include "AI.h"
#include "SkillDefs.h"
#include "EngineTimer.h"
#include "WeaponDB.h"
#include "sys/win/mpstrconv.h"
#include "GameModeMgr.h"
#include "ServerConnectionMgr.h"

#define MAXIMUM_BLOCK_PRIORITY	255.0f
#define MINIMUM_FRICTION		5.0f
#define MAXIMUM_FRICTION		15.0f
#define MINIMUM_FORCE			0.0f

VarTrack g_vtAIDamageAdjustEasy;
VarTrack g_vtAIDamageAdjustNormal;
VarTrack g_vtAIDamageAdjustHard;
VarTrack g_vtAIDamageAdjustVeryHard;
VarTrack g_vtDamageDebug;
VarTrack g_vtDemoModeInvulnerable;
VarTrack g_vtHealthTest;
VarTrack g_vtAIGodTrack;


extern CGameServerShell* g_pGameServerShell;
extern VarTrack g_DamageScale;
extern VarTrack g_HealScale;
const float DamageStruct::kInfiniteDamage = 100000.0f; // Infinite damage

CMDMGR_BEGIN_REGISTER_CLASS( CDestructible )
	
	ADD_MESSAGE( DESTROY,	1,	NULL,	MSG_HANDLER( CDestructible, HandleDestroyMsg ),	"DESTROY", "TODO:CMDDESC", "TODO:CMDEXP" )
	ADD_MESSAGE( NEVERDESTROY,	2,	NULL,	MSG_HANDLER( CDestructible, HandleNeverDestroyMsg ),	"NEVERDESTROY <1 or 0>", "This message allows you to set the NeverDestroy flag through commands.  This message does not actually destroy the object but allows destruction if NeverDestroy was set to true initially.", "msg WorldModel01 (NEVERDESTROY 1)" )
	ADD_MESSAGE( RESET,		1,	NULL,	MSG_HANDLER( CDestructible, HandleResetMsg ),	"RESET", "TODO:CMDDESC", "TODO:CMDEXP" )

CMDMGR_END_REGISTER_CLASS( CDestructible, IAggregate )

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DamageStruct::DoDamage
//
//	PURPOSE:	Send the MID_DAMAGE message to do the damage
//
// ----------------------------------------------------------------------- //

bool DamageStruct::DoDamage(HOBJECT hDamager, HOBJECT hVictim)
{
    if (!hVictim) return false;

	CAutoMessage cMsg;
	cMsg.Writeuint32(MID_DAMAGE);
	Save(cMsg);
	g_pLTServer->SendToObject(cMsg.Read(), hDamager, hVictim, MESSAGE_GUARANTEED);

    return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DamageStruct::Save
//
//	PURPOSE:	called to save this object to the provided stream
//
// ----------------------------------------------------------------------- //
void DamageStruct::Save(ILTMessage_Write *pMsg)
{
	if (!pMsg) return;

	SAVE_BYTE(eType);
	SAVE_FLOAT(fDamage);
	SAVE_FLOAT(fPenetration);
	SAVE_FLOAT(fDuration);
	SAVE_FLOAT(fImpulseForce);
	SAVE_HOBJECT(hDamager);
	SAVE_HOBJECT(hContainer);
	SAVE_HRECORD( hAmmo );

	//write out the positional information if it is provided
	SAVE_BOOL(bPositionalInfo);
	if(bPositionalInfo)
	{
		SAVE_VECTOR(vDir);
		SAVE_VECTOR(vPos);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DamageStruct::Load
//
//	PURPOSE:	called to load this object from the provided stream
//
// ----------------------------------------------------------------------- //
void DamageStruct::Load(ILTMessage_Read *pMsg)
{
	if (!pMsg) return;

	LOAD_BYTE_CAST(eType, DamageType);
	LOAD_FLOAT(fDamage);
	LOAD_FLOAT(fPenetration);
	LOAD_FLOAT(fDuration);
	LOAD_FLOAT(fImpulseForce);
	LOAD_HOBJECT(hDamager);
	LOAD_HOBJECT(hContainer);
	LOAD_HRECORD( hAmmo, g_pWeaponDB->GetAmmoCategory() );

	//load in the positional information if it was provided
	LOAD_BOOL(bPositionalInfo);
	if(bPositionalInfo)
	{
		//we also need to read the position information
		LOAD_VECTOR(vDir);
		LOAD_VECTOR(vPos);
	}
	else
	{
		//no positional data provided, reset to the defaults
		vDir.Init(0.0f, 1.0f, 0.0f);
		vPos.Init();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::CDestructible
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CDestructible::CDestructible() : IAggregate( "CDestructible" )
{
    m_hObject                   = NULL;
    m_hLastDamager              = NULL;

	m_fMass						= 1.0f;
	m_fHitPoints				= 1.0f;
	m_fMaxHitPoints				= 1.0f;
	m_fArmorPoints				= 0.0f;
	m_fMaxArmorPoints			= 1.0f;
	m_fDeathDamage				= 0.0f;
	m_fDeathImpulseForce		= 0.0f;
	m_fDeathHitNodeImpulseForceScale = 0.0f;
	m_fAccFrameImpulseForce		= 0.0f;
	m_nCurFrameTimeMS				= 0;
	m_fLastDamage				= 0.0f;
	m_fLastArmorAbsorb			= 0.0f;
	m_nLastDamageTimeMS			= 0;
	m_nCantDamageFlags			= 0;
	m_eLastDamageType			= DT_UNSPECIFIED;
	m_eDeathType				= DT_UNSPECIFIED;

	m_vDeathDir.Init();
	m_vLastDamageDir.Init();

	m_fDamagePercentCommand			= 0.0f;
	m_nDamageTriggerCounter			= 0;
	m_nDamageTriggerNumSends		= 1;

	m_iPreventDestructionCount	= 0;

	// damage filter function
    m_pDamageFilterFunction = 0;
    m_pDamageFilterObject = 0;

	m_hDeathAmmo = NULL;

	m_pszDamageCommand = NULL;
	m_pszDamagerMessage = NULL;
	m_pszDeathCommand = NULL;
	m_pszPlayerDeathCommand = NULL;
	m_pszKillerMessage = NULL;
	m_pszDamagePercentCommand = NULL;
	m_pszPlayerDamageCommand = NULL;

	m_DestructibleFlags = kDestructibleFlag_CanHeal | kDestructibleFlag_CanRepair | kDestructibleFlag_CanDamage;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::~CDestructible
//
//	PURPOSE:	Destructible
//
// ----------------------------------------------------------------------- //

CDestructible::~CDestructible()
{
	delete[] m_pszDamageCommand;
	delete[] m_pszDamagerMessage;
	delete[] m_pszDeathCommand;
	delete[] m_pszPlayerDeathCommand;
	delete[] m_pszKillerMessage;
	delete[] m_pszDamagePercentCommand;
	delete[] m_pszPlayerDamageCommand;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::EngineMessageFn
//
//	PURPOSE:	Handle message from the engine
//
// ----------------------------------------------------------------------- //

uint32 CDestructible::EngineMessageFn(LPBASECLASS pObject, uint32 messageID, void *pData, float fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			int nInfo = (int)fData;
			if (nInfo == PRECREATE_WORLDFILE || nInfo == PRECREATE_STRINGPROP)
			{
				ReadProp(pObject, &((ObjectCreateStruct*)pData)->m_cProperties);
			}
		}
		break;

		case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
				InitialUpdate(pObject);
			}
		}
		break;

		case MID_UPDATE:
		{
			Update();
		}
		break;

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
//
//	ROUTINE:	CDestructible::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

uint32 CDestructible::ObjectMessageFn(LPBASECLASS pObject, HOBJECT hSender, ILTMessage_Read *pMsg)
{
	pMsg->SeekTo(0);
	uint32 messageID = pMsg->Readuint32();
	switch(messageID)
	{
		case MID_DAMAGE:
		{
			HandleDamage(pObject, hSender, pMsg);
		}
		break;

		case MID_REPAIR:
		{
			if (CanRepair())
			{
				HandleRepair(pObject, hSender, pMsg);
			}
		}
		break;

		case MID_HEAL:
		{
			if (CanHeal( ))
			{
				HandleHeal(pObject, hSender, pMsg);
			}
		}
		break;

		default : break;
	}

    return IAggregate::ObjectMessageFn(pObject, hSender, pMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

bool CDestructible::ReadProp(LPBASECLASS pObject, const GenericPropList *pProps)
{
	if( !pProps )
		return false;

	SetMass( pProps->GetReal( "Mass", 30.0f ));
	SetHitPoints( pProps->GetReal( "HitPoints", 100.0f ));
	SetMaxHitPoints( m_fHitPoints );
	SetArmorPoints( pProps->GetReal( "Armor", 100.0f ));
	SetMaxArmorPoints( m_fArmorPoints );
	SetDamagePercentCommand( pProps->GetReal( "DamagePercentForCommand", 50.0f ));
	
	delete[] m_pszDamageCommand;
	m_pszDamageCommand		= LTStrDup( pProps->GetCommand( "DamageCommand", "" ));
	delete[] m_pszPlayerDamageCommand;
	m_pszPlayerDamageCommand = LTStrDup( pProps->GetCommand( "PlayerDamageCommand", "" ));
	delete[] m_pszDamagerMessage;
	m_pszDamagerMessage		= LTStrDup( pProps->GetString( "DamagerMessage", "" ));
	delete[] m_pszDamagePercentCommand;
	m_pszDamagePercentCommand	= LTStrDup( pProps->GetCommand( "DamagePercentCommand", "" ));
	delete[] m_pszDeathCommand;
	m_pszDeathCommand		= LTStrDup( pProps->GetCommand( "DeathCommand", "" ));
	delete[] m_pszPlayerDeathCommand;
	m_pszPlayerDeathCommand	= LTStrDup( pProps->GetCommand( "PlayerDeathCommand", "" ));
	delete[] m_pszKillerMessage;
	m_pszKillerMessage		= LTStrDup( pProps->GetString( "KillerMessage", "" ));

	m_nDamageTriggerCounter		= pProps->GetLongInt( "DamageTriggerCounter", m_nDamageTriggerCounter );
	m_nDamageTriggerNumSends	= pProps->GetLongInt( "DamageTriggerNumSends", m_nDamageTriggerNumSends );

	SetCanHeal( pProps->GetBool( "CanHeal", CanHeal( )));
	SetCanRepair( pProps->GetBool( "CanRepair", CanRepair()));
	SetCanDamage( pProps->GetBool( "CanDamage", GetCanDamage()));
	SetNeverDestroy( pProps->GetBool( "NeverDestroy", GetNeverDestroy( )));

	// Set the can't damage flags from the damage mask...
	const char *pszDamageMask = pProps->GetString( "DamageMask", "<none>" );
	DamageFlags df = g_pDTDB->GetDamageMaskFlags( pszDamageMask );
	SetCantDamageFlags( (df == 0 ? 0 : ~df) );

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::Init
//
//	PURPOSE:	Initialize object - no longer used (InitialUpdate() does this)
//
// ----------------------------------------------------------------------- //

bool CDestructible::Init(HOBJECT hObject)
{
	if (!m_hObject) m_hObject = hObject;

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::InitialUpdate
//
//	PURPOSE:	Handle object initial update
//
// ----------------------------------------------------------------------- //

void CDestructible::InitialUpdate(LPBASECLASS pObject)
{
	if (!pObject || !pObject->m_hObject) return;
	if (!m_hObject) m_hObject = pObject->m_hObject;

	// Set the mass, which will also set the blocking priority and friction coefficient...

	SetMass( m_fMass );
	
	// Determine if we are a player object or not

	if (IsPlayer(m_hObject))
	{
        m_DestructibleFlags |= kDestructibleFlag_IsPlayer;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::Update
//
//	PURPOSE:	Handle object updates
//
// ----------------------------------------------------------------------- //

void CDestructible::Update()
{
	// Update progressive damage...Don't allow progressive damage to happen 
	// in cinematics (fixes bugs with the player breaking cinematics)...

	if (Camera::IsActive())
	{
		ClearProgressiveDamage();
	}
	else
	{
		UpdateProgressiveDamage();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::ClearProgressiveDamage
//
//	PURPOSE:	Remove all, or a specific DamageType, progressive damage being done to the object.
//
// ----------------------------------------------------------------------- //

void CDestructible::ClearProgressiveDamage( DamageType DT /*= DT_INVALID*/, bool bUpdateProgressiveDamage /*= true*/ )
{
	if (DT == DT_INVALID)
	{
		ltstd::reset_vector(m_ProgressiveDamage);
	}
	else
	{
		std::remove(m_ProgressiveDamage.begin(), m_ProgressiveDamage.end(), DT);
	}

	if ( bUpdateProgressiveDamage )
	{
		UpdateProgressiveDamage();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::ClearProgressiveDamage
//
//	PURPOSE:	Remove all, or a specific DamageType, progressive damage being done to the object.
//
// ----------------------------------------------------------------------- //

void CDestructible::ClearProgressiveDamage( DamageFlags nDamageFlag )
{
	for ( DamageType i = static_cast< DamageType >( DT_INVALID + 1 );
	      i < kNumDamageTypes;
	      i = static_cast< DamageType >( i + 1 ) )
	{
		if ( DamageTypeToFlag( i ) & nDamageFlag )
		{
			ClearProgressiveDamage( i, false );
		}
	}

	UpdateProgressiveDamage();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::UpdateProgressiveDamage
//
//	PURPOSE:	Update any progressive damage being done to the object.
//
// ----------------------------------------------------------------------- //

void CDestructible::UpdateProgressiveDamage()
{
	DamageStruct damage;

    float fFrameTime = ObjectContextTimer( m_hObject ).GetTimerElapsedS();

	DamageFlags nActiveDamageFlags = 0;
	bool bBleeding = false;

	// Loop over all the progressive damage done to us...

	for (uint32 i=0; i<m_ProgressiveDamage.size();)
	{
		m_ProgressiveDamage[i].fDuration -= fFrameTime;

		// Process the damage for this frame...

		damage = m_ProgressiveDamage[i];
		damage.fDamage = damage.fDamage * fFrameTime;
		if (m_ProgressiveDamage[i].fDuration <= 0.0f)
		{
			//if the duration is <= 0.0f, DoDamage() will interpret it as instant damage
			//   and the wrong client FX will play.
			damage.fDuration = 0.001f;
		}
		DoDamage(damage);

		// Add this to the active damageflags.
		nActiveDamageFlags |= DamageTypeToFlag( damage.eType );

		// Remove the element if expired.
		if (m_ProgressiveDamage[i].fDuration <= 0.0f)
		{
			m_ProgressiveDamage[i] = m_ProgressiveDamage.back();
			m_ProgressiveDamage.resize(m_ProgressiveDamage.size()-1);
		}
		else
		{
			i++;
		}
	}

	//only player specific code below...
	if ( GetIsPlayer( ))
	{
		// Get the player...
		CPlayerObj *pPlayer = dynamic_cast<CPlayerObj*>(g_pLTServer->HandleToObject( m_hObject ));
		if( pPlayer ) 
		{

			// Set our damage flags if we're bleeding/poisoned/stunned/sleeping...
			// [RP] 10/30/02 - Don't set damage flags if we can't be damaged...
			if(GetCanDamage())
			{
				pPlayer->SetDamageFlags( nActiveDamageFlags );
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::AddProgressiveDamage
//
//	PURPOSE:	Add progressive damage to the object
//
// ----------------------------------------------------------------------- //

void CDestructible::AddProgressiveDamage(DamageStruct & damage)
{
    float fLeastDuration = 100000.0f;
	int nIndex = -1;
	
	HOBJECT hDamager	= damage.hDamager;

	// No since adding progressive damage if we are already dead...

	if( IsDead( ))
		return;

	// If the new damage is from a container, see if this container
	// is already damaging us...

	ProgressiveDamageList::iterator it = std::find(m_ProgressiveDamage.begin(), m_ProgressiveDamage.end(), damage.hContainer);
	if (it != m_ProgressiveDamage.end())
	{
		(*it) = damage;
		return;
	}

	m_ProgressiveDamage.push_back(damage);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::IsTakingProgressiveDamage
//
//	PURPOSE:	returns TRUE if currently affected by specified type of progressive damage.
//
// ----------------------------------------------------------------------- //

bool CDestructible::IsTakingProgressiveDamage( DamageType DT )
{
	return std::find(m_ProgressiveDamage.begin(), m_ProgressiveDamage.end(), DT) != m_ProgressiveDamage.end();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::SetMass
//
//	PURPOSE:	Set the blocking priority for this object
//
// ----------------------------------------------------------------------- //

void CDestructible::SetMass(float fMass)
{
	// Make sure we don't try and go above infinite mass as that will mess with the blocking priority...

	m_fMass = Clamp( fMass, 0.0f, INFINITE_MASS - 1.0f );

	SetBlockingPriority();

	// Set the friction based on the mass of the object...

	if (!m_hObject) return;

    g_pPhysicsLT->SetMass(m_hObject, m_fMass);

    float fFricCoeff = MINIMUM_FRICTION + (m_fMass * MAXIMUM_FRICTION / INFINITE_MASS);
	g_pPhysicsLT->SetFrictionCoefficient(m_hObject, fFricCoeff);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::SetBlockingPriority
//
//	PURPOSE:	Set the blocking priority for this object
//
// ----------------------------------------------------------------------- //

void CDestructible::SetBlockingPriority()
{
	if (!m_hObject) return;

    uint8 nPriority = (uint8)(m_fMass * MAXIMUM_BLOCK_PRIORITY / INFINITE_MASS);
	if (nPriority <= 0) nPriority = 1;

    g_pLTServer->SetBlockingPriority(m_hObject, nPriority);
    g_pPhysicsLT->SetForceIgnoreLimit(m_hObject, MINIMUM_FORCE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::GetMaxHitPoints
//
//	PURPOSE:	Returns the maximum hit points for this object
//
// ----------------------------------------------------------------------- //

float CDestructible::GetMaxHitPoints() const
{
    float fAdjustedMax = m_fMaxHitPoints;

	if (GetIsPlayer( ))
	{
        CPlayerObj* pPlayer = (CPlayerObj*)g_pLTServer->HandleToObject(m_hObject);
		if (pPlayer)
		{
			// See if we should increase/decrease the max...
		}
	}

	return fAdjustedMax;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::GetMaxArmorPoints
//
//	PURPOSE:	Returns the maximum armor points for this object
//
// ----------------------------------------------------------------------- //

float CDestructible::GetMaxArmorPoints() const
{
    float fAdjustedMax = m_fMaxArmorPoints;

	if (GetIsPlayer( ))
	{
        CPlayerObj* pPlayer = (CPlayerObj*)g_pLTServer->HandleToObject(m_hObject);
		if (pPlayer)
		{
			// See if we should increase/decrease the max...
		}
	}

	return fAdjustedMax;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::RegisterFilterFunction
//
//	PURPOSE:	Register the filter function
//
// ----------------------------------------------------------------------- //

bool  CDestructible::RegisterFilterFunction( DamageFilterFunction pFn, GameBase *pObject )
{
	if ( ( 0 == pFn ) || ( 0 == pObject ) )
	{
		// if one parameter is non-null, the both must be nonnull
		ASSERT( 0 != pFn );
		ASSERT( 0 != pObject );
	}

	m_pDamageFilterFunction = pFn;
	m_pDamageFilterObject = pObject;

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::HandleDamage
//
//	PURPOSE:	Handle damage message
//
// ----------------------------------------------------------------------- //

void CDestructible::HandleDamage(LPBASECLASS pObject, HOBJECT hSender, ILTMessage_Read *pMsg)
{
	DamageStruct damage;
	damage.InitFromMessage(pMsg);

	// Send things through the filer function, if any...
	if( m_pDamageFilterFunction )
	{
		bool bResult = m_pDamageFilterFunction( m_pDamageFilterObject, &damage );
		if ( !bResult )
		{
			// the filter return false, which means to disregard
			// processing this damage
			return;
		}
	}

	// Check for progressive damage...

	if (damage.fDuration > 0.0f)
	{
		AddProgressiveDamage(damage);
	}
	else
	{
		DoDamage(damage);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::DoDamage
//
//	PURPOSE:	Do the damage
//
// ----------------------------------------------------------------------- //

void CDestructible::DoDamage(DamageStruct & damage)
{
    LTVector vDir		= damage.GetDamageDir();
	float fDamage		= damage.fDamage;
	float fPenetration	= damage.fPenetration;
	DamageType eType	= damage.eType;
	HOBJECT hDamager	= damage.hDamager;

	ILTBaseClass *pObject	= g_pLTServer->HandleToObject( m_hObject );
	ILTBaseClass *pDamager	= g_pLTServer->HandleToObject( hDamager );


	// We want to accumulate all of the damage impulse forces 
	// applied to us over each frame.  This is important for
	// multiple vector weapons.
	uint32 nCurFrameTimeMS = g_pLTServer->GetTimeMS();
	if (m_nCurFrameTimeMS < nCurFrameTimeMS)
	{
		// We're starting a new frame, clear accumulated impulse forces
		// and reset our frame time...
		m_fAccFrameImpulseForce = 0;
		m_nCurFrameTimeMS = nCurFrameTimeMS;
	}

	// Accumulate the force done by the damage...we need to do this here before 
	// the check for IsDead( ) as we want to accumulate force even if we're dead...
	if ( m_nCurFrameTimeMS == nCurFrameTimeMS )
	{
		m_fAccFrameImpulseForce += damage.fImpulseForce;
	}

	// See if we should actually process damage...

	if (IsDead( )) 
	{
		// Update our death impulse force if it has changed...This is possible if we 
		// were killed by a multiple vector weapon with one of the first few vectors 
		// but more vectors still applied a force to us...
		if (m_fAccFrameImpulseForce > m_fDeathImpulseForce)
		{
			m_fDeathImpulseForce = m_fAccFrameImpulseForce;
		}

		return;
	}

	// See if this type of damage applies to us...

	if (m_nCantDamageFlags)
	{
		if (m_nCantDamageFlags & DamageTypeToFlag(eType)) 
		{
			return;
		}
	}
	

	// If the damage is only for worldmodels make sure we are a world model...

	if ( eType == DT_WORLDONLY && !IsWorldModel( m_hObject ) )
	{
		damage.fDamage = 0.0f;
		return;
	}

	// Don't damage us if we are a client only object...
	uint32 dwUserFlags = 0;
	g_pCommonLT->GetObjectFlags( m_hObject, OFT_User, dwUserFlags );
	if( dwUserFlags & USRFLG_CLIENT_RIGIDBODY_ONLY )
		return;
	
	//sanity check on penetration values
	fPenetration = LTCLAMP(fPenetration,0.0f,1.0f);

	// Adjust the damage based on difficulty, who I am and who I damaged...

	fDamage = AdjustDamage(fDamage, hDamager);
	

	// Process damage through any gear items...
	if (GetIsPlayer( ))
	{
		fDamage = ProcessGear(fDamage, eType);
	}

    float fAbsorb = 0.0f;

	// Do rag-doll, and adjust damage if it was greater than 0...(if the damage
	// was zero, we still want to process it)...

	if (fDamage > 0.0f)
	{
		//calculate how much damage gets through the armor
		if (m_fArmorPoints > 0.0 && ArmorCanAbsorb(eType))
		{
			float fPenDamage = fDamage * fPenetration;
			fDamage -= fPenDamage;

			float fAdjustedArmor = m_fArmorPoints;
			fAbsorb = (fDamage > fAdjustedArmor ? fAdjustedArmor : fDamage);
			fDamage	-= fAbsorb;
			fDamage += fPenDamage;
		}
	}

	// [KLS 7/15/02] Negative damage will actually give us hit points, so make sure
	// we don't do that ;)
	if (fDamage < 0.0)
	{
		fDamage = 0.0f;
	}

	// [KLS 7/15/02] If we're a player, now that we have processed gear and armor, 
	// adjust the actual damage done based on our skill level...

	if (GetIsPlayer( ))
	{
        CPlayerObj* pPlayer = (CPlayerObj*) g_pLTServer->HandleToObject(m_hObject);
		if (pPlayer)
		{
			fDamage *= GetSkillValue(eStaminaDamage);
		}

	}


	// Save these for later...

	m_fLastArmorAbsorb	= fAbsorb;
	m_fLastDamage		= fDamage;
	m_eLastDamageType	= eType;
	m_vLastDamageDir	= vDir;
	m_nLastDamageTimeMS	= g_pLTServer->GetTimeMS();
	SetLastDamager(hDamager);


	// Actually apply the damage

	if (!GetCanDamage() || DebugDamageOn())
	{
		return;
	}
	else
	{
		m_fArmorPoints -= fAbsorb;

		if (m_fArmorPoints < 0.0f)
		{
			m_fArmorPoints = 0.0f;
		}

		m_fHitPoints -= fDamage;

		if ( m_nDamageTriggerCounter > 0 )
		{
			m_nDamageTriggerCounter--;
		}
	}

// KLS - ADD DEBUG INFORMATION HERE 
// What was the initial damage amount
// what was the final damage amount (after adjustments)
// how much went to armor (fabsorb)
// how much went to hit points  (fdamage)
// what is final hit points (m_fHitPoints) and final armor (m_fArmorPoints)
// after damage was done

#ifndef _FINAL
	if (IsCharacter( m_hObject ) && g_vtDamageDebug.GetFloat() > 0)
	{
		HitLocation eLoc = HL_UNKNOWN;
		ModelsDB::HNODE hModelNode = NULL;
		if (IsPlayer(m_hObject))
		{
			CPlayerObj* pPlayer = (CPlayerObj*) g_pLTServer->HandleToObject(m_hObject);
			if (pPlayer)
			{
				g_pLTServer->CPrint("Damage Information for player: %s", MPW2A(pPlayer->GetNetUniqueName()).c_str());
				hModelNode = pPlayer->GetModelNodeLastHit();
			}
		}
		else if (IsAI(m_hObject))
		{
			CAI* pAI = (CAI*) g_pLTServer->HandleToObject(m_hObject);
			if (pAI)
			{
				g_pLTServer->CPrint("Damage Information for player: %s", pAI->GetName());
				hModelNode = pAI->GetModelNodeLastHit();
			}
		}
		else
		{
			g_pLTServer->CPrint("Damage Information:");
		}

		if( hModelNode )
		{
			eLoc = g_pModelsDB->GetNodeLocation( hModelNode );
		}

		g_pLTServer->CPrint("  Hit Location             = %s", StringFromHitLocation(eLoc));
		if (hModelNode)
		{
			g_pLTServer->CPrint("          Node             = %s", g_pModelsDB->GetNodeName(hModelNode));
		}
		g_pLTServer->CPrint("  Initial Damage Amount    = %.2f", damage.fDamage);
		g_pLTServer->CPrint("  Adjusted Damage Amount   = %.2f", fDamage);
		g_pLTServer->CPrint("  Damage absorbed by Armor = %.2f", fAbsorb);
		g_pLTServer->CPrint("  Final Hit Points         = %.2f", m_fHitPoints);
		g_pLTServer->CPrint("  Final Armor Points       = %.2f", m_fArmorPoints);
	}

#endif // _FINAL

	// If this is supposed to send a damage trigger, send it now...

	if (m_nDamageTriggerCounter <= 0)
	{
		if (m_nDamageTriggerNumSends != 0)
		{
			m_nDamageTriggerNumSends--;

			if( !LTStrEmpty( m_pszDamageCommand ))
			{
				g_pCmdMgr->QueueCommand( m_pszDamageCommand, m_hLastDamager, m_hObject );
			}

			if( !LTStrEmpty( m_pszPlayerDamageCommand ) != 0 && IsPlayer( hDamager ))
			{
				g_pCmdMgr->QueueCommand( m_pszPlayerDamageCommand, m_hLastDamager, m_hObject);
			}
		}
	}

	// If we ought to send a percentage based damage command, do it now.

	if ( !LTStrEmpty( m_pszDamagePercentCommand ) && (m_fHitPoints < m_fMaxHitPoints * m_fDamagePercentCommand) )
	{
		g_pCmdMgr->QueueCommand( m_pszDamagePercentCommand, m_hLastDamager, m_hObject );
	}
	
	// Send message to object that damaged us...

	if (hDamager && !LTStrEmpty( m_pszDamagerMessage ))
	{
		g_pCmdMgr->QueueMessage( pDamager, pDamager, m_pszDamagerMessage );
	}
	
	// Send instant damage flags for characters...

	if( !(damage.fDuration > 0.0f) )
	{
		CCharacter *pChar = CCharacter::DynamicCast( m_hObject );
		if( pChar )
		{
			pChar->SetInstantDamageFlags( DamageTypeToFlag( eType ));
		}
	}


#ifndef _FINAL
	// Don't kill AI if AIGod is set.
	if( IsAI( m_hObject ) )
	{
		if (!g_vtAIGodTrack.IsInitted())
		{
			g_vtAIGodTrack.Init(g_pLTServer, "AIGod", NULL, 0.0f);
		}

		if( g_vtAIGodTrack.GetFloat() > 0.0f )
		{
			return;
		}
	}
#endif // _FINAL


	// See if we're dead...

	if (m_fHitPoints <= 1.0f && !GetNeverDestroy( ) && 0==m_iPreventDestructionCount && ( fDamage > 0.f ) )
	{
		if (GetIsPlayer( ))
		{
			//in health test mode allow damage but not death, and notify player that they would have died
			if (!IsMultiplayerGameServer() && g_vtHealthTest.GetFloat() > 0.0f)
			{
				// Give us full hit points...
				m_fHitPoints = 100.0f;

				CPlayerObj* pPlayer = dynamic_cast< CPlayerObj* >( g_pLTServer->HandleToObject( m_hObject));
				if (!pPlayer) return;

				CAutoMessage cMsg;
				cMsg.Writeuint8(MID_PLAYER_EVENT);
				cMsg.Writeuint8(kPEHealthTestMode);
				g_pLTServer->SendToClient(cMsg.Read(), pPlayer->GetClient(), MESSAGE_GUARANTEED);

				return;
			}

			//in demo mode allow damage but not death...
			if (!IsMultiplayerGameServer() && g_vtDemoModeInvulnerable.GetFloat() > 0.0f)
			{
				// Give us a couple hit points...
				m_fHitPoints = 2.0f;
				return;
			}
		}

		m_fDeathDamage	= fDamage;
		m_eDeathType	= eType;
		m_vDeathDir		= vDir;

		// Use the impulse force that has been accumulated over the current frame
		// for our death force.  We want to use the accumulated force in case we
		// were killed by a multiple vector weapon with one of the last few vectors. 
		// In this case we want the earlier vectors' forces to be accounted for in the 
		// death force...
		m_fDeathImpulseForce = m_fAccFrameImpulseForce;

		// Cache the ammo type that delivered the death damage...
		m_hDeathAmmo = damage.hAmmo;

		// Save off the amount we should use to scale the impulse force associated with
		// the hit node...
		if (damage.hAmmo)
		{
			HAMMODATA hAmmoData = g_pWeaponDB->GetAmmoData(damage.hAmmo, IsAI(damage.hDamager));
			m_fDeathHitNodeImpulseForceScale = g_pWeaponDB->GetFloat(hAmmoData, WDB_AMMO_fHitNodeImpulseForceScale);
		}


		HandleDestruction(hDamager);

		if (GetIsPlayer( ))
		{
			ProcessPlayerDeath(hDamager,damage.hAmmo);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::SetLastDamager
//
//	PURPOSE:	Save our last damager
//
// ----------------------------------------------------------------------- //

void CDestructible::SetLastDamager(HOBJECT hDamager)
{
	// Save info on the guy who hit us...
	m_hLastDamager = hDamager;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::ProcessPlayerDeath
//
//	PURPOSE:	Process the death of a player
//
// ----------------------------------------------------------------------- //

void CDestructible::ProcessPlayerDeath(HOBJECT hDamager, HAMMO hAmmo)
{
	if (!GetIsPlayer( )) return;

	// Stop player movement if dead...

	LTVector vZero;
	vZero.Init();
	g_pPhysicsLT->SetVelocity(m_hObject, vZero);

	CPlayerObj* pVictim = dynamic_cast< CPlayerObj* >( g_pLTServer->HandleToObject( m_hObject));
	if (!pVictim) return;

	pVictim->GetMissionStats()->dwNumTimesKilled++;

	bool bKillerIdentified = false;

	// If another player killed us, give them the score.
	CPlayerObj* pPlayerKiller = dynamic_cast< CPlayerObj* >( g_pLTServer->HandleToObject( hDamager ));
	if( pPlayerKiller )
	{

		bKillerIdentified = true;
		// Check if we killed ourselves.
		if (pVictim == pPlayerKiller)
		{
			GameClientData* pGameClientData = ServerConnectionMgr::Instance().GetGameClientData( pVictim->GetClient( ));
			if( pGameClientData && !pGameClientData->RequestedTeamChange( ))
				pGameClientData->GetPlayerScore()->AddEvent(CPlayerScore::eSuicide);
		}
		else
		{
			if( GameModeMgr::Instance( ).m_grbUseTeams )
			{
				if( pPlayerKiller->GetTeamID() == pVictim->GetTeamID() )
				{
					// The player is either stupid or a jack-ass so penalize him and the team...
					GameClientData* pPlayerKillerGameClientData = ServerConnectionMgr::Instance().GetGameClientData( pPlayerKiller->GetClient( ));
					if( pPlayerKillerGameClientData )
						pPlayerKillerGameClientData->GetPlayerScore()->AddEvent(CPlayerScore::eTeamKill);
				}
				else
				{
					// Only get a death score for non-team kills to avoid griefers.
					GameClientData* pGameClientData = ServerConnectionMgr::Instance().GetGameClientData( pVictim->GetClient( ));
					if( pGameClientData )
						pGameClientData->GetPlayerScore()->AddEvent(CPlayerScore::eDeath);

					// The player killed another player of a different team so this is valid...
					GameClientData* pPlayerKillerGameClientData = ServerConnectionMgr::Instance().GetGameClientData( pPlayerKiller->GetClient( ));
					if( pPlayerKillerGameClientData )
						pPlayerKillerGameClientData->GetPlayerScore()->AddEvent(CPlayerScore::eKill);
				}
			}
			else
			{
				GameClientData* pGameClientData = ServerConnectionMgr::Instance().GetGameClientData( pVictim->GetClient( ));
				if( pGameClientData )
					pGameClientData->GetPlayerScore()->AddEvent(CPlayerScore::eDeath);

				GameClientData* pPlayerKillerGameClientData = ServerConnectionMgr::Instance().GetGameClientData( pPlayerKiller->GetClient( ));
				if( pPlayerKillerGameClientData )
					pPlayerKillerGameClientData->GetPlayerScore()->AddEvent(CPlayerScore::eKill);
			}
		}
	}

	if( !bKillerIdentified )
	{
		// If the killer was not an AI, then assume we killed ourselves.
		CAI* pAIKiller = dynamic_cast< CAI* >( g_pLTServer->HandleToObject( hDamager ));
		if( pAIKiller )
		{
			bKillerIdentified = true;
		}
		else
		{
			// No killer, so they must have killed themselves, but don't count it against them
			//	if they were switching teams
			GameClientData* pGameClientData = ServerConnectionMgr::Instance().GetGameClientData( pVictim->GetClient( ));
			if( pGameClientData && !pGameClientData->RequestedTeamChange( ))
				pGameClientData->GetPlayerScore()->AddEvent(CPlayerScore::eSuicide);
		}
	}


	if (g_pGameServerShell)
	{
		g_pGameServerShell->SetUpdateGameServ();
	}

	GameClientData* pGameClientData = ServerConnectionMgr::Instance().GetGameClientData( pVictim->GetClient( ));
	if( pGameClientData && !pGameClientData->RequestedTeamChange( ))
	{
		HCLIENT hClientVictim = pVictim->GetClient();
		HCLIENT hClientKiller = ( pPlayerKiller ) ? pPlayerKiller->GetClient() : NULL;
		if (hClientVictim )
		{
			uint32 nVictimID = g_pLTServer->GetClientID (hClientVictim);
			uint32 nKillerID = ( hClientKiller ) ? g_pLTServer->GetClientID (hClientKiller) : (uint32)-1;

			bool	bHeadshot = false;
			
			ModelsDB::HNODE hModelNode = pVictim->GetModelNodeLastHit();
			if( hModelNode )
			{
				bHeadshot = (HL_HEAD == g_pModelsDB->GetNodeLocation( hModelNode ));
			}

			CAutoMessage cMsg;
			cMsg.Writeuint8(MID_PLAYER_SCORED);
			cMsg.Writeuint32(nVictimID);
			cMsg.Writeuint32(nKillerID);
			cMsg.WriteDatabaseRecord(g_pLTDatabase,hAmmo);
			cMsg.Writebool(bHeadshot);
			g_pLTServer->SendToClient(cMsg.Read(), NULL, MESSAGE_GUARANTEED);

			// Notify kill event.
			CPlayerObj::PlayerScoredKillEventParams cParams( CPlayerObj::PlayerScoredKillEvent, m_hObject, hDamager, bHeadshot, hAmmo );			
			CPlayerObj::PlayerScoredKillEvent.DoNotify(cParams);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::ProcessGear
//
//	PURPOSE:	Adjust damage according to what gear items we have
//
// ----------------------------------------------------------------------- //

float CDestructible::ProcessGear(float fDamage, DamageType eDamageType)
{
	CPlayerObj *pPlayer = dynamic_cast<CPlayerObj*>(g_pLTServer->HandleToObject( m_hObject ));
	if( pPlayer ) 
	{
		// See if we have protection from this damage...
		float fProtection = pPlayer->GetInventory()->GetGearProtection(eDamageType);
		fDamage *= (1.0f - fProtection);
	}

	return fDamage;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::HandleDestruction
//
//	PURPOSE:	Handle destruction
//
// ----------------------------------------------------------------------- //

void CDestructible::HandleDestruction(HOBJECT hDamager)
{
	if (IsDead( )) return;

    m_DestructibleFlags |= kDestructibleFlag_Dead;

    ILTBaseClass *pDamager = g_pLTServer->HandleToObject(m_hObject);

	if( !LTStrEmpty( m_pszDeathCommand ))
	{
		g_pCmdMgr->QueueCommand( m_pszDeathCommand, m_hLastDamager, m_hObject );
	}
	
	if( !LTStrEmpty( m_pszPlayerDeathCommand ) && IsPlayer( hDamager ))
	{
		g_pCmdMgr->QueueCommand( m_pszPlayerDeathCommand, m_hLastDamager, m_hObject );
	}

	// Send message to object that killed us...

	if(hDamager && !LTStrEmpty( m_pszKillerMessage ))
	{
		g_pCmdMgr->QueueMessage( pDamager, pDamager, m_pszKillerMessage );
	}

	if (IsPlayer(hDamager) && IsCharacter(m_hObject) && !GetIsPlayer( ))
	{
        CCharacter* pChar = (CCharacter*) g_pLTServer->HandleToObject(m_hObject);
        CPlayerObj* pPlayer = (CPlayerObj*) g_pLTServer->HandleToObject(hDamager);
		if (pPlayer && pChar)
		{
			//TODO: verify alignment
			pPlayer->GetMissionStats()->dwNumEnemyKills++;
		}
	}

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::HandleHeal
//
//	PURPOSE:	Handle heal message.
//
// ----------------------------------------------------------------------- //

void CDestructible::HandleHeal(LPBASECLASS pObject, HOBJECT hSender, ILTMessage_Read *pMsg)
{
	if (IsDead( )) return;

    float fAmount = pMsg->Readfloat() * g_HealScale.GetFloat(1.0f);

	// See if we can heal

 	bool bHealed = Heal(fAmount);

	CAutoMessage cMsg;
	cMsg.Writeuint32(MID_PICKEDUP);
	cMsg.Writebool(bHealed);
	cMsg.Writebool( false); // bWeaponsStay
	g_pLTServer->SendToObject(cMsg.Read(), pObject->m_hObject, hSender, MESSAGE_GUARANTEED);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::Heal
//
//	PURPOSE:	Add some value to hit points
//
// ----------------------------------------------------------------------- //

bool CDestructible::Heal(float fAmount)
{
    float fMax = GetMaxHitPoints();
    if (m_fHitPoints >= fMax) return false;

	// only heal what we need to (cap hit points at maximum)

	if (m_fHitPoints + fAmount > fMax)
	{
		fAmount = fMax - m_fHitPoints;
	}

	// now actually heal the object

	DoActualHealing (fAmount);

    return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::DoActualHealing()
//
//	PURPOSE:	Simply adds a value to the hit points variable
//
// ----------------------------------------------------------------------- //

void CDestructible::DoActualHealing(float fAmount)
{
	// NOTE: This function should only be called directly from the ultra heal
	//		 powerup code, as it does not do any bounds checking on the hit points.
	//       All other healing should be done through the HandleHeal() function above.

	if(!CanHeal( )) return;

    m_fHitPoints += fAmount;

	if (m_fHitPoints > 1.0f)
	{
		SetDead( false );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::HandleRepair()
//
//	PURPOSE:	Handle Repair message
//
// ----------------------------------------------------------------------- //

void CDestructible::HandleRepair(LPBASECLASS pObject, HOBJECT hSender, ILTMessage_Read *pMsg)
{
    float fAmount = pMsg->Readfloat();

 	bool bRepaired = Repair(fAmount);

	CAutoMessage cMsg;
	cMsg.Writeuint32(MID_PICKEDUP);
	cMsg.Writebool( bRepaired );
	cMsg.Writebool( false ); // bWeaponsStay
	g_pLTServer->SendToObject(cMsg.Read(), pObject->m_hObject, hSender, MESSAGE_GUARANTEED);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::Repair()
//
//	PURPOSE:	Add some value to armor
//
// ----------------------------------------------------------------------- //
bool CDestructible::Repair(float fAmount)
{
    if (!CanRepair()) return false;

    float fMax = GetMaxArmorPoints();
    if (m_fArmorPoints >= fMax) return false;

	m_fArmorPoints += fAmount;

	if (m_fArmorPoints > fMax)
	{
		m_fArmorPoints = fMax;
	}

	if (IsCharacter(m_hObject))
	{
		CCharacter *pChar = dynamic_cast<CCharacter*>(g_pLTServer->HandleToObject( m_hObject ));
		pChar->UpdateSurfaceFlags();
	}

    return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::Reset
//
//	PURPOSE:	Reset
//
// ----------------------------------------------------------------------- //

void CDestructible::Reset( float fHitPts, float fArmorPts )
{
	float fMaxHitPoints		= GetMaxHitPoints();
	float fMaxArmorPoints	= GetMaxArmorPoints();

	m_fHitPoints	= fHitPts <= fMaxHitPoints ? fHitPts : fMaxHitPoints;
	m_fArmorPoints	= fArmorPts <= fMaxArmorPoints ? fArmorPts : fMaxArmorPoints;
	m_eDeathType	= DT_UNSPECIFIED;
	m_fDeathDamage	= 0.0f;
	SetDead( false );

	m_fDeathImpulseForce = 0.0f;
	m_fAccFrameImpulseForce = 0.0f;
	m_nCurFrameTimeMS	= 0;
	m_fDeathHitNodeImpulseForceScale = 0.0f;

	m_eLastDamageType = DT_UNSPECIFIED;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::HandleDestroyMsg
//
//	PURPOSE:	Handle a DESTROY message...
//
// ----------------------------------------------------------------------- //

void CDestructible::HandleDestroyMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	DamageStruct damage;

	damage.fDamage	= damage.kInfiniteDamage;
	damage.hDamager = hSender;

	damage.DoDamage( m_hObject, m_hObject);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::HandleNeverDestroyMsg
//
//	PURPOSE:	Handle a NEVERDESTROY message...
//
// ----------------------------------------------------------------------- //

void CDestructible::HandleNeverDestroyMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	if( crParsedMsg.GetArgCount() > 1 )
	{
		SetNeverDestroy( IsTrueChar( *crParsedMsg.GetArg(1) ));
	}	
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::HandleResetMsg
//
//	PURPOSE:	Handle a RESET message...
//
// ----------------------------------------------------------------------- //

void CDestructible::HandleResetMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	Reset(GetMaxHitPoints(), GetMaxArmorPoints());
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::DebugDamageOn
//
//	PURPOSE:	See if the object can be damaged
//
// ----------------------------------------------------------------------- //

bool CDestructible::DebugDamageOn()
{
    if (!m_hObject) return false;

    HCONSOLEVAR hVar  = g_pLTServer->GetConsoleVariable("SetDamage");
    const char* pVal = g_pLTServer->GetConsoleVariableString(hVar);

    if (!pVal) return false;

    return ((bool) !atoi(pVal));
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::AdjustDamage
//
//	PURPOSE:	Adjust the damage if necessary
//
// ----------------------------------------------------------------------- //

float CDestructible::AdjustDamage( float fDamage, HOBJECT hDamager )
{
	// Only do following adjustments for characters damaging characters...

	if( !IsCharacter( m_hObject ) || !IsCharacter( hDamager ) )
	{
		return fDamage;
	}



	// AI damaging a player...

	if( IsAI( hDamager ) && IsPlayer( m_hObject ))
	{

		float fDifficultyFactor = 1.0f;
    		
		// If we're an AI damage is based on the current difficutly setting...

		switch (g_pGameServerShell->GetDifficulty())
		{
			case GD_EASY:
				fDifficultyFactor = g_vtAIDamageAdjustEasy.GetFloat();
			break;

			case GD_NORMAL:
				fDifficultyFactor = g_vtAIDamageAdjustNormal.GetFloat();
			break;

			case GD_HARD:
				fDifficultyFactor = g_vtAIDamageAdjustHard.GetFloat();
			break;

			case GD_VERYHARD:
				fDifficultyFactor = g_vtAIDamageAdjustVeryHard.GetFloat();
			break;

			default :
			break;
		}

		return fDamage * fDifficultyFactor;
	}


	return fDamage;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CDestructible::Save(ILTMessage_Write *pMsg, uint8 nType)
{
	if (!pMsg) return;

	SAVE_HOBJECT(m_hObject);
	SAVE_HOBJECT(m_hLastDamager);

	SAVE_FLOAT(m_fHitPoints);
	SAVE_FLOAT(m_fMaxHitPoints);
	SAVE_FLOAT(m_fArmorPoints);
	SAVE_FLOAT(m_fMaxArmorPoints);
	SAVE_FLOAT(m_fMass);
	SAVE_FLOAT(m_fLastDamage);
	SAVE_FLOAT(m_fLastArmorAbsorb);

	SAVE_TIME_MS(m_nLastDamageTimeMS);
	SAVE_FLOAT(m_fDeathDamage);
	SAVE_FLOAT(m_fDeathHitNodeImpulseForceScale);
	SAVE_FLOAT(m_fDeathImpulseForce);
	SAVE_FLOAT(m_fAccFrameImpulseForce);
	SAVE_DWORD(m_nCurFrameTimeMS);
	SAVE_BYTE( m_DestructibleFlags );
	SAVE_INT(m_iPreventDestructionCount);
	SAVE_BYTE(m_eDeathType);
	SAVE_BYTE(m_eLastDamageType);
	SAVE_DWORD(m_nDamageTriggerCounter);
	SAVE_DWORD(m_nDamageTriggerNumSends);
	SAVE_TYPE(m_nCantDamageFlags);

	SAVE_CHARSTRING( m_pszDamageCommand );
	SAVE_CHARSTRING( m_pszDamagerMessage );
	SAVE_CHARSTRING( m_pszDeathCommand );
	SAVE_CHARSTRING( m_pszPlayerDeathCommand );
	SAVE_CHARSTRING( m_pszKillerMessage );
	SAVE_CHARSTRING( m_pszDamagePercentCommand );
	SAVE_CHARSTRING( m_pszPlayerDamageCommand );
 
	SAVE_VECTOR(m_vDeathDir);
	SAVE_VECTOR(m_vLastDamageDir);
	SAVE_FLOAT( m_fDamagePercentCommand );

	SAVE_INT(m_ProgressiveDamage.size());
	for (ProgressiveDamageList::iterator it=m_ProgressiveDamage.begin(); it!=m_ProgressiveDamage.end(); ++it)
	{
		it->Save(pMsg);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CDestructible::Load(ILTMessage_Read *pMsg, uint8 nType)
{
	if (!pMsg) return;

	LOAD_HOBJECT(m_hObject);
	LOAD_HOBJECT(m_hLastDamager);
	LOAD_FLOAT(m_fHitPoints);
	LOAD_FLOAT(m_fMaxHitPoints);
	LOAD_FLOAT(m_fArmorPoints);
	LOAD_FLOAT(m_fMaxArmorPoints);
	LOAD_FLOAT(m_fMass);
	LOAD_FLOAT(m_fLastDamage);
	LOAD_FLOAT(m_fLastArmorAbsorb);
	LOAD_TIME_MS(m_nLastDamageTimeMS);
	LOAD_FLOAT(m_fDeathDamage);
	LOAD_FLOAT(m_fDeathHitNodeImpulseForceScale);
	LOAD_FLOAT(m_fDeathImpulseForce);
	LOAD_FLOAT(m_fAccFrameImpulseForce);
	LOAD_DWORD(m_nCurFrameTimeMS);
	LOAD_BYTE( m_DestructibleFlags );
	LOAD_INT(m_iPreventDestructionCount);
	LOAD_BYTE_CAST(m_eDeathType, DamageType);
	LOAD_BYTE_CAST(m_eLastDamageType, DamageType);
	LOAD_DWORD(m_nDamageTriggerCounter);
	LOAD_DWORD(m_nDamageTriggerNumSends);
	LOAD_TYPE(m_nCantDamageFlags);

	char szString[1024] = {0};
	LOAD_CHARSTRING( szString, ARRAY_LEN( szString ));
	delete[] m_pszDamageCommand;
	m_pszDamageCommand = LTStrDup( szString );
	LOAD_CHARSTRING( szString, ARRAY_LEN( szString ));
	delete[] m_pszDamagerMessage;
	m_pszDamagerMessage = LTStrDup( szString );
	LOAD_CHARSTRING( szString, ARRAY_LEN( szString ));
	delete[] m_pszDeathCommand;
	m_pszDeathCommand = LTStrDup( szString );
	LOAD_CHARSTRING( szString, ARRAY_LEN( szString ));
	delete[] m_pszPlayerDeathCommand;
	m_pszPlayerDeathCommand = LTStrDup( szString );
	LOAD_CHARSTRING( szString, ARRAY_LEN( szString ));
	delete[] m_pszKillerMessage;
	m_pszKillerMessage = LTStrDup( szString );
	LOAD_CHARSTRING( szString, ARRAY_LEN( szString ));
	delete[] m_pszDamagePercentCommand;
	m_pszDamagePercentCommand = LTStrDup( szString );
	LOAD_CHARSTRING( szString, ARRAY_LEN( szString ));
	delete[] m_pszPlayerDamageCommand;
	m_pszPlayerDamageCommand = LTStrDup( szString );
	
	LOAD_VECTOR(m_vDeathDir);
	LOAD_VECTOR(m_vLastDamageDir);
	LOAD_FLOAT( m_fDamagePercentCommand );

	int nProgressiveDamage;
	LOAD_INT(nProgressiveDamage);
	m_ProgressiveDamage.resize(nProgressiveDamage);
	for (ProgressiveDamageList::iterator it=m_ProgressiveDamage.begin(); it!=m_ProgressiveDamage.end(); ++it)
	{
		it->Load(pMsg);
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CDestructiblePlugin::PreHook_PropChanged
//
//  PURPOSE:	Check our command strings
//
// ----------------------------------------------------------------------- //

LTRESULT CDestructiblePlugin::PreHook_PropChanged( const char *szObjName,
													   const char *szPropName, 
													   const int  nPropType, 
													   const GenericProp &gpPropValue,
													   ILTPreInterface *pInterface,
													   const char *szModifiers)
{
	// Check if the props are our commands and then just send it to the CommandMgr..

	if( !LTStrICmp( "DamageCommand", szPropName ) ||
		!LTStrICmp( "DeathCommand", szPropName ) ||
		!LTStrICmp( "PlayerDamageCommand", szPropName ) ||
		!LTStrICmp( "PlayerDeathCommand", szPropName ) )
	{
		if( m_CommandMgrPlugin.PreHook_PropChanged( szObjName, 
													szPropName, 
													nPropType, 
													gpPropValue,
													pInterface,
													szModifiers ) == LT_OK )
		{
			return LT_OK;
		}
	}

	return LT_UNSUPPORTED;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CDestructiblePlugin::PreHook_EditStringList
//
//  PURPOSE:	Fill out the string lists...
//
// ----------------------------------------------------------------------- //

LTRESULT CDestructiblePlugin::PreHook_EditStringList( const char *szRezPath,
													  const char *szPropName,
													  char **aszStrings,
													  uint32 *pcStrings,
													  const uint32 cMaxStrings,
													  const uint32 cMaxStringLength )
{
	if( LTStrIEquals( szPropName, "DamageMask" ))
	{
		if( CategoryPlugin::Instance( ).PopulateStringList( g_pDTDB->GetDamageMasksCategory( ), aszStrings,
															pcStrings, cMaxStringLength, cMaxStringLength ))
		{
			return LT_OK;
		}
	}

	return LT_UNSUPPORTED;
}
