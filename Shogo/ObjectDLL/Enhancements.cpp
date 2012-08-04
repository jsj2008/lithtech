// ----------------------------------------------------------------------- //
//
// MODULE  : Enhancements.cpp
//
// PURPOSE : Implementation of enhancement items
//
// CREATED : 1/28/98
//
// ----------------------------------------------------------------------- //

#include "Enhancements.h"
#include "ServerRes.h"
#include "InventoryTypes.h"

BEGIN_CLASS(DamageEnhancement)
	ADD_REALPROP(RespawnTime, 120.0f)
END_CLASS_DEFAULT(DamageEnhancement, EnhancementItem, NULL, NULL)

DamageEnhancement::DamageEnhancement()
{
	m_nModelName = IDS_MODEL_DAMAGEENHANCEMENT;
	m_nModelSkin = IDS_SKIN_DAMAGEENHANCEMENT;
	m_nSoundName = IDS_SOUND_DAMAGEENHANCEMENT;
	m_nEnhancementSubType = IST_ENHANCEMENT_DAMAGE;
	m_eType		 = PIT_ENHANCEMENT_DAMAGE;
}

BEGIN_CLASS(MeleeDamageEnhancement)
	ADD_REALPROP(RespawnTime, 120.0f)
END_CLASS_DEFAULT(MeleeDamageEnhancement, EnhancementItem, NULL, NULL)

MeleeDamageEnhancement::MeleeDamageEnhancement()
{
	m_nModelName = IDS_MODEL_MELEEDAMAGEENHANCEMENT;
	m_nModelSkin = IDS_SKIN_MELEEDAMAGEENHANCEMENT;
	m_nSoundName = IDS_SOUND_MELEEDAMAGEENHANCEMENT;
	m_nEnhancementSubType = IST_ENHANCEMENT_MELEEDAMAGE;
	m_eType		 = PIT_ENHANCEMENT_MELEEDAMAGE;
}

BEGIN_CLASS(ProtectionEnhancement)
	ADD_REALPROP(RespawnTime, 120.0f)
END_CLASS_DEFAULT(ProtectionEnhancement, EnhancementItem, NULL, NULL)

ProtectionEnhancement::ProtectionEnhancement()
{
	m_nModelName = IDS_MODEL_PROTECTIONENHANCEMENT;
	m_nModelSkin = IDS_SKIN_PROTECTIONENHANCEMENT;
	m_nSoundName = IDS_SOUND_PROTECTIONENHANCEMENT;
	m_nEnhancementSubType = IST_ENHANCEMENT_PROTECTION;
	m_eType		 = PIT_ENHANCEMENT_PROTECTION;
}

BEGIN_CLASS(EnergyProtectionEnhancement)
	ADD_REALPROP(RespawnTime, 120.0f)
END_CLASS_DEFAULT(EnergyProtectionEnhancement, EnhancementItem, NULL, NULL)

EnergyProtectionEnhancement::EnergyProtectionEnhancement()
{
	m_nModelName = IDS_MODEL_ENERGYPROTECTIONENHANCEMENT;
	m_nModelSkin = IDS_SKIN_ENERGYPROTECTIONENHANCEMENT;
	m_nSoundName = IDS_SOUND_ENERGYPROTECTIONENHANCEMENT;
	m_nEnhancementSubType = IST_ENHANCEMENT_ENERGYPROTECTION;
	m_eType		 = PIT_ENHANCEMENT_ENERGYPROTECTION;
}

BEGIN_CLASS(ProjectileProtectionEnhancement)
	ADD_REALPROP(RespawnTime, 120.0f)
END_CLASS_DEFAULT(ProjectileProtectionEnhancement, EnhancementItem, NULL, NULL)

ProjectileProtectionEnhancement::ProjectileProtectionEnhancement()
{
	m_nModelName = IDS_MODEL_PROJECTILEPROTECTIONENHANCEMENT;
	m_nModelSkin = IDS_SKIN_PROJECTILEPROTECTIONENHANCEMENT;
	m_nSoundName = IDS_SOUND_PROJECTILEPROTECTIONENHANCEMENT;
	m_nEnhancementSubType = IST_ENHANCEMENT_PROJECTILEPROTECTION;
	m_eType		 = PIT_ENHANCEMENT_PROJECTILEPROTECTION;
}

BEGIN_CLASS(ExplosiveProtectionEnhancement)
	ADD_REALPROP(RespawnTime, 120.0f)
END_CLASS_DEFAULT(ExplosiveProtectionEnhancement, EnhancementItem, NULL, NULL)

ExplosiveProtectionEnhancement::ExplosiveProtectionEnhancement()
{
	m_nModelName = IDS_MODEL_EXPLOSIVEPROTECTIONENHANCEMENT;
	m_nModelSkin = IDS_SKIN_EXPLOSIVEPROTECTIONENHANCEMENT;
	m_nSoundName = IDS_SOUND_EXPLOSIVEPROTECTIONENHANCEMENT;
	m_nEnhancementSubType = IST_ENHANCEMENT_EXPLOSIVEPROTECTION;
	m_eType		 = PIT_ENHANCEMENT_EXPLOSIVEPROTECTION;
}

BEGIN_CLASS(RegenEnhancement)
	ADD_REALPROP(RespawnTime, 120.0f)
END_CLASS_DEFAULT(RegenEnhancement, EnhancementItem, NULL, NULL)

RegenEnhancement::RegenEnhancement()
{
	m_nModelName = IDS_MODEL_REGENENHANCEMENT;
	m_nModelSkin = IDS_SKIN_REGENENHANCEMENT;
	m_nSoundName = IDS_SOUND_REGENENHANCEMENT;
	m_nEnhancementSubType = IST_ENHANCEMENT_REGEN;
	m_eType		 = PIT_ENHANCEMENT_REGEN;
}

BEGIN_CLASS(HealthEnhancement)
	ADD_REALPROP(RespawnTime, 120.0f)
END_CLASS_DEFAULT(HealthEnhancement, EnhancementItem, NULL, NULL)

HealthEnhancement::HealthEnhancement()
{
	m_nModelName = IDS_MODEL_HEALTHENHANCEMENT;
	m_nModelSkin = IDS_SKIN_HEALTHENHANCEMENT;
	m_nSoundName = IDS_SOUND_HEALTHENHANCEMENT;
	m_nEnhancementSubType = IST_ENHANCEMENT_HEALTH;
	m_eType		 = PIT_ENHANCEMENT_HEALTH;
}

BEGIN_CLASS(ArmorEnhancement)
	ADD_REALPROP(RespawnTime, 120.0f)
END_CLASS_DEFAULT(ArmorEnhancement, EnhancementItem, NULL, NULL)

ArmorEnhancement::ArmorEnhancement()
{
	m_nModelName = IDS_MODEL_ARMORENHANCEMENT;
	m_nModelSkin = IDS_SKIN_ARMORENHANCEMENT;
	m_nSoundName = IDS_SOUND_ARMORENHANCEMENT;
	m_nEnhancementSubType = IST_ENHANCEMENT_ARMOR;
	m_eType		 = PIT_ENHANCEMENT_ARMOR;
}

