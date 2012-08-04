// ----------------------------------------------------------------------- //
//
// MODULE  : RiotWeapons.h
//
// PURPOSE : Riot Weapons - Definitions of the Riot weapon classes
//
// CREATED : 9/30/97
//
// ----------------------------------------------------------------------- //

#ifndef __RIOT_WEAPONS_H__
#define __RIOT_WEAPONS_H__

#include "Weapon.h"

class CProjectile;

// Mecha weapons...

class CPulseRifle : public CWeapon
{
	public :
		CPulseRifle();

	protected : 
		CProjectile* CreateProjectile(ObjectCreateStruct & theStruct);
};

class CShredder : public CWeapon
{
	public :
		CShredder();

	protected : 
		CProjectile* CreateProjectile(ObjectCreateStruct & theStruct);
};

class CBullgut : public CWeapon
{
	public :
		CBullgut();

	protected : 
		CProjectile* CreateProjectile(ObjectCreateStruct & theStruct);
};

class CJuggernaut : public CWeapon
{
	public :
		CJuggernaut();
	protected : 
		CProjectile* CreateProjectile(ObjectCreateStruct & theStruct);
};

class CSpider : public CWeapon
{
	public :
		CSpider();

	protected : 
		CProjectile* CreateProjectile(ObjectCreateStruct & theStruct);
};

class CRedRiot : public CWeapon
{
	public :
		CRedRiot();

	protected : 
		CProjectile* CreateProjectile(ObjectCreateStruct & theStruct);
};

class CSniperRifle : public CWeapon
{
	public :
		CSniperRifle();
};

class CEnergyBaton : public CWeapon
{
	public :
		CEnergyBaton();
};

class CEnergyBlade : public CWeapon
{
	public :
		CEnergyBlade();
};

class CKatana : public CWeapon
{
	public :
		CKatana();
};

class CMonoKnife : public CWeapon
{
	public :
		CMonoKnife();
};


// On-foot weapons...

class CColt45 : public CWeapon
{
	public :
		CColt45();

};

class CShotgun : public CWeapon
{
	public :
		CShotgun();
};

class CMac10 : public CWeapon
{
	public :
		CMac10();
};

class CAssaultRifle : public CWeapon
{
	public :
		CAssaultRifle();
};

class CEnergyGrenade : public CWeapon
{
	public :
		CEnergyGrenade();

	protected : 
		CProjectile* CreateProjectile(ObjectCreateStruct & theStruct);
};

class CTOW : public CWeapon
{
	public :
		CTOW();

	protected : 
		CProjectile* CreateProjectile(ObjectCreateStruct & theStruct);
};

class CLaserCannon : public CWeapon
{
	public :
		CLaserCannon();
};

class CKatoGrenade : public CWeapon
{
	public :
		CKatoGrenade();
	protected :
		virtual CProjectile* CreateProjectile(ObjectCreateStruct & theStruct);
};

class CTanto : public CWeapon
{
	public :
		CTanto();
};

class CSqueakyToy : public CWeapon
{
	public :
		CSqueakyToy();
};

#endif // __RIOT_WEAPONS_H__
