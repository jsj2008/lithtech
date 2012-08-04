// ----------------------------------------------------------------------- //
//
// MODULE  : Key.cpp
//
// PURPOSE : Key implementation for Keyframer class
//
// CREATED : 10/7/97
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "key.h"
#include "iltserver.h"


BEGIN_CLASS(Key)
	ADD_REALPROP(TimeStamp, 0.0f)
	ADD_STRINGPROP(SoundName, "")
	ADD_REALPROP_FLAG(SoundRadius, 0.0f,PF_RADIUS)
	ADD_STRINGPROP(MessageTarget, "")
	ADD_STRINGPROP(MessageName, "")
	ADD_STRINGPROP(BPrintMessage, "")
	ADD_VECTORPROP_FLAG(BezierPrev, PF_BEZIERPREVTANGENT)
	ADD_VECTORPROP_FLAG(BezierNext, PF_BEZIERNEXTTANGENT)
	ADD_LONGINTPROP(LightFrames, 1)
END_CLASS_DEFAULT(Key, BaseClass, NULL, NULL)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Key::Key()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

Key::Key() : BaseClass(OT_NORMAL)
{
	m_fTimeStamp		= 0.0f;
	m_fSoundRadius		= 0.0f;
	m_hstrSoundName		= NULL;
	m_hstrMessageTarget = NULL;
	m_hstrMessageName	= NULL;
	m_hstrBPrintMessage = NULL;
	m_vPitchYawRoll.Init();

	// {MD}
	m_BezierPrevTangent.Init();
	m_BezierNextTangent.Init();

	m_LightFrames = 1;
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
    ILTServer* pServerDE = GetServerDE();
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

uint32 Key::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			if (fData == PRECREATE_WORLDFILE)
			{
				ReadProp((ObjectCreateStruct*)pData);
			}

			break;
		}

		case MID_INITIALUPDATE:
		{
            InitialUpdate((LTVector *)pData);
			break;
		}

		default : break;
	}

	return BaseClass::EngineMessageFn(messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Key::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

LTBOOL Key::ReadProp(ObjectCreateStruct *)
{
    ILTServer* pServerDE = GetServerDE();
    if (!pServerDE) return LTFALSE;

	GenericProp genProp;

	pServerDE->GetPropReal("TimeStamp", &m_fTimeStamp);
	pServerDE->GetPropReal("SoundRadius", &m_fSoundRadius);

	if (pServerDE->GetPropGeneric("SoundName", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
			m_hstrSoundName = pServerDE->CreateString(genProp.m_String);
		}
	}

	if (pServerDE->GetPropGeneric("MessageTarget", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
			m_hstrMessageTarget = pServerDE->CreateString(genProp.m_String);
		}
	}

	if (pServerDE->GetPropGeneric("MessageName", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
			m_hstrMessageName = pServerDE->CreateString(genProp.m_String);
		}
	}

	if (pServerDE->GetPropGeneric("BPrintMessage", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
			m_hstrBPrintMessage = pServerDE->CreateString(genProp.m_String);
		}
	}

	pServerDE->GetPropRotationEuler("Rotation", &m_vPitchYawRoll);

	// {MD}
	// Read the bezier tangent vectors.
	if (pServerDE->GetPropGeneric("BezierPrev", &genProp) == LT_OK)
	{
		m_BezierPrevTangent = genProp.m_Vec;
	}

	if (pServerDE->GetPropGeneric("BezierNext", &genProp) == LT_OK)
	{
		m_BezierNextTangent = genProp.m_Vec;
	}

	if (pServerDE->GetPropGeneric("LightFrames", &genProp) == LT_OK)
	{
		m_LightFrames = genProp.m_Long;
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Key::InitialUpdate()
//
//	PURPOSE:	First update
//
// ----------------------------------------------------------------------- //

LTBOOL Key::InitialUpdate(LTVector* pMovement)
{
    ILTServer* pServerDE = GetServerDE();
    if (!pServerDE) return LTFALSE;

	pServerDE->SetNextUpdate (m_hObject, 0.0f);

    return LTTRUE;
}