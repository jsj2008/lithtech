// ----------------------------------------------------------------------- //
//
// MODULE  : ScaleSprite.cpp
//
// PURPOSE : ScaleSprite class - implementation
//
// CREATED : 12/07/97
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ScaleSprite.h"
#include "iltserver.h"
#include "ObjectMsgs.h"
#include "SFXFuncs.h"

BEGIN_CLASS(ScaleSprite)
	ADD_DESTRUCTIBLE_AGGREGATE(PF_GROUP1, 0)
    ADD_BOOLPROP_FLAG(StartOn, LTTRUE, 0)
	ADD_STRINGPROP_FLAG(Filename, "", PF_FILENAME)
	ADD_STRINGPROP_FLAG(DamagedFilename, "", PF_FILENAME)
	ADD_STRINGPROP_FLAG(DestroyedFilename, "", PF_FILENAME)
	ADD_VECTORPROP_VAL_FLAG(Dims, 20.0f, 20.0f, 1.0f, PF_DIMS | PF_LOCALDIMS)
	ADD_COLORPROP(Color, 255.0f, 255.0f, 255.0f)
	ADD_REALPROP(Alpha, 1.0f)
	ADD_REALPROP(ScaleX, 0.5f)
	ADD_REALPROP(ScaleY, 0.5f)
	PROP_DEFINEGROUP(AdditionalFlags, PF_GROUP2)
        ADD_BOOLPROP_FLAG(FlushWithWorld, LTFALSE, PF_GROUP2)
        ADD_BOOLPROP_FLAG(Rotatable, LTFALSE, PF_GROUP2)
        ADD_BOOLPROP_FLAG(Glow, LTTRUE, PF_GROUP2)
        ADD_BOOLPROP_FLAG(ZBias, LTTRUE, PF_GROUP2)
        ADD_BOOLPROP_FLAG(Additive, LTTRUE, PF_GROUP2)
        ADD_BOOLPROP_FLAG(Multiply, LTFALSE, PF_GROUP2)
	ADD_LENSFLARE_PROPERTIES(PF_GROUP3)
END_CLASS_DEFAULT(ScaleSprite, GameBase, NULL, NULL)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScaleSprite::ScaleSprite
//
//	PURPOSE:	Initialize
//
// ----------------------------------------------------------------------- //

ScaleSprite::ScaleSprite() : GameBase(OT_SPRITE)
{
	VEC_SET(m_vScale, 1.0f, 1.0f, 1.0f);
	VEC_SET(m_vColor, 1.0f, 1.0f, 1.0f);
	m_fAlpha				= 1.0f;
    m_bFlushWithWorld       = LTFALSE;
    m_bRotatable            = LTFALSE;
    m_bStartOn              = LTTRUE;
	m_dwAdditionalFlags		= 0;
    m_hstrDamagedFile       = LTNULL;
    m_hstrDestroyedFile     = LTNULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScaleSprite::~ScaleSprite
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

ScaleSprite::~ScaleSprite()
{
	if (m_hstrDestroyedFile)
	{
        g_pLTServer->FreeString(m_hstrDestroyedFile);
	}

	if (m_hstrDamagedFile)
	{
        g_pLTServer->FreeString(m_hstrDamagedFile);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScaleSprite::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 ScaleSprite::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
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
//	ROUTINE:	ScaleSprite::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

uint32 ScaleSprite::ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead)
{
	switch(messageID)
	{
 		case MID_TRIGGER:
		{
			const char* szMsg = (const char*)g_pLTServer->ReadFromMessageDWord(hRead);

			if (stricmp(szMsg, "DESTROY") == 0)
			{
				SetDestroyed();
			}
			else if (stricmp(szMsg, "DAMAGE") == 0)
			{
				SetDamaged();
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
//	ROUTINE:	ScaleSprite::ReadProp()
//
//	PURPOSE:	Update Properties
//
// ----------------------------------------------------------------------- //

void ScaleSprite::ReadProp(ObjectCreateStruct *pStruct)
{
	GenericProp genProp;

	::GetLensFlareProperties(m_LensInfo);

    if (g_pLTServer->GetPropGeneric("DamagedFilename", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
             m_hstrDamagedFile = g_pLTServer->CreateString(genProp.m_String);
		}
	}

    if (g_pLTServer->GetPropGeneric("DestroyedFilename", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
             m_hstrDestroyedFile = g_pLTServer->CreateString(genProp.m_String);
		}
	}

    if (g_pLTServer->GetPropGeneric("ScaleX", &genProp) == LT_OK)
	{
		m_vScale.x = genProp.m_Float;
	}

    if (g_pLTServer->GetPropGeneric("ScaleY", &genProp) == LT_OK)
	{
		m_vScale.y = genProp.m_Float;
	}

    if (g_pLTServer->GetPropGeneric("Dims", &genProp) == LT_OK)
	{
		// Not used - This property is only to allow level designers
		// to see where the sprite is...
	}

    if (g_pLTServer->GetPropGeneric("Alpha", &genProp) == LT_OK)
	{
		m_fAlpha = genProp.m_Float;
	}

    if (g_pLTServer->GetPropGeneric("Color", &genProp) == LT_OK)
	{
		VEC_COPY(m_vColor, genProp.m_Vec);
		VEC_MULSCALAR(m_vColor, m_vColor, 1.0f/255.0f);
	}

    if (g_pLTServer->GetPropGeneric("Rotatable", &genProp) == LT_OK)
	{
		m_bRotatable = genProp.m_Bool;
	}

    if (g_pLTServer->GetPropGeneric("StartOn", &genProp) == LT_OK)
	{
		m_bStartOn = genProp.m_Bool;
	}

    if (g_pLTServer->GetPropGeneric("FlushWithWorld", &genProp) == LT_OK)
	{
		m_bFlushWithWorld = genProp.m_Bool;
	}

	m_dwAdditionalFlags = 0;
    if (g_pLTServer->GetPropGeneric("Glow", &genProp) == LT_OK)
	{
		m_dwAdditionalFlags |= (genProp.m_Bool ? FLAG_GLOWSPRITE : 0);
	}

    if (g_pLTServer->GetPropGeneric("ZBias", &genProp) == LT_OK)
	{
		m_dwAdditionalFlags |= (genProp.m_Bool ? FLAG_SPRITEBIAS : 0);
	}

    if (g_pLTServer->GetPropGeneric("Additive", &genProp) == LT_OK)
	{
		if (genProp.m_Bool)
		{
			pStruct->m_Flags2 |= FLAG2_ADDITIVE;
			pStruct->m_Flags  |= FLAG_FOGDISABLE;
		}
	}

    if (g_pLTServer->GetPropGeneric("Multiply", &genProp) == LT_OK)
	{
		if (genProp.m_Bool)
		{
			pStruct->m_Flags2 |= FLAG2_MULTIPLY;
			pStruct->m_Flags  |= FLAG_FOGDISABLE;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScaleSprite::PostPropRead()
//
//	PURPOSE:	Finalize some data.
//
// ----------------------------------------------------------------------- //

void ScaleSprite::PostPropRead(ObjectCreateStruct *pStruct)
{
	if (pStruct)
	{
		pStruct->m_SkinName[0] = '\0';
		pStruct->m_NextUpdate = 0.1f;
		pStruct->m_Scale = m_vScale;

		pStruct->m_Flags |= 0;

		// Make sure the sprite is rotateable...

		if (m_bRotatable || m_bFlushWithWorld)
		{
			pStruct->m_Flags |= FLAG_ROTATEABLESPRITE;
		}

		pStruct->m_Flags |= m_dwAdditionalFlags;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScaleSprite::InitialUpdate
//
//	PURPOSE:	Do initial updating
//
// ----------------------------------------------------------------------- //

LTBOOL ScaleSprite::InitialUpdate()
{
    if (!m_hObject) return LTFALSE;

	// Do everything in Update (we need to make sure all the objects in
	// the world have been loaded)...

	SetNextUpdate(GetRandom(0.001f, 0.3f));

    g_pLTServer->SetObjectColor(m_hObject, m_vColor.x, m_vColor.y, m_vColor.z, m_fAlpha);

	if (m_LensInfo.bCreateSprite)
	{
		::BuildLensFlareSFXMessage(m_LensInfo, this);
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScaleSprite::Update
//
//	PURPOSE:	Do one update
//
// ----------------------------------------------------------------------- //

LTBOOL ScaleSprite::Update()
{
	if (m_bStartOn)
	{
        uint32 dwFlags = g_pLTServer->GetObjectUserFlags(m_hObject);
        g_pLTServer->SetObjectUserFlags(m_hObject, dwFlags | USRFLG_VISIBLE);

        dwFlags = g_pLTServer->GetObjectFlags(m_hObject);
        g_pLTServer->SetObjectFlags(m_hObject, dwFlags | FLAG_VISIBLE);
	}

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

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScaleSprite::SetDestroyed
//
//	PURPOSE:	Set to destroyed sprite
//
// ----------------------------------------------------------------------- //

void ScaleSprite::SetDestroyed()
{
	if (m_hstrDestroyedFile)
	{
        g_pLTServer->SetObjectFilenames(m_hObject, g_pLTServer->GetStringData(m_hstrDestroyedFile), "");
	}
	else
	{
        g_pLTServer->RemoveObject(m_hObject);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScaleSprite::SetDamaged
//
//	PURPOSE:	Set to damaged sprite
//
// ----------------------------------------------------------------------- //

void ScaleSprite::SetDamaged()
{
	if (!m_hstrDamagedFile) return;

    g_pLTServer->SetObjectFilenames(m_hObject, g_pLTServer->GetStringData(m_hstrDamagedFile), "");
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScaleSprite::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void ScaleSprite::Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags)
{
	if (!hWrite) return;

    g_pLTServer->WriteToMessageVector(hWrite, &m_vScale);
    g_pLTServer->WriteToMessageVector(hWrite, &m_vColor);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fAlpha);
    g_pLTServer->WriteToMessageByte(hWrite, m_bFlushWithWorld);
    g_pLTServer->WriteToMessageByte(hWrite, m_bRotatable);
    g_pLTServer->WriteToMessageByte(hWrite, m_bStartOn);
    g_pLTServer->WriteToMessageDWord(hWrite, m_dwAdditionalFlags);
    g_pLTServer->WriteToMessageHString(hWrite, m_hstrDamagedFile);
    g_pLTServer->WriteToMessageHString(hWrite, m_hstrDestroyedFile);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScaleSprite::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void ScaleSprite::Load(HMESSAGEREAD hRead, uint32 dwLoadFlags)
{
	if (!hRead) return;

    g_pLTServer->ReadFromMessageVector(hRead, &m_vScale);
    g_pLTServer->ReadFromMessageVector(hRead, &m_vColor);
    m_fAlpha                = g_pLTServer->ReadFromMessageFloat(hRead);
    m_bFlushWithWorld       = (LTBOOL) g_pLTServer->ReadFromMessageByte(hRead);
    m_bRotatable            = (LTBOOL) g_pLTServer->ReadFromMessageByte(hRead);
    m_bStartOn              = (LTBOOL) g_pLTServer->ReadFromMessageByte(hRead);
    m_dwAdditionalFlags     = g_pLTServer->ReadFromMessageDWord(hRead);
    m_hstrDamagedFile       = g_pLTServer->ReadFromMessageHString(hRead);
    m_hstrDestroyedFile     = g_pLTServer->ReadFromMessageHString(hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScaleSprite::CacheFiles
//
//	PURPOSE:	Cache resources associated with this object
//
// ----------------------------------------------------------------------- //

void ScaleSprite::CacheFiles()
{
    char* pFile = LTNULL;
	if (m_hstrDamagedFile)
	{
        pFile = g_pLTServer->GetStringData(m_hstrDamagedFile);
		if (pFile)
		{
            g_pLTServer->CacheFile(FT_SPRITE, pFile);
		}
	}

	if (m_hstrDestroyedFile)
	{
        pFile = g_pLTServer->GetStringData(m_hstrDestroyedFile);
		if (pFile)
		{
            g_pLTServer->CacheFile(FT_SPRITE, pFile);
		}
	}
}