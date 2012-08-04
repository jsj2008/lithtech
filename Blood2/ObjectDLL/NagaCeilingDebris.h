
#ifndef _NAGACEILINGDEBRIS_H
#define _NAGACEILINGDEBRIS_H

#include "b2baseclass.h"

class NagaCeilingDebris : B2BaseClass
{
	public:

		NagaCeilingDebris();
		~NagaCeilingDebris();

		DDWORD EngineMessageFn(DDWORD messageID, void *pData, float fData);
		void Update();

	protected:

		DFLOAT	m_fStartTime;
		DBOOL	m_bFall;

		DFLOAT	m_fXRot;
		DFLOAT	m_fYRot;
		DFLOAT	m_fZRot;
};

#endif