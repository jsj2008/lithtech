// ----------------------------------------------------------------------- //
//
// MODULE  : Alarm.cpp
//
// PURPOSE : Implementation of the alarm
//
// CREATED : 3/27/99
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "Alarm.h"
#include "iltserver.h"
#include "ServerUtilities.h"
#include "SoundMgr.h"
#include "PlayerObj.h"
#include "CharacterMgr.h"
#include "ObjectMsgs.h"
#include "ParsedMsg.h"
#include "AITarget.h"
#include "AI.h"
#include "AIStimulusMgr.h"
#include "AIRegion.h"
#include "AIUtils.h"
#include "AIVolumeMgr.h"
#include "AIVolume.h"

LINKFROM_MODULE( Alarm );

extern CAIStimulusMgr* g_pAIStimulusMgr;

#define REGION_STRING_LEN	256

// Statics

static char *s_szActivate	= "ACTIVATE";
static char *s_szLock		= "LOCK";
static char *s_szUnlock		= "UNLOCK";

// ----------------------------------------------------------------------- //
//
//	CLASS:		Alarm
//
//	PURPOSE:	An alarm object
//
// ----------------------------------------------------------------------- //

#pragma force_active on
BEGIN_CLASS(Alarm)

	ADD_STRINGPROP_FLAG(Filename, "Props\\Models\\Alarm.ltb", PF_DIMS | PF_LOCALDIMS | PF_FILENAME | PF_MODEL)
	ADD_STRINGPROP_FLAG(Skin, "Props\\Skins\\Alarm.dtx", PF_FILENAME)
	ADD_VECTORPROP_VAL_FLAG(Scale, 1.0f, 1.0f, 1.0f, PF_HIDDEN)
	ADD_VISIBLE_FLAG(0, 0)
	ADD_SOLID_FLAG(0, PF_HIDDEN)
	ADD_GRAVITY_FLAG(0, PF_HIDDEN)
	ADD_SHADOW_FLAG(0, 0)
    ADD_BOOLPROP_FLAG(MoveToFloor, LTFALSE, PF_HIDDEN)
	ADD_REALPROP_FLAG(Alpha, 1.0f, PF_HIDDEN)

    ADD_BOOLPROP(PlayerUsable, LTFALSE)
    ADD_STRINGPROP_FLAG(PlayerActivateCommand, "", 0)
	ADD_REALPROP_FLAG(SoundTime, 10.0f, 0)

	ADD_STRINGPROP_FLAG(AlertRegions, "", 0)
	ADD_STRINGPROP_FLAG(RespondRegions, "", 0)
	ADD_STRINGPROP_FLAG(SearchRegions, "", 0)

	ADD_BOOLPROP_FLAG(CanTransition, LTFALSE, PF_HIDDEN)
	
END_CLASS_DEFAULT_FLAGS_PLUGIN(Alarm, Prop, NULL, NULL, 0, CAlarmPlugin)
#pragma force_active off

//
// Register the class with the command mgr plugin and add any messages to the class
//

CMDMGR_BEGIN_REGISTER_CLASS( Alarm )

	CMDMGR_ADD_MSG( LOCK, 1, NULL, "LOCK" )
	CMDMGR_ADD_MSG( UNLOCK, 1, NULL, "UNLOCK" )

CMDMGR_END_REGISTER_CLASS( Alarm, Prop )

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CAlarmPlugin::PreHook_PropChanged
//
//  PURPOSE:	Check our command strings
//
// ----------------------------------------------------------------------- //

LTRESULT CAlarmPlugin::PreHook_PropChanged( const char *szObjName,
											const char *szPropName, 
											const int  nPropType, 
											const GenericProp &gpPropValue,
											ILTPreInterface *pInterface,
											const char *szModifiers )
{
	// First send it down to our propplugin...

	if( CPropPlugin::PreHook_PropChanged( szObjName,
										  szPropName,
										  nPropType,
										  gpPropValue,
										  pInterface,
										  szModifiers ) == LT_OK )
	{
		return LT_OK;
	}
	
	// Only our command is marked for change notification so just send it to the CommandMgr..

	if( m_CommandMgrPlugin.PreHook_PropChanged( szObjName, 
												szPropName, 
												nPropType, 
												gpPropValue,
												pInterface,
												szModifiers ) == LT_OK )
	{
		return LT_OK;
	}

	return LT_UNSUPPORTED;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Alarm::Alarm()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

Alarm::Alarm() : Prop ()
{
	m_eState = eStateOff;
    m_bPlayerUsable = LTFALSE;
	m_bLocked = LTFALSE;

	m_fAlarmSoundTime = 0.f;

	m_hstrPlayerActivateCommand = LTNULL;

	m_hstrAlertRegions = LTNULL;
	m_hstrRespondRegions = LTNULL;
	m_hstrSearchRegions = LTNULL;

	m_vRegionsGroupExtentsMin = LTVector( 0.f, 0.f, 0.f);
	m_vRegionsGroupExtentsMax = LTVector( 0.f, 0.f, 0.f);
	m_fRegionsGroupRadius = 0.f;

    m_damage.m_bRemoveOnDeath = LTFALSE;

	m_pDebrisOverride = "Metal small";
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Alarm::~Alarm()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

Alarm::~Alarm()
{
    if( !g_pLTServer ) return;

	m_eState = eStateOff;
    m_bPlayerUsable = LTFALSE;

	FREE_HSTRING(m_hstrPlayerActivateCommand);

	FREE_HSTRING(m_hstrAlertRegions);
	FREE_HSTRING(m_hstrRespondRegions);
	FREE_HSTRING(m_hstrSearchRegions);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Alarm::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 Alarm::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				ReadProp((ObjectCreateStruct*)pData);
			}

			PostPropRead((ObjectCreateStruct*)pData);
			break;
		}

		case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
				InitialUpdate();
			}

			break;
		}

		case MID_ALLOBJECTSCREATED:
		{
			CreateRegionLists();
			break;
		}
		
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

		default : break;
	}

	return Prop::EngineMessageFn(messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Alarm::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

LTBOOL Alarm::ReadProp(ObjectCreateStruct *pInfo)
{
    if (!pInfo) return LTFALSE;

	GenericProp genProp;

    if ( g_pLTServer->GetPropGeneric("PlayerUsable", &genProp ) == LT_OK )
		m_bPlayerUsable = genProp.m_Bool;

    if ( g_pLTServer->GetPropGeneric("SoundTime", &genProp ) == LT_OK )
		m_fAlarmSoundTime = genProp.m_Float;

	if ( g_pLTServer->GetPropGeneric( "PlayerActivateCommand", &genProp ) == LT_OK )
	{
		if ( genProp.m_String[0] )
		{
			m_hstrPlayerActivateCommand = g_pLTServer->CreateString( genProp.m_String );
		}
	}

    if ( g_pLTServer->GetPropGeneric("AlertRegions", &genProp ) == LT_OK )
	{
		if ( genProp.m_String[0] )
		{
			m_hstrAlertRegions = g_pLTServer->CreateString( genProp.m_String );
		}
	}

    if ( g_pLTServer->GetPropGeneric("RespondRegions", &genProp ) == LT_OK )
	{
		if ( genProp.m_String[0] )
		{
			m_hstrRespondRegions = g_pLTServer->CreateString( genProp.m_String );
		}
	}

    if ( g_pLTServer->GetPropGeneric("SearchRegions", &genProp ) == LT_OK )
	{
		if ( genProp.m_String[0] )
		{
			m_hstrSearchRegions = g_pLTServer->CreateString( genProp.m_String );
		}
	}

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Alarm::PostPropRead()
//
//	PURPOSE:	Update Properties
//
// ----------------------------------------------------------------------- //

void Alarm::PostPropRead(ObjectCreateStruct *pStruct)
{
	if ( !pStruct ) return;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Alarm::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

uint32 Alarm::ObjectMessageFn(HOBJECT hSender, ILTMessage_Read *pMsg)
{
    if (!g_pLTServer) return 0;

	pMsg->SeekTo(0);
	uint32 messageID = pMsg->Readuint32();

	switch(messageID)
	{
		case MID_DAMAGE:
		{
			// Let our damage aggregate process the message first...

            uint32 dwRet = Prop::ObjectMessageFn(hSender, pMsg);

			// Check to see if we have been destroyed

			if ( m_damage.IsDead() )
			{
				m_eState = eStateDestroyed;
			}

			// TODO: Check to see if we have been disbled

			return dwRet;
		}

		default : break;
	}

	return Prop::ObjectMessageFn(hSender, pMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Alarm::OnTrigger()
//
//	PURPOSE:	Handle a trigger message
//
// ----------------------------------------------------------------------- //

bool Alarm::OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg)
{
	static CParsedMsg::CToken s_cTok_Activate(s_szActivate);
	static CParsedMsg::CToken s_cTok_Lock(s_szLock);
	static CParsedMsg::CToken s_cTok_Unlock(s_szUnlock);

	if ( cMsg.GetArg(0) == s_cTok_Activate )
	{
		// If the alarm is activated from a command, the sender is
		// NULL (dammit!).  So treat it as player-activated.
		// If the alarm is activated by something other than AI
		// (e.g. a command object) consider it player activated.

		if( !IsAI( hSender ) )
		{
			CPlayerObj *pPlayer = g_pCharacterMgr->FindPlayer();
			hSender = pPlayer->m_hObject;
		}

		HOBJECT hStimulus = hSender;

		if( IsPlayer(hSender) )
		{
			// Run the alarm's player-activate command.

			if( m_hstrPlayerActivateCommand )
			{
				const char *szCmd = g_pLTServer->GetStringData( m_hstrPlayerActivateCommand );

				if( g_pCmdMgr->IsValidCmd( szCmd ) )
				{
					g_pCmdMgr->Process( szCmd, m_hObject, m_hObject );
				}
			}
		}
		else
		{
			// The stimulus is the target of the AI who activated the alarm.

			CAI* pAI = (CAI*)g_pLTServer->HandleToObject(hSender);
			hStimulus = pAI->GetTarget()->GetObject();
		}

		// Ensure that lists of Alert and Respond regions are set up.

		if( ( m_fRegionsGroupRadius == 0.f ) && 
			( ( m_lstAlertRegions.size() > 0 ) || 
			( m_lstRespondRegions.size() > 0 ) || 
			( m_lstSearchRegions.size() > 0 ) ) ) 
		{
			CalculateRegionsGroupRadius();
		}

		// Place an alarm stimulus.
		// The stimulus position and radius are set to values that encompass
		// all of the regions affected by the alarm.

		g_pAIStimulusMgr->RegisterStimulus(kStim_EnemyAlarmSound, hStimulus, m_hObject, m_vRegionsGroupCenter, m_fRegionsGroupRadius, m_fAlarmSoundTime);
		AITRACE( AIShowAlarms, ( m_hObject, "Triggering alarm" ) );
	}
	else if ( cMsg.GetArg(0) == s_cTok_Lock )
	{
		m_bLocked = LTTRUE;
	}
	else if ( cMsg.GetArg(0) == s_cTok_Unlock )
	{
		m_bLocked = LTFALSE;
	}
	else
		return Prop::OnTrigger(hSender, cMsg);

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Alarm::InitialUpdate()
//
//	PURPOSE:	First update
//
// ----------------------------------------------------------------------- //

LTBOOL Alarm::InitialUpdate()
{
	// Make sure we're non-solid...	

	g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, 0, FLAG_SOLID);

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Alarm::CreateRegionLists()
//
//	PURPOSE:	Create lists of relevant regions
//
// ----------------------------------------------------------------------- //

void Alarm::CreateRegionLists()
{
	LTVector vPos;
	LTFLOAT fSearchY = 64.f;
	g_pLTServer->GetObjectPos( m_hObject, &vPos );
	AIVolume* pContainingVolume = g_pAIVolumeMgr->FindContainingVolume( LTNULL, vPos, eAxisAll, fSearchY, LTNULL );
	if( pContainingVolume )
	{
		AITRACE( AIShowAlarms, ( m_hObject, "In AIVolume: %s", pContainingVolume->GetName() ) );
	}
	else {
		char szName[64];
		g_pLTServer->GetObjectName( m_hObject, szName, sizeof(szName) );
		AIError( "INVALID ALARM: Alarm '%s' is not in an AIVolume!", szName );
	}

	AITRACE( AIShowAlarms, ( m_hObject, "Setting Alert Regions: %s", ::ToString( m_hstrAlertRegions ) ) );
	AITRACE( AIShowAlarms, ( m_hObject, "Setting Respond Regions: %s", ::ToString( m_hstrRespondRegions ) ) );
	AITRACE( AIShowAlarms, ( m_hObject, "Setting Search Regions: %s", ::ToString( m_hstrSearchRegions ) ) );

	CreateRegionList( m_hstrAlertRegions, &m_lstAlertRegions );
	CreateRegionList( m_hstrRespondRegions, &m_lstRespondRegions );
	CreateRegionList( m_hstrSearchRegions, &m_lstSearchRegions );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Alarm::CreateRegionList()
//
//	PURPOSE:	Create a list of relevant regions
//
// ----------------------------------------------------------------------- //

void Alarm::CreateRegionList(HSTRING& hstrRegions, AIREGION_LIST* pList)
{
	// Create a list of regions.
	// Named object may be a region, a list of regions separated by semicolons.

	if( hstrRegions )
	{
		char szRegions[REGION_STRING_LEN + 1];		
		
		const char* pszRegions = g_pLTServer->GetStringData( hstrRegions );
		AIASSERT( strlen( pszRegions ) <= REGION_STRING_LEN, m_hObject, "Alarm::CreateRegionList: RegionList is longer than 256 chars." );
		strncpy( szRegions, pszRegions, REGION_STRING_LEN );
		
		char* tok = strtok( szRegions, "; " );
		while( tok )
		{
			HOBJECT hObject;
			if( ( LT_OK == FindNamedObject( tok, hObject ) ) &&
				( IsAIRegion( hObject ) ) )
			{
				pList->push_back( hObject );
			}
			else {
				char szError[128];
				sprintf( szError, "Alarm::CreateRegionList: '%s' is not a region.", tok ); 
				AIASSERT( 0, m_hObject, szError );
			}

			tok = strtok( LTNULL, "; " );
		}
	}

	FREE_HSTRING( hstrRegions );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Alarm::CalculateRegionsGroupRadius
//
//	PURPOSE:	Calculate the center and radius of the sphere that encompasses
//
// ----------------------------------------------------------------------- //

void Alarm::CalculateRegionsGroupRadius()
{
	CalculateGroupExtents( &m_lstAlertRegions );
	CalculateGroupExtents( &m_lstRespondRegions );
	CalculateGroupExtents( &m_lstSearchRegions );

	// Calculate the center and radius of the sphere that encompasses
	// all regions affected by this alarm.

	m_vRegionsGroupCenter = m_vRegionsGroupExtentsMin + ( ( m_vRegionsGroupExtentsMax - m_vRegionsGroupExtentsMin ) * 0.5f );
	m_fRegionsGroupRadius = m_vRegionsGroupCenter.Dist( m_vRegionsGroupExtentsMin );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Alarm::CalculateGroupExtents
//
//	PURPOSE:	Calculate new extents encompassing regions responding to this alarm.
//
// ----------------------------------------------------------------------- //

void Alarm::CalculateGroupExtents(AIREGION_LIST* pList)
{
	LTVector vExtent;
	AIRegion* pRegion;
	AIREGION_LIST::iterator it;
	for( it = pList->begin(); it != pList->end(); ++it )
	{
		pRegion = (AIRegion*)g_pLTServer->HandleToObject( *it );
		if( pRegion )
		{
			if( m_vRegionsGroupExtentsMin == m_vRegionsGroupExtentsMax )
			{
				m_vRegionsGroupExtentsMin = pRegion->GetExtentsMin();
				m_vRegionsGroupExtentsMax = pRegion->GetExtentsMax();
			}
			else {
				vExtent = pRegion->GetExtentsMin();
				m_vRegionsGroupExtentsMin.x = Min( m_vRegionsGroupExtentsMin.x, vExtent.x );
				m_vRegionsGroupExtentsMin.y = Min( m_vRegionsGroupExtentsMin.y, vExtent.y );
				m_vRegionsGroupExtentsMin.z = Min( m_vRegionsGroupExtentsMin.z, vExtent.z );

				vExtent = pRegion->GetExtentsMax();
				m_vRegionsGroupExtentsMax.x = Max( m_vRegionsGroupExtentsMax.x, vExtent.x );
				m_vRegionsGroupExtentsMax.y = Max( m_vRegionsGroupExtentsMax.y, vExtent.y );
				m_vRegionsGroupExtentsMax.z = Max( m_vRegionsGroupExtentsMax.z, vExtent.z );
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Alarm::IsRegionInRespondGroup
//
//	PURPOSE:	Returns TRUE if region is in Respond group.
//
// ----------------------------------------------------------------------- //

LTBOOL Alarm::IsRegionInRespondGroup(HOBJECT hRegion)
{
	AIREGION_LIST::iterator it;
	for( it = m_lstRespondRegions.begin(); it != m_lstRespondRegions.end(); ++it )
	{
		if( hRegion == *it )
		{
			return LTTRUE;
		}
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Alarm::IsRegionInAlertGroup
//
//	PURPOSE:	Returns TRUE if region is in Alert group.
//
// ----------------------------------------------------------------------- //

LTBOOL Alarm::IsRegionInAlertGroup(HOBJECT hRegion)
{
	AIREGION_LIST::iterator it;
	for( it = m_lstAlertRegions.begin(); it != m_lstAlertRegions.end(); ++it )
	{
		if( hRegion == *it )
		{
			return LTTRUE;
		}
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Alarm::IsRegionCovered
//
//	PURPOSE:	Returns TRUE if region is covered by alarm.
//
// ----------------------------------------------------------------------- //

LTBOOL Alarm::IsRegionCovered(HOBJECT hRegion)
{
	if( IsRegionInRespondGroup( hRegion ) )
	{
		return LTTRUE;
	}

	if( IsRegionInAlertGroup( hRegion ) )
	{
		return LTTRUE;
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Alarm::GetSearchRegion
//
//	PURPOSE:	Returns handle to search region.
//
// ----------------------------------------------------------------------- //

HOBJECT	Alarm::GetSearchRegion(uint32 iRegion)
{
	if( iRegion < m_lstSearchRegions.size() )
	{
		return m_lstSearchRegions[iRegion]; 
	}

	AIASSERT( 0, m_hObject, "Alarm::GetSearchRegion: Region index out of range." ); 
	return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Alarm::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void Alarm::Save(ILTMessage_Write *pMsg)
{
	if (!g_pLTServer || !pMsg) return;

    SAVE_DWORD(m_eState);
	SAVE_BOOL(m_bPlayerUsable);
	SAVE_BOOL(m_bLocked);
	SAVE_FLOAT(m_fAlarmSoundTime);
	SAVE_HSTRING(m_hstrPlayerActivateCommand);

	SAVE_HSTRING(m_hstrAlertRegions);
	SAVE_HSTRING(m_hstrRespondRegions);
	SAVE_HSTRING(m_hstrSearchRegions);

	AIREGION_LIST::iterator it;

	SAVE_DWORD( m_lstAlertRegions.size() );
	for( it = m_lstAlertRegions.begin(); it != m_lstAlertRegions.end(); ++it )
	{
		SAVE_HOBJECT( *it );
	}

	SAVE_DWORD( m_lstRespondRegions.size() );
	for( it = m_lstRespondRegions.begin(); it != m_lstRespondRegions.end(); ++it )
	{
		SAVE_HOBJECT( *it );
	}

	SAVE_DWORD( m_lstSearchRegions.size() );
	for( it = m_lstSearchRegions.begin(); it != m_lstSearchRegions.end(); ++it )
	{
		SAVE_HOBJECT( *it );
	}

	SAVE_VECTOR( m_vRegionsGroupExtentsMin );
	SAVE_VECTOR( m_vRegionsGroupExtentsMax );
	SAVE_VECTOR( m_vRegionsGroupCenter );
	SAVE_FLOAT(	m_fRegionsGroupRadius );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Alarm::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void Alarm::Load(ILTMessage_Read *pMsg)
{
	if (!g_pLTServer || !pMsg) return;

    LOAD_DWORD_CAST(m_eState, State);
	LOAD_BOOL(m_bPlayerUsable);
	LOAD_BOOL(m_bLocked);
	LOAD_FLOAT(m_fAlarmSoundTime);
	LOAD_HSTRING(m_hstrPlayerActivateCommand);

	LOAD_HSTRING(m_hstrAlertRegions);
	LOAD_HSTRING(m_hstrRespondRegions);
	LOAD_HSTRING(m_hstrSearchRegions);

	uint32 cRegions;
	AIREGION_LIST::iterator it;

	LOAD_DWORD( cRegions );
	m_lstAlertRegions.resize( cRegions );
	for( it = m_lstAlertRegions.begin(); it != m_lstAlertRegions.end(); ++it )
	{
		LOAD_HOBJECT( *it );
	}

	LOAD_DWORD( cRegions );
	m_lstRespondRegions.resize( cRegions );
	for( it = m_lstRespondRegions.begin(); it != m_lstRespondRegions.end(); ++it )
	{
		LOAD_HOBJECT( *it );
	}

	LOAD_DWORD( cRegions );
	m_lstSearchRegions.resize( cRegions );
	for( it = m_lstSearchRegions.begin(); it != m_lstSearchRegions.end(); ++it )
	{
		LOAD_HOBJECT( *it );
	}

	LOAD_VECTOR( m_vRegionsGroupExtentsMin );
	LOAD_VECTOR( m_vRegionsGroupExtentsMax );
	LOAD_VECTOR( m_vRegionsGroupCenter );
	LOAD_FLOAT(	m_fRegionsGroupRadius );
}

