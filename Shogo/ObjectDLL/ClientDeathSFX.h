// ----------------------------------------------------------------------- //
//
// MODULE  : CClientDeathSFX.h
//
// PURPOSE : CClientDeathSFX - Definition
//
// CREATED : 6/15/98
//
// ----------------------------------------------------------------------- //

#ifndef __CLIENT_DEATH_SFX_H__
#define __CLIENT_DEATH_SFX_H__

#include "cpp_server_de.h"
#include <memory.h>  // for memset


struct CLIENTDEATHFX
{
	CLIENTDEATHFX::CLIENTDEATHFX();
	
	DBYTE	nModelId;
	DBYTE	nSize;
	DBYTE	nDeathType;
	DBYTE	nCharClass;

	DVector vPos;
	DVector	vDeathDir;
};

inline CLIENTDEATHFX::CLIENTDEATHFX()
{
	memset(this, 0, sizeof(CLIENTDEATHFX));
}


void CreateClientDeathFX(CLIENTDEATHFX & fxStruct);


#endif // __CLIENT_DEATH_SFX_H__
