// ----------------------------------------------------------------------- //
//
// MODULE  : BodyArmor.cpp
//
// PURPOSE : Riot Body Armor powerups - Implementation
//
// CREATED : 1/28/97
//
// ----------------------------------------------------------------------- //

#include "BodyArmor.h"
#include "ServerRes.h"

BEGIN_CLASS(BodyArmor_50)
END_CLASS_DEFAULT(BodyArmor_50, ArmorBase, NULL, NULL)

BodyArmor_50::BodyArmor_50() : ArmorBase()
{
	m_nArmor = 50;
	m_nModelName = IDS_MODEL_BODYARMOR_50;
	m_nModelSkin = IDS_SKIN_BODYARMOR_50;
	m_nSoundName = IDS_SOUND_BODYARMOR_50;
	m_eType		 = PIT_ARMOR_BODY50;
}

BEGIN_CLASS(BodyArmor_100)
END_CLASS_DEFAULT(BodyArmor_100, ArmorBase, NULL, NULL)

BodyArmor_100::BodyArmor_100() : ArmorBase()
{
	m_nArmor = 100;
	m_nModelName = IDS_MODEL_BODYARMOR_100;
	m_nModelSkin = IDS_SKIN_BODYARMOR_100;
	m_nSoundName = IDS_SOUND_BODYARMOR_100;
	m_eType		 = PIT_ARMOR_BODY100;
}

BEGIN_CLASS(BodyArmor_200)
END_CLASS_DEFAULT(BodyArmor_200, ArmorBase, NULL, NULL)

BodyArmor_200::BodyArmor_200() : ArmorBase()
{
	m_nArmor = 200;
	m_nModelName = IDS_MODEL_BODYARMOR_200;
	m_nModelSkin = IDS_SKIN_BODYARMOR_200;
	m_nSoundName = IDS_SOUND_BODYARMOR_200;
	m_eType		 = PIT_ARMOR_BODY200;
}
