// ----------------------------------------------------------------------- //
//
// MODULE  : UltraPowerupItem.cpp
//
// PURPOSE : Baseclass implementation for UltraPowerup items
//
// CREATED : 1/28/98
//
// ----------------------------------------------------------------------- //

#include "UltraPowerupItem.h"
#include "InventoryTypes.h"
#include "RiotMsgIds.h"
#include "cpp_server_de.h"
#include "RiotObjectUtilities.h"

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //
//
//	CLASS:		UpgradeItem
//
//	PURPOSE:	Baseclass for upgrade items
//
// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

BEGIN_CLASS(UltraPowerupItem)
	ADD_REALPROP(RespawnTime, 120.0f)
	ADD_STRINGPROP_FLAG(SoundFile, "", PF_HIDDEN)
	ADD_BOOLPROP_FLAG(Rotate, 1, PF_HIDDEN)
END_CLASS_DEFAULT(UltraPowerupItem, Powerup, NULL, NULL)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	UltraPowerupItem::UltraPowerupItem()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

UltraPowerupItem::UltraPowerupItem() : Powerup()
{
	m_bRotate = DFALSE;
	m_nModelName = 0;
	m_nModelSkin = 0;
	m_nSoundName = 0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	UltraPowerupItem::~UltraPowerupItem()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

UltraPowerupItem::~UltraPowerupItem()
{
	if( !g_pServerDE ) return;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	UltraPowerupItem::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD UltraPowerupItem::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	DDWORD dwRet;
	GenericProp genProp;

	if( g_pServerDE )
	{
		switch(messageID)
		{
			case MID_PRECREATE:
			{
				dwRet = Powerup::EngineMessageFn(messageID, pData, fData);
				if ( fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP )
				{
					if( g_pServerDE->GetPropGeneric( "RespawnTime", &genProp ) == DE_OK )
						m_fRespawnDelay = genProp.m_Float;
					else
						m_fRespawnDelay = 120.0f;
				}
					
				PostPropRead((ObjectCreateStruct*)pData);
				return dwRet;
				break;
			}
		
			case MID_INITIALUPDATE:
			{
				if (fData != INITIALUPDATE_SAVEGAME)
				{
					DVector vDims;
					VEC_SET(vDims, 20.0f, 25.0f, 10.0f);
					g_pServerDE->SetObjectDims(m_hObject, &vDims);
				}
				break;
			}

			case MID_SAVEOBJECT:
			{
				Save((HMESSAGEWRITE)pData, (DDWORD)fData);
			}
			break;

			case MID_LOADOBJECT:
			{
				Load((HMESSAGEREAD)pData, (DDWORD)fData);
			}
			break;

			default : break;
		}
	}

	return Powerup::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	UltraPowerupItem::PostPropRead()
//
//	PURPOSE:	Update Properties
//
// ----------------------------------------------------------------------- //

void UltraPowerupItem::PostPropRead(ObjectCreateStruct *pStruct)
{
	if (!g_pServerDE) return;

	// set the model name, skin name, and sound file name

	HSTRING hString = DNULL;
	
	hString = g_pServerDE->FormatString (m_nModelName);
	if (hString)
	{
		SAFE_STRCPY(pStruct->m_Filename, g_pServerDE->GetStringData (hString));
	}
	else
	{
		SAFE_STRCPY(pStruct->m_Filename, "dummy string");	// this will force the dummy model
	}
	g_pServerDE->FreeString (hString);

	hString = g_pServerDE->FormatString (m_nModelSkin);
	if (hString) SAFE_STRCPY(pStruct->m_SkinName, g_pServerDE->GetStringData (hString));
	g_pServerDE->FreeString (hString);

	if( m_hstrSoundFile )
		g_pServerDE->FreeString( m_hstrSoundFile );
	m_hstrSoundFile = g_pServerDE->FormatString (m_nSoundName);

	m_bRotate = DFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	UltraPowerupItem::ObjectTouch
//
//	PURPOSE:	Handle touch notifications
//
// ----------------------------------------------------------------------- //

void UltraPowerupItem::ObjectTouch (HOBJECT hObject)
{
	if (!g_pServerDE) return;

	// make sure it was a player that touched us (or should we let AI pick us up too?)...

	if (!IsPlayer (hObject)) return;

	// send a message to ourselves, telling us we've been picked up...

	HMESSAGEWRITE hWrite = g_pServerDE->StartMessageToObject(this, m_hObject, MID_PICKEDUP);
	g_pServerDE->WriteToMessageFloat (hWrite, m_fRespawnDelay);
	g_pServerDE->EndMessage (hWrite);
	
	Powerup::ObjectTouch (hObject);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	UltraPowerupItem::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void UltraPowerupItem::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hWrite) return;

	pServerDE->WriteToMessageDWord(hWrite, m_nModelName);
	pServerDE->WriteToMessageDWord(hWrite, m_nModelSkin);
	pServerDE->WriteToMessageDWord(hWrite, m_nSoundName);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	UltraPowerupItem::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void UltraPowerupItem::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hRead) return;

	m_nModelName	= pServerDE->ReadFromMessageDWord(hRead);
	m_nModelSkin	= pServerDE->ReadFromMessageDWord(hRead);
	m_nSoundName	= pServerDE->ReadFromMessageDWord(hRead);
}
