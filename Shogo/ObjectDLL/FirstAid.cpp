// ----------------------------------------------------------------------- //
//
// MODULE  : FirstAid.cpp
//
// PURPOSE : Riot Energy/Health powerups - Implementation
//
// CREATED : 1/28/97
//
// ----------------------------------------------------------------------- //

#include "FirstAid.h"
#include "ServerRes.h"

BEGIN_CLASS(FirstAid_10)
END_CLASS_DEFAULT(FirstAid_10, FirstAidBase, NULL, NULL)

FirstAid_10::FirstAid_10() : FirstAidBase()
{
	m_nHealth = 10;
	m_nModelName = IDS_MODEL_FIRSTAID_10;
	m_nModelSkin = IDS_SKIN_FIRSTAID_10;
	m_nSoundName = IDS_SOUND_FIRSTAID_10;
	m_eType		 = PIT_FIRSTAID_10;
}

BEGIN_CLASS(FirstAid_15)
END_CLASS_DEFAULT(FirstAid_15, FirstAidBase, NULL, NULL)

FirstAid_15::FirstAid_15() : FirstAidBase()
{
	m_nHealth = 15;
	m_nModelName = IDS_MODEL_FIRSTAID_15;
	m_nModelSkin = IDS_SKIN_FIRSTAID_15;
	m_nSoundName = IDS_SOUND_FIRSTAID_15;
	m_eType		 = PIT_FIRSTAID_15;
}

BEGIN_CLASS(FirstAid_25)
END_CLASS_DEFAULT(FirstAid_25, FirstAidBase, NULL, NULL)

FirstAid_25::FirstAid_25() : FirstAidBase()
{
	m_nHealth = 25;
	m_nModelName = IDS_MODEL_FIRSTAID_25;
	m_nModelSkin = IDS_SKIN_FIRSTAID_25;
	m_nSoundName = IDS_SOUND_FIRSTAID_25;
	m_eType		 = PIT_FIRSTAID_25;
}

BEGIN_CLASS(FirstAid_50)
END_CLASS_DEFAULT(FirstAid_50, FirstAidBase, NULL, NULL)

FirstAid_50::FirstAid_50() : FirstAidBase()
{
	m_nHealth = 50;
	m_nModelName = IDS_MODEL_FIRSTAID_50;
	m_nModelSkin = IDS_SKIN_FIRSTAID_50;
	m_nSoundName = IDS_SOUND_FIRSTAID_50;
	m_eType		 = PIT_FIRSTAID_50;
}
