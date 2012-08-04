// ----------------------------------------------------------------------- //
//
// MODULE  : AIActivityExchangeWeapons.h
//
// PURPOSE : 
//
// CREATED : 7/14/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AIACTIVITYEXCHANGEWEAPONS_H_
#define _AIACTIVITYEXCHANGEWEAPONS_H_

LINKTO_MODULE(AIActivityExchangeWeapons);

#include "AIActivityAbstract.h"

class CAI;
class WeaponItem;

// ----------------------------------------------------------------------- //
//
//	CLASS:		AIActivityExchangeWeapons
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

class CAIActivityExchangeWeapons : public CAIActivityAbstract
{
	typedef CAIActivityAbstract super;
	
public:
	DECLARE_AI_FACTORY_CLASS_SPECIFIC( Activity, CAIActivityExchangeWeapons, kActivity_ExchangeWeapons );

	// Ctor/Dtor

	CAIActivityExchangeWeapons();
	virtual ~CAIActivityExchangeWeapons();

	// Save/Load

	virtual void Load(ILTMessage_Read *pMsg);
	virtual void Save(ILTMessage_Write *pMsg);

	// CAIActivityAbstract overrides.

	virtual void	InitActivity();

	virtual bool	FindActivityParticipants();

	virtual bool	ActivateActivity();
	virtual void	DeactivateActivity();
	virtual bool	UpdateActivity();

	virtual void	ClearDeadAI();

private:
	PREVENT_OBJECT_COPYING(CAIActivityExchangeWeapons);

	bool			NeedsBetterWeapon(CAI* pAI, float flSquadWeaponPreferencePerEnemy ) const;
	WeaponItem*		GetBestWeaponItem( CAI* pAI ) const;
	int				GetWeaponImprovement( CAI* pAI, HRECORD hRecord ) const;
	float			CalculateWeaponPreferencePerEnemy() const;

	LTObjRef		m_hSelectedAI;
	LTObjRef		m_hSelectedWeaponItem;
};

#endif // _AIACTIVITYEXCHANGEWEAPONS_H_

