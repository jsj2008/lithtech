// ----------------------------------------------------------------------- //
//
// MODULE  : GibFX.h
//
// PURPOSE : Gib - Definition
//
// CREATED : 6/15/98
//
// ----------------------------------------------------------------------- //

#ifndef __GIB_FX_H__
#define __GIB_FX_H__

#include "SpecialFX.h"
#include "client_physics.h"
#include "ModelButeMgr.h"
#include "GibTypes.h"
#include "ContainerCodes.h"

#define MAX_GIB 20

struct GIBCREATESTRUCT : public SFXCREATESTRUCT
{
    GIBCREATESTRUCT();

    LTRotation   rRot;
    LTVector     vPos;
    LTVector     vMinVel;
    LTVector     vMaxVel;
    LTFLOAT      fLifeTime;
    LTFLOAT      fFadeTime;
    uint8       nGibFlags;
    uint8       nCode;
    LTBOOL       bRotate;
    uint8       nSize;
	ModelId		eModelId;
	ModelStyle	eModelStyle;
    LTBOOL       bSubGibs;
    LTBOOL       bBloodSplats;
    uint8       nNumGibs;
	GibType		eGibTypes[MAX_GIB];
};

inline GIBCREATESTRUCT::GIBCREATESTRUCT()
{
    rRot.Init();
	vPos.Init();
	vMinVel.Init();
	vMaxVel.Init();
	fLifeTime		= 0.0f;
	fFadeTime		= 0.0f;
	nGibFlags		= 0;
	nCode			= 0;
	nSize			= 0;
	nNumGibs		= 0;
	eModelId		= eModelIdInvalid;
	eModelStyle		= eModelStyleInvalid;
    bSubGibs        = LTFALSE;
    bBloodSplats    = LTFALSE;
    bRotate         = LTFALSE;

	for (int i=0; i < MAX_GIB; i++)
	{
		eGibTypes[i] = GT_FIRST;
	}
}

class CGibFX : public CSpecialFX
{
	public :

		CGibFX() : CSpecialFX()
		{
			VEC_INIT(m_vMinVel);
			VEC_INIT(m_vMaxVel);

			m_fLifeTime		= 0.0f;
			m_fFadeTime		= 0.0f;

            m_bFirstUpdate  = LTTRUE;
			m_fLastTime		= -1.0f;
			m_fStartTime	= -1.0f;

			memset(m_Emitters, 0, sizeof(MovingObject)*MAX_GIB);
            memset(m_ActiveEmitters, 0, sizeof(LTBOOL)*MAX_GIB);
            memset(m_BounceCount, 0, sizeof(uint8)*MAX_GIB);
			memset(m_hGib, 0, sizeof(HOBJECT)*MAX_GIB);
			memset(m_pGibTrail, 0, sizeof(CSpecialFX*)*MAX_GIB);
			memset(m_eGibTypes, 0, sizeof(GibType)*MAX_GIB);
            memset(m_fGibLife, 0, sizeof(LTFLOAT)*MAX_GIB);
			m_nNumGibs		= 0;
			m_nGibFlags		= 0;
            m_bSubGibs      = LTFALSE;
            m_bBloodSplats  = LTFALSE;

			m_nNumRandomGibs = 2;

			m_eCode				= CC_NO_CONTAINER;

			m_eModelId			= eModelIdInvalid;
			m_eModelStyle		= eModelStyleInvalid;

            m_bCurGibOnGround   = LTFALSE;
            m_bPlayBounceSound  = LTTRUE;

            m_bRotate           = LTFALSE;
			m_fPitch			= 0.0f;
			m_fYaw				= 0.0f;
			m_fPitchVel			= 0.0f;
			m_fYawVel			= 0.0f;
		}

		~CGibFX()
		{
			RemoveAllFX();
		}

        virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
        virtual LTBOOL CreateObject(ILTClient* pClientDE);
        virtual LTBOOL Update();

		virtual uint32 GetSFXID() { return SFX_GIB_ID; }

	private :

        LTRotation m_rRot;       // Direction of velocities
        LTVector m_vPos;         // Where Gib starts

        LTVector m_vMinVel;      // Minimum emitter velocity
        LTVector m_vMaxVel;      // Maximum emitter velocity

        LTFLOAT  m_fFadeTime;    // When system should start to fade
        LTFLOAT  m_fLifeTime;    // How long system stays around
        LTFLOAT  m_fStartTime;   // When did we start this crazy thing

        LTFLOAT  m_fLastTime;    // Last time we created some particles
        LTBOOL   m_bFirstUpdate; // First update

		ContainerCode	m_eCode;			// Container code

		ModelId			m_eModelId;			// Model
		ModelStyle		m_eModelStyle;		// Style of model
		ModelType		m_eModelType;		// Type of model gibbed

		MovingObject	m_Emitters[MAX_GIB];			// Gib Emitters
        uint8           m_nNumGibs;                     // Num in array
        uint8           m_nGibFlags;                    // MoveObject flags
        LTBOOL           m_ActiveEmitters[MAX_GIB];      // Active?
        uint8           m_BounceCount[MAX_GIB];         // Number of bounces
		HLOCALOBJ		m_hGib[MAX_GIB];				// Gib models
		CSpecialFX*		m_pGibTrail[MAX_GIB];			// Blood trails
		GibType			m_eGibTypes[MAX_GIB];			// Types of gibs
        LTFLOAT          m_fGibLife[MAX_GIB];            // Life time of the gib
        uint8           m_nNumRandomGibs;               // Num random gibs

        LTBOOL           m_bCurGibOnGround;
        LTBOOL           m_bPlayBounceSound;
        LTBOOL           m_bSubGibs;
        LTBOOL           m_bBloodSplats;

		// emitter rotation stuff...

        LTBOOL           m_bRotate;
        LTFLOAT          m_fPitch;
        LTFLOAT          m_fYaw;
        LTFLOAT          m_fRoll;
        LTFLOAT          m_fPitchVel;
        LTFLOAT          m_fYawVel;
        LTFLOAT          m_fRollVel;

		ClientIntersectInfo m_info;  // Last bounce info

        LTBOOL       UpdateEmitter(MovingObject* pObject);
        void        UpdateGib(int nIndex, LTBOOL bBounced);
		HLOCALOBJ	CreateGib(GibType eType);
		HLOCALOBJ	CreateRandomGib();
		CSpecialFX*	CreateGibTrail(HLOCALOBJ hObj);

		void		CreateBloodSpray();
		void		CreateMiniBloodExplosion(int nIndex);
		void		CreateLingeringSmoke(int nIndex);
		void		HandleBounce(int nIndex);
		void		HandleDoneBouncing(int nIndex);
		char*		GetBounceSound();
		char*		GetGibDieSound();

		void		RemoveAllFX();
        LTBOOL       OkToRemoveGib(HLOCALOBJ hGib);
};

#endif // __GIB_FX_H__