#if !defined(_BASESPELLDEF_H_)
#define _BASESPELLDEF_H_

// Some constants
const unsigned MaxStrLen = 128;

#include "EffectTypes.h"
#include "HashTable.h"
//#include "ztools.h"

// Bitwise flags to describe spell

#define			SPELLBITS_CASTER_HEALTH					(1<<0)
#define			SPELLBITS_TARGET_HEALTH					(1<<1)														
#define			SPELLBITS_CASTER_SANITY					(1<<2)
#define			SPELLBITS_TARGET_SANITY					(1<<3)
#define			SPELLBITS_CASTER_MELEE_HEALTH_DAMAGE	(1<<4)
#define			SPELLBITS_TARGET_MELEE_HEALTH_DAMAGE	(1<<5)														
#define			SPELLBITS_CASTER_MELEE_SANITY_DAMAGE	(1<<6)
#define			SPELLBITS_TARGET_MELEE_SANITY_DAMAGE	(1<<7)
#define			SPELLBITS_CASTER_SPEED					(1<<8)
#define			SPELLBITS_TARGET_SPEED					(1<<9)
#define			SPELLBITS_SUMMON						(1<<10)
#define			SPELLBITS_GLYPH							(1<<11)
#define			SPELLBITS_ENV_EFFECT					(1<<12)
#define			SPELLBITS_INVISIBLE						(1<<13)
#define			SPELLBITS_LEVITATION					(1<<14)
#define			SPELLBITS_SHIELD						(1<<15)
#define			SPELLBITS_REFLECT						(1<<16)
#define			SPELLBITS_DISPELL						(1<<17)
#define			SPELLBITS_PROJECTILE					(1<<18)
#define			SPELLBITS_TARGET						(1<<19)
#define			SPELLBITS_POINT							(1<<20)
#define			SPELLBITS_SELF							(1<<21)
#define			SPELLBITS_DETECT_HIDDEN					(1<<22)
#define			SPELLBITS_INHIBIT_SPELL_CAST			(1<<23)
#define			SPELLBITS_PUSH							(1<<24)
#define			SPELLBITS_PULL							(1<<25)
#define			SPELLBITS_WALL							(1<<26)
#define			SPELLBITS_PIT							(1<<27)
#define			SPELLBITS_SHIELDBREAK					(1<<28)
#define			SPELLBITS_PROTECTION					(1<<29)
#define			SPELLBITS_DUMMY1						(1<<30)
#define			SPELLBITS_DUMMY2						(1<<31)

// Totem defines....

#define TOTEM_NONE				0x00000000
#define TOTEM_FIRE				0x00000001
#define TOTEM_SUN				0x00000002
#define TOTEM_ILLUSION			0x00000004
#define TOTEM_SCIENCE			0x00000008
#define TOTEM_DEMONOLOGY		0x00000010
#define TOTEM_DEATH				0x00000020
#define TOTEM_STORM				0x00000040
#define TOTEM_TRUTH				0x00000080
#define TOTEM_BLOOD				0x00000100
#define TOTEM_ICE				0x00000200
#define TOTEM_ALL				0x000003FF

//////////////////////////////////////////////////////////////////////////////
//
// class CBaseSpellDef
//
//		This is non Lithtech dependent, and holds all data that is a spell def.
//		We do this so other non Lithtech apps such as the Spell Editor can use
//      use the spell parser
//
class CBaseSpellDef
{
public:  // Enumerations

	enum Defs
	{
		ENUM,
		INTEGER,
		HEX,
		FLOAT,
		STRING,
		VECTOR,
		D4VECTOR,
		CLRKEY
	};

	enum SpellTypes
	{
		SPELL_SELF,
		SPELL_POINT,
		SPELL_TARGETING,
		SPELL_TARGETACTIVATEABLE
	};

	enum SubTypes
	{
		SUB_PROJECTILE,
		SUB_BLOCKING,
		SUB_GLYPH,
		SUB_NONE
	};

	enum ModifierTypes
	{
		MT_BINDING,
		MT_NONE
	};

	enum PhaseType
	{
		PHASE_CAST,
		PHASE_ACTIVE,
		PHASE_RESOLUTION
	};

	enum TotemType
	{
		TT_FIRE,
		TT_SUN,
		TT_ILLUSION,
		TT_SCIENCE,
		TT_DEMONOLOGY,
		TT_DEATH,
		TT_STORM,
		TT_TRUTH,
		TT_BLOOD,
		TT_ICE,
		TT_ALL,
		TT_NONE
	};

public:  // Methods

	CBaseSpellDef();
	~CBaseSpellDef();

	virtual CBaseSpellDef* construct() { return new CBaseSpellDef; }

	void WriteToStream(ostream& os, BOOL bWriteThing = FALSE);
	void CalcCRC(DWORD dwKey, int nLen);
	BOOL CheckCRC(int nDefSize, const CString& csGuid);

	bool IsSelfSpell() { return m_type == SPELL_SELF; }
	bool IsPointSpell() { return m_type == SPELL_POINT; }
	bool IsTargetingSpell() { return m_type == SPELL_TARGETING; }

	SpellTypes GetType() { return (SpellTypes)m_type; }
	SubTypes GetSubType() { return (SubTypes)m_subType; }

	DWORD CalcSize();


public: // Structure definitions

	struct ClrKey
	{
		float m_tmKey;
		DWORD m_dwColor;
	};

	struct Id
	{
		// Since DVector now has constructors which aren't allowed in unions I'm adding
		// this struct for use by the IdType union below to solve the problem.  Hence you
		// won't use this struct, you should treat it as a DVector.
	private:
		struct TrivialVector
		{
			float x, y, z;
			operator const DVector() const
			{ 
				DVector vec(x, y, z);
				return vec; 
			}
		};
	public:

		const char *m_szName;
		int m_type;
		union IdType
		{
			float m_float;
			int m_int;
			TrivialVector m_vec;
			const char *m_string;
			float m_d4vec[4];
			ClrKey m_clrKey;
		} m_data;
	};

	struct Fx
	{
		Fx() : m_idList(zPtrColl::active) { }
		const char *m_szName;
		zGDList<Id> m_idList;
	};

	struct Phase
	{
		Phase() : m_fxList(zPtrColl::active), m_effectCount(0) { }
		~Phase() { m_effectList.purge(); }
		PhaseType m_phase;
		float m_phaseLength;
		zGDList<Fx> m_fxList;
		int m_effectCount;
		zGSList<CBaseEffect> m_effectList;
	};


public:  // members

	CString m_sGuid;

	CString m_sName;

	CString m_sDescription;
	CString m_sModelName;
	CString m_sSkinName;
	CString m_sIcon;
	CString m_sCastAniName;
	CString m_sAiCastAniName;
	
	DWORD m_dwKey1;
	DWORD m_dwKey2;
	DWORD m_dwKey3;
	DWORD m_dwKey4;
	DWORD m_dwKey5;
	DWORD m_dwKey6;
	DWORD m_dwKey7;
	DWORD m_dwKey8;
	
	int m_type;
	int m_subType;
	int m_modifier;
	int m_totemType;
	int m_totemLevel;
	int m_isModelVisible;
	int m_health;
	DWORD m_spellBits;

	int m_casterSanityCost;
	int m_casterHealthCost;
	int m_targetSanityCost;
	int m_targetHealthCost;

	float m_lifeSpan;
	float m_speed;
	float m_modelScale;
	float m_activationOffset;
	int m_castLoopCount;

	float m_fYVelocity;
	float m_fYGravity;

	zGDList<Phase> m_phaseList;
	int m_phaseCount;

	static CHashTable	sm_hashStrings;

private:  // methods

	void WritePhase(ostream& os, Phase* pPhase);
	void WriteFx(ostream& os, Fx* pFx);
	void WriteId(ostream& os, Id* pId, const int tabPrefix);


};

char* TotemTypeEnumStringVals[];
extern int g_nNumTotems;
extern int g_nNumRetailTotems;

char* SpellBitStringVals[];
extern int g_nNumSpellBits;

#endif