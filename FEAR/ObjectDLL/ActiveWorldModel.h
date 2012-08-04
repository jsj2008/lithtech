// ----------------------------------------------------------------------- //
//
// MODULE  : ActiveWorldModel.h
//
// PURPOSE : The ActiveWorldModel object
//
// CREATED : 5/16/01
//
// (c) 2001-2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __ACTIVE_WORLD_MODEL_H__
#define __ACTIVE_WORLD_MODEL_H__

//
// Includes...
//

	#include "WorldModel.h"
	#include "AIEnumStimulusTypes.h"

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
	#define AWM_PROP_ROTATEAROUNDCENTER	(1<<11)	// Object will rotate around transform center, unless rotation point specified.

	// Current state defines...

	#define	AWM_STATE_ON		1
	#define	AWM_STATE_POWERON	2
	#define AWM_STATE_OFF		3
	#define	AWM_STATE_POWEROFF	4
	#define AWM_STATE_PAUSE		5
	#define AWM_STATE_INITIAL	6
// Not sure if these should be states, what happens if a ROTATE and MOVE message are received simultaneously?  
	#define AWM_STATE_ROTATE	7
	#define AWM_STATE_MOVE		8
	
	#define AWM_INITIAL_STATE	true	// Used to differentiate setting a state on startup...

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
	#define AWM_DEFAULT_POWEROFFRADIUS	0.0f
	#define AWM_DEFAULT_MOVEDELAY		0.0f
	#define AWM_DEFAULT_ONWAITTIME		0.0f
	#define AWM_DEFAULT_OFFWAITTIME		0.0f

	// Prop define helpers

	#define AWM_SET_TYPE_STATIC		ADD_LONGINTPROP_FLAG(AWMType, AWM_TYPE_STATIC, PF_HIDDEN, "TODO:PROPDESC")
	
	#define AWM_SET_TYPE_SLIDING	ADD_LONGINTPROP_FLAG(AWMType, AWM_TYPE_SLIDING, PF_HIDDEN, "TODO:PROPDESC") \
									ADD_STRINGPROP_FLAG(RotationPoint, "", PF_OBJECTLINK | PF_HIDDEN, "The name of a Point object which this WorldModel will rotate around.  If no valid object name is given the WorldModel will rotate around the position of the bound object." ) \
									ADD_VECTORPROP_FLAG(RotationAngles, PF_HIDDEN, "These represent how far the WorldModel will rotate around the RotationPoint in the specified axi when turned on.  (0.0 90.0 0.0) will rotate the WorldModel about the RotationPoint 90 degrees around the WorldModels local Y axis.") \
									ADD_VECTORPROP_VAL_FLAG(StartingAngles, 0.0f, 0.0f, 0.0f, PF_HIDDEN, "Specifes an optional initial rotation of the WorldModel.  These are in degrees relative to the rotation of the WorldModel when in the off state.  A StartingAngle of (0.0, 15.0, 0.0) will load the WorldModel with a rotation of 15 degrees around the WorldModels' local Y axis in the on direction." ) \
									ADD_BOOLPROP_FLAG(RotateAway, false, PF_GROUP(3) | PF_HIDDEN, "TODO:PROPDESC") \

	#define AWM_SET_TYPE_ROTATING	ADD_LONGINTPROP_FLAG(AWMType, AWM_TYPE_ROTATING, PF_HIDDEN, "TODO:PROPDESC") \
									ADD_BOOLPROP_FLAG( BoxPhysics, false, 0, "In the engine, WorldModels have two possible physics models. In the first, the player can walk on and touch every curve and corner of their surface. In the second, the player can only interact with a bounding box that surrounds all the brushes in the WorldModel, just like the box you would get if you selected them in WorldEdit. For geometry with a simple rectangular shape, this is preferred because it's cheaper to calculate. However, for a lot of objects, it's limiting. If you need a player to be able to shoot through the bars in a prison door, you will need BoxPhysics set to FALSE." ) \
									ADD_VECTORPROP_VAL_FLAG(MoveDir, 0.0f, 0.0f, 0.0f, PF_HIDDEN, "This is the direction of movement this object will go when turned on.  This is relative to the objects local coordinates so a MoveDir of (0.0 1.0 0.0) will always move in the objects positive Y direction.") \
									ADD_REALPROP_FLAG(MoveDist, 0.0f, PF_HIDDEN, "This is the distance the object will move in the direction specified in MoveDir.") \
	
//
// Structs...
//

class ActiveWorldModel : public WorldModel
{
	public: // Methods...

		ActiveWorldModel( );
		virtual ~ActiveWorldModel( );

		// Returns true if the world model is blocked, false if it is not.
		bool GetBlocked() const { return m_bBlocked; }

		// Returns true if this is a rotating world model, false if it is not.
		bool IsRotatingWorldModel() const { return AWM_TYPE_ROTATING == m_nAWMType; }

		static void GetPrefetchResourceList(const char* pszObjectName, IObjectResourceGatherer* pInterface, ResourceList& Resources );

	protected: // Members...

		uint8		m_nAWMType;			// What kind of action do we do?
		LTObjRef	m_hActivateObj;		// The object that activated us

		// Movement vars...

		LTVector	m_vMoveDir;			  // Direction to open
		LTVector	m_vOnPos;			  // Position we're in when on
		LTVector	m_vOffPos;			  // Position we're in when off
		LTVector	m_vMoveToPos;
		LTVector	m_vMoveDistances;		
		LTVector	m_vPausePos;			// Position when we get paused		
		float		m_fPowerOnTime;			// Speed of our movement
		float		m_fPowerOnMultiplier;	// Factor used to temporarily speed up or slow down movement.
		float		m_fInitMoveDist;		// Initial dist value to move.
		float		m_fMoveDist;			// Distance to open
		float		m_fOnWaitTm;			// Length of time to stay on
		float		m_fOffWaitTm;			// Length of time to stay off
		float		m_fPowerOffTime;		// Movement speed while powering off
		float		m_fActiveTime;			// Time it takes to rotate or move
		double		m_fMoveStartTm;			// Time movement started
		float		m_fMoveDelay;			// Time to wait to move
		double		m_fMoveStopTm;			// Time movement stopped
		float		m_fCurTm;				// How long into our current movement

		// Rotation vars...

		LTRigidTransform	m_tOriginalTrans;	// The original transform of this world model as placed in the level

		float		m_fPitch;			// AWM's Pitch ( Rotation around X )
		float		m_fYaw;				// AWM's Yaw ( Rotation around Y )
		float		m_fRoll;			// AWM's Roll ( Rotation around Z )
		std::string	m_sRotationPt;		// Name of the rotation point object
		LTVector	m_vRotationPt;		// Point we are rotating around
		LTVector	m_vRotPtOffset;		// Offset from Original Pos to Rotation Point
		LTVector	m_vRotationAngles;	// How much of a rotation we will apply
		LTVector	m_vStartingAngles;	// Rotation from the off angles the WorldModel will be in when the world loads...
		LTVector	m_vInitOnAngles;	// The original on angles so we can change the actual ones to apply
		LTVector	m_vInitOffAngles;	// The original off angles so we can change the actual ones to apply
		LTVector	m_vRotateDir;		// Direction we rotate
		LTVector	m_vInitRotDir;		// The original direction so we can change the actual one to apply
		bool		m_bBlocked;			// Is the movement blocked?
		LTVector	m_vRotateToAngles;	// Angles to rotate into when rotating with a ROTATE message
		LTVector	m_vRotateFromAngles;// Angles to rotate from when rotating with a ROTATE message
		
		// State vars...

		uint32		m_dwPropFlags;		// Property info
		uint8		m_nStartingState;	// Which state did we start in
		uint8		m_nCurState;		// Which state are we currently in
		uint8		m_nLastState;		// Which state were we in before we paused
		uint8		m_nWaveform;		// How do we move
		uint8		m_nInitWaveform;	// Initial waveform settin.

		bool		m_bCheckForChars;	// Should we check for characters in our PowerOffRadius?
		float		m_fPowerOffRadius;	// Distance characters must be away from m_vOffPos for PowerOff to occur (if NOT 0)

		// Sound vars...

		std::string	m_sPowerOnSnd;		// sound to play when PowerOn starts
		std::string	m_sOnSnd;			// sound to play when fully on
		std::string	m_sPowerOffSnd;		// sound to play when PowerOff starts
		std::string	m_sOffSnd;			// sound to play when fully off
		std::string	m_sLockedSnd;		// Sound played when AWM is activated but locked
		LTVector	m_vSoundPos;		// position to play sound (optional)
		float		m_fSoundRadius;		// Radius of sound
		bool		m_bLoopSounds;		// Are we looping our open/close sounds?
		HLTSOUND	m_hsndLastSound;	// Handle of last sound playing

		// Disturbance stimulus vars...

		EnumAIStimulusID m_eStimID;				// ID of visual stimulus. Used to remove it if reversed.
		uint32		m_nActivateAlarmLevel;		// How alarming is activating.
		bool		m_bSearchedForNode;			// Did we already look for a SmartObject node pointing at this object?
		LTObjRef	m_hSmartObjectNode;			// AINodeSmartObjectNode pointing at this object.

		// Command vars...

		std::string		m_sOnCmd;		// Command to send when AWM turns on
		std::string		m_sOffCmd;		// Command to send when AWM turns off
		std::string		m_sPowerOnCmd;	// Command to send when AWM starts PowerOn
		std::string		m_sPowerOffCmd;	// Command to send when AWM starts PowerOff
		std::string		m_sLockedCmd;	// Command to send when a locked AWM is activated

		
	protected: // Methods...

		// Engine message handlers
		
		virtual void	OnObjectCreated( );
		virtual	void	OnEveryObjectCreated( );
		virtual	void	OnUpdate( const double &fCurTime );
		virtual void	Lock( bool bLock );
		virtual void	OnSave( ILTMessage_Write *pMsg, uint32 dwSaveFlags );
		virtual void	OnLoad( ILTMessage_Read *pMsg, uint32 dwSaveFlags );

		virtual void	ReadProps( const GenericPropList *pProps );
		virtual void	PostReadProp( ObjectCreateStruct *pOcs );

		// State methods...

		virtual void	UpdateOn( const double &fCurTime );
		virtual void	UpdatePowerOn( const double &fCurTime );
		virtual void	UpdateOff( const double &fCurTime );
		virtual void	UpdatePowerOff( const double &fCurTime );
		virtual void	UpdateRotate( const double &fCurTime );
		virtual void	UpdateMove( const double &fCurTime );
		virtual void	SetPowerOn( double fTime, uint8 nWaveform );
		virtual void	SetOn( bool bInitialState );
		virtual void	SetPowerOff( double fTime, uint8 nWaveform );
		virtual void	FinishSetPowerOff();
		virtual void	SetOff( bool bInitialState );
		virtual void	SetRotate( float fRotX, float fRotY, float fRotZ, float fTime, uint8 nWaveform );
		virtual void	SetMove( float fDistX, float fDistY, float fDistZ, float fTime, uint8 nWaveform );
		virtual void	ToggleState( float fTime = -1.0f, uint8 nWaveform = (uint8)-1 );

		void	SetActiveObj( HOBJECT hObj );

		// Movement and Rotation methods...
		
		void	ChangeDir( float fTime, uint8 nWaveform );

		float	GetWaveformValue( float fSpeed, float fPercent ) const;
		bool	CalculateNewPosRot( LTVector &vOutPos, LTRotation &rOutRot, const LTVector &vFinalPos, float fTotalTime, float fPercent, bool bFailOnCollision = false ) const;

		virtual void SetStartingAngles( const LTVector& vStartingAngles );

		// Sound methods...
	
		void	StartSound( const char *pSoundName, const bool bLoop );
		void	StopSound( );

		// Disturbance stimulus methods...

		void	RegisterDisturbanceStimulus( bool bAudioStim );

		// Activation...

		virtual void	Activate( HOBJECT hObj );

	private: // Methods...
				
		// Movement and Rotation methods...

		bool	CalcAngle( float &fAngle, float fInitial, float fTarget, float fDir, float fTotalTime, float fSpeed ) const;
		bool	GetTestPosRot( LTVector &vTestPos, LTRotation &rTestRot );
		bool	TestObjectCollision( HOBJECT hTest, const LTVector &vTestPos, const LTRotation &rTestRot, HOBJECT *pCollisionObj = NULL ) const;
		inline	bool TestActivateObjectCollision( const LTVector &vTestPos, const LTRotation &rTestRot )
		{
			if( !m_hActivateObj ) return false;
			return TestObjectCollision( m_hActivateObj, vTestPos, rTestRot );
		}

		// Message handlers...

		DECLARE_MSG_HANDLER( ActiveWorldModel, HandleActivateMsg );
		DECLARE_MSG_HANDLER( ActiveWorldModel, HandleToggleMsg );
		DECLARE_MSG_HANDLER( ActiveWorldModel, HandleLockMsg );
		DECLARE_MSG_HANDLER( ActiveWorldModel, HandleUnlockMsg );
		DECLARE_MSG_HANDLER( ActiveWorldModel, HandleOnMsg );
		DECLARE_MSG_HANDLER( ActiveWorldModel, HandleOffMsg );
		DECLARE_MSG_HANDLER( ActiveWorldModel, HandlePauseMsg );
		DECLARE_MSG_HANDLER( ActiveWorldModel, HandleResumeMsg );
		DECLARE_MSG_HANDLER( ActiveWorldModel, HandleSetPowerOnMultiplayerMsg );
		DECLARE_MSG_HANDLER( ActiveWorldModel, HandleCanActivateMsg );
		DECLARE_MSG_HANDLER( ActiveWorldModel, HandleSetStartingAnglesMsg );
		DECLARE_MSG_HANDLER( ActiveWorldModel, HandleRotateMsg );
		DECLARE_MSG_HANDLER( ActiveWorldModel, HandleMoveMsg );

	private:

		PREVENT_OBJECT_COPYING( ActiveWorldModel );
};

// WorldEdit plugin class

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
};

#endif // __ACTIVE_WORLD_MODEL_H__
