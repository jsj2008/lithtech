// ----------------------------------------------------------------------- //
//
// MODULE  : CClientSFX.cpp
//
// PURPOSE : CClientSFX - Base class for client-side sfx objects - Implementation
//
// CREATED : 10/21/97
//
// ----------------------------------------------------------------------- //

#include "ClientSFX.h"


BEGIN_CLASS(CClientSFX)
END_CLASS_DEFAULT_FLAGS(CClientSFX, BaseClass, NULL, NULL, CF_HIDDEN)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientSFX::CClientSFX
//
//	PURPOSE:	Initialize id data member
//
// ----------------------------------------------------------------------- //

CClientSFX::CClientSFX(DBYTE nType) : BaseClass(nType)
{
}

