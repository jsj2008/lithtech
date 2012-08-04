// ----------------------------------------------------------------------- //
//
// MODULE  : AINavMeshGenCarver.h
//
// PURPOSE : AI NavMesh generator Carver class definition
//
// CREATED : 11/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AI_NAVMESH_GEN_CARVER_H_
#define _AI_NAVMESH_GEN_CARVER_H_

#include "AINavMeshGenTypes.h"
#include "IGameWorldPacker.h"

//-----------------------------------------------------------------

class CAINavMeshGenCarver
{
public:

	 CAINavMeshGenCarver( IGameWorldPacker::PFNERRORCALLBACK pfnErrorCallback );
	~CAINavMeshGenCarver();

	void	InitNMGCarver();

	// Construction.

	void	SetNMGCarverID( ENUM_NMGCarverID eID ) { m_eNMGCarverID = eID; }
	void	AddNMGCarverVert( const LTVector& vVert );
	void	SetNMGCarverHeight( float h ) { m_fNMGCarverHeight = h; }

	// Carving.

	bool	CarveNMGPoly( CAINavMeshGenPoly* pPoly, AINAVMESHGEN_POLY_LIST* plstNewPolys, AINAVMESHGEN_POLY_LIST* plstDeletePolys, AINAVMESHGEN_POLY_LIST* plstCarvedPolys );

	// Data access.

	ENUM_NMGCarverID	GetNMGCarverID() const { return m_eNMGCarverID; }
	SAINAVMESHGEN_AABB*	GetAABB() { return &m_aabbNMGCarverBounds; }

	// Debug.

	void	PrintCarverVerts();

protected:

void	SelectFirstCarvingPlane( CAINavMeshGenPoly* pPoly, VECTOR_LIST* plstCarverIntersects );

protected:

	enum ENUM_SIDE
	{
		kLeft,
		kRight,
		kTop,
		kBottom,
	};

protected:

	VECTOR_LIST			m_lstNMGCarverVerts;
	float				m_fNMGCarverHeight;
	SAINAVMESHGEN_AABB	m_aabbNMGCarverBounds;
	ENUM_NMGCarverID	m_eNMGCarverID;

	// error callback used when running from the World Packer
	IGameWorldPacker::PFNERRORCALLBACK	m_pfnErrorCallback;
};

//-----------------------------------------------------------------

#endif // _AI_NAVMESH_GEN_CARVER_H_
