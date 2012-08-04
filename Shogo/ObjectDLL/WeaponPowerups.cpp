// ----------------------------------------------------------------------- //
//
// MODULE  : WeaponPowerups.cpp
//
// PURPOSE : Riot weapon powerups - Implementation
//
// CREATED : 10/7/97
//
// ----------------------------------------------------------------------- //

#include "WeaponPowerups.h"
#include "RiotMsgIds.h"
#include "cpp_server_de.h"
#include "WeaponDefs.h"
#include "BaseCharacter.h"

#define UPDATE_DELTA	0.1f

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //
//
//	CLASS:		WeaponPowerup
//
//	PURPOSE:	Large Weapon powerups
//
// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

BEGIN_CLASS(WeaponPowerup)
	ADD_BOOLPROP_FLAG(Rotate, 0, PF_HIDDEN)
	ADD_REALPROP(RespawnTime, 30.0f)
	ADD_STRINGPROP_FLAG(SoundFile, "", PF_HIDDEN)
	ADD_LONGINTPROP( WeaponType, 0 )
	ADD_LONGINTPROP( Ammo, 0 )
	ADD_BOOLPROP( Small, 0 )
	ADD_BOOLPROP( Large, 0 )
	ADD_VECTORPROP_VAL_FLAG( Dims, 10.0f, 25.0f, 10.0f, PF_HIDDEN | PF_DIMS )
END_CLASS_DEFAULT(WeaponPowerup, Powerup, NULL, NULL)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WeaponPowerup::WeaponPowerup
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

WeaponPowerup::WeaponPowerup() : Powerup()
{ 
	m_dwAmmo		= 0; 
	m_iWeaponType	= 0;
	m_eModelSize	= MS_NORMAL;

	m_bInformClient	= DFALSE;
	m_bBounce = DFALSE;

	AddAggregate(&m_bounce);
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WeaponPowerup::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD WeaponPowerup::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return DFALSE;

	switch(messageID)
	{
		case MID_UPDATE:
		{
			DDWORD dwRet = Powerup::EngineMessageFn(messageID, pData, fData);
			if (m_bounce.IsDoneBouncing())
			{
				pServerDE->SetNextUpdate(m_hObject, 0.0f);
			}
			else
			{
				pServerDE->SetNextUpdate(m_hObject, UPDATE_DELTA);
			}

			return dwRet;
		}
		break;

		case MID_PRECREATE:
		{
			DDWORD dwRet = Powerup::EngineMessageFn(messageID, pData, fData);

			if ( fData == 1.0f || fData == 2.0f )
				ReadProp(( ObjectCreateStruct * )pData );

			PostPropRead(( ObjectCreateStruct * )pData );

			return dwRet;
		}
		break;

		case MID_INITIALUPDATE:
		{
			DDWORD dwRet = Powerup::EngineMessageFn(messageID, pData, fData);

			if (fData != INITIALUPDATE_SAVEGAME)
			{
				InitialUpdate(pServerDE);
			}

			return dwRet;
		}

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

	return Powerup::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WeaponPowerup::ReadProp
//
//	PURPOSE:	Read object properties
//
// ----------------------------------------------------------------------- //
void WeaponPowerup::ReadProp( ObjectCreateStruct *pStruct )
{
	GenericProp genProp;

	if( g_pServerDE->GetPropGeneric( "Ammo", &genProp ) == DE_OK )
		m_dwAmmo = ( DDWORD )genProp.m_Long;
	if( g_pServerDE->GetPropGeneric( "WeaponType", &genProp ) == DE_OK)
		m_iWeaponType = ( DBYTE )genProp.m_Long;
	if( g_pServerDE->GetPropGeneric( "Small", &genProp ) == DE_OK)
		m_eModelSize = genProp.m_Bool ? MS_SMALL : m_eModelSize;
	if( g_pServerDE->GetPropGeneric( "Large", &genProp ) == DE_OK)
		m_eModelSize = genProp.m_Bool ? MS_LARGE : m_eModelSize;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WeaponPowerup::PostPropRead
//
//	PURPOSE:	Handle post property read engine messages
//
// ----------------------------------------------------------------------- //
void WeaponPowerup::PostPropRead( ObjectCreateStruct *pStruct )
{
	if (pStruct)
	{
		switch( m_iWeaponType )
		{
			case GUN_PULSERIFLE_ID:
			{
				SAFE_STRCPY(pStruct->m_Filename, "Models\\powerups\\PulseRifle.abc");
				SAFE_STRCPY(pStruct->m_SkinName, "Skins\\powerups\\PulseRifle_a.dtx");
				break;
			}
			case GUN_SPIDER_ID:
			{
				SAFE_STRCPY(pStruct->m_Filename, "Models\\powerups\\Spider.abc");
				SAFE_STRCPY(pStruct->m_SkinName, "Skins\\powerups\\Spider_a.dtx");
				break;
			}
			case GUN_BULLGUT_ID:
			{
				SAFE_STRCPY(pStruct->m_Filename, "Models\\powerups\\Bullgut.abc");
				SAFE_STRCPY(pStruct->m_SkinName, "Skins\\powerups\\Bullgut_a.dtx");
				break;
			}
			case GUN_SNIPERRIFLE_ID:
			{
				SAFE_STRCPY(pStruct->m_Filename, "Models\\powerups\\SniperRifle.abc");
				SAFE_STRCPY(pStruct->m_SkinName, "Skins\\powerups\\SniperRifle_a.dtx");
				break;
			}
			case GUN_JUGGERNAUT_ID:
			{
				SAFE_STRCPY(pStruct->m_Filename, "Models\\powerups\\Juggernaut.abc");
				SAFE_STRCPY(pStruct->m_SkinName, "Skins\\powerups\\Juggernaut_a.dtx");
				break;
			}
			case GUN_SHREDDER_ID:
			{
				SAFE_STRCPY(pStruct->m_Filename, "Models\\powerups\\Shredder.abc");
				SAFE_STRCPY(pStruct->m_SkinName, "Skins\\powerups\\Shredder_a.dtx");
				break;
			}
			case GUN_REDRIOT_ID:
			{
				SAFE_STRCPY(pStruct->m_Filename, "Models\\powerups\\RedRiot.abc");
				SAFE_STRCPY(pStruct->m_SkinName, "Skins\\powerups\\RedRiot_a.dtx");
				break;
			}

			case GUN_COLT45_ID:
			{
				SAFE_STRCPY(pStruct->m_Filename, "Models\\powerups\\Colt45.abc");
				SAFE_STRCPY(pStruct->m_SkinName, "Skins\\powerups\\Colt45_a.dtx");
				break;
			}
			case GUN_SHOTGUN_ID:
			{
				SAFE_STRCPY(pStruct->m_Filename, "Models\\powerups\\Shotgun.abc");
				SAFE_STRCPY(pStruct->m_SkinName, "Skins\\powerups\\Shotgun_a.dtx");
				break;
			}
			case GUN_ASSAULTRIFLE_ID:
			{
				SAFE_STRCPY(pStruct->m_Filename, "Models\\powerups\\AssaultRifle.abc");
				SAFE_STRCPY(pStruct->m_SkinName, "Skins\\powerups\\AssaultRifle_a.dtx");
				break;
			}
			case GUN_ENERGYGRENADE_ID:
			{
				SAFE_STRCPY(pStruct->m_Filename, "Models\\powerups\\EnergyGrenade.abc");
				SAFE_STRCPY(pStruct->m_SkinName, "Skins\\powerups\\EnergyGrenade_a.dtx");
				break;
			}
			case GUN_KATOGRENADE_ID:
			{
				SAFE_STRCPY(pStruct->m_Filename, "Models\\powerups\\KatoGrenade.abc");
				SAFE_STRCPY(pStruct->m_SkinName, "Skins\\powerups\\KatoGrenade_a.dtx");
				break;
			}
			case GUN_MAC10_ID:
			{
				SAFE_STRCPY(pStruct->m_Filename, "Models\\powerups\\Machinegun.abc");
				SAFE_STRCPY(pStruct->m_SkinName, "Skins\\powerups\\Machinegun_a.dtx");
				break;
			}
			case GUN_TOW_ID:
			{
				SAFE_STRCPY(pStruct->m_Filename, "Models\\powerups\\TOW.abc");
				SAFE_STRCPY(pStruct->m_SkinName, "Skins\\powerups\\TOW_a.dtx");
				break;
			}
			case GUN_LASERCANNON_ID:
			{
				SAFE_STRCPY(pStruct->m_Filename, "Models\\powerups\\lasercannon.abc");
				SAFE_STRCPY(pStruct->m_SkinName, "Skins\\powerups\\lasercannon_a.dtx");
				break;
			}
			case GUN_SQUEAKYTOY_ID:
			{
				SAFE_STRCPY(pStruct->m_Filename, "Models\\powerups\\SqueakyToy.abc");
				SAFE_STRCPY(pStruct->m_SkinName, "Skins\\powerups\\SqueakyToy_a.dtx");
				break;
			}
			default:
			{
				SAFE_STRCPY(pStruct->m_Filename, "Models\\powerups\\Colt45.abc");
				SAFE_STRCPY(pStruct->m_SkinName, "Skins\\powerups\\Colt45_a.dtx");
				break;
			}
		}


		// Set up the appropriate pick up sound...

		if (m_hstrSoundFile) 
		{
			g_pServerDE->FreeString(m_hstrSoundFile);
		}

		if (GUN_FIRSTMECH_ID <= m_iWeaponType && m_iWeaponType <= GUN_LASTMECH_ID)
		{
			m_hstrSoundFile = g_pServerDE->CreateString("Sounds\\Powerups\\Weapon_mca.wav");
		}
		else
		{
			m_hstrSoundFile = g_pServerDE->CreateString("Sounds\\Powerups\\Weapon_onfoot.wav");
		}

		m_bBounce = DFALSE;
		m_bRotate = DFALSE;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WeaponPowerup::InitialUpdate
//
//	PURPOSE:	Handle inital update engine messages
//
// ----------------------------------------------------------------------- //

void WeaponPowerup::InitialUpdate( CServerDE *pServerDE )
{
	DVector vDims, vScale, vNewDims;

	pServerDE->GetModelAnimUserDims(m_hObject, &vDims, pServerDE->GetModelAnimation(m_hObject));

	vScale = GetHandWeaponScale((RiotWeaponId)m_iWeaponType, m_eModelSize);
	pServerDE->ScaleObject(m_hObject, &vScale);

	// Set object dims based on scale value...

	vNewDims.x = vScale.x * vDims.x;
	vNewDims.y = vScale.y * vDims.y;
	vNewDims.z = vScale.z * vDims.z;

	g_pServerDE->SetObjectDims(m_hObject, &vNewDims);

	pServerDE->SetNextUpdate(m_hObject, UPDATE_DELTA);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WeaponPowerup::ObjectTouch
//
//	PURPOSE:	Add weapon powerup to object
//
// ----------------------------------------------------------------------- //

void WeaponPowerup::ObjectTouch(HOBJECT hObject)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hObject) return;

	// If we hit non-character objects, just ignore them...

	HCLASS hCharClass = pServerDE->GetClass("CBaseCharacter");
	HCLASS hObjClass  = pServerDE->GetObjectClass(hObject);

	if (pServerDE->IsKindOf(hObjClass, hCharClass))
	{
		CBaseCharacter* pCharObj = (CBaseCharacter*)pServerDE->HandleToObject(hObject);

		if (pCharObj && !pCharObj->IsDead() && pCharObj->CanCarryWeapon(m_iWeaponType))
		{
			HMESSAGEWRITE hMessage = pServerDE->StartMessageToObject(this, hObject, MID_ADDWEAPON);
			pServerDE->WriteToMessageByte( hMessage, m_iWeaponType );
			pServerDE->WriteToMessageFloat( hMessage, ( float )m_dwAmmo );
			pServerDE->EndMessage(hMessage);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WeaponPowerup::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void WeaponPowerup::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hWrite) return;

	pServerDE->WriteToMessageByte(hWrite, m_iWeaponType);
	pServerDE->WriteToMessageByte(hWrite, m_eModelSize);
	pServerDE->WriteToMessageDWord(hWrite, m_dwAmmo);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WeaponPowerup::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void WeaponPowerup::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hRead) return;

	m_iWeaponType	= pServerDE->ReadFromMessageByte(hRead);
	m_eModelSize	= (ModelSize) pServerDE->ReadFromMessageByte(hRead);
	m_dwAmmo		= pServerDE->ReadFromMessageDWord(hRead);

	// Set our object to the correct filenames...

	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct)
	PostPropRead(&theStruct);

	pServerDE->SetModelFilenames(m_hObject, theStruct.m_Filename, 
								 theStruct.m_SkinName);
}



BEGIN_CLASS(PulseRiflePowerup)
	ADD_LONGINTPROP_FLAG(WeaponType, GUN_PULSERIFLE_ID, PF_HIDDEN)
	ADD_LONGINTPROP(Ammo, GetSpawnedAmmo(GUN_PULSERIFLE_ID))
END_CLASS_DEFAULT(PulseRiflePowerup, WeaponPowerup, NULL, NULL)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PulseRiflePowerup::PulseRiflePowerup
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

PulseRiflePowerup::PulseRiflePowerup() : WeaponPowerup()
{ 
	m_dwAmmo		= GetSpawnedAmmo(GUN_PULSERIFLE_ID); 
	m_iWeaponType	= GUN_PULSERIFLE_ID;
}

BEGIN_CLASS(SpiderPowerup)
	ADD_LONGINTPROP_FLAG(WeaponType, GUN_SPIDER_ID, PF_HIDDEN)
	ADD_LONGINTPROP(Ammo, GetSpawnedAmmo(GUN_SPIDER_ID))
END_CLASS_DEFAULT(SpiderPowerup, WeaponPowerup, NULL, NULL)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SpiderPowerup::SpiderPowerup
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

SpiderPowerup::SpiderPowerup() : WeaponPowerup()
{ 
	m_dwAmmo		= GetSpawnedAmmo(GUN_SPIDER_ID); 
	m_iWeaponType	= GUN_SPIDER_ID;
}

BEGIN_CLASS(BullgutPowerup)
	ADD_LONGINTPROP_FLAG(WeaponType, GUN_BULLGUT_ID, PF_HIDDEN)
	ADD_LONGINTPROP(Ammo, GetSpawnedAmmo(GUN_BULLGUT_ID))
END_CLASS_DEFAULT(BullgutPowerup, WeaponPowerup, NULL, NULL)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BullgutPowerup::BullgutPowerup
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

BullgutPowerup::BullgutPowerup() : WeaponPowerup()
{ 
	m_dwAmmo		= GetSpawnedAmmo(GUN_BULLGUT_ID); 
	m_iWeaponType	= GUN_BULLGUT_ID;
}

BEGIN_CLASS(SniperRiflePowerup)
	ADD_LONGINTPROP_FLAG(WeaponType, GUN_SNIPERRIFLE_ID, PF_HIDDEN)
	ADD_LONGINTPROP(Ammo, GetSpawnedAmmo(GUN_SNIPERRIFLE_ID))
END_CLASS_DEFAULT(SniperRiflePowerup, WeaponPowerup, NULL, NULL)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SniperRiflePowerup::SniperRiflePowerup
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

SniperRiflePowerup::SniperRiflePowerup() : WeaponPowerup()
{ 
	m_dwAmmo		= GetSpawnedAmmo(GUN_SNIPERRIFLE_ID); 
	m_iWeaponType	= GUN_SNIPERRIFLE_ID;
}

BEGIN_CLASS(JuggernautPowerup)
	ADD_LONGINTPROP_FLAG(WeaponType, GUN_JUGGERNAUT_ID, PF_HIDDEN)
	ADD_LONGINTPROP(Ammo, GetSpawnedAmmo(GUN_JUGGERNAUT_ID))
END_CLASS_DEFAULT(JuggernautPowerup, WeaponPowerup, NULL, NULL)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	JuggernautPowerup::JuggernautPowerup
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

JuggernautPowerup::JuggernautPowerup() : WeaponPowerup()
{ 
	m_dwAmmo		= GetSpawnedAmmo(GUN_JUGGERNAUT_ID); 
	m_iWeaponType	= GUN_JUGGERNAUT_ID;
}

BEGIN_CLASS(ShredderPowerup)
	ADD_LONGINTPROP_FLAG(WeaponType, GUN_SHREDDER_ID, PF_HIDDEN)
	ADD_LONGINTPROP(Ammo, GetSpawnedAmmo(GUN_SHREDDER_ID))
END_CLASS_DEFAULT(ShredderPowerup, WeaponPowerup, NULL, NULL)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ShredderPowerup::ShredderPowerup
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

ShredderPowerup::ShredderPowerup() : WeaponPowerup()
{ 
	m_dwAmmo		= GetSpawnedAmmo(GUN_SHREDDER_ID); 
	m_iWeaponType	= GUN_SHREDDER_ID;
}

BEGIN_CLASS(RedRiotPowerup)
	ADD_LONGINTPROP_FLAG(WeaponType, GUN_REDRIOT_ID, PF_HIDDEN)
	ADD_LONGINTPROP(Ammo, GetSpawnedAmmo(GUN_REDRIOT_ID))
END_CLASS_DEFAULT(RedRiotPowerup, WeaponPowerup, NULL, NULL)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RedRiotPowerup::RedRiotPowerup
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

RedRiotPowerup::RedRiotPowerup() : WeaponPowerup()
{ 
	m_dwAmmo		= GetSpawnedAmmo(GUN_REDRIOT_ID); 
	m_iWeaponType	= GUN_REDRIOT_ID;
}

BEGIN_CLASS(PistolsPowerup)
	ADD_LONGINTPROP_FLAG(WeaponType, GUN_COLT45_ID, PF_HIDDEN)
	ADD_LONGINTPROP(Ammo, GetSpawnedAmmo(GUN_COLT45_ID))
END_CLASS_DEFAULT(PistolsPowerup, WeaponPowerup, NULL, NULL)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PistolsPowerup::PistolsPowerup
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

PistolsPowerup::PistolsPowerup() : WeaponPowerup()
{ 
	m_dwAmmo		= GetSpawnedAmmo(GUN_COLT45_ID); 
	m_iWeaponType	= GUN_COLT45_ID;
}

BEGIN_CLASS(ShotgunPowerup)
	ADD_LONGINTPROP_FLAG(WeaponType, GUN_SHOTGUN_ID, PF_HIDDEN)
	ADD_LONGINTPROP(Ammo, GetSpawnedAmmo(GUN_SHOTGUN_ID))
END_CLASS_DEFAULT(ShotgunPowerup, WeaponPowerup, NULL, NULL)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ShotgunPowerup::ShotgunPowerup
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

ShotgunPowerup::ShotgunPowerup() : WeaponPowerup()
{ 
	m_dwAmmo		= GetSpawnedAmmo(GUN_SHOTGUN_ID); 
	m_iWeaponType	= GUN_SHOTGUN_ID;
}

BEGIN_CLASS(AssaultRiflePowerup)
	ADD_LONGINTPROP_FLAG(WeaponType, GUN_ASSAULTRIFLE_ID, PF_HIDDEN)
	ADD_LONGINTPROP(Ammo, GetSpawnedAmmo(GUN_ASSAULTRIFLE_ID))
END_CLASS_DEFAULT(AssaultRiflePowerup, WeaponPowerup, NULL, NULL)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AssaultRiflePowerup::AssaultRiflePowerup
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

AssaultRiflePowerup::AssaultRiflePowerup() : WeaponPowerup()
{ 
	m_dwAmmo		= GetSpawnedAmmo(GUN_ASSAULTRIFLE_ID); 
	m_iWeaponType	= GUN_ASSAULTRIFLE_ID;
}

BEGIN_CLASS(EnergyGrenadePowerup)
	ADD_LONGINTPROP_FLAG(WeaponType, GUN_ENERGYGRENADE_ID, PF_HIDDEN)
	ADD_LONGINTPROP(Ammo, GetSpawnedAmmo(GUN_ENERGYGRENADE_ID))
END_CLASS_DEFAULT(EnergyGrenadePowerup, WeaponPowerup, NULL, NULL)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	EnergyGrenadePowerup::EnergyGrenadePowerup
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

EnergyGrenadePowerup::EnergyGrenadePowerup() : WeaponPowerup()
{ 
	m_dwAmmo		= GetSpawnedAmmo(GUN_ENERGYGRENADE_ID); 
	m_iWeaponType	= GUN_ENERGYGRENADE_ID;
}

BEGIN_CLASS(KatoGrenadePowerup)
	ADD_LONGINTPROP_FLAG(WeaponType, GUN_KATOGRENADE_ID, PF_HIDDEN)
	ADD_LONGINTPROP(Ammo, GetSpawnedAmmo(GUN_KATOGRENADE_ID))
END_CLASS_DEFAULT(KatoGrenadePowerup, WeaponPowerup, NULL, NULL)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KatoGrenadePowerup::KatoGrenadePowerup
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

KatoGrenadePowerup::KatoGrenadePowerup() : WeaponPowerup()
{ 
	m_dwAmmo		= GetSpawnedAmmo(GUN_KATOGRENADE_ID); 
	m_iWeaponType	= GUN_KATOGRENADE_ID;
}

BEGIN_CLASS(MachineGunPowerup)
	ADD_LONGINTPROP_FLAG(WeaponType, GUN_MAC10_ID, PF_HIDDEN)
	ADD_LONGINTPROP(Ammo, GetSpawnedAmmo(GUN_MAC10_ID))
END_CLASS_DEFAULT(MachineGunPowerup, WeaponPowerup, NULL, NULL)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	MachineGunPowerup::MachineGunPowerup
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

MachineGunPowerup::MachineGunPowerup() : WeaponPowerup()
{ 
	m_dwAmmo		= GetSpawnedAmmo(GUN_MAC10_ID); 
	m_iWeaponType	= GUN_MAC10_ID;
}

BEGIN_CLASS(TOWPowerup)
	ADD_LONGINTPROP_FLAG(WeaponType, GUN_TOW_ID, PF_HIDDEN)
	ADD_LONGINTPROP(Ammo, GetSpawnedAmmo(GUN_TOW_ID))
END_CLASS_DEFAULT(TOWPowerup, WeaponPowerup, NULL, NULL)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TOWPowerup::TOWPowerup
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

TOWPowerup::TOWPowerup() : WeaponPowerup()
{ 
	m_dwAmmo		= GetSpawnedAmmo(GUN_TOW_ID); 
	m_iWeaponType	= GUN_TOW_ID;
}

BEGIN_CLASS(LaserCannonPowerup)
	ADD_LONGINTPROP_FLAG(WeaponType, GUN_LASERCANNON_ID, PF_HIDDEN)
	ADD_LONGINTPROP(Ammo, GetSpawnedAmmo(GUN_LASERCANNON_ID))
END_CLASS_DEFAULT(LaserCannonPowerup, WeaponPowerup, NULL, NULL)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LaserCannonPowerup::LaserCannonPowerup
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

LaserCannonPowerup::LaserCannonPowerup() : WeaponPowerup()
{ 
	m_dwAmmo		= GetSpawnedAmmo(GUN_LASERCANNON_ID); 
	m_iWeaponType	= GUN_LASERCANNON_ID;
}

BEGIN_CLASS(SqueakyToyPowerup)
	ADD_LONGINTPROP_FLAG(WeaponType, GUN_SQUEAKYTOY_ID, PF_HIDDEN)
	ADD_LONGINTPROP(Ammo, GetSpawnedAmmo(GUN_SQUEAKYTOY_ID))
END_CLASS_DEFAULT(SqueakyToyPowerup, WeaponPowerup, NULL, NULL)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SqueakyToyPowerup::SqueakyToyPowerup
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

SqueakyToyPowerup::SqueakyToyPowerup() : WeaponPowerup()
{ 
	m_dwAmmo		= GetSpawnedAmmo(GUN_SQUEAKYTOY_ID); 
	m_iWeaponType	= GUN_SQUEAKYTOY_ID;
}