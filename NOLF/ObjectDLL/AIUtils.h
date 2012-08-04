// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#ifndef __AI_UTILS_H__
#define __AI_UTILS_H__

// Globals

extern class CGameServerShell* g_pGameServerShell;

// Helper functions

inline LTFLOAT FOV2DP(LTFLOAT fFOV)
{
	LTFLOAT fFOVRadians = 90.0f - fFOV/2.0f;
	fFOVRadians = DEG2RAD(fFOVRadians);
	return (LTFLOAT)sin(fFOVRadians);
}

LTBOOL FindGrenadeDangerPosition(const LTVector& vPos, LTFLOAT fDangerRadiusSqr, LTVector* pvDangerPos, class CGrenade** ppGrenade);


// GetDifficultyFactor

LTFLOAT GetDifficultyFactor();
inline LTFLOAT RAISE_BY_DIFFICULTY(LTFLOAT fValue) { return fValue*GetDifficultyFactor(); }
inline LTFLOAT LOWER_BY_DIFFICULTY(LTFLOAT fValue) { return fValue/GetDifficultyFactor(); }

inline LTBOOL IsTrueChar(char ch)
{
	return (ch == 't' || ch == 'T' || ch == 'y' || ch == 'Y' || ch == '1');
}

inline LTBOOL IsFalseChar(char ch)
{
	return (ch == 'f' || ch == 'F' || ch == 'n' || ch == 'N' || ch == '0');
}

LTBOOL GroundFilterFn(HOBJECT hObj, void *pUserData);
LTBOOL WorldFilterFn(HOBJECT hObj, void *pUserData);

// Enums

enum Direction
{
	eDirectionRight,
	eDirectionLeft,
	eDirectionForward,
	eDirectionBackward,
};

#define SAVE_DIRECTION(x) SAVE_DWORD(x)
#define LOAD_DIRECTION(x) LOAD_DWORD_CAST(x, Direction)

enum SenseType
{
	stInvalid				= 255,
	stHearEnemyWeaponFire	= 0,
	stHearEnemyWeaponImpact	= 1,
	stSeeEnemy				= 2,
	stSeeAllyDeath			= 3,
	stSeeEnemyFootprint		= 4,
	stSeeEnemyFlashlight	= 5,
	stHearEnemyFootstep		= 6,
	stHearEnemyDisturbance	= 7,
	stHearAllyDeath			= 8,
	stHearAllyPain			= 9,
	stHearAllyWeaponFire	= 10,
};

enum SenseClass
{
	scInvalid				= 255,
	scDelay					= 0,
	scStimulation			= 1,
};

enum SenseOutcome
{
	soNone					= 255,

	// Stimulation outcomes

	soFullStimulation		= 1,
	soFalseStimulationLimit	= 2,
	soFalseStimulation		= 3,

	// Delay outcomes

	soReactionDelayFinished	= 4,
};

// Statics

extern int g_cIntersectSegmentCalls;

// Constants

extern const LTFLOAT c_fFOV180;
extern const LTFLOAT c_fFOV160;
extern const LTFLOAT c_fFOV140;
extern const LTFLOAT c_fFOV120;
extern const LTFLOAT c_fFOV90;
extern const LTFLOAT c_fFOV75;
extern const LTFLOAT c_fFOV60;
extern const LTFLOAT c_fFOV45;
extern const LTFLOAT c_fFOV30;

extern const LTFLOAT c_fUpdateDelta;
extern const LTFLOAT c_fDeactivationTime;

extern const LTFLOAT c_fFacingThreshhold;

extern const char c_szKeyFireWeapon[];
extern const char c_szKeyBodySlump[];
extern const char c_szKeyPickUp[];

extern const char c_szActivate[];

extern char c_szNoReaction[];

// Defines

//#define NUKE_REACTIONS 1

#define IMPLEMENT_ALIGNMENTS(ai) \
	IMPLEMENT_ALIGNMENT(ai, BAD) \
	IMPLEMENT_ALIGNMENT(ai, GOOD) \
	IMPLEMENT_ALIGNMENT_NEUTRAL(ai) \

#define IMPLEMENT_ALIGNMENT(ai, alignment) \
 \
BEGIN_CLASS(AI_##alignment##_##ai##) \
END_CLASS_DEFAULT(AI_##alignment##_##ai##, AI_##ai##, NULL, NULL) \
 \
AI_##alignment##_##ai##::AI_##alignment##_##ai##() : AI_##ai##() \
{ \
	m_cc = ##alignment##; \
	m_fSenseUpdateRate = 0.10f; \
    m_hstrAttributeTemplate = g_pLTServer->CreateString("AI_" #alignment "_" #ai ); \
} \

#ifdef NUKE_REACTIONS

	#define IMPLEMENT_ALIGNMENT_NEUTRAL(ai) \
	BEGIN_CLASS(AI_NEUTRAL_##ai##) \
	END_CLASS_DEFAULT(AI_NEUTRAL_##ai##, AI_##ai##, NULL, NULL) \
	 \
	AI_NEUTRAL_##ai##::AI_NEUTRAL_##ai##() : AI_##ai##() \
	{ \
		m_cc = NEUTRAL; \
		m_fSenseUpdateRate = 0.25f; \
		m_hstrAttributeTemplate = g_pLTServer->CreateString("AI_NEUTRAL_" #ai ); \
	} \

#else

	#define IMPLEMENT_ALIGNMENT_NEUTRAL(ai) \
	 \
	BEGIN_CLASS(AI_NEUTRAL_##ai##) \
			ADD_STRINGPROP_FLAG(ISE1st,					c_szNoReaction,			PF_GROUP2|PF_DYNAMICLIST) \
			ADD_STRINGPROP_FLAG(ISE,					"Distress",				PF_GROUP2|PF_STATICLIST) \
			ADD_STRINGPROP_FLAG(ISEFalse1st,			c_szNoReaction,			PF_HIDDEN) \
			ADD_STRINGPROP_FLAG(ISEFalse,				c_szNoReaction,			PF_HIDDEN) \
			ADD_STRINGPROP_FLAG(ISEFlashlight1st,		c_szNoReaction,			PF_HIDDEN) \
			ADD_STRINGPROP_FLAG(ISEFlashlight,			c_szNoReaction,			PF_HIDDEN) \
			ADD_STRINGPROP_FLAG(ISEFlashlightFalse1st,	c_szNoReaction,			PF_HIDDEN) \
			ADD_STRINGPROP_FLAG(ISEFlashlightFalse,		c_szNoReaction,			PF_HIDDEN) \
			ADD_STRINGPROP_FLAG(ISEFootprint1st,		c_szNoReaction,			PF_HIDDEN) \
			ADD_STRINGPROP_FLAG(ISEFootprint,			c_szNoReaction,			PF_HIDDEN) \
			ADD_STRINGPROP_FLAG(ISADeath1st,			c_szNoReaction,			PF_GROUP2|PF_DYNAMICLIST) \
			ADD_STRINGPROP_FLAG(ISADeath,				"Panic",				PF_GROUP2|PF_STATICLIST) \
			ADD_STRINGPROP_FLAG(IHEFootstep1st,			c_szNoReaction,			PF_HIDDEN) \
			ADD_STRINGPROP_FLAG(IHEFootstep,			c_szNoReaction,			PF_HIDDEN) \
			ADD_STRINGPROP_FLAG(IHEFootstepFalse1st,	c_szNoReaction,			PF_HIDDEN) \
			ADD_STRINGPROP_FLAG(IHEFootstepFalse,		c_szNoReaction,			PF_HIDDEN) \
			ADD_STRINGPROP_FLAG(IHEWeaponFire1st,		c_szNoReaction,			PF_GROUP2|PF_DYNAMICLIST) \
			ADD_STRINGPROP_FLAG(IHEWeaponFire,			"Panic",				PF_GROUP2|PF_STATICLIST) \
			ADD_STRINGPROP_FLAG(IHEWeaponImpact1st,		c_szNoReaction,			PF_GROUP2|PF_DYNAMICLIST) \
			ADD_STRINGPROP_FLAG(IHEWeaponImpact,		"Panic",				PF_GROUP2|PF_STATICLIST) \
			ADD_STRINGPROP_FLAG(IHEDisturbance1st,		c_szNoReaction,			PF_HIDDEN) \
			ADD_STRINGPROP_FLAG(IHEDisturbance,			c_szNoReaction,			PF_HIDDEN) \
			ADD_STRINGPROP_FLAG(IHAPain1st,				c_szNoReaction,			PF_GROUP2|PF_DYNAMICLIST) \
			ADD_STRINGPROP_FLAG(IHAPain,				"Panic",				PF_GROUP2|PF_STATICLIST) \
			ADD_STRINGPROP_FLAG(IHADeath1st,			c_szNoReaction,			PF_GROUP2|PF_DYNAMICLIST) \
			ADD_STRINGPROP_FLAG(IHADeath,				"Panic",				PF_GROUP2|PF_STATICLIST) \
			ADD_STRINGPROP_FLAG(IHAWeaponFire1st,		c_szNoReaction,			PF_HIDDEN) \
			ADD_STRINGPROP_FLAG(IHAWeaponFire,			c_szNoReaction,			PF_HIDDEN) \
	\
			ADD_STRINGPROP_FLAG(GSE1st,					c_szNoReaction,			PF_GROUP5|PF_DYNAMICLIST) \
			ADD_STRINGPROP_FLAG(GSE,					"Distress",				PF_GROUP5|PF_STATICLIST) \
			ADD_STRINGPROP_FLAG(GSEFalse1st,			c_szNoReaction,			PF_HIDDEN) \
			ADD_STRINGPROP_FLAG(GSEFalse,				c_szNoReaction,			PF_HIDDEN) \
			ADD_STRINGPROP_FLAG(GSEFlashlight1st,		c_szNoReaction,			PF_HIDDEN) \
			ADD_STRINGPROP_FLAG(GSEFlashlight,			c_szNoReaction,			PF_HIDDEN) \
			ADD_STRINGPROP_FLAG(GSEFlashlightFalse1st,	c_szNoReaction,			PF_HIDDEN) \
			ADD_STRINGPROP_FLAG(GSEFlashlightFalse,		c_szNoReaction,			PF_HIDDEN) \
			ADD_STRINGPROP_FLAG(GSEFootprint1st,		c_szNoReaction,			PF_HIDDEN) \
			ADD_STRINGPROP_FLAG(GSEFootprint,			c_szNoReaction,			PF_HIDDEN) \
			ADD_STRINGPROP_FLAG(GSADeath1st,			c_szNoReaction,			PF_GROUP5|PF_DYNAMICLIST) \
			ADD_STRINGPROP_FLAG(GSADeath,				"Panic",				PF_GROUP5|PF_STATICLIST) \
			ADD_STRINGPROP_FLAG(GHEFootstep1st,			c_szNoReaction,			PF_HIDDEN) \
			ADD_STRINGPROP_FLAG(GHEFootstep,			c_szNoReaction,			PF_HIDDEN) \
			ADD_STRINGPROP_FLAG(GHEFootstepFalse1st,	c_szNoReaction,			PF_HIDDEN) \
			ADD_STRINGPROP_FLAG(GHEFootstepFalse,		c_szNoReaction,			PF_HIDDEN) \
			ADD_STRINGPROP_FLAG(GHEWeaponFire1st,		c_szNoReaction,			PF_GROUP5|PF_DYNAMICLIST) \
			ADD_STRINGPROP_FLAG(GHEWeaponFire,			"Panic",				PF_GROUP5|PF_STATICLIST) \
			ADD_STRINGPROP_FLAG(GHEWeaponImpact1st,		c_szNoReaction,			PF_GROUP5|PF_DYNAMICLIST) \
			ADD_STRINGPROP_FLAG(GHEWeaponImpact,		"Panic",				PF_GROUP5|PF_STATICLIST) \
			ADD_STRINGPROP_FLAG(GHEDisturbance1st,		c_szNoReaction,			PF_HIDDEN) \
			ADD_STRINGPROP_FLAG(GHEDisturbance,			c_szNoReaction,			PF_HIDDEN) \
			ADD_STRINGPROP_FLAG(GHAPain1st,				c_szNoReaction,			PF_GROUP5|PF_DYNAMICLIST) \
			ADD_STRINGPROP_FLAG(GHAPain,				"Panic",				PF_GROUP5|PF_STATICLIST) \
			ADD_STRINGPROP_FLAG(GHADeath1st,			c_szNoReaction,			PF_GROUP5|PF_DYNAMICLIST) \
			ADD_STRINGPROP_FLAG(GHADeath,				"Panic",				PF_GROUP5|PF_STATICLIST) \
			ADD_STRINGPROP_FLAG(GHAWeaponFire1st,		c_szNoReaction,			PF_HIDDEN) \
			ADD_STRINGPROP_FLAG(GHAWeaponFire,			c_szNoReaction,			PF_HIDDEN) \
	\
			ADD_STRINGPROP_FLAG(CanSeeEnemyFlashlight,			"FALSE", PF_GROUP3|PF_HIDDEN) \
			ADD_STRINGPROP_FLAG(SeeEnemyFlashlightDistance,		"0", PF_GROUP3|PF_RADIUS|PF_HIDDEN) \
			ADD_STRINGPROP_FLAG(CanSeeEnemyFootprint,			"FALSE", PF_GROUP3|PF_HIDDEN) \
			ADD_STRINGPROP_FLAG(SeeEnemyFootprintDistance,		"FALSE", PF_GROUP3|PF_RADIUS|PF_HIDDEN) \
			ADD_STRINGPROP_FLAG(CanHearEnemyFootstep,			"0",	PF_GROUP3|PF_HIDDEN) \
			ADD_STRINGPROP_FLAG(HearEnemyFootstepDistance,		"FALSE", PF_GROUP3|PF_RADIUS|PF_HIDDEN) \
			ADD_STRINGPROP_FLAG(CanHearEnemyDisturbance,		"FALSE",	PF_GROUP3|PF_HIDDEN) \
			ADD_STRINGPROP_FLAG(HearEnemyDisturbanceDistance,	"0", PF_GROUP3|PF_RADIUS|PF_HIDDEN) \
	END_CLASS_DEFAULT(AI_NEUTRAL_##ai##, AI_##ai##, NULL, NULL) \
	 \
	AI_NEUTRAL_##ai##::AI_NEUTRAL_##ai##() : AI_##ai##() \
	{ \
		m_cc = NEUTRAL; \
		m_fSenseUpdateRate = 1.0f; \
		m_hstrAttributeTemplate = g_pLTServer->CreateString("AI_NEUTRAL_" #ai ); \
	} \

#endif

#define DEFINE_ALIGNMENTS(ai) \
	DEFINE_ALIGNMENT(ai, BAD) \
	DEFINE_ALIGNMENT(ai, GOOD) \
	DEFINE_ALIGNMENT(ai, NEUTRAL) \

#define DEFINE_ALIGNMENT(ai, alignment) \
 \
class AI_##alignment##_##ai## : public AI_##ai## \
{ \
	public : \
 \
 		AI_##alignment##_##ai##(); \
}; \

#define DEFINE_HUMAN(human) \
 \
class AI_##human## : public CAIHuman \
{ \
	public : \
\
 		AI_##human##();\
};\
\
DEFINE_ALIGNMENTS(##human##)

#define BEGIN_IMPLEMENT_HUMAN(human, alignment) \
 \
BEGIN_CLASS(AI_##human##)\
	ADD_VECTORPROP_VAL_FLAG(Dims, 24.0f, 53.0f, 24.0f, PF_DIMS|PF_HIDDEN)\

#define END_IMPLEMENT_HUMAN(human, alignment) \
 \
END_CLASS_DEFAULT_FLAGS(AI_##human##, CAIHuman, NULL, NULL, CF_HIDDEN)\
\
AI_##human##::AI_##human##() : CAIHuman() \
{ \
	m_eModelId = g_pModelButeMgr->GetModelId(#human);\
	m_eModelSkeleton = g_pModelButeMgr->GetModelSkeleton(m_eModelId);\
	m_cc = ##alignment##;\
    m_hstrAttributeTemplate = g_pLTServer->CreateString(#human);\
}\
\
IMPLEMENT_ALIGNMENTS(##human##);\

#define ADD_ATTACHMENT(pos, attachment) ADD_STRINGPROP_FLAG(##pos##,	attachment,			PF_GROUP6|PF_STATICLIST)

class CAIReactions
{
	public :

		HSTRING m_ahstrSeeEnemy[2];
		HSTRING m_ahstrSeeEnemyFalse[2];
		HSTRING m_ahstrSeeEnemyFlashlight[2];
		HSTRING m_ahstrSeeEnemyFlashlightFalse[2];
		HSTRING m_ahstrHearEnemyFootstep[2];
		HSTRING m_ahstrHearEnemyFootstepFalse[2];
		HSTRING m_ahstrHearEnemyWeaponFire[2];
		HSTRING m_ahstrHearEnemyWeaponImpact[2];
		HSTRING m_ahstrSeeAllyDeath[2];
		HSTRING m_ahstrSeeEnemyFootprint[2];
		HSTRING m_ahstrHearEnemyDisturbance[2];
		HSTRING m_ahstrHearAllyPain[2];
		HSTRING m_ahstrHearAllyDeath[2];
		HSTRING m_ahstrHearAllyWeaponFire[2];

        CAIReactions()
		{
            m_ahstrSeeEnemy[0] = LTNULL;
            m_ahstrSeeEnemy[1] = LTNULL;
            m_ahstrSeeEnemyFalse[0] = LTNULL;
            m_ahstrSeeEnemyFalse[1] = LTNULL;
            m_ahstrSeeEnemyFlashlight[0] = LTNULL;
            m_ahstrSeeEnemyFlashlight[1] = LTNULL;
            m_ahstrSeeEnemyFlashlightFalse[0] = LTNULL;
            m_ahstrSeeEnemyFlashlightFalse[1] = LTNULL;
            m_ahstrHearEnemyFootstep[0] = LTNULL;
            m_ahstrHearEnemyFootstep[1] = LTNULL;
            m_ahstrHearEnemyFootstepFalse[0] = LTNULL;
            m_ahstrHearEnemyFootstepFalse[1] = LTNULL;
            m_ahstrHearEnemyWeaponFire[0] = LTNULL;
            m_ahstrHearEnemyWeaponFire[1] = LTNULL;
            m_ahstrHearEnemyWeaponImpact[0] = LTNULL;
            m_ahstrHearEnemyWeaponImpact[1] = LTNULL;
            m_ahstrSeeAllyDeath[0] = LTNULL;
            m_ahstrSeeAllyDeath[1] = LTNULL;
            m_ahstrSeeEnemyFootprint[0] = LTNULL;
            m_ahstrSeeEnemyFootprint[1] = LTNULL;
            m_ahstrHearEnemyDisturbance[0] = LTNULL;
            m_ahstrHearEnemyDisturbance[1] = LTNULL;
            m_ahstrHearAllyPain[0] = LTNULL;
            m_ahstrHearAllyPain[1] = LTNULL;
            m_ahstrHearAllyDeath[0] = LTNULL;
            m_ahstrHearAllyDeath[1] = LTNULL;
            m_ahstrHearAllyWeaponFire[0] = LTNULL;
            m_ahstrHearAllyWeaponFire[1] = LTNULL;
		}

        ~CAIReactions()
		{
			FREE_HSTRING(m_ahstrSeeEnemy[0]);
			FREE_HSTRING(m_ahstrSeeEnemy[1]);
			FREE_HSTRING(m_ahstrSeeEnemyFalse[0]);
			FREE_HSTRING(m_ahstrSeeEnemyFalse[1]);
			FREE_HSTRING(m_ahstrSeeEnemyFlashlight[0]);
			FREE_HSTRING(m_ahstrSeeEnemyFlashlight[1]);
			FREE_HSTRING(m_ahstrSeeEnemyFlashlightFalse[0]);
			FREE_HSTRING(m_ahstrSeeEnemyFlashlightFalse[1]);
			FREE_HSTRING(m_ahstrHearEnemyFootstep[0]);
			FREE_HSTRING(m_ahstrHearEnemyFootstep[1]);
			FREE_HSTRING(m_ahstrHearEnemyFootstepFalse[0]);
			FREE_HSTRING(m_ahstrHearEnemyFootstepFalse[1]);
			FREE_HSTRING(m_ahstrHearEnemyWeaponFire[0]);
			FREE_HSTRING(m_ahstrHearEnemyWeaponFire[1]);
			FREE_HSTRING(m_ahstrHearEnemyWeaponImpact[0]);
			FREE_HSTRING(m_ahstrHearEnemyWeaponImpact[1]);
			FREE_HSTRING(m_ahstrSeeAllyDeath[0]);
			FREE_HSTRING(m_ahstrSeeAllyDeath[1]);
			FREE_HSTRING(m_ahstrSeeEnemyFootprint[0]);
			FREE_HSTRING(m_ahstrSeeEnemyFootprint[1]);
			FREE_HSTRING(m_ahstrHearEnemyDisturbance[0]);
			FREE_HSTRING(m_ahstrHearEnemyDisturbance[1]);
			FREE_HSTRING(m_ahstrHearAllyPain[0]);
			FREE_HSTRING(m_ahstrHearAllyPain[1]);
			FREE_HSTRING(m_ahstrHearAllyDeath[0]);
			FREE_HSTRING(m_ahstrHearAllyDeath[1]);
			FREE_HSTRING(m_ahstrHearAllyWeaponFire[0]);
			FREE_HSTRING(m_ahstrHearAllyWeaponFire[1]);
		}

        void Save(HMESSAGEWRITE hWrite)
		{
			SAVE_HSTRING(m_ahstrSeeEnemy[0]);
			SAVE_HSTRING(m_ahstrSeeEnemy[1]);
			SAVE_HSTRING(m_ahstrSeeEnemyFalse[0]);
			SAVE_HSTRING(m_ahstrSeeEnemyFalse[1]);
			SAVE_HSTRING(m_ahstrSeeEnemyFlashlight[0]);
			SAVE_HSTRING(m_ahstrSeeEnemyFlashlight[1]);
			SAVE_HSTRING(m_ahstrSeeEnemyFlashlightFalse[0]);
			SAVE_HSTRING(m_ahstrSeeEnemyFlashlightFalse[1]);
			SAVE_HSTRING(m_ahstrHearEnemyFootstep[0]);
			SAVE_HSTRING(m_ahstrHearEnemyFootstep[1]);
			SAVE_HSTRING(m_ahstrHearEnemyFootstepFalse[0]);
			SAVE_HSTRING(m_ahstrHearEnemyFootstepFalse[1]);
			SAVE_HSTRING(m_ahstrHearEnemyWeaponFire[0]);
			SAVE_HSTRING(m_ahstrHearEnemyWeaponFire[1]);
			SAVE_HSTRING(m_ahstrHearEnemyWeaponImpact[0]);
			SAVE_HSTRING(m_ahstrHearEnemyWeaponImpact[1]);
			SAVE_HSTRING(m_ahstrSeeAllyDeath[0]);
			SAVE_HSTRING(m_ahstrSeeAllyDeath[1]);
			SAVE_HSTRING(m_ahstrSeeEnemyFootprint[0]);
			SAVE_HSTRING(m_ahstrSeeEnemyFootprint[1]);
			SAVE_HSTRING(m_ahstrHearEnemyDisturbance[0]);
			SAVE_HSTRING(m_ahstrHearEnemyDisturbance[1]);
			SAVE_HSTRING(m_ahstrHearAllyPain[0]);
			SAVE_HSTRING(m_ahstrHearAllyPain[1]);
			SAVE_HSTRING(m_ahstrHearAllyDeath[0]);
			SAVE_HSTRING(m_ahstrHearAllyDeath[1]);
			SAVE_HSTRING(m_ahstrHearAllyWeaponFire[0]);
			SAVE_HSTRING(m_ahstrHearAllyWeaponFire[1]);
		}

        void Load(HMESSAGEREAD hRead)
		{
			LOAD_HSTRING(m_ahstrSeeEnemy[0]);
			LOAD_HSTRING(m_ahstrSeeEnemy[1]);
			LOAD_HSTRING(m_ahstrSeeEnemyFalse[0]);
			LOAD_HSTRING(m_ahstrSeeEnemyFalse[1]);
			LOAD_HSTRING(m_ahstrSeeEnemyFlashlight[0]);
			LOAD_HSTRING(m_ahstrSeeEnemyFlashlight[1]);
			LOAD_HSTRING(m_ahstrSeeEnemyFlashlightFalse[0]);
			LOAD_HSTRING(m_ahstrSeeEnemyFlashlightFalse[1]);
			LOAD_HSTRING(m_ahstrHearEnemyFootstep[0]);
			LOAD_HSTRING(m_ahstrHearEnemyFootstep[1]);
			LOAD_HSTRING(m_ahstrHearEnemyFootstepFalse[0]);
			LOAD_HSTRING(m_ahstrHearEnemyFootstepFalse[1]);
			LOAD_HSTRING(m_ahstrHearEnemyWeaponFire[0]);
			LOAD_HSTRING(m_ahstrHearEnemyWeaponFire[1]);
			LOAD_HSTRING(m_ahstrHearEnemyWeaponImpact[0]);
			LOAD_HSTRING(m_ahstrHearEnemyWeaponImpact[1]);
			LOAD_HSTRING(m_ahstrSeeAllyDeath[0]);
			LOAD_HSTRING(m_ahstrSeeAllyDeath[1]);
			LOAD_HSTRING(m_ahstrSeeEnemyFootprint[0]);
			LOAD_HSTRING(m_ahstrSeeEnemyFootprint[1]);
			LOAD_HSTRING(m_ahstrHearEnemyDisturbance[0]);
			LOAD_HSTRING(m_ahstrHearEnemyDisturbance[1]);
			LOAD_HSTRING(m_ahstrHearAllyPain[0]);
			LOAD_HSTRING(m_ahstrHearAllyPain[1]);
			LOAD_HSTRING(m_ahstrHearAllyDeath[0]);
			LOAD_HSTRING(m_ahstrHearAllyDeath[1]);
			LOAD_HSTRING(m_ahstrHearAllyWeaponFire[0]);
			LOAD_HSTRING(m_ahstrHearAllyWeaponFire[1]);
		}
};

void ReadPropAIReactions(GenericProp& genProp, CAIReactions* pCAIReactions, const char* szName);

const static int c_nAIMaxReactons = 64;

struct REACTIONSTRUCT
{
	const char* szSense;
	const char* aszReactions[c_nAIMaxReactons];
};

#endif