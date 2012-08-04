// ----------------------------------------------------------------------- //
//
// MODULE  : CPVWeaponModel.h
//
// PURPOSE : CPVWeaponModel definition
//
// CREATED : 10/31/97
//
// ----------------------------------------------------------------------- //

#ifndef __PV_WEAPON_MODEL_H__
#define __PV_WEAPON_MODEL_H__

#include "cpp_engineobjects_de.h"

class CWeapon;

class CPVWeaponModel : public BaseClass
{
	public :

		CPVWeaponModel();
		virtual ~CPVWeaponModel();

		void SetParent(CWeapon* pParent);
		CWeapon* GetParent()     const { return m_pParent; }

		DVector GetFlashOffset() const { return m_vFlashOffset; }

		void Fire();

		
	protected :

		DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData);

		void CreateFlash();
		void RemoveFlash();
		void StartFlash();
		DBOOL UpdateFlash();

		void AttachFlash();
		void SetupParent();

	private :

		CWeapon*	m_pParent;			// Weapon we're associated with
		HOBJECT		m_hParentObject;	// The object associated with our parent		
		HOBJECT		m_hFlashObject;		// Muzzle flash object
		DFLOAT		m_fFlashStartTime;	// When did flash start
		DBOOL		m_bFired;			// Did we fire the weapon?
		DVector		m_vFlashOffset;		// Flash offset from model origin

		void Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags);

		void	InitialUpdate();
		void	Update();
		void	StringKey(ArgList* pArgList);
};

#endif // __PV_WEAPON_MODEL_H__

