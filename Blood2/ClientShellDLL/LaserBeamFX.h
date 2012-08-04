// ----------------------------------------------------------------------- //
//
// MODULE  : LaserBeamFX.h
//
// PURPOSE : Special FX class for general laser beam FX using models
//
// CREATED : 8/10/98
//
// ----------------------------------------------------------------------- //

#ifndef __LASERBEAM_FX_H__
#define __LASERBEAM_FX_H__

#include "SpecialFX.h"

// ----------------------------------------------------------------------- //

struct LASERBEAMCREATESTRUCT : public SFXCREATESTRUCT
{
	LASERBEAMCREATESTRUCT::LASERBEAMCREATESTRUCT();

	DVector		vSource;
	DVector		vDest;
	DBYTE		nType;
};

// ----------------------------------------------------------------------- //

inline LASERBEAMCREATESTRUCT::LASERBEAMCREATESTRUCT()
{
	memset(this, 0, sizeof(LASERBEAMCREATESTRUCT));
}

// ----------------------------------------------------------------------- //

class CLaserBeamFX : public CSpecialFX
{
	public :

		CLaserBeamFX() : CSpecialFX() {}

		virtual DBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		DBOOL	CreateObject(CClientDE *pClientDE);
		virtual DBOOL Update();

	private :

		void	SetupBeam();
		DBOOL	UpdateBeam();

		DVector		m_vSource;		// Source of the laser
		DVector		m_vDest;		// Destination of the laser

		DFLOAT		m_fMinScale;	// Minimum system radius
		DFLOAT		m_fMaxScale;	// Maximum system radius
		DVector		m_vScale;		// Current scale for the beam

		DFLOAT		m_fDuration;	// Time to make the beam hang around
		DFLOAT		m_fScaleTime;	// Time to scale from min to max
		DFLOAT		m_fFadeTime;	// Time to fade off at the end of lifetime
		DFLOAT		m_fAlpha;		// Alpha for the model

		DBYTE		m_nType;		// Type of laser (BIG, SMALL, COLOR)
		DBYTE		m_bWaveForm;	// Scaling waveform to use

		char		*m_hstrModel;	// Name of the beam model to use
		char		*m_hstrTexture;	// Name of the texture to use

		DFLOAT		m_fStartTime;	// When did we get a trigger message
		DBOOL		m_bFirstUpdate;	// Is it the first update
};

#endif // __LASERBEAM_FX_H__