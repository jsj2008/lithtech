//**************************************************************************//
//*****		File:		SurfaceFragmentFX.h		****************************//
//*****		Author:		Andy Mattingly			****************************//
//*****		Updated:	6-18-98					****************************//
//**************************************************************************//

#ifndef	_SURFACEFRAGMENTFX_H_
#define	_SURFACEFRAGMENTFX_H_

//**************************************************************************//

#include "SpecialFX.h"

//**************************************************************************//

#define		SURFFRAG_WOOD		0
#define		SURFFRAG_STONE		1
#define		SURFFRAG_FIRE		2

//**************************************************************************//

struct SURFFRAGCREATESTRUCT : public SFXCREATESTRUCT
{
	SURFFRAGCREATESTRUCT::SURFFRAGCREATESTRUCT();

	DVector		vNormal;		// Normal of the wall where impact occured
	DFLOAT		fOffset;		// Offset from the wall
	DVector		vPos;			// Initial position of the sprites
	DVector		vVel;			// Initial velocity of the sprites
	DVector		vDecel;			// Decelleration of the velocity
	DVector		vGravity;		// Gravity to be applyed
	DFLOAT		fDuration;		// How long to display the sprite

	DVector		vScale;			// Scale of the sprite
	DRotation	rRotation;		// Rotation of the sprite
	DBOOL		nType;			// 0 = Wood, 1 = Stone, 2 = Fire, etc, etc,

	DBOOL		bFade;			// Fade the sprite?
	DBOOL		bRotate;		// Should the sprite randomly rotate?
	DBOOL		bMove;			// Move the sprite with its vel, decel, and gravity?
};

//**************************************************************************//

inline SURFFRAGCREATESTRUCT::SURFFRAGCREATESTRUCT()
{
	memset(this, 0, sizeof(SURFFRAGCREATESTRUCT));
}

//**************************************************************************//

class CSurfaceFragmentFX : public CSpecialFX
{
	public :

		CSurfaceFragmentFX();
		virtual ~CSurfaceFragmentFX();

		virtual DBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual DBOOL CreateObject(CClientDE* pClientDE);
		virtual DBOOL Update();

	private :

		DBOOL	m_bFirstUpdate;
		DBOOL	m_bFade;
		DBOOL	m_bRotate;
		DBOOL	m_bMove;

		float	red, green, blue, alpha;

		DVector		m_vNormal;			// Normal of the wall where impact occured
		DFLOAT		m_fOffset;			// Offset from the wall
		DVector		m_vPos;				// Normal of the surface we impacted on or direction
		DVector		m_vVel;				// Initial velocity of the sprite
		DVector		m_vDecel;			// Decelleration of the velocity
		DVector		m_vGravity;			// Gravity to apply to the sprite

		DVector		m_vScale;			// Scale of the sprite
		DRotation	m_rRotation;		// Rotation of the sprite
		DFLOAT		m_fPitchVel;		// Rotation variables
		DFLOAT		m_fYawVel;
		DFLOAT		m_fPitch;
		DFLOAT		m_fYaw;

		DFLOAT		m_fDuration;		// how long should we stay alive?
		DFLOAT		m_fStartTime;		// when were we created?

		DBOOL		m_nType;			// 0 for 64x64 or 1 for 128x128
};

//**************************************************************************//

#endif