
#include "bdefs.h"

#include "light_table.h"
#include "ltsysoptim.h"

// MACROS...
#define LTRGB_TO_VECTOR(dest, src)							\
    (dest).x = ((src)[0]);									\
    (dest).y = ((src)[1]);									\
    (dest).z = ((src)[2]);

// CLightTable...
CLightTable::CLightTable()
{
	Reset();
}

CLightTable::~CLightTable()
{
	FreeAll();
}

void CLightTable::Reset()
{
	m_pLightData				= NULL;
	m_DataDims					= TVector3<int32>(0,0,0);
	m_vWorldBasePos				= LTVector(0.0f,0.0f,0.0f);
	m_vWorldToLightDataScale	= LTVector(0.0f,0.0f,0.0f);
}

void CLightTable::FreeAll()
{
	delete[] m_pLightData;
	m_pLightData = NULL;

	ClearLightGroups();

	Reset();
}

// Decompresses the data in pCompressed, storing it in pOut...
bool CLightTable::Load_RLE_DeCompress(ILTStream* pStream, uint8* pOutData, uint32 iUncompSize)
{
	// Index in the output buffer...
	uint8* pCurrOut = pOutData;
	// Run through the input buffer...
	while (pCurrOut < pOutData + iUncompSize)
	{
		// Read in the tag...
		uint8 nTag; pStream->Read(&nTag,sizeof(uint8));
		// Is a run or a span....
		bool bIsRun = (nTag & 0x80) ? true : false;
		uint32 nRunLen = (uint32)(nTag & 0x7F) + 1;
		if (bIsRun)
		{
			uint8 pRunData[3];
			// Read the color...
			pStream->Read(pRunData,sizeof(uint8)*3);
			for (uint32 nCurrPel = 0; nCurrPel < nRunLen; ++nCurrPel)
			{
				pCurrOut[0] = pRunData[0];
				pCurrOut[1] = pRunData[1];
				pCurrOut[2] = pRunData[2];
				// Update the output position...
				pCurrOut += 3;
			}
		}
		else
		{
			pStream->Read(pCurrOut,sizeof(uint8) * 3 * nRunLen);
			pCurrOut += 3 * nRunLen;
		}
	}

	// Double check..
	assert(pCurrOut == pOutData + iUncompSize);
	return true;
}

extern int32 g_CV_CompressLightGrid;
bool CLightTable::Load(ILTStream* pStream)
{
	*pStream >> m_vWorldBasePos;
	LTVector vGridSize;
	*pStream >> vGridSize;
	pStream->Read(&m_DataDims,sizeof(m_DataDims));
	m_vWorldToLightDataScale.x = 1.0f / vGridSize.x;
	m_vWorldToLightDataScale.y = 1.0f / vGridSize.y;
	m_vWorldToLightDataScale.z = 1.0f / vGridSize.z;

	// Read in the size of the compressed data...
	uint32 iSizeCompressed;
	*pStream >> iSizeCompressed;
	uint32 iSize = m_DataDims.x * m_DataDims.y * m_DataDims.z * 3;

	// Read in the data and store decompressed...
	LT_MEM_TRACK_ALLOC(m_pLightData = new uint8[iSize],LT_MEM_TYPE_WORLD);
	if (!m_pLightData)
		return false;
	if (!Load_RLE_DeCompress(pStream,m_pLightData,iSize))
		return false;

	return true;
}

void CLightTable::GetLightVal(const LTVector& vWorldPos,bool bFilter,LTRGB* pRGB) const
{
	TVector3<int32> gridCoords;
	LTVector finalColor;
    LTVector samples[8], ySamples[2], xySamples[2];

    // Figure out which grid point we lie on.
    LTVector fSamplePt = (vWorldPos - m_vWorldBasePos) * m_vWorldToLightDataScale;
    gridCoords.x = LTCLAMP(ltfptosi(fSamplePt.x), -1, m_DataDims.x-1);
    gridCoords.y = LTCLAMP(ltfptosi(fSamplePt.y), -1, m_DataDims.y-1);
    gridCoords.z = LTCLAMP(ltfptosi(fSamplePt.z), -1, m_DataDims.z-1);
	TVector3<int32> gridOfs(1,1,1);
	if ((gridCoords.x == (m_DataDims.x - 1)) || (gridCoords.x < 0))
		gridOfs.x = 0;
	if ((gridCoords.y == (m_DataDims.y - 1)) || (gridCoords.y < 0))
		gridOfs.y = 0;
	if ((gridCoords.z == (m_DataDims.z - 1)) || (gridCoords.z < 0))
		gridOfs.z = 0;
	gridCoords.x = LTMAX(gridCoords.x, 0);
	gridCoords.y = LTMAX(gridCoords.y, 0);
	gridCoords.z = LTMAX(gridCoords.z, 0);

    // Get 0-1 for the sample.
    fSamplePt.x   = fSamplePt.x - ltfloorf(fSamplePt.x);
    fSamplePt.y   = fSamplePt.y - ltfloorf(fSamplePt.y);
    fSamplePt.z   = fSamplePt.z - ltfloorf(fSamplePt.z);

    // Get the 8 box points and bilinear interpolate.
	if (m_pLightData)
	{
		// UnCompressed data...
		const uint8* pBase = &m_pLightData[gridCoords.z*m_DataDims.x*m_DataDims.y*3 + gridCoords.y*m_DataDims.x*3 + gridCoords.x*3];
		uint32 nLineOfs = gridOfs.y * m_DataDims.x * 3;
		uint32 nXOfs = gridOfs.x * 3;
		LTRGB_TO_VECTOR(samples[0], &pBase[nLineOfs]);
		LTRGB_TO_VECTOR(samples[1], &pBase[nLineOfs + nXOfs]);
		LTRGB_TO_VECTOR(samples[2], &pBase[0]);
		LTRGB_TO_VECTOR(samples[3], &pBase[nXOfs]);

		pBase += gridOfs.z*m_DataDims.x*m_DataDims.y*3;
		LTRGB_TO_VECTOR(samples[4], &pBase[nLineOfs]);
		LTRGB_TO_VECTOR(samples[5], &pBase[nLineOfs + nXOfs]);
		LTRGB_TO_VECTOR(samples[6], &pBase[0]);
		LTRGB_TO_VECTOR(samples[7], &pBase[nXOfs]);
	}
	else
	{
		pRGB->r = 0x80;
		pRGB->g = 0x80;
		pRGB->b = 0x80;
		return;
	}

	AddLightGroupSamples(samples, gridCoords);

	VEC_LERP(ySamples[0], samples[0], samples[2], fSamplePt.y);
    VEC_LERP(ySamples[1], samples[1], samples[3], fSamplePt.y);
    VEC_LERP(xySamples[0], ySamples[0], ySamples[1], fSamplePt.x);
    VEC_LERP(ySamples[0], samples[4], samples[6], fSamplePt.y);
    VEC_LERP(ySamples[1], samples[5], samples[7], fSamplePt.y);
    VEC_LERP(xySamples[1], ySamples[0], ySamples[1], fSamplePt.x);
    VEC_LERP(finalColor, xySamples[0], xySamples[1], fSamplePt.z);

    pRGB->r = (uint8)ltfptoui(LTCLAMP(finalColor.x, 0, 255.0f));
    pRGB->g = (uint8)ltfptoui(LTCLAMP(finalColor.y, 0, 255.0f));
    pRGB->b = (uint8)ltfptoui(LTCLAMP(finalColor.z, 0, 255.0f));
}

void CLightTable::AddLightGroupSamples(LTVector aSamples[], const TVector3<int32> &vGridCoords) const
{
	TVector3<int32> vExtents;

	int32 nX, nY, nZ;

	TLightGroupList::const_iterator iCurLG = m_aLightGroups.begin();
	for (; iCurLG != m_aLightGroups.end(); ++iCurLG)
	{
		const SLightGroup &cLG = *iCurLG;

		// If we hit a black light, we're at the end of the active lightgroups
		if (iCurLG->IsBlack())
			break;

		// Make sure our range overlaps with the lightgroup. This first tests the XZ axis
		//since most levels are laid out largely planar
		if ((iCurLG->GetMin().x > (vGridCoords.x + 1)) ||
			(iCurLG->GetMax().x <= vGridCoords.x) ||
			(iCurLG->GetMin().z > (vGridCoords.z + 1)) ||
			(iCurLG->GetMax().z <= vGridCoords.z) ||
			(iCurLG->GetMin().y > (vGridCoords.y + 1)) ||
			(iCurLG->GetMax().y <= vGridCoords.y))
			continue;

		//offset the sample
		vExtents		= iCurLG->GetMax() - iCurLG->GetMin() - TVector3<int32>(1, 1, 1);

		nX = vGridCoords.x - iCurLG->GetMin().x;
		nY = vGridCoords.y - iCurLG->GetMin().y;
		nZ = vGridCoords.z - iCurLG->GetMin().z;

		// Add in the samples
		if((nX >= 0) && (nY < vExtents.y) && (nZ >= 0))
			aSamples[0] += iCurLG->GetSample(nX, nY + 1, nZ);

		if((nX < vExtents.x) && (nY < vExtents.y) && (nZ >= 0))
			aSamples[1] += iCurLG->GetSample(nX + 1, nY + 1, nZ);

		if((nX >= 0) && (nY >= 0) && (nZ >= 0))
			aSamples[2] += iCurLG->GetSample(nX, nY, nZ);

		if((nX < vExtents.x) && (nY >= 0) && (nZ >= 0))
			aSamples[3] += iCurLG->GetSample(nX + 1, nY, nZ);

		if((nX >= 0) && (nY < vExtents.y) && (nZ < vExtents.z))
			aSamples[4] += iCurLG->GetSample(nX, nY + 1, nZ + 1);

		if((nX < vExtents.x) && (nY < vExtents.y) && (nZ < vExtents.z))
			aSamples[5] += iCurLG->GetSample(nX + 1, nY + 1, nZ + 1);

		if((nX >= 0) && (nY >= 0) && (nZ < vExtents.z))
			aSamples[6] += iCurLG->GetSample(nX, nY, nZ + 1);

		if((nX < vExtents.x) && (nY >= 0) && (nZ < vExtents.z))
			aSamples[7] += iCurLG->GetSample(nX + 1, nY, nZ + 1);
	}
}

bool CLightTable::LoadLightGroup(ILTStream *pStream, uint32 nID, const LTVector &vColor)
{
	// Add the lightgroup to the list
	bool bBlack = (vColor.x < (1.0f / 255.0f)) || (vColor.y < (1.0f / 255.0f)) || (vColor.z < (1.0f / 255.0f));
	SLightGroup lg;
	if (bBlack)
	{
		m_aLightGroups.push_back(lg);
	}
	else
	{
		m_aLightGroups.push_front(lg);
	}
	SLightGroup &cLG = bBlack ? m_aLightGroups.back() : m_aLightGroups.front();

	// Set up the stuff we got from our caller
	cLG.m_nID = nID;
	cLG.m_vColor = vColor;

	//load in the min and the extent
	TVector3<int32> vMin, vExtents;

	// Read the rest in from the file
	*pStream >> vMin;
	*pStream >> vExtents;

	// Setup the size of the light group
	cLG.SetupExtents(vMin, vExtents);

	//allocate the memory for the samples and read them in
	LT_MEM_TRACK_ALLOC(cLG.m_pSamples = new uint8[cLG.GetTotalSampleCount()],LT_MEM_TYPE_WORLD);

	if (pStream->Read(cLG.m_pSamples, cLG.GetTotalSampleCount()) != LT_OK)
		return false;

	return true;
}

void CLightTable::SetLightGroupColor(uint32 nID, const LTVector &vColor)
{
	// Find the light group
	TLightGroupList::iterator iCurLG = m_aLightGroups.begin();
	for (; iCurLG != m_aLightGroups.end(); ++iCurLG)
	{
		if (iCurLG->m_nID == nID)
			break;
	}

	if (iCurLG != m_aLightGroups.end())
	{
		// Update the color
		iCurLG->m_vColor = vColor;
		// Move it to the front/back of the list
		m_aLightGroups.splice(iCurLG->IsBlack() ? m_aLightGroups.end() : m_aLightGroups.begin(), m_aLightGroups, iCurLG);
	}
}
