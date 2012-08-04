// ----------------------------------------------------------------------- //
//
// MODULE  : SpellPickups.cpp
//
// PURPOSE : Blood2 Ammunition pickups - implementation
//
// CREATED : 12/11/97
//
// ----------------------------------------------------------------------- //

#include "SpellPickups.h"
#include "cpp_server_de.h"
#include "SharedDefs.h"
#include "generic_msg_de.h"


// *********************************************************************** //
//
//	CLASS:		SpellPickup
//
//	PURPOSE:	Base Spell pickup item
//
// *********************************************************************** //


BEGIN_CLASS(SpellPickup)
END_CLASS_DEFAULT(SpellPickup, PickupObject, NULL, NULL)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SpellPickup::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD SpellPickup::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return DFALSE;

	switch(messageID)
	{
		case MID_POSTPROPREAD:
		{
			ObjectCreateStruct *pStruct = (ObjectCreateStruct*)pData;
			
			// Set the model filename
			_mbscpy((unsigned char*)pStruct->m_Filename, (const unsigned char*)"Models/Powerups/");
			_mbscat((unsigned char*)pStruct->m_Filename, (const unsigned char*)m_szFile);
			_mbscat((unsigned char*)pStruct->m_Filename, (const unsigned char*)".abc");

			// Set the skin filename
			_mbscpy((unsigned char*)pStruct->m_SkinName, (const unsigned char*)"Skins/Powerups/");
			_mbscat((unsigned char*)pStruct->m_SkinName, (const unsigned char*)m_szFile);
			_mbscat((unsigned char*)pStruct->m_SkinName, (const unsigned char*)".dtx");

			break;
		}

		case MID_INITIALUPDATE:
		{
			DVector vDims;
			VEC_SET(vDims, 20.0f, 25.0f, 10.0f);

			pServerDE->SetObjectDims(m_hObject, &vDims);
		}

		default : break;
	}

	return PickupObject::EngineMessageFn(messageID, pData, lData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SpellPickup::ObjectTouch
//
//	PURPOSE:	handles an object touch
//
// ----------------------------------------------------------------------- //

void SpellPickup::ObjectTouch(HOBJECT hObject)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	HMESSAGEWRITE hMessage = pServerDE->StartMessageToObject(this, hObject, MID_ADDAMMO);
	pServerDE->WriteToMessageByte(hMessage, (DBYTE)m_nSpellType);
	pServerDE->EndMessage(hMessage);
}


/*
// *********************************************************************** //
//
//	CLASS:		BulletAmmoPU
//
//	PURPOSE:	Bullet
//
// *********************************************************************** //

BEGIN_CLASS(BulletAmmoPU)
	ADD_LONGINTPROP( Count, 100 )
END_CLASS_DEFAULT(BulletAmmoPU, AmmoPickup, NULL, NULL)

// Constructor
BulletAmmoPU::BulletAmmoPU() : AmmoPickup()
{
	m_nAmmoType		= AMMO_BULLET;
	m_fAmmoCount	= 100.0f;
	m_szFile		= "BulletAmmo_pu";
}

*/