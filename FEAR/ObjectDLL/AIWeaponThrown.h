// ----------------------------------------------------------------------- //
//
// MODULE  : AIWeaponThrown.h
//
// PURPOSE : Thrown Weapon Declaration
//
// CREATED : 10/9/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "AIWeaponAbstract.h"

class AIWBM_AIWeaponTemplateThrown;

class CAIWeaponThrown : public CAIWeaponAbstract
{
public:
	typedef CAIWeaponAbstract super;

	DECLARE_AI_FACTORY_CLASS_SPECIFIC( WeaponClass, CAIWeaponThrown, kAIWeaponClassType_Thrown );

	CAIWeaponThrown();

	// AIWeaponAbstract implementation
	virtual void		Save(ILTMessage_Write *pMsg);
	virtual void		Load(ILTMessage_Read *pMsg);

	virtual ENUM_RangeStatus GetRangeStatus(const LTVector& vSourcePos, const LTVector& vDestPos) const
	{ return AIWeaponUtils::GetWeaponRangeStatus(m_pAIWeaponRecord, vSourcePos, vDestPos); }

	virtual void Init(CWeapon* pWeapon, CAI* pAI);
	virtual void Deselect(CAI*) {}
	virtual void Update(CAI*);
	virtual void UpdateAnimation( CAI* pAI );
	virtual void Aim(CAI* pAI) { DefaultAim(pAI); }
	virtual void Fire(CAI* pAI);
	virtual void Reload(CAI* pAI){ DefaultReload(pAI); }
	virtual void HandleModelString( CAI* pAI, const CParsedMsg& cParsedMsg );
	virtual LTVector GetFirePosition(CAI* pAI);

protected:

	void	HideWeapon( bool bHide );

private:
	// Private to prevent copy, as it is currently unsupported.
	CAIWeaponThrown(const CAIWeaponThrown&);
	CAIWeaponThrown& operator=(CAIWeaponThrown& rhs);

private:

	bool	m_bHidden;

	HMODELANIM m_hLastUserAnimation;
};
