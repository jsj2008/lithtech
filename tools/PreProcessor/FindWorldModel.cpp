

#include "bdefs.h"
#include "editpoly.h"
#include "editregion.h"
#include "findworldmodel.h"
#include "de_world.h"
#include "editobjects.h"
#include "processing.h"
#include "preworld.h"

static bool IsWorldModel(HCLASSMODULE hModule, CWorldNode *pNode)
{
	if (pNode->GetType() != Node_Object)
		return false;

	// If we failed loading, use the old behavior
	if (!hModule)
		return true;

	// Look up the class flags
	CBaseEditObj *pEditObj = pNode->AsObject();
	ClassDef* pClassDef = cb_FindClass(hModule, pEditObj->GetClassName());
	// If I don't know what you are, assume you're not a worldmodel
	if (!pClassDef)
		return false;

	// Return the worldmodel class flag
	return (pClassDef->m_ClassFlags & CF_WORLDMODEL) != 0;
}


static void RecurseAndAddMainBrushes(HCLASSMODULE hModule, CWorldNode *pNode, CMoArray<CEditBrush*> &brushes)
{
	uint32 type;
	GPOS pos;
	
	if((pNode->GetType() == Node_Brush) && !ShouldIgnoreNode(pNode))
	{
		type = GetBrushType(pNode->AsBrush());
		brushes.Append(pNode->AsBrush());
	}
	else if(IsWorldModel(hModule, pNode))
	{
		return;
	}

	// Go into children.
	for(pos=pNode->m_Children; pos; )
		RecurseAndAddMainBrushes(hModule, pNode->m_Children.GetNext(pos), brushes);
}


static void GatherWorldModelBrushes(
	HCLASSMODULE hModule,
	CWorldNode *pNode, 
	CMoArray<CEditBrush*> &brushes)
{
	uint32 type;
	GPOS pos;

	if((pNode->GetType() == Node_Brush) && !ShouldIgnoreNode(pNode))
	{
		type = GetBrushType(pNode->AsBrush());
		brushes.Append(pNode->AsBrush());
	}

	for(pos=pNode->m_Children; pos; )
	{
		CWorldNode *pChild = pNode->m_Children.GetNext(pos);

		// Don't steal from child worldmodels
		if (IsWorldModel(hModule, pChild))
			continue;
		
		GatherWorldModelBrushes(
			hModule,
			pChild, 
			brushes);
	}
}


static void RecurseAndAddWorldModels(
	HCLASSMODULE hModule,
	CEditRegion *pRegion, 
	CWorldNode *pNode, 
	CMoArray<CWorldModelDef*> &worldModels)
{
	if(IsWorldModel(hModule, pNode))
	{
		CBaseEditObj *pEditObj = pNode->AsObject();

		CVectorProp *pProp = (CVectorProp*)pEditObj->m_PropList.GetProp("Pos");
		if(pProp)
		{		
			CMoArray<CEditBrush*> brushes;
			brushes.SetCacheSize(4096);
		
			GatherWorldModelBrushes(hModule, pNode, brushes);
			if(brushes > 0)
			{
				// Make a WorldModel out of the brushes.
				CWorldModelDef *pDef = new CWorldModelDef;
				
				// Default WorldModel properties.
				pDef->m_WorldInfoFlags = WIF_MOVEABLE;
				
				pDef->m_Brushes.CopyArray(brushes);

				CStringProp *pNameProp = (CStringProp*)pEditObj->m_PropList.GetProp(g_NameName);
				if(!pNameProp)
				{
					DrawStatusText(eST_Error, "world model without a Name property found.");
					return;
				}

				SAFE_STRCPY(pDef->m_WorldName, pNameProp->m_String);
				
				worldModels.Append(pDef);
				brushes.SetSize(0);
			}
		}
	}

	// Recurse into children.
	for(GPOS pos=pNode->m_Children; pos; )
		RecurseAndAddWorldModels(hModule, pRegion, pNode->m_Children.GetNext(pos), worldModels);
}


static uint32 GetRealProp(CPropList *pList, char *pName, DWORD defaultValue)
{
	CBaseProp	*pProp;

	pProp = pList->GetProp(pName);
	if(!pProp)
		return defaultValue;

	if(pProp->m_Type == PT_LONGINT || pProp->m_Type == PT_REAL)
	{
		return (DWORD)((CRealProp*)pProp)->m_Value;	
	}
	else
	{
		return defaultValue;
	}
}


bool GetBoolPropVal(CPropList *pList, char *pName, bool bDefault)
{
	CBoolProp *pProp;

	if(pProp = (CBoolProp*)pList->GetMatchingProp(pName, PT_BOOL))
	{
		return (pProp->m_Value) ? true : false;
	}
	else
	{
		return bDefault;
	}
}


void FindWorldModels(CEditRegion *pRegion, CMoArray<CWorldModelDef*> &worldModels)
{
	// Load the class module
	int status, version;
	char dllName[512];

	HCLASSMODULE hClassModule;

	LTSNPrintF(dllName, sizeof(dllName), "%s\\object.lto", g_pGlobs->m_ProjectDir);

	status = cb_LoadModule(dllName, NULL, &hClassModule, &version);
	if(status != CB_NOERROR)
	{
		hClassModule = NULL;
		DrawStatusText(eST_Warning, "Unable to load class module for worldmodel building.");
	}

	// Recurse and add brushes.. stop when you hit a worldmodel
	CMoArray<CEditBrush*> mainBrushes;
	RecurseAndAddMainBrushes(hClassModule, &pRegion->m_RootNode, mainBrushes);
	if(mainBrushes.GetSize() > 0)
	{
		CWorldModelDef *pDef = new CWorldModelDef;
		pDef->m_Brushes.CopyArray(mainBrushes);
		pDef->m_WorldInfoFlags = WIF_MAINWORLD;
		SAFE_STRCPY(pDef->m_WorldName, MAINWORLD_MODEL_NAME);

		worldModels.Append(pDef);
	}

	// Fill in the worldmodels
	RecurseAndAddWorldModels(hClassModule, pRegion, &pRegion->m_RootNode, worldModels);

	// Unload the class module
	if(hClassModule)
		cb_UnloadModule(hClassModule);
}


// ------------------------------------------------------------------------------------- //
// CWorldModelDef.
// ------------------------------------------------------------------------------------- //

CWorldModelDef::CWorldModelDef()
{
	m_WorldInfoFlags = 0;
	m_WorldName[0] = 0;
	m_Brushes.SetCacheSize(1024);
}


uint32 CWorldModelDef::CalcNumPolies()
{
	DWORD i, total;

	total = 0;
	for(i=0; i < m_Brushes.GetSize(); i++)
	{
		total += m_Brushes[i]->m_Polies.GetSize();
	}

	return total;
}

uint32 GetBrushFlags_Type(CEditBrush *pBrush, uint32 nCurBrushFlags)
{
	CStringProp *pStringProp;

	pStringProp = (CStringProp*)pBrush->m_PropList.GetMatchingProp("Type", PT_STRING);
	ASSERT(pStringProp);

	const char *pStringValue = pStringProp->GetString();
	if (stricmp(pStringValue, "Normal") == 0)
	{
		nCurBrushFlags |= SURF_SOLID;
	}
	else if (stricmp(pStringValue, "SkyPortal") == 0)
	{
		nCurBrushFlags = SURF_SOLID | SURF_SKY | SURF_NOSUBDIV | SURF_CLIPLIGHT;
	}
	else if (stricmp(pStringValue, "Occluder") == 0)
	{
		nCurBrushFlags = SURF_VISBLOCKER | SURF_INVISIBLE | SURF_NOSUBDIV | SURF_NONEXISTENT;
	}
	else if (stricmp(pStringValue, "RBSplitter") == 0)
	{
		nCurBrushFlags = SURF_RBSPLITTER | SURF_NONEXISTENT;
	}
	else if (stricmp(pStringValue, "Blocker") == 0)
	{
		nCurBrushFlags = SURF_NONEXISTENT | SURF_INVISIBLE  | SURF_PHYSICSBLOCKER;
	}
	else if (stricmp(pStringValue, "RenderOnly") == 0)
	{
		nCurBrushFlags |= SURF_NONEXISTENT;
	}
	else if (stricmp(pStringValue, "NonSolid") == 0)
	{
		nCurBrushFlags &= ~SURF_SOLID;
	}
	else if (stricmp(pStringValue, "ParticleBlocker") == 0)
	{
		nCurBrushFlags = SURF_NONEXISTENT | SURF_INVISIBLE | SURF_NOSUBDIV | SURF_PARTICLEBLOCKER;
	}
	else
	{
		ASSERT(!"Unknown brush type found!");
		nCurBrushFlags |= SURF_SOLID;
	}

	return nCurBrushFlags;
}

uint32 GetBrushFlags_Lighting(CEditBrush *pBrush)
{
	uint32 brushType = 0;

	CStringProp *pStringProp;

	pStringProp = (CStringProp*)pBrush->m_PropList.GetMatchingProp("Lighting", PT_STRING);
	ASSERT(pStringProp);

	const char *pStringValue = pStringProp->GetString();
	if (stricmp(pStringValue, "Lightmap") == 0)
	{
		brushType |= SURF_LIGHTMAP | SURF_GOURAUDSHADE;
	}
	else if (stricmp(pStringValue, "Gouraud") == 0)
	{
		brushType |= SURF_GOURAUDSHADE;
	}
	else if (stricmp(pStringValue, "ShadowMesh") == 0)
	{
		brushType |= SURF_SHADOWMESH | SURF_GOURAUDSHADE;
	}
	else if (stricmp(pStringValue, "Flat") == 0)
	{
		brushType |= SURF_FLATSHADE;
	}
	else
	{
		ASSERT(!"Unknown brush lighting found!");
		brushType |= SURF_LIGHTMAP | SURF_GOURAUDSHADE;
	}

	return brushType;
}

uint32 GetBrushType(CEditBrush *pBrush)
{
	// Get the base lighting
	uint32 brushType = GetBrushFlags_Lighting(pBrush);

	// Get the other flags
	if(GetBoolPropVal(&pBrush->m_PropList, "NotAStep"))
	{
		brushType |= SURF_NOTASTEP;
	}

	if(GetBoolPropVal(&pBrush->m_PropList, "ClipLight", true))
	{
		brushType |= SURF_CLIPLIGHT;
	}

	if(GetBoolPropVal(&pBrush->m_PropList, "CastShadowMesh", true))
	{
		brushType |= SURF_CASTSHADOWMESH;
	}

	if(GetBoolPropVal(&pBrush->m_PropList, "ReceiveLight", true))
	{
		brushType |= SURF_RECEIVELIGHT;
	}

	if(GetBoolPropVal(&pBrush->m_PropList, "ReceiveShadows", true))
	{
		brushType |= SURF_RECEIVESHADOWS;
	}

	if(GetBoolPropVal(&pBrush->m_PropList, "ReceiveSunlight", true))
	{
		brushType |= SURF_RECEIVESUNLIGHT;
	}

	// Get the base type  (Needs to come last so it can override settings)
	brushType = GetBrushFlags_Type(pBrush, brushType);

	return brushType;
}

uint8 GetLMGridSize(CEditBrush *pBrush)
{
	DWORD	val = GetRealProp(&pBrush->m_PropList, "LMGridSize", 0);

	//does this need to be clamped?
	if(val > MAX_LM_GRID_SIZE)
		val = MAX_LM_GRID_SIZE;

	return (uint8)val;
}

