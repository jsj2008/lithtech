// ----------------------------------------------------------------------- //
//
// MODULE  : ShogoSecret.cpp
//
// PURPOSE : ShogoSecret implementation
//
// CREATED : 8/6/98
//
// ----------------------------------------------------------------------- //

#include "ShogoSecret.h"
#include "RiotObjectUtilities.h"
#include "generic_msg_de.h"

BEGIN_CLASS(ShogoSecret)
	ADD_STRINGPROP(SoundFile, "sounds\\powerups\\letter.wav")
	ADD_STRINGPROP(Skin, "skins\\powerups\\shogo_letters.dtx")
END_CLASS_DEFAULT(ShogoSecret, PickupItem, NULL, NULL)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ShogoSecret::ShogoSecret()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

ShogoSecret::ShogoSecret()
{
	m_dwFlags |= FLAG_ENVIRONMENTMAP;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ShogoSecret::ObjectTouch
//
//	PURPOSE:	Handle touch notifications
//
// ----------------------------------------------------------------------- //

void ShogoSecret::ObjectTouch(HOBJECT hObject)
{
	if (!g_pServerDE) return;

	// make sure it was a player that touched us (or should we let AI pick us up too?)...

	if (!IsPlayer(hObject)) return;

	// send a message to ourselves, telling us we've been picked up...

	HMESSAGEWRITE hWrite = g_pServerDE->StartMessageToObject(this, m_hObject, MID_PICKEDUP);
	g_pServerDE->WriteToMessageFloat(hWrite, 0.0f);
	g_pServerDE->EndMessage(hWrite);
	
	PickupItem::ObjectTouch(hObject);
}


BEGIN_CLASS(Shogo_S)
	ADD_STRINGPROP_FLAG(Filename, "Models\\Powerups\\Shogo_s.abc", PF_DIMS)
END_CLASS_DEFAULT(Shogo_S, ShogoSecret, NULL, NULL)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Shogo_S::Shogo_S()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

Shogo_S::Shogo_S()
{
	m_eType = PIT_SHOGO_S;
}

BEGIN_CLASS(Shogo_H)
	ADD_STRINGPROP_FLAG(Filename, "Models\\Powerups\\Shogo_h.abc", PF_DIMS)
END_CLASS_DEFAULT(Shogo_H, ShogoSecret, NULL, NULL)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Shogo_H::Shogo_H()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

Shogo_H::Shogo_H()
{
	m_eType = PIT_SHOGO_H;
}

BEGIN_CLASS(Shogo_O)
	ADD_STRINGPROP_FLAG(Filename, "Models\\Powerups\\Shogo_o.abc", PF_DIMS)
END_CLASS_DEFAULT(Shogo_O, ShogoSecret, NULL, NULL)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Shogo_O::Shogo_O()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

Shogo_O::Shogo_O()
{
	m_eType = PIT_SHOGO_O;
}

BEGIN_CLASS(Shogo_G)
	ADD_STRINGPROP_FLAG(Filename, "Models\\Powerups\\Shogo_g.abc", PF_DIMS)
END_CLASS_DEFAULT(Shogo_G, ShogoSecret, NULL, NULL)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Shogo_G::Shogo_G()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

Shogo_G::Shogo_G()
{
	m_eType = PIT_SHOGO_G;
}
