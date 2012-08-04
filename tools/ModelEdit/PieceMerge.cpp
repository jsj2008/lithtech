#include "precompile.h"
#include "model.h"
#include "PieceMerge.h"
#include <algorithm>

using namespace std;


CPieceMerge::CPieceMerge( Model* model, vector<ModelPiece*>& pieces )
{
	ASSERT( model );
	m_Model = model;

	for( vector<ModelPiece*>::iterator it = pieces.begin(); it != pieces.end(); it++ )
	{
		m_Pieces.push_front( *it );
	}

	// store the original pointers in order (for vertex animation remapping)
	m_OriginalPieces.reserve( m_Model->m_Pieces.GetSize() );
	for( uint32 curPiece = 0; curPiece < m_Model->m_Pieces; curPiece++ )
	{
		m_OriginalPieces.push_back( m_Model->m_Pieces[curPiece] );
	}
}


CPieceMerge::~CPieceMerge()
{
}


// merge the pieces and return the number of resulting pieces
uint32 CPieceMerge::Merge( void )
{
	list<ModelPiece*> mergePieces;
	uint32 numNewPieces = 0;

	// grab any remaining mergeable pieces and merge them together
	while( m_Pieces.size() )
	{
		if( GetMergeablePieces( mergePieces ) )
			MergePieces( mergePieces );

		numNewPieces++;
	}

	// we've probably just trampled vertex animation indices, so remap them
	RemapVertexAnimations();

	// return resulting number of pieces
	return numNewPieces;
}


// get a list of pieces that can be merged with the first piece in the list and remove them from the list of pieces waiting to be merged
// returns false if no pieces were mergeable with the first piece in the list
bool CPieceMerge::GetMergeablePieces( list<ModelPiece*>& pieces )
{
	pieces.clear();

	int numMergeablePieces = 0;
	int numPiecesLeft = m_Pieces.size();
	if( !numPiecesLeft )
		return false;

	// grab the item we will be comparing others against
	list<ModelPiece*>::iterator it = m_Pieces.begin();
	ModelPiece* first = *it;
	it = m_Pieces.erase( it );

	// loop through all the items in the list looking for ones that are mergeable with the first one
	while( it != m_Pieces.end() )
	{
		if( ArePiecesMergeable( first, *it ) )
		{
			// the pieces are mergeable, move the current one to the merge list
			pieces.push_back( *it );
			it = m_Pieces.erase( it );
			numMergeablePieces++;
		}
		else
			it++;
	}

	// if matches were found, add the first element to the mergeable pieces array
	if( numMergeablePieces )
	{
		pieces.push_front( first );
		numMergeablePieces++;
	}

	return( numMergeablePieces >= 2 );
}


// returns true if two model pieces are mergeable
bool CPieceMerge::ArePiecesMergeable( ModelPiece* a, ModelPiece* b )
{
	// must be skeletally animated
	if( a->m_isVA || b->m_isVA )
		return false;

	// must have the same LOD offsets
	uint32 aMinLOD, aMaxLOD, bMinLOD, bMaxLOD;
	a->GetMinMaxLODOffset( aMinLOD, aMaxLOD );
	b->GetMinMaxLODOffset( bMinLOD, bMaxLOD );
	if( (aMinLOD != bMinLOD) || (aMaxLOD != bMaxLOD) )
		return false;

	// must have same number of LODs
	uint32 aNumLODs = a->NumLODs();
	uint32 bNumLODs = b->NumLODs();
	if( aNumLODs != bNumLODs )
		return false;

	// check each LOD for matching info
	for( uint32 curLOD = 0; curLOD < aNumLODs; curLOD++ )
	{
		PieceLOD* aLOD = a->GetLOD( curLOD );
		PieceLOD* bLOD = b->GetLOD( curLOD );

		// must have the same LOD distance
		if( a->GetLODDist( curLOD ) != b->GetLODDist( curLOD ) )
			return false;

		// must have same number of textures
		if( aLOD->m_nNumTextures != bLOD->m_nNumTextures )
			return false;

		// texture indices that matter must be the same
		for( uint32 curTex = 0; curTex < aLOD->m_nNumTextures; curTex++ )
		{
			if( aLOD->m_iTextures[curTex] != bLOD->m_iTextures[curTex] )
				return false;
		}

		// must have the same renderstyle
		if( aLOD->m_iRenderStyle != bLOD->m_iRenderStyle )
			return false;
	}

	return true;
}


// merge these pieces into one and delete the original pieces (must be mergeable pieces)
bool CPieceMerge::MergePieces( list<ModelPiece*>& pieces )
{
	// create the new modelpiece
	ModelPiece* newPiece = new ModelPiece( m_Model );
//	ModelPiece* newPiece = LNew_1P( m_Model->GetAlloc(), ModelPiece, m_Model );

	// the piece we will be copying information from
	ModelPiece* originalPiece = pieces.front();

	// copy the name
	newPiece->SetName( originalPiece->GetName() );

	// copy the LOD offset information
	uint32 minLODOffset, maxLODOffset;
	originalPiece->GetMinMaxLODOffset( minLODOffset, maxLODOffset );
	newPiece->SetMinMaxLODOffset( minLODOffset, maxLODOffset );

	// copy LOD weight (not really used, but copy anyway)
	float originalLODWeight = originalPiece->GetLODWeight();
	newPiece->SetLODWeight( originalLODWeight );

	// create space for LODs
	uint32 numLODs = originalPiece->NumLODs();
	newPiece->m_LODs.SetCacheSize( numLODs );
	newPiece->m_LODDists.SetCacheSize( numLODs );

	// create the LODs
	for( uint32 curLOD = 0; curLOD < numLODs; curLOD++ )
	{
		// create the new LOD (copy assignment, so don't allocate on heap)
		PieceLOD newLOD;
		newLOD.Init( m_Model );

		// grab the original LOD to copy information from
		PieceLOD* originalLOD = originalPiece->GetLOD( curLOD );
		ASSERT( originalLOD );
		float originalLODDist = originalPiece->GetLODDist( curLOD );

		// copy material information
		newLOD.SetMaterialInfo( *originalLOD );

		// count the number of tris and verts
		uint32 numTris = 0;
		uint32 numVerts = 0;

		list<ModelPiece*>::iterator pieceIt;

		for( pieceIt = pieces.begin(); pieceIt != pieces.end(); pieceIt++ )
		{
			PieceLOD* origLOD = (*pieceIt)->GetLOD( curLOD );
			numTris += origLOD->m_Tris.GetSize();
			numVerts += origLOD->m_Verts.GetSize();
		}

		// allocate space for the new tris and verts
		ASSERT( numVerts && numTris );
		newLOD.m_Tris.SetSize( numTris );
		newLOD.m_Verts.SetSize( numVerts );

		// copy the geometry data
		uint32 curTri = 0;
		uint32 curVert = 0;

		for( pieceIt = pieces.begin(); pieceIt != pieces.end(); pieceIt++ )
		{
			PieceLOD* origLOD = (*pieceIt)->GetLOD( curLOD );

			// copy the triangles
			for( uint32 curOrigTri = 0; curOrigTri < origLOD->m_Tris; curOrigTri++ )
			{
				newLOD.m_Tris[curTri] = origLOD->m_Tris[curOrigTri];

				// need to update indices
				for( uint32 curIndex = 0; curIndex < 3; curIndex++ )
				{
					newLOD.m_Tris[curTri].m_Indices[curIndex] += curVert;
				}

				curTri++;
			}

			// copy the verts
			for( uint32 curOrigVert = 0; curOrigVert < origLOD->m_Verts; curOrigVert++ )
			{
				newLOD.m_Verts[curVert] = origLOD->m_Verts[curOrigVert];

				// need to create new weight array
				uint32 numWeights = newLOD.m_Verts[curVert].m_nWeights;
				newLOD.m_Verts[curVert].m_Weights = new NewVertexWeight[numWeights];	// this memory will be leaked :(
				for( uint32 curWeight = 0; curWeight < numWeights; curWeight++ )
				{
					newLOD.m_Verts[curVert].m_Weights[curWeight].m_iNode = origLOD->m_Verts[curOrigVert].m_Weights[curWeight].m_iNode;
					for( uint32 weightVec = 0; weightVec < 4; weightVec++ )
					{
						newLOD.m_Verts[curVert].m_Weights[curWeight].m_Vec[weightVec] = origLOD->m_Verts[curOrigVert].m_Weights[curWeight].m_Vec[weightVec];
					}
				}

				curVert++;
			}
		}

		ASSERT( (curVert == numVerts) && (curTri == numTris) );

		// update the used node list so nodes don't get removed
		newLOD.CalcUsedNodeList();

		// add the new LOD to the piece
		newPiece->AddLOD( newLOD, originalLODDist, true );
	}

	// kill the old pieces and replace with the new one
	uint32 insertAt = m_Model->m_Pieces.FindElement( pieces.front() );

	for( list<ModelPiece*>::iterator it = pieces.begin(); it != pieces.end(); it++ )
	{
		uint32 removeAt = m_Model->m_Pieces.FindElement( *it );
		m_Model->m_Pieces.Remove( removeAt );
		delete *it;
	}

	m_Model->m_Pieces.Insert( insertAt, newPiece );

	return true;
}


// fix vertex animation piece mappings that were trampled in the merge
bool CPieceMerge::RemapVertexAnimations( void )
{
	for( uint32 curPieceNum = 0; curPieceNum < m_Model->NumPieces(); curPieceNum++ )
	{
		ModelPiece* curPiece = m_Model->GetPiece( curPieceNum );

		// skip it if it's not vertex animated
		if( !curPiece->m_isVA )
			continue;

		// find the original index for this piece
		vector<ModelPiece*>::iterator it = find( m_OriginalPieces.begin(), m_OriginalPieces.end(), curPiece );
		uint32 originalIndex = it - m_OriginalPieces.begin();
		ASSERT( it != m_OriginalPieces.end() );

		// now find the vertex animation that references this piece
		for( uint32 curAnimNum = 0; curAnimNum < m_Model->NumAnims(); curAnimNum++ )
		{
			ModelAnim* curAnim = m_Model->GetAnim( curAnimNum );
			AnimNode* rootAnim = curAnim->GetRootNode();

			RemapVertexAnimationsRecurse( rootAnim, originalIndex, curPieceNum );
		}
	}

	return true;
}


void CPieceMerge::RemapVertexAnimationsRecurse( AnimNode* curNode, uint32 originalIndex, uint32 newIndex )
{
	if( !curNode )
		return;

	if( curNode->GetVertexAnimTarget() == (int)originalIndex )
		curNode->SetVertexAnimTarget( (int)newIndex );

	// recurse through children
	uint32 numChildren = curNode->NumChildren();

	for( uint32 curChild = 0; curChild < numChildren; curChild++ )
	{
		AnimNode* child = curNode->GetChild( curChild );
		RemapVertexAnimationsRecurse( child, originalIndex, newIndex );
	}
}
