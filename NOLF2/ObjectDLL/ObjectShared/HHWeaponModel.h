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
#include "GameBase.h"

class CWeapon;
struct WEAPON;

LINKTO_MODULE( HHWeaponModel );

class CHHWeaponModel : public GameBase
{
	public :

		CHHWeaponModel();
		virtual ~CHHWeaponModel();

		void Setup(CWeapon* pParent, WEAPON const *pWeaponData);

		CWeapon* GetParent() const { return m_pParentWeapon; }
		
		void KillLoopSound();

	protected :

        uint32	EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);
		void	Update();

	private :

		void SetParent(CWeapon* pParent);

		CWeapon*	m_pParentWeapon;
		LTObjRef	m_hParentObject;	// The object associated with our parent

		HLTSOUND    m_hLoopSound;
		uint8       m_nLoopSoundId;
		
		void	InitialUpdate();
		void	StringKey(ArgList* pArgList);
};

#endif // __HH_WEAPON_MODEL_H__