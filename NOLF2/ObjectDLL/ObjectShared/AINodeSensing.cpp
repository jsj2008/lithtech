// ----------------------------------------------------------------------- //
//
// MODULE  : AINodeSensing.cpp
//
// PURPOSE : AINodeSensing implementation
//
// CREATED : 4/08/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AINodeSensing.h"
#include "AIButeMgr.h"
#include "AISenseRecorderGame.h"
#include "AIStimulusMgr.h"
#include "AI.h"
#include "ObjectRelationMgr.h"
#include "ParsedMsg.h"
#include "CharacterMgr.h"

LINKFROM_MODULE( AINodeSensing );

extern LTFLOAT s_fSenseUpdateBasis;

// ----------------------------------------------------------------------- //

BEGIN_CLASS(AINodeSensing)

	ADD_BOOLPROP_FLAG(Face,						LTTRUE,			0|PF_HIDDEN)
	ADD_STRINGPROP_FLAG(Alignment,				"None", 		0|PF_STATICLIST|PF_HIDDEN)

	ADD_REALPROP_FLAG(Radius,					128.0f,			0|PF_RADIUS)
	ADD_STRINGPROP_FLAG(AITemplate,				"Microphone", 	0|PF_STATICLIST|PF_HIDDEN)
	ADD_STRINGPROP_FLAG(LowCommand,				"",				0)
	ADD_REALPROP_FLAG(LowResetTime,				120.f,			0)
	ADD_LONGINTPROP_FLAG(MediumAlarmThreshold,	20,				0)
	ADD_STRINGPROP_FLAG(MediumCommand,			"",				0)
	ADD_REALPROP_FLAG(MediumResetTime,			120.f,			0)
	ADD_LONGINTPROP_FLAG(HighAlarmThreshold,	50,				0)
	ADD_STRINGPROP_FLAG(HighCommand,			"",				0)
	ADD_REALPROP_FLAG(HighResetTime,			120.f,			0)
	ADD_STRINGPROP_FLAG(CleanupCommand,			"",				0)

END_CLASS_DEFAULT_FLAGS_PLUGIN(AINodeSensing, AINode, NULL, NULL, 0, AINodeSensingPlugin)

CMDMGR_BEGIN_REGISTER_CLASS(AINodeSensing)
CMDMGR_END_REGISTER_CLASS(AINodeSensing, AINode)

// ----------------------------------------------------------------------- //

AINodeSensing::AINodeSensing()
{
	m_nTemplateID = -1;
	m_dwSenses = kSense_None;

	m_fSenseUpdateRate	= 0.0f;
	m_fNextSenseUpdate	= 0.0f;

	m_nAlarmLevel = 0;

	m_nHighAlarmThreshold = 50;						
	m_nMediumAlarmThreshold = 20;						

	for( uint8 iLevel = kLow; iLevel <= kHigh; ++iLevel )
	{
		m_fTimer[iLevel]	 = 0.f;
		m_fResetTime[iLevel] = 0.f;
		m_hstrCmd[iLevel]	 = LTNULL;
	}

	m_hstrCleanupCmd	= LTNULL;
	m_eCmdToRun = kNone;
	
	m_eSenseType = kSense_None;

	// Currently, sensing nodes do not support vision.
	m_rngSightGrid.Set( 0, 0 );

	m_pAISenseRecorder	= debug_new(CAISenseRecorderGame);
	m_pAISenseRecorder->Init(this);

	m_pRelationMgr = debug_new( CObjectRelationMgr );
}

AINodeSensing::~AINodeSensing()
{
	if( m_bNodeEnabled )
	{
		g_pAIStimulusMgr->RemoveSensingObject( this );
	}

	for( uint8 iLevel = kLow; iLevel <= kHigh; ++iLevel )
	{
		FREE_HSTRING( m_hstrCmd[iLevel] );
	}

	debug_delete(m_pAISenseRecorder);
	debug_delete(m_pRelationMgr);
}

// ----------------------------------------------------------------------- //

void AINodeSensing::ReadProp(ObjectCreateStruct* pocs)
{
	super::ReadProp(pocs);

	// Butes.

	if( g_pLTServer->GetPropGeneric( "AITemplate", &g_gp ) == LT_OK )
	{
		if ( g_gp.m_String[0] )
		{
			m_nTemplateID = g_pAIButeMgr->GetTemplateIDByName( g_gp.m_String );
		}
	}

	// Alarm Thresholds.

	if ( g_pLTServer->GetPropGeneric( "HighAlarmThreshold", &g_gp ) == LT_OK )
	{
		m_nHighAlarmThreshold = g_gp.m_Long;
	}

	if ( g_pLTServer->GetPropGeneric( "MediumAlarmThreshold", &g_gp ) == LT_OK )
	{
		m_nMediumAlarmThreshold = g_gp.m_Long;
	}

	// Reset times.

	if ( g_pLTServer->GetPropGeneric( "HighResetTime", &g_gp ) == LT_OK )
	{
		m_fResetTime[kHigh] = g_gp.m_Float;
	}

	if ( g_pLTServer->GetPropGeneric( "MediumResetTime", &g_gp ) == LT_OK )
	{
		m_fResetTime[kMedium] = g_gp.m_Float;
	}

	if ( g_pLTServer->GetPropGeneric( "LowResetTime", &g_gp ) == LT_OK )
	{
		m_fResetTime[kLow] = g_gp.m_Float;
	}

	// Commands.

	if ( g_pLTServer->GetPropGeneric( "HighCommand", &g_gp ) == LT_OK )
	{
		if ( g_gp.m_String[0] )
		{
			m_hstrCmd[kHigh] = g_pLTServer->CreateString( g_gp.m_String );
		}
	}

	if ( g_pLTServer->GetPropGeneric( "MediumCommand", &g_gp ) == LT_OK )
	{
		if ( g_gp.m_String[0] )
		{
			m_hstrCmd[kMedium] = g_pLTServer->CreateString( g_gp.m_String );
		}
	}

	if ( g_pLTServer->GetPropGeneric( "LowCommand", &g_gp ) == LT_OK )
	{
		if ( g_gp.m_String[0] )
		{
			m_hstrCmd[kLow] = g_pLTServer->CreateString( g_gp.m_String );
		}
	}

	if ( g_pLTServer->GetPropGeneric( "CleanupCommand", &g_gp ) == LT_OK )
	{
		if ( g_gp.m_String[0] )
		{
			m_hstrCleanupCmd = g_pLTServer->CreateString( g_gp.m_String );
		}
	}


	// If a template was found, set the sense flags.

	if( m_nTemplateID >= 0 )
	{
		AIBM_Template* pTemplate = g_pAIButeMgr->GetTemplate( m_nTemplateID );
		if( pTemplate )
		{
			// Set flags for stimuli that this node responds to.
			AISenseDistanceMap::iterator it;
			EnumAISenseType eSenseType = kSense_InvalidType;
			for( uint8 nSenseType = 0; nSenseType < kSense_Count; ++nSenseType )
			{
				eSenseType = (EnumAISenseType)(1 << nSenseType);
				it = pTemplate->mapSenseDistance.find( eSenseType );
				if( it != pTemplate->mapSenseDistance.end() )
				{
					m_dwSenses |= eSenseType;
					m_pAISenseRecorder->AddSense( eSenseType, it->second );
				}
			}

			// Set the Update Rate in the Attributes now, as this information
			// is not related to the characters alignment in any way.  Default
			// value should
			m_fSenseUpdateRate = pTemplate->fUpdateRate;

			// Set next sense update.
			m_fNextSenseUpdate = g_pLTServer->GetTime() + s_fSenseUpdateBasis;
			s_fSenseUpdateBasis += .02f;
			if ( s_fSenseUpdateBasis > 0.5f )
			{
				s_fSenseUpdateBasis = 0.0f;
			}
		}
	}
}

// ----------------------------------------------------------------------- //

void AINodeSensing::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_DWORD( m_nTemplateID );
	SAVE_DWORD( m_dwSenses );
	SAVE_FLOAT( m_fSenseUpdateRate );
	SAVE_TIME( m_fNextSenseUpdate );
	SAVE_DWORD( m_nAlarmLevel );
	SAVE_DWORD( m_nHighAlarmThreshold );
	SAVE_DWORD( m_nMediumAlarmThreshold );						

	for( uint8 iLevel = kLow; iLevel <= kHigh; ++iLevel )
	{
		SAVE_TIME( m_fTimer[iLevel] );
		SAVE_FLOAT( m_fResetTime[iLevel] );
		SAVE_HSTRING( m_hstrCmd[iLevel]	);
	}
	
	SAVE_HSTRING( m_hstrCleanupCmd );
	SAVE_DWORD( m_eCmdToRun );
	SAVE_DWORD( m_eSenseType );

	m_pAISenseRecorder->Save( pMsg );
	m_pRelationMgr->Save( pMsg );
}

void AINodeSensing::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_DWORD( m_nTemplateID );
	LOAD_DWORD( m_dwSenses );
	LOAD_FLOAT( m_fSenseUpdateRate );
	LOAD_TIME( m_fNextSenseUpdate );
	LOAD_DWORD( m_nAlarmLevel );
	LOAD_DWORD( m_nHighAlarmThreshold );
	LOAD_DWORD( m_nMediumAlarmThreshold );

	for( uint8 iLevel = kLow; iLevel <= kHigh; ++iLevel )
	{
		LOAD_TIME( m_fTimer[iLevel] );
		LOAD_FLOAT( m_fResetTime[iLevel] );
		LOAD_HSTRING( m_hstrCmd[iLevel]	);
	}
	
	LOAD_HSTRING( m_hstrCleanupCmd );
	LOAD_DWORD_CAST( m_eCmdToRun, EnumAlarmLevel );
	LOAD_DWORD_CAST( m_eSenseType, EnumAISenseType );

	m_pAISenseRecorder->Load( pMsg );
	m_pRelationMgr->Load( pMsg );

	if( m_bNodeEnabled )
	{
		g_pAIStimulusMgr->AddSensingObject( this );
	}

}

// ----------------------------------------------------------------------- //

LTBOOL AINodeSensing::IsAlert()
{
	return ( m_nAlarmLevel >= m_nMediumAlarmThreshold );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AINodeSensing::*ProcessingStimulus()
//
//	PURPOSE:	Handle stimuli
//
// ----------------------------------------------------------------------- //

LTBOOL AINodeSensing::GetDoneProcessingStimuli() const
{
	if( m_pAISenseRecorder )
	{
		return m_pAISenseRecorder->GetDoneProcessingStimuli();
	}

	return LTFALSE;
}

void AINodeSensing::SetDoneProcessingStimuli(LTBOOL bDone)
{
	if( m_pAISenseRecorder )
	{
		m_pAISenseRecorder->SetDoneProcessingStimuli( bDone );
	}
}

void AINodeSensing::ClearProcessedStimuli()
{
	if( m_pAISenseRecorder )
	{
		m_pAISenseRecorder->ClearProcessedStimuli();
	}
}

LTBOOL AINodeSensing::ProcessStimulus(CAIStimulusRecord* pRecord)
{
	if( m_pAISenseRecorder )
	{
		return m_pAISenseRecorder->ProcessStimulus( pRecord );
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AINodeSensing::*IntersectSegmentCount()
//
//	PURPOSE:	Handle IntersectSegment Counting.
//
// ----------------------------------------------------------------------- //

int	AINodeSensing::GetIntersectSegmentCount() const
{
	if( m_pAISenseRecorder )
	{
		return m_pAISenseRecorder->GetIntersectSegmentCount();
	}

	return 0;
}

void AINodeSensing::ClearIntersectSegmentCount()
{
	if( m_pAISenseRecorder )
	{
		m_pAISenseRecorder->ClearIntersectSegmentCount();
	}
}

void AINodeSensing::IncrementIntersectSegmentCount()
{
	if( m_pAISenseRecorder )
	{
		m_pAISenseRecorder->IncrementIntersectSegmentCount();
	}
}

// ----------------------------------------------------------------------- //

LTBOOL AINodeSensing::HandleSenseRecord(CAIStimulusRecord* pStimulusRecord, uint32 nCycle)
{
	if( m_pAISenseRecorder )
	{
		return m_pAISenseRecorder->UpdateSenseRecord( pStimulusRecord, nCycle );
	}

	return LTFALSE;
}

void AINodeSensing::HandleSenses(uint32 nCycle)
{
	if( m_pAISenseRecorder )
	{
		m_pAISenseRecorder->HandleSenses( nCycle );
	}
}

void AINodeSensing::HandleSenseTrigger(AISenseRecord* pSenseRecord)
{
	// Reset alarm level if any timers have expired.

	LTFLOAT fCurTime = g_pLTServer->GetTime();

	if( ( ( m_fTimer[kHigh] > 0.f ) && ( m_fTimer[kHigh] < fCurTime ) ) ||
		( ( m_fTimer[kMedium] > 0.f ) && ( m_fTimer[kMedium] < fCurTime ) ) || 
		( ( m_fTimer[kMedium] > 0.f ) && ( m_fTimer[kLow] < fCurTime ) ) )
	{
		ResetSensingNode();
	}


	// Increment alarm level.

	m_nAlarmLevel += pSenseRecord->nLastStimulusAlarmLevel;

	// Record what sense was triggered.

	m_eSenseType = pSenseRecord->eSenseType;

	//
	// Trigger commands.
	//

	// High alarm level.

	if( m_hstrCmd[kHigh] && ( m_nAlarmLevel >= m_nHighAlarmThreshold ) )
	{
		if( m_fTimer[kHigh] == 0.f )
		{
			m_fTimer[kHigh] = fCurTime + m_fResetTime[kHigh];
			m_fTimer[kMedium] = 0.f;
			m_fTimer[kLow] = 0.f;

			// Run command from update, because we are in the critical section 
			// of the StimulusMgr update loop.

			m_eCmdToRun = kHigh;
			SetNextUpdate(UPDATE_NEXT_FRAME);
		}
	}

	// Medium alarm level.

	else if( m_hstrCmd[kMedium] && ( m_nAlarmLevel >= m_nMediumAlarmThreshold ) )
	{
		if( ( m_fTimer[kMedium] == 0.f ) && ( m_fTimer[kHigh] == 0.f ) )
		{
			m_fTimer[kMedium] = fCurTime + m_fResetTime[kMedium];
			m_fTimer[kLow] = 0.f;

			// Run command from update, because we are in the critical section 
			// of the StimulusMgr update loop.

			m_eCmdToRun = kMedium;
			SetNextUpdate(UPDATE_NEXT_FRAME);		
		}
	}

	// Low alarm level.

	else if( m_hstrCmd[kLow] && ( m_fTimer[kLow] == 0.f ) && 
			( m_fTimer[kMedium] == 0.f ) && ( m_fTimer[kHigh] == 0.f ) )
	{
		m_fTimer[kLow] = fCurTime + m_fResetTime[kLow];
	
		// Run command from update, because we are in the critical section 
		// of the StimulusMgr update loop.

		m_eCmdToRun = kLow;
		SetNextUpdate(UPDATE_NEXT_FRAME);	
	}
}

LTFLOAT AINodeSensing::GetSenseDistance(EnumAISenseType eSenseType)
{
	return m_fRadius; 
}

// ----------------------------------------------------------------------- //

void AINodeSensing::ResetSensingNode()
{
	for( uint8 iLevel = kLow; iLevel <= kHigh; ++iLevel )
	{
		m_fTimer[iLevel] = 0.f;
	}

	m_nAlarmLevel = 0;

	if( m_hstrCleanupCmd )
	{
		const char *szCmd = g_pLTServer->GetStringData( m_hstrCleanupCmd );
		if( g_pCmdMgr->IsValidCmd( szCmd ) )
		{
			g_pCmdMgr->Process( szCmd, m_hObject, LTNULL );
		}
	}
}

// ----------------------------------------------------------------------- //

bool AINodeSensing::OnTrigger( HOBJECT hSender, const CParsedMsg &cMsg )
{
	static CParsedMsg::CToken s_cTok_Enable("ENABLE");
	static CParsedMsg::CToken s_cTok_Disable("DISABLE");

	if( cMsg.GetArg(0) == s_cTok_Enable )
	{
		if( !m_bNodeEnabled )
		{
			g_pAIStimulusMgr->AddSensingObject( this );
		}
	}
	else if( cMsg.GetArg(0) == s_cTok_Disable )
	{
		if( m_bNodeEnabled )
		{
			g_pAIStimulusMgr->RemoveSensingObject( this );
		}
	}

	return super::OnTrigger( hSender, cMsg );
}

// ----------------------------------------------------------------------- //

uint32 AINodeSensing::EngineMessageFn(uint32 messageID, void *pv, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_INITIALUPDATE:
		{
			if( (int)fData != INITIALUPDATE_SAVEGAME )
			{
				InitialUpdate();
			}
		}
		break;

		case MID_UPDATE:
		{
			// Run a command that was set as the result of a sense trigger.

			if( m_eCmdToRun != kNone )
			{
				SendSenseCommand();
			}
		}
		break;
	}

	return super::EngineMessageFn(messageID, pv, fData);
}

// ----------------------------------------------------------------------- //

void AINodeSensing::InitialUpdate()
{
	AIBM_Template* pTemplate = g_pAIButeMgr->GetTemplate( m_nTemplateID );
	if( pTemplate )
	{
		const char* const pszAlignment = pTemplate->szAlignment;
		AIASSERT( pszAlignment != NULL, m_hObject, "No Alignment" );

		m_pRelationMgr->Init( m_hObject, pszAlignment );
	}

	if( m_bNodeEnabled )
	{
		g_pAIStimulusMgr->AddSensingObject( this );
	}
}

// ----------------------------------------------------------------------- //

void AINodeSensing::SendSenseCommand()
{
	// Run a command as a result of a sense trigger.

	const char *szCmd = g_pLTServer->GetStringData( m_hstrCmd[m_eCmdToRun] );
	if( g_pCmdMgr->IsValidCmd( szCmd ) )
	{
		AITRACE( AIShowNodes, ( m_hObject, "Running Sense Command: %s", szCmd ) );
		g_pCmdMgr->Process( szCmd, m_hObject, LTNULL );
	}

	m_eCmdToRun = kNone;
}

// ----------------------------------------------------------------------- //

void AINodeSensing::CopySense(CAI* pAI)
{
	if( !m_pAISenseRecorder || !pAI)
	{
		return;
	}

	AISenseRecord *pSenseRecord = m_pAISenseRecorder->GetSense( m_eSenseType );
	if( !pSenseRecord )
	{
		return;
	}

	CAISenseRecorderAbstract* pSenseRecorder = pAI->GetSenseRecorder();
	if( pSenseRecorder )
	{
		// Copy the node's sense record, and force the AI to handle it.

		pSenseRecorder->CopySenseRecord( pSenseRecord );
		pSenseRecorder->HandleSenses( pSenseRecord->nCycle );
	}
}

// ----------------------------------------------------------------------- //

const RelationSet& AINodeSensing::GetSenseRelationSet() const
{
	if( m_pRelationMgr )
	{
		return m_pRelationMgr->GetRelationUser()->GetRelations();
	}

	static RelationSet rs;
	return rs;
}

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

static LTBOOL s_bSensingPluginInitted;
extern CAIButeMgr s_AIButeMgr;

LTRESULT AINodeSensingPlugin::PreHook_EditStringList(const char* szRezPath, const char* szPropName, char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength)
{
	if ( !s_bSensingPluginInitted )
	{
		char szFile[256];

		// Make sure the weaponmgr plugin is inited.
		m_WeaponMgrPlugin.PreHook_EditStringList(szRezPath, szPropName, aszStrings, pcStrings, cMaxStrings, cMaxStringLength);

		sprintf(szFile, "%s\\Attributes\\AIButes.txt", szRezPath);
		s_AIButeMgr.SetInRezFile(LTFALSE);
		s_AIButeMgr.Init(szFile);

		s_bSensingPluginInitted = LTTRUE;
	}

	if ( !_strcmpi("AITemplate", szPropName) )
	{
		// TODO: make sure we don't overflow cMaxStringLength or cMaxStrings
		uint32 cTemplates = s_AIButeMgr.GetNumTemplates();
		_ASSERT(cMaxStrings >= cTemplates);
		for ( uint32 iTemplate = 0 ; iTemplate < cTemplates ; iTemplate++ )
		{
			strcpy(aszStrings[(*pcStrings)++], s_AIButeMgr.GetTemplate(iTemplate)->szName);
		}

		return LT_OK;
	}

	// No one wants it

	return LT_UNSUPPORTED;
}
