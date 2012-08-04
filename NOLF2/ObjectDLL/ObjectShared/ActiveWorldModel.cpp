// ----------------------------------------------------------------------- //
//
// MODULE  : ActiveWorldModel.cpp
//
// PURPOSE : ActiveWorldModel implementation
//
// CREATED : 5/16/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//

	#include "stdafx.h"
	#include "ActiveWorldModel.h"
	#include "ServerSoundMgr.h"
	#include "AIStimulusMgr.h"
	#include "AINode.h"
	#include "AINodeMgr.h"
	#include "AIState.h"
	#include "ParsedMsg.h"
	#include "KeyMgr.h"
	#include "AIUtils.h"

	extern CAIStimulusMgr* g_pAIStimulusMgr;

//
// Globals...
//

	const char* c_aWaveTypes[] =
	{
		"Linear",
		"Sine",
		"SlowOff",
		"SlowOn",
	};

//
// Defines...
//
	
LINKFROM_MODULE( ActiveWorldModel );

//
// Add Props...
//



BEGIN_CLASS( ActiveWorldModel )

	AWM_SET_TYPE_STATIC

	// Add an options group...

	PROP_DEFINEGROUP(Options, PF_GROUP(3))
		ADD_BOOLPROP_FLAG(PlayerActivate, LTFALSE, PF_GROUP(3))
		ADD_BOOLPROP_FLAG(AIActivate, LTFALSE, PF_GROUP(3))
		ADD_BOOLPROP_FLAG(StartOn, LTFALSE, PF_GROUP(3))
		ADD_BOOLPROP_FLAG(TriggerOff, LTTRUE, PF_GROUP(3))
		ADD_BOOLPROP_FLAG(RemainOn, LTTRUE, PF_GROUP(3))
		ADD_BOOLPROP_FLAG(ForceMove, LTFALSE, PF_GROUP(3))
		ADD_BOOLPROP_FLAG(ForceMoveOn, LTFALSE, PF_GROUP(3))
		ADD_BOOLPROP_FLAG(ForceMoveOff, LTFALSE, PF_GROUP(3))
		ADD_BOOLPROP_FLAG(Locked, LTFALSE, PF_GROUP(3))
		ADD_BOOLPROP_FLAG(RotateAway, LTTRUE, PF_GROUP(3))
		ADD_STRINGPROP_FLAG(Waveform, "Linear", PF_STATICLIST | PF_GROUP(3))
		ADD_STRINGPROP_FLAG(ActivateType, "<none>", PF_STATICLIST | PF_GROUP(3))

	// Add a sounds group...

	PROP_DEFINEGROUP(Sounds, PF_GROUP(4))
		ADD_STRINGPROP_FLAG(PowerOnSound, "", PF_FILENAME | PF_GROUP(4))
		ADD_STRINGPROP_FLAG(OnSound, "", PF_FILENAME | PF_GROUP(4))
		ADD_STRINGPROP_FLAG(PowerOffSound, "", PF_FILENAME | PF_GROUP(4))
		ADD_STRINGPROP_FLAG(OffSound, "", PF_FILENAME | PF_GROUP(4))
		ADD_STRINGPROP_FLAG(LockedSound, "", PF_FILENAME | PF_GROUP(4))
		ADD_VECTORPROP_VAL_FLAG(SoundPos, 0.0f, 0.0f, 0.0f, PF_GROUP(4))
		ADD_REALPROP_FLAG(SoundRadius, 1000.0f, PF_RADIUS | PF_GROUP(4))
		ADD_BOOLPROP_FLAG(LoopSounds, LTFALSE, PF_GROUP(4))

	// Add a commands group...

	PROP_DEFINEGROUP(Commands, PF_GROUP(5))
		ADD_STRINGPROP_FLAG(OnCommand, "", PF_GROUP(5) | PF_NOTIFYCHANGE)
		ADD_STRINGPROP_FLAG(OffCommand, "", PF_GROUP(5) | PF_NOTIFYCHANGE)
		ADD_STRINGPROP_FLAG(PowerOnCommand, "", PF_GROUP(5) | PF_NOTIFYCHANGE)
		ADD_STRINGPROP_FLAG(PowerOffCommand, "", PF_GROUP(5) | PF_NOTIFYCHANGE)
		ADD_STRINGPROP_FLAG(LockedCommand, "", PF_GROUP(5) | PF_NOTIFYCHANGE)

	// Add to the disturbance group...

	ADD_LONGINTPROP_FLAG(ActivateAlarmLevel, 0, PF_GROUP(6))
	

	// Add the movement properties...

	ADD_VECTORPROP_VAL_FLAG(MoveDir, 0.0f, 0.0f, 0.0f, 0)
	ADD_REALPROP_FLAG(MoveDist, 0.0f, 0)

	// Add the rotation properties...

	ADD_STRINGPROP_FLAG(RotationPoint, "", PF_OBJECTLINK)
	ADD_VECTORPROP_FLAG(RotationAngles, 0)
	
	// Add the shared properties...

	ADD_REALPROP_FLAG(PowerOnTime, AWM_DEFAULT_POWERONTIME, 0)
	ADD_REALPROP_FLAG(PowerOffTime, AWM_DEFAULT_POWEROFFTIME, 0)
	ADD_REALPROP_FLAG(MoveDelay, AWM_DEFAULT_MOVEDELAY, 0)
	ADD_REALPROP_FLAG(OnWaitTime, AWM_DEFAULT_ONWAITTIME, 0)
	ADD_REALPROP_FLAG(OffWaitTime, AWM_DEFAULT_OFFWAITTIME, 0)

END_CLASS_DEFAULT_FLAGS_PLUGIN( ActiveWorldModel, WorldModel, NULL, NULL, CF_HIDDEN | CF_WORLDMODEL, CActiveWorldModelPlugin )


//
// Register the calss with the command mgr plugin and add any messages to the class
//

CMDMGR_BEGIN_REGISTER_CLASS( ActiveWorldModel )

	CMDMGR_ADD_MSG( ACTIVATE, 1, NULL, "ACTIVATE" )
	CMDMGR_ADD_MSG( TRIGGER, 1, NULL, "TRIGGER" )
	CMDMGR_ADD_MSG( LOCK, 1, NULL, "LOCK" )
	CMDMGR_ADD_MSG( UNLOCK, 1, NULL, "UNLOCK" )
	CMDMGR_ADD_MSG( ON, 1, NULL, "ON" )
	CMDMGR_ADD_MSG( OFF, 1, NULL, "OFF" )
	CMDMGR_ADD_MSG( PAUSE, 1, NULL, "PAUSE" )
	CMDMGR_ADD_MSG( RESUME, 1, NULL, "RESUME" )

CMDMGR_END_REGISTER_CLASS( ActiveWorldModel, WorldModel )

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CActiveWorldModelPlugin::PreHook_EditStringList
//
//  PURPOSE:	Populate our string lists with desired data
//
// ----------------------------------------------------------------------- //

LTRESULT CActiveWorldModelPlugin::PreHook_EditStringList( const char *szRezPath, 
														  const char *szPropName,
														  char **aszStrings,
														  uint32 *pcStrings,
														  const uint32 cMaxStrings,
														  const uint32 cMaxStringLength )
{
	ASSERT( szPropName && aszStrings && pcStrings );
	
	// Send to the World model first to see if it will handle the property list

	if( CWorldModelPlugin::PreHook_EditStringList( szRezPath,
												   szPropName,
												   aszStrings,
												   pcStrings,
												   cMaxStrings,
												   cMaxStringLength ) == LT_OK )
	{
		return LT_OK;
	}

	// See if we can handle the list...

	if( !_stricmp( szPropName, "Waveform" ) )
	{
		// Fill the list with our wave types...

		for( int i = 0; i <= AWM_WAVE_MAXTYPES; i++ )
		{
			strcpy( aszStrings[(*pcStrings)++], c_aWaveTypes[i] );
		}

		return LT_OK;		
	}
	else if( !_stricmp( szPropName, "ActivateType" ))
	{
		if( m_ActivateTypeMgrPlugin.PreHook_EditStringList( szRezPath,
															szPropName,
															aszStrings,
															pcStrings,
															cMaxStrings,
															cMaxStringLength ) == LT_OK )
		{
			return LT_OK;
		}
	}

	return LT_UNSUPPORTED;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CActiveWorldModelPlugin::PreHook_PropChanged
//
//  PURPOSE:	Check our command strings
//
// ----------------------------------------------------------------------- //

LTRESULT CActiveWorldModelPlugin::PreHook_PropChanged( const char *szObjName,
													   const char *szPropName, 
													   const int  nPropType, 
													   const GenericProp &gpPropValue,
													   ILTPreInterface *pInterface,
													   const char *szModifiers )
{
	ASSERT( szObjName && szPropName && pInterface );

	// Send to our base class first...

	if( CWorldModelPlugin::PreHook_PropChanged( szObjName,
												szPropName,
												nPropType,
												gpPropValue,
												pInterface,
												szModifiers ) == LT_OK )
	{
		return LT_OK;
	}

	// Only our commands are marked for change notification so just send it to the CommandMgr..

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
// ActiveWorldModel implementation...
//

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ActiveWorldModel::ActiveWorldModel
//
//  PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

ActiveWorldModel::ActiveWorldModel( )
:	WorldModel( ),
	m_nAWMType( AWM_TYPE_STATIC ),
	m_vMoveDir( 0, 0, 0 ),
	m_vOnPos( 0, 0, 0 ),
	m_vOffPos( 0, 0, 0 ),
	m_vPausePos( 0, 0, 0),
	m_fPowerOnTime( AWM_DEFAULT_POWERONTIME ),
	m_fPowerOnMultiplier( 1.0f ),
	m_fMoveDist( 0.0f ),
	m_fOnWaitTm( 0.0f ),
	m_fOffWaitTm( 0.0f ),
	m_fPowerOffTime( AWM_DEFAULT_POWEROFFTIME ),
	m_fMoveStartTm( -10.0f ),
	m_fMoveDelay( AWM_DEFAULT_MOVEDELAY ),
	m_fMoveStopTm( -10.0f ),
	m_dwPropFlags( 0 ),
	m_nCurState( 0 ),
	m_nLastState( 0 ),
	m_nStartingState( 0 ),
	m_nWaveform( AWM_WAVE_LINEAR ),
	m_hActivateObj( LTNULL ),
	m_vOriginalPos( 0.0f, 0.0f, 0.0f ),
	m_fPitch( 0.0f ),
	m_fYaw( 0.0f ),
	m_fRoll( 0.0f ),
	m_hstrRotationPt( LTNULL ),
	m_vRotationPt( 0.0f, 0.0f, 0.0f ),
	m_vRotPtOffset( 0.0f, 0.0f, 0.0f ),
	m_vRotationAngles( 0.0f, 0.0f, 0.0f ),
	m_vOnAngles( 0.0f, 0.0f, 0.0f ),
	m_vInitOnAngles( 0.0f, 0.0f, 0.0f ),
	m_vOffAngles( 0.0f, 0.0f, 0.0f ),
	m_vRotateDir( 0.0f, 0.0f, 0.0f ),
	m_vInitRotDir( 0.0f, 0.0f, 0.0f ),
	m_hstrPowerOnSnd( LTNULL ),
	m_hstrOnSnd( LTNULL ),
	m_hstrPowerOffSnd( LTNULL ),
	m_hstrOffSnd( LTNULL ),
	m_hstrLockedSnd( LTNULL ),
	m_vSoundPos( 0.0f, 0.0f, 0.0f ),
	m_fSoundRadius( 0.0f ),
	m_bLoopSounds( LTFALSE ),
	m_hsndLastSound( LTNULL ),
	m_hstrOnCmd( LTNULL ),
	m_hstrOffCmd( LTNULL ),
	m_hstrPowerOnCmd( LTNULL ),
	m_hstrPowerOffCmd( LTNULL ),
	m_hstrLockedCmd( LTNULL ),
	m_nActivateAlarmLevel( 0 ),
	m_eStimID( kStimID_Unset ),
	m_bSearchedForNode( LTFALSE ),
	m_hUseObjectNode( LTNULL )
{

	m_fOnWaitTm = AWM_DEFAULT_ONWAITTIME;
	m_fOffWaitTm = AWM_DEFAULT_OFFWAITTIME;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ActiveWorldModel::~ActiveWorldModel
//
//  PURPOSE:	Destroy object
//
// ----------------------------------------------------------------------- //

ActiveWorldModel::~ActiveWorldModel( )
{
	// Free strings...

	FREE_HSTRING( m_hstrRotationPt );
	FREE_HSTRING( m_hstrPowerOnSnd );
	FREE_HSTRING( m_hstrOnSnd );
	FREE_HSTRING( m_hstrPowerOffSnd );
	FREE_HSTRING( m_hstrOffSnd );
	FREE_HSTRING( m_hstrLockedSnd );
	FREE_HSTRING( m_hstrOnCmd );
	FREE_HSTRING( m_hstrOffCmd );
	FREE_HSTRING( m_hstrPowerOnCmd );
	FREE_HSTRING( m_hstrPowerOffCmd );
	FREE_HSTRING( m_hstrLockedCmd );

	StopSound( );
	
	if(m_eStimID != kStimID_Unset)
	{
		g_pAIStimulusMgr->RemoveStimulus(m_eStimID);
	}
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ActiveWorldModel::ReadProps
//
//  PURPOSE:	Read in property values
//
// ----------------------------------------------------------------------- //

void ActiveWorldModel::ReadProps( ObjectCreateStruct *pOCS )
{
	_ASSERT( pOCS != NULL );

	// Read base class props first

	WorldModel::ReadProps( pOCS );

	bool		bFlag;
	int32		nType;
	GenericProp	GenProp;

	// Find out what kind of action this AWM does

	if(g_pLTServer->GetPropLongInt("AWMType", &nType ) == LT_OK)
	{
		m_nAWMType = (uint8)nType;
	}

	// Develop Property option flags...

	if( g_pLTServer->GetPropBool( "PlayerActivate", &bFlag ) == LT_OK )
		m_dwPropFlags |= bFlag ? AWM_PROP_PLAYERACTIVATE : 0;

	if( g_pLTServer->GetPropBool( "AIActivate", &bFlag ) == LT_OK )
		m_dwPropFlags |= bFlag ? AWM_PROP_AIACTIVATE : 0;

	if( g_pLTServer->GetPropBool( "StartOn", &bFlag ) == LT_OK )
		m_dwPropFlags |= bFlag ? AWM_PROP_STARTON : 0;

	if( g_pLTServer->GetPropBool( "TriggerOff", &bFlag ) == LT_OK )
		m_dwPropFlags |= bFlag ? AWM_PROP_TRIGGEROFF  : 0;

	if( g_pLTServer->GetPropBool( "RemainOn", &bFlag ) == LT_OK )
		m_dwPropFlags |= bFlag ? AWM_PROP_REMAINON : 0;

	if( g_pLTServer->GetPropBool( "ForceMove", &bFlag ) == LT_OK )
		m_dwPropFlags |= bFlag ? AWM_PROP_FORCEMOVE : 0;

	if( g_pLTServer->GetPropBool( "ForceMoveOn", &bFlag ) == LT_OK )
		m_dwPropFlags |= bFlag ? AWM_PROP_FORCEMOVEON : 0;

	if( g_pLTServer->GetPropBool( "ForceMoveOff", &bFlag ) == LT_OK )
		m_dwPropFlags |= bFlag ? AWM_PROP_FORCEMOVEOFF : 0;

	if( g_pLTServer->GetPropBool( "Locked", &bFlag ) == LT_OK )
		m_dwPropFlags |= bFlag ? AWM_PROP_LOCKED : 0;

	if( g_pLTServer->GetPropBool( "RotateAway", &bFlag ) == LT_OK )
		m_dwPropFlags |= bFlag ? AWM_PROP_ROTATEAWAY : 0;

	// Get the waveform...
	
	if( g_pLTServer->GetPropGeneric( "Waveform", &GenProp ) == LT_OK )
	{
		if( GenProp.m_String[0] )
		{
			for( int i = 0; i <= AWM_WAVE_MAXTYPES; i++ )
			{
				if( !_stricmp( GenProp.m_String, c_aWaveTypes[i] ) )
				{
					m_nWaveform = i;
					break;
				}
			}	
		}
	}
	
	// Get the activate type...

	if( m_dwPropFlags & AWM_PROP_PLAYERACTIVATE )
	{
		if( g_pLTServer->GetPropGeneric( "ActivateType", &GenProp ) == LT_OK )
		{
			if( GenProp.m_String[0] )
			{
				m_ActivateTypeHandler.SetActivateType( GenProp.m_String );
			}
		}
	}


	// Read sound options...

	if( g_pLTServer->GetPropGeneric( "PowerOnSound", &GenProp ) == LT_OK )
	{
		if( GenProp.m_String[0] )
		{
			m_hstrPowerOnSnd = g_pLTServer->CreateString( GenProp.m_String );
		}
	}

	if( g_pLTServer->GetPropGeneric( "OnSound", &GenProp ) == LT_OK )
	{
		if( GenProp.m_String[0] )
		{
			m_hstrOnSnd = g_pLTServer->CreateString( GenProp.m_String );
		}
	}

	if( g_pLTServer->GetPropGeneric( "PowerOffSound", &GenProp ) == LT_OK )
	{
		if( GenProp.m_String[0] )
		{
			m_hstrPowerOffSnd = g_pLTServer->CreateString( GenProp.m_String );
		}
	}

	if( g_pLTServer->GetPropGeneric( "OffSound", &GenProp ) == LT_OK )
	{
		if( GenProp.m_String[0] )
		{
			m_hstrOffSnd = g_pLTServer->CreateString( GenProp.m_String );
		}
	}

	if( g_pLTServer->GetPropGeneric( "LockedSound", &GenProp ) == LT_OK )
	{
		if( GenProp.m_String[0] )
		{
			m_hstrLockedSnd = g_pLTServer->CreateString( GenProp.m_String );
		}
	}

	g_pLTServer->GetPropVector( "SoundPos", &m_vSoundPos );
	g_pLTServer->GetPropReal( "SoundRadius", &m_fSoundRadius );
	g_pLTServer->GetPropBool( "LoopSounds", &bFlag );
	m_dwPropFlags |= bFlag ? AWM_PROP_LOOPSOUNDS : 0;


	// Read Commands to send...

	if( g_pLTServer->GetPropGeneric( "OnCommand", &GenProp ) == LT_OK )
	{
		if( GenProp.m_String[0] )
		{
			m_hstrOnCmd = g_pLTServer->CreateString( GenProp.m_String );
		}
	}

	if( g_pLTServer->GetPropGeneric( "OffCommand", &GenProp ) == LT_OK )
	{
		if( GenProp.m_String[0] )
		{
			m_hstrOffCmd = g_pLTServer->CreateString( GenProp.m_String );
		}
	}

	if( g_pLTServer->GetPropGeneric( "PowerOnCommand", &GenProp ) == LT_OK )
	{
		if( GenProp.m_String[0] )
		{
			m_hstrPowerOnCmd = g_pLTServer->CreateString( GenProp.m_String );
		}
	}

	if( g_pLTServer->GetPropGeneric( "PowerOffCommand", &GenProp ) == LT_OK )
	{
		if( GenProp.m_String[0] )
		{
			m_hstrPowerOffCmd = g_pLTServer->CreateString( GenProp.m_String );
		}
	}

	if( g_pLTServer->GetPropGeneric( "LockedCommand", &GenProp ) == LT_OK )
	{
		if( GenProp.m_String[0] )
		{
			m_hstrLockedCmd = g_pLTServer->CreateString( GenProp.m_String );
		}
	}

	
	// Read disturbance props...

	if( g_pLTServer->GetPropGeneric( "ActivateAlarmLevel", &GenProp ) == LT_OK )
	{
		m_nActivateAlarmLevel = (uint32)GenProp.m_Long;
	}
	
  

	// Read movement props...

	g_pLTServer->GetPropVector( "MoveDir", &m_vMoveDir );
	if( m_vMoveDir.MagSqr() < 0.0001f )
		m_vMoveDir.Init( 0.0f, 1.0f, 0.0f );

	g_pLTServer->GetPropReal( "MoveDist", &m_fMoveDist );
	g_pLTServer->GetPropReal( "PowerOnTime", &m_fPowerOnTime );
	g_pLTServer->GetPropReal( "PowerOffTime", &m_fPowerOffTime );
	g_pLTServer->GetPropReal( "MoveDelay", &m_fMoveDelay );
	g_pLTServer->GetPropReal( "OnWaitTime", &m_fOnWaitTm );
	g_pLTServer->GetPropReal( "OffWaitTime", &m_fOffWaitTm );

	// Read rotation props...

	g_pLTServer->GetPropVector( "RotationAngles", &m_vRotationAngles );
	
	// Initially set the rotation point incase we have no point object 

	m_vRotationPt = pOCS->m_Pos;

	char szTemp[MAX_CS_FILENAME_LEN];
	g_pLTServer->GetPropString( "RotationPoint", szTemp, MAX_CS_FILENAME_LEN );
	if( szTemp[0] )
	{
// tagRP: TODO - Get this working for Points inside a PreFab
		/*
		char *szPreFabRef = strtok( pOCS->m_Name, "." );

		// If this is a prefab object... Add the ref name to the RotationPoint name

		if( szPreFabRef[0] )
		{
			char str[MAX_CS_FILENAME_LEN];

			SAFE_STRCPY( str, szPreFabRef );
			strcat( str, "." );
			strcat( str, szTemp );
			SAFE_STRCPY( szTemp, str );
		}
		*/

		m_hstrRotationPt = g_pLTServer->CreateString( szTemp );
	}
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ActiveWorldModel::PostReadProp
//
//  PURPOSE:	Initialize data after the property values are read
//
// ----------------------------------------------------------------------- //

void ActiveWorldModel::PostReadProp( ObjectCreateStruct *pOCS )
{
	_ASSERT( pOCS != NULL );

	// Init base class data first

	WorldModel::PostReadProp( pOCS );

	pOCS->m_Flags |= FLAG_TOUCH_NOTIFY | FLAG_GOTHRUWORLD;

	if (m_nAWMType == AWM_TYPE_ROTATING)
		pOCS->m_Flags2 |= FLAG2_DISABLEPREDICTION;

	m_fPowerOffTime = ( m_fPowerOffTime > 0.0f ) ? m_fPowerOffTime : m_fPowerOnTime;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ActiveWorldModel::OnObjectCreated
//
//  PURPOSE:	Our object is now created... Init some more data
//
// ----------------------------------------------------------------------- //

void ActiveWorldModel::OnObjectCreated( )
{
	// Send to base class first

	WorldModel::OnObjectCreated();

	// Get the user flags from the object as they may have been adjusted by our parent...
	uint32	dwUsrFlags = 0;
	g_pCommonLT->GetObjectFlags( m_hObject, OFT_User, dwUsrFlags );

	if ( ( fabs(m_fMoveDist) >= 0.01f ) || ( m_vRotationAngles.LengthSquared() >= 0.01f )  )
	{
		dwUsrFlags |= USRFLG_MOVEABLE;
	}

	// See if we can be activated or not

	if( m_dwPropFlags & AWM_PROP_PLAYERACTIVATE )
	{
		dwUsrFlags |= USRFLG_CAN_ACTIVATE;

		// If we can be activated create the special fx message...

		m_ActivateTypeHandler.SetDisabled( !!(m_dwPropFlags & AWM_PROP_LOCKED), false );
		m_ActivateTypeHandler.CreateActivateTypeMsg();
	}

	// If we are forced to move then crush objects, otherwise don't

	if( !(m_dwPropFlags & AWM_PROP_FORCEMOVE) && !(m_dwPropFlags & AWM_PROP_FORCEMOVEON) &&
		!(m_dwPropFlags & AWM_PROP_FORCEMOVEOFF ))
	{
		dwUsrFlags |= USRFLG_CANT_CRUSH;
	}

	g_pCommonLT->SetObjectFlags( m_hObject, OFT_User, dwUsrFlags, USRFLG_MOVEABLE | USRFLG_CAN_ACTIVATE | USRFLG_CANT_CRUSH );

	g_pLTServer->GetObjectPos( m_hObject, &m_vOriginalPos );

	// Calculate on and off positions and other data based on type...
	
	switch( m_nAWMType )
	{
		case AWM_TYPE_SLIDING:
		{
			// Current Position is Off position 

			m_vOffPos = m_vOriginalPos;

			// Set movement locally rather than globaly

			LTRotation	rRot = m_hackInitialRot;
			LTMatrix	mMat;
			
			rRot.ConvertToMatrix( mMat );
			m_vMoveDir = mMat * m_vMoveDir;
			m_vMoveDir.Normalize();

			// Now get the On position

			LTVector vTemp = m_vMoveDir * m_fMoveDist;
			
			m_vOnPos = m_vOffPos + vTemp;
		}
		break;

		case AWM_TYPE_ROTATING:
		{
			m_vOffAngles.Init( m_fPitch, m_fYaw, m_fRoll );

			m_vOnAngles.x = MATH_DEGREES_TO_RADIANS( m_vRotationAngles.x ) + m_fPitch;
			m_vOnAngles.y = MATH_DEGREES_TO_RADIANS( m_vRotationAngles.y ) + m_fYaw;
			m_vOnAngles.z = MATH_DEGREES_TO_RADIANS( m_vRotationAngles.z ) + m_fRoll;

			// The AWM must rotate at least 1 degree

			const LTFLOAT c_fMinDelta = MATH_DEGREES_TO_RADIANS( 1.0f );

			// Determine direction to rotate in X...

			LTFLOAT fOffset = m_vOffAngles.x - m_vOnAngles.x;
			if( fOffset > c_fMinDelta )
			{
				m_vRotateDir.x = -1.0f;
			}
			else if( fOffset < c_fMinDelta )
			{
				m_vRotateDir.x = 1.0f;
			}

			// Determine direction to rotate in Y...

			fOffset = m_vOffAngles.y - m_vOnAngles.y;
			if( fOffset > c_fMinDelta )
			{
				m_vRotateDir.y = -1.0f;
			}
			else if( fOffset < c_fMinDelta )
			{
				m_vRotateDir.y = 1.0f;
			}

			// Determine direction to rotate in Z...

			fOffset = m_vOffAngles.z - m_vOnAngles.z;
			if( fOffset > c_fMinDelta )
			{
				m_vRotateDir.z = -1.0f;
			}
			else if( fOffset < c_fMinDelta )
			{
				m_vRotateDir.z = 1.0f;
			}

			// Save our initial values...

			m_vInitOnAngles	= m_vOnAngles;
			m_vInitRotDir	= m_vRotateDir;

		}
		break;

		case AWM_TYPE_STATIC:
		default :
			break;
	}

	if( m_dwPropFlags & AWM_PROP_STARTON )
	{
		SetPowerOn();
	}
	else
		SetOff( LTTRUE );

	// Record the starting state, so that we know when to place visual disturbance stimulus.

	m_nStartingState = m_nCurState;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ActiveWorldModel::OnAllObjectsCreated
//
//  PURPOSE:	Now that all objects are loaded we can look for our rotation point object
//
// ----------------------------------------------------------------------- //

void ActiveWorldModel::OnEveryObjectCreated( )
{
	// Send to base class first

	WorldModel::OnEveryObjectCreated();

	// See if we have a rotation point object...

	if( m_hstrRotationPt )
	{
		ObjArray<HOBJECT, 1> objArray;
		g_pLTServer->FindNamedObjects( g_pLTServer->GetStringData( m_hstrRotationPt ), objArray );

		if( objArray.NumObjects() > 0 )
		{
			g_pLTServer->GetObjectPos( objArray.GetObject(0), &m_vRotationPt );

			if( IsKindOf( objArray.GetObject(0), "Point" ) )
			{
				// We have a valid rotation point object now remove it from the world

				g_pLTServer->RemoveObject( objArray.GetObject(0) );
			}
			else
				g_pLTServer->CPrint( "ActiveWorldModel: RotationPointObject NOT a Point!!" );
		}

		// Clean up...

		FREE_HSTRING( m_hstrRotationPt );
	}

	// Calculate the rotation point offset (allows for the
	// door to be movied (keyframed) and still rotate correctly... maybe?

	m_vRotPtOffset = m_vOriginalPos - m_vRotationPt;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ActiveWorldModel::OnUpdate
//
//  PURPOSE:	Handle our update
//
// ----------------------------------------------------------------------- //

void ActiveWorldModel::OnUpdate( const LTFLOAT &fCurTime )
{
	// No need to call WorldModel::OnUpdate()

	SetNextUpdate( UPDATE_NEXT_FRAME );

	switch( m_nCurState )
	{
		case AWM_STATE_ON:
		{
			UpdateOn( fCurTime );
		}
		break;

		case AWM_STATE_POWERON:
		{
			UpdatePowerOn( fCurTime );
		}
		break;

		case AWM_STATE_OFF:
		{
			UpdateOff( fCurTime );
		}
		break;

		case AWM_STATE_POWEROFF:
		{
			UpdatePowerOff( fCurTime );
		}
		break;

		case AWM_STATE_PAUSE:
		default : break;
	}
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ActiveWorldModel::UpdateOn
//
//  PURPOSE:	Handle updating the On state
//
// ----------------------------------------------------------------------- //

void ActiveWorldModel::UpdateOn( const LTFLOAT &fCurTime )
{
	// Immediately try turnning off

	SetPowerOff( );
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ActiveWorldModel::UpdatePowerOn
//
//  PURPOSE:	Handel updating the PowerOn state
//
// ----------------------------------------------------------------------- //

void ActiveWorldModel::UpdatePowerOn( const LTFLOAT &fCurTime )
{
	if( fCurTime < m_fMoveStartTm ) return;

	LTVector	vNewPos;
	LTRotation	rNewRot;
	LTBOOL		bDoneInX = LTFALSE;
	LTBOOL		bDoneInY = LTFALSE;
	LTBOOL		bDoneInZ = LTFALSE;

	LTVector	vOldAngles( m_fPitch, m_fYaw, m_fRoll );
	LTVector	vOldPos;

	g_pLTServer->GetObjectPos( m_hObject, &vOldPos );

	LTFLOAT		fPowerOnTime = m_fPowerOnTime * m_fPowerOnMultiplier;
	LTFLOAT		fPercent = 1.0f;
	if( fPowerOnTime > 0.001f )
	{
		fPercent = m_fCurTm / fPowerOnTime;
	}
	fPercent = Clamp( fPercent, 0.0f, 1.0f );

	// Calculate new pitch yaw and roll...

	bDoneInX = CalcAngle( m_fPitch,	m_vOffAngles.x, m_vOnAngles.x, m_vRotateDir.x, fPowerOnTime, fPercent );
	bDoneInY = CalcAngle( m_fYaw,	m_vOffAngles.y, m_vOnAngles.y, m_vRotateDir.y, fPowerOnTime, fPercent );
	bDoneInZ = CalcAngle( m_fRoll,	m_vOffAngles.z, m_vOnAngles.z, m_vRotateDir.z, fPowerOnTime, fPercent );

	if( !CalculateNewPosRot( vNewPos, rNewRot, m_vOnPos, fPowerOnTime, fPercent, 
		( !(m_dwPropFlags & AWM_PROP_FORCEMOVE) && !(m_dwPropFlags & AWM_PROP_FORCEMOVEON))))
	{
		// Restore our angles...

		m_fPitch = vOldAngles.x;
		m_fYaw	 = vOldAngles.y;
		m_fRoll	 = vOldAngles.z;

		return;
	}

	g_pLTServer->MoveObject( m_hObject, &vNewPos );
	
	// Check to see if we actually moved anywhere...

	LTVector	vPos;
	g_pLTServer->GetObjectPos( m_hObject, &vPos );
	if( !vPos.NearlyEquals( vNewPos, MATH_EPSILON ) )
	{
		// Restore our angles...

		m_fPitch = vOldAngles.x;
		m_fYaw	 = vOldAngles.y;
		m_fRoll	 = vOldAngles.z;

		return;
	}
	

	g_pLTServer->RotateObject( m_hObject, &rNewRot );
	
	// See if we are done updating the PowerOn state

	if( (m_nAWMType == AWM_TYPE_SLIDING) && (vNewPos == m_vOnPos) )
	{
		SetOn();
	}
	else if( (m_nAWMType == AWM_TYPE_ROTATING) && (bDoneInX && bDoneInY && bDoneInZ) )
	{
		SetOn();
	}

	m_fCurTm += g_pLTServer->GetFrameTime();
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ActiveWorldModel::UpdateOff
//
//  PURPOSE:	Handel updating the Off state
//
// ----------------------------------------------------------------------- //

void ActiveWorldModel::UpdateOff( const LTFLOAT &fCurTime )
{
	// Do nothing, we should never get here

	_ASSERT( LTFALSE );
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ActiveWorldModel::UpdatePowerOff
//
//  PURPOSE:	Handel updating the PowerOff state
//
// ----------------------------------------------------------------------- //

void ActiveWorldModel::UpdatePowerOff( const LTFLOAT &fCurTime )
{
	if( fCurTime < m_fMoveStartTm + m_fMoveDelay ) return;

	LTVector	vNewPos;
	LTRotation	rNewRot;
    LTBOOL		bDoneInX = LTFALSE;
    LTBOOL		bDoneInY = LTFALSE;
    LTBOOL		bDoneInZ = LTFALSE;

    LTVector	vOldAngles(m_fPitch, m_fYaw, m_fRoll);
	LTVector	vOldPos;

	g_pLTServer->GetObjectPos( m_hObject, &vOldPos );

	LTFLOAT	fPercent = 1.0f;
	if( m_fPowerOffTime > 0.001f )
	{
		fPercent = m_fCurTm / m_fPowerOffTime;
	}
	fPercent = Clamp( fPercent, 0.0f, 1.0f );

	// Calculate new pitch, yaw, and roll...

	bDoneInX = CalcAngle(m_fPitch, m_vOnAngles.x, m_vOffAngles.x, -m_vRotateDir.x, m_fPowerOffTime, fPercent);
	bDoneInY = CalcAngle(m_fYaw,   m_vOnAngles.y, m_vOffAngles.y, -m_vRotateDir.y, m_fPowerOffTime, fPercent);
	bDoneInZ = CalcAngle(m_fRoll,  m_vOnAngles.z, m_vOffAngles.z, -m_vRotateDir.z, m_fPowerOffTime, fPercent);

	if( !CalculateNewPosRot( vNewPos, rNewRot, m_vOffPos, m_fPowerOffTime, fPercent, 
		( !(m_dwPropFlags & AWM_PROP_FORCEMOVE) && !(m_dwPropFlags & AWM_PROP_FORCEMOVEOFF))))
	{
		// Restore our angles...

		m_fPitch = vOldAngles.x;
		m_fYaw	 = vOldAngles.y;
		m_fRoll	 = vOldAngles.z;

		return;
	}

	g_pLTServer->MoveObject( m_hObject, &vNewPos );
	
	// Check to see if we actually moved anywhere...

	LTVector vPos;
	g_pLTServer->GetObjectPos( m_hObject, &vPos );
	if( !vPos.NearlyEquals( vNewPos, MATH_EPSILON ) )
	{
		// Restore our angles...

		m_fPitch = vOldAngles.x;
		m_fYaw	 = vOldAngles.y;
		m_fRoll	 = vOldAngles.z;

		return;
	}

	g_pLTServer->RotateObject( m_hObject, &rNewRot );	

	if( (m_nAWMType == AWM_TYPE_SLIDING) && (vNewPos == m_vOffPos) )
	{
		SetOff();
	}
	else if( (m_nAWMType == AWM_TYPE_ROTATING) && (bDoneInX && bDoneInY && bDoneInZ) )
	{
		SetOff();
	}

	m_fCurTm += g_pLTServer->GetFrameTime();

}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ActiveWorldModel::OnTrigger
//
//  PURPOSE:	Handle recieving a trigger msg from another object
//
// ----------------------------------------------------------------------- //

bool ActiveWorldModel::OnTrigger( HOBJECT hSender, const CParsedMsg &cMsg )
{
	static CParsedMsg::CToken s_cTok_Activate("ACTIVATE");
	static CParsedMsg::CToken s_cTok_Trigger("TRIGGER");
	static CParsedMsg::CToken s_cTok_Lock("LOCK");
	static CParsedMsg::CToken s_cTok_Unlock("UNLOCK");
	static CParsedMsg::CToken s_cTok_On("ON");
	static CParsedMsg::CToken s_cTok_Off("OFF");
	static CParsedMsg::CToken s_cTok_SetPowerOnMultiplier("SETPOWERONMULTIPLIER");
	static CParsedMsg::CToken s_cTok_Pause("PAUSE");
	static CParsedMsg::CToken s_cTok_Resume("RESUME");
	
	// Let base class handle the message first

	bool bResult = WorldModel::OnTrigger( hSender, cMsg );

	if( cMsg.GetArg(0) == s_cTok_Activate )
	{
		Activate( hSender );
	}
	else if( cMsg.GetArg(0) == s_cTok_Trigger )
	{
		HandleTriggerMsg();
	}
	else if( cMsg.GetArg(0) == s_cTok_Lock )
	{
		HandleLock( LTTRUE );
	}
	else if( cMsg.GetArg(0) == s_cTok_Unlock )
	{
		HandleLock( LTFALSE );
	}
	else if( cMsg.GetArg(0) == s_cTok_On )
	{
		if( m_nCurState == AWM_STATE_OFF || m_nCurState == AWM_STATE_POWEROFF )
		{
			SetActiveObj( hSender );
			HandleTriggerMsg();
		}
	}
	else if( cMsg.GetArg(0) == s_cTok_Off )
	{
		if( m_nCurState == AWM_STATE_ON || m_nCurState == AWM_STATE_POWERON )
		{
			SetActiveObj( hSender );
			HandleTriggerMsg();
		}
	}
	else if( cMsg.GetArg(0) == s_cTok_SetPowerOnMultiplier )
	{
		if( cMsg.GetArgCount() > 1 )
		{
			m_fPowerOnMultiplier = (LTFLOAT)atof(cMsg.GetArg(1));
		}
	}
	else if( cMsg.GetArg(0) == s_cTok_Pause )
	{
		HandlePause( );
	}
	else if( cMsg.GetArg(0) == s_cTok_Resume )
	{
		HandleResume( );
	}
	else
		return bResult;

	return true;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ActiveWorldModel::HandleTriggerMsg
//
//  PURPOSE:	Handles a trigger command
//
// ----------------------------------------------------------------------- //

void ActiveWorldModel::HandleTriggerMsg( )
{
	// Are we locked...
	
	if( m_dwPropFlags & AWM_PROP_LOCKED )
	{
		if( !g_pKeyMgr->CanCharacterControlObject( m_hActivateObj, m_hObject ))
		{
			// Play a sound if one was specified...

			if( m_hstrLockedSnd )
				StartSound( m_hstrLockedSnd, LTFALSE );
			
			// Send a command if we have one...

			if( m_hstrLockedCmd )
			{
				const char	*pCmd = g_pLTServer->GetStringData( m_hstrLockedCmd );

				if( pCmd && g_pCmdMgr->IsValidCmd( pCmd ))
				{
					g_pCmdMgr->Process( pCmd, m_hObject, m_hActivateObj );
				}
			}

			// We are locked so don't try and change states

			return;			
		}
	}

	// We are not locked so do our thing...

	// Hanlde a state change...

	float fCurTime = g_pLTServer->GetTime();

	switch( m_nCurState )
	{
		case AWM_STATE_OFF:
		{
			if( fCurTime >= m_fMoveStopTm + m_fOffWaitTm )
			{
				// We are now going to turn on

				SetPowerOn();
			}
		}
		break;

		case AWM_STATE_POWEROFF:
		{
			ChangeDir();
		}	
		break;

		case AWM_STATE_POWERON:
		{
			ChangeDir();
		}	
		break;

		case AWM_STATE_ON:
		{
			if( m_dwPropFlags & AWM_PROP_TRIGGEROFF )
			{
				if( fCurTime >= m_fMoveStopTm + m_fOnWaitTm )
				{
					// We are going to turn off

					SetPowerOff();
				}
			}
			else
			{
				 SetOn( LTTRUE );  // call to reset the times
			}
		}
		break;

		default : break;
	};

}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ActiveWorldModel::HandlePause
//
//  PURPOSE:	Handle a PAUSE command
//
// ----------------------------------------------------------------------- //

void ActiveWorldModel::HandlePause( )
{
	// Only PAUSE if we are currently in a moving state...

	if( m_nCurState == AWM_STATE_POWERON || m_nCurState == AWM_STATE_POWEROFF )
	{
		m_nLastState = m_nCurState;
		m_nCurState = AWM_STATE_PAUSE;

		g_pLTServer->GetObjectPos( m_hObject, &m_vPausePos );
	}
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ActiveWorldModel::HandleResume
//
//  PURPOSE:	Handle a RESUME command
//
// ----------------------------------------------------------------------- //

void ActiveWorldModel::HandleResume( )
{
	if( m_nCurState == AWM_STATE_PAUSE )
	{
		m_nCurState = m_nLastState;
	
		// Recalculate the on/off pos incase we were moved by a keyframer (movement only?? )

		LTVector	vPos;
		g_pLTServer->GetObjectPos( m_hObject, &vPos );
			
		LTVector	vDelta = m_vPausePos - vPos;
								
		m_vOriginalPos	-= vDelta;
		m_vOffPos		-= vDelta;
		m_vOnPos		-= vDelta;

		if( m_nAWMType == AWM_TYPE_ROTATING ) 
		{
			// Recalculate the point we rotate around incase we were moved by a keyframer...

			if( m_nCurState == AWM_STATE_OFF )
			{
				m_vRotationPt = m_vOriginalPos - m_vRotPtOffset;
			}
		}
		
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ActiveWorldModel::HandleLock
//
//  PURPOSE:	Handle a LOCK or UNLOCK command
//
// ----------------------------------------------------------------------- //

void ActiveWorldModel::HandleLock(LTBOOL bLock)
{
	if( bLock )
	{
		m_dwPropFlags |= AWM_PROP_LOCKED;
	}
	else
	{
		m_dwPropFlags &= ~AWM_PROP_LOCKED;
	}

	m_ActivateTypeHandler.SetDisabled( !!bLock );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ActiveWorldModel::SetPowerOn
//
//  PURPOSE:	Sets the AWM to the PowerOn state
//
// ----------------------------------------------------------------------- //

void ActiveWorldModel::SetPowerOn( )
{
	// Recalculate the on/off pos incase we were moved by a keyframer (movement only?? )

	if( m_nCurState == AWM_STATE_OFF )
	{
		g_pLTServer->GetObjectPos( m_hObject, &m_vOriginalPos );

		m_vOffPos = m_vOriginalPos;
		LTVector vTemp = m_vMoveDir * m_fMoveDist;
		m_vOnPos = m_vOffPos + vTemp;
	}

	// Flip direction and angles if we want to rotate away form the active obj... 

	if( m_nAWMType == AWM_TYPE_ROTATING ) 
	{
		// Recalculate the point we rotate around incase we were moved by a keyframer...

		if( m_nCurState == AWM_STATE_OFF )
		{
			m_vRotationPt = m_vOriginalPos - m_vRotPtOffset;
		}

		if( (m_dwPropFlags & AWM_PROP_ROTATEAWAY) && m_hActivateObj )
		{
			// To calculate the correct direction to open, calculate the position
			// of the AWM in both possible open positions.  Whichever direction
			// moves the AWM to the farthest position away from the activate
			// object's position is the direction we should rotate the AWM.

			LTVector	vObjPos;
			g_pLTServer->GetObjectPos( m_hActivateObj, &vObjPos );

			LTVector	vOldAngles( m_fPitch, m_fYaw, m_fRoll );
			
			const LTFLOAT	cf100Percent = 1.0f;

			// Calculate the AWM's position if it rotated normally...

			m_fPitch	= m_vInitOnAngles.x == 0.0f ? 0.0f : m_vInitOnAngles.x > 0.0f ? 1 : -1;
			m_fYaw		= m_vInitOnAngles.y == 0.0f ? 0.0f : m_vInitOnAngles.y > 0.0f ? 1 : -1;
			m_fRoll		= m_vInitOnAngles.z == 0.0f ? 0.0f : m_vInitOnAngles.y > 0.0f ? 1 : -1;

			LTVector	vTestPos1, vTestPos2;
			LTRotation	rTestRot;

			CalculateNewPosRot( vTestPos1, rTestRot, LTVector(0,0,0), m_fPowerOnTime, cf100Percent );

			// Calculate the AWM's position if it rotated in opposite direction...

			m_fPitch	= m_vInitOnAngles.x == 0.0f ? 0.0f : m_vInitOnAngles.x > 0.0f ? -1 : 1;
			m_fYaw		= m_vInitOnAngles.y == 0.0f ? 0.0f : m_vInitOnAngles.y > 0.0f ? -1 : 1;
			m_fRoll		= m_vInitOnAngles.z == 0.0f ? 0.0f : m_vInitOnAngles.y > 0.0f ? -1 : 1;

			CalculateNewPosRot( vTestPos2, rTestRot, LTVector(0,0,0), m_fPowerOnTime, cf100Percent );

			// Restore the real angles...

			m_fPitch	= vOldAngles.x;
			m_fYaw		= vOldAngles.y;
			m_fRoll		= vOldAngles.z;

			// Set the direction we want...

			if( vTestPos1.DistSqr( vObjPos ) < vTestPos2.DistSqr( vObjPos ))
			{
				// Opposite of normal rotation...

				m_vOnAngles.x		= -m_vInitOnAngles.x;
				m_vOnAngles.y		= -m_vInitOnAngles.y; 
				m_vOnAngles.z		= -m_vInitOnAngles.z; 

				m_vRotateDir		= -m_vInitRotDir; 
			}
			else
			{
				// Normal rotation...

				m_vOnAngles			= m_vInitOnAngles;
				m_vRotateDir		= m_vInitRotDir;
			}
		}
	}

	StartSound( m_hstrPowerOnSnd, m_dwPropFlags & AWM_PROP_LOOPSOUNDS );

	SetNextUpdate( UPDATE_NEXT_FRAME );

	// Set the new state

	m_nCurState = AWM_STATE_POWERON;

	// Send a command if we have one...

	if( m_hstrPowerOnCmd )
	{
		const char	*pCmd = g_pLTServer->GetStringData( m_hstrPowerOnCmd );

		if( pCmd && g_pCmdMgr->IsValidCmd( pCmd ))
		{
			g_pCmdMgr->Process( pCmd, m_hObject, m_hActivateObj );
		}
	}
	
	m_fMoveStartTm = g_pLTServer->GetTime() + m_fMoveDelay;
	m_fCurTm = 0.0f;

	// Register disturbance stimulus for sound and visual.

	RegisterDisturbanceStimulus(LTTRUE);

	// Set the on state for our activate object.

	m_ActivateTypeHandler.SetState( ACTIVATETYPE::eOn );
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ActiveWorldModel::SetOn
//
//  PURPOSE:	Sets the AWM to the On state
//
// ----------------------------------------------------------------------- //

void ActiveWorldModel::SetOn( LTBOOL bInitialize )
{
	if( m_dwPropFlags & AWM_PROP_TRIGGEROFF || m_dwPropFlags & AWM_PROP_REMAINON )
	{
		SetNextUpdate( UPDATE_NEVER );
	}
	else
	{
		// We are going to auto shut off

		SetNextUpdate( m_fOnWaitTm + UPDATE_NEXT_FRAME );
	}

	// Set the new state

	m_nCurState = AWM_STATE_ON;

	// Clear any multiplier applied to the movement.

	m_fPowerOnMultiplier = 1.0f;

	if( !bInitialize )
	{
		StartSound( m_hstrOnSnd, m_dwPropFlags & AWM_PROP_LOOPSOUNDS );

		// Send a command if we have one...

		if( m_hstrOnCmd )
		{
			const char	*pCmd = g_pLTServer->GetStringData( m_hstrOnCmd );

			if( pCmd && g_pCmdMgr->IsValidCmd( pCmd ))
			{
				g_pCmdMgr->Process( pCmd, m_hObject, m_hActivateObj );
			}
		}

		m_fMoveStopTm = g_pLTServer->GetTime();
	}

	// Register disturbance stimulus for visual.

	RegisterDisturbanceStimulus(LTFALSE);

	// Set the on state for our activate object.

	m_ActivateTypeHandler.SetState( ACTIVATETYPE::eOn );
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ActiveWorldModel::SetPowerOff
//
//  PURPOSE:	Sets the AWM to the PowerOff state
//
// ----------------------------------------------------------------------- //

void ActiveWorldModel::SetPowerOff( )
{
	// Recalculate the on/off pos incase we were moved by a keyframer (movement only?? )

	if( m_nCurState == AWM_STATE_ON )
	{
		g_pLTServer->GetObjectPos( m_hObject, &m_vOnPos );

		LTVector vTemp = m_vMoveDir * m_fMoveDist;
		m_vOffPos = m_vOnPos - vTemp;
	}

	StartSound( m_hstrPowerOffSnd, m_dwPropFlags & AWM_PROP_LOOPSOUNDS );

	SetNextUpdate( UPDATE_NEXT_FRAME );
	m_nCurState = AWM_STATE_POWEROFF;
	
	// Send a command if we have one...

	if( m_hstrPowerOffCmd )
	{
		const char	*pCmd = g_pLTServer->GetStringData( m_hstrPowerOffCmd );

		if( pCmd && g_pCmdMgr->IsValidCmd( pCmd ))
		{
			g_pCmdMgr->Process( pCmd, m_hObject, m_hActivateObj );
		}
	}

	m_fMoveStartTm = g_pLTServer->GetTime() + m_fMoveDelay;
	m_fCurTm = 0.0f;

	// Register disturbance stimulus for sound and visual.

	RegisterDisturbanceStimulus(LTTRUE);

	// Set the off state for our activate object.

	m_ActivateTypeHandler.SetState( ACTIVATETYPE::eOff );
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ActiveWorldModel::SetOff
//
//  PURPOSE:	Sets the AWM to the Off state
//
// ----------------------------------------------------------------------- //

void ActiveWorldModel::SetOff( LTBOOL bInitialized )
{
	// Set the state and stop updating before we send any commands...

	SetNextUpdate( UPDATE_NEVER );
	m_nCurState = AWM_STATE_OFF;

	if( !bInitialized )
	{
		if( m_nAWMType == AWM_TYPE_ROTATING )
		{
			m_vOnAngles	= m_vInitOnAngles;
			m_vRotateDir = m_vInitRotDir;
		}

		StartSound( m_hstrOffSnd, m_dwPropFlags & AWM_PROP_LOOPSOUNDS );

		// Send a command if we have one...

		if( m_hstrOffCmd )
		{
			const char	*pCmd = g_pLTServer->GetStringData( m_hstrOffCmd );

			if( pCmd && g_pCmdMgr->IsValidCmd( pCmd ))
			{
				g_pCmdMgr->Process( pCmd, m_hObject, m_hActivateObj );
			}
		}

		m_fMoveStopTm = g_pLTServer->GetTime();
	}

	// Register disturbance stimulus for visual.

	RegisterDisturbanceStimulus(LTFALSE);

	// Set the off state for our activate object.

	m_ActivateTypeHandler.SetState( ACTIVATETYPE::eOff );
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ActiveWorldModel::GetWaveformValue
//
//  PURPOSE:	Develop a value based on the desired speed and the percent
//				completed through the AWM's wave
//
//	NOTE:		The equation used to determine fNewSpeed is a sinusoidal function
//				of time:
//						  v(t) = pi*D/(2*T) * sin(pi*t/T) 
//				In our case v(t) is fNewSpeed, D/T is fRate and t/T is fPercent.
// ----------------------------------------------------------------------- //

LTFLOAT ActiveWorldModel::GetWaveformValue( LTFLOAT fRate, LTFLOAT fPercent )
{
	// If we have linear motion just return the speed

	if( m_nWaveform == AWM_WAVE_LINEAR ) return fRate;

	LTFLOAT	fNewSpeed;
	LTFLOAT	fScalePercent = fRate * 0.001f;
	
	// Get speed based on waveform

	switch( m_nWaveform )
	{
		case AWM_WAVE_SINE:
		{
			fNewSpeed = MATH_HALFPI * fRate * (LTFLOAT)sin( fPercent * MATH_PI );
		}
		break; 

		case AWM_WAVE_SLOWOFF:
		{
			fNewSpeed = MATH_HALFPI * fRate * (LTFLOAT)cos( fPercent * MATH_HALFPI ) - fScalePercent;
		}
		break;

		case AWM_WAVE_SLOWON:
		{
			fNewSpeed = MATH_HALFPI * fRate * (LTFLOAT)sin( fPercent * MATH_HALFPI ) + fScalePercent;
		}
		break;

		default : 
			fNewSpeed = 0.0f;
		break;
	}

	return fNewSpeed;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ActiveWorldModel::ChangeDir
//
//  PURPOSE:	Move AWM in the opposite direction... change state
//
// ----------------------------------------------------------------------- //

void ActiveWorldModel::ChangeDir( )
{
	// If the force move option is set don't change

	if( m_dwPropFlags & AWM_PROP_FORCEMOVE ) return;

	if(( m_nCurState == AWM_STATE_POWERON ) && ( m_dwPropFlags & AWM_PROP_FORCEMOVEON ))
		return;
	if(( m_nCurState == AWM_STATE_POWEROFF ) && ( m_dwPropFlags & AWM_PROP_FORCEMOVEOFF ))
		return;


	LTVector	vTestPos;
	LTRotation	rTestRot;
	LTBOOL		bChange = LTTRUE;

	// Test collision against the object that activated us if we have one

	if( m_hActivateObj )
	{
		if( GetTestPosRot( vTestPos, rTestRot ) )
		{
			bChange = !TestActivateObjectCollision( vTestPos, rTestRot );
		}
	}

	if( bChange )
	{
		switch( m_nCurState )
		{
			case AWM_STATE_POWEROFF:
			{
				SetPowerOn();
			}
			break;

			case AWM_STATE_POWERON:
			{
				SetPowerOff();
			}
			break;

			default :
			break;
		}
	}
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ActiveWorldModel::CalcAngle
//
//  PURPOSE:	Calculate the new value of fAngle 
//
// ----------------------------------------------------------------------- //

LTBOOL ActiveWorldModel::CalcAngle( LTFLOAT &fAngle, LTFLOAT fInitial, LTFLOAT fTarget, LTFLOAT fDir, LTFLOAT fTotalTime, LTFLOAT fPercent )
{
	// If were are not rotating then we are at the target angle

	if( m_nAWMType != AWM_TYPE_ROTATING ) return LTTRUE;

	if( fPercent >= 1.0f ) 
	{
		fAngle = fTarget;
		return LTTRUE;
	}

	LTBOOL	bRet		= LTFALSE;	// Are we at the target angle?
	LTFLOAT	fRate		= ( fTarget - fInitial );
	if( fTotalTime > 0.001f )
	{
		fRate = fRate / fTotalTime;
	}
	LTFLOAT	fAmount		= GetWaveformValue( fRate, fPercent ) * g_pLTServer->GetFrameTime();


	if( fDir != 0.0f )
	{
		if( fDir > 0.0f )
		{
			if( fAngle < fTarget )
			{
				fAngle += fAmount;

				if( fAngle > fTarget )
					fAngle = fTarget;
			}
			else
			{
				fAngle = fTarget;
                bRet   = LTTRUE;
			}
		}
		else
		{
			if( fAngle > fTarget )
			{
				fAngle += fAmount;

				if( fAngle < fTarget )
					fAngle = fTarget;
			}
			else
			{
				fAngle = fTarget;
                bRet   = LTTRUE;
			}
		}
	}
	else
	{
        bRet = LTTRUE;
	}

	return bRet;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ActiveWorldModel::CalculateNewPosRot
//
//  PURPOSE:	Calculate a new Position and Rotation based on given and current values
//
// ----------------------------------------------------------------------- //

LTBOOL ActiveWorldModel::CalculateNewPosRot( LTVector &vOutPos, LTRotation &rOutRot, LTVector &vFinalPos, LTFLOAT fTotalTime, LTFLOAT fPercent, LTBOOL bTestCollision )
{
	LTRotation	rRotation( m_fPitch, m_fYaw, m_fRoll );

	switch( m_nAWMType )
	{
		case AWM_TYPE_SLIDING:
		{
			g_pLTServer->GetObjectPos( m_hObject, &vOutPos );

			LTVector vDir = vFinalPos - vOutPos;
			LTFLOAT	fDistLeft = vDir.Mag();
			vDir.Normalize();

			LTFLOAT	fRate = m_fMoveDist;
			if( fTotalTime > 0.001f )
			{
				fRate = fRate / fTotalTime;
			}
			LTFLOAT fDistMove = GetWaveformValue( fRate, fPercent ) * g_pLTServer->GetFrameTime();
			
			// Make sure we dont move farther than we're supposed to

			if( (fDistMove >= fDistLeft) || (fPercent >= 1.0f) )
			{
				vOutPos = vFinalPos;
			}
			else
			{
				vDir *= fDistMove;
				vOutPos += vDir;
			}

			// If we didn't actually move there is no point in testing collisions (also
			// sometimes TestObjectCollision thinks there was a collision because our
			// activate object may be touching us, but "that's impossible" since we didn't
			// actually move)...
			// 
			// Note: we do this check here just to make sure vOutPos is set correctly...
			if (m_fMoveDist == 0.0f)
			{
				return LTTRUE;
			}

		}
		break;

		case AWM_TYPE_ROTATING:
		{
			LTMatrix	mRotation;
			
			rOutRot = m_hackInitialRot * rRotation * ~m_hackInitialRot;
			rOutRot.ConvertToMatrix( mRotation );

			vOutPos = (mRotation * m_vRotPtOffset) + m_vRotationPt;
		}
		break;

		case AWM_TYPE_STATIC:
		default:
			break;

	}

	if( bTestCollision )
	{
		return !TestObjectCollision( LTNULL, vOutPos, rRotation );
	}


	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ActiveWorldModel::GetTestPosRot
//
//  PURPOSE:	Get the test position and rotation of the AWM
//
// ----------------------------------------------------------------------- //

LTBOOL ActiveWorldModel::GetTestPosRot( LTVector &vTestPos, LTRotation &rTestRot )
{
	LTVector	vFinalPos;
	LTFLOAT		fTime;
	LTVector	vOldAngles(m_fPitch, m_fYaw, m_fRoll);
	
	const LTFLOAT	cf10Percent = 0.1f;

	// Set vars based on state...

	switch( m_nCurState )
	{
		case AWM_STATE_POWERON:
		{
			vFinalPos	= m_vOffPos;
			fTime		= m_fPowerOffTime;

			if( m_nAWMType == AWM_TYPE_ROTATING )
			{
				// Calculate new pitch, yaw, and roll...

				CalcAngle( m_fPitch, m_fPitch, m_vOffAngles.x, -m_vRotateDir.x, fTime, cf10Percent );
				CalcAngle( m_fYaw,   m_fYaw,   m_vOffAngles.y, -m_vRotateDir.y, fTime, cf10Percent );
				CalcAngle( m_fRoll,  m_fRoll,  m_vOffAngles.z, -m_vRotateDir.z, fTime, cf10Percent );
			}
		}
		break;

		case AWM_STATE_POWEROFF:
		{
			vFinalPos	= m_vOnPos;
			fTime		= m_fPowerOnTime;

			if( m_nAWMType == AWM_TYPE_ROTATING )
			{
				// Calculate new pitch, yaw, and roll...

				CalcAngle( m_fPitch, m_fPitch, m_vOnAngles.x, m_vRotateDir.x, fTime, cf10Percent );
				CalcAngle( m_fYaw,   m_fYaw,   m_vOnAngles.y, m_vRotateDir.y, fTime, cf10Percent );
				CalcAngle( m_fRoll,  m_fRoll,  m_vOnAngles.z, m_vRotateDir.z, fTime, cf10Percent );
			}
		}
		break;

		// If we are in any other state we are not moving, so there is nothing to test

		default :
			return LTFALSE;
		break;

	};

	CalculateNewPosRot( vTestPos, rTestRot, vFinalPos, fTime, cf10Percent );

	// Restore real angles...

	m_fPitch = vOldAngles.x;
	m_fYaw	 = vOldAngles.y;
	m_fRoll	 = vOldAngles.z;

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ActiveWorldModel::TestObjectCollision
//
//  PURPOSE:	See if the test object would collide with the AWM if the AWM were in
//				the test position and rotation
//
// ----------------------------------------------------------------------- //

LTBOOL ActiveWorldModel::TestObjectCollision( HOBJECT hTest, const LTVector &vTestPos, const LTRotation &rTestRot, HOBJECT *pCollisionObj )
{
	// Get an array of all objects that would intersect with us... remember that there may be 
	// bulletholes and model props attached to us so make the array some what large.

	ObjArray<HOBJECT, 100> objArray;
	if( g_pLTServer->FindWorldModelObjectIntersections( m_hObject, vTestPos, rTestRot, objArray ) != LT_OK )
	{
		return LTFALSE;
	}

	// Loop through all the objects that would collide with us and check against the test object...

	HOBJECT hObj = LTNULL;
	for( uint32 i = 0; i < objArray.NumObjects(); i++ )
	{
		hObj = objArray.GetObject( i );
		if( hObj && IsPlayer( hObj ))
		{
			// If there was no test object return true if any players collide...

			if( !hTest || hTest == hObj )
			{
				// Output the collision object if asked for...

				if( pCollisionObj )
				{
					*pCollisionObj = hObj;
				}

				if( m_nAWMType == AWM_TYPE_ROTATING) 
				{
					if( hTest || ( hObj == m_hActivateObj ))
					{
						LTVector vObjPos, vAWMCurPos;
						g_pLTServer->GetObjectPos(hObj, &vObjPos);
						g_pLTServer->GetObjectPos(m_hObject, &vAWMCurPos);

						// If the AWM's new position is farther away from the touch object
						// then its current position, we'll assume there can't be a collision.

						if( vObjPos.DistSqr( vAWMCurPos ) <= vObjPos.DistSqr( vTestPos ) )
						{
							return LTFALSE;
						}
						else
						{
							return LTTRUE;
						}
					}
					else
					{
						return LTTRUE;
					}
				}
				else
				{
					return LTTRUE;
				}
			}
		}
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ActiveWorldModel::Activate
//
//  PURPOSE:	Handle object activation
//
// ----------------------------------------------------------------------- //

void ActiveWorldModel::Activate( HOBJECT hObj )
{
	if( !hObj ) return;

	// Only characters ca activate us

	if( !IsCharacter( hObj ) ) return;
	
	// If the object is an AI make sure we can be activated by AI

	LTBOOL	bAIActivated = IsAI( hObj );
	if( bAIActivated && !(m_dwPropFlags & AWM_PROP_AIACTIVATE) ) return;

	// Can we be activated by a player

	if( !bAIActivated && !(m_dwPropFlags & AWM_PROP_PLAYERACTIVATE) ) return;

	SetActiveObj( hObj );

	// Lets do what we came here to do

	HandleTriggerMsg();
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ActiveWorldModel::SetActiveObj
//
//  PURPOSE:	Sets the object that activated us
//
// ----------------------------------------------------------------------- //

void ActiveWorldModel::SetActiveObj( HOBJECT hObj )
{
	m_hActivateObj = hObj;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ActiveWorldModel::RegisterDisturbanceStimulus
//
//  PURPOSE:	Register sound stimulus and register or remove
//				visual stimulus.
//
// ----------------------------------------------------------------------- //

void ActiveWorldModel::RegisterDisturbanceStimulus(LTBOOL bAudioStim)
{
	// Register audio and visual stimulus if specified and triggered.

	if( (m_hActivateObj == LTNULL) ||
		(m_nActivateAlarmLevel == 0) || 
		(m_fStimRadius <= 0.f) || 
		(!IsCharacter(m_hActivateObj)) )
	{
		return;
	}


	LTVector vPos;
	g_pLTServer->GetObjectPos(m_hObject, &vPos);

	if(bAudioStim == LTTRUE)
	{
		g_pAIStimulusMgr->RegisterStimulus( kStim_EnemyDisturbanceSound, m_nActivateAlarmLevel, m_hActivateObj, m_hObject, vPos, m_fStimRadius );
	}

	// Add or remove a visual stimulus, depending on the starting state of the object.

	if(m_nCurState != m_nStartingState)
	{
		if( m_eStimID == kStimID_Unset )
		{
			m_eStimID = g_pAIStimulusMgr->RegisterStimulus( kStim_EnemyDisturbanceVisible, m_nActivateAlarmLevel + 1, m_hActivateObj, m_hObject, vPos, m_fStimRadius );

			// Search for an AINodeUseObject pointing at this object.

			if( !m_bSearchedForNode )
			{
				AINode* pNode = g_pAINodeMgr->FindUseObjectNode( kNode_Disturbance, m_hObject, LTTRUE );
				m_hUseObjectNode = ( pNode ) ? pNode->m_hObject : LTNULL;
				m_bSearchedForNode = LTTRUE;
			}

			// If a node exists, toggle it to its Disturbed state.

			if( m_hUseObjectNode )
			{
				AINodeUseObject* pUseObjectNode = (AINodeUseObject*)g_pLTServer->HandleToObject( m_hUseObjectNode );
				AIASSERT( pUseObjectNode, m_hObject, "ActiveWorldModel::RegisterDisturbanceStimulus: Cannot find UseObject Node" );

				pUseObjectNode->SetSmartObjectState( kState_SmartObjectDisturbed );
			}
		}
	}

	// If a node exists, toggle it to its Default state.
	
	else if(m_eStimID != kStimID_Unset)
	{
		g_pAIStimulusMgr->RemoveStimulus(m_eStimID);
		m_eStimID = kStimID_Unset;

		if( m_hUseObjectNode )
		{
			AINodeUseObject* pUseObjectNode = (AINodeUseObject*)g_pLTServer->HandleToObject( m_hUseObjectNode );
			AIASSERT( pUseObjectNode, m_hObject, "ActiveWorldModel::RegisterDisturbanceStimulus: Cannot find UseObject Node" );

			pUseObjectNode->SetSmartObjectState( kState_SmartObjectDefault );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ActiveWorldModel::StartSound
//
//  PURPOSE:	Start playing a sound given a filename
//
// ----------------------------------------------------------------------- //

void ActiveWorldModel::StartSound( const HSTRING hstrSoundName, const LTBOOL bLoop )
{
	// Kill any currently playing sound

	StopSound();

	if( !hstrSoundName ) return;

	const char *pSoundName = g_pLTServer->GetStringData( hstrSoundName );
	if( !pSoundName ) return;

	uint32 dwSndFlags = PLAYSOUND_3D | PLAYSOUND_REVERB;

	if( bLoop & (m_dwPropFlags & AWM_PROP_LOOPSOUNDS) )
	{
		dwSndFlags |= PLAYSOUND_LOOP | PLAYSOUND_GETHANDLE;
	}

	// If we didn't specify a position attach it to the object

	if( m_vSoundPos.NearlyEquals( LTVector( 0.0f, 0.0f, 0.0f ), 0.0f ))
	{
		dwSndFlags |= PLAYSOUND_ATTACHED;

		m_hsndLastSound = g_pServerSoundMgr->PlaySoundFromObject( m_hObject, pSoundName, m_fSoundRadius, SOUNDPRIORITY_MISC_MEDIUM, 
																  dwSndFlags, SMGR_DEFAULT_VOLUME, 1.0f, m_fSoundRadius * 0.25f );
	}
	else
	{
		LTVector vPos, vSndPos;
		g_pLTServer->GetObjectPos( m_hObject, &vPos );
		vSndPos = vPos + m_vSoundPos;

		m_hsndLastSound = g_pServerSoundMgr->PlaySoundFromPos( vSndPos, pSoundName, m_fSoundRadius, SOUNDPRIORITY_MISC_MEDIUM,
															   dwSndFlags, SMGR_DEFAULT_VOLUME, 1.0f, m_fSoundRadius * 0.25f ); 
	}
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ActiveWorldModel::StopSound
//
//  PURPOSE:	Stop the currently playing sound
//
// ----------------------------------------------------------------------- //

void ActiveWorldModel::StopSound( )
{
	// If there is a sound playing... stop it

	if( m_hsndLastSound )
	{
		if( m_dwPropFlags & AWM_PROP_LOOPSOUNDS )
		{
			g_pLTServer->SoundMgr()->KillSoundLoop( m_hsndLastSound );
		}
		else
		{
			g_pLTServer->SoundMgr()->KillSound( m_hsndLastSound );
		}
		
		m_hsndLastSound = LTNULL;
	}
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ActiveWorldModel::OnSave
//
//  PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void ActiveWorldModel::OnSave( ILTMessage_Write *pMsg, uint32 dwSaveFlags )
{
	if( !pMsg ) return;

	// Save base class vars first

	WorldModel::OnSave( pMsg, dwSaveFlags );

	SAVE_BYTE( m_nAWMType );
	SAVE_HOBJECT( m_hActivateObj );
	
	// Movement vars...

	SAVE_VECTOR( m_vMoveDir );
	SAVE_VECTOR( m_vOnPos );
	SAVE_VECTOR( m_vOffPos );
	SAVE_VECTOR( m_vPausePos );
	SAVE_FLOAT( m_fPowerOnTime );
	SAVE_FLOAT( m_fPowerOnMultiplier );
	SAVE_FLOAT( m_fMoveDist );
	SAVE_FLOAT( m_fOnWaitTm );
	SAVE_FLOAT( m_fOffWaitTm );
	SAVE_FLOAT( m_fPowerOffTime );
	SAVE_TIME( m_fMoveStartTm );
	SAVE_FLOAT( m_fMoveDelay );
	SAVE_TIME( m_fMoveStopTm );
	SAVE_FLOAT( m_fCurTm );

	// Rotation vars...

	SAVE_VECTOR( m_vOriginalPos );
	SAVE_FLOAT( m_fPitch );
	SAVE_FLOAT( m_fYaw );
	SAVE_FLOAT( m_fRoll );
	SAVE_VECTOR( m_vRotationPt );		// No need to save m_hstrRotationPt since we allready have the position
	SAVE_VECTOR( m_vRotPtOffset );
	SAVE_VECTOR( m_vRotationAngles );
	SAVE_VECTOR( m_vOnAngles );
	SAVE_VECTOR( m_vInitOnAngles );
	SAVE_VECTOR( m_vOffAngles );
	SAVE_VECTOR( m_vRotateDir );
	SAVE_VECTOR( m_vInitRotDir );

	// State vars...

	SAVE_DWORD( m_dwPropFlags );
	SAVE_BYTE( m_nCurState );
	SAVE_BYTE( m_nStartingState );
	SAVE_BYTE( m_nLastState );
	SAVE_BYTE( m_nWaveform );
	
	// Sound vars...	
	
	SAVE_HSTRING( m_hstrPowerOnSnd );
	SAVE_HSTRING( m_hstrOnSnd );
	SAVE_HSTRING( m_hstrPowerOffSnd );
	SAVE_HSTRING( m_hstrOffSnd );
	SAVE_HSTRING( m_hstrLockedSnd );
	SAVE_VECTOR( m_vSoundPos );
	SAVE_FLOAT( m_fSoundRadius );
	SAVE_BOOL( m_bLoopSounds );
	
	// Disturbance stimulus vars...

	SAVE_DWORD( m_eStimID );					
	SAVE_DWORD(	m_nActivateAlarmLevel );		
	SAVE_BOOL( m_bSearchedForNode );
	SAVE_HOBJECT( m_hUseObjectNode );

	// Command vars...

	SAVE_HSTRING( m_hstrOnCmd );
	SAVE_HSTRING( m_hstrOffCmd );
	SAVE_HSTRING( m_hstrPowerOnCmd );
	SAVE_HSTRING( m_hstrPowerOffCmd );
	SAVE_HSTRING( m_hstrLockedCmd );

}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ActiveWorldModel::OnLoad
//
//  PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void ActiveWorldModel::OnLoad( ILTMessage_Read *pMsg, uint32 dwSaveFlags )
{
	if( !pMsg ) return;

	// Load base class vars first

	WorldModel::OnLoad( pMsg, dwSaveFlags );

	LOAD_BYTE( m_nAWMType );
	LOAD_HOBJECT( m_hActivateObj );
	
	// Movement vars...

	LOAD_VECTOR( m_vMoveDir );
	LOAD_VECTOR( m_vOnPos );
	LOAD_VECTOR( m_vOffPos );
	LOAD_VECTOR( m_vPausePos );
	LOAD_FLOAT( m_fPowerOnTime );
	LOAD_FLOAT( m_fPowerOnMultiplier );
	LOAD_FLOAT( m_fMoveDist );
	LOAD_FLOAT( m_fOnWaitTm );
	LOAD_FLOAT( m_fOffWaitTm );
	LOAD_FLOAT( m_fPowerOffTime );
	LOAD_TIME( m_fMoveStartTm );
	LOAD_FLOAT( m_fMoveDelay );
	LOAD_TIME( m_fMoveStopTm );
	LOAD_FLOAT( m_fCurTm );

	// Rotation vars...

	LOAD_VECTOR( m_vOriginalPos );
	LOAD_FLOAT( m_fPitch );
	LOAD_FLOAT( m_fYaw );
	LOAD_FLOAT( m_fRoll );
	LOAD_VECTOR( m_vRotationPt );
	LOAD_VECTOR( m_vRotPtOffset );
	LOAD_VECTOR( m_vRotationAngles );
	LOAD_VECTOR( m_vOnAngles );
	LOAD_VECTOR( m_vInitOnAngles );
	LOAD_VECTOR( m_vOffAngles );
	LOAD_VECTOR( m_vRotateDir );
	LOAD_VECTOR( m_vInitRotDir );

	// State vars...

	LOAD_DWORD( m_dwPropFlags );
	LOAD_BYTE( m_nCurState );
	LOAD_BYTE( m_nStartingState );
	LOAD_BYTE( m_nLastState );
	LOAD_BYTE( m_nWaveform );
	
	// Sound vars...	
	
	LOAD_HSTRING( m_hstrPowerOnSnd );
	LOAD_HSTRING( m_hstrOnSnd );
	LOAD_HSTRING( m_hstrPowerOffSnd );
	LOAD_HSTRING( m_hstrOffSnd );
	LOAD_HSTRING( m_hstrLockedSnd );
	LOAD_VECTOR( m_vSoundPos );
	LOAD_FLOAT( m_fSoundRadius );
	LOAD_BOOL( m_bLoopSounds );
	
	// Disturbance stimulus vars...

	LOAD_DWORD_CAST( m_eStimID, EnumAIStimulusID );					
	LOAD_DWORD(	m_nActivateAlarmLevel );				
	LOAD_BOOL( m_bSearchedForNode );
	LOAD_HOBJECT( m_hUseObjectNode );

	// Command vars...

	LOAD_HSTRING( m_hstrOnCmd );
	LOAD_HSTRING( m_hstrOffCmd );
	LOAD_HSTRING( m_hstrPowerOnCmd );
	LOAD_HSTRING( m_hstrPowerOffCmd );
	LOAD_HSTRING( m_hstrLockedCmd );


	// Start playing sounds if we are supposed to

	if( m_dwPropFlags & AWM_PROP_LOOPSOUNDS )
	{
		switch( m_nCurState )
		{
			case AWM_STATE_OFF:
			{
				StartSound( m_hstrOffSnd, LTTRUE );
			}
			break;

			case AWM_STATE_ON:
			{
				StartSound( m_hstrOnSnd, LTTRUE );
			}
			break;
		
			case AWM_STATE_POWERON:
			{
				StartSound( m_hstrPowerOnSnd, LTTRUE );
			}
			break;

			case AWM_STATE_POWEROFF:
			{
				StartSound( m_hstrPowerOffSnd, LTTRUE );
			}
			break;
		}
	}
}
