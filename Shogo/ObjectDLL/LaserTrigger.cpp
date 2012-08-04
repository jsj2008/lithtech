// ----------------------------------------------------------------------- //
//
// MODULE  : LaserTrigger.h
//
// PURPOSE : LaserTrigger - Implementation
//
// CREATED : 4/30/98
//
// ----------------------------------------------------------------------- //

#include "LaserTrigger.h"
#include "cpp_server_de.h"
#include "ClientServerShared.h"
#include "stdio.h"

BEGIN_CLASS(LaserTrigger)
	ADD_VECTORPROP_VAL_FLAG(Dims, 5.0f, 5.0f, 200.0f, PF_DIMS) 
	ADD_COLORPROP(Color, 255.0f, 0.0f, 0.0f) 
	ADD_REALPROP(Alpha, 0.7f)
END_CLASS_DEFAULT(LaserTrigger, Trigger, NULL, NULL)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LaserTrigger::LaserTrigger()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

LaserTrigger::LaserTrigger() : Trigger()
{
	VEC_SET(m_vColor, 1.0f, 1.0f, 1.0f);
	m_fAlpha = 1.0f;
	m_hModel = DNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LaserTrigger::~LaserTrigger()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

LaserTrigger::~LaserTrigger()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	if (m_hModel)
	{
		pServerDE->RemoveObject(m_hModel);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LaserTrigger::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD LaserTrigger::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			if (fData == 1.0f || fData == 2.0f)
			{
				ReadProp((ObjectCreateStruct*)pData);
			}
		}
		break;

		case MID_INITIALUPDATE:
		{
			DDWORD dwRet = Trigger::EngineMessageFn(messageID, pData, fData);
			InitialUpdate((int)fData);
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

	return Trigger::EngineMessageFn(messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LaserTrigger::HandleTriggerMsg
//
//	PURPOSE:	Handle object messages
//
// ----------------------------------------------------------------------- //

void LaserTrigger::HandleTriggerMsg(HOBJECT hSender, HMESSAGEREAD hRead)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	HSTRING hMsg = pServerDE->ReadFromMessageHString(hRead);
	char* pStr = pServerDE->GetStringData(hMsg);
	if (!pStr) return;

	// See if we hide/show ourself :)

	if (_stricmp(pStr, "LOCK") == 0)
	{
		if (m_hModel)
		{
			DDWORD dwFlags = pServerDE->GetObjectFlags(m_hModel);
			pServerDE->SetObjectFlags(m_hModel, dwFlags & ~FLAG_VISIBLE);
		}
	}
	else if (_stricmp(pStr, "UNLOCK") == 0) 
	{
		if (m_hModel)
		{
			DDWORD dwFlags = pServerDE->GetObjectFlags(m_hModel);
			pServerDE->SetObjectFlags(m_hModel, dwFlags | FLAG_VISIBLE);
		}
	}

	pServerDE->FreeString(hMsg);

	// Reset it so trigger can read it :)
	
	pServerDE->ResetRead(hRead);

	Trigger::HandleTriggerMsg(hSender, hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LaserTrigger::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

DBOOL LaserTrigger::ReadProp(ObjectCreateStruct *pData)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !pData) return DFALSE;

	GenericProp genProp;

	if (pServerDE->GetPropGeneric("Alpha", &genProp) == DE_OK)
		m_fAlpha = genProp.m_Float;

	if (pServerDE->GetPropGeneric("Color", &genProp) == DE_OK)
		VEC_COPY(m_vColor, genProp.m_Vec);

	VEC_DIVSCALAR(m_vColor, m_vColor, 255.0f);
	VEC_MULSCALAR(m_vColor, m_vColor, m_fAlpha);

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LaserTrigger::InitialUpdate()
//
//	PURPOSE:	Handle initial update
//
// ----------------------------------------------------------------------- //

void LaserTrigger::InitialUpdate(int nInfo)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	// Don't need to create the model when restoring a save game...

	if (nInfo == INITIALUPDATE_SAVEGAME) return;


	DVector vPos, vDims;
	pServerDE->GetObjectPos(m_hObject, &vPos);
	pServerDE->GetObjectDims(m_hObject, &vDims);
	
	DRotation rRot;
	pServerDE->GetObjectRotation(m_hObject, &rRot);

	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

	SAFE_STRCPY(theStruct.m_Filename, "Models\\Props\\1x1_square.abc");
	SAFE_STRCPY(theStruct.m_SkinName, "SpecialFX\\Explosions\\beam.dtx");

	VEC_COPY(theStruct.m_Pos, vPos);
	ROT_COPY(theStruct.m_Rotation, rRot);
	theStruct.m_ObjectType  = OT_MODEL;
	theStruct.m_Flags		= FLAG_VISIBLE;

	HCLASS hClass = g_pServerDE->GetClass("BaseClass");
	if (!hClass) return;

	LPBASECLASS pModel = pServerDE->CreateObject(hClass, &theStruct);
	if (!pModel) return;

	m_hModel = pModel->m_hObject;

	VEC_MULSCALAR(vDims, vDims, 2.0f);
	pServerDE->ScaleObject(m_hModel, &vDims);
	pServerDE->SetObjectColor(m_hModel, m_vColor.x, m_vColor.y, m_vColor.z, m_fAlpha);
	pServerDE->SetObjectUserFlags(m_hModel, USRFLG_NIGHT_INFRARED);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LaserTrigger::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void LaserTrigger::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hWrite) return;

	pServerDE->WriteToLoadSaveMessageObject(hWrite, m_hModel);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LaserTrigger::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void LaserTrigger::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hRead) return;

	pServerDE->ReadFromLoadSaveMessageObject(hRead, &m_hModel);
}