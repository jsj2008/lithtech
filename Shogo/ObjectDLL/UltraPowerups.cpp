// ----------------------------------------------------------------------- //
//
// MODULE  : UltraPowerups.cpp
//
// PURPOSE : Implementation of UltraPowerup items
//
// CREATED : 2/18/98
//
// ----------------------------------------------------------------------- //

#include "UltraPowerups.h"
#include "ServerRes.h"
#include "InventoryTypes.h"
#include "cpp_server_de.h"
#include "generic_msg_de.h"
#include "basecharacter.h"
#include "playerobj.h"
#include "RiotObjectUtilities.h"

// UltraDamage

BEGIN_CLASS(UltraDamage)
	ADD_REALPROP(RespawnTime, 120.0f)
END_CLASS_DEFAULT(UltraDamage, UltraPowerupItem, NULL, NULL)

UltraDamage::UltraDamage()
{
	m_nModelName = IDS_MODEL_ULTRADAMAGE;
	m_nModelSkin = IDS_SKIN_ULTRADAMAGE;
	m_nSoundName = IDS_SOUND_ULTRADAMAGE;
	m_bTimed	 = DTRUE;
	m_fTimeLimit = 30.0f;
	m_eType		 = PIT_ULTRA_DAMAGE;
}

// UltraPowerSurge

BEGIN_CLASS(UltraPowerSurge)
	ADD_REALPROP(RespawnTime, 300.0f)
END_CLASS_DEFAULT(UltraPowerSurge, UltraPowerupItem, NULL, NULL)

UltraPowerSurge::UltraPowerSurge()
{
	m_nModelName = IDS_MODEL_ULTRAPOWERSURGE;
	m_nModelSkin = IDS_SKIN_ULTRAPOWERSURGE;
	m_nSoundName = IDS_SOUND_ULTRAPOWERSURGE;
	m_eType		 = PIT_ULTRA_POWERSURGE;
}

void UltraPowerSurge::ObjectTouch (HOBJECT hObject)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;
	
	// make sure it's a mech, not an on-foot guy that touched us...

	if (!IsBaseCharacter (hObject)) return;
	CBaseCharacter* pChar = (CBaseCharacter*)pServerDE->HandleToObject (hObject);
	if (!pChar || !pChar->IsMecha()) return;

	// attempt to heal the object

	HMESSAGEWRITE hMessage = pServerDE->StartMessageToObject(this, hObject, MID_ULTRAHEAL);
	pServerDE->WriteToMessageFloat(hMessage, 1000.0f);
	pServerDE->EndMessage(hMessage);

	UltraPowerupItem::ObjectTouch (hObject);
}

// UltraHealth

BEGIN_CLASS(UltraHealth)
	ADD_REALPROP(RespawnTime, 300.0f)
END_CLASS_DEFAULT(UltraHealth, UltraPowerupItem, NULL, NULL)

UltraHealth::UltraHealth()
{
	m_nModelName = IDS_MODEL_ULTRAHEALTH;
	m_nModelSkin = IDS_SKIN_ULTRAHEALTH;
	m_nSoundName = IDS_SOUND_ULTRAHEALTH;
	m_eType		 = PIT_ULTRA_HEALTH;
}

void UltraHealth::ObjectTouch (HOBJECT hObject)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;
	
	// make sure it's an on-foot guy, not a mech that touched us...

	if (!IsBaseCharacter (hObject)) return;
	CBaseCharacter* pChar = (CBaseCharacter*)pServerDE->HandleToObject (hObject);
	if (!pChar || pChar->IsMecha()) return;

	// attempt to heal the object

	HMESSAGEWRITE hMessage = pServerDE->StartMessageToObject(this, hObject, MID_ULTRAHEAL);
	pServerDE->WriteToMessageFloat(hMessage, 100.0f);
	pServerDE->EndMessage(hMessage);

	UltraPowerupItem::ObjectTouch (hObject);
}

// UltraShield

BEGIN_CLASS(UltraShield)
	ADD_REALPROP(RespawnTime, 300.0f)
END_CLASS_DEFAULT(UltraShield, UltraPowerupItem, NULL, NULL)

UltraShield::UltraShield()
{
	m_nModelName = IDS_MODEL_ULTRASHIELD;
	m_nModelSkin = IDS_SKIN_ULTRASHIELD;
	m_nSoundName = IDS_SOUND_ULTRASHIELD;
	m_bTimed	 = DTRUE;
	m_fTimeLimit = 30.0f;
	m_eType		 = PIT_ULTRA_SHIELD;
}

// UltraStealth

BEGIN_CLASS(UltraStealth)
	ADD_REALPROP(RespawnTime, 300.0f)
END_CLASS_DEFAULT(UltraStealth, UltraPowerupItem, NULL, NULL)

UltraStealth::UltraStealth()
{
	m_nModelName = IDS_MODEL_ULTRASTEALTH;
	m_nModelSkin = IDS_SKIN_ULTRASTEALTH;
	m_nSoundName = IDS_SOUND_ULTRASTEALTH;
	m_bTimed	 = DTRUE;
	m_fTimeLimit = 30.0f;
	m_eType		 = PIT_ULTRA_STEALTH;
}

void UltraStealth::ObjectTouch (HOBJECT hObject)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;
	
	// tell the character to save it's current model color

	if (IsBaseCharacter (hObject))
	{
		CBaseCharacter* pChar = (CBaseCharacter*)pServerDE->HandleToObject (hObject);
		pChar->SaveModelColor();
	}
	
	UltraPowerupItem::ObjectTouch (hObject);
}

// UltraReflect

BEGIN_CLASS(UltraReflect)
	ADD_REALPROP(RespawnTime, 300.0f)
END_CLASS_DEFAULT(UltraReflect, UltraPowerupItem, NULL, NULL)

UltraReflect::UltraReflect()
{
	m_nModelName = IDS_MODEL_ULTRAREFLECT;
	m_nModelSkin = IDS_SKIN_ULTRAREFLECT;
	m_nSoundName = IDS_SOUND_ULTRAREFLECT;
	m_bTimed	 = DTRUE;
	m_fTimeLimit = 30.0f;
	m_eType		 = PIT_ULTRA_REFLECT;
}

// UltraNightVision

BEGIN_CLASS(UltraNightVision)
	ADD_REALPROP(RespawnTime, 300.0f)
END_CLASS_DEFAULT(UltraNightVision, UltraPowerupItem, NULL, NULL)

UltraNightVision::UltraNightVision()
{
	m_nModelName = IDS_MODEL_ULTRANIGHTVISION;
	m_nModelSkin = IDS_SKIN_ULTRANIGHTVISION;
	m_nSoundName = IDS_SOUND_ULTRANIGHTVISION;
	m_bTimed	 = DTRUE;
	m_fTimeLimit = 30.0f;
	m_eType		 = PIT_ULTRA_NIGHTVISION;
}

void UltraNightVision::ObjectTouch (HOBJECT hObject)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;
	
	// make sure it's an on-foot guy, not a mech that touched us...

	if (!IsBaseCharacter (hObject)) return;
	CBaseCharacter* pChar = (CBaseCharacter*)pServerDE->HandleToObject (hObject);
	if (!pChar || pChar->IsMecha()) return;
	
	UltraPowerupItem::ObjectTouch (hObject);
}

// UltraInfrared

BEGIN_CLASS(UltraInfrared)
	ADD_REALPROP(RespawnTime, 300.0f)
END_CLASS_DEFAULT(UltraInfrared, UltraPowerupItem, NULL, NULL)

UltraInfrared::UltraInfrared()
{
	m_nModelName = IDS_MODEL_ULTRAINFRARED;
	m_nModelSkin = IDS_SKIN_ULTRAINFRARED;
	m_nSoundName = IDS_SOUND_ULTRAINFRARED;
	m_bTimed	 = DTRUE;
	m_fTimeLimit = 30.0f;
	m_eType		= PIT_ULTRA_INFRARED;
}

void UltraInfrared::ObjectTouch (HOBJECT hObject)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;
	
	// make sure it's a mech, not an on-foot guy that touched us...

	if (!IsBaseCharacter (hObject)) return;
	CBaseCharacter* pChar = (CBaseCharacter*)pServerDE->HandleToObject (hObject);
	if (!pChar || !pChar->IsMecha()) return;
	
	UltraPowerupItem::ObjectTouch (hObject);
}

// UltraSilencer

BEGIN_CLASS(UltraSilencer)
	ADD_REALPROP(RespawnTime, 300.0f)
END_CLASS_DEFAULT(UltraSilencer, UltraPowerupItem, NULL, NULL)

UltraSilencer::UltraSilencer()
{
	m_nModelName = IDS_MODEL_ULTRASILENCER;
	m_nModelSkin = IDS_SKIN_ULTRASILENCER;
	m_nSoundName = IDS_SOUND_ULTRASILENCER;
	m_bTimed	 = DTRUE;
	m_fTimeLimit = 30.0f;
	m_eType		 = PIT_ULTRA_SILENCER;
}

void UltraSilencer::ObjectTouch (HOBJECT hObject)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;
	
	// make sure it's an on-foot guy, not a mech that touched us...

	if (!IsBaseCharacter (hObject)) return;
	CBaseCharacter* pChar = (CBaseCharacter*)pServerDE->HandleToObject (hObject);
	if (!pChar || pChar->IsMecha()) return;
	
	UltraPowerupItem::ObjectTouch (hObject);
}

// UltraRestore

BEGIN_CLASS(UltraRestore)
	ADD_REALPROP(RespawnTime, 120.0f)
END_CLASS_DEFAULT(UltraRestore, UltraPowerupItem, NULL, NULL)

UltraRestore::UltraRestore()
{
	m_nModelName = IDS_MODEL_ULTRARESTORE;
	m_nModelSkin = IDS_SKIN_ULTRARESTORE;
	m_nSoundName = IDS_SOUND_ULTRARESTORE;
	m_eType		 = PIT_ULTRA_RESTORE;
}

void UltraRestore::ObjectTouch (HOBJECT hObject)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;
	
	// send it very large values for heal and repair - the normal heal and repair functions
	// will cap the values at the maximums...

	HMESSAGEWRITE hMessage = pServerDE->StartMessageToObject(this, hObject, MID_HEAL);
	pServerDE->WriteToMessageFloat(hMessage, 20000.0f);
	pServerDE->EndMessage(hMessage);
	
	hMessage = pServerDE->StartMessageToObject(this, hObject, MID_REPAIR);
	pServerDE->WriteToMessageFloat(hMessage, 20000.0f);
	pServerDE->EndMessage(hMessage);

	UltraPowerupItem::ObjectTouch (hObject);
}
