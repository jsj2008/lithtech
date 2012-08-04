#include "bdefs.h"

#include <stddef.h>
#include "render.h"
#include "consolecommands.h"
#include "dsys_interface.h"
#include "clientmgr.h"
#include "sysclientde_impl.h"
#include "varsetter.h"
#include "videomgr.h"
#include "sysconsole_impl.h"
#include "dtxmgr.h"

//------------------------------------------------------------------
//------------------------------------------------------------------
// Holders and their headers.
//------------------------------------------------------------------
//------------------------------------------------------------------

//IClientFileMgr
#include "client_filemgr.h"
static IClientFileMgr *client_file_mgr;
define_holder(IClientFileMgr, client_file_mgr);

//the ILTClient game interface
#include "iltclient.h"
static ILTClient *ilt_client;
define_holder(ILTClient, ilt_client);

//IClientShell game client shell object.
#include "iclientshell.h"
static IClientShell *i_client_shell;
define_holder(IClientShell, i_client_shell);

//ILTCommon client instance.
#include "iltcommon.h"
static ILTCommon *ilt_common_client;
define_holder_to_instance(ILTCommon, ilt_common_client, Client);




// ------------------------------------------------------------ //
// Globals..
// ------------------------------------------------------------ //
SysCache g_SysCache;

extern int32	g_CV_CursorCenter;

RMode g_RMode;


// ------------------------------------------------------------ //
// Internal functions.
// ------------------------------------------------------------ //

void r_UnloadSystemTexture(TextureData *pTexture)
{
	if(!pTexture)
		return;

	ASSERT(g_SysCache.m_CurMem >= pTexture->m_AllocSize);
	g_SysCache.m_CurMem -= pTexture->m_AllocSize;

	delete[] pTexture->m_pDataBuffer; 
	pTexture->m_pDataBuffer = NULL; 
	
	if (pTexture->m_pSharedTexture) 
		pTexture->m_pSharedTexture->m_pEngineData = LTNULL; 

	dl_RemoveAt(&g_SysCache.m_List, &pTexture->m_Link);
	delete pTexture;
}

// Loads the texture and installs it.
LTRESULT r_LoadSystemTexture(SharedTexture *pSharedTexture)
{
	LTRESULT dResult;
	FileRef ref;

	FileIdentifier *pIdent = pSharedTexture->m_pFile;

	if (!pIdent) 
		return LT_NOTINITIALIZED;

	uint32 nBaseWidth;
	uint32 nBaseHeight;

	//the texture data that is associated with the texture
	TextureData* pTextureData = NULL;

	ILTStream *pStream = client_file_mgr->OpenFileIdentifier(pIdent);
	if (pStream) 
	{
		dResult = dtx_Create(pStream, &pTextureData, nBaseWidth, nBaseHeight);
		pStream->Release();

		if (dResult != LT_OK) 
			return dResult; 
	}
	else 
	{
		RETURN_ERROR_PARAM(1, r_LoadSystemTexture, LT_MISSINGFILE, pIdent->m_Filename); 
	}

	//make sure to setup the texture information
	pSharedTexture->SetTextureInfo(nBaseWidth, nBaseHeight, pTextureData->m_PFormat);

	// Add the new texture to the MRU list.
	dl_AddHead(&g_SysCache.m_List, &pTextureData->m_Link, pTextureData);
	g_SysCache.m_CurMem += pTextureData->m_AllocSize;

	// Store its pointer in the SharedTexture.
	pSharedTexture->m_pEngineData = pTextureData;
	pTextureData->m_pSharedTexture = pSharedTexture;

	// Load in any linked textures depending upon what the user entered in the command string. Note
	//that these are exclusive

	ConParse parse;

	//Normal detail texturing
	parse.Init(pTextureData->m_Header.m_CommandString);
	if (parse.ParseFind("DetailTex", LTFALSE, 1)) 
	{

        //validate the argument count
        if(parse.m_nArgs < 2)
        {
		    RETURN_ERROR_PARAM(1, r_LoadSystemTexture, LT_INVALIDDATA, "DetailTex : parse.m_Args[1]"); 
        }

		ref.m_FileType = FILE_CLIENTFILE;
		ref.m_pFilename = parse.m_Args[1];
		pSharedTexture->SetLinkedTexture(eLinkedTex_Detail, g_pClientMgr->AddSharedTexture(&ref));
		pSharedTexture->m_eTexType = eSharedTexType_Detail; 
		return LT_OK;
	}

	//Normal environment mapping
	parse.Init(pTextureData->m_Header.m_CommandString);
	if (parse.ParseFind("EnvMap", LTFALSE, 1)) 
	{

        //validate the argument count
        if(parse.m_nArgs < 2)
        {
		    RETURN_ERROR_PARAM(1, r_LoadSystemTexture, LT_INVALIDDATA, "EnvMap : parse.m_Args[1]"); 
        }

		ref.m_FileType = FILE_CLIENTFILE;
		ref.m_pFilename = parse.m_Args[1];
		pSharedTexture->SetLinkedTexture(eLinkedTex_EnvMap, g_pClientMgr->AddSharedTexture(&ref));
		pSharedTexture->m_eTexType = eSharedTexType_EnvMap; 
		return LT_OK;
	}

	//Environment mapping blended with the alpha of the base texture
	parse.Init(pTextureData->m_Header.m_CommandString);
	if (parse.ParseFind("EnvMapAlpha", LTFALSE, 1)) 
	{

        //validate the argument count
        if(parse.m_nArgs < 2)
        {
		    RETURN_ERROR_PARAM(1, r_LoadSystemTexture, LT_INVALIDDATA, "EnvMapAlpha : parse.m_Args[1]"); 
        }

		ref.m_FileType = FILE_CLIENTFILE;
		ref.m_pFilename = parse.m_Args[1];
		pSharedTexture->SetLinkedTexture(eLinkedTex_EnvMap, g_pClientMgr->AddSharedTexture(&ref));
		pSharedTexture->m_eTexType = eSharedTexType_EnvMapAlpha; 
		return LT_OK;
	}

	//Environment mapping with a bumpmap texture
	parse.Init(pTextureData->m_Header.m_CommandString);
	if (parse.ParseFind("EnvBumpMap", LTFALSE, 2)) 
	{
        //validate the argument count
        if(parse.m_nArgs < 3)
        {
            if(parse.m_nArgs < 2)
            {
		        RETURN_ERROR_PARAM(1, r_LoadSystemTexture, LT_INVALIDDATA, "EnvBumpMap : parse.m_Args[1]"); 
            }

		    RETURN_ERROR_PARAM(1, r_LoadSystemTexture, LT_INVALIDDATA, "EnvBumpMap : parse.m_Args[2]"); 
        }

		//handle loading up the environment map first
		ref.m_FileType = FILE_CLIENTFILE;
		ref.m_pFilename = parse.m_Args[1];
		pSharedTexture->SetLinkedTexture(eLinkedTex_EnvMap, g_pClientMgr->AddSharedTexture(&ref));

		//now handle loading up the bumpmap
		ref.m_FileType = FILE_CLIENTFILE;
		ref.m_pFilename = parse.m_Args[2];
		pSharedTexture->SetLinkedTexture(eLinkedTex_BumpMap, g_pClientMgr->AddSharedTexture(&ref));

		//setup the shader type
		pSharedTexture->m_eTexType = eSharedTexType_EnvBumpMap; 
		return LT_OK;
	}

	//Environment mapping with a bumpmap texture but no fallback
	parse.Init(pTextureData->m_Header.m_CommandString);
	if (parse.ParseFind("EnvBumpMapNoFallback", LTFALSE, 2)) 
	{

        //validate the argument count
        if(parse.m_nArgs < 3)
        {
            if(parse.m_nArgs < 2)
            {
		        RETURN_ERROR_PARAM(1, r_LoadSystemTexture, LT_INVALIDDATA, "EnvBumpMapNoFallback : parse.m_Args[1]"); 
            }

		    RETURN_ERROR_PARAM(1, r_LoadSystemTexture, LT_INVALIDDATA, "EnvBumpMapNoFallback : parse.m_Args[2]"); 
        }

		//handle loading up the environment map first
		ref.m_FileType = FILE_CLIENTFILE;
		ref.m_pFilename = parse.m_Args[1];
		pSharedTexture->SetLinkedTexture(eLinkedTex_EnvMap, g_pClientMgr->AddSharedTexture(&ref));

		//now handle loading up the bumpmap
		ref.m_FileType = FILE_CLIENTFILE;
		ref.m_pFilename = parse.m_Args[2];
		pSharedTexture->SetLinkedTexture(eLinkedTex_BumpMap, g_pClientMgr->AddSharedTexture(&ref));

		//setup the shader type
		pSharedTexture->m_eTexType = eSharedTexType_EnvBumpMap_NoFallback; 
		return LT_OK;
	}


	//DOT3 bumpmap texture
	parse.Init(pTextureData->m_Header.m_CommandString);
	if (parse.ParseFind("DOT3BumpMap", LTFALSE, 1)) 
	{

		//validate the argument count
		if(parse.m_nArgs < 2)
		{
			RETURN_ERROR_PARAM(1, r_LoadSystemTexture, LT_INVALIDDATA, "DOT3BumpMap : parse.m_Args[1]"); 
		}


		//now handle loading up the bumpmap
		ref.m_FileType = FILE_CLIENTFILE;
		ref.m_pFilename = parse.m_Args[1];
		pSharedTexture->SetLinkedTexture(eLinkedTex_BumpMap, g_pClientMgr->AddSharedTexture(&ref));

		//setup the shader type
		pSharedTexture->m_eTexType = eSharedTexType_DOT3BumpMap; 
		return LT_OK;
	}



	//Environment mapping with a DOT3 bumpmap texture but no fallback
	parse.Init(pTextureData->m_Header.m_CommandString);
	if (parse.ParseFind("DOT3EnvBumpMap", LTFALSE, 2)) 
	{

        //validate the argument count
        if(parse.m_nArgs < 3)
        {
            if(parse.m_nArgs < 2)
            {
		        RETURN_ERROR_PARAM(1, r_LoadSystemTexture, LT_INVALIDDATA, "DOT3EnvBumpMap : parse.m_Args[1]"); 
            }

		    RETURN_ERROR_PARAM(1, r_LoadSystemTexture, LT_INVALIDDATA, "DOT3EnvBumpMap : parse.m_Args[2]"); 
        }

		//handle loading up the environment map first
		ref.m_FileType = FILE_CLIENTFILE;
		ref.m_pFilename = parse.m_Args[1];
		pSharedTexture->SetLinkedTexture(eLinkedTex_EnvMap, g_pClientMgr->AddSharedTexture(&ref));

		//now handle loading up the bumpmap
		ref.m_FileType = FILE_CLIENTFILE;
		ref.m_pFilename = parse.m_Args[2];
		pSharedTexture->SetLinkedTexture(eLinkedTex_BumpMap, g_pClientMgr->AddSharedTexture(&ref));

		//setup the shader type
		pSharedTexture->m_eTexType = eSharedTexType_DOT3EnvBumpMap; 
		return LT_OK;
	}


	//Effect File Support
	parse.Init(pTextureData->m_Header.m_CommandString);
	if (parse.ParseFind("Effect", LTFALSE, 1)) 
	{
		//validate the argument count
		if(parse.m_nArgs < 2)
		{
			RETURN_ERROR_PARAM(1, r_LoadSystemTexture, LT_INVALIDDATA, "Effect : parse.m_Args[1]"); 
		}

		// Set up our shader ID
		pSharedTexture->m_nShaderID = (int)atoi(parse.m_Args[1]);
		pSharedTexture->m_eTexType = eSharedTexType_Effect;

		if(parse.m_nArgs > 2)
		{
			ref.m_FileType = FILE_CLIENTFILE;
			ref.m_pFilename = parse.m_Args[2];
			pSharedTexture->SetLinkedTexture(eLinkedTex_EffectTexture1, g_pClientMgr->AddSharedTexture(&ref));
			pSharedTexture->m_eTexType = eSharedTexType_Effect;
		}

		if(parse.m_nArgs > 3)
		{
			ref.m_FileType = FILE_CLIENTFILE;
			ref.m_pFilename = parse.m_Args[3];
			pSharedTexture->SetLinkedTexture(eLinkedTex_EffectTexture2, g_pClientMgr->AddSharedTexture(&ref));
			pSharedTexture->m_eTexType = eSharedTexType_Effect;
		}

		if(parse.m_nArgs > 4)
		{
			ref.m_FileType = FILE_CLIENTFILE;
			ref.m_pFilename = parse.m_Args[4];
			pSharedTexture->SetLinkedTexture(eLinkedTex_EffectTexture3, g_pClientMgr->AddSharedTexture(&ref));
			pSharedTexture->m_eTexType = eSharedTexType_Effect;
		}

		if(parse.m_nArgs > 5)
		{
			ref.m_FileType = FILE_CLIENTFILE;
			ref.m_pFilename = parse.m_Args[5];
			pSharedTexture->SetLinkedTexture(eLinkedTex_EffectTexture4, g_pClientMgr->AddSharedTexture(&ref));
			pSharedTexture->m_eTexType = eSharedTexType_Effect;
		}

		return LT_OK;
	}

	//just a normal texture...
	return LT_OK;
}


// ------------------------------------------------------------ //
// RenderStruct function implementations.
// ------------------------------------------------------------ //
LTObject* r_ProcessAttachment(LTObject *pParent, Attachment *pAttachment)
{
	// Use CommonLT::GetAttachmentTransform.
	LTransform tAttachment;
	LTRESULT dResult = ilt_common_client->GetAttachmentTransform((HATTACHMENT)pAttachment, tAttachment, LTTRUE);
	if (dResult != LT_OK) 
		return LTNULL;

	LTObject *pChild = g_pClientMgr->FindObject(pAttachment->m_nChildID);
	if (!pChild) 
		return LTNULL;

	if (pChild->m_Pos != tAttachment.m_Pos)
	{
		i_client_shell->OnObjectMove( (HOBJECT)pChild, LTFALSE, &tAttachment.m_Pos );
		g_pClientMgr->MoveObject(pChild, &tAttachment.m_Pos, LTFALSE);
	}

	if (pChild->m_Rotation != tAttachment.m_Rot)
	{
		i_client_shell->OnObjectRotate( (HOBJECT)pChild, LTFALSE, &tAttachment.m_Rot );
		g_pClientMgr->RotateObject(pChild, &tAttachment.m_Rot);
	}

	if (tAttachment.m_Scale.x != 1.0f || tAttachment.m_Scale.y != 1.0f || tAttachment.m_Scale.z != 1.0f)
	{
		g_pClientMgr->ScaleObject(pChild, &tAttachment.m_Scale);
	}
	
	return pChild;
}

static SharedTexture* r_GetSharedTexture(const char *pFilename)
{
	FileRef ref;

	ref.m_FileType = FILE_ANYFILE;
	ref.m_pFilename = (char *)pFilename; // Note : I hate when things aren't const, and they should be..

	return g_pClientMgr->AddSharedTexture(&ref);
}


//this will run through and release any textures that have a valid file pointer so that they can
//be recreated later on demand
void r_TerminateAllRecreatableTextureData()
{
	// Get rid of all the loaded textures.
	LTLink* pListHead = &g_SysCache.m_List.m_Head;

	LTLink* pCur = pListHead->m_pNext;

	while(pCur != pListHead) 
	{
		//cache the next pointer since this one will be removed
		LTLink* pNext = pCur->m_pNext;

		TextureData* pTexture = (TextureData*)pCur->m_pData;

		//all texture datas in this list must be bound to a shared texture
		if(pTexture && pTexture->m_pSharedTexture)
		{
			//determine if we could recreate this shared texture later
			if(pTexture->m_pSharedTexture->m_pFile)
			{
				//it can be recreated later, so remove this texture
				r_UnloadSystemTexture((TextureData*)pCur->m_pData);
			}
		}
		else
		{
			assert(!"Invalid texture data was added to the list of textures");
		}

		pCur = pNext; 
	}
}

//this should be called to access texture data of a texture. If it has no image and the shared texture
//is properly setup, it will load the image data and bind it to the shared texture
TextureData*	r_GetTextureData(SharedTexture *pTexture)
{
	// Is it already loaded?
	if(!pTexture->m_pEngineData)
	{
		//not loaded, so load it up
		r_LoadSystemTexture(pTexture);
	}

	return (TextureData*)pTexture->m_pEngineData;
}

//this should be called to access information on a texture. It will ensure that it is filled out.
//The values are invalid if it returns false
bool r_GetTextureInfo(SharedTexture *pTexture, uint32& nWidth, uint32& nHeight, PFormat& Format)
{
	//see if this texture already has valid data associated with it
	if(pTexture->GetFlags() & ST_VALIDTEXTUREINFO)
	{
		//it is valid, we can just use that
		pTexture->GetTextureInfo(nWidth, nHeight, Format);
		return true;
	}

	//the texture hasn't been setup yet, so get the texture data, which should load it and set
	//it up
	r_GetTextureData(pTexture);

	//now it should be valid
	if(!(pTexture->GetFlags() & ST_VALIDTEXTUREINFO))
	{
		//failed to get the information from the texture
		return false;
	}

	//we can now just get the information
	pTexture->GetTextureInfo(nWidth, nHeight, Format);
	return true;
}

static const char*	r_GetTextureName(const SharedTexture *pTexture)
{
	return pTexture->m_pFile->m_Filename;
}

static void r_FreeTexture(SharedTexture *pTexture)
{
	if (pTexture->m_pEngineData) 
	{
		TextureData* pTextureData = (TextureData*)pTexture->m_pEngineData;
		r_UnloadSystemTexture(pTextureData); 
	}
}


void r_RunConsoleString(char *pStr)
{
	cc_HandleCommand(&g_ClientConsoleState, pStr);
}

static void r_ConsolePrint(char *pMsg, ...)
{
	char msg[300];
	va_list marker;

	va_start(marker, pMsg);
	LTVSNPrintF(msg, sizeof(msg), pMsg, marker);
	va_end(marker);

	con_WhitePrintf(msg);
}

void r_GenericTextPrint(char *pMsg, const LTRect *pRect, uint32 textColor)
{
	GETCONSOLE()->DrawTextLine(pMsg,pRect,textColor);
}

HLTPARAM r_GetParameter(char *pName)
{
	return (HLTPARAM)cc_FindConsoleVar(&g_ClientConsoleState, pName);
}

float r_GetParameterValueFloat(HLTPARAM hParam)
{
	if(hParam)
	{
		return ((LTCommandVar*)hParam)->floatVal;
	}
	else
	{
		return 0.0f;
	}
}

char* r_GetParameterValueString(HLTPARAM hParam)
{
	if(hParam)
	{
		return ((LTCommandVar*)hParam)->pStringVal;
	}
	else
	{
		return LTNULL;
	}
}

static uint32 r_GetObjectFrameCode()
{
	return g_pClientMgr->m_ObjectMgr.GetFrameCode();
}

static uint32 r_IncObjectFrameCode()
{
	return g_pClientMgr->m_ObjectMgr.IncFrameCode();
}

static void r_InitSysCache(SysCache *pCache)
{
	dl_InitList(&pCache->m_List);
	pCache->m_CurMem = 0;
}


// ------------------------------------------------------------ //
// The global RenderStruct.
// ------------------------------------------------------------ //

RenderStruct g_Render;


// ------------------------------------------------------------ //
// Interface functions.
// ------------------------------------------------------------ //

// For MemoryWatch.
unsigned long GetInternalTextureMemory()
{
	return g_SysCache.m_CurMem;
}

unsigned long GetRendererTextureMemory()
{
	return g_Render.m_SystemTextureMemory;
}

void r_InitRenderStruct(bool bFullClear)
{
	if(bFullClear)
		memset(&g_Render, 0, sizeof(g_Render));
	else
		memset(&g_Render, 0, offsetof(RenderStruct, m_DontClearMarker));
	
	g_Render.ProcessAttachment			= r_ProcessAttachment;	
	g_Render.GetSharedTexture			= r_GetSharedTexture;
	g_Render.GetTexture					= r_GetTextureData;
	g_Render.GetTextureName				= r_GetTextureName;
	g_Render.FreeTexture				= r_FreeTexture;
	g_Render.RunConsoleString			= r_RunConsoleString;
	g_Render.ConsolePrint				= r_ConsolePrint;
	g_Render.GetParameter				= r_GetParameter;
	g_Render.GetParameterValueFloat		= r_GetParameterValueFloat;
	g_Render.GetParameterValueString	= r_GetParameterValueString;
	g_Render.IncObjectFrameCode			= r_IncObjectFrameCode;
	g_Render.GetObjectFrameCode			= r_GetObjectFrameCode;

	g_Render.m_GlobalLightDir.Init(0.0f, -2.0f, -1.0f);
    g_Render.m_GlobalLightDir.Norm();
}

bool g_bFirstTimeInit = true;
LTRESULT r_InitRender(RMode *pMode)
{
	RenderStructInit init;
	int initStatus;
	HWND hWnd;

	// Don't get in here recursively.
	if(g_ClientGlob.m_bInitializingRenderer)
		return LT_OK;


	VarSetter<BOOL> setter(&g_ClientGlob.m_bInitializingRenderer, LTTRUE, LTFALSE);

	r_TermRender(0, false);


	hWnd = (HWND)dsi_GetMainWindow();
	ShowWindow(hWnd, SW_RESTORE);

	if (g_bFirstTimeInit) 
		r_InitSysCache(&g_SysCache);
	
	// Setup the init request.
	memset(&init, 0, sizeof(init));
	init.m_hWnd = (void*)hWnd;
	memcpy(&init.m_Mode, pMode, sizeof(RMode));


	// Set up the RenderStruct.
	rdll_RenderDLLSetup(&g_Render);

	// Store these.. the renderer may change them for pixel doubling.
	g_Render.m_Width = pMode->m_Width;
	g_Render.m_Height = pMode->m_Height;

	// Try to initialize the renderer for the requested mode.
	initStatus = g_Render.Init(&init);
	if(initStatus != RENDER_OK || init.m_RendererVersion != LTRENDER_VERSION)
	{
		g_pClientMgr->SetupError(LT_ERRORLOADINGRENDERDLL, "Init Failed");
		RETURN_ERROR_PARAM(1, r_InitRender, LT_ERRORLOADINGRENDERDLL, "Init Failed");
	}

	// Init the console.
	g_pClientMgr->InitConsole();

	// Restore interface surfaces that were backed up.
	if(!cis_RendererIsHere(r_GetRenderStruct()))
	{
		g_pClientMgr->SetupError(LT_UNABLETORESTOREVIDEO);
		RETURN_ERROR(1, r_InitRender, LT_UNABLETORESTOREVIDEO);
	}

	g_Render.m_bInitted = true;
	g_Render.m_bLoaded = true;

    // Let the game do a loading screen or something.
    if (i_client_shell != NULL) {
        i_client_shell->OnEvent(LTEVENT_RENDERALMOSTINITTED, 0);
    }

	// Bind any open worlds.
	if(!g_pClientMgr->BindClientShellWorlds())
	{
		g_Render.m_bInitted = false;
		g_Render.m_bLoaded = false;
		RETURN_ERROR(1, r_InitRender, LT_ERRORBINDINGWORLD);
	}

	// Set focus and capture the mouse.  We leave things like resizing the window to the render DLL.
	SetFocus(hWnd);

	// Bind any open textures.
	g_pClientMgr->BindSharedTextures();

	// Store this config..
	memcpy(&g_RMode, &init.m_Mode, sizeof(RMode));

	char cmd[200];
	// Set the console variables.
	LTSNPrintF(cmd, sizeof(cmd), "CardDesc %s", init.m_Mode.m_InternalName);
	c_CommandHandler(cmd);

	LTSNPrintF(cmd, sizeof(cmd), "ScreenWidth %d", init.m_Mode.m_Width);
	c_CommandHandler(cmd);

	LTSNPrintF(cmd, sizeof(cmd), "ScreenHeight %d", init.m_Mode.m_Height);
	c_CommandHandler(cmd);

	LTSNPrintF(cmd, sizeof(cmd), "BitDepth %d", init.m_Mode.m_BitDepth);
	c_CommandHandler(cmd);

	// The console can load its background now that we're initialized.
	con_LoadBackground();

    // Tell the game the renderer has initialized.
    if (i_client_shell != NULL) 
	{
        i_client_shell->OnEvent(LTEVENT_RENDERINIT, 0);
    }
    
	g_bFirstTimeInit = false;
	return LT_OK;
}

LTRESULT r_TermRender(int surfaceHandling, bool bUnLoadDLL)
{
	LTLink *pCur, *pNext, *pListHead;

	if(g_Render.m_bInitted || (g_Render.m_bLoaded && bUnLoadDLL))
	{
        // Tell the game the renderer has initialized.
        if (i_client_shell != NULL) 
		{
            i_client_shell->OnEvent(LTEVENT_RENDERTERM, 0);
        }

		// Notify the video stuff.
		if (g_pClientMgr->m_pVideoMgr)
		{
			g_pClientMgr->m_pVideoMgr->OnRenderTerm();
		}

		// Notify the client interface system-dependent module..
		if(!cis_RendererGoingAway())
		{
			// If the surface backup failed and we care, give a message.
			if(surfaceHandling == 1)
			{
				g_pClientMgr->SetupError(LT_UNABLETORESTOREVIDEO);
				RETURN_ERROR(1, r_TermRender, LT_UNABLETORESTOREVIDEO);
			}
		}
		
		// Unbind everybody from the renderer.
		g_pClientMgr->TermConsole();
		g_pClientMgr->UnbindSharedTextures(bUnLoadDLL);
		g_pClientMgr->UnbindClientShellWorlds();

		g_Render.Term(bUnLoadDLL);
		
		// Un-clip the cursor.
		if (g_CV_CursorCenter) 
		{
			ClipCursor(LTNULL); 
		}

		if (bUnLoadDLL) 
		{
			// Get rid of all the loaded textures.
			pListHead = &g_SysCache.m_List.m_Head;
			pCur = pListHead->m_pNext;
			while(pCur != pListHead) 
			{
				pNext = pCur->m_pNext;
				r_UnloadSystemTexture((TextureData*)pCur->m_pData);
				pCur = pNext; 
			}
			ASSERT(g_SysCache.m_CurMem == 0); 
		
			g_Render.m_bLoaded = false;
			r_InitRenderStruct(false); // Reinitialize the renderstruct.
		}
		g_Render.m_bInitted = false;
	}

	return LT_OK;
}


void r_BindTexture(SharedTexture *pSharedTexture, LTBOOL bTextureChanged)
{
	if (g_Render.m_bInitted) 
	{
		g_Render.BindTexture(pSharedTexture, bTextureChanged ? true : false); 
	}
}

void r_UnbindTexture(SharedTexture *pSharedTexture, bool bUnLoad_EngineData)
{
	if (g_Render.m_bInitted) 
	{
		g_Render.UnbindTexture(pSharedTexture); 
	}

	if (bUnLoad_EngineData && pSharedTexture->m_pEngineData) 
	{
		r_UnloadSystemTexture((TextureData*)pSharedTexture->m_pEngineData);
		pSharedTexture->m_pEngineData = LTNULL; 
	}
}


