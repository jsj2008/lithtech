// ----------------------------------------------------------------------- //
//
// MODULE  : AINavMeshGenMacros.h
//
// PURPOSE : AI NavMesh generator macros for things that differ
//           between game code and tools code.
//
// CREATED : 12/07/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AI_NAVMESH_GEN_MACROS_H_
#define _AI_NAVMESH_GEN_MACROS_H_

//-----------------------------------------------------------------

#define NAVMESH_MSG( msg ) \
	if( m_pfnErrorCallback ) \
		m_pfnErrorCallback( IGameWorldPacker::eSeverity_Message, msg ); \
	else \
		AITRACE( AIShowNavMesh, ( (HOBJECT)NULL, msg ) )

#define NAVMESH_MSG1( msg, arg ) \
	if( m_pfnErrorCallback ) \
		m_pfnErrorCallback( IGameWorldPacker::eSeverity_Message, msg, arg ); \
	else \
		AITRACE( AIShowNavMesh, ( (HOBJECT)NULL, msg, arg ) )

//-----------------------------------------------------------------

#define NAVMESH_ERROR( msg ) \
	if( m_pfnErrorCallback ) \
		m_pfnErrorCallback( IGameWorldPacker::eSeverity_Error, msg ); \
	else \
		LTASSERT( 0, msg )

#define NAVMESH_ERROR1( msg, d1 ) \
	if( m_pfnErrorCallback ) \
	m_pfnErrorCallback( IGameWorldPacker::eSeverity_Error, msg, d1 ); \
	else \
	LTASSERT_PARAM1( 0, msg, d1 )

#define NAVMESH_ERROR2( msg, d1, d2 ) \
	if( m_pfnErrorCallback ) \
	m_pfnErrorCallback( IGameWorldPacker::eSeverity_Error, msg, d1, d2 ); \
	else \
	LTASSERT_PARAM2( 0, msg, d1, d2 )

#define NAVMESH_ERROR2( msg, d1, d2 ) \
	if( m_pfnErrorCallback ) \
		m_pfnErrorCallback( IGameWorldPacker::eSeverity_Error, msg, d1, d2 ); \
	else \
		LTASSERT_PARAM2( 0, msg, d1, d2 )

#define NAVMESH_ERROR3( msg, d1, d2, d3 ) \
	if( m_pfnErrorCallback ) \
		m_pfnErrorCallback( IGameWorldPacker::eSeverity_Error, msg, d1, d2, d3 ); \
	else \
		LTASSERT_PARAM3( 0, msg, d1, d2, d3 )

#define NAVMESH_ERROR5( msg, d1, d2, d3, d4, d5 ) \
	if( m_pfnErrorCallback ) \
		m_pfnErrorCallback( IGameWorldPacker::eSeverity_Error, msg, d1, d2, d3, d4, d5 ); \
	else \
		LTASSERT_PARAM5( 0, msg, d1, d2, d3, d4, d5 )

//-----------------------------------------------------------------

#define NAVMESH_NEW		debug_new
#define NAVMESH_NEW1	debug_new1
#define NAVMESH_NEWA	debug_newa
#define NAVMESH_DELETE	debug_delete

//-----------------------------------------------------------------

#define NAVMESH_CONVERTPOS( pos ) \
	(m_pfnErrorCallback) ? (pos + m_vWorldOffset) : ConvertToDEditPos( pos )

//-----------------------------------------------------------------

#endif // _AI_NAVMESH_GEN_MACROS_H_
