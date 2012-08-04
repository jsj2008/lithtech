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
#include "ParsedMsg.h"
#include "SFXFuncs.h"

LINKFROM_MODULE( ScaleSprite );

#pragma force_active on
BEGIN_CLASS(ScaleSprite)
	ADD_DESTRUCTIBLE_AGGREGATE(PF_GROUP(1), 0)
    ADD_BOOLPROP_FLAG(StartOn, LTTRUE, 0)
	ADD_STRINGPROP_FLAG(Filename, "", PF_FILENAME)
	ADD_STRINGPROP_FLAG(DamagedFilename, "", PF_FILENAME)
	ADD_STRINGPROP_FLAG(DestroyedFilename, "", PF_FILENAME)
	ADD_VECTORPROP_VAL_FLAG(Dims, 20.0f, 20.0f, 1.0f, PF_DIMS | PF_LOCALDIMS)
	ADD_COLORPROP(Color, 255.0f, 255.0f, 255.0f)
	ADD_REALPROP(Alpha, 1.0f)
	ADD_REALPROP(ScaleX, 0.5f)
	ADD_REALPROP(ScaleY, 0.5f)
	PROP_DEFINEGROUP(AdditionalFlags, PF_GROUP(2))
        ADD_BOOLPROP_FLAG(FlushWithWorld, LTFALSE, PF_GROUP(2))
        ADD_BOOLPROP_FLAG(Rotatable, LTFALSE, PF_GROUP(2))
        ADD_BOOLPROP_FLAG(Glow, LTTRUE, PF_GROUP(2))
        ADD_BOOLPROP_FLAG(ZBias, LTTRUE, PF_GROUP(2))
        ADD_BOOLPROP_FLAG(Additive, LTTRUE, PF_GROUP(2))
        ADD_BOOLPROP_FLAG(Multiply, LTFALSE, PF_GROUP(2))
	ADD_LENSFLARE_PROPERTIES(PF_GROUP(3))
END_CLASS_DEFAULT_FLAGS_PLUGIN(ScaleSprite, GameBase, NULL, NULL, 0, CScaleSpritePlugin)
#pragma force_active off

CMDMGR_BEGIN_REGISTER_CLASS( ScaleSprite )

	CMDMGR_ADD_MSG( DESTROY, 1, NULL, "DESTROY" )
	CMDMGR_ADD_MSG( DAMAGE, 1, NULL, "DAMAGE" )
	CMDMGR_ADD_MSG( ON, 1, NULL, "ON" )
	CMDMGR_ADD_MSG( OFF, 1, NULL, "OFF" )

CMDMGR_END_REGISTER_CLASS( ScaleSprite, GameBase )


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScaleSpritePlugin::PreHook_PropChanged
//
//	PURPOSE:	Test any commands we might have
//
// ----------------------------------------------------------------------- //

LTRESULT CScaleSpritePlugin::PreHook_PropChanged( const char *szObjName, 
												  const char *szPropName,
												  const int nPropType,
												  const GenericProp &gpPropValue,
												  ILTPreInterface *pInterface,
												  const char *szModifiers )
{
	// Since we don't have any props that need notification, just pass it to the Destructible plugin...

	if( LT_OK == m_DestructiblePlugin.PreHook_PropChanged( szObjName, szPropName, nPropType, gpPropValue, pInterface, szModifiers ))
	{
		return LT_OK;
	}

	return LT_UNSUPPORTED;
}


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
//	ROUTINE:	ScaleSprite::OnTrigger
//
//	PURPOSE:	Handle trigger messages
//
// ----------------------------------------------------------------------- //

bool ScaleSprite::OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg)
{
	static CParsedMsg::CToken s_cTok_Destroy("DESTROY");
	static CParsedMsg::CToken s_cTok_Damage("DAMAGE");
	static CParsedMsg::CToken s_cTok_On("ON");
	static CParsedMsg::CToken s_cTok_Off("OFF");

	if (cMsg.GetArg(0) == s_cTok_Destroy)
	{
		SetDestroyed();
	}
	else if (cMsg.GetArg(0) == s_cTok_Damage)
	{
		SetDamaged();
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

		pStruct->m_Flags |= m_dwAdditionalFlags | FLAG_FORCECLIENTUPDATE;
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
		g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, USRFLG_VISIBLE, USRFLG_VISIBLE);

		g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, FLAG_VISIBLE, FLAG_VISIBLE);
	}

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
        SetObjectFilenames(m_hObject, g_pLTServer->GetStringData(m_hstrDestroyedFile), "");
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

    SetObjectFilenames(m_hObject, g_pLTServer->GetStringData(m_hstrDamagedFile), "");
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScaleSprite::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void ScaleSprite::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
	if (!pMsg) return;

	SAVE_VECTOR(m_vScale);
    SAVE_VECTOR(m_vColor);
    SAVE_FLOAT(m_fAlpha);
    SAVE_BOOL(m_bFlushWithWorld);
    SAVE_BOOL(m_bRotatable);
    SAVE_BOOL(m_bStartOn);
    SAVE_DWORD(m_dwAdditionalFlags);
    SAVE_HSTRING(m_hstrDamagedFile);
    SAVE_HSTRING(m_hstrDestroyedFile);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScaleSprite::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void ScaleSprite::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
	if (!pMsg) return;

	LOAD_VECTOR(m_vScale);
    LOAD_VECTOR(m_vColor);
    LOAD_FLOAT(m_fAlpha);
    LOAD_BOOL(m_bFlushWithWorld);
    LOAD_BOOL(m_bRotatable);
    LOAD_BOOL(m_bStartOn);
    LOAD_DWORD(m_dwAdditionalFlags);
    LOAD_HSTRING(m_hstrDamagedFile);
    LOAD_HSTRING(m_hstrDestroyedFile);
}



