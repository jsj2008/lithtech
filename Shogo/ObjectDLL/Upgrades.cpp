// ----------------------------------------------------------------------- //
//
// MODULE  : Upgrades.cpp
//
// PURPOSE : Implementation of upgrade items
//
// CREATED : 1/28/98
//
// ----------------------------------------------------------------------- //

#include "Upgrades.h"
#include "ServerRes.h"
#include "InventoryTypes.h"

BEGIN_CLASS(DamageUpgrade)
END_CLASS_DEFAULT(DamageUpgrade, UpgradeItem, NULL, NULL)

DamageUpgrade::DamageUpgrade()
{
	m_nModelName = IDS_MODEL_DAMAGEUPGRADE;
	m_nModelSkin = IDS_SKIN_DAMAGEUPGRADE;
	m_nSoundName = IDS_SOUND_DAMAGEUPGRADE;
	m_nUpgradeSubType = IST_UPGRADE_DAMAGE;
	m_eType		 = PIT_UPGRADE_DAMAGE;
}

BEGIN_CLASS(ProtectionUpgrade)
END_CLASS_DEFAULT(ProtectionUpgrade, UpgradeItem, NULL, NULL)

ProtectionUpgrade::ProtectionUpgrade()
{
	m_nModelName = IDS_MODEL_PROTECTIONUPGRADE;
	m_nModelSkin = IDS_SKIN_PROTECTIONUPGRADE;
	m_nSoundName = IDS_SOUND_PROTECTIONUPGRADE;
	m_nUpgradeSubType = IST_UPGRADE_PROTECTION;
	m_eType		 = PIT_UPGRADE_PROTECTION;
}

BEGIN_CLASS(RegenUpgrade)
END_CLASS_DEFAULT(RegenUpgrade, UpgradeItem, NULL, NULL)

RegenUpgrade::RegenUpgrade()
{
	m_nModelName = IDS_MODEL_REGENUPGRADE;
	m_nModelSkin = IDS_SKIN_REGENUPGRADE;
	m_nSoundName = IDS_SOUND_REGENUPGRADE;
	m_nUpgradeSubType = IST_UPGRADE_REGEN;
	m_eType		 = PIT_UPGRADE_REGEN;
}

BEGIN_CLASS(HealthUpgrade)
END_CLASS_DEFAULT(HealthUpgrade, UpgradeItem, NULL, NULL)

HealthUpgrade::HealthUpgrade()
{
	m_nModelName = IDS_MODEL_HEALTHUPGRADE;
	m_nModelSkin = IDS_SKIN_HEALTHUPGRADE;
	m_nSoundName = IDS_SOUND_HEALTHUPGRADE;
	m_nUpgradeSubType = IST_UPGRADE_HEALTH;
	m_eType		 = PIT_UPGRADE_HEALTH;
}

BEGIN_CLASS(ArmorUpgrade)
END_CLASS_DEFAULT(ArmorUpgrade, UpgradeItem, NULL, NULL)

ArmorUpgrade::ArmorUpgrade()
{
	m_nModelName = IDS_MODEL_ARMORUPGRADE;
	m_nModelSkin = IDS_SKIN_ARMORUPGRADE;
	m_nSoundName = IDS_SOUND_ARMORUPGRADE;
	m_nUpgradeSubType = IST_UPGRADE_ARMOR;
	m_eType		 = PIT_UPGRADE_ARMOR;
}

BEGIN_CLASS(TargetingUpgrade)
END_CLASS_DEFAULT(TargetingUpgrade, UpgradeItem, NULL, NULL)

TargetingUpgrade::TargetingUpgrade()
{
	m_nModelName = IDS_MODEL_TARGETINGUPGRADE;
	m_nModelSkin = IDS_SKIN_TARGETINGUPGRADE;
	m_nSoundName = IDS_SOUND_TARGETINGUPGRADE;
	m_nUpgradeSubType = IST_UPGRADE_TARGETING;
	m_eType		 = PIT_UPGRADE_TARGETING;
}

