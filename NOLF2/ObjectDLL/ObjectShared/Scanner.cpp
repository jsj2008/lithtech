// ----------------------------------------------------------------------- //
//
// MODULE  : CScanner.cpp
//
// PURPOSE : Implementation of CScanner class
//
// CREATED : 6/7/99
//
// (c) 1999-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "Scanner.h"
#include "iltserver.h"
#include "ServerUtilities.h"
#include "gameservershell.h"
#include "SoundMgr.h"
#include "PlayerObj.h"
#include "CharacterMgr.h"
#include "SurfaceFunctions.h"
#include "VolumeBrushTypes.h"
#include "ObjectMsgs.h"
#include "ObjectRelationMgr.h"
#include "RelationMgr.h"
#include "AIStimulusMgr.h"
#include "AISenseRecorderAbstract.h"

extern LTFLOAT s_fSenseUpdateBasis;

LINKFROM_MODULE( Scanner );

// ----------------------------------------------------------------------- //
//
//	CLASS:		CScanner
//
//	PURPOSE:	Scans for players
//
// ----------------------------------------------------------------------- //
#pragma force_active on
BEGIN_CLASS(CScanner)

	ADD_STRINGPROP_FLAG(Filename, "", PF_DIMS | PF_LOCALDIMS | PF_FILENAME | PF_MODEL)
	ADD_STRINGPROP_FLAG(Skin, "", PF_FILENAME)
	ADD_STRINGPROP_FLAG(DestroyedFilename, "", PF_FILENAME)
	ADD_STRINGPROP_FLAG(DestroyedSkin, "", PF_FILENAME)
	ADD_VECTORPROP_VAL_FLAG(Scale, 1.0f, 1.0f, 1.0f, PF_HIDDEN)
	ADD_VISIBLE_FLAG(1, PF_HIDDEN)
	ADD_SOLID_FLAG(1, PF_HIDDEN)
	ADD_GRAVITY_FLAG(0, PF_HIDDEN)
    ADD_BOOLPROP_FLAG(MoveToFloor, LTFALSE, PF_HIDDEN)
	ADD_REALPROP_FLAG(Translucency, 1.0f, PF_HIDDEN)

	ADD_REALPROP_FLAG(HitPoints, 1.0f, PF_GROUP(1))
	ADD_REALPROP_FLAG(MaxHitPoints, 10.0f, PF_GROUP(1))
	ADD_REALPROP_FLAG(Armor, 0.0f, PF_GROUP(1))
	ADD_REALPROP_FLAG(MaxArmor, 0.0f, PF_GROUP(1))

	ADD_REALPROP(FOV, 45.0)
	ADD_REALPROP_FLAG(VisualRange, 1000.0, PF_RADIUS)

	ADD_STRINGPROP_FLAG(SpotCommand, "", PF_NOTIFYCHANGE)
	
	ADD_BOOLPROP_FLAG(CanTransition, LTFALSE, PF_HIDDEN)

END_CLASS_DEFAULT_FLAGS_PLUGIN(CScanner, Prop, NULL, NULL, CF_HIDDEN, CScannerPlugin)
#pragma force_active off

//
// Register the class with the command mgr plugin and add any messages to the class
//

CMDMGR_BEGIN_REGISTER_CLASS( CScanner )
CMDMGR_END_REGISTER_CLASS( CScanner, Prop )

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScannerPlugin::PreHook_PropChanged()
//
//	PURPOSE:	Check properties that have changed...
//
// ----------------------------------------------------------------------- //

LTRESULT CScannerPlugin::PreHook_PropChanged( const char *szObjName,
											  const char *szPropName,
											  const int nPropType,
											  const GenericProp &gpPropValue,
											  ILTPreInterface *pInterface,
											  const char *szModifiers )
{
	// See if the propplugin will handle the change...

	if( CPropPlugin::PreHook_PropChanged( szObjName,
										  szPropName,
										  nPropType,
										  gpPropValue,
										  pInterface,
										  szModifiers ) == LT_OK )
	{
		return LT_OK;
	}
	
	// check to see if it is one of our values that has changed...

	if( !_stricmp( szPropName, "SpotCommand" ) )
	{
		if( m_CommandMgrPlugin.PreHook_PropChanged( szObjName,
													szPropName,
													nPropType,
													gpPropValue,
													pInterface,
													szModifiers ) == LT_OK )
		{
			return LT_OK;
		}
	}

	return LT_UNSUPPORTED;
}

// Filter functions

bool CScanner::DefaultFilterFn(HOBJECT hObj, void *pUserData)
{
    if (!hObj || !g_pLTServer) return false;

	SURFACE* pSurf = g_pSurfaceMgr->GetSurface(GetSurfaceType(hObj));
	if ( pSurf && pSurf->bCanSeeThrough )
	{
        return false;
	}

    HCLASS hBody = g_pLTServer->GetClass("Body");

    if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hObj), hBody))
	{
        return false;
	}

    static HCLASS hHitbox = g_pLTServer->GetClass("CCharacterHitBox");

    if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hObj), hHitbox))
	{
        return false;
	}

	return LiquidFilterFn(hObj, pUserData);
}

bool CScanner::BodyFilterFn(HOBJECT hObj, void *pUserData)
{
    if (!hObj || !g_pLTServer) return false;

	SURFACE* pSurf = g_pSurfaceMgr->GetSurface(GetSurfaceType(hObj));
	if ( pSurf && pSurf->bCanSeeThrough )
	{
        return false;
	}

    static HCLASS hHitbox = g_pLTServer->GetClass("CCharacterHitBox");

    if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hObj), hHitbox))
	{
        return false;
	}

	return LiquidFilterFn(hObj, pUserData);
}

bool CScanner::SeeThroughPolyFilterFn(HPOLY hPoly, void *pUserData)
{
    if ( INVALID_HPOLY == hPoly ) return false;

	SURFACE* pSurf = g_pSurfaceMgr->GetSurface(GetSurfaceType(hPoly));
	if ( !pSurf )
	{
		g_pLTServer->CPrint("Warning, HPOLY had no associated surface!");
		return false;
	}

    if ( pSurf->bCanSeeThrough )
	{
		return false;
	}

    return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScanner::CScanner()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

CScanner::CScanner() : Prop()
{
	//we don't want the prop being able to deactivate us sine we will control
	//that ourselves based upon the state of the camera
	m_bCanDeactivate	= false;

	m_fFOV				= 0.0f;
	m_fVisualRange		= 1000.0f;
	m_fVisualRangeSqr	= 1000.0f * 1000.0f;

	m_vInitialPitchYawRoll.Init(0, 0, 0);

    m_hstrDestroyedFilename = LTNULL;
    m_hstrDestroyedSkin = LTNULL;

    m_hstrSpotCommand   = LTNULL;
    
    m_hLastDetectedEnemy = LTNULL;
	m_vLastDetectedDeathPos.Init(0, 0, 0);

	// We do not want our object removed when we die

    m_damage.m_bRemoveOnDeath = LTFALSE;

    m_bCanProcessDetection = LTTRUE;

	m_pObjectRelationMgr = debug_new(CObjectRelationMgr);
	m_pObjectRelationMgr->ClearRelationSystem();

	m_hStimulus = LTNULL;
	m_eStimulusID = kStimID_Unset;
	m_eStimulusSenseType = kSense_None;
	m_fStimulusTime = 0.f;

	m_fLastResetTime = 0.f;

	m_bSensing = LTFALSE;

	// IAISensing members.

	m_dwSenses = kSense_SeeEnemy | kSense_SeeAllyDeath;
	m_fSenseUpdateRate	= 0.1f;

	// Set next sense update.
	m_fNextSenseUpdate = g_pLTServer->GetTime() + s_fSenseUpdateBasis;
	s_fSenseUpdateBasis += .02f;
	if( s_fSenseUpdateBasis > 0.5f )
	{
		s_fSenseUpdateBasis = 0.0f;
	}

	m_bDoneProcessingStimuli = LTTRUE;

	m_cIntersectSegmentCount = 0;

	// Currently, scanners do not support sight grids.
	m_rngSightGrid.Set( 0, 0 );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScanner::~CScanner()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

CScanner::~CScanner()
{
	FREE_HSTRING(m_hstrDestroyedFilename);
	FREE_HSTRING(m_hstrDestroyedSkin);
	FREE_HSTRING(m_hstrSpotCommand);

	debug_delete( m_pObjectRelationMgr );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CScanner::GetRelation*()
//              
//	PURPOSE:	Relation Accessor methods
//              
//----------------------------------------------------------------------------
const RelationData& CScanner::GetRelationData()
{
	return m_pObjectRelationMgr->GetData();
}
const RelationSet& CScanner::GetRelationSet()
{
	return m_pObjectRelationMgr->GetRelationUser()->GetRelations();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScanner::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 CScanner::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch (messageID)
	{
		case MID_PRECREATE:
		{
			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				ObjectCreateStruct* pStruct = (ObjectCreateStruct*)pData;
				ReadProp(pStruct);

				// Don't stand on the player...

				pStruct->m_Flags |= FLAG_DONTFOLLOWSTANDING;
			}
		}
		break;

		case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
			}

			InitRelationInformation();
		}
		break;

		case MID_SAVEOBJECT:
		{
			Save((ILTMessage_Write*)pData);
		}
		break;

		case MID_LOADOBJECT:
		{
			Load((ILTMessage_Read*)pData);
		}
		break;

		case MID_ACTIVATING:
		{
			EnableSensing( LTTRUE );
		}
		break;

		case MID_DEACTIVATING:
		{
			EnableSensing( LTFALSE );
		}
		break;

		default : break;
	}

	return Prop::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScanner::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

LTBOOL CScanner::ReadProp(ObjectCreateStruct *pInfo)
{
    if (!pInfo) return LTFALSE;

	GenericProp genProp;

    if (g_pLTServer->GetPropGeneric("FOV", &genProp) == LT_OK)
	{
		m_fFOV = genProp.m_Float;

		// Change FOV into the value we're going to compare in the dot product
		// for fov

		m_fFOV = (float)sin(DEG2RAD(90.0f-m_fFOV/2.0f));
	}

    if (g_pLTServer->GetPropGeneric("VisualRange", &genProp) == LT_OK)
	{
		m_fVisualRange = genProp.m_Float;

		// All values compared against VisualRange will be squared, so square
		// it too

		m_fVisualRangeSqr = m_fVisualRange*m_fVisualRange;
	}

    if (g_pLTServer->GetPropGeneric("DestroyedFilename", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
            m_hstrDestroyedFilename = g_pLTServer->CreateString(genProp.m_String);
		}
	}

    if (g_pLTServer->GetPropGeneric("DestroyedSkin", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
            m_hstrDestroyedSkin = g_pLTServer->CreateString(genProp.m_String);
		}
	}

    if (g_pLTServer->GetPropGeneric("SpotCommand", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
            m_hstrSpotCommand = g_pLTServer->CreateString(genProp.m_String);
		}
	}

    LTVector vAngles;
    if (g_pLTServer->GetPropRotationEuler("Rotation", &vAngles) == LT_OK)
	{
		m_vInitialPitchYawRoll = vAngles;
	}

	g_pLTServer->GetObjectPos( m_hObject, &m_vScannerPos );

    return LTTRUE;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CScanner::InitRelationInformation()
//              
//	PURPOSE:	Initialize the scanner information.  This function changes
//				per implementation of the scanner class.
//              
//----------------------------------------------------------------------------
void CScanner::InitRelationInformation(void)
{
	// Currently, set the relation data and relation set based on this
	// class name as key.  Change this key as desired if we wish to instead
	// have the relation information set based on mapper value or any other
	// data source.

	char szClassName[128];
	g_pLTServer->GetClassName( g_pLTServer->GetObjectClass(m_hObject), szClassName, sizeof(szClassName));
	m_pObjectRelationMgr->Init( m_hObject, szClassName );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CScanner::EnableSensing()
//              
//	PURPOSE:	Turn on/off sensing.
//              
//----------------------------------------------------------------------------

void CScanner::EnableSensing(LTBOOL bEnable)
{
	if( bEnable != m_bSensing )
	{
		if( bEnable )
		{
			g_pAIStimulusMgr->AddSensingObject( this );
			m_bSensing = LTTRUE;
		}
		else {
			g_pAIStimulusMgr->RemoveSensingObject( this );
			m_bSensing = LTFALSE;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScanner::*ProcessingStimuli()
//
//	PURPOSE:	Process stimuli.
//
// ----------------------------------------------------------------------- //

LTBOOL CScanner::GetDoneProcessingStimuli() const
{
	return m_bDoneProcessingStimuli;
}

void CScanner::SetDoneProcessingStimuli(LTBOOL bDone)
{
	m_bDoneProcessingStimuli = bDone;
}

void CScanner::ClearProcessedStimuli()
{
	m_mapProcessedStimuli.clear();
}

LTBOOL CScanner::ProcessStimulus(CAIStimulusRecord* pRecord)
{
	if( m_mapProcessedStimuli.find( pRecord->m_eStimulusID ) == m_mapProcessedStimuli.end() )
	{
		m_mapProcessedStimuli.insert( AI_PROCESSED_STIMULI_MAP::value_type( pRecord->m_eStimulusID, 0 ) );
		return LTTRUE;
	}
		
	return LTFALSE;	
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScanner::*IntersectSegmentCount()
//
//	PURPOSE:	Handle IntersectSegment Counting.
//
// ----------------------------------------------------------------------- //

int	CScanner::GetIntersectSegmentCount() const
{
	return m_cIntersectSegmentCount;
}

void CScanner::ClearIntersectSegmentCount()
{
	m_cIntersectSegmentCount = 0;
}

void CScanner::IncrementIntersectSegmentCount()
{
	++m_cIntersectSegmentCount;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CScanner::HandleSenseRecord()
//              
//	PURPOSE:	Check if an objetc of interest has come into view.
//              
//----------------------------------------------------------------------------

LTBOOL CScanner::HandleSenseRecord(CAIStimulusRecord* pStimulusRecord, uint32 nCycle)
{
	// NULL stimulus.

	if( !pStimulusRecord )
	{
		AIASSERT( pStimulusRecord, m_hObject, "CScanner::HandleSenseRecord: NULL stimulus record." );
		return LTFALSE;
	}

	// Ignore stimulus older than the last reset time.
	// This prevents scanners from seeing bodies that already existed 
	// last time the scanner detected something. Scanners should only detect
	// stimulus that is new since that last time the detection command was run.

	if( pStimulusRecord->m_fTimeStamp <= m_fLastResetTime )
	{
		return LTFALSE;
	}

	// Do visibility test.

	LTBOOL bSeeObject = LTFALSE;
	if( IsBody( pStimulusRecord->m_hStimulusSource ) )
	{
		bSeeObject = CanSeeObject( BodyFilterFn, pStimulusRecord->m_hStimulusSource );
	}
	else {
		bSeeObject = CanSeeObject( DefaultFilterFn, pStimulusRecord->m_hStimulusSource );
	}

	// Record what was seen.

	if( bSeeObject )
	{
		m_hStimulus = pStimulusRecord->m_hStimulusSource;
		m_eStimulusID = pStimulusRecord->m_eStimulusID;
		m_eStimulusSenseType = pStimulusRecord->m_pAIBM_Stimulus->eSenseType;
		m_fStimulusTime = g_pLTServer->GetTime();
		return LTTRUE;
	}

	// Nothing was seen.

	m_hStimulus = LTNULL;
	return LTFALSE;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CScanner::GetSenseDistance()
//              
//	PURPOSE:	Get the distance the scanner can see.
//              
//----------------------------------------------------------------------------

LTFLOAT CScanner::GetSenseDistance(EnumAISenseType eSenseType)
{
	return m_fVisualRange; 
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CScanner::GetSenseRelationSet()
//              
//	PURPOSE:	Get the relation set for the scanner.
//              
//----------------------------------------------------------------------------

const RelationSet& CScanner::GetSenseRelationSet() const
{
	if( m_pObjectRelationMgr )
	{
		return m_pObjectRelationMgr->GetRelationUser()->GetRelations();
	}

	static RelationSet rs;
	return rs;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScanner::UpdateDetect()
//
//	PURPOSE:	Checks to see if we can see anything
//
// ----------------------------------------------------------------------- //

CScanner::DetectState CScanner::UpdateDetect()
{
	g_pLTServer->GetObjectPos( m_hObject, &m_vScannerPos );

	// Update the relations, flushing those dynamic ones which are expired,
	// and reinforcing any for which are last enemy etc.
	UpdateRelations();

	// If it has been too long since our last stimulation, clear stimulus.

	if( m_fStimulusTime + m_fSenseUpdateRate < g_pLTServer->GetTime() )
	{
		m_hStimulus = LTNULL;
	}

	if ( m_hStimulus )
	{
		// Set the focus timer if it hasn't been set...(i.e., the
		// first time we see a "situation")...

        LTBOOL bFocus = !!(GetFocusTime() > 0.0f);

		if (bFocus && !m_FocusTimer.GetDuration())
		{
			m_FocusTimer.Start(GetFocusTime());
		}
		else if (!bFocus || m_FocusTimer.Stopped())
		{
			// Only process detection once (unless it is reset
			// someplace else...)...

			if (m_bCanProcessDetection)
			{
				if ( m_eStimulusSenseType == kSense_SeeEnemy )
				{
					SetLastDetectedEnemy( m_hStimulus );
				}
				else if ( m_eStimulusSenseType == kSense_SeeAllyDeath )
				{
					LTVector vPos;
					g_pLTServer->GetObjectPos( m_hStimulus, &vPos );
					SetLastDetectedDeathPos( vPos );
				}

				// Process the detection command...
				
				if( m_hstrSpotCommand )
				{
					const char *pCmd = g_pLTServer->GetStringData( m_hstrSpotCommand );

					if( g_pCmdMgr->IsValidCmd( pCmd ) )
					{
						g_pCmdMgr->Process( pCmd, m_hObject, m_hObject );
					}
				}
				
                SetCanProcessDetection( LTFALSE );
			}

			return DS_DETECTED;
		}

		// Clear stimulus for bodies while focusing, so that
		// scanner will continue to respond to same stimulus
		// until body is detected, or body is removed.
		// Live enemies create a new stimulus every update, so
		// clearing is not necessary.

		if( m_eStimulusSenseType == kSense_SeeAllyDeath )
		{
			g_pAIStimulusMgr->ClearResponder( m_eStimulusID, m_hObject );
		}

		return DS_FOCUSING;
	}
	else
	{
		// If the focus timer has stopped (i.e., we tried to focus on
		// a character but he moved before we detected him, stop the
		// timer (which will reset the duration)...

		if (m_FocusTimer.Stopped())
		{
			m_FocusTimer.Stop();
		}
		else
		{
			return DS_FOCUSING;
		}
	}

	return DS_CLEAR;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CScanner::UpdateRelations()
//              
//	PURPOSE:	Calls the Hook, which can be used to maintain relations
//				through m_pObjectRelationMgr->ResetRelationTime calls with an
//				object the scanner has established a dynamic relationship with
//              
//----------------------------------------------------------------------------
void CScanner::UpdateRelations()
{
	PreUpdateRelationsHook();
	m_pObjectRelationMgr->Update( true );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CScanner::PreUpdateRelationsHook()
//              
//	PURPOSE:	Refresh the relation with the lastdetectedenemy.  If the
//				CScanner has any dynamic relationships with the
//				m_hLastDetectedEnemy, this will keep them around.
//              
//----------------------------------------------------------------------------
void CScanner::PreUpdateRelationsHook()
{
	if ( m_hLastDetectedEnemy )
	{
		CObjectRelationMgr* pORM;
		pORM = CRelationMgr::GetGlobalRelationMgr()->GetObjectRelationMgr(m_hLastDetectedEnemy);
		if (!pORM)
			return;

		m_pObjectRelationMgr->ResetRelationTime( pORM );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScanner::SetDestroyedModel()
//
//	PURPOSE:	Set our model to the destroyed version
//
// ----------------------------------------------------------------------- //

void CScanner::SetDestroyedModel()
{
	if (!m_hstrDestroyedFilename || !m_hstrDestroyedSkin) return;

    const char* szDestroyedFilename = g_pLTServer->GetStringData(m_hstrDestroyedFilename);
    const char* szDestroyedSkin     = g_pLTServer->GetStringData(m_hstrDestroyedSkin);

    LTRESULT hResult = SetObjectFilenames(m_hObject, szDestroyedFilename, szDestroyedSkin);
    _ASSERT(hResult == LT_OK);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScanner::SetCanProcessDetection()
//
//	PURPOSE:	Toggle processing
//
// ----------------------------------------------------------------------- //

void CScanner::SetCanProcessDetection(LTBOOL bCanProcess)
{
	// Record last reset time, so earlier stimulus can be ignored.

	if( bCanProcess && ( !m_bCanProcessDetection ) )
	{
		m_fLastResetTime = g_pLTServer->GetTime();
	}

	m_bCanProcessDetection = bCanProcess;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScanner::SetLastDetectedEnemy()
//
//	PURPOSE:	Set our last detected enemy
//
// ----------------------------------------------------------------------- //

void CScanner::SetLastDetectedEnemy(HOBJECT hObj)
{
	m_hLastDetectedEnemy = hObj;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScanner::CanSeeObject()
//
//	PURPOSE:	Is this object visible to us?
//
// ----------------------------------------------------------------------- //

LTBOOL CScanner::CanSeeObject(ObjectFilterFn ofn, HOBJECT hObject)
{
	_ASSERT(hObject);
    if (!hObject) return LTFALSE;

    LTVector vPos;
	g_pLTServer->GetObjectPos(hObject, &vPos);

    LTVector vDir;
	vDir = vPos - GetScanPosition();

	if (VEC_MAGSQR(vDir) >= m_fVisualRangeSqr)
	{
        return LTFALSE;
	}

	vDir.Normalize();

    LTRotation rRot = GetScanRotation();

    LTVector vUp, vRight, vForward;
	vUp = rRot.Up();
	vRight = rRot.Right();
	vForward = rRot.Forward();

    LTFLOAT fDp = vDir.Dot(vForward);

	if (fDp < m_fFOV)
	{
        return LTFALSE;
	}

	// See if we can see the position in question

	IntersectQuery IQuery;
	IntersectInfo IInfo;

	VEC_COPY(IQuery.m_From, GetScanPosition());
	VEC_COPY(IQuery.m_To, vPos);
	IQuery.m_Flags	  = INTERSECT_OBJECTS | IGNORE_NONSOLID | INTERSECT_HPOLY;
	IQuery.m_FilterFn = ofn;
	IQuery.m_PolyFilterFn = SeeThroughPolyFilterFn;

    if (g_pLTServer->IntersectSegment(&IQuery, &IInfo))
	{
		if (IInfo.m_hObject == hObject)
		{
            return LTTRUE;
		}
	}

    return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScanner::CanSeePos()
//
//	PURPOSE:	Is this position visible to us?
//
// ----------------------------------------------------------------------- //

LTBOOL CScanner::CanSeePos(ObjectFilterFn ofn, const LTVector& vPos)
{
    LTVector vDir;
	vDir = vPos - GetScanPosition();

	if (vDir.MagSqr() >= m_fVisualRangeSqr)
	{
        return LTFALSE;
	}

	vDir.Normalize();

    LTRotation rRot = GetScanRotation();

    LTVector vUp, vRight, vForward;
	vUp = rRot.Up();
	vRight = rRot.Right();
	vForward = rRot.Forward();

    LTFLOAT fDp = vDir.Dot(vForward);

	if (fDp < m_fFOV)
	{
        return LTFALSE;
	}

	// See if we can see the position in question

	IntersectQuery IQuery;
	IntersectInfo IInfo;

	VEC_COPY(IQuery.m_From, GetScanPosition());
	VEC_COPY(IQuery.m_To, vPos);
	IQuery.m_Flags	  = INTERSECT_OBJECTS | IGNORE_NONSOLID | INTERSECT_HPOLY;
	IQuery.m_FilterFn = ofn;
	IQuery.m_PolyFilterFn = SeeThroughPolyFilterFn;

    if (!g_pLTServer->IntersectSegment(&IQuery, &IInfo))
	{
        return LTTRUE;
	}

    return LTFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScanner::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CScanner::Save(ILTMessage_Write *pMsg)
{
	if (!pMsg) return;

	SAVE_BOOL(m_bCanProcessDetection);

	SAVE_FLOAT(m_fFOV);
    SAVE_FLOAT(m_fVisualRange);
    SAVE_FLOAT(m_fVisualRangeSqr);

	SAVE_VECTOR(m_vInitialPitchYawRoll);
	SAVE_VECTOR(m_vScannerPos);

	SAVE_HSTRING(m_hstrDestroyedFilename);
    SAVE_HSTRING(m_hstrDestroyedSkin);
    SAVE_HSTRING(m_hstrSpotCommand);
    
	SAVE_HOBJECT( m_hLastDetectedEnemy );
    SAVE_VECTOR( m_vLastDetectedDeathPos );

	m_FocusTimer.Save(pMsg);
	m_pObjectRelationMgr->Save(pMsg);

	SAVE_HOBJECT( m_hStimulus );
	SAVE_DWORD(	m_eStimulusID );
	SAVE_DWORD(	m_eStimulusSenseType );
	SAVE_TIME( m_fStimulusTime );
	SAVE_BOOL( m_bSensing );

	SAVE_TIME( m_fLastResetTime );

	// Save IAISensing members.

	SAVE_DWORD( m_dwSenses );
	SAVE_FLOAT( m_fSenseUpdateRate );
	SAVE_TIME( m_fNextSenseUpdate );

	SAVE_BOOL( m_bDoneProcessingStimuli );

	SAVE_DWORD( m_mapProcessedStimuli.size() );
	AI_PROCESSED_STIMULI_MAP::iterator psit;
	for( psit = m_mapProcessedStimuli.begin(); psit != m_mapProcessedStimuli.end(); ++psit )
	{
		SAVE_DWORD( psit->first );
	}

	SAVE_DWORD( m_cIntersectSegmentCount );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScanner::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CScanner::Load(ILTMessage_Read *pMsg)
{
	if (!pMsg) return;

    LOAD_BOOL(m_bCanProcessDetection);

	LOAD_FLOAT(m_fFOV);
    LOAD_FLOAT(m_fVisualRange);
    LOAD_FLOAT(m_fVisualRangeSqr);

	LOAD_VECTOR(m_vInitialPitchYawRoll);
	LOAD_VECTOR(m_vScannerPos);

	LOAD_HSTRING(m_hstrDestroyedFilename);
    LOAD_HSTRING(m_hstrDestroyedSkin);
    LOAD_HSTRING(m_hstrSpotCommand);
    
	LOAD_HOBJECT( m_hLastDetectedEnemy );
    LOAD_VECTOR( m_vLastDetectedDeathPos );

	m_FocusTimer.Load(pMsg);
	m_pObjectRelationMgr->Load(pMsg);

	LOAD_HOBJECT( m_hStimulus );
	LOAD_DWORD_CAST( m_eStimulusID, EnumAIStimulusID );
	LOAD_DWORD_CAST( m_eStimulusSenseType, EnumAISenseType );
	LOAD_TIME( m_fStimulusTime );
	LOAD_BOOL( m_bSensing );

	LOAD_TIME( m_fLastResetTime );

	// Load IAISensing members.

	LOAD_DWORD( m_dwSenses );
	LOAD_FLOAT( m_fSenseUpdateRate );
	LOAD_TIME( m_fNextSenseUpdate );

	LOAD_BOOL( m_bDoneProcessingStimuli );

	EnumAIStimulusID eStimID;
	uint32 cStimulus;
	LOAD_DWORD( cStimulus );
	for( uint32 iStimulus=0; iStimulus < cStimulus; ++iStimulus )
	{
		LOAD_DWORD_CAST( eStimID, EnumAIStimulusID );
		m_mapProcessedStimuli.insert( AI_PROCESSED_STIMULI_MAP::value_type( eStimID, 0 ) );
	}

	LOAD_DWORD( m_cIntersectSegmentCount );
}


