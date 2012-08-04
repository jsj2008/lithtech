// ----------------------------------------------------------------------- //
//
// MODULE  : CharacterFX.h
//
// PURPOSE : Character special fx class - Definition
//
// CREATED : 8/24/98
//
// (c) 1998-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __CHARACTER_FX_H__
#define __CHARACTER_FX_H__

#include "SpecialFX.h"
#include "SurfaceMgr.h"
#include "NodeController.h"
#include "ModelButeMgr.h"
#include "iltmodel.h"
#include "SharedFXStructs.h"
#include "PolyLineFX.h"
#include "Timer.h"
#include "FlashLight.h"
#include "ClientTrackedNodeMgr.h"
#include "ClientTrackedNodeContext.h"
#include "ClientFXMgr.h"
#include "iltfontmanager.h"
#include "FXButeMgr.h"
#include "HitBox.h"

#define MAX_DINGS	5

class CCharacterFX : public CSpecialFX
{
	public :

		CCharacterFX() : CSpecialFX()
		{
            m_pBubbles              = LTNULL;
            m_pCigarette	        = LTNULL;
            m_pZzz					= LTNULL;
			m_fNextBubbleTime		= -1.0f;
			m_fNextCigaretteTime	= -1.0f;
			m_fNextSmokepuffTime	= -1.0f;
			m_fNextZzzTime			= -1.0f;
			m_fNextHeartTime		= -1.0f;
            m_bLeftFoot             = LTTRUE;
            m_eLastSurface			= ST_UNKNOWN;
			m_hDialogueSnd			= LTNULL;
			m_hVehicleSound			= LTNULL;
			m_bSubtitle				= LTFALSE;
			m_bOnVehicle			= LTFALSE;
			m_hWeaponLoopSound		= LTNULL;
								
			for (int i=0; i < MAX_DINGS; i++)
			{
				m_fNextDingTime[i] = -1.0f;
			}

			m_szInfoString[0] = 0;
			m_szInfoString[kMaxInfoStringLength] = 0;

			m_hUpperTorsoNode		= INVALID_TRACKEDNODE;
			m_hLowerTorsoNode		= INVALID_TRACKEDNODE;
			m_hHeadNode				= INVALID_TRACKEDNODE;

			m_pStr					= NULL;
			m_nLastDamageFlags		= 0;
			m_nInstantDamageFlags	= 0;
			
			m_bWasPlayingSpecialDamageAni = false;

			m_bDamageFxTrackingOverride = false;

			m_bPlayerDead			= false;

			m_p3rdPersonDamageFX	= NULL;
			m_nNum3rdPersonDamageFX	= 0;

			m_bPitchEnabled			= false;
		}

		~CCharacterFX();

		void DoFootStep()
		{
			// Alternate feet...
			m_bLeftFoot = !m_bLeftFoot;
            DoFootStepKey(m_hServerObject, LTTRUE);
		}

		void PlayDingSound();

        void PlayMovementSound(LTVector vPos, SurfaceType eSurface,
			LTBOOL bLeftFoot, PlayerPhysicsModel eModel=PPM_NORMAL);

 		void ResetSoundBufferData() { m_NodeController.ResetSoundBufferData(); }
 
		virtual LTBOOL	Init(HLOCALOBJ hServObj, ILTMessage_Read *pMsg);
        virtual LTBOOL	Init(SFXCREATESTRUCT* psfxCreateStruct);
        virtual LTBOOL	CreateObject(ILTClient* pClientDE);
        virtual LTBOOL	Update();
		virtual void	OnModelKey(HLOCALOBJ hObj, ArgList *pArgs);
        virtual LTBOOL	OnServerMessage(ILTMessage_Read *pMsg);
		virtual void	OnObjectRotate( LTRotation *pNewRot );

		virtual void	ChangeModel(ModelId eModelId);

		ModelId			GetModelId() const { return m_cs.eModelId; }
		ModelSkeleton	GetModelSkeleton() const { return m_cs.eModelSkeleton; }
		ModelType		GetModelType() const { return m_cs.eModelType; }

        HLTSOUND        PlayLipSyncSound(char* szSound, LTFLOAT fRadius, LTBOOL & bSubtitle, bool bSubtitlePriority, bool bGetHandle = true);
		void			PlayTaunt(uint32 nTauntId, LTBOOL bForce=LTTRUE);
		LTBOOL			IsPlayingTaunt() { return m_NodeController.IsLipSynching(); }

        SurfaceType     GetLastSurface() const { return m_eLastSurface; }

		virtual uint32 GetSFXID() { return SFX_CHARACTER_ID; }

		HOBJECT	GetHitBox() const { return m_HitBox.GetObject(); }
				
		bool	CanBeCarried() { return m_cs.bCanCarry; }
		bool	CanWake() { return m_cs.bCanWake; }

		const char * GetInfoString() const { return m_szInfoString; }

		// show or hide all client effects attached to this character model
		void ShowAttachClientFX();
		void HideAttachClientFX();
		
		virtual void Render(HOBJECT hCamera);

		bool	IsPlayerDead() { return m_cs.bIsPlayer && m_bPlayerDead; }
		bool	IsUnconscious() { return !!(m_cs.nDamageFlags & DamageTypeToFlag( DT_SLEEPING )); }
		bool	Slipped() { return !!(m_cs.nDamageFlags & DamageTypeToFlag( DT_SLIPPERY ));	}

		static uint32 GetNumPlayersInGame() { return m_lstPlayersInGame.size(); }

		uint8	GetCarrying() { return m_cs.nCarrying; }
		
		virtual void EnablePitch( bool bEnable );
			
		enum Constants
		{
			kMaxInfoStringLength = 1023
		};

		CHARCREATESTRUCT	m_cs;

	protected :
		// create or destroy all client effects attached to this character model
		void CreateAttachClientFX();
		void RemoveAttachClientFX();

        void CreateUnderwaterFX(const LTVector & vPos);
        void UpdateUnderwaterFX(LTVector & vPos);
		void RemoveUnderwaterFX();

		void CreateCigaretteFX();
		void UpdateCigaretteFX();
		void RemoveCigaretteFX();

		void CreateZzzFX();
		void UpdateZzzFX();
		void RemoveZzzFX();

		void UpdateFlashLightFX();
		void RemoveFlashLightFX();

		void UpdateBreathFX();
		
		void UpdateDamageFX();
		void ShutdownDamageFX();

		void UpdatePlayerAlphaCycle( );
		void RemovePlayerAlphaCycle( );

		void UpdateSounds();
		void HandleDialogueMsg(ILTMessage_Read *pMsg);
		void HandleTauntMsg(ILTMessage_Read *pMsg);
		void HandleWeaponSoundLoopMsg(ILTMessage_Read *pMsg);
		void HandlePitchMsg( ILTMessage_Read *pMsg );
		void UpdatePitch( );
		void KillWeaponLoopSound();

		void ResetPitchTracking();

        void    DoFootStepKey(HLOCALOBJ hObj, LTBOOL bForceSound=LTFALSE);
		void	CreateTrail(SurfaceType eType, IntersectInfo & iInfo);
		void	CreateFootprint(SurfaceType eType, IntersectInfo & iInfo);
		void    UpdateOnVehicle();
		void	UpdateMultiVehicleSounds();
		void	UpdateVehicleContour( LTRotation &rCharacterRot );

        void    CreateVehicleTrail(CPolyLineFX* pTrail, LTVector vDir,
            LTVector vStartPoint, LTVector vEndPoint, LTBOOL bNewTrail);

		void	InitLocalPlayer();

		void	UpdateAttachments();

		void	KillLipSyncSound( bool bSendNotification );

		void	UpdateCarrying();


		CFlashLight3rdPerson	m_3rdPersonFlashlight;	// Only used if fx is seen from 3rd person

		CSpecialFX*			m_pBubbles;			// Bubbles fx
		CSpecialFX*			m_pCigarette;		// Cigarette for poodle
		CSpecialFX*			m_pZzz;				// Zzz for sleeping dudes

		CBaseScaleFX		m_CigaretteModel;
		BSCREATESTRUCT		m_scalecs;

        LTFLOAT             m_fNextBubbleTime;
        LTFLOAT             m_fNextHeartTime;
        LTFLOAT             m_fNextCigaretteTime;
        LTFLOAT             m_fNextSmokepuffTime;
        LTFLOAT             m_fNextZzzTime;
        LTBOOL              m_bLeftFoot;
        SurfaceType         m_eLastSurface;

		CPolyLineFX			m_VehicleTrail1;
		CPolyLineFX			m_VehicleTrail2;
		CNodeController		m_NodeController;	// Our node controller

		//the amount of time that has currently elapsed for this breath interval
		float				m_fBreathElapsedTime;
		//the amount of time that needs to elapse for another breath to be emitted
		float				m_fBreathEndTime;

		HLTSOUND			m_hDialogueSnd;
		uint8				m_nUniqueDialogueId;
		bool				m_bSubtitlePriority;

		HLTSOUND			m_hVehicleSound;
		LTBOOL				m_bSubtitle;
		LTBOOL				m_bOnVehicle;

		LTFLOAT				m_fNextDingTime[MAX_DINGS];

		char				m_szInfoString[kMaxInfoStringLength + 1];
		
		HLTSOUND			m_hWeaponLoopSound;
		
		HTRACKEDNODE		m_hUpperTorsoNode;
		HTRACKEDNODE		m_hLowerTorsoNode;
		HTRACKEDNODE		m_hHeadNode;

		CClientTrackedNodeContext	m_TrackedNodeContext;

		CLIENTFX_LINK_NODE	m_AttachClientFX;

		CLIENTFX_LINK		*m_p3rdPersonDamageFX;
		uint32				m_nNum3rdPersonDamageFX;

		CLIENTFX_LINK		m_link3rdPersonInstFX;
		CLIENTFX_LINK		m_linkChatFX;
		
		CUIFormattedPolyString *m_pStr;
		LTVector				m_vStrPos;

		DamageFlags			m_nLastDamageFlags;
		DamageFlags			m_nInstantDamageFlags;
		
		bool				m_bWasPlayingSpecialDamageAni;
		bool				m_bDamageFxTrackingOverride;

		bool				m_bPlayerDead;

		bool				m_bUpdateAttachments;

		CTimer				m_PlayerAlphaCycleTimer;

		CHitBox				m_HitBox;

		bool				m_bPitchEnabled;
		
		typedef std::vector<CCharacterFX*> CharFXList;
		static CharFXList	m_lstPlayersInGame;
};

#endif 
