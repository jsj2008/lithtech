// ----------------------------------------------------------------------- //
//
// MODULE  : Mine.cpp
//
// PURPOSE : Mine - Definition
//
// CREATED : 2/26/00
//
// (c) 2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "Mine.h"

BEGIN_CLASS(Mine)

// Parent overrides...

	ADD_STRINGPROP_FLAG(ImpactFXName, "Mine", PF_STATICLIST)
    ADD_BOOLPROP_FLAG(RemoveWhenDone, LTTRUE, PF_HIDDEN)


// New properties...

    ADD_BOOLPROP(MoveToFloor, LTTRUE)

	ADD_VECTORPROP_VAL_FLAG(Dims, 25.0f, 25.0f, 25.0f, PF_DIMS)
	ADD_REALPROP_FLAG(ActivateDelay, 0.1f, 0)

END_CLASS_DEFAULT_FLAGS(Mine, Explosion, NULL, NULL, 0)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Mine::Mine()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

Mine::Mine() : Explosion()
{
	m_fActivateDelay = 0.2f;
	m_vDims.Init(25, 25, 25);

    m_bMoveToFloor  = LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Mine::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 Mine::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_UPDATE :
		{
			Update();
		}
		break;

		case MID_INITIALUPDATE :
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
                g_pLTServer->SetObjectDims(m_hObject, &m_vDims);

				m_fxcs.fMinRadius = (m_vDims.x + m_vDims.y + m_vDims.z) / 3.0f;
				m_fxcs.fMaxRadius = m_fDamageRadius;

                HMESSAGEWRITE hMessage = g_pLTServer->StartSpecialEffectMessage(this);
                g_pLTServer->WriteToMessageByte(hMessage, SFX_MINE_ID);
                m_fxcs.Write(g_pLTServer, hMessage);
                g_pLTServer->EndMessage(hMessage);

				if (m_bMoveToFloor)
				{
					MoveObjectToFloor(m_hObject);
				}
			}
		}
		break;

		case MID_TOUCHNOTIFY :
		{
			HOBJECT hObj = (HOBJECT)pData;
			if (IsCharacter(hObj))
			{
				TouchNotify(hObj);
			}
		}
		break;

		case MID_PRECREATE :
		{
			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				ReadProp();
			}

			ObjectCreateStruct *pStruct = (ObjectCreateStruct*)pData;
			if (pStruct)
			{
				pStruct->m_Flags |= FLAG_TOUCH_NOTIFY | FLAG_GOTHRUWORLD | FLAG_FORCECLIENTUPDATE;
			}
		}
		break;

		case MID_SAVEOBJECT:
		{
            Save((HMESSAGEWRITE)pData, (uint32)fData);
		}
		break;

		case MID_LOADOBJECT:
		{
            Load((HMESSAGEREAD)pData, (uint32)fData);
		}
		break;

		default : break;
	}

	return Explosion::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Mine::ReadProp
//
//	PURPOSE:	Read object properties
//
// ----------------------------------------------------------------------- //

void Mine::ReadProp()
{
	GenericProp genProp;

    if (g_pLTServer->GetPropGeneric("ActivateDelay", &genProp) == LT_OK)
	{
		m_fActivateDelay = genProp.m_Float;
	}

    if (g_pLTServer->GetPropGeneric("Dims", &genProp) == LT_OK)
	{
		m_vDims = genProp.m_Vec;
	}

    if (g_pLTServer->GetPropGeneric("MoveToFloor", &genProp) == LT_OK)
	{
		 m_bMoveToFloor = genProp.m_Bool;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Mine::TouchNotify
//
//	PURPOSE:	Handle character touch
//
// ----------------------------------------------------------------------- //

void Mine::TouchNotify(HOBJECT hObj)
{
	// TO DO:?
	// Play activate sound...

	// Wait for it...

	if (m_fActivateDelay > 0.0f)
	{
        g_pLTServer->SetNextUpdate(m_hObject, m_fActivateDelay);
	}
	else
	{
		Start();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Mine::Update
//
//	PURPOSE:	Handle updates
//
// ----------------------------------------------------------------------- //

void Mine::Update()
{
	Start();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Mine::GetBoundingBoxColor()
//
//	PURPOSE:	Get the color of the bounding box
//
// ----------------------------------------------------------------------- //

LTVector Mine::GetBoundingBoxColor()
{
    return LTVector(0.5, 0.5, 0);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Mine::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void Mine::Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags)
{
	if (!hWrite) return;

    g_pLTServer->WriteToMessageFloat(hWrite, m_fActivateDelay);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Mine::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void Mine::Load(HMESSAGEREAD hRead, uint32 dwLoadFlags)
{
	if (!hRead) return;

    m_fActivateDelay = g_pLTServer->ReadFromMessageFloat(hRead);
}