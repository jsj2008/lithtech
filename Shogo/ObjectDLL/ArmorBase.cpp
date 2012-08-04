// ----------------------------------------------------------------------- //
//
// MODULE  : ArmorBase.cpp
//
// PURPOSE : Riot Armor powerups - Implementation
//
// CREATED : 1/28/97
//
// ----------------------------------------------------------------------- //

#include "ArmorBase.h"
#include "RiotMsgIds.h"
#include "cpp_server_de.h"

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //
//
//	CLASS:		ArmorBase
//
//	PURPOSE:	Base Armor powerups
//
// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

BEGIN_CLASS(ArmorBase)
	ADD_BOOLPROP_FLAG(Rotate, 1, PF_HIDDEN)
	ADD_REALPROP(RespawnTime, 20.0f)
	ADD_STRINGPROP_FLAG(SoundFile, "", PF_HIDDEN)
END_CLASS_DEFAULT(ArmorBase, Powerup, NULL, NULL)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ArmorBase::ArmorBase
//
//	PURPOSE:	Handle object initialization
//
// ----------------------------------------------------------------------- //

ArmorBase::ArmorBase() : Powerup()
{
	m_nArmor = 0;
	m_nModelName = 0;
	m_nModelSkin = 0;
	m_nSoundName = 0;
	m_bRotate = DFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ArmorBase::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD ArmorBase::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return DFALSE;

	switch(messageID)
	{
		case MID_PRECREATE:
		{
			DDWORD dwRet = Powerup::EngineMessageFn(messageID, pData, fData);

			ObjectCreateStruct* pStruct = (ObjectCreateStruct*)pData;
			if (pStruct)
			{
				HSTRING hString = DNULL;
				
				hString = pServerDE->FormatString (m_nModelName);
				if (hString)
				{
					SAFE_STRCPY(pStruct->m_Filename, pServerDE->GetStringData (hString));
				}
				else
				{
					SAFE_STRCPY(pStruct->m_Filename, "dummy string");	// this will force the dummy model
				}
				pServerDE->FreeString (hString);

				hString = pServerDE->FormatString (m_nModelSkin);
				if (hString) SAFE_STRCPY(pStruct->m_SkinName, pServerDE->GetStringData (hString));
				pServerDE->FreeString (hString);

				if( m_hstrSoundFile )
					pServerDE->FreeString( m_hstrSoundFile );
				m_hstrSoundFile = pServerDE->FormatString (m_nSoundName);
			}

			m_bRotate = DFALSE;

			return dwRet;
		}
		
		case MID_INITIALUPDATE:
		{
			DVector vDims;
			VEC_SET(vDims, 20.0f, 25.0f, 10.0f);

			pServerDE->SetObjectDims(m_hObject, &vDims);
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

	return Powerup::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ArmorBase::AddPowerup
//
//	PURPOSE:	Add powerup to object
//
// ----------------------------------------------------------------------- //

void ArmorBase::ObjectTouch(HOBJECT hObject)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	HMESSAGEWRITE hMessage = pServerDE->StartMessageToObject(this, hObject, MID_REPAIR);
	pServerDE->WriteToMessageFloat(hMessage, (float)m_nArmor);
	pServerDE->EndMessage(hMessage);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ArmorBase::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void ArmorBase::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hWrite) return;

	pServerDE->WriteToMessageDWord(hWrite, m_nArmor);
	pServerDE->WriteToMessageDWord(hWrite, m_nModelName);
	pServerDE->WriteToMessageDWord(hWrite, m_nModelSkin);
	pServerDE->WriteToMessageDWord(hWrite, m_nSoundName);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ArmorBase::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void ArmorBase::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hRead) return;

	m_nArmor	 = pServerDE->ReadFromMessageDWord(hRead);
	m_nModelName = pServerDE->ReadFromMessageDWord(hRead);
	m_nModelSkin = pServerDE->ReadFromMessageDWord(hRead);
	m_nSoundName = pServerDE->ReadFromMessageDWord(hRead);
}