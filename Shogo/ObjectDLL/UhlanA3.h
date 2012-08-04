// ----------------------------------------------------------------------- //
//
// MODULE  : UhlanA3.h
//
// PURPOSE : UhlanA3 - Definition
//
// CREATED : 1/28/98
//
// ----------------------------------------------------------------------- //

#ifndef __UHLANA3_H__
#define __UHLANA3_H__

#include "cpp_engineobjects_de.h"
#include "Vehicle.h"


class UhlanA3 : public Vehicle
{
	public :

 		UhlanA3();

	protected :

		DDWORD	EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData);

		virtual void UpdateWeapon();
		virtual void UpdateAnimation();
		virtual void SetAnimationIndexes();

		virtual DVector GetFirePos(DVector* pvPos);

		HMODELANIM	m_hShredderAni;
		HMODELANIM	m_hRocketsAni;

		DBOOL		m_bSaveAllowmovement;
		DBOOL		m_bChangeAnimation;

	private :

		void Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags);
};

class UCA_UhlanA3 : public UhlanA3
{
	public :

 		UCA_UhlanA3();
};


class CMC_UhlanA3 : public UhlanA3
{
	public :

 		CMC_UhlanA3();
};

class FALLEN_UhlanA3 : public UhlanA3
{
	public :

 		FALLEN_UhlanA3();
};

class CRONIAN_UhlanA3 : public UhlanA3
{
	public :

 		CRONIAN_UhlanA3();
};

class SHOGO_UhlanA3 : public UhlanA3
{
	public :

 		SHOGO_UhlanA3();
};

#endif // __UHLANA3_H__
