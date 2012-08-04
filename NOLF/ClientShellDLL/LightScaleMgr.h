#ifndef __LIGHTSCALEMGR_H
#define __LIGHTSCALEMGR_H

#include "iltclient.h"

#include "BankedList.h"

enum LightEffectType
{
	LightEffectWorld = 0,
	LightEffectEnvironment,
	LightEffectPowerup,
	LightEffectDamage,
	LightEffectInterface
};

struct LS_EFFECT
{
	LS_EFFECT()			{ nRed = 0.0f; nGreen = 0.0f; nBlue = 0.0f; eType = LightEffectWorld; }

    LTFLOAT              nRed;
    LTFLOAT              nGreen;
    LTFLOAT              nBlue;
	LightEffectType		eType;

	LS_EFFECT*			pNext;
};

class CLightScaleMgr
{
public:

    CLightScaleMgr()    { m_pEffects = LTNULL; m_TimeOfDayScale.Init(); }
	~CLightScaleMgr()	{ Term(); }

    LTBOOL       Init();
	void		Term();

    LTVector     GetTimeOfDayScale() {return m_TimeOfDayScale;}
    void        SetTimeOfDayScale(LTVector &scale);

    void        SetLightScale (LTVector* pVec, LightEffectType eType)        { SetLightScale (pVec->x, pVec->y, pVec->z, eType); }
    void        SetLightScale (LTFLOAT nRed, LTFLOAT nGreen, LTFLOAT nBlue, LightEffectType eType);

    void        ClearLightScale (LTVector* pVec, LightEffectType eType)      { ClearLightScale (pVec->x, pVec->y, pVec->z, eType); }
    void        ClearLightScale (LTFLOAT nRed, LTFLOAT nGreen, LTFLOAT nBlue, LightEffectType eType);

    void        ClearAllLightScales ();

    void        Disable()   { LTVector vec(1, 1, 1); g_pLTClient->SetGlobalLightScale(&vec); }
	void		Enable()	{ SetLightScale(); }

protected:

	void		SetLightScale();

	static CBankedList<LS_EFFECT> *GetLSEffectBank();

protected:

	LS_EFFECT*	m_pEffects;
    LTVector     m_TimeOfDayScale;
};

#endif