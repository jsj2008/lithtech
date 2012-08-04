// ----------------------------------------------------------------------- //
//
// MODULE  : TripLaser.h
//
// PURPOSE : TripLaser - Definition
//
// CREATED : 10/23/97
//
// ----------------------------------------------------------------------- //

#ifndef __TRIPLASER_H__
#define __TRIPLASER_H__

#include "cpp_engineobjects_de.h"
#include "cpp_server_de.h"
#include "B2BaseClass.h"


class TripLaser : public B2BaseClass
{
	public :

		TripLaser();
		~TripLaser();

	protected :

		DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData);
		DDWORD ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);
	
	private :

		DBOOL	ReadProp(ObjectCreateStruct *pStruct);
		void	Update();
		void	InitialUpdate();
		void	HandleTouch(HOBJECT hObj);
		void	TriggerMsg(HOBJECT hSender, HSTRING hMsg);
		void	Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void	Load(HMESSAGEREAD hWrite, DDWORD dwLoadFlags);

		DVector	m_vDims;
		DVector m_vColor;
		DFLOAT	m_fAlpha;
		DFLOAT	m_fDamage;
		DFLOAT	m_fDamageTime;

		HSTRING	m_hstrTriggerTarget;
		HSTRING	m_hstrTriggerMessage;

		HSTRING	m_hstrSound;
		HSTRING	m_hstrTouchSound;
		HSOUNDDE m_hSound;

		DVector	m_vEndPoints[2];	// End points of the laser
		DDWORD	m_dwFlags;
		DBOOL	m_bTriggered;
		DBOOL	m_bRemove;
};

#endif // __TRIPLASER_H__
