// ------------------------------------------------------ //
// Processing.cpp - does all the main processing for 
// the preprocessor.
// ------------------------------------------------------ //

#include "bdefs.h"
#include <stdarg.h>

#include "bspgen.h"

#include "lightmapmaker.h"

#include "editpoly.h"
#include "editregion.h"
#include "brushtoworld.h"
#include "findworldmodel.h"
#include "processing.h"
#include "threads.h"
#include "replacetextures.h"
#include "gettextureflags.h"
#include "node_ops.h"
#include "pregeometry.h"
#include "create_world_tree.h"
#include "parse_world_info.h"
#include "createphysicsbsp.h"
#include "splitpoly.h"
#include "lightmapdefs.h"
#include "ltamgr.h"
#include "packerfactory.h"
#include "createdecals.h"
#include "createpolyedges.h"
#include "ApplyAmbientOverride.h"
#include "convertkeydata.h"
#include "convertscatter.h"
#include "fillingroupobjects.h"
#include "CenterWorldAroundOrigin.h"
#include "ApplyRenderGroups.h"
#include "ipackeroutput.h"
#include <float.h>

//number of plane lists to use for finding coplanar planes' hash table
#define NUM_PLANE_LISTS		128

CEditRegion					*g_InputRegion = NULL;
CStringHolder				g_StringHolder;

FILE						*g_pLogFile = NULL;
FILE						*g_pErrorLogFile = NULL;

static char					g_InputFileTitle[MAX_PATH];
CProcessorGlobs				*g_pGlobs = NULL;
bool					g_bInFullVis = false;




//---------------------Memory Allocation----------------------------//
void* dalloc(unsigned int size)
{
	void *pRet;
	
	pRet = malloc((size_t)size);
	if(!pRet)
		throw CLithMemException();

	return pRet;
}

void dfree(void *ptr)
{
	free(ptr);
}

void* dalloc_z(unsigned int size)
{
	void *pRet;
	
	pRet = malloc((size_t)size);
	if(!pRet)
		throw CLithMemException();
	
	memset(pRet, 0, size);
	return pRet;
}

// Hook Stdlith's base allocators.
void* DefStdlithAlloc(uint32 size)
{
	return malloc(size);
}

void DefStdlithFree(void *ptr)
{
	free(ptr);
}

void dsi_PrintToConsole(char *pMsg, ...)
{
}

void dsi_OnReturnError(int err)
{
}
//-----------------------------------------------------------------//

//given an argument list, this will build up a file name. This must be handled
//specifically in case there are spaces in the file name. nArgC should be the
//number of arguments that can possibly be in the file. Returns if the file
//was properly terminated with a quote or not
static bool BuildFileName(char** pArgV, uint32 nArgC, char* pszBuffer, uint32 nBuffLen)
{
	ASSERT(nArgC > 0);
	ASSERT(nBuffLen > 0);

	//clear out the buffer
	pszBuffer[0] = '\0';

	//see if our filename is just one argument
	if(pArgV[0][0] != '\"')
	{
		//it is just one arg. Copy it over and bail
		strncpy(pszBuffer, pArgV[0], nBuffLen);
		return true;
	}

	//copy over everything past the quote
	strncpy(pszBuffer, &(pArgV[0][1]), nBuffLen);

	//see if we ended on a quote
	if((strlen(pszBuffer) > 0) && (pszBuffer[strlen(pszBuffer) - 1] == '\"'))
	{
		//remove the quote and bail
		pszBuffer[strlen(pszBuffer) - 1] = '\0';
		return true;
	}

	//now add on the arguments until we find one with a quote on the end
	for(uint32 nCurrArg = 1; nCurrArg < nArgC; nCurrArg++)
	{
		//tack on the space
		strncat(pszBuffer, " ", nBuffLen);

		//see if we hit the end
		uint32 nArgLen = strlen(pArgV[nCurrArg]);
		if(nArgLen && (pArgV[nCurrArg][nArgLen - 1] == '\"'))
		{
			//take off the quote
			pArgV[nCurrArg][nArgLen - 1] = '\0';

			//we have found the end
			strncat(pszBuffer, pArgV[nCurrArg], nBuffLen);
			return true;
		}

		//not the end, just add this to it
		strncat(pszBuffer, pArgV[nCurrArg], nBuffLen);
	}

	//never found an ending quote
	return false;
}

void InitProcessOptions(CProcessorGlobs *pGlobs, const char* pszInputFile)
{
	ASSERT(pGlobs);

	pGlobs->m_fLMSampleExtrude			= (PReal)0.05;
	pGlobs->m_bLMOverSample				= false;
	pGlobs->m_bLMSuperSample			= false;
	pGlobs->m_bLightAnimations			= false;
	pGlobs->m_bIgnoreHidden				= false;
	pGlobs->m_bIgnoreFrozen				= false;
	pGlobs->m_bShadowMesh				= false;
	pGlobs->m_bVolumetricAmbient		= false;
	pGlobs->m_bObjectsOnly				= false;
	pGlobs->m_bPortals					= true;
	pGlobs->m_bCenterWorldAroundOrigin	= true;
	pGlobs->m_nLMNumSamplesOnSide		= 1;

	LTStrCpy(pGlobs->m_ProjectDir, ".", sizeof(pGlobs->m_ProjectDir));
	LTStrCpy(pGlobs->m_InputFile, pszInputFile, sizeof(pGlobs->m_InputFile));

	//set up the output name to match the input name, but change the extension to
	//dat
	uint32 nCurrInChar		= 0;
	uint32 nExtensionPos	= 0;

	//copy the string, and identify where the extension starts
	while(pGlobs->m_InputFile[nCurrInChar] != '\0')
	{
		pGlobs->m_OutputFile[nCurrInChar] = pGlobs->m_InputFile[nCurrInChar];

		//see if we hit the extension
		if(pGlobs->m_InputFile[nCurrInChar] == '.')
		{
			nExtensionPos = nCurrInChar;
		}
		nCurrInChar++;
	}
	
	//now switch the extension
	if(nCurrInChar > 0)
	{
		strcpy(&(pGlobs->m_OutputFile[nExtensionPos]), ".dat");
	}	
}

static void OpenLogFile(const char* pszPostFix, FILE*& OutFp, bool bDateTimeStamp)
{
	char	testFilename[MAX_PATH + 1];
	uint32	i;
	FILE	*fp;

	//default it to NULL
	OutFp = NULL;

	// Find a free file.
	for(i=0; i < 5000; i++)
	{
		LTSNPrintF(testFilename, sizeof(testFilename), "%s%s_%d.log", g_InputFileTitle, pszPostFix, i);
		fp = fopen(testFilename, "rt");
		if(!fp)
			break;
	}

	LTSNPrintF(testFilename, sizeof(testFilename), "%s%s_%d.log", g_InputFileTitle, pszPostFix, i);
	OutFp = fopen(testFilename, "wt");
	
	//write out a date/time stamp for this log
	if(OutFp && bDateTimeStamp)
	{
		char			timeString[128], dateString[128];

		thd_GetTimeString(timeString);
		thd_GetDateString(dateString);

		fprintf(OutFp, "Log file created on %s at %s\r\n", dateString, timeString);
	}
}


static void CloseLogFiles()
{
	if(g_pLogFile)
	{
		fclose(g_pLogFile);
		g_pLogFile = NULL;
	}

	if(g_pErrorLogFile)
	{
		fclose(g_pErrorLogFile);
		g_pErrorLogFile = NULL;
	}
}

void ActivateTask(const char* pszTask)
{
	//try activating the task. If it fails, we need to add it
	ASSERT(g_pGlobs);
	ASSERT(g_pGlobs->m_pIOutput);

	g_pGlobs->m_pIOutput->CreateTask(pszTask);
	g_pGlobs->m_pIOutput->ActivateTask(pszTask);
}

void ActivateSubTask(const char* pszSubTask)
{
	//try activating the task. If it fails, we need to add it
	ASSERT(g_pGlobs);
	ASSERT(g_pGlobs->m_pIOutput);

	g_pGlobs->m_pIOutput->ActivateSubTask(pszSubTask);
}

void SetProgressBar(float percent)
{
	ASSERT(g_pGlobs);
	ASSERT(g_pGlobs->m_pIOutput);

	g_pGlobs->m_pIOutput->UpdateProgress(percent * 100);
}

void InternalDrawStatusText(EStatusTextType eType, const char *pMsg, va_list marker)
{
	char msg[512];

	LTVSNPrintF(msg, sizeof(msg), pMsg, marker);

	//see if we need to append a prefix
	char pszFinalMessage[512];
	bool bToErrorLog = false;

	EMsgType eMsgType = MSG_NORMAL;

	switch(eType)
	{
	case eST_Error:
		LTSNPrintF(pszFinalMessage, sizeof(pszFinalMessage), "Error: %s", msg);
		eMsgType = MSG_ERROR;
		bToErrorLog = true;
		break;
	case eST_Warning:
		LTSNPrintF(pszFinalMessage, sizeof(pszFinalMessage), "Warning: %s", msg);
		eMsgType = MSG_WARNING;
		bToErrorLog = true;
		break;
	default:
		LTSNPrintF(pszFinalMessage, sizeof(pszFinalMessage), "%s", msg);
		eMsgType = MSG_NORMAL;
		bToErrorLog = false;
		break;
	}

	//write it out to the log file if it is open
	if(g_pLogFile)
	{
		fprintf(g_pLogFile, "%s\r\n", pszFinalMessage);
		fflush(g_pLogFile);
	}

	//now see if it needs to go out to an error log
	if(bToErrorLog)
	{
		//make sure that the file is open
		if(!g_pErrorLogFile)
		{
			OpenLogFile("Error", g_pErrorLogFile, true);
		}

		if(g_pErrorLogFile)
		{
			fprintf(g_pErrorLogFile, "%s\r\n", pszFinalMessage);
			fflush(g_pErrorLogFile);
		}
	}

	//and display it to the output
	g_pGlobs->m_pIOutput->LogMessage(eMsgType, pszFinalMessage);
}


void DrawStatusText(EStatusTextType eType, const char *pMsg, ...)
{
	va_list marker;

	va_start(marker, pMsg);
	InternalDrawStatusText(eType, pMsg, marker);
	va_end(marker);
}

void DrawStatusTextIfMainWorld(EStatusTextType eType, const char *pMsg, ...)
{
	va_list marker;

	if(g_pGlobs->m_bMakingWorldModel)
		return;

	va_start(marker, pMsg);
	InternalDrawStatusText(eType, pMsg, marker);
	va_end(marker);
}

//given a node, it will look at the processor settings and determine if it should be ignored
bool ShouldIgnoreNode(const CWorldNode* pNode)
{
	//determine if this is hidden and we are ignoring hidden
	if(g_pGlobs->m_bIgnoreHidden && pNode->IsFlagSet(NODEFLAG_HIDDEN))
		return true;

	//now check for ignoring frozen
	if(g_pGlobs->m_bIgnoreFrozen && pNode->IsFlagSet(NODEFLAG_FROZEN))
		return true;

	//we can handle it
	return false;
}

//recursively parses through the node tree looking for light groups and lights. Upon finding
//a light, it will fill in which light group it belongs to
static uint32 FillInLightGroupFields(CWorldNode* pNode, const char* pszLightGroupName)
{
	//sanity check
	if(!pNode)
		return 0;

	const char* pszFinalName = pszLightGroupName;

	uint32 nNumLightsUnder = 0;

	//see if this is an object...
	if(!ShouldIgnoreNode(pNode) && (pNode->GetType() == Node_Object))
	{
		//we have an object, check the class
		if(stricmp(pNode->GetClassName(), "LightGroup") == 0)
		{
			//see if we have any light groups within another light group
			if(pszLightGroupName)
			{
				DrawStatusText(eST_Warning, "Found lightgroup %s within lightgroup %s", pNode->GetName(), pszLightGroupName);
			}

			//this is a light group, we want to pass its name to our children
			pszFinalName = pNode->GetName();
		}
		else if(pszLightGroupName)
		{
			if(	(stricmp(pNode->GetClassName(), "Light") == 0) ||
						(stricmp(pNode->GetClassName(), "ObjectLight") == 0) ||
						(stricmp(pNode->GetClassName(), "DirLight") == 0) )
			{
				//this is a light, so we need to fill out the light group property
				CBaseProp* pProp = pNode->GetPropertyList()->GetProp("LightGroup");

				if(pProp)
				{
					if(pProp->GetType() == LT_PT_STRING)
					{
						//we have a valid property, copy the light group name into it
						strcpy(((CStringProp*)pProp)->m_String, pszFinalName);
					}
					else
					{
						DrawStatusText(eST_Error, "Found a property LightGroup that needs to be a string!");
					}
				}
				else
				{
					//we need to add this property
					CStringProp* pProp = new CStringProp("LightGroup");

					if(pProp)
					{
						strcpy(pProp->m_String, pszFinalName);
						pNode->GetPropertyList()->m_Props.Append(pProp);
					}

				}

				nNumLightsUnder++;
			}
		}
	}

	//now we need to recurse into all of our children filling out these properties
	for(GPOS pos = pNode->m_Children; pos; )
	{
		CWorldNode* pChild =  pNode->m_Children.GetNext(pos);
		nNumLightsUnder += FillInLightGroupFields(pChild, pszFinalName);
	}

	//see if we have an empty light group
	if(pszFinalName != pszLightGroupName)
	{
		//this was a lightgroup
		if(nNumLightsUnder == 0)
		{
			DrawStatusText(eST_Warning, "Found empty lightgroup %s", pszFinalName);
		}

		//we don't want to inform parents of our lights
		return 0;
	}
	else
	{
		//report how many lights are under this node
		return nNumLightsUnder;
	}
}

static void DisplayPrefabInstantiationError(const char *pMsg)
{
	DrawStatusText(eST_Error, "%s", pMsg);
}


static void RecurseAndDisconnectPrefabs(CEditRegion *pRegion, CWorldNode *pRoot)
{
	// Try to disconnect this node
	if (pRoot->GetType() == Node_PrefabRef)
	{
		CPrefabRef *pPrefabRoot = (CPrefabRef*)pRoot;

		if (!ShouldIgnoreNode(pRoot))
		{
			CWorldNode *pNewRoot = pPrefabRoot->InstantiatePrefab(pRegion, DisplayPrefabInstantiationError);
			if (!pNewRoot)
				DrawStatusText(eST_Error, "Unable to instantiate prefab %s (%s)", pPrefabRoot->GetName(), pPrefabRoot->GetPrefabFilename());
		}				
	}

	// Disconnect any children that might need to be disconnected
	GPOS iCurChild = pRoot->m_Children.GetHeadPosition();
	while (iCurChild)
	{
		CWorldNode *pChild = pRoot->m_Children.GetNext(iCurChild);
		RecurseAndDisconnectPrefabs(pRegion, pChild);
	}

	//now if it was a prefab, we need to delete that node
	if (pRoot->GetType() == Node_PrefabRef)
	{
		no_DestroyNode(pRegion, pRoot, FALSE);
	}
}

static bool LoadInputRegionLTA(LTVector& vWorldOffset)
{
	uint32 nVersion;
	bool bBinary = false;

	//time the loading of the LTA file
	clock_t nStartTime = clock();
	RegionLoadStatus status = g_InputRegion->LoadFile( g_pGlobs->m_InputFile, NULL, nVersion, bBinary );
	clock_t nEndTime = clock();

	if(status == REGIONLOAD_INVALIDFILE)
	{
		DrawStatusText(eST_Error, "The file is corrupt (error code %d).", g_InputRegion->m_LoadErrorCode);
		return false;
	}
	
	//display the time needed to load the LTA file
	DrawStatusText(eST_Normal, "LTA file loaded in %.2f seconds", 
					(nEndTime - nStartTime) / (float)CLOCKS_PER_SEC);

	//bind all prefabs
	g_InputRegion->GetPrefabMgr()->SetRootPath(g_pGlobs->m_ProjectDir);
	g_InputRegion->GetPrefabMgr()->BindAllRefs(g_InputRegion->GetRootNode());

	//update our version
	bool bModified;

	if(!bBinary)
	g_InputRegion->PostLoadUpdateVersion(nVersion, bModified);

	//disconnect all prefabs
	DrawStatusText(eST_Normal, "Instantiating prefabs");
	RecurseAndDisconnectPrefabs(g_InputRegion, g_InputRegion->GetRootNode());

	//move the entire world so that it is centered around the origin, this greatly
	//helps our accuracy on most levels

	if(g_pGlobs->m_bCenterWorldAroundOrigin)
	{
		//see if we need to center the world
		DrawStatusText(eST_Normal, "Centering world around origin");
		CenterWorldAroundOrigin(*g_InputRegion, vWorldOffset);
		DrawStatusText(eST_Normal, "World offset by (%.2f %.2f %.2f)", vWorldOffset.x, vWorldOffset.y, vWorldOffset.z);
	}
	else
	{
		vWorldOffset.Init();
		DrawStatusText(eST_Warning, "Not offsetting world. This is only recommended for engineering and should not be used for other purposes");
	}

		//create any edges that are applicable
		DrawStatusText(eST_Normal, "Creating polygon edging");
		CreatePolyEdges(g_InputRegion);

		//removes all decal objects and if we are generating geometry, will create the
		//appropriate world model geometry
		DrawStatusText(eST_Normal, "Creating decal geometry");
		CreateDecals(g_InputRegion, !g_pGlobs->m_bObjectsOnly);

		//now we need to provide the oppertunity for overriding ambient on certain objects
		DrawStatusText(eST_Normal, "Applying ambient overrides");
		ApplyAmbientOverride(g_InputRegion);

		//fill in any group objects with the objects that they contain
		DrawStatusText(eST_Normal, "Filling in group objects");
		FillInGroupObjects(g_InputRegion);

	//have all the lights in the level figure out what light group they are in
	DrawStatusText(eST_Normal, "Filling in light group fields");
	FillInLightGroupFields(g_InputRegion->GetRootNode(), NULL);

		//apply the render group to objects under the render group objects
		DrawStatusText(eST_Normal, "Applying render groups");
		ApplyRenderGroups(g_InputRegion);

	uint32			i;
	LTVector		vPos;
	
		DrawStatusText(eST_Normal, "Removing hidden and frozen objects");
	for(i=0; i < g_InputRegion->m_Objects; i++)
		{
			CBaseEditObj* pObject = g_InputRegion->m_Objects[i];

			// Delete hidden objects
			if (ShouldIgnoreNode(pObject))
			{
				no_DestroyNode(g_InputRegion, pObject, FALSE);
				--i;
			}		
		}
	
	g_pGlobs->m_nInputPolies = g_InputRegion->GetTotalNumPolies();
	g_pGlobs->m_nInputVertices = g_InputRegion->GetTotalNumPoints();
	g_pGlobs->m_nInputObjects = g_InputRegion->m_Objects.GetSize();
	
	g_InputRegion->ClearSelections();

	return true;  //! for now
}

bool MakeBsp(CPreWorld *pWorld)
{
	CBspGen			bspGen;
	uint32			i;
	GPOS			pos;
	CPrePoly		*pPoly;
	CBspGenOptions	options;
	bool			bRet;

	options.m_pWorld			= pWorld;
	options.m_nThreadsToUse		= g_pGlobs->m_nThreads;

	bRet = bspGen.GenerateBspTree(&options);
	

	// Make all polies full brightness, so they're not required to light it.
	for(pos=pWorld->m_Polies; pos; )
	{
		pPoly = pWorld->m_Polies.GetNext(pos);
		
		for(i=0; i < pPoly->NumVerts(); i++)
		{
			pPoly->Color(i).Init(255.0f, 255.0f, 255.0f);
		}
	}

	return bRet;	
}

// ------------------------------------------------------------------------- //
// Sets surface flags, gotten from the property lists.
// ------------------------------------------------------------------------- //

inline uint32 GetHashCode(PVector &normal, PReal &dist)
{
	return	(uint32)((double)normal.x * 10000.0f) +
			(uint32)((double)normal.y * 10000.0f) +
			(uint32)((double)normal.z * 10000.0f) +
			(uint32)((double)dist * 10000.0f);
}


// Finds a plane in the list that matches the specifications you pass in.
// The actual floating point values must match exactly for a plane to be returned.
CPrePlane* FindPlaneExact(CLinkedList<CPrePlane*> *pList, PVector &normal, PReal dist)
{
	LPOS		pos;
	CPrePlane	*pTestPlane, *pBestPlane;
	double		minDist;


	minDist = 10000000.0f;
	pBestPlane = NULL;
	for(pos=pList->GetHeadPosition(); pos; )
	{
		pTestPlane = pList->GetNext(pos);
		
		if(pTestPlane->m_Normal.x == normal.x &&
			pTestPlane->m_Normal.y == normal.y &&
			pTestPlane->m_Normal.z == normal.z && 
			pTestPlane->m_Dist == dist)
		{
			return pTestPlane;
		}
	}

	return NULL;
}


void GenerateSurfaces(GenList<CEditBrush*> &brushes, CPreWorld *pWorld)
{
	uint32		j;
	CEditBrush	*pBrush;
	CEditPoly	*pPoly;
	CPreSurface	*pSurface;
	CStringProp *pProp;
	CColorProp	*pColorProp;
	char		baseTextureName[512];
	PVector		normal;
	uint32		iList;
	PReal		dist;
	GenListPos	pos;

	CLinkedList<CPrePlane*> planeLists[NUM_PLANE_LISTS];


	for(pos=brushes.GenBegin(); brushes.GenIsValid(pos); )
	{
		pBrush = brushes.GenGetNext(pos);

		for(j=0; j < pBrush->m_Polies; j++)
		{
			pPoly = pBrush->m_Polies[j];

			// Setup the surface.
			pSurface = new CPreSurface;

			uint32 nCurrTex;
			for(nCurrTex = 0; nCurrTex < CPreSurface::NUM_TEXTURES; nCurrTex++)
			{
				pSurface->m_Texture[nCurrTex].m_TextureO = pPoly->GetTexture(nCurrTex).GetO();
				pSurface->m_Texture[nCurrTex].m_TextureP = pPoly->GetTexture(nCurrTex).GetP();
				pSurface->m_Texture[nCurrTex].m_TextureQ = pPoly->GetTexture(nCurrTex).GetQ();
				pSurface->m_Texture[nCurrTex].m_pTextureName = pPoly->GetTexture(nCurrTex).m_pTextureName;
			}

			//now we need to make sure that all the textures come before the NULL, this implementation
			//only works with 2 textures
			assert((CPreSurface::NUM_TEXTURES == 2) && "This implementation needs to be expanded for the specified number of textures");
			if(pSurface->m_Texture[1].IsValid() && !pSurface->m_Texture[0].IsValid())
			{
				//we need to move it into the first channel
				std::swap(pSurface->m_Texture[0].m_TextureO, pSurface->m_Texture[1].m_TextureO);
				std::swap(pSurface->m_Texture[0].m_TextureP, pSurface->m_Texture[1].m_TextureP);
				std::swap(pSurface->m_Texture[0].m_TextureQ, pSurface->m_Texture[1].m_TextureQ);
				std::swap(pSurface->m_Texture[0].m_pTextureName, pSurface->m_Texture[1].m_pTextureName);
			}

			normal = pPoly->m_Plane.m_Normal;
			dist = (PReal)pPoly->m_Plane.m_Dist;

			// (Using the hash table makes this about 40 times faster for large levels).
			// NOTE: it still only accepts EXACT duplicate planes so it doesn't lose 
			// precision here.. the hash table is only for speed (at one point, it didn't
			// always use exact duplicates and some levels with rotated brushes had tons
			// of problems).
			iList = GetHashCode(normal, dist) % NUM_PLANE_LISTS;
			pSurface->m_pPlane = FindPlaneExact(&planeLists[iList], normal, dist);
			if(!pSurface->m_pPlane)
			{
				pSurface->m_pPlane = pWorld->AddPlane(
					pPoly->m_Plane.m_Normal, pPoly->m_Plane.m_Dist);
				
				planeLists[iList].AddTail(pSurface->m_pPlane);
			}

			pSurface->m_Flags = GetBrushType(pBrush);
			
			// Hack Mc. Hackelson.. force SURF_INVISIBLE if using a certain texture name..
			for(nCurrTex = 0; nCurrTex < CPreSurface::NUM_TEXTURES; nCurrTex++)
			{
				if(pPoly->GetTexture(nCurrTex).m_pTextureName)
				{
					CHelpers::ExtractNames(pPoly->GetTexture(nCurrTex).m_pTextureName, NULL, baseTextureName, NULL, NULL);
					
					if(stricmp(baseTextureName, "invisible.dtx") == 0)
					{
						pSurface->m_Flags |= SURF_INVISIBLE;

						//however, if this is invisible, make sure that it isn't lightmapped
						pSurface->m_Flags &= ~SURF_LIGHTMAP;
					}
				}
			}
			
			if((pProp = (CStringProp*)pBrush->m_PropList.GetProp("TextureEffect")) && (pProp->m_Type == PT_STRING) && (pProp->m_String[0] != 0))
			{
				pSurface->m_pTextureEffect = pWorld->m_StringHolder.AddString(pProp->m_String);
			}

			//copy over properties to the surface
			pSurface->m_LMGridSize	= GetLMGridSize(pBrush);
			if (!pSurface->m_LMGridSize)
				pSurface->m_LMGridSize = pWorld->GetDefaultLMGridSize();
			
			if(	(pColorProp = (CColorProp*)pBrush->m_PropList.GetProp("AmbientLight")) && 
				(pColorProp->m_Type == PT_COLOR))
			{
				pSurface->m_Ambient[0] = (uint8)(pColorProp->m_Vector.x);
				pSurface->m_Ambient[1] = (uint8)(pColorProp->m_Vector.y);
				pSurface->m_Ambient[2] = (uint8)(pColorProp->m_Vector.z);
			}

			CRealProp* pRealProp = (CRealProp*)pBrush->m_PropList.GetProp("LightPenScale");
			if(pRealProp && pRealProp->m_Type == PT_REAL)
			{
				pSurface->m_fLightPenScale = pRealProp->m_Value;
			}

			pPoly->m_pUser1 = pSurface;
			pSurface->m_pOriginalPoly = pPoly;

			pWorld->m_Surfaces.AddTail(pSurface);
		}
	}

	pWorld->SetupSurfaceTextureVectors();
}


static void GetWorldStats(CPreMainWorld *pMainWorld)
{
	CPreWorld	*pWorld;
	uint32		i;

	g_pGlobs->m_nOutputPolies		= 0;
	g_pGlobs->m_nOutputVertices		= 0;
	g_pGlobs->m_TreeDepth			= 0;
	g_pGlobs->m_LMDataSize			= pMainWorld->CalcLMDataSize();
	
	for(i=0; i < pMainWorld->m_WorldModels; i++)
	{
		pWorld = pMainWorld->m_WorldModels[i];

		if(pWorld->m_WorldInfoFlags & WIF_MAINWORLD)
		{
			g_pGlobs->m_nOutputPolies	+= pWorld->m_Polies;
			g_pGlobs->m_nOutputVertices += pWorld->m_nPointsSaved;
			g_pGlobs->m_TreeDepth		+= pWorld->GetTreeDepth();
		}
	}
}


static void ShowStatusText(DWORD startTime, DWORD endTime)
{
	DrawStatusText(eST_Normal,  "Done in %.2f minutes", ((endTime - startTime) / 1000.0f) / 60.0f );
	
	DrawStatusText(eST_Normal,  "" );
	
	DrawStatusText(eST_Normal,  "Number of input polies: %d", g_pGlobs->m_nInputPolies );
	DrawStatusText(eST_Normal,  "Number of input vertices: %d", g_pGlobs->m_nInputVertices );
	
	DrawStatusText(eST_Normal,  "" );
	
	DrawStatusText(eST_Normal,  "Number of output polies: %d", g_pGlobs->m_nOutputPolies );
	DrawStatusText(eST_Normal,  "Number of output vertices: %d", g_pGlobs->m_nOutputVertices );
	DrawStatusText(eST_Normal,  "Tree depth: %d", g_pGlobs->m_TreeDepth );
	
	DrawStatusText(eST_Normal,  "" );

	DrawStatusText(eST_Normal,  "Lightmap data size: %dk", (g_pGlobs->m_LMDataSize + 512) / 1024 );
	DrawStatusText(eST_Normal,  "Number of objects: %d", g_pGlobs->m_nInputObjects );

	DrawStatusText(eST_Normal,  "" );
}

static PReal GetMaxLMSize(const char *pInfoString)
{
	ConParse parse;
	uint32 size;

	parse.Init(pInfoString);
	if(parse.ParseFind("MaxLMSize", FALSE, 2))
	{
		size = atoi(parse.m_Args[1]);
		if(IsValidLightmapSize(size))
			return (PReal)size;
	}
	
	return DEFAULT_LIGHTMAP_PIXELS;
}


static PReal GetLMGridSize(const char *pInfoString)
{
	ConParse parse;

	parse.Init(pInfoString);
	if(parse.ParseFind("LMGridSize", FALSE, 2))
	{
		return (PReal)atoi(parse.m_Args[1]); // Needs to be an integer value.
	}
	
	return DEFAULT_LIGHTMAP_GRID_SIZE;
}


bool MergeBrushes(
	GenList<CEditBrush*> &brushes,
	CPreWorld *pWorld)
{
	DrawStatusTextIfMainWorld(eST_Normal, "Merging %d brushes.", brushes.GenGetSize());

	if(!BrushToWorld(brushes, pWorld))
	{
		return false;
	}

	return true;
}



static bool CreatePhysicsBSPs(
	CEditRegion		*pRegion,
	CPreMainWorld	*pMainWorld, 
	CWorldModelDef	*pWorldModel)
{
	DrawStatusText(eST_Normal, "Creating physics BSP");

	CPreWorld *pPhysicsBSP = new CPreWorld(pMainWorld);
	pPhysicsBSP->m_WorldInfoFlags = WIF_PHYSICSBSP | WIF_MAINWORLD;

	pMainWorld->m_WorldModels.GenAppend(pPhysicsBSP);
	SAFE_STRCPY(pPhysicsBSP->m_WorldName, "PhysicsBSP");

	if(!CreatePhysicsBSP(pRegion, pWorldModel, pPhysicsBSP, true))
		return false;

	return true;
}


// Creates the initial CPreMainWorld from the list of world models.
static bool ImportGeometry(
	CEditRegion *pRegion, 
	CMoArray<CWorldModelDef*> &worldModels, 
	CPreMainWorld *pMainWorld)
{
	CPreWorld		*pPreWorld;
	CWorldModelDef	*pWorldModel, *pMainWorldModel;
	PVector			min, max;
	uint32			i;


	if(worldModels.GetSize() == 0)
	{
		DrawStatusText(eST_Error, "Must have at least one brush!");
		return false;
	}

	// Create the world models.
	if(g_InputRegion->m_Objects.GetSize() != pMainWorld->m_Objects.GetSize())
	{
		DrawStatusText(eST_Error, "world object counts don't match!");
		return false;
	}

	// Create the visibility and physics BSPs.

	// Create CPreWorlds and generate surfaces for all the world models.  
	// The surfaces are needed so the polies have planes in CreateWorldTree callbacks.
	for(i=0; i < worldModels; i++)
	{
		pWorldModel = worldModels[i];

		// Create the world.
		pPreWorld = new CPreWorld(pMainWorld);
		pMainWorld->m_WorldModels.Append(pPreWorld);

		// Create the surfaces.
		GenerateSurfaces(pWorldModel->m_Brushes, pPreWorld);
	}


	ActivateSubTask("Creating World Tree");

	// Determine WorldTree subdivision (get bounding box of WIF_MAINWORLDs and filter
	// polies down to minimum size).
	if(!CreateWorldTree(&pMainWorld->m_WorldTree, worldModels, pMainWorld->m_pInfoString))
		return false;

	DrawStatusText(eST_Normal, "WorldTree nodes: %d", pMainWorld->m_WorldTree.m_nNodes);
	DrawStatusText(eST_Normal, "WorldTree depth: %d", pMainWorld->m_WorldTree.m_nDepth);
	DrawStatusText(eST_Normal, "WorldTree root size: %.2f", pMainWorld->m_WorldTree.m_RootNode.m_MinSize);

	
	// Don't include the main world model in the rest of this stuff.
	// (it gets setup after all the worldmodels).
	pMainWorldModel = worldModels[0];
	delete pMainWorld->m_WorldModels[0];
	pMainWorld->m_WorldModels.Remove(0);
	worldModels.Remove(0);

	char pszSubTaskBuffer[512];

	for(i=0; i < worldModels; i++)
	{
		pWorldModel = worldModels[i];
		pPreWorld = pMainWorld->m_WorldModels[i];


		// Copy info over.
		pPreWorld->m_WorldInfoFlags = pWorldModel->m_WorldInfoFlags;
		SAFE_STRCPY(pPreWorld->m_WorldName, pWorldModel->m_WorldName);

		g_pGlobs->m_bMakingWorldModel = !(pPreWorld->m_WorldInfoFlags & WIF_MAINWORLD);

		//setup our subtask
		LTSNPrintF(pszSubTaskBuffer, sizeof(pszSubTaskBuffer), "%s BSP", pWorldModel->m_WorldName);
		ActivateSubTask(pszSubTaskBuffer);

		if(!CreatePhysicsBSP(
				pRegion,
				pWorldModel,
				pPreWorld,
				false
				)
			)
		{
			return false;
		}
	}


	g_pGlobs->m_bMakingWorldModel = false;

	// Now create the main visibility and physics BSPs.
	// This must happen after the other WorldModels have been processed because it 
	// uses the terrain polies.
	if(!CreatePhysicsBSPs(pRegion, pMainWorld, pMainWorldModel))
		return false;

	// Get the texture flags.
	GetTextureFlags(pMainWorld);
	
	// Minimize the surface texture vectors (so certain hardware accelerators don't 
	// get flickering polies).
	pMainWorld->MinimizeSurfaceTCoords();
	
	GetWorldStats(pMainWorld);
	return true;
}

// removes the SURF_LIGHTMAP flag from all surfaces in this world
static bool ClearLightmapFlag(CPreMainWorld *pMainWorld)
{
	uint32		i;
	CPreWorld	*pWorld;
	GPOS		pos;
	CPreSurface *pSurface;

	if (!pMainWorld)
		return false;

	// for each worldmodel
	for (i = 0; i < pMainWorld->m_WorldModels; i++)
	{
		pWorld = pMainWorld->m_WorldModels[i];

		// for each surface
		for(pos=pWorld->m_Surfaces; pos; )
		{
			pSurface = pWorld->m_Surfaces.GetNext(pos);
			// clear the lightmap flag for this surface
			pSurface->m_Flags &= ~SURF_LIGHTMAP;
		}
	}
	return true;
}

//pack for a specific platform
static bool PackForPlatform(CPreMainWorld* pMainWorld, const char* pszPlatform)
{
	//first off, we need to get the name of the file that we are loading without the
	//extension

	char pszFilename[MAX_PATH];

	strncpy(pszFilename, g_pGlobs->m_OutputFile, MAX_PATH);

	//now pull out the extension
	for(int32 nCharPos = strlen(pszFilename) - 1; nCharPos >= 0; nCharPos--)
	{
		//see if we hit the end of the filename, and are moving into the path
		if((pszFilename[nCharPos] == '\\') || (pszFilename[nCharPos] == '/'))
			break;

		if(pszFilename[nCharPos] == '.')
		{
			pszFilename[nCharPos] = '\0';
			break;
		}
	}

	//now determine the platform we are packing for
	IWorldPacker* pPacker = CPackerFactory::Create(pszPlatform);

	if(pPacker == NULL)
	{
		DrawStatusText(eST_Error, "Unable to find a packer for the %s platform", pszPlatform);
		return false;
	}

	DrawStatusText(eST_Normal, "Packing world for the %s platform", pszPlatform);

	bool bSuccess = pPacker->PackWorld(pszFilename, pMainWorld, g_pGlobs->m_bObjectsOnly);

	if(bSuccess == false)
	{
		DrawStatusText(eST_Error, "An error occurred packing the file for the %s platform", pszPlatform);
	}

	delete pPacker;
	return bSuccess;
}

void DoProcessing(CProcessorGlobs *pGlobs)
{
	CPreMainWorld	world;
	uint32			endTime;
	char			timeString[128], dateString[128];
	CLightMapMaker	lightMaker;
	std::vector<CLightingPoint> lightingPoints;

	g_pGlobs = pGlobs;
	g_pGlobs->m_bMakingWorldModel = false;

	//start in our very first task
	ActivateTask("Initializing");

	uint32 startTime = clock();

	// Setup the region.
	g_InputRegion = new CEditRegion;

	CHelpers::ExtractNames(g_pGlobs->m_InputFile, NULL, NULL, g_InputFileTitle, NULL);

	if(g_pGlobs->m_bLogFile)
		OpenLogFile("", g_pLogFile, false);

	DrawStatusText(eST_Normal, "Processing %s", g_pGlobs->m_InputFile);
	DrawStatusText(eST_Normal, "");

	thd_GetTimeString(timeString);
	thd_GetDateString(dateString);
	DrawStatusText(eST_Normal, "Date: %s, Time: %s", dateString, timeString);

	uint32 nProcessors = thd_Init();
	DrawStatusText(eST_Normal, "This machine has %d processor%s.", nProcessors, (nProcessors>1)?"s":"");
	DrawStatusText(eST_Normal, "Using %d threads.", g_pGlobs->m_nThreads);

	g_InputRegion->m_pStringHolder = &g_StringHolder;

	LithTry
	{
		// Try to update the objects if the LTA file exists.
		DrawStatusText(eST_Normal, "Getting objects from LTA file.");
		if(LoadInputRegionLTA(world.m_vWorldOffset))
		{
			// convert keyframer objects to use blind object data for key information (removes key objects)
			DrawStatusText(eST_Normal, "Converting key data");
			if( !ConvertKeyData( g_InputRegion, &world ) )
				goto EndProcessorThread;

			DrawStatusText(eST_Normal, "Copying object heirarchy");
			CopyObjectList(g_InputRegion->m_Objects, world.m_Objects);
			DuplicateObjectHeirarchy(g_InputRegion->m_Objects, world.m_Objects);
			world.m_pInfoString = world.m_StringHolder.AddString(g_InputRegion->m_pInfoString);
		}
	
		if(!g_pGlobs->m_bObjectsOnly)  
		{
			world.TermGeometry();

			// Get stuff from the info string.
			g_pGlobs->m_MaxLMSize = GetMaxLMSize(world.m_pInfoString);
			world.m_DefaultLMGridSize = (uint32)GetLMGridSize(world.m_pInfoString);

			DrawStatusText(eST_Normal, "Max Lightmap Size: %.2f", g_pGlobs->m_MaxLMSize);
			DrawStatusText(eST_Normal, "Default Lightmap Grid Size: %d", world.GetDefaultLMGridSize());

			CMoArray<CWorldModelDef*> worldModels;
			FindWorldModels(g_InputRegion, worldModels);

			ActivateTask("Import Geometry");
			if(!ImportGeometry(g_InputRegion, worldModels, &world))
				goto EndProcessorThread;

			//do some cleanup of all the worlds
			world.RemoveAllUnusedGeometry();
			world.UpdateBoundingBox();

			// create scatter particles (scatter object specific)
			if( !ConvertScatter( &world, lightingPoints ) )
				goto EndProcessorThread;

			// Lighting.
			if(g_pGlobs->m_bLight)
			{
				ActivateTask("Lighting");

				// Light the world...
				DrawStatusText(eST_Normal, "-------------------");
				DrawStatusText(eST_Normal, "Lighting world");

				lightMaker.Init(&world,
					g_pGlobs->m_bShadows);

				world.m_LightTable.InitLightTableSize(
												&world.m_PosMin,
												&world.m_PosMax,
												ParseLightTableRes(world.m_pInfoString));

				clock_t Start = clock();
				lightMaker.ProcessLight(world.m_LightTable, &world.m_pLightGrid, lightingPoints);
				clock_t End = clock();

				DrawStatusText(eST_Normal, "Lighting took %.2f seconds", (float)(End - Start) / CLOCKS_PER_SEC);
			}	

			// If vertex lighting only was specified, clear the lightmap flag from all surfaces
			if(g_pGlobs->m_bVerticesOnly || !g_pGlobs->m_bLight)
			{
				ClearLightmapFlag(&world);
			}
		}

		ActivateTask("Packing");

		//pack the world file for the specified platform
		if( !PackForPlatform(&world, "PC"))
			goto EndProcessorThread;

		GetWorldStats( &world );
	}
	LithCatch(CLithException &x)
	{
		if( x.GetExceptionType() == LITH_MEMEXCEPTION )
			DrawStatusText(eST_Error, "Ran out of memory!");
		else
			DrawStatusText(eST_Error, "Internal error!  (Old file format?)");
	
		goto EndProcessorThread;
	}



	// (Only shows the status text if everything went successfully.)
	endTime = clock();

	DrawStatusText(eST_Normal, "");
	DrawStatusText(eST_Normal, "-------------------");
	ShowStatusText( startTime, endTime );


EndProcessorThread:;

	g_InputRegion->Term();
	delete g_InputRegion;

	thd_Term();

	CloseLogFiles();
}
