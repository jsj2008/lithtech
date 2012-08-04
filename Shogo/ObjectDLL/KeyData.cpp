// ----------------------------------------------------------------------- //
//
// MODULE  : KeyData.cpp
//
// PURPOSE : KeyData implementation for Keyframer class
//
// CREATED : 12/31/97
//
// ----------------------------------------------------------------------- //

#include "KeyData.h"
#include "cpp_server_de.h"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyData::KeyData()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

KeyData::KeyData()
{
	ROT_INIT(m_rRot);
	VEC_INIT(m_vPos);

	m_nKeyType			= POSITION_KEY;
	m_fTimeStamp		= 0.0f;
	m_fRealTime			= 0.0f;
	m_fSoundRadius		= 0.0f;
	m_hstrSoundName		= NULL;
	m_hstrMessageTarget = NULL;
	m_hstrMessageName	= NULL;
	m_hstrBPrintMessage = NULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyData::~KeyData()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

KeyData::~KeyData()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (pServerDE)
	{
		if (m_hstrSoundName) pServerDE->FreeString (m_hstrSoundName);
		if (m_hstrMessageTarget) pServerDE->FreeString (m_hstrMessageTarget);
		if (m_hstrMessageName) pServerDE->FreeString (m_hstrMessageName);
		if (m_hstrBPrintMessage) pServerDE->FreeString (m_hstrBPrintMessage);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyData::Copy()
//
//	PURPOSE:	Copy the Key object's data
//
// ----------------------------------------------------------------------- //

DBOOL KeyData::Copy(Key* pKey)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pKey || !pServerDE) return DFALSE;

	pServerDE->GetObjectRotation(pServerDE->ObjectToHandle(pKey), &m_rRot);
	pServerDE->GetObjectPos(pServerDE->ObjectToHandle(pKey), &m_vPos);
			
	m_fTimeStamp	= pKey->m_fTimeStamp;
	m_fSoundRadius	= pKey->m_fSoundRadius;


	// Copy the sound name if applicable...

	if (pKey->m_hstrSoundName)
	{
		m_hstrSoundName = pServerDE->CopyString(pKey->m_hstrSoundName);
		m_nKeyType |= SOUND_KEY;
	}


	// We only care about messages if we have both a target and a message...

	if (pKey->m_hstrMessageTarget && pKey->m_hstrMessageName)
	{
		m_hstrMessageTarget = pServerDE->CopyString(pKey->m_hstrMessageTarget);
		m_hstrMessageName = pServerDE->CopyString(pKey->m_hstrMessageName);
		m_nKeyType |= MESSAGE_KEY;
	}


	// Copy the BPrint message...

	if (pKey->m_hstrBPrintMessage)
	{
		m_hstrBPrintMessage = pServerDE->CopyString(pKey->m_hstrBPrintMessage);
		m_nKeyType |= BPRINT_KEY;
	}


	return DTRUE;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyData::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void KeyData::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !hWrite) return;

	pServerDE->WriteToMessageRotation(hWrite, &m_rRot);
	pServerDE->WriteToMessageVector(hWrite, &m_vPos);
	pServerDE->WriteToMessageDWord(hWrite, m_nKeyType);
	pServerDE->WriteToMessageFloat(hWrite, m_fTimeStamp);
	pServerDE->WriteToMessageFloat(hWrite, m_fRealTime);
	pServerDE->WriteToMessageFloat(hWrite, m_fSoundRadius);
	pServerDE->WriteToMessageHString(hWrite, m_hstrSoundName);
	pServerDE->WriteToMessageHString(hWrite, m_hstrMessageTarget);
	pServerDE->WriteToMessageHString(hWrite, m_hstrMessageName);
	pServerDE->WriteToMessageHString(hWrite, m_hstrBPrintMessage);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyData::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void KeyData::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !hRead) return;

	pServerDE->ReadFromMessageRotation(hRead, &m_rRot);
	pServerDE->ReadFromMessageVector(hRead, &m_vPos);
	m_nKeyType			= pServerDE->ReadFromMessageDWord(hRead);
	m_fTimeStamp		= pServerDE->ReadFromMessageFloat(hRead);
	m_fRealTime			= pServerDE->ReadFromMessageFloat(hRead);
	m_fSoundRadius		= pServerDE->ReadFromMessageFloat(hRead);
	m_hstrSoundName		= pServerDE->ReadFromMessageHString(hRead);
	m_hstrMessageTarget	= pServerDE->ReadFromMessageHString(hRead);
	m_hstrMessageName	= pServerDE->ReadFromMessageHString(hRead);
	m_hstrBPrintMessage	= pServerDE->ReadFromMessageHString(hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyData::CacheFiles
//
//	PURPOSE:	CacheFiles resources used by this object
//
// ----------------------------------------------------------------------- //

void KeyData::CacheFiles()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	char* pFile = DNULL;
	if (m_hstrSoundName)
	{
		pFile = pServerDE->GetStringData(m_hstrSoundName);
		if (pFile)
		{
			pServerDE->CacheFile(FT_SOUND, pFile);
		}
	}
}