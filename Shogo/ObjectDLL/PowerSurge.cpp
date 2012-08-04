// ----------------------------------------------------------------------- //
//
// MODULE  : PowerSurge.cpp
//
// PURPOSE : Riot Energy/Health powerups - Implementation
//
// CREATED : 1/28/97
//
// ----------------------------------------------------------------------- //

#include "PowerSurge.h"
#include "ServerRes.h"

BEGIN_CLASS(PowerSurge_50)
END_CLASS_DEFAULT(PowerSurge_50, FirstAidBase, NULL, NULL)

PowerSurge_50::PowerSurge_50() : FirstAidBase()
{
	m_nHealth = 50;
	m_nModelName = IDS_MODEL_POWERSURGE_50;
	m_nModelSkin = IDS_SKIN_POWERSURGE_50;
	m_nSoundName = IDS_SOUND_POWERSURGE_50;
	m_eType		 = PIT_POWERSURGE_50;
}

BEGIN_CLASS(PowerSurge_100)
END_CLASS_DEFAULT(PowerSurge_100, FirstAidBase, NULL, NULL)

PowerSurge_100::PowerSurge_100() : FirstAidBase()
{
	m_nHealth = 100;
	m_nModelName = IDS_MODEL_POWERSURGE_100;
	m_nModelSkin = IDS_SKIN_POWERSURGE_100;
	m_nSoundName = IDS_SOUND_POWERSURGE_100;
	m_eType		 = PIT_POWERSURGE_100;
}

BEGIN_CLASS(PowerSurge_150)
END_CLASS_DEFAULT(PowerSurge_150, FirstAidBase, NULL, NULL)

PowerSurge_150::PowerSurge_150() : FirstAidBase()
{
	m_nHealth = 150;
	m_nModelName = IDS_MODEL_POWERSURGE_150;
	m_nModelSkin = IDS_SKIN_POWERSURGE_150;
	m_nSoundName = IDS_SOUND_POWERSURGE_150;
	m_eType		 = PIT_POWERSURGE_150;
}

BEGIN_CLASS(PowerSurge_250)
END_CLASS_DEFAULT(PowerSurge_250, FirstAidBase, NULL, NULL)

PowerSurge_250::PowerSurge_250() : FirstAidBase()
{
	m_nHealth = 250;
	m_nModelName = IDS_MODEL_POWERSURGE_250;
	m_nModelSkin = IDS_SKIN_POWERSURGE_250;
	m_nSoundName = IDS_SOUND_POWERSURGE_250;
	m_eType		 = PIT_POWERSURGE_250;
}
