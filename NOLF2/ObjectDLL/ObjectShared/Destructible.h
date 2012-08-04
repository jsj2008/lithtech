// ----------------------------------------------------------------------- //
//
// MODULE  : Destructible.h
//
// PURPOSE : Destructible class
//
// CREATED : 9/23/97
//
// (c) 1997-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __DESTRUCTIBLE_H__
#define __DESTRUCTIBLE_H__

#include "iaggregate.h"
#include "ltengineobjects.h"
#include "ClientServerShared.h"
#include "DamageTypes.h"
#include "CommandMgr.h"
#include "LTObjRef.h"
#include "GameBase.h"

struct DamageStruct;

//
// The damage filter function allows classes to register a function
// that will give it "first dibs" on the damage message, before any
// damage is dealt.  This function can change any aspect of the
// DamageStruct, which will affect how things are subsequently handled.
//
// Return false to stop ALL processing of the damage, return true
// to continue processing the damage as normal.
//
typedef bool (*DamageFilterFunction)( GameBase *pObject,
                                      DamageStruct *pDamageStruct );

// Use ADD_DESTRUCTIBLE_AGGREGATE() in your class definition to enable
// the following properties in the editor.  For example:
//
//BEGIN_CLASS(CMyCoolObj)
//	ADD_DESTRUCTIBLE_AGGREGATE()
//	ADD_STRINGPROP(Filename, "")
//  ...
//

#define ADD_DESTRUCTIBLE_AGGREGATE_COMMANDS( group, flags ) \
		ADD_REALPROP_FLAG(DamagePercentForCommand, 50.0f, (group) | (flags)) \
		ADD_LONGINTPROP_FLAG(DamageTriggerCounter, 0, (group) | (flags)) \
		ADD_STRINGPROP_FLAG(DamageCommand, "", (group) | (flags) | PF_NOTIFYCHANGE) \
		ADD_LONGINTPROP_FLAG(DamageTriggerNumSends, 1, (group) | (flags)) \
		ADD_STRINGPROP_FLAG(DamagerMessage, "", (group) | (flags)) \
		ADD_STRINGPROP_FLAG(DeathCommand, "", (group) | (flags) | PF_NOTIFYCHANGE) \
		ADD_STRINGPROP_FLAG(DamagePercentCommand, "", (group) | (flags) | PF_NOTIFYCHANGE) \
		ADD_STRINGPROP_FLAG(PlayerDeathCommand, "", (group) | (flags) | PF_NOTIFYCHANGE) \
		ADD_STRINGPROP_FLAG(PlayerDamageCommand, "", (group) | (flags) | PF_NOTIFYCHANGE) \
		ADD_STRINGPROP_FLAG(KillerMessage, "", (group) | (flags)) \


#define ADD_DESTRUCTIBLE_AGGREGATE(group, flags) \
	PROP_DEFINEGROUP(DamageProperties, (group) | (flags)) \
		ADD_REALPROP_FLAG(Mass, 30.0f, (group) | (flags)) \
		ADD_REALPROP_FLAG(HitPoints, 100.0f, (group) | (flags)) \
		ADD_REALPROP_FLAG(MaxHitPoints, 100.0f, (group) | (flags)) \
		ADD_REALPROP_FLAG(Energy, 100.0f,  (group) | (flags)) \
		ADD_REALPROP_FLAG(MaxEnergy, 100.0f, (group) | (flags)) \
		ADD_REALPROP_FLAG(Armor, 100.0f, (group) | (flags)) \
		ADD_REALPROP_FLAG(MaxArmor, 100.0f, (group) | (flags)) \
		ADD_DESTRUCTIBLE_AGGREGATE_COMMANDS( (group), (flags)) \
        ADD_BOOLPROP_FLAG(CanHeal, LTTRUE, (group) | (flags)) \
        ADD_BOOLPROP_FLAG(CanRepair, LTTRUE, (group) | (flags)) \
        ADD_BOOLPROP_FLAG(CanDamage, LTTRUE, (group) | (flags)) \
        ADD_BOOLPROP_FLAG(NeverDestroy, LTFALSE, (group) | (flags))


#define MAX_PROGRESSIVE_DAMAGE 20
#define MAX_GEAR_ITEMS 10

struct DamageStruct : public ILTObjRefReceiver
{
  public :

	DamageStruct()
	{
		Clear();
		hDamager.SetReceiver( *this );
	}

	DamageStruct( DamageStruct const& other )
	{
		Copy( other );
		hDamager.SetReceiver( *this );
	}

	DamageStruct& operator=( DamageStruct const& other )
	{
		if( &other != this )
			Copy( other );

		return *this;
	}

	void Copy( DamageStruct const& other )
	{
		if( &other == this )
			return;

		vDir = other.vDir;
		eType = other.eType;
		hDamager = other.hDamager;
		hContainer = other.hContainer;
		fDamage = other.fDamage;
		fDuration = other.fDuration;
	}

	void Clear()
	{
		vDir.Init();
		fDamage		= 0.0f;
		fDuration	= 0.0f;
		eType		= DT_UNSPECIFIED;
        hDamager    = LTNULL;
        hContainer  = LTNULL;
	}

    void Save(ILTMessage_Write *pMsg)
	{
		if (!pMsg) return;

		SAVE_VECTOR(vDir);
		SAVE_FLOAT(fDamage);
		SAVE_FLOAT(fDuration);
		SAVE_BYTE(eType);
		SAVE_HOBJECT(hDamager);
		SAVE_HOBJECT(hContainer);
		SAVE_BYTE(nAmmoId);
	}

    void Load(ILTMessage_Read *pMsg)
	{
		if (!pMsg) return;

		LOAD_VECTOR(vDir);
		LOAD_FLOAT(fDamage);
		LOAD_FLOAT(fDuration);
		LOAD_BYTE_CAST(eType, DamageType);
		LOAD_HOBJECT(hDamager);
		LOAD_HOBJECT(hContainer);
		LOAD_BYTE(nAmmoId);
	}
 
	const static LTFLOAT kInfiniteDamage;

    LTBOOL InitFromMessage(ILTMessage_Read *pMsg);
    LTBOOL DoDamage(LPBASECLASS pDamager, HOBJECT hVictim, HOBJECT hContainer=LTNULL);

	// Handler if damager is deleted.
	virtual void OnLinkBroken( LTObjRefNotifier *pRef, HOBJECT hObj )
	{
		if( pRef == &hDamager )
		{
			Clear( );
		}
	}

    LTVector		vDir;
	DamageType		eType;
	LTObjRefNotifier	hDamager;
	LTObjRef		hContainer;
    LTFLOAT			fDamage;
    LTFLOAT			fDuration;  // Progressive damage only...
	uint8			nAmmoId;
};


class CDestructible : public IAggregate
{
	public :

		CDestructible();
		virtual ~CDestructible();

        LTBOOL	Init(HOBJECT hObject);

        void	Reset(LTFLOAT fHitPts, LTFLOAT fArmorPts, LTFLOAT fEnergyPoints);
        LTBOOL  Repair(LTFLOAT fAmount);
        LTBOOL  Heal(LTFLOAT fAmount);
        LTBOOL  AddGear(uint8 nGearId);
		LTBOOL	HasGear( uint8 nGearId ) { return ( nGearId < MAX_GEAR_ITEMS ) ? m_bGearOwned[nGearId] : LTFALSE; }
		inline	void RemoveAllGear();

		void ClearProgressiveDamage( DamageType DT = DT_INVALID, bool bUpdateProgressiveDamage = true );
		void ClearProgressiveDamage( DamageFlags nDamageFlags );
		LTBOOL IsTakingProgressiveDamage( DamageType DT );

		virtual void HandleDestruction(HOBJECT hDamager, DamageType eDamageType);
		virtual void HandleDestruction(HOBJECT hDamager);

        void SetMass(LTFLOAT fMass);
        void SetMaxHitPoints(LTFLOAT fmhp)			{ m_fMaxHitPoints = fmhp; }
        void SetHitPoints(LTFLOAT fhp)				{ m_fHitPoints = fhp; }
		void SetMaxEnergy(LTFLOAT fmhp)				{ m_fMaxEnergy = fmhp; }
        void SetEnergy(LTFLOAT fhp)					{ m_fEnergy = fhp; }
        void SetMaxArmorPoints(LTFLOAT fmap)		{ m_fMaxArmorPoints = fmap; }
        void SetArmorPoints(LTFLOAT fap)			{ m_fArmorPoints = fap; }
		void SetDamagePercentCommand(LTFLOAT fper)	{ m_fDamagePercentCommand = fper; }
        void SetCanHeal(LTBOOL bCanHeal)			{ m_bCanHeal = bCanHeal; }
        void SetCanRepair(LTBOOL bCanRepair)		{ m_bCanRepair = bCanRepair; }
        void SetCanDamage(LTBOOL bCanDamage)		{ m_bCanDamage = bCanDamage; }
        void SetNeverDestroy(LTBOOL bNeverDestroy)	{ m_bNeverDestroy = bNeverDestroy; }
        void SetApplyDamagePhysics(LTBOOL b)		{ m_bApplyDamagePhysics = b; }
        void	SetCantDamageFlags(DamageFlags nFlags)	{ m_nCantDamageFlags = nFlags; }
		DamageFlags	GetCantDamageFlags() const			{ return m_nCantDamageFlags; }
        void	ClearCantDamageFlags(DamageFlags nTypes){ m_nCantDamageFlags &= ~nTypes; }
		LTBOOL	IsCantDamageType(DamageType dt)		{ return !!(m_nCantDamageFlags & DamageTypeToFlag(dt)); }

        LTBOOL      GetCanHeal()            const { return m_bCanHeal; }
        LTBOOL      GetCanRepair()          const { return m_bCanRepair; }
        LTBOOL      GetCanDamage()          const { return m_bCanDamage; }
        LTFLOAT     GetMass()               const { return m_fMass; }
        LTFLOAT     GetHitPoints()          const { return m_fHitPoints; }
		LTFLOAT     GetEnergy()		        const { return m_fEnergy; }
        LTFLOAT     GetArmorPoints()        const { return m_fArmorPoints; }
        LTBOOL      IsDead()                const { return m_bDead; }
		DamageType	GetDeathType()			const { return m_eDeathType; }
		DamageType	GetLastDamageType()		const { return m_eLastDamageType; }
        LTFLOAT     GetDeathDamage()        const { return m_fDeathDamage; }
        LTFLOAT     GetLastDamage()         const { return m_fLastDamage; }
        LTFLOAT     GetLastArmorAbsorb()    const { return m_fLastArmorAbsorb; }
        LTVector    GetDeathDir()           const { return m_vDeathDir; }
        LTVector    GetLastDamageDir()      const { return m_vLastDamageDir; }
        LTFLOAT     GetMaxHitPoints()       const;
		LTFLOAT		GetMaxEnergy()			const;
        LTFLOAT     GetMaxArmorPoints()     const;
		HOBJECT		GetLastDamager()		const { return m_hLastDamager; }

        LTFLOAT     GetStealthModifier();
        LTBOOL      HasAirSupply();

        LTBOOL      GetNeverDestroy()       const { return m_bNeverDestroy; }

		LTBOOL  RegisterFilterFunction( DamageFilterFunction pFn, GameBase *pObject );

	protected :

        uint32 EngineMessageFn(LPBASECLASS pObject, uint32 messageID, void *pData, LTFLOAT lData);
        uint32 ObjectMessageFn(LPBASECLASS pObject, HOBJECT hSender, ILTMessage_Read *pMsg);

		void	SetBlockingPriority();
        void    ApplyDamage(LTVector vDir, LTFLOAT fDamage, DamageType eType, HOBJECT hDamager);
        LTFLOAT ProcessGear(LTFLOAT fDamage, DamageType eDamageType, HOBJECT hDamager, LTVector* pvDir);
		void	ProcessPlayerDeath(HOBJECT hDamager);			// This has nothing to do with PlayerDeathTriggerTarget/Message!!!
        LTBOOL  CanRagDoll(LTVector & vDir, DamageType eType);
        LTBOOL  ArmorCanAbsorb(DamageType eType);
		void	SetLastDamager(HOBJECT hDamager);
		
		float	AdjustDamage( float fDamage, HOBJECT hDamager );	

		LTObjRef	m_hObject;			// The object I'm associated with.
		LTObjRef	m_hLastDamager;		// Our last damager

	private :

		DamageType	m_eDeathType;		// How did we die
		DamageType	m_eLastDamageType;	// What damaged us last
        LTFLOAT     m_fLastDamage;      // Amount of last damage done
        LTFLOAT     m_fLastArmorAbsorb; // Amount of last damage absorbed by armor
        LTFLOAT     m_fDeathDamage;     // Amount of damage done on death blow
        LTVector    m_vDeathDir;        // Direction death came from
        LTVector    m_vLastDamageDir;   // Direction of last damage

        LTBOOL      m_bApplyDamagePhysics;  // Rag doll?
        LTFLOAT     m_fMass;
        LTFLOAT     m_fMaxHitPoints;
        LTFLOAT     m_fHitPoints;
        LTFLOAT     m_fMaxArmorPoints;
        LTFLOAT     m_fArmorPoints;
        LTBOOL      m_bDead;
		LTFLOAT		m_fDamagePercentCommand;

		// [kml] 2/8/02
		LTFLOAT		m_fMaxEnergy;
		LTFLOAT		m_fEnergy;

        LTBOOL      m_bIsPlayer;

		// Process heal, repair, damage messages?
        LTBOOL      m_bCanHeal;
        LTBOOL      m_bCanRepair;
        LTBOOL      m_bCanDamage;
        LTBOOL      m_bNeverDestroy;

		std::string	m_sDamageCommand;
		std::string	m_sDamagerMessage;
		std::string	m_sDeathCommand;
		std::string	m_sPlayerDeathCommand;	// Like DeathTrigger, but only sent if PLAYER killed us
		std::string	m_sKillerMessage;
		std::string	m_sDamagePercentCommand;
		std::string m_sPlayerDamageCommand;

        uint32		m_nDamageTriggerCounter;    // How many times we have to be damaged before we send the trigger
        uint32		m_nDamageTriggerNumSends;   // Number of times to send damage trigger message
        DamageFlags	m_nCantDamageFlags;        // What can't damage us

		// Progressive damage being done...

		DamageStruct	m_ProgressiveDamage[MAX_PROGRESSIVE_DAMAGE];

	    LTBOOL          m_bGearOwned[MAX_GEAR_ITEMS];

		DamageFilterFunction m_pDamageFilterFunction;
		GameBase *m_pDamageFilterObject;

	private :

        LTBOOL ReadProp(LPBASECLASS pObject, ObjectCreateStruct *pInfo);
		void InitialUpdate(LPBASECLASS pObject);
		void Update();

		void HandleTrigger(LPBASECLASS pObject, HOBJECT hSender, ILTMessage_Read *pMsg);
		void HandleDamage(LPBASECLASS pObject, HOBJECT hSender, ILTMessage_Read *pMsg);
		void HandleHeal(LPBASECLASS pObject, HOBJECT hSender, ILTMessage_Read *pMsg);
		void HandleRepair(LPBASECLASS pObject, HOBJECT hSender, ILTMessage_Read *pMsg);
		void HandleCrush(LPBASECLASS pObject, HOBJECT hSender);
		void HandleTouch(LPBASECLASS pObject, HOBJECT hSender, LTFLOAT fForce);
		void HandleAddGear(LPBASECLASS pObject, HOBJECT hSender, ILTMessage_Read *pMsg);
        void Save(ILTMessage_Write *pMsg, uint8 nType);
        void Load(ILTMessage_Read *pMsg, uint8 nType);
        void DoActualHealing(LTFLOAT fAmount);

		void DoDamage(DamageStruct & damage);
		void UpdateProgressiveDamage();
		void AddProgressiveDamage(DamageStruct & damage);

        LTBOOL DebugDamageOn();
};

inline LTBOOL CDestructible::CanRagDoll(LTVector & vDir, DamageType eType)
{
	if (m_hObject)
	{
        uint32 dwFlags;
		g_pCommonLT->GetObjectFlags(m_hObject, OFT_Flags, dwFlags);
		if ((dwFlags & FLAG_SOLID) && (dwFlags & FLAG_GRAVITY))
		{
			return (m_bApplyDamagePhysics && !m_bIsPlayer && (eType != DT_BULLET) &&
					(m_fMass < INFINITE_MASS) && (vDir.MagSqr() > 0.001f));
		}
	}

    return LTFALSE;
}

inline LTBOOL CDestructible::ArmorCanAbsorb(DamageType eType)
{
	// [KLS 5/15/02] Updated so armor won't ignore melee damage...
	return ( (eType == DT_BULLET) || 
		     (eType == DT_EXPLODE) ||
			 (eType == DT_MELEE) ||
			 (eType == DT_SWORD) );
}

inline void CDestructible::RemoveAllGear()
{
	memset(m_bGearOwned, 0, sizeof(m_bGearOwned));
}

class CDestructiblePlugin : public IObjectPlugin
{
	public :

		virtual LTRESULT PreHook_PropChanged( 
			const	char		*szObjName,
			const	char		*szPropName,
			const	int			nPropType,
			const	GenericProp	&gpPropValue,
					ILTPreInterface	*pInterface,
			const	char		*szModifiers );

	protected :

		CCommandMgrPlugin m_CommandMgrPlugin;
};

#endif // __DESTRUCTIBLE_H__
