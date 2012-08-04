// ----------------------------------------------------------------------- //
//
// MODULE  : CBouncer.h
//
// PURPOSE : CBouncer class definition - handle bouncing off things
//
// CREATED : 3/18/98
//
// ----------------------------------------------------------------------- //

#ifndef __BOUNCER_H__
#define __BOUNCER_H__

#include "cpp_aggregate_de.h"
#include "cpp_engineobjects_de.h"


class CBouncer : public Aggregate
{
	public :

		CBouncer();
		virtual ~CBouncer();
	
		DBOOL IsDoneBouncing() const { return m_bDoneBouncing; }

		void SetBounceSound(char* pSound);
		void SetBounceSound2(char* pSound);
		void SetBounceCount(int nCount)		{ m_nBounceCount = nCount; }

	protected :

		DDWORD EngineMessageFn(LPBASECLASS pObject, DDWORD messageID, void *pData, DFLOAT lData);

	private :

		DFLOAT	m_fPitchVel;
		DFLOAT	m_fYawVel;
		DFLOAT	m_fPitch;
		DFLOAT	m_fYaw;
		
		DDWORD	m_nBounceCount;
		HSTRING	m_hstrBounceSound;
		HSTRING	m_hstrBounceSound2;

		DVector	m_vLastPos;
		DBOOL	m_bDoneBouncing;

		void Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags);

		void InitialUpdate(LPBASECLASS pObject);
		void Update(LPBASECLASS pObject);
		void HandleTouch(LPBASECLASS pObject, HOBJECT hObj);
};

#endif // __BOUNCER_H__

