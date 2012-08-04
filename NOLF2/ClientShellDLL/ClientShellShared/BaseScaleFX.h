 // ----------------------------------------------------------------------- //
//
// MODULE  : BaseScaleFX.h
//
// PURPOSE : BaseScale special fx class - Definition
//
// CREATED : 5/27/98
//
// (c) 1998-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __BASE_SCALE_FX_H__
#define __BASE_SCALE_FX_H__

#include "SpecialFX.h"
#include "client_physics.h"
#include "ButeListReader.h"

struct BSCREATESTRUCT : public SFXCREATESTRUCT
{
    BSCREATESTRUCT();

    LTRotation  rRot;
    LTVector    vPos;
    LTVector    vVel;
    LTVector    vInitialScale;
    LTVector    vFinalScale;
    LTVector    vInitialColor;
    LTVector    vFinalColor;
    LTFLOAT     fLifeTime;
    LTFLOAT     fDelayTime;
    LTFLOAT     fInitialAlpha;
    LTFLOAT     fFinalAlpha;
    LTFLOAT     fMinRotateVel;
    LTFLOAT     fMaxRotateVel;
	const char*	pFilename;
    uint32      dwFlags;
    LTBOOL      bUseUserColors;
    LTBOOL      bLoop;
    LTBOOL      bAdditive;
    LTBOOL      bMultiply;
    LTBOOL      bRotate;
    LTBOOL      bRotateLeft;
    LTBOOL      bFaceCamera;
	LTBOOL		bPausable;
    uint8       nRotationAxis;
    uint8       nType;
	uint8		nMenuLayer;

	CButeListReader* pRenderStyleReader;
	CButeListReader* pSkinReader;
};

inline BSCREATESTRUCT::BSCREATESTRUCT()
{
	fLifeTime		= 0.0f;
	fDelayTime		= 0.0f;
	fInitialAlpha	= 0.0f;
	fFinalAlpha		= 0.0f;
	fMinRotateVel	= 1.0f;
	fMaxRotateVel	= 1.0f;
    pFilename       = LTNULL;
	dwFlags			= 0;
    bUseUserColors  = LTFALSE;
    bLoop           = LTFALSE;
    bAdditive       = LTFALSE;
    bMultiply       = LTFALSE;
    bRotate         = LTFALSE;
    bFaceCamera     = LTFALSE;
	bPausable		= LTFALSE;
	nRotationAxis	= 0;
	nType			= OT_SPRITE;
	nMenuLayer		= 0;

	rRot.Init();
	vPos.Init();
	vVel.Init();
	vInitialScale.Init();
	vFinalScale.Init();
	vInitialColor.Init(-1.0f, -1.0f, -1.0f);
	vFinalColor.Init(-1.0f, -1.0f, -1.0f);

	pRenderStyleReader = LTNULL;
	pSkinReader = LTNULL;


}


class CBaseScaleFX : public CSpecialFX
{
	public :

		CBaseScaleFX()
		{
			m_rRot.Init();
			m_vPos.Init();
			m_vVel.Init();
			m_vInitialScale.Init(1.0f, 1.0f, 1.0f);
			m_vFinalScale.Init(1.0f, 1.0f, 1.0f);
			m_vInitialColor.Init(1.0f, 1.0f, 1.0f);
			m_vFinalColor.Init(1.0f, 1.0f, 1.0f);

			m_fLifeTime		= 1.0f;
			m_fInitialAlpha	= 1.0f;
			m_fFinalAlpha	= 1.0f;
			m_fRotateVel	= 1.0f;

            m_pFilename     = LTNULL;

			m_pSkinReader = LTNULL;
			m_pRenderStyleReader = LTNULL;

			m_fStartTime	= 0.0f;
			m_fDelayTime	= 0.0f;
			m_fEndTime		= 0.0f;
			m_fElapsedTime  = 0.0f;

			m_dwFlags		= 0;
			m_nType			= 0;
			m_nRotDir		= 1;
            m_bLoop         = LTFALSE;
            m_bAdditive     = LTFALSE;
            m_bMultiply     = LTFALSE;
            m_bRotate       = LTFALSE;
            m_bFaceCamera   = LTFALSE;
			m_bPausable		= LTFALSE;
			m_nRotationAxis = 0;

            m_bUseUserColors = LTFALSE;

			m_nMenuLayer	= 0;
		}

        virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
        virtual LTBOOL Update();
        virtual LTBOOL CreateObject(ILTClient* pClientDE);

        virtual LTBOOL Reset();

		void	AdjustScale(LTFLOAT fScaleMultiplier);

		virtual uint32 GetSFXID() { return SFX_SCALE_ID; }

	protected :

        LTRotation	m_rRot;

        LTVector	m_vPos;
        LTVector	m_vVel;
        LTVector    m_vInitialScale;
        LTVector    m_vFinalScale;
        LTVector    m_vInitialColor;
        LTVector    m_vFinalColor;

        LTFLOAT     m_fLifeTime;
        LTFLOAT     m_fDelayTime;
		LTFLOAT		m_fElapsedTime;
        LTFLOAT     m_fInitialAlpha;
        LTFLOAT     m_fFinalAlpha;
        LTFLOAT     m_fRotateVel;

		const char*	m_pFilename;

		CButeListReader* m_pRenderStyleReader;
		CButeListReader* m_pSkinReader;

        uint32      m_dwFlags;
        LTBOOL      m_bUseUserColors;
        LTBOOL      m_bLoop;
        LTBOOL      m_bAdditive;
        LTBOOL      m_bMultiply;
        LTBOOL      m_bRotate;
        LTBOOL      m_bFaceCamera;
		LTBOOL		m_bPausable;
		uint8		m_nRotationAxis;

		int				m_nRotDir;
		unsigned short	m_nType;
        LTFLOAT         m_fStartTime;
        LTFLOAT         m_fEndTime;
		MovingObject	m_movingObj;

        virtual void UpdateAlpha(LTFLOAT fTimeDelta);
        virtual void UpdateScale(LTFLOAT fTimeDelta);
        virtual void UpdatePos(LTFLOAT fTimeDelta);
        virtual void UpdateRot(LTFLOAT fTimeDelta);
};

#endif // __BASE_SCALE_FX_H__