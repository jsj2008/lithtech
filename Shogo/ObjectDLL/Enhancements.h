// ----------------------------------------------------------------------- //
//
// MODULE  : Enhancements.h
//
// PURPOSE : Riot First Aid Powerups 
//
// CREATED : 1/28/97
//
// ----------------------------------------------------------------------- //

#ifndef __ENHANCEMENTS_H__
#define __ENHANCEMENTS_H__

#include "EnhancementItem.h"

class DamageEnhancement : public EnhancementItem
{
	public:

		DamageEnhancement();
};

class MeleeDamageEnhancement : public EnhancementItem
{
	public:

		MeleeDamageEnhancement();
};

class ProtectionEnhancement : public EnhancementItem
{
	public:

		ProtectionEnhancement();
};

class EnergyProtectionEnhancement : public EnhancementItem
{
	public:

		EnergyProtectionEnhancement();
};

class ProjectileProtectionEnhancement : public EnhancementItem
{
	public:

		ProjectileProtectionEnhancement();
};

class ExplosiveProtectionEnhancement : public EnhancementItem
{
	public:

		ExplosiveProtectionEnhancement();
};

class RegenEnhancement : public EnhancementItem
{
	public:

		RegenEnhancement();
};

class HealthEnhancement : public EnhancementItem
{
	public:

		HealthEnhancement();
};

class ArmorEnhancement : public EnhancementItem
{
	public:

		ArmorEnhancement();
};

#endif //  __ENHANCEMENTS_H__