#include "bdefs.h"
#include "modelallocations.h"

//-----------------------------------------------------------------------------
// util : WordAlign( size ) => new size 
//-----------------------------------------------------------------------------
inline uint32 WordAlign(uint32 total)
{
	return (total + 3) & ~3;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
ModelAllocations::ModelAllocations()
{
	Clear();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void ModelAllocations::Clear()
{
	m_nKeyFrames = 0;
	m_nParentAnims = 0;
	m_nNodes = 0;
	m_nPieces = 0;
	m_nChildModels = 0;
	m_nTris = 0;
	m_nVerts = 0;
	m_nVertexWeights = 0;
	m_nLODs = 0;
	m_nSockets = 0;
	m_nWeightSets = 0;
	m_nStrings = 0;
	m_StringLengths = 0;
	m_nAnimData = 0;

	m_VertAnimDataSize = 0 ;
}


bool ModelAllocations::Load( ILTStream &str, uint32 FileVersion )
{
	str >> m_nKeyFrames;
	str >> m_nParentAnims;
	str >> m_nNodes;
	str >> m_nPieces;
	str >> m_nChildModels;
	str >> m_nTris;
	str >> m_nVerts;
	str >> m_nVertexWeights;
	str >> m_nLODs;
	str >> m_nSockets;
	str >> m_nWeightSets;
	str >> m_nStrings;
	str >> m_StringLengths;
	str >> m_VertAnimDataSize;
	str >> m_nAnimData;
	
	return str.ErrorStatus() == LT_OK;
}

// this is not going to be very accurate
uint32 ModelAllocations::CalcAllocationSize()
{
	uint32 total = 0;

	uint32 nChildNodes = m_nNodes - 1;

	total += m_nStrings * WordAlign(sizeof(ModelString)); // All the strings.
	// KEF - 3/23/00 - The extra 3*m_nStrings is because the string length calculation isn't word aligned..
	total += WordAlign(m_StringLengths + 3 * m_nStrings);

	total += WordAlign(m_VertAnimDataSize);										// size of vert dat.
	total += WordAlign(sizeof(AnimNode))			* m_nNodes * m_nParentAnims;	// ModelAnim::m_pAnimNodes.
	total += WordAlign(sizeof(AnimKeyFrame))		* m_nKeyFrames;					// ModelAnim::m_KeyFrames.
	
	total += WordAlign(sizeof(ModelNode*))			* nChildNodes;					// ModelNode::m_Children array.
	total += WordAlign(sizeof(ModelNode))			* nChildNodes;					// ModelNode::m_Children.

	total += WordAlign(sizeof(ModelPiece*))			* m_nPieces;					// Model::m_Pieces array.
	total += WordAlign(sizeof(ModelPiece))			* m_nPieces;					// Model::m_Pieces.
		
	total += WordAlign(sizeof(float))				* m_nWeightSets * m_nNodes;		// WeightSet::m_Weights.
	total += WordAlign(sizeof(WeightSet*))			* m_nWeightSets;				// Model::m_WeightSets array.
	total += WordAlign(sizeof(WeightSet))			* m_nWeightSets;				// Model::m_WeightSets.
	
	total += WordAlign(sizeof(ModelSocket*))		* m_nSockets;					// Model::m_Sockets array.
	total += WordAlign(sizeof(ModelSocket))			* m_nSockets;					// Model::m_Sockets.

	total += WordAlign(sizeof(ModelNode*))			* m_nNodes;						// Model::m_FlatNodeList.
	total += WordAlign(sizeof(ModelAnim))			* m_nParentAnims;				// Model::m_Anims.

	total += WordAlign(sizeof(float))				* m_nLODs;						// ModelPiece::m_pLODDists
	total += WordAlign(sizeof(CDIModelDrawable*))	* m_nLODs;						// ModelPiece::m_RenderObjects

	total += WordAlign(m_nAnimData);

	return total;
}









