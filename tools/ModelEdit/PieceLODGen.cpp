#include "precompile.h"
#include "model.h"
#include "model_cleanup.h"
#include "PieceLODGen.h"


// generate the LOD from the source LOD
bool CBuildPieceLOD::BuildLOD()
{
	// can't generate LODs for vertex animated pieces
	if( m_ModelPiece->m_isVA )
		return false;

	// remove any unused vertices
//	gn_RemoveUnusedVertices( m_Model );

	// initialize internal structures
	SetupGenLODModel();

	m_Collapses.SetCacheSize( 10000 );

	unsigned numStartTris = m_PieceLOD->m_Tris;
	unsigned numCollapses = 0;

	// generate edge collapses
	while( 1 )
	{
		EdgeCollapse edgeCollapse;

		// find the best edge collapse candidate
		if( !FindShortestEdge( edgeCollapse ) )
			break;

		numCollapses++;

		// update structures using this edge collapse
		ProcessEdgeCollapse( edgeCollapse );
		m_Collapses.Append( edgeCollapse );

		// check to see if we should stop collapsing edges
		if( (numStartTris - (numCollapses * 2)) <= (numStartTris * m_Percent / 100.0f) )
			break;
	}

	// add this LOD to the model
	if( (m_Collapses.GetSize() <= 0) || !AddLOD() )
		return false;

	return true;
}


// initialize internal structures
void CBuildPieceLOD::SetupGenLODModel( void )
{
	m_TriActive.SetSize( m_PieceLOD->m_Tris.GetSize() );
	m_Tris.SetSize( m_PieceLOD->m_Tris.GetSize() );
	for( uint32 i = 0; i < m_PieceLOD->m_Tris; i++ )
	{
		m_TriActive[i] = true;
		m_Tris[i] = m_PieceLOD->m_Tris[i];
	}

	m_Verts.SetSize( m_PieceLOD->m_Verts.GetSize() );
	for( i = 0; i < m_PieceLOD->m_Verts; i++ )
	{
		m_Verts[i] = m_PieceLOD->m_Verts[i];
		m_Verts[i].m_iReplacement = (short)i;
	}
}


// find the best edge collapse candidate
bool CBuildPieceLOD::FindShortestEdge( EdgeCollapse& edgeCollapse )
{
	// check the number of remaining tris
	uint32 numActiveTris = CalcNumActiveTris();
	if( (numActiveTris < m_MinNumTris) || (numActiveTris <= 2) )
		return false;

	unsigned curTri, curVert, triVert, triNextVert;
	float testLen;
	unsigned testTris[2];
	float shortestEdge = FLT_MAX;
	bool foundEdge = false;

	for( curTri = 0; curTri < m_Tris; curTri++ )
	{
		// skip already collapsed tris
		if( !m_TriActive[curTri] )
			continue;

		ModelTri* tri = &m_Tris[curTri];

		for( curVert = 0; curVert < 3; curVert++ )
		{
			// get the length of the candidate edge
			triVert = tri->m_Indices[curVert];
			triNextVert = tri->m_Indices[(curVert+1)%3];
			testLen = (m_Verts[triVert].m_Vec - m_Verts[triNextVert].m_Vec).Mag();

			// check to see if we have a good collapse
			if( (testLen < shortestEdge) && (testLen > 0.0f) && TestEdgeCollapse( triVert, triNextVert, testTris ) )
			{
				// make sure it's not too long
				if( testLen >= m_MaxEdgeLen )
					continue;

				// fill in the collapse info
				shortestEdge = testLen;
				edgeCollapse.m_Verts[0] = triVert;
				edgeCollapse.m_Verts[1] = triNextVert;
				edgeCollapse.m_Tris[0] = testTris[0];
				edgeCollapse.m_Tris[1] = testTris[1];
				foundEdge = true;
			}
		}
	}

	return foundEdge;
}


// count the number of uncollapsed triangles
unsigned CBuildPieceLOD::CalcNumActiveTris( void )
{
	unsigned count = 0;

	for( unsigned i = 0; i < m_TriActive; i++ )
	{
		if( m_TriActive[i] )
			count++;
	}

	return count;
}


// returns true if 2 and only 2 triangles share the edge
// fills in tris with the tris that share the edge
bool CBuildPieceLOD::TestEdgeCollapse( unsigned vert0, unsigned vert1, unsigned* tris )
{
	unsigned test0, test1, curTri, curVert;
	unsigned numTrisFound = 0;

	for( curTri = 0; curTri < m_Tris; curTri++ )
	{
		// skip already collapsed tris
		if( !m_TriActive[curTri] )
			continue;

		ModelTri* tri = &m_Tris[curTri];

		for( curVert = 0; curVert < 3; curVert++ )
		{
			test0 = tri->m_Indices[curVert];
			test1 = tri->m_Indices[(curVert+1)%3];

			if( ((test0 == vert0) && (test1 == vert1)) ||
				((test1 == vert0) && (test0 == vert1)) )
			{
				// oops, more than 2 tris share this edge
				if( numTrisFound >= 2 )
					return false;

				tris[numTrisFound++] = curTri;
			}
		}
	}

	return (numTrisFound == 2) && (tris[0] != tris[1]);
}


// updates structures with an edge collapse
bool CBuildPieceLOD::ProcessEdgeCollapse( const EdgeCollapse& edgeCollapse )
{
	unsigned curTri, curVert;
	UVPair uvs[2][2];
	UVPair newUVs[2];
	unsigned triVerts[2][2];
	ModelTri* uvRefTris[2];

	// deactivate the tris
	ASSERT( m_TriActive[edgeCollapse.m_Tris[0]] );
	ASSERT( m_TriActive[edgeCollapse.m_Tris[1]] );
	m_TriActive[edgeCollapse.m_Tris[0]] = false;
	m_TriActive[edgeCollapse.m_Tris[1]] = false;

	for( unsigned i = 0; i < 2; i++ )
	{
		// grab the 0-2 index of each vert on the tri
		if( !GetTriVert( edgeCollapse.m_Tris[i], edgeCollapse.m_Verts[0], triVerts[i][0] ) ||
			!GetTriVert( edgeCollapse.m_Tris[i], edgeCollapse.m_Verts[1], triVerts[i][1] ) )
		{
			ASSERT(0);
			return false;
		}

		uvRefTris[i] = &m_Tris[edgeCollapse.m_Tris[i]];

		uvs[i][0] = uvRefTris[i]->m_UVs[triVerts[i][0]];
		uvs[i][1] = uvRefTris[i]->m_UVs[triVerts[i][1]];
		newUVs[i].tu = uvs[i][0].tu + (uvs[i][1].tu - uvs[i][0].tu) * 0.5f;
		newUVs[i].tv = uvs[i][0].tv + (uvs[i][1].tv - uvs[i][0].tv) * 0.5f;
	}

	// update the UVs of any adjacent verts
	for( curTri = 0; curTri < m_Tris; curTri++ )
	{
		ModelTri* tri = &m_Tris[curTri];

		// don't do anything with the tris being collapsed
		if( (curTri == edgeCollapse.m_Tris[0]) || (curTri == edgeCollapse.m_Tris[1]) )
			continue;

		for( curVert = 0; curVert < 3; curVert++ )
		{
			for( i = 0; i < 2; i++ )
			{
				if( tri->m_Indices[curVert] == uvRefTris[i]->m_Indices[triVerts[i][0]] )
				{
					if( (fabs( tri->m_UVs[curVert].tu - uvs[i][0].tu ) < 0.001f) && (fabs( tri->m_UVs[curVert].tv - uvs[i][0].tv ) < 0.001f) )
						tri->m_UVs[curVert] = newUVs[i];
				}
				else if( tri->m_Indices[curVert] == uvRefTris[i]->m_Indices[triVerts[i][1]] )
				{
					if( (fabs( tri->m_UVs[curVert].tu - uvs[i][1].tu ) < 0.001f) && (fabs( tri->m_UVs[curVert].tv - uvs[i][1].tv ) < 0.001f) )
						tri->m_UVs[curVert] = newUVs[i];
				}
			}
		}
	}

	// add the new vertex
	MergeVerts( edgeCollapse );

	// update the triangle indices
	for( curTri = 0; curTri < m_Tris; curTri++ )
	{
		ModelTri* tri = &m_Tris[curTri];

		for( curVert = 0; curVert < 3; curVert++ )
		{
			if( (tri->m_Indices[curVert] == edgeCollapse.m_Verts[0]) || (tri->m_Indices[curVert] == edgeCollapse.m_Verts[1]) )
				tri->m_Indices[curVert] = m_Verts.LastI();
		}
	}

	// any verts that reference only the removed triangles need to have any replacements
	// referencing them updated to reference the new vertex instead
	UpdateDeadVertexReferences( edgeCollapse, m_Verts.LastI() );

	return true;
}


// given a triangle and a vert index, get which vert on the tri has that index
bool CBuildPieceLOD::GetTriVert( unsigned tri, unsigned vertIndex, unsigned& vert )
{
	ModelTri* t = &m_Tris[tri];

	for( unsigned i = 0; i < 3; i++ )
	{
		if( t->m_Indices[i] == vertIndex )
		{
			vert = i;
			return true;
		}
	}

	return false;
}


// merge two verts into a new vert
void CBuildPieceLOD::MergeVerts( const EdgeCollapse& edgeCollapse )
{
	ModelVert newVert;

	m_Verts.Append( newVert );

	ModelVert* vert0 = &m_Verts[edgeCollapse.m_Verts[0]];
	ModelVert* vert1 = &m_Verts[edgeCollapse.m_Verts[1]];

	MarkReplacements( edgeCollapse.m_Verts[0], m_Verts.LastI() );
	MarkReplacements( edgeCollapse.m_Verts[1], m_Verts.LastI() );

	vert0->m_iReplacement = (short)m_Verts.LastI();
	vert1->m_iReplacement = (short)m_Verts.LastI();
	ModelVert* vert = &m_Verts.Last();
	vert->m_iReplacement = (short)m_Verts.LastI();

	ASSERT( vert0->m_iReplacement != (vert0 - m_Verts.GetArray()) );
	ASSERT( vert1->m_iReplacement != (vert1 - m_Verts.GetArray()) );

	vert->m_Vec = vert0->m_Vec + (vert1->m_Vec - vert0->m_Vec) * 0.5f;
	vert->m_Normal = vert0->m_Normal + (vert1->m_Normal - vert0->m_Normal) * 0.5f;
	vert->m_Normal.Norm();

	NewVertexWeight *weight, *newWeight;

	// add vert0s weights
	for( unsigned i = 0; i < vert0->m_nWeights; i++ )
	{
		weight = &vert0->m_Weights[i];
		newWeight = vert->AddWeight();

		newWeight->m_Vec[0] = weight->m_Vec[0] * 0.5f;
		newWeight->m_Vec[1] = weight->m_Vec[1] * 0.5f;
		newWeight->m_Vec[2] = weight->m_Vec[2] * 0.5f;
		newWeight->m_Vec[3] = weight->m_Vec[3] * 0.5f;
		newWeight->m_iNode = weight->m_iNode;
	}

	// add vert1s weights
	for( i = 0; i < vert1->m_nWeights; i++ )
	{
		weight = &vert1->m_Weights[i];

		// check to see if the weight already exists from vert0
		if( !(newWeight = FindWeight( vert, weight->m_iNode )) )
			newWeight = vert->AddWeight();

		newWeight->m_Vec[0] += weight->m_Vec[0] * 0.5f;
		newWeight->m_Vec[1] += weight->m_Vec[1] * 0.5f;
		newWeight->m_Vec[2] += weight->m_Vec[2] * 0.5f;
		newWeight->m_Vec[3] += weight->m_Vec[3] * 0.5f;
		newWeight->m_iNode = weight->m_iNode;
	}
}


// check to see if a vert already has a weight for a node
NewVertexWeight* CBuildPieceLOD::FindWeight( ModelVert* vert, unsigned node )
{
	for( unsigned i = 0; i < vert->m_nWeights; i++ )
	{
		if( vert->m_Weights[i].m_iNode == node )
			return &vert->m_Weights[i];
	}

	return NULL;
}


// update any m_iReplacements referencing vertIndex to newVertIndex
void CBuildPieceLOD::MarkReplacements( unsigned vertIndex, unsigned newVertIndex )
{
	ModelVert* vert;

	for( unsigned i = 0; i < m_Verts; i++ )
	{
		vert = &m_Verts[i];

		if( vert->m_iReplacement == vertIndex )
			vert->m_iReplacement = newVertIndex;
	}
}


// any verts that reference only the removed triangles need to have any replacements
// referencing them updated to reference the new vertex instead
void CBuildPieceLOD::UpdateDeadVertexReferences( const EdgeCollapse& edgeCollapse, unsigned newIndex )
{
	unsigned curTri, curVert;

	ModelTri* tris[2];
	tris[0] = &m_Tris[edgeCollapse.m_Tris[0]];
	tris[1] = &m_Tris[edgeCollapse.m_Tris[1]];

	// make a list for each vertex of what tris reference it
	CMoArray<ModelTriPtrArray> vertTriRefs;
	vertTriRefs.SetSize( m_Verts.GetSize() );
	for( curTri = 0; curTri < m_Tris; curTri++ )
	{
		ModelTri* tri = &m_Tris[curTri];

		if( !m_TriActive[curTri] && (tri != tris[0] && tri != tris[1]) )
			continue;

		for( curVert = 0; curVert < 3; curVert++ )
		{
			vertTriRefs[tri->m_Indices[curVert]].Append( tri );
		}
	}

	// look for vertices that are only referenced by these tris
	for( curVert = 0; curVert < vertTriRefs; curVert++ )
	{
		ModelTriPtrArray& curArray = vertTriRefs[curVert];

		if( (curArray.GetSize() == 1) && (curArray[0] == tris[0] || curArray[0] == tris[1]) )
		{
			MarkReplacements( curVert, newIndex );
		}
		else if( (curArray.GetSize() == 2) &&
				 (curArray[0] == tris[0] || curArray[0] == tris[1]) &&
				 (curArray[1] == tris[0] || curArray[1] == tris[1]) )
		{
			MarkReplacements( curVert, newIndex );
		}
	}
}


// add a new LOD to the model using the current set of edge collapses
bool CBuildPieceLOD::AddLOD( void )
{
	unsigned curEdge, curTri, curVert, curTriVert;
	PieceLOD newLOD;

	newLOD.Init( m_Model );

	// find the tris that should be in the new LOD
	CMoArray<bool> usedTris;
	unsigned numUsedTris = m_Tris.GetSize();
	usedTris.SetSizeInit2( numUsedTris, true );

	for( curEdge = 0; curEdge < m_Collapses; curEdge++ )
	{
		usedTris[m_Collapses[curEdge].m_Tris[0]] = usedTris[m_Collapses[curEdge].m_Tris[1]] = false;
		numUsedTris -= 2;
	}

	// setup the new LODs triangle list
	unsigned curAddTri = 0;
	newLOD.m_Tris.SetSize( numUsedTris );

	for( curTri = 0; curTri < usedTris; curTri++ )
	{
		if( !usedTris[curTri] )
			continue;

		newLOD.m_Tris[curAddTri++] = m_Tris[curTri];
	}

	// find the verts that should be in the new LOD
	CMoArray<bool> usedVerts;
	unsigned numUsedVerts = 0;
	usedVerts.SetSizeInit2( m_Verts.GetSize(), false );
	
	for( curTri = 0; curTri < m_Tris; curTri++ )
	{
		for( curTriVert = 0; curTriVert < 3; curTriVert++ )
		{
			if( !usedVerts[m_Tris[curTri].m_Indices[curTriVert]] )
			{
				usedVerts[m_Tris[curTri].m_Indices[curTriVert]] = true;
				numUsedVerts++;
			}
		}
	}

	// setup the new LODs vertex list
	CMoArray<unsigned> vertIndexRemap;
	unsigned curAddVert = 0;
	vertIndexRemap.SetSizeInit2( m_Verts.GetSize(), UINT_MAX );
	newLOD.m_Verts.SetSize( numUsedVerts );

	for( curVert = 0; curVert < m_Verts; curVert++ )
	{
		if( !usedVerts[curVert] )
			continue;

		newLOD.m_Verts[curAddVert] = m_Verts[curVert];

		vertIndexRemap[curVert] = curAddVert++;
	}

	ASSERT( numUsedVerts == curAddVert );

	// remap the triangle indices
	for( curTri = 0; curTri < newLOD.m_Tris; curTri++ )
	{
		ModelTri* tri = &newLOD.m_Tris[curTri];

		for( curTriVert = 0; curTriVert < 3; curTriVert++ )
		{
			ASSERT( tri->m_Indices[curTriVert] != UINT_MAX );
			tri->m_Indices[curTriVert] = vertIndexRemap[tri->m_Indices[curTriVert]];
		}
	}

	// copy over material info
	newLOD.SetMaterialInfo( *m_PieceLOD );

	newLOD.CalcUsedNodeList();

	// add the new LOD to the piece
	m_ModelPiece->AddLOD( newLOD, m_Distance, true );

	return true;
}

