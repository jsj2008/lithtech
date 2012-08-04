#pragma warning (disable:4786)

#include "bdefs.h"
#include "modelallocations.h"

//-----------------------------------------------------------------------------
// util : WordAlign( size ) => new size 
//-----------------------------------------------------------------------------
static inline uint32 WordAlign(uint32 total)
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

	//holder fields
	m_nAnimDataPos = 0;
}


//-----------------------------------------------------------------------------
// InitFromModel 
// calculate the size of the model.
//-----------------------------------------------------------------------------
bool ModelAllocations::InitFromModel(Model *pModel)
{
	uint32 iAnim, iPiece, iLOD, iKey, iNode, iChild;
	ModelAnim *pAnim;
	ModelPiece *pPiece;
	ModelString *pString;
	PieceLOD *pLOD;
	ModelStringList tempStringList;
	
	Clear();

	m_nLODs = 0 ;
	for( iPiece = 0 ; iPiece < pModel->NumPieces() ; iPiece++ )
		m_nLODs += pModel->GetPiece(iPiece)->NumLODs();
	
	m_FileVersion = pModel->GetFileVersion() ;

	m_nParentAnims = pModel->CalcNumParentAnims();
	m_nNodes = pModel->NumNodes();
	m_nPieces = pModel->NumPieces();
	m_nChildModels = pModel->NumChildModels();
	m_nSockets = pModel->NumSockets();
	m_nWeightSets = pModel->NumWeightSets();
	m_nVertexWeights = pModel->CalcNumVertexWeights();
	
	// Count keyframes & vertex animation sizes
	for(iAnim=0; iAnim < m_nParentAnims; iAnim++)
	{
		pAnim = pModel->GetAnim(iAnim);

		m_nKeyFrames += pAnim->NumKeyFrames();
		AnimNode *pAN = pAnim->GetRootNode();

		if(pAN != NULL )
			m_VertAnimDataSize += pAN->GetTotalVertexAnimMemSize();
	}

	// Count geometry stuff.
	for(iPiece=0; iPiece < pModel->NumPieces(); iPiece++)
	{
		pPiece = pModel->GetPiece(iPiece);
		
		for(iLOD=0; iLOD < pPiece->NumLODs(); iLOD++)
		{
			pLOD = pPiece->GetLOD(iLOD);

			m_nTris += pLOD->m_Tris.GetSize();
			m_nVerts += pLOD->m_Verts.GetSize();
		}
	}

	// Count strings.
	for(iAnim=0; iAnim < m_nParentAnims; iAnim++)
	{
		pAnim = pModel->GetAnim(iAnim);
		
		tempStringList.AddString(pAnim->GetName());

		for(iKey=0; iKey < pAnim->NumKeyFrames(); iKey++)
			tempStringList.AddString(pAnim->m_KeyFrames[iKey].m_pString);
	}

	for(iNode=0; iNode < pModel->NumNodes(); iNode++)
		tempStringList.AddString(pModel->GetNode(iNode)->GetName());

	tempStringList.AddString(pModel->m_CommandString);

	for(iChild=1; iChild < pModel->NumChildModels(); iChild++)
		tempStringList.AddString(pModel->GetChildModel(iChild)->m_pFilename);

	for(uint32 nSocket = 0; nSocket < pModel->NumSockets(); nSocket++)
		tempStringList.AddString(pModel->GetSocket(nSocket)->m_Name);

	for(uint32 nPiece = 0; nPiece < pModel->NumPieces(); nPiece++)
		tempStringList.AddString(pModel->GetPiece(nPiece)->GetName());

	for(uint32 nWeightSet = 0; nWeightSet < pModel->NumWeightSets(); nWeightSet++)
		tempStringList.AddString(pModel->GetWeightSet(nWeightSet)->m_Name);

	// Tally it up...
	for(pString=tempStringList.m_StringList; pString; pString=pString->m_pNext)
	{
		++m_nStrings;
		m_StringLengths += strlen(pString->m_String);
	}

	//this will be filled in later
	m_nAnimData = 0;

	return true;
}

//Updates a field that had its position saved
void ModelAllocations::UpdateField(ILTStream& str, uint32 nFieldPos, uint32 nVal)
{
	//save the current position
	uint32 nCurrPos = str.GetPos();

	str.SeekTo(nFieldPos);

	str << nVal;

	str.SeekTo(nCurrPos);
}

bool ModelAllocations::Save(ILTStream &str)
{
	str << m_nKeyFrames;
	str << m_nParentAnims;
	str << m_nNodes;
	str << m_nPieces;
	str << m_nChildModels;
	str << m_nTris;
	str << m_nVerts;
	str << m_nVertexWeights;
	str << m_nLODs;
	str << m_nSockets;
	str << m_nWeightSets;
	str << m_nStrings;
	str << m_StringLengths;
	str << m_VertAnimDataSize ;

	//save this position
	m_nAnimDataPos = str.GetPos();

	str << m_nAnimData;


	return str.ErrorStatus() == LT_OK;
}


