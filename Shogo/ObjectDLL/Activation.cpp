//----------------------------------------------------------
//
// MODULE  : Activation.cpp
//
// PURPOSE : Activation aggreate
//
// CREATED : 12/14/97
//
//----------------------------------------------------------

#include "Activation.h"
#include "cpp_server_de.h"


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CActivation::CActivation()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CActivation::CActivation() : Aggregate()
{
	m_hstrActivateCondition = DNULL;
	m_bIsActive = DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CActivation::~CActivation()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CActivation::~CActivation()
{
	if (m_hstrActivateCondition)
	{
		CServerDE* pServerDE = BaseClass::GetServerDE();
		if (!pServerDE) return;

		pServerDE->FreeString(m_hstrActivateCondition);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CActivation::EngineMessageFn()
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD CActivation::EngineMessageFn(LPBASECLASS pObject, DDWORD messageID, void *pData, DFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			if (fData == 1.0f)
			{
				ReadProp(pObject, (ObjectCreateStruct*)pData);
			}
		}
		break;

		case MID_INITIALUPDATE:
		{
			InitialUpdate(pObject, fData);
		}
		break;

		case MID_SAVEOBJECT:
		{
			Save(pObject, (HMESSAGEWRITE)pData);
		}
		break;

		case MID_LOADOBJECT:
		{
			Load(pObject, (HMESSAGEREAD)pData);
		}
		break;

		default : break;
	}

	return Aggregate::EngineMessageFn(pObject, messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CActivation::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

void CActivation::ReadProp(LPBASECLASS pObject, ObjectCreateStruct* pStruct)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	char buf[MAX_CS_FILENAME_LEN];
	buf[0] = '\0';

	pServerDE->GetPropString("ActivateCondition", buf, MAX_CS_FILENAME_LEN);

	if (buf[0])
	{
		m_hstrActivateCondition = pServerDE->CreateString(buf);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CActivation::InitialUpdate()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //
	
void CActivation::InitialUpdate(LPBASECLASS pObject, DFLOAT fInfo)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !pObject) return;

	if (fInfo == INITIALUPDATE_SAVEGAME) return;


	// See if a condition must be met to make us active...

	if (!m_hstrActivateCondition) return;

	char* pString = pServerDE->GetStringData(m_hstrActivateCondition);
	if (!pString) return;

	char buf[300];  // Temp buffer, so we don't modify real data...
	SAFE_STRCPY(buf, pString);

	char* pVariable = strtok(buf, " ");
	if (!pVariable) return;


	// Get the value of the variable...

	char* pActualValue = strtok(NULL, " ");

	HCONVAR	hVar = pServerDE->GetGameConVar(pVariable);

	if (hVar)
	{
		char* pExpectedValue = pServerDE->GetVarValueString(hVar);

		if (pExpectedValue && pActualValue)
		{
			if (stricmp(pExpectedValue, pActualValue) == 0)
			{
				return;
			}
		}
	}


	// If we get here, the activation condition wasn't met...

	m_bIsActive = DFALSE;
	pServerDE->SetObjectFlags(pObject->m_hObject, 0);
	pServerDE->SetObjectState(pObject->m_hObject, OBJSTATE_INACTIVE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CActivation::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CActivation::Save(LPBASECLASS pObject, HMESSAGEWRITE hWrite)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !hWrite) return;

	pServerDE->WriteToMessageHString(hWrite, m_hstrActivateCondition);
	pServerDE->WriteToMessageByte(hWrite, m_bIsActive);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CActivation::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CActivation::Load(LPBASECLASS pObject, HMESSAGEREAD hRead)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !hRead) return;

	m_hstrActivateCondition	= pServerDE->ReadFromMessageHString(hRead);
	m_bIsActive				= (DBOOL) pServerDE->ReadFromMessageByte(hRead);

	if (!m_bIsActive)
	{
		pServerDE->SetObjectFlags(pObject->m_hObject, 0);
		pServerDE->SetObjectState(pObject->m_hObject, OBJSTATE_INACTIVE);
	}
}