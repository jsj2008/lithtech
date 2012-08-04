// ----------------------------------------------------------------------- //
//
// MODULE  : CClientMarkSFX.cpp
//
// PURPOSE : CClientMarkSFX - Implementation
//
// CREATED : 11/6/97
//
// ----------------------------------------------------------------------- //

#include "ClientMarkSFX.h"
#include "cpp_server_de.h"
#include "SFXMsgIds.h"


BEGIN_CLASS(CClientMarkSFX)
END_CLASS_DEFAULT_FLAGS(CClientMarkSFX, CClientSFX, NULL, NULL, CF_HIDDEN)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientMarkSFX::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD CClientMarkSFX::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return 0;

	switch(messageID)
	{
		case MID_INITIALUPDATE:
		{
			pServerDE->SetNextUpdate(m_hObject, 0.0001f);
		}
		break;

		case MID_LINKBROKEN:
		{
			pServerDE->RemoveObject(m_hObject);
		}
		break;

		default : break;
	}

	return CClientSFX::EngineMessageFn(messageID, pData, lData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientMarkSFX::Setup
//
//	PURPOSE:	Send message to client with data
//
// ----------------------------------------------------------------------- //

void CClientMarkSFX::Setup( DVector *pvPos, DVector *pvDir, DBYTE nWeaponType, DBYTE nAmmoType, SurfaceType eSurfType, HOBJECT hLinkObj)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	DRotation rotation;
	pServerDE->AlignRotation( &rotation, pvDir, pvDir );

	HSTRING hstrSprite = DNULL;
	char *pSprite = GetMarkSprite(nWeaponType, nAmmoType, eSurfType);
	if (pSprite) hstrSprite = pServerDE->CreateString(pSprite);

	DFLOAT fScale = (nAmmoType == AMMO_BULLET) ? 0.05f : 0.035f;

	if (hstrSprite)
	{
//		DVector vColor;
//		VEC_SET(vColor,0.0f,0.0f,0.0f);
 
		HMESSAGEWRITE hMessage = pServerDE->StartSpecialEffectMessage(this);
		pServerDE->WriteToMessageByte(hMessage, SFX_MARK_ID);
		pServerDE->WriteToMessageRotation(hMessage, &rotation);
		pServerDE->WriteToMessageFloat( hMessage, fScale );
		pServerDE->WriteToMessageHString( hMessage, hstrSprite );
		pServerDE->WriteToMessageByte(hMessage, DTRUE);
		pServerDE->EndMessage(hMessage);

		pServerDE->FreeString(hstrSprite);
	}

	if (hLinkObj)
	{
		pServerDE->CreateInterObjectLink(m_hObject, hLinkObj);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientMarkSFX::GetMarkSprite()
//
//	PURPOSE:	Get a mark sprite associated with this weapon and surface
//
// ----------------------------------------------------------------------- //

char* CClientMarkSFX::GetMarkSprite(DBYTE nWeaponType, DBYTE nAmmoType, SurfaceType eSurfType)
{
	if (!g_pServerDE) return DNULL;

	char* pMark = DNULL;
	// Don't place bullet holes for non-vector weapons...

	switch(eSurfType)
	{
		case SURFTYPE_UNKNOWN:
			break;

		case SURFTYPE_GLASS:
			pMark = g_pServerDE->IntRandom(0,1) ? "sprites/glshole1.spr" : "sprites/glshole2.spr";
			break;

		case SURFTYPE_METAL:
			pMark = "sprites/blthole.spr";
			break;

		case SURFTYPE_PLASTIC:
		case SURFTYPE_CLOTH:
			pMark = "sprites/blthole2.spr";
			break;

		case SURFTYPE_WOOD:
			pMark = "sprites/blthole5.spr";
			break;

		case SURFTYPE_STONE:
		default:
			pMark = g_pServerDE->IntRandom(0,1) ? "sprites/blthole3.spr" : "sprites/blthole4.spr";
			break;
	}

	return pMark;
}


