//**************************************************************************//
//*****		File:		SmokeImpactFX.h			****************************//
//*****		Author:		Andy Mattingly			****************************//
//*****		Updated:	6-17-98					****************************//
//**************************************************************************//

#ifndef	_SMOKEIMPACTFX_H_
#define	_SMOKEIMPACTFX_H_

//**************************************************************************//

#include "SpecialFX.h"

//**************************************************************************//

struct SMOKECREATESTRUCT : public SFXCREATESTRUCT
{
	SMOKECREATESTRUCT::SMOKECREATESTRUCT();

	DVector		vNormal;		// Normal of the wall where impact occured
	DFLOAT		fOffset;		// Offset from the wall
	DVector		vPos;			// Initial position of the sprite
	DVector		vVel;			// Initial velocity of the sprite
	DVector		vDecel;			// Decelleration of the velocity
	DVector		vGravity;		// Gravity to be applyed
	DFLOAT		fDuration;		// How long to display the sprite
	DFLOAT		fDelay;			// Amount of time to wait before starting the FX

	DBOOL		bUpdateScale;	// Scale the sprite?
	DVector		vMinScale;		// Smallest scale to start at
	DVector		vMaxScale;		// Largest scale to go to
	DFLOAT		fInitAlpha;		// The initial alpha value to fade from

	DVector		vColor;			// Color of the smoke
	char		*pSpriteFile;	// File to use for the smoke sprite

	DBOOL		bFade;			// Fade the sprite?
	DBOOL		bRotate;		// Choose a rotating sprite instead of a still sprite?
	DBOOL		bMove;			// Move the sprite with its vel, decel, and gravity?
};

//**************************************************************************//

inline SMOKECREATESTRUCT::SMOKECREATESTRUCT()
{
	memset(this, 0, sizeof(SMOKECREATESTRUCT));
}

//**************************************************************************//

class CSmokeImpactFX : public CSpecialFX
{
	public :

		CSmokeImpactFX();
		virtual ~CSmokeImpactFX();

		virtual DBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual DBOOL CreateObject(CClientDE* pClientDE);
		virtual DBOOL Update();

	private :

		DBOOL	m_bFirstUpdate;
		DBOOL	m_bFade;
		DBOOL	m_bRotate;
		DBOOL	m_bMove;

		float	red, green, blue, alpha;

		DVector	m_vNormal;			// Normal of the wall where impact occured
		DFLOAT	m_fOffset;			// Offset from the wall
		DVector	m_vPos;				// Normal of the surface we impacted on or direction
		DVector	m_vVel;				// Initial velocity of the sprite
		DVector	m_vDecel;			// Decelleration of the velocity
		DVector m_vGravity;			// Gravity to apply to the sprite

		DFLOAT	m_fDuration;		// how long should we stay alive?
		DFLOAT	m_fDelay;			// Amount of time to wait before starting the FX
		DFLOAT	m_fStartDelay;		// When the delay time started
		DFLOAT	m_fStartTime;		// when were we created?

		DVector m_vColor;			// Color of the smoke

		char	*m_pSpriteFile;		// File to use for the sprite
		DBOOL	m_bUpdateScale;		// should we update the scaling of the sprite?
		DVector	m_vScale;			// current scale value
		DVector	m_vMinScale;		// where scaling starts
		DVector	m_vMaxScale;		// where scaling stops
		DFLOAT	m_fInitAlpha;		// The initial alpha value to fade from
};

//**************************************************************************//

#endif