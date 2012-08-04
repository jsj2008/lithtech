// ----------------------------------------------------------------------- //
//
// MODULE  : LineBallFX.h
//
// PURPOSE : LineBall special fx class - Definition
//
// CREATED : 9/06/97
//
// ----------------------------------------------------------------------- //

#ifndef __LINE_BALL_FX_H__
#define __LINE_BALL_FX_H__

#include "BaseLineSystemFX.h"

struct LBCREATESTRUCT : public SFXCREATESTRUCT
{
	LBCREATESTRUCT::LBCREATESTRUCT();

	LTRotation	rRot;
	LTVector		vPos;
	LTVector		vStartColor;
	LTVector		vEndColor;
	LTVector		vInitialScale;
	LTVector		vFinalScale;
	LTFLOAT		fSystemStartAlpha;
	LTFLOAT		fSystemEndAlpha;
	LTFLOAT		fStartAlpha;
	LTFLOAT		fEndAlpha;
	LTFLOAT		fOffset;
	LTFLOAT		fLifeTime;
	LTFLOAT		fLineLength;
};

inline LBCREATESTRUCT::LBCREATESTRUCT()
{
	memset(this, 0, sizeof(LBCREATESTRUCT));
	rRot.Init();
}


class CLineBallFX : public CBaseLineSystemFX
{
	public :

		CLineBallFX() : CBaseLineSystemFX() 
		{
			m_rRot.Init();
			VEC_SET(m_vStartColor, 1.0f, 1.0f, 1.0f);
			VEC_SET(m_vEndColor, 1.0f, 1.0f, 1.0f);
			VEC_SET(m_vInitialScale, 1.0f, 1.0f, 1.0f);
			VEC_SET(m_vFinalScale, 1.0f, 1.0f, 1.0f);

			m_fSystemStartAlpha	= 1.0f;
			m_fSystemEndAlpha	= 1.0f;
			m_fStartAlpha		= 1.0f;
			m_fEndAlpha			= 1.0f;
			m_fStartTime		= 0.0f;
			m_fLifeTime			= 0.0f;
			m_fOffset			= 10.0f;
			m_fLineLength		= 200.0f;
			m_bFirstUpdate		= LTTRUE;
		}

		virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual LTBOOL Update();

	protected :

		LTRotation	m_rRot;
		LTVector		m_vStartColor;
		LTVector		m_vEndColor;
		LTVector		m_vInitialScale;
		LTVector		m_vFinalScale;
		LTFLOAT		m_fStartAlpha;
		LTFLOAT		m_fEndAlpha;

		LTBOOL		m_bFirstUpdate;
		LTFLOAT		m_fStartTime;
		LTFLOAT		m_fLifeTime;
		LTFLOAT		m_fOffset;		// In degrees
		LTFLOAT		m_fLineLength;
		LTFLOAT		m_fSystemStartAlpha;
		LTFLOAT		m_fSystemEndAlpha;

		void		CreateLines();
		void		UpdateAlpha(LTFLOAT fTimeDelta);
		void		UpdateScale(LTFLOAT fTimeDelta);
		void		UpdateRotation(LTFLOAT fTimeDelta);
};

#endif // __LINE_BALL_FX_H__