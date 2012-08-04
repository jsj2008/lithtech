// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#include "stdafx.h"
#include "AINode.h"
#include "ServerUtilities.h"
#include "AIUtils.h"
#include "AIButeMgr.h"
#include "AIHuman.h"
#include "AIVolumeMgr.h"

// LT Object implementation

BEGIN_CLASS(AINode)

	ADD_VECTORPROP_VAL_FLAG(Dims, 16.0f, 16.0f, 16.0f, PF_HIDDEN | PF_DIMS)

	PROP_DEFINEGROUP(CoverInfo, PF_GROUP1)

		ADD_BOOLPROP_FLAG(Duck,								LTFALSE,		PF_GROUP1)
		ADD_BOOLPROP_FLAG(BlindFire,						LTFALSE,		PF_GROUP1)
		ADD_BOOLPROP_FLAG(1WayRoll,							LTFALSE,		PF_GROUP1)
		ADD_BOOLPROP_FLAG(2WayRoll,							LTFALSE,		PF_GROUP1)
		ADD_BOOLPROP_FLAG(1WayStep,							LTFALSE,		PF_GROUP1)
		ADD_BOOLPROP_FLAG(2WayStep,							LTFALSE,		PF_GROUP1)
		ADD_BOOLPROP_FLAG(IgnoreCoverDir,					LTFALSE,		PF_GROUP1)
		ADD_REALPROP_FLAG(CoverFov,							90.0f,			PF_GROUP1|PF_FIELDOFVIEW)
		ADD_REALPROP_FLAG(CoverRadius,						512.0f,			PF_GROUP1|PF_RADIUS)
		ADD_REALPROP_FLAG(CoverThreatRadius,				256.0f,			PF_GROUP1|PF_RADIUS)
		ADD_STRINGPROP_FLAG(CoverThreatRadiusReaction,		"ATTACK",		PF_GROUP1)
		ADD_STRINGPROP_FLAG(CoverObject,					"",				PF_GROUP1|PF_OBJECTLINK)
		ADD_REALPROP_FLAG(CoverTimeout,						15.0f,			PF_GROUP1)
		ADD_STRINGPROP_FLAG(CoverTimeoutCmd,				"REEVALUATE",	PF_GROUP1)
		ADD_STRINGPROP_FLAG(CoverDamageCmd,					"ATTACK",		PF_GROUP1)
		ADD_REALPROP_FLAG(CoverHitpointsBoost,				2.0f,			PF_GROUP1)

	PROP_DEFINEGROUP(PanicInfo, PF_GROUP2)

		ADD_BOOLPROP_FLAG(Stand,							LTFALSE,		PF_GROUP2)
		ADD_BOOLPROP_FLAG(Crouch,							LTFALSE,		PF_GROUP2)
		ADD_REALPROP_FLAG(PanicRadius,						512.0f,			PF_GROUP2|PF_RADIUS)
		ADD_STRINGPROP_FLAG(PanicObject,					"",				PF_GROUP2|PF_OBJECTLINK)

	PROP_DEFINEGROUP(VantageInfo, PF_GROUP3)

		ADD_BOOLPROP_FLAG(Vantage,							LTFALSE,		PF_GROUP3)
		ADD_BOOLPROP_FLAG(IgnoreVantageDir,					LTFALSE,		PF_GROUP3)
		ADD_REALPROP_FLAG(VantageFov,						90.0f,			PF_GROUP3/*|PF_FIELDOFVIEW*/)
		ADD_REALPROP_FLAG(VantageRadius,					512.0f,			PF_GROUP3|PF_RADIUS)
		ADD_REALPROP_FLAG(VantageThreatRadius,				256.0f,			PF_GROUP3|PF_RADIUS)
		ADD_STRINGPROP_FLAG(VantageThreatRadiusReaction,	"ATTACK",		PF_GROUP3)
		ADD_STRINGPROP_FLAG(VantageDamageCmd,				"REEVALUATE",	PF_GROUP3)

	PROP_DEFINEGROUP(SearchInfo, PF_GROUP4)

		ADD_BOOLPROP_FLAG(ShineFlashlight,					LTFALSE,		PF_GROUP4)
		ADD_BOOLPROP_FLAG(LookUnder,						LTFALSE,		PF_GROUP4)
		ADD_BOOLPROP_FLAG(LookOver,							LTFALSE,		PF_GROUP4)
		ADD_BOOLPROP_FLAG(LookLeft,							LTFALSE,		PF_GROUP4)
		ADD_BOOLPROP_FLAG(LookRight,						LTFALSE,		PF_GROUP4)
		ADD_BOOLPROP_FLAG(Alert1,							LTFALSE,		PF_GROUP4)
		ADD_BOOLPROP_FLAG(Alert2,							LTFALSE,		PF_GROUP4)
		ADD_BOOLPROP_FLAG(Alert3,							LTFALSE,		PF_GROUP4)

	PROP_DEFINEGROUP(MiscInfo, PF_GROUP5)

		ADD_STRINGPROP_FLAG(UseObject,						"",				PF_GROUP5|PF_OBJECTLINK)
		ADD_REALPROP_FLAG(UseObjectRadius,					512.0f,			PF_GROUP5|PF_RADIUS)

		ADD_STRINGPROP_FLAG(PickupObject,					"",				PF_GROUP5|PF_OBJECTLINK)
		ADD_REALPROP_FLAG(PickupObjectRadius,				512.0f,			PF_GROUP5|PF_RADIUS)

		ADD_STRINGPROP_FLAG(BackupCmd,						"",				PF_GROUP5)
		ADD_REALPROP_FLAG(BackupRadius,						512.0f,			PF_GROUP5|PF_RADIUS)

		ADD_STRINGPROP_FLAG(TrainingFailureCmd,				"",				PF_GROUP5)

		ADD_BOOLPROP_FLAG(Poodle,							LTFALSE,		PF_GROUP5)

END_CLASS_DEFAULT(AINode, BaseClass, NULL, NULL)

// Statics

const uint32    CAINode::kInvalidNodeID				= 0xFFFFFFFF;

const uint32    CAINode::kSearchFlagShineFlashlight	= 0x01;
const uint32    CAINode::kSearchFlagLookUnder		= 0x02;
const uint32    CAINode::kSearchFlagLookOver		= 0x04;
const uint32    CAINode::kSearchFlagLookLeft		= 0x08;
const uint32    CAINode::kSearchFlagLookRight		= 0x10;
const uint32    CAINode::kSearchFlagAlert1			= 0x20;
const uint32    CAINode::kSearchFlagAlert2			= 0x40;
const uint32    CAINode::kSearchFlagAlert3			= 0x80;
const uint32    CAINode::kSearchFlagAny				= 0xFF;
const uint32	CAINode::kNumSearchFlags			= 8;

const uint32    CAINode::kCoverFlagDuck				= 0x01;
const uint32    CAINode::kCoverFlagBlind			= 0x02;
const uint32    CAINode::kCoverFlag1WayCorner		= 0x04;
const uint32    CAINode::kCoverFlag2WayCorner		= 0x08;
const uint32    CAINode::kCoverFlagAny				= 0x0F;
const uint32	CAINode::kNumCoverFlags				= 4;

const uint32    CAINode::kPanicFlagStand			= 0x01;
const uint32    CAINode::kPanicFlagCrouch			= 0x02;
const uint32    CAINode::kPanicFlagAny				= 0x03;
const uint32	CAINode::kNumPanicFlags				= 2;

const uint32    CAINode::kVantageFlagVantage		= 0x01;
const uint32    CAINode::kVantageFlagAny			= 0x01;
const uint32	CAINode::kNumVantageFlags			= 1;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AINode::AINode()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

AINode::AINode() : BaseClass(OT_NORMAL)
{
	m_dwSearchFlags = 0x0;

	m_bIgnoreCoverDir = LTFALSE;
	m_fCoverFov = 120.0f;
	m_hstrCoverObject = LTNULL;
	m_dwCoverFlags = 0x0;
	m_fCoverRadiusSqr = 0.0f;
	m_fCoverThreatRadiusSqr = 0.0f;
	m_hstrCoverThreatRadiusReaction = LTNULL;
	m_fCoverTimeout = 999999.0f;
	m_hstrCoverTimeoutCmd = LTNULL;
	m_hstrCoverDamageCmd = LTNULL;

	m_fCoverHitpointsBoost = 2.0f;

	m_dwPanicFlags = 0x0;
	m_hstrPanicObject = LTNULL;
	m_fPanicRadiusSqr = 0.0f;

	m_bIgnoreVantageDir = LTFALSE;
	m_fVantageFov = 120.0f;
	m_dwVantageFlags = 0x0;
	m_fVantageRadiusSqr = 0.0f;
	m_fVantageThreatRadiusSqr = 0.0f;
	m_hstrVantageThreatRadiusReaction = LTNULL;
	m_hstrVantageDamageCmd = LTNULL;

	m_hstrUseObject = LTNULL;
	m_fUseObjectRadiusSqr = 0.0f;

	m_hstrPickupObject = LTNULL;
	m_fPickupObjectRadiusSqr = 0.0f;

	m_hstrBackupCmd = LTNULL;
	m_fBackupRadiusSqr = 0.0f;

	m_hstrTrainingFailureCmd = LTNULL;

	m_bPoodle = LTFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AINode::~AINode()
//
//	PURPOSE:	Destroy the object
//
// ----------------------------------------------------------------------- //

AINode::~AINode()
{
	FREE_HSTRING(m_hstrCoverObject);
	FREE_HSTRING(m_hstrCoverThreatRadiusReaction);
	FREE_HSTRING(m_hstrCoverDamageCmd);
	FREE_HSTRING(m_hstrCoverTimeoutCmd);
	FREE_HSTRING(m_hstrVantageThreatRadiusReaction);
	FREE_HSTRING(m_hstrVantageDamageCmd);
	FREE_HSTRING(m_hstrPanicObject);
	FREE_HSTRING(m_hstrUseObject);
	FREE_HSTRING(m_hstrPickupObject);
	FREE_HSTRING(m_hstrBackupCmd);
	FREE_HSTRING(m_hstrTrainingFailureCmd);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AINode::ReadProp
//
//	PURPOSE:	Reads properties
//
// ----------------------------------------------------------------------- //

LTBOOL AINode::ReadProp(ObjectCreateStruct *pData)
{
	GenericProp genProp;
    if (!g_pLTServer || !pData) return LTFALSE;

	m_dwSearchFlags = 0x0;
	m_dwCoverFlags = 0x0;
	m_dwPanicFlags = 0x0;
	m_dwVantageFlags = 0x0;

    if ( g_pLTServer->GetPropGeneric( "ShineFlashlight", &genProp ) == LT_OK )
		m_dwSearchFlags |= genProp.m_Bool ? CAINode::kSearchFlagShineFlashlight : 0x0;

    if ( g_pLTServer->GetPropGeneric( "LookUnder", &genProp ) == LT_OK )
		m_dwSearchFlags |= genProp.m_Bool ? CAINode::kSearchFlagLookUnder : 0x0;

    if ( g_pLTServer->GetPropGeneric( "LookOver", &genProp ) == LT_OK )
		m_dwSearchFlags |= genProp.m_Bool ? CAINode::kSearchFlagLookOver : 0x0;

    if ( g_pLTServer->GetPropGeneric( "LookLeft", &genProp ) == LT_OK )
		m_dwSearchFlags |= genProp.m_Bool ? CAINode::kSearchFlagLookLeft : 0x0;

    if ( g_pLTServer->GetPropGeneric( "LookRight", &genProp ) == LT_OK )
		m_dwSearchFlags |= genProp.m_Bool ? CAINode::kSearchFlagLookRight : 0x0;

    if ( g_pLTServer->GetPropGeneric( "Alert1", &genProp ) == LT_OK )
		m_dwSearchFlags |= genProp.m_Bool ? CAINode::kSearchFlagAlert1 : 0x0;

    if ( g_pLTServer->GetPropGeneric( "Alert2", &genProp ) == LT_OK )
		m_dwSearchFlags |= genProp.m_Bool ? CAINode::kSearchFlagAlert2 : 0x0;

    if ( g_pLTServer->GetPropGeneric( "Alert3", &genProp ) == LT_OK )
		m_dwSearchFlags |= genProp.m_Bool ? CAINode::kSearchFlagAlert3 : 0x0;

    if ( g_pLTServer->GetPropGeneric( "Duck", &genProp ) == LT_OK )
		m_dwCoverFlags |= genProp.m_Bool ? CAINode::kCoverFlagDuck : 0x0;

    if ( g_pLTServer->GetPropGeneric( "BlindFire", &genProp ) == LT_OK )
		m_dwCoverFlags |= genProp.m_Bool ? CAINode::kCoverFlagBlind : 0x0;

    if ( g_pLTServer->GetPropGeneric( "1WayRoll", &genProp ) == LT_OK )
		m_dwCoverFlags |= genProp.m_Bool ? CAINode::kCoverFlag1WayCorner : 0x0;

    if ( g_pLTServer->GetPropGeneric( "2WayRoll", &genProp ) == LT_OK )
		m_dwCoverFlags |= genProp.m_Bool ? CAINode::kCoverFlag2WayCorner : 0x0;

    if ( g_pLTServer->GetPropGeneric( "1WayStep", &genProp ) == LT_OK )
		m_dwCoverFlags |= genProp.m_Bool ? CAINode::kCoverFlag1WayCorner : 0x0;

    if ( g_pLTServer->GetPropGeneric( "2WayStep", &genProp ) == LT_OK )
		m_dwCoverFlags |= genProp.m_Bool ? CAINode::kCoverFlag2WayCorner : 0x0;

    if ( g_pLTServer->GetPropGeneric( "IgnoreCoverDir", &genProp ) == LT_OK )
		m_bIgnoreCoverDir = genProp.m_Bool;

    if ( g_pLTServer->GetPropGeneric( "CoverFov", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			m_fCoverFov = genProp.m_Float;

    if ( g_pLTServer->GetPropGeneric( "Stand", &genProp ) == LT_OK )
		m_dwPanicFlags |= genProp.m_Bool ? CAINode::kPanicFlagStand : 0x0;

    if ( g_pLTServer->GetPropGeneric( "Crouch", &genProp ) == LT_OK )
		m_dwPanicFlags |= genProp.m_Bool ? CAINode::kPanicFlagCrouch : 0x0;

    if ( g_pLTServer->GetPropGeneric( "Vantage", &genProp ) == LT_OK )
		m_dwVantageFlags |= genProp.m_Bool ? CAINode::kVantageFlagVantage : 0x0;

    if ( g_pLTServer->GetPropGeneric( "IgnoreVantageDir", &genProp ) == LT_OK )
		m_bIgnoreVantageDir = genProp.m_Bool;

    if ( g_pLTServer->GetPropGeneric( "VantageFov", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			m_fVantageFov = genProp.m_Float;

    if ( g_pLTServer->GetPropGeneric( "CoverObject", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
            m_hstrCoverObject = g_pLTServer->CreateString( genProp.m_String );

    if ( g_pLTServer->GetPropGeneric( "CoverRadius", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			m_fCoverRadiusSqr = genProp.m_Float*genProp.m_Float;

    if ( g_pLTServer->GetPropGeneric( "CoverThreatRadius", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			m_fCoverThreatRadiusSqr = genProp.m_Float*genProp.m_Float;

    if ( g_pLTServer->GetPropGeneric( "CoverThreatRadiusReaction", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
            m_hstrCoverThreatRadiusReaction = g_pLTServer->CreateString( genProp.m_String );

    if ( g_pLTServer->GetPropGeneric( "CoverDamageCmd", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
            m_hstrCoverDamageCmd = g_pLTServer->CreateString( genProp.m_String );

    if ( g_pLTServer->GetPropGeneric( "CoverTimeout", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			m_fCoverTimeout = genProp.m_Float;

    if ( g_pLTServer->GetPropGeneric( "CoverHitpointsBoost", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			m_fCoverHitpointsBoost = genProp.m_Float;

    if ( g_pLTServer->GetPropGeneric( "CoverTimeoutCmd", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
            m_hstrCoverTimeoutCmd = g_pLTServer->CreateString( genProp.m_String );

    if ( g_pLTServer->GetPropGeneric( "VantageRadius", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			m_fVantageRadiusSqr = genProp.m_Float*genProp.m_Float;

    if ( g_pLTServer->GetPropGeneric( "VantageThreatRadius", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			m_fVantageThreatRadiusSqr = genProp.m_Float*genProp.m_Float;

    if ( g_pLTServer->GetPropGeneric( "VantageThreatRadiusReaction", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
            m_hstrVantageThreatRadiusReaction = g_pLTServer->CreateString( genProp.m_String );

    if ( g_pLTServer->GetPropGeneric( "VantageDamageCmd", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
            m_hstrVantageDamageCmd = g_pLTServer->CreateString( genProp.m_String );

    if ( g_pLTServer->GetPropGeneric( "PanicObject", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
            m_hstrPanicObject = g_pLTServer->CreateString( genProp.m_String );

    if ( g_pLTServer->GetPropGeneric( "PanicRadius", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			m_fPanicRadiusSqr = genProp.m_Float*genProp.m_Float;

    if ( g_pLTServer->GetPropGeneric( "UseObject", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
            m_hstrUseObject = g_pLTServer->CreateString( genProp.m_String );

    if ( g_pLTServer->GetPropGeneric( "UseObjectRadius", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			m_fUseObjectRadiusSqr = genProp.m_Float*genProp.m_Float;

    if ( g_pLTServer->GetPropGeneric( "PickupObject", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
            m_hstrPickupObject = g_pLTServer->CreateString( genProp.m_String );

    if ( g_pLTServer->GetPropGeneric( "PickupRadius", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			m_fPickupObjectRadiusSqr = genProp.m_Float*genProp.m_Float;

    if ( g_pLTServer->GetPropGeneric( "BackupCmd", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
            m_hstrBackupCmd = g_pLTServer->CreateString( genProp.m_String );

    if ( g_pLTServer->GetPropGeneric( "TrainingFailureCmd", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
            m_hstrTrainingFailureCmd = g_pLTServer->CreateString( genProp.m_String );

	if ( g_pLTServer->GetPropGeneric( "BackupRadius", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			m_fBackupRadiusSqr = genProp.m_Float*genProp.m_Float;

	LTVector vAngles;
    if ( g_pLTServer->GetPropRotationEuler( "Rotation", &vAngles ) == LT_OK )
		m_vInitialPitchYawRoll = vAngles;

    if ( g_pLTServer->GetPropGeneric( "Poodle", &genProp ) == LT_OK )
		m_bPoodle = genProp.m_Bool;

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AINode::EngineMessageFn
//
//	PURPOSE:	Handles engine message functions
//
// ----------------------------------------------------------------------- //

uint32 AINode::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
            uint32 dwRet = BaseClass::EngineMessageFn(messageID, pData, fData);

			int nInfo = (int)fData;
			if (nInfo == PRECREATE_WORLDFILE || nInfo == PRECREATE_STRINGPROP)
			{
				ReadProp((ObjectCreateStruct*)pData);
			}
			return dwRet;
			break;
		}

		default : break;
	}

	return BaseClass::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINode::CAINode()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

CAINode::CAINode()
{
	VEC_INIT(m_vPos);
	VEC_INIT(m_vUp);
	VEC_INIT(m_vForward);
	VEC_INIT(m_vRight);
	m_dwID = CAINode::kInvalidNodeID;

	m_hstrName = LTNULL;

	m_bLocked = LTFALSE;

	m_dwSearchFlags = 0x0;
	m_bSearched = LTFALSE;

	m_bIgnoreCoverDir = LTFALSE;
	m_fCoverFovDp = 0.0f;
	m_dwCoverFlags = 0x0;
	m_hstrCoverObject = LTNULL;
	m_fCoverRadiusSqr = 0.0f;
	m_fCoverThreatRadiusSqr = 0.0f;
	m_hstrCoverThreatRadiusReaction = LTNULL;
	m_hstrCoverDamageCmd = LTNULL;
	m_fCoverTimeout = 999999.0f;
	m_hstrCoverTimeoutCmd = LTNULL;

	m_fCoverHitpointsBoost = 2.0f;

	m_dwPanicFlags = 0x0;
	m_hstrPanicObject = LTNULL;
	m_fPanicRadiusSqr = 0.0f;

	m_bIgnoreVantageDir = LTFALSE;
	m_fVantageFovDp = 0.0f;
	m_dwVantageFlags = 0x0;
	m_fVantageRadiusSqr = 0.0f;
	m_fVantageThreatRadiusSqr = 0.0f;
	m_hstrVantageThreatRadiusReaction = LTNULL;
	m_hstrVantageDamageCmd = LTNULL;

	m_hstrUseObject = LTNULL;
	m_fUseObjectRadiusSqr = 0.0f;

	m_hstrPickupObject = LTNULL;
	m_fPickupObjectRadiusSqr = 0.0f;

	m_hstrBackupCmd = LTNULL;
	m_fBackupRadiusSqr = 0.0f;

	m_hstrTrainingFailureCmd = LTNULL;

	m_bPoodle = LTFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINode::~CAINode()
//
//	PURPOSE:	Destroy the object
//
// ----------------------------------------------------------------------- //

CAINode::~CAINode()
{
	FREE_HSTRING(m_hstrName);
	FREE_HSTRING(m_hstrCoverObject);
	FREE_HSTRING(m_hstrCoverThreatRadiusReaction);
	FREE_HSTRING(m_hstrCoverDamageCmd);
	FREE_HSTRING(m_hstrCoverTimeoutCmd);
	FREE_HSTRING(m_hstrVantageThreatRadiusReaction);
	FREE_HSTRING(m_hstrVantageDamageCmd);
	FREE_HSTRING(m_hstrPanicObject);
	FREE_HSTRING(m_hstrUseObject);
	FREE_HSTRING(m_hstrPickupObject);
	FREE_HSTRING(m_hstrBackupCmd);
	FREE_HSTRING(m_hstrTrainingFailureCmd);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINode::Init()
//
//	PURPOSE:	Initialize us from an engine object AINode
//
// ----------------------------------------------------------------------- //

void CAINode::Init(uint32 dwID, const AINode& node)
{
	m_dwID = dwID;

    m_hstrName = g_pLTServer->CreateString(g_pLTServer->GetObjectName(node.m_hObject));

	m_dwSearchFlags = node.m_dwSearchFlags;

	m_dwCoverFlags = node.m_dwCoverFlags;
	m_bIgnoreCoverDir = node.m_bIgnoreCoverDir;
	m_fCoverFovDp = FOV2DP(node.m_fCoverFov);
    m_hstrCoverObject = g_pLTServer->CopyString(node.m_hstrCoverObject);
	m_fCoverRadiusSqr = node.m_fCoverRadiusSqr;
	m_fCoverThreatRadiusSqr = node.m_fCoverThreatRadiusSqr;
    m_hstrCoverThreatRadiusReaction = g_pLTServer->CopyString(node.m_hstrCoverThreatRadiusReaction);
	m_fCoverTimeout = node.m_fCoverTimeout;
    m_hstrCoverTimeoutCmd = g_pLTServer->CopyString(node.m_hstrCoverTimeoutCmd);
    m_hstrCoverDamageCmd = g_pLTServer->CopyString(node.m_hstrCoverDamageCmd);

	m_fCoverHitpointsBoost = node.m_fCoverHitpointsBoost;

	m_dwVantageFlags = node.m_dwVantageFlags;
	m_bIgnoreVantageDir = node.m_bIgnoreVantageDir;
	m_fVantageFovDp = FOV2DP(node.m_fVantageFov);
	m_fVantageRadiusSqr = node.m_fVantageRadiusSqr;
	m_fVantageThreatRadiusSqr = node.m_fVantageThreatRadiusSqr;
    m_hstrVantageThreatRadiusReaction = g_pLTServer->CopyString(node.m_hstrVantageThreatRadiusReaction);
    m_hstrVantageDamageCmd = g_pLTServer->CopyString(node.m_hstrVantageDamageCmd);

    m_hstrUseObject = g_pLTServer->CopyString(node.m_hstrUseObject);
	m_fUseObjectRadiusSqr = node.m_fUseObjectRadiusSqr;
    
	m_hstrPickupObject = g_pLTServer->CopyString(node.m_hstrPickupObject);
	m_fPickupObjectRadiusSqr = node.m_fPickupObjectRadiusSqr;

    m_hstrBackupCmd = g_pLTServer->CopyString(node.m_hstrBackupCmd);
	m_fBackupRadiusSqr = node.m_fBackupRadiusSqr;

    m_hstrTrainingFailureCmd = g_pLTServer->CopyString(node.m_hstrTrainingFailureCmd);

	m_dwPanicFlags = node.m_dwPanicFlags;
    m_hstrPanicObject = g_pLTServer->CopyString(node.m_hstrPanicObject);
	m_fPanicRadiusSqr = node.m_fPanicRadiusSqr;

    g_pLTServer->GetObjectPos(node.m_hObject, &m_vPos);

	if ( m_dwCoverFlags & CAINode::kCoverFlag1WayCorner && 
		 m_dwCoverFlags & CAINode::kCoverFlag2WayCorner )
	{
		// We can't be two way AND one way cover!

		g_pLTServer->CPrint("Warning, AINode %s has 2way and 1way cover specified!", g_pLTServer->GetStringData(m_hstrName));
		m_dwCoverFlags = 0;
	}

	LTRotation rRot;
	g_pMathLT->SetupEuler(rRot, EXPANDVEC(node.m_vInitialPitchYawRoll));

	if ( m_dwCoverFlags & CAINode::kCoverFlag2WayCorner )
	{
		// Clean up pitch/yaw/roll if we're two way cover

		LTVector vNull, vForward;
		g_pMathLT->GetRotationVectors(rRot, vNull, vNull, vForward);

		LTFLOAT fPitch, fYaw, fRoll;

		fPitch = 0.0f;
		fYaw = (LTFLOAT)acos(vForward.z);
		fRoll = 0.0f;

		g_pMathLT->SetupEuler(rRot, fPitch, fYaw, fRoll);
	}

	g_pMathLT->GetRotationVectors(rRot, m_vRight, m_vUp, m_vForward);

	m_bPoodle = node.m_bPoodle;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINode::Verify()
//
//	PURPOSE:	Sanity check
//
// ----------------------------------------------------------------------- //

void CAINode::Verify()
{
	if ( NULL == g_pAIVolumeMgr->FindContainingVolume(m_vPos, 106.0f) )
	{
		g_pLTServer->CPrint("Warning, AINode \"%s\" is not in an AIVolume!", g_pLTServer->GetStringData(m_hstrName));
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINode::Save()
//
//	PURPOSE:	Save the node
//
// ----------------------------------------------------------------------- //

void CAINode::Save(HMESSAGEWRITE hWrite)
{
	SAVE_VECTOR(m_vPos);
	SAVE_VECTOR(m_vUp);
	SAVE_VECTOR(m_vForward);
	SAVE_VECTOR(m_vRight);
	SAVE_DWORD(m_dwID);
	SAVE_HSTRING(m_hstrName);
	SAVE_DWORD(m_dwSearchFlags);
	SAVE_BOOL(m_bSearched);
	SAVE_DWORD(m_dwCoverFlags);
	SAVE_DWORD(m_dwPanicFlags);
	SAVE_DWORD(m_dwVantageFlags);
	SAVE_BOOL(m_bLocked);
	SAVE_HSTRING(m_hstrCoverObject);
	SAVE_HSTRING(m_hstrPanicObject);
	SAVE_HSTRING(m_hstrUseObject);
	SAVE_HSTRING(m_hstrPickupObject);
	SAVE_HSTRING(m_hstrBackupCmd);
	SAVE_HSTRING(m_hstrTrainingFailureCmd);
	SAVE_BOOL(m_bPoodle);
	SAVE_BOOL(m_bIgnoreCoverDir);
	SAVE_BOOL(m_bIgnoreVantageDir);
	SAVE_FLOAT(m_fCoverFovDp);
	SAVE_FLOAT(m_fVantageFovDp);
	SAVE_FLOAT(m_fCoverRadiusSqr);
	SAVE_FLOAT(m_fPanicRadiusSqr);
	SAVE_FLOAT(m_fUseObjectRadiusSqr);
	SAVE_FLOAT(m_fPickupObjectRadiusSqr);
	SAVE_FLOAT(m_fBackupRadiusSqr);
	SAVE_FLOAT(m_fCoverThreatRadiusSqr);
	SAVE_HSTRING(m_hstrCoverThreatRadiusReaction);
	SAVE_FLOAT(m_fVantageRadiusSqr);
	SAVE_FLOAT(m_fVantageThreatRadiusSqr);
	SAVE_HSTRING(m_hstrVantageThreatRadiusReaction);
	SAVE_FLOAT(m_fCoverTimeout);
	SAVE_FLOAT(m_fCoverHitpointsBoost);
	SAVE_HSTRING(m_hstrCoverTimeoutCmd);
	SAVE_HSTRING(m_hstrCoverDamageCmd);
	SAVE_HSTRING(m_hstrVantageDamageCmd);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINode::Load()
//
//	PURPOSE:	Load the node
//
// ----------------------------------------------------------------------- //

void CAINode::Load(HMESSAGEREAD hRead)
{
	LOAD_VECTOR(m_vPos);
	LOAD_VECTOR(m_vUp);
	LOAD_VECTOR(m_vForward);
	LOAD_VECTOR(m_vRight);
	LOAD_DWORD(m_dwID);
	LOAD_HSTRING(m_hstrName);
	LOAD_DWORD(m_dwSearchFlags);
	LOAD_BOOL(m_bSearched);
	LOAD_DWORD(m_dwCoverFlags);
	LOAD_DWORD(m_dwPanicFlags);
	LOAD_DWORD(m_dwVantageFlags);
	LOAD_BOOL(m_bLocked);
	LOAD_HSTRING(m_hstrCoverObject);
	LOAD_HSTRING(m_hstrPanicObject);
	LOAD_HSTRING(m_hstrUseObject);
	LOAD_HSTRING(m_hstrPickupObject);
	LOAD_HSTRING(m_hstrBackupCmd);
	LOAD_HSTRING(m_hstrTrainingFailureCmd);
	LOAD_BOOL(m_bPoodle);
	LOAD_BOOL(m_bIgnoreCoverDir);
	LOAD_BOOL(m_bIgnoreVantageDir);
	LOAD_FLOAT(m_fCoverFovDp);
	LOAD_FLOAT(m_fVantageFovDp);
	LOAD_FLOAT(m_fCoverRadiusSqr);
	LOAD_FLOAT(m_fPanicRadiusSqr);
	LOAD_FLOAT(m_fUseObjectRadiusSqr);
	LOAD_FLOAT(m_fPickupObjectRadiusSqr);
	LOAD_FLOAT(m_fBackupRadiusSqr);
	LOAD_FLOAT(m_fCoverThreatRadiusSqr);
	LOAD_HSTRING(m_hstrCoverThreatRadiusReaction);
	LOAD_FLOAT(m_fVantageRadiusSqr);
	LOAD_FLOAT(m_fVantageThreatRadiusSqr);
	LOAD_HSTRING(m_hstrVantageThreatRadiusReaction);
	LOAD_FLOAT(m_fCoverTimeout);
	LOAD_FLOAT(m_fCoverHitpointsBoost);
	LOAD_HSTRING(m_hstrCoverTimeoutCmd);
	LOAD_HSTRING(m_hstrCoverDamageCmd);
	LOAD_HSTRING(m_hstrVantageDamageCmd);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINode::Search()
//
//	PURPOSE:	Searches the node
//
// ----------------------------------------------------------------------- //

void CAINode::Search()
{
	_ASSERT(IsSearchable());
	_ASSERT(!m_bSearched);

	m_bSearched = LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINode::GetSearchStatus()
//
//	PURPOSE:	Gets the node's Search status relative to a threat
//
// ----------------------------------------------------------------------- //

SearchStatus CAINode::GetSearchStatus(const LTVector& vPos, HOBJECT hThreat) const
{
	_ASSERT(IsSearchable());

	if ( m_bSearched )
	{
		return eSearchStatusSearchedRecently;
	}

	return eSearchStatusOk;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINode::GetCoverStatus()
//
//	PURPOSE:	Gets the node's cover status relative to a threat
//
// ----------------------------------------------------------------------- //

CoverStatus CAINode::GetCoverStatus(const LTVector& vPos, HOBJECT hThreat) const
{
	_ASSERT(IsCover());

	LTVector vThreatPos;
    g_pLTServer->GetObjectPos(hThreat, &vThreatPos);

	if ( VEC_DISTSQR(m_vPos, vThreatPos) < m_fCoverThreatRadiusSqr )
	{
		return eCoverStatusThreatInsideRadius;
	}

	if ( IsIgnoreCoverDir() ) return eCoverStatusOk;

	LTVector vThreatDir = vThreatPos - m_vPos;
	vThreatDir.Norm();

	if ( vThreatDir.Dot(m_vForward) > m_fCoverFovDp )
	{
		return eCoverStatusOk;
	}
	else
	{
		return eCoverStatusThreatOutsideFOV;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINode::IsPanicFromThreat()
//
//	PURPOSE:	Is Panic from the threat
//
// ----------------------------------------------------------------------- //

LTBOOL CAINode::IsPanicFromThreat(const LTVector& vPos, HOBJECT hThreat) const
{
	_ASSERT(IsPanic());

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINode::GetVantageStatus()
//
//	PURPOSE:	Get the node's vantage status relative to a threat
//
// ----------------------------------------------------------------------- //

VantageStatus CAINode::GetVantageStatus(const LTVector& vPos, HOBJECT hThreat) const
{
	_ASSERT(IsVantage());

	LTVector vThreatPos;
    g_pLTServer->GetObjectPos(hThreat, &vThreatPos);

	if ( VEC_DISTSQR(m_vPos, vThreatPos) < m_fVantageThreatRadiusSqr )
	{
		return eVantageStatusThreatInsideRadius;
	}

	if ( IsIgnoreVantageDir() ) return eVantageStatusOk;

	LTVector vThreatDir = vThreatPos - m_vPos;
	vThreatDir.Norm();

	if ( vThreatDir.Dot(m_vForward) > m_fVantageFovDp )
	{
		return eVantageStatusOk;
	}
	else
	{
		return eVantageStatusThreatOutsideFOV;
	}
}
