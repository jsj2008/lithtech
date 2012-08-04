// ----------------------------------------------------------------------- //
//
// MODULE  : PropType.cpp
//
// PURPOSE : Model PropType - Definition
//
// CREATED : 4/26/2000
//
// (c) 2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "PropType.h"

BEGIN_CLASS(PropType)
	ADD_DESTRUCTIBLE_MODEL_AGGREGATE(PF_GROUP1, PF_HIDDEN)

	// Hide all of our parent properties (these are all set via the
	// PropTypes.txt bute file)
	ADD_STRINGPROP_FLAG(Filename, "", PF_HIDDEN)
	ADD_STRINGPROP_FLAG(Skin, "", PF_HIDDEN)
	ADD_VECTORPROP_VAL_FLAG(Scale, 1.0f, 1.0f, 1.0f, 0)
	ADD_VISIBLE_FLAG(1, PF_HIDDEN)
	ADD_SOLID_FLAG(0, PF_HIDDEN)
	ADD_GRAVITY_FLAG(0, PF_HIDDEN)
	ADD_SHADOW_FLAG(0, PF_HIDDEN)
    ADD_BOOLPROP_FLAG(MoveToFloor, LTTRUE, PF_HIDDEN)
    ADD_BOOLPROP_FLAG(DetailTexture, LTFALSE, PF_HIDDEN)
    ADD_BOOLPROP_FLAG(Chrome, LTFALSE, PF_HIDDEN)
	ADD_REALPROP_FLAG(Alpha, 1.0f, PF_HIDDEN)
	ADD_COLORPROP_FLAG(ObjectColor, 255.0f, 255.0f, 255.0f, PF_HIDDEN)
    ADD_CHROMAKEY_FLAG(LTFALSE, PF_HIDDEN)
    ADD_BOOLPROP_FLAG(Additive, LTFALSE, PF_HIDDEN)
    ADD_BOOLPROP_FLAG(Multiply, LTFALSE, PF_HIDDEN)
    ADD_BOOLPROP_FLAG(RayHit, LTTRUE, PF_HIDDEN)
	ADD_STRINGPROP_FLAG(TouchSound, "", PF_FILENAME | PF_HIDDEN)
	ADD_REALPROP_FLAG(TouchSoundRadius, 500.0, PF_RADIUS | PF_HIDDEN)

	// The list of available prop types...
	ADD_STRINGPROP_FLAG(Type, "", PF_STATICLIST | PF_DIMS | PF_LOCALDIMS)
END_CLASS_DEFAULT_FLAGS_PLUGIN(PropType, Prop, NULL, NULL, 0, CPropTypePlugin)

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
				ReadProp(pStruct);
			}

			return dwRet;
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

void PropType::ReadProp(ObjectCreateStruct *pData)
{
	if (!pData) return;

    PROPTYPE* pPropType = LTNULL;
	GenericProp genProp;

    if (g_pLTServer->GetPropGeneric("Type", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
			pPropType = g_pPropTypeMgr->GetPropType(genProp.m_String);
		}
	}

	if (!pPropType || !pPropType->szFilename[0]) return;

	SAFE_STRCPY(pData->m_Filename, pPropType->szFilename);

	uint32 iSkin = 0;
	ConParse conParse;
	conParse.Init(pPropType->szSkin);
    while (g_pLTServer->Common()->Parse(&conParse) == LT_OK)
	{
		if (conParse.m_nArgs > 0)
		{
			SAFE_STRCPY(pData->m_SkinNames[iSkin], conParse.m_Args[0]);
			iSkin++;
		}

		if (iSkin >= MAX_MODEL_TEXTURES)
			break;
	}
	pData->m_SkinName[MAX_CS_FILENAME_LEN] = '\0';

    m_fTouchSoundRadius = (LTFLOAT) pPropType->nTouchSoundRadius;

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
	m_dwFlags = (pPropType->bChrome	? (m_dwFlags | FLAG_ENVIRONMENTMAP) : (m_dwFlags & ~FLAG_ENVIRONMENTMAP));
	m_dwFlags = (pPropType->bDetailTexture ? (m_dwFlags | FLAG_DETAILTEXTURE) : (m_dwFlags & ~FLAG_DETAILTEXTURE));

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

	if (pPropType->bAdditive)
	{
		m_dwFlags2 |= FLAG2_ADDITIVE;
		m_dwFlags  |= FLAG_FOGDISABLE;
	}
	else
	{
		m_dwFlags2 &= ~FLAG2_ADDITIVE;
	}

	if (pPropType->bMultiply)
	{
		m_dwFlags2 |= FLAG2_MULTIPLY;
		m_dwFlags  |= FLAG_FOGDISABLE;
	}
	else
	{
		m_dwFlags2 &= ~FLAG2_MULTIPLY;
	}

	if (pPropType->bChromaKey)
	{
		m_dwFlags2 |= FLAG2_CHROMAKEY;
	}
	else
	{
		m_dwFlags2 &= ~FLAG2_CHROMAKEY;
	}

	if (pPropType->szTouchSound[0])
	{
		FREE_HSTRING(m_hstrTouchSound);

        m_hstrTouchSound = g_pLTServer->CreateString(pPropType->szTouchSound);
		m_dwFlags |= FLAG_TOUCH_NOTIFY;
	}

	m_damage.SetArmorPoints(0.0f);
	m_damage.SetMaxArmorPoints(0.0f);
    m_damage.SetCanHeal(LTFALSE);
    m_damage.SetCanRepair(LTFALSE);
    m_damage.SetApplyDamagePhysics(LTFALSE);

	if (pPropType->nHitPoints >= 0)
	{
        m_damage.SetMaxHitPoints((LTFLOAT)pPropType->nHitPoints);
        m_damage.SetHitPoints((LTFLOAT)pPropType->nHitPoints);
	}
	else
	{
		m_damage.SetMaxHitPoints(1.0f);
		m_damage.SetHitPoints(1.0f);
        m_damage.SetCanDamage(LTFALSE);
	}

	// Set the debris type...

	if (pPropType->szDebrisType[0])
	{
		DEBRIS* pDebris = g_pDebrisMgr->GetDebris(pPropType->szDebrisType);
		if (pDebris)
		{
			m_damage.m_nDebrisId = pDebris->nId;
		}
	}


	// Make sure our object is set up correctly...

	pData->m_Flags  = m_dwFlags;
	pData->m_Flags2 = m_dwFlags2; 
}