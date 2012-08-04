// ----------------------------------------------------------------------- //
//
// MODULE  : AIWeaponAbstract.h
//
// PURPOSE : Abstract AIWeapon Declaration
//
// CREATED : 10/9/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef AIWEAPONABSTRACT_H
#define AIWEAPONABSTRACT_H

#include "AIWeaponMgr.h"
#include "AIClassFactory.h"


#define FIRE_MISS	true


enum ENUM_AIFiringState
{
	kAIFiringState_None,
	kAIFiringState_Aiming,
	kAIFiringState_Firing,
	kAIFiringState_CineFiring,
	kAIFiringState_Reloading,
	kAIFiringState_Throwing,
};


class CAIWeaponAbstract : public CAIClassAbstract
{
public:
	DECLARE_AI_FACTORY_CLASS_ABSTRACT_SPECIFIC( WeaponClass );

	CAIWeaponAbstract();

	virtual void		Save(ILTMessage_Write *pMsg);
	virtual void		Load(ILTMessage_Read *pMsg);

	// Queries:

	HWEAPON GetWeaponRecord();

	virtual ENUM_RangeStatus GetRangeStatus(
		const LTVector& vSourcePos,
		const LTVector& vDestPos) const = 0;

	virtual bool IsAIWeaponReadyToFire() { return true; }

	// Actions:

	virtual void Init(CWeapon* pWeapon, CAI* pAI) = 0;
	virtual void Deselect(CAI*) = 0;
	virtual void Update(CAI*) = 0;
	virtual void UpdateAnimation(CAI*) = 0;
	virtual void Aim(CAI*) = 0;
	virtual void Fire(CAI*) = 0;
	virtual void Reload(CAI*) = 0;
	virtual void HandleModelString( CAI* pAI, const CParsedMsg& cParsedMsg ) = 0;
	virtual LTVector GetFirePosition(CAI* pAI) = 0;
	HMODELSOCKET GetWeaponSocket() const { return m_hWeaponSocket; }
	bool CanDropWeapon() const { return m_bCanDropWeapon; }
	void Cinefire(CAI*);

protected:
	// Private to prevent copy, as it is currently unsupported.
	CAIWeaponAbstract(const CAIWeaponAbstract&);
	CAIWeaponAbstract& operator=(CAIWeaponAbstract& rhs);

	void	UpdateWeaponAnimation( CAI* pAI );
	HMODELANIM	SyncWeaponAnimation( CAI* pAI, HMODELANIM hLastUserAnimation );

	// Default Utility functions which can be used directly.  These 
	// represent the original system and default behaviors.
	float GetPerturbScale() { return 4.0f; }

	void DefaultHandleModelString( CAI* pAI, const CParsedMsg& cParsedMsg );
	void DefaultReload(CAI* pAI);
	bool DefaultInit(CWeapon* pWeapon, CAI* pAI);
	bool DefaultFire(CAI* pAI, const LTVector& vTargetPos, bool bAnimatesReload);
	bool DefaultThrow(CAI* pAI);
	void DefaultAim(CAI* pAI);
	LTVector DefaultGetFirePosition(CAI* pAI);
	void DefaultCalculateBurst( CAI* pAI, double* pfOutBurstInterval, int* pnOutBurstShots );

	struct AimContext
	{
		AimContext() : 
			m_iHit(0),
			m_iMiss(0),
			m_cHits(0),
			m_cMisses(0)
		{
		}

		uint32	m_iHit;
		uint32	m_iMiss;
		uint32	m_cHits;
		uint32	m_cMisses;
	};

	virtual bool GetShootPosition( CAI* pAI, AimContext& Context, LTVector& outvShootPos );
	void GetBlindFirePosition(CAI* pAI, LTVector& outvShootPos, bool bMiss );

	CWeapon*			m_pWeapon;
	std::string			m_szFireSocketName;
	ENUM_AIFiringState	m_eFiringState;
	uint32				m_iAnimRandomSeed;
	double				m_fRandomSeedSelectionTime;
	float				m_flWeaponContextInaccuracyScalar;
	HMODELSOCKET		m_hWeaponSocket;
	bool				m_bCanDropWeapon;

	const AIDB_AIWeaponRecord* m_pAIWeaponRecord;
};

#endif // AIWEAPONABSTRACT_H
