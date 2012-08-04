// ----------------------------------------------------------------------- //
//
// MODULE  : VolumeBrush.cpp
//
// PURPOSE : VolumeBrush implementation
//
// CREATED : 1/29/98
//
// (c) 1998-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "VolumeBrush.h"
#include "iltserver.h"
#include "ServerUtilities.h"
#include "PolyGrid.h"
#include "Character.h"
#include "SFXMsgIds.h"
#include "ObjectMsgs.h"

#define UPDATE_DELTA					0.01f
#define LIQUID_GRAVITY					-200.0f
#define TRIGGER_MSG_ON					"ON"
#define TRIGGER_MSG_OFF					"OFF"

#include "CVarTrack.h"
static CVarTrack vtRemoveFilters;

BEGIN_CLASS(VolumeBrush)
	ADD_VISIBLE_FLAG(0, 0)
    ADD_BOOLPROP(Hidden, LTFALSE)
	ADD_REALPROP(Viscosity, 0.0f)
	ADD_REALPROP(Friction, 1.0f)
	ADD_VECTORPROP_VAL(Current, 0.0f, 0.0f, 0.0f)
	ADD_REALPROP(Damage, 0.0f)
 	ADD_STRINGPROP_FLAG(DamageType, "CHOKE", PF_STATICLIST)
	ADD_COLORPROP_FLAG(TintColor, 255.0f, 255.0f, 255.0f, 0)
    ADD_COLORPROP_FLAG(LightAdd, 0.0f, 0.0f, 0.0f, 0)
 	ADD_STRINGPROP_FLAG(SoundFilter, "Dynamic", PF_STATICLIST)
    ADD_BOOLPROP(CanPlayMovementSounds, LTTRUE)
	PROP_DEFINEGROUP(SurfaceStuff, PF_GROUP1)
        ADD_BOOLPROP_FLAG(ShowSurface, LTTRUE, PF_GROUP1)
		ADD_STRINGPROP_FLAG(SpriteSurfaceName, "", PF_GROUP1|PF_FILENAME)
		ADD_REALPROP_FLAG(XScaleMin, 15.0f, PF_GROUP1)
		ADD_REALPROP_FLAG(XScaleMax, 25.0f, PF_GROUP1)
		ADD_REALPROP_FLAG(YScaleMin, 15.0f, PF_GROUP1)
		ADD_REALPROP_FLAG(YScaleMax, 25.0f, PF_GROUP1)
		ADD_REALPROP_FLAG(XScaleDuration, 60.0f, PF_GROUP1)
		ADD_REALPROP_FLAG(YScaleDuration, 60.0f, PF_GROUP1)
		ADD_REALPROP_FLAG(SurfaceHeight, 5.0f, PF_GROUP1)
		ADD_COLORPROP_FLAG(SurfaceColor1, 255.0f, 255.0f, 255.0f, PF_GROUP1)
		ADD_COLORPROP_FLAG(SurfaceColor2, 255.0f, 255.0f, 255.0f, PF_GROUP1)
		ADD_REALPROP_FLAG(SurfaceAlpha, 0.7f, PF_GROUP1)
        ADD_BOOLPROP_FLAG(Additive, LTFALSE, PF_GROUP1)
        ADD_BOOLPROP_FLAG(Multiply, LTFALSE, PF_GROUP1)
		ADD_LONGINTPROP_FLAG(NumSurfacePolies, 160, PF_GROUP1)
	PROP_DEFINEGROUP(FogStuff, PF_GROUP2)
        ADD_BOOLPROP_FLAG(FogEnable, LTFALSE, PF_GROUP2)
		ADD_REALPROP_FLAG(FogFarZ, 300.0f, PF_GROUP2)
		ADD_REALPROP_FLAG(FogNearZ, -100.0f, PF_GROUP2)
		ADD_COLORPROP_FLAG(FogColor, 0.0f, 0.0f, 0.0f, PF_GROUP2)
END_CLASS_DEFAULT_FLAGS_PLUGIN(VolumeBrush, GameBase, NULL, NULL, 0, CVolumePlugin)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	VolumeBrush::VolumeBrush()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

VolumeBrush::VolumeBrush() : GameBase(OT_CONTAINER)
{
	m_nSfxMsgId			= SFX_VOLUMEBRUSH_ID;
	m_dwSaveFlags		= 0;
	m_eContainerCode	= CC_VOLUME;
	m_fDamage			= 0.0f;
	m_eDamageType		= DT_UNSPECIFIED;
    m_bShowSurface      = LTTRUE;
	m_fSurfaceHeight	= 5.0f;
	m_fViscosity		= 0.0f;
	m_fFriction			= 1.0f;
    m_hSurfaceObj       = LTNULL;
    m_bHidden           = LTFALSE;
	m_fGravity			= LIQUID_GRAVITY;
	m_nSoundFilterId	= 0;
	m_bCanPlayMoveSnds	= LTTRUE;

	VEC_INIT(m_vLastPos);
	VEC_INIT(m_vCurrent);
	VEC_SET(m_vTintColor, 255.0f, 255.0f, 255.0f);
	VEC_INIT(m_vLightAdd);

    m_bFogEnable    = LTFALSE;
	m_fFogFarZ		= 300.0f;
	m_fFogNearZ		= -100.0f;
	VEC_INIT(m_vFogColor);

	// Surface related stuff...

	m_fXScaleMin = 15.0f;
	m_fXScaleMax = 25.0f;
	m_fYScaleMin = 15.0f;
	m_fYScaleMax = 25.0f;
	m_fXScaleDuration = 10.0f;
	m_fYScaleDuration = 10.0f;
    m_hstrSurfaceSprite = LTNULL;
	m_dwNumSurfPolies = 160;
	m_fSurfAlpha = 0.7f;
    m_bAdditive = LTFALSE;
    m_bMultiply = LTFALSE;

	VEC_SET(m_vSurfaceColor1, 255.0f, 255.0f, 255.0f);
	VEC_SET(m_vSurfaceColor2, 255.0f, 255.0f, 255.0f);

	m_dwFlags = FLAG_CONTAINER | FLAG_TOUCH_NOTIFY | FLAG_GOTHRUWORLD | FLAG_FORCECLIENTUPDATE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	VolumeBrush::~VolumeBrush
//
//	PURPOSE:	Deallocate
//
// ----------------------------------------------------------------------- //

VolumeBrush::~VolumeBrush()
{
	if (m_hstrSurfaceSprite)
	{
        g_pLTServer->FreeString(m_hstrSurfaceSprite);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	VolumeBrush::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 VolumeBrush::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
    uint32 dwRet;

	switch(messageID)
	{
		case MID_UPDATE:
		{
			Update();
			break;
		}

		case MID_AFFECTPHYSICS:
		{
			UpdatePhysics((ContainerPhysics*)pData);
			break;
		}

		case MID_PRECREATE:
		{
			dwRet = GameBase::EngineMessageFn(messageID, pData, fData);
			if (fData == PRECREATE_WORLDFILE)
			{
				ReadProp((ObjectCreateStruct*)pData);
			}

			PostPropRead((ObjectCreateStruct*)pData);
			return dwRet;
			break;
		}

		case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
				InitialUpdate();
			}

			CacheFiles();
			break;
		}

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


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	VolumeBrush::ObjectMessageFn()
//
//	PURPOSE:	Handler for server object messages.
//
// --------------------------------------------------------------------------- //

uint32 VolumeBrush::ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead)
{
	switch (messageID)
	{
		case MID_TRIGGER:
		{
            const char* szMsg = (const char*)g_pLTServer->ReadFromMessageDWord(hRead);
			HandleTrigger(hSender, szMsg);
		}
		break;
	}

	return GameBase::ObjectMessageFn(hSender, messageID, hRead);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	VolumeBrush::TriggerMsg()
//
//	PURPOSE:	Handler for volume brush trigger messages.
//
// --------------------------------------------------------------------------- //

void VolumeBrush::HandleTrigger(HOBJECT hSender, const char* szMsg)
{
	if (m_bHidden && (stricmp(szMsg, TRIGGER_MSG_ON) == 0))
	{
        uint32 dwFlags = g_pLTServer->GetObjectUserFlags(m_hObject);
        g_pLTServer->SetObjectUserFlags(m_hObject, dwFlags | USRFLG_VISIBLE);
        g_pLTServer->SetObjectFlags(m_hObject, m_dwSaveFlags);

		if (m_hSurfaceObj)
		{
            dwFlags = g_pLTServer->GetObjectUserFlags(m_hSurfaceObj);
            g_pLTServer->SetObjectUserFlags(m_hSurfaceObj, dwFlags | USRFLG_VISIBLE);
		}

        m_bHidden = LTFALSE;
	}
	else if (!m_bHidden && (stricmp(szMsg, TRIGGER_MSG_OFF) == 0))
	{
        m_dwSaveFlags  = g_pLTServer->GetObjectFlags(m_hObject);
        uint32 dwFlags = g_pLTServer->GetObjectUserFlags(m_hObject);
        g_pLTServer->SetObjectUserFlags(m_hObject, dwFlags & ~USRFLG_VISIBLE);
        g_pLTServer->SetObjectFlags(m_hObject, 0);

		if (m_hSurfaceObj)
		{
            dwFlags = g_pLTServer->GetObjectUserFlags(m_hSurfaceObj);
            g_pLTServer->SetObjectUserFlags(m_hSurfaceObj, dwFlags & ~USRFLG_VISIBLE);
		}

        m_bHidden = LTTRUE;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	VolumeBrush::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

void VolumeBrush::ReadProp(ObjectCreateStruct *pStruct)
{
	if (!pStruct) return;

	char szData[MAX_CS_FILENAME_LEN+1];

    g_pLTServer->GetPropBool("Hidden", &m_bHidden);
    g_pLTServer->GetPropBool("ShowSurface", &m_bShowSurface);

    g_pLTServer->GetPropVector("TintColor", &m_vTintColor);
    g_pLTServer->GetPropVector("LightAdd", &m_vLightAdd);

    g_pLTServer->GetPropBool("FogEnable", &m_bFogEnable);
    g_pLTServer->GetPropReal("FogFarZ", &m_fFogFarZ);
    g_pLTServer->GetPropReal("FogNearZ", &m_fFogNearZ);
    g_pLTServer->GetPropVector("FogColor", &m_vFogColor);

  	GenericProp genProp;
    if (g_pLTServer->GetPropGeneric("DamageType", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
			m_eDamageType = StringToDamageType((const char*)genProp.m_String);
		}
	}

	// Polygrid stuff...

    if (g_pLTServer->GetPropString("SpriteSurfaceName", szData, MAX_CS_FILENAME_LEN) == LT_OK)
	{
        m_hstrSurfaceSprite = g_pLTServer->CreateString( szData );
	}

    g_pLTServer->GetPropReal("XScaleMin", &m_fXScaleMin);
    g_pLTServer->GetPropReal("XScaleMax", &m_fXScaleMax);
    g_pLTServer->GetPropReal("YScaleMin", &m_fYScaleMin);
    g_pLTServer->GetPropReal("YScaleMax", &m_fYScaleMax);
    g_pLTServer->GetPropReal("XScaleDuration", &m_fXScaleDuration);
    g_pLTServer->GetPropReal("YScaleDuration", &m_fYScaleDuration);
    g_pLTServer->GetPropReal("SurfaceHeight", &m_fSurfaceHeight);
    g_pLTServer->GetPropColor("SurfaceColor1", &m_vSurfaceColor1);
    g_pLTServer->GetPropColor("SurfaceColor2", &m_vSurfaceColor2);
    g_pLTServer->GetPropReal("Viscosity", &m_fViscosity);
    g_pLTServer->GetPropReal("Friction", &m_fFriction);
    g_pLTServer->GetPropVector("Current", &m_vCurrent);
    g_pLTServer->GetPropReal("Damage", &m_fDamage);
    g_pLTServer->GetPropReal("SurfaceAlpha", &m_fSurfAlpha);
    g_pLTServer->GetPropBool("Additive", &m_bAdditive);
    g_pLTServer->GetPropBool("Multiply", &m_bMultiply);
    g_pLTServer->GetPropBool("CanPlayMovementSounds", &m_bCanPlayMoveSnds);

	long nLongVal;
    if (g_pLTServer->GetPropLongInt("NumSurfacePolies", &nLongVal) == LT_OK)
	{
        m_dwNumSurfPolies = (uint32)nLongVal;
	}

    if (g_pLTServer->GetPropGeneric("SoundFilter", &genProp) == LT_OK)
	{
		SOUNDFILTER* pFilter = g_pSoundFilterMgr->GetFilter(genProp.m_String);
		if (pFilter)
		{
			m_nSoundFilterId = pFilter->nId;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	VolumeBrush::PostPropRead
//
//	PURPOSE:	Set some final values.
//
// ----------------------------------------------------------------------- //

void VolumeBrush::PostPropRead(ObjectCreateStruct *pStruct)
{
	if (!pStruct) return;

	pStruct->m_Flags |= m_dwFlags;
	pStruct->m_ObjectType = OT_CONTAINER;
	SAFE_STRCPY(pStruct->m_Filename, pStruct->m_Name);
	pStruct->m_SkinName[0] = '\0';
    pStruct->m_ContainerCode = (uint16)m_eContainerCode;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	VolumeBrush::InitialUpdate()
//
//	PURPOSE:	First update
//
// ----------------------------------------------------------------------- //

void VolumeBrush::InitialUpdate()
{
// TESTING!!!!
	if (!vtRemoveFilters.IsInitted())
	{
		vtRemoveFilters.Init(g_pLTServer, "RemoveFilters", LTNULL, 0.0f);
	}
	if (vtRemoveFilters.GetFloat())
	{
		g_pLTServer->CPrint("Removing Filter: %s", g_pLTServer->GetObjectName(m_hObject));
		g_pLTServer->RemoveObject(m_hObject);
		return;
	}
// TESTING!!!!


	// Tell the client about any special fx (fog)...

	CreateSpecialFXMsg();


	// Save volume brush's initial flags...

    m_dwSaveFlags = g_pLTServer->GetObjectFlags(m_hObject);


    uint32 dwUserFlags = g_pLTServer->GetObjectUserFlags(m_hObject);
	dwUserFlags |= USRFLG_IGNORE_PROJECTILES;
	if (!m_bHidden) dwUserFlags |= USRFLG_VISIBLE;

    g_pLTServer->SetObjectUserFlags(m_hObject, dwUserFlags);


	// Create the surface if necessary.  We only need to do updates if we have
	// a surface (in case somebody decides to move the brush, we need to update
	// the surface's position)...

	if (m_bShowSurface)
	{
		CreateSurface();
        SetNextUpdate(UPDATE_DELTA);
	}


	// Normalize friction (1 = normal, 0 = no friction, 2 = double)...

	if (m_fFriction < 0.0) m_fFriction = 0.0f;
	else if (m_fFriction > 1.0) m_fFriction = 1.0f;


	// Normalize viscosity (1 = no movement, 0 = full movement)...

	if (m_fViscosity < 0.0) m_fViscosity = 0.0f;
	else if (m_fViscosity > 1.0) m_fViscosity = 1.0f;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	VolumeBrush::CreateSpecialFXMsg()
//
//	PURPOSE:	Create the special fx message
//
// ----------------------------------------------------------------------- //

void VolumeBrush::CreateSpecialFXMsg()
{
    HMESSAGEWRITE hMessage = g_pLTServer->StartSpecialEffectMessage(this);
	WriteSFXMsg(hMessage);
    g_pLTServer->EndMessage(hMessage);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	VolumeBrush::WriteSFXMsg()
//
//	PURPOSE:	Write the volume brush specific sfx
//
// ----------------------------------------------------------------------- //

void VolumeBrush::WriteSFXMsg(HMESSAGEWRITE hMessage)
{
	if (!hMessage) return;

    g_pLTServer->WriteToMessageByte(hMessage, m_nSfxMsgId);

    g_pLTServer->WriteToMessageByte(hMessage, (uint8)m_bFogEnable);
    g_pLTServer->WriteToMessageFloat(hMessage, m_fFogFarZ);
    g_pLTServer->WriteToMessageFloat(hMessage, m_fFogNearZ);
    g_pLTServer->WriteToMessageVector(hMessage, &m_vFogColor);
    g_pLTServer->WriteToMessageVector(hMessage, &m_vTintColor);
    g_pLTServer->WriteToMessageVector(hMessage, &m_vLightAdd);
    g_pLTServer->WriteToMessageByte(hMessage, m_nSoundFilterId);
    g_pLTServer->WriteToMessageByte(hMessage, m_bCanPlayMoveSnds);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	VolumeBrush::Update()
//
//	PURPOSE:	Update the brush
//
// ----------------------------------------------------------------------- //

void VolumeBrush::Update()
{
	if (m_bHidden) return;

	// Only do updates if we have a surface...

	if (m_hSurfaceObj)
	{
        SetNextUpdate(UPDATE_DELTA);
	}
	else
	{
        SetNextUpdate(0.0f);
	}


	// See if we have moved...

    LTVector vPos;
    g_pLTServer->GetObjectPos(m_hObject, &vPos);

	if (!Equal(m_vLastPos, vPos))
	{
		VEC_COPY(m_vLastPos, vPos);

		// Set the surface to its new position...

        LTVector vDims;
        g_pLTServer->GetObjectDims(m_hObject, &vDims);

		vPos.y += vDims.y - (m_fSurfaceHeight/2.0f);

        g_pLTServer->SetObjectPos(m_hSurfaceObj, &vPos);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	VolumeBrush::CreateSurface()
//
//	PURPOSE:	Create the poly grid surface
//
// ----------------------------------------------------------------------- //

void VolumeBrush::CreateSurface()
{
	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

    LTVector vPos, vDims, vScale;
	VEC_INIT(vScale);

    g_pLTServer->GetObjectDims(m_hObject, &vDims);
    g_pLTServer->GetObjectPos(m_hObject, &vPos);

    LTRotation rRot;
    g_pLTServer->GetObjectRotation(m_hObject, &rRot);

	VEC_COPY(m_vLastPos, vPos);

	vPos.y += vDims.y - (m_fSurfaceHeight/2.0f);
	VEC_COPY(theStruct.m_Pos, vPos);
    theStruct.m_Rotation = rRot;

    HCLASS hClass = g_pLTServer->GetClass("PolyGrid");

    PolyGrid* pSurface = LTNULL;

	if (hClass)
	{
        pSurface = (PolyGrid *)g_pLTServer->CreateObject(hClass, &theStruct);
	}

	if (pSurface)
	{
		m_hSurfaceObj = pSurface->m_hObject;
		vDims.y		  = m_fSurfaceHeight;

        LTFLOAT fXPan = 1.0f + (m_vCurrent.x * 0.01f);
        LTFLOAT fYPan = 1.0f + (m_vCurrent.y * 0.01f);

		if (!m_bHidden)
		{
            g_pLTServer->SetObjectUserFlags(m_hSurfaceObj, USRFLG_VISIBLE);
		}

		pSurface->Setup(&vDims, &m_vSurfaceColor1, &m_vSurfaceColor2,
						m_hstrSurfaceSprite, m_fXScaleMin, m_fXScaleMax,
						m_fYScaleMin, m_fYScaleMax, m_fXScaleDuration,
						m_fYScaleDuration, fXPan, fYPan, m_fSurfAlpha,
						m_dwNumSurfPolies, m_bAdditive, m_bMultiply);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	VolumeBrush::UpdatePhysics()
//
//	PURPOSE:	Update the physics of the passed in object
//
// ----------------------------------------------------------------------- //

void VolumeBrush::UpdatePhysics(ContainerPhysics* pCPStruct)
{
	if (m_bHidden || !pCPStruct || !pCPStruct->m_hObject) return;

    LTFLOAT fUpdateDelta = g_pLTServer->GetFrameTime();


	// Let the character know if they are in liquid...

	if (IsLiquid(m_eContainerCode) && IsCharacter(pCPStruct->m_hObject))
	{
        CCharacter* pCharacter = (CCharacter*)g_pLTServer->HandleToObject(pCPStruct->m_hObject);
		if (pCharacter)
		{
			pCharacter->UpdateInLiquid(this, pCPStruct);
		}
	}


	// Player container physics is done on the client...

	if (!IsPlayer(pCPStruct->m_hObject))
	{
		// Dampen velocity based on the viscosity of the container...

        LTVector vVel, vCurVel;
		vVel = vCurVel = pCPStruct->m_Velocity;

		if (m_fViscosity > 0.0f && VEC_MAG(vCurVel) > 1.0f)
		{
            LTVector vDir;
			VEC_COPY(vDir, vCurVel);
			VEC_NORM(vDir);

            LTFLOAT fAdjust = MAX_CONTAINER_VISCOSITY * m_fViscosity * fUpdateDelta;

			vVel = (vDir * fAdjust);

			if (VEC_MAG(vVel) < VEC_MAG(vCurVel))
			{
				VEC_SUB(vVel, vCurVel, vVel);
			}
			else
			{
				VEC_INIT(vVel);
			}

			vVel += (m_vCurrent * fUpdateDelta);

			pCPStruct->m_Velocity = vVel;
		}


		// Do special liquid handling...

		if (IsLiquid(m_eContainerCode))
		{
			UpdateLiquidPhysics(pCPStruct);
		}
	}


	// Update damage...

	// Make damage relative to update delta...

    LTFLOAT fDamage = 0.0f;
	if (m_fDamage > 0.0f)
	{
		fDamage = m_fDamage * fUpdateDelta;
	}

	// Damage using progressive damage.  This insures that the correct
	// damage effect is shown on the client...

	if (fDamage)
	{
		DamageStruct damage;

		damage.eType	  = m_eDamageType;
		damage.fDamage	  = fDamage;
		damage.hDamager   = m_hObject;

		// Use progressive damage...
		damage.fDuration  = 0.25f;
		damage.hContainer = m_hObject;

		damage.DoDamage(this, pCPStruct->m_hObject);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	VolumeBrush::UpdateLiquidPhysics()
//
//	PURPOSE:	Update liquid specific physics of the passed in object
//				(really, under liquid physics)
//
// ----------------------------------------------------------------------- //

void VolumeBrush::UpdateLiquidPhysics(ContainerPhysics* pCPStruct)
{
	if (!pCPStruct || !pCPStruct->m_hObject) return;

	// Apply liquid gravity to object...

	if (pCPStruct->m_Flags & FLAG_GRAVITY)
	{
		pCPStruct->m_Flags &= ~FLAG_GRAVITY;
		pCPStruct->m_Acceleration.y += m_fGravity;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	VolumeBrush::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void VolumeBrush::Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags)
{
	if (!hWrite) return;

    g_pLTServer->WriteToLoadSaveMessageObject(hWrite, m_hSurfaceObj);
    g_pLTServer->WriteToMessageVector(hWrite, &m_vCurrent);
    g_pLTServer->WriteToMessageVector(hWrite, &m_vSurfaceColor1);
    g_pLTServer->WriteToMessageVector(hWrite, &m_vSurfaceColor2);
    g_pLTServer->WriteToMessageVector(hWrite, &m_vLastPos);
    g_pLTServer->WriteToMessageVector(hWrite, &m_vFogColor);

    g_pLTServer->WriteToMessageHString(hWrite, m_hstrSurfaceSprite);

    g_pLTServer->WriteToMessageFloat(hWrite, m_fViscosity);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fFriction);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fDamage);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fSurfaceHeight);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fGravity);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fXScaleMin);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fXScaleMax);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fYScaleMin);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fYScaleMax);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fXScaleDuration);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fYScaleDuration);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fFogFarZ);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fFogNearZ);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fSurfAlpha);

    g_pLTServer->WriteToMessageDWord(hWrite, m_dwNumSurfPolies);
    g_pLTServer->WriteToMessageDWord(hWrite, m_dwFlags);
    g_pLTServer->WriteToMessageDWord(hWrite, m_dwSaveFlags);

    g_pLTServer->WriteToMessageByte(hWrite, m_eDamageType);
    g_pLTServer->WriteToMessageByte(hWrite, m_eContainerCode);
    g_pLTServer->WriteToMessageByte(hWrite, m_bShowSurface);
    g_pLTServer->WriteToMessageByte(hWrite, m_bFogEnable);
    g_pLTServer->WriteToMessageByte(hWrite, m_bHidden);
    g_pLTServer->WriteToMessageByte(hWrite, m_nSoundFilterId);
    g_pLTServer->WriteToMessageByte(hWrite, m_bCanPlayMoveSnds);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	VolumeBrush::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void VolumeBrush::Load(HMESSAGEREAD hRead, uint32 dwLoadFlags)
{
	if (!hRead) return;

    g_pLTServer->ReadFromLoadSaveMessageObject(hRead, &m_hSurfaceObj);
    g_pLTServer->ReadFromMessageVector(hRead, &m_vCurrent);
    g_pLTServer->ReadFromMessageVector(hRead, &m_vSurfaceColor1);
    g_pLTServer->ReadFromMessageVector(hRead, &m_vSurfaceColor2);
    g_pLTServer->ReadFromMessageVector(hRead, &m_vLastPos);
    g_pLTServer->ReadFromMessageVector(hRead, &m_vFogColor);

    m_hstrSurfaceSprite = g_pLTServer->ReadFromMessageHString(hRead);

    m_fViscosity        = g_pLTServer->ReadFromMessageFloat(hRead);
    m_fFriction         = g_pLTServer->ReadFromMessageFloat(hRead);
    m_fDamage           = g_pLTServer->ReadFromMessageFloat(hRead);
    m_fSurfaceHeight    = g_pLTServer->ReadFromMessageFloat(hRead);
    m_fGravity          = g_pLTServer->ReadFromMessageFloat(hRead);
    m_fXScaleMin        = g_pLTServer->ReadFromMessageFloat(hRead);
    m_fXScaleMax        = g_pLTServer->ReadFromMessageFloat(hRead);
    m_fYScaleMin        = g_pLTServer->ReadFromMessageFloat(hRead);
    m_fYScaleMax        = g_pLTServer->ReadFromMessageFloat(hRead);
    m_fXScaleDuration   = g_pLTServer->ReadFromMessageFloat(hRead);
    m_fYScaleDuration   = g_pLTServer->ReadFromMessageFloat(hRead);
    m_fFogFarZ          = g_pLTServer->ReadFromMessageFloat(hRead);
    m_fFogNearZ         = g_pLTServer->ReadFromMessageFloat(hRead);
    m_fSurfAlpha        = g_pLTServer->ReadFromMessageFloat(hRead);

    m_dwNumSurfPolies   = g_pLTServer->ReadFromMessageDWord(hRead);
    m_dwFlags           = g_pLTServer->ReadFromMessageDWord(hRead);
    m_dwSaveFlags       = g_pLTServer->ReadFromMessageDWord(hRead);

    m_eDamageType       = (DamageType) g_pLTServer->ReadFromMessageByte(hRead);
    m_eContainerCode    = (ContainerCode) g_pLTServer->ReadFromMessageByte(hRead);
    m_bShowSurface      = (LTBOOL) g_pLTServer->ReadFromMessageByte(hRead);
    m_bFogEnable        = (LTBOOL) g_pLTServer->ReadFromMessageByte(hRead);
    m_bHidden           = (LTBOOL) g_pLTServer->ReadFromMessageByte(hRead);
    m_nSoundFilterId    = g_pLTServer->ReadFromMessageByte(hRead);
	m_bCanPlayMoveSnds  = (LTBOOL) g_pLTServer->ReadFromMessageByte(hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	VolumeBrush::CacheFiles
//
//	PURPOSE:	Cache resources used by this the object
//
// ----------------------------------------------------------------------- //

void VolumeBrush::CacheFiles()
{
    char* pFile = LTNULL;
	if (m_hstrSurfaceSprite)
	{
        pFile = g_pLTServer->GetStringData(m_hstrSurfaceSprite);
		if (pFile)
		{
            g_pLTServer->CacheFile(FT_SPRITE, pFile);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVolumePlugin::PreHook_EditStringList
//
//	PURPOSE:	Requests a state change
//
// ----------------------------------------------------------------------- //
LTRESULT CVolumePlugin::PreHook_EditStringList(const char* szRezPath,
											   const char* szPropName,
											   char** aszStrings,
                                               uint32* pcStrings,
                                               const uint32 cMaxStrings,
                                               const uint32 cMaxStringLength)
{
	// See if we can handle the property...

	if (_strcmpi("DamageType", szPropName) == 0)
	{
	   if (!aszStrings || !pcStrings) return LT_UNSUPPORTED;
		_ASSERT(aszStrings && pcStrings);

		// Add an entry for each supported damage type

		for (int i=0; i < c_nDTInfoArraySize; i++)
		{
			if (!DTInfoArray[i].bGadget)
			{
				_ASSERT(cMaxStrings > (*pcStrings) + 1);

				uint32 dwNameLen = strlen(DTInfoArray[i].pName);

				if (dwNameLen < cMaxStringLength &&
					((*pcStrings) + 1) < cMaxStrings)
				{
					strcpy(aszStrings[(*pcStrings)++], DTInfoArray[i].pName);
				}
			}
		}

		return LT_OK;
	}
	else if (_strcmpi("SoundFilter", szPropName) == 0)
	{
		m_SoundFilterMgrPlugin.PreHook_EditStringList(szRezPath, szPropName,
			aszStrings, pcStrings, cMaxStrings, cMaxStringLength);

		if (!m_SoundFilterMgrPlugin.PopulateStringList(aszStrings, pcStrings,
			 cMaxStrings, cMaxStringLength)) return LT_UNSUPPORTED;

		return LT_OK;
	}
	return LT_UNSUPPORTED;
}
