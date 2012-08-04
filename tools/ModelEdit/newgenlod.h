
// This module does the work of generating model LODs.

#ifndef __NEWGENLOD_H__
#define __NEWGENLOD_H__


	#include "model.h"

	
	class LODRequestInfo
	{
	public:
		// Desired number of triangles at this LOD.
		DWORD		m_nTris;

		// Distance this LOD occurs at.
		float		m_Dist;

	
	// Don't fill this stuff in.. used by BuildLODs..
	public:

		BOOL		m_bProcessed;
	};

	
	class BuildLODRequest
	{
	public:

						BuildLODRequest()
						{
							m_pModel = NULL;
							m_MaxEdgeLength = 100.0f;
							m_nMinPieceTris = 6;
						}


		float			GetPieceWeight(DWORD i)
		{
			return (i >= m_PieceWeights.GetSize()) ? 1.0f : m_PieceWeights[i];
		}

	public:
	
		Model			*m_pModel;

		// A weight on the edge length for each piece (so if a piece has a higher weight,
		// its triangles will be collapsed last).
		// Use GetPieceWeight to be safe...
		CMoArray<float>				m_PieceWeights;

		// Tells how to generate each LOD.
		CMoArray<LODRequestInfo>	m_LODInfos;
		
		// It won't collapse edges larger than this value
		// (unless it has to...)
		float			m_MaxEdgeLength;

		// It won't create edge collapses when pieces get <= this number of triangles
		// (unless it has to...)
		DWORD			m_nMinPieceTris;
	};


	BOOL BuildLODs(BuildLODRequest *pRequest);


#endif




