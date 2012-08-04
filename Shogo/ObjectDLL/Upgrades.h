// ----------------------------------------------------------------------- //
//
// MODULE  : Upgrades.h
//
// PURPOSE : Riot First Aid Powerups 
//
// CREATED : 1/28/97
//
// ----------------------------------------------------------------------- //

#ifndef __UPGRADES_H__
#define __UPGRADES_H__

#include "UpgradeItem.h"

class DamageUpgrade : public UpgradeItem
{
	public:

		DamageUpgrade();
};

class ProtectionUpgrade : public UpgradeItem
{
	public:

		ProtectionUpgrade();
};

class RegenUpgrade : public UpgradeItem
{
	public:

		RegenUpgrade();
};

class HealthUpgrade : public UpgradeItem
{
	public:

		HealthUpgrade();
};

class ArmorUpgrade : public UpgradeItem
{
	public:

		ArmorUpgrade();
};

class TargetingUpgrade : public UpgradeItem
{
	public:

		TargetingUpgrade();
};


#endif //  __UPGRADES_H__