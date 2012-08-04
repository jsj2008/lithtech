// ----------------------------------------------------------------------- //
//
// MODULE  : PropType.cpp
//
// PURPOSE : Model PropType - Definition
//
// CREATED : 4/26/2000
//
// (c) 2000-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "PropType.h"
#include "VersionMgr.h"

LINKFROM_MODULE( PropType );


BEGIN_CLASS(PropType)
	ADD_DESTRUCTIBLE_MODEL_AGGREGATE(PF_GROUP(1), PF_HIDDEN)
	PROP_DEFINEGROUP(DamageProperties, PF_GROUP(1))
		ADD_DESTRUCTIBLE_AGGREGATE_COMMANDS(PF_GROUP(1), 0)

	// Hide all of our parent properties (these are all set via the
	// PropTypes.txt bute file)
	ADD_STRINGPROP_FLAG(Filename, "", PF_HIDDEN | PF_MODEL)
	ADD_STRINGPROP_FLAG(Skin, "", PF_HIDDEN)
	ADD_VECTORPROP_VAL_FLAG(Scale, 1.0f, 1.0f, 1.0f, 0)
	ADD_VISIBLE_FLAG(1, PF_HIDDEN)
	ADD_SOLID_FLAG(0, PF_HIDDEN)
	ADD_GRAVITY_FLAG(0, PF_HIDDEN)
	ADD_SHADOW_FLAG(0, PF_HIDDEN)
    ADD_BOOLPROP_FLAG(MoveToFloor, LTTRUE, PF_HIDDEN)
	ADD_REALPROP_FLAG(Alpha, 1.0f, PF_HIDDEN)
	ADD_COLORPROP_FLAG(ObjectColor, 255.0f, 255.0f, 255.0f, PF_HIDDEN)
    ADD_BOOLPROP_FLAG(Additive, LTFALSE, PF_HIDDEN)
    ADD_BOOLPROP_FLAG(Multiply, LTFALSE, PF_HIDDEN)
    ADD_BOOLPROP_FLAG(RayHit, LTTRUE, PF_HIDDEN)

	// The list of available prop types...
	ADD_STRINGPROP_FLAG(Type, "", PF_STATICLIST | PF_DIMS | PF_LOCALDIMS)
END_CLASS_DEFAULT_FLAGS_PLUGIN(PropType, Prop, NULL, NULL, 0, CPropTypePlugin)


// Register with the CommandMgr...

CMDMGR_BEGIN_REGISTER_CLASS( PropType )
CMDMGR_END_REGISTER_CLASS( PropType, Prop )


LTRESULT CPropTypePlugin::PreHook_EditStringList(
	const char* szRezPath,
	const char* szPropName,
	char** aszStrings,
    uint32* pcStrings,
    const uint32 cMaxStrings,
    const uint32 cMaxStringLength)
{

	if ( LT_OK == CPropPlugin::PreHook_EditStringList(szRezPath, szPropName,
		aszStrings, pcStrings, cMaxStrings, cMaxStringLength) )
	{
		return LT_OK;
	}
	else if (_strcmpi("Type", szPropName) == 0)
	{
		if (m_PropTypeMgrPlugin.PreHook_EditStringList(szRezPath,
			szPropName, aszStrings, pcStrings, cMaxStrings, cMaxStringLength) == LT_OK)
		{
			qsort( aszStrings, *pcStrings, sizeof( char * ), CaseInsensitiveCompare );

			return LT_OK;
		}
	}

	return LT_UNSUPPORTED;
}

LTRESULT CPropTypePlugin::PreHook_Dims(
			const char* szRezPath,
			const char* szPropValue,
			char* szModelFilenameBuf,
			int	  nModelFilenameBufLen,
			LTVector & vDims)
{
	if (m_PropTypeMgrPlugin.PreHook_Dims(szRezPath, szPropValue,
		szModelFilenameBuf, nModelFilenameBufLen, vDims) == LT_OK)
	{
		return LT_OK;
	}

	return LT_UNSUPPORTED;

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PropType::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 PropType::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
            uint32 dwRet = Prop::EngineMessageFn(messageID, pData, fData);

			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				ObjectCreateStruct* pStruct = (ObjectCreateStruct*)pData;
				if( !ReadProp(pStruct))
					return 0;
			}

			return dwRet;
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

	return Prop::EngineMessageFn(messageID, pData, fData);
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PropType::PropRead()
//
//	PURPOSE:	Update properties
//
// ----------------------------------------------------------------------- //

bool PropType::ReadProp(ObjectCreateStruct *pData)
{
	if (!pData) 
		return false;

	// Type

	GenericProp genProp;

    if (g_pLTServer->GetPropGeneric("Type", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
			m_sPropType = genProp.m_String;
		}
	}

	
	// Model
	PROPTYPE* pPropType = g_pPropTypeMgr->GetPropType(( char* )m_sPropType.c_str( ));
	if (!pPropType || pPropType->sFilename.empty( )) 
		return false;

	pPropType->SetupModel( pData );

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PropType::Setup()
//
//	PURPOSE:	Sets up proptype object from PROPTYPE struct.
//
// ----------------------------------------------------------------------- //

bool PropType::SetPropType( char const* pszPropType )
{
	// Check inputs.
	// Check inputs.
	if( !pszPropType || !pszPropType[0] )
		return false;

	m_sPropType = pszPropType;

	return Setup( );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PropType::Setup()
//
//	PURPOSE:	Sets up proptype based on member variables.
//
// ----------------------------------------------------------------------- //

bool PropType::Setup( )
{
	PROPTYPE* pPropType = g_pPropTypeMgr->GetPropType(( char* )m_sPropType.c_str( ));
	if( !pPropType )
		return false;

	ObjectCreateStruct ocs;
	pPropType->SetupModel( &ocs );

	// Disturbance

	PROP_DISTURB* pDisturb = pPropType->pDisturb;
	if(pDisturb != LTNULL)
	{
		if(m_pDisturb == LTNULL)
		{
			m_pDisturb = debug_new(PropDisturbStruct);
			m_dwFlags |= FLAG_TOUCH_NOTIFY;
		}

		if ( !pDisturb->sTouchSound.empty( ))
		{
		    m_pDisturb->sTouchSoundName = pDisturb->sTouchSound;
		}

		m_pDisturb->pPD = pDisturb;

		m_damage.SetDestroyedStimulus(pDisturb->fStimRadius, pDisturb->nDestroyAlarmLevel);
	}

	m_vObjectColor	= pPropType->vObjectColor;
	m_bMoveToFloor	= pPropType->bMoveToFloor;
	m_fAlpha		= pPropType->fAlpha;

	// Only use the prop type scale if the scale wasn't set in DEdit...

	if (m_vScale.x == 1.0 && m_vScale.y == 1.0 && m_vScale.z == 1.0)
	{
		m_vScale = pPropType->vScale;
	}

	m_dwFlags = (pPropType->bVisible ? (m_dwFlags | FLAG_VISIBLE) : (m_dwFlags & ~FLAG_VISIBLE));
	m_dwFlags = (pPropType->bSolid ? (m_dwFlags | FLAG_SOLID) : (m_dwFlags & ~FLAG_SOLID));
	m_dwFlags = (pPropType->bGravity ? (m_dwFlags | FLAG_GRAVITY) : (m_dwFlags & ~FLAG_GRAVITY));
	m_dwFlags = (pPropType->bShadow	? (m_dwFlags | FLAG_SHADOW) : (m_dwFlags & ~FLAG_SHADOW));

	if (pPropType->bRayHit)
	{
		// Set touch notify so projectiles can impact with us...
		m_dwFlags |= FLAG_TOUCH_NOTIFY;
		m_dwFlags |= FLAG_RAYHIT;
	}
	else
	{
		m_dwFlags &= ~FLAG_TOUCH_NOTIFY;
		m_dwFlags &= ~FLAG_RAYHIT;
	}

	if(pPropType->bActivateable)
	{
		m_dwUsrFlgs |= USRFLG_CAN_ACTIVATE;
		
		// Since we are activateable try and set our activate type...

		if( !pPropType->sActivateType.empty() )
		{
			m_ActivateTypeHandler.SetActivateType( pPropType->sActivateType.c_str() );
		}
	}

	m_bTouchable = pPropType->bTouchable;

	m_damage.SetArmorPoints(0.0f);
	m_damage.SetMaxArmorPoints(0.0f);
    m_damage.SetCanHeal(LTFALSE);
    m_damage.SetCanRepair(LTFALSE);
    m_damage.SetApplyDamagePhysics(LTFALSE);
	m_damage.m_bRemoveOnDeath = LTFALSE;

	if (pPropType->nHitPoints >= 0)
	{
        m_damage.SetMaxHitPoints((LTFLOAT)pPropType->nHitPoints);
        m_damage.SetHitPoints((LTFLOAT)pPropType->nHitPoints);
	}
	else
	{
		m_damage.SetMaxHitPoints(1.0f);
		m_damage.SetHitPoints(1.0f);
        m_damage.SetNeverDestroy( LTTRUE );
	}

	// Set the debris type...

	if (!pPropType->sDebrisType.empty( ))
	{
		DEBRIS* pDebris = g_pDebrisMgr->GetDebris(pPropType->sDebrisType.c_str( ));
		if (pDebris)
		{
			m_damage.m_nDebrisId = pDebris->nId;
		}
	}

	if( LT_OK != g_pCommonLT->SetObjectFilenames(m_hObject, &ocs))
		return false;

	// Let the prop do some setup.
	if( !Prop::Setup( ))
		return false;

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PropType::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void PropType::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
	if (!pMsg)
		return;

	SAVE_CHARSTRING( m_sPropType.c_str( ));
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PropType::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void PropType::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
	if (!pMsg)
		return;

	if( g_pVersionMgr->GetCurrentSaveVersion( ) > CVersionMgr::kSaveVersion__1_2 )
	{
		char szPropType[256] = "";
		LOAD_CHARSTRING( szPropType, ARRAY_LEN( szPropType ));
	}
}