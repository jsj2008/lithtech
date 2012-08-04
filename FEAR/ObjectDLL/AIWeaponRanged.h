// ----------------------------------------------------------------------- //
//
// MODULE  : AIWeaponRanged.h
//
// PURPOSE : CAIWeaponRanged class declaration
//
// CREATED : 10/9/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef CAIWeaponRanged_H
#define CAIWeaponRanged_H

#include "AIWeaponMgr.h"
#include "AIWeaponAbstract.h"

class AIWBM_AIWeaponTemplateRanged;

class CAIWeaponRanged : public CAIWeaponAbstract
{
	typedef CAIWeaponAbstract super;

public:
	DECLARE_AI_FACTORY_CLASS_SPECIFIC( WeaponClass, CAIWeaponRanged, kAIWeaponClassType_Ranged );

	CAIWeaponRanged();

	// AIWeaponAbstract implementation
	virtual void		Save(ILTMessage_Write *pMsg);
	virtual void		Load(ILTMessage_Read *pMsg);

	virtual ENUM_RangeStatus GetRangeStatus(const LTVector& vSourcePos, const LTVector& vDestPos) const
	{ return AIWeaponUtils::GetWeaponRangeStatus(m_pAIWeaponRecord, vSourcePos, vDestPos); }

	virtual bool IsAIWeaponReadyToFire();

	virtual void Init(CWeapon*, CAI*);
	virtual void Deselect(CAI* pAI);
	virtual void Update(CAI*);
	virtual void UpdateAnimation( CAI* pAI );
	virtual void Aim(CAI* pAI);
	virtual void Fire(CAI*);
	virtual void Reload(CAI* pAI){ DefaultReload(pAI); }
	virtual void HandleModelString( CAI* pAI, const CParsedMsg& cParsedMsg ) {DefaultHandleModelString(pAI, cParsedMsg ); }
	virtual LTVector GetFirePosition(CAI* pAI) { return DefaultGetFirePosition(pAI); }

private:
	// Private to prevent copy, as it is currently unsupported.
	CAIWeaponRanged(const CAIWeaponRanged&);
	CAIWeaponRanged& operator=(CAIWeaponRanged& rhs);

	void	CalculateBurst( CAI* pAI );
	void	ClearBurstInterval();
	void	RandomizeFireAnimation( CAI* pAI );

	double	m_fBurstInterval;
	int		m_nBurstShots;

	AimContext m_AimContext;

	HMODELANIM m_hLastUserAnimation;
};

#endif // CAIWeaponRanged_H
