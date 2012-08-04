// ----------------------------------------------------------------------- //
//
// MODULE  : GameWorldPackerImpl.h
//
// PURPOSE : Declares the CGameWorldPackerImpl class.  This class
//           implements the IGameWorldPacker interface.
//
// CREATED : 12/10/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __GAMEWORLDPACKERIMPL_H__
#define __GAMEWORLDPACKERIMPL_H__

#include "IGameWorldPacker.h"

class CGameWorldPackerImpl : public IGameWorldPacker
{
public:
	virtual bool NavMeshProcess( /* IN */ const uint8* pPCRawData, /* OUT */ ILTOutConverter& Converter, const LTVector& vWorldOffset, PFNERRORCALLBACK pfnErrorCallback );
};

#endif  // __GAMEWORLDPACKERIMPL_H__
