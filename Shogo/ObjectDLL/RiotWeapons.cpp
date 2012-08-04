// ----------------------------------------------------------------------- //
//
// MODULE  : RiotWeapons.cpp
//
// PURPOSE : Riot Weapons - Implementation of the Riot weapon classes
//
// CREATED : 9/30/97
//
// ----------------------------------------------------------------------- //

#include "RiotWeapons.h"
#include "cpp_server_de.h"
#include "PlayerObj.h"
#include "WeaponFXTypes.h"


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPulseRifle::CPulseRifle
//
//	PURPOSE:	Create the pulse rifle gun
//
// ----------------------------------------------------------------------- //

CPulseRifle::CPulseRifle() : CWeapon()
{
	m_nId			 = GUN_PULSERIFLE_ID;
	m_eDamageType	 = DT_ENERGY;

	m_pHandWeaponName	= "Models\\Powerups\\PulseRifle.abc";
	m_pHandWeaponSkin	= "Skins\\Powerups\\PulseRifle_a.dtx";

	m_nAmmoInClip		= GetShotsPerClip(m_nId);

	m_fMinFireDuration	= 0.5f;
	m_fMaxFireDuration	= 3.0f;
	m_fMinFireRest		= 1.0f;
	m_fMaxFireRest		= 5.0f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPulseRifle::CPulseRifle
//
//	PURPOSE:	Create the pulse rifle projectile
//
// ----------------------------------------------------------------------- //

CProjectile* CPulseRifle::CreateProjectile(ObjectCreateStruct & theStruct)
{
	CProjectile* pRet = DNULL;

	CServerDE* pServerDE = BaseClass::GetServerDE();

	if (pServerDE)
	{
		HCLASS hClass = pServerDE->GetClass("CPulseRifleProjectile");

		if (hClass)
		{
			pRet = (CProjectile*)pServerDE->CreateObject(hClass, &theStruct);
		}
	}

	return pRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CShredder::CShredder
//
//	PURPOSE:	Create the shredder gun
//
// ----------------------------------------------------------------------- //

CShredder::CShredder() : CWeapon()
{
	m_nId			 = GUN_SHREDDER_ID;
	m_eDamageType	 = DT_EXPLODE;

	m_pHandWeaponName	= "Models\\Powerups\\Shredder.abc";
	m_pHandWeaponSkin	= "Skins\\Powerups\\Shredder_a.dtx";

	m_fMinFireDuration	= 0.5f;
	m_fMaxFireDuration	= 2.0f;
	m_fMinFireRest		= 5.0f;
	m_fMaxFireRest		= 10.0f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CJuggernaut::CreateProjectile
//
//	PURPOSE:	Create the juggernaut projectile
//
// ----------------------------------------------------------------------- //

CProjectile* CShredder::CreateProjectile(ObjectCreateStruct & theStruct)
{
	CProjectile* pRet = DNULL;

	CServerDE* pServerDE = BaseClass::GetServerDE();

	if (pServerDE)
	{
		HCLASS hClass = pServerDE->GetClass("CJuggernautProjectile");

		if (hClass)
		{
			pRet = (CProjectile*)pServerDE->CreateObject(hClass, &theStruct);
		}
	}

	return pRet;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBullgut::CBullgut
//
//	PURPOSE:	Create the bullgut gun
//
// ----------------------------------------------------------------------- //

CBullgut::CBullgut() : CWeapon()
{
	m_nId			 = GUN_BULLGUT_ID;
	m_eDamageType	 = DT_EXPLODE;

	m_bCanLockOnTarget	= DTRUE;

	m_pHandWeaponName	= "Models\\Powerups\\Bullgut.abc";
	m_pHandWeaponSkin	= "Skins\\Powerups\\Bullgut_a.dtx";

	m_nAmmoInClip		= GetShotsPerClip(m_nId);

	m_fMinFireDuration	= 0.5f;
	m_fMaxFireDuration	= 1.0f;
	m_fMinFireRest		= 5.0f;
	m_fMaxFireRest		= 10.0f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBullgut::CreateProjectile
//
//	PURPOSE:	Create the bullgut projectile
//
// ----------------------------------------------------------------------- //

CProjectile* CBullgut::CreateProjectile(ObjectCreateStruct & theStruct)
{
	CProjectile* pRet = DNULL;

	CServerDE* pServerDE = BaseClass::GetServerDE();

	if (pServerDE)
	{
		HCLASS hClass = pServerDE->GetClass("CBullgutProjectile");

		if (hClass)
		{
			pRet = (CProjectile*)pServerDE->CreateObject(hClass, &theStruct);
		}
	}

	return pRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CJuggernaut::CJuggernaut
//
//	PURPOSE:	Create the juggernaut gun
//
// ----------------------------------------------------------------------- //

CJuggernaut::CJuggernaut() : CWeapon()
{
	m_nId			 = GUN_JUGGERNAUT_ID;
	m_eDamageType	 = DT_EXPLODE;

	m_pHandWeaponName	= "Models\\Powerups\\Juggernaut.abc";
	m_pHandWeaponSkin	= "Skins\\Powerups\\Juggernaut_a.dtx";

	m_fMinFireDuration	= 0.5f;
	m_fMaxFireDuration	= 2.0f;
	m_fMinFireRest		= 5.0f;
	m_fMaxFireRest		= 10.0f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CJuggernaut::CreateProjectile
//
//	PURPOSE:	Create the juggernaut projectile
//
// ----------------------------------------------------------------------- //

CProjectile* CJuggernaut::CreateProjectile(ObjectCreateStruct & theStruct)
{
	CProjectile* pRet = DNULL;

	CServerDE* pServerDE = BaseClass::GetServerDE();

	if (pServerDE)
	{
		HCLASS hClass = pServerDE->GetClass("CJuggernautProjectile");

		if (hClass)
		{
			pRet = (CProjectile*)pServerDE->CreateObject(hClass, &theStruct);
		}
	}

	return pRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSpider::CSpider
//
//	PURPOSE:	Create the spider gun
//
// ----------------------------------------------------------------------- //

CSpider::CSpider() : CWeapon()
{
	m_nId			 = GUN_SPIDER_ID;
	m_eDamageType	 = DT_EXPLODE;

	m_bCanLockOnTarget	= DTRUE;

	m_pHandWeaponName	= "Models\\Powerups\\Spider.abc";
	m_pHandWeaponSkin	= "Skins\\Powerups\\Spider_a.dtx";

	m_fMinFireDuration	= 0.5f;
	m_fMaxFireDuration	= 2.0f;
	m_fMinFireRest		= 5.0f;
	m_fMaxFireRest		= 10.0f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSpider::CreateProjectile
//
//	PURPOSE:	Create the spider projectile
//
// ----------------------------------------------------------------------- //

CProjectile* CSpider::CreateProjectile(ObjectCreateStruct & theStruct)
{
	CProjectile* pRet = DNULL;

	CServerDE* pServerDE = BaseClass::GetServerDE();

	if (pServerDE)
	{
		HCLASS hClass = pServerDE->GetClass("CStickyGrenadeProjectile");

		if (hClass)
		{
			pRet = (CProjectile*)pServerDE->CreateObject(hClass, &theStruct);
		}
	}

	return pRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRedRiot::CRedRiot
//
//	PURPOSE:	Create the red riot gun
//
// ----------------------------------------------------------------------- //

CRedRiot::CRedRiot() : CWeapon()
{
	m_nId			 = GUN_REDRIOT_ID;
	m_eDamageType	 = DT_KATO;

	m_pHandWeaponName	= "Models\\Powerups\\RedRiot.abc";
	m_pHandWeaponSkin	= "Skins\\Powerups\\RedRiot_a.dtx";

	m_fMinFireDuration	= 0.5f;
	m_fMaxFireDuration	= 1.0f;
	m_fMinFireRest		= 20.0f;
	m_fMaxFireRest		= 30.0f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRedRiot::CreateProjectile
//
//	PURPOSE:	Create the red riot projectile
//
// ----------------------------------------------------------------------- //

CProjectile* CRedRiot::CreateProjectile(ObjectCreateStruct & theStruct)
{
	CProjectile* pRet = DNULL;

	CServerDE* pServerDE = BaseClass::GetServerDE();

	if (pServerDE)
	{
		HCLASS hClass = pServerDE->GetClass("CRedRiotProjectile");

		if (hClass)
		{
			pRet = (CProjectile*)pServerDE->CreateObject(hClass, &theStruct);
		}
	}

	return pRet;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSniperRifle::CSniperRifle
//
//	PURPOSE:	Create the sniper rifle gun
//
// ----------------------------------------------------------------------- //

CSniperRifle::CSniperRifle() : CWeapon()
{
	m_nId			 = GUN_SNIPERRIFLE_ID;
	m_eDamageType	 = DT_BURST;

	m_pHandWeaponName	= "Models\\Powerups\\SniperRifle.abc";
	m_pHandWeaponSkin	= "Skins\\Powerups\\SniperRifle_a.dtx";

	m_nAmmoInClip		= GetShotsPerClip(m_nId);

	m_fZoomDamageMult	= 7.0f;
	m_fMinFireDuration	= 0.5f;
	m_fMaxFireDuration	= 2.0f;
	m_fMinFireRest		= 5.0f;
	m_fMaxFireRest		= 10.0f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CEnergyBaton::CEnergyBaton
//
//	PURPOSE:	Create the energy baton
//
// ----------------------------------------------------------------------- //

CEnergyBaton::CEnergyBaton() : CWeapon()
{
	m_nId			 = GUN_ENERGYBATON_ID;
	m_eDamageType	 = DT_MELEE;

	m_pHandWeaponName	= "Models\\Powerups\\EnergyBaton.abc";
	m_pHandWeaponSkin	= "Skins\\Powerups\\EnergyBaton.dtx";

	m_bInfiniteAmmo	= DTRUE;

	m_fMinFireDuration	= 0.5f;
	m_fMaxFireDuration	= 5.0f;
	m_fMinFireRest		= 2.0f;
	m_fMaxFireRest		= 5.0f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CEnergyBlade::CEnergyBlade
//
//	PURPOSE:	Create the energy blade
//
// ----------------------------------------------------------------------- //

CEnergyBlade::CEnergyBlade() : CWeapon()
{
	m_nId			 = GUN_ENERGYBLADE_ID;
	m_eDamageType	 = DT_MELEE;

	m_pHandWeaponName	= "Models\\Powerups\\EnergyBlade.abc";
	m_pHandWeaponSkin	= "Skins\\Powerups\\EnergyBlade.dtx";

	m_bInfiniteAmmo	= DTRUE;

	m_fMinFireDuration	= 0.5f;
	m_fMaxFireDuration	= 5.0f;
	m_fMinFireRest		= 2.0f;
	m_fMaxFireRest		= 5.0f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CKatana::CKatana
//
//	PURPOSE:	Create the katana
//
// ----------------------------------------------------------------------- //

CKatana::CKatana() : CWeapon()
{
	m_nId			 = GUN_KATANA_ID;
	m_eDamageType	 = DT_MELEE;

	m_pHandWeaponName	= "Models\\Powerups\\Katana.abc";
	m_pHandWeaponSkin	= "Skins\\Powerups\\Katana.dtx";

	m_bInfiniteAmmo	= DTRUE;

	m_fMinFireDuration	= 0.5f;
	m_fMaxFireDuration	= 5.0f;
	m_fMinFireRest		= 2.0f;
	m_fMaxFireRest		= 5.0f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMonoKnife::CMonoKnife
//
//	PURPOSE:	Create the katana
//
// ----------------------------------------------------------------------- //

CMonoKnife::CMonoKnife() : CWeapon()
{
	m_nId			 = GUN_MONOKNIFE_ID;
	m_eDamageType	 = DT_MELEE;

	m_pHandWeaponName	= "Models\\Powerups\\MonoKnife.abc";
	m_pHandWeaponSkin	= "Skins\\Powerups\\MonoKnife.dtx";

	m_bInfiniteAmmo	= DTRUE;

	m_fMinFireDuration	= 0.5f;
	m_fMaxFireDuration	= 5.0f;
	m_fMinFireRest		= 2.0f;
	m_fMaxFireRest		= 5.0f;
}


// On-foot weapons...

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CColt45::CColt45
//
//	PURPOSE:	Create the colt 45 gun
//
// ----------------------------------------------------------------------- //

CColt45::CColt45() : CWeapon()
{
	m_nId			 = GUN_COLT45_ID;
	m_eDamageType	 = DT_PUNCTURE;

	m_pHandWeaponName	= "Models\\Powerups\\Colt45.abc";
	m_pHandWeaponSkin	= "Skins\\Powerups\\Colt45_a.dtx";

	m_nAmmoInClip	= GetShotsPerClip(m_nId);

	m_fMinFireDuration	= 0.5f;
	m_fMaxFireDuration	= 2.0f;
	m_fMinFireRest		= 5.0f;
	m_fMaxFireRest		= 10.0f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CShotgun::CShotgun
//
//	PURPOSE:	Create the shotgun
//
// ----------------------------------------------------------------------- //

CShotgun::CShotgun() : CWeapon()
{
	m_nId			 = GUN_SHOTGUN_ID;
	m_eDamageType	 = DT_PUNCTURE;

	m_pHandWeaponName	= "Models\\Powerups\\Shotgun.abc";
	m_pHandWeaponSkin	= "Skins\\Powerups\\Shotgun_a.dtx";

	m_fMinFireDuration	= 0.5f;
	m_fMaxFireDuration	= 1.0f;
	m_fMinFireRest		= 5.0f;
	m_fMaxFireRest		= 10.0f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMac10::CMac10
//
//	PURPOSE:	Create the mac 10 machine gun
//
// ----------------------------------------------------------------------- //

CMac10::CMac10() : CWeapon()
{
	m_nId			 = GUN_MAC10_ID;
	m_eDamageType	 = DT_PUNCTURE;

	m_pHandWeaponName	= "Models\\Powerups\\Machinegun.abc";
	m_pHandWeaponSkin	= "Skins\\Powerups\\Machinegun_a.dtx";

	m_nAmmoInClip	= GetShotsPerClip(m_nId);

	m_fMinFireDuration	= 1.5f;
	m_fMaxFireDuration	= 3.0f;
	m_fMinFireRest		= 5.0f;
	m_fMaxFireRest		= 10.0f;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAssaultRifle::CAssaultRifle
//
//	PURPOSE:	Create the assault riffle
//
// ----------------------------------------------------------------------- //

CAssaultRifle::CAssaultRifle() : CWeapon()
{
	m_nId			 = GUN_ASSAULTRIFLE_ID;
	m_eDamageType	 = DT_PUNCTURE;

	m_pHandWeaponName	= "Models\\Powerups\\AssaultRifle.abc";
	m_pHandWeaponSkin	= "Skins\\Powerups\\AssaultRifle_a.dtx";

	m_nAmmoInClip	= GetShotsPerClip(m_nId);

	m_fZoomDamageMult	= 1.5f;
	m_fMinFireDuration	= 1.5f;
	m_fMaxFireDuration	= 4.0f;
	m_fMinFireRest		= 10.0f;
	m_fMaxFireRest		= 15.0f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CEnergyGrenade::CEnergyGrenade
//
//	PURPOSE:	Create the energy grenade launcher
// ----------------------------------------------------------------------- //

CEnergyGrenade::CEnergyGrenade() : CWeapon()
{
	m_nId			 = GUN_ENERGYGRENADE_ID;
	m_eDamageType	 = DT_EXPLODE;

	m_pHandWeaponName	= "Models\\Powerups\\EnergyGrenade.abc";
	m_pHandWeaponSkin	= "Skins\\Powerups\\EnergyGrenade_a.dtx";

	m_nAmmoInClip	= GetShotsPerClip(m_nId);

	m_fMinFireDuration	= 0.5f;
	m_fMaxFireDuration	= 2.0f;
	m_fMinFireRest		= 10.0f;
	m_fMaxFireRest		= 15.0f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CEnergyGrenade::CreateProjectile
//
//	PURPOSE:	Create the energy grenade projectile
//
// ----------------------------------------------------------------------- //

CProjectile* CEnergyGrenade::CreateProjectile(ObjectCreateStruct & theStruct)
{
	CProjectile* pRet = DNULL;

	CServerDE* pServerDE = BaseClass::GetServerDE();

	if (pServerDE)
	{
		HCLASS hClass = pServerDE->GetClass("CGrenadeProjectile");

		if (hClass)
		{
			pRet = (CProjectile*)pServerDE->CreateObject(hClass, &theStruct);
		}
	}

	return pRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTOW::CTOW
//
//	PURPOSE:	Create the TOW rocket launcher
//
// ----------------------------------------------------------------------- //

CTOW::CTOW() : CWeapon()
{
	m_nId			 = GUN_TOW_ID;
	m_eDamageType	 = DT_EXPLODE;

	m_bCanLockOnTarget	= DTRUE;

	m_pHandWeaponName	= "Models\\Powerups\\TOW.abc";
	m_pHandWeaponSkin	= "Skins\\Powerups\\TOW_a.dtx";

	m_nAmmoInClip		= GetShotsPerClip(m_nId);

	m_fMinFireDuration	= 0.5f;
	m_fMaxFireDuration	= 2.0f;
	m_fMinFireRest		= 10.0f;
	m_fMaxFireRest		= 15.0f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTOW::CreateProjectile
//
//	PURPOSE:	Create the TOW rocket launcher projectile
//
// ----------------------------------------------------------------------- //

CProjectile* CTOW::CreateProjectile(ObjectCreateStruct & theStruct)
{
	CProjectile* pRet = DNULL;

	CServerDE* pServerDE = BaseClass::GetServerDE();

	if (pServerDE)
	{
		HCLASS hClass = pServerDE->GetClass("CTOWProjectile");

		if (hClass)
		{
			pRet = (CProjectile*)pServerDE->CreateObject(hClass, &theStruct);
		}
	}

	return pRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLaserCannon::CLaserCannon
//
//	PURPOSE:	Create the laser cannon
//
// ----------------------------------------------------------------------- //

CLaserCannon::CLaserCannon() : CWeapon()
{
	m_nId			 = GUN_LASERCANNON_ID;
	m_eDamageType	 = DT_ENERGY;

	m_pHandWeaponName	= "Models\\Powerups\\LaserCannon.abc";
	m_pHandWeaponSkin	= "Skins\\Powerups\\LaserCannon_a.dtx";

	m_fMinFireDuration	= 1.5f;
	m_fMaxFireDuration	= 3.0f;
	m_fMinFireRest		= 10.0f;
	m_fMaxFireRest		= 15.0f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CKatoGrenade::CKatoGrenade
//
//	PURPOSE:	Create the KatoGrenade rocket launcher
//
// ----------------------------------------------------------------------- //

CKatoGrenade::CKatoGrenade() : CWeapon()
{
	m_nId			 = GUN_KATOGRENADE_ID;
	m_eDamageType	 = DT_KATO;

	m_pHandWeaponName	= "Models\\Powerups\\KatoGrenade.abc";
	m_pHandWeaponSkin	= "Skins\\Powerups\\KatoGrenade_a.dtx";

	m_nAmmoInClip		= GetShotsPerClip(m_nId);

	m_fMinFireDuration	= 0.5f;
	m_fMaxFireDuration	= 2.0f;
	m_fMinFireRest		= 5.0f;
	m_fMaxFireRest		= 10.0f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CKatoGrenade::CreateProjectile
//
//	PURPOSE:	Create the hand grenade projectile
//
// ----------------------------------------------------------------------- //

CProjectile* CKatoGrenade::CreateProjectile(ObjectCreateStruct & theStruct)
{
	CProjectile* pRet = DNULL;

	CServerDE* pServerDE = BaseClass::GetServerDE();

	if (pServerDE)
	{
		HCLASS hClass = pServerDE->GetClass("CKatoGrenadeProjectile");

		if (hClass)
		{
			pRet = (CProjectile*)pServerDE->CreateObject(hClass, &theStruct);
		}
	}

	return pRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTanto::CTanto
//
//	PURPOSE:	Create the tanto
//
// ----------------------------------------------------------------------- //

CTanto::CTanto() : CWeapon()
{
	m_nId			 = GUN_TANTO_ID;
	m_eDamageType	 = DT_MELEE;

	m_pHandWeaponName	= "Models\\Powerups\\Tanto.abc";
	m_pHandWeaponSkin	= "Skins\\Powerups\\Tanto_a.dtx";

	m_nAmmoInClip	= GetShotsPerClip(m_nId);
	m_bInfiniteAmmo	= DTRUE;

	m_fMinFireDuration	= 2.5f;
	m_fMaxFireDuration	= 4.0f;
	m_fMinFireRest		= 5.0f;
	m_fMaxFireRest		= 10.0f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSqueakyToy::CSqueakyToy
//
//	PURPOSE:	Create the squeeky toy gun
//
// ----------------------------------------------------------------------- //

CSqueakyToy::CSqueakyToy() : CWeapon()
{
	m_nId			 = GUN_SQUEAKYTOY_ID;
	m_eDamageType	 = DT_SQUEAKY;

	m_pHandWeaponName	= "Models\\Powerups\\SqueakyToy.abc";
	m_pHandWeaponSkin	= "Skins\\Powerups\\SqueakyToy_a.dtx";

	m_bInfiniteAmmo	= DTRUE;

	m_fMinFireDuration	= 0.5f;
	m_fMaxFireDuration	= 2.0f;
	m_fMinFireRest		= 5.0f;
	m_fMaxFireRest		= 10.0f;
}

