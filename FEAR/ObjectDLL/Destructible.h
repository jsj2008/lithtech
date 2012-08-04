// ----------------------------------------------------------------------- //
//
// MODULE  : Destructible.h
//
// PURPOSE : Destructible class
//
// CREATED : 9/23/97
//
// (c) 1997-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __DESTRUCTIBLE_H__
#define __DESTRUCTIBLE_H__

#include "iaggregate.h"
#include "ltengineobjects.h"
#include "ClientServerShared.h"
#include "DamageTypes.h"
#include "CommandMgr.h"
#include "ltobjref.h"
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
		ADD_REALPROP_FLAG(DamagePercentForCommand, 0.5f, (group) | (flags), "When HitPoints are below DamagePercentForCommand percent of the default HitPoints this command will be processed. (Value should be between 0 and 1)") \
		ADD_LONGINTPROP_FLAG(DamageTriggerCounter, 0, (group) | (flags), "This specifies how many times the object must be damaged before the damage message will be sent.") \
		ADD_COMMANDPROP_FLAG(DamageCommand, "", (group) | (flags) | PF_NOTIFYCHANGE, "This is the command string that is sent when the object receives damage from any source.") \
		ADD_LONGINTPROP_FLAG(DamageTriggerNumSends, 1, (group) | (flags), "This specifies how many times the damage message will be sent.") \
		ADD_STRINGPROP_FLAG(DamagerMessage, "", (group) | (flags), "This is the command string that is sent to the object that caused the damage.") \
		ADD_COMMANDPROP_FLAG(DeathCommand, "", (group) | (flags) | PF_NOTIFYCHANGE, "This is the command string that is sent when the object has been destroyed.") \
		ADD_COMMANDPROP_FLAG(DamagePercentCommand, "", (group) | (flags) | PF_NOTIFYCHANGE, "When HitPoints is below DamagePercentForCommand * MaxHitPoints this command will be processed.") \
		ADD_COMMANDPROP_FLAG(PlayerDeathCommand, "", (group) | (flags) | PF_NOTIFYCHANGE, "This is the command string that is sent when the player destroys the object.") \
		ADD_COMMANDPROP_FLAG(PlayerDamageCommand, "", (group) | (flags) | PF_NOTIFYCHANGE, "This is the command string that is sent when the player damages the object.") \
		ADD_STRINGPROP_FLAG(KillerMessage, "", (group) | (flags), "This command string is sent to the object that destroys this object.") \


#define ADD_DESTRUCTIBLE_AGGREGATE(group, flags) \
	PROP_DEFINEGROUP(DamageProperties, (group) | (flags), "This is a subset of properties that define how the object deals with damage.") \
		ADD_REALPROP_FLAG(Mass, 30.0f, (group) | (flags), "This value sets the mass of the object within the game.") \
		ADD_REALPROP_FLAG(HitPoints, 100.0f, (group) | (flags), "This is the number of hitpoints the object has when it is first created in the world.") \
		ADD_REALPROP_FLAG(Armor, 100.0f, (group) | (flags), "This is the amount of armor that the object has when it is created.") \
		ADD_DESTRUCTIBLE_AGGREGATE_COMMANDS( (group), (flags)) \
		ADD_BOOLPROP_FLAG(CanHeal, true, (group) | (flags), "Toggles whether the object can be healed.") \
		ADD_BOOLPROP_FLAG(CanRepair, true, (group) | (flags), "Toggles whether the object can be repaired.") \
		ADD_BOOLPROP_FLAG(CanDamage, true, (group) | (flags), "Toggles whether the object can be damaged.") \
		ADD_BOOLPROP_FLAG(NeverDestroy, false, (group) | (flags), "Toggles whether the object can be destroyed.") \
		ADD_STRINGPROP_FLAG(DamageMask, "<none>", (group) | (flags) | PF_STATICLIST, "A DamageMask is a list of damage types that are allowed to damage an object they are applied to.  Choose none to allow all damage affect this object.  Each DamageMask is specified in the database." )


//a structure that contains all the data necessary to apply damage to another object, including the
//type of damage, who created the damage, over what duration the damage should be applied, and optionally,
//the position and direction of where the damage was applied
struct DamageStruct : 
	public ILTObjRefReceiver
{
public :

	DamageStruct()
	{
		Clear();
		hDamager.SetReceiver( *this );
	}

	DamageStruct( DamageStruct const& other )
	{
		// Set the receiver first.  Assigning to hDamager asserts 
		// (inside the copy) asserts if it isn't already set.
		hDamager.SetReceiver( *this );
		Copy( other );
	}

	DamageStruct& operator=( DamageStruct const& other )
	{
		Copy( other );
		return *this;
	}

	bool operator==( DamageType DT ) const { return (eType == DT); }
	bool operator==( HOBJECT hObj ) const { return (hContainer == hObj); }

	//called to copy over all the data from the provided object. This will perform
	//the self copy check.
	void Copy( DamageStruct const& other )
	{
		if( &other == this )
			return;

		eType		= other.eType;
		hDamager	= ( HOBJECT )other.hDamager;
		hContainer	= other.hContainer;
		fDamage		= other.fDamage;
		fPenetration= other.fPenetration;
		fDuration	= other.fDuration;
		hAmmo		= other.hAmmo;
		fImpulseForce = other.fImpulseForce;

		bPositionalInfo = other.bPositionalInfo;
		vDir			= other.vDir;
		vPos			= other.vPos;
	}

	//resets all data associated with this object to defaults
	void Clear()
	{
		eType		= DT_UNSPECIFIED;
		hDamager	= NULL;
		hContainer	= NULL;
		fDamage		= 0.0f;
		fPenetration= 0.0f;
		fDuration	= 0.0f;		
		hAmmo		= NULL;
		fImpulseForce = 0.0f;

		//clear out the positional information
		bPositionalInfo = false;
		vDir.Init(0.0f, 1.0f, 0.0f);		
		vPos.Init();
	}

	//called to determine if positional data has been provided. If it hasn't, the position and direction
	//will be valid, but will not be meaningful
	bool	HasPositionalInfo() const				{ return bPositionalInfo; }

	//called to get the position of damage. This will be the origin unless positional information has
	//been provided.
	const LTVector&	GetDamagePos() const			{ return vPos; }

	//called to get the direction of damage. This will be the origin unless positional information has
	//been provided.
	const LTVector&	GetDamageDir() const			{ return vDir; }

	//called to provide the associated positional information
	void	SetPositionalInfo(const LTVector& vInPos, const LTVector& vInDir)
	{
		bPositionalInfo = true;
		vDir = vInDir;
		vPos = vInPos;
	}

	//called to save this object to the provided stream
    void Save(ILTMessage_Write *pMsg);

	//called to load this object from the provided stream
    void Load(ILTMessage_Read *pMsg);
 
	//a constant value that represents infinite damage should be applied
	const static float kInfiniteDamage;

	//called to construct this object from a message packet
	bool InitFromMessage(ILTMessage_Read *pMsg)
	{
		Load(pMsg);
		return true;
	}

	//called to send all of the data associated with this structure to the designated victim. The passed
	//in damager will act as the sender of the message, although this may be actually different than the
	//damager object handle that is provided.
	bool DoDamage(HOBJECT hDamager, HOBJECT hVictim);

	//Handler if damager is deleted, in which case we want to clear ourselves out so as not to apply any more
	//damage
	virtual void OnLinkBroken( LTObjRefNotifier *pRef, HOBJECT /*hOb*/ )
	{
		if( pRef == &hDamager )
		{
			Clear( );
		}
	}

	//the type of damage that this is
	DamageType			eType;

	//the object that we are supposed to be damaging. When this goes away, we clear ourselves out
	LTObjRefNotifier	hDamager;

	//the container that is causing the damage if it is coming from a container (i.e. lava)
	LTObjRef			hContainer;

	//the amount of damage to apply over the course of the lifetime
	float				fDamage;

	//the percentage of damage that penetrates armor
	float				fPenetration;

	//the duration to apply the damage for, only relevant for progressive damage
	float				fDuration;	

	//the type of ammo that was used to cause this damage if it came from a projectile
	HAMMO				hAmmo;

	//the amount of physical impulse force caused by this damage 
	float				fImpulseForce;

private:

	//flag indicating whether or not the positional data has been specified for this damage, if it
	//has, the associated direction and position will have meaning, otherwise they will just be defaults
	bool				bPositionalInfo;

	//the direction the damage came from, always valid, but only useful if the positional information
	//flag is set
	LTVector			vDir;

	//the position of impact that the damage came from, always valid, but only useful if the 
	//positional information flag is set
	LTVector			vPos;
};


class CDestructible : public IAggregate
{
	public :

		CDestructible();
		virtual ~CDestructible();

		bool	Init(HOBJECT hObject);

		void	Reset(float fHitPts, float fArmorPts);
		bool	Repair(float fAmount);
		bool	Heal(float fAmount);

		void	ClearProgressiveDamage( DamageType DT = DT_INVALID, bool bUpdateProgressiveDamage = true );
		void	ClearProgressiveDamage( DamageFlags nDamageFlags );
		bool	IsTakingProgressiveDamage( DamageType DT );

		virtual void HandleDestruction(HOBJECT hDamager);

		void SetMass(float fMass);
		void SetMaxHitPoints(float fmhp)				{ m_fMaxHitPoints = fmhp; }
		void SetHitPoints(float fhp)					{ m_fHitPoints = fhp; }
		void SetMaxArmorPoints(float fmap)				{ m_fMaxArmorPoints = fmap; }
		void SetArmorPoints(float fap)					{ m_fArmorPoints = fap; }
		void SetDamagePercentCommand(float fper)		{ m_fDamagePercentCommand = (fper < 0.0f ? 0.0f : (fper > 1.0f ? 1.0f : fper)); }
		void SetCanHeal(bool bCanHeal)					{ m_DestructibleFlags = ( m_DestructibleFlags & ~kDestructibleFlag_CanHeal ) | (( bCanHeal ) ? kDestructibleFlag_CanHeal : 0 ); }
		void SetCanRepair(bool bCanRepair)				{ m_DestructibleFlags = ( m_DestructibleFlags & ~kDestructibleFlag_CanRepair ) | (( bCanRepair ) ? kDestructibleFlag_CanRepair : 0 ); }
		void SetCanDamage(bool bCanDamage)				{ m_DestructibleFlags = ( m_DestructibleFlags & ~kDestructibleFlag_CanDamage ) | (( bCanDamage ) ? kDestructibleFlag_CanDamage : 0 ); }
		void SetNeverDestroy(bool bNeverDestroy)		{ m_DestructibleFlags = ( m_DestructibleFlags & ~kDestructibleFlag_NeverDestroy ) | (( bNeverDestroy ) ? kDestructibleFlag_NeverDestroy : 0 ); }
		void SetDead( bool bDead )						{ m_DestructibleFlags = ( m_DestructibleFlags & ~kDestructibleFlag_Dead ) | (( bDead ) ? kDestructibleFlag_Dead : 0 ); }
		void AddPreventDestructionRef()					{ ++m_iPreventDestructionCount; }
		void ReleaseAllowDestructionRef()				{ --m_iPreventDestructionCount; LTASSERT(m_iPreventDestructionCount >= 0, "AddPreventDestructionRef : m_iPreventDestructionCount is less than 0."); }
		void SetCantDamageFlags(DamageFlags nFlags)	{ m_nCantDamageFlags = nFlags; }
		DamageFlags	GetCantDamageFlags() const			{ return m_nCantDamageFlags; }
        void ClearCantDamageFlags(DamageFlags nTypes){ m_nCantDamageFlags &= ~nTypes; }
		bool IsCantDamageType(DamageType dt)			{ return !!(m_nCantDamageFlags & DamageTypeToFlag(dt)); }

		bool		CanHeal()				const { return (( m_DestructibleFlags & kDestructibleFlag_CanHeal ) != 0 ); }
		bool		CanRepair()				const { return (( m_DestructibleFlags & kDestructibleFlag_CanRepair ) != 0 ); }
		bool		GetCanDamage()			const { return (( m_DestructibleFlags & kDestructibleFlag_CanDamage ) != 0 ); }
		float		GetMass()				const { return m_fMass; }
		float		GetHitPoints()			const { return m_fHitPoints; }
		float		GetArmorPoints()		const { return m_fArmorPoints; }
		bool		IsDead()				const { return (( m_DestructibleFlags & kDestructibleFlag_Dead ) != 0 ); }
		DamageType	GetDeathType()			const { return m_eDeathType; }
		DamageType	GetLastDamageType()		const { return m_eLastDamageType; }
		float		GetDeathDamage()		const { return m_fDeathDamage; }
		float		GetLastDamage()			const { return m_fLastDamage; }
		double		GetLastDamageTime()		const { return m_nLastDamageTimeMS / 1000.0f; }
		float		GetLastArmorAbsorb()	const { return m_fLastArmorAbsorb; }
        LTVector    GetDeathDir()           const { return m_vDeathDir; }
		float		GetDeathImpulseForce()  const { return m_fDeathImpulseForce; }
		float		GetDeathHitNodeImpulseForceScale()  const { return m_fDeathHitNodeImpulseForceScale; }
        LTVector    GetLastDamageDir()      const { return m_vLastDamageDir; }
		float		GetMaxHitPoints()		const;
		float		GetMaxArmorPoints()		const;
		HOBJECT		GetLastDamager()		const { return m_hLastDamager; }

		bool		GetNeverDestroy()		const { return (( m_DestructibleFlags & kDestructibleFlag_NeverDestroy ) != 0 ); }
		bool		GetIsPlayer( ) const	{ return (( m_DestructibleFlags & kDestructibleFlag_IsPlayer ) != 0 ); }

		bool		RegisterFilterFunction( DamageFilterFunction pFn, GameBase *pObject );

		HAMMO		GetDeathAmmo( )			const { return m_hDeathAmmo; }

	protected :

		uint32	EngineMessageFn(LPBASECLASS pObject, uint32 messageID, void *pData, float lData);
        uint32	ObjectMessageFn(LPBASECLASS pObject, HOBJECT hSender, ILTMessage_Read *pMsg);

		void	SetBlockingPriority();
		float	ProcessGear(float fDamage, DamageType eDamageType);
		void	ProcessPlayerDeath(HOBJECT hDamager, HAMMO hAmmo);			// This has nothing to do with PlayerDeathTriggerTarget/Message!!!
		bool	ArmorCanAbsorb(DamageType eType);
		void	SetLastDamager(HOBJECT hDamager);
		
		float	AdjustDamage( float fDamage, HOBJECT hDamager );	

		LTObjRef	m_hObject;			// The object I'm associated with.
		LTObjRef	m_hLastDamager;		// Our last damager

	private :

		DamageType	m_eDeathType;			// How did we die
		DamageType	m_eLastDamageType;		// What damaged us last
		float		m_fLastDamage;			// Amount of last damage done
		float		m_fLastArmorAbsorb;		// Amount of last damage absorbed by armor
		uint32		m_nLastDamageTimeMS;		// Time last damage was done
		float		m_fDeathDamage;			// Amount of damage done on death blow
		HAMMO		m_hDeathAmmo;			// Ammo type that caused death
    
		LTVector    m_vDeathDir;			// Direction death came from
        LTVector    m_vLastDamageDir;		// Direction of last damage

		float		m_fDeathImpulseForce;	// Amount of physical force done by death blow
		float		m_fAccFrameImpulseForce;// Amount of physical force done during the current frame
		uint32		m_nCurFrameTimeMS;		// Current frame time
		float		m_fDeathHitNodeImpulseForceScale;// Amount m_fDeathImpulseForce should be scaled when applied to the hit node

		float		m_fMass;
		float		m_fMaxHitPoints;
		float		m_fHitPoints;
		float		m_fMaxArmorPoints;
		float		m_fArmorPoints;
		float		m_fDamagePercentCommand;

		enum DestructibleFlags
		{
			kDestructibleFlag_IsPlayer			= (1<<0),
			kDestructibleFlag_Dead				= (1<<1),
			kDestructibleFlag_CanHeal			= (1<<2),
			kDestructibleFlag_CanRepair			= (1<<3),
			kDestructibleFlag_CanDamage			= (1<<4),
			kDestructibleFlag_NeverDestroy		= (1<<5),
		};
		uint8 m_DestructibleFlags;

		int			m_iPreventDestructionCount;

		char const* m_pszDamageCommand;
		char const* m_pszDamagerMessage;
		char const* m_pszDeathCommand;
		char const* m_pszPlayerDeathCommand;	// Like DeathTrigger, but only sent if PLAYER killed us
		char const* m_pszKillerMessage;
		char const* m_pszDamagePercentCommand;
		char const* m_pszPlayerDamageCommand;

        uint32		m_nDamageTriggerCounter;    // How many times we have to be damaged before we send the trigger
        uint32		m_nDamageTriggerNumSends;   // Number of times to send damage trigger message
        DamageFlags	m_nCantDamageFlags;        // What can't damage us

		// Progressive damage being done...

		typedef std::vector<DamageStruct> ProgressiveDamageList;
		ProgressiveDamageList m_ProgressiveDamage;

		DamageFilterFunction m_pDamageFilterFunction;
		GameBase *m_pDamageFilterObject;

	private :

		bool ReadProp(LPBASECLASS pObject, const GenericPropList *pProps);
		void InitialUpdate(LPBASECLASS pObject);
		void Update();

		void HandleDamage(LPBASECLASS pObject, HOBJECT hSender, ILTMessage_Read *pMsg);
		void HandleHeal(LPBASECLASS pObject, HOBJECT hSender, ILTMessage_Read *pMsg);
		void HandleRepair(LPBASECLASS pObject, HOBJECT hSender, ILTMessage_Read *pMsg);
        void Save(ILTMessage_Write *pMsg, uint8 nType);
        void Load(ILTMessage_Read *pMsg, uint8 nType);
		void DoActualHealing(float fAmount);

		void DoDamage(DamageStruct & damage);
		void UpdateProgressiveDamage();
		void AddProgressiveDamage(DamageStruct & damage);

		bool DebugDamageOn();


		// Message Handlers...

		DECLARE_MSG_HANDLER( CDestructible, HandleDestroyMsg );
		DECLARE_MSG_HANDLER( CDestructible, HandleNeverDestroyMsg );
		DECLARE_MSG_HANDLER( CDestructible, HandleResetMsg );
};

inline bool CDestructible::ArmorCanAbsorb(DamageType eType)
{
	// [KLS 5/15/02] Updated so armor won't ignore melee damage...
	return ( (eType == DT_BULLET) || 
			(eType == DT_EXPLODE) ||
			(eType == DT_HELMET_PIERCING) ||
			(eType == DT_ENERGY) ||
			(eType == DT_SUPERNATURAL) ||
			 (eType == DT_MELEE) );
}

class CDestructiblePlugin : public IObjectPlugin
{
	public :

		virtual LTRESULT PreHook_PropChanged( const	char *szObjName,
											  const	char *szPropName,
											  const	int nPropType,
											  const	GenericProp	&gpPropValue,
											  ILTPreInterface *pInterface,
											  const	char *szModifiers );

		//called by WorldEdit whenever the a string list associated with this object needs to be filled in
		virtual LTRESULT PreHook_EditStringList( const char *szRezPath,
												 const char *szPropName, 
												 char **aszStrings, 
												 uint32 *pcStrings, 
												 const uint32 cMaxStrings, 
												 const uint32 cMaxStringLength );

	protected :

		CCommandMgrPlugin m_CommandMgrPlugin;
};

#endif // __DESTRUCTIBLE_H__
