
// ----------------------------------------------------------------------- //
//
// MODULE  : ClientDebugLine.h
//
// PURPOSE : ClientDebugLine - Definition
//
// CREATED : 12/7/97
//
// ----------------------------------------------------------------------- //

#ifndef __CLIENTDEBUGLINE_H__
#define __CLIENTDEBUGLINE_H__

#include "ClientSFX.h"


class CClientDebugLine : public CClientSFX
{
	public:

		void Setup(DVector *vFrom, DVector *vTo);

	protected :

		DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData);
};


#endif // __CLIENTDEBUGLINE_H__
