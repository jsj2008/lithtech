// ----------------------------------------------------------------------- //
//
// MODULE  : HandWeaponModel.h
//
// PURPOSE : Player view weapon model definition
//
// CREATED : 1/15/98
//
// ----------------------------------------------------------------------- //

#ifndef __HANDWEAPONMODEL_H__
#define __HANDWEAPONMODEL_H__

#include "cpp_engineobjects_de.h"
#include "cpp_server_de.h"

typedef struct HandModelData_t
{

	DDWORD	dwClientID;
	DBOOL	bLeftHand;

}	HandModelData;

class CWeapon;

class CHandWeaponModel : public BaseClass
{
	public :

		CHandWeaponModel();
		virtual ~CHandWeaponModel();
		void	SetPlayerOwned( DBOOL bOwnedByPlayer ) { m_bOwnedByPlayer = bOwnedByPlayer; }
		void	SetWeaponOwner( CWeapon *pWeapon ) { m_pWeaponOwner = pWeapon; }
		void	SetVisible(DBOOL bVisible);
		void	SetType(DDWORD dwType) { m_dwType = dwType; }
		void	SetClient(HCLIENT hClient, DBOOL bLeftHand) { m_hClient = hClient; m_bLeftHand = bLeftHand; }
		void	Drop();
		DBOOL	Update();

	protected :

		DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData);

	private :

		DBOOL	m_bDropped;
		DDWORD	m_dwType;
		DBOOL	m_bOwnedByPlayer;
		DBOOL	m_bLeftHand;
		HCLIENT	m_hClient;
		DBOOL	m_bFirstUpdate;
		DBOOL	m_bVisible;
		CWeapon *m_pWeaponOwner;
		DDWORD	m_dwClientID;
};


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CHandWeaponModel::SetVisible()
//
//	PURPOSE:	Sets or clears the visible flag
//
// ----------------------------------------------------------------------- //

inline void CHandWeaponModel::SetVisible(DBOOL bVisible)
{
	m_bVisible = bVisible;
}


#endif // __HANDWEAPONMODEL_H__

