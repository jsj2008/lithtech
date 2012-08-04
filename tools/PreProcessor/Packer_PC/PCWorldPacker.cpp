#include "bdefs.h"
#include "preworld.h"
#include "prepoly.h"
#include "geomroutines.h"
#include "pregeometry.h"
#include "processing.h"
#include "sysstreamsim.h"
#include "processing.h"
#include "filemarker.h"
#include "PCWorldPacker.h"
#include "pcrenderworld.h"


// 33 - Added multiple world models
// 34 - Compressed vis lists
// 35 - Added pPoly->m_CompositeX and pPoly->m_CompositeY
// 36 - Lightmaps from 1 byte to 2 bytes
// 37 - World names are saved now (used for WorldModels .. their name = their object name).
// 38 - Surface detail level saved out.
// 39 - Added a bounding box to the WorldBsp structure (mainly for WorldModel info..)
// 40 - Added the tree depth for each BSP
// 42 - Fixed propLen in the object list.
// 43 - Added surface effects.
// 44 - Changed surface effects to strings.
// 45 - Removed world vertex colors and made them per-poly and removed edge index per vertex.
// 46 - Added lightmapping info like total lightmap data size.
// 47 - Added surface shade.
// 48 - Added texture flags to the surfaces.
// 49 - Added portals.
// 50 - Added position extents (calculated in processing.cpp).
// 51 - Surface m_Flags WORD to DWORD.
// 52 - Added leaf vis dist.
// 53 - Added TextureP and TextureQ (lightmap system is P and Q).
// 54 - Added the EllipseBSPs.
// 55 - Added m_pInfoString.
// 56 - Added header dwords and vertex flags.
// 57 - Added some dummy dwords at the start of the file and added m_LMGridSize.
// 58 - Added vertex normals.
// 59 - Added the portal list.
// 60 - World tree restructuring
// 61 - Lightmap animations
// 62 - Poly animref lists.
// 63 - Shadow maps.
// 64 - Removed Surface::P and Surface::Q.
// 65 - Changed leaf touch lists to point at polies.
// 66 - Added visibility and physics BSPs. Made touch lists store world index and poly index.
// 67 - Added 24 bit lightmaps and per surface LM Grid Sizes
// 68 - Added the new physics data
// 69 - Changed PBlockTable format and added specialized vert type data structs.
// 70 - Added the light grid array.
// 71 - Added the new rendering data
// 72 - Added physics blockers
// 73 - Added world models to the new rendering data
// 74 - Increased the PNode world node index to 32-bits
// 75 - Removed all the extra physics data, including the PBlocks
// 76 - Added light groups
// 77 - Added the ambient light table to the light groups
// 78 - Added support for texture effects
// 79 - Added particle blocker data
// 80 - Added surface merging and removed a lot of dead space
// 81 - Added indexed blind data for objects to use as they wish
// 82 - Made all textures have multiple UV coordinates for dual texturing
// 83 - Upped the vertex indices to 32 bit for large new levels
// 84 - Added names to the occluders
//    - Removed world LMGridSize
// 85 - Added the world offset
#define CURRENT_DAT_VERSION	85

//utility macro to aid in writing a vector out to a file

#define SaveVector(file, type, vec) \
	{\
		(file) << (type)((vec).x);\
		(file) << (type)((vec).y);\
		(file) << (type)((vec).z);\
	}

//given a world, it will run through and build a map of 
static uint32 MergeSurfaces(CPreWorld* pWorld)
{
	//have it go through and tag used surfaces
	pWorld->TagUsedSurfaces();

	//allocate a list of surfaces
	CPreSurface** ppSurfaceList = new CPreSurface* [pWorld->m_Surfaces.GetSize()];
	uint32    nSurfCount = 0;

	//now we need to go through and find similar surfaces and remove those
	for(GPOS Curr = pWorld->m_Surfaces; Curr; )
	{
		CPreSurface* pSurf = pWorld->m_Surfaces.GetNext(Curr);

		//skip surfaces that aren't used
		if(!pSurf->m_bUsed)
			continue;

		//see if one in the list already matches
		bool bFound = false;
		for(uint32 nTest = 0; nTest < nSurfCount; nTest++)
		{
			CPreSurface* pTest = ppSurfaceList[nTest];

			//match up the textures
			bool bMatchTex = true;
			for(uint32 nCurrTex = 0; nCurrTex < CPreSurface::NUM_TEXTURES; nCurrTex++)
			{
				if(pSurf->m_Texture[nCurrTex].IsValid() != pTest->m_Texture[nCurrTex].IsValid())
				{
					bMatchTex = false;
					break;
				}

				//only continue if the texture stage is valid
				if(!pSurf->m_Texture[nCurrTex].IsValid())
					continue;

				//compare the properties
				if(pSurf->m_Texture[nCurrTex].m_pTextureName != pTest->m_Texture[nCurrTex].m_pTextureName)
				{
					bMatchTex = false;
					break;
				}
			}

			//don't bother continuing if the textures don't match
			if(!bMatchTex)
				continue;

			if(pSurf->m_Flags == pTest->m_Flags)
			{
				bFound = true;

				//they do match, map this surface to the base one and bail
				pSurf->m_UseIndex	= nTest;
				pSurf->m_bUsed		= FALSE;

				break;
			}
		}

		//if it wasn't found, we need to add it to the list
		if(!bFound)
		{
			ppSurfaceList[nSurfCount] = pSurf;
			pSurf->m_UseIndex = nSurfCount;

			nSurfCount++;
		}
	}

	//we can free our temporary list now...
	delete [] ppSurfaceList;

	return nSurfCount;
}

static bool WriteWorldBspData( CPreWorld* pWorld, CAbstractIO &file )
{
	uint32					i, k, curPoly;
	uint32					index;
	CPrePoly				*pPoly;
	CMoArray<char*>			texNames;
	uint32					namesLen;

	uint32					nPointsUsed;
	CMoByteArray			pointsUsed;
	CMoDWordArray			indexMap;
	GPOS					pos;
	CPrePlane				*pPlane;
	CPreSurface				*pSurface;
	CNode					*pNode;

	uint32 nUsedSurfaces;
	PVector min, max;
	PVector minBox, maxBox, center, dims;
	
	uint32 nStartOfData = file.GetCurPos();


	pWorld->m_TextureNames.Term();
	pWorld->m_TextureNames.SetCacheSize( 100 );
	pWorld->GetTextureNames( pWorld->m_TextureNames );

	SetNodeIndices(pWorld->m_Nodes);

	pWorld->SetupIndexMap( pointsUsed, indexMap, nPointsUsed );
	pWorld->m_nPointsSaved = nPointsUsed;
	
	file << pWorld->m_WorldInfoFlags;
	// Write the name (mainly used for WorldModels).
	file.WriteString(pWorld->m_WorldName);

	// Tells where the client data is.
	file << nPointsUsed;

	file << (uint32)pWorld->m_Planes.GetSize();
	
	nUsedSurfaces = MergeSurfaces(pWorld);
	file << nUsedSurfaces;

	//write out how many portals...but the engine doesn't support portals anymore, so just write
	//out 0
	file << (uint32)0;		

	file << pWorld->m_Polies.GetSize();

	//write out how many leaves. But the engine doesn't use these either, so just write 0
	file << (uint32)0;

	file << pWorld->NumPolyVerts();

	//we also don't care about the visible list size
	file << (uint32)0;
	//or leaf lists
	file << (uint32)0;

	file << pWorld->m_Nodes.GetSize();	

	// Setup the bounding box and save it off.
	pWorld->GetBoundingBox(&min, &max);

	file << (float)min.x << (float)min.y << (float)min.z;
	file << (float)max.x << (float)max.y << (float)max.z;

	//save out the translation vector
	file << (float)pWorld->m_WorldTranslation.x;
	file << (float)pWorld->m_WorldTranslation.y;
	file << (float)pWorld->m_WorldTranslation.z;


	// Save out texture name stuff.
	namesLen = 0;
	for( i=0; i < pWorld->m_TextureNames.GetSize(); i++ )
	{
		namesLen += strlen(pWorld->m_TextureNames[i]) + 1;
	}

	uint32 nTexturesStart = file.GetCurPos();

	file << namesLen;
	file << pWorld->m_TextureNames.GetSize();

	for( i=0; i < pWorld->m_TextureNames.GetSize(); i++ )
	{
		file.Write( pWorld->m_TextureNames[i], strlen(pWorld->m_TextureNames[i]) + 1 );	
	}

	uint32 nPolyVertCountStart = file.GetCurPos();


	// Give each poly its index.
	pWorld->SetPolyWorldIndices();
	pWorld->SetPolyIndices();
	
	// Give each plane its indices.
	pWorld->SetPlaneIndices();

	// Write out the number of indices for polies.
	for(pos = pWorld->m_Polies; pos; )
	{
		CPrePoly* pPrePoly = pWorld->m_Polies.GetNext(pos);
		file << (BYTE)pPrePoly->NumVerts();
	}

	uint32 nPlaneStart = file.GetCurPos();

	// Write out the planes.
	for( pos = pWorld->m_Planes.GetHeadPosition(); pos; )
	{
		pPlane = pWorld->m_Planes.GetNext( pos );

		SaveVector( file, float, pPlane->m_Normal );
		file << (float)pPlane->m_Dist;
	}

	uint32 nSurfaceStart = file.GetCurPos();

	// Save out the surfaces.
	for( pos = pWorld->m_Surfaces.GetHeadPosition(); pos; )
	{
		pSurface = pWorld->m_Surfaces.GetNext( pos );
	
		if(pSurface->m_bUsed)
		{
			file << (uint32)pSurface->m_Flags;
			file << pWorld->FindTextureName( pSurface->m_Texture[0].m_pTextureName, pWorld->m_TextureNames );
			file << pSurface->m_Texture[0].m_TextureFlags;
		}
	}

	uint32 nPolygonStart = file.GetCurPos();

	// Save the polygons.
	curPoly = 0;
	for(pos = pWorld->m_Polies; pos; )
	{
		pPoly = pWorld->m_Polies.GetNext(pos);

		ASSERT(pPoly->m_Index == curPoly);
	
		// Surface index.
		file << (uint32)pPoly->GetSurface()->m_UseIndex;
		file << (uint32)pPoly->GetSurface()->m_pPlane->GetIndex();

		// Write the vertices.
		for(k=0; k < pPoly->NumVerts(); k++)
		{
			file << (uint32)indexMap[pPoly->Index(k)];
		}

		++curPoly;
	}

	uint32 nNodeStart = file.GetCurPos();

	// Save the trees (save the whales, save the earth?).
	for(pos = pWorld->m_Nodes; pos; )
	{
		pNode = pWorld->m_Nodes.GetNext(pos);
		
		//index = m_Polies.FindElement( pNode->m_pPoly );
		index = pNode->m_pPoly->m_Index;
		ASSERT( index != BAD_INDEX );
		file << index;
		ASSERT(index < pWorld->m_Polies.GetSize());
		
		//the leaves are no longer read in by the engine
		file << (WORD)0;
		
		file << NodeToIndex(pNode->m_Sides[0]);
		file << NodeToIndex(pNode->m_Sides[1]);
	}

	uint32 nPointStart = file.GetCurPos();

	// Save m_Points.
	for( i = 0; i < pWorld->m_Points; i++ )
	{
		if( pointsUsed[i] )
		{
			file << (CReal)pWorld->m_Points[i].x;
			file << (CReal)pWorld->m_Points[i].y;
			file << (CReal)pWorld->m_Points[i].z;
		}
	}

	uint32 nSectionStart = file.GetCurPos();

	// Root poly index..
	file << NodeToIndex(pWorld->m_RootNode);

	// Sections, of which have been removed, so always just write out 0. This should
	// be removed in the future
	file << (uint32)0;

	uint32 nEndOfData = file.GetCurPos();

	/*
	DrawStatusText("  World Model : %s (%d bytes)", pWorld->m_WorldName, nEndOfData - nStartOfData);
	if (pWorld->m_pMainWorld && pWorld->m_pMainWorld->GetPhysicsBSP() == pWorld)
	{
		DrawStatusText("   Textures : %d bytes", nPolyVertCountStart - nTexturesStart);
		DrawStatusText("   Poly Vertex count : %d bytes", nPlaneStart - nPolyVertCountStart);
		DrawStatusText("   Planes : %d bytes", nSurfaceStart - nPlaneStart);
		DrawStatusText("   Surfaces : %d bytes", nPolygonStart - nSurfaceStart);
		DrawStatusText("   Polygons : %d bytes", nNodeStart - nPolygonStart);
		DrawStatusText("   Nodes : %d bytes", nPointStart - nNodeStart);
		DrawStatusText("   Vertices : %d bytes", nSectionStart - nPointStart);
		DrawStatusText("   Sections : %d bytes", nEndOfData - nSectionStart);
	}
	*/

	return true;
}

// Outputs a span (for RLE compression of the lightgrid) into a byte array, and updates the pointer accordingly...
static void LightGrid_OutputSpan(uint32 iPass, bool bRun, uint32 nLength, uint8* pInData, CAbstractIO &file, uint32* pOutSize)
{
	// Sanity checks
	assert(pInData); assert(nLength <= 128); assert(nLength > 0);

	// Output the tag. The high bit indicates a run, the other 7 bits indicate the span length (-1)
	uint8 nTag	= (bRun) ? 0x80 : 0x00;
	nTag	   |= (uint8)(nLength - 1);		// This is the run length for runs and the span length for non-runs...
	if (iPass) file.Write(&nTag,sizeof(uint8));
	else *pOutSize  += sizeof(uint8);

	if (bRun) {								// Output the span...only a single color if it is a run...
		if (iPass) file.Write(pInData,sizeof(uint8) * 3); 
		else *pOutSize += sizeof(uint8) * 3; }
	else {									// Output all the data in the span...
		if (iPass) file.Write(pInData,sizeof(uint8) * nLength * 3); 
		else *pOutSize += sizeof(uint8) * nLength * 3; }
}

// Compress the LightGrid data (RLE compression)...
//	Tag the first byte, the high bit indicates if it's a run, the remaining bits
// indicate: for a run, the number of repeat colors or for a non-run, the data span length.
// Note: make sure our runs and span's don't cross an x-boundry (so we can store in mem compressed and quick lookup)...
static bool LightGrid_SaveCompressed(uint8* pInData, uint32 X, uint32 Y, uint32 Z, CAbstractIO &file)
{
	if (!pInData) { return false; }			// Basic error checking...

	// Do it twice - first time just figure out the size...
	uint32 outLen = 0;
	for (uint32 iPass = 0; iPass < 2; ++iPass) {
		if (iPass) file.Write(&outLen,sizeof(uint32));

		uint32 nBufferLen	= X * Y * Z * 3;	// Number of bytes in the input buffer
		bool   bInRun		= false;			// Flag indicating if we are in a run
		uint32 nSpanLen		= 0;				// The span, whether it be run, or raw data

		// Run through the input buffer...
		uint32 x			= X;				// To make sure we don't cross an X boundry (see note above)...
		for (uint32 nCurrPel = 0; nCurrPel < nBufferLen; nCurrPel += 3) {
			uint32 nRunLen = 1;					// Run length...

			// Check and see if we are starting a run
			for (uint32 nRunPel = nCurrPel + 3; nRunPel < nBufferLen; nRunPel += 3, ++nRunLen) {
				// Check to see if the next data matches the current one...
				if ((pInData[nCurrPel + 0] != pInData[nRunPel + 0]) || (pInData[nCurrPel + 1] != pInData[nRunPel + 1]) || (pInData[nCurrPel + 2] != pInData[nRunPel + 2])) {
					break; }					// Different color, bust out

				if (x-nRunLen-nSpanLen == 0) { break; }	// On a boundry, bust on out...
				if (nRunLen > 127) { break; } }	// Need to make sure that the run length is still in range

			if (nRunLen >= 2) {					// See if we hit a run
				if (nSpanLen > 0) {				// We hit a run - output the old span if we had one
					LightGrid_OutputSpan(iPass,false, nSpanLen, &pInData[nCurrPel - nSpanLen * 3], file, &outLen); 
					x -= nSpanLen; nSpanLen = 0; } // Reset the span...

				// Output the run span...
				LightGrid_OutputSpan(iPass,true, nRunLen, &pInData[nCurrPel], file, &outLen);
				x -= nRunLen;
		
				nSpanLen = 0;					// Start the new span...
				nCurrPel += (nRunLen-1) * 3; }	// Update the offset (the -1 is to counteract the loop increment)...
			else {								// No run, so add this onto the current span...
				++nSpanLen; }

			if (nSpanLen > 0 && (nSpanLen > 127 || (x-nSpanLen == 0))) {	// Check to see if we have to output a span (and if so, output it)...
				LightGrid_OutputSpan(iPass,false, nSpanLen, &pInData[nCurrPel - nSpanLen * 3], file, &outLen);
				x -= nSpanLen; nSpanLen = 0; }	// Reset the span...

			if (x == 0) x = X; }				// If we're at the end of a X line, reset it...

		if (nSpanLen > 0) {						// Make sure we finished off the final span
			LightGrid_OutputSpan(iPass,false, nSpanLen, &pInData[nCurrPel - nSpanLen * 3], file, &outLen); } }

	return true;
}

static bool SaveFastLight_LightGrid(CPreMainWorld* pMainWorld, CAbstractIO &file)
{
	file << pMainWorld->m_LightTable.m_LookupStart.x;	// World Base Pos...
	file << pMainWorld->m_LightTable.m_LookupStart.y;
	file << pMainWorld->m_LightTable.m_LookupStart.z;
	file << pMainWorld->m_LightTable.m_BlockSize.x;		// World Dims...
	file << pMainWorld->m_LightTable.m_BlockSize.y;
	file << pMainWorld->m_LightTable.m_BlockSize.z;
	file << pMainWorld->m_LightTable.m_LookupSize[0];
	file << pMainWorld->m_LightTable.m_LookupSize[1];
	file << pMainWorld->m_LightTable.m_LookupSize[2];

	if (!pMainWorld->m_pLightGrid) return true;		// Write out the data...

	// Alloc us up a tmp buffer and fill in data...
	uint8* pLightData = new uint8[pMainWorld->m_LightTable.m_LookupSize[2] * pMainWorld->m_LightTable.m_LookupSize[1] * pMainWorld->m_LightTable.m_LookupSize[0] * 3];
	if (!pLightData) return false;
	uint8* pDat = pLightData;
	for (uint32 z = 0; z < pMainWorld->m_LightTable.m_LookupSize[2]; ++z) {
		for (uint32 y = 0; y < pMainWorld->m_LightTable.m_LookupSize[1]; ++y) {
			for (uint32 x = 0; x < pMainWorld->m_LightTable.m_LookupSize[0]; ++x) {
				uint32 iIndex = z * pMainWorld->m_LightTable.m_LookupSize[0] * pMainWorld->m_LightTable.m_LookupSize[1] + y * pMainWorld->m_LightTable.m_LookupSize[0] + x;
				*pDat = (uint8)LTMIN((uint32)pMainWorld->m_pLightGrid[iIndex].x,0xFF); ++pDat;
				*pDat = (uint8)LTMIN((uint32)pMainWorld->m_pLightGrid[iIndex].y,0xFF); ++pDat;
				*pDat = (uint8)LTMIN((uint32)pMainWorld->m_pLightGrid[iIndex].z,0xFF); ++pDat; } } } 
				//file.Write(&r,sizeof(uint8)); file.Write(&g,sizeof(uint8)); file.Write(&b,sizeof(uint8)); } } } 

	if (!LightGrid_SaveCompressed(pLightData,pMainWorld->m_LightTable.m_LookupSize[0],pMainWorld->m_LightTable.m_LookupSize[1],pMainWorld->m_LightTable.m_LookupSize[2],file)) return false;
	delete[] pLightData;

	return true;
}

static bool SaveLightGroups(CPreMainWorld* pMainWorld, CAbstractIO &file)
{
	// Write the light groups
	file << (uint32)pMainWorld->m_aLightGroups.size();
	CPreMainWorld::TLightGroupList::const_iterator iCurLightGroup = pMainWorld->m_aLightGroups.begin();
	for (; iCurLightGroup != pMainWorld->m_aLightGroups.end(); ++iCurLightGroup)
	{
		if (!file.WriteString((*iCurLightGroup)->GetName()))
			return false;
		SaveVector(file, float, (*iCurLightGroup)->GetColor());

		// Note : Poly data is saved as part of the rendering tree

		// Save the light grid
		const CPreLightGroup::SLightGrid &cLightGrid = (*iCurLightGroup)->GetLightGrid();
		SaveVector(file, uint32, cLightGrid.m_vOffset);
		SaveVector(file, uint32, cLightGrid.m_vSize);
		file.Write(cLightGrid.m_pSamples, cLightGrid.GetTotalSampleCount());
	}

	return true;
}

static bool SaveRenderData(CPreMainWorld* pMainWorld, CAbstractIO &file, const char* pszFilename, bool bObjectsOnly)
{
	CPCRenderWorld cRenderWorld;

	if (!cRenderWorld.Process(pMainWorld))
		return false;

	if (!cRenderWorld.Write(file))
		return false;

	if (!SaveLightGroups(pMainWorld, file))
		return false;

	return true;
}
   
static bool SaveExtraPhysicsData(CPreMainWorld* pMainWorld, CAbstractIO &file)
{
	// Get the list of blocker polys from the world
	CPreMainWorld::TBlockerPolyList &aPolyList = pMainWorld->GetBlockerPolys();

	// Remember how many we've got
	file << (uint32)aPolyList.GetSize();

	// Write 'em out
	for (uint32 nCurPoly = 0; nCurPoly < aPolyList.GetSize(); ++nCurPoly)
	{
		CPreBlockerPoly *pCurPoly = aPolyList[nCurPoly];

		CPrePlane *pPlane = &pCurPoly->GetPlane();
		file << pPlane->m_Normal.x;
		file << pPlane->m_Normal.y;
		file << pPlane->m_Normal.z;
		file << pPlane->m_Dist;

		file << (uint32)pCurPoly->NumVerts();
		for (uint32 nCurVert = 0; nCurVert < pCurPoly->NumVerts(); ++nCurVert)
		{
			// All we care about is your position.
			LTVector vPos = pCurPoly->Vert(nCurVert).m_Vec;
			file << vPos.x << vPos.y << vPos.z;
		}
	}

	// Leave a trailer for future expansion
	file << (uint32)0;

	return true;
}


static bool SaveParticleBlockers(CPreMainWorld* pMainWorld, CAbstractIO& file)
{
	CPreMainWorld::TBlockerPolyList& aPolyList = pMainWorld->GetParticleBlockerPolys();

	// write out the number of polygons
	file << (uint32)aPolyList.GetSize();

	// write out the polygons
	for( uint32 nCurPoly = 0; nCurPoly < aPolyList.GetSize(); nCurPoly++ )
	{
		CPreBlockerPoly* pCurPoly = aPolyList[nCurPoly];

		CPrePlane* pPlane = &pCurPoly->GetPlane();
		file << pPlane->m_Normal.x;
		file << pPlane->m_Normal.y;
		file << pPlane->m_Normal.z;
		file << pPlane->m_Dist;

		file << (uint32)pCurPoly->NumVerts();
		for( uint32 nCurVert = 0; nCurVert < pCurPoly->NumVerts(); nCurVert++ )
		{
			LTVector vPos = pCurPoly->Vert( nCurVert ).m_Vec;
			file << vPos.x << vPos.y << vPos.z;
		}
	}

	// leave a trailer for future expansion
	file << (uint32)0;

	return true;
}


static bool SaveBlindObjectData( CPreMainWorld* pMainWorld, CAbstractIO& file )
{
	// write out the number of blind data chunks
	file << (uint32)pMainWorld->m_BlindObjectData.size();

	// add each blind data chunk
	std::vector<CPreBlindData*>::iterator it = pMainWorld->m_BlindObjectData.begin();
	for( ; it != pMainWorld->m_BlindObjectData.end(); it++ )
	{
		// write the size of the data
		uint32 dataSize = (*it)->size;
		file << dataSize;

		// write the chunk ID
		uint32 dataID = (*it)->id;
		file << dataID;

		// write the actual data if there is any
		if( dataSize )
		{
			void* data = (*it)->data;
			ASSERT( data );

			file.Write( data, dataSize );
		}
	}

	return true;
}

static uint32 GetBlindObjectDataSize( CPreMainWorld* pMainWorld )
{
	uint32 nTotalSize = 0;

	// the number of blind object data chunks
	nTotalSize += 4;

	// add each blind data chunk
	std::vector<CPreBlindData*>::iterator it = pMainWorld->m_BlindObjectData.begin();
	for( ; it != pMainWorld->m_BlindObjectData.end(); it++ )
	{
		//the size of the data chunk
		nTotalSize += 4;

		//The chunk ID
		nTotalSize += 4;

		// write the size of the data
		uint32 dataSize = (*it)->size;

		//the actual data
		nTotalSize += dataSize;
	}

	return nTotalSize;
}

static void ReportFileSize(EStatusTextType eType, const char *pMsg, uint32 nSize)
{
	char pszBuffer[512];

	//convert to MB
	if(nSize > 1024 * 1024)
	{
		LTSNPrintF(pszBuffer, sizeof(pszBuffer), "%s : %.1f MB", pMsg, (float)nSize / (float)(1024 * 1024));
	}
	else if(nSize > 1024)
	{
		LTSNPrintF(pszBuffer, sizeof(pszBuffer), "%s : %.1f KB", pMsg, (float)nSize / (float)1024);
	}
	else
	{
		LTSNPrintF(pszBuffer, sizeof(pszBuffer), "%s : %d B", pMsg, nSize);
	}

	DrawStatusText(eType, pszBuffer);
}


static bool SaveFile(CPreMainWorld* pMainWorld, CAbstractIO &outFile, const char* pszFilename, bool bObjectsOnly)
{
	uint32 version;
	uint32 i;
	BOOL bRet;
	DStream *pWrapper;

	// Setup the memory IO.
	CAbstractIO &file = outFile;

	// Write a small header.
	version = CURRENT_DAT_VERSION;
	file << version;

	CFileMarker objectListMarker(file, FALSE);
	CFileMarker blindDataMarker(file, FALSE);
	CFileMarker lightGridMarker(file, FALSE);
	CFileMarker physicsDataMarker(file, FALSE);
	CFileMarker particleDataMarker(file, FALSE);
	CFileMarker renderDataMarker(file, FALSE);

	file << (uint32)0 << (uint32)0 << (uint32)0 << (uint32)0;
	file << (uint32)0 << (uint32)0 << (uint32)0 << (uint32)0;

	file << (uint32)strlen(pMainWorld->m_pInfoString);
	file.Write(pMainWorld->m_pInfoString, strlen(pMainWorld->m_pInfoString));
	
	// Write the position box.
	SaveVector(file, float, pMainWorld->m_PosMin);
	SaveVector(file, float, pMainWorld->m_PosMax);

	//save out the offset from this world to the source world
	SaveVector(file, float, pMainWorld->m_vWorldOffset);

	// WorldTree layout.
	pWrapper = streamsim_AbstractIOWrapper(&file);
	if(!pWrapper)
		return false;

	uint32 nWorldTreeStart = file.GetCurPos();

	bRet = pMainWorld->m_WorldTree.SaveLayout(pWrapper);
	pWrapper->Release();
	
	if(!bRet)
		return false;

	uint32 nWorldModelStart = file.GetCurPos();

	// Save the sub-worlds.
	file << (uint32)pMainWorld->m_WorldModels.GetSize();
	for( i=0; i < pMainWorld->m_WorldModels.GetSize(); i++ )
	{
		//dummy 4 bytes
		file << (uint32)0;
		if(!WriteWorldBspData(pMainWorld->m_WorldModels[i], file))
			return false;
	}

	uint32 nObjectStart = file.GetCurPos();

	objectListMarker.Mark();
	SaveObjectList(file, pMainWorld->m_Objects);

	uint32 nBlindDataStart = file.GetCurPos();
	blindDataMarker.Mark();
	if( !SaveBlindObjectData( pMainWorld, file ) )
		return false;

	uint32 nLightGridStart = file.GetCurPos();

	lightGridMarker.Mark();
	if (!SaveFastLight_LightGrid(pMainWorld,file)) return false;

	uint32 nLightData = file.GetCurPos();

	uint32 nPhysicsStart = file.GetCurPos();

	//now save out all the physics data
	physicsDataMarker.Mark();
	if(!SaveExtraPhysicsData(pMainWorld, file))
		return false;

	// save out particle blocker data
	uint32 nParticleStart = file.GetCurPos();
	particleDataMarker.Mark();
	if( !SaveParticleBlockers( pMainWorld, file ) )
		return false;

	uint32 nRenderStart = file.GetCurPos();

	DrawStatusText(eST_Normal, "  ");

	renderDataMarker.Mark();

	if (!SaveRenderData(pMainWorld, file, pszFilename, bObjectsOnly))
		return false;

	DrawStatusText(eST_Normal, "  ");
	ReportFileSize(eST_Normal, "World .DAT file Size", file.GetCurPos());
	ReportFileSize(eST_Normal, "  World Tree", nWorldModelStart - nWorldTreeStart);
	ReportFileSize(eST_Normal, "  World Models", nObjectStart - nWorldModelStart);
	ReportFileSize(eST_Normal, "  Objects", nBlindDataStart - nObjectStart);
	ReportFileSize(eST_Normal, "  Blind Object Data", nLightGridStart - nBlindDataStart);
	ReportFileSize(eST_Normal, "  Light Grid", nPhysicsStart - nLightGridStart);
	ReportFileSize(eST_Normal, "  Blockers", nParticleStart - nPhysicsStart);
	ReportFileSize(eST_Normal, "  Particle Blockers", nRenderStart - nParticleStart);
	ReportFileSize(eST_Normal, "  Render Data", file.GetCurPos() - nRenderStart);

	return true;
}

template <class T>
T ReadMemory(uint8* pMemory)
{
	return *((T*)pMemory);
}

template<class T>
void WriteMemory(uint8* pMemory, T Val)
{
	*((T*)pMemory) = Val;
}

static bool DoObjectsOnlySave(const char* pszOutFile, CPreMainWorld* pMainWorld)
{
	//we first need to open up the output file and read it all in
	CMoFileIO InFile;

	if(!InFile.Open(pszOutFile, "rb"))
		return false;

	//load it all into memory
	uint32 nFileSize = InFile.GetLen();

	//allocate the block and load it into memory
	uint8* pLevel = new uint8[nFileSize];

	//check the allocation
	if(!pLevel)
		return false;

	//load it all in
	if(!InFile.Read(pLevel, nFileSize))
	{
		delete [] pLevel;
		return false;
	}

	//go ahead and close up
	InFile.Close();

	//alright, now we need to skip the version, and read in the offset to the object data
	uint32 nFileVersion			= ReadMemory<uint32>(&pLevel[0]);
	uint32 nObjectMarker		= ReadMemory<uint32>(&pLevel[4]);
	uint32 nBlindDataMarker		= ReadMemory<uint32>(&pLevel[8]);
	uint32 nLightGridMarker		= ReadMemory<uint32>(&pLevel[12]);
	uint32 nPhysicsMarker		= ReadMemory<uint32>(&pLevel[16]);
	uint32 nParticleDataMarker	= ReadMemory<uint32>(&pLevel[20]);
	uint32 nRenderDataMarker	= ReadMemory<uint32>(&pLevel[24]);

	if(nFileVersion != CURRENT_DAT_VERSION)
	{
		delete [] pLevel;
		return false;
	}

	//we need to determine what part of the file we are going to copy
	uint32 nCopyUpTo = nObjectMarker;
	uint32 nCopyFrom = nLightGridMarker;

	//determine the old object size
	uint32 nOldObjectSize = nBlindDataMarker - nObjectMarker;

	//determine the new object size
	uint32 nNewObjectSize = GetObjectListSize(pMainWorld->m_Objects);

	//determine the old blind object data size
	uint32 nOldBlindDataSize = nLightGridMarker - nBlindDataMarker;

	//determine the new blind data size
	uint32 nNewBlindDataSize = GetBlindObjectDataSize(pMainWorld);

	//update the markers past the object marker
	int32 nMarkerOffset = (int32)(nNewObjectSize + nNewBlindDataSize) - (int32)(nOldObjectSize + nOldBlindDataSize);

	nBlindDataMarker	+= (int32)(nNewObjectSize) - (int32)(nOldObjectSize);
	nLightGridMarker	+= nMarkerOffset;
	nPhysicsMarker		+= nMarkerOffset;
	nParticleDataMarker	+= nMarkerOffset;
	nRenderDataMarker	+= nMarkerOffset;

	//now update the memory to reflect the new offsets
	WriteMemory(&pLevel[8], nBlindDataMarker);
	WriteMemory(&pLevel[12], nLightGridMarker);
	WriteMemory(&pLevel[16], nPhysicsMarker);
	WriteMemory(&pLevel[20], nParticleDataMarker);
	WriteMemory(&pLevel[24], nRenderDataMarker);

	//alright, now what we need to do is save out the first part of the file, and then the last part
	CMoFileIO OutFile;

	// Save out the data.
	while(1)
	{
		if(OutFile.Open(pszOutFile, "wb"))
		{
			//save out the first part of the file
			OutFile.Write(pLevel, nCopyUpTo);

			SaveObjectList(OutFile, pMainWorld->m_Objects);
			SaveBlindObjectData(pMainWorld, OutFile);

			//now save out the last part of the file
			OutFile.Write(&pLevel[nCopyFrom], nFileSize - nCopyFrom);
			break;
		}
		else
		{
			//unable to open up the output stream, let the user do the old
			//retry or cancel
			int mbStatus = AskQuestion("Error opening the DAT file for output.  Try again?", QUES_YES | QUES_NO | QUES_CANCEL);
			
			if(mbStatus == QUES_CANCEL)
			{
				delete [] pLevel;
				return false;
			}
			else if(mbStatus == QUES_NO)
			{
				//determine if the DAT file exists and is read only
				CFileStatus Status;
				if(CFile::GetStatus(pszOutFile, Status))
				{
					if(Status.m_attribute & CFile::readOnly)
					{
						//this is a read only file, prompt the user and see if they would like to
						//make it writable
						if(AskQuestion("Would you like to make the DAT file writable?", QUES_YES|QUES_NO) == QUES_YES)
						{
							//we need to make this writable
							Status.m_attribute &= ~CFile::readOnly;
							CFile::SetStatus(pszOutFile, Status);
							continue;
						}			
					}
				}
				break;
			}
		}
	}

	delete [] pLevel;
	return true;
}

//-------------------------------------------------------------------
// CPCWorldPacker
//-------------------------------------------------------------------
CPCWorldPacker::CPCWorldPacker()
{
}

CPCWorldPacker::~CPCWorldPacker()
{
}


//handles the saving of the file to disk in the appropriate format.
bool CPCWorldPacker::PackWorld(	const char* pszFilename, CPreMainWorld* pMainWorld, bool bObjectsOnly)
{
	DrawStatusText(eST_Normal, "  ");

	//first off, we need to make our own filename
	char pszOutput[MAX_PATH + 1];

	//get the root filename
	LTStrCpy(pszOutput, pszFilename, sizeof(pszOutput));

	//append the extension onto the end
	LTStrCat(pszOutput, ".dat", sizeof(pszOutput));

	//now we need to open up the AbstractIO stream for writing out the actual file
	CMoFileIO outFile;

	//see if this is an object only process. If it is, do an objects only save
	if(bObjectsOnly)
	{
		DoObjectsOnlySave(pszOutput, pMainWorld);
	}
	else
	{
		// Save out the data.
		while(1)
		{
			if(outFile.Open(pszOutput, "wb"))
			{
				bool bRet = SaveFile(pMainWorld, outFile, pszFilename, bObjectsOnly);
				if(!bRet)
				{
					DrawStatusText(eST_Error, "CPreWorld::SaveFile failed!");
				}

				return bRet;
			}
			else
			{
				//unable to open up the output stream, let the user do the old
				//retry or cancel
				int mbStatus = AskQuestion("Error opening the DAT file for output.  Try again?", QUES_YES | QUES_NO | QUES_CANCEL);
				
				if(mbStatus == QUES_CANCEL)
				{
					return false;
				}
				else if(mbStatus == QUES_NO)
				{
					//determine if the DAT file exists and is read only
					CFileStatus Status;
					if(CFile::GetStatus(pszOutput, Status))
					{
						if(Status.m_attribute & CFile::readOnly)
						{
							//this is a read only file, prompt the user and see if they would like to
							//make it writable
							if(AskQuestion("Would you like to make the DAT file writable?", QUES_YES|QUES_NO) == QUES_YES)
							{
								//we need to make this writable
								Status.m_attribute &= ~CFile::readOnly;
								CFile::SetStatus(pszOutput, Status);
								continue;
							}			
						}
					}
					break;
				}
			}
		}
	}

	return true;
}

