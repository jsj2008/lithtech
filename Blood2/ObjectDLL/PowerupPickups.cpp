// ----------------------------------------------------------------------- //
//
// MODULE  : PowerupPickups.cpp
//
// PURPOSE : Blood2 Ammunition pickups - implementation
//
// CREATED : 12/11/97
//
// ----------------------------------------------------------------------- //

#include "PowerupPickups.h"
#include "cpp_server_de.h"
#include "SharedDefs.h"
#include "generic_msg_de.h"
#include "BloodServerShell.h"


// *********************************************************************** //
//
//	CLASS:		PowerupPickup
//
//	PURPOSE:	Base Powerup pickup item
//
// *********************************************************************** //


BEGIN_CLASS(PowerupPickup)
END_CLASS_DEFAULT(PowerupPickup, PickupObject, NULL, NULL)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PowerupPickup::ObjectTouch
//
//	PURPOSE:	handles an object touch
//
// ----------------------------------------------------------------------- //

void PowerupPickup::ObjectTouch(HOBJECT hObject)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	HMESSAGEWRITE hMessage = pServerDE->StartMessageToObject(this, hObject, MID_ADDPOWERUP);
	pServerDE->WriteToMessageByte(hMessage, (DBYTE)m_nType);
	pServerDE->WriteToMessageFloat(hMessage, m_fValue * m_fValueMult);
	pServerDE->EndMessage(hMessage);
}



// *********************************************************************** //
//
//	CLASS:		Health
//
//	PURPOSE:	Base Health pickup item
//
// *********************************************************************** //


BEGIN_CLASS(HealthBase)
END_CLASS_DEFAULT_FLAGS(HealthBase, PowerupPickup, NULL, NULL, CF_HIDDEN)


// Constructor
HealthBase::HealthBase() : PowerupPickup( )
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	HealthBase::EngineMessageFn
//
//	PURPOSE:	engine msg handler
//
// ----------------------------------------------------------------------- //

DDWORD HealthBase::EngineMessageFn(DDWORD messageID, void *pData, float fData)
{
	switch(messageID)
	{
		case MID_INITIALUPDATE:
		{
			if( g_pBloodServerShell->IsMultiplayerGame( ))
			{
				m_fRespawnTime = ( DFLOAT )g_pBloodServerShell->GetNetGameInfo( )->m_nHealthRespawn;
				if( g_pBloodServerShell->GetNetGameInfo( )->m_nHealthLevel == LEVEL_NONE )
					g_pServerDE->RemoveObject( m_hObject );
				else
					m_fValueMult = ConvertNetItemMultiplier( g_pBloodServerShell->GetNetGameInfo( )->m_nHealthLevel );
			}

			break;
		}
	}

	return PowerupPickup::EngineMessageFn(messageID, pData, fData);
}

// *********************************************************************** //
//
//	CLASS:		HealthPU
//
//	PURPOSE:	
//
// *********************************************************************** //

BEGIN_CLASS(HealthPU)
END_CLASS_DEFAULT(HealthPU, HealthBase, NULL, NULL)

// Constructor
HealthPU::HealthPU() : HealthBase( )
{
	m_nType	= POWERUP_HEALTH;
	m_szFile		= "Health_pu";
	m_szObjectName	= "Life Essence";
	m_nNameID		= IDS_POWERUP_HEALTH;
	m_fValue		= 25.0f;
	m_szPickupSound	= "sounds\\powerups\\health1a.wav";
}


// *********************************************************************** //
//
//	CLASS:		MegaHealthPU
//
//	PURPOSE:	
//
// *********************************************************************** //

BEGIN_CLASS(MegaHealthPU)
END_CLASS_DEFAULT(MegaHealthPU, HealthBase, NULL, NULL)

// Constructor
MegaHealthPU::MegaHealthPU() : HealthBase( )
{
	m_nType	= POWERUP_MEGAHEALTH;
	m_szFile		= "MegaHealth_pu";
	m_szObjectName	= "Life Seed";
	m_nNameID		= IDS_POWERUP_MEGAHEALTH;
	m_fValue		= 100.0f;
	m_szPickupSound	= "sounds\\powerups\\health2.wav";
}


// *********************************************************************** //
//
//	CLASS:		Armor
//
//	PURPOSE:	Base Armor pickup item
//
// *********************************************************************** //


BEGIN_CLASS(ArmorBase)
END_CLASS_DEFAULT_FLAGS(ArmorBase, PowerupPickup, NULL, NULL, CF_HIDDEN)


// Constructor
ArmorBase::ArmorBase() : PowerupPickup( )
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ArmorBase::EngineMessageFn
//
//	PURPOSE:	engine msg handler
//
// ----------------------------------------------------------------------- //

DDWORD ArmorBase::EngineMessageFn(DDWORD messageID, void *pData, float fData)
{
	switch(messageID)
	{
		case MID_INITIALUPDATE:
		{
			if( g_pBloodServerShell->IsMultiplayerGame( ))
			{
				m_fRespawnTime = ( DFLOAT )g_pBloodServerShell->GetNetGameInfo( )->m_nArmorRespawn;
				if( g_pBloodServerShell->GetNetGameInfo( )->m_nArmorLevel == LEVEL_NONE )
					g_pServerDE->RemoveObject( m_hObject );
				else
					m_fValueMult = ConvertNetItemMultiplier( g_pBloodServerShell->GetNetGameInfo( )->m_nArmorLevel );
			}

			break;
		}
	}

	return PowerupPickup::EngineMessageFn(messageID, pData, fData);
}

// *********************************************************************** //
//
//	CLASS:		WardPU
//
//	PURPOSE:	
//
// *********************************************************************** //

BEGIN_CLASS(WardPU)
END_CLASS_DEFAULT(WardPU, ArmorBase, NULL, NULL)

// Constructor
WardPU::WardPU() : ArmorBase()
{
	m_nType	= POWERUP_WARD;
	m_szFile		= "Ward_pu";
	m_szObjectName	= "Ward";
	m_nNameID		= IDS_POWERUP_WARD;
	m_fValue		= 25.0f;
	m_szPickupSound	= "sounds\\powerups\\armour1.wav";
}


// *********************************************************************** //
//
//	CLASS:		NecroWardPU
//
//	PURPOSE:	
//
// *********************************************************************** //

BEGIN_CLASS(NecroWardPU)
END_CLASS_DEFAULT(NecroWardPU, ArmorBase, NULL, NULL)

// Constructor
NecroWardPU::NecroWardPU() : ArmorBase()
{
	m_nType	= POWERUP_NECROWARD;
	m_szFile		= "NecroWard_pu";
	m_szObjectName	= "NecroWard";
	m_nNameID		= IDS_POWERUP_NECROWARD;
	m_fValue		= 100.0f;
	m_szPickupSound	= "sounds\\powerups\\powerup3.wav";
}


// *********************************************************************** //
//
//	CLASS:		Enhancement
//
//	PURPOSE:	Base Enhancement pickup item
//
// *********************************************************************** //


BEGIN_CLASS(EnhancementBase)
END_CLASS_DEFAULT_FLAGS(EnhancementBase, PowerupPickup, NULL, NULL, CF_HIDDEN)


// Constructor
EnhancementBase::EnhancementBase() : PowerupPickup( )
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	EnhancementBase::EngineMessageFn
//
//	PURPOSE:	engine msg handler
//
// ----------------------------------------------------------------------- //

DDWORD EnhancementBase::EngineMessageFn(DDWORD messageID, void *pData, float fData)
{
	switch(messageID)
	{
		case MID_INITIALUPDATE:
		{
			if( g_pBloodServerShell->IsMultiplayerGame( ))
			{
				m_fRespawnTime = ( DFLOAT )g_pBloodServerShell->GetNetGameInfo( )->m_nPowerupsRespawn;
				if( g_pBloodServerShell->GetNetGameInfo( )->m_nPowerupsLevel == LEVEL_NONE )
					g_pServerDE->RemoveObject( m_hObject );
				else
					m_fValueMult = ConvertNetItemMultiplier( g_pBloodServerShell->GetNetGameInfo( )->m_nPowerupsLevel );
			}

			break;
		}
	}

	return PowerupPickup::EngineMessageFn(messageID, pData, fData);
}

// *********************************************************************** //
//
//	CLASS:		InvulnerabilityPU
//
//	PURPOSE:	
//
// *********************************************************************** //

BEGIN_CLASS(InvulnerabilityPU)
END_CLASS_DEFAULT(InvulnerabilityPU, EnhancementBase, NULL, NULL)

// Constructor
InvulnerabilityPU::InvulnerabilityPU() : EnhancementBase()
{
	m_nType	= POWERUP_INVULNERABILITY;
	m_szFile		= "Invulnerability_pu";
	m_szObjectName	= "Willpower";
	m_nNameID		= IDS_POWERUP_WILLPOWER;
	m_fValue		= 30.0f;
}


// *********************************************************************** //
//
//	CLASS:		StealthPU
//
//	PURPOSE:	
//
// *********************************************************************** //

BEGIN_CLASS(StealthPU)
END_CLASS_DEFAULT(StealthPU, EnhancementBase, NULL, NULL)

// Constructor
StealthPU::StealthPU() : EnhancementBase()
{
	m_nType	= POWERUP_STEALTH;
	m_szFile		= "Stealth_pu";
	m_szObjectName	= "Stealth";
	m_nNameID		= IDS_POWERUP_STEALTH;
	m_fValue		= 30.0f;
}


// *********************************************************************** //
//
//	CLASS:		AngerPU
//
//	PURPOSE:	
//
// *********************************************************************** //

BEGIN_CLASS(AngerPU)
END_CLASS_DEFAULT(AngerPU, EnhancementBase, NULL, NULL)

// Constructor
AngerPU::AngerPU() : EnhancementBase()
{
	m_nType	= POWERUP_ANGER;
	m_szFile		= "Anger_pu";
	m_szObjectName	= "The Anger";
	m_nNameID		= IDS_POWERUP_ANGER;
	m_fValue		= 30.0f;
}


// *********************************************************************** //
//
//	CLASS:		RevenantPU
//
//	PURPOSE:	
//
// *********************************************************************** //

BEGIN_CLASS(RevenantPU)
END_CLASS_DEFAULT(RevenantPU, EnhancementBase, NULL, NULL)

// Constructor
RevenantPU::RevenantPU() : EnhancementBase()
{
	m_nType	= POWERUP_REVENANT;
	m_szFile		= "Revenant_pu";
	m_szObjectName	= "Revenant";
	m_nNameID		= IDS_POWERUP_REVENANT;
	m_fValue		= 30.0f;
}


