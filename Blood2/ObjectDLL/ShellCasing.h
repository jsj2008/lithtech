
#ifndef __SHELLCASING_H__
#define __SHELLCASING_H__



#include "cpp_engineobjects_de.h"
#include "cpp_server_de.h"
#include "dlink.h"


#define MAX_SHELLS	200


class CShellCasing : public BaseClass
{
	public :

		CShellCasing();
		virtual		~CShellCasing();

		void		Setup(HOBJECT hFiredFrom);
		void		Init(DBOOL bFiredFromLeft);

	public:

		static DLink	m_Head;
		static DDWORD	m_dwNumShells;

	protected :

		virtual		DDWORD ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);
		virtual		DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData);
		DBOOL		InitialUpdate();
		DBOOL		Update();
		void		HandleTouch(HOBJECT hObj);

	private:

		DFLOAT		m_fExpireTime;
		DVector		m_vLastPos;
		HOBJECT		m_hFiredFrom;
		
		DFLOAT		m_fPitchVel;
		DFLOAT		m_fYawVel;
		DFLOAT		m_fPitch;
		DFLOAT		m_fYaw;

		DLink		m_Link;

		DBOOL		m_bFiredFromLeft;
        DBOOL       m_bInVisible;
        int         m_nVisibleUpdate;
		int			m_nBounceCount;        
};


#endif  // __SHELLCASING_H__