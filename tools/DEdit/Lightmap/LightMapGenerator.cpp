#include "bdefs.h"
#include "LightMapGenerator.h"
#include "LightMapHolder.h"
#include "LitVertexHolder.h"


CLightMapGenerator::CLightMapGenerator() :
	m_nCurrVertex(0),
	m_pCurrLightMap(NULL),
	m_nCurrX(0),
	m_nCurrY(0)
{
}


CLightMapGenerator::~CLightMapGenerator()
{
}


//registers a holder as dirty, this will treat this polygon as freshly dirtied
//and if it is being processed, that processing will be cancelled.
bool CLightMapGenerator::DirtyLightMapHolder(ILightMapHolder* pHolder, bool bSearchForDuplicates)
{
	//sanity check
	ASSERT(pHolder);

	if(m_LightMapList.IsActiveHolder(pHolder))
	{
		ResetActiveLightMap();
	}

	//add this to the lightmap list
	return m_LightMapList.AddHolder(pHolder, bSearchForDuplicates);
}

//registers an object that is vertex lit as being dirtied. Duplicates will not
//be added and if it is being processed, it will be reset
bool CLightMapGenerator::DirtyVertexHolder(ILitVertexHolder* pHolder)
{
	ASSERT(pHolder);

	if(m_VertexList.IsActiveHolder(pHolder))
	{
		ResetActiveVertex();
	}

	//add this to the vertex list
	return m_VertexList.AddHolder(pHolder, false);
}

//sorts the holder list based upon a camera position and orientation
bool CLightMapGenerator::SortDirtyList(const LTVector& vCameraPos, const LTVector& vCameraDir)
{
	//sort both the holder lists
	bool bRV = m_VertexList.Sort(vCameraPos, vCameraDir);
	bRV	&= m_LightMapList.Sort(vCameraPos, vCameraDir);

	return bRV;
}

//called to caluclate lightmaps for the specified amount of time (in ms)
//This will proceed to calculate lightmaps for that period of time. Returns
//the number of lightmaps finished
uint32 CLightMapGenerator::CalculateLightmaps(uint32 nNumMS)
{
	//calculate the ending time
	clock_t nEndTime = clock() + nNumMS;

	uint32 nNumFinished = 0;

	//first calculate the vertices
	while(clock() < nEndTime)
	{
		ILitVertexHolder* pHolder = (ILitVertexHolder*)m_VertexList.GetActiveHolder();

		//can't process what we don't have...
		if(pHolder == NULL)
			break;

		//see if we are starting on a new holder
		if(m_nCurrVertex == 0)
		{
			SetupNewVertex();
		}

		//see if we are done with this lightmap
		if(m_nCurrVertex >= pHolder->GetNumVertices())
		{
			//done with this holder, finish it up...
			FinishActiveVertex();
			nNumFinished++;
		}
		else
		{
			//we need to generate some vertex texels!
			CalculateActiveVertex();
		}
	}

	//now we loop until we are out of time calcualting lightmaps
	while(clock() < nEndTime)
	{
		//first off, see if we are starting a new lightmap
		if(m_pCurrLightMap == NULL)
		{
			//see if we have an active lightmap
			if(m_LightMapList.GetActiveHolder() == NULL)
			{
				//no lightmap to process, just break out of the loop
				break;
			}

			//setup our new lightmap
			if(!SetupNewActiveLightMap())
			{
				//failed to succeed in that. Remove the active lightmap
				m_LightMapList.RemoveHolder(m_LightMapList.GetActiveHolder());
				continue;
			}
		}

		//sanity check
		ASSERT(m_pCurrLightMap);

		//see if we are done with this lightmap
		if(m_nCurrY >= m_pCurrLightMap->GetHeight())
		{
			//done with this lightmap, finish it up...
			FinishActiveLightMap();
			nNumFinished++;
		}
		else
		{
			//we need to generate some lightmap texels!
			CalculateActiveTexel();
		}
	}

	return nNumFinished;
}

//finishes the currently active lightmap
void CLightMapGenerator::FinishActiveLightMap()
{
	//ok, now we need to pass this lightmap onto the holder and clear everything out
	//for the next pass
	//get the holder we will be working with
	ILightMapHolder* pHolder = (ILightMapHolder*)m_LightMapList.GetActiveHolder();
	ASSERT(pHolder > 0);

	pHolder->SetLightMap(	m_vCurrO, 
							m_vCurrXInc * m_pCurrLightMap->GetWidth(),
							m_vCurrYInc * m_pCurrLightMap->GetHeight(),
							m_pCurrLightMap);
	
	//okay, we are done with that
	m_pCurrLightMap = NULL;
	m_nCurrX = 0;
	m_nCurrY = 0;

	//now remove the first lightmap
	RemoveLightMapHolder(pHolder);
}

//calculates the next active lightmap texel
void CLightMapGenerator::CalculateActiveTexel()
{
	//find the sample in world space

	//calculate the position
	LTVector vPos = m_vCurrO + m_vCurrXInc * m_nCurrX + m_vCurrYInc * m_nCurrY;

	//calculate the index and the image
	uint32 nIndex = (m_nCurrY * m_pCurrLightMap->GetWidth() + m_nCurrX) * 3;
	uint8* pImage = m_pCurrLightMap->GetImage();

	//calculate the sample
	m_SampleGen.CalcSample(	vPos, m_vCurrNormal, m_LightOptions,
							pImage[nIndex + 0], pImage[nIndex + 1], pImage[nIndex + 2]);

	//update the positions
	m_nCurrX++;

	if(m_nCurrX >= m_pCurrLightMap->GetWidth())
	{
		m_nCurrX = 0;
		m_nCurrY++;
	}
}

//sets up the active lightmap data to correspond to the information needed to build
//the lightmap information for the first holder in the list
bool CLightMapGenerator::SetupNewActiveLightMap()
{
	//get the holder we will be working with
	ILightMapHolder* pHolder = (ILightMapHolder*)m_LightMapList.GetActiveHolder();

	//new lightmap. If we don't have any to process though, just bail
	if(pHolder == NULL)
	{
		//none to generate now
		return false;
	}

	//get the extents of this holder
	LTVector vO, vX, vY;
	pHolder->GetExtents(vO, vX, vY);

	//now create the lightmap
	m_pCurrLightMap = m_Allocator.AllocateLightMap(vX.Mag(), vY.Mag());

	//make sure it worked
	if(m_pCurrLightMap == NULL)
		return false;

	//now we need to calculate our increments we will use
	
	//first find the origin
	m_vCurrO	= vO;

	//now calculate the increments
	m_vCurrXInc = vX / (m_pCurrLightMap->GetWidth() - 1.0f);
	m_vCurrYInc = vY / (m_pCurrLightMap->GetHeight() - 1.0f);

	//but that needs to be shifted back and up 1/2 texel due to filtering
	m_vCurrO -= (m_vCurrXInc / 2);
	m_vCurrO -= (m_vCurrYInc / 2);


	//just to be safe
	m_nCurrX = 0;
	m_nCurrY = 0;

	//get the normal
	m_vCurrNormal = pHolder->GetNormal();

	//need to get the options from the holder
	pHolder->GetLightOptions(m_LightOptions);

	//success
	return true;
}


//called when a holder is being removed. It then needs to be pulled out of the
//list of holders to be processed
bool CLightMapGenerator::RemoveLightMapHolder(ILightMapHolder* pHolder)
{
	if(m_LightMapList.IsActiveHolder(pHolder))
		ResetActiveLightMap();

	//now remove it
	return m_LightMapList.RemoveHolder(pHolder);
}

//called when a holder is being removed. It then needs to be pulled out of the
//list of holders to be processed
bool CLightMapGenerator::RemoveVertexHolder(ILitVertexHolder* pHolder)
{
	//see if this is our active holder
	if(m_VertexList.IsActiveHolder(pHolder))
		ResetActiveVertex();

	//now remove it
	return m_VertexList.RemoveHolder(pHolder);
}


//this is called to clear the entire list of holders
void CLightMapGenerator::ClearHolderList()
{
	m_VertexList.Free();
	m_LightMapList.Free();

	//reset our current processing
	ResetActiveLightMap();
	ResetActiveVertex();
}

//resets the lightmap that is currently being generated
void CLightMapGenerator::ResetActiveLightMap()
{
	//the lightmap currently being processed
	delete m_pCurrLightMap;
	m_pCurrLightMap = NULL;
	
	m_nCurrX = 0;
	m_nCurrY = 0;
}

//sets up for a new vertex light holder
void CLightMapGenerator::SetupNewVertex()
{
	//need to get the options from the holder
	m_VertexList.GetActiveHolder()->GetLightOptions(m_LightOptions);
}


//resets the vertex data that is currently being generated
void CLightMapGenerator::ResetActiveVertex()
{
	m_nCurrVertex = 0;
}

//finishes the currently active vertex holder
void CLightMapGenerator::FinishActiveVertex()
{
	m_nCurrVertex = 0;
	
	ILitVertexHolder* pHolder = (ILitVertexHolder*)m_VertexList.GetActiveHolder();

	//we want to remove this holder from the vertex holder list
	pHolder->FinishedLighting();
	m_VertexList.RemoveHolder(pHolder);
}

//calculates the next active vertex color
void CLightMapGenerator::CalculateActiveVertex()
{
	//get the active holder
	ILitVertexHolder* pHolder = (ILitVertexHolder*)m_VertexList.GetActiveHolder();

	//ensure we have a valid holder
	if(pHolder == NULL)
		return;

	//get the position and normal of the vertex we are calculating
	LTVector vPos;
	LTVector vNormal;

	if(!pHolder->GetVertex(m_nCurrVertex, vPos, vNormal))
		return;

	//now calculate the sample
	uint8 nR, nG, nB;
	m_SampleGen.CalcSample(vPos, vNormal, m_LightOptions, nR, nG, nB);

	//set the color
	pHolder->SetVertexLightColor(m_nCurrVertex, nR, nG, nB);

	//move onto the next vertex
	m_nCurrVertex++;
}

//determines if there are any holders left to be processed
bool CLightMapGenerator::IsHolderListEmpty() const
{
	return	(m_LightMapList.GetNumHolders() == 0) &&
			(m_VertexList.GetNumHolders() == 0);
}


