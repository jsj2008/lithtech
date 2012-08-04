// ----------------------------------------------------------------------- //
//
// MODULE  : GameStartPoint.cpp
//
// PURPOSE : GameStartPoint - Definition
//
// CREATED : 3/26/98
//
// ----------------------------------------------------------------------- //

#include "GameStartPoint.h"
#include "cpp_server_de.h"
#include "Trigger.h"
#include <mbstring.h>


BEGIN_CLASS(GameStartPoint)
	ADD_BOOLPROP(Multiplayer, DFALSE)
	ADD_STRINGPROP(TriggerTarget, "")
	ADD_STRINGPROP(TriggerMessage, "")
	ADD_LONGINTPROP(TeamID, 0)				// Team ID (1 or 2, or 0 for any)
END_CLASS_DEFAULT_FLAGS(GameStartPoint, StartPoint, NULL, NULL, CF_ALWAYSLOAD)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameStartPoint::GameStartPoint
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

GameStartPoint::GameStartPoint() : StartPoint()
{ 
	m_bMultiplayer			= DFALSE; 
	m_hstrName				= DNULL;
	VEC_INIT(m_vPitchYawRoll);
	m_hstrTriggerTarget		= DNULL;
	m_hstrTriggerMessage	= DNULL;
	m_nTeamID               = 0;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameStartPoint::~GameStartPoint
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

GameStartPoint::~GameStartPoint()
{ 
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	if (m_hstrName)
	{
		pServerDE->FreeString(m_hstrName);
	}
	if (m_hstrTriggerTarget)
	{
		pServerDE->FreeString(m_hstrTriggerTarget);
	}
	if (m_hstrTriggerMessage)
	{
		pServerDE->FreeString(m_hstrTriggerMessage);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameStartPoint::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD GameStartPoint::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			DDWORD dwRetVal = StartPoint::EngineMessageFn(messageID, pData, fData);
			if (fData == 1.0f)
				ReadProp();
			
			ObjectCreateStruct *pStruct = (ObjectCreateStruct*)pData;
			// change the name so it's easy to find later
			if (pStruct)
			{
				_mbscpy((unsigned char*)pStruct->m_Name, (const unsigned char*)"Blood2StartPoint");
			}
			return dwRetVal;
		}

		default : break;
	}

	return StartPoint::EngineMessageFn(messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameStartPoint::ReadProp
//
//	PURPOSE:	Read properties
//
// ----------------------------------------------------------------------- //

DBOOL GameStartPoint::ReadProp()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return DFALSE;

	GenericProp genProp;

	if (pServerDE->GetPropGeneric("Multiplayer", &genProp) == DE_OK)
		m_bMultiplayer = genProp.m_Bool;

	if (pServerDE->GetPropGeneric("Name", &genProp) == DE_OK)
	{
		if (genProp.m_String[0])
			 m_hstrName = pServerDE->CreateString(genProp.m_String);
	}

	if (g_pServerDE->GetPropGeneric("TeamID", &genProp) == DE_OK)
	{
		m_nTeamID = (int)genProp.m_Long;
	}

	DRotation rRot;
	ROT_INIT(rRot);

	if (pServerDE->GetPropGeneric("Rotation", &genProp) == DE_OK)
	{
		m_vPitchYawRoll.x = genProp.m_Vec.x;
		m_vPitchYawRoll.y = genProp.m_Vec.y;
		m_vPitchYawRoll.z = genProp.m_Vec.z;
	}

	if (pServerDE->GetPropGeneric("TriggerTarget", &genProp) == DE_OK)
	{
		if (genProp.m_String[0])
			 m_hstrTriggerTarget = pServerDE->CreateString(genProp.m_String);
	}

	if (pServerDE->GetPropGeneric("TriggerMessage", &genProp) == DE_OK)
	{
		if (genProp.m_String[0])
			 m_hstrTriggerMessage = pServerDE->CreateString(genProp.m_String);
	}
	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameStartPoint::SendTrigger
//
//	PURPOSE:	Send it's trigger message (if applicable)
//
// ----------------------------------------------------------------------- //

void GameStartPoint::SendTrigger()
{
	if (!m_hstrTriggerTarget || !m_hstrTriggerMessage || !g_pServerDE)
		return;

	SendTriggerMsgToObjects(this, m_hstrTriggerTarget, m_hstrTriggerMessage);
}