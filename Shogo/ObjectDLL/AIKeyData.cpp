// ----------------------------------------------------------------------- //
//
// MODULE  : CAIKeyData.cpp
//
// PURPOSE : CAIKeyData implementation for Keyframer class
//
// CREATED : 2/9/98
//
// ----------------------------------------------------------------------- //

#include "AIKeyData.h"
#include "cpp_engineobjects_de.h"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIKeyData::CAIKeyData()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

CAIKeyData::CAIKeyData()
{
	VEC_INIT(m_vPos);
	m_Name[0] = '\0';
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIKeyData::Copy()
//
//	PURPOSE:	Copy the Key object's data
//
// ----------------------------------------------------------------------- //

DBOOL CAIKeyData::Copy(HOBJECT hKey)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!hKey || !pServerDE) return DFALSE;

	pServerDE->GetObjectPos(hKey, &m_vPos);

	char* pName = pServerDE->GetObjectName(hKey);
	if (pName)
	{
		strncpy(m_Name, pName, MAX_AIKEY_NAME_LENGTH);
		m_Name[MAX_AIKEY_NAME_LENGTH] = '\0';
	}

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIKeyData::Save()
//
//	PURPOSE:	Save the key data's data
//
// ----------------------------------------------------------------------- //

void CAIKeyData::Save(HMESSAGEWRITE hWrite)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !hWrite) return;

	pServerDE->WriteToMessageVector(hWrite, &m_vPos);

	HSTRING hstr = DNULL;
	if (m_Name[0]) hstr = pServerDE->CreateString(m_Name);
	pServerDE->WriteToMessageHString(hWrite, hstr);
	if (hstr) pServerDE->FreeString(hstr);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIKeyData::Load()
//
//	PURPOSE:	Load the key data's data
//
// ----------------------------------------------------------------------- //

void CAIKeyData::Load(HMESSAGEREAD hRead)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !hRead) return;

	pServerDE->ReadFromMessageVector(hRead, &m_vPos);

	HSTRING hstr = pServerDE->ReadFromMessageHString(hRead);

	if (hstr)
	{
		char* pData = pServerDE->GetStringData(hstr);
		if (pData && pData[0])
		{
			SAFE_STRCPY(m_Name, pData);
		}

		pServerDE->FreeString(hstr);
	}
}
