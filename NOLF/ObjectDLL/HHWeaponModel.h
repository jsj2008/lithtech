// ----------------------------------------------------------------------- //
//
// MODULE  : CHHWeaponModel.h
//
// PURPOSE : CHHWeaponModel definition - Hand Held weapon model support
//
// CREATED : 10/31/97
//
// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HH_WEAPON_MODEL_H__
#define __HH_WEAPON_MODEL_H__

#include "ltengineobjects.h"

class CWeapon;

class CHHWeaponModel : public BaseClass
{
	public :

		CHHWeaponModel();
		virtual ~CHHWeaponModel();

		void Setup(CWeapon* pParent, WEAPON* pWeaponData);

		CWeapon* GetParent()     const { return m_pParent; }

	protected :

        uint32 EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);

	private :

		void SetParent(CWeapon* pParent);

		CWeapon*	m_pParent;			// Weapon we're associated with
		HOBJECT		m_hParentObject;	// The object associated with our parent

		void	InitialUpdate();
		void	StringKey(ArgList* pArgList);
};

#endif // __HH_WEAPON_MODEL_H__