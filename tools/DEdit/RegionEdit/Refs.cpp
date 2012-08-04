//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
//------------------------------------------------------------------
//
//	FILE	  : Refs.cpp
//
//	PURPOSE	  : Implements all the C<X>Ref classes.
//
//	CREATED	  : November 12 1996
//
//
//------------------------------------------------------------------

// Includes....
#include "bdefs.h"
#include "refs.h"
#include "editbrush.h"
#include "editpoly.h"


CEditVert& CVertRef::operator () ()
{
	ASSERT( m_pBrush );
	ASSERT( m_iVert < m_pBrush->m_Points );

	return m_pBrush->m_Points[ m_iVert ];
}


CEditVert& CEdgeRef::Vert1()
{
	ASSERT( m_pBrush );
	ASSERT( m_Vert1 < m_pBrush->m_Points );

	return m_pBrush->m_Points[ m_Vert1 ];
}


CEditVert& CEdgeRef::Vert2()
{
	ASSERT( m_pBrush );
	ASSERT( m_Vert2 < m_pBrush->m_Points );

	return m_pBrush->m_Points[ m_Vert2 ];
}


CEditPoly* CPolyRef::operator () ()
{
	ASSERT( IsValid() );
	ASSERT( m_iPoly < m_pBrush->m_Polies );

	return m_pBrush->m_Polies[ m_iPoly ];
}


CVertRef CPolyRef::Vert( uint32 i )
{
	ASSERT( IsValid() );
	ASSERT( m_iPoly < m_pBrush->m_Polies );
	ASSERT( i < m_pBrush->m_Polies[m_iPoly]->NumVerts() );

	return CVertRef( m_pBrush, m_pBrush->m_Polies[m_iPoly]->m_Indices[i] );
}





