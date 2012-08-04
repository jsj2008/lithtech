// ----------------------------------------------------------------------- //
//
// MODULE	: ClientWeapon.h
//
// PURPOSE : Generic client-side weapon 
//
// CREATED : 9/27/97 (was WeaponModel.h)
//
// (c) 1997-2004 Monolith Productions, Inc.	All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __CLIENT_WEAPON_H__
#define __CLIENT_WEAPON_H__

//
// Includes...
//

#include "ClientFXMgr.h"
#include "MsgIDs.h"
#include "WeaponDisplay.h"
#include "WeaponPath.h"
#include "ConditionalAnimationController.h"
#include "GameRenderLayers.h"

//
// Defines...
//
#define WM_MAX_FIRE_ANIS			3
#define WM_MAX_IDLE_ANIS			3


//
// Typedefs...
//

typedef void (*ClientWeaponCallBackFn)( HWEAPON hWeapon, void *pData );


//
// Forwards...
//

struct CLIENTFX_LINK;
enum SurfaceType;

class CWeaponModelData
{
	public: // Methods...

		CWeaponModelData( )
		:	m_hObject					( NULL ),
			m_hSilencerModel			( NULL ),
			m_hScopeModel				( NULL ),
			m_hHandSocket				( INVALID_MODEL_SOCKET ),
			m_hSilencerSocket			( INVALID_MODEL_SOCKET ),
			m_hScopeSocket				( INVALID_MODEL_SOCKET ),
			m_hBreachSocket				( INVALID_MODEL_SOCKET ),
			m_hMuzzleSocket				( INVALID_MODEL_SOCKET ),
			m_vWeaponOffset				( 0.0f, 0.0f, 0.0f ),
			m_vFlashPos					( 0.0f, 0.0f, 0.0f ),
			m_nAmmoInClip				( 0 ),
			m_nNumFiresInARow			( 0 ),
			m_hWeaponRecord				( NULL ),
			m_hWeaponModelStruct		( NULL ),
			m_pDisplay					( NULL ),
			m_aFlashlights				(),
			m_pPersistentClientFX		( NULL ),
			m_nNumPersistentClientFX	( 0 ),
			m_pbLoadPersistentFXVisible	( NULL )
		{ }

		~CWeaponModelData( );
		
		// Initialize the weapon with the appropriate recordes and initialize data...
		void Init( HWEAPON hWeapon, HATTRIBUTE hWeaponModelStruct );

		// Remove weapon models and other objects...
		void Term( );

		// Reset some members...
		void ResetData( );

		void Save( ILTMessage_Write *pMsg );
		void Load( ILTMessage_Read *pMsg );

		// Used to fill in information realating to the weapon model...
		void PopulateCreateStruct( ObjectCreateStruct &rOCS ) const;

		// Create a model object...
		HOBJECT CreateModelObject( HOBJECT hOldObj, ObjectCreateStruct &rOCS );

		// Set the visibility of the weapon model and any attached mods...
		void SetVisibility( bool bVisible, bool bShadow );

		// Create the model used as the weapon...
		bool CreateWeaponModel( const char *pszSocket );
		void RemoveWeaponModel( );

		// Move the weapon model to the appropriate socket...
		void UpdateWeaponPosition( );

		// Silencer model
		bool CreateSilencer( HMOD hSilencer );
		void UpdateSilencer( );
		void SetSilencerVisibility( bool bVisible, bool bShadow );
		void RemoveSilencer( );

		// Scope model
		bool CreateScope( HMOD hScope );
		void UpdateScope( );
		void SetScopeVisibility( bool bVisible, bool bShadow );
		void RemoveScope( );

		// Muzzle flash routines
		void CreateMuzzleFlash( );
		void RemoveMuzzleFlash( );
		void StartMuzzleFlash( );
		void UpdateMuzzleFlash( bool bVisible );
		void UpdateMuzzleFlashPos( );

		// Persistent ClientFX...
		void CreatePersistentClientFX( );
		void ShutdownPersistentClientFX( );
		void ShowPersistentClientFX( uint32 nFX );
		void HidePersistentClientFX( uint32 nFX );
		void SetPersistentClientFXVisibility( bool bVisible );

		// Update the transform used to 
		void GetBreachTransform( LTRigidTransform &rBreachTransform );

		LTVector	GetWeaponOffset() const	{ return m_vWeaponOffset; }
		void		SetWeaponOffset( LTVector const &v ) { 	m_vWeaponOffset = v; }

		int32 GetAmmoInClip( ) const { return m_nAmmoInClip; }

		// Determine if the weapon can fire, based on ammo in clip and allowed fire frequency...
		bool CanFire( );

		
	public: // Members...

		// Actual weapon model object...
		LTObjRef		m_hObject;

		// Silencer mod model object...
		LTObjRef		m_hSilencerModel;
		
		// Scope mod model object..
		LTObjRef		m_hScopeModel;
		
		// Socket on the PlayerBody the weapon model is attached to...
		HMODELSOCKET	m_hHandSocket;

		// Socket on the weapon model the silencer is attached to...
		HMODELSOCKET	m_hSilencerSocket;
		
		// Socket on the weapon model the scop is attached to...
		HMODELSOCKET	m_hScopeSocket;

		// Socket on the weapon model to eject shell casings...
		HMODELSOCKET	m_hBreachSocket;

		// Socket on the weapon model to display the muzzle flash...
		HMODELSOCKET	m_hMuzzleSocket;

		// Weapon offsets for setting the position...
		LTVector		m_vWeaponOffset;

		// An instance of a FXEdit created FX used for the weapon muzzle flash... 
		CClientFXLink	m_MuzzleFlashFX;

		// Position of flash on the 
		LTVector		m_vFlashPos;
		
		// When did flash start
		StopWatchTimer  m_FlashTime;

		// Ammount of ammo in this weapon models clip...
		int32			m_nAmmoInClip;

		// Keep track of how many times in a row the weapon fires...
		int32			m_nNumFiresInARow;

		// custom display object
		WeaponDisplayInterf* m_pDisplay;

		// Weapon record for weapon this model belongs to...
		HWEAPON			m_hWeaponRecord;

		// Struct attribute of this weapon model from teh wepon record...
		HATTRIBUTE		m_hWeaponModelStruct;

		// Attached flashlights...
		struct SFlashlightData
		{
			CParsedMsg::CToken tok;
			Flashlight* light;
			char cachedName[64];
		};
		typedef std::vector<SFlashlightData, LTAllocator<SFlashlightData, LT_MEM_TYPE_GAMECODE> > TFlashlightData;
		TFlashlightData m_aFlashlights;

		// Attached FxED created ClientFX...
		class CPersistentClientFX
		{
			public: // Methods...

				CPersistentClientFX( )
				:	m_ClientFXLink( )
				,	m_bVisible( false )
				{ }

				~CPersistentClientFX( )
				{
					// Ensure the ClientFX is Shutdown properly...
					if( m_ClientFXLink.IsValid( ))
					{
						g_pGameClientShell->GetSimulationTimeClientFXMgr().ShutdownClientFX( &m_ClientFXLink );
					}
				}

				// Link to the ClientFX instance...
				CClientFXLink	m_ClientFXLink;

				// Visibility of the ClientFX...
				bool			m_bVisible;
		};

		CPersistentClientFX	*m_pPersistentClientFX;
		uint32 m_nNumPersistentClientFX;

		bool *m_pbLoadPersistentFXVisible;
};


//
// class CClientWeapon
//
class CClientWeapon
{
public:

	CClientWeapon();
	virtual	~CClientWeapon();

    // handle messages
	virtual bool		OnMessage( uint8 messageID, ILTMessage_Read *pMsg ) 
	{ 
		LTUNREFERENCED_PARAMETER(messageID);
		LTUNREFERENCED_PARAMETER(pMsg);
		return false; 
	}

	// callbacks from the animation system
	virtual bool		OnModelKey( HLOCALOBJ hObj, ANIMTRACKERID hTrackerID, ArgList* pArgs );

	// checks state of weapon to determine if model keys should be ignored.
	virtual bool		SkipModelKeys() const;

	// enter/exit world functionality
	virtual void		OnEnterWorld();
	virtual void		OnExitWorld();

	// standard init/term functions
	virtual bool		Init( HWEAPON hWeapon );
	virtual void		Term();

	// main update function
	virtual WeaponState	Update( );

	// Just update the position of the weapon model...
	void UpdateWeaponPosition( LTVector const &vOffset );	// offset is the additional offset

	// extended animation support
	bool	HandleAnimationStimulus( const char* pszStimulus );
	bool	HandlingAnimationStimulus( const char* pszStimulus ) const;
	bool	HandlingAnimationStimulusGroup( const char* pszStimulus ) const;
	bool	ActiveAnimationStimulus() const;
	void	UpdateAnimControllers();
	void	ResetAnimControllers();

	// Manually set the weapons position and rotation...
	// NOTE: This needs to be done everyframe or UpdateWeaponPosition() will override the value...
	void SetWeaponTransform( const LTTransform &rWeaponTrans );

	// external interfaces to change the ammo
	virtual void		ChangeAmmoWithReload( HAMMO hNewAmmo, bool bForce= false );
	virtual void		ChangeAmmoImmediate( HAMMO hNewAmmo, int nAmmoAmount = -1, bool bForce= false );
	virtual void		ReloadClip( bool bPlayReload = true, int nNewAmmo=-1, bool bForce = false, bool bNotifyServer = false );
	virtual void		DecrementAmmo();

	// external blocking control
	virtual bool		Block( HOBJECT hFrom );

	// external ammo check control
	virtual bool		CheckAmmo();

	// stun gun support
	virtual bool		Stun();
	virtual void		OffhandFire(){ m_LeftHandWeapon.StartMuzzleFlash(); Fire(); }

	// set the information about the camera that the weapon needs to know
	virtual void		SetCameraInfo( LTRotation const &rCamRot, LTVector const &vCamPos );

	// Update the weapon model filenames...
	virtual void		ResetWeaponFilenames( );

	// Set the LOD distance bias on the weapon models...
	virtual void		SetWeaponLODDistanceBias( float fLODDistBias );
	virtual void		SetWeaponDepthBiasTableIndex( ERenderLayer eRenderLayer );

	// to update the weapon bob
	virtual void		UpdateBob( float fWidth, float fHeight );

	// Do NOT call these functions directly.	If you want to
	// enable/disable or show/hide the weapons, call the functions
	// in CClientWeaponMgr.
	virtual void		SetVisible( bool bVis = true, bool bShadow = true );
	virtual void		SetDisable( bool bDisable = true );
	
	// Do NOT call these directly.	They are used CClientWeaponMgr.
	virtual void		Select(bool bImmediate);
	virtual bool		Deselect( ClientWeaponCallBackFn cbFn, void *pData );
	virtual bool		Activate();	 // Create the weapon's resources
	virtual void		Deactivate(); // Shut down the weapon (destroy resources)

	virtual void		SwitchToComplimentaryWeapon();

	virtual HWEAPON		GetWeaponRecord()	const { return m_hWeapon; }
	virtual HAMMO		GetAmmoRecord()		const { return m_hAmmo; }

	virtual int32		GetAmmoInClips()		const { return (m_RightHandWeapon.GetAmmoInClip( ) + m_LeftHandWeapon.GetAmmoInClip( )); }
	virtual bool		HasAmmo()			const;
	virtual bool		CanUseAmmo( HAMMO hAmmo ) const;
	virtual bool		CanChangeToAmmo( HAMMO hAmmo ) const;
	virtual HAMMO		GetNextAvailableAmmo( HAMMO hAmmo = NULL );
	virtual HAMMO		GetBestAvailableAmmo( ) const;

	virtual uint8		GetActivationType( HOBJECT hTarget, float fTargetRange ) const;
	uint8				GetActivationType() const { return m_nActivationType; }

	// state info
	virtual WeaponState	GetState()		const { return m_eState; }

	// misc functions
	virtual bool		IsMeleeWeapon() const { return (m_hAmmo && (DT_MELEE == g_pWeaponDB->GetAmmoInstDamageType(m_hAmmo)	)); }
	
	virtual bool		IsSemiAuto() const { return m_bSemiAuto;  }

	virtual bool		IsSwapping() const;

	virtual bool		CanPerformFinishingMove(HOBJECT hTarget) const;

	virtual LTVector	GetWeaponOffset() const	{ return m_RightHandWeapon.m_vWeaponOffset; }
	virtual void		SetWeaponOffset( LTVector const &v ) { 	m_RightHandWeapon.m_vWeaponOffset = v; }

	HLOCALOBJ			GetRightHandModel( ) const { return m_RightHandWeapon.m_hObject; }
	HLOCALOBJ			GetLeftHandModel( ) const { return m_LeftHandWeapon.m_hObject; }

	LTVector2			GetMinProximity() const { return m_vMinProximity; }

	// This needs to be exposed for the PlayerStats.	Eventually
	// this should be moved so creation/deletion of the mods
	// are transparent to the outside system (not all games
	// need mods).
	virtual void		CreateMods();

	// load/save functionality
	virtual void		Load( ILTMessage_Read *pMsg );
	virtual void		Save( ILTMessage_Write *pMsg );

	virtual void		ResetWeapon();

	virtual void		SetPaused( bool bPaused );
	
	virtual void		ClearFiring( bool bReset=true );

	// Retrieve the cached animation property for the weapon...
	virtual EnumAnimProp		GetAnimationProperty( ) const { return m_eWeaponAnimProp; }

	// Drop a live grenade right at the players position. (Used on player death)
	virtual void DropGrenade( );

// PLAYER_BODY

	// send all relevant fire information to the server
	void	SendFireMessage( float fPerturb, LTVector const &vFirePos, LTVector const &vDir );
	
	void	SetFireNode( const char *szNode ) { m_sFireNode = szNode; }

	// Set the weapon models playing the specified animation... (if it exists)
	void SetWeaponModelAnimation( const char *pszAnimationName );

	// The PlayerBody needs to relay some model keys to the weapon...
	bool HandleModelKeyFromPlayerBody( HLOCALOBJ hObj, ANIMTRACKERID hTrackerID, ArgList* pArgs );
// !PLAYER_BODY

	// queue a fire animation
	void			QueueFireAnimation();

	LTVector&		GetFlashPos( ) { return (m_bFireLeftHand ? m_LeftHandWeapon.m_vFlashPos : m_RightHandWeapon.m_vFlashPos); }
	
	void GetBreachTransform( LTRigidTransform &rBreachTransform )
	{ 
		(m_bFireLeftHand ? m_LeftHandWeapon.GetBreachTransform( rBreachTransform ) :
						   m_RightHandWeapon.GetBreachTransform( rBreachTransform )); 
	}

	// breakable weapon feedback
	bool	IsWeaponAboutToBreak() { return m_bWeaponDamageThresholdExceeded; }
	void	SetWeaponAboutToBreak( bool bVal ) { m_bWeaponDamageThresholdExceeded = bVal; }

	// Control the visibility of a custom weapon (good for non-selected weapons, grenade weapons, etc.)
	void	ShowCustomWeapon();
	void	HideCustomWeapon();

	// weapon display control
	void	UpdateWeaponDisplay(bool bFiredWeapon=false);

protected:

	//
	//
	// Protected interfaces
	//
	//

	// Create the weapon model and 
	virtual bool	CreateWeaponModels();

	WeaponState			UpdateAmmoFromFire( bool bDecrementAmmo = true );
	virtual WeaponState	UpdateModelState( );

	// Update the weapon based on current state and
	// animation and determines what animation to play next.
	virtual void	UpdateFiring( );
	virtual void	UpdateAltFiring( );
	virtual void	UpdateGrenadeThrow( );
	virtual void	UpdateNonFiring( ); 
	
	// initialize all the animations for this model
	virtual void	InitAnimations( bool bAllowSelectOverride = false );

	// play model animations
	void			PlayAnimation( uint32 dwAni, float fRate = 1.0f, bool bLooping = false);
	virtual bool	PlaySelectAnimation();
	virtual bool	PlayDeselectAnimation();
	virtual bool	PlayReloadAnimation(bool bInterrupt);
	virtual bool	PlayIdleAnimation();
	virtual bool	PlayFireAnimation( bool bResetAni, bool bPlayOnce = false );
	virtual bool	PlayAltFireAnimation( bool bResetAni, bool bPlayOnce = false );
	virtual bool	PlayGrenadeAnimation( bool bResetAni, bool bPlayOnce = false );
	virtual bool	PlayBlockAnimation();
	virtual bool	PlayCheckAmmoAnimation();

	//check to see if our current animation is done playing
	virtual bool	IsAnimationDone() const;


	// selet animation
	bool		IsSelectAni( uint32 dwAni ) const;
	uint32		GetSelectAni() const;

	// deselect animation
	bool		IsDeselectAni( uint32 dwAni ) const;
	uint32		GetDeselectAni() const;

	// reload animation
	bool		IsReloadAni( uint32 dwAni ) const;
	uint32		GetReloadAni() const;

	uint32		GetPlayerAni() const;

	// idle animation
	bool		IsIdleAni( uint32 dwAni ) const;
	uint32		GetIdleAni() const;
	uint32		GetSubtleIdleAni() const;
	float		GetNextIdlePeriod() const;

	// pre fire animation
	virtual bool	IsPreFireAni( uint32 dwAni ) const;
	virtual uint32	GetPreFireAni() const;

	// fire animation
	virtual bool	IsFireAni( uint32 dwAni , bool bCheckNormalOnly = false) const;
	virtual uint32	GetFireAni( ) const;

	// post fire
	virtual bool	IsPostFireAni( uint32 dwAni ) const;
	virtual uint32	GetPostFireAni() const;

	// grenade animations
	void			PlayThrowAni( EnumAnimProp eProp, bool bLocked ) const;

	// mod helpers
	void			UpdateMods();
	void			RemoveMods();
	void			SetModVisibility( bool bVisible, bool bShadow );

	// silencer mod
	void			CreateSilencer();
	void			UpdateSilencer();

	// scope mod
	void			CreateScope();
	void			UpdateScope();

	// Correctly distributes the new ammount of ammo between the left and right clips for dual and sinlge weapons...
	void			SplitAmmoBetweenClips( );

// PLAYER_BODY...
public:
	// weapon fire helpers
	virtual void	Fire( );

	virtual void	SendFinishMessage( HOBJECT hTarget, float fImpulse, LTVector vDir, LTVector vImpactPos );
	virtual void	SendRagdollFinishMessage( HOBJECT hTarget, float fDuration, float fBreakForce, LTVector vImpactPos );

protected:
// !PLAYER_BODY...

	// send all relevant fire information to the server for TRIGGER weapons
	virtual void	SendTriggerFireMessage( );

	// Send all relevant drop information to the server...
	virtual void	SendDropGrenadeMessage( float fPerturb,
											LTVector const &vFirePos, 
											LTVector const &vDir );


	// get the last type of fire sound played
	uint8			GetLastSndFireType() const;

	// Get information to use to fire the weapon, such
	// as weapon position and the forward vector.
	bool GetFireVectors( LTVector &vRight, LTVector &vUp, LTVector &vForward, LTVector &vFirePos ) const;

	// set the model's state, do any special setup necessary
	// to enter particular states
	void		SetState( WeaponState eNewState );
	
	// special cases
	virtual void	DoSpecialEndFire();
	void			DoSpecialCreateModel();
	void			SpecialShowPieces( bool bShow = true, bool bForce = false );

	// handle client side impact effects
	void			AddImpact( HLOCALOBJ hObj, const LTVector &vFrom, const LTVector &vImpactPoint,
							 const LTVector &vNormal, const LTVector &vPath, SurfaceType eType,
							 HMODELNODE hHitNode );

	static bool WeaponPath_OnImpactCB( CWeaponPath::COnImpactCBData &rImpactData, void *pUserData );

// PLAYER_BODY prototype
public:
	void			KillLoopSound();
	
	bool			HasLoopSound() const { return m_hLoopSound != NULL;	}
	void			StopLoopSound();

protected:

	// reset data before creating the weapon's resources
	void			ResetData();

	// Actually handle the model keys recieved...
	bool			HandleModelKey( HLOCALOBJ hObj, ANIMTRACKERID hTrackerID, ArgList* pArgs );

	// keyframe handlers
	virtual bool	HandleFireKey( HLOCALOBJ hObj, ArgList* pArgs );
	virtual bool	HandleFXKey( HLOCALOBJ hObj, ArgList* pArgs );

	// remove all keyframed FXEdit effects that have expired
	bool			RemoveFinishedKeyframedFX();

#ifndef _FINAL
	// helpers for writing simulation log file entries
	void			WriteFireMessageToSimulationLog(const LTVector& vFirePos, const LTVector& vDir, const uint8 nRandomSeed, const uint8 nPerturbCount, const float fPerturb, const int32 nFireTime);
#endif

	// helper to show / hide model pieces
	void			DisplayWeaponModelPieces( HWEAPON hWeapon, const char* pszAttribute, bool bShow );

	//
	//
	// Protected Data
	//
	//

	ClientWeaponCallBackFn	m_cbDeselect;
	void			*m_pcbData;

	CWeaponModelData	m_RightHandWeapon;
	CWeaponModelData	m_LeftHandWeapon;

	// off-hand weapons (ie: forensics)
	CClientWeapon*		m_pVisibleCustomWeapon;
	std::string			m_sCustomStimulusGroup;

	// mod posessions
	bool			m_bHaveSilencer;	// Do we have a silencer mod
	bool			m_bHaveScope;		 // Do we have a scope mod

	HWEAPON			m_hWeapon;
	HAMMO			m_hAmmo;

	HWEAPON			m_hComplimentaryWeapon;

	float			m_fBobHeight;
	float			m_fBobWidth;

	StopWatchTimer	m_NextIdleTime;
	bool			m_bFire;
	bool			m_bFireHandled;	// was the player able to fire last update
	bool			m_bDryFireHandled;

	bool			m_bFireLeftHand;
	
	// Total amount of new ammo to distribute between the weapon clips...
	int32			m_nNewAmmoInClip;

	// What are we currently doing
	WeaponState		m_eState;
	WeaponState		m_eRequestedState;

	HMODELANIM		m_nSelectAni;			// Select weapon
	HMODELANIM		m_nDeselectAni;			// Deselect weapon
	HMODELANIM		m_nReloadAni;			// Reload weapon
	HMODELANIM		m_nPlayerAni;			// Default player ani

	HMODELANIM		m_nIdleAnis[WM_MAX_IDLE_ANIS];	// Idle animations
	HMODELANIM		m_nFireAnis[WM_MAX_FIRE_ANIS];	// Fire animations

	HMODELANIM		m_nPreFireAni;			// Optional animation to play before the actual fire ani
	HMODELANIM		m_nPostFireAni;			// Optional animation to play after the actual fire ani

	uint16			m_wIgnoreFX;	// FX to ignore for current vector/projectile

	bool			m_bWeaponDeselected;	// Did we just deselect the weapon

	bool			m_bDisabled;	// Is the weapon disabled
	bool			m_bVisible;	 // Is the weapon visible (should it be)

	bool			m_bControllingFlashLight; // Does this weapon control the pv flash light?

	LTRotation		m_rCamRot;
	LTVector		m_vCamPos;

	HLTSOUND		m_hLoopSound;
	uint8			m_nLoopSoundId;

	bool			m_bFirstSelection; // Is this the first time we are selecting this weapon?

	// used to determine if we should make a tracer
	int				m_nTracerNumber;

	// auto switch weapons
	bool			m_bAutoSwitchEnabled;
	bool			m_bAutoSwitch;

	// keyframed ClientFX
	CClientFXLinkNode	m_KeyframedClientFX;
	
	// Is the weapon supposed to be paused...
	bool			m_bPaused;

	// Animation property used to specify which animations the PlayerBody will use...
	EnumAnimProp	m_eWeaponAnimProp;

	std::string		m_sFireNode;

	StopWatchTimer	m_FireTimer;

	// timer to delay weapon switches
	StopWatchTimer  m_tmrFastSwitchDelay;


	// An Alt-Fire was requested, it must be fulfilled...
	bool			m_bAltFireRequested;

	// A grenade throw was requested, it must be fulfilled...
	bool			m_bGrenadeRequested;
	bool			m_bReloadInterrupted;

	// one shot per fire command?
	bool			m_bSemiAuto;

	//lock set after each fire to prevent looping in semi-auto weapons
	bool			m_bSemiAutoLock;

	// can do finishing moves using this weapon
	bool			m_bSupportsFinishingMoves;

	// queued up fire animation
	bool			m_bQueuedFireAnimation;
	bool			m_bQueuedBlockAnimation;

	// critical hit data
	std::string		m_sCriticalHitSocket;
	std::string		m_sCriticalHitImpactSocket;
	float			m_fCriticalHitDistanceSquared;
	float			m_fCriticalHitAngleCos;
	float			m_fCriticalHitViewAngleCos;

	// When true UpdateWeaponPosition will ignore the HandSocket transform...
	bool m_bWeaponTransformSet;

	// Cached activation type (used by context sensitive tools - forensics, entry, etc.)
	uint8 m_nActivationType;

	// When to deselect our current tool.
	double m_fDeselectToolTime;

	// Last special aim state before starting a transition animation.
	EnumAnimProp m_eLastSpecialAim;

	// Used to seed the weapon path random number generator and send to server in order to 
	// recreate the weapon path...
	uint8 m_nRandomSeed;

	// Used to keep perturb distribution relatively even, sent to the server to 
	// recreate the weapon path...
	uint8 m_nPerturbCount;

	// how close can we get to enemies with this weapon
	LTVector2 m_vMinProximity;

	// weapon will break soon, display a warning on the hud
	bool	m_bWeaponDamageThresholdExceeded;

	// extended animation support
	typedef std::vector<ConditionalAnimationController> AnimControllers;
	AnimControllers	m_aAnimControllers;
};


#endif //__CLIENT_WEAPON_H__
