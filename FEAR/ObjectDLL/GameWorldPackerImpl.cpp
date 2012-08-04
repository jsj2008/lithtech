// ----------------------------------------------------------------------- //
//
// MODULE  : GameWorldPackerImpl.cpp
//
// PURPOSE : Defines the CGameWorldPackerImpl class.  This class
//           implements the IGameWorldPacker interface.
//
// CREATED : 12/10/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "GameWorldPackerImpl.h"
#include "AINavMeshGen.h"

//----------------------
// GetIGameWorldPacker
//
// Handles acquiring an IGameWorldPacker object for the calling binaries

extern "C"
{
	MODULE_EXPORT IGameWorldPacker* GetIGameWorldPacker()
	{
		//we only have a singleton of this object so return that
		static CGameWorldPackerImpl s_GameWorldPacker;
		return &s_GameWorldPacker;
	}
}

// processes the NavMesh
bool CGameWorldPackerImpl::NavMeshProcess( /* IN */ const uint8* pPCRawData, /* OUT */ ILTOutConverter& Converter, const LTVector& vWorldOffset, PFNERRORCALLBACK pfnErrorCallback )
{
	CAINavMeshGen AINavMeshGen(pfnErrorCallback);

	bool bReturn = true;
	if( AINavMeshGen.ImportRawNavMesh(pPCRawData) )
	{
		AINavMeshGen.InitNavMeshGen( vWorldOffset );
		if( !AINavMeshGen.ExportPackedNavMesh(Converter) )
			bReturn = false;
		AINavMeshGen.TermNavMeshGen();
	}
	else
	{
		return false;
	}

	return bReturn;
}
