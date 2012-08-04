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
#include "DamageTypes.h"

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
	ADD_LONGINTPROP(DamageTriggerNumSends, 1) \
	ADD_STRINGPROP(DamagerMessage, "") \
	ADD_STRINGPROP(DeathTriggerTarget, "") \
	ADD_STRINGPROP(DeathTriggerMessage, "") \
	ADD_STRINGPROP(KillerMessage, "") \
	ADD_BOOLPROP(CanHeal, DTRUE) \
	ADD_BOOLPROP(CanRepair, DTRUE) \
	ADD_BOOLPROP(CanDamage, DTRUE) \
	ADD_BOOLPROP(NeverDestroy, DFALSE)


class CDestructable : public Aggregate
{
	public :

		CDestructable();
		virtual ~CDestructable();

		DBOOL Init(HOBJECT hObject);
		
		void Reset(DFLOAT fHitPts, DFLOAT fArmorPts);
		DBOOL Repair(DFLOAT fAmount);
		DBOOL Heal(DFLOAT fAmount);

		virtual void HandleDestruction( HOBJECT hDamager );

		void SetMass(DFLOAT fMass);
		void SetMaxHitPoints(DFLOAT fmhp)	{ m_fMaxHitPoints = fmhp; }
		void SetHitPoints(DFLOAT fhp)		{ m_fHitPoints = fhp; }
		void SetMaxArmorPoints(DFLOAT fmap)	{ m_fMaxArmorPoints = fmap; }
		void SetArmorPoints(DFLOAT fap)		{ m_fArmorPoints = fap; }
		void SetCanHeal(DBOOL bCanHeal)		{ m_bCanHeal = bCanHeal; }
		void SetCanRepair(DBOOL bCanRepair)	{ m_bCanRepair = bCanRepair; }
		void SetCanDamage(DBOOL bCanDamage) { m_bCanDamage = bCanDamage; }

		void SetApplyDamagePhysics(DBOOL b) { m_bApplyDamagePhysics = b; }
		void SetCantDamageTypes(DDWORD dwTypes) { m_dwCantDamageTypes = dwTypes; }

		DBOOL  GetCanHeal()			const { return m_bCanHeal; }
		DBOOL  GetCanRepair()		const { return m_bCanRepair; }
		DBOOL  GetCanDamage()		const { return m_bCanDamage; }
		DFLOAT GetMass()			const { return m_fMass; }
		DFLOAT GetHitPoints()		const { return m_fHitPoints; }
		DFLOAT GetArmorPoints()		const { return m_fArmorPoints; }
		DBOOL  IsDead()				const { return m_bDead; }
		DamageType GetDeathType()	const { return m_eDeathType; }
		DFLOAT GetDeathDamage()		const { return m_fDeathDamage; }
		DFLOAT GetLastDamage()		const { return m_fLastDamage; }
		DVector GetDeathDir()		const { return m_vDeathDir; }
		DVector GetLastDamageDir()	const { return m_vLastDamageDir; }
		DFLOAT GetMaxHitPoints()	const;
		DFLOAT GetMaxArmorPoints()	const;

	protected :

		DDWORD EngineMessageFn(LPBASECLASS pObject, DDWORD messageID, void *pData, DFLOAT lData);
		DDWORD ObjectMessageFn(LPBASECLASS pObject, HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);
	
		void   SetBlockingPriority();
		void   ApplyDamage(DVector vDir, DFLOAT fDamage, DamageType eType, HOBJECT hHeHitMe);
		DFLOAT ProcessPowerups(LPBASECLASS pObject, DFLOAT fDamage, DamageType eDamageType, HOBJECT hHeHitMe, DVector* pvDir);
		void   HandleRegen();

		HOBJECT		m_hObject;		// The object I'm associated with

	private :

		DamageType	m_eDeathType;	// How did we die
		DFLOAT		m_fLastDamage;  // Amount of last damage done
		DFLOAT		m_fDeathDamage;	// Amount of damage done on death blow
		DVector		m_vDeathDir;	// Direction death came from
		DVector		m_vLastDamageDir;  // Direction of last damage

		DBOOL		m_bApplyDamagePhysics;	// Rag doll?
		DFLOAT		m_fMass;
		DFLOAT		m_fMaxHitPoints;
		DFLOAT		m_fHitPoints;
		DFLOAT		m_fMaxArmorPoints;
		DFLOAT		m_fArmorPoints;
		DBOOL		m_bDead;
		
		DBOOL		m_bPlayer;
		DFLOAT		m_fNextRegen;

		// Process heal, repair, damage messages?
		DBOOL		m_bCanHeal;
		DBOOL		m_bCanRepair;
		DBOOL		m_bCanDamage;
		DBOOL		m_bNeverDestroy;

		HSTRING	m_hstrDamageTriggerTarget;
		HSTRING	m_hstrDamageTriggerMessage;
		HSTRING	m_hstrDamagerMessage;
		HSTRING	m_hstrDeathTriggerTarget;
		HSTRING m_hstrDeathTriggerMessage;
		HSTRING	m_hstrKillerMessage;

		DDWORD	m_nDamageTriggerNumSends;	// Number of times to send damage trigger message
		DDWORD	m_dwCantDamageTypes;		// What can't damage us

	private :

		// Handle reading the destructable's properties...
		DBOOL ReadProp(LPBASECLASS pObject, ObjectCreateStruct *pInfo);
		void InitialUpdate(LPBASECLASS pObject);
		void Update();

		void HandleTrigger(LPBASECLASS pObject, HOBJECT hSender, HMESSAGEREAD hRead);
		void HandleDamage(LPBASECLASS pObject, HOBJECT hSender, HMESSAGEREAD hRead);
		void HandleHeal(LPBASECLASS pObject, HOBJECT hSender, HMESSAGEREAD hRead);
		void HandleUltraHeal(LPBASECLASS pObject, HOBJECT hSender, HMESSAGEREAD hRead);
		void HandleRepair(LPBASECLASS pObject, HOBJECT hSender, HMESSAGEREAD hRead);
		void HandleCrush(LPBASECLASS pObject, HOBJECT hSender);
		void HandleTouch(LPBASECLASS pObject, HOBJECT hSender, DFLOAT fForce);
		void Save(HMESSAGEWRITE hWrite, DBYTE nType);
		void Load(HMESSAGEREAD hRead, DBYTE nType);
		void DoActualHealing(DFLOAT fAmount);
		void HandleCriticalHit(HOBJECT hHeHitMe, DFLOAT & fDamage);

		DBOOL DebugDamageOn();
};

#endif // __DESTRUCTABLE_H