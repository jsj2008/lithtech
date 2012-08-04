//----------------------------------------------------------
//
// MODULE  : Destructable.h
//
// PURPOSE : Destructable class
//
// CREATED : 9/23/97
//
//----------------------------------------------------------

#ifndef __DESTRUCTABLE_H
#define __DESTRUCTABLE_H

#include "cpp_aggregate_de.h"
#include "cpp_engineobjects_de.h"
#include "DLink.h"

// Use ADD_DESTRUCTABLE_AGGREGATE() in your class definition to enable
// the following properties in the editor.  For example:
//
//BEGIN_CLASS(CMyCoolObj)
//	ADD_DESTRUCTABLE_AGGREGATE()
//	ADD_STRINGPROP(Filename, "")
//  ...
//

#define ADD_DESTRUCTABLE_AGGREGATE() \
	ADD_STRINGPROP(DamageTriggerTarget, "") \
	ADD_STRINGPROP(DamageTriggerMessage, "") \
	ADD_STRINGPROP(DeathTriggerTarget, "") \
	ADD_STRINGPROP(DeathTriggerMessage, "") \
	ADD_BOOLPROP(TriggerDestroyOnly, DFALSE) \
	ADD_STRINGPROP(SpawnObject, "") \
	ADD_VECTORPROP(SpawnObjectVel) \
	ADD_BOOLPROP(CanDamage, DTRUE)

class CBaseCharacter;
class CInventoryMgr;
class CAnim_Sound;

class CDestructable : public Aggregate
{
	public :

		CDestructable();
		virtual ~CDestructable();

		DBOOL	Init(HOBJECT hObject, CInventoryMgr* pInv = DNULL, CAnim_Sound* pAnimSound = DNULL);
		HOBJECT	GetObject()  { return m_hObject; }
		
		void	Reset();
		DBOOL	AddWard(DFLOAT fAmount);
		DBOOL	AddNecroWard(DFLOAT fAmount);
		DBOOL	Heal(DFLOAT fAmount);
		DBOOL	MegaHeal(DFLOAT fAmount);

		// @cmember Handle object destruction
		virtual void HandleDestruction();

		void	SetMass(DFLOAT fMass);
		void	SetMaxHitPoints(DFLOAT fmhp)	{ m_fMaxHitPoints = fmhp; }
		void	SetMaxMegaHitPoints(DFLOAT fmhp) { m_fMaxMegaHitPoints = fmhp; }
		void	SetHitPoints(DFLOAT  fhp)		{ if (m_fHitPoints = fhp) m_bDead = DFALSE; }
		void	SetMaxArmorPoints(DFLOAT fmap)	{ m_fMaxArmorPoints = fmap; }
		void	SetMaxNecroArmorPoints(DFLOAT fmnwp) { m_fMaxNecroArmorPoints = fmnwp; }
		void	SetArmorPoints(DFLOAT fap)		{ m_fArmorPoints = fap; }
		void	CalculateResistance(DBYTE nResistValue);
		void	SetApplyDamagePhysics(DBOOL bApply) { m_bApplyDamagePhysics = bApply; }
		
		void	SetDestructable( DBOOL bDestructable ) { m_bDestructable = bDestructable; }
		DBOOL	IsDestructable( ) const { return m_bDestructable; }

		DFLOAT	GetMass()			    const { return m_fMass; }
		DFLOAT	GetMaxHitPoints()	    const { return m_fMaxHitPoints; }
		DFLOAT	GetHitPoints()		    const { return m_fHitPoints; }
		DFLOAT	GetDeathHitPoints()	    const { return m_fDeathHitPoints; }
		DFLOAT	GetMaxArmorPoints()	    const { return m_fMaxArmorPoints; }
		DFLOAT	GetArmorPoints()		const { return m_fArmorPoints; }
		DBYTE	GetLastDamageType()	    const { return m_nLastDamageType; }
		DBOOL	GetApplyDamagePhysics() const { return m_bApplyDamagePhysics; }
        
		int		GetSideHit()			const { return m_nSideHit; }
		int		GetNodeHit()			const { return m_nNodeHit; }

		DBYTE	GetLastDamageLocation()	const { return m_nLastDamageLocation; }
		DFLOAT	GetLastDamagePercent()	const { return m_fLastDamagePercent; }
		DFLOAT	GetLastDamageAmount() const { return m_fLastDamageAmount; }
		void	GetLastDamageDirection(DVector *vDir) { VEC_COPY(*vDir, m_vLastDamageDirection); }

		void	SetDead(DBOOL bDead)			{m_bDead = bDead;}
		DBOOL	IsDead()				const { return (m_fDeathDelay > 0) ? DFALSE : m_bDead; }
		void	SetGodMode(DBOOL bGodMode)	{ m_bGodMode = bGodMode; }

		void	SetNighInvulnerable(DBOOL bValue) { m_bNighInvulnerable = bValue; }

		HOBJECT GetWhoKilledMeLast() const { return m_hWhoKilledMeLast; }
		HOBJECT GetLastDamager() const	{ return m_hLastDamager; }
		void	ClearLastDamager()		{ m_hLastDamager = DNULL; }

		void	SetDeathDelay(DFLOAT fDelay) { m_fDeathDelay = fDelay; }
		
		//efficient means to track destructable objects in a level
		static DLink	m_DestructableHead;
		static DDWORD	m_dwNumDestructables;
		DLink			m_Link;

		DBOOL	IsAddedVelocity() const { return m_bAddVelocity; }
		void	GetAddedVelocity(DVector *pVel) 
				{ 
					VEC_COPY(*pVel, m_vAddVelocity); 
					VEC_INIT(m_vAddVelocity); 
					m_bAddVelocity = DFALSE; 
				}

		DBOOL   IsPlayerToPlayerDamageOk(HOBJECT hDamager, HOBJECT hOwner);
	
		void	SetStartingCharacterValues();

	protected:

		DDWORD	EngineMessageFn(LPBASECLASS pObject, DDWORD messageID, void *pData, DFLOAT lData);
		DDWORD	ObjectMessageFn(LPBASECLASS pObject, HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);
		void	Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void	Load(HMESSAGEREAD hWrite, DDWORD dwLoadFlags);
	
	private:

		// Handle reading the destructable's properties...
		DBOOL ReadProp(LPBASECLASS pObject, ObjectCreateStruct *pStruct);

		void HandleDamage(HOBJECT hSender, HMESSAGEREAD hRead);
		void ApplyDamagePhysics(DFLOAT fDamage, DVector *pvDir);
		void HandleHeal(HOBJECT hSender, HMESSAGEREAD hRead);
		void HandleRepair(HOBJECT hSender, HMESSAGEREAD hRead);
		void HandleTrigger(HOBJECT hSender, HMESSAGEREAD hRead);

		int	 SetProperNode(int nNode);
		int	 CalculateHitLimb(DVector vDir, DVector vPos, DFLOAT fDamage = 10.0f);

		// pointer to the owner's object (if it's a base character)
//		CBaseCharacter*	m_pOwner;

		// @cmember should we apply physics to object when damaged?
		DBOOL m_bApplyDamagePhysics;

		// @cmember mass of the object
		DFLOAT m_fMass;

		// @cmember Maximum damage this thing can take
		DFLOAT m_fMaxHitPoints;

		// @cmember Maximum damage this thing can take
		DFLOAT m_fMaxMegaHitPoints;

		// @cmember current strength of the item
		DFLOAT m_fHitPoints;

		// @cmember current strength of the item
		DFLOAT m_fDeathHitPoints;

		// @cmember Maximum amount of armor on this thing
		DFLOAT m_fMaxArmorPoints;

		// @cmember Maximum amount of armor on this thing
		DFLOAT m_fMaxNecroArmorPoints;

		// @cmember current amount of armor on this thing
		DFLOAT m_fArmorPoints;

		// @cmember player's resistance value
		DFLOAT m_fResistance;

		// @cmember the object I am associated with
		HOBJECT	m_hObject; 

		// @cmember a flag to tell me if I've been destroyed
		DBOOL	m_bDead;
		DBOOL	m_bNighInvulnerable;

		DBYTE	m_nLastDamageType;
		DBYTE	m_nLastDamageLocation;
		DVector	m_vLastDamageDirection;
		DFLOAT	m_fLastDamageAmount;
	    DFLOAT  m_fLastDamagePercent;       // Percentage of Damage

		HOBJECT	m_hWhoKilledMeLast;

		HSTRING	m_hstrDamageTriggerTarget;
		HSTRING	m_hstrDamageTriggerMessage;
		HSTRING	m_hstrDeathTriggerTarget;
		HSTRING m_hstrDeathTriggerMessage;

		HSTRING m_hstrSpawnObject;
		DVector m_vSpawnObjectVel;

		DBOOL	m_bGodMode;
		DBOOL	m_bTriggerOnly;

		DFLOAT	m_fDeathDelay;

		//for characters/AI
		CInventoryMgr*	m_pInventoryMgr;
		CAnim_Sound*		m_pAnim_Sound;

		HOBJECT		m_hLastDamager;
		int			m_nSideHit;					// 0=front,6=back
		int			m_nNodeHit;					

		DBOOL		m_bAddVelocity;
		DVector		m_vAddVelocity;

		DBOOL		m_bDestructable;

};

#endif // __DESTRUCTABLE_H