// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#ifndef __ANIMATIONMGR_H__
#define __ANIMATIONMGR_H__

#include "AnimationParser.h"
#pragma warning (disable : 4786)
#include <map>
#include <vector>
#include <string>
//
// MAPS: Hash tables of string values for enums.
//
typedef std::multimap<long, EnumAnimMovement> ANIM_MOVEMENT_HASH;
typedef std::multimap<long, EnumAnimPropGroup> PROP_GROUP_HASH;
typedef std::multimap<long, EnumAnimProp> PROP_HASH;

//
// MAP: Lookup table associating the group with the anim prop.
//
typedef std::map<EnumAnimProp, EnumAnimPropGroup> PROP_GROUP_MAP;


// CAnimationProp

class CAnimationProp
{
	public :

		// Ctors/Dtors/etc

		CAnimationProp()
		{
			m_eGroup = kAPG_Invalid;
			m_eProp  = kAP_None;
		}

		void Set(EnumAnimPropGroup eAnimPropGroup, EnumAnimProp eAnimProp)
		{
			m_eGroup = eAnimPropGroup;
			m_eProp  = eAnimProp;
		}

		// Accessors.

		EnumAnimProp GetProp() const { return m_eProp; }

		// Save/load

		void Save(ILTMessage_Write *pMsg);
		void Load(ILTMessage_Read *pMsg);

		// Comparison

		LTBOOL operator==(const CAnimationProp& Prop) const
		{
			return ((m_eGroup == Prop.m_eGroup) && ((m_eProp == kAP_Any) || (Prop.m_eProp == kAP_Any) || (Prop.m_eProp == m_eProp)));
		}

		// LessThan is used for sorting props in STL maps.
		LTBOOL operator<(const CAnimationProp& Prop) const
		{
			return (m_eProp < Prop.m_eProp);
		}

		LTBOOL Contains(const CAnimationProp& Prop) const
		{
			return (Prop.m_eProp == kAP_None || (*this == Prop));
		}

		LTBOOL Disjoint(const CAnimationProp& Prop) const
		{
			return (Prop.m_eProp == kAP_None || !(*this == Prop));
		}

		// Clear

		void Clear() { m_eProp = kAP_None; }

	protected :

		friend class CAnimationProp;
		friend class CAnimationProps;
		friend class CAnimationMgr;

		EnumAnimPropGroup	m_eGroup;
		EnumAnimProp		m_eProp;
};

// CAnimationProps

class CAnimationProps
{
	public :

		// Ctors/Dtors/etc

		CAnimationProps();

		// Save/load

		void Save(ILTMessage_Write *pMsg);
		void Load(ILTMessage_Read *pMsg);

		// Comparison

		bool operator==(const CAnimationProps& Props) const
		{
			for ( uint32 iProp = 0 ; iProp < kAPG_Count ; ++iProp )
			{
				if ( !(m_aProps[iProp] == Props.m_aProps[iProp]) )
				{
					return false;
				}
			}

			return true;
		}

		// LessThan is used for sorting props in STL maps.
		bool operator<(const CAnimationProps& Props) const
		{
			for ( uint32 iProp = 0 ; iProp < kAPG_Count ; ++iProp )
			{
				if ( (m_aProps[iProp] < Props.m_aProps[iProp]) )
				{
					return true;
				}

				if ( (Props.m_aProps[iProp] < m_aProps[iProp]) )
				{
					return false;
				}
			}

			return false;
		}

		bool Contains(const CAnimationProps& Props) const
		{
			for ( uint32 iProp = 0 ; iProp < kAPG_Count ; iProp++ )
			{
				if ( !m_aProps[iProp].Contains(Props.m_aProps[iProp]) )
				{
					return false;
				}
			}

			return true;
		}

		bool Disjoint(const CAnimationProps& Props) const
		{
			for ( uint32 iProp = 0 ; iProp < kAPG_Count ; iProp++ )
			{
				if ( !m_aProps[iProp].Disjoint(Props.m_aProps[iProp]) )
				{
					return false;
				}
			}

			return true;
		}

		// Simple accessors

		void Clear();
		void Set(const CAnimationProp& Prop);
		bool IsSet(const CAnimationProp& Prop) const;

		void Set(EnumAnimPropGroup eAnimPropGroup, EnumAnimProp eAnimProp);
		bool IsSet(EnumAnimPropGroup eAnimPropGroup, EnumAnimProp eAnimProp) const;
		EnumAnimProp Get(EnumAnimPropGroup eAnimPropGroup) const { return m_aProps[eAnimPropGroup].m_eProp; }


		// Debugging

		void GetString(char* szBuffer, uint nBufferLength);

	protected :

		CAnimationProp		m_aProps[kAPG_Count];
};


// CAnimationInstance

class CAnimationInstance
{
	public :

		uint32 GetIndex() const { return m_iIndex; }
		HMODELANIM GetAni() const { return m_hAni; }

	protected :

		friend class CAnimationMgr;

		uint32				m_iIndex;
		HMODELANIM			m_hAni;
};

// CTransitionInstance

class CTransitionInstance
{
	public :

		uint32 GetIndex() const { return m_iIndex; }
		HMODELANIM GetAni() const { return m_hAni; }

	protected :

		friend class CAnimationMgr;

		uint32				m_iIndex;
		HMODELANIM			m_hAni;
};

// CAnimationContext

class CAnimationContext
{
	public :

		// Ctors/Dtors/etc

		void Init(uint32 cAnimationInstances, uint32 cTransitionInstances);
		void Term();

		LTRESULT Reset( HOBJECT hobj );

#ifndef _CLIENTBUILD
		// Save/load

		void Save(ILTMessage_Write *pMsg);
		void Load(ILTMessage_Read *pMsg);
#endif _CLIENTBUILD

		// Updates

		void Update();

		// Locking/Unlocking

		void Lock();
		void Unlock();
		LTBOOL IsLocked() { return m_bLocked; }
		void ClearLock();

		// AnimRate

		void SetAnimRate(LTFLOAT fAnimRate);

		// Props

		void ClearProps() { m_Props.Clear(); }
		void SetProps(const CAnimationProps& Props) { m_Props = Props; }
		void SetProp(const CAnimationProp& Prop) { m_Props.Set(Prop); }
		void SetProp(EnumAnimPropGroup eAnimPropGroup, EnumAnimProp eAnimProp);
		EnumAnimProp GetProp(EnumAnimPropGroup eAnimPropGroup) { return m_Props.Get(eAnimPropGroup); }

		LTBOOL IsPropSet(const CAnimationProp& Prop) const;
		LTBOOL IsPropSet(EnumAnimPropGroup eGroup, EnumAnimProp eProp) const;
		EnumAnimProp GetCurrentProp(EnumAnimPropGroup eGroup) const;

		void GetPropsString(char* szBuffer, uint nBufferLength);

		LTBOOL AnimationExists( CAnimationProps Props );

		HMODELANIM GetAni( CAnimationProps Props );
		LTFLOAT GetAnimationLength( CAnimationProps Props );
		LTFLOAT GetCurAnimationLength();
		void ClearCurAnimation() { m_iAnimation = m_cAnimationInstances - 1; }

		// Randomization

		uint32	ChooseRandomSeed( CAnimationProps Props );
		uint32	GetRandomSeed() { return m_iRandomSeed; }
		void	SetRandomSeed(uint32 iSeed) { m_iRandomSeed = iSeed; }

		// Movement

		EnumAnimMovement GetAnimMovementType() const;

		// Special

		void SetSpecial(const char* szName);
		void SetSpecial(const CAnimationProps& Props);
		void PlaySpecial();
		void ClearSpecial();
		void LoopSpecial();
		void LingerSpecial();
		void StopSpecial();
		LTBOOL IsSpecialDone();
		LTBOOL IsPlayingSpecial();

		// Transition

		LTBOOL IsTransitioning() const { return (m_eState == eStateTransition); }
		void PlayTransition(LTBOOL bPlayTransition) { m_bPlayTransition = bPlayTransition; }

	protected :

		enum State
		{
			eStateLock,
			eStatePostLock,
			eStateClearLock,
			eStateTransition,
			eStateTransitionComplete,
			eStateNormal,
			eStateStartSpecial,
			eStateStartSpecialLoop,
			eStateStartSpecialLinger,
			eStateStopSpecial,
			eStateClearSpecial,
			eStateSpecial,
			eStateSpecialLoop,
			eStateSpecialLinger,
		};

	protected :

		friend class CAnimationMgr;

		LTObjRef				m_hObject;

		HMODELANIM				m_haniSpecial;
		EnumAnimMovement		m_eSpecialMovement;
		uint32					m_iSpecialAnimation;

		LTBOOL					m_bLocked;
		uint32					m_iAnimationLocked;

		CAnimationMgr*			m_pAnimationMgr;

		CAnimationProps			m_Props;
		State					m_eState;

		uint32					m_cAnimationInstances;
		CAnimationInstance*		m_aAnimationInstances;
		uint32					m_iAnimation;
		uint32					m_iAnimationFrom;
		uint32					m_iCachedAnimation;
		uint32					m_iRandomSeed;

		uint32					m_cTransitionInstances;
		CTransitionInstance*	m_aTransitionInstances;
		uint32					m_iTransition;
		LTBOOL					m_bPlayTransition;
};

// CAnimation

class CAnimation
{
	public :

		CAnimation();

		// Simple accessors

		uint32 GetIndex() const { return m_iIndex; }
		const CAnimationProps& GetProps() const { return m_Props; }
		const char* GetName() const { return m_szName.c_str(); }

	protected :

		friend class CAnimationMgr;

		//char				m_szName[32]; // t.f fix
		std::string         m_szName ;
		uint32				m_iIndex;
		EnumAnimMovement	m_eAnimMovement;
		CAnimationProps		m_Props;
};

typedef std::multimap<CAnimationProps, uint32> ANIM_PROP_MAP;	// Maps animations to index, sorted by props.


// CTransition

class CTransition
{
	public :

		CTransition();

		// Simple accessors

		uint32 GetIndex() const { return m_iIndex; }
		const char* GetName() const { return m_szName.c_str(); }

	protected :

		friend class CAnimationMgr;

		//char				m_szName[32];
		std::string         m_szName ;
		uint32				m_iIndex;
		EnumAnimMovement	m_eAnimMovement;

		CAnimationProps		m_PropsInitial;
		CAnimationProps		m_PropsAdd;
		CAnimationProps		m_PropsRemove;
		CAnimationProps		m_PropsConstant;
		CAnimationProps		m_PropsNot;
};

// CAnimationMgr

class CAnimationMgr
{
	public :

		// Ctors/Dtors/etc

		CAnimationMgr();
		~CAnimationMgr() { Term(); }

		LTBOOL Init(const char* szFilename);
		void Term();

		LTBOOL IsInitialized() const { return m_bInitialized; }
		const char* GetFilename() const { return m_szFilename; }

		// Contexts

		CAnimationContext* CreateAnimationContext(HOBJECT hObject);
		void DestroyAnimationContext(CAnimationContext* pAnimationContex);
		LTRESULT		ResetAnimationContext( CAnimationContext *, HOBJECT );
		// Props

		PROP_GROUP_MAP* GetPropertyMap() { return &m_mapProperties; }

		const char*	GetPropName(const CAnimationProp& Prop) const;

		// Movement

		EnumAnimMovement			GetAnimationMovement(uint32 iAnimation) const;
		EnumAnimMovement			GetTransitionMovement(uint32 iTransition) const;

		// Animations

		const CAnimation& FindAnimation(const CAnimationProps& Props, uint32* piCachedAnimation, uint32* piRandomSeed, LTBOOL bFailureIsError, HOBJECT hObject);
		const CAnimation& GetAnimation(uint32 iAnimation, HOBJECT hObject) const;
		uint32 CountAnimations(const CAnimationProps& Props);

		// Transitions

		const CTransition& FindTransition(const CAnimationProps& PropsFrom, const CAnimationProps& PropsTo) const;
		const CTransition& GetTransition(uint32 iTransition) const { return m_aTransitions[iTransition]; }

	protected :

		// Ctors/Dtors/etc

		LTBOOL Init(Animation::CAnimationParser& AnimationParser);

		// Animations

		HMODELANIM GetAnimationInstance(HOBJECT hObject, const char *szName);

	protected :

		LTBOOL				m_bInitialized;
		char*				m_szFilename;

		ANIM_PROP_MAP		m_mapAnimationProps;

		CAnimation*			m_aAnimations;
		uint32				m_cAnimations;

		CTransition*		m_aTransitions;
		uint32				m_cTransitions;

		PROP_GROUP_MAP		m_mapProperties;
};

// CAnimationMgrList

extern class CAnimationMgrList* g_pAnimationMgrList;

typedef std::vector<CAnimationMgr*> ANIMATION_MGR_LIST;

class CAnimationMgrList
{
public:
	CAnimationMgrList();
	~CAnimationMgrList();

	void Init();
	void Term();

	CAnimationMgr*	GetAnimationMgr(const char* szAnimationMgrPath);

	static EnumAnimMovement		GetAnimMovementFromName(const char* szName);
	static EnumAnimPropGroup	GetPropGroupFromName(const char* szName);
	static EnumAnimProp			GetPropFromName(const char* szName);
	static uint32				MakePropHashKey(const char* szKey);

protected:

	ANIMATION_MGR_LIST	m_lstAnimationMgrs;

	static ANIM_MOVEMENT_HASH	s_hashAnimMovement;
	static PROP_GROUP_HASH		s_hashPropGroup;
	static PROP_HASH			s_hashProp;
};

#endif
