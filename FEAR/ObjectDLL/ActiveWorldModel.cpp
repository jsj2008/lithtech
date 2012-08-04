// ----------------------------------------------------------------------- //
//
// MODULE  : ActiveWorldModel.cpp
//
// PURPOSE : ActiveWorldModel implementation
//
// CREATED : 5/16/01
//
// (c) 2001-2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//

	#include "Stdafx.h"
	#include "ActiveWorldModel.h"
	#include "ServerSoundMgr.h"
	#include "AIStimulusMgr.h"
	#include "AINode.h"
	#include "AINodeMgr.h"
	#include "AIState.h"
	#include "ParsedMsg.h"
	#include "AIUtils.h"
	#include "CharacterMgr.h"

	extern CAIStimulusMgr* g_pAIStimulusMgr;

//
// Globals...
//

	static const char* c_aWaveTypes[] =
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

	PROP_DEFINEGROUP(Options, PF_GROUP(3), "This is a subset of properties that define some behavior of the Object")
		ADD_BOOLPROP_FLAG(PlayerActivate, false, PF_GROUP(3), "If TRUE the player can directly interact with this object by pressing use.")
		ADD_BOOLPROP_FLAG(AIActivate, false, PF_GROUP(3), "If TRUE this lets the AI know they can interact with this object.  When FALSE the AI will treat the object like it doesn't exist.")
		ADD_BOOLPROP_FLAG(StartOn, false, PF_GROUP(3), "When set to TRUE the object will be turnning on as soon as the game loads.")
		ADD_BOOLPROP_FLAG(TriggerOff, true, PF_GROUP(3), "If this is set to FALSE the player can not directly turn the object off by pressing use.  The object can however be turned off by a message from another object like a switch.  If set to TRUE the player can directly turn off or close the object by pressing use.")
		ADD_BOOLPROP_FLAG(RemainOn, true, PF_GROUP(3), "If this is FALSE the Object will start turnning itself off or close as soon as it turns on or opens.  If TRUE the object will stay on or open untill told to turn off, either by the player or a message.")
		ADD_BOOLPROP_FLAG(ForceMove, false, PF_GROUP(3), "If set to TRUE, the object will not be stopped by the player or AI.  It will continue on its path like there is nothing in the way.")
		ADD_BOOLPROP_FLAG(ForceMoveOn, false, PF_GROUP(3), "If set to TRUE, the object will not be stopped by the player or AI when going on.  It will continue on its path like there is nothing in the way.")
		ADD_BOOLPROP_FLAG(ForceMoveOff, false, PF_GROUP(3), "If set to TRUE, the object will not be stopped by the player or AI when going off.  It will continue on its path like there is nothing in the way.")
		ADD_BOOLPROP_FLAG(Locked, false, PF_GROUP(3), "When set to TRUE the object starts in a ""Locked"" state.  It can't be activated or triggered by a message unless it is unlocked.  To unlock an object send it an UNLOCK message.  To lock it again send a LOCK message." )
		ADD_BOOLPROP_FLAG(RotateAway, true, PF_GROUP(3), "If set to TRUE RotatingWorldModels will rotate away from the player or AI that activated it.")
		ADD_STRINGPROP_FLAG(Waveform, "Linear", PF_STATICLIST | PF_GROUP(3), "A list of predefined wave types for movement.  Linear is a constant rate, objects will move at the same rate throught its whole path or rotation. SlowOn means the object starts moving slowly then picks up pace and will then stay constant.  SlowOff will move constantly at first but will slow down at the end of the movement or rotation.  Sine starts and ends slowly but looks very smooth and natural.")
		ADD_STRINGPROP_FLAG(ActivationType, "Default", PF_STATICLIST | PF_GROUP(3), "A list of different activatable types used for player interaction.")

	// Add a sounds group...

	PROP_DEFINEGROUP(Sounds, PF_GROUP(4), "This is a subset of properties that define sound behavior and let you specify different sounds to play during different states.")
		ADD_STRINGPROP_FLAG(PowerOnSound, "", PF_FILENAME | PF_GROUP(4), "The path of any .wav file can be entered here.  This sound will begin playing as soon as the object starts to turn on.")
		ADD_STRINGPROP_FLAG(OnSound, "", PF_FILENAME | PF_GROUP(4), "The path of any .wav file can be entered here.  This sound will begin playing as soon as the object is fully turned on.")
		ADD_STRINGPROP_FLAG(PowerOffSound, "", PF_FILENAME | PF_GROUP(4), "The path of any .wav file can be entered here.  This sound will begin playing as soon as the object starts to turn off.")
		ADD_STRINGPROP_FLAG(OffSound, "", PF_FILENAME | PF_GROUP(4), "The path of any .wav file can be entered here.  This sound will begin playing as soon as the object is fully turned off.")
		ADD_STRINGPROP_FLAG(LockedSound, "", PF_FILENAME | PF_GROUP(4), "The path of any .wav file can be entered here.  If this object is locked and the player or another object tries to activate it, this sound will begin to play.")
		ADD_VECTORPROP_VAL_FLAG(SoundPos, 0.0f, 0.0f, 0.0f, PF_GROUP(4), "The position that the sounds will be played from, relative to the objects position.  If this is (0.0 0.0 0.0) the sounds will play at the center of the object, (0.0 25.0 0.0) will place the sounds 25 units above the objects center.")
		ADD_REALPROP_FLAG(SoundRadius, 1000.0f, PF_RADIUS | PF_GROUP(4), "This is the extent of the sounds, they will not be heard beyond this distance from the SoundPos.")
		ADD_BOOLPROP_FLAG(LoopSounds, false, PF_GROUP(4), "When set to TRUE, the sounds will comtinue to loop untill they change state.  When FALSE the sounds will play their full length once and will then be destroyed.")

	// Add a commands group...

	PROP_DEFINEGROUP(Commands, PF_GROUP(5), "This is a subset that lets you write commands for specific states that the object will cary out when in that state.")
		ADD_COMMANDPROP_FLAG(OnCommand, "", PF_GROUP(5) | PF_NOTIFYCHANGE, "A command that this object will execute when it is fully on.")
		ADD_COMMANDPROP_FLAG(OffCommand, "", PF_GROUP(5) | PF_NOTIFYCHANGE, "A command that this object will execue when it is fully off.")
		ADD_COMMANDPROP_FLAG(PowerOnCommand, "", PF_GROUP(5) | PF_NOTIFYCHANGE, "A command that this object will execute when starting to turn on.")
		ADD_COMMANDPROP_FLAG(PowerOffCommand, "", PF_GROUP(5) | PF_NOTIFYCHANGE, "A command that this object will execute when starting to turn off.")
		ADD_COMMANDPROP_FLAG(LockedCommand, "", PF_GROUP(5) | PF_NOTIFYCHANGE, "If this object is locked and the player or another object tries to activate it, this command will be executed.")

	// Add to the disturbance group...

	ADD_LONGINTPROP_FLAG(ActivateAlarmLevel, 0, PF_GROUP(6), "How alarming it is to the AI when this object is activated. Corresponds to thresholds in AIBrains in aibutes.txt.")
	

	// Add the movement properties...

	ADD_VECTORPROP_VAL_FLAG(MoveDir, 0.0f, 0.0f, 0.0f, 0, "This is the direction of movement this object will go when turned on.  This is relative to the objects local coordinates so a MoveDir of (0.0 1.0 0.0) will always move in the objects positive Y direction.")
	ADD_REALPROP_FLAG(MoveDist, 0.0f, 0, "This is the distance the object will move in the direction specified in MoveDir.")

	// Add the rotation properties...

	ADD_BOOLPROP_FLAG(RotateAroundCenter, false, 0, "When set to TRUE, object will use the center of worldmodel geometry as rotation point.  When sent to FALSE, object will use WorldModel object as rotation point.  If RotationPoint is specified, this property is ignored.")
	ADD_STRINGPROP_FLAG(RotationPoint, "", PF_OBJECTLINK, "The name of a Point object which this WorldModel will rotate around.  If no valid object name is given the WorldModel will rotate around the position of the bound object." )
	ADD_VECTORPROP_FLAG(RotationAngles, 0, "These represent how far the WorldModel will rotate around the RotationPoint in the specified axi when turned on.  (0.0 90.0 0.0) will rotate the WorldModel about the RotationPoint 90 degrees around the WorldModels local Y axis.")
	ADD_VECTORPROP_VAL_FLAG(StartingAngles, 0.0f, 0.0f, 0.0f, 0, "Specifes an optional initial rotation of the WorldModel.  These are in degrees relative to the rotation of the WorldModel when in the off state.  A StartingAngle of (0.0, 15.0, 0.0) will load the WorldModel with a rotation of 15 degrees around the WorldModels' local Y axis in the on direction." )
	
	// Add the shared properties...

	ADD_REALPROP_FLAG(PowerOnTime, AWM_DEFAULT_POWERONTIME, 0, "Sets the time in seconds for how long it takes the WorldModel to go from the Off state to the on state.")
	ADD_REALPROP_FLAG(PowerOffTime, AWM_DEFAULT_POWEROFFTIME, 0, "If other than 0.0, sets the time in seconds for how long it takes the WorldModel to go from the On state to the off state.  If this is 0.0 then the PowerOnTime value is used.")
	ADD_REALPROP_FLAG(PowerOffRadius, AWM_DEFAULT_POWEROFFRADIUS, 0, "If other than 0.0, sets the minimum distance in centimeters from the WorldModels's power off position that ALL characters must be for the WorldModel to power off.")
	ADD_REALPROP_FLAG(MoveDelay, AWM_DEFAULT_MOVEDELAY, 0, "Amount of delay in seconds between the time the WorldModel is triggered and when it begins its movement.")
	ADD_REALPROP_FLAG(OnWaitTime, AWM_DEFAULT_ONWAITTIME, 0, "Amount of time in seconds that the WorldModel will remain on before turnning off automatically, and the amount of time before the WorldModel can be triggered on again.")
	ADD_REALPROP_FLAG(OffWaitTime, AWM_DEFAULT_OFFWAITTIME, 0, "Amount of time in secomds before the WorldModel can be turned on after being turned off.")

	ADD_PREFETCH_RESOURCE_PROPS()

END_CLASS_FLAGS_PLUGIN_PREFETCH(ActiveWorldModel, WorldModel, CF_HIDDEN | CF_WORLDMODEL, CActiveWorldModelPlugin, DefaultPrefetch<ActiveWorldModel>, "This is the base class for Sliding, Rotating and Spinning WorldModes.  ActiveWorldModel is hidden and objects of type ActiveWorldModel cannot be placed in WorldEdit.")

template< uint8 TWaveformParam >
static bool ValidateWaveform( ILTPreInterface *pInterface, ConParse &cpMsgParams )
{
	// Check the wave form to make sure they entered a correct value...
	if( cpMsgParams.m_nArgs == TWaveformParam )
	{
		for( uint8 nWave = 0; nWave <= AWM_WAVE_MAXTYPES; ++nWave )
		{
			if( LTStrIEquals( cpMsgParams.m_Args[TWaveformParam - 1], c_aWaveTypes[nWave] ) )
			{
				// We have a match...
				return true;
			}
		}

		if( CCommandMgrPlugin::s_bShowMsgErrors )
		{
			pInterface->ShowDebugWindow( true );
			pInterface->CPrint( "ERROR! - ValidateMsgRotate()" );
			pInterface->CPrint( "    MSG - %s - Waveform argument '%s' is not a valid waveform.", LTStrUpr(cpMsgParams.m_Args[0]), cpMsgParams.m_Args[TWaveformParam - 1] );
		}

		// Fail to supply a correct wave form
		return false;
	}

	return true;
}

namespace // un-named
{
	LTVector RemoveComponent(const LTVector& vVector, const LTVector& vComponent)
	{
		// vComponent should be a normalized vector.

		return vVector - vComponent*vComponent.Dot(vVector);
	}

} // namespace un-named

//
// Register the class with the command mgr plugin and add any messages to the class
//

CMDMGR_BEGIN_REGISTER_CLASS( ActiveWorldModel )

	ADD_MESSAGE( ACTIVATE,	1,	NULL,	MSG_HANDLER( ActiveWorldModel, HandleActivateMsg ),	"ACTIVATE", "This is an internal message used strictly by the game to activate a WorldModel.  Do NOT send any WorldModel an ACTIVATE message.", "DO NOT USE" )
	ADD_MESSAGE( TOGGLE,	1,	NULL,	MSG_HANDLER( ActiveWorldModel, HandleToggleMsg ),	"TOGGLE", "Toggles the ON / OFF state of the WorldModel.", "msg WorldModel01 TOGGLE;" )
	ADD_MESSAGE( LOCK,		1,	NULL,	MSG_HANDLER( ActiveWorldModel, HandleLockMsg ),		"LOCK", "Locks the WorldModel.  A locked WorldModel may not be activated by a message or by the player.", "msg WorldModel01 LOCK;" )
	ADD_MESSAGE( UNLOCK,	1,	NULL,	MSG_HANDLER( ActiveWorldModel, HandleUnlockMsg ),	"UNLOCK", "Unlocks the WorldModel so it may be activated through messages or by the player.", "msg WorldModel01 UNLOCK;" )
	ADD_MESSAGE( PAUSE,		1,	NULL,	MSG_HANDLER( ActiveWorldModel, HandlePauseMsg ),	"PAUSE", "Pauses the WorldModel while moving or rotating.  Sending a PAUSE message to a WorldModel while is is in the ON or OFF state has no effect.  Send a RESUME message to continue the movement or rotation.", "msg WorldModel01 PAUSE;" )
	ADD_MESSAGE( RESUME,	1,	NULL,	MSG_HANDLER( ActiveWorldModel, HandleResumeMsg ),	"RESUME", "Resumes the movement or rotation from a paused WorldModel.  Sending a RESUME message to a WorldModel while it is not paused has no effect.", "msg WorldModel01 RESUME;" )
	ADD_MESSAGE( SETPOWERONMULTIPLIER, 2, NULL, MSG_HANDLER( ActiveWorldModel, HandleSetPowerOnMultiplayerMsg ), "SETPOWERONMULTIPLIER <multiplyer>", "Setting a power on multiplyer will temporairly increase or decrease the speed at which the WorldModel turns on.  This is off a base multiplyer of 1 and it will be reset after the first time the WorldModel turns on after setting a multiplyer.  Use the ON and OFF with the optional time value for more precise control.", "msg WorldModel01 (SETPOWERONMULTIPLYER 0.5);  Setting the multiplyer to 0.5 will enable the WorldModel to take twice as long to reach the ON state." )
	ADD_MESSAGE( CANACTIVATE, 2, NULL,	MSG_HANDLER( ActiveWorldModel, HandleCanActivateMsg ), "CANACTIVATE <1 or 0>", "Enables or disables the players ability to activate the WorldModel using the action key", "msg WorldModel01 (CANACTIVATE 0);" )
	ADD_MESSAGE( SETSTARTINGANGLES, 4, NULL, MSG_HANDLER( ActiveWorldModel, HandleSetStartingAnglesMsg ), "SETSTARTINGANGLES <X> <Y> <Z>", "Allows you to set the WorldModel's StartingAngles via a message", "msg WorldModel01 (SETSTARTINGANGLES 0 15 0);" )
	
	ADD_MESSAGE_ARG_RANGE_FLAGS( ON,		1,	3,	ValidateWaveform<3>,	MSG_HANDLER( ActiveWorldModel, HandleOnMsg ),		0,	"ON [time] [waveform]", "Turn the WorldModel on.  The optional parameter [time] is the seconds it takes for the WorldModel to reach the on state.  Valid values for the optional parameter [waveform] are: Linear, Sine, SlowOn and SlowOff.", "To turn the WorldModel on using default values the command would look like:<BR><BR>msg WorldModel01 ON;<BR><BR>To turn a WorldModel on over 2 seconds in a linear movement, the command would look like:<BR><BR>msg WorldModel01 (ON 2.0 Linear);" )
	ADD_MESSAGE_ARG_RANGE_FLAGS( OFF,		1,	3,	ValidateWaveform<3>,	MSG_HANDLER( ActiveWorldModel, HandleOffMsg ),		0,	"OFF [time] [waveform]", "Turn the WorldModel off.  The optional parameter [time] is the seconds it takes for the WorldModel to reach the off state.  Valid values for the optional parameter [waveform] are: Linear, Sine, SlowOn and SlowOff.", "To turn the WorldModel off using default values the command would look like:<BR><BR>msg WorldModel01 OFF;<BR><BR>To turn a WorldModel off over half a second in a sine wave movement, the command would look like:<BR><BR>msg WorldModel01 (OFF 0.5 Sine);" )
	ADD_MESSAGE_ARG_RANGE_FLAGS( ROTATE,	4,	6,	ValidateWaveform<6>,	MSG_HANDLER( ActiveWorldModel, HandleRotateMsg ),	0,	"ROTATE <X> <Y> <Z> [time] [waveform]", "Rotate the WorldModel to the specified angle around each axis.  All angles are in degrees and relative to the off position of the WorldModel.  The optional parameter [time] is the seconds it takes for the WorldModel to reach the angles.  Valid values for the optional parameter [waveform] are: Linear, Sine, SlowOn and SlowOff.", "msg WorldModel01 (ROTATE 0 -15 0 2.5 SlowOn);" )
	ADD_MESSAGE_ARG_RANGE_FLAGS( MOVE,		4,	6,	ValidateWaveform<6>,	MSG_HANDLER( ActiveWorldModel, HandleMoveMsg ),		0,	"MOVE <X> <Y> <Z> [time] [waveform]", "Move the WorldModel the specified distance along each axis.  All distances are in centimeters and relative to the off position of the WorldModel.  The optional parameter [time] is the seconds it takes for the WorldModel to move the distance.  Valid values for the optional parameter [waveform] are: Linear, Sine, SlowOn and SlowOff.", "msg WorldModel01 (MOVE -100 50 0 0.5 Linear);" )

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

	if( !LTStrICmp( szPropName, "Waveform" ) )
	{
		// Fill the list with our wave types...

		for( int i = 0; i <= AWM_WAVE_MAXTYPES; i++ )
		{
			LTStrCpy( aszStrings[(*pcStrings)++], c_aWaveTypes[i], cMaxStringLength );
		}

		return LT_OK;		
	}
	else if( !LTStrICmp( szPropName, "ActivationType" ))
	{
		if( CategoryPlugin::Instance().PopulateStringList( DATABASE_CATEGORY( Activate ).GetCategory(), 
			aszStrings, pcStrings, cMaxStrings, cMaxStringLength ))
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
	m_vMoveToPos( 0, 0, 0 ),
	m_vMoveDistances( 0, 0, 0 ),
	m_vPausePos( 0, 0, 0),
	m_fPowerOnTime( AWM_DEFAULT_POWERONTIME ),
	m_fPowerOnMultiplier( 1.0f ),
	m_fMoveDist( 0.0f ),
	m_fInitMoveDist( 0.0f ),
	m_fPowerOffTime( AWM_DEFAULT_POWEROFFTIME ),
	m_fMoveStartTm( -10.0f ),
	m_fMoveDelay( AWM_DEFAULT_MOVEDELAY ),
	m_fMoveStopTm( -10.0f ),
	m_dwPropFlags( 0 ),
	m_nCurState( 0 ),
	m_nLastState( 0 ),
	m_nStartingState( 0 ),
	m_nWaveform( AWM_WAVE_LINEAR ),
	m_nInitWaveform( AWM_WAVE_LINEAR ),
	m_hActivateObj( NULL ),
	m_tOriginalTrans( LTRigidTransform::GetIdentity() ),
	m_fPitch( 0.0f ),
	m_fYaw( 0.0f ),
	m_fRoll( 0.0f ),
	m_sRotationPt( ),
	m_vRotationPt( 0.0f, 0.0f, 0.0f ),
	m_vRotPtOffset( 0.0f, 0.0f, 0.0f ),
	m_vRotationAngles( 0.0f, 0.0f, 0.0f ),
	m_vStartingAngles( 0.0f, 0.0f, 0.0f ),
	m_vInitOnAngles( 0.0f, 0.0f, 0.0f ),
	m_vInitOffAngles( 0.0f, 0.0f, 0.0f ),
	m_fActiveTime( 0.0f ),
	m_vRotateToAngles( 0.0f, 0.0f, 0.0f ),
	m_vRotateFromAngles( 0.0f, 0.0f, 0.0f ),
	m_vRotateDir( 0.0f, 0.0f, 0.0f ),
	m_vInitRotDir( 0.0f, 0.0f, 0.0f ),
	m_sPowerOnSnd( ),
	m_sOnSnd( ),
	m_sPowerOffSnd( ),
	m_sOffSnd( ),
	m_sLockedSnd( ),
	m_vSoundPos( 0.0f, 0.0f, 0.0f ),
	m_fSoundRadius( 0.0f ),
	m_bLoopSounds( false ),
	m_hsndLastSound( NULL ),
	m_sOnCmd( ),
	m_sOffCmd( ),
	m_sPowerOnCmd( ),
	m_sPowerOffCmd( ),
	m_sLockedCmd( ),
	m_nActivateAlarmLevel( 0 ),
	m_eStimID( kStimID_Unset ),
	m_bSearchedForNode( false ),
	m_hSmartObjectNode( NULL ),
	m_fOnWaitTm( AWM_DEFAULT_ONWAITTIME ),
	m_fOffWaitTm( AWM_DEFAULT_OFFWAITTIME ),
	m_bCheckForChars ( false ),
	m_fPowerOffRadius ( AWM_DEFAULT_POWEROFFRADIUS ),
	m_bBlocked(false)
{

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

void ActiveWorldModel::ReadProps( const GenericPropList *pProps )
{
	LTASSERT(pProps != NULL, "ActiveWorldModel ReadProps encountered without property object");

	// Read base class props first

	WorldModel::ReadProps( pProps );

	// Find out what kind of action this AWM does
	m_nAWMType = (uint8)pProps->GetLongInt( "AWMType", m_nAWMType );

	// Develop Property option flags...

	m_dwPropFlags |= (pProps->GetBool( "PlayerActivate", false ) ? AWM_PROP_PLAYERACTIVATE : 0);
	m_dwPropFlags |= (pProps->GetBool( "AIActivate", false ) ? AWM_PROP_AIACTIVATE : 0);
	m_dwPropFlags |= (pProps->GetBool( "StartOn", false ) ? AWM_PROP_STARTON : 0);
	m_dwPropFlags |= (pProps->GetBool( "TriggerOff", true ) ? AWM_PROP_TRIGGEROFF  : 0);
	m_dwPropFlags |= (pProps->GetBool( "RemainOn", true ) ? AWM_PROP_REMAINON : 0);
	m_dwPropFlags |= (pProps->GetBool( "ForceMove", false ) ? AWM_PROP_FORCEMOVE : 0);
	m_dwPropFlags |= (pProps->GetBool( "ForceMoveOn", false ) ? AWM_PROP_FORCEMOVEON : 0);
	m_dwPropFlags |= (pProps->GetBool( "ForceMoveOff", false ) ? AWM_PROP_FORCEMOVEOFF : 0);
	m_dwPropFlags |= (pProps->GetBool( "Locked", false ) ? AWM_PROP_LOCKED : 0);
	m_dwPropFlags |= (pProps->GetBool( "RotateAway", true ) ? AWM_PROP_ROTATEAWAY : 0);
	m_dwPropFlags |= (pProps->GetBool( "RotateAroundCenter", false ) ? AWM_PROP_ROTATEAROUNDCENTER : 0);

	// Get the waveform...
	
	const char *pszWaveForm = pProps->GetString( "Waveform", "Linear" );
	if( pszWaveForm && pszWaveForm[0] )
	{
		for( uint8 nWaveform = 0; nWaveform <= AWM_WAVE_MAXTYPES; ++nWaveform )
		{
			if( LTStrIEquals( pszWaveForm, c_aWaveTypes[nWaveform] ) )
			{
				m_nInitWaveform = m_nWaveform = nWaveform;
				break;
			}
		}	
	}
	

	// Read sound options...

	m_sPowerOnSnd	= pProps->GetString( "PowerOnSound", "" );
	m_sOnSnd		= pProps->GetString( "OnSound", "" );
	m_sPowerOffSnd	= pProps->GetString( "PowerOffSound", "" );
	m_sOffSnd		= pProps->GetString( "OffSound", "" );
	m_sLockedSnd	= pProps->GetString( "LockedSound", "" );
	m_vSoundPos		= pProps->GetVector( "SoundPos", m_vSoundPos );
	m_fSoundRadius	= pProps->GetReal( "SoundRadius", m_fSoundRadius );

	m_dwPropFlags |= (pProps->GetBool( "LoopSounds", false ) ? AWM_PROP_LOOPSOUNDS : 0);


	// Read Commands to send...

	m_sOnCmd		= pProps->GetCommand( "OnCommand", "" );
	m_sOffCmd		= pProps->GetCommand( "OffCommand", "" );
	m_sPowerOnCmd	= pProps->GetCommand( "PowerOnCommand", "" );
	m_sPowerOffCmd	= pProps->GetCommand( "PowerOffCommand", "" );
	m_sLockedCmd	= pProps->GetCommand( "LockedCommand", "" );

	
	// Read disturbance props...

	m_nActivateAlarmLevel = (uint32)pProps->GetLongInt( "ActivateAlarmLevel", m_nActivateAlarmLevel );
  

	// Read movement props...

	m_vMoveDir = pProps->GetVector( "MoveDir", m_vMoveDir );
	if( m_vMoveDir.MagSqr() < 0.0001f )
		m_vMoveDir.Init( 0.0f, 1.0f, 0.0f );

	m_fInitMoveDist		= pProps->GetReal( "MoveDist", m_fInitMoveDist );
	m_fPowerOnTime		= pProps->GetReal( "PowerOnTime", m_fPowerOnTime );
	m_fPowerOffTime		= pProps->GetReal( "PowerOffTime", m_fPowerOffTime );
	m_fPowerOffRadius	= pProps->GetReal( "PowerOffRadius", m_fPowerOffRadius );
	m_fMoveDelay		= pProps->GetReal( "MoveDelay", m_fMoveDelay );
	m_fOnWaitTm			= pProps->GetReal( "OnWaitTime", m_fOnWaitTm );
	m_fOffWaitTm		= pProps->GetReal( "OffWaitTime", m_fOffWaitTm );

	// Read rotation props...

	// Convert the degrees to radians as they are read in...
	m_vRotationAngles = pProps->GetVector( "RotationAngles", m_vRotationAngles ) * (MATH_PI / 180.0f);
	m_vStartingAngles = pProps->GetVector( "StartingAngles", m_vStartingAngles ) * (MATH_PI / 180.0f);	
	
	m_sRotationPt = pProps->GetString( "RotationPoint", "" );


	// Get the activate type...

	if( m_dwPropFlags & AWM_PROP_PLAYERACTIVATE )
	{
		const char *pszActivateType = pProps->GetString( "ActivationType", "Default" ) ;
		if( !LTStrEmpty( pszActivateType ) && !LTStrIEquals( pszActivateType, "<none>" ))
			m_ActivateTypeHandler.SetActivateType( pszActivateType );
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
	LTASSERT(pOCS != NULL, "Invalid Object Creaet Struct");

	// Init base class data first

	WorldModel::PostReadProp( pOCS );

	pOCS->m_Flags |= FLAG_TOUCH_NOTIFY | FLAG_GOTHRUWORLD;

	if (m_nAWMType == AWM_TYPE_ROTATING)
		pOCS->m_Flags2 |= FLAG2_DISABLEPREDICTION;

	m_fPowerOffTime = ( m_fPowerOffTime > 0.0f ) ? m_fPowerOffTime : m_fPowerOnTime;

	// Initially set the rotation point incase we have no point object 

	m_vRotationPt = pOCS->m_Pos;
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
	if( fabs(m_fInitMoveDist) >= 0.01f || m_vRotationAngles.MagSqr() >= 0.01f )
	{
		dwUsrFlags |= USRFLG_MOVEABLE;
	}

	g_pCommonLT->SetObjectFlags( m_hObject, OFT_User, dwUsrFlags, USRFLG_MOVEABLE );

	g_pCommonLT->SetObjectFlags( m_hObject, OFT_User, ((m_dwPropFlags & AWM_PROP_PLAYERACTIVATE) ? USRFLG_CAN_ACTIVATE : 0), USRFLG_CAN_ACTIVATE );

	g_pLTServer->GetObjectTransform( m_hObject, &m_tOriginalTrans );
	if( m_tOriginalTrans.m_rRot.GetComponentMagSqr( ) < 0.8f || LTIsNaN( m_tOriginalTrans.m_rRot ))
	{
		LTERROR( "Invalid original transform." );
		m_tOriginalTrans.m_rRot.Identity();
	}

	// Consider our rotation point the same as our worldmodel center.
	if( m_dwPropFlags & AWM_PROP_ROTATEAROUNDCENTER )
	{
		m_vRotationPt = m_tOriginalTrans.m_vPos;
	}

	// See if we can be activated or not

	if( m_dwPropFlags & AWM_PROP_PLAYERACTIVATE )
	{
		dwUsrFlags |= USRFLG_CAN_ACTIVATE;

		// If we can be activated create the special fx message...

		bool bIsLocked = (m_dwPropFlags & AWM_PROP_LOCKED) != 0;
		m_ActivateTypeHandler.SetDisabled( bIsLocked, false );
		m_ActivateTypeHandler.CreateActivateTypeMsg();
	}


	// Calculate on and off positions and other data based on type...
	
	switch( m_nAWMType )
	{
		case AWM_TYPE_SLIDING:
		{
			// Current Position is Off position 
			m_vOffPos = m_tOriginalTrans.m_vPos;
			
			// Set movement locally rather than globaly
			m_vMoveDir.Normalize();

			// Now get the On position
			LTVector vTemp = m_tOriginalTrans.m_rRot.RotateVector(m_vMoveDir * m_fInitMoveDist);
			m_vOnPos = m_vOffPos + vTemp;
		}
		break;

		case AWM_TYPE_ROTATING:
		{
			// Determine initial values...
			m_vInitOffAngles.Init( m_fPitch, m_fYaw, m_fRoll );
			m_vInitOnAngles = m_vInitOffAngles + m_vRotationAngles;

			// The AWM must rotate at least 1 degree
			const float c_fMinDelta = MATH_DEGREES_TO_RADIANS( 1.0f );

			// Determine direction to rotate in X...
			float fOffset = m_vInitOffAngles.x - m_vInitOnAngles.x;
			if( fOffset > c_fMinDelta )
			{
				m_vInitRotDir.x = -1.0f;
			}
			else if( fOffset < c_fMinDelta )
			{
				m_vInitRotDir.x = 1.0f;
			}

			// Determine direction to rotate in Y...
			fOffset = m_vInitOffAngles.y - m_vInitOnAngles.y;
			if( fOffset > c_fMinDelta )
			{
				m_vInitRotDir.y = -1.0f;
			}
			else if( fOffset < c_fMinDelta )
			{
				m_vInitRotDir.y = 1.0f;
			}

			// Determine direction to rotate in Z...
			fOffset = m_vInitOffAngles.z - m_vInitOnAngles.z;
			if( fOffset > c_fMinDelta )
			{
				m_vInitRotDir.z = -1.0f;
			}
			else if( fOffset < c_fMinDelta )
			{
				m_vInitRotDir.z = 1.0f;
			}

			// Save our initial values...
			m_vRotateDir	= m_vInitRotDir;
		}
		break;

		case AWM_TYPE_STATIC:
		default :
			break;
	}

	if( m_dwPropFlags & AWM_PROP_STARTON )
	{
		SetPowerOn( m_fPowerOnTime, m_nInitWaveform );
	}
	else
	{
		SetOff( AWM_INITIAL_STATE );
	}

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

	//if we've already been initialized, re-initializing will stomp existing data
	//	so we'll bail early to prevent save/load issues
	if( m_WorldModelFlags & kWorldModelFlag_Initialized )
	{
		return;
	}

	// Send to base class first

	WorldModel::OnEveryObjectCreated();

	// See if we have a rotation point object...
	if( !m_sRotationPt.empty() )
	{
		ObjArray<HOBJECT, 1> objArray;
		g_pLTServer->FindNamedObjects( m_sRotationPt.c_str(), objArray );

		if( objArray.NumObjects() > 0 )
		{
			g_pLTServer->GetObjectPos( objArray.GetObject(0), &m_vRotationPt );

			// We have a rotation point, so don't bother with rotate around center flag.
			m_dwPropFlags &= ~AWM_PROP_ROTATEAROUNDCENTER;

			if( IsKindOf( objArray.GetObject(0), "Point" ) )
			{
				// We have a valid rotation point object now remove it from the world

				g_pLTServer->RemoveObject( objArray.GetObject(0) );
			}
			else
				g_pLTServer->CPrint( "ActiveWorldModel: RotationPointObject NOT a Point!!" );
		}
	}

	// If the rotation point is at the center of the worldmodel, then lock it in to be centered
	// to avoid floating point error.
	if( m_vRotationPt.NearlyEquals( m_tOriginalTrans.m_vPos, 0.1f ) || m_dwPropFlags & AWM_PROP_ROTATEAROUNDCENTER )
	{
		// Consider us centered, unless we have a rotation point.
		m_vRotPtOffset.Init( );
	}
	else
	{
	// Calculate the rotation point offset (allows for the
	// door to be movied (keyframed) and still rotate correctly... maybe?
	m_vRotPtOffset = m_tOriginalTrans.GetInverse() * m_vRotationPt;
	}

	SetStartingAngles(m_vStartingAngles);
}

// ----------------------------------------------------------------------- //

void ActiveWorldModel::SetStartingAngles( const LTVector& vStartingAngles )
{
	m_vStartingAngles = vStartingAngles;

	// If starting angles were specified and the WorldModel is not set to start on then rotate the object to the starting angles...
	if( m_nAWMType == AWM_TYPE_ROTATING && !(m_dwPropFlags & AWM_PROP_STARTON) && (m_vStartingAngles.MagSqr( ) > 0.0f))
	{
		m_fPitch = m_vStartingAngles.x;
		m_fYaw = m_vStartingAngles.y;
		m_fRoll = m_vStartingAngles.z;

		LTRotation rStartRot( m_fPitch, m_fYaw, m_fRoll );
		LTRigidTransform tOffset(m_vRotPtOffset, LTRotation::GetIdentity());
		LTRigidTransform tPtOfRot = m_tOriginalTrans * tOffset;
		LTRigidTransform tRotation = tPtOfRot * LTRigidTransform(LTVector::GetIdentity(), rStartRot) * tPtOfRot.GetInverse() * m_tOriginalTrans;
		
		if( tRotation.m_rRot.GetComponentMagSqr( ) < 0.8f || LTIsNaN( tRotation.m_rRot ))
		{
			LTERROR( "Invalid transform." );
			tRotation.m_rRot.Identity();
		}

        g_pLTServer->SetObjectTransform( m_hObject, tRotation );
		
		// Set the initial state 
		m_nCurState = AWM_STATE_INITIAL;
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ActiveWorldModel::OnUpdate
//
//  PURPOSE:	Handle our update
//
// ----------------------------------------------------------------------- //

void ActiveWorldModel::OnUpdate( const double &fCurTime )
{
	// No need to call WorldModel::OnUpdate()

	SetNextUpdate( UPDATE_NEXT_FRAME );

	// Clear the blocked flag.

	m_bBlocked = false;

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

		case AWM_STATE_ROTATE:
		{
			UpdateRotate( fCurTime );
		}
		break;

		case AWM_STATE_MOVE:
		{
			UpdateMove( fCurTime );
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

void ActiveWorldModel::UpdateOn( const double &fCurTime )
{
	// Immediately try turnning off

	SetPowerOff( m_fPowerOffTime, m_nInitWaveform );
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ActiveWorldModel::UpdatePowerOn
//
//  PURPOSE:	Handel updating the PowerOn state
//
// ----------------------------------------------------------------------- //

void ActiveWorldModel::UpdatePowerOn( const double &fCurTime )
{
	if( fCurTime < m_fMoveStartTm ) return;

	LTVector	vNewPos;
	LTRotation	rNewRot;
	bool		bDoneInX = false;
	bool		bDoneInY = false;
	bool		bDoneInZ = false;

	LTVector	vOldAngles( m_fPitch, m_fYaw, m_fRoll );
	LTVector	vOldPos;

	g_pLTServer->GetObjectPos( m_hObject, &vOldPos );

	float		fPowerOnTime = m_fActiveTime * m_fPowerOnMultiplier;
	float		fPercent = 1.0f;
	if( fPowerOnTime > 0.001f )
	{
		fPercent = m_fCurTm / fPowerOnTime;
	}
	fPercent = Clamp( fPercent, 0.0f, 1.0f );

	// Calculate new pitch yaw and roll...

	bDoneInX = CalcAngle( m_fPitch,	m_vRotateFromAngles.x, m_vRotateToAngles.x, m_vRotateDir.x, fPowerOnTime, fPercent );
	bDoneInY = CalcAngle( m_fYaw,	m_vRotateFromAngles.y, m_vRotateToAngles.y, m_vRotateDir.y, fPowerOnTime, fPercent );
	bDoneInZ = CalcAngle( m_fRoll,	m_vRotateFromAngles.z, m_vRotateToAngles.z, m_vRotateDir.z, fPowerOnTime, fPercent );

	if( !CalculateNewPosRot( vNewPos, rNewRot, m_vOnPos, fPowerOnTime, fPercent, 
		( !(m_dwPropFlags & AWM_PROP_FORCEMOVE) && !(m_dwPropFlags & AWM_PROP_FORCEMOVEON))))
	{
		// Restore our angles...

		m_fPitch = vOldAngles.x;
		m_fYaw	 = vOldAngles.y;
		m_fRoll	 = vOldAngles.z;

		m_bBlocked = true;

		return;
	}

	g_pLTServer->Physics( )->MoveObject( m_hObject, vNewPos, 0 );
	
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
	
	if( rNewRot.GetComponentMagSqr( ) < 0.8f || LTIsNaN( rNewRot ))
	{
		LTERROR( "Invalid transform." );
		rNewRot.Identity();
	}

	g_pLTServer->RotateObject( m_hObject, rNewRot );
	
	// See if we are done updating the PowerOn state

	if( (m_nAWMType == AWM_TYPE_SLIDING) && (vNewPos == m_vOnPos) )
	{
		SetOn( !AWM_INITIAL_STATE );
	}
	else if( (m_nAWMType == AWM_TYPE_ROTATING) && (bDoneInX && bDoneInY && bDoneInZ) )
	{
		SetOn( !AWM_INITIAL_STATE );
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

void ActiveWorldModel::UpdateOff( const double &fCurTime )
{
	// Do nothing, we should never get here

	LTASSERT(false, "TODO: Add description here");
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ActiveWorldModel::UpdatePowerOff
//
//  PURPOSE:	Handel updating the PowerOff state
//
// ----------------------------------------------------------------------- //

void ActiveWorldModel::UpdatePowerOff( const double &fCurTime )
{
	// Check to see if we can actually poweroff (i.e., no characters are in our 
	// poweroff radius)

	if (m_bCheckForChars && m_fPowerOffRadius > 0.0f)
	{
		if ( g_pCharacterMgr->FindCharactersWithinRadius( NULL, m_vOffPos, m_fPowerOffRadius, NULL) )
		{
			// Can't update the poweroff state until there are no more
			// characters in our power off radius...
			return;
		}
		else
		{
			m_bCheckForChars = false;
			FinishSetPowerOff();
		}
	}


	if( fCurTime < m_fMoveStartTm + m_fMoveDelay ) return;

	LTVector	vNewPos;
	LTRotation	rNewRot;
    bool		bDoneInX = false;
    bool		bDoneInY = false;
    bool		bDoneInZ = false;

    LTVector	vOldAngles(m_fPitch, m_fYaw, m_fRoll);
	LTVector	vOldPos;

	g_pLTServer->GetObjectPos( m_hObject, &vOldPos );

	float	fPercent = 1.0f;
	if( m_fPowerOffTime > 0.001f )
	{
		fPercent = m_fCurTm / m_fActiveTime;
	}
	fPercent = Clamp( fPercent, 0.0f, 1.0f );

	// Calculate new pitch, yaw, and roll...

	bDoneInX = CalcAngle(m_fPitch, m_vRotateFromAngles.x, m_vRotateToAngles.x, m_vRotateDir.x, m_fPowerOffTime, fPercent);
	bDoneInY = CalcAngle(m_fYaw,   m_vRotateFromAngles.y, m_vRotateToAngles.y, m_vRotateDir.y, m_fPowerOffTime, fPercent);
	bDoneInZ = CalcAngle(m_fRoll,  m_vRotateFromAngles.z, m_vRotateToAngles.z, m_vRotateDir.z, m_fPowerOffTime, fPercent);

	if( !CalculateNewPosRot( vNewPos, rNewRot, m_vOffPos, m_fPowerOffTime, fPercent, 
		( !(m_dwPropFlags & AWM_PROP_FORCEMOVE) && !(m_dwPropFlags & AWM_PROP_FORCEMOVEOFF))))
	{
		// Restore our angles...

		m_fPitch = vOldAngles.x;
		m_fYaw	 = vOldAngles.y;
		m_fRoll	 = vOldAngles.z;

		return;
	}

	g_pLTServer->Physics( )->MoveObject( m_hObject, vNewPos, 0 );
	
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

	if( rNewRot.GetComponentMagSqr( ) < 0.8f || LTIsNaN( rNewRot ))
	{
		LTERROR( "Invalid transform." );
		rNewRot.Identity();
	}

	g_pLTServer->RotateObject( m_hObject, rNewRot );	

	if( (m_nAWMType == AWM_TYPE_SLIDING) && (vNewPos == m_vOffPos) )
	{
		SetOff( !AWM_INITIAL_STATE );
	}
	else if( (m_nAWMType == AWM_TYPE_ROTATING) && (bDoneInX && bDoneInY && bDoneInZ) )
	{
		SetOff( !AWM_INITIAL_STATE );
	}

	m_fCurTm += g_pLTServer->GetFrameTime();

}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ActiveWorldModel::UpdatePowerOff
//
//  PURPOSE:	Handel updating the PowerOff state
//
// ----------------------------------------------------------------------- //

void ActiveWorldModel::UpdateRotate( const double &fCurTime )
{
	if( fCurTime < m_fMoveStartTm )
		return;

	LTVector	vNewPos;
	LTRotation	rNewRot;
	bool		bDoneInX = false;
	bool		bDoneInY = false;
	bool		bDoneInZ = false;

	LTVector	vOldAngles( m_fPitch, m_fYaw, m_fRoll );
	LTVector	vOldPos;

	g_pLTServer->GetObjectPos( m_hObject, &vOldPos );

	float		fPowerOnTime = m_fActiveTime * m_fPowerOnMultiplier;
	float		fPercent = 1.0f;
	if( fPowerOnTime > 0.001f )
	{
		fPercent = m_fCurTm / fPowerOnTime;
	}
	fPercent = Clamp( fPercent, 0.0f, 1.0f );

	// Calculate new pitch yaw and roll...

	bDoneInX = CalcAngle( m_fPitch,	m_vRotateFromAngles.x, m_vRotateToAngles.x, m_vRotateDir.x, fPowerOnTime, fPercent );
	bDoneInY = CalcAngle( m_fYaw,	m_vRotateFromAngles.y, m_vRotateToAngles.y, m_vRotateDir.y, fPowerOnTime, fPercent );
	bDoneInZ = CalcAngle( m_fRoll,	m_vRotateFromAngles.z, m_vRotateToAngles.z, m_vRotateDir.z, fPowerOnTime, fPercent );

	if( !CalculateNewPosRot( vNewPos, rNewRot, m_vOnPos, fPowerOnTime, fPercent, 
		( !(m_dwPropFlags & AWM_PROP_FORCEMOVE) && !(m_dwPropFlags & AWM_PROP_FORCEMOVEON))))
	{
		// Restore our angles...
		m_fPitch = vOldAngles.x;
		m_fYaw	 = vOldAngles.y;
		m_fRoll	 = vOldAngles.z;

		m_bBlocked = true;

		return;
	}

	g_pLTServer->Physics( )->MoveObject( m_hObject, vNewPos, 0 );

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

	if( rNewRot.GetComponentMagSqr( ) < 0.8f || LTIsNaN( rNewRot ))
	{
		LTERROR( "Invalid transform." );
		rNewRot.Identity();
	}

	g_pLTServer->RotateObject( m_hObject, rNewRot );

	// See if we are done updating the Rotate state
	if( (m_nAWMType == AWM_TYPE_SLIDING) && (vNewPos == m_vOnPos) )
	{
		// Set on as initial so we don't play sounds or commands...
		SetOn( AWM_INITIAL_STATE );
	}
	else if( (m_nAWMType == AWM_TYPE_ROTATING) && (bDoneInX && bDoneInY && bDoneInZ) )
	{
		SetOn( AWM_INITIAL_STATE );
	}

	m_fCurTm += g_pLTServer->GetFrameTime();
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ActiveWorldModel::UpdatePowerOff
//
//  PURPOSE:	Handel updating the PowerOff state
//
// ----------------------------------------------------------------------- //

void ActiveWorldModel::UpdateMove( const double &fCurTime )
{
	LTVector	vNewPos;
	LTRotation	rNewRot;
	LTVector	vOldPos;

	g_pLTServer->GetObjectPos( m_hObject, &vOldPos );

	float		fPowerOnTime = m_fActiveTime * m_fPowerOnMultiplier;
	float		fPercent = 1.0f;
	if( fPowerOnTime > 0.001f )
	{
		fPercent = m_fCurTm / fPowerOnTime;
	}
	fPercent = Clamp( fPercent, 0.0f, 1.0f );

	if( !CalculateNewPosRot( vNewPos, rNewRot, m_vMoveToPos, fPowerOnTime, fPercent, 
		( !(m_dwPropFlags & AWM_PROP_FORCEMOVE) && !(m_dwPropFlags & AWM_PROP_FORCEMOVEON))))
	{
		m_bBlocked = true;
		return;
	}

	g_pLTServer->Physics( )->MoveObject( m_hObject, vNewPos, 0 );

	// Check to see if we actually moved anywhere...

	LTVector	vPos;
	g_pLTServer->GetObjectPos( m_hObject, &vPos );
	if( !vPos.NearlyEquals( vNewPos, MATH_EPSILON ) )
		return;

	// See if we are done updating the Move state
	if( (m_nAWMType == AWM_TYPE_SLIDING) && (vNewPos == m_vMoveToPos) )
	{
		// Set on as initial so we don't play sounds or commands...
		SetOn( AWM_INITIAL_STATE );
	}
	
	m_fCurTm += g_pLTServer->GetFrameTime();
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ActiveWorldModel::ToggleState
//
//  PURPOSE:	Toggles the world model. Switch state
//
// ----------------------------------------------------------------------- //

void ActiveWorldModel::ToggleState( float fTime /*= -1.0f*/, uint8 nWaveform /*= (uint8)-1*/ )
{
	// Are we locked...

	if( m_dwPropFlags & AWM_PROP_LOCKED )
	{
		// Play a sound if one was specified...

		if( !m_sLockedSnd.empty() )
			StartSound( m_sLockedSnd.c_str(), false );

		// Send a command if we have one...

		if( !m_sLockedCmd.empty() )
		{
			g_pCmdMgr->QueueCommand( m_sLockedCmd.c_str(), m_hActivateObj, m_hActivateObj );
		}

		// We are locked so don't try and change states

		return;			
	}

	// We are not locked so do our thing...

	// Hanlde a state change...

	double fCurTime = g_pLTServer->GetTime();

	switch( m_nCurState )
	{
		case AWM_STATE_OFF:
		{
			if( fCurTime >= m_fMoveStopTm + m_fOffWaitTm )
			{
				// We are now going to turn on

				SetPowerOn( fTime, nWaveform );
			}
		}
		break;

		case AWM_STATE_POWEROFF:
		{
			ChangeDir( fTime, nWaveform );
		}	
		break;

		case AWM_STATE_POWERON:
		{
			ChangeDir( fTime, nWaveform );
		}	
		break;

		case AWM_STATE_ON:
		{
			if( fCurTime >= m_fMoveStopTm + m_fOnWaitTm )
			{
				// We are going to turn off
				SetPowerOff( fTime, nWaveform );
			}
		}
		break;

		case AWM_STATE_INITIAL:
		{
			SetPowerOn( fTime, nWaveform );
		}
		break;

		default :
		break;
	};
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
	if( !hObj )
		return;

	// Only characters can activate us...
	if( !IsCharacter( hObj ) )
		return;

	// If the object is an AI make sure we can be activated by AI...

	bool	bAIActivated = IsAI( hObj );
	if( bAIActivated && !(m_dwPropFlags & AWM_PROP_AIACTIVATE) )
		return;

	// Can we be activated by a player

	if( !bAIActivated && !(m_dwPropFlags & AWM_PROP_PLAYERACTIVATE) )
		return;

	SetActiveObj( hObj );

	// Lets do what we came here to do
	ToggleState( );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ActiveWorldModel::HandleActivateMsg
//
//  PURPOSE:	Handle a ACTIVATE message...
//
// ----------------------------------------------------------------------- //

void ActiveWorldModel::HandleActivateMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg  )
{
	// Let our base handle it first...
	WorldModel::HandleActivateMsg( hSender, crParsedMsg );

	Activate( hSender );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ActiveWorldModel::HandleToggleMsg
//
//  PURPOSE:	Handle a TOGGLE message...
//
// ----------------------------------------------------------------------- //

void ActiveWorldModel::HandleToggleMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	ToggleState( );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ActiveWorldModel::HandleTriggerMsg
//
//  PURPOSE:	Handle a ON message...
//
// ----------------------------------------------------------------------- //

void ActiveWorldModel::HandleOnMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	float fTime = -1.0f;
	uint8 nWaveform = (uint8)-1;

	if( crParsedMsg.GetArgCount( ) > 1 )
	{
		fTime = (float)atof( crParsedMsg.GetArg( 1 ));
	}

	if( crParsedMsg.GetArgCount( ) > 2 )
	{
		for( uint8 nWave = 0; nWave <= AWM_WAVE_MAXTYPES; ++nWave )
		{
			if( LTStrIEquals( crParsedMsg.GetArg( 2 ).c_str( ), c_aWaveTypes[nWave] ) )
			{
				nWaveform = nWave;
				break;
			}
		}	
	}

	if( m_nCurState == AWM_STATE_OFF || m_nCurState == AWM_STATE_POWEROFF )
	{
		SetActiveObj( hSender );
		ToggleState( fTime, nWaveform );
	}
	else if( m_nCurState == AWM_STATE_INITIAL || m_nCurState == AWM_STATE_ROTATE || m_nCurState == AWM_STATE_MOVE )
	{
		SetActiveObj( hSender );
		SetPowerOn( fTime, nWaveform );
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ActiveWorldModel::HandleOffMsg
//
//  PURPOSE:	Handle a OFF message...
//
// ----------------------------------------------------------------------- //

void ActiveWorldModel::HandleOffMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	float fTime = -1.0f;
	uint8 nWaveform = (uint8)-1;

	if( crParsedMsg.GetArgCount( ) > 1 )
	{
		fTime = (float)atof( crParsedMsg.GetArg( 1 ));
	}

	if( crParsedMsg.GetArgCount( ) > 2 )
	{
		for( uint8 nWave = 0; nWave <= AWM_WAVE_MAXTYPES; ++nWave )
		{
			if( LTStrIEquals( crParsedMsg.GetArg( 2 ).c_str( ), c_aWaveTypes[nWave] ) )
			{
				nWaveform = nWave;
				break;
			}
		}	
	}
	
	if( m_nCurState == AWM_STATE_ON || m_nCurState == AWM_STATE_POWERON )
	{
		SetActiveObj( hSender );
		ToggleState( fTime, nWaveform );
	}
	else if( m_nCurState == AWM_STATE_INITIAL || m_nCurState == AWM_STATE_ROTATE || m_nCurState == AWM_STATE_MOVE )
	{	
		SetActiveObj( hSender );
		SetPowerOff( fTime, nWaveform );
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ActiveWorldModel::HandlePause
//
//  PURPOSE:	Handle a PAUSE command
//
// ----------------------------------------------------------------------- //

void ActiveWorldModel::HandlePauseMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
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

void ActiveWorldModel::HandleResumeMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	if( m_nCurState == AWM_STATE_PAUSE )
	{
		m_nCurState = m_nLastState;
	
		// Recalculate the on/off pos incase we were moved by a keyframer (movement only?? )

		LTVector	vPos;
		g_pLTServer->GetObjectPos( m_hObject, &vPos );
			
		LTVector	vDelta = m_vPausePos - vPos;
								
		m_tOriginalTrans.m_vPos	-= vDelta;
		m_vOffPos				-= vDelta;
		m_vOnPos				-= vDelta;

		if( m_nAWMType == AWM_TYPE_ROTATING ) 
		{
			// Recalculate the point we rotate around incase we were moved by a keyframer...

			if( m_nCurState == AWM_STATE_OFF )
			{
				m_vRotationPt = m_tOriginalTrans * m_vRotPtOffset;
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ActiveWorldModel::HandleLockMsg
//
//  PURPOSE:	Handle a LOCK message...
//
// ----------------------------------------------------------------------- //

void ActiveWorldModel::HandleLockMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	Lock( true );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ActiveWorldModel::HandleUnlockMsg
//
//  PURPOSE:	Handle a UNLOCK message...
//
// ----------------------------------------------------------------------- //

void ActiveWorldModel::HandleUnlockMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	Lock( false );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ActiveWorldModel::HandleSetPowerOnMultiplayerMsg
//
//  PURPOSE:	Handle a SETPOWERONMULTIPLIER message...
//
// ----------------------------------------------------------------------- //

void ActiveWorldModel::HandleSetPowerOnMultiplayerMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	if( crParsedMsg.GetArgCount() > 1 )
	{
		m_fPowerOnMultiplier = (float)atof( crParsedMsg.GetArg(1) );
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ActiveWorldModel::HandleCanActivateMsg
//
//  PURPOSE:	Handle a CANACTIVATE message...
//
// ----------------------------------------------------------------------- //

void ActiveWorldModel::HandleCanActivateMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	if( crParsedMsg.GetArgCount( ) > 1 )
	{
		bool bCanActivate = IsTrueChar( *crParsedMsg.GetArg( 1 ));
		m_ActivateTypeHandler.SetCanActivate( bCanActivate );
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ActiveWorldModel::HandleSetStartingAnglesMsg
//
//  PURPOSE:	Handle a SETSTARTINGANGLES message...
//
// ----------------------------------------------------------------------- //

void ActiveWorldModel::HandleSetStartingAnglesMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	if( crParsedMsg.GetArgCount( ) > 3 )
	{
		LTVector vStartingAngles(
			(float)atof( crParsedMsg.GetArg( 1 )) * (MATH_PI / 180.0f),
			(float)atof( crParsedMsg.GetArg( 2 )) * (MATH_PI / 180.0f),
			(float)atof( crParsedMsg.GetArg( 3 )) * (MATH_PI / 180.0f));
		SetStartingAngles( vStartingAngles );
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ActiveWorldModel::HandleRotateMsg
//
//  PURPOSE:	Handle a ROTATE message...
//
// ----------------------------------------------------------------------- //

void ActiveWorldModel::HandleRotateMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	float fRotX = MATH_DEGREES_TO_RADIANS( (float)atof( crParsedMsg.GetArg( 1 )) );
	float fRotY = MATH_DEGREES_TO_RADIANS( (float)atof( crParsedMsg.GetArg( 2 )) );
	float fRotZ = MATH_DEGREES_TO_RADIANS( (float)atof( crParsedMsg.GetArg( 3 )) );

	float fTime = m_fPowerOnTime;
	uint8 nWaveform = m_nInitWaveform;

	if( crParsedMsg.GetArgCount( ) > 4 )
	{
		fTime = (float)atof( crParsedMsg.GetArg( 4 ));
	}

	if( crParsedMsg.GetArgCount( ) > 5 )
	{
		for( uint8 nWave = 0; nWave <= AWM_WAVE_MAXTYPES; ++nWave )
		{
			if( LTStrIEquals( crParsedMsg.GetArg( 5 ).c_str( ), c_aWaveTypes[nWave] ) )
			{
				nWaveform = nWave;
				break;
			}
		}	
	}

	SetRotate( fRotX, fRotY, fRotZ, fTime, nWaveform );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ActiveWorldModel::HandleMoveMsg
//
//  PURPOSE:	Handle a MOVE message...
//
// ----------------------------------------------------------------------- //

void ActiveWorldModel::HandleMoveMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	float fDistX = (float)atof( crParsedMsg.GetArg( 1 ));
	float fDistY = (float)atof( crParsedMsg.GetArg( 2 ));
	float fDistZ = (float)atof( crParsedMsg.GetArg( 3 ));

	float fTime = m_fPowerOnTime;
	uint8 nWaveform = m_nInitWaveform;

	if( crParsedMsg.GetArgCount( ) > 4 )
	{
		fTime = (float)atof( crParsedMsg.GetArg( 4 ));
	}

	if( crParsedMsg.GetArgCount( ) > 5 )
	{
		for( uint8 nWave = 0; nWave <= AWM_WAVE_MAXTYPES; ++nWave )
		{
			if( LTStrIEquals( crParsedMsg.GetArg( 5 ).c_str( ), c_aWaveTypes[nWave] ) )
			{
				nWaveform = nWave;
				break;
			}
		}	
	}

	SetMove( fDistX, fDistY, fDistZ, fTime, nWaveform );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ActiveWorldModel::Lock
//
//  PURPOSE:	Handle a LOCK or UNLOCK command
//
// ----------------------------------------------------------------------- //

void ActiveWorldModel::Lock( bool bLock )
{
	if( bLock )
	{
		m_dwPropFlags |= AWM_PROP_LOCKED;
	}
	else
	{
		m_dwPropFlags &= ~AWM_PROP_LOCKED;
	}

	m_ActivateTypeHandler.SetDisabled( bLock );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ActiveWorldModel::SetPowerOn
//
//  PURPOSE:	Sets the AWM to the PowerOn state
//
// ----------------------------------------------------------------------- //

void ActiveWorldModel::SetPowerOn( double fTime, uint8 nWaveform )
{
	// Recalculate the on/off pos in case we were moved by a keyframer (movement only?? )

	if( m_nCurState == AWM_STATE_OFF )
	{
		g_pLTServer->GetObjectTransform( m_hObject, &m_tOriginalTrans );
		if( m_tOriginalTrans.m_rRot.GetComponentMagSqr( ) < 0.8f || LTIsNaN( m_tOriginalTrans.m_rRot ))
		{
			LTERROR( "Invalid original transform." );
			m_tOriginalTrans.m_rRot.Identity();
		}

		m_vOffPos = m_tOriginalTrans.m_vPos;
		
		LTVector vTemp = m_tOriginalTrans.m_rRot.RotateVector(m_vMoveDir * m_fInitMoveDist);
		m_vOnPos = m_vOffPos + vTemp;

		// Remove previous movement...
		if( m_vMoveDistances.MagSqr( ) > 0.0f )
		{
			m_vOffPos -= m_vMoveDistances - vTemp;
			m_vMoveDistances.Init( );
		}
	}

	// Move the initial distance specified by the object...
	m_fMoveDist = (m_vOnPos - m_vOffPos).Mag( );

	// Flip direction and angles if we want to rotate away form the active obj... 

	if( m_nAWMType == AWM_TYPE_ROTATING ) 
	{
		// Determine the starting angle of the rotation...
		if( m_nCurState == AWM_STATE_INITIAL )
		{
			m_vRotateFromAngles = m_vStartingAngles;
		}
		else
		{
			m_vRotateFromAngles.Init( m_fPitch, m_fYaw, m_fRoll );
		}
		
		// Normal rotation...
		m_vRotateDir		= m_vInitRotDir;
		m_vRotateToAngles	= m_vInitOnAngles;

		if( (m_dwPropFlags & AWM_PROP_ROTATEAWAY) && m_hActivateObj )
		{
			// To calculate the correct direction to open, calculate the position
			// of the AWM in the closed and opened position.  Take the cross product
			// of those two vectors, relative to the rotation point, and compare
			// it with the cross product of the activator's position and the
			// AWM's current position.  If those two cross products are in 
			// opposite directions, the rotation needs to be reversed.
			// The one other complication is that all those relative vectors
			// need to be in the plane of the rotation (the plane formed by
			// the arc of the position change from closed to open).

			LTVector	vObjPos;
			g_pLTServer->GetObjectPos( m_hActivateObj, &vObjPos );

			LTVector	vCurrentPos;
			g_pLTServer->GetObjectPos( m_hObject, &vCurrentPos );

			LTVector	vOldAngles( m_fPitch, m_fYaw, m_fRoll );

			const float	cf0Percent = 0.0f;
			const float	cf100Percent = 1.0f;
			
			LTVector	vOffPos, vOnPos;
			LTRotation	rOffRot, rOnRot;

			// Calculate the AWM's rest position...
			m_fPitch	= m_vInitOffAngles.x;
			m_fYaw		= m_vInitOffAngles.y;
			m_fRoll		= m_vInitOffAngles.z;

			CalculateNewPosRot( vOffPos, rOffRot, LTVector(0,0,0), m_fPowerOnTime, cf0Percent );

			// Calculate the AWM's position if it rotated normally...
			m_fPitch	= m_vInitOnAngles.x;
			m_fYaw		= m_vInitOnAngles.y;
			m_fRoll		= m_vInitOnAngles.z;


			CalculateNewPosRot( vOnPos, rOnRot, LTVector(0,0,0), m_fPowerOnTime, cf100Percent );

			// Restore the real angles...
			m_fPitch	= vOldAngles.x;
			m_fYaw		= vOldAngles.y;
			m_fRoll		= vOldAngles.z;


			
			// We want to determine the axis of rotation so that we can do all of our calculations
			// in the plane of the rotation.
			const LTRotation rOffToOn = rOnRot*(~rOffRot);
			const LTVector vAxis = LTVector(rOffToOn.m_Quat[LTRotation::QX], rOffToOn.m_Quat[LTRotation::QY], rOffToOn.m_Quat[LTRotation::QZ]).GetUnit();

			// Determine our positions relative to the rotation point in the plane of the rotation.
			const LTVector vFlatOffPos = RemoveComponent(vOffPos - m_vRotationPt, vAxis);
			const LTVector vFlatOnPos = RemoveComponent(vOnPos - m_vRotationPt, vAxis);

			const LTVector vFlatObjPos = RemoveComponent(vObjPos - m_vRotationPt, vAxis);
			const LTVector vFlatCurrentPos = RemoveComponent(vCurrentPos - m_vRotationPt, vAxis);

			// If the cross-product from the activator to our current position is
			// in the opposite direction of the cross product of the off position to
			// the on position, then we need to rotate in the opposite
			// direction in order to swing away from the activator.
			const LTVector vOffCrossOn = vFlatOffPos.Cross(vFlatOnPos);
			const LTVector vObjCrossCurrent = vFlatObjPos.Cross(vFlatCurrentPos);
			if( vObjCrossCurrent.Dot(vOffCrossOn) < 0.0f )
			{
				// Opposite of normal rotation...
				m_vRotateDir		= -m_vInitRotDir; 
				m_vRotateToAngles	= -m_vInitOnAngles;
			}
		}
	}

	StartSound( m_sPowerOnSnd.c_str(), !!(m_dwPropFlags & AWM_PROP_LOOPSOUNDS) );

	SetNextUpdate( UPDATE_NEXT_FRAME );

	// Set the new state
	m_nCurState		= AWM_STATE_POWERON;
	m_nWaveform		= (nWaveform == (uint8)-1 ? m_nInitWaveform : nWaveform);
	m_fActiveTime	= (float)(fTime > 0.0f ? fTime : m_fPowerOnTime);

	// Send a command if we have one...
	if( !m_sPowerOnCmd.empty() )
	{
		g_pCmdMgr->QueueCommand( m_sPowerOnCmd.c_str(), m_hActivateObj, m_hActivateObj );
	}
	
	m_fMoveStartTm = g_pLTServer->GetTime() + m_fMoveDelay;
	m_fCurTm = 0.0f;

	// Register disturbance stimulus for sound and visual.

	RegisterDisturbanceStimulus(true);

	// Set the on state for our activate object.

	m_ActivateTypeHandler.SetState( ACTIVATETYPE::eOn );

	if( !(m_dwPropFlags & AWM_PROP_TRIGGEROFF) )
	{
		// The Player cannot use activate set the WorldModel off so remove the can activate flag...
		m_ActivateTypeHandler.SetCanActivate( false );
	}
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ActiveWorldModel::SetOn
//
//  PURPOSE:	Sets the AWM to the On state
//
// ----------------------------------------------------------------------- //

void ActiveWorldModel::SetOn( bool bInitialState )
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

	if( !bInitialState )
	{
		if( m_nAWMType == AWM_TYPE_ROTATING )
		{
			m_vRotateFromAngles = m_vInitOffAngles;
		}

		StartSound( m_sOnSnd.c_str(), false ); // the on sound NEVER loops

		// Send a command if we have one...

		if( !m_sOnCmd.empty() )
		{
			g_pCmdMgr->QueueCommand( m_sOnCmd.c_str(), m_hActivateObj, m_hActivateObj );
		}

		m_fMoveStopTm = g_pLTServer->GetTime();
	}

	// Register disturbance stimulus for visual.

	RegisterDisturbanceStimulus(false);

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

void ActiveWorldModel::SetPowerOff( double fTime, uint8 nWaveform )
{
	// Recalculate the on/off pos incase we were moved by a keyframer (movement only?? )

	if( m_nCurState == AWM_STATE_ON )
	{
		g_pLTServer->GetObjectPos( m_hObject, &m_vOnPos );

		LTVector vTemp = m_tOriginalTrans.m_rRot.RotateVector(m_vMoveDir * m_fInitMoveDist);
		m_vOffPos = m_vOnPos - vTemp;

		// Remove previous movement...
		if( m_vMoveDistances.MagSqr( ) > 0.0f )
		{
			m_vOffPos -= m_vMoveDistances - vTemp;
			m_vMoveDistances.Init( );
		}
	}

	SetNextUpdate( UPDATE_NEXT_FRAME );
	m_nCurState		= AWM_STATE_POWEROFF;
	m_nWaveform		= (nWaveform == (uint8)-1 ? m_nInitWaveform : nWaveform);
	m_fMoveDist		= (m_vOnPos - m_vOffPos).Mag( );
	m_fActiveTime	= (float)(fTime > 0.0f ? fTime : m_fPowerOffTime);

	// Check to see if we can actually poweroff (i.e., no characters are in our 
	// poweroff radius)

	if (m_fPowerOffRadius > 0.0f)
	{
		if ( g_pCharacterMgr->FindCharactersWithinRadius( NULL, m_vOffPos, m_fPowerOffRadius, NULL) )
		{
			// UpdatePowerOff will need to finish setting the power to off once
			// there are no more characters in the power off radius
			m_bCheckForChars = true;
			return;
		}
	}

	FinishSetPowerOff();
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ActiveWorldModel::FinishSetPowerOff
//
//  PURPOSE:	Finish setting the AWM to the PowerOff state
//
// ----------------------------------------------------------------------- //

void ActiveWorldModel::FinishSetPowerOff( )
{
	// Set the start and end angles...
	m_vRotateToAngles = m_vInitOffAngles;
	m_vRotateFromAngles.Init( m_fPitch, m_fYaw, m_fRoll );

	// Determine direction to rotate...
	m_vRotateDir.x = (m_vRotateFromAngles.x > m_vRotateToAngles.x ? -1.0f : 1.0f );
	m_vRotateDir.y = (m_vRotateFromAngles.y > m_vRotateToAngles.y ? -1.0f : 1.0f );
	m_vRotateDir.z = (m_vRotateFromAngles.z > m_vRotateToAngles.z ? -1.0f : 1.0f );

	StartSound( m_sPowerOffSnd.c_str(), !!(m_dwPropFlags & AWM_PROP_LOOPSOUNDS) );
	
	// Send a command if we have one...

	if( !m_sPowerOffCmd.empty() )
	{
		g_pCmdMgr->QueueCommand( m_sPowerOffCmd.c_str(), m_hActivateObj, m_hActivateObj );
	}

	m_fMoveStartTm = g_pLTServer->GetTime() + m_fMoveDelay;
	m_fCurTm = 0.0f;

	// Register disturbance stimulus for sound and visual.

	RegisterDisturbanceStimulus(true);

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

void ActiveWorldModel::SetOff( bool bInitialState )
{
	// Set the state and stop updating before we send any commands...

	SetNextUpdate( UPDATE_NEVER );
	m_nCurState = AWM_STATE_OFF;

	if( !bInitialState )
	{
		if( m_nAWMType == AWM_TYPE_ROTATING )
		{
			m_vRotateDir = m_vInitRotDir;
			m_vRotateToAngles = m_vInitOnAngles;
		}

		StartSound( m_sOffSnd.c_str(), false );	// the Off sound never loops

		// Send a command if we have one...

		if( !m_sOffCmd.empty() )
		{
			g_pCmdMgr->QueueCommand( m_sOffCmd.c_str(), m_hActivateObj, m_hActivateObj );
		}

		m_fMoveStopTm = g_pLTServer->GetTime();
	}

	// Register disturbance stimulus for visual.

	RegisterDisturbanceStimulus(false);

	// Set the off state for our activate object.

	m_ActivateTypeHandler.SetState( ACTIVATETYPE::eOff );

	// If the player was prohibitted from triggering the door off, undo this
	// and make the door activatable again.  Only do this if the worldmodel 
	// is flagged as activatable by players.
	// NOTE: This does not handle any changes made to the worldmodel via
	// the CANACTIVATE message.  If this causes issues, we may need to add more 
	// state checking/storing to HandleCanActivateMsg so that it updates 
	// AWM_PROP_PLAYERACTIVATE when it changes.

	if( !(m_dwPropFlags & AWM_PROP_TRIGGEROFF) 
		&& (m_dwPropFlags & AWM_PROP_PLAYERACTIVATE) )
	{
		m_ActivateTypeHandler.SetCanActivate( true );
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ActiveWorldModel::SetRotate
//
//  PURPOSE:	Sets the AWM to the Rotate state
//
// ----------------------------------------------------------------------- //

void ActiveWorldModel::SetRotate( float fRotX, float fRotY, float fRotZ, float fTime, uint8 nWaveform )
{
	// Set start and end angles...
	m_vRotateToAngles.Init( fRotX, fRotY, fRotZ );
	m_vRotateFromAngles.Init( m_fPitch, m_fYaw, m_fRoll );

	// Determine direction to rotate...
	m_vRotateDir.x = (m_vRotateFromAngles.x > m_vRotateToAngles.x ? -m_vInitRotDir.x : m_vInitRotDir.x );
	m_vRotateDir.y = (m_vRotateFromAngles.y > m_vRotateToAngles.y ? -m_vInitRotDir.y : m_vInitRotDir.y );
	m_vRotateDir.z = (m_vRotateFromAngles.z > m_vRotateToAngles.z ? -m_vInitRotDir.z : m_vInitRotDir.z );

	m_fActiveTime = fTime;
	m_nWaveform = nWaveform;

	m_fCurTm = 0.0f;

	m_nCurState = AWM_STATE_ROTATE;
	SetNextUpdate( UPDATE_NEXT_FRAME );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ActiveWorldModel::SetMove
//
//  PURPOSE:	Sets the AWM to the Move state
//
// ----------------------------------------------------------------------- //

void ActiveWorldModel::SetMove( float fDistX, float fDistY, float fDistZ, float fTime, uint8 nWaveform )
{
	LTVector vPos;
	g_pLTServer->GetObjectPos( m_hObject, &vPos );

	m_vMoveDistances.Init( fDistX, fDistY, fDistZ );
	m_vMoveToPos = m_tOriginalTrans.m_vPos + m_vMoveDistances;

	m_fMoveDist = (vPos - m_vMoveToPos).Mag( );

	m_fActiveTime = fTime;
	m_nWaveform = nWaveform;

	m_fCurTm = 0.0f;

	m_nCurState = AWM_STATE_MOVE;
	SetNextUpdate( UPDATE_NEXT_FRAME );
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

float ActiveWorldModel::GetWaveformValue( float fRate, float fPercent ) const
{
	// If we have linear motion just return the speed

	if( m_nWaveform == AWM_WAVE_LINEAR ) return fRate;

	float	fNewSpeed;
	float	fScalePercent = fRate * 0.001f;
	
	// Get speed based on waveform

	switch( m_nWaveform )
	{
		case AWM_WAVE_SINE:
		{
			fNewSpeed = MATH_HALFPI * fRate * (float)sin( fPercent * MATH_PI );
		}
		break; 

		case AWM_WAVE_SLOWOFF:
		{
			fNewSpeed = MATH_HALFPI * fRate * (float)cos( fPercent * MATH_HALFPI ) - fScalePercent;
		}
		break;

		case AWM_WAVE_SLOWON:
		{
			fNewSpeed = MATH_HALFPI * fRate * (float)sin( fPercent * MATH_HALFPI ) + fScalePercent;
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

void ActiveWorldModel::ChangeDir( float fTime, uint8 nWaveform )
{
	// If the force move option is set don't change

	if( m_dwPropFlags & AWM_PROP_FORCEMOVE ) return;

	if(( m_nCurState == AWM_STATE_POWERON ) && ( m_dwPropFlags & AWM_PROP_FORCEMOVEON ))
		return;
	if(( m_nCurState == AWM_STATE_POWEROFF ) && ( m_dwPropFlags & AWM_PROP_FORCEMOVEOFF ))
		return;


	LTVector	vTestPos;
	LTRotation	rTestRot;
	bool		bChange = true;

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
				SetPowerOn( fTime, nWaveform );
			}
			break;

			case AWM_STATE_POWERON:
			{
				SetPowerOff( fTime, nWaveform );
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

bool ActiveWorldModel::CalcAngle( float &fAngle, float fInitial, float fTarget, float fDir, float fTotalTime, float fPercent ) const
{
	// If were are not rotating then we are at the target angle

	if( m_nAWMType != AWM_TYPE_ROTATING ) return true;

	if( fPercent >= 1.0f ) 
	{
		fAngle = fTarget;
		return true;
	}

	bool	bRet		= false;	// Are we at the target angle?
	float	fRate		= ( fTarget - fInitial );
	if( fTotalTime > 0.001f )
	{
		fRate = fRate / fTotalTime;
	}
	float	fAmount		= GetWaveformValue( fRate, fPercent ) * g_pLTServer->GetFrameTime();


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
                bRet   = true;
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
                bRet   = true;
			}
		}
	}
	else
	{
        bRet = true;
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

bool ActiveWorldModel::CalculateNewPosRot( LTVector &vOutPos, LTRotation &rOutRot, const LTVector &vFinalPos, float fTotalTime, float fPercent, bool bFailOnCollision ) const
{
	LTRotation rObjRot;
	g_pLTServer->GetObjectRotation(m_hObject, &rObjRot);

	LTRotation	rRotation( m_fPitch, m_fYaw, m_fRoll );

	switch( m_nAWMType )
	{
	case AWM_TYPE_SLIDING:
		{
			rOutRot = rObjRot;
			g_pLTServer->GetObjectPos( m_hObject, &vOutPos );

			LTVector vDir = vFinalPos - vOutPos;
			float	fDistLeft = vDir.Mag();
			if( fDistLeft > 0.f )
			{
				vDir.Normalize();
			}

			float	fRate = m_fMoveDist;
			if( fTotalTime > 0.001f )
			{
				fRate = fRate / fTotalTime;
			}
			float fDistMove = GetWaveformValue( fRate, fPercent ) * g_pLTServer->GetFrameTime();

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
				return true;
			}

		}
		break;

	case AWM_TYPE_ROTATING:
		{
			if( m_vRotPtOffset.MagSqr() > 0.0f )
			{
				LTRigidTransform tOffset(m_vRotPtOffset, LTRotation::GetIdentity());
				LTRigidTransform tPtOfRot = m_tOriginalTrans * tOffset;
				LTRigidTransform tRotation = tPtOfRot * LTRigidTransform(LTVector::GetIdentity(), rRotation) * tPtOfRot.GetInverse() * m_tOriginalTrans;
				rOutRot = tRotation.m_rRot;
				rOutRot.Normalize();
				vOutPos = tRotation.m_vPos;
			}
			else
			{
				rOutRot = m_tOriginalTrans.m_rRot * rRotation;
				rOutRot.Normalize();
				vOutPos = m_tOriginalTrans.m_vPos;
			}
		}
		break;

	case AWM_TYPE_STATIC:
	default:
		break;

	}


	//we need to test for collisions even if we're forcing the move because the object we're 
	// colliding with might need to react (e.g. bodies need to gib)
	bool bCollision = TestObjectCollision( NULL, vOutPos, rRotation );
	if( bFailOnCollision )
	{
		return !bCollision;
	}

	return true;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ActiveWorldModel::GetTestPosRot
//
//  PURPOSE:	Get the test position and rotation of the AWM
//
// ----------------------------------------------------------------------- //

bool ActiveWorldModel::GetTestPosRot( LTVector &vTestPos, LTRotation &rTestRot )
{
	LTVector	vFinalPos;
	float		fTime;
	LTVector	vOldAngles(m_fPitch, m_fYaw, m_fRoll);
	
	const float	cf10Percent = 0.1f;

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
				CalcAngle( m_fPitch, m_fPitch,	m_vRotateFromAngles.x, -m_vRotateDir.x, fTime, cf10Percent );
				CalcAngle( m_fYaw,   m_fYaw,	m_vRotateFromAngles.y, -m_vRotateDir.y, fTime, cf10Percent );
				CalcAngle( m_fRoll,  m_fRoll,	m_vRotateFromAngles.z, -m_vRotateDir.z, fTime, cf10Percent );
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
				CalcAngle( m_fPitch, m_fPitch,	m_vRotateFromAngles.x, -m_vRotateDir.x, fTime, cf10Percent );
				CalcAngle( m_fYaw,   m_fYaw,	m_vRotateFromAngles.y, -m_vRotateDir.y, fTime, cf10Percent );
				CalcAngle( m_fRoll,  m_fRoll,	m_vRotateFromAngles.z, -m_vRotateDir.z, fTime, cf10Percent );
			}
		}
		break;

		// If we are in any other state we are not moving, so there is nothing to test

		default :
			return false;
		break;

	};

	CalculateNewPosRot( vTestPos, rTestRot, vFinalPos, fTime, cf10Percent );

	// Restore real angles...

	m_fPitch = vOldAngles.x;
	m_fYaw	 = vOldAngles.y;
	m_fRoll	 = vOldAngles.z;

	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ActiveWorldModel::TestObjectCollision
//
//  PURPOSE:	See if the test object would collide with the AWM if the AWM were in
//				the test position and rotation
//
// ----------------------------------------------------------------------- //

bool ActiveWorldModel::TestObjectCollision( HOBJECT hTest, const LTVector &vTestPos, const LTRotation &rTestRot, HOBJECT *pCollisionObj ) const
{
	// Get an array of all objects that would intersect with us... remember that there may be 
	// bulletholes and model props attached to us so make the array some what large.

	ObjArray<HOBJECT, 100> objArray;
	if( g_pLTServer->FindWorldModelObjectIntersections( m_hObject, vTestPos, rTestRot, objArray ) != LT_OK )
	{
		return false;
	}

	// Loop through all the objects that would collide with us and check against the test object...

	HOBJECT hObj = NULL;
	for( uint32 i = 0; i < objArray.NumObjects(); ++i )
	{
		hObj = objArray.GetObject( i );
		if( hObj )
		{
			if	(IsPlayer( hObj ))
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
								return false;
							}
							else
							{
								return true;
							}
						}
						else
						{
							return true;
						}
					}
					else
					{
						return true;
					}
				}
			}
			if	(IsAI( hObj ))
			{
				CCharacter* pChar = dynamic_cast<CCharacter*>(g_pLTServer->HandleToObject( hObj ));
				if (pChar->IsDead())
				{
					//if we are in a forced move situation, and there is a dead AI in the way, pop it
					if  ( (m_dwPropFlags & AWM_PROP_FORCEMOVE ) ||
						  ( (m_nCurState == AWM_STATE_POWERON ) && ( m_dwPropFlags & AWM_PROP_FORCEMOVEON )) ||
						  ( (m_nCurState == AWM_STATE_POWEROFF) && ( m_dwPropFlags & AWM_PROP_FORCEMOVEOFF ))
						)
					{
						pChar->ForceGib();
					}
				}

			}

			
		}
	}

	return false;
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

void ActiveWorldModel::RegisterDisturbanceStimulus(bool bAudioStim)
{
	// Register audio and visual stimulus if specified and triggered.

	if( (m_hActivateObj == NULL) ||
		(m_nActivateAlarmLevel == 0) || 
		(m_DamageWorldModel.GetDestroyedStimulusRadius( ) <= 0.0f) || 
		(!IsCharacter(m_hActivateObj)) )
	{
		return;
	}


	LTVector vPos;
	g_pLTServer->GetObjectPos(m_hObject, &vPos);

	// Add or remove a visual stimulus, depending on the starting state of the object.

	if(m_nCurState != m_nStartingState)
	{
		if( m_eStimID == kStimID_Unset )
		{
			// Search for an AINodeSmartObject pointing at this object.

			if( !m_bSearchedForNode )
			{
				// [jeffo 4/7/04]
				// Removed support for disturbance nodes.
				m_bSearchedForNode = true;
			}

			// If a node exists, toggle it to its Disturbed state.
			/***
			if( m_hSmartObjectNode )
			{
				AINodeSmartObject* pSmartObjectNode = (AINodeSmartObject*)g_pLTServer->HandleToObject( m_hSmartObjectNode );
				AIASSERT( pSmartObjectNode, m_hObject, "ActiveWorldModel::RegisterDisturbanceStimulus: Cannot find SmartObject Node" );

				pSmartObjectNode->SetSmartObjectState( kState_SmartObjectDisturbed );
			}
			***/
		}
	}

	// If a node exists, toggle it to its Default state.
	
	else if(m_eStimID != kStimID_Unset)
	{
		g_pAIStimulusMgr->RemoveStimulus(m_eStimID);
		m_eStimID = kStimID_Unset;

		/****
		if( m_hSmartObjectNode )
		{
			AINodeSmartObject* pSmartObjectNode = (AINodeSmartObject*)g_pLTServer->HandleToObject( m_hSmartObjectNode );
			AIASSERT( pSmartObjectNode, m_hObject, "ActiveWorldModel::RegisterDisturbanceStimulus: Cannot find SmartObject Node" );

			pSmartObjectNode->SetSmartObjectState( kState_SmartObjectDefault );
		}
		****/
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ActiveWorldModel::StartSound
//
//  PURPOSE:	Start playing a sound given a filename
//
// ----------------------------------------------------------------------- //

void ActiveWorldModel::StartSound( const char *pSoundName, const bool bLoop )
{
	// Kill any currently playing sound

	StopSound();

	if( !pSoundName || !pSoundName[0] ) return;

	uint32 dwSndFlags = PLAYSOUND_3D | PLAYSOUND_REVERB | PLAYSOUND_USEOCCLUSION;

	if( bLoop & !!(m_dwPropFlags & AWM_PROP_LOOPSOUNDS) )
	{
		dwSndFlags |= PLAYSOUND_LOOP | PLAYSOUND_GETHANDLE;
	}

	// check if it's a valid record. If it is, we want to pass
	// it through the database sound stuff. Otherwise, it's
	// to be played as a wave file. I want all sounds
	// to go through the database eventually, so this is
	// for backwards compatibility. -- Terry
	HRECORD hSoundSR = g_pSoundDB->GetSoundDBRecord( pSoundName );

	// If we didn't specify a position attach it to the object

	if( m_vSoundPos.NearlyEquals( LTVector( 0.0f, 0.0f, 0.0f ), 0.0f ))
	{
		dwSndFlags |= PLAYSOUND_ATTACHED;

		if (hSoundSR)
		{
			m_hsndLastSound = g_pServerSoundMgr->PlayDBSoundFromObject( m_hObject, hSoundSR, m_fSoundRadius, SOUNDPRIORITY_MISC_MEDIUM, 
				dwSndFlags, SMGR_DEFAULT_VOLUME, 1.0f, m_fSoundRadius * 0.25f,
				DEFAULT_SOUND_CLASS, PLAYSOUND_MIX_OBJECTS);
		}
		else
		{
			m_hsndLastSound = g_pServerSoundMgr->PlaySoundFromObject( m_hObject, pSoundName, NULL, m_fSoundRadius, SOUNDPRIORITY_MISC_MEDIUM, 
																  dwSndFlags | PLAYSOUND_USEOCCLUSION, SMGR_DEFAULT_VOLUME, 1.0f, m_fSoundRadius * 0.25f,
																  DEFAULT_SOUND_CLASS, PLAYSOUND_MIX_OBJECTS);
		}
	}
	else
	{
		LTVector vPos, vSndPos;
		g_pLTServer->GetObjectPos( m_hObject, &vPos );
		vSndPos = vPos + m_vSoundPos;

		if (hSoundSR)
		{
			m_hsndLastSound = g_pServerSoundMgr->PlayDBSoundFromPos( vSndPos, hSoundSR, m_fSoundRadius, SOUNDPRIORITY_MISC_MEDIUM,
				dwSndFlags, SMGR_DEFAULT_VOLUME, 1.0f, m_fSoundRadius * 0.25f,
				DEFAULT_SOUND_CLASS, PLAYSOUND_MIX_OBJECTS); 
		}
		else
		{
			m_hsndLastSound = g_pServerSoundMgr->PlaySoundFromPos( vSndPos, pSoundName, NULL, m_fSoundRadius, SOUNDPRIORITY_MISC_MEDIUM,
															   dwSndFlags | PLAYSOUND_USEOCCLUSION, SMGR_DEFAULT_VOLUME, 1.0f, m_fSoundRadius * 0.25f,
															   DEFAULT_SOUND_CLASS, PLAYSOUND_MIX_OBJECTS); 
		}
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
		
		m_hsndLastSound = NULL;
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
	SAVE_VECTOR( m_vMoveToPos );
	SAVE_VECTOR( m_vMoveDistances );
	SAVE_FLOAT( m_fPowerOnTime );
	SAVE_FLOAT( m_fPowerOnMultiplier );
	SAVE_FLOAT( m_fMoveDist );
	SAVE_FLOAT( m_fInitMoveDist );
	SAVE_FLOAT( m_fOnWaitTm );
	SAVE_FLOAT( m_fOffWaitTm );
	SAVE_FLOAT( m_fPowerOffTime );
	SAVE_TIME( m_fMoveStartTm );
	SAVE_FLOAT( m_fMoveDelay );
	SAVE_TIME( m_fMoveStopTm );
	SAVE_FLOAT( m_fCurTm );
	SAVE_FLOAT( m_fActiveTime );

	// Rotation vars...

	SAVE_RIGIDTRANSFORM( m_tOriginalTrans );
	SAVE_FLOAT( m_fPitch );
	SAVE_FLOAT( m_fYaw );
	SAVE_FLOAT( m_fRoll );
	SAVE_VECTOR( m_vRotationPt );		// No need to save m_sRotationPt since we already have the position
	SAVE_VECTOR( m_vRotPtOffset );
	SAVE_VECTOR( m_vRotationAngles );
	SAVE_VECTOR( m_vStartingAngles );
	SAVE_VECTOR( m_vInitOnAngles );
	SAVE_VECTOR( m_vInitOffAngles );
	SAVE_VECTOR( m_vRotateDir );
	SAVE_VECTOR( m_vInitRotDir );
	SAVE_bool(m_bBlocked);
	SAVE_VECTOR( m_vRotateToAngles );
	SAVE_VECTOR( m_vRotateFromAngles );

	// State vars...

	SAVE_DWORD( m_dwPropFlags );
	SAVE_BYTE( m_nCurState );
	SAVE_BYTE( m_nStartingState );
	SAVE_BYTE( m_nLastState );
	SAVE_BYTE( m_nWaveform );
	SAVE_BYTE( m_nInitWaveform );
	SAVE_BOOL( m_bCheckForChars );
	SAVE_FLOAT( m_fPowerOffRadius );

	// Sound vars...	
	
	SAVE_STDSTRING( m_sPowerOnSnd );
	SAVE_STDSTRING( m_sOnSnd );
	SAVE_STDSTRING( m_sPowerOffSnd );
	SAVE_STDSTRING( m_sOffSnd );
	SAVE_STDSTRING( m_sLockedSnd );
	SAVE_VECTOR( m_vSoundPos );
	SAVE_FLOAT( m_fSoundRadius );
	SAVE_BOOL( m_bLoopSounds );
	
	// Disturbance stimulus vars...

	SAVE_DWORD( m_eStimID );
	SAVE_DWORD(	m_nActivateAlarmLevel );
	SAVE_BOOL( m_bSearchedForNode );
	SAVE_HOBJECT( m_hSmartObjectNode );

	// Command vars...

	SAVE_STDSTRING( m_sOnCmd );
	SAVE_STDSTRING( m_sOffCmd );
	SAVE_STDSTRING( m_sPowerOnCmd );
	SAVE_STDSTRING( m_sPowerOffCmd );
	SAVE_STDSTRING( m_sLockedCmd );
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
	LOAD_VECTOR( m_vMoveToPos );
	LOAD_VECTOR( m_vMoveDistances );
	LOAD_FLOAT( m_fPowerOnTime );
	LOAD_FLOAT( m_fPowerOnMultiplier );
	LOAD_FLOAT( m_fMoveDist );
	LOAD_FLOAT( m_fInitMoveDist );
	LOAD_FLOAT( m_fOnWaitTm );
	LOAD_FLOAT( m_fOffWaitTm );
	LOAD_FLOAT( m_fPowerOffTime );
	LOAD_TIME( m_fMoveStartTm );
	LOAD_FLOAT( m_fMoveDelay );
	LOAD_TIME( m_fMoveStopTm );
	LOAD_FLOAT( m_fCurTm );
	LOAD_FLOAT( m_fActiveTime );

	// Rotation vars...

	LOAD_RIGIDTRANSFORM( m_tOriginalTrans );
	LOAD_FLOAT( m_fPitch );
	LOAD_FLOAT( m_fYaw );
	LOAD_FLOAT( m_fRoll );
	LOAD_VECTOR( m_vRotationPt );
	LOAD_VECTOR( m_vRotPtOffset );
	LOAD_VECTOR( m_vRotationAngles );
	LOAD_VECTOR( m_vStartingAngles );
	LOAD_VECTOR( m_vInitOnAngles );
	LOAD_VECTOR( m_vInitOffAngles );
	LOAD_VECTOR( m_vRotateDir );
	LOAD_VECTOR( m_vInitRotDir );
	LOAD_bool(m_bBlocked);
	LOAD_VECTOR( m_vRotateToAngles );
	LOAD_VECTOR( m_vRotateFromAngles );

	// State vars...

	LOAD_DWORD( m_dwPropFlags );
	LOAD_BYTE( m_nCurState );
	LOAD_BYTE( m_nStartingState );
	LOAD_BYTE( m_nLastState );
	LOAD_BYTE( m_nWaveform );
	LOAD_BYTE( m_nInitWaveform );
	LOAD_BOOL( m_bCheckForChars );
	LOAD_FLOAT( m_fPowerOffRadius );
	
	// Sound vars...	
	
	LOAD_STDSTRING( m_sPowerOnSnd );
	LOAD_STDSTRING( m_sOnSnd );
	LOAD_STDSTRING( m_sPowerOffSnd );
	LOAD_STDSTRING( m_sOffSnd );
	LOAD_STDSTRING( m_sLockedSnd );
	LOAD_VECTOR( m_vSoundPos );
	LOAD_FLOAT( m_fSoundRadius );
	LOAD_BOOL( m_bLoopSounds );
	
	// Disturbance stimulus vars...

	LOAD_DWORD_CAST( m_eStimID, EnumAIStimulusID );
	LOAD_DWORD(	m_nActivateAlarmLevel );
	LOAD_BOOL( m_bSearchedForNode );
	LOAD_HOBJECT( m_hSmartObjectNode );

	// Command vars...

	LOAD_STDSTRING( m_sOnCmd );
	LOAD_STDSTRING( m_sOffCmd );
	LOAD_STDSTRING( m_sPowerOnCmd );
	LOAD_STDSTRING( m_sPowerOffCmd );
	LOAD_STDSTRING( m_sLockedCmd );

	// Start playing sounds if we are supposed to

	if( m_dwPropFlags & AWM_PROP_LOOPSOUNDS )
	{
		switch( m_nCurState )
		{
			/*	// these sounds are never supposed to loop (according to james)
			case AWM_STATE_OFF:
			{
				StartSound( m_sOffSnd.c_str(), true );
			}
			break;

			case AWM_STATE_ON:
			{
				StartSound( m_sOnSnd.c_str(), true );
			}
			break;
			*/
		
			case AWM_STATE_POWERON:
			{
				StartSound( m_sPowerOnSnd.c_str(), true );
			}
			break;

			case AWM_STATE_POWEROFF:
			{
				StartSound( m_sPowerOffSnd.c_str(), true );
			}
			break;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ActiveWorldModel::GetPrefetchResourceList
//
//	PURPOSE:	Determines the list of all needed resources
//
// ----------------------------------------------------------------------- //

void ActiveWorldModel::GetPrefetchResourceList(const char* pszObjectName, IObjectResourceGatherer* pInterface, ResourceList& Resources )
{
	// get the world model sounds
	AddSoundResourceToObjectGatherer(pInterface, Resources, pszObjectName, "PowerOnSound");
	AddSoundResourceToObjectGatherer(pInterface, Resources, pszObjectName, "OnSound");
	AddSoundResourceToObjectGatherer(pInterface, Resources, pszObjectName, "PowerOffSound");
	AddSoundResourceToObjectGatherer(pInterface, Resources, pszObjectName, "OffSound");
	AddSoundResourceToObjectGatherer(pInterface, Resources, pszObjectName, "LockedSound");
}
