// ----------------------------------------------------------------------- //
//
// MODULE  : AmmoPickups.cpp
//
// PURPOSE : Blood2 Ammunition pickups - implementation
//
// CREATED : 10/24/97
//
// ----------------------------------------------------------------------- //

#include "AmmoPickups.h"
#include "cpp_server_de.h"
#include "SharedDefs.h"
#include "generic_msg_de.h"
#include "ClientRes.h"


// *********************************************************************** //
//
//	CLASS:		AmmoPickup
//
//	PURPOSE:	Base Ammo pickup item
//
// *********************************************************************** //


BEGIN_CLASS(AmmoPickup)
END_CLASS_DEFAULT(AmmoPickup, PickupObject, NULL, NULL)

DLink AmmoPickup::m_PUHead;
DDWORD AmmoPickup::m_dwNumPU = 0;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AmmoPickup::EngineMessageFn
//
//	PURPOSE:	engine msg handler
//
// ----------------------------------------------------------------------- //

DDWORD AmmoPickup::EngineMessageFn(DDWORD messageID, void *pData, float fData)
{
	switch(messageID)
	{
		case MID_INITIALUPDATE:
		{
			// insert it into the list
			dl_Insert( &m_PUHead, &m_Link );
			m_Link.m_pData = ( void * )this;
			m_dwNumPU++;

			if( g_pBloodServerShell->IsMultiplayerGame( ))
			{
				m_fRespawnTime = ( DFLOAT )g_pBloodServerShell->GetNetGameInfo( )->m_nAmmoRespawn;
				if( g_pBloodServerShell->GetNetGameInfo( )->m_nAmmoLevel == LEVEL_NONE )
					g_pServerDE->RemoveObject( m_hObject );
				else
					m_fValueMult = ConvertNetItemMultiplier( g_pBloodServerShell->GetNetGameInfo( )->m_nAmmoLevel );
			}

			break;
		}
	}

	return PickupObject::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AmmoPickup::ObjectTouch
//
//	PURPOSE:	handles an object touch
//
// ----------------------------------------------------------------------- //

void AmmoPickup::ObjectTouch(HOBJECT hObject)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	HMESSAGEWRITE hMessage = pServerDE->StartMessageToObject(this, hObject, MID_ADDAMMO);
	pServerDE->WriteToMessageByte(hMessage, (DBYTE)m_nType);
	pServerDE->WriteToMessageFloat(hMessage, m_fValue * m_fValueMult );
	pServerDE->EndMessage(hMessage);
}



// *********************************************************************** //
//
//	CLASS:		BulletAmmoPU
//
//	PURPOSE:	Bullet
//
// *********************************************************************** //

BEGIN_CLASS(BulletAmmoPU)
END_CLASS_DEFAULT(BulletAmmoPU, AmmoPickup, NULL, NULL)

// Constructor
BulletAmmoPU::BulletAmmoPU() : AmmoPickup()
{
	m_nType		= AMMO_BULLET;
	m_fValue	= AMMOCOUNT_BULLET;
	m_szFile		= "BulletAmmo_pu";
	m_szObjectName	= "Bullets";
	m_nNameID	= IDS_AMMO_BULLETS;
}


// *********************************************************************** //
//
//	CLASS:		ShellAmmoPU
//
//	PURPOSE:	Shells
//
// *********************************************************************** //

BEGIN_CLASS(ShellAmmoPU)
END_CLASS_DEFAULT(ShellAmmoPU, AmmoPickup, NULL, NULL)

// Constructor
ShellAmmoPU::ShellAmmoPU() : AmmoPickup()
{
	m_nType		= AMMO_SHELL;
	m_fValue	= AMMOCOUNT_SHELL;
	m_szFile		= "ShellAmmo_pu";
	m_szObjectName	= "Shells";
	m_nNameID	= IDS_AMMO_SHELLS;
}


// *********************************************************************** //
//
//	CLASS:		BMGAmmoPU
//
//	PURPOSE:	Rounds for the sniper rifle
//
// *********************************************************************** //

BEGIN_CLASS(BMGAmmoPU)
END_CLASS_DEFAULT(BMGAmmoPU, AmmoPickup, NULL, NULL)

// Constructor
BMGAmmoPU::BMGAmmoPU() : AmmoPickup()
{
	m_nType		= AMMO_BMG;
	m_fValue	= AMMOCOUNT_BMG;
	m_szFile		= "BMGAmmo_pu";
	m_szObjectName	= "BMG Rounds";
	m_nNameID	= IDS_AMMO_BMG;
}


// *********************************************************************** //
//
//	CLASS:		FlareAmmoPU
//
//	PURPOSE:	Flares
//
// *********************************************************************** //

BEGIN_CLASS(FlareAmmoPU)
END_CLASS_DEFAULT(FlareAmmoPU, AmmoPickup, NULL, NULL)

// Constructor
FlareAmmoPU::FlareAmmoPU() : AmmoPickup()
{
	m_nType		= AMMO_FLARE;
	m_fValue	= AMMOCOUNT_FLARE;
	m_szFile		= "FlareAmmo_pu";
	m_szObjectName	= "Flares";
	m_nNameID	= IDS_AMMO_FLARES;
}


// *********************************************************************** //
//
//	CLASS:		DieBugDieAmmoPU
//
//	PURPOSE:	Cans of 'Die Bug Die'
//
// *********************************************************************** //

BEGIN_CLASS(DieBugDieAmmoPU)
END_CLASS_DEFAULT(DieBugDieAmmoPU, AmmoPickup, NULL, NULL)

// Constructor
DieBugDieAmmoPU::DieBugDieAmmoPU() : AmmoPickup()
{
	m_nType		= AMMO_DIEBUGDIE;
	m_fValue	= AMMOCOUNT_DIEBUGDIE;
	m_szFile		= "DieBugDieAmmo_pu";
	m_szObjectName	= "DieBugDie";
	m_nNameID	= IDS_AMMO_DIEBUGDIE;
}


// *********************************************************************** //
//
//	CLASS:		HowitzerAmmoPU
//
//	PURPOSE:	Grenade/Rocket ammunition
//
// *********************************************************************** //

BEGIN_CLASS(HowitzerAmmoPU)
END_CLASS_DEFAULT(HowitzerAmmoPU, AmmoPickup, NULL, NULL)

// Constructor
HowitzerAmmoPU::HowitzerAmmoPU() : AmmoPickup()
{
	m_nType			= AMMO_HOWITZER;
	m_fValue		= AMMOCOUNT_HOWITZER;
	m_szFile		= "HowitzerAmmo_pu";
	m_szObjectName	= "Howizter Shells";
	m_nNameID	= IDS_AMMO_HOWITZER;
}


// *********************************************************************** //
//
//	CLASS:		FuelAmmoPU
//
//	PURPOSE:	Fuel tank refill...
//
// *********************************************************************** //

BEGIN_CLASS(FuelAmmoPU)
END_CLASS_DEFAULT(FuelAmmoPU, AmmoPickup, NULL, NULL)

// Constructor
FuelAmmoPU::FuelAmmoPU() : AmmoPickup()
{
	m_nType		= AMMO_FUEL;
	m_fValue	= AMMOCOUNT_FUEL;
	m_szFile		= "FuelAmmo_pu";
	m_szObjectName	= "FuelTank";
	m_nNameID	= IDS_AMMO_FUEL;
}


// *********************************************************************** //
//
//	CLASS:		BatteryAmmoPU
//
//	PURPOSE:	Chemical Batteries
//
// *********************************************************************** //

BEGIN_CLASS(BatteryAmmoPU)
END_CLASS_DEFAULT(BatteryAmmoPU, AmmoPickup, NULL, NULL)

// Constructor
BatteryAmmoPU::BatteryAmmoPU() : AmmoPickup()
{
	m_nType		= AMMO_BATTERY;
	m_fValue	= AMMOCOUNT_BATTERY;
	m_szFile		= "BatteryAmmo_pu";
	m_szObjectName	= "Chemical Batteries";
	m_nNameID	= IDS_AMMO_BATTERY;
}


// *********************************************************************** //
//
//	CLASS:		ManaAmmoPU
//
//	PURPOSE:	Mana
//
// *********************************************************************** //
/*
BEGIN_CLASS(ManaAmmoPU)
	ADD_REALPROP( Value, 100 )
END_CLASS_DEFAULT(ManaAmmoPU, AmmoPickup, NULL, NULL)

// Constructor
ManaAmmoPU::ManaAmmoPU() : AmmoPickup()
{
	m_nType		= AMMO_MANA;
	m_fValue	= 100.0f;
	m_szFile		= "ManaAmmo_pu";
	m_szObjectName	= "Mana";
}
*/