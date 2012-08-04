// ----------------------------------------------------------------------- //
//
// MODULE  : ModelButeMgr.h
//
// PURPOSE : ModelButeMgr definition
//
// CREATED : 05.22.1999
//
// ----------------------------------------------------------------------- //

#ifndef __MODELBUTE_MGR_H__
#define __MODELBUTE_MGR_H__

// Includes

#include "GameButeMgr.h"
#include "ClientServerShared.h"

// Defines

#define NODEFLAG_NONE			(0)		// This node has no special flags
#define	NODEFLAG_CRITICALHIT	(1)		// This node is a "critical hit" node
#define	NODEFLAG_TALK			(2)		// This node is a "talk" (mouth) node
#define	NODEFLAG_TIRE			(4)		// This node is a "tire" node
#define NODEFLAG_TURNING_TIRE	(8)		// This node is a "turning tire" node
#define NODEFLAG_ROTOR			(16)	// This node is a "rotor" node (for helicopters)
#define NODEFLAG_EYE			(32)	// This node is a "eye" node (for humans)
#define NODEFLAG_WALLSTICK		(64)	// This node is a "wall stick" node (for humans)

#define MBMGR_DEFAULT_FILE "Attributes\\ModelButes.txt"

// Enums

enum ModelId
{
	eModelIdInvalid = 0xFF,
};

enum ModelSkeleton
{
	eModelSkeletonInvalid = 0xFF,
};

enum ModelNode
{
	eModelNodeInvalid = 0xFF,
};

enum ModelStyle
{
	eModelStyleInvalid = 0xFF,
	eModelStyleDefault = 0x00,
};

enum ModelType
{
	eModelTypeHuman,
	eModelTypeAnimal,
	eModelTypeVehicle,
	eModelTypeGenericProp,
	eModelTypeInvalid = 0xFF,
};

enum ModelNScript
{
	eModelNScriptInvalid = 0xFF,
};

// Classes

class CModelButeMgr;
extern CModelButeMgr* g_pModelButeMgr;

class CModelButeMgr : public CGameButeMgr
{
	public : // Public member variables

		CModelButeMgr();
		~CModelButeMgr();

        LTBOOL       Init(ILTCSBase *pInterface, const char* szAttributeFile=MBMGR_DEFAULT_FILE);
		void		Term();

		CButeMgr*	GetButeMgr() { return &m_buteMgr; }

		// Models

		int				GetNumModels() { return m_cModels; }
		ModelId			GetModelId(const char *szName);
		const char*		GetModelName(ModelId eModelId);
		const char*		GetModelSex(ModelId eModelId);
		const char*		GetModelNationality(ModelId eModelId);
		ModelType		GetModelType(ModelId eModelId);
        LTBOOL           GetModelEnvironmentMap(ModelId eModelId);
		ModelSkeleton	GetModelSkeleton(ModelId eModelId);
        LTFLOAT          GetModelMass(ModelId eModelId);
        LTFLOAT          GetModelHitPoints(ModelId eModelId);
        LTFLOAT          GetModelMaxHitPoints(ModelId eModelId);
        LTFLOAT          GetModelArmor(ModelId eModelId);
        LTFLOAT          GetModelMaxArmor(ModelId eModelId);

		// Styles

		int				GetNumStyles() { return m_cStyles; }
		ModelStyle		GetStyle(const char* szName);
		const char*		GetStyleName(ModelStyle eStyle);

		// Skeleton

		int				GetNumSkeletons() { return m_cSkeletons; }
		int				GetSkeletonNumNodes(ModelSkeleton eModelSkeleton);
		ModelNode		GetSkeletonNode(ModelSkeleton eModelSkeleton, const char* szName);
		const char*		GetSkeletonDefaultFrontDeathAni(ModelSkeleton eModelSkeleton);
		const char*		GetSkeletonDefaultBackDeathAni(ModelSkeleton eModelSkeleton);
		const char*		GetSkeletonDefaultFrontLongRecoilAni(ModelSkeleton eModelSkeleton);
		const char*		GetSkeletonDefaultBackLongRecoilAni(ModelSkeleton eModelSkeleton);
		const char*		GetSkeletonDefaultFrontShortRecoilAni(ModelSkeleton eModelSkeleton);
		const char*		GetSkeletonDefaultBackShortRecoilAni(ModelSkeleton eModelSkeleton);
		ModelNode		GetSkeletonDefaultHitNode(ModelSkeleton eModelSkeleton);
		const char*		GetSkeletonNodeName(ModelSkeleton eModelSkeleton, ModelNode eModelNode);
        uint32          GetSkeletonNodeFlags(ModelSkeleton eModelSkeleton, ModelNode eModelNode);
		const char*		GetSkeletonNodeFrontDeathAni(ModelSkeleton eModelSkeleton, ModelNode eModelNode);
		const char*		GetSkeletonNodeBackDeathAni(ModelSkeleton eModelSkeleton, ModelNode eModelNode);
		const char*		GetSkeletonNodeFrontLongRecoilAni(ModelSkeleton eModelSkeleton, ModelNode eModelNode);
		const char*		GetSkeletonNodeBackLongRecoilAni(ModelSkeleton eModelSkeleton, ModelNode eModelNode);
		const char*		GetSkeletonNodeFrontShortRecoilAni(ModelSkeleton eModelSkeleton, ModelNode eModelNode);
		const char*		GetSkeletonNodeBackShortRecoilAni(ModelSkeleton eModelSkeleton, ModelNode eModelNode);
        LTFLOAT          GetSkeletonNodeDamageFactor(ModelSkeleton eModelSkeleton, ModelNode eModelNode);
		ModelNode		GetSkeletonNodeParent(ModelSkeleton eModelSkeleton, ModelNode eModelNode);
		ModelNode		GetSkeletonNodeRecoilParent(ModelSkeleton eModelSkeleton, ModelNode eModelNode);
		HitLocation		GetSkeletonNodeLocation(ModelSkeleton eModelSkeleton, ModelNode eModelNode);
        LTFLOAT          GetSkeletonNodeHitRadius(ModelSkeleton eModelSkeleton, ModelNode eModelNode);
        LTFLOAT          GetSkeletonNodeHitPriority(ModelSkeleton eModelSkeleton, ModelNode eModelNode);

		// Node Scripts

		int				GetNumNodeScripts() { return m_cNScripts; }
		int				GetNScriptNumPts(ModelNScript eModelNScript);
		const char*		GetNScriptNodeName(ModelNScript eModelNScript);
        uint8           GetNScriptFlags(ModelNScript eModelNScript);
        LTFLOAT          GetNScriptPtTime(ModelNScript eModelNScript, int nPt);
        const LTVector&  GetNScriptPtPosOffset(ModelNScript eModelNScript, int nPt);
        const LTVector&  GetNScriptPtRotOffset(ModelNScript eModelNScript, int nPt);

#ifndef _CLIENTBUILD
		ModelStyle		GetModelStyleFromProperty(char* pPropVal);
#endif

		// Files

		const char*		GetModelFilename(ModelId eModelId, ModelStyle eModelStyle, const char* szCinematicExtension = LTNULL);
		const char*		GetMultiModelFilename(ModelId eModelId, ModelStyle eModelStyle);
		const char*		GetBodySkinFilename(ModelId eModelId, ModelStyle eModelStyle, const char* szBodySkinExtension = LTNULL);
		const char*		GetHeadSkinFilename(ModelId eModelId, ModelStyle eModelStyle, const char* szHeadOverride = LTNULL);
		const char*		GetHandsSkinFilename(ModelStyle eModelStyle);

	protected : // Protected inner classes

		class CStyle;
		class CNode;
		class CSkeleton;
		class CModel;
		class CNScriptPt;
		class CNScript;

	protected : // Protected member variables

		int				m_cModels;
		CModel*			m_aModels;

		int				m_cStyles;
		CStyle*			m_aStyles;

		int				m_cSkeletons;
		CSkeleton*		m_aSkeletons;

		int				m_cNScripts;
		CNScript*		m_aNScripts;
};

// ----------------------------------------------------------------------- //
//
// !!!!!!!!!!!!!!!!  CModelButeMgr inner class definitions  !!!!!!!!!!!!!!
//
// ----------------------------------------------------------------------- //

class CModelButeMgr::CStyle
{
	public :

		CStyle()
		{
			m_szName[0] = 0;
		}

	public :

		char		m_szName[32];
};

class CModelButeMgr::CNode
{
	public :

		CNode()
		{
			m_szName[0] = 0;
			m_szFrontDeathAni[0] = 0;
			m_szBackDeathAni[0] = 0;
			m_szFrontLongRecoilAni[0] = 0;
			m_szBackLongRecoilAni[0] = 0;
			m_szFrontShortRecoilAni[0] = 0;
			m_szBackShortRecoilAni[0] = 0;
			m_dwFlags = 0x0;
			m_fDamageFactor = 0.0f;
			m_eModelNodeParent = eModelNodeInvalid;
			m_eModelNodeRecoilParent = eModelNodeInvalid;
			m_eHitLocation = HL_UNKNOWN;
			m_fHitRadius = 10.0f;
			m_fHitPriority = 0.0f;
		}

	public :

		char			m_szName[32];
		char			m_szFrontDeathAni[32];
		char			m_szBackDeathAni[32];
		char			m_szFrontLongRecoilAni[32];
		char			m_szBackLongRecoilAni[32];
		char			m_szFrontShortRecoilAni[32];
		char			m_szBackShortRecoilAni[32];
        uint32          m_dwFlags;
        LTFLOAT          m_fDamageFactor;
		ModelNode		m_eModelNodeParent;
		ModelNode		m_eModelNodeRecoilParent;
		HitLocation		m_eHitLocation;
        LTFLOAT          m_fHitRadius;
        LTFLOAT          m_fHitPriority;
};

class CModelButeMgr::CSkeleton
{
	public :

		CSkeleton()
		{
			m_cNodes = 0;
            m_aNodes = LTNULL;
			m_szDefaultFrontDeathAni[0] = 0;
			m_szDefaultBackDeathAni[0] = 0;
			m_szDefaultFrontLongRecoilAni[0] = 0;
			m_szDefaultBackLongRecoilAni[0] = 0;
			m_szDefaultFrontShortRecoilAni[0] = 0;
			m_szDefaultBackShortRecoilAni[0] = 0;
			m_eModelNodeDefaultHit = eModelNodeInvalid;
		}

	public :

		CNode*		m_aNodes;
		int			m_cNodes;
		char		m_szDefaultFrontDeathAni[32];
		char		m_szDefaultBackDeathAni[32];
		char		m_szDefaultFrontLongRecoilAni[32];
		char		m_szDefaultBackLongRecoilAni[32];
		char		m_szDefaultFrontShortRecoilAni[32];
		char		m_szDefaultBackShortRecoilAni[32];
		ModelNode	m_eModelNodeDefaultHit;
};

class CModelButeMgr::CNScriptPt
{
	public :

		CNScriptPt()
		{
			m_fTime = 0.0f;
			m_vPosOffset.Init();
			m_vRotOffset.Init();
		}

	public :

        LTFLOAT      m_fTime;
        LTVector     m_vPosOffset;
        LTVector     m_vRotOffset;
};

class CModelButeMgr::CNScript
{
	public :

		CNScript()
		{
            m_aNScriptPts = LTNULL;
			m_cNScriptPts = 0;

			m_szName[0] = 0;
			m_bFlags = 0;
		}

	public:

		CNScriptPt*		m_aNScriptPts;
		int				m_cNScriptPts;

		char			m_szName[32];
        uint8           m_bFlags;
};

class CModelButeMgr::CModel
{
	public :

		CModel()
		{
			m_szName[0] = 0;
			m_szSex[0] = 0;
			m_szNationality[0] = 0;
			m_eModelSkeleton = eModelSkeletonInvalid;
			m_eModelType = eModelTypeInvalid;
            m_bModelEnvironmentMap = LTFALSE;
			m_fModelMass = 0.0f;
			m_fModelHitPoints = 0.0f;
			m_fModelMaxHitPoints = 0.0f;
			m_fModelArmor = 0.0f;
			m_fModelMaxArmor = 0.0f;
		}

	public :

		char			m_szName[32];
		char			m_szSex[32];
		char			m_szNationality[32];
		ModelSkeleton	m_eModelSkeleton;
		ModelType		m_eModelType;
        LTBOOL           m_bModelEnvironmentMap;
        LTFLOAT          m_fModelMass;
        LTFLOAT          m_fModelHitPoints;
        LTFLOAT          m_fModelMaxHitPoints;
        LTFLOAT          m_fModelArmor;
        LTFLOAT          m_fModelMaxArmor;
};


////////////////////////////////////////////////////////////////////////////
//
// CModelButeMgrPlugin is used to help facilitate populating the DEdit
// object properties that use CModelButeMgr
//
////////////////////////////////////////////////////////////////////////////
#ifndef _CLIENTBUILD

#include "iobjectplugin.h"

class CModelButeMgrPlugin : public IObjectPlugin
{
	public:

    virtual LTRESULT PreHook_EditStringList(
		const char* szRezPath,
		const char* szPropName,
		char** aszStrings,
        uint32* pcStrings,
        const uint32 cMaxStrings,
        const uint32 cMaxStringLength);

    void PopulateStringList(char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength);

	protected :

        static LTBOOL            sm_bInitted;
		static CModelButeMgr	sm_ButeMgr;
};

#endif // _CLIENTBUILD


#endif // __MODELBUTE_MGR_H__