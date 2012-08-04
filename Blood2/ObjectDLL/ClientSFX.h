// ----------------------------------------------------------------------- //
//
// MODULE  : CClientSFX.h
//
// PURPOSE : CClientSFX - Base class for client-side sfx objects - Definition
//
// CREATED : 10/21/97
//
// ----------------------------------------------------------------------- //

#ifndef __CLIENT_SFX_H__
#define __CLIENT_SFX_H__

#include "cpp_engineobjects_de.h"
#include "B2BaseClass.h"


class CClientSFX : public B2BaseClass
{
	public :

		CClientSFX(DBYTE nType=OT_NORMAL);
		virtual ~CClientSFX() {}

	protected :
		DDWORD		EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData);
};

#endif // __CLIENT_SFX_H__
