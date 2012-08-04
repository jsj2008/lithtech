// ----------------------------------------------------------------------- //
//
// MODULE  : WeaponPickups.cpp
//
// PURPOSE : Blood2 Ammunition pickups - implementation
//
// CREATED : 10/24/97
//
// ----------------------------------------------------------------------- //

#include "WeaponPickups.h"
#include "cpp_server_de.h"
#include "SharedDefs.h"
#include "generic_msg_de.h"
#include "bloodservershell.h"
#include "ClientServerShared.h"

// *********************************************************************** //
//
//	CLASS:		WeaponPickup
//
//	PURPOSE:	Base Weapon pickup item
//
// *********************************************************************** //


BEGIN_CLASS(WeaponPickup)
END_CLASS_DEFAULT(WeaponPickup, PickupObject, NULL, NULL)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WeaponPickup::EngineMessageFn
//
//	PURPOSE:	engine msg handler
//
// ----------------------------------------------------------------------- //

DDWORD WeaponPickup::EngineMessageFn(DDWORD messageID, void *pData, float fData)
{
	switch(messageID)
	{
		case MID_INITIALUPDATE:
		{
			if( g_pBloodServerShell->IsMultiplayerGame( ))
			{
				g_pServerDE->RemoveObject( m_hObject );
			}
			else
			{
				DDWORD dwFlags = g_pServerDE->GetObjectUserFlags( m_hObject );
				dwFlags |= USRFLG_GLOW;
				g_pServerDE->SetObjectUserFlags( m_hObject, dwFlags );
			}


			break;
		}
	}

	return PickupObject::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WeaponPickup::ObjectTouch
//
//	PURPOSE:	handles an object touch
//
// ----------------------------------------------------------------------- //

void WeaponPickup::ObjectTouch(HOBJECT hObject)
{
	HMESSAGEWRITE hMessage;
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	hMessage = pServerDE->StartMessageToObject(this, hObject, MID_ADDWEAPON);
	pServerDE->WriteToMessageByte(hMessage, m_nType);
	pServerDE->WriteToMessageFloat(hMessage, m_fValue * m_fValueMult );
	pServerDE->EndMessage(hMessage);

}



// *********************************************************************** //
//
//	CLASS:		BerettaPU
//
//	PURPOSE:	Beretta
//
// *********************************************************************** //

BEGIN_CLASS(BerettaPU)
END_CLASS_DEFAULT(BerettaPU, WeaponPickup, NULL, NULL)

// Constructor
BerettaPU::BerettaPU() : WeaponPickup()
{
	m_nType				= WEAP_BERETTA;
	m_szFile			= "Beretta_pu";
	m_szObjectName		= "Beretta";
	m_nNameID			= IDS_WEAPON_BERETTA;
	m_fValue = 12.0f;
}



// *********************************************************************** //
//
//	CLASS:		ShotgunPU
//
//	PURPOSE:	Shotgun
//
// *********************************************************************** //

BEGIN_CLASS(ShotgunPU)
END_CLASS_DEFAULT(ShotgunPU, WeaponPickup, NULL, NULL)

// Constructor
ShotgunPU::ShotgunPU() : WeaponPickup()
{
	m_nType		= WEAP_SHOTGUN;
	m_szFile			= "Shotgun_pu";
	m_szObjectName		= "Sawed-off Shotgun";
	m_nNameID			= IDS_WEAPON_SHOTGUN;
	m_fValue = 8.0f;
}

#ifdef _ADD_ON

// *********************************************************************** //
//
//	CLASS:		CombatShotgunPU
//
//	PURPOSE:	Combat Shotgun
//
// *********************************************************************** //

BEGIN_CLASS(CombatShotgunPU)
END_CLASS_DEFAULT(CombatShotgunPU, WeaponPickup, NULL, NULL)

// Constructor
CombatShotgunPU::CombatShotgunPU() : WeaponPickup()
{
	m_nType				= WEAP_COMBATSHOTGUN;
	m_szFile			= "CombatShotgun_pu";
	m_szObjectName		= "Combat Shotgun";
	m_nNameID			= IDS_WEAPON_COMBATSHOTGUN;
	m_fValue = 8.0f;
}

#endif

// *********************************************************************** //
//
//	CLASS:		SniperRiflePU
//
//	PURPOSE:	SniperRifle
//
// *********************************************************************** //

BEGIN_CLASS(SniperRiflePU)
END_CLASS_DEFAULT(SniperRiflePU, WeaponPickup, NULL, NULL)

// Constructor
SniperRiflePU::SniperRiflePU() : WeaponPickup()
{
	m_nType		= WEAP_SNIPERRIFLE;
	m_szFile			= "Sniperrifle_pu";
	m_szObjectName		= "Sniper Rifle";
	m_nNameID			= IDS_WEAPON_SNIPER;
	m_fValue = 10.0f;
}


// *********************************************************************** //
//
//	CLASS:		AssaultRiflePU
//
//	PURPOSE:	AssaultRifle
//
// *********************************************************************** //

BEGIN_CLASS(AssaultRiflePU)
END_CLASS_DEFAULT(AssaultRiflePU, WeaponPickup, NULL, NULL)

// Constructor
AssaultRiflePU::AssaultRiflePU() : WeaponPickup()
{
	m_nType		= WEAP_ASSAULTRIFLE;
	m_szFile			= "m16_pu";
	m_szObjectName		= "Assault Rifle";
	m_nNameID			= IDS_WEAPON_M16;
	m_fValue = 50.0f;
}


// *********************************************************************** //
//
//	CLASS:		SubMachineGunPU
//
//	PURPOSE:	SubMachineGun
//
// *********************************************************************** //

BEGIN_CLASS(SubMachineGunPU)
END_CLASS_DEFAULT(SubMachineGunPU, WeaponPickup, NULL, NULL)

// Constructor
SubMachineGunPU::SubMachineGunPU() : WeaponPickup()
{
	m_nType		= WEAP_SUBMACHINEGUN;
	m_szFile			= "Mac10_pu";
	m_szObjectName		= "Sub-machine Gun";
	m_nNameID			= IDS_WEAPON_MAC10;
	m_fValue = 20.0f;
}


// *********************************************************************** //
//
//	CLASS:		FlareGunPU
//
//	PURPOSE:	FlareGun
//
// *********************************************************************** //

BEGIN_CLASS(FlareGunPU)
END_CLASS_DEFAULT(FlareGunPU, WeaponPickup, NULL, NULL)

// Constructor
FlareGunPU::FlareGunPU() : WeaponPickup()
{
	m_nType		= WEAP_FLAREGUN;
	m_szFile			= "FlarePistol_pu";
	m_szObjectName		= "Flare Pistol";
	m_nNameID			= IDS_WEAPON_FLAREGUN;
	m_fValue = 8.0f;
}


// *********************************************************************** //
//
//	CLASS:		HowitzerPU
//
//	PURPOSE:	AssaultCannon
//
// *********************************************************************** //

BEGIN_CLASS(HowitzerPU)
END_CLASS_DEFAULT(HowitzerPU, WeaponPickup, NULL, NULL)

// Constructor
HowitzerPU::HowitzerPU() : WeaponPickup()
{
	m_nType		= WEAP_HOWITZER;
	m_szFile			= "Howitzer_pu";
	m_szObjectName		= "Howitzer";
	m_nNameID			= IDS_WEAPON_HOWITZER;
	m_fValue = 5.0f;
}


// *********************************************************************** //
//
//	CLASS:		BugSprayPU
//
//	PURPOSE:	FlameRifle
//
// *********************************************************************** //

BEGIN_CLASS(BugSprayPU)
END_CLASS_DEFAULT(BugSprayPU, WeaponPickup, NULL, NULL)

// Constructor
BugSprayPU::BugSprayPU() : WeaponPickup()
{
	m_nType				= WEAP_BUGSPRAY;
	m_szFile			= "BugSpray_pu";
	m_szObjectName		= "Bug Spray Canister";
	m_nNameID			= IDS_WEAPON_BUGSPRAY;
	m_fValue = 20.0f;
}


// *********************************************************************** //
//
//	CLASS:		NapalmCannonPU
//
//	PURPOSE:	Napalm Cannon
//
// *********************************************************************** //

BEGIN_CLASS(NapalmCannonPU)
END_CLASS_DEFAULT(NapalmCannonPU, WeaponPickup, NULL, NULL)

// Constructor
NapalmCannonPU::NapalmCannonPU() : WeaponPickup()
{
	m_nType		= WEAP_NAPALMCANNON;
	m_szFile			= "NapalmCannon_pu";
	m_szObjectName		= "Napalm Cannon";
	m_nNameID			= IDS_WEAPON_NAPALM;
	m_fValue = 5.0f;
}


// *********************************************************************** //
//
//	CLASS:		MiniGunPU
//
//	PURPOSE:	MiniGun
//
// *********************************************************************** //

BEGIN_CLASS(MiniGunPU)
END_CLASS_DEFAULT(MiniGunPU, WeaponPickup, NULL, NULL)

// Constructor
MiniGunPU::MiniGunPU() : WeaponPickup()
{
	m_nType		= WEAP_MINIGUN;
	m_szFile			= "Minigun_pu";
	m_szObjectName		= "Vulcan Cannon";
	m_nNameID			= IDS_WEAPON_MINIGUN;
	m_fValue = 100.0f;
}


// *********************************************************************** //
//
//	CLASS:		VoodooDollPU
//
//	PURPOSE:	VoodooDoll
//
// *********************************************************************** //

BEGIN_CLASS(VoodooDollPU)
END_CLASS_DEFAULT(VoodooDollPU, WeaponPickup, NULL, NULL)

// Constructor
VoodooDollPU::VoodooDollPU() : WeaponPickup()
{
	m_nType		= WEAP_VOODOO;
	m_szFile			= "Voodoo_pu";
	m_szObjectName		= "Voodoo Doll";
	m_nNameID			= IDS_WEAPON_VOODOO;
	m_fValue = 0;
}


// *********************************************************************** //
//
//	CLASS:		OrbPU
//
//	PURPOSE:	Orb
//
// *********************************************************************** //

BEGIN_CLASS(OrbPU)
END_CLASS_DEFAULT(OrbPU, WeaponPickup, NULL, NULL)

// Constructor
OrbPU::OrbPU() : WeaponPickup()
{
	m_nType				= WEAP_ORB;
	m_szFile			= "Orb_pu";
	m_szObjectName		= "The Orb";
	m_nNameID			= IDS_WEAPON_ORB;
	m_fValue = 0;
}


// *********************************************************************** //
//
//	CLASS:		DeathRayPU
//
//	PURPOSE:	LaserRifle
//
// *********************************************************************** //

BEGIN_CLASS(DeathRayPU)
END_CLASS_DEFAULT(DeathRayPU, WeaponPickup, NULL, NULL)

// Constructor
DeathRayPU::DeathRayPU() : WeaponPickup()
{
	m_nType				= WEAP_DEATHRAY;
	m_szFile			= "DeathRay_pu";
	m_szObjectName		= "Cabalco Death Ray";
	m_nNameID			= IDS_WEAPON_DEATHRAY;
	m_fValue = 50.0f;
}


// *********************************************************************** //
//
//	CLASS:		LifeLeechPU
//
//	PURPOSE:	LifeLeech
//
// *********************************************************************** //

BEGIN_CLASS(LifeLeechPU)
END_CLASS_DEFAULT(LifeLeechPU, WeaponPickup, NULL, NULL)

// Constructor
LifeLeechPU::LifeLeechPU() : WeaponPickup()
{
	m_nType				= WEAP_LIFELEECH;
	m_szFile			= "Lifeleech_pu";
	m_szObjectName		= "Life Leech";
	m_nNameID			= IDS_WEAPON_LIFELEECH;
	m_fValue = 0;
}

#ifdef _ADD_ON

// *********************************************************************** //
//
//	CLASS:		FlayerPU
//
//	PURPOSE:	Flayer
//
// *********************************************************************** //

BEGIN_CLASS(FlayerPU)
END_CLASS_DEFAULT(FlayerPU, WeaponPickup, NULL, NULL)

// Constructor
FlayerPU::FlayerPU() : WeaponPickup()
{
	m_nType				= WEAP_FLAYER;
	m_szFile			= "Flayer_pu";
	m_szObjectName		= "Flayer";
	m_nNameID			= IDS_WEAPON_FLAYER;
	m_fValue = 0;
}

#endif

// *********************************************************************** //
//
//	CLASS:		TeslaCannonPU
//
//	PURPOSE:	TeslaCannon
//
// *********************************************************************** //

BEGIN_CLASS(TeslaCannonPU)
END_CLASS_DEFAULT(TeslaCannonPU, WeaponPickup, NULL, NULL)

// Constructor
TeslaCannonPU::TeslaCannonPU() : WeaponPickup()
{
	m_nType				= WEAP_TESLACANNON;
	m_szFile			= "TeslaCannon_pu";
	m_szObjectName		= "Tesla Cannon";
	m_nNameID			= IDS_WEAPON_TESLA;
	m_fValue = 40.0f;
}



// *********************************************************************** //
//
//	CLASS:		SingularityPU
//
//	PURPOSE:	Singularity
//
// *********************************************************************** //

BEGIN_CLASS(SingularityPU)
END_CLASS_DEFAULT(SingularityPU, WeaponPickup, NULL, NULL)

// Constructor
SingularityPU::SingularityPU() : WeaponPickup()
{
	m_nType				= WEAP_SINGULARITY;
	m_szFile			= "Singularity_pu";
	m_szObjectName		= "Singularity Generator";
	m_nNameID			= IDS_WEAPON_SINGULARITY;
	m_fValue = 100.0f;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SpawnWeaponPickup
//
//	PURPOSE:	Generates a weapon pickup object, used when a player or AI
//				drops a weapon.
//
// ----------------------------------------------------------------------- //

HOBJECT SpawnWeaponPickup(DDWORD dwWeapType, HOBJECT hOwner, DVector *pvPos, DRotation *prRot, DBOOL bIncludeAmmo)
{
	HCLASS hClass = NULL;
	HOBJECT hObject = NULL;

	switch (dwWeapType)
	{
		case WEAP_BERETTA: hClass = g_pServerDE->GetClass("BerettaPU"); break;
		case WEAP_SHOTGUN: hClass = g_pServerDE->GetClass("ShotgunPU"); break;
		case WEAP_SNIPERRIFLE: hClass = g_pServerDE->GetClass("SniperRiflePU"); break;
		case WEAP_ASSAULTRIFLE: hClass = g_pServerDE->GetClass("AssaultRiflePU"); break;
		case WEAP_SUBMACHINEGUN: hClass = g_pServerDE->GetClass("SubMachineGunPU"); break;
		case WEAP_FLAREGUN: hClass = g_pServerDE->GetClass("FlareGunPU"); break;
		case WEAP_HOWITZER: hClass = g_pServerDE->GetClass("HowitzerPU"); break;
		case WEAP_BUGSPRAY: hClass = g_pServerDE->GetClass("BugSprayPU"); break;
		case WEAP_NAPALMCANNON: hClass = g_pServerDE->GetClass("NapalmCannonPU"); break;
		case WEAP_MINIGUN: hClass = g_pServerDE->GetClass("MiniGunPU"); break;
		case WEAP_VOODOO: hClass = g_pServerDE->GetClass("VoodooDollPU"); break;
		case WEAP_ORB: hClass = g_pServerDE->GetClass("OrbPU"); break;
		case WEAP_DEATHRAY: hClass = g_pServerDE->GetClass("DeathRayPU"); break;
		case WEAP_LIFELEECH: hClass = g_pServerDE->GetClass("LifeLeechPU"); break;
		case WEAP_TESLACANNON: hClass = g_pServerDE->GetClass("TeslaCannonPU"); break;
		case WEAP_SINGULARITY: hClass = g_pServerDE->GetClass("SingularityPU"); break;
#ifdef _ADD_ON
		case WEAP_COMBATSHOTGUN: hClass = g_pServerDE->GetClass("CombatShotgunPU"); break;
		case WEAP_FLAYER: hClass = g_pServerDE->GetClass("FlayerPU"); break;
#endif

	}
	if (hClass)
	{
		ObjectCreateStruct ocStruct;
		DVector vPos;
		DRotation rRot;

		if (pvPos && prRot)
		{
			VEC_COPY(vPos, *pvPos);
			ROT_COPY(rRot, *prRot);
		}
		else
		{
			g_pServerDE->GetObjectPos(hOwner, &vPos);
			g_pServerDE->GetObjectRotation(hOwner, &rRot);
		}

		INIT_OBJECTCREATESTRUCT(ocStruct);

		VEC_COPY(ocStruct.m_Pos, vPos)
		ROT_COPY(ocStruct.m_Rotation, rRot)
		ocStruct.m_Flags = (FLAG_VISIBLE | FLAG_TOUCH_NOTIFY /* | FLAG_SHADOW */ );
		
		WeaponPickup* pObject = (WeaponPickup*)g_pServerDE->CreateObject(hClass, &ocStruct);
		hObject = g_pServerDE->ObjectToHandle(pObject);
		pObject->SetBounce( DFALSE );
		pObject->SpawnItem(&vPos);
		if( !bIncludeAmmo )
			pObject->SetValue( 0.0f );
	}
	return hObject;
}