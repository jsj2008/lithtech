//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
//------------------------------------------------------------------
//
//	FILE	  : LightMapMaker.cpp
//
//	PURPOSE	  : Implements the CLightMapMaker class.
//
//	CREATED	  : September 30 1996
//
//
//------------------------------------------------------------------

// Includes....
#include "bdefs.h"
#include "lightmapmaker.h"
#include "geomroutines.h"
#include "editpoly.h"
#include "editregion.h"
#include "editobjects.h"
#include "de_world.h"
#include "processing.h"
#include "gettextureinfo.h"
#include "pregeometry.h"
#include "conparse.h"
#include "parse_world_info.h"
#include "genericprop_setup.h"
#include "iobjectplugin.h"
#include "threads.h"
#include "lightmapdefs.h"
#include "prelightmap.h"
#include "noise.h"

//the name to use for the main base lightmap animation (every world has this one)
// This isn't used by the engine anymore, so it doesn't have much meaning anymore
#define BASE_LIGHTANIM_NAME "LightAnim_BASE"

//maximum number of iterations to run through while creating texture lights
//(max tiling that texture lights will be applied on)
#define TEXTURE_LIGHT_ITERATIONS	100

//this is the maximum number of lights that can affect a single
//lightmapped polygon (this should be plenty, since if each light
//only contributes 1 value, it will still max out at 255)
#define MAX_LIGHTS_PER_POLYGON		256

typedef CMoArray<PVector> PVectorArray;

// Lighting group base class name
#define LIGHTGROUP_CLASSNAME "Engine_LightGroup"
#define LIGHTGROUP_CLASSNAME_GAME "LightGroup"

// Lightgroup lightmap animation name
#define LIGHTGROUP_ANIMNAME "LG_ANIM"

// -------------------------------------------------------------------------------- //
// Helpers.
// -------------------------------------------------------------------------------- //

static inline bool ShouldLightmap(CPrePoly *pPoly)
{
	return (pPoly->GetSurfaceFlags() & SURF_LIGHTMAP) && 
		!(pPoly->GetSurfaceFlags() & SURF_INVISIBLE) &&
		!(pPoly->GetSurfaceFlags() & SURF_SKY);
}

static inline PVector CalcLightSample(
							const CLightDef *pLight,
							const PVector &lightPos,
							const PVector &lightDir,
							const PVector &samplePos, 
							float fSurfaceDot,
							PReal distSqr)
{
	float distPercent;

	//calculate the distance attenuation
	switch (pLight->m_eAttenuation)
	{
		case CLightDef::eAttenuation_D3D :
		{
			distPercent = 1.0f / (pLight->m_A + pLight->m_B * sqrtf(distSqr) + pLight->m_C * distSqr); 
		}
		break;
		case CLightDef::eAttenuation_Quartic : 
		{
			distPercent = 1.0f - (distSqr / pLight->m_MaxDistSqr);
			distPercent *= distPercent;
		}
		break;
		case CLightDef::eAttenuation_Linear : 
		{
			distPercent = 1.0f - (sqrtf(distSqr) / pLight->m_MaxDist);
		}
		break;
	}


	if(pLight->m_bDirectional)
	{
		PVector rayDir		= samplePos - lightPos;
		float dot			= rayDir.Dot(lightDir) / rayDir.Mag();		
		float fovDotLimit	= pLight->GetCosFOV();

		if(dot > fovDotLimit)
		{
			float anglePercent = (dot - fovDotLimit) / (1.0f - fovDotLimit);

			return pLight->m_InnerColor * (anglePercent * distPercent * fSurfaceDot);
		}
		else
		{
			return PVector(0.0f, 0.0f, 0.0f);
		}
	}
	else
	{
		return pLight->m_InnerColor * (distPercent * fSurfaceDot);
	}
}

static float CalcSampleSurfaceDot(const PVector &lightPos, const PVector &samplePos, const PVector &vNormal)
{
  	LTVector vSampleDir = lightPos - samplePos;
  	vSampleDir.Norm();
  	float fSurfaceDot = vSampleDir.Dot(vNormal);
  	return LTMAX(fSurfaceDot, 0.0f);
}

static inline PVector CalcLightSample(
							const CLightDef *pLight,
							const PVector &lightPos,
							const PVector &lightDir,
							const PVector &samplePos, 
							const PVector &surfNormal,
							PReal distSqr)
{
	float fSampleScale = CalcSampleSurfaceDot(lightPos, samplePos, surfNormal);
	return CalcLightSample(pLight, lightPos, lightDir, samplePos, fSampleScale, distSqr);
}

// -------------------------------------------------------------------------------- //
// LMMWorld.
// -------------------------------------------------------------------------------- //

LMMWorld::LMMWorld()
{
	m_bUsingTransform = false;
}

// -------------------------------------------------------------------------------- //
// CLightMapMaker.
// -------------------------------------------------------------------------------- //

CLightMapMaker::CLightMapMaker()
{
	m_pWorld		= NULL; 
	m_pCurAnim		= NULL;
	m_pCurFrame		= NULL;
	m_ThreadCS		= NULL;
	m_fSunRayLength = 0.0f;
	m_bApplyBrushAmbient = true;
}


CLightMapMaker::~CLightMapMaker()
{
	Term();
}


bool CLightMapMaker::Init(CPreMainWorld *pWorld,
                          bool bClipLight)
{
	LTVector vMin, vMax;
	ConParse cParse;
	uint32 i, nTotalPolies;
	CPreWorld *pModel;
	GPOS pos;


	Term();

	// Setup our critical section.
	m_ThreadCS = thd_CreateCriticalSection();
	if(!m_ThreadCS)
		return false;

	m_pWorld			= pWorld;
	m_bClipLight		= bClipLight;
	m_bOverSample		= g_pGlobs->m_bLMOverSample;

	// Get ambient light levels out of the world info string.
	ParseAmbientLight(pWorld->m_pInfoString, &m_AmbientLight);

	if(!m_LMMWorlds.SetSize(pWorld->m_WorldModels.GetSize()))
		return false;

	// Setup our polygon list and extents.
	vMin.Init((PReal)MAX_CREAL, (PReal)MAX_CREAL, (PReal)MAX_CREAL);
	vMax.Init((PReal)-MAX_CREAL, (PReal)-MAX_CREAL, (PReal)-MAX_CREAL);

	nTotalPolies = 0;
	for(i=0; i < pWorld->m_WorldModels; i++)
	{
		pModel = pWorld->m_WorldModels[i];
		pModel->UpdateBoundingBox();
		m_LMMWorlds[i].m_pWorld = pModel;
		
		VEC_MIN(vMin, vMin, pModel->m_PosMin);
		VEC_MAX(vMax, vMax, pModel->m_PosMax);

		// Set up the poly indices
		pModel->SetPolyIndices();
		pModel->SetPolyWorldIndices();

		// Set its poly pointers.
		for(pos=pModel->m_Polies; pos; )
		{
			pModel->m_Polies.GetNext(pos)->m_pWorld = &m_LMMWorlds[i];
		}

		nTotalPolies += pModel->m_Polies.GetSize();
	}

	m_fSunRayLength = (vMax - vMin).Mag() + 1.0f;


	// Setup our 'all polies' list.
	if(!m_Polies.SetSize(nTotalPolies))
	{
		return false;
	}

  	uint32 iOutPoly = 0;

  	for(i=0; i < pWorld->m_WorldModels; i++)
  	{
  		pModel = pWorld->m_WorldModels[i];
   
  		for(pos=pModel->m_Polies; pos; )
  		{
  			m_Polies[iOutPoly] = pModel->m_Polies.GetNext(pos);

			// Make sure the extents for this polygon have been calculated
			m_Polies[iOutPoly]->CalcExtents(1.0f);

  			iOutPoly++;
  		}
  	}
  
  	ASSERT(iOutPoly == m_Polies.GetSize());
	
	m_pCurLightGroup = 0;

	DrawStatusTextIfMainWorld(eST_Normal, "Ambient light (%.1f, %.1f, %.1f)", VEC_EXPAND(m_AmbientLight));	
	return true;
}


void CLightMapMaker::Term()
{
	DeleteAndClearArray(m_FullLightMaps);
	DeleteAndClearArray( m_LightDefs );

	m_LightingBSP.FreeBSP();

	if(m_ThreadCS)
	{
		thd_DeleteCriticalSection(m_ThreadCS);
		m_ThreadCS = NULL;
	}
}

//calculates the amount of shift that is necessary for the lightmap positions. This
//allows for super sampling of lightmaps by a pixel or two, yet still have the positions
//be correct
PReal CLightMapMaker::GetShiftAmount(const CPrePoly* pPoly) const
{
	if(m_bOverSample)
	{
		return -((m_pWorld->GetLMGridSize(pPoly)) / 2.0f);
	}

	//no oversampling, so no shift is needed
	return 0;
}


PVector CLightMapMaker::CalcSunLightSample(const PVector& dir) const
{
	PReal dot = -m_SunLightDir.Dot(dir) * m_SunLightBrightScale;	// -m_SunLightBrightScale to m_SunLightBrightScale
	if (dot < 0.0f)
	{
		dot = 0.0f;
	}
	
	return m_SunLightInner * dot;
}


void CLightMapMaker::InitPolyLightmapFill(CPrePoly *poly, CFullLightMap *lightmap) 
{
    //what color we will fill the lightmap with.
	PVector fillColor(m_AmbientLight);
	
	if(m_bApplyBrushAmbient)
	{
		fillColor += PVector(	poly->GetSurface()->m_Ambient[0],
								poly->GetSurface()->m_Ambient[1],
								poly->GetSurface()->m_Ambient[2]);
	}

	lightmap->ColorFill(fillColor);
}

void CLightMapMaker::InitPolyLightmapMinMax(CPrePoly *poly, CFullLightMap *lightmap) {
	lightmap->m_Min = lightmap->m_Max = poly->Pt(0);
	for(uint32 i=1; i < poly->NumVerts(); i++)
	{
		VEC_MIN(lightmap->m_Min, lightmap->m_Min, poly->Pt(i));
		VEC_MAX(lightmap->m_Max, lightmap->m_Max, poly->Pt(i));
	}
}


bool CLightMapMaker::SetupNewLightAnim(const char *pName)
{
	CPreLightAnim *pAnim;
	uint32 iWorld;
	CPrePoly *pPoly;
	CPrePolyRef newRef;
	CPreWorld *pModel;
	GPOS pos;


	// Setup the base light anim.
	pAnim = new CPreLightAnim;
	if(!pAnim)
		return false;
	
	if(!m_pWorld->m_LightAnims.Append(pAnim))
	{
		delete pAnim;
		return false;
	}

	SAFE_STRCPY(pAnim->m_Name, pName);
	pAnim->m_Polies.SetCacheSize(1024);

	// Add the polies that have lightmapping, which may be none if vertex
	//lighting is enabled
	if(!g_pGlobs->m_bVerticesOnly)
	{
		for(iWorld=0; iWorld < m_pWorld->m_WorldModels; iWorld++)
		{
			pModel = m_pWorld->m_WorldModels[iWorld];
			
			for(pos=pModel->m_Polies; pos; )
			{
				pPoly = pModel->m_Polies.GetNext(pos);

				if(ShouldLightmap(pPoly))
				{
					newRef.m_iWorld = pPoly->m_WorldIndex;
					newRef.m_iPoly = pPoly->m_Index;
					if(!pAnim->m_Polies.Append(newRef))
						return false;
				}
			}
		}
	}

	m_pCurAnim = pAnim;
	return true;
}


bool CLightMapMaker::AddAnimFrame()
{
	CPreLightFrame *pFrame;


	if(!m_pCurAnim)
		return false;

	pFrame = new CPreLightFrame;
	if(!pFrame)
		return false;

	if(!m_pCurAnim->m_Frames.Append(pFrame))
	{
		delete pFrame;
		return false;
	}

	m_pCurFrame = pFrame;
	return true;
}

// Build our internal poly intersection tree
bool CLightMapMaker::InitPolyTree()
{
	//sanity checks
	if(m_pWorld == NULL)
	{
		DrawStatusText(eST_Error, "World not initialized for InitPolyTree");
		return false;
	}

	DrawStatusText(eST_Normal, "  Setting up world for lighting");
	clock_t nStartTime = clock();

	// Clear the poly trees
	m_cPolyTree_World.Clear();

	// Go count the polys that we're going to be using
	uint32 nNumSkyPolys = 0;
	uint32 nNumWorldPolys = 0;

	uint32 nCurrWorld; // Note : VC is evil
	for(nCurrWorld = 0; nCurrWorld < m_pWorld->m_WorldModels.GetSize(); nCurrWorld++)
	{

		CPreWorld* pWorld = m_pWorld->m_WorldModels[nCurrWorld];

		if(!(pWorld->m_WorldInfoFlags & WIF_PHYSICSBSP))
		{
			//determine if this world model should be factored into the BSP
			if(!ShouldBlockLight(pWorld))
			{
				continue;
			}
		}

		GPOS PolyPos;
		for(PolyPos = pWorld->m_Polies; PolyPos; )
		{
			CPrePoly* pPoly = pWorld->m_Polies.GetNext(PolyPos);

			//see if this is a polygon we want
			if (!(pPoly->GetSurfaceFlags() & SURF_SKY))
				++nNumWorldPolys;
		}
	}

	// Reserve space in the poly trees
	m_cPolyTree_World.ReservePolys(nNumWorldPolys);

	// run through all the world models
	for(nCurrWorld = 0; nCurrWorld < m_pWorld->m_WorldModels.GetSize(); nCurrWorld++)
	{
		CPreWorld* pWorld = m_pWorld->m_WorldModels[nCurrWorld];

		if(!(pWorld->m_WorldInfoFlags & WIF_PHYSICSBSP))
		{
			//determine if this world model should be factored into the BSP
			if(!ShouldBlockLight(pWorld))
			{
				continue;
			}
		}

		GPOS PolyPos;
		for(PolyPos = pWorld->m_Polies; PolyPos; )
		{
			CPrePoly* pPoly = pWorld->m_Polies.GetNext(PolyPos);

			//see if this is a polygon we want
			if((pPoly->GetSurfaceFlags() & SURF_CLIPLIGHT) == 0)
				continue;

			CPrePoly* pNewPoly = pPoly->Clone();

			//check for out of memory conditions
			if(pNewPoly == NULL)
			{
				DrawStatusText(eST_Error, "Could not clone polygon for building lightmapping data");
				return false;
			}

			//update the vectors for the new polygon
			for(uint32 nCurrVert = 0; nCurrVert < pNewPoly->NumVerts(); nCurrVert++)
			{
				pNewPoly->Pt(nCurrVert) = pWorld->PolyPt(pPoly, nCurrVert);
			}

			//this is a shadow casting light. Transform if we need to
			if(pPoly->m_pWorld->m_bUsingTransform)
			{
				//transform all of the vertices of the new polygon
				for(uint32 nCurrVert = 0; nCurrVert < pNewPoly->NumVerts(); nCurrVert++)
				{
					pNewPoly->Pt(nCurrVert) = pPoly->m_pWorld->m_mTransform * pNewPoly->Pt(nCurrVert);
				}					
			}

			// Add this polygon to the correct list
			if (!(pPoly->GetSurfaceFlags() & SURF_SKY))
				m_cPolyTree_World.AddPoly(pNewPoly, pPoly);

			// Delete the poly
			delete pNewPoly;
		}
	}

	ASSERT(m_cPolyTree_World.GetPolyCount() == nNumWorldPolys);

	m_cPolyTree_World.BuildNodes();

	clock_t nEndTime = clock();

	// Display statistics about the lighting data
	DrawStatusText(eST_Normal, "  Lighting data built in %.2f seconds", (nEndTime - nStartTime) / (float)CLOCKS_PER_SEC);
	DrawStatusText(eST_Normal, "  Lighting polys : %d", m_cPolyTree_World.GetPolyCount());
	DrawStatusText(eST_Normal, "  Lighting nodes : %d", m_cPolyTree_World.GetNodeCount());

	return true;
}

//used to build up our internal BSP used for segment intersections
bool CLightMapMaker::InitBSP()
{
	//sanity checks
	if(m_pWorld == NULL)
	{
		DrawStatusText(eST_Error, "World not initialized for InitBSP");
		return false;
	}

	//we need to build up a list of every polygon that blocks light
	PrePolyArray	PolyList;
	PolyList.SetCacheSize(2048);

	//run through all the world models
	for(uint32 nCurrWorld = 0; nCurrWorld < m_pWorld->m_WorldModels.GetSize(); nCurrWorld++)
	{
		CPreWorld* pWorld = m_pWorld->m_WorldModels[nCurrWorld];

		//we want to have the physics and select world models block light
		if(pWorld->m_WorldInfoFlags & WIF_VISBSP)
		{
			continue;
		}

		if(!(pWorld->m_WorldInfoFlags & WIF_PHYSICSBSP))
		{
			//determine if this world model should be factored into the BSP
			if(!ShouldBlockLight(pWorld))
			{
				continue;
			}
		}

		GPOS PolyPos;
		for(PolyPos = pWorld->m_Polies; PolyPos; )
		{
			CPrePoly* pPoly = pWorld->m_Polies.GetNext(PolyPos);

			//see if this is a polygon we want
			if(pPoly->GetSurfaceFlags() & SURF_CLIPLIGHT)
			{
				CPrePoly* pNewPoly = pPoly->Clone();

				//check for out of memory conditions
				if(pNewPoly == NULL)
				{
					DeleteAndClearArray(PolyList);
					DrawStatusText(eST_Error, "Not enough memory to allocate poly list for BSP");
					return false;
				}

				//update the vectors for the new polygon
				for(uint32 nCurrVert = 0; nCurrVert < pNewPoly->NumVerts(); nCurrVert++)
				{
					pNewPoly->Pt(nCurrVert) = pWorld->PolyPt(pPoly, nCurrVert);
				}

				//this is a shadow casting light. Transform if we need to
				if(pPoly->m_pWorld->m_bUsingTransform)
				{
					//transform all of the vertices of the new polygon
					for(uint32 nCurrVert = 0; nCurrVert < pNewPoly->NumVerts(); nCurrVert++)
					{
						pNewPoly->Pt(nCurrVert) = pPoly->m_pWorld->m_mTransform * pNewPoly->Pt(nCurrVert);
					}					
				}

				//add this polygon to the list
				PolyList.Add(pNewPoly);
			}			
		}
	}

	//now we need to create our BSP
	bool bSuccess = m_LightingBSP.BuildBSP(PolyList);

	if(!bSuccess)
	{
		DrawStatusText(eST_Error, "An error occurred while building the lighting BSP. This is most likely due to low memory");
	}
	else
	{

		//display statistics about the lighting BSP
		DrawStatusText(eST_Normal, "Number of polies in lighting: %d", PolyList.GetSize());
		DrawStatusText(eST_Normal, "Lighting BSP nodes: %d", m_LightingBSP.GetNumNodes());
		DrawStatusText(eST_Normal,	"Lighting BSP first null node: %d", m_LightingBSP.GetDepthOfFirstNullChild());
		DrawStatusText(eST_Normal,	"Lighting BSP longest chain: %d", m_LightingBSP.GetLongestChainLen());
		DrawStatusText(eST_Normal,	"Lighting BSP Full Nodes: %d", m_LightingBSP.GetTwoChildrenCount());
		
		//other stats that aren't quite as useful

		//DrawStatusText(eST_Normal, "Lighting BSP depth: %d", m_LightingBSP.GetTreeDepth());
		//DrawStatusText(eST_Normal, "Lighting BSP min depth: %d", m_LightingBSP.GetMinTreeDepth());
		//DrawStatusText(eST_Normal, "Lighting BSP Num Leaves: %d", m_LightingBSP.GetLeafCount());
		//DrawStatusText(eST_Normal, "Lighting BSP Num Single: %d", m_LightingBSP.GetSingleChildrenCount());
		
	}
	

	//clean up and return
	DeleteAndClearArray(PolyList);

	return bSuccess;
}

//generates a lighting grid based upon the table that is passed in. The Callee
//is responsible for freeing the associated memory
bool CLightMapMaker::CreateLightGrid(const LightTable &Dims, PVector** pTable, bool bAmbient)
{
	ASSERT(pTable);

	//allocate it
	*pTable = new PVector[Dims.m_FullLookupSize];

	//see if it worked
	if(*pTable == NULL)
	{
		//failure
		DrawStatusText(eST_Error, "Unable to allocate %dk for light grid", (Dims.m_FullLookupSize * sizeof(PVector)) / 1024);
		Term();
		return false;
	}

	//create the stack for the ray casts
	CLightBSPStack *pStack = m_LightingBSP.CreateStack();

	if(!pStack)
	{
		//failure
		DrawStatusText(eST_Error, "Unable to allocate memory for the BSP stack");
		Term();
		return false;
	}

	//calculate light values for all the points
	
	//current index into the array
	uint32 nIndex = 0;

	PReal fXSize = (PReal)Dims.m_BlockSize.x;
	PReal fYSize = (PReal)Dims.m_BlockSize.y;
	PReal fZSize = (PReal)Dims.m_BlockSize.z;

	uint32 nTotalPoints = Dims.m_LookupSize[0] * Dims.m_LookupSize[1] * Dims.m_LookupSize[2];

	for(uint32 nZ = 0; nZ < Dims.m_LookupSize[2]; nZ++)
	{
		for(uint32 nY = 0; nY < Dims.m_LookupSize[1]; nY++)
		{
			for(uint32 nX = 0; nX < Dims.m_LookupSize[0]; nX++, nIndex++)
			{
				(*pTable)[nIndex] = LightGridPoint(	PVector(nX * fXSize, nY * fYSize, nZ * fZSize) + 
													Dims.m_LookupStart, m_bClipLight, bAmbient, pStack);
			}

			// Tell the user about our status
			SetProgressBar((float)(nIndex + 1) / (float)nTotalPoints);
		}
	}

	//clear out our callback now
	SetProgressBar(0.0f);

	//free our stack
	m_LightingBSP.FreeStack(pStack);

	//success
	return true;
}


bool CLightMapMaker::LightLightingPoints( const std::vector<CLightingPoint>& lightingPoints )
{
	//create the stack for the ray casts
	CLightBSPStack *pStack = m_LightingBSP.CreateStack();

	if(!pStack)
	{
		//failure
		DrawStatusText(eST_Error, "Unable to allocate memory for the BSP stack");
		Term();
		return false;
	}


	std::vector<CLightingPoint>::const_iterator it = lightingPoints.begin();

	uint32 nNumPointsToLight = lightingPoints.size();
	uint32 nNumPointsLit = 0;

	for( ; it != lightingPoints.end(); it++ )
	{
		LTVector pos = (*it).pos;
		LTVector normal = (*it).normal;
		bool useNormal = !!((*it).lightingFlags & CLightingPoint::LIGHT_USENORMAL);
		bool useOnlyAmbient = !!((*it).lightingFlags & CLightingPoint::LIGHT_ONLYUSEAMBIENT);

		LTVector color = m_AmbientLight;

		if(!useOnlyAmbient)
		{
			for( uint32 curLight = 0; curLight < m_LightDefs; curLight++ )
			{
				CLightDef* light = m_LightDefs[curLight];

				if( !light->m_bLightWorld )
					continue;

				if( DoesLightHit( light, pos, true, pStack ) )
				{
					LTVector lightSample;

					if( useNormal )
						lightSample = CalcLightSample( light, light->m_Pos, light->m_Direction, pos, normal, (light->m_Pos - pos).MagSqr() );
					else
						lightSample = CalcLightSample( light, light->m_Pos, light->m_Direction, pos, 1.0f, (light->m_Pos - pos).MagSqr() );

					color += lightSample;
				}
			}

			if( DoesSunlightHit( pos, pStack ) )
			{
				if( useNormal )
					color += CalcSunLightSample( normal );
				else
					color += m_SunLightInner;
			}
		}

		color += (*it).colorOffset;

		uint32 pointColor = (0xff << 24);

		if( color.x >= 255.0f )
			pointColor |= (0xff << 16);
		else if( color.x > 0.0f )
			pointColor |= ((uint32)(color.x) << 16);

		if( color.y >= 255.0f )
			pointColor |= (0xff << 8);
		else if( color.y > 0.0f )
			pointColor |= ((uint32)(color.y) << 8);

		if( color.z >= 255.0f )
			pointColor |= 0xff;
		else if( color.z > 0.0f )
			pointColor |= (uint32)(color.z);

		*((*it).color) = pointColor;

		//update our status bar
		nNumPointsLit++;

		SetProgressBar((float)nNumPointsLit / (float)nNumPointsToLight);
	}

	m_LightingBSP.FreeStack(pStack);

	return true;
}


bool CLightMapMaker::ProcessLight(const LightTable &Dims, PVector** pTable, const std::vector<CLightingPoint>& lightingPoints)
{
	ASSERT( m_pWorld );

	// Setup.
	m_pWorld->RemoveAllLightMaps();

	// Add the main light animation.
	if(!SetupNewLightAnim(BASE_LIGHTANIM_NAME) || !AddAnimFrame())
	{
		Term();
		return false;
	}

	// Setup light definitions from the objects in the world.
	GetLightDefs();

	//create the BSP for lighting
	if(InitPolyTree() == false)
	{
		Term();
		return false;
	}

	ActivateSubTask("Building Lighting BSP");

	//create the BSP for lighting
	uint32 nLightBSPStart = clock();
	if(InitBSP() == false)
	{
		Term();
		return false;
	}
	uint32 nLightBSPElapsed = clock() - nLightBSPStart;	
	DrawStatusText(eST_Normal, "Lighting BSP took %.2f seconds to generate", (float)nLightBSPElapsed / (float)CLOCKS_PER_SEC);

	//calculate the light grid
	DrawStatusText(eST_Normal, "Light table resolution: (%dx%dx%d)",	Dims.m_LookupSize[0], 
																		Dims.m_LookupSize[1], 
																		Dims.m_LookupSize[2]);

	ActivateSubTask("Lighting Grid");

	uint32 nLightGridStart = clock();
	if(!CreateLightGrid(Dims, pTable, true))
	{
		DrawStatusText(eST_Error, "CreateLightGrid failed.");
		Term();
		return false;
	}
	uint32 nLightGridElapsed = clock() - nLightGridStart;	
	DrawStatusText(eST_Normal, "Lighting grid took %.2f seconds to generate", (float)nLightGridElapsed / (float)CLOCKS_PER_SEC);

	uint32 nLGSize = Dims.m_FullLookupSize * sizeof(DRGBColor);
	DrawStatusText(eST_Normal, "Lighting grid size in memory: %dk", (nLGSize + 512) / 1024);


	uint32 nLightPointsStart = clock();

	if( !LightLightingPoints( lightingPoints ) )
	{
		DrawStatusText(eST_Error, "LightLightingPoints failed.");
		Term();
		return false;
	}

	uint32 nLightPointsElapsed = clock() - nLightPointsStart;
	DrawStatusText(eST_Normal, "Lighting points took %.2f seconds to generate", (float)nLightPointsElapsed / (float)CLOCKS_PER_SEC);

	// Setup vertex colors for non-lightmapped mode.
	ActivateSubTask("Lighting Vertices");

	uint32 nVertexStart = clock();

	if(!SetVertexColors())
	{
		DrawStatusText(eST_Error, "SetVertexColors failed.");
		Term();
		return false;
	}

	uint32 nVertexElapsed = clock() - nVertexStart;
	DrawStatusText(eST_Normal, "Vertex colors took %.2f seconds to generate", (float)nVertexElapsed / (float)CLOCKS_PER_SEC);

	if(!g_pGlobs->m_bVerticesOnly)
	{
		// Setup WorldModels with their regular info.
		SetupWorldModels();

		ActivateSubTask("Base Lightmaps");

		uint32 nBaseLMStart = clock();

		if(!CalculateLightFrame())
		{			
			Term();
			return false;
		}
		uint32 nBaseLMElapsed = clock() - nBaseLMStart;
		DrawStatusText(eST_Normal, "Base lightmaps took %.2f seconds to generate", (float)nBaseLMElapsed / (float)CLOCKS_PER_SEC);

	}

	// Now build the light animations.
	if(g_pGlobs->m_bLightAnimations)
	{
		uint32 nLGStart = clock();

		if (!BuildLightGroups(Dims))
		{
			Term();
			return false;
		}

		uint32 nLGElapsed = clock() - nLGStart;
		DrawStatusText(eST_Normal, "Lightgroups took %.2f seconds to generate", (float)nLGElapsed / (float)CLOCKS_PER_SEC);
	}

	Term();
	return true;
}


bool CLightMapMaker::BuildLightGroups(const LightTable &Dims)
{
	uint32 nCurObject;

	//while building lightgroups we shouldn't add the per brush ambient since they
	//are entirely additive
	m_bApplyBrushAmbient = false;

	uint32 nNumLightGroups = 0;
	for (nCurObject = 0; nCurObject < m_pWorld->m_Objects.GetSize(); ++nCurObject)
	{
		CBaseEditObj *pCurObject = m_pWorld->m_Objects[nCurObject];

		// We're only looking for lightgroup objects...
		if (IsLightGroupObject(pCurObject))
			++nNumLightGroups;
	}

	if (!nNumLightGroups)
		return true;

	CLightBSPStack* pStack = m_LightingBSP.CreateStack();
	if(!pStack)
		return false;


	// Turn off the ambient light
	m_AmbientLight.Init();

	uint32 nNumLightGroupsProcessed = 0;

	for (nCurObject = 0; nCurObject < m_pWorld->m_Objects.GetSize(); ++nCurObject)
	{
		CBaseEditObj *pCurObject = m_pWorld->m_Objects[nCurObject];

		// We're only looking for lightgroup objects...
		if (!IsLightGroupObject(pCurObject))
			continue;

		// Tell the user about our status
		++nNumLightGroupsProcessed;

		char pszBuffer[512];
		LTSNPrintF(pszBuffer, sizeof(pszBuffer), "Light Group %d of %d", nNumLightGroupsProcessed, nNumLightGroups);
		ActivateSubTask(pszBuffer);

		// Make a new group
		CPreLightGroup *pLightGroup = new CPreLightGroup;
		pLightGroup->SetName(pCurObject->GetName());

		// Use the new group
		m_pCurLightGroup = pLightGroup;

		// Collect the lights in this group
		GetLightDefs();

		// Figure out the color of this group
		LTVector vColor;
		vColor.Init();
		for (uint32 nCurLight = 0; nCurLight < m_LightDefs.GetSize(); ++nCurLight)
		{
			CLightDef *pCurLight = m_LightDefs[nCurLight];
			
			vColor += pCurLight->m_InnerColor;
		}

		float fLargest = vColor.x;
		if (vColor.y > fLargest)
			fLargest = vColor.y;
		if (vColor.z > fLargest)
			fLargest = vColor.z;
		
		// Don't allow black light groups
		if (fLargest < 0.001f)
		{
			delete pLightGroup;
			continue;
		}

		vColor /= fLargest;

		pLightGroup->SetColor(vColor);

		// Calculate the lighting for this group
		if (!CalculateLightGroupData(Dims, pStack))
		{
			delete pLightGroup;
			continue;
		}

		// Add it to the list
		m_pWorld->m_aLightGroups.push_back(pLightGroup);
	}

	// Clear out the current light group out of paranoia
	m_pCurLightGroup = 0;

	//reenable the per brush ambient
	m_bApplyBrushAmbient = true;

	m_LightingBSP.FreeStack(pStack);

	return true;
}

void CLightMapMaker::ThreadCB(void *pUser)
{
	((CLightMapMaker*)pUser)->ThreadMain();
}


//creates the filter for the supersampled lighting. The callee is expected to free
//the allocated filter with delete[]
static PVector* CreateSubSampleFilter(uint32 nNumSamplesX, uint32 nNumSamplesY, uint32& nFilterSize)
{
	//the filter consists of a listing of XY values ranging from 0 to 1. This
	//is then used as an offset from the lightmap pixel in order calculate lighting


	//figure out how large this filter needs to be
	nFilterSize = nNumSamplesX * nNumSamplesY + 1;

	//first create the filter to contain the grid, plus one sample at the lightmap
	//position
	PVector* pvFilter = new PVector[nFilterSize];

	//make sure this worked
	if(pvFilter == NULL)
	{
		nFilterSize = 0;
		return NULL;
	}

	//create the point exactly at the LM texel
	pvFilter[0].Init(0, 0, 1);

	//now create the grid that fills in the area to the right and below the lm texel
	for(uint32 nCurrY = 0; nCurrY < nNumSamplesY; nCurrY++)
	{
		for(uint32 nCurrX = 0; nCurrX < nNumSamplesX; nCurrX++)
		{
			pvFilter[nCurrY * nNumSamplesX + nCurrX + 1].Init(
				(float)(nCurrX + 1) / (nNumSamplesX + 1),
				(float)(nCurrY + 1) / (nNumSamplesY + 1),
				1);
		}
	}

	//give them back the new filter
	return pvFilter;
}

void CLightMapMaker::ThreadMain()
{
	//a pointer to the filter used for supersampling
	PVector*	pvFilter	= NULL;
	uint32		nFilterSize = 0;

	//create the stack that will be used for ray intersections
	CLightBSPStack *pStack = m_LightingBSP.CreateStack();

	if(!pStack)
	{
		return;
	}

	//if we are going to use super sampling, we need to create the filter
	if(g_pGlobs->m_bLMSuperSample)
	{
		uint32 nNumSamplesX			= g_pGlobs->m_nLMNumSamplesOnSide;
		uint32 nNumSamplesY			= g_pGlobs->m_nLMNumSamplesOnSide;

		 pvFilter = CreateSubSampleFilter(nNumSamplesX, nNumSamplesY, nFilterSize);

		 //make sure this worked. If it didn't we are in low memory conditions
		 if(pvFilter == NULL)
		 {
			 ThreadSignalError();
			 m_LightingBSP.FreeStack(pStack);
			 return;
		 }

	}

	uint32 iPoly;

	// Get polies to light..
	while(ThreadGetNextJob(iPoly))
	{
		//the success of lighting the polygon
		bool bStatus = true;

		//see if we want to use super sampling to generate our lightmaps
		if(g_pGlobs->m_bLMSuperSample)
		{
			bStatus = ThreadLightPolySuperSample(iPoly, nFilterSize, pvFilter, pStack);
		}
		else
		{
			//we want to use the old fashioned way
			bStatus = (ThreadLightPoly(iPoly, pStack)) ? true : false;
		}

		//check the status to see if we need to bail
		if(bStatus == false)
		{
			ThreadSignalError();
			break;
		}
	}

	//need to clean up the filter if we made one
	delete [] pvFilter;

	m_LightingBSP.FreeStack(pStack);
}


bool CLightMapMaker::ThreadLightPoly(uint32 iPoly, CLightBSPStack* pStack)
{
	CPrePoly				*pPoly;
	PVector					colorSum;
	PVector					shade;
	CFullLightMap			*pLightMap, tempMap;
	CPreLightMap			*pPreLightMap;
	GenListPos				lightPos;
	CLightDef*				pLightList[MAX_LIGHTS_PER_POLYGON];


	pPoly = m_pWorld->GetLMPoly(&m_pCurAnim->m_Polies[iPoly]);
	if(!pPoly)
		return false;

	if(!ShouldLightmap(pPoly))
		return true;

	// Make the full light map.
	pLightMap = m_FullLightMaps[iPoly];

	// Make the output lightmap.
	pPreLightMap = m_pCurFrame->m_PolyMaps[iPoly];

	// Figure out how big this poly's lightmaps are.
	GetPolyLightmapSize(pPoly);

	// Setup the lightmap..
	
	if(!InitPolygonFullLightMap(pPoly, pLightMap))
		return false;

	ASSERT(LTDIFF(pLightMap->m_Width, pPoly->m_LMWidth) < 2 && 
		LTDIFF(pLightMap->m_Height, pPoly->m_LMHeight) < 2);

	//if this polygon is receiving lights, we need to calculate the positions and light them
	if(pPoly->GetSurfaceFlags() & SURF_RECEIVELIGHT)
	{
		// Make a list of lights that affect this poly.
		// We do this before calling ComputeLighmapPixelPositions so we can avoid calling
		// it if possible. This speeds up lightmap animations a lot because 
		// most of them only affect a few polies.
		uint32 nNumLights = CalcLightsTouchingPoly(pPoly, iPoly, pLightList, MAX_LIGHTS_PER_POLYGON);

		// Light it
		if(!ApplyLightsToPoly(pPoly, iPoly, nNumLights, pLightList, pStack))
			return false;
	}

	// Convert..
	return FinishLightMap(iPoly, pLightMap);
}

bool CLightMapMaker::ThreadGetNextJob(uint32 &iPoly)
{
	bool bRet;

	if(!m_ThreadCS)
		return false;

	bRet = true;

	thd_EnterCriticalSection(m_ThreadCS);
		
		if(m_bThreadError || !m_pCurAnim)
		{
			bRet = false;
		}
		else if(m_iCurThreadPoly < m_pCurAnim->m_Polies.GetSize())
		{
			iPoly = m_iCurThreadPoly;
			m_iCurThreadPoly++;

			// Show progress..
			SetProgressBar((float)m_iCurThreadPoly / m_pCurAnim->m_Polies.GetSize());
			
			bRet = true;
		}
		else
		{
			bRet = false;
		}
	
	thd_LeaveCriticalSection(m_ThreadCS);

	return bRet;
}


void CLightMapMaker::ThreadSignalError()
{
	if(!m_ThreadCS)
		return;

	thd_EnterCriticalSection(m_ThreadCS);
		m_bThreadError = true;
	thd_LeaveCriticalSection(m_ThreadCS);
}

//called to add all the edges of a polygon into the list. Returns the number of edges that were added
uint32 CLightMapMaker::AddPolyEdgesToList(const CPrePoly* pPoly, CLightMapMaker::TPlaneList *pEdgePlanes)
{
	uint32 nNumEdges = 0;

	LTVector vPrevPt = pPoly->Pt(pPoly->NumVerts() - 1);
	for (uint nCurPt = 0; nCurPt < pPoly->NumVerts(); ++nCurPt)
	{
		LTVector vNextPt = pPoly->Pt(nCurPt);
		LTVector vEdge = vNextPt - vPrevPt;
		float fMag = vEdge.Mag();
		if (fMag < 0.001f)
			continue;

		vEdge /= fMag;
		LTPlane cEdgePlane(pPoly->Normal().Cross(vEdge), vNextPt);

		pEdgePlanes->push_back(cEdgePlane);
		nNumEdges++;

		vPrevPt = vNextPt;
	}

	return nNumEdges;
}

// Note : This only calculates planes for edges which don't have a neighboring poly on the same plane
void CLightMapMaker::CalcPolyEdgePlanes(const CPrePoly *pPoly, CLightMapMaker::TPlaneList *pEdgePlanes, CLightMapMaker::TPlaneCountList *pEdgeCounts)
{
	const float k_fDuplicateNormalEpsilon = 0.01f;
	const float k_fDuplicateDistanceEpsilon = 0.01f;
	const float k_fPtOnPlaneEpsilon = 0.01f;

	//add this polygon's edges to the list
	uint32 nNumEdges = AddPolyEdgesToList(pPoly, pEdgePlanes);

	//add this onto our index list
	pEdgeCounts->push_back(nNumEdges);

	for (uint nCurPoly = 0; nCurPoly < m_Polies.GetSize(); ++nCurPoly)
	{
		const CPrePoly *pCurPoly = m_Polies[nCurPoly];

		// Skip the poly we're actually calculating...
		if (pCurPoly == pPoly)
			continue;

		// It's not a neighbor if it doesn't overlap
		if (!pPoly->OverlapExtents(*pCurPoly))
			continue;

		// It's not a neighbor if it's not on the same plane
		if ((fabsf(pPoly->GetPlane()->m_Dist - pCurPoly->GetPlane()->m_Dist) > k_fDuplicateDistanceEpsilon) ||
			!pPoly->GetPlane()->m_Normal.NearlyEquals(pCurPoly->GetPlane()->m_Normal, k_fDuplicateNormalEpsilon))
			continue;

		// Add it to the edge list
		pEdgeCounts->push_back(AddPolyEdgesToList(pCurPoly, pEdgePlanes));
	}
}


LTVector CLightMapMaker::MovePointIntoPoly(const LTVector &vPos, const CLightMapMaker::TPlaneList &cEdgePlanes, const CLightMapMaker::TPlaneCountList &cEdgeCounts)
{
	//some sanity checks
	assert(cEdgeCounts.size() > 0);

	//see if this lies within the source polygon, finding the closest edge, just in case it lies
	//outside of boundaries
	bool   bOutside = false;

	uint32 nEndEdge  = 0;
	uint32 nCurrEdge = 0;

	float fClosestDistSqr = 1000000.0f;
	LTVector vClosest = vPos;

	//alright, we are outside of the source polygon, we need to determine if it is inside any neighboring
	//polygons, if it is, we can use the original version, otherwise we need to send back our version
	//we pushed inside the polygon
	for(uint32 nCurrPoly = 0; nCurrPoly < cEdgeCounts.size(); nCurrPoly++)
	{
		LTVector vResult = vPos;

		//figure out what edge we are supposed to stop on
		nEndEdge += cEdgeCounts[nCurrPoly];

		//now see if it is within that polygon
		bOutside = false;
		for(; nCurrEdge < nEndEdge; nCurrEdge++)
		{
			float fEdgeDist = cEdgePlanes[nCurrEdge].DistTo(vResult);
			if (fEdgeDist > -0.0001f)
			{
				bOutside = true;
				vResult -= cEdgePlanes[nCurrEdge].m_Normal * (fEdgeDist + 0.1f);
			}
		}

		//see if we were within this polygon
		if(!bOutside)
		{
			//we were, the source position is OK
			return vPos;
		}

		//see if this is the closest point
		float fDistSqr = (vResult - vPos).MagSqr();
		if(fDistSqr < fClosestDistSqr)
		{
			fClosestDistSqr = fDistSqr;
			vClosest = vResult;
		}
	}

	//we weren't within any of the neighboring polygons either, use the modified version
	return vClosest;
}


bool CLightMapMaker::ApplyLightsToPoly(
	CPrePoly *pPoly, uint32 nPolyAnimIndex,
	uint32 nNumLights, CLightDef **pLights, CLightBSPStack* pStack)
{
	uint32 x, y;
	PVector dir, norm, intersection, uVectorAdd, vVectorAdd;
	PVector uRayOrigin, vRayOrigin;
	PVector rayOrigin, rayDir, vecTo;
	PVector colorAdd, *pCurMapPos, neighborSum;
	CFullLightMap *pLightMap;

	PVector polyNorm = pPoly->GetPlane()->m_Normal;

	pLightMap = m_FullLightMaps[nPolyAnimIndex];

	// Precalculate increments and stuff..
	vVectorAdd = pPoly->InverseQ() * (float)m_pWorld->GetLMGridSize(pPoly);
	uVectorAdd = pPoly->InverseP() * (float)m_pWorld->GetLMGridSize(pPoly);

	//determine if we need to shift the lightmap for supersampling
	PReal fShift = GetShiftAmount(pPoly);

	vRayOrigin = pPoly->PolyO() +	pPoly->InverseP() * fShift + 
									pPoly->InverseQ() * fShift;
	
	if(pPoly->m_pWorld->m_bUsingTransform)
	{
		// Transform into the world model's space.
		MatVMul_InPlace_3x3(&pPoly->m_pWorld->m_mTransform, &uVectorAdd);
		MatVMul_InPlace_3x3(&pPoly->m_pWorld->m_mTransform, &vVectorAdd);
		vRayOrigin = pPoly->m_pWorld->m_mTransform * vRayOrigin;
	}

	//determine if we want to clip the lights falling on this polygon
	bool bClipLight = (pPoly->GetSurfaceFlags() & SURF_RECEIVESHADOWS) && m_bClipLight;

	//determine if we want static sunlight
	bool bSunlight = m_bStaticSunLight && (pPoly->GetSurfaceFlags() & SURF_RECEIVESUNLIGHT);
	if(pPoly->Normal().Dot(m_SunLightDir) > 0.0f)
	{
		bSunlight = false;
	}

	// Create the edge planes
	static TPlaneList cEdgePlanes;
	cEdgePlanes.clear();
	static TPlaneCountList cEdgeCounts;
	cEdgeCounts.clear();

	CalcPolyEdgePlanes(pPoly, &cEdgePlanes, &cEdgeCounts);
	
	pCurMapPos = pLightMap->m_Map.GetArray();
	for(y=0; y < pLightMap->m_Height; y++)
	{
		uRayOrigin = vRayOrigin;
		
		for(x=0; x < pLightMap->m_Width; x++)
		{
			LTVector vResult = LightDirectionalPoint(
				MovePointIntoPoly(uRayOrigin, cEdgePlanes, cEdgeCounts), uRayOrigin, pPoly, 
				bSunlight, bClipLight, 
				pLights, nNumLights, pStack);

			*pCurMapPos += vResult;
			
			uRayOrigin += uVectorAdd;
			++pCurMapPos;
		}

		vRayOrigin += vVectorAdd;
	}

	return true;
}

bool CLightMapMaker::CalculateLightFrame()
{
	if(!m_pCurAnim || !m_pCurFrame || !m_ThreadCS)
	{
		ASSERT(false);
		return false;
	}

	DeleteAndClearArray(m_FullLightMaps);


	// Create all the objects.
	if(!m_FullLightMaps.SetSizeInit2(m_pCurAnim->m_Polies.GetSize(), NULL) ||
		!m_pCurFrame->m_PolyMaps.SetSizeInit2(m_pCurAnim->m_Polies.GetSize(), NULL) ||
		!AllocateArray(m_FullLightMaps, (CFullLightMap*)NULL) ||
		!AllocateArray(m_pCurFrame->m_PolyMaps, (CPreLightMap*)NULL))
	{
		DeleteAndClearArray(m_FullLightMaps);
		DeleteAndClearArray(m_pCurFrame->m_PolyMaps);
		return false;
	}

	
	// Spawn each thread.  Each thread will ask for a poly and provide notification
	// when it's done.
	m_iCurThreadPoly = 0;
	m_bThreadError = false;

	// Do the stuff that used to use threads to do the work
	ThreadCB(this);

	return !m_bThreadError;
}

// Changes a lightmap from raw color data to shades of grey based on a normalized color
void NormalizeLightmap(CPreLightMap *pLightmap, const LTVector &vColor)
{
	// Decompress it
	LM_DATA aDecompBuffer[LIGHTMAP_MAX_DATA_SIZE];
	pLightmap->Decompress(aDecompBuffer);
	LM_DATA *pEndOfData = &aDecompBuffer[pLightmap->m_Width * pLightmap->m_Height * 3];
	LM_DATA *pCurData = aDecompBuffer;
	for (; pCurData != pEndOfData; pCurData += 3)
	{
		LTVector vLMColor((float)pCurData[0], (float)pCurData[1], (float)pCurData[2]);
		float fIntensity = vLMColor.Dot(vColor);
		fIntensity = LTCLAMP(fIntensity, 0.0f, 255.0f);
		pCurData[0] = pCurData[1] = pCurData[2] = (uint8)fIntensity;
	}

	// Re-compress it
	pLightmap->Compress(pLightmap->m_Width, pLightmap->m_Height, aDecompBuffer);
}

bool CLightMapMaker::CalculateLightGroupLightGrid(const LightTable &Dims, const LTVector &vNormalizedColor)
{
	// Create the raw data
	LTVector *pRawData;
	if (!CreateLightGrid(Dims, &pRawData, false))
		return false;

	// Convert the raw data into byte intensities
	uint32 nNumSamples = Dims.m_FullLookupSize;
	uint8 *pCookedData = new uint8[Dims.m_FullLookupSize];
	LTVector *pRawFinger = pRawData;
	uint8 *pCookedFinger = pCookedData;
	LTVector *pRawEnd = &pRawData[nNumSamples];
	uint8 *pCookedEnd = &pCookedData[nNumSamples];
	for (; pRawFinger != pRawEnd; ++pRawFinger, ++pCookedFinger)
	{
		float fIntensity = pRawFinger->Dot(vNormalizedColor);
		*pCookedFinger = (uint8)LTMIN(fIntensity, 255.0f);
	}

	// Ok, we're done with the raw data now...
	delete[] pRawData;
	pRawData = 0;

	// Get the bounds of the data
	uint32 nMinX = Dims.m_LookupSize[0], nMaxX = 0;
	uint32 nMinY = Dims.m_LookupSize[1], nMaxY = 0;
	uint32 nMinZ = Dims.m_LookupSize[2], nMaxZ = 0;
	pCookedFinger = pCookedData;
	for (uint32 nCurZ = 0; nCurZ < Dims.m_LookupSize[2]; ++nCurZ)
	{
		for (uint32 nCurY = 0; nCurY < Dims.m_LookupSize[1]; ++nCurY)
		{
			for (uint32 nCurX = 0; nCurX < Dims.m_LookupSize[0]; ++nCurX, ++pCookedFinger)
			{
				if (*pCookedFinger == 0)
					continue;

				nMinX = LTMIN(nMinX, nCurX);
				nMaxX = LTMAX(nMaxX, nCurX);
				nMinY = LTMIN(nMinY, nCurY);
				nMaxY = LTMAX(nMaxY, nCurY);
				nMinZ = LTMIN(nMinZ, nCurZ);
				nMaxZ = LTMAX(nMaxZ, nCurZ);
			}
		}
	}

	// Set up the final data
	CPreLightGroup::SLightGrid &cLightGrid = m_pCurLightGroup->GetLightGrid();

	// Check for an empty set
	if ((nMinX > nMaxX) || (nMinY > nMaxY) || (nMinZ > nMaxZ))
	{
		cLightGrid.m_vOffset.Init(0,0,0);
		cLightGrid.m_vSize.Init(0,0,0);
		delete[] pCookedData;
		return true;
	}

	// Set up the bounds
	cLightGrid.m_vOffset.Init(nMinX, nMinY, nMinZ);
	cLightGrid.m_vSize.Init(nMaxX - nMinX + 1, nMaxY - nMinY + 1, nMaxZ - nMinZ + 1);

	// Allocate the data
	ASSERT(cLightGrid.m_pSamples == 0);
	cLightGrid.m_pSamples = new uint8[cLightGrid.GetTotalSampleCount()];

	// Copy it into place
	uint32 nYStride = Dims.m_LookupSize[0];
	uint32 nZStride = Dims.m_LookupSize[1] * nYStride;
	uint8 *pZLine = &pCookedData[nZStride * nMinZ];
	uint8 *pSampleFinger = cLightGrid.m_pSamples;
	for (uint32 nCopyZ = nMinZ; nCopyZ <= nMaxZ; ++nCopyZ)
	{
		uint8 *pYLine = &pZLine[nYStride * nMinY];
		for (uint32 nCopyY = nMinY; nCopyY <= nMaxY; ++nCopyY)
		{
			ASSERT((&pSampleFinger[cLightGrid.m_vSize.x] <= &cLightGrid.m_pSamples[cLightGrid.GetTotalSampleCount()]) &&
				(&pYLine[nMinX + cLightGrid.m_vSize.x] <= &pCookedData[Dims.m_FullLookupSize]));
			memcpy(pSampleFinger, &pYLine[nMinX], cLightGrid.m_vSize.x);
			pSampleFinger += cLightGrid.m_vSize.x;
			pYLine += nYStride;
		}
		pZLine += nZStride;
	}

	delete[] pCookedData;
	return true;
}

bool CLightMapMaker::CalculateLightGroupData(const LightTable &Dims, CLightBSPStack* pStack)
{
	// Get 1/3 the color, which means we can use the dot product for calculating an average intensity value
	LTVector vNormalizedColor = m_pCurLightGroup->GetColor() / 3.0f;

	// Generate the lighting grid
	if (!CalculateLightGroupLightGrid(Dims, vNormalizedColor))
		return false;

	// Calculate the lightmaps for this group
	if(SetupNewLightAnim(LIGHTGROUP_ANIMNAME) && AddAnimFrame() && CalculateLightFrame())
	{
		RemoveExcessPolies();
	}
	else
		m_pCurAnim = 0;

	typedef std::vector<LTVector> TRawList;
	TRawList aRawLightData;
	CPreLightGroup_PolyData::TVertexData aCookedLightData;
	CPreLightGroup_PolyData polyData;

	for (uint32 nCurPoly = 0; nCurPoly < m_Polies.GetSize(); ++nCurPoly)
	{
		CPrePoly *pCurPoly = m_Polies[nCurPoly];

		// Early-reject polygons that obviously won't touch this group
		// NYI

		CPrePolyRef hPoly;
		hPoly.m_iPoly = pCurPoly->m_Index;
		hPoly.m_iWorld = pCurPoly->m_WorldIndex;

		CPreLightMap *pLightmap = 0;
		// Try to steal the lightmap from the lightmap animation
		if (m_pCurAnim && ((pCurPoly->GetSurfaceFlags() & SURF_LIGHTMAP) != 0))
		{
			for (uint32 nLMPoly = 0; nLMPoly < m_pCurAnim->m_Polies.GetSize(); ++nLMPoly)
			{
				// Did we find it?
				if (m_pCurAnim->m_Polies[nLMPoly] == hPoly)
				{
					// Take it.
					pLightmap = m_pCurAnim->m_Frames[0]->m_PolyMaps[nLMPoly];
					m_pCurAnim->m_Frames[0]->m_PolyMaps[nLMPoly] = 0;
					break;
				}
			}
		}

		bool bHasLitVerts = true;

		// Get the lighting for this polygon
		aRawLightData.resize(pCurPoly->NumVerts());
		if (!LightPolygon(pCurPoly, &aRawLightData.front(), pStack))
		{
			bHasLitVerts = false;
			if (!pLightmap)
				continue;
		}

		// Convert the lighting data to the right format
		aCookedLightData.clear();
		aCookedLightData.reserve(aRawLightData.size());

		uint8 nMax = 0;

		TRawList::const_iterator iCurRawSample = aRawLightData.begin();
		for (; iCurRawSample != aRawLightData.end(); ++iCurRawSample)
		{
			float fIntensity = iCurRawSample->Dot(vNormalizedColor);
			LTCLAMP(fIntensity, 0, 255);
			uint8 nValue = (uint8)fIntensity;
			nMax = LTMAX(nMax, nValue);
			aCookedLightData.push_back(nValue);
		}

		if (!nMax)
			bHasLitVerts = false;

		// If it wouldn't add anything, skip this poly
		if (!bHasLitVerts && (pLightmap == 0))
			continue;

		// Normalize the lightmap
		if (pLightmap)
			NormalizeLightmap(pLightmap, vNormalizedColor);

		// Add an entry to the light group
		m_pCurLightGroup->GetPolyData().push_back(polyData);
		CPreLightGroup_PolyData &cNewData = m_pCurLightGroup->GetPolyData().back();
		cNewData.m_aVertexIntensities.swap(aCookedLightData);
		cNewData.m_hPoly = hPoly;
		cNewData.m_pLightmap = pLightmap;
	}

	// Get rid of the lightgroup LM animation
	if (m_pCurAnim)
	{
		m_pWorld->RemoveLightAnim(m_pCurAnim);
		// Yes, the above function call also deletes the animation.  Don't ask me why.
		m_pCurAnim = 0;
	}

	return true;
}


void CLightMapMaker::GetPolyLightmapSize(CPrePoly *pPoly)
{
	uint16 xPixels, yPixels;

	pPoly->CalculateTextureSize(FALSE, g_pGlobs->m_MaxLMSize, (float)m_pWorld->GetLMGridSize(pPoly),
		xPixels, yPixels);

	pPoly->m_LMWidth = xPixels / (uint16)m_pWorld->GetLMGridSize(pPoly) + 1;
	pPoly->m_LMHeight = yPixels / (uint16)m_pWorld->GetLMGridSize(pPoly) + 1;

	if(pPoly->m_LMWidth > LIGHTMAP_MAX_PIXELS_I || pPoly->m_LMHeight > LIGHTMAP_MAX_PIXELS_I)
	{
		LTVector vPolyCenter = pPoly->CalcCenter();

		DrawStatusText(eST_Warning, "Invalid poly lightmap size (%dx%d), clamping. (Polygon center: %.2f %.2f %.2f)", pPoly->m_LMWidth, pPoly->m_LMHeight, VEC_EXPAND(vPolyCenter));
		pPoly->m_LMWidth = DMIN(pPoly->m_LMWidth, LIGHTMAP_MAX_PIXELS_I);
		pPoly->m_LMHeight = DMIN(pPoly->m_LMHeight, LIGHTMAP_MAX_PIXELS_I);
	}
}


// Helper function for LightPolygon to select the right destination for values
static LTVector &GetPolygonVertexColor(CPrePoly *pPoly, LTVector *pColorArray, uint32 nIndex)
{
	if (pColorArray)
		return pColorArray[nIndex];
	else
		return pPoly->Vert(nIndex).m_Color;
}

// Lights a polygon's vertices.  pColorArray is non-null for lightgroup processing
// Returns true if the polygon was affected by any lighting
bool CLightMapMaker::LightPolygon(CPrePoly *pPoly, LTVector *pColorArray, CLightBSPStack* pStack)
{
	uint32 k;

	//determine if this light should receive sunlight
	bool bReceiveSunlight = m_bStaticSunLight && (pPoly->GetSurfaceFlags() & SURF_RECEIVESUNLIGHT) && !pColorArray;
	
	//make sure that we want to light these verts
	if ((pPoly->GetSurfaceFlags() & (	SURF_GOURAUDSHADE |
										SURF_LIGHTMAP |
										SURF_FLATSHADE )) == 0)
	{
		//don't want to vertex light this polygon
		return false;
	}

	//get the ambient value of this brush
	PVector vAmbient(m_AmbientLight);
	
	if(m_bApplyBrushAmbient)
	{
		vAmbient += PVector(	pPoly->GetSurface()->m_Ambient[0], 
								pPoly->GetSurface()->m_Ambient[1], 
								pPoly->GetSurface()->m_Ambient[2]);

        // Make sure to clamp the ambient in case we
        // early out of this function.
        VEC_CLAMP(vAmbient, 0, 255);
	}

	//we do. Init all the colors to be that of the ambient light
	for(k=0; k < pPoly->NumVerts(); k++)
	{
		GetPolygonVertexColor(pPoly, pColorArray, k) = pColorArray ? LTVector(0,0,0) : vAmbient;
	}

	//now if this polygon is receiving light, give it the appropriate lighting
	if((pPoly->GetSurfaceFlags() & SURF_RECEIVELIGHT) == 0)
		return false;

	//determine if we are to receive shadows
	bool bClipLight = (m_bClipLight && (pPoly->GetSurfaceFlags() & SURF_RECEIVESHADOWS));

	if(pPoly->GetSurfaceFlags() & (SURF_GOURAUDSHADE|SURF_LIGHTMAP))
	{
		// Gouraud.
		for(k=0; k < pPoly->NumVerts(); k++)
		{
			GetPolygonVertexColor(pPoly, pColorArray, k) += 
				LightDirectionalPoint(pPoly->Vert(k).m_Vec,
					pPoly->Vert(k).m_Normal,
					bReceiveSunlight, 
					bClipLight, pStack);
		}
	}
	else if(pPoly->GetSurfaceFlags() & SURF_FLATSHADE)
	{
		CPolyVert center;
		center.m_Vec = pPoly->GetCenter();
		center.m_Color = LightDirectionalPoint(	center.m_Vec, pPoly->Normal(), bReceiveSunlight, 
															bClipLight, pStack);

		for(k=0; k < pPoly->NumVerts(); ++k)
		{
			GetPolygonVertexColor(pPoly, pColorArray, k) += center.m_Color;
		}
	}

	//we now need to clamp all the colors
	for(k=0; k < pPoly->NumVerts(); k++)
	{
		LTVector &vColor = GetPolygonVertexColor(pPoly, pColorArray, k);
		VEC_CLAMP(vColor, 0, 255);
	}

	return true;
}

bool CLightMapMaker::SetVertexColors()
{
	CLightBSPStack* pStack = m_LightingBSP.CreateStack();

	if(!pStack)
		return false;

	for(uint32 i=0; i < m_Polies.GetSize(); i++)
	{
		LightPolygon(m_Polies[i], NULL, pStack);

		SetProgressBar((float)i / (float)m_Polies.GetSize());
	}

	m_LightingBSP.FreeStack(pStack);

	return true;
}


void CLightMapMaker::AddLightDef(TLightDef *pLightDef, const PVector& pos, CPrePoly *pPoly)
{
	CLightDef *pLight;

	pLight = new CLightDef;
	m_LightDefs.Append( pLight );

	pLight->m_Pos				= pos;
	pLight->m_Direction			= pPoly->Normal();
	pLight->m_InnerColor		= pLightDef->m_InnerColor * pLightDef->m_BrightScale;
	pLight->m_bClip				= pLightDef->m_bClipLight;
	pLight->m_bDirectional		= pLightDef->m_bDirectional;
	
	pLight->m_MaxDist			= pLightDef->m_Radius;
	pLight->m_MaxDistSqr		= pLight->m_MaxDist * pLight->m_MaxDist;

	//now we need to set up the coefficients of the light
	if(pLightDef->m_bApproxLinear)
	{
		//need to use the default values, scaled appropriately
		pLight->m_A			= 1;
		pLight->m_B			= 0;
		pLight->m_C			= 19 / (pLightDef->m_Radius * pLightDef->m_Radius);
	}
	else
	{
		//use the given values
		pLight->m_A			= pLightDef->m_Coefficients.x; 
		pLight->m_B			= pLightDef->m_Coefficients.y;
		pLight->m_C			= pLightDef->m_Coefficients.z;
	}

	//set the default field of view
	pLight->SetFOV(pLightDef->m_FOVDegrees * MATH_PI / 180.0f);

	//need to check the bright scale, and if it is negative, we should make it
	//positive, but still get the same effect by negating the colors
	if(pLightDef->m_BrightScale < 0.0f)
	{
		pLight->m_InnerColor  = -pLight->m_InnerColor;
	}
}

bool CLightMapMaker::IsLightGroupObject(CBaseEditObj *pObject)
{
	if (strcmp(pObject->GetClassName(), LIGHTGROUP_CLASSNAME) == 0)
		return true;
	else if (strcmp(pObject->GetClassName(), LIGHTGROUP_CLASSNAME_GAME) == 0)
		return true;
	else
		return false;
}

// Walk up the heirarchy looking for a light group object
const char *CLightMapMaker::GetObjectLightGroupName(CBaseEditObj *pObject)
{
	//see if we can just read it in from the light property
	CBaseProp* pLightGroupProp = pObject->GetPropertyList()->GetProp("LightGroup");
	
	if(pLightGroupProp && (pLightGroupProp->GetType() == LT_PT_STRING))
	{
		//we can, but this function requires that if there is no light group, we return NULL
		const char* pszLG = ((CStringProp*)pLightGroupProp)->m_String;

		if(strlen(pszLG) > 0)
			return pszLG;

		//no light group
		return NULL;
	}

	//no such luck, we will have to run through the heirarchy and find any containing parents
	CBaseEditObj *pParent = (CBaseEditObj *)pObject->GetParent();

	while (pParent)
	{
		if (!ShouldIgnoreNode(pObject))
		{
			if (IsLightGroupObject(pParent))
				return pParent->GetName();
		}
		pParent = (CBaseEditObj *)pParent->GetParent();
	}

	// We got to the top without hitting a lightgroup object...  :(
	return NULL;
}

void CLightMapMaker::GetLightDefs()
{
	uint32			i;
	CPropList		*pPropList;
	CLightDef		*pLight;
	CColorProp		*pColor;
	CVectorProp		*pVectorProp;
	CBoolProp		*pBoolProp;
	CBaseEditObj	*pObject;
	CRealProp		*pRealProp;
	CStringProp		*pStringProp;
	CRotationProp	*pRot;
	LTVector		vUp, vRight, vForward;
	LTMatrix		mTemp;


	ClearLightDefs();

	for( i=0; i < m_pWorld->m_Objects.GetSize(); i++ )
	{
		pObject = m_pWorld->m_Objects[i];
		pPropList = &pObject->m_PropList;

		//skip over hidden items (this fixes the bug where hidden lights were
		//being processed). Note: This also relies on the LTA fix since in the
		//original ed version light flags were not being properly loaded in
		//-JohnO
		if(ShouldIgnoreNode(pObject))
		{
			continue;
		}  

		if(	(strcmp(pObject->GetClassName(), "Light") == 0) || 
			(strcmp(pObject->GetClassName(), "DirLight") == 0) ||
			(strcmp(pObject->GetClassName(), "ObjectLight") == 0))
		{
			// Make sure we're dealing with a light that fits into our current light group
			const char *pLightGroupName = GetObjectLightGroupName(pObject);
			if ((pLightGroupName && !m_pCurLightGroup) ||
				(!pLightGroupName && m_pCurLightGroup))
				continue;
			else if (m_pCurLightGroup && pLightGroupName)
			{
				if (stricmp(m_pCurLightGroup->GetName(), pLightGroupName) != 0)
					continue;
			}

			pLight = new CLightDef;
			m_LightDefs.Append( pLight );

			float fBrightScale = 1.0f;

			pLight->m_pObject = pObject;

			//see if it is a directional light
			pLight->m_bDirectional = (strcmp(pObject->GetClassName(), "DirLight") == 0);

			//only light the world if it is not an object light
			pLight->m_bLightWorld = (strcmp(pObject->GetClassName(), "ObjectLight") != 0);

			// LightColor maps to InnerColor.
			if(pColor = (CColorProp*)pPropList->GetProp("LightColor"))
			{
				pLight->m_InnerColor = pColor->m_Vector;
			}
			if(pColor = (CColorProp*)pPropList->GetProp("InnerColor"))
			{
				pLight->m_InnerColor = pColor->m_Vector;
			}

			if(pVectorProp = (CVectorProp*)pPropList->GetProp("Pos"))
			{
				pLight->m_Pos = pVectorProp->m_Vector;
			}

			if((pRot = (CRotationProp*)pPropList->GetProp("Rotation")) && pRot->m_Type == PT_ROTATION)
			{
				gr_GetEulerVectors(
					pRot->GetEulerAngles(),
					vRight,
					vUp,
					vForward);
				pLight->m_Direction = vForward;
			}

			if((pBoolProp = (CBoolProp*)pPropList->GetProp("ClipLight")) && pBoolProp->m_Type == PT_BOOL)
			{
				pLight->m_bClip = (pBoolProp->m_Value) ? true : false;
			}

			if((pBoolProp = (CBoolProp*)pPropList->GetProp("CastShadowMesh")) && pBoolProp->m_Type == PT_BOOL)
			{
				pLight->m_bShadowMesh = (pBoolProp->m_Value) ? true : false;
			}

			if((pBoolProp = (CBoolProp*)pPropList->GetProp("FastLightObjects")) && pBoolProp->m_Type == PT_BOOL)
			{
				pLight->m_bLightGrid = (pBoolProp->m_Value) ? true : false;
			}

			if((pRealProp = (CRealProp*)pPropList->GetProp("FOV")) && pRealProp->m_Type == PT_REAL)
			{
				pLight->SetFOV((pRealProp->m_Value / 360.0f) * MATH_CIRCLE);
			}

			if((pRealProp = (CRealProp*)pPropList->GetProp("LightRadius")) && pRealProp->m_Type == PT_REAL)
			{
				pLight->m_MaxDist = pRealProp->m_Value;
			}
			
			if((pRealProp = (CRealProp*)pPropList->GetProp("BrightScale")) && pRealProp->m_Type == PT_REAL)
			{
				fBrightScale = pRealProp->m_Value;
			}

			if((pRealProp = (CRealProp*)pPropList->GetProp("Size")) && pRealProp->m_Type == PT_REAL)
			{
				pLight->m_fSize = pRealProp->m_Value;
			}

			if((pStringProp = (CStringProp*)pPropList->GetProp("Attenuation")) && pStringProp->m_Type == PT_STRING)
			{
				if (stricmp(pStringProp->m_String, "D3D") == 0)
					pLight->m_eAttenuation = CLightDef::eAttenuation_D3D;
				else if (stricmp(pStringProp->m_String, "Quartic") == 0)
					pLight->m_eAttenuation = CLightDef::eAttenuation_Quartic;
				else if (stricmp(pStringProp->m_String, "Linear") == 0)
					pLight->m_eAttenuation = CLightDef::eAttenuation_Linear;
			}

			// setup the lighting attenuation constants
			PVector AttCoefs( 1.0f, 0.0f, 19.0f );
			PVector AttExps( 0.0f, 0.0f, -2.0f );

			if(pVectorProp = (CVectorProp*)pPropList->GetProp("AttCoefs"))
			{
				AttCoefs = pVectorProp->m_Vector;
			}

			if( pVectorProp = (CVectorProp*)pPropList->GetProp("AttExps"))
			{
				AttExps = pVectorProp->m_Vector;
			}

			// adjust for negative lights
			if(fBrightScale < 0.0f)
			{
				fBrightScale *= -1.0f;
			}

			pLight->m_InnerColor *= fBrightScale;

			// finalize lighting attenuation constants
			pLight->m_A = AttCoefs.x * powf( pLight->m_MaxDist, AttExps.x );
			pLight->m_B = AttCoefs.y * powf( pLight->m_MaxDist, AttExps.y );
			pLight->m_C = AttCoefs.z * powf( pLight->m_MaxDist, AttExps.z );
			
			pLight->m_MaxDistSqr = pLight->m_MaxDist * pLight->m_MaxDist;
		}
		else if((strcmp(pObject->GetClassName(), "StaticSunLight") == 0) && !m_pCurLightGroup)
		{
			m_bStaticSunLight = true;

			if((pColor = (CColorProp*)pPropList->GetProp("InnerColor")) &&
				pColor->m_Type == PT_COLOR)
			{
				m_SunLightInner = pColor->m_Vector;
			}

			if((pRot = (CRotationProp*)pPropList->GetProp("Rotation")) &&
				pRot->m_Type == PT_ROTATION)
			{
				gr_SetupMatrixEuler(pRot->GetEulerAngles(), mTemp.m);
				mTemp.GetBasisVectors(&vRight, &vUp, &m_SunLightDir);
			}

			if((pRealProp = (CRealProp*)pPropList->GetProp("BrightScale")) && 
				pRealProp->m_Type == PT_REAL)
			{
				m_SunLightBrightScale = pRealProp->m_Value;
			}
		}
	}
}


bool CLightMapMaker::ShouldBlockLight(CPreWorld *pWorld)
{
	uint32 i;
	CBaseEditObj *pObj;
	CBoolProp *pProp;

	// The physics BSP blocks light, but the vis bsp does not.
	if(pWorld->m_WorldInfoFlags & WIF_PHYSICSBSP)
		return true;

	// Find its object..
	for(i=0; i < m_pWorld->m_Objects; i++)
	{
		pObj = m_pWorld->m_Objects[i];

		if(stricmp(pObj->GetName(), pWorld->m_WorldName) == 0)
		{
			if(pProp = (CBoolProp*)pObj->m_PropList.GetMatchingProp("BlockLight", PT_BOOL))
			{
				if(pProp->m_Value)
					return true;
			}
		}
	}

	return false;
}


bool CLightMapMaker::SetupWorldModels()
{
	uint32 iModel;
	LMMWorld *pWorld;
	PVector pts[8];
	bool bInvisible;

	
	for(iModel=0; iModel < m_LMMWorlds; iModel++)
	{
		pWorld = &m_LMMWorlds[iModel];

		// Initialize its transform.
		pWorld->m_bUsingTransform = false;
		pWorld->m_mTransform.Identity();
		bInvisible = false;

		pWorld->m_mInverseTransform = pWorld->m_mTransform.MakeInverse();
	}

	return true;
}


void CLightMapMaker::ClearLightDefs()
{
	// Init the global dir light.
	m_SunLightInner.Init();
	m_bStaticSunLight = false;
	DeleteAndClearArray(m_LightDefs);
}


bool CLightMapMaker::LightTouchesPoly( CLightDef *pLight, CPrePoly *pPoly, uint32 nPolyIndex )
{
	PVector tLightPos;
	if(pPoly->m_pWorld->m_bUsingTransform)
	{
		tLightPos = pPoly->m_pWorld->m_mInverseTransform * pLight->m_Pos;
	}
	else
	{
		tLightPos = pLight->m_Pos;
	}

	// Backface removal?
	if(m_bClipLight && pLight->m_bClip)
	{
		if(DIST_TO_PLANE(tLightPos, *pPoly->GetPlane()) < 0.0f)
			return false;
	}

	CFullLightMap *pLightMap = m_FullLightMaps[nPolyIndex];
	for(uint32 i=0; i < 3; i++)
	{
		if((tLightPos[i]-pLight->m_MaxDist) > pLightMap->m_Max[i])
			return false;
		else if((tLightPos[i]+pLight->m_MaxDist) < pLightMap->m_Min[i])
			return false;
	}

	return true;
}


void CLightMapMaker::ClampMapValues( CFullLightMap *pMap, uint32 nSamplesU, uint32 nSamplesV )
{
	// Clamp the values.
	uint32			u, v;
	for( u=0; u < nSamplesU; u++ )
	{
		for( v=0; v < nSamplesV; v++ )
		{
			PVector& Vec = pMap->Map( u, v );
			VEC_CLAMP(Vec, 0.0f, 255.0f);
		}
	}
}

void CLightMapMaker::DownSample( CFullLightMap *pIn, CFullLightMap *pOut )
{
	uint32 x, y;
	PVector *pCurTopIn, *pCurBottomIn, *pCurOut;

	
	if(pIn->m_Width == 0 || pIn->m_Height == 0)
		return;

	pOut->SetSize(pIn->m_Width-1, pIn->m_Height-1);
	pCurOut = pOut->m_Map.GetArray();

	for(y=0; y < pOut->m_Height; y++)
	{
		pCurTopIn = &pIn->m_Map[y*pIn->m_Width];
		pCurBottomIn = &pIn->m_Map[(y+1)*pIn->m_Width];

		for(x=0; x < pOut->m_Width; x++)
		{
			*pCurOut = (*pCurTopIn + *pCurBottomIn + *(pCurTopIn+1) + *(pCurBottomIn+1)) * 0.25f;
			++pCurOut;
			++pCurTopIn;
			++pCurBottomIn;
		}
	}
}


bool CLightMapMaker::ConvertMap( CFullLightMap *pIn, CPreLightMap *pOut )
{
	uint32 u, v;
	PVector *pCurIn;
	LM_DATA *pCurOut;
	LM_DATA tempData[LIGHTMAP_MAX_DATA_SIZE];


	if(pIn->m_Width > LIGHTMAP_MAX_PIXELS_I || pIn->m_Height > LIGHTMAP_MAX_PIXELS_I)
	{
		ASSERT(false);
		return false;
	}

	// Convert the values.
	pCurOut = tempData;
	pCurIn = pIn->m_Map.GetArray();
	for(v=0; v < pIn->m_Height; v++)
	{
		for(u=0; u < pIn->m_Width; u++)
		{
			//convert the vector into a RGB888 in bytes
			pCurOut[0] = (LM_DATA)pCurIn->x;
			pCurOut[1] = (LM_DATA)pCurIn->y;
			pCurOut[2] = (LM_DATA)pCurIn->z;
			pCurOut += 3;

			pCurIn++;
		}
	}

	return pOut->Compress(pIn->m_Width, pIn->m_Height, tempData);
}

bool CLightMapMaker::RemoveExcessPolies()
{
	uint32 iFrame, iPoly;
	bool bAllBlack;
	CPreLightFrame *pFrame;
	CPreLightMap *pMap;

	if(!m_pCurAnim)
		return false;

	for(iPoly=0; iPoly < m_pCurAnim->m_Polies; iPoly++)
	{
		// Are all the frames black?
		bAllBlack = true;
		for(iFrame=0; iFrame < m_pCurAnim->m_Frames; iFrame++)
		{
			pFrame = m_pCurAnim->m_Frames[iFrame];

			if(pFrame->m_PolyMaps.GetSize() != m_pCurAnim->m_Polies.GetSize())
				return false;

			pMap = pFrame->m_PolyMaps[iPoly];
			if(!pMap->AllBlack())
			{
				bAllBlack = false;
				break;
			}
		}

		if(bAllBlack)
		{
			// The lightmap is entirely black, we can simply disregard it
			m_pCurAnim->m_Polies.Remove(iPoly);
			
			for(iFrame=0; iFrame < m_pCurAnim->m_Frames; iFrame++)
			{
				pFrame = m_pCurAnim->m_Frames[iFrame];
				
				delete pFrame->m_PolyMaps[iPoly];
				pFrame->m_PolyMaps.Remove(iPoly);
			}

			--iPoly;
		}
	}

	return true;
}


//*****************************************************************************************
// Lightmapping using super sampling
// ****************************************************************************************
bool CLightMapMaker::InitPolygonFullLightMap(CPrePoly* pPoly, CFullLightMap* pLightMap)
{
	//Figure out how big this poly's lightmaps are.
    GetPolyLightmapSize(pPoly);

    // Setup the lightmap..

	//see if we are adding a buffer edge for the oversampling
	if(m_bOverSample)
	{
		pLightMap->SetSize(	pPoly->m_LMWidth + 1, pPoly->m_LMHeight + 1);
	}
	else
	{
		pLightMap->SetSize(	pPoly->m_LMWidth, pPoly->m_LMHeight);
	}


    //fill the lightmap with the initial color.
    InitPolyLightmapFill(pPoly, pLightMap);

    //compute the min/max of the lightmap.
    InitPolyLightmapMinMax(pPoly, pLightMap);

	return true;
}

//runs through the light definitions and determines which lights effect this polygon.
//returns the number of lights that touch the surface
uint32 CLightMapMaker::CalcLightsTouchingPoly(CPrePoly *pPoly, uint32 nPolyAnimIndex, CLightDef** pLightList, uint32 nMaxListSize)
{
	uint32 nNumLights = 0;

	//make a list of all the lights that touch this polygon.
    for (uint32 nCurrLight = 0; nCurrLight < m_LightDefs; nCurrLight++) {
        //get the light.
        CLightDef *pLight = m_LightDefs[nCurrLight];

		//make sure that this light is supposed to light the world
		if(!pLight->m_bLightWorld)
		{
			continue;
		}

        //check if it hits the poly
        if (LightTouchesPoly(pLight, pPoly, nPolyAnimIndex)) {
            //it hits, add it to the list.
            pLightList[nNumLights] = pLight;

			nNumLights++;

			//see if we have to bail
			if(nNumLights >= nMaxListSize)
			{
				//Note: Should probably inform the user with some warning
				break;
			}
        }
    }

	//tell them how many hits
	return nNumLights;
}

//calculates the lighting for a directional point in space
PVector CLightMapMaker::LightDirectionalPoint(	const PVector& vPos, const PVector& vNormal,
												bool bApplySunLight, bool bClipLight, CLightBSPStack* pStack) const
{
	PVector vColor(0, 0, 0);

	//the position we will be taking the sample from
	PVector vSamplePos = vPos + vNormal * g_pGlobs->m_fLMSampleExtrude;

	//make a list of all the lights that touch this polygon.
    for (uint32 nCurrLight = 0; nCurrLight < m_LightDefs; nCurrLight++) {
        //get the light.
        CLightDef *pLight = m_LightDefs[nCurrLight];

		//make sure that this light is supposed to light the world
		if(!pLight->m_bLightWorld)
		{
			continue;
		}

		// Don't light back-facing polys
		LTVector vPtToLight = pLight->m_Pos - vPos;
		if (vPtToLight.Dot(vNormal) <= 0.0f)
			continue;

        //check if it hits the point
        if (DoesLightHit(pLight, vSamplePos, bClipLight, pStack)) 
		{
			//calculate the contribution of the light
			vColor += CalcLightSample(
							pLight,
							pLight->m_Pos,
							pLight->m_Direction,
							vPos, 
							vNormal,
							vPtToLight.MagSqr());
        }
    }
	//also need to see if the sunlight can hit this point
	if(bApplySunLight)
	{
		if(DoesSunlightHit(vSamplePos, pStack))
		{
			//we hit a sky portal
			vColor += CalcSunLightSample(vNormal);
		}
	}

	return vColor;
}

float GetAveragePolyDist(const LTVector &vPos, const CPrePoly *pPoly)
{
	if (pPoly->NumVerts() == 0)
		return 0.0f;

	float fMin = (vPos - pPoly->Pt(0)).Mag();
	float fMax = fMin;
	for (uint32 nCurPt = 1; nCurPt < pPoly->NumVerts(); ++nCurPt)
	{
		float fDist = (vPos - pPoly->Pt(nCurPt)).Mag();
		fMin = LTMIN(fMin, fDist);
		fMax = LTMAX(fMax, fDist);
	}

	return (fMin + fMax) / 2;
}

float GetPolyArea(const CPrePoly *pPoly)
{
	if (pPoly->NumVerts() < 3)
		return 0.0f;

	float fResult = 0.0f;

	for (uint32 nCurPt = 1; (nCurPt + 1) < pPoly->NumVerts(); ++nCurPt)
	{
		LTVector vEdge0 = pPoly->Pt(nCurPt) - pPoly->Pt(0);
		LTVector vEdge1 = pPoly->Pt(nCurPt + 1) - pPoly->Pt(0);
		fResult += vEdge1.Cross(vEdge0).Mag();
	}

	return fResult * 0.5f;
}

LTVector CLightMapMaker::CalcGridPointAmbient(const LTVector &vPos)
{
	// Find the closest polygon (well, roughly anyway...)
	const CPrePoly *pClosestPoly = 0;
	float fClosestDist = m_cPolyTree_World.GetClosestPoly(vPos, &pClosestPoly);
	if (!pClosestPoly)
		return LTVector(0.0f, 0.0f, 0.0f);

	// If not doing full ambient calc, use that ambient value
	if (!g_pGlobs->m_bVolumetricAmbient)
	{
		const uint8* pPolyAmbient = pClosestPoly->GetSurface()->m_Ambient;
		return LTVector(pPolyAmbient[0], pPolyAmbient[1], pPolyAmbient[2]);
	}

	// Guess on a good distance to select polygons from
		// (Allow user control, default to 2.0)
		// NYI
	float fAmbientVolume = fClosestDist * 2.0f;

	float fTotalArea = 0.0f;
	LTVector vTotalAmbient(0.0f, 0.0f, 0.0f);

	// Find all the polygons in that radius
	const float k_fBackfacingPolyEpsilon = 0.01f;
	for (uint32 nFindPolys = 0; nFindPolys < m_Polies.GetSize(); ++nFindPolys)
	{
		const CPrePoly *pCurPoly = m_Polies[nFindPolys];
		float fPolyDist = (vPos - pCurPoly->GetCenter()).Mag() - pCurPoly->GetRadius();
		float fPlaneDist = pCurPoly->GetPlane()->DistTo(vPos);
		if ((fPolyDist > fAmbientVolume) || (fPlaneDist < k_fBackfacingPolyEpsilon))
			continue;

		float fAvePolyDist = GetAveragePolyDist(vPos, pCurPoly);
		float fPolyArea = GetPolyArea(pCurPoly) / fAvePolyDist;
		const uint8 *pPolyAmbient = pCurPoly->GetSurface()->m_Ambient;
		fTotalArea += fPolyArea;
		vTotalAmbient += LTVector(pPolyAmbient[0], pPolyAmbient[1], pPolyAmbient[2]) * fPolyArea;
	}

	// Return the result
	if (fTotalArea == 0.0f)
		return LTVector(0.0f, 0.0f, 0.0f);
	
	return vTotalAmbient / fTotalArea;
}

//given a point in space it will calculate the light that hits it
PVector CLightMapMaker::LightGridPoint( const PVector& vPos, bool bClipLight, bool bAmbient, CLightBSPStack* pStack)
{
	PVector vColor(0.0f, 0.0f, 0.0f);

	if (bAmbient)
		vColor += m_AmbientLight + CalcGridPointAmbient(vPos);
	
	//make a list of all the lights that touch this polygon.
    for (uint32 nCurrLight = 0; nCurrLight < m_LightDefs; nCurrLight++) 
	{
        //get the light.
        CLightDef *pLight = m_LightDefs[nCurrLight];

		//make sure that this light lights the grid
		if(!pLight->m_bLightGrid)
		{
			//doesn't light grid, don't consider it
			continue;
		}
		
        //check if it hits the point
        if (DoesLightHit(pLight, vPos, bClipLight, pStack))
		{
			//calculate the contribution of the light
			LTVector vLight = CalcLightSample(
								pLight,
								pLight->m_Pos,
								pLight->m_Direction,
								vPos, 
								1.0f,					// Always consider this to be maximum intensity
								(pLight->m_Pos - vPos).MagSqr());

			//we need to scale this based upon the object lighting scale since this lighting
			//only effects objects
			vLight *= pLight->m_fObjectBrightScale;

			//add it to our final color
			vColor += vLight;
		}
	}
	return vColor;
}


//given a position, it will calculate the amount of light that strikes the position of a 
//point on a polygon. This is intended for use on polygons (so the light list can
//be cached across the surface)
PVector CLightMapMaker::LightDirectionalPoint(	const PVector& vIntersectPos, const PVector &vLightPos, CPrePoly* pPoly,
												bool bApplySunLight, bool bClipLight, 
												CLightDef** pLightList, uint32 nNumLights, CLightBSPStack* pStack)
{
	//the color contribution
	PVector vColor(0, 0, 0);

	//the position we will be taking the sample from
	PVector vSamplePos = vIntersectPos;
	
	// Force it to be on the plane of the polygon for the purposes of the intersection
	vSamplePos -= pPoly->Normal() * pPoly->GetPlane()->DistTo(vSamplePos);
	// Adjust using the LMSampleExtrude value
	vSamplePos += pPoly->Normal() * g_pGlobs->m_fLMSampleExtrude;

	float fLightPenScale = pPoly->GetSurface()->m_fLightPenScale;

	//find the contribution to this point from all the lights
	for(uint32 nCurrLight = 0; nCurrLight < nNumLights; nCurrLight++)
	{
		//the light we will be processing
		CLightDef* pLight = pLightList[nCurrLight];

		LTVector vAdjSamplePos = vSamplePos;

		if (fLightPenScale > 0.001f)
		{
			LTVector vPtToLight = pLight->m_Pos - vSamplePos;
			float fPtToLightMag = vPtToLight.Mag();
			vPtToLight /= fPtToLightMag;
			if (fLightPenScale < fPtToLightMag)
				vAdjSamplePos += vPtToLight * fLightPenScale;
		}

		if(DoesLightHit(pLight, vAdjSamplePos, bClipLight, pStack))
		{

			//calculate the contribution of the light
			vColor += CalcLightSample(
							pLight,
							pLight->m_Pos,
							pLight->m_Direction,
							vLightPos,
							pPoly->Normal(),
							(pLight->m_Pos - vLightPos).MagSqr());
		}
		
	}//end for light

	//also need to see if the sunlight can hit this point
	if(bApplySunLight)
	{
		if(DoesSunlightHit(vSamplePos - m_SunLightDir * fLightPenScale, pStack))
		{
			//we hit a sky portal
			vColor += CalcSunLightSample(pPoly->Normal());
		}
	}

	return vColor;
}

//this will determine if a light hits the sample point specified
bool CLightMapMaker::DoesLightHit(const CLightDef* pLight, const PVector& vPos, bool bClipLight, CLightBSPStack* pStack) const
{
	LTVector vToLight = pLight->m_Pos - vPos;

	//distance cull the light
	float fOriginalDist = vToLight.MagSqr();
	if(fOriginalDist > pLight->m_MaxDistSqr)
	{
		return false;
	}

	//if this is a directional light, we can also do FOV culling here
	if(pLight->m_bDirectional)
	{
		//we can dot the lights direction with the direction to
		//the point, and do a test
		vToLight.Normalize();

		if(pLight->m_Direction.Dot(-vToLight) < pLight->GetCosFOV())
		{
			return false;
		}
	}


	//see if we are clipping the light or not
	if(bClipLight && pLight->m_bClip)
	{
		
		if(m_LightingBSP.IntersectSegment(vPos, pLight->m_Pos, pStack, NULL))
			return false;
	}

	return true;
}

// Does sunlight fall on this point?
bool CLightMapMaker::DoesSunlightHit( const PVector& vPos, CLightBSPStack* pStack) const
{
	CIntersectInfo Info;
	if(!m_LightingBSP.IntersectSegment(vPos, vPos - m_SunLightDir * m_fSunRayLength, pStack, &Info))
		return false;

	return (Info.m_nFlags & LMPOLY_SKY) != 0;
}

//runs through the list of samples, and adds the color to the output color
//based upon a weighting formula
static void GetContribution(LTVector* pvColors, LTVector* pvSamples, 
							uint32 nNumSamples, float fXOff, float fYOff,
							LTVector& vColor, float& fTotalWeight)
{
	for(uint32 nCurrSample = 0; nCurrSample < nNumSamples; nCurrSample++)
	{
		//get the position of this sample point
		float fXPos = fabsf(pvSamples[nCurrSample].x + fXOff);
		float fYPos = fabsf(pvSamples[nCurrSample].y + fYOff);

		//calculate the weight of this point
		float fWeight = (1 - LTMAX(fXPos, fYPos));
		fWeight = pvSamples[nCurrSample].z;
		
		//factor in the color contribution
		vColor += pvColors[nCurrSample] * fWeight;
		fTotalWeight += fWeight;
	}
}

bool CLightMapMaker::FinishLightMap(uint32 nPolyIndex, CFullLightMap* pLightMap)
{
	CFullLightMap DownSampledMap;
	
	if(m_bOverSample)
	{
		DownSample(pLightMap, &DownSampledMap);
		pLightMap = &DownSampledMap;
	}

	//clamp the light values.
    ClampMapValues(pLightMap, pLightMap->m_Width, pLightMap->m_Height);

    //get the output lightmap.
    CPreLightMap *pOutputLM = m_pCurFrame->m_PolyMaps[nPolyIndex];

    //make our output lightmap.
    ConvertMap(pLightMap, pOutputLM);

	return true;
}

bool CLightMapMaker::ThreadLightPolySuperSample(uint32 nPolyIndex, uint32 nNumSamples, 
													LTVector* pvSampleList, CLightBSPStack* pStack)
{
	
	//get the polygon we are going to be lighting
	CPrePoly *pPoly = m_pWorld->GetLMPoly(&m_pCurAnim->m_Polies[nPolyIndex]);

    if (pPoly == NULL) 
	{
		ASSERT(false);
        return false;
    }

	//make sure that we should lightmap this polgon
	if(!ShouldLightmap(pPoly))
	{
		//done
		return true;
	}

	//get the temporary working lightmap
    CFullLightMap *pLightMap = m_FullLightMaps[nPolyIndex];

	//initialize the lightmap
	InitPolygonFullLightMap(pPoly, pLightMap);

	//if we don't want the polygon to receive light, we are done here
	if(!(pPoly->GetSurfaceFlags() & SURF_RECEIVELIGHT))
	{
		return FinishLightMap(nPolyIndex, pLightMap);
	}

	//calculate the width and height of the lightmap
	int32 nLMWidth  = pPoly->m_LMWidth;
	int32 nLMHeight = pPoly->m_LMHeight;

	if(m_bOverSample)
	{
		nLMWidth++;
		nLMHeight++;
	}

	//we need to allocate room for the sample buffer. This will need to have room for
	//an entire row above the current pixel, and one extra for the pixel that came
	//just before the one being processed, and one more for the current pixel
	//being processed. There are two buffers, one to tell if the
	//sample was inside or outside the world, and another to tell the color
	//SB = Sample Buffer
	LTVector*	pvSBColor	= new LTVector[nNumSamples * (nLMWidth + 3)];

	//figure out indices for where each subsample should be calculated
	uint32 nCurrentSample	= nNumSamples * (nLMWidth + 2);
	uint32 nPrevSample		= nNumSamples * (nLMWidth + 1);

	//make sure the allocation succeeded
	if(pvSBColor == NULL)
	{
		return false;
	}

	//determine if we need to apply sunlight to this polygon
	bool bApplySunLight = false;

	if(	m_bStaticSunLight && 
		(pPoly->Normal().Dot(m_SunLightDir) <= 0.0f) && 
		(pPoly->GetSurfaceFlags() & SURF_RECEIVESUNLIGHT))
	{
		bApplySunLight = true;
	}

	//determine if we want to clip the light
	bool bClipLight = (pPoly->GetSurfaceFlags() & SURF_RECEIVESHADOWS) && m_bClipLight;


	//the list of lights
	CLightDef*	pLightList[MAX_LIGHTS_PER_POLYGON];
	uint32		nNumLights = CalcLightsTouchingPoly(pPoly, nPolyIndex, pLightList, MAX_LIGHTS_PER_POLYGON);	

	if((nNumLights == 0) && !bApplySunLight)
	{
		return FinishLightMap(nPolyIndex, pLightMap);
	}

	uint32 nGridSize = m_pWorld->GetLMGridSize(pPoly);

	//the increments per lm texel in each direction
	LTVector vColInc = pPoly->InverseP() * (float)nGridSize;
	LTVector vRowInc = pPoly->InverseQ() * (float)nGridSize;

	//determine if we need to shift the lightmap for supersampling
	PReal fShift = GetShiftAmount(pPoly);

	//the starting position for each row
	LTVector vRowStart =	pPoly->PolyO() + 
							pPoly->InverseP() * fShift + 
							pPoly->InverseQ() * fShift;

	//we also need to back up our start point to compensate for the fact that we calc
	//extra samples above and to the left
	vRowStart = vRowStart - vColInc - vRowInc;

	//see if these need to be adjusted based upon a world model transform
	if(pPoly->m_pWorld->m_bUsingTransform)
	{
		// Transform into the world model's space.
		MatVMul_InPlace_3x3(&pPoly->m_pWorld->m_mTransform, &vColInc);
		MatVMul_InPlace_3x3(&pPoly->m_pWorld->m_mTransform, &vRowInc);
		vRowStart = pPoly->m_pWorld->m_mTransform * vRowStart;
	}

	// Pre-calc the poly edge planes
	static TPlaneList cEdgePlanes;
	cEdgePlanes.clear();
	static TPlaneCountList cEdgeCounts;
	cEdgeCounts.clear();

	CalcPolyEdgePlanes(pPoly, &cEdgePlanes, &cEdgeCounts);

	//the current lightmap pixel that is being set
	LTVector* pCurrTexel = pLightMap->m_Map.GetArray(); 

	//now we run through every pixel and light it
	for(int32 nCurrY = -1; nCurrY < nLMHeight; nCurrY++)
	{
		//reset the current position
		LTVector vCurrPos = vRowStart;

		for(int32 nCurrX = -1; nCurrX < nLMWidth; nCurrX++)
		{
			//now we need to run through every sample
			for(uint32 nCurrSub = 0; nCurrSub < nNumSamples; nCurrSub++)
			{
				//find the point we will be sampling
				LTVector vSubPos = vCurrPos +	vColInc * pvSampleList[nCurrSub].x + 
												vRowInc * pvSampleList[nCurrSub].y;

				//now we need to move the point onto the plane of the polygon
				vSubPos -= pPoly->Normal() * DIST_TO_PLANE(vSubPos, *(pPoly->GetPlane()));

				//now we need to get the contribution
				pvSBColor[nCurrentSample + nCurrSub] = 
						LightDirectionalPoint(	MovePointIntoPoly(vSubPos, cEdgePlanes, cEdgeCounts), vSubPos,
												pPoly, bApplySunLight, 
												bClipLight, 
												pLightList, nNumLights, pStack); 
				
			}//end for subsample

			//see if we need to modify the color
			if((nCurrX >= 0) && (nCurrY >= 0))
			{
				//init the actual pixel color
				LTVector vColor(0, 0, 0);

				//find the weight of the color
				float fTotalWeight = 0.0f;
				
				//now we run through the 4 quadrants and add up all the light
				//and factor in its weights

				//add the box to the upper left
				uint32 nSample = nCurrX * nNumSamples;
				GetContribution(&(pvSBColor[nSample]), pvSampleList, nNumSamples, -1, -1, vColor, fTotalWeight);
				
				//add the box above
				nSample = (nCurrX + 1) * nNumSamples;
				GetContribution(&(pvSBColor[nSample]), pvSampleList, nNumSamples, 0, -1, vColor, fTotalWeight);
				
				//add in the box to the left
				GetContribution(&(pvSBColor[nPrevSample]), pvSampleList, nNumSamples, -1, 0, vColor, fTotalWeight);
			
				//now we add up the current square
				GetContribution(&(pvSBColor[nCurrentSample]), pvSampleList, nNumSamples, 0, 0, vColor, fTotalWeight);

				//now we need to normalize the light
				if(fTotalWeight > 0.0f)
				{
					vColor /= fTotalWeight;
				}

				//now we can set the lm texel 
				*pCurrTexel += vColor;

				//move onto the next texel
				pCurrTexel++;

			}

			//now copy the previous square into the buffer
			if(nCurrX >= 0)
			{	
				for(uint32 nSample = 0; nSample < nNumSamples; nSample++)
				{
					pvSBColor[nCurrX * nNumSamples + nSample] =
						pvSBColor[nPrevSample + nSample];
				}
			}

			//now we swap the buffers that are used for current and previous
			uint32 nTemp	= nCurrentSample;
			nCurrentSample	= nPrevSample;
			nPrevSample		= nTemp;

			//update where the next texel will be located
			vCurrPos += vColInc;

		}//end for X

		//now copy the previous square into the buffer
		if(nCurrX >= 0)
		{	
			for(uint32 nSample = 0; nSample < nNumSamples; nSample++)
			{
				pvSBColor[nCurrX * nNumSamples + nSample] =
					pvSBColor[nCurrentSample + nSample];
			}
		}

		//update where the next position will be
		vRowStart += vRowInc;

	}//end for Y

	// Convert..
	FinishLightMap(nPolyIndex, pLightMap);

	//clean up the sample buffers
	delete [] pvSBColor;

    //make the lightmap correctly.
    return true;
}



