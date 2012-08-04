// ----------------------------------------------------------------------- //
//
// MODULE  : FXButeMgr.h
//
// PURPOSE : FXButeMgr definition - Controls attributes of all weapons
//
// CREATED : 12/09/99
//
// (c) 1999-2002 Monolith Productions, Inc.  All Rights Reserved
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
#include "SpecialFX.h"
#include "BaseScaleFX.h"
#include "DynamicLightFX.h"
#include "SoundFX.h"
#endif

struct PROJECTILEFX;
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

#define	PV_MAX_SCALEFX_TYPES	10
#define	PV_MAX_DLIGHTFX_TYPES	10
#define PV_MAX_SOUNDFX_TYPES	10

struct PROJECTILECLASSDATA
{
	PROJECTILECLASSDATA()
	{
		szName[0] = '\0';
	}

	virtual ~PROJECTILECLASSDATA() {}

    virtual LTBOOL   Init(CButeMgr & buteMgr, char* aTagName);
	virtual void	Cache(CFXButeMgr* pFXButeMgr) {}

	char	szName[FXBMGR_MAX_NAME_LENGTH];
};

typedef CTList<PROJECTILECLASSDATA*> ProjClassDataList;

struct PROXCLASSDATA : public PROJECTILECLASSDATA
{
	PROXCLASSDATA();
	virtual ~PROXCLASSDATA() {}

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

struct KITTYCLASSDATA : public PROJECTILECLASSDATA
{
	KITTYCLASSDATA();
	virtual ~KITTYCLASSDATA();

	LTBOOL	Init(CButeMgr & buteMgr, char* aTagName);
	void	Cache(CFXButeMgr* pFXButeMgr);

	char	*pArmSound;
	char	*pActivateSound;
	char	*pArmedFX;
	char	*pArmedFXRed;
	char	*pArmedFXBlue;

	bool	bLoopArmSound;
	bool	bLoopArmedFX;
	
	int		nActivateRadius;
	int		nDetonateRadius;
	int		nSoundRadius;

	LTFLOAT	fArmDelay;
	LTFLOAT	fDetonateTime;
	LTFLOAT	fChaseVelocity;
};

struct BEARTRAPCLASSDATA : public PROJECTILECLASSDATA
{
	BEARTRAPCLASSDATA();
	virtual ~BEARTRAPCLASSDATA();

	LTBOOL Init( CButeMgr & buteMgr, char *aTagName );

	int		nDetonateRadius;
	LTFLOAT	fArmDelay;
};

struct BANANACLASSDATA : public PROJECTILECLASSDATA
{
	BANANACLASSDATA();
	~BANANACLASSDATA();

	LTBOOL Init( CButeMgr &buteMgr, char *aTagName );

	int		nDetonateRadius;
	float	fArmDelay;
};

struct SPEARCLASSDATA : public PROJECTILECLASSDATA
{
	SPEARCLASSDATA();
	~SPEARCLASSDATA();

	LTBOOL	Init( CButeMgr &buteMgr, char *aTagName );

	float		fStickPercent;
	bool		bCanWallStick;
	LTVector	vDimsScale;
};

struct SPAWNCLASSDATA : public PROJECTILECLASSDATA
{
	SPAWNCLASSDATA();
	virtual ~SPAWNCLASSDATA();

	LTBOOL	Init( CButeMgr &buteMgr, char *aTagName );

	char			*szSpawnObject;
	CButeListReader	blrObjectProps;
	LTBOOL			bStickToWorld;
};

struct DISCCLASSDATA : public PROJECTILECLASSDATA
{
	DISCCLASSDATA();
	virtual ~DISCCLASSDATA();

	LTBOOL	Init( CButeMgr &buteMgr, char *aTagName );

	LTFLOAT			fInitialAngleMin;
	LTFLOAT			fInitialAngleMax;
	LTFLOAT			fInitialAngleSurge;

	LTFLOAT			fReturnVelocity;
	LTFLOAT			fReturnHeightOffset;
	char			szReturnFXName[ FXBMGR_MAX_NAME_LENGTH ];

	LTFLOAT			fTurnRateMin;
	LTFLOAT			fTurnRateMax;
	LTFLOAT			fTurnRateSurge;

	LTFLOAT			fIncidentAngleToControlLineMin;
	LTFLOAT			fIncidentAngleToControlLineMax;
	LTFLOAT			fIncidentAngleToControlLineSurge;

	LTFLOAT			fIncidentAngleToControlLineDecayMin;
	LTFLOAT			fIncidentAngleToControlLineDecayMax;
	LTFLOAT			fIncidentAngleToControlLineDecaySurge;

	// player view impact effect to play for the swat block)
	// (when the player is impacted by a disc while blocking)
	char			szSwatDefendPVFXName[ FXBMGR_MAX_NAME_LENGTH ];

	// player view effect to play when the user's defense timing is
	// really really close (critical "blow")
	char			szSwatCriticalDefendPVFXName[ FXBMGR_MAX_NAME_LENGTH ];

	// number 0.0f-1.0f, when the disc hits this threshold, play the
	// critical block (1.0f == 100% == maximum defense)
	LTFLOAT			fSwatCriticalDefendThreshold;

	// player view effect to play for "persistant" blocking (the hold block)
	// (when the player is impacted by a disc while blocking)
	char			szHoldDefendPVFXName[ FXBMGR_MAX_NAME_LENGTH ];

	// player view effect to play for forarm blocking
	// (when the player is impacted by a disc while blocking)
	char			szArmDefendPVFXName[ FXBMGR_MAX_NAME_LENGTH ];

	//
	// defense stuff
	//

	// swat defense timing values
	float			fSwatDefendStartDefendPercentage;
	float			fSwatDefendEndDefendPercentage;
	float			fSwatDefendMidpoint;
	float			fSwatDefendMaxDefendPercentage;
	float			fSwatDefendStartMaxDefendPercentage;
	float			fSwatDefendEndMaxDefendPercentage;

	// hold defense timing values
	float			fHoldDefendStartDefendPercentage;
	float			fHoldDefendEndDefendPercentage;
	float			fHoldDefendMaxDefendPercentage;
	float			fHoldDefendStartMaxDefendPercentage;
	float			fHoldDefendEndMaxDefendPercentage;

	// swat defense orientation values
	float			fSwatDefendOrientationMinDefendPercentage;
	float			fSwatDefendOrientationMaxDefendPercentage;
	float			fSwatDefendOrientationDeadZone;
	float			fSwatDefendOrientationMaxZone;
	
	// swat defense orientation values
	float			fHoldDefendOrientationMinDefendPercentage;
	float			fHoldDefendOrientationMaxDefendPercentage;
	float			fHoldDefendOrientationDeadZone;
	float			fHoldDefendOrientationMaxZone;

	// (this is yucky :( forgive me)
	// This is true if the class is actually a ClusterDiscClassData
	// so that the code doesn't have to special case specific
	// weapons.  Luckily, this is set automatically.
	bool			bClusterDiscInfo;
};

struct CLUSTERDISCCLASSDATA : public DISCCLASSDATA
{
	CLUSTERDISCCLASSDATA();
	virtual ~CLUSTERDISCCLASSDATA();

	LTBOOL			Init( CButeMgr &buteMgr, char *aTagName );

	// horizontal spread
	float			fShardHorizontalSpreadMin;
	float			fShardHorizontalSpreadMax;
	float			fShardHorizontalSpreadSurge;

	// randomization of the shard direction
	float			fShardHorizontalPerturbMin;
	float			fShardHorizontalPerturbMax;
	float			fShardHorizontalPerturbSurge;

	// vertical spread
	float			fShardVerticalSpreadMin;
	float			fShardVerticalSpreadMax;
	float			fShardVerticalSpreadSurge;

	// shard projectile name
	char			szShardWeaponName[ FXBMGR_MAX_NAME_LENGTH ];

	// number of shards to emit
	int				nShardsTotalMin;
	int				nShardsTotalMax;
	int				nShardsTotalSurge;

	// number of shards perturb
	int				nShardsTotalPerturbMin;
	int				nShardsTotalPerturbMax;
	int				nShardsTotalPerturbSurge;
};

struct CLUSTERAUTOBURSTDISCCLASSDATA : public CLUSTERDISCCLASSDATA
{
	CLUSTERAUTOBURSTDISCCLASSDATA();
	virtual ~CLUSTERAUTOBURSTDISCCLASSDATA();
	
	LTBOOL			Init(CButeMgr &buteMgr,char *aTagName);
	
	float			fMinBurstDistance;
	float			fMaxBurstDistance;
};

struct PROJECTILEFX
{
	PROJECTILEFX();

	LTBOOL      Init(CButeMgr & buteMgr, char* aTagName);
	void        Cache(CFXButeMgr* pFXButeMgr);

	int         nId;

	char        szName[FXBMGR_MAX_NAME_LENGTH];
	char        szFlareSprite[FXBMGR_MAX_FILE_PATH];
	char        szSound[FXBMGR_MAX_FILE_PATH];
	char        szClass[FXBMGR_MAX_FILE_PATH];
	char        szModel[FXBMGR_MAX_FILE_PATH];
	char        szSkin[FXBMGR_MAX_FILE_PATH];

	// ClientFX effect to spawn when the projectile launches
	char        szFXName[FXBMGR_MAX_NAME_LENGTH];
	int         dwFXFlags;

	// effects to play when the disc ricochets off world geometry
	char        szRicochetFXName[ FXBMGR_MAX_NAME_LENGTH ];

	// the maximum angle, in RAIDIANS, that the disc will ricochet off
	// world geometry (this angle is specified in relation to the
	// surface, NOT the normal)
	//
	// (Note, the illustration only works with a nonproportional font)
	//    normal
	//       |     /
	//       |    / incident vector
	//       |   /
	//       |  /
	//       | /
	//       |/max ricochet angle
	//  -----+-----------------
	//
	LTFLOAT     fMaxRicochetAngle;

	// maximum number of ricochets allowed
	int         nMaxRicochets;

	int         nVelocity;
	int         nAltVelocity;
	LTFLOAT     fFireOffset;
	LTFLOAT     fLifeTime;
	LTFLOAT     fGravityOverride;
	int         nFlags;
	LTVector    vLightColor;
	int         nLightRadius;
	int         nSoundRadius;
	LTFLOAT     fFlareScale;
	uint32      dwObjectFlags;
	LTVector    vModelScale;
	int         nSmokeTrailType;

	// (boolean) true if the projectile can hit projecitle of the same kind
	int         nCanImpactSameKind;

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
	int			nAIStimulusType;
	int			nAIAlarmLevel;
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

	char		szFXName[FXBMGR_MAX_NAME_LENGTH];
};

typedef CTList<IMPACTFX*> ImpactFXList;

// ImpactFX Create Struct

struct IFXCS
{
	IFXCS()
	:	vPos			( 0.0f, 0.0f, 0.0f ),
		vDir			( 0.0f, 0.0f, 0.0f ),
		vSurfNormal		( 0.0f, 0.0f, 0.0f ),
		rSurfRot		( 0.0f, 0.0f, 0.0f, 1.0f ),
		eSurfType		( ST_UNKNOWN ),
		eCode			( CC_NO_CONTAINER ),
		bPlaySound		( LTFALSE ),
		fBlastRadius	( 0.0f ),
		fTintRange		( 0.0f )
	{
	
	}

    LTVector		vPos;           // Position of FX
    LTVector		vDir;           // Direction for scale fx
    LTVector		vSurfNormal;    // Normal of surface of impact
    LTRotation		rSurfRot;       // Rotation aligned with surface normal
	SurfaceType		eSurfType;		// Type of surface impacting on
	ContainerCode	eCode;			// Container code fx is currently in
    LTBOOL			bPlaySound;     // Should the sound be played
    LTFLOAT			fBlastRadius;   // Radius for blast mark
    LTFLOAT			fTintRange;     // Max range for screen tints
};

struct BEAMFX;
struct FIREFX
{
	FIREFX();
	~FIREFX();

    LTBOOL   Init(CButeMgr & buteMgr, char* aTagName);
	void	 Cache(CFXButeMgr* pFXButeMgr);

	int			nId;

	char		*szName;
	char		*szShellModel;
	char		*szShellSkin;
	char		*szFXName;
	char		*szBeamFXName;

	int			nFlags;
    LTVector    vShellScale;
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

struct SPRINKLEFX
{
	SPRINKLEFX();
	virtual ~SPRINKLEFX();

	LTBOOL		Init( CButeMgr &ButeMgr, char *aTagName );
	void		Cache( CFXButeMgr *pFXButeMgr );

	int			nId;

	char		*szName;
	char		*szFileName;
    char		*szSkinName;
    
	uint32		dwCount;
    float		fSpeed;
    float		fSize;
    float		fSpawnRadius;
    LTVector	vAnglesVel;
    LTVector	vColorMin;
    LTVector	vColorMax;
};

typedef CTList<SPRINKLEFX*>	SprinkleFXList;


class CFXButeMgr : public CGameButeMgr
{
	public :

		CFXButeMgr();
		~CFXButeMgr();

        LTBOOL           Init(const char* szAttributeFile=FXBMGR_DEFAULT_FILE);
		void			Term();

        void            Reload();

#if defined(_CLIENTBUILD) || defined(__PSX2)
        CSpecialFX* CreateScaleFX(CScaleFX* pScaleFX, const LTVector &vPos,
            const LTVector &vDir, const LTVector* pvSurfaceNormal, const LTRotation* prRot,
            CBaseScaleFX* pFX=LTNULL);

        CSpecialFX* CreatePShowerFX(CPShowerFX* pPShowerFX, const LTVector &vPos,
            const LTVector &vDir, const LTVector &vSurfaceNormal);

		CSpecialFX*	CreatePolyDebrisFX(CPolyDebrisFX* pPolyDebrisFX,
            const LTVector &vPos, const LTVector &vDir, const LTVector &vSurfaceNormal);

        CSpecialFX* CreatePExplFX(PEXPLFX* pPExplFX, const LTRotation &rSurfaceRot,
            SurfaceType eSurfaceType, const LTVector &vPos, ContainerCode eCode);

        CSpecialFX* CreateDLightFX(DLIGHTFX* pDLightFX, const LTVector &vPos,
            CDynamicLightFX* pFX=LTNULL);

        CSpecialFX* CreateBeamFX(BEAMFX* pBeamFX, const LTVector &vStartPos,
			const LTVector &vEndPos);

		void CreateImpactFX(IMPACTFX* pImpactFX, IFXCS & cs);

		CSpecialFX* CreateSoundFX(SOUNDFX* pSoundFX, const LTVector &vPos,
			CSoundFX* pFX);

		void CreatePusherFX(PUSHERFX* pPusherFX, const LTVector &vPos);


#endif // _CLIENTBUILD

		SPRINKLEFX*		GetSprinkleFX( int nSprinkleFXId );
		SPRINKLEFX*		GetSprinkleFX( char *pName );

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
		SprinkleFXList			m_SprinkleFXList;	// All sprinkle fx
};

////////////////////////////////////////////////////////////////////////////
//
// CFXButeMgrPlugin is used to help facilitate populating the DEdit object
// properties that use FXButeMgr
//
////////////////////////////////////////////////////////////////////////////
#ifndef _CLIENTBUILD
#ifndef __PSX2

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

#endif // !__PSX2
#endif // _CLIENTBUILD

#endif // __FX_BUTE_MGR_H__