// ----------------------------------------------------------------------- //
//
// MODULE  : Powerup.cpp
//
// PURPOSE : Powerup implementation
//
// CREATED : 10/1/97
//
// ----------------------------------------------------------------------- //

#include "Powerup.h"
#include "cpp_server_de.h"
#include "RiotObjectUtilities.h"
#include "PickupItem.h"
#include "BaseCharacter.h"

BEGIN_CLASS(Powerup)
END_CLASS_DEFAULT(Powerup, PickupItem, NULL, NULL)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Powerup::Powerup()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

Powerup::Powerup()
{
	m_bTimed = DFALSE;
	m_fTimeLimit = 30.0f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Powerup::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD Powerup::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return DFALSE;

	switch(messageID)
	{
		case MID_PRECREATE:
		{
			DDWORD dwRet = PickupItem::EngineMessageFn(messageID, pData, fData);

			// Make sure we have some type of powerup sound...

			if (!m_hstrSoundFile)
			{
				m_hstrSoundFile = pServerDE->CreateString("Sounds\\Powerup\\Powerup19.wav");
			}

			return dwRet;
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

	return PickupItem::EngineMessageFn(messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Powerup::ObjectTouch
//
//	PURPOSE:	Handle touch notifications
//
// ----------------------------------------------------------------------- //

void Powerup::ObjectTouch (HOBJECT hObject)
{
	if (g_pServerDE)
	{
		if (m_bTimed)
		{
			// see if it was a player that touched us
			// if so, add this powerup to it's list of timed powerups

			if (IsBaseCharacter (hObject))
			{
				CBaseCharacter* pCharacter = (CBaseCharacter*) g_pServerDE->HandleToObject (hObject);
				
				TimedPowerup* pPowerup = new TimedPowerup (g_pServerDE->GetTime() + m_fTimeLimit, m_eType);
				if (pPowerup)
				{
					if (!pCharacter || !pCharacter->AddTimedPowerup (pPowerup))
					{
						delete pPowerup;
					}
				}
			}
		}

		PickupItem::ObjectTouch (hObject);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Powerup::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void Powerup::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hWrite) return;

	pServerDE->WriteToMessageFloat(hWrite, m_fTimeLimit);
	pServerDE->WriteToMessageByte(hWrite, m_bTimed);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Powerup::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void Powerup::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hRead) return;

	m_fTimeLimit	= pServerDE->ReadFromMessageFloat(hRead);
	m_bTimed		= (DBOOL) pServerDE->ReadFromMessageByte(hRead);
}