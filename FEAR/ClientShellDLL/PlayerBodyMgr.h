// ----------------------------------------------------------------------- //
//
// MODULE  : PlayerBodyMgr.h
//
// PURPOSE : PlayerBodyMgr definition - Manages the full-body player view
//
// CREATED : 10/06/03
//
// (c) 2003-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __PLAYER_BODY_MGR_H__
#define __PLAYER_BODY_MGR_H__

#include "AnimationProp.h"
#include "ClientNodeTrackerContext.h"
#include "AnimationContext.h"
#include "AnimationDescriptors.h"
#include "ConditionalAnimationController.h"

class CClientWeapon;

class CPlayerBodyMgr
{
	private: // Singelton...

		CPlayerBodyMgr();
		~CPlayerBodyMgr();

		CPlayerBodyMgr(	const CPlayerBodyMgr &other );
		CPlayerBodyMgr& operator=( const CPlayerBodyMgr &other );

	public: // Singelton...

		__declspec(noinline) static CPlayerBodyMgr& Instance( )	{ static CPlayerBodyMgr sPlayerBody; return sPlayerBody; }


	public:	// Methods...

		enum PlayerBodyContext
		{
			kMainContext,
			kLowerContext,
			kUpperContext,
			kCustomContext,

			kNumContexts
		};

		// Do some initial setup...
		bool	Init();

		// Reset the player body model with the specified model template...
		bool	ResetModel( ModelsDB::HMODEL hModel );

		// Update the PlayerBody filenames...
		void	ResetModelFilenames( );
		
		// Update the player body and it's animations...
		void	Update();

		// Test if the PlayerBody will allow saves...
		bool	CanSave( );

		// Save the PlayerBody...
		void	Save( ILTMessage_Write *pMsg );

		// Load the PlayerBody...
		void	Load( ILTMessage_Read *pMsg );

		// Specifically update the player body when not in the playing state...
		void	UpdateNotPlaying( );

		//Update the grenade attachment so that is in the player's hand
		void	UpdateGrenadePosition( );

		// Write the animation info to be sent to the player on the server...
		bool	WriteAnimationInfo( ILTMessage_Write *pMsg );

		// Recieve model keys from the player body...
		bool	OnModelKey( HLOCALOBJ hObj, ANIMTRACKERID hTrackerID, ArgList* pArgList );

		// Handle leaving a world...
		void	OnExitWorld();

		// Recieve a message...
		void	OnMessage( ILTMessage_Read *pMsg );

		// Set the animation property on all of our animation contexts...
		enum	AnimPlay { kLocked, kUnlocked };
		bool	SetAnimProp( EnumAnimPropGroup eAnimPropGroup, EnumAnimProp eAnimProp, AnimPlay ePlay = kUnlocked );

		// Set the animation property on an individual animation context...
		bool	SetAnimProp( PlayerBodyContext kContext, EnumAnimPropGroup eAnimPropGroup, EnumAnimProp eAnimProp );
		
		// Test to see if any of the contexts are locked and have the specified property set...
		bool	IsLocked( EnumAnimPropGroup eAnimPropGroup, EnumAnimProp eAnimProp );

		// Clears the group from each locked properties...
		void	ClearLockedProperties( EnumAnimPropGroup eGroup = kAPG_Invalid );

		// Retrieve the animation property for the specifed group...
		EnumAnimProp GetAnimProp( EnumAnimPropGroup eAnimPropGroup ) const { return m_AnimProps.Get( eAnimPropGroup ); }

		// Retrieve the previous animation property for the specified group...
		EnumAnimProp GetLastAnimProp( EnumAnimPropGroup eAnimPropGroup ) const { return m_LastAnimProps.Get( eAnimPropGroup ); }
		
		// Retrieve the current animatio property for the specified group of the specified tracker...
		EnumAnimProp GetCurrentAnimProp( PlayerBodyContext eContext, EnumAnimPropGroup eAnimPropGroup );

		// Conditional Animation support.
		ConditionalAnimState& GetConditionalAnimState() { return m_ConditionalAnimState; }

		// Expose the player body object for others...
		HOBJECT	GetObject()	const { return m_hPlayerBody; }

		// Retrieve the limits of aim tracking...
		bool	GetAimTrackingLimits( LTRect2f &rLimits );
		
		// Specify the limits of aim tracking...
		void	SetAimTrackingLimits( const LTRect2f &rLimits );
		
		// Set the limits for the aim tracking to the default values...
		void	SetAimTrackingDefaultLimits( );

		// Specify the maximum speed used to reach the limits of the aim tracking...
		void	SetAimTrackingMaxSpeed( float fMaxSpeed );

		// Set the maximum speed for the aim tracking to the default value...
		void	SetAimTrackingDefaultMaxSpeed( );

		// Hide or show the player body model...
		void	HidePlayerBody( bool bHide, bool bControlShadow = true );

		// Check if playerbody is hidden.
		bool	IsPlayerBodyHidden( ) const;

		// See if we're in a close encounter.
		bool	InCloseEncounter() const { return m_bInCloseEncounter; }

		// Get the current context used for weapon animations...
		PlayerBodyContext GetWeaponContext( );

		// Retrieve the tracker ID for the given context...
		ANIMTRACKERID GetContextTrackerID( PlayerBodyContext eContext );

		// Retrieve a specific context interface
		CAnimationContext* GetAnimationContext( PlayerBodyContext eContext );

		// Get the current animation props
		const CAnimationProps& GetAnimationProps();

		// Retrieve the current movement type...
		EnumAnimDesc GetMovementDescriptor( ) const { return m_eMovementDescriptor; }

		// Retrieve the current camera type...
		EnumAnimDesc GetCameraDescriptor( ) const { return m_eCameraDescriptor;	}

		// Retreive the current input type...
		EnumAnimDesc GetInputDescriptor( ) const { return m_eInputDescriptor; }

		// Test the movement descriptor for movement encoding...
		bool	IsMovementEncoded( );

		bool	IsPlayingSpecial( ) const { return m_bPlayingSpecial; }

		// Clears the special animation information.
		void	ClearPlayingSpecial( );

		// Retrieve the players grenade weapon...
		// NOTE: The permanent weapons (melee and grenade) should be moved into the player manager class.
		CClientWeapon* GetGrenadeWeapon( ) const { return m_pGrenadeWeapon; }

		ModelsDB::HMODEL GetModel( ) const { return m_hModel; }

// PLAYER_BODY - Temp just to remove the var track from everywhere
		void	Enable();
		void	Disable();
		bool	IsEnabled() const { return m_bEnabled; }

		// Initialize the animations for the player body model...
		bool	InitAnimations();

		// Sends the trackers that have been added to the playerbody model.
		bool	SendTrackerAdds( );

		// Extended animation support (wrapper functions for the current weapon).
		bool	HandleAnimationStimulus( const char* pszStimulus );
		bool	HandlingAnimationStimulus( const char* pszStimulus ) const;
		bool	HandlingAnimationStimulusGroup( const char* pszStimulus ) const;
		bool	ActiveAnimationStimulus() const;

		// Certain animations may not want to use head bob, so disable it for those...
		bool	CanUseHeadBob( );

		// Certain animatins may not allow crouching...
		bool	CanCrouch( );


	private: // Methods...

		// Handle releasing the player body object and animation contexts...
		void	Term();

		// Initially create the model used for the player body...
		bool	InitPlayerBody();

		// Initialize the physics data for the player body model...
		bool	InitPhysics( );

		// Update the dimensions of the player body...
		void	UpdateDims();

		// Test if the current dims are the dims that are actually wanted...
		bool	AreDimsCorrect();

		// Set the wanted dims and factor in any offset...
		void	ResetDims( LTVector &rvOffset );

		// Update the aim tracking...
		void	UpdateAimTracking();

		// Update the locked animation properties (ie, clear them if no longer locked)...
		void	UpdateLockedAnimationProperties();

		// Count how many animations the context has with the animation properties set...
		uint32	CountAnimations( PlayerBodyContext eContext );

		// Update the property for look context sensitivity...
		void	UpdateLookContextProperties( );

		// Update the property for action...
		void	UpdateActionProperties( );

		// Actually update each animation context...
		void	UpdateAnimationContexts( );

		// The weapon model should be plyaing the same animation as the PlayerBody if it has it...
		void	UpdateWeaponModel( );

		// Update the movement type based on which contexts are enabled and which animations are playing...
		void	UpdateMovementType( );

		// Update the camera type based on which contexts are enabled and which animations are playing...
		void	UpdateCameraType( );

		// Update the input based on which contexts are enabled and which animations are playing...
		void	UpdateInputType( );

		// Determines if a full body animation should play and handles playing it if it should...
		// Returns true if full body animation is playing, false otherwise...
		bool	UpdateFullBodyAnimation( );

		// Handles playing an animation on the upper body...
		void	UpdateUpperBodyAnimation( );

		// Update the animation properties that lock the upper body...
		void	UpdateUpperLockedAnimProps( );

		// Handles playing an animation on the lower body...
		void	UpdateLowerBodyAnimation( );

        // Update the animation properties that lock the lower body...
		void	UpdateLowerLockedAnimProps( );

		// Handles playing an animation on the Custom body...
		void	UpdateCustomBodyAnimation( );

		// Update the animation properties that lock the Custom body...
		void	UpdateCustomLockedAnimProps( );

		// Update the state of the special animation, if one's playing...
		void	UpdatePlayingSpecialAnimation( );

		// Determine if animation blending is needed and set the proper blend data...
		void	UpdateAnimationBlending( );

		// Scale our animation rate to the distance we're actually traveling
		void	SetupMovementScaling();

		// Check animations to see if it's time to stop a close encounter...
		void	UpdateCloseEncounter();

				
	private: // Members...

		// Animation stuff

		CAnimationContext	*m_pMainAnimationContext;	// Our full body animation context
		CAnimationContext	*m_pLowerAnimationContext;	// Lower tracker context
		CAnimationContext	*m_pUpperAnimationContext;	// Upper tracker context
		CAnimationContext	*m_pCustomAnimationContext;	// Custom tracker context
		
		HOBJECT				m_hPlayerBody;
		
		// NOTE: The permanent weapons (melee and grenade) should be moved into the player manager class.
		CClientWeapon		*m_pMeleeWeapon;
		CClientWeapon		*m_pGrenadeWeapon;

		// Clearing the cached ani means a new ani will be chosen when the current one finishes...
		bool				m_bMainClearCachedAni;
		bool				m_bLowerClearCachedAni;
		bool				m_bUpperClearCachedAni;
		bool				m_bCustomClearCachedAni;

		// If a full body anim is playing then the upper and lower trackers are disabled...
		bool				m_bPlayingFullBody;
		bool				m_bPlayingCustom;

		// The model template ID used for the player body model...
		ModelsDB::HMODEL	m_hModel;

		// The last animation used to set the dims...
		uint32				m_dwLastDimsAnim;

		// The last animation used set the weaopn model animation...
		uint32				m_dwLastWeaponContextAnim;

		// The dims the player body wishes to have...
		LTVector			m_vWantedDims;

		// The torso tracking context...
		CClientNodeTrackerContext	m_NodeTrackerContext;
		
		// Current animation propertiess for the player body...
		CAnimationProps		m_AnimProps;

		// Animation property overrides for each context...
		CAnimationProps		m_UpperAnimProps;
		CAnimationProps		m_LowerAnimProps;
		CAnimationProps		m_CustomAnimProps;

		// Previous animation properties for the player body...
		CAnimationProps		m_LastAnimProps;

		// Locked animation properties that will lock any context that plays an animation with one of them...
		CAnimationProps		m_MainLockedAnimProps;
		CAnimationProps		m_UpperLockedAnimProps;
		CAnimationProps		m_LowerLockedAnimProps;
		CAnimationProps		m_CustomLockedAnimProps;

		// A throttle for sending the animation time, we don't need constantly send it...
		float				m_fSendAnimTime;

		// Forcing sending of the animation time allows the server to keep in synch with the client...
		bool				m_bForceSendAnimTime;

		// The type of movement controlled by the animation....
		EnumAnimDesc		m_eMovementDescriptor;

		// The descriptor of how camera is controlled by the animation...
		EnumAnimDesc		m_eCameraDescriptor;

		// The descriptor of how input is controlled by the animation...
		EnumAnimDesc		m_eInputDescriptor;

		// Playing a special animation on the contexts, don't do normal updates untill cleared...
		bool				m_bPlayingSpecial;

		// The special animation that is playing is supposed to linger (ie. hold on last frame of anim)...
		bool				m_bLingerSpecial;

				
// PLAYER_BODY - Temp just to remove the var track from everywhere
		bool				m_bEnabled;
		bool				m_bStartPlayerBody;

		// animation rate used to match movement anims to actual movement speed
		float				m_fLowerAnimRate;

		// close encounter support
		bool				m_bInCloseEncounter;
		ANIMTRACKERID		m_nCloseEncounterTrackerID;
		HMODELANIM			m_hCloseEncounterAnim;
		double				m_fCloseEncounterEndTime;
		float				m_fCloseEncounterOldNearZ;

		// conditional animation support
		ConditionalAnimState m_ConditionalAnimState;
};

#endif // __PLAYER_BODY_MGR_H__
