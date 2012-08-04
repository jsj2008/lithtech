// ----------------------------------------------------------------------- //
//
// MODULE  : CTFFlagBase.cpp
//
// PURPOSE : FlagBase object to place in CTF level.
//
// CREATED : 05/04/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//

#include "Stdafx.h"
#include "CTFFlagBase.h"
#include "CTFFlag.h"
#include "PlayerObj.h"
#include "ObjectMsgs.h"
#include "PropsDB.h"
#include "GameModeMgr.h"
#include "PlayerInventory.h"
#include "StateMachine.h"

LINKFROM_MODULE( CTFFlagBase )

// Statemachine to handle FlagBase states.
class FlagBaseStateMachine : public MacroStateMachine
{
public:

	FlagBaseStateMachine( )
	{
		m_pFlagBase = NULL;
	}

	// Initialize to a flagbase.
	bool Init( CTFFlagBase& flagBase )
	{
		m_pFlagBase = &flagBase;
		return true;
	}

	// Send touch event to state.
	bool DoTouchEvent( HOBJECT hToucher )
	{
		FlagBaseStateMachine::FlagBaseEventParams eventParams;
		eventParams.m_hObject = hToucher;
		return DoUserEvent( FlagBaseStateMachine::eFlagBaseEvent_Touched, eventParams );
	}

	// Send steal event to state.
	bool DoStealEvent( HOBJECT hStealer )
	{
		FlagBaseStateMachine::FlagBaseEventParams eventParams;
		eventParams.m_hObject = hStealer;
		return DoUserEvent( FlagBaseStateMachine::eFlagBaseEvent_Steal, eventParams );
	}

	// Send friendly cap event to state.
	bool DoFriendlyCapEvent( HOBJECT hFriendly )
	{
		FlagBaseEventParams eventParams;
		eventParams.m_hObject = hFriendly;
		return DoUserEvent( FlagBaseStateMachine::eFlagBaseEvent_FriendlyCap, eventParams );
	}

	// Send return event to state.
	bool DoReturnEvent( )
	{
		EventParams cParams;
		return DoUserEvent( FlagBaseStateMachine::eFlagBaseEvent_Return, cParams );
	}

	// Send return event to state.
	bool DoRemoveEvent( )
	{
		EventParams cParams;
		return DoUserEvent( FlagBaseStateMachine::eFlagBaseEvent_Remove, cParams );
	}

protected:

	// Statemachine event handlers.
	bool HasFlag_OnEnter( MacroStateMachine::EventParams& eventParams );
	bool HasFlag_OnTouched( MacroStateMachine::EventParams& eventParams );
	bool HasFlag_OnSteal( MacroStateMachine::EventParams& eventParams );
	bool HasFlag_OnRemove( MacroStateMachine::EventParams& eventParams );
	bool HasFlag_OnFriendlyCap( MacroStateMachine::EventParams& eventParams );
	bool NoFlag_OnEnter( MacroStateMachine::EventParams& eventParams );
	bool NoFlag_OnReturn( MacroStateMachine::EventParams& eventParams );

	// FlagBase event paramaters.
	struct FlagBaseEventParams : public EventParams
	{
		FlagBaseEventParams( )
		{
			m_hObject = NULL;
		}

		HOBJECT m_hObject;
	};

	// Statemachine events for flagbase.
	enum EFlagBaseEvent
	{
		// FlagBase touched.
		eFlagBaseEvent_Touched = EVENT_User,
		// Flag stolen.
		eFlagBaseEvent_Steal,
		// Flag removed.
		eFlagBaseEvent_Remove,
		// Flag returned.
		eFlagBaseEvent_Return,
		// Friendly player making capture.
		eFlagBaseEvent_FriendlyCap,
	};

	// State table.
	MSM_BeginTable( FlagBaseStateMachine )
		MSM_BeginState( kCTFFlagBaseState_HasFlag )
			MSM_OnEnter( HasFlag_OnEnter )
			MSM_OnEvent( eFlagBaseEvent_Touched, HasFlag_OnTouched )
			MSM_OnEvent( eFlagBaseEvent_Steal, HasFlag_OnSteal )
			MSM_OnEvent( eFlagBaseEvent_Remove, HasFlag_OnRemove )
			MSM_OnEvent( eFlagBaseEvent_FriendlyCap, HasFlag_OnFriendlyCap )
		MSM_EndState( )
		MSM_BeginState( kCTFFlagBaseState_NoFlag )
			MSM_OnEnter( NoFlag_OnEnter )
			MSM_OnEvent( eFlagBaseEvent_Return, NoFlag_OnReturn )
		MSM_EndState( )
	MSM_EndTable( )

private:

	// The flagbase that owns us.
	CTFFlagBase* m_pFlagBase;
};

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTFFlagBase::HasFlag_OnTouched
//
//	PURPOSE:	Handle a touch event.
//
// ----------------------------------------------------------------------- //
bool FlagBaseStateMachine::HasFlag_OnEnter( MacroStateMachine::EventParams& eventParams )
{
	// Make sure our flag exists
	if( !m_pFlagBase->m_hFlag )
	{
		LTERROR( "Missing flag object." );
		return false;
	}

	// Play the has flag animation.
	char const* pszAniName = DATABASE_CATEGORY( CTFFlagBase ).GETRECORDATTRIB( m_pFlagBase->GetFlagBaseRec( ), BaseAniHasFlag );
	if( !LTStrEmpty( pszAniName ))
	{
		HMODELANIM hAni = INVALID_MODEL_ANIM;
		g_pModelLT->GetAnimIndex( m_pFlagBase->m_hObject, pszAniName, hAni );
		if( INVALID_MODEL_ANIM != hAni )
		{
			g_pModelLT->SetPlaying( m_pFlagBase->m_hObject, MAIN_TRACKER, true );
			g_pModelLT->SetLooping( m_pFlagBase->m_hObject, MAIN_TRACKER, true );
			g_pModelLT->SetCurAnim( m_pFlagBase->m_hObject, MAIN_TRACKER, hAni, true );
		}
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTFFlagBase::HasFlag_OnTouched
//
//	PURPOSE:	Handle a touch event.
//
// ----------------------------------------------------------------------- //
bool FlagBaseStateMachine::HasFlag_OnTouched( MacroStateMachine::EventParams& eventParams )
{
	FlagBaseEventParams& flagBaseEventParams = ( FlagBaseEventParams& )eventParams;

	// Check if we were touched by a player.  Ignore all others.
	CPlayerObj* pPlayerObj = CPlayerObj::DynamicCast( flagBaseEventParams.m_hObject );
	if( !pPlayerObj )
		return false;

	// Don't allow non-alive players to touch the flagbase.
	if( !pPlayerObj->IsAlive( ))
		return false;

	// If touched by enemy, see if they can grab our flag.
	if( pPlayerObj->GetTeamID() != m_pFlagBase->GetTeamId( ))
	{
		// Player already carrying a flag, can't grab ours.
		if( pPlayerObj->GetInventory()->GetCTFFlag( ))
			return false;

		// Enemy is stealing the flag.
		if( !DoStealEvent( pPlayerObj->m_hObject ))
			return false;
	}
	else
	{
		// If friendly player isn't holding an enemy flag, there's nothing to do.
		if( !pPlayerObj->GetInventory()->GetCTFFlag( ))
			return false;

		// Friendly is capturing enemy flag.
		if( !DoFriendlyCapEvent( pPlayerObj->m_hObject ))
			return false;
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTFFlagBase::HasFlag_OnSteal
//
//	PURPOSE:	Flag steal handler.
//
// ----------------------------------------------------------------------- //
bool FlagBaseStateMachine::HasFlag_OnSteal( MacroStateMachine::EventParams& eventParams )
{
	// Make sure steals are allowed.
	if( !m_pFlagBase->m_bAllowSteals )
		return false;

	FlagBaseEventParams& flagBaseEventParams = ( FlagBaseEventParams& )eventParams;

	CPlayerObj* pPlayerObj = CPlayerObj::DynamicCast( flagBaseEventParams.m_hObject );
	if( !pPlayerObj )
		return false;

	// Make sure they don't already have a flag.
	if( pPlayerObj->GetInventory()->GetCTFFlag( ))
		return false;

	// Get our flag object.
	CTFFlag* pFlag = CTFFlag::DynamicCast( m_pFlagBase->m_hFlag );
	if( !pFlag )
		return false;

	// Tell the flag that it was stolen.
	if( !pFlag->FlagStolen( pPlayerObj->m_hObject ))
		return false;

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTFFlagBase::HasFlag_OnRemove
//
//	PURPOSE:	Flag remove handler.
//
// ----------------------------------------------------------------------- //
bool FlagBaseStateMachine::HasFlag_OnRemove( MacroStateMachine::EventParams& eventParams )
{
	return SetState( kCTFFlagBaseState_NoFlag );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTFFlagBase::HasFlag_OnFriendlyCap
//
//	PURPOSE:	Friendly flag capture handler.
//
// ----------------------------------------------------------------------- //
bool FlagBaseStateMachine::HasFlag_OnFriendlyCap( MacroStateMachine::EventParams& eventParams )
{
	FlagBaseEventParams& flagBaseEventParams = ( FlagBaseEventParams& )eventParams;

	CPlayerObj* pPlayerObj = CPlayerObj::DynamicCast( flagBaseEventParams.m_hObject );
	if( !pPlayerObj )
		return false;

	// Make sure they have a flag.
	HOBJECT hFlag = pPlayerObj->GetInventory()->GetCTFFlag( );
	if( !hFlag )
		return false;

	// Tell the flag it was captured.
	CTFFlag* pFlag = CTFFlag::DynamicCast( hFlag );
	if( !pFlag )
		return false;
	if( !pFlag->FlagCapture())
		return false;

	// Send capture assist to our flag.
	CTFFlag* pOurFlag = CTFFlag::DynamicCast( m_pFlagBase->m_hFlag );
	if( !pOurFlag )
		return false;
	if( !pOurFlag->CaptureAssist( flagBaseEventParams.m_hObject ))
		return false;

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTFFlagBase::HasFlag_OnTouched
//
//	PURPOSE:	Handle a touch event.
//
// ----------------------------------------------------------------------- //
bool FlagBaseStateMachine::NoFlag_OnEnter( MacroStateMachine::EventParams& eventParams )
{
	// Play the no flag animation.
	char const* pszAniName = DATABASE_CATEGORY( CTFFlagBase ).GETRECORDATTRIB( m_pFlagBase->GetFlagBaseRec( ), BaseAniNoFlag );
	if( !LTStrEmpty( pszAniName ))
	{
		HMODELANIM hAni = INVALID_MODEL_ANIM;
		g_pModelLT->GetAnimIndex( m_pFlagBase->m_hObject, pszAniName, hAni );
		if( INVALID_MODEL_ANIM != hAni )
		{
			g_pModelLT->SetPlaying( m_pFlagBase->m_hObject, MAIN_TRACKER, true );
			g_pModelLT->SetLooping( m_pFlagBase->m_hObject, MAIN_TRACKER, true );
			g_pModelLT->SetCurAnim( m_pFlagBase->m_hObject, MAIN_TRACKER, hAni, true );
		}
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTFFlagBase::NoFlag_OnReturn
//
//	PURPOSE:	Flag return handler.
//
// ----------------------------------------------------------------------- //
bool FlagBaseStateMachine::NoFlag_OnReturn( MacroStateMachine::EventParams& eventParams )
{
	return SetState( kCTFFlagBaseState_HasFlag );
}


// Plugin class for hooking into the level editor for displaying entries in listboxes and displaying the model...
class CTFFlagBasePlugin : public IObjectPlugin
{
public: // Methods...

	virtual LTRESULT PreHook_EditStringList( 
		const char *szRezPath,
		const char *szPropName,
		char **aszStrings,
		uint32 *pcStrings,
		const uint32 cMaxStrings,
		const uint32 cMaxStringLen );

	virtual LTRESULT PreHook_Dims(
		const char* szRezPath,
		const char* szPropName, 
		const char* szPropValue,
		char* szModelFilenameBuf,
		int	  nModelFilenameBufLen,
		LTVector & vDims,
		const char* pszObjName, 
		ILTPreInterface *pInterface);

	virtual LTRESULT PreHook_PropChanged( 
		const	char		*szObjName,
		const	char		*szPropName,
		const	int			nPropType,
		const	GenericProp	&gpPropValue,
		ILTPreInterface		*pInterface,
		const	char		*szModifiers );

private: // Members...

	CCommandMgrPlugin			m_CommandMgrPlugin;
};


BEGIN_CLASS( CTFFlagBase )
	ADD_STRINGPROP_FLAG( Filename, "", PF_HIDDEN | PF_MODEL, "This hidden property is needed in order to get the model visible within WorldEdit." )
	ADD_STRINGPROP_FLAG( FlagBase, "", PF_STATICLIST | PF_DIMS | PF_LOCALDIMS, "Record within the game database GameModes/CTF/FlagBase to use for defining flagbase." )
	ADD_STRINGPROP_FLAG(Team, "NoTeam", PF_STATICLIST, "This is a dropdown that allows you to set which team this object belongs to.")
	ADD_BOOLPROP( MoveToFloor, true, "If true the object is moved to the floor when created in the game." )
	ADD_BOOLPROP( AllowSteals, true, "If true the flag can be stolen from this flagbase." )
	ADD_PREFETCH_RESOURCE_PROPS()
END_CLASS_FLAGS_PLUGIN_PREFETCH( CTFFlagBase, GameBase, CF_DONTSAVE, CTFFlagBasePlugin, DefaultPrefetch<CTFFlagBase>, "Places a CTFFlagBase within the level."  )


extern bool ValidateMsgBool( ILTPreInterface *pInterface, ConParse &cpMsgParams );

// Register with the CommandMgr...

CMDMGR_BEGIN_REGISTER_CLASS( CTFFlagBase )
CMDMGR_END_REGISTER_CLASS( CTFFlagBase, GameBase )


LTRESULT CTFFlagBasePlugin::PreHook_EditStringList( const char *szRezPath,
											   const char *szPropName,
											   char **aszStrings,
											   uint32 *pcStrings,
											   const uint32 cMaxStrings,
											   const uint32 cMaxStringLen )
{
	if( !aszStrings || !pcStrings )
	{
		LTERROR( "Invalid input parameters" );
		return LT_UNSUPPORTED;
	}

	if( LTStrEquals( szPropName, "FlagBase" ))
	{
		// Fill the first string in the list with a <none> selection...
		LTStrCpy( aszStrings[(*pcStrings)++], "", cMaxStringLen );

		// Add an entry for each flagbase.
		uint8 nNumRecords = DATABASE_CATEGORY( CTFFlagBase ).GetNumRecords();
		for( uint8 nRecordIndex = 0; nRecordIndex < nNumRecords; ++nRecordIndex )
		{
			LTASSERT( cMaxStrings > (*pcStrings) + 1, "Too many flagbase's to fit in the list.  Enlarge list size?" );

			HRECORD hRecord = DATABASE_CATEGORY( CTFFlagBase ).GetRecordByIndex( nRecordIndex );
			if( !hRecord )
				continue;

			const char *pszRecordName = DATABASE_CATEGORY( CTFFlagBase ).GetRecordName( hRecord );
			if( !pszRecordName )
				continue;

			if( (LTStrLen( pszRecordName ) < cMaxStringLen) && ((*pcStrings) + 1 < cMaxStrings) )
			{
				LTStrCpy( aszStrings[(*pcStrings)++], pszRecordName, cMaxStringLen );
			}

		}

		// Sort the list so things are easier to find.  Skip the first item, since it's the <none> selection.
		qsort( aszStrings + 1, *pcStrings - 1, sizeof(char *), CaseInsensitiveCompare );

		return LT_OK;
	}

	// Handle team...
	if( LTStrIEquals( "Team", szPropName ))
	{
		TeamPopulateEditStringList( aszStrings, pcStrings, cMaxStrings, cMaxStringLen );
		return LT_OK;
	}

	return LT_UNSUPPORTED;
}

LTRESULT CTFFlagBasePlugin::PreHook_Dims( const char* szRezPath,
										 const char* szPropName, 
									 const char* szPropValue,
									 char* szModelFilenameBuf,
									 int nModelFilenameBufLen,
									 LTVector &vDims,
									 const char* pszObjName, 
									 ILTPreInterface *pInterface)
{
	// Don't proceed without a value value.
	if( LTStrEmpty( szPropValue ))
		return LT_OK;

	// Get FlagBase used.
	HRECORD hRecord = DATABASE_CATEGORY( CTFFlagBase ).GetRecordByName( szPropValue );
	if( !hRecord )
		return LT_UNSUPPORTED;

	// Get the prop used.
	HRECORD hProp = DATABASE_CATEGORY( CTFFlagBase ).GETRECORDATTRIB( hRecord, BaseProp );
	if( !hProp )
		return LT_UNSUPPORTED;

	// Get the model from the props category.
	const char *pszModel = g_pPropsDB->GetPropFilename( hProp );
	if( !pszModel )
		return LT_UNSUPPORTED;

	LTStrCpy( szModelFilenameBuf, pszModel, nModelFilenameBufLen );

	return LT_OK;
}

LTRESULT CTFFlagBasePlugin::PreHook_PropChanged( const char *szObjName, 
											const char *szPropName,
											const int nPropType,
											const GenericProp &gpPropValue,
											ILTPreInterface *pInterface,
											const char *szModifiers )
{

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


//
// CTFFlagBase class implementation...
//

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CTFFlagBase::CTFFlagBase
//
//  PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

CTFFlagBase::CTFFlagBase( ) : GameBase( OT_MODEL )
{
	m_bMoveToFloor = true;
	m_hFlag.SetReceiver( *this );
	m_nTeamId = INVALID_TEAM;
	m_pFlagBaseStateMachine = NULL;
	m_hFlagBaseRec = NULL;
	m_bAllowSteals = true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CTFFlagBase::~CTFFlagBase
//
//  PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

CTFFlagBase::~CTFFlagBase( )
{
	if( m_pFlagBaseStateMachine )
	{
		delete m_pFlagBaseStateMachine;
		m_pFlagBaseStateMachine = NULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTFFlagBase::EngineMessageFn
//
//	PURPOSE:	Handle messages from the engine...
//
// ----------------------------------------------------------------------- //
uint32 CTFFlagBase::EngineMessageFn( uint32 messageID, void *pData, float fData )
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			// If we don't have ctf rules defined, don't bother with the object.
			if( !GameModeMgr::Instance().GetCTFRulesRecord( ))
				return 0;

			uint32 dwRet = GameBase::EngineMessageFn( messageID, pData, fData );

			ObjectCreateStruct* pStruct = (ObjectCreateStruct*)pData;

			if( fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP )
			{
				if( !ReadProp( &pStruct->m_cProperties ))
					return 0;
			}

			if( !PostReadProp( pStruct ))
				return 0;

			return dwRet;
		}
		break;

		case MID_INITIALUPDATE:
		{
			uint32 dwRet = GameBase::EngineMessageFn( messageID, pData, fData );

			if (fData != INITIALUPDATE_SAVEGAME)
			{
				InitialUpdate();
			}

			return dwRet;
		}
		break;

		case MID_UPDATE:
		{
			Update( );
		}
		break;

		case MID_TOUCHNOTIFY:
		{
			HandleTouchNotify(( HOBJECT )pData);
		}
		break;

		default : 
		break;
	}

	return GameBase::EngineMessageFn( messageID, pData, fData );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTFFlagBase::ReadProp
//
//	PURPOSE:	Read in the properties of the object... 
//
// ----------------------------------------------------------------------- //
bool CTFFlagBase::ReadProp( const GenericPropList *pProps )
{
	char const* pszFlagBase = pProps->GetString( "FlagBase", "" );
	if( LTStrEmpty( pszFlagBase ))
		return false;
	m_hFlagBaseRec = DATABASE_CATEGORY( CTFFlagBase ).GetRecordByName( pszFlagBase );
	if( !m_hFlagBaseRec )
		return false;

	m_bMoveToFloor = pProps->GetBool( "MoveToFloor", m_bMoveToFloor );
	m_bAllowSteals = pProps->GetBool( "AllowSteals", m_bAllowSteals );
	
	if( GameModeMgr::Instance( ).m_grbUseTeams )
	{
		const char *pszTeam = pProps->GetString( "Team", "" );
		m_nTeamId = TeamStringToTeamId( pszTeam );
	}
	else
	{
		m_nTeamId = INVALID_TEAM;
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTFFlagBase::PostReadProp
//
//	PURPOSE:	Configure the ObjectCreateStruct for creating the object
//
// ----------------------------------------------------------------------- //
bool CTFFlagBase::PostReadProp( ObjectCreateStruct *pStruct )
{
	// Get the prop used.
	HRECORD hProp = DATABASE_CATEGORY( CTFFlagBase ).GETRECORDATTRIB( m_hFlagBaseRec, BaseProp );
	if( !hProp )
		return false;

	// Fill in the model and material names.
	char const* pszPropFilename = g_pPropsDB->GetPropFilename( hProp );
	if( LTStrEmpty( pszPropFilename ))
		return false;
	LTStrCpy( pStruct->m_Filename, pszPropFilename, LTARRAYSIZE( pStruct->m_Filename ));

	g_pPropsDB->CopyMaterialFilenames( hProp, pStruct->m_Materials[0], LTARRAYSIZE( pStruct->m_Materials ),
		LTARRAYSIZE( pStruct->m_Materials[0] ));

	// Make the flagbase get touch notifies and be visible.
	pStruct->m_Flags |= FLAG_TOUCH_NOTIFY | FLAG_VISIBLE;

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTFFlagBase::InitialUpdate
//
//	PURPOSE:	Handle a MID_INITIALUPDATE message from the engine....
//
// ----------------------------------------------------------------------- //
void CTFFlagBase::InitialUpdate( )
{
	// Set the base model diminsions...
	LTVector vDims;
	HMODELANIM hAnimBase = INVALID_MODEL_ANIM;
	g_pModelLT->GetCurAnim( m_hObject, MAIN_TRACKER, hAnimBase );
	g_pModelLT->GetModelAnimUserDims (m_hObject, hAnimBase, &vDims);
	g_pPhysicsLT->SetObjectDims( m_hObject, &vDims, 0 );

	// Make sure object starts on floor if the flag is set...
	if( m_bMoveToFloor )
	{
		MoveObjectToFloor( m_hObject );
	}

	// Create the flag object this base will own.
	CreateFlag( );

	// Create our statemachine object.
	LT_MEM_TRACK_ALLOC(m_pFlagBaseStateMachine = new FlagBaseStateMachine, LT_MEM_TYPE_GAMECODE);
	m_pFlagBaseStateMachine->Init( *this );
	m_pFlagBaseStateMachine->SetState( kCTFFlagBaseState_HasFlag );

	CreateSpecialFX( false );

	SetNextUpdate( UPDATE_NEVER );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTFFlagBase::Update
//
//	PURPOSE:	Handle a MID_UPDATE message from the engine....
//
// ----------------------------------------------------------------------- //
void CTFFlagBase::Update( )
{
	SetNextUpdate( UPDATE_NEXT_FRAME );

	if( !m_pFlagBaseStateMachine )
		return;

	// Update the statemachine.
	m_pFlagBaseStateMachine->Update();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTFFlagBase::OnLinkBroken
//
//	PURPOSE:	Handle links getting broken.
//
// ----------------------------------------------------------------------- //
void CTFFlagBase::OnLinkBroken( LTObjRefNotifier *pRef, HOBJECT hObj )
{
	GameBase::OnLinkBroken( pRef, hObj );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTFFlagBase::CreateSpecialFX
//
//	PURPOSE:	Send relevant information to clients...
//
// ----------------------------------------------------------------------- //

void CTFFlagBase::CreateSpecialFX( bool bUpdateClients )
{
	if( !m_pFlagBaseStateMachine )
		return;

	CTFFLAGBASECREATESTRUCT cs;
	cs.m_hFlagBaseRec = m_hFlagBaseRec;
	cs.m_nTeamId = m_nTeamId;
	cs.m_eCTFFlagBaseState = ( CTFFlagBaseState )m_pFlagBaseStateMachine->GetState();
	cs.m_hFlag = m_hFlag;

	{
		CAutoMessage cMsg;
		cMsg.Writeuint8( SFX_CTFFLAGBASE_ID );
		cs.Write( cMsg );
		g_pLTServer->SetObjectSFXMessage( m_hObject, cMsg.Read( ));
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTFFlagBase::HandleTouchNotify
//
//	PURPOSE:	Handle a MID_TOUCHNOTIFY message from the engine....
//
// ----------------------------------------------------------------------- //

void CTFFlagBase::HandleTouchNotify( HOBJECT hToucher )
{
	if( !m_pFlagBaseStateMachine )
		return;

	m_pFlagBaseStateMachine->DoTouchEvent( hToucher );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTFFlagBase::CreateFlag
//
//	PURPOSE:	Creates the flag object.
//
// ----------------------------------------------------------------------- //
bool CTFFlagBase::CreateFlag( )
{
	HCLASS hFlagClass = g_pLTServer->GetClass( "CTFFlag" );
	if( !hFlagClass )
		return false;

	// Create the flag at the base location.
	ObjectCreateStruct theStruct;
	g_pLTServer->GetObjectPos( m_hObject, &theStruct.m_Pos );
	g_pLTServer->GetObjectRotation( m_hObject, &theStruct.m_Rotation );
	theStruct.m_UserData = ( uint32 )this;
	CTFFlag* pCTFFlag = ( CTFFlag* )g_pLTServer->CreateObject( hFlagClass, &theStruct );
	if( !pCTFFlag )
		return false;

	m_hFlag = pCTFFlag->m_hObject;
	
	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTFFlagBase::ReturnFlag
//
//	PURPOSE:	Returns the flag to the base.
//
// ----------------------------------------------------------------------- //
bool CTFFlagBase::ReturnFlag( )
{
	if( !m_pFlagBaseStateMachine )
		return true;

	return m_pFlagBaseStateMachine->DoReturnEvent( );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTFFlagBase::RemoveFlag
//
//	PURPOSE:	Removes the flag.
//
// ----------------------------------------------------------------------- //
bool CTFFlagBase::RemoveFlag( )
{
	if( !m_pFlagBaseStateMachine )
		return true;

	return m_pFlagBaseStateMachine->DoRemoveEvent( );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTFFlagBase::GetPrefetchResourceList
//
//	PURPOSE:	Determines the list of all needed resources
//
// ----------------------------------------------------------------------- //

void CTFFlagBase::GetPrefetchResourceList(const char* pszObjectName, IObjectResourceGatherer* pInterface, ResourceList& Resources )
{
	// get the CP record
	char szType[256];
	pInterface->GetPropString(pszObjectName, "FlagBase", szType, LTARRAYSIZE(szType), NULL);
	if( LTStrEmpty( szType ))
		return;
	HRECORD hFlagBaseRec = DATABASE_CATEGORY( CTFFlagBase ).GetRecordByName( szType );
	if( !hFlagBaseRec )
		return;

	GetRecordResources( Resources, hFlagBaseRec, true );
}


