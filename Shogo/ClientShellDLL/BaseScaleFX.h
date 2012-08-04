 // ----------------------------------------------------------------------- //
//
// MODULE  : BaseScaleFX.h
//
// PURPOSE : BaseScale special fx class - Definition
//
// CREATED : 5/27/98
//
// ----------------------------------------------------------------------- //

#ifndef __BASE_SCALE_FX_H__
#define __BASE_SCALE_FX_H__

#include "SpecialFX.h"
#include "client_physics.h"

struct BSCREATESTRUCT : public SFXCREATESTRUCT
{
	BSCREATESTRUCT::BSCREATESTRUCT();

	LTRotation	rRot;
	LTVector		vPos;
	LTVector		vVel;
	LTVector		vInitialScale;
	LTVector		vFinalScale;
	LTVector		vInitialColor;
	LTVector		vFinalColor;
	LTFLOAT		fLifeTime;
	LTFLOAT		fDelayTime;
	LTFLOAT		fInitialAlpha;
	LTFLOAT		fFinalAlpha;
	char*		pFilename;
	char*		pSkin;
	uint32		dwFlags;
	LTBOOL		bUseUserColors;
	LTBOOL		bLoop;
};

inline BSCREATESTRUCT::BSCREATESTRUCT()
{
	memset(this, 0, sizeof(BSCREATESTRUCT));
	rRot.Init();
	VEC_SET(vInitialColor, -1.0f, -1.0f, -1.0f);
	VEC_SET(vFinalColor, -1.0f, -1.0f, -1.0f);
}


class CBaseScaleFX : public CSpecialFX
{
	public :

		CBaseScaleFX()
		{
			m_rRot.Init();
			VEC_INIT(m_vPos);
			VEC_INIT(m_vVel);
			VEC_SET(m_vInitialScale, 1.0f, 1.0f, 1.0f);
			VEC_SET(m_vFinalScale, 1.0f, 1.0f, 1.0f);
			VEC_SET(m_vInitialColor, 1.0f, 1.0f, 1.0f);
			VEC_SET(m_vFinalColor, 1.0f, 1.0f, 1.0f);

			m_fLifeTime		= 1.0f;
			m_fInitialAlpha	= 1.0f;
			m_fFinalAlpha	= 1.0f;
			m_pFilename		= LTNULL;
			m_pSkin			= LTNULL;

			m_fStartTime	= 0.0f;
			m_fDelayTime	= 0.0f;
			m_fEndTime		= 0.0f;
			
			m_dwFlags		= 0;
			m_nType			= 0;
			m_nRotDir		= 1;
			m_bLoop			= LTFALSE;

			m_bUseUserColors = LTFALSE;
		}

		virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual LTBOOL Update();
		virtual LTBOOL CreateObject(ILTClient* pClientDE);

	protected :

		LTRotation	m_rRot;
		LTVector		m_vPos;
		LTVector		m_vVel;
		LTVector		m_vInitialScale;
		LTVector		m_vFinalScale;
		LTVector		m_vInitialColor;
		LTVector		m_vFinalColor;
		LTFLOAT		m_fLifeTime;
		LTFLOAT		m_fDelayTime;
		LTFLOAT		m_fInitialAlpha;
		LTFLOAT		m_fFinalAlpha;
		char*		m_pFilename;
		char*		m_pSkin;
		uint32		m_dwFlags;
		LTBOOL		m_bUseUserColors;
		LTBOOL		m_bLoop;
		
		int				m_nRotDir;
		unsigned short	m_nType;
		LTFLOAT			m_fStartTime;
		LTFLOAT			m_fEndTime;
		MovingObject	m_movingObj;

		virtual void UpdateAlpha(LTFLOAT fTimeDelta);
		virtual void UpdateScale(LTFLOAT fTimeDelta);
		virtual void UpdatePos(LTFLOAT fTimeDelta);
		virtual void UpdateRot(LTFLOAT fTimeDelta);
};

#endif // __BASE_SCALE_FX_H__