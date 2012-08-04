// ----------------------------------------------------------------------- //
//
// MODULE  : ArmorRepair.cpp
//
// PURPOSE : Riot Body Armor powerups - Implementation
//
// CREATED : 1/28/97
//
// ----------------------------------------------------------------------- //

#include "ArmorRepair.h"
#include "ServerRes.h"

BEGIN_CLASS(ArmorRepair_100)
END_CLASS_DEFAULT(ArmorRepair_100, ArmorBase, NULL, NULL)

ArmorRepair_100::ArmorRepair_100() : ArmorBase()
{
	m_nArmor = 100;
	m_nModelName = IDS_MODEL_ARMORREPAIR_100;
	m_nModelSkin = IDS_SKIN_ARMORREPAIR_100;
	m_nSoundName = IDS_SOUND_ARMORREPAIR_100;
	m_eType		 = PIT_ARMOR_REPAIR100;
}

BEGIN_CLASS(ArmorRepair_250)
END_CLASS_DEFAULT(ArmorRepair_250, ArmorBase, NULL, NULL)

ArmorRepair_250::ArmorRepair_250() : ArmorBase()
{
	m_nArmor = 250;
	m_nModelName = IDS_MODEL_ARMORREPAIR_250;
	m_nModelSkin = IDS_SKIN_ARMORREPAIR_250;
	m_nSoundName = IDS_SOUND_ARMORREPAIR_250;
	m_eType		 = PIT_ARMOR_REPAIR250;
}

BEGIN_CLASS(ArmorRepair_500)
END_CLASS_DEFAULT(ArmorRepair_500, ArmorBase, NULL, NULL)

ArmorRepair_500::ArmorRepair_500() : ArmorBase()
{
	m_nArmor = 500;
	m_nModelName = IDS_MODEL_ARMORREPAIR_500;
	m_nModelSkin = IDS_SKIN_ARMORREPAIR_500;
	m_nSoundName = IDS_SOUND_ARMORREPAIR_500;
	m_eType		 = PIT_ARMOR_REPAIR500;
}
