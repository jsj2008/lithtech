// ----------------------------------------------------------------------- //
//
// MODULE  : DebrisFX.h
//
// PURPOSE : Debris - Definition
//
// CREATED : 5/31/98
//
// ----------------------------------------------------------------------- //

#ifndef __DEBRIS_FX_H__
#define __DEBRIS_FX_H__

#include "SpecialFX.h"
#include "client_physics.h"
#include "DebrisTypes.h"

#define MAX_DEBRIS 20

struct DEBRISCREATESTRUCT : public SFXCREATESTRUCT
{
	DEBRISCREATESTRUCT::DEBRISCREATESTRUCT();

	LTRotation	rRot;
	LTVector		vPos;
	LTVector		vMinVel;
	LTVector		vMaxVel;
	LTFLOAT		fLifeTime;
	LTFLOAT		fFadeTime;
	uint8		nNumDebris;
	uint8		nDebrisFlags;
	LTBOOL		bRotate;
	uint8		nDebrisType;
	LTFLOAT		fMinScale;
	LTFLOAT		fMaxScale;
	LTBOOL		bPlayBounceSound;
	LTBOOL		bPlayExplodeSound;
	LTBOOL		bForceRemove;
};

inline DEBRISCREATESTRUCT::DEBRISCREATESTRUCT()
{
	memset(this, 0, sizeof(DEBRISCREATESTRUCT));
	rRot.Init();
}

class CDebrisFX : public CSpecialFX
{
	public :

		CDebrisFX() : CSpecialFX() 
		{
			VEC_INIT(m_vMinVel);
			VEC_INIT(m_vMaxVel);

			m_fLifeTime		= 0.0f;
			m_fFadeTime		= 0.0f;

			m_bFirstUpdate	= LTTRUE;
			m_fLastTime		= -1.0f;
			m_fStartTime	= -1.0f;

			memset(m_Emmitters, 0, sizeof(MovingObject)*MAX_DEBRIS);
			memset(m_ActiveEmmitters, 0, sizeof(LTBOOL)*MAX_DEBRIS);
			memset(m_BounceCount, 0, sizeof(uint8)*MAX_DEBRIS);
			memset(m_hDebris, 0, sizeof(HOBJECT)*MAX_DEBRIS);
			memset(m_fPitch, 0, sizeof(LTFLOAT)*MAX_DEBRIS);
			memset(m_fYaw, 0, sizeof(LTFLOAT)*MAX_DEBRIS);
			memset(m_fRoll, 0, sizeof(LTFLOAT)*MAX_DEBRIS);
			memset(m_fPitchVel, 0, sizeof(LTFLOAT)*MAX_DEBRIS);
			memset(m_fYawVel, 0, sizeof(LTFLOAT)*MAX_DEBRIS);
			memset(m_fRollVel, 0, sizeof(LTFLOAT)*MAX_DEBRIS);
			memset(m_fDebrisLife, 0, sizeof(LTFLOAT)*MAX_DEBRIS);

			m_nNumDebris		= 0;
			m_nDebrisFlags		= 0;
			m_eDebrisType		= DBT_GENERIC;

			m_bRotate			= LTFALSE;
			m_fMinScale			= 1.0f;
			m_fMaxScale			= 1.0f;
			m_bForceRemove		= LTFALSE;

			m_bPlayBounceSound	= LTFALSE;
			m_bPlayExplodeSound	= LTFALSE;
		}

		~CDebrisFX()
		{
			for (int i=0; i < m_nNumDebris; i++)
			{
				if (m_hDebris[i] && m_pClientDE)
				{
					m_pClientDE->RemoveObject(m_hDebris[i]);
				}
			}
		}

		virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual LTBOOL CreateObject(ILTClient* pClientDE);
		virtual LTBOOL Update();

	private :

		LTRotation m_rRot;		// Direction of velocities
		LTVector	m_vPos;			// Where debris starts

		LTVector	m_vLastPos;		// Last Particle particle position
		LTVector	m_vMinVel;		// Minimum emmitter velocity
		LTVector	m_vMaxVel;		// Maximum emmitter velocity

		LTFLOAT	m_fFadeTime;	// When system should start to fade
		LTFLOAT	m_fLifeTime;	// How long system stays around
		LTFLOAT	m_fStartTime;	// When did we start this crazy thing

		LTFLOAT	m_fLastTime;	// Last time we created some particles
		LTBOOL	m_bFirstUpdate;	// First update

		MovingObject	m_Emmitters[MAX_DEBRIS];			// Debris emmitters
		uint8			m_nNumDebris;						// Num in array
		uint8			m_nDebrisFlags;						// MoveObject flags
		LTBOOL			m_ActiveEmmitters[MAX_DEBRIS];		// Active?	
		uint8			m_BounceCount[MAX_DEBRIS];			// Number of bounces
		HLOCALOBJ		m_hDebris[MAX_DEBRIS];
		LTFLOAT			m_fDebrisLife[MAX_DEBRIS];

		DebrisType		m_eDebrisType;		// Debris type
		LTFLOAT			m_fMinScale;		// Min model scale
		LTFLOAT			m_fMaxScale;		// Max model scale
	
		LTBOOL			m_bPlayBounceSound;	 // Play a sound when we bounce
		LTBOOL			m_bPlayExplodeSound; // Play a sound when we explode

		LTBOOL			m_bForceRemove;

		// Emmitter rotation stuff...
		
		LTBOOL			m_bRotate;
		LTFLOAT			m_fPitch[MAX_DEBRIS];		
		LTFLOAT			m_fYaw[MAX_DEBRIS];
		LTFLOAT			m_fRoll[MAX_DEBRIS];
		LTFLOAT			m_fPitchVel[MAX_DEBRIS];
		LTFLOAT			m_fYawVel[MAX_DEBRIS];
		LTFLOAT			m_fRollVel[MAX_DEBRIS];

		LTBOOL		UpdateEmmitter(MovingObject* pObject);
		HLOCALOBJ	CreateDebris();
		LTBOOL		OkToRemoveDebris(HLOCALOBJ hDebris);
};

#endif // __DEBRIS_FX_H__