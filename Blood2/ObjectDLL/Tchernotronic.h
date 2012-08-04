// ----------------------------------------------------------------------- //
//
// MODULE  : Tchernotronic.h
//
// PURPOSE : Tchernotronic - Definition
//
// CREATED : 5/11/98
//
// ----------------------------------------------------------------------- //

#ifndef __TCHERNOTRONIC_H__
#define __TCHERNOTRONIC_H__

#include "cpp_engineobjects_de.h"
#include "cpp_server_de.h"
#include "B2BaseClass.h"


class Tchernotronic : public B2BaseClass
{
	public :

		Tchernotronic();
		~Tchernotronic();

	protected :

		DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData);
		DDWORD ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);
	
	private :

		DBOOL	ReadProp(ObjectCreateStruct *pStruct);
		void	InitialUpdate(DVector *pMovement);
		DBOOL	Update(DVector *pMovement);
		void	TriggerMsg(HOBJECT hSender, HSTRING hMsg);
		DBOOL	PlayAnimation(DDWORD dwNewAni);
		void	AddExplosion(DVector &vPos);
		void	Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void	Load(HMESSAGEREAD hWrite, DDWORD dwLoadFlags);

		HSTRING	m_hstrTriggerTarget;
		HSTRING	m_hstrTriggerMessage;

		DDWORD	m_nRestAnim;
		DDWORD	m_nMovingAnim;
		DBOOL	m_bTimer;
		DFLOAT	m_fRemoveTime;
};

#endif // __TCHERNOTRONIC_H__
