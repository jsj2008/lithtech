// ----------------------------------------------------------------------- //
//
// MODULE  : ViewWeaponModel.h
//
// PURPOSE : Player view weapon model definition
//
// CREATED : 1/15/98
//
// ----------------------------------------------------------------------- //

#ifndef __VIEWWEAPONMODEL_H__
#define __VIEWWEAPONMODEL_H__

#include "cpp_engineobjects_de.h"
#include "cpp_server_de.h"


class CWeapon;

class CViewWeaponModel : public BaseClass
{
	public :

		CViewWeaponModel();
		~CViewWeaponModel()
		{
			if( m_hLoopSound )
				g_pServerDE->KillSound( m_hLoopSound );
		}
		DBOOL	Init(CWeapon* pParent);
		void	UseModelKeys(DBOOL bUse);
		void	SetVisible(DBOOL bVisible);

	protected :

		DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData);

	private :

		CWeapon* m_pParent;		// Parent weapon object
		HSOUNDDE m_hLoopSound;	// Looping sound reference

		void	OnStringKey(ArgList* pArgList);
};


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CViewWeaponMode::UseModelKeys()
//
//	PURPOSE:	Sets whether or not to use modelkeys for this model.
//
// ----------------------------------------------------------------------- //

inline void CViewWeaponModel::UseModelKeys(DBOOL bUse)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hObject) return;

	DDWORD dwFlags = pServerDE->GetObjectFlags(m_hObject);

	if (bUse)
		dwFlags |= FLAG_MODELKEYS;
	else
		dwFlags &= ~FLAG_MODELKEYS;

	pServerDE->SetObjectFlags(m_hObject, dwFlags);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CViewWeaponModel::SetVisible()
//
//	PURPOSE:	Sets or clears the visible flag
//
// ----------------------------------------------------------------------- //

inline void CViewWeaponModel::SetVisible(DBOOL bVisible)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hObject) return;

	DDWORD dwFlags = pServerDE->GetObjectFlags(m_hObject);

	if (bVisible)
		dwFlags |= FLAG_VISIBLE;
	else
		dwFlags &= ~FLAG_VISIBLE;

	pServerDE->SetObjectFlags(m_hObject, dwFlags);
}


#endif // __VIEWWEAPONMODEL_H__

