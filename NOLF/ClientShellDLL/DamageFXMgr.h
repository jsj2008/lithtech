// ----------------------------------------------------------------------- //
//
// MODULE  : DamageFXMgr.h
//
// PURPOSE : Damage FX Manager class - Definition
//
// CREATED : 1/20/00
//
// (c) 2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __DAMAGE_FX_MGR_H__
#define __DAMAGE_FX_MGR_H__

#include "GameButeMgr.h"
#include "sprinklesfx.h"

#define DM_DEFAULT_FILE "Attributes\\DamageFX.txt"

class CDamageFXMgr : public CGameButeMgr
{
public:
	CDamageFXMgr();
	~CDamageFXMgr();

    LTBOOL       Init(ILTCSBase *pInterface, const char* szAttributeFile=DM_DEFAULT_FILE);

    LTBOOL       WriteFile() { return m_buteMgr.Save(); }
	void		Reload()    { m_buteMgr.Parse(m_strAttributeFile); }


	void Update();
	void Clear();

	void StartBleedingFX();
    void StopBleedingFX(LTBOOL bFade = LTTRUE);

	void StartPoisonFX();
    void StopPoisonFX(LTBOOL bFade = LTTRUE);

	void StartStunFX();
    void StopStunFX(LTBOOL bFade = LTTRUE);

	void StartSleepingFX();
    void StopSleepingFX(LTBOOL bFade = LTTRUE);

	void StartBurnFX();
    void StopBurnFX(LTBOOL bFade = LTTRUE);

	void StartChokeFX();
    void StopChokeFX(LTBOOL bFade = LTTRUE);

	void StartElectrocuteFX();
    void StopElectrocuteFX(LTBOOL bFade = LTTRUE);

    LTBOOL IsBleeding()  {return m_bBleeding || m_bBleedingFade; }
    LTBOOL IsPoisoned()  {return m_bPoison || m_bPoisonFade; }
    LTBOOL IsStunned()   {return m_bStun || m_bStunFade; }
    LTBOOL IsSleeping()  {return m_bSleeping || m_bSleepingFade; }
    LTBOOL IsBurning()   {return m_bBurn || m_bBurnFade; }
    LTBOOL IsChoking()   {return m_bChoke || m_bChokeFade; }
    LTBOOL IsElectrocuted()  {return m_bElectrocute || m_bElectrocuteFade; }

protected:
    LTBOOL       GetBool(char *pTag,char *pAttribute);
    LTFLOAT      GetFloat(char *pTag,char *pAttribute);
	int			GetInt(char *pTag,char *pAttribute);
    LTIntPt      GetPoint(char *pTag,char *pAttribute);
    uint32      GetDWord(char *pTag,char *pAttribute);
	void		GetString(char *pTag,char *pAttribute, char *pBuf, int nBufLen);
    LTVector     GetVector(char *pTag,char *pAttribute);

private:
	void UpdateBleedingFX();
	void UpdatePoisonFX();
	void UpdateStunFX();
	void UpdateSleepingFX();
	void UpdateBurnFX();
	void UpdateChokeFX();
	void UpdateElectrocuteFX();

	void CreateSprinkles();
	void DestroySprinkles();

private:
    LTBOOL   m_bBleeding;
    LTBOOL   m_bPoison;
    LTBOOL   m_bStun;
    LTBOOL   m_bSleeping;
    LTBOOL   m_bBurn;
    LTBOOL   m_bChoke;
    LTBOOL   m_bElectrocute;
    LTBOOL   m_bBleedingFade;
    LTBOOL   m_bPoisonFade;
    LTBOOL   m_bStunFade;
    LTBOOL   m_bSleepingFade;
    LTBOOL   m_bBurnFade;
    LTBOOL   m_bChokeFade;
    LTBOOL   m_bElectrocuteFade;

    LTVector m_vPoisonColor;
    LTVector m_vStunColor;
    LTVector m_vBurnColor;
    LTVector m_vElectrocuteColor;

    LTFLOAT  m_fFOVXOffset;
    LTFLOAT  m_fFOVYOffset;
    LTFLOAT  m_fFOVXDir;
    LTFLOAT  m_fFOVYDir;
    LTFLOAT  m_fColorRDir;
    LTFLOAT  m_fColorGDir;
    LTFLOAT  m_fColorBDir;
    LTFLOAT  m_fRotDir;
    LTFLOAT  m_fOffsetRot;
    LTFLOAT  m_fMaxRot;
    LTFLOAT  m_fMinRot;
    LTFLOAT  m_fMoveMult;
    LTFLOAT  m_fStunDir;

    HLTSOUND    m_hBleedingSound;
    HLTSOUND    m_hPoisonSound;
    HLTSOUND    m_hStunSound;
    HLTSOUND    m_hSleepingSound;
    HLTSOUND    m_hBurnSound;
    HLTSOUND    m_hChokeSound;
    HLTSOUND    m_hElectrocuteSound;

    uint8           m_nNumSprinkles;
	SprinklesFX*	m_pSprinkles;


};

#endif //__DAMAGE_FX_MGR_H__