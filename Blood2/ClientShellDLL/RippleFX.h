//**************************************************************************//
//*****		File:		RippleFX.h				****************************//
//*****		Author:		Andy Mattingly			****************************//
//*****		Updated:	6-27-98					****************************//
//**************************************************************************//

#ifndef	_RIPPLEFX_H_
#define	_RIPPLEFX_H_

//**************************************************************************//

#include "SpecialFX.h"

//**************************************************************************//

struct RIPPLECREATESTRUCT : public SFXCREATESTRUCT
{
	RIPPLECREATESTRUCT::RIPPLECREATESTRUCT();

	DVector		vNormal;		// Normal of the wall where impact occured
	DFLOAT		fOffset;		// Offset from the wall
	DVector		vPos;			// Initial position of the sprite
	DFLOAT		fDuration;		// How long to display the sprite
	DFLOAT		fDelay;			// Amount of time to wait before starting the FX

	DBOOL		bUpdateScale;	// Scale the sprite?
	DVector		vMinScale;		// Smallest scale to start at
	DVector		vMaxScale;		// Largest scale to go to
	DFLOAT		fInitAlpha;		// The initial alpha value to fade from

	DBOOL		bFade;			// Fade the sprite?
	char		*pSpriteFile;	// Sprite to use for the ripple or shockwave
};

//**************************************************************************//

inline RIPPLECREATESTRUCT::RIPPLECREATESTRUCT()
{
	memset(this, 0, sizeof(RIPPLECREATESTRUCT));
}

//**************************************************************************//

class CRippleFX : public CSpecialFX
{
	public :

		CRippleFX();
		virtual ~CRippleFX();

		virtual DBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual DBOOL CreateObject(CClientDE* pClientDE);
		virtual DBOOL Update();

	private :

		DBOOL	m_bFirstUpdate;
		DBOOL	m_bFade;

		float	red, green, blue, alpha;

		DVector	m_vNormal;			// Normal of the wall where impact occured
		DFLOAT	m_fOffset;			// Offset from the wall
		DVector	m_vPos;				// Normal of the surface we impacted on or direction

		DFLOAT	m_fDuration;		// how long should we stay alive?
		DFLOAT	m_fDelay;			// Amount of time to wait before starting the FX
		DFLOAT	m_fStartDelay;		// When the delay time started
		DFLOAT	m_fStartTime;		// when were we created?

		DBOOL	m_bUpdateScale;		// should we update the scaling of the sprite?
		DVector	m_vScale;			// current scale value
		DVector	m_vMinScale;		// where scaling starts
		DVector	m_vMaxScale;		// where scaling stops
		DFLOAT	m_fInitAlpha;		// The initial alpha value to fade from

		char	*m_pSpriteFile;
};

//**************************************************************************//

#endif