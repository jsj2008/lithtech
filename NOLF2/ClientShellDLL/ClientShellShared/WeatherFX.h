// ----------------------------------------------------------------------- //
//
// MODULE  : WeatherFX.h
//
// PURPOSE : Weather special fx class - Definition
//
// CREATED : 3/23/99
//
// ----------------------------------------------------------------------- //

#ifndef __WEATHER_FX_H__
#define __WEATHER_FX_H__

#include "VolumeBrushFX.h"
#include "LineSystemFX.h"
#include "ParticleSystemFX.h"
#include "SurfaceMgr.h"
#include "BaseScaleFX.h"

#define NUM_SPLASH_SPRITES		20


struct WFXCREATESTRUCT : public VBCREATESTRUCT
{
    WFXCREATESTRUCT();

	void Read(ILTMessage_Read *pMsg);

    uint32  dwFlags;
    LTFLOAT  fViewDist;
};

inline WFXCREATESTRUCT::WFXCREATESTRUCT()
{
	dwFlags		= 0;
	fViewDist	= 0.0f;
}


inline void WFXCREATESTRUCT::Read(ILTMessage_Read *pMsg)
{
	VBCREATESTRUCT::Read(pMsg);

	dwFlags		= pMsg->Readuint32();
	fViewDist	= pMsg->Readfloat();
}

class CWeatherFX : public CVolumeBrushFX
{
	public :

		CWeatherFX() : CVolumeBrushFX()
		{
            m_bFirstUpdate  = LTTRUE;
			m_fArea			= 1.0;
			m_dwFlags		= 0;
			m_fFloorY		= 0.0f;
			m_eSurfaceType	= ST_UNKNOWN;
			m_fViewDist		= 1000.0f;

			m_vRainVel.Init();
			m_vSnowVel.Init();
			m_vRainPos.Init();
			m_vPos.Init();
			m_vDims.Init();
		}

        virtual LTBOOL Update();
        virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
        virtual LTBOOL CreateObject(ILTClient* pClientDE);

		void	DoSplash(LSLineStruct* pLine);

		virtual uint32 GetSFXID() { return SFX_WEATHER_ID; }

	protected :

        LTBOOL		m_bFirstUpdate;
        uint32		m_dwFlags;
        LTFLOAT		m_fFloorY;
        LTFLOAT		m_fViewDist;
		double		m_fArea;

		SurfaceType	m_eSurfaceType;

        LTVector	m_vRainVel;
        LTVector	m_vSnowVel;
        LTVector	m_vRainPos;
        LTVector	m_vPos;
        LTVector	m_vDims;

		CLineSystemFX		m_Rain;
		CParticleSystemFX	m_Snow;

		CBaseScaleFX	m_Splash[NUM_SPLASH_SPRITES];

        LTBOOL   CreateSnow();
        LTBOOL   CreateRain();
		void	CreateSplashSprites();
};

#endif // __WEATHER_FX_H__