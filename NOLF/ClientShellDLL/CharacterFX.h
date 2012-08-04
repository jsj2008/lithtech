// ----------------------------------------------------------------------- //
//
// MODULE  : CharacterFX.h
//
// PURPOSE : Character special fx class - Definition
//
// CREATED : 8/24/98
//
// (c) 1998-2000 Monolith Productions, Inc.  All Rights Reserved
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
#include "LaserBeam.h"
#include "PolyLineFX.h"
#include "Timer.h"

#define MAX_DINGS	5

class CCharacterFX : public CSpecialFX
{
	public :

		CCharacterFX() : CSpecialFX()
		{
            m_pBubbles              = LTNULL;
            m_pCigarette	        = LTNULL;
            m_pSmokepuffs           = LTNULL;
            m_pZzz					= LTNULL;
            m_pHearts               = LTNULL;
            m_pLaser                = LTNULL;
			m_fNextBubbleTime		= -1.0f;
			m_fNextCigaretteTime	= -1.0f;
			m_fNextSmokepuffTime	= -1.0f;
			m_fNextZzzTime			= -1.0f;
			m_fNextHeartTime		= -1.0f;
            m_bLeftFoot             = LTTRUE;
            m_eLastSurface			= ST_UNKNOWN;
            m_bDrawZipCord          = LTFALSE;
			m_vZipCordEndPoint.Init();
			m_hMarker				= LTNULL;
			m_fNextMarkerTime		= -1.0f;
			m_eMarkerState			= MS_UNKNOWN;
			m_hDialogueSnd			= LTNULL;
			m_hVehicleSound			= LTNULL;
			m_bSubtitle				= LTFALSE;
			m_bOnVehicle			= LTFALSE;
			m_bOnMotorcycle			= LTFALSE;

			for (int i=0; i < MAX_DINGS; i++)
			{
				m_fNextDingTime[i] = -1.0f;
			}
		}

		~CCharacterFX();

		enum eMarkerState
		{
			MS_UNKNOWN = 0,
			MS_TEAM,
			MS_CHAT,
			MS_NUM_STATES,		// must be last state

		};

		void DoFootStep()
		{
			// Alternate feet...
			m_bLeftFoot = !m_bLeftFoot;
            DoFootStepKey(m_hServerObject, LTTRUE);
		}

		void PlayDingSound();

        void PlayMovementSound(LTVector vPos, SurfaceType eSurface,
			LTBOOL bLeftFoot, PlayerPhysicsModel eModel=PPM_NORMAL);

        virtual LTBOOL Init(HLOCALOBJ hServObj, HMESSAGEREAD hMessage);
        virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
        virtual LTBOOL CreateObject(ILTClient* pClientDE);
        virtual LTBOOL Update();
		virtual void  OnModelKey(HLOCALOBJ hObj, ArgList *pArgs);
        virtual LTBOOL OnServerMessage(HMESSAGEREAD hMessage);

		ModelId			GetModelId() const { return m_cs.eModelId; }
		ModelSkeleton	GetModelSkeleton() const { return m_cs.eModelSkeleton; }
		ModelType		GetModelType() const { return m_cs.eModelType; }
		ModelStyle		GetModelStyle() const { return m_cs.eModelStyle; }

        HLTSOUND        PlayLipSyncSound(char* szSound, LTFLOAT fRadius, LTBOOL & bSubtitle);
		void			PlayTaunt(uint32 nTauntId, LTBOOL bForce=LTTRUE);
		LTBOOL			IsPlayingTaunt() { return m_NodeController.IsLipSynching(); }

        SurfaceType     GetLastSurface() const { return m_eLastSurface; }

		CHARCREATESTRUCT	m_cs;

		enum Constants
		{
			kMaxAnimTrackers = 5,
		};

        LTAnimTracker         m_aAnimTrackers[kMaxAnimTrackers];              // Our pre-allocated anim trackers

		void ResetMarkerState();

		virtual uint32 GetSFXID() { return SFX_CHARACTER_ID; }

	protected :

        void CreateUnderwaterFX(LTVector & vPos);
        void UpdateUnderwaterFX(LTVector & vPos);
		void RemoveUnderwaterFX();

		void CreateCigaretteFX();
		void UpdateCigaretteFX();
		void RemoveCigaretteFX();

		void CreateSmokepuffsFX();
		void UpdateSmokepuffsFX();
		void RemoveSmokepuffsFX();

		void CreateZzzFX();
		void UpdateZzzFX();
		void RemoveZzzFX();

		void CreateHeartsFX();
		void UpdateHeartsFX();
		void RemoveHeartsFX();

		void UpdateLaserFX();
		void RemoveLaserFX();

		void UpdateBreathFX();
		void UpdateDamageFX();

        void TurnOnZipCord(LTVector vEndPoint);
		void TurnOffZipCord();
		void UpdateZipCordFX();

        void CreateTeamFX();
        void CreateChatFX();
        void CreateSleepingFX();
        void UpdateMarkerFX();
		void RemoveMarkerFX();
		LTBOOL NextMarkerState();

		void UpdateSounds();
		void HandleDialogueMsg(HMESSAGEREAD hMessage);
		void HandleTauntMsg(HMESSAGEREAD hMessage);

		void HandleZipcordMsg(HMESSAGEREAD hMessage);

        void    DoFootStepKey(HLOCALOBJ hObj, LTBOOL bForceSound=LTFALSE);
		void	CreateTrail(SurfaceType eType, IntersectInfo & iInfo);
		void	CreateFootprint(SurfaceType eType, IntersectInfo & iInfo);
		void    UpdateOnVehicle();
		void	UpdateMultiVehicleSounds();

        void    CreateVehicleTrail(CPolyLineFX* pTrail, LTVector vDir,
            LTVector vStartPoint, LTVector vEndPoint, LTBOOL bMotorcycle,
            LTBOOL bNewTrail);

		void	InitLocalPlayer();

		CFlashLightAI		m_Flashlight;		// Flashlight (only used if we're an AI)

		CSpecialFX*			m_pBubbles;			// Bubbles fx
		CSpecialFX*			m_pCigarette;		// Cigarette for poodle
		CSpecialFX*			m_pSmokepuffs;		// Smokepuffs for poodle
		CSpecialFX*			m_pZzz;				// Zzz for sleeping dudes
		CSpecialFX*			m_pHearts;			// Hearts for poodle

		CBaseScaleFX		m_CigaretteModel;
		BSCREATESTRUCT		m_scalecs;

		CLaserBeam*			m_pLaser;			// Laser fx
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

		CTimer				m_BreathTimer;

		CPolyLineFX			m_ZipCord;
        LTBOOL              m_bDrawZipCord;
        LTVector            m_vZipCordEndPoint;

		HOBJECT				m_hMarker;		// Multiplayer marker sprite
		LTFLOAT				m_fNextMarkerTime;
		eMarkerState		m_eMarkerState;

		HLTSOUND			m_hDialogueSnd;
		HLTSOUND			m_hVehicleSound;
		LTBOOL				m_bSubtitle;
		LTBOOL				m_bOnVehicle;
		LTBOOL				m_bOnMotorcycle;

		LTFLOAT				m_fNextDingTime[MAX_DINGS];
};

#endif // CHARCREATESTRUCT