
#include "bdefs.h"
#include "dedit_concommand.h"
#include "edithelpers.h"
#include "editprojectmgr.h"
#include "texture.h"
#include "mainfrm.h"
#include "regiondoc.h"
#include "regionview.h"
#include "projectbar.h"


// ----------------------------------------------------------------- // 
// Globals.
// ----------------------------------------------------------------- // 

ConsoleState g_DEditConsoleState;
SDWORD g_MipMapOffset = 0;
SDWORD g_bBilinear = 1;
SDWORD g_CV_MaxTextureMemory = (128 * 1024 * 1024);  // Default 128 megs.



// ----------------------------------------------------------------- // 
// Helpers.
// ----------------------------------------------------------------- // 

static void* dedit_cAlloc(uint32 size)
{
	return dalloc(size);
}

static void dedit_cFree(void *ptr)
{
	dfree(ptr);
}


static void dcon_FreeTextures(int argc, char *argv[])
{
	DLink listHead, *pCur;
	DFileIdent *pIdent;

	// Free all the textures.
	dfm_GetFileListWithUser(GetFileMgr(), &listHead);
	for(pCur=listHead.m_pNext; pCur != &listHead; pCur=pCur->m_pNext)
	{
		pIdent = (DFileIdent*)pCur->m_pData;
		if(pIdent->m_pUser && pIdent->m_UserType == 0)
		{
			dib_DestroyDibTexture((CTexture*)pIdent->m_pUser);
			ASSERT(!pIdent->m_pUser);
		}
	}
	
	dfm_FreeFileList(GetFileMgr(), &listHead);

	// Redraw everything.
	GetMainFrame()->UpdateAllRegions(REGIONVIEWUPDATE_REDRAW);
}


static void _ListNodes(CWorldNode *pNode, int indentation)
{
	static char spaces[100];
	int i;
	GPOS pos;

	spaces[0] = 0;
	for(i=0; i < indentation; i++)
	{
		strcat(spaces, "   ");
	}

	AddDebugMessage("%s%s: Class '%s', Name '%s', ID: '%d'", 
		spaces, 
		pNode->m_Type==Node_Brush ? "- BRUSH" : (pNode->m_Type==Node_Object ? "- OBJECT" : "- NULL"),
		pNode->GetClassName(), pNode->GetNodeLabel(), pNode->GetUniqueID());

	for(pos=pNode->m_Children; pos; )
	{
		_ListNodes(pNode->m_Children.GetNext(pos), indentation+1);
	}
}


static void dcon_ListNodes(int argc, char *argv[])
{
	CProjectBar *pBar;
	DWORD i;
	CEditRegion *pRegion;

	pBar = GetProjectBar();
	for(i=0; i < pBar->m_RegionDocs; i++)
	{
		_ListNodes(pBar->m_RegionDocs[i]->m_Region.GetRootNode(), 0);
	}
}

static void dcon_Clear(int argc, char *argv[])
{
	// Clear the console
	GetDebugDlg()->Clear();
}

// Note : This can't use CEditRegion::RenameTexture because that initializes the texture space.  Which we don't want.
// (It also counts the number of instances, which doesn't give nearly as much information...)
static void dcon_ReplaceTextures(int argc, char *argv[])
{
	if (argc < 2)
	{
		// Display an error.  Must have 2 arguments
		AddDebugMessage("Not enough texture arguments specified.  Please use the form:");
		AddDebugMessage("  ReplaceTex CurrentTexture NewTexture");
		return;
	}

	// Get the current region
	CRegionDoc  *pDoc = GetActiveRegionDoc();
	if (!pDoc)
	{
		AddDebugMessage("No active document available.");
		return;
	}

	CEditRegion *pRegion = pDoc->GetRegion();
	if (!pRegion)
	{
		AddDebugMessage("No active region available.");
		return;
	}

	// Tell the user what we're doing..
	AddDebugMessage("Searching %d brushes for replacement...", pRegion->m_Brushes.GetSize());

	// Keep track of the replacement count
	uint32 nReplaceCount = 0;

	// Point into the brush list
	LPOS iPos = pRegion->m_Brushes.GetHeadPosition();

	// Replace textures on all brush matching argv[0] with argv[1]
	while (iPos)
	{
		// Get the brush
		CEditBrush *pBrush = pRegion->m_Brushes.GetNext(iPos);

		// Did we find a match on this brush?
		bool bFoundMatch = false;

		// For every polygon..
		for (uint32 nPolyLoop = 0; nPolyLoop < pBrush->m_Polies; ++nPolyLoop)
		{
			CEditPoly *pPoly = pBrush->m_Polies[nPolyLoop];

			for(uint32 nCurrTex = 0; nCurrTex < CEditPoly::NUM_TEXTURES; nCurrTex++)
			{
				// Is this your texture?
				if (stricmp(pPoly->GetTexture(nCurrTex).m_pTextureName, argv[0]) == 0)
				{
					// Replace it...
					pPoly->GetTexture(nCurrTex).m_pTextureName = pRegion->m_pStringHolder->AddString(argv[1]);
					pPoly->GetTexture(nCurrTex).UpdateTextureID();

					// Found a match..
					bFoundMatch = true;
				}
			}
		}

		if (bFoundMatch)
			++nReplaceCount;
	}

	// And the results were...?
	AddDebugMessage("Done.  Replaced texture on %d brushes", nReplaceCount);

	// Redraw the region & mark it as changed if necessary
	if (nReplaceCount)
	{
		pDoc->Modify();
		pDoc->RedrawAllViews();
	}
}


// ----------------------------------------------------------------- // 
// Tables.
// ----------------------------------------------------------------- // 

static DECommandStruct g_DEditCommandStructs[] =
{
	"freetextures", dcon_FreeTextures, 0,
	"listnodes", dcon_ListNodes, 0,
	"clear", dcon_Clear, 0,
	"replacetex", dcon_ReplaceTextures, 0
};	

#define NUM_DEDITCOMMANDSTRUCTS	(sizeof(g_DEditCommandStructs) / sizeof(DECommandStruct))


static DEEngineVar g_DEditVars[] =
{
	EV_LONG("mipmapoffset", &g_MipMapOffset),
	EV_LONG("bilinear", &g_bBilinear),
	EV_LONG("MaxTextureMemory", &g_CV_MaxTextureMemory)
};

#define NUM_DEDITVARS	(sizeof(g_DEditVars) / sizeof(DEEngineVar))


// ----------------------------------------------------------------- // 
// Interface functions.
// ----------------------------------------------------------------- // 

void dedit_InitConsoleCommands()
{
	g_DEditConsoleState.m_SaveFns = NULL;
	g_DEditConsoleState.m_nSaveFns = 0;

	g_DEditConsoleState.m_pEngineVars = g_DEditVars;
	g_DEditConsoleState.m_nEngineVars = NUM_DEDITVARS;

	g_DEditConsoleState.m_pCommandStructs = g_DEditCommandStructs;
	g_DEditConsoleState.m_nCommandStructs = NUM_DEDITCOMMANDSTRUCTS;

	g_DEditConsoleState.Alloc = dedit_cAlloc;
	g_DEditConsoleState.Free = dedit_cFree;

	g_DEditConsoleState.ConsolePrint = AddDebugMessage;

	cc_InitState(&g_DEditConsoleState);
}


void dedit_TermConsoleCommands()
{
	cc_TermState(&g_DEditConsoleState);
}


void dedit_CommandHandler(char *pCommand)
{
	cc_HandleCommand(&g_DEditConsoleState, pCommand);
}




