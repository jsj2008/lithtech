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
#include "SFXFuncs.h"

BEGIN_CLASS(ObjectiveSprite)
	ADD_STRINGPROP_FLAG(Filename, "", PF_HIDDEN | PF_FILENAME)
    ADD_BOOLPROP_FLAG(StartOn, LTFALSE, 0)
	ADD_LONGINTPROP(ObjectiveNumber, 1)
	ADD_LONGINTPROP(PlayerTeamFilter, 0)
	ADD_REALPROP(ScaleX, 0.5f)
	ADD_REALPROP(ScaleY, 0.5f)

END_CLASS_DEFAULT(ObjectiveSprite, GameBase, NULL, NULL)


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

			CacheFiles();
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


	return GameBase::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ObjectiveSprite::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

uint32 ObjectiveSprite::ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead)
{
	switch(messageID)
	{
 		case MID_TRIGGER:
		{
			const char* szMsg = (const char*)g_pLTServer->ReadFromMessageDWord(hRead);

			if (stricmp(szMsg, "COMPLETE") == 0)
			{
				SetCompleted();
			}
			else if (stricmp(szMsg, "RESET") == 0)
			{
				Reset();
			}
			else if (stricmp(szMsg, "ON") == 0)
			{
				if (m_hObject)
				{
                    uint32 dwFlags = g_pLTServer->GetObjectFlags(m_hObject);
                    g_pLTServer->SetObjectFlags(m_hObject, dwFlags | FLAG_VISIBLE);

                    uint32 dwUsrFlags = g_pLTServer->GetObjectUserFlags(m_hObject);
                    g_pLTServer->SetObjectUserFlags(m_hObject, dwUsrFlags | USRFLG_VISIBLE);
				}
			}
			else if (stricmp(szMsg, "OFF") == 0)
			{
				if (m_hObject)
				{
                    uint32 dwFlags = g_pLTServer->GetObjectFlags(m_hObject);
                    g_pLTServer->SetObjectFlags(m_hObject, dwFlags & ~FLAG_VISIBLE);

                    uint32 dwUsrFlags = g_pLTServer->GetObjectUserFlags(m_hObject);
                    g_pLTServer->SetObjectUserFlags(m_hObject, dwUsrFlags & ~USRFLG_VISIBLE);
				}
			}
		}
		default : break;
	}

	return GameBase::ObjectMessageFn (hSender, messageID, hRead);
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

	SetNextUpdate(0.001f);

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
	SetNextUpdate(0.0f);
 
	// BUG - This isn't quite right.  Sometimes this works (flipping the sprite)
	// other times the sprite shouldn't be flipped...Not sure what the bug is.
	// For some reason the sprites are sometimes backwards...Get the rotation
	// so we can flip it...

    LTRotation rRot;
    LTVector vPos, vDir, vU, vR, vF;
    g_pLTServer->GetObjectPos(m_hObject, &vPos);
    g_pLTServer->GetObjectRotation(m_hObject, &rRot);
    g_pLTServer->GetRotationVectors(&rRot, &vU, &vR, &vF);
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
    g_pLTServer->SetObjectFilenames(m_hObject, OBJSPR_COMP, "");
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
		g_pLTServer->SetObjectFilenames(m_hObject, OBJSPR_1, "");
		break;
	case 2:
		g_pLTServer->SetObjectFilenames(m_hObject, OBJSPR_2, "");
		break;
	case 3:
		g_pLTServer->SetObjectFilenames(m_hObject, OBJSPR_3, "");
		break;
	case 4:
		g_pLTServer->SetObjectFilenames(m_hObject, OBJSPR_4, "");
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

void ObjectiveSprite::Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags)
{
	if (!hWrite) return;


    g_pLTServer->WriteToMessageByte(hWrite, m_nObjectiveNum);
    g_pLTServer->WriteToMessageByte(hWrite, m_bStartOn);
	m_ObjSpriteStruct.Save(hWrite);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ObjectiveSprite::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void ObjectiveSprite::Load(HMESSAGEREAD hRead, uint32 dwLoadFlags)
{
	if (!hRead) return;

    m_nObjectiveNum         = g_pLTServer->ReadFromMessageByte(hRead);
    m_bStartOn              = (LTBOOL) g_pLTServer->ReadFromMessageByte(hRead);
	m_ObjSpriteStruct.Load(hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ObjectiveSprite::CacheFiles
//
//	PURPOSE:	Cache resources associated with this object
//
// ----------------------------------------------------------------------- //

void ObjectiveSprite::CacheFiles()
{
	char szFilename[64];
	switch (m_nObjectiveNum)
	{
	case 1:
		strcpy(szFilename,OBJSPR_1);
		break;
	case 2:
		strcpy(szFilename,OBJSPR_2);
		break;
	case 3:
		strcpy(szFilename,OBJSPR_3);
		break;
	case 4:
		strcpy(szFilename,OBJSPR_4);
		break;
	}
	g_pLTServer->CacheFile(FT_SPRITE, szFilename);
	g_pLTServer->CacheFile(FT_SPRITE, OBJSPR_COMP);
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
    HMESSAGEWRITE hMessage = g_pLTServer->StartSpecialEffectMessage(this);
    g_pLTServer->WriteToMessageByte(hMessage, SFX_OBJSPRITE_ID);
    m_ObjSpriteStruct.Write(g_pLTServer, hMessage);
    g_pLTServer->EndMessage(hMessage);
}

