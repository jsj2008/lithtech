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
#include "DebrisMgr.h"
#include "BankedList.h"

#define MAX_DEBRIS 20

struct DEBRISCREATESTRUCT : public SFXCREATESTRUCT
{
    DEBRISCREATESTRUCT();

    LTRotation	rRot;
    LTVector	vPos;
    LTVector	vMinVel;
    LTVector	vMaxVel;
    LTVector	vMinDOffset;
    LTVector	vMaxDOffset;
	LTVector	vMinWorldVel;
	LTVector	vMaxWorldVel;
    LTFLOAT     fMinLifeTime;
    LTFLOAT     fMaxLifeTime;
    LTFLOAT     fFadeTime;
    uint8		nNumDebris;
    LTBOOL      bRotate;
    LTBOOL      bBounce;
    uint8       nDebrisId;
    uint8       nMinBounce;
    uint8       nMaxBounce;
    LTFLOAT     fMinScale;
    LTFLOAT     fMaxScale;
    LTFLOAT     fGravityScale;
    LTFLOAT     fAlpha;
    LTBOOL      bPlayBounceSound;
    LTBOOL      bPlayExplodeSound;
    LTBOOL      bForceRemove;
    LTBOOL      bDirOffsetOnly;
	char		szWorldSpaceFX[128];
	char		szImpactSpaceFX[128];
};

inline DEBRISCREATESTRUCT::DEBRISCREATESTRUCT()
{
	rRot.Init();
	vPos.Init();
	vMinVel.Init();
	vMaxVel.Init();
	vMinDOffset.Init();
	vMaxDOffset.Init();
	vMinWorldVel.Init();
	vMaxWorldVel.Init();
	fMinLifeTime		= 0.0f;
	fMaxLifeTime		= 0.0f;
	fFadeTime			= 0.0f;
	nNumDebris			= 0;
    bRotate             = LTFALSE;
	bBounce				= LTTRUE;
	nDebrisId			= DEBRISMGR_INVALID_ID;
	nMinBounce			= 0;
	nMaxBounce			= 0;
	fMinScale			= 0.0f;
	fMaxScale			= 0.0f;
	fAlpha				= 1.0f;
	fGravityScale		= 1.0f;
    bPlayBounceSound    = LTTRUE;
    bPlayExplodeSound   = LTTRUE;
    bForceRemove        = LTFALSE;
	szWorldSpaceFX[0]	= '\0';
	szImpactSpaceFX[0]	= '\0';
}

class CDebrisFX : public CSpecialFX
{
	public :

		CDebrisFX();

		~CDebrisFX();

        virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
        virtual LTBOOL CreateObject(ILTClient* pClientDE);
        virtual LTBOOL Update();

		virtual uint32 GetSFXID() { return SFX_DEBRIS_ID; }

	protected :

		DEBRISCREATESTRUCT	m_ds;

        LTFLOAT  m_fStartTime;   // When did we start this crazy thing
        LTFLOAT  m_fLastTime;    // Last time we created some particles
        LTBOOL   m_bFirstUpdate; // First update

		// Accessors for the old arrays..
		LTFLOAT	GetDebrisLife(int i);
		MovingObject *GetEmitter(int i);

        virtual LTBOOL		UpdateEmitter(MovingObject* pObject, 
			LTBOOL & bRemove, LTBOOL & bBounceOnGround);

        virtual LTBOOL      IsValidDebris(int i);
        virtual void        CreateDebris(int i, LTVector vPos);
        virtual LTBOOL      OkToRemoveDebris(int i);
		virtual void		RemoveDebris(int i);
		virtual void		RotateDebrisToRest(int i);
        virtual void        SetDebrisPos(int i, const LTVector &vPos);
        virtual LTBOOL      GetDebrisPos(int i, LTVector &vPos);
        virtual void        SetDebrisRot(int i, const LTRotation &rRot);

	private:
		// Internal representation of the debris arrays

		struct DebrisTracker
		{
			DebrisTracker();
			~DebrisTracker();

			MovingObject m_Emitter;  // Debris emitter
			LTBOOL m_bActiveEmitter; // Active?
			uint32 m_BounceCount;    // Number of bounces
			HOBJECT m_hDebris;
			LTFLOAT m_fDebrisLife;

			// Emitter rotation tracking
			LTFLOAT m_fPitch, m_fYaw, m_fRoll;
			LTFLOAT m_fPitchVel, m_fYawVel, m_fRollVel;
		};

		static CBankedList<DebrisTracker> *GetDebrisBank();

		void ClearDebrisList();

		DebrisTracker *m_DebrisList[MAX_DEBRIS];
};

#endif // __DEBRIS_FX_H__