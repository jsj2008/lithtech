// ----------------------------------------------------------------------- //
//
// MODULE  : ObjectiveSprite.cpp
//
// PURPOSE : ObjectiveSprite class - implementation
//
// CREATED : 12/07/97
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ObjectiveSprite.h"
#include "iltserver.h"
#include "ObjectMsgs.h"
#include "ParsedMsg.h"
#include "SFXFuncs.h"

LINKFROM_MODULE( ObjectiveSprite );

#pragma force_active on
BEGIN_CLASS(ObjectiveSprite)
	ADD_STRINGPROP_FLAG(Filename, "", PF_HIDDEN | PF_FILENAME)
    ADD_BOOLPROP_FLAG(StartOn, LTFALSE, 0)
	ADD_LONGINTPROP(ObjectiveNumber, 1)
	ADD_REALPROP(ScaleX, 0.5f)
	ADD_REALPROP(ScaleY, 0.5f)

END_CLASS_DEFAULT(ObjectiveSprite, GameBase, NULL, NULL)
#pragma force_active off


#define OBJSPR_COMP "spr\\spr0039.spr"
#define OBJSPR_1	"spr\\spr0040.spr"
#define OBJSPR_2	"spr\\spr0041.spr"
#define OBJSPR_3	"spr\\spr0042.spr"
#define OBJSPR_4	"spr\\spr0043.spr"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ObjectiveSprite::ObjectiveSprite
//
//	PURPOSE:	Initialize
//
// ----------------------------------------------------------------------- //

ObjectiveSprite::ObjectiveSprite() : GameBase(OT_SPRITE)
{
	VEC_SET(m_vScale, 1.0f, 1.0f, 1.0f);
	m_nObjectiveNum			= 1;
    m_bStartOn              = LTFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ObjectiveSprite::~ObjectiveSprite
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

ObjectiveSprite::~ObjectiveSprite()
{
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ObjectiveSprite::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 ObjectiveSprite::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
    uint32 dwRet;

	switch(messageID)
	{
		case MID_PRECREATE:
		{
			dwRet = GameBase::EngineMessageFn(messageID, pData, fData);
			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				ReadProp((ObjectCreateStruct *)pData);
				PostPropRead((ObjectCreateStruct *)pData);
			}
			return dwRet;
		}
		break;

		case MID_UPDATE:
		{
			Update();
		}
		break;

		case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
				InitialUpdate();
			}
		}
		break;

		case MID_SAVEOBJECT:
		{
            Save((ILTMessage_Write*)pData, (uint32)fData);
		}
		break;

		case MID_LOADOBJECT:
		{
            Load((ILTMessage_Read*)pData, (uint32)fData);
		}
		break;

		default : break;
	}


	return GameBase::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ObjectiveSprite::OnTrigger
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

bool ObjectiveSprite::OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg)
{
	static CParsedMsg::CToken s_cTok_Complete("COMPLETE");
	static CParsedMsg::CToken s_cTok_Reset("RESET");
	static CParsedMsg::CToken s_cTok_On("ON");
	static CParsedMsg::CToken s_cTok_Off("OFF");

	if (cMsg.GetArg(0) == s_cTok_Complete)
	{
		SetCompleted();
	}
	else if (cMsg.GetArg(0) == s_cTok_Reset)
	{
		Reset();
	}
	else if (cMsg.GetArg(0) == s_cTok_On)
	{
		if (m_hObject)
		{
			g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, FLAG_VISIBLE, FLAG_VISIBLE);
			g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, USRFLG_VISIBLE, USRFLG_VISIBLE);
		}
	}
	else if (cMsg.GetArg(0) == s_cTok_Off)
	{
		if (m_hObject)
		{
			g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, 0, FLAG_VISIBLE);
			g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, 0, USRFLG_VISIBLE);
		}
	}
	else
		return GameBase::OnTrigger(hSender, cMsg);

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ObjectiveSprite::ReadProp()
//
//	PURPOSE:	Update Properties
//
// ----------------------------------------------------------------------- //

void ObjectiveSprite::ReadProp(ObjectCreateStruct *pStruct)
{
	GenericProp genProp;

    if (g_pLTServer->GetPropGeneric("StartOn", &genProp) == LT_OK)
	{
		m_bStartOn = genProp.m_Bool;
	}

    if (g_pLTServer->GetPropGeneric("ScaleX", &genProp) == LT_OK)
	{
		m_vScale.x = genProp.m_Float;
	}

    if (g_pLTServer->GetPropGeneric("ScaleY", &genProp) == LT_OK)
	{
		m_vScale.y = genProp.m_Float;
	}


    if (g_pLTServer->GetPropGeneric("ObjectiveNumber", &genProp) == LT_OK)
	{
		long nLongVal = genProp.m_Long;
		if (nLongVal < 1)
			nLongVal = 1;
		else if (nLongVal > 4)
			nLongVal = 4;
		m_nObjectiveNum = (uint8)nLongVal;
	}

	
	m_ObjSpriteStruct.ReadProps();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ObjectiveSprite::PostPropRead()
//
//	PURPOSE:	Finalize some data.
//
// ----------------------------------------------------------------------- //

void ObjectiveSprite::PostPropRead(ObjectCreateStruct *pStruct)
{
	if (pStruct)
	{
		switch (m_nObjectiveNum)
		{
		case 1:
			strcpy(pStruct->m_Filename,OBJSPR_1);
			break;
		case 2:
			strcpy(pStruct->m_Filename,OBJSPR_2);
			break;
		case 3:
			strcpy(pStruct->m_Filename,OBJSPR_3);
			break;
		case 4:
			strcpy(pStruct->m_Filename,OBJSPR_4);
			break;
		default:
			strcpy(pStruct->m_Filename,OBJSPR_COMP);
			break;
		}
		pStruct->m_SkinName[0] = '\0';
		pStruct->m_NextUpdate = 0.1f;
		pStruct->m_Scale = m_vScale;
		
		pStruct->m_Flags |= m_bStartOn ? FLAG_VISIBLE : 0;
		pStruct->m_Flags |= FLAG_ROTATEABLESPRITE;

	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ObjectiveSprite::InitialUpdate
//
//	PURPOSE:	Do initial updating
//
// ----------------------------------------------------------------------- //

LTBOOL ObjectiveSprite::InitialUpdate()
{
    if (!m_hObject) return LTFALSE;

	CreateSFXMsg();
    g_pLTServer->SetObjectColor(m_hObject, 1.0f, 1.0f, 1.0f, 0.999f);

	SetNextUpdate(UPDATE_NEXT_FRAME);

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ObjectiveSprite::Update
//
//	PURPOSE:	Do one update
//
// ----------------------------------------------------------------------- //

LTBOOL ObjectiveSprite::Update()
{
	SetNextUpdate(UPDATE_NEVER);
 
	// BUG - This isn't quite right.  Sometimes this works (flipping the sprite)
	// other times the sprite shouldn't be flipped...Not sure what the bug is.
	// For some reason the sprites are sometimes backwards...Get the rotation
	// so we can flip it...

    LTRotation rRot;
    LTVector vPos, vDir, vU, vR, vF;
	g_pLTServer->GetObjectPos(m_hObject, &vPos);
	g_pLTServer->GetObjectRotation(m_hObject, &rRot);
	vU = rRot.Up();
	vR = rRot.Right();
	vF = rRot.Forward();
/*
	if (m_bFlushWithWorld)
	{
		// Align the sprite to the surface directly behind the sprite
		// (i.e., opposite the forward vector)...

		VEC_NORM(vF);
		VEC_MULSCALAR(vDir, vF, -1.0f);


		// Determine where on the surface to place the sprite...

		IntersectInfo iInfo;
		IntersectQuery qInfo;

		VEC_COPY(qInfo.m_From, vPos);
		VEC_COPY(qInfo.m_Direction, vDir);
		qInfo.m_Flags	 = IGNORE_NONSOLID | INTERSECT_OBJECTS | INTERSECT_HPOLY;
        qInfo.m_FilterFn = LTNULL;

        if (g_pLTServer->CastRay(&qInfo, &iInfo))
		{
            LTVector vTemp;
			VEC_COPY(vPos, iInfo.m_Point);
			VEC_COPY(vDir, iInfo.m_Plane.m_Normal);

			// Place the sprite just above the surface...

			VEC_MULSCALAR(vTemp, vDir, 1.0f);
			VEC_ADD(vPos, vPos, vTemp);

            g_pLTServer->SetObjectPos(m_hObject, &vPos);
		}
	}
*/
    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ObjectiveSprite::SetCompleted
//
//	PURPOSE:	Set to completed sprite
//
// ----------------------------------------------------------------------- //

void ObjectiveSprite::SetCompleted()
{
    SetObjectFilenames(m_hObject, OBJSPR_COMP, "");
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ObjectiveSprite::Reset
//
//	PURPOSE:	Reset to initial sprite
//
// ----------------------------------------------------------------------- //

void ObjectiveSprite::Reset()
{
	switch (m_nObjectiveNum)
	{
	case 1:
		SetObjectFilenames(m_hObject, OBJSPR_1, "");
		break;
	case 2:
		SetObjectFilenames(m_hObject, OBJSPR_2, "");
		break;
	case 3:
		SetObjectFilenames(m_hObject, OBJSPR_3, "");
		break;
	case 4:
		SetObjectFilenames(m_hObject, OBJSPR_4, "");
		break;
	}


}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ObjectiveSprite::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void ObjectiveSprite::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
	if (!pMsg) return;

	SAVE_BYTE(m_nObjectiveNum);
	SAVE_BOOL(m_bStartOn);
	m_ObjSpriteStruct.Save(pMsg);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ObjectiveSprite::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void ObjectiveSprite::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
	if (!pMsg) return;

    LOAD_BYTE(m_nObjectiveNum);
    LOAD_BOOL(m_bStartOn);
	m_ObjSpriteStruct.Load(pMsg);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ObjectiveSprite::CreateSFXMsg
//
//	PURPOSE:	Create our special fx message
//
// ----------------------------------------------------------------------- //

void ObjectiveSprite::CreateSFXMsg()
{
	CAutoMessage cMsg;
	cMsg.Writeuint8(SFX_OBJSPRITE_ID);
    m_ObjSpriteStruct.Write(cMsg);
	g_pLTServer->SetObjectSFXMessage(m_hObject, cMsg.Read());
}

