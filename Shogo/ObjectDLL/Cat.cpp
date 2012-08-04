// ----------------------------------------------------------------------- //
//
// MODULE  : Cat.cpp
//
// PURPOSE : Cat pickup item
//
// CREATED : 3/26/98
//
// ----------------------------------------------------------------------- //

#include "Cat.h"
#include "InventoryTypes.h"
#include "cpp_server_de.h"
#include "generic_msg_de.h"
#include "DamageTypes.h"
#include "RiotObjectUtilities.h"

BEGIN_CLASS(Cat)
	ADD_STRINGPROP(SqueakyTarget, "")
	ADD_STRINGPROP(SqueakyMessage, "")
	ADD_STRINGPROP(DeathSound, "Sounds\\Props\\CatDeath.wav")
	ADD_STRINGPROP(DeathTriggerTarget, "")
	ADD_STRINGPROP(DeathTriggerMessage, "")
	ADD_STRINGPROP(SqueakedAtSound, "Sounds\\Props\\kitty.wav")
	ADD_REALPROP(SoundRadius, 3000.0f)
END_CLASS_DEFAULT(Cat, InventoryItem, NULL, NULL)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Cat::Cat()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

Cat::Cat() : InventoryItem()
{
	m_hstrSqueakyTarget			= DNULL;
	m_hstrSqueakyMessage		= DNULL;
	m_hstrDeathSound			= DNULL;
	m_hstrSqueakedAtSound		= DNULL;
	m_hstrDeathTriggerTarget	= DNULL;
	m_hstrDeathTriggerMessage	= DNULL;
	m_fSoundRadius				= 3000.0f;
	m_eType						= PIT_CAT;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Cat::~Cat()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

Cat::~Cat()
{
	if( !g_pServerDE ) return;

	if (m_hstrSqueakyTarget)
	{
		g_pServerDE->FreeString(m_hstrSqueakyTarget);
	}
	
	if (m_hstrSqueakyMessage)
	{
		g_pServerDE->FreeString(m_hstrSqueakyMessage);
	}

	if (m_hstrSqueakedAtSound)
	{
		g_pServerDE->FreeString(m_hstrSqueakedAtSound);
	}

	if (m_hstrDeathSound)
	{
		g_pServerDE->FreeString(m_hstrDeathSound);
	}

	if (m_hstrDeathTriggerTarget)
	{
		g_pServerDE->FreeString(m_hstrDeathTriggerTarget);
	}

	if (m_hstrDeathTriggerMessage)
	{
		g_pServerDE->FreeString(m_hstrDeathTriggerMessage);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Cat::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD Cat::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	if( g_pServerDE )
	{
		switch(messageID)
		{
			case MID_PRECREATE:
			{
				ObjectCreateStruct* pInfo = (ObjectCreateStruct*)pData;
				if (!pInfo) break;

				if (fData == 1.0f || fData == 2.0f)
				{
					ReadProp(pInfo);
				}

				pInfo->m_Flags |= FLAG_RAYHIT;
				
				CacheFiles();
			}
			break;

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

	return InventoryItem::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Cat::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

DDWORD Cat::ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead)
{
	switch(messageID)
	{
		case MID_DAMAGE:
		{
			CServerDE* pServerDE = GetServerDE();
			if (!pServerDE) break;
			
			DVector vDir;
			pServerDE->ReadFromMessageVector(hRead, &vDir);
			DFLOAT fDamage   = pServerDE->ReadFromMessageFloat(hRead);
			DamageType eType = (DamageType)pServerDE->ReadFromMessageByte(hRead);
			HOBJECT hHeHitMe = pServerDE->ReadFromMessageObject(hRead);

			if (eType == DT_SQUEAKY)
			{
				if (m_hstrSqueakedAtSound)
				{
					char* pSound = pServerDE->GetStringData(m_hstrSqueakedAtSound);
					if (pSound) PlaySoundFromObject(m_hObject, pSound, m_fSoundRadius, SOUNDPRIORITY_MISC_MEDIUM );
				}

				// If we're supposed to trigger something, trigger it here
	
				if (m_hstrSqueakyTarget && m_hstrSqueakyMessage)
				{
					SendTriggerMsgToObjects(this, m_hstrSqueakyTarget, m_hstrSqueakyMessage);
				}
			}
			else
			{
				if (m_hstrDeathSound)
				{
					DVector vPos;
					pServerDE->GetObjectPos(m_hObject, &vPos);
					char* pSound = pServerDE->GetStringData(m_hstrDeathSound);
					if (pSound) PlaySoundFromPos(&vPos, pSound, m_fSoundRadius, SOUNDPRIORITY_MISC_MEDIUM );
				}

				if (m_hstrDeathTriggerTarget && m_hstrDeathTriggerMessage)
				{
					SendTriggerMsgToObjects(this, m_hstrDeathTriggerTarget, m_hstrDeathTriggerMessage);
				}

				pServerDE->RemoveObject(m_hObject);
			}
			break;
		}

		default : break;
	}

	return InventoryItem::ObjectMessageFn(hSender, messageID, hRead);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Cat::ReadProp()
//
//	PURPOSE:	Read Properties
//
// ----------------------------------------------------------------------- //

void Cat::ReadProp(ObjectCreateStruct *pStruct)
{
	if (!g_pServerDE || !pStruct) return;

	GenericProp genProp;

	if ( g_pServerDE->GetPropGeneric("SqueakyTarget", &genProp ) == DE_OK )
	{
		if ( genProp.m_String[0] )
			m_hstrSqueakyTarget = g_pServerDE->CreateString( genProp.m_String );
	}
	
	if ( g_pServerDE->GetPropGeneric("SqueakyMessage", &genProp ) == DE_OK )
	{
		if( genProp.m_String[0] )
			m_hstrSqueakyMessage = g_pServerDE->CreateString( genProp.m_String );
	}	
	
	if ( g_pServerDE->GetPropGeneric("DeathSound", &genProp ) == DE_OK )
	{
		if( genProp.m_String[0] )
			m_hstrDeathSound = g_pServerDE->CreateString( genProp.m_String );
	}

	if (g_pServerDE->GetPropGeneric("DeathTriggerTarget", &genProp) == DE_OK)
	{
		if (genProp.m_String[0])
		{
			m_hstrDeathTriggerTarget = g_pServerDE->CreateString(genProp.m_String);
		}
	}

	if (g_pServerDE->GetPropGeneric("DeathTriggerMessage", &genProp) == DE_OK)
	{
		if (genProp.m_String[0])
		{
			m_hstrDeathTriggerMessage = g_pServerDE->CreateString(genProp.m_String);
		}
	}

	if ( g_pServerDE->GetPropGeneric("SqueakedAtSound", &genProp ) == DE_OK )
	{
		if( genProp.m_String[0] )
			m_hstrSqueakedAtSound = g_pServerDE->CreateString( genProp.m_String );
	}

	if ( g_pServerDE->GetPropGeneric("SoundRadius", &genProp ) == DE_OK )
	{
		m_fSoundRadius = genProp.m_Float;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Cat::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void Cat::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	if (!g_pServerDE || !hWrite) return;

	g_pServerDE->WriteToMessageHString(hWrite, m_hstrSqueakyTarget);
	g_pServerDE->WriteToMessageHString(hWrite, m_hstrSqueakyMessage);
	g_pServerDE->WriteToMessageHString(hWrite, m_hstrSqueakedAtSound);
	g_pServerDE->WriteToMessageHString(hWrite, m_hstrDeathSound);
	g_pServerDE->WriteToMessageHString(hWrite, m_hstrDeathTriggerTarget);
	g_pServerDE->WriteToMessageHString(hWrite, m_hstrDeathTriggerMessage);
	g_pServerDE->WriteToMessageFloat(hWrite, m_fSoundRadius);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Cat::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void Cat::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	if (!g_pServerDE || !hRead) return;

	m_hstrSqueakyTarget			= g_pServerDE->ReadFromMessageHString(hRead);
	m_hstrSqueakyMessage		= g_pServerDE->ReadFromMessageHString(hRead);
	m_hstrSqueakedAtSound		= g_pServerDE->ReadFromMessageHString(hRead);
	m_hstrDeathSound			= g_pServerDE->ReadFromMessageHString(hRead);
	m_hstrDeathTriggerTarget	= g_pServerDE->ReadFromMessageHString(hRead);
	m_hstrDeathTriggerMessage	= g_pServerDE->ReadFromMessageHString(hRead);
	m_fSoundRadius				= g_pServerDE->ReadFromMessageFloat(hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Cat::CacheFiles
//
//	PURPOSE:	Cache resources used by the object
//
// ----------------------------------------------------------------------- //

void Cat::CacheFiles()
{
	if (!g_pServerDE) return;

	char* pFile = DNULL;

	if (m_hstrSqueakedAtSound)
	{
		pFile = g_pServerDE->GetStringData(m_hstrSqueakedAtSound);
		if (pFile)
		{
			 g_pServerDE->CacheFile(FT_SOUND ,pFile);
		}
	}

	if (m_hstrDeathSound)
	{
		pFile = g_pServerDE->GetStringData(m_hstrDeathSound);
		if (pFile)
		{
			 g_pServerDE->CacheFile(FT_SOUND ,pFile);
		}
	}
}
