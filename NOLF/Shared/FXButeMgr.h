// ----------------------------------------------------------------------- //
//
// MODULE  : FXButeMgr.h
//
// PURPOSE : FXButeMgr definition - Controls attributes of all weapons
//
// CREATED : 12/09/99
//
// (c) 1999-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __FX_BUTE_MGR_H__
#define __FX_BUTE_MGR_H__

#include "GameButeMgr.h"
#include "ltbasetypes.h"
#include "FXStructs.h"
#include "SurfaceDefs.h"
#include "ContainerCodes.h"
#include "DebrisMgr.h"

#ifdef  _CLIENTBUILD
#include "..\ClientShellDLL\SpecialFX.h"
#include "..\ClientShellDLL\BaseScaleFX.h"
#include "..\ClientShellDLL\DynamicLightFX.h"
#include "..\ClientShellDLL\SoundFX.h"
#endif

class CFXButeMgr;
extern CFXButeMgr* g_pFXButeMgr;

#define FXBMGR_DEFAULT_FILE			"Attributes\\Fx.txt"

#define FXBMGR_INVALID_ID			255

#define FXBMGR_MAX_FILE_PATH		64
#define FXBMGR_MAX_NAME_LENGTH		32

#define	IMPACT_MAX_DEBRISFX_TYPES	10
#define	IMPACT_MAX_SCALEFX_TYPES	25
#define	IMPACT_MAX_PEXPLFX_TYPES	5
#define	IMPACT_MAX_DLIGHTFX_TYPES	5
#define	IMPACT_MAX_PDEBRISFX_TYPES	3
#define IMPACT_MAX_PSHOWERFX		5

#define FIRE_MAX_BEAMFX				5

#define	PV_MAX_SCALEFX_TYPES	10
#define	PV_MAX_DLIGHTFX_TYPES	10
#define PV_MAX_SOUNDFX_TYPES	10

struct PROJECTILECLASSDATA
{
	PROJECTILECLASSDATA()
	{
		szName[0] = '\0';
	}

    virtual LTBOOL   Init(CButeMgr & buteMgr, char* aTagName);
	virtual void	Cache(CFXButeMgr* pFXButeMgr) {}

	char	szName[FXBMGR_MAX_NAME_LENGTH];
};

typedef CTList<PROJECTILECLASSDATA*> ProjClassDataList;

struct PROXCLASSDATA : public PROJECTILECLASSDATA
{
	PROXCLASSDATA();

    LTBOOL   Init(CButeMgr & buteMgr, char* aTagName);
	void	Cache(CFXButeMgr* pFXButeMgr);

	char	szArmSound[FXBMGR_MAX_FILE_PATH];
 	char	szActivateSound[FXBMGR_MAX_FILE_PATH];

	int		nActivateRadius;
	int		nArmSndRadius;
	int		nActivateSndRadius;

    LTFLOAT  fArmDelay;
    LTFLOAT  fActivateDelay;
};

struct PROJECTILEFX
{
	PROJECTILEFX();

    LTBOOL   Init(CButeMgr & buteMgr, char* aTagName);
	void	Cache(CFXButeMgr* pFXButeMgr);

	int			nId;

	char		szName[FXBMGR_MAX_NAME_LENGTH];
	char		szFlareSprite[FXBMGR_MAX_FILE_PATH];
	char		szSound[FXBMGR_MAX_FILE_PATH];
	char		szClass[FXBMGR_MAX_FILE_PATH];
	char		szModel[FXBMGR_MAX_FILE_PATH];
	char		szSkin[FXBMGR_MAX_FILE_PATH];

	int			nAltVelocity;
	int			nVelocity;
    LTFLOAT     fLifeTime;
	int			nFlags;
    LTVector    vLightColor;
	int			nLightRadius;
	int			nSoundRadius;
    LTFLOAT     fFlareScale;
    uint32      dwObjectFlags;
    LTVector    vModelScale;
	int			nSmokeTrailType;

	// Data specific to our class (i.e., szClass)...
	PROJECTILECLASSDATA* pClassData;
};

typedef CTList<PROJECTILEFX*> ProjectileFXList;




struct PUSHERFX;
struct IMPACTFX
{
	IMPACTFX();

    LTBOOL   Init(CButeMgr & buteMgr, char* aTagName);
	void	Cache(CFXButeMgr* pFXButeMgr);

	int			nId;

	char		szName[FXBMGR_MAX_NAME_LENGTH];
	char		szSound[FXBMGR_MAX_FILE_PATH];
	char		szMark[FXBMGR_MAX_FILE_PATH];

	int			nSoundRadius;
	int			nAISoundRadius;
	LTBOOL		bAIIgnoreSurface;
	int			nFlags;
    LTFLOAT     fMarkScale;
    LTVector    vTintColor;
    LTFLOAT     fTintRampUp;
    LTFLOAT     fTintRampDown;
    LTFLOAT     fTintMaxTime;
    LTVector    vBlastColor;
    LTFLOAT     fBlastTimeMin;
    LTFLOAT     fBlastTimeMax;
    LTFLOAT     fBlastFadeMin;
    LTFLOAT     fBlastFadeMax;
    LTBOOL      bDoSurfaceFX;
	LTBOOL		bIgnoreFlesh;
	LTBOOL		bIgnoreLiquid;

	PUSHERFX*	pPusherFX;

	int			nNumDebrisFXTypes;
	int			aDebrisFXTypes[IMPACT_MAX_DEBRISFX_TYPES];

	int			nNumScaleFXTypes;
	int			aScaleFXTypes[IMPACT_MAX_SCALEFX_TYPES];

	int			nNumPExplFX;
	int			aPExplFXTypes[IMPACT_MAX_PEXPLFX_TYPES];

	int			nNumDLightFX;
	int			aDLightFXTypes[IMPACT_MAX_DLIGHTFX_TYPES];

	int			nNumPolyDebrisFX;
	int			aPolyDebrisFXTypes[IMPACT_MAX_PDEBRISFX_TYPES];

	int			nNumPShowerFX;
	int			aPShowerFXIds[IMPACT_MAX_PSHOWERFX];
};

typedef CTList<IMPACTFX*> ImpactFXList;

// ImpactFX Create Struct

struct IFXCS
{
	IFXCS()
	{
		vPos.Init();
		vDir.Init();
		vSurfNormal.Init();
		rSurfRot.Init();
		eSurfType		= ST_UNKNOWN;
		eCode			= CC_NO_CONTAINER;
        bPlaySound      = LTFALSE;
		fBlastRadius	= 0.0f;
		fTintRange		= 0.0f;
	}

    LTVector         vPos;           // Position of FX
    LTVector         vDir;           // Direction for scale fx
    LTVector         vSurfNormal;    // Normal of surface of impact
    LTRotation       rSurfRot;       // Rotation aligned with surface normal
	SurfaceType		 eSurfType;		// Type of surface impacting on
	ContainerCode	 eCode;			// Container code fx is currently in
    LTBOOL           bPlaySound;     // Should the sound be played
    LTFLOAT          fBlastRadius;   // Radius for blast mark
    LTFLOAT          fTintRange;     // Max range for screen tints
};

struct BEAMFX;
struct FIREFX
{
	FIREFX();

    LTBOOL   Init(CButeMgr & buteMgr, char* aTagName);
	void	 Cache(CFXButeMgr* pFXButeMgr);

	int			nId;

	char		szName[FXBMGR_MAX_NAME_LENGTH];
	char		szShellModel[FXBMGR_MAX_FILE_PATH];
	char		szShellSkin[FXBMGR_MAX_FILE_PATH];

	int			nFlags;
    LTVector    vShellScale;

	int			nNumBeamFX;
	BEAMFX*		pBeamFX[FIRE_MAX_BEAMFX];
};

typedef CTList<FIREFX*> FireFXList;


struct PEXPLFX
{
	PEXPLFX();

    LTBOOL   Init(CButeMgr & buteMgr, char* aTagName);
	void	Cache(CFXButeMgr* pFXButeMgr);

	int			nId;

	char		szName[FXBMGR_MAX_NAME_LENGTH];
	char		szFile[FXBMGR_MAX_FILE_PATH];
    LTVector     vPosOffset;
	int			nNumPerPuff;
	int			nNumEmitters;
	int			nNumSteps;
    LTBOOL       bCreateDebris;
    LTBOOL       bRotateDebris;
    LTBOOL       bIgnoreWind;
    LTBOOL       bDoBubbles;
    LTBOOL       bAdditive;
    LTBOOL       bMultiply;
    LTVector     vColor1;
    LTVector     vColor2;
    LTVector     vMinVel;
    LTVector     vMaxVel;
    LTVector     vMinDriftVel;
    LTVector     vMaxDriftVel;
    LTFLOAT      fLifeTime;
    LTFLOAT      fFadeTime;
    LTFLOAT      fOffsetTime;
    LTFLOAT      fRadius;
    LTFLOAT      fGravity;
};

typedef CTList<PEXPLFX*> PExplFXList;


struct DLIGHTFX
{
	DLIGHTFX();

    LTBOOL   Init(CButeMgr & buteMgr, char* aTagName);
	void	Cache(CFXButeMgr* pFXButeMgr);

	int		nId;

	char	szName[FXBMGR_MAX_NAME_LENGTH];

    LTVector vColor;
    LTFLOAT  fMinRadius;
    LTFLOAT  fMaxRadius;
    LTFLOAT  fMinTime;
    LTFLOAT  fMaxTime;
    LTFLOAT  fRampUpTime;
    LTFLOAT  fRampDownTime;
};

typedef CTList<DLIGHTFX*> DLightFXList;


struct PVFX
{
	PVFX();

    LTBOOL   Init(CButeMgr & buteMgr, char* aTagName);
	void	Cache(CFXButeMgr* pFXButeMgr);

	int		nId;

	char	szName[FXBMGR_MAX_NAME_LENGTH];
	char	szSocket[FXBMGR_MAX_NAME_LENGTH];

	int		nNumScaleFXTypes;
	int		aScaleFXTypes[PV_MAX_SCALEFX_TYPES];

	int		nNumDLightFX;
	int		aDLightFXTypes[PV_MAX_DLIGHTFX_TYPES];

	int		nNumSoundFX;
	int		aSoundFXTypes[PV_MAX_SOUNDFX_TYPES];
};

typedef CTList<PVFX*> PVFXList;


struct CParticleMuzzleFX
{
	CParticleMuzzleFX();

    LTBOOL   Init(CButeMgr & buteMgr, char* aTagName);
	void	Cache(CFXButeMgr* pFXButeMgr);

	char	szName[FXBMGR_MAX_NAME_LENGTH];
	int		nId;

    LTVector     vColor1;
    LTVector     vColor2;
    LTFLOAT      fLength;
    LTFLOAT      fDuration;
    LTFLOAT      fRadius;
    LTFLOAT      fMaxScale;
	int			 nNumParticles;
	char		 szFile[FXBMGR_MAX_FILE_PATH];
    LTBOOL       bAdditive;
    LTBOOL       bMultiply;
};

typedef CTList<CParticleMuzzleFX*> ParticleMuzzleFXList;



struct CMuzzleFX
{
	CMuzzleFX();

    LTBOOL   Init(CButeMgr & buteMgr, char* aTagName);
	void	Cache(CFXButeMgr* pFXButeMgr);

	char	szName[FXBMGR_MAX_NAME_LENGTH];
	int		nId;

	float	fDuration;

	CParticleMuzzleFX*	pPMuzzleFX;
	CScaleFX*			pScaleFX;
	DLIGHTFX*			pDLightFX;
};

typedef CTList<CMuzzleFX*> MuzzleFXList;

struct TRACERFX
{
	TRACERFX();

    LTBOOL      Init(CButeMgr & buteMgr, char* aTagName);
	void		Cache(CFXButeMgr* pFXButeMgr);

	int			nId;

	char		szName[FXBMGR_MAX_NAME_LENGTH];
	char		szTexture[FXBMGR_MAX_FILE_PATH];

	int			nFrequency;
	float		fVelocity;
	float		fWidth;
	float		fInitialAlpha;
	float		fFinalAlpha;
    LTVector    vColor;
};

typedef CTList<TRACERFX*> TracerFXList;


struct BEAMFX
{
	BEAMFX();

    LTBOOL      Init(CButeMgr & buteMgr, char* aTagName);
	void		Cache(CFXButeMgr* pFXButeMgr);

	int			nId;

	char		szName[FXBMGR_MAX_NAME_LENGTH];
	char		szTexture[FXBMGR_MAX_FILE_PATH];

	float		fDuration;
	float		fWidth;
	float		fInitialAlpha;
	float		fFinalAlpha;
    LTVector    vColor;
    LTBOOL      bAlignUp;
    LTBOOL      bAlignFlat;
};

typedef CTList<BEAMFX*> BeamFXList;


struct SOUNDFX
{
	SOUNDFX();

    LTBOOL      Init(CButeMgr & buteMgr, char* aTagName);
	void		Cache(CFXButeMgr* pFXButeMgr);

	int			nId;

	char		szName[FXBMGR_MAX_NAME_LENGTH];
	char		szFile[FXBMGR_MAX_FILE_PATH];

	LTBOOL		bLoop;
	LTFLOAT		fRadius;
	LTFLOAT		fPitchShift;
};

typedef CTList<SOUNDFX*> SoundFXList;


struct PUSHERFX
{
	PUSHERFX();

    LTBOOL      Init(CButeMgr & buteMgr, char* aTagName);
	void		Cache(CFXButeMgr* pFXButeMgr);

	int			nId;

	char		szName[FXBMGR_MAX_NAME_LENGTH];

	LTFLOAT		fRadius;
	LTFLOAT		fStartDelay;
	LTFLOAT		fDuration;
	LTFLOAT		fStrength;
};

typedef CTList<PUSHERFX*> PusherFXList;



class CFXButeMgr : public CGameButeMgr
{
	public :

		CFXButeMgr();
		~CFXButeMgr();

        LTBOOL           Init(ILTCSBase *pInterface, const char* szAttributeFile=FXBMGR_DEFAULT_FILE);
		void			Term();

        void            Reload(ILTCSBase *pInterface);

#if defined(_CLIENTBUILD)
        CSpecialFX* CreateScaleFX(CScaleFX* pScaleFX, LTVector vPos,
            LTVector vDir, LTVector* pvSurfaceNormal, LTRotation* prRot,
            CBaseScaleFX* pFX=LTNULL);

        CSpecialFX* CreatePShowerFX(CPShowerFX* pPShowerFX, LTVector vPos,
            LTVector vDir, LTVector vSurfaceNormal);

		CSpecialFX*	CreatePolyDebrisFX(CPolyDebrisFX* pPolyDebrisFX,
            LTVector vPos, LTVector vDir, LTVector vSurfaceNormal);

        CSpecialFX* CreatePExplFX(PEXPLFX* pPExplFX, LTRotation rSurfaceRot,
            SurfaceType eSurfaceType, LTVector vPos, ContainerCode eCode);

        CSpecialFX* CreateDLightFX(DLIGHTFX* pDLightFX, LTVector vPos,
            CDynamicLightFX* pFX=LTNULL);

        CSpecialFX* CreateBeamFX(BEAMFX* pBeamFX, LTVector vStartPos,
			LTVector vEndPos);

		void CreateImpactFX(IMPACTFX* pImpactFX, IFXCS & cs);

		CSpecialFX* CreateSoundFX(SOUNDFX* pSoundFX, LTVector vPos,
			CSoundFX* pFX);

		void CreatePusherFX(PUSHERFX* pPusherFX, LTVector vPos);

#endif // _CLIENTBUILD

		PUSHERFX*		GetPusherFX(int nFXId);
		PUSHERFX*		GetPusherFX(char* pName);

		SOUNDFX*		GetSoundFX(int nSoundFXId);
		SOUNDFX*		GetSoundFX(char* pName);

		TRACERFX*		GetTracerFX(int nTracerFXId);
		TRACERFX*		GetTracerFX(char* pName);

		BEAMFX*			GetBeamFX(int nBeamFXId);
		BEAMFX*			GetBeamFX(char* pName);

		CMuzzleFX*		GetMuzzleFX(int nMuzzleFXId);
		CMuzzleFX*		GetMuzzleFX(char* pName);

		CParticleMuzzleFX*	GetParticleMuzzleFX(int nParticleMuzzleFXId);
		CParticleMuzzleFX*	GetParticleMuzzleFX(char* pName);

		PVFX*			GetPVFX(int nPVFXId);
		PVFX*			GetPVFX(char* pName);

		CScaleFX*		GetScaleFX(int nScaleFXId);
		CScaleFX*		GetScaleFX(char* pName);

		CPShowerFX*		GetPShowerFX(int nPShowerFXId);
		CPShowerFX*		GetPShowerFX(char* pName);

		CPolyDebrisFX*	GetPolyDebrisFX(int nPolyDebrisFXId);
		CPolyDebrisFX*	GetPolyDebrisFX(char* pName);

		PEXPLFX*		GetPExplFX(int nPExplFXId);
		PEXPLFX*		GetPExplFX(char* pName);

		DLIGHTFX*		GetDLightFX(int nDLightFXId);
		DLIGHTFX*		GetDLightFX(char* pName);

#ifndef _CLIENTBUILD
		int				GetNumImpactFX() const { return m_ImpactFXList.GetLength(); }
        LTBOOL           ReadImpactFXProp(char* pPropName, uint8 & nImpactFXId);
#endif // _CLIENTBUILD

		IMPACTFX*		GetImpactFX(int nImpactFXId);
		IMPACTFX*		GetImpactFX(char* pName);

		PROJECTILEFX*	GetProjectileFX(int nProjFXId);
		PROJECTILEFX*	GetProjectileFX(char* pName);

		PROJECTILECLASSDATA*	GetProjectileClassData(char* pName);

		FIREFX*			GetFireFX(int nFireFXId);
		FIREFX*			GetFireFX(char* pName);

	protected :

		ProjectileFXList		m_ProjectileFXList;	// All projectile fx types
		ProjClassDataList		m_ProjClassDataList;// All projectile class data
		ImpactFXList			m_ImpactFXList;		// All impact fx types
		FireFXList				m_FireFXList;		// All fire fx types
		ScaleFXList				m_ScaleFXList;		// All scale fx types
		PExplFXList				m_PExplFXList;		// All particle explosion fx types
		DLightFXList			m_DLightFXList;		// All dynamic light fx types
		PShowerFXList			m_PShowerFXList;	// All particle shower fx
		PolyDebrisFXList		m_PolyDebrisFXList; // All poly debris fx
		PVFXList				m_PVFXList;			// All player-view fx
		ParticleMuzzleFXList	m_PartMuzzleFXList; // All particle muzzle fx
		MuzzleFXList			m_MuzzleFXList;		// All weapon muzzle fx
		TracerFXList			m_TracerFXList;		// All weapon tracer fx
		BeamFXList				m_BeamFXList;		// All weapon beam fx
		SoundFXList				m_SoundFXList;		// All sound fx
		PusherFXList			m_PusherFXList;		// All pusher fx
};

////////////////////////////////////////////////////////////////////////////
//
// CFXButeMgrPlugin is used to help facilitate populating the DEdit object
// properties that use FXButeMgr
//
////////////////////////////////////////////////////////////////////////////
#ifndef _CLIENTBUILD

#include "iobjectplugin.h"

class CFXButeMgrPlugin : public IObjectPlugin
{
	public:

        virtual LTRESULT PreHook_EditStringList(
			const char* szRezPath,
			const char* szPropName,
			char** aszStrings,
            uint32* pcStrings,
            const uint32 cMaxStrings,
            const uint32 cMaxStringLength);

        LTBOOL PopulateStringList(char** aszStrings, uint32* pcStrings,
            const uint32 cMaxStrings, const uint32 cMaxStringLength);

	protected :

		CDebrisMgrPlugin		m_DebrisMgrPlugin;
		static CFXButeMgr		sm_FXButeMgr;
};

#endif // _CLIENTBUILD

#endif // __FX_BUTE_MGR_H__