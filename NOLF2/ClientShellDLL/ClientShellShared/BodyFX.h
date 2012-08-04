// ----------------------------------------------------------------------- //
//
// MODULE  : BodyFX.h
//
// PURPOSE : Body special fx class - Definition
//
// CREATED : 02.01.2000
//
// (c) 1998-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __BODY_FX_H__
#define __BODY_FX_H__

#include "SpecialFX.h"
#include "ModelButeMgr.h"
#include "SharedFXStructs.h"
#include "Animator.h"
#include "ClientFXMgr.h"
#include "HitBox.h"

class CRagDoll;

class CBodyFX : public CSpecialFX
{
	public :

		// Ctors/Dtors/etc

		CBodyFX();
		~CBodyFX();

        LTBOOL Init(HLOCALOBJ hServObj, ILTMessage_Read *pMsg);
        LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);

        LTBOOL CreateObject(ILTClient* pClientDE);

		// Updates

        LTBOOL	Update();
		void	UpdateAttachments();

		// Handlers

        LTBOOL OnServerMessage(ILTMessage_Read *pMsg);
		void OnModelKey(HLOCALOBJ hObj, ArgList *pArgs);

		uint8 GetClientId() const { return m_bs.nClientId; }
		
		virtual uint32 GetSFXID() { return SFX_BODY_ID; }

		HOBJECT	GetHitBox() const { return m_HitBox.GetObject(); }

		bool	CanBeCarried() const { return m_bs.bCanBeCarried; }
		bool	CanBeRevived() const { return m_bs.bCanBeRevived; }
		
		ModelSkeleton GetModelSkeleton() const { return g_pModelButeMgr->GetModelSkeleton(m_bs.eModelId); }

		float	GetTimeCreated() const { return m_fCreateTime;	}

		void	RemoveClientAssociation( );

	protected :

		// Updates

		void UpdateFade();
		void FadeBackpack();

		bool HandleDeathFXKey( HLOCALOBJ hObj, ArgList* pArgList );
		bool HandleFXKey( HLOCALOBJ hObj, ArgList* pArgList );

		bool SetupRagDoll();

		bool CreateFX(char* pFXName);

		void CreateDamageFX(DamageType eType);

		void CreateBackpack();
		void UpdateBackpack();
		void RemoveBackpack();

	protected :

		CRagDoll*			m_pRagDoll;

		BODYCREATESTRUCT	m_bs;					// Our createstruct
		HOBJECT				m_hBackpack;
		bool				m_bFadeToBackpack;


		LTFLOAT				m_fFaderTime;
		LTFLOAT				m_fFaderTimer;
		LTFLOAT				m_fBackpackFaderTime;
		LTFLOAT				m_fBackpackFaderTimer;

		bool				m_bHidden;

		// keyframed ClientFX
		CLIENTFX_LINK		m_fxDeath;
		CLIENTFX_LINK		m_fx;

		CHitBox				m_HitBox;
		
		float				m_fCreateTime;	// Time stamp of when this body was created.
};

#endif