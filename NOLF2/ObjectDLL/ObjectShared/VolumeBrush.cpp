// ----------------------------------------------------------------------- //
//
// MODULE  : VolumeBrush.cpp
//
// PURPOSE : VolumeBrush implementation
//
// CREATED : 1/29/98
//
// (c) 1998-2002 Monolith Productions, Inc.  All Rights Reserved
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
#include "ParsedMsg.h"
#include "SurfaceMgr.h"
#include "MsgIds.h"
#include "SurfaceFunctions.h"

LINKFROM_MODULE( VolumeBrush );

#define UPDATE_DELTA					0.01f
#define LIQUID_GRAVITY					-200.0f
#define TRIGGER_MSG_ON					"ON"
#define TRIGGER_MSG_OFF					"OFF"

#include "CVarTrack.h"
static CVarTrack vtRemoveFilters;

#pragma force_active on
BEGIN_CLASS(VolumeBrush)
	ADD_VISIBLE_FLAG(0, 0)
    ADD_BOOLPROP(Hidden, false)
	ADD_REALPROP(Viscosity, 0.0f)
	ADD_REALPROP(Friction, 1.0f)
	ADD_VECTORPROP_VAL(Current, 0.0f, 0.0f, 0.0f)
	ADD_REALPROP(Damage, 0.0f)
 	ADD_STRINGPROP_FLAG(DamageType, "CHOKE", PF_STATICLIST)
	ADD_COLORPROP_FLAG(TintColor, 255.0f, 255.0f, 255.0f, 0)
    ADD_COLORPROP_FLAG(LightAdd, 0.0f, 0.0f, 0.0f, 0)
 	ADD_STRINGPROP_FLAG(SoundFilter, "Dynamic", PF_STATICLIST)
    ADD_BOOLPROP(CanPlayMovementSounds, true)

	PROP_DEFINEGROUP(FogStuff, PF_GROUP(2))
        ADD_BOOLPROP_FLAG(FogEnable, false, PF_GROUP(2))
		ADD_REALPROP_FLAG(FogFarZ, 300.0f, PF_GROUP(2))
		ADD_REALPROP_FLAG(FogNearZ, -100.0f, PF_GROUP(2))
		ADD_COLORPROP_FLAG(FogColor, 0.0f, 0.0f, 0.0f, PF_GROUP(2))

	ADD_STRINGPROP_FLAG(SurfaceOverride, "", PF_STATICLIST)
	ADD_STRINGPROP_FLAG(PhysicsModel, "", PF_STATICLIST | PF_HIDDEN)
    ADD_BOOLPROP_FLAG(RayHit, false, 0)

END_CLASS_DEFAULT_FLAGS_PLUGIN(VolumeBrush, GameBase, NULL, NULL, CF_WORLDMODEL, CVolumePlugin)
#pragma force_active off

//
// Register the class with the command mgr plugin and add any messages to the class
//

CMDMGR_BEGIN_REGISTER_CLASS( VolumeBrush )

	CMDMGR_ADD_MSG( ON, 1, NULL, "ON" )
	CMDMGR_ADD_MSG( OFF, 1, NULL, "OFF" )

CMDMGR_END_REGISTER_CLASS( VolumeBrush, GameBase )

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ReadHStringProp
//
//	PURPOSE:	Utility function to read in a string property and create
//				an HString from it. The callee is responsible for freeing
//				the returned string
//
// ----------------------------------------------------------------------- //
HSTRING ReadHStringProp(const char* pszPropName, ILTServer* pServer);

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
	m_fViscosity		= 0.0f;
	m_fFriction			= 1.0f;
    m_bHidden           = false;
	m_fGravity			= LIQUID_GRAVITY;
	m_nSoundFilterId	= 0;
	m_bCanPlayMoveSnds	= true;

	m_vCurrent.Init();
	m_vTintColor.Init(255.0f, 255.0f, 255.0f);
	m_vLightAdd.Init();

    m_bFogEnable    = false;
	m_fFogFarZ		= 300.0f;
	m_fFogNearZ		= -100.0f;
	m_vFogColor.Init();

	m_dwFlags = FLAG_CONTAINER | FLAG_TOUCH_NOTIFY | FLAG_GOTHRUWORLD | FLAG_FORCECLIENTUPDATE;

	m_eSurfaceOverrideType  = ST_UNKNOWN;
	m_ePPhysicsModel		= PPM_NORMAL;
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
		case MID_AFFECTPHYSICS:
		{
			UpdatePhysics((ContainerPhysics*)pData);
		}
		break;

		case MID_PRECREATE:
		{
			dwRet = GameBase::EngineMessageFn(messageID, pData, fData);
			if (fData == PRECREATE_WORLDFILE)
			{
				ObjectCreateStruct	*pOCS = (ObjectCreateStruct*)pData;

				// tagRP: HACK - Save off the rotation then get rid of it to deal with the double rotation problem
					pOCS->m_Rotation.Identity();
				// end HACK

				ReadProp((ObjectCreateStruct*)pData);
			}

			PostPropRead((ObjectCreateStruct*)pData);
			return dwRet;
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


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	VolumeBrush::OnTrigger()
//
//	PURPOSE:	Handler for volume brush trigger messages.
//
// --------------------------------------------------------------------------- //

bool VolumeBrush::OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg)
{
	static CParsedMsg::CToken s_cTok_On(TRIGGER_MSG_ON);
	static CParsedMsg::CToken s_cTok_Off(TRIGGER_MSG_OFF);

	if (cMsg.GetArg(0) == s_cTok_On)
	{
		Show();
	}
	else if (cMsg.GetArg(0) == s_cTok_Off)
	{
		Hide();
	}
	else
	{
		return GameBase::OnTrigger(hSender, cMsg);
	}

	return true;
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

    g_pLTServer->GetPropBool("Hidden", &m_bHidden);
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

    g_pLTServer->GetPropReal("Viscosity", &m_fViscosity);
    g_pLTServer->GetPropReal("Friction", &m_fFriction);
    g_pLTServer->GetPropVector("Current", &m_vCurrent);
    g_pLTServer->GetPropReal("Damage", &m_fDamage);
    g_pLTServer->GetPropBool("CanPlayMovementSounds", &m_bCanPlayMoveSnds);

	bool bRayHit = false;
    g_pLTServer->GetPropBool("RayHit", &bRayHit);
	if (bRayHit)
	{
		m_dwFlags |= FLAG_RAYHIT;
	}

    if (g_pLTServer->GetPropGeneric("SoundFilter", &genProp) == LT_OK)
	{
		SOUNDFILTER* pFilter = g_pSoundFilterMgr->GetFilter(genProp.m_String);
		if (pFilter)
		{
			m_nSoundFilterId = pFilter->nId;
		}
	}

	if( g_pLTServer->GetPropGeneric( "SurfaceOverride", &genProp ) == LT_OK)
	{
		if( genProp.m_String[0] && g_pSurfaceMgr )
		{
			SURFACE *pSurface = g_pSurfaceMgr->GetSurface( genProp.m_String );
			if( pSurface )
			{
				m_eSurfaceOverrideType = pSurface->eType;
			}
		}
	}

	if( g_pLTServer->GetPropGeneric("PhysicsModel", &genProp ) == LT_OK )
	{
		m_ePPhysicsModel = GetPlayerPhysicsModelFromPropertyName(genProp.m_String);
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
	// Tell the client about any special fx (fog)...

	CreateSpecialFXMsg();

	// Save volume brush's initial flags...

	g_pCommonLT->GetObjectFlags(m_hObject, OFT_Flags, m_dwSaveFlags);


	uint32 dwUsrFlags = (((m_bHidden) ? 0 : USRFLG_VISIBLE) | USRFLG_IGNORE_PROJECTILES | SurfaceToUserFlag(m_eSurfaceOverrideType));
    g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, dwUsrFlags, 0xFF000000 | USRFLG_VISIBLE | USRFLG_IGNORE_PROJECTILES);

	// Normalize friction (1 = normal, 0 = no friction, 2 = double)...

	if (m_fFriction < 0.0) m_fFriction = 0.0f;
	else if (m_fFriction > 1.0) m_fFriction = 1.0f;


	// Normalize viscosity (1 = no movement, 0 = full movement)...

	if (m_fViscosity < 0.0) m_fViscosity = 0.0f;
	else if (m_fViscosity > 1.0) m_fViscosity = 1.0f;

	// Don't eat ticks please...

	SetNextUpdate(UPDATE_NEVER);
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
	CAutoMessage cMsg;
	WriteSFXMsg(cMsg);
	g_pLTServer->SetObjectSFXMessage(m_hObject, cMsg.Read());
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	VolumeBrush::WriteSFXMsg()
//
//	PURPOSE:	Write the volume brush specific sfx
//
// ----------------------------------------------------------------------- //

void VolumeBrush::WriteSFXMsg(ILTMessage_Write *pMsg)
{
	if (!pMsg) return;

    pMsg->Writeuint8(m_nSfxMsgId);

    pMsg->Writebool(m_bFogEnable);
    pMsg->Writefloat(m_fFogFarZ);
    pMsg->Writefloat(m_fFogNearZ);
    pMsg->WriteLTVector(m_vFogColor);
    pMsg->WriteLTVector(m_vTintColor);
    pMsg->WriteLTVector(m_vLightAdd);
    pMsg->Writeuint8(m_nSoundFilterId);
    pMsg->Writebool(m_bCanPlayMoveSnds);
	pMsg->Writeuint8(m_eSurfaceOverrideType);

	pMsg->WriteLTVector(GetCurrent());
	pMsg->Writefloat(GetGravity());
	pMsg->Writefloat(GetViscosity());
	pMsg->Writefloat(GetFriction());
	pMsg->Writeuint8(GetPhysicsModel());
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

		damage.DoDamage(this, pCPStruct->m_hObject, m_hObject);
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

void VolumeBrush::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
	if (!pMsg) return;

	SAVE_VECTOR(m_vCurrent);
    SAVE_VECTOR(m_vFogColor);
    SAVE_FLOAT(m_fViscosity);
    SAVE_FLOAT(m_fFriction);
    SAVE_FLOAT(m_fDamage);
    SAVE_FLOAT(m_fGravity);
    SAVE_DWORD(m_dwSaveFlags);
    SAVE_BYTE(m_eDamageType);
    SAVE_BYTE(m_eContainerCode);
    SAVE_BOOL(m_bHidden);
	SAVE_BYTE(m_ePPhysicsModel);

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	VolumeBrush::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void VolumeBrush::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
	if (!pMsg) return;

    LOAD_VECTOR(m_vCurrent);
    LOAD_VECTOR(m_vFogColor);
    LOAD_FLOAT(m_fViscosity);
    LOAD_FLOAT(m_fFriction);
    LOAD_FLOAT(m_fDamage);
    LOAD_FLOAT(m_fGravity);
    LOAD_DWORD(m_dwSaveFlags);
    LOAD_BYTE_CAST(m_eDamageType, DamageType);
    LOAD_BYTE_CAST(m_eContainerCode, ContainerCode);
    LOAD_BOOL(m_bHidden);
	LOAD_BYTE_CAST(m_ePPhysicsModel, PlayerPhysicsModel);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	VolumeBrush::Show()
//
//	PURPOSE:	Show the volume brush
//
// --------------------------------------------------------------------------- //

void VolumeBrush::Show()
{
	if (!m_bHidden)
	{
		return;
	}

    g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, USRFLG_VISIBLE, USRFLG_VISIBLE);
	g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, m_dwSaveFlags, FLAGMASK_ALL);

    m_bHidden = false;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	VolumeBrush::Hide()
//
//	PURPOSE:	Hide the volume brush
//
// --------------------------------------------------------------------------- //

void VolumeBrush::Hide()
{
	if (m_bHidden)
	{
		return;
	}

    g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, 0, USRFLG_VISIBLE);
	g_pCommonLT->GetObjectFlags(m_hObject, OFT_Flags, m_dwSaveFlags);
	g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, 0, FLAGMASK_ALL);

    m_bHidden = true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVolumePlugin::PreHook_EditStringList
//
//	PURPOSE:	Requests a state change
//
// ----------------------------------------------------------------------- //

CVolumePlugin::CVolumePlugin()
{
	m_pSurfaceMgrPlugin = debug_new( CSurfaceMgrPlugin );
	m_pSoundFilterMgrPlugin = debug_new( CSoundFilterMgrPlugin );
}

CVolumePlugin::~CVolumePlugin()
{
	debug_delete( m_pSurfaceMgrPlugin );
	debug_delete( m_pSoundFilterMgrPlugin );
}


LTRESULT CVolumePlugin::PreHook_EditStringList(const char* szRezPath,
											   const char* szPropName,
											   char** aszStrings,
                                               uint32* pcStrings,
                                               const uint32 cMaxStrings,
                                               const uint32 cMaxStringLength)
{
	ASSERT( m_pSurfaceMgrPlugin != NULL );
	ASSERT( m_pSoundFilterMgrPlugin != NULL );
	
	// See if we can handle the property...

	if (_strcmpi("DamageType", szPropName) == 0)
	{
	   if (!aszStrings || !pcStrings) return LT_UNSUPPORTED;
		_ASSERT(aszStrings && pcStrings);

		// Add an entry for each supported damage type

		for (int i=0; i < kNumDamageTypes; i++)
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
		m_pSoundFilterMgrPlugin->PreHook_EditStringList(szRezPath, szPropName,
			aszStrings, pcStrings, cMaxStrings, cMaxStringLength);

		if (!m_pSoundFilterMgrPlugin->PopulateStringList(aszStrings, pcStrings,
			 cMaxStrings, cMaxStringLength)) return LT_UNSUPPORTED;

		return LT_OK;
	}
	else if( _stricmp( "SurfaceOverride", szPropName ) == 0)
	{
		m_pSurfaceMgrPlugin->PreHook_EditStringList(szRezPath, szPropName,
			aszStrings, pcStrings, cMaxStrings, cMaxStringLength);

		if (m_pSurfaceMgrPlugin->PopulateStringList(aszStrings, pcStrings,
			 cMaxStrings, cMaxStringLength))
		{
			return LT_OK;
		}
	}
	else if( _stricmp("PhysicsModel", szPropName ) == 0 )
	{
		for( int i = PPM_FIRST; i < PPM_NUM_MODELS; ++i )
		{
			_ASSERT(cMaxStrings > (*pcStrings) + 1);
			if( cMaxStrings > (*pcStrings) + 1 )
			{
				strcpy( aszStrings[(*pcStrings)++], GetPropertyNameFromPlayerPhysicsModel( (PlayerPhysicsModel)i ));
			}
		}

		return LT_OK;
	}

	return LT_UNSUPPORTED;
}
