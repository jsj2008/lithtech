// ----------------------------------------------------------------------- //
//
// MODULE  : ActiveWorldModel.h
//
// PURPOSE : The ActiveWorldModel object
//
// CREATED : 5/16/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __ACTIVE_WORLD_MODEL_H__
#define __ACTIVE_WORLD_MODEL_H__

// 
// Forward Declarations...
//

enum EnumAIStimulusID;

//
// Includes...
//

	#include "WorldModel.h"
	

LINKTO_MODULE( ActiveWorldModel );

//
// Defines...
//
	// Property defines...	

	#define AWM_PROP_PLAYERACTIVATE		(1<<0)	// Can be activated to trigger
	#define	AWM_PROP_AIACTIVATE			(1<<1)	// AI can trigger it
	#define AWM_PROP_STARTON			(1<<2)	// Start in the on state
	#define	AWM_PROP_TRIGGEROFF			(1<<3)	// Must be triggered off
	#define AWM_PROP_REMAINON			(1<<4)	// Remains on
	#define	AWM_PROP_FORCEMOVE			(1<<5)	// Force a move even if something is blocking us
	#define	AWM_PROP_FORCEMOVEON		(1<<6)	// Force a move when on even if something is blocking us
	#define	AWM_PROP_FORCEMOVEOFF		(1<<7)	// Force a move when off even if something is blocking us
	#define AWM_PROP_LOCKED				(1<<8)	// AWM's won't change state while locked... (this can be set/unset during gameplay)
	#define	AWM_PROP_LOOPSOUNDS			(1<<9)	// Do the sounds loop or play once
	#define AWM_PROP_ROTATEAWAY			(1<<10)	// Does the rotating WorldModel/Door rotate away from the activate obj? 

	// Current state defines...

	#define	AWM_STATE_ON		1
	#define	AWM_STATE_POWERON	2
	#define AWM_STATE_OFF		3
	#define	AWM_STATE_POWEROFF	4
	#define AWM_STATE_PAUSE		5

	// Waveform type defines...

	#define	AWM_WAVE_LINEAR		0
	#define	AWM_WAVE_SINE		1
	#define	AWM_WAVE_SLOWOFF	2
	#define	AWM_WAVE_SLOWON		3
	#define AWM_WAVE_MAXTYPES	3	// Keep this the same as actual last type ( if you add another typem change this value )

	// Type defines...

	#define AWM_TYPE_STATIC		0
	#define AWM_TYPE_SLIDING	1
	#define AWM_TYPE_ROTATING	2

	// Property defaults...

	#define AWM_DEFAULT_POWERONTIME		1.0f
	#define AWM_DEFAULT_POWEROFFTIME	0.0f
	#define AWM_DEFAULT_MOVEDELAY		0.0f
	#define AWM_DEFAULT_ONWAITTIME		0.0f
	#define AWM_DEFAULT_OFFWAITTIME		0.0f
	
	// Prop define helpers

	#define AWM_SET_TYPE_STATIC		ADD_LONGINTPROP_FLAG(AWMType, AWM_TYPE_STATIC, PF_HIDDEN)
	
	#define AWM_SET_TYPE_SLIDING	ADD_LONGINTPROP_FLAG(AWMType, AWM_TYPE_SLIDING, PF_HIDDEN) \
									ADD_STRINGPROP_FLAG(RotationPoint, "", PF_OBJECTLINK | PF_HIDDEN) \
									ADD_VECTORPROP_FLAG(RotationAngles, PF_HIDDEN) \
									ADD_BOOLPROP_FLAG(RotateAway, LTFALSE, PF_GROUP(3) | PF_HIDDEN) \

	#define AWM_SET_TYPE_ROTATING	ADD_LONGINTPROP_FLAG(AWMType, AWM_TYPE_ROTATING, PF_HIDDEN) \
									ADD_BOOLPROP_FLAG( BoxPhysics, LTFALSE, 0 ) \
									ADD_VECTORPROP_VAL_FLAG(MoveDir, 0.0f, 0.0f, 0.0f, PF_HIDDEN) \
									ADD_REALPROP_FLAG(MoveDist, 0.0f, PF_HIDDEN) \
	
//
// Structs...
//

class ActiveWorldModel : public WorldModel
{
	public: // Methods...

		ActiveWorldModel( );
		virtual ~ActiveWorldModel( );


	protected: // Members...

		uint8		m_nAWMType;			// What kind of action do we do?
		LTObjRef	m_hActivateObj;		// The object that activated us

		// Movement vars...

		LTVector	m_vMoveDir;			  // Direction to open
		LTVector	m_vOnPos;			  // Position we're in when on
		LTVector	m_vOffPos;			  // Position we're in when off
		LTVector	m_vPausePos;		  // Position when we get paused		
		LTFLOAT		m_fPowerOnTime;		  // Speed of our movement
		LTFLOAT		m_fPowerOnMultiplier; // Factor used to temporarily speed up or slow down movement.
		LTFLOAT		m_fMoveDist;		  // Distance to open
		LTFLOAT		m_fOnWaitTm;		  // Length of time to stay on
		LTFLOAT		m_fOffWaitTm;		  // Length ot time to stay off
		LTFLOAT		m_fPowerOffTime;	  // Movement speed while powering off
		LTFLOAT		m_fMoveStartTm;		  // Time movement started
		LTFLOAT		m_fMoveDelay;		  // Time to wait to move
		LTFLOAT		m_fMoveStopTm;		  // Time movement stopped
		LTFLOAT		m_fCurTm;			  // How long into our current movement

		// Rotation vars...

		LTVector	m_vOriginalPos;		// The original position perhaps
		LTFLOAT		m_fPitch;			// AWM's Pitch ( Rotation around X )
		LTFLOAT		m_fYaw;				// AWM's Yaw ( Rotation around Y )
		LTFLOAT		m_fRoll;			// AWM's Roll ( Rotation around Z )
		HSTRING		m_hstrRotationPt;	// Name of the rotation point object
		LTVector	m_vRotationPt;		// Point we are rotating around
		LTVector	m_vRotPtOffset;		// Offset from Original Pos to Rotation Point
		LTVector	m_vRotationAngles;	// How much of a rotaion we will apply
		LTVector	m_vOnAngles;		// Angles when the AWM is "on"
		LTVector	m_vInitOnAngles;	// The original angles so we can change the actual ones to apply
		LTVector	m_vOffAngles;		// Angles when the AWM is "off"
		LTVector	m_vRotateDir;		// Direction we rotate
		LTVector	m_vInitRotDir;		// The original direction so we can change the actual one to apply
		
		// State vars...

		uint32		m_dwPropFlags;		// Property info
		uint8		m_nStartingState;	// Which state did we start in
		uint8		m_nCurState;		// Which state are we currently in
		uint8		m_nLastState;		// Which state were we in before we paused
		uint8		m_nWaveform;		// How do we move

		// Sound vars...

		HSTRING		m_hstrPowerOnSnd;	// sound to play when PowerOn starts
		HSTRING		m_hstrOnSnd;		// sound to play when fully on
		HSTRING		m_hstrPowerOffSnd;	// sound to play when PowerOff starts
		HSTRING		m_hstrOffSnd;		// sound to play when fully off
		HSTRING		m_hstrLockedSnd;	// Sound played when AWM is activated but locked
		LTVector	m_vSoundPos;		// position to play sound (optional)
        LTFLOAT		m_fSoundRadius;		// Radius of sound
        LTBOOL		m_bLoopSounds;		// Are we looping our open/close sounds?
		HLTSOUND    m_hsndLastSound;	// Handle of last sound playing

		// Disturbance stimulus vars...

		EnumAIStimulusID m_eStimID;				// ID of visual stimulus. Used to remove it if reversed.
		uint32		m_nActivateAlarmLevel;		// How alarming is activating.
		LTBOOL		m_bSearchedForNode;			// Did we already look for a UseObject node pointing at this object?
		LTObjRef	m_hUseObjectNode;			// AINodeUseObjectNode pointing at this object.

		// Command vars...

		HSTRING		m_hstrOnCmd;		// Command to send when AWM turns on
		HSTRING		m_hstrOffCmd;		// Command to send when AWM turns off
		HSTRING		m_hstrPowerOnCmd;	// Command to send when AWM starts PowerOn
		HSTRING		m_hstrPowerOffCmd;	// Command to send when AWM starts PowerOff
		HSTRING		m_hstrLockedCmd;	// Command to send when a locked AWM is activated

		
	protected: // Methods...

		// Engine message handlers
		
		virtual void	OnObjectCreated( );
		virtual	void	OnEveryObjectCreated( );
		virtual	void	OnUpdate( const LTFLOAT &fCurTime );
		virtual	bool	OnTrigger( HOBJECT hSender, const CParsedMsg &cMsg );
		virtual void	HandleTriggerMsg( );
		virtual void	HandlePause( );
		virtual void	HandleResume( );
		virtual void	HandleLock(LTBOOL bLock);
		virtual void	OnSave( ILTMessage_Write *pMsg, uint32 dwSaveFlags );
		virtual void	OnLoad( ILTMessage_Read *pMsg, uint32 dwSaveFlags );

		virtual void	ReadProps( ObjectCreateStruct *pOCS );
		virtual void	PostReadProp( ObjectCreateStruct *pOcs );

		// State methods...

		virtual void	UpdateOn( const LTFLOAT &fCurTime );
		virtual void	UpdatePowerOn( const LTFLOAT &fCurTime );
		virtual void	UpdateOff( const LTFLOAT &fCurTime );
		virtual void	UpdatePowerOff( const LTFLOAT &fCurTime );
		virtual void	SetPowerOn();
		virtual void	SetOn( LTBOOL bInitialize = LTFALSE );
		virtual void	SetPowerOff();
		virtual void	SetOff( LTBOOL bInitialize = LTFALSE );

		void	SetActiveObj( HOBJECT hObj );

		// Movement and Rotation methods...
		
		void	ChangeDir( );

		LTFLOAT	GetWaveformValue( LTFLOAT fSpeed, LTFLOAT fPercent );
		LTBOOL	CalculateNewPosRot( LTVector &vOutPos, LTRotation &rOutRot, LTVector &vFinalPos, LTFLOAT fTotalTime, LTFLOAT fPercent, LTBOOL bTestCollision = LTFALSE );

		// Sound methods...
	
		void    StartSound( const HSTRING hstrSoundName, const LTBOOL bLoop );
		void	StopSound( );

		// Disturbance stimulus methods...

		void	RegisterDisturbanceStimulus( LTBOOL bAudioStim );

		// Activation...

		virtual void	Activate( HOBJECT hObj );

	private: // Methods...
				
		// Movement and Rotation methods...

		LTBOOL	CalcAngle( LTFLOAT &fAngle, LTFLOAT fInitial, LTFLOAT fTarget, LTFLOAT fDir, LTFLOAT fTotalTime, LTFLOAT fSpeed );
		LTBOOL	GetTestPosRot( LTVector &vTestPos, LTRotation &rTestRot );
		LTBOOL	TestObjectCollision( HOBJECT hTest, const LTVector &vTestPos, const LTRotation &rTestRot, HOBJECT *pCollisionObj = LTNULL );
		inline	LTBOOL TestActivateObjectCollision( const LTVector &vTestPos, const LTRotation &rTestRot )
		{
			if( !m_hActivateObj ) return LTFALSE;
			return TestObjectCollision( m_hActivateObj, vTestPos, rTestRot );
		}
	
};

// DEdit plugin class

class CActiveWorldModelPlugin : public CWorldModelPlugin
{
	public: // Methods...

		virtual LTRESULT PreHook_EditStringList( const char *szRezPath,
												 const char *szPropName,
												 char **aszStrings,
												 uint32 *pcStrings,
												 const uint32 cMaxStrings,
												 const uint32 cMaxStringLength );
		
		virtual LTRESULT PreHook_PropChanged( const	char		*szObjName,
											  const	char		*szPropName,
											  const	int			nPropType,
											  const	GenericProp	&gpPropValue,
													ILTPreInterface	*pInterface,
											  const char		*szModifiers );

	protected: // Members...

		CCommandMgrPlugin		m_CommandMgrPlugin;
		CActivateTypeMgrPlugin	m_ActivateTypeMgrPlugin;
};

#endif // __ACTIVE_WORLD_MODEL_H__