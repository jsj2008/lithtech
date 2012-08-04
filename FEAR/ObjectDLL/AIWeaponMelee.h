// ----------------------------------------------------------------------- //
//
// MODULE  : AIWeaponMelee.h
//
// PURPOSE : CAIWeaponMelee class declaration
//
// CREATED : 10/9/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef CAIWeaponMelee_H
#define CAIWeaponMelee_H

#include "AIWeaponMgr.h"
#include "AIWeaponAbstract.h"

class AIWBM_AIWeaponTemplateMelee;

class CAIWeaponMelee : public CAIWeaponAbstract
{
	typedef CAIWeaponAbstract super;

public:

	DECLARE_AI_FACTORY_CLASS_SPECIFIC( WeaponClass, CAIWeaponMelee, kAIWeaponClassType_Melee );

	CAIWeaponMelee();

	// AIWeaponAbstract implementation
	virtual void		Save(ILTMessage_Write *pMsg);
	virtual void		Load(ILTMessage_Read *pMsg);

	virtual ENUM_RangeStatus GetRangeStatus(const LTVector& vSourcePos, const LTVector& vDestPos) const
	{ return AIWeaponUtils::GetWeaponRangeStatus(m_pAIWeaponRecord, vSourcePos, vDestPos); }

	virtual void Init(CWeapon*, CAI*);
	virtual void Deselect(CAI*) {}
	virtual void Update(CAI*);
	virtual void UpdateAnimation( CAI* pAI );
	virtual void Aim(CAI* pAI);
	virtual void Fire(CAI*);
	virtual void Reload(CAI* pAI){ DefaultReload(pAI); }
	virtual void HandleModelString( CAI* pAI, const CParsedMsg& cParsedMsg );
	virtual LTVector GetFirePosition(CAI* pAI);
	virtual bool GetShootPosition( CAI* pAI, AimContext& Context, LTVector& outvShootPos );

private:
	void CalculateBurst(CAI*);

	// Private to prevent copy, as it is currently unsupported.
	CAIWeaponMelee(const CAIWeaponMelee&);
	CAIWeaponMelee& operator=(CAIWeaponMelee& rhs);

	double				m_fBurstInterval;
	int					m_nBurstShots;
	HMODELANIM			m_hLastUserAnimation;

	bool				m_bForceHit;

	AimContext m_Context;
};

#endif // CAIWeaponMelee_H
