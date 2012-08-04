// ----------------------------------------------------------------------- //
//
// MODULE  : AINavMeshGenImportExportGame.cpp
//
// PURPOSE : Game-side AI NavMesh generator import/export implementation.
//
// CREATED : 12/04/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AINavMeshGen.h"
#include "AINavMeshGenPoly.h"
#include "AINavMeshGenCarver.h"
#include "AINavMeshGenQuadTree.h"
#include "AINavMeshGenMacros.h"

#include "AINavMesh.h"
#include "AINavMeshLinkAbstract.h"
#include "AINodeMgr.h"

#if defined(PLATFORM_XENON)
// XENON: Necessary code for implementing runtime swapping
#include "endianswitch.h"
#endif // PLATFORM_XENON

#define AINAVMESH_BLINDOBJECTID		0x83f47c31		// ID for AINavMesh blind object data

bool ImportCarver( CAINavMeshGenCarver* pCarver, uint8*& curBlindData );


//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGen::ImportRawNavMesh
//              
//	PURPOSE:	Import raw NavMesh data.
//              
//----------------------------------------------------------------------------

bool CAINavMeshGen::ImportRawNavMesh( const uint8* blindData )
	{
	// sanity check
	if( !blindData )
		return false;

	// Run-time NavMesh is required to access the BlindObject data.

	// ********************************************************************
	// the blindData buffer needs to be right after the first uint32 
	// that indicates whether or not the data is raw data or processed data
	// ********************************************************************

	const uint32 nSizeFloat = sizeof(float);
	const uint32 nSizeInt = sizeof(int);
	const uint32 nSizeShort = sizeof(uint16);

	const uint8* curBlindData = blindData;

#if defined(PLATFORM_XENON)
	// XENON: Swap data at runtime
	LittleEndianToNative((uint32*)curBlindData, 1);
#endif // PLATFORM_XENON

	// Get the AINavMesh version number.

	m_nAINavMeshVersion = *((uint32*)curBlindData);
	curBlindData += nSizeInt;

	// Bail if incorrect version!

	if( m_nAINavMeshVersion != AINAVMESH_VERSION_NUMBER )
	{
		NAVMESH_ERROR2( "CAINavMeshGen::ImportBrushes: AINavMesh version numbers do not match! WorldPacker: %d Object: %d", m_nAINavMeshVersion, AINAVMESH_VERSION_NUMBER );
		return false;
	}

#if defined(PLATFORM_XENON)
	// XENON: Swap data at runtime
		// Note: Each vertex is an LTVector
		LittleEndianToNative((LTVector*)curBlindData, 2);
#endif // PLATFORM_XENON

	// get the bounds of the NavMesh.
	m_aabbWorldBox.vMin.x = *((float*)curBlindData);
	curBlindData += nSizeFloat;
	m_aabbWorldBox.vMin.y = *((float*)curBlindData);
	curBlindData += nSizeFloat;
	m_aabbWorldBox.vMin.z = *((float*)curBlindData);
	curBlindData += nSizeFloat;

	m_aabbWorldBox.vMax.x = *((float*)curBlindData);
	curBlindData += nSizeFloat;
	m_aabbWorldBox.vMax.y = *((float*)curBlindData);
	curBlindData += nSizeFloat;
	m_aabbWorldBox.vMax.z = *((float*)curBlindData);
	curBlindData += nSizeFloat;

	// Create quad tree to hold NavMesh polys.

	m_pNMGQuadTree = CreateNMGQTNode();
	m_pNMGQuadTree->InitQuadTreeNode( m_aabbWorldBox, CAINavMeshGenQuadTreeNode::kNMGQTLevel_Root );

#if defined(PLATFORM_XENON)
	// XENON: Swap data at runtime
	LittleEndianToNative((uint32*)curBlindData, 2);
#endif // PLATFORM_XENON

	// get the number of NavMesh objects
	uint32 cNavMeshObjects = *((uint32*)curBlindData);
	curBlindData += nSizeInt;

	// get the number of brushes
	uint32 cBrushes = *((uint32*)curBlindData);
	curBlindData += nSizeInt;

	uint32 cVerts;

	CAINavMeshGenPoly* pPoly;
	for( uint32 iBrush=0; iBrush < cBrushes; ++iBrush )
	{
		// Create new poly.

		pPoly = NAVMESH_NEW1( CAINavMeshGenPoly, m_pfnErrorCallback );

#if defined(PLATFORM_XENON)
		// XENON: Swap data at runtime
		// Note: Each brush has 3 uint32s at the beginning
		LittleEndianToNative((uint32*)curBlindData, 3);
#endif // PLATFORM_XENON

		// Set the parent NavMesh.

		pPoly->SetNMGParentNavMeshID( ( ENUM_NMGNavMeshID )*((uint32*)curBlindData) );
		curBlindData += nSizeInt;

		// Set the NavMeshLink ID.

		pPoly->SetNMGLinkID( ( ENUM_NMGLinkID )*((uint32*)curBlindData) );
		curBlindData += nSizeInt;

		// Read the number of verts.

		cVerts = *((uint32*)curBlindData);	
		curBlindData += nSizeInt;

#if defined(PLATFORM_XENON)
		// XENON: Swap data at runtime
		// Note: Each vertex is an LTVector
		LittleEndianToNative((LTVector*)curBlindData, cVerts);
#endif // PLATFORM_XENON

		for( uint32 iVert=0; iVert < cVerts; ++iVert )
		{
			// get the verts.
			LTVector vVert;
			vVert.x = *((float*)curBlindData);
			curBlindData += nSizeFloat;
			vVert.y = *((float*)curBlindData);
			curBlindData += nSizeFloat;
			vVert.z = *((float*)curBlindData);
			curBlindData += nSizeFloat;

			// Get vertex indices from vert pool.
			// This reduces memory usage by vectors, and
			// simplifies finding neighbors.

			ENUM_NMGVertID eVert = AddVertToPool( vVert );
			pPoly->AddNMGVert( eVert );
		}

		// Ensure raw polys do not contain superfluous verts.

		pPoly->SimplifyNMGPoly();

		// Init Poly.

		pPoly->InitNMGPoly();

		// Add new poly to NavMesh.

		AddNMGPoly( pPoly );
	}


	// get the number of carvers
	uint32 cCarvers = *((uint32*)curBlindData);
	curBlindData += nSizeInt;

#if defined(PLATFORM_XENON)
	// XENON: Swap data at runtime
	LittleEndianToNative(&cCarvers);
#endif // PLATFORM_XENON

	CAINavMeshGenCarver* pCarver;
	uint32 iCarver;
	for( iCarver=0; iCarver < cCarvers; ++iCarver )
	{
		// Create new Carver.

		pCarver = NAVMESH_NEW1( CAINavMeshGenCarver, m_pfnErrorCallback );

		// Read carver from BlindObjectData.

		ImportCarver( pCarver, (uint8*&)curBlindData );

		// Add new Carver to NavMesh.

		AddNMGCarver( pCarver );
	}


	// get the number of AIRegions
	uint32 cAIRegions = *((uint32*)curBlindData);
	curBlindData += nSizeInt;

#if defined(PLATFORM_XENON)
	// XENON: Swap data at runtime
	LittleEndianToNative(&cAIRegions);
#endif // PLATFORM_XENON

	SAINAVMESHGEN_REGION* pAIRegion;
	for( uint32 iAIRegion=0; iAIRegion < cAIRegions; ++iAIRegion )
	{
		// Create new AIRegions.

		pAIRegion = NAVMESH_NEW( SAINAVMESHGEN_REGION );

		// Get the number of Carvers in the AIRegion.

		cCarvers = *((uint32*)curBlindData);
		curBlindData += nSizeInt;

#if defined(PLATFORM_XENON)
		// XENON: Swap data at runtime
		LittleEndianToNative(&cCarvers);
#endif // PLATFORM_XENON

		// Allocate space for the list of carvers.

		pAIRegion->lstCarvers.reserve( cCarvers );

		for( iCarver=0; iCarver < cCarvers; ++iCarver )
		{
			// Create new Carver.

			pCarver = NAVMESH_NEW1( CAINavMeshGenCarver, m_pfnErrorCallback );

			// Read carver from BlindObjectData.

			ImportCarver( pCarver, (uint8*&)curBlindData );

			// Add carver to AIRegion's list of carvers.

			pAIRegion->lstCarvers.push_back( pCarver );
		}

		// Add new AIRegion to NavMesh.

		AddNMGRegion( pAIRegion );
	}


	// get the number of AINodes
	uint32 cAINodes = *((uint32*)curBlindData);
	curBlindData += nSizeInt;
	
#if defined(PLATFORM_XENON)
	// XENON: Swap data at runtime
	LittleEndianToNative(&cAINodes);
#endif // PLATFORM_XENON

	// Resize the node list.

	m_lstNMGNodes.resize( cAINodes );

	uint32 nNameLen;
	char szNameBuffer[64];
	SAINAVMESHGEN_NODE* pNMGNode;
	AINAVMESHGEN_NODE_LIST::iterator itNMGNode = m_lstNMGNodes.begin();
	for( itNMGNode = m_lstNMGNodes.begin(); itNMGNode != m_lstNMGNodes.end(); ++itNMGNode )
	{
#if defined(PLATFORM_XENON)
		// XENON: Swap data at runtime
		// Note: Each node has 2 uint32s at the beginning
		LittleEndianToNative((uint32*)curBlindData, 1);
#endif // PLATFORM_XENON

		// Read the node ID from blind data.

		pNMGNode = &( *itNMGNode );
		pNMGNode->eNMGNodeID = (ENUM_NMGNodeID)( *((uint32*)curBlindData) );
		curBlindData += nSizeInt;

#if defined(PLATFORM_XENON)
		// XENON: Swap data at runtime
		// Note: Each node has 2 uint32s at the beginning
		LittleEndianToNative((uint16*)curBlindData, 1);
#endif // PLATFORM_XENON

		// Read the length of the string name.

		nNameLen = *((uint16*)curBlindData);
		curBlindData += nSizeShort;

		// Copy the name.
		uint32 iChar;
		for( iChar=0; iChar < nNameLen; ++iChar )
		{
			szNameBuffer[iChar] = *((char*)curBlindData);
			++curBlindData;
		}
		szNameBuffer[iChar] = '\0';
		pNMGNode->strName = szNameBuffer;

#if defined(PLATFORM_XENON)
		// XENON: Swap data at runtime
		// Note: Each vertex is an LTVector
		LittleEndianToNative((LTVector*)curBlindData, 1);
#endif // PLATFORM_XENON

		// Read the position.

		pNMGNode->vPos.x = *((float*)curBlindData);
		curBlindData += nSizeFloat;
		pNMGNode->vPos.y = *((float*)curBlindData);
		curBlindData += nSizeFloat;
		pNMGNode->vPos.z = *((float*)curBlindData);
		curBlindData += nSizeFloat;
		}


	// get the number of links
	uint32 cNMGLinks = *((uint32*)curBlindData);
	curBlindData += nSizeInt;

#if defined(PLATFORM_XENON)
	// XENON: Swap data at runtime
	LittleEndianToNative(&cNMGLinks);
#endif // PLATFORM_XENON

	// Resize the navmesh link list.

	m_lstNMGLinks.resize( cNMGLinks );

	SAINAVMESHGEN_LINK* pNMGLink;
	AINAVMESHGEN_LINK_LIST::iterator itNMGLink;
	for( itNMGLink = m_lstNMGLinks.begin(); itNMGLink != m_lstNMGLinks.end(); ++itNMGLink )
	{
#if defined(PLATFORM_XENON)
		// XENON: Swap data at runtime
		// Note: Each link has 3 uint32s at the beginning
		LittleEndianToNative((uint32*)curBlindData, 3);
#endif // PLATFORM_XENON

		// Read the link ID from blind data.

		pNMGLink = &( *itNMGLink );
		pNMGLink->eNMGLinkID = (ENUM_NMGLinkID)( *((uint32*)curBlindData) );
		curBlindData += nSizeInt;

		// Read the sensory flag.

		pNMGLink->bIsSensoryLink = !!( *((uint32*)curBlindData) );
		curBlindData += nSizeInt;

		// Read the number of boundary verts.

		cVerts = *((uint32*)curBlindData);	
		curBlindData += nSizeInt;

#if defined(PLATFORM_XENON)
		// XENON: Swap data at runtime
		// Note: Each vertex is an LTVector
		LittleEndianToNative((LTVector*)curBlindData, cVerts);
#endif // PLATFORM_XENON

		pNMGLink->lstLinkBounds.resize( cVerts );
		for( uint32 iVert=0; iVert < cVerts; ++iVert )
		{
			// get the verts.
			LTVector vVert;
			vVert.x = *((float*)curBlindData);
			curBlindData += nSizeFloat;
			vVert.y = *((float*)curBlindData);
			curBlindData += nSizeFloat;
			vVert.z = *((float*)curBlindData);
			curBlindData += nSizeFloat;

			pNMGLink->lstLinkBounds[iVert] = vVert;
		}
		}


	// get the number of restrictions
	uint32 cNMGRestrictions = *((uint32*)curBlindData);
	curBlindData += nSizeInt;

#if defined(PLATFORM_XENON)
	// XENON: Swap data at runtime
	LittleEndianToNative(&cNMGRestrictions);
#endif // PLATFORM_XENON

	// Resize the navmesh restrictions list.

	m_lstNMGRestrictions.resize( cNMGRestrictions );

	// Read restrictions and generate masks.

	uint32 nTypeLen;
	char szTypeBuffer[32];
	ENUM_AIAttributesID eAttributesID;
	SAINAVMESHGEN_RESTRICTIONS* pNMGRestrictions;
	AINAVMESHGEN_RESTRICTIONS_LIST::iterator itNMGRestrictions;
	for( itNMGRestrictions = m_lstNMGRestrictions.begin(); itNMGRestrictions != m_lstNMGRestrictions.end(); ++itNMGRestrictions )
		{
#if defined(PLATFORM_XENON)
		// XENON: Swap data at runtime
		// Note: Each restriction has 1 uint32s at the beginning
		LittleEndianToNative((uint32*)curBlindData, 1);
#endif // PLATFORM_XENON

		// Read the navmesh index from blind data.

		pNMGRestrictions = &( *itNMGRestrictions );
		pNMGRestrictions->eNavMeshID = (ENUM_NMGNavMeshID)( *((uint32*)curBlindData) );
		curBlindData += nSizeInt;

#if defined(PLATFORM_XENON)
		// XENON: Swap data at runtime
		// Note: Each restriction has 1 uint16s next
		LittleEndianToNative((uint16*)curBlindData, 1);
#endif // PLATFORM_XENON

		// Read the AINavMeshType strings length.

		nTypeLen = *((uint16*)curBlindData);
		curBlindData += nSizeShort;

		// Read the AINavMeshType string and convert it to a pointer.

		char szAINavMeshTypeBuffer[64];
		uint32 iChar;
		for( iChar=0; iChar < nTypeLen; ++iChar )
		{
			szAINavMeshTypeBuffer[iChar] = *((char*)curBlindData);
			++curBlindData;
		}
		szAINavMeshTypeBuffer[iChar] = '\0';

		uint32 dwAINavMeshTypeCharacterMask = 0;
 		for ( uint32 i = 0; i < g_pAIDB->GetNumAINavMeshTypes(); ++i )
		{
			const AIDB_AINavMeshTypeRecord* pCurrentRecord = g_pAIDB->GetAINavMeshTypeRecord( i );
			if ( LTStrEquals( pCurrentRecord->m_szName,  szAINavMeshTypeBuffer ) )
			{
				dwAINavMeshTypeCharacterMask = pCurrentRecord->m_dwCharacterTypeMask;
				break;
			}
		}

#if defined(PLATFORM_XENON)
		// XENON: Swap data at runtime
		// Note: Each restriction has 2 uint32s here
		LittleEndianToNative((uint32*)curBlindData, 2);
#endif // PLATFORM_XENON

		// Read the include flag.

		bool bInclude = !!( *((uint32*)curBlindData) );
		curBlindData += nSizeInt;

		// Setup mask for include or exclude.

		if( bInclude )
		{
			pNMGRestrictions->dwCharTypeMask = 0;
		}
		else {
			pNMGRestrictions->dwCharTypeMask = ALL_CHAR_TYPES;
		}

		// Read the number of character types.

		uint32 cTypes = *((uint32*)curBlindData);
		curBlindData += nSizeInt;

		// Add each character type to the mask.

		for( uint32 iType=0; iType < cTypes; ++iType )
	{
#if defined(PLATFORM_XENON)
			// XENON: Swap data at runtime
			// Note: Each node has 2 uint32s at the beginning
			LittleEndianToNative((uint16*)curBlindData, 1);
#endif // PLATFORM_XENON

			// Read the length of the string type.

			nTypeLen = *((uint16*)curBlindData);
			curBlindData += nSizeShort;

			// Read the string type.
			uint32 iChar;
			for( iChar=0; iChar < nTypeLen; ++iChar )
			{
				szTypeBuffer[iChar] = *((char*)curBlindData);
				++curBlindData;
			}
			szTypeBuffer[iChar] = '\0';

			// Convert the string to an ID, and set its bit in the mask.

			eAttributesID = g_pAIDB->GetAIAttributesRecordID( szTypeBuffer );
			if( eAttributesID != kAIAttributesID_Invalid )
			{
				if( bInclude )
				{
					pNMGRestrictions->dwCharTypeMask |= ( 1 << eAttributesID );
				}
				else {
					pNMGRestrictions->dwCharTypeMask &= ~( 1 << eAttributesID );
				}
			}
		}

		// If no restrictions are set, this nav mesh is usable by all types by 
		// default.  This logic matches that in AICharacterRestrictions.cpp.

		if( 0 == cTypes )
		{
			pNMGRestrictions->dwCharTypeMask = ALL_CHAR_TYPES;
		}

		// Reduce the mask to only include types listed in the AINavMeshType 
		// filter.  If this isn't done, polies which should not be merged may 
		// be.  This logic matches that in AINavMesh.cpp

		pNMGRestrictions->dwCharTypeMask &= dwAINavMeshTypeCharacterMask;
	}

	return true;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	ImportCarver()
//              
//	PURPOSE:	Import NavMesh polygons by converting Brushes.
//              
//----------------------------------------------------------------------------

bool ImportCarver( CAINavMeshGenCarver* pCarver, uint8*& curBlindData )
{
	// Bail if AINavMesh or carver does not exist.

	if( !pCarver )
	{
		return false;
	}

	uint32 nSizeFloat = sizeof(float);
	uint32 nSizeInt = sizeof(int);

	float fHeight = *((float*)curBlindData);	
	curBlindData += nSizeFloat;

#if defined(PLATFORM_XENON)
	// XENON: Swap data at runtime
	LittleEndianToNative(&fHeight);
#endif // PLATFORM_XENON

	// Set height of the Carver.

	pCarver->SetNMGCarverHeight( fHeight );

	uint32 cVerts = *((uint32*)curBlindData);	
	curBlindData += nSizeInt;

#if defined(PLATFORM_XENON)
	// XENON: Swap data at runtime
	LittleEndianToNative(&cVerts);
	// Note: Each carver is a list of LTVectors
	LittleEndianToNative((LTVector*)curBlindData, cVerts);
#endif // PLATFORM_XENON

	for( uint32 iVert=0; iVert < cVerts; ++iVert )
	{
		LTVector vVert;
		vVert.x = *((float*)curBlindData);
		curBlindData += nSizeFloat;
		vVert.y = *((float*)curBlindData);
		curBlindData += nSizeFloat;
		vVert.z = *((float*)curBlindData);
		curBlindData += nSizeFloat;

		// Add the vertex to the carver.

		pCarver->AddNMGCarverVert( vVert );
	}

	// Init Carver.

	pCarver->InitNMGCarver();

	return true;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGen::ExportPackedNavMesh
//              
//	PURPOSE:	Export a packed NavMesh.
//              
//----------------------------------------------------------------------------

bool CAINavMeshGen::ExportPackedNavMesh( ILTOutConverter& Converter )
{
	// Pack it.
	if( !PackRunTimeNavMesh(Converter) )
		return false;

	return true;
}

