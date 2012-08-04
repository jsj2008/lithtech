// ----------------------------------------------------------------------- //
//
// MODULE  : CharacterFX.h
//
// PURPOSE : Character special fx class - Definition
//
// CREATED : 8/24/98
//
// (c) 1998-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __CHARACTER_FX_H__
#define __CHARACTER_FX_H__

#include "SpecialFX.h"
#include "NodeController.h"
#include "iltmodel.h"
#include "SharedFXStructs.h"
#include "FlashLight.h"
#include "ClientNodeTrackerContext.h"
#include "ClientFXMgr.h"
#include "HitBox.h"
#include "NodeBlink.h"
#include "AnimationDescriptors.h"
#include "EngineTimer.h"
#include "objectdetector.h"
#include "LeanNodeController.h"
#include "CharacterPhysics.h"
#include "CharacterDisplay.h"
#include "ClientMeleeCollisionController.h"
#include "PlayerRigidBody.h"
#include "TurretFX.h"

#define MAX_DINGS	5

class CHUDNavMarker;
class RotationInterpolator;

class CCharacterFX : public CSpecialFX
{
	public :

		CCharacterFX() : CSpecialFX()
		{
			m_bLeftFoot				= true;
            m_eLastSurface			= ST_UNKNOWN;
			m_hDialogueSnd			= NULL;
			m_hDialogueRadioSnd		= NULL;
			m_bSubtitle				= false;
			m_hWeaponLoopSound		= NULL;
			m_hSlideSound			= NULL;


			m_nLastDamageFlags		= 0;
			m_nInstantDamageFlags	= 0;
			
			m_bWasPlayingSpecialDamageAni = false;

			m_FadeTimer.SetEngineTimer( RealTimeTimer::Instance());

			m_p3rdPersonDamageFX	= NULL;
			m_nNum3rdPersonDamageFX	= 0;

			m_bStrVisible = false;

			m_pHUDItem = NULL;
			m_hHUDType = NULL;

			m_eBodyState = eBodyStateNormal;
			m_eBodyCondition = kAD_None;

			for( uint32 i = 0; i < MAX_DINGS; i++ )
				m_DingTimers[i].SetEngineTimer( RealTimeTimer::Instance( ));

			m_fFootstepVolume = -1.0f;
			m_hBodyMovementSound = NULL;

			m_flBlockWindowEndTime = 0.0f;

			m_psz1stPersonInstFXName = NULL;
			m_psz3rdPersonInstFXName = NULL;

			m_bInitRagDoll = false;
			m_hWallStickObject = NULL;
			m_pNameDisplay = NULL;
			m_pInsigniaDisplay = NULL;
			m_pRotationInterpolator = NULL;
			m_bFirst3d = true;
			m_hDeathFX = NULL;
			m_bSeverBody = false;
			m_bIsUnderWater = false;

			m_pTurretFX = NULL;
		}

		~CCharacterFX();

		enum EFootStep
		{
			eFootStep_Left,
			eFootStep_Right,
			eFootStep_Alternate,
		};

		void DoFootStep( EFootStep eFootStep )
		{
			switch( eFootStep )
			{
			case eFootStep_Left:
				m_bLeftFoot = true;
				break;
			case eFootStep_Right:
				m_bLeftFoot = false;
				break;
			case eFootStep_Alternate:
				m_bLeftFoot = !m_bLeftFoot;
				break;
			}
			DoFootStepKey(GetLocalContextObject(), true);
		}

		void PlayDingSound();

		void SetFootstepVolume( float fScale ) { m_fFootstepVolume = fScale; }
		void PlayFootstepSound( LTVector const& vPos, SurfaceType eSurface, bool bLeftFoot, PlayerPhysicsModel ePPModel = PPM_NORMAL );

		void SetMovementSound( HRECORD hRecord ) { m_hBodyMovementSound = hRecord; }
		void PlayMovementSound();

 		void ResetSoundBufferData() { m_NodeController.ResetSoundBufferData(); }
 
		virtual bool	Init(HLOCALOBJ hServObj, ILTMessage_Read *pMsg);
		virtual bool	Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual bool	CreateObject(ILTClient* pClientDE);
		virtual bool	Update();
		virtual void	OnModelKey(HLOCALOBJ hObj, ArgList *pArgs, ANIMTRACKERID nTrackerId);
		virtual bool	OnServerMessage(ILTMessage_Read *pMsg);
		virtual void	OnObjectRotate( LTRotation *pNewRot );

		// Gets the model record for this index.
		ModelsDB::HMODEL MPModelIndexToModel( uint8 nMPModelIndex ) const;

		// Checks if we are currently using the mp model specified.
		virtual bool	IsUsingMPModel( uint8 nMPModelIndex ) const;
		
		// Switch to the mp model.
		virtual void	ChangeMPModel( uint8 nMPModelIndex );

		ModelsDB::HMODEL GetModel() const { return m_cs.hModel; }
		ModelsDB::HSKELETON GetModelSkeleton() const { return g_pModelsDB->GetModelSkeleton( m_cs.hModel ); }

		// The caller is responsible for killing the sound, the subtitle, the dialog icon, and the radio sound.
		HLTSOUND		PlayLipSyncSound(const char* pszSoundID, float fOuterRadius, float fInnerRadius, bool & bSubtitle, bool bSubtitlePriority, int16 nMixChannel, bool bGetHandle, const char* szIcon = NULL, const bool bUseRadioSound = false, HLTSOUND* pRadioSoundHandle  = NULL );

		void			PlayBroadcast(uint32 nBroadcastId, bool bForce, uint32 nClientID, int8 nPriority, bool bPlayerControlled);
		bool			IsPlayingBroadcast() const;

        SurfaceType     GetLastSurface() const { return m_eLastSurface; }

		virtual uint32 GetSFXID() { return SFX_CHARACTER_ID; }

		HOBJECT	GetHitBox() const { return m_HitBox.GetObject(); }

		const wchar_t * GetInfoString() const { return m_wsInfoString.c_str(); }

		// show or hide all client effects attached to this character model
		void ShowAttachClientFX( uint32 nElement );
		void HideAttachClientFX( uint32 nElement );
		
		virtual void Render(HOBJECT hCamera);

		bool	IsPlayerDead() { return m_cs.bIsPlayer && m_cs.bIsDead; }
		bool	IsSpectating() { return m_cs.bIsSpectating; }
		bool	IsDead() { return m_cs.bIsDead; }

		typedef std::vector<CCharacterFX*, LTAllocator<CCharacterFX*, LT_MEM_TYPE_CLIENTSHELL> > CharFXList;
		static CharFXList& GetCharFXList() { return m_lstCharacters; }

		enum Constants
		{
			kMaxInfoStringLength = 1023
		};

		//update HUDElements related to this Character (e.g. team markers)
		void	UpdateHUDData();
		void	HandleChat();

		CHARCREATESTRUCT	m_cs;

		bool		IsBlockWindowOpen() const { return m_flBlockWindowEndTime > g_pLTClient->GetTime(); }
		bool		IsBlocking() const { return m_MeleeCollisionController.IsBlocking(); }
		void		HandleMeleeModelKey(HLOCALOBJ hObj, ArgList *pArgs, ANIMTRACKERID nTrackerId) { m_MeleeCollisionController.HandleModelKey(hObj, pArgs, nTrackerId); }
	
		BodyState GetBodyState() const { return m_eBodyState; }
		EnumAnimDesc GetBodyCondition() const { return m_eBodyCondition; }

		void		DoUpdateAttachments( ) { m_bUpdateAttachments = true; }

		void		InitDisplays();
		void		ResetDisplays();

		void		CreateSlowMoFX(const char *pszFX);
		void		RemoveSlowMoFX();

		// Update the aim at node tracker with the correct position...
		void	UpdatePlayerAimPosition( );

		CClientNodeTrackerContext& GetNodeTrackerContext( ) { return m_NodeTrackerContext; }

		CLeanNodeController& GetLeanNodeController( ) { return m_LeanNodeController; }

		// Enable / Disable the aim at node tracker...
		void	EnablePlayerAimTracking( bool bEnable );

		// Cache the turret the player is activating and handle anyother player setup for operating a turret...
		void SetOperatingTurret( CTurretFX *pTurret ) {m_pTurretFX = pTurret; }

		bool IsOperatingTurret( ) const { return (m_pTurretFX != NULL);	}
		CTurretFX* GetTurret( ) const { return m_pTurretFX; }


	protected :
		void InitDead();
		void ClearDead();
		void UpdateDead();
		void UpdateFade();

		// create or destroy all client effects attached to this character model
		void CreateAttachClientFX();
		void RemoveAttachClientFX();

        void CreateUnderwaterFX(const LTVector & vPos);
        void UpdateUnderwaterFX();
		void RemoveUnderwaterFX();

		void UpdateFlashLightFX();
		void RemoveFlashLightFX();

//		void UpdateBreathFX();
		
		void UpdateDamageFX();
		void ShutdownDamageFX();

		void UpdateSounds();
		void HandleDialogueMsg(ILTMessage_Read *pMsg);
		void HandleBroadcastMsg(ILTMessage_Read *pMsg);
		void HandleWeaponSoundMsg(ILTMessage_Read *pMsg);
		void HandleWeaponSoundLoopMsg(ILTMessage_Read *pMsg);
		void HandleBodyStateMsg(ILTMessage_Read *pMsg);
		void HandleSeverMsg(ILTMessage_Read *pMsg);

		void KillWeaponLoopSound();
		void KillSlideSound();

		// Reset the node tracking context...
		void	ResetNodeTrackerContext();
 		void	ResetNodeBlinkContext();

		void	DoFootStepKey(HLOCALOBJ hObj, bool bForceSound=false);
		bool	IsFootStepException(HLOCALOBJ hObj);
		void	CreateTrail(SurfaceType eType, IntersectInfo & iInfo);
		void	CreateFootprint(SurfaceType eType, IntersectInfo & iInfo);

		void	InitLocalPlayer();

		void	UpdateAttachments();

		void	KillLipSyncSound( bool bSendNotification );

		// Change the character model if needed...
		void	UpdateCharacterModel( );

		bool	IsLocalClient() const;

		// This will get the object to use for the local context.  So, if the characterfx
		// is the local player, it will return the playerbody object, otherwise, 
		// it will return the server's hobject of the characterfx.
		HOBJECT GetLocalContextObject( );


	protected:

		Flashlight			m_3rdPersonFlashlight;	// Only used if fx is seen from 3rd person

		bool				m_bLeftFoot;
        SurfaceType         m_eLastSurface;

		CNodeController		m_NodeController;	// Our node controller

		//the amount of time that has currently elapsed for this breath interval
//		float				m_fBreathElapsedTime;
		//the amount of time that needs to elapse for another breath to be emitted
//		float				m_fBreathEndTime;

		HLTSOUND			m_hDialogueSnd;
		HLTSOUND			m_hDialogueRadioSnd;
		uint8				m_nUniqueDialogueId;
		bool				m_bSubtitlePriority;

		bool				m_bSubtitle;

		std::string			m_sDialogueIcon;

		StopWatchTimer		m_DingTimers[MAX_DINGS];

		std::wstring		m_wsInfoString;
		
		HLTSOUND			m_hWeaponLoopSound;
		HLTSOUND			m_hSlideSound;
		
		CClientNodeTrackerContext	m_NodeTrackerContext;

		CClientMeleeCollisionController m_MeleeCollisionController;

		BlinkController		m_BlinkController;

		CClientFXLinkNode	m_AttachClientFX;

		CClientFXLink		*m_p3rdPersonDamageFX;
		uint32				m_nNum3rdPersonDamageFX;

		CClientFXLink		m_link1stPersonInstFX;
		const char*			m_psz1stPersonInstFXName;

		CClientFXLink		m_link3rdPersonInstFX;
		const char*			m_psz3rdPersonInstFXName;

		CClientFXLink		m_linkChatFX;
		
		CClientFXLink		m_linkLoopFX;
		CClientFXLink		m_linkSlowMoFX;

		CLTGUIString		m_Str;
		LTVector			m_vPos;
		uint32				m_nFontHeight;
		bool				m_bStrVisible;

		DamageFlags			m_nLastDamageFlags;
		DamageFlags			m_nInstantDamageFlags;
		
		bool				m_bWasPlayingSpecialDamageAni;
		bool				m_bUpdateAttachments;

		CHitBox				m_HitBox;

		static CharFXList	m_lstCharacters;

		CHUDNavMarker *	m_pHUDItem;
		HRECORD			m_hHUDType;

		BodyState			m_eBodyState;
		EnumAnimDesc		m_eBodyCondition;

		ObjectDetectorLink	m_iFocusLink;
		ObjectDetectorLink	m_iAttackPredictionLink;

		StopWatchTimer		m_FadeTimer;

		ObjRefVector		m_hSeveredParts;				// links to the parts I've lost
		bool				m_bSeverBody;

		HRECORD				m_hDeathFX;

		float				m_fFootstepVolume;
		HRECORD				m_hBodyMovementSound;

		double				m_flBlockWindowEndTime;

		CLeanNodeController	m_LeanNodeController;

		CCharacterPhysics	m_CharacterPhysics;

		// Initialize ragdoll after the character has died...
		bool				m_bInitRagDoll;

		// Model created when we got stuck to a wall...
		LTObjRef			m_hWallStickObject;

		CharacterDisplay*	m_pNameDisplay;
		CharacterDisplaySimple*	m_pInsigniaDisplay;

		static int32		m_nBroadcastPriority;
		static float		m_fBroadcastOuterRadius;
		static float		m_fBroadcastInnerRadius;
		bool				m_bPlayerBroadcast;

		RotationInterpolator* m_pRotationInterpolator;

		bool				m_bFirst3d;

		bool				m_bIsUnderWater;

		CPlayerRigidBody	m_PlayerRigidBody;

		CTurretFX*			m_pTurretFX;
};

#endif 

// EOF

