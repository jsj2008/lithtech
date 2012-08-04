// ----------------------------------------------------------------------- //
//
// MODULE  : Key.cpp
//
// PURPOSE : Key implementation for Keyframer class
//
// CREATED : 10/7/97
//
// ----------------------------------------------------------------------- //

#include "key.h"
#include "cpp_server_de.h"


BEGIN_CLASS(Key)
	ADD_REALPROP(TimeStamp, 0.0f)
	ADD_STRINGPROP(SoundName, "")
	ADD_REALPROP_FLAG(SoundRadius, 0.0f,PF_RADIUS)
	ADD_BOOLPROP(LoopSound, DFALSE)
	ADD_STRINGPROP(MessageTarget, "")
	ADD_STRINGPROP(MessageName, "")
	ADD_STRINGPROP(BPrintMessage, "")
END_CLASS_DEFAULT_FLAGS(Key, B2BaseClass, NULL, NULL, CF_ALWAYSLOAD)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Key::Key()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

Key::Key() : B2BaseClass(OT_NORMAL)
{
	m_fTimeStamp		= 0.0f;
	m_fSoundRadius		= 0.0f;
	m_bLoopSound		= DFALSE;
	m_hstrSoundName		= NULL;
	m_hstrMessageTarget = NULL;
	m_hstrMessageName	= NULL;
	m_hstrBPrintMessage = NULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Key::~Key()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

Key::~Key()
{
	CServerDE* pServerDE = GetServerDE();
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
//	ROUTINE:	Key::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD Key::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			if (fData == 1.0f)
				ReadProp((ObjectCreateStruct*)pData);
			break;
		}

		case MID_INITIALUPDATE:
		{
			InitialUpdate((DVector *)pData);
			break;
		}

		default : break;
	}

	return B2BaseClass::EngineMessageFn(messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Key::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

DBOOL Key::ReadProp(ObjectCreateStruct *pStruct)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !pStruct) return DFALSE;

	char buf[MAX_CS_FILENAME_LEN];

	buf[0] = '\0';
	pServerDE->GetPropString("SoundName", buf, MAX_CS_FILENAME_LEN);
	if (buf[0]) m_hstrSoundName = pServerDE->CreateString(buf);

	buf[0] = '\0';
	pServerDE->GetPropString("MessageTarget", buf, MAX_CS_FILENAME_LEN);
	if (buf[0]) m_hstrMessageTarget = pServerDE->CreateString(buf);

	buf[0] = '\0';
	pServerDE->GetPropString("MessageName", buf, MAX_CS_FILENAME_LEN);
	if (buf[0]) m_hstrMessageName = pServerDE->CreateString(buf);

	buf[0] = '\0';
	pServerDE->GetPropString("BPrintMessage", buf, MAX_CS_FILENAME_LEN);
	if (buf[0]) m_hstrBPrintMessage = pServerDE->CreateString(buf);

	pServerDE->GetPropReal("TimeStamp", &m_fTimeStamp);
	pServerDE->GetPropReal("SoundRadius", &m_fSoundRadius);
	pServerDE->GetPropBool("LoopSound", &m_bLoopSound);

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Key::InitialUpdate()
//
//	PURPOSE:	First update
//
// ----------------------------------------------------------------------- //

DBOOL Key::InitialUpdate(DVector* pMovement)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return DFALSE;

	pServerDE->SetNextUpdate (m_hObject, 0.0f);

	return DTRUE;
}
