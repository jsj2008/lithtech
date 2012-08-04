//------------------------------------------------------------------
//
//   MODULE  : CLIENTFX.CPP
//
//   PURPOSE : Implements client FX
//
//   CREATED : On 10/6/98 At 9:21:36 PM
//
//------------------------------------------------------------------

// Includes....

#include "stdafx.h"
#include "resource.h"
#include "ClientFX.h"
#include "SpriteSystemFX.h"
#include "SpriteFX.h"
#include "DynaLightFX.h"
#include "PlaySoundFX.h"
#include "CamJitterFX.h"
#include "CamWobbleFX.h"
#include "BouncyChunkFX.h"
#include "NullFX.h"
#include "FallingStuffFX.h"
#include "PolyTubeFX.h"
#include "PlayRandomSoundFX.h"
#include "LTBModelFX.h"
#include "LTBBouncyChunkFX.h"
#include "ParticleSystemFX.h"
#include "CreateFX.h"
#include "FlareSpriteFX.h"
#include "lightningfx.h"

#ifdef _WIN32
	#include "windows.h"
#endif

ILTClient *g_pLTClient;
define_holder(ILTClient, g_pLTClient);

// Function prototypes
CBaseFX*								fxCreateParticleSystem();
void									fxGetParticleSystemProps(CFastList<FX_PROP> *pList);

CBaseFX*								fxCreateSprite();
void									fxGetSpriteProps(CFastList<FX_PROP> *pList);

CBaseFX*								fxCreateDynaLight();
void									fxGetDynaLightProps(CFastList<FX_PROP> *pList);

CBaseFX*								fxCreatePlaySound();
void									fxGetPlaySoundProps(CFastList<FX_PROP> *pList);

CBaseFX*								fxCreateCamJitter();
void									fxGetCamJitterProps(CFastList<FX_PROP> *pList);

CBaseFX*								fxCreateCamWobble();
void									fxGetCamWobbleProps(CFastList<FX_PROP> *pList);

CBaseFX*								fxCreateBouncyChunk();
void									fxGetBouncyChunkProps(CFastList<FX_PROP> *pList);

CBaseFX*								fxCreateLTBBouncyChunk();
void									fxGetLTBBouncyChunkProps(CFastList<FX_PROP> *pList);

CBaseFX*								fxCreateNull();
void									fxGetNullProps(CFastList<FX_PROP> *pList);

CBaseFX*								fxCreateFallingStuff();
void									fxGetFallingStuffProps(CFastList<FX_PROP> *pList);

CBaseFX*								fxCreatePolyTube();
void									fxGetPolyTubeProps(CFastList<FX_PROP> *pList);

CBaseFX*								fxCreatePlayRandomSound();
void									fxGetPlayRandomSoundProps(CFastList<FX_PROP> *pList);

CBaseFX*								fxCreateLTBModel();
void									fxGetLTBModelProps(CFastList<FX_PROP> *pList);

CBaseFX*								fxCreateSpriteSystem();
void									fxGetSpriteSystemProps(CFastList<FX_PROP> *pList);

CBaseFX*								fxCreateCreateFX();
void									fxGetCreateProps(CFastList<FX_PROP> *pList);

CBaseFX*								fxCreateFlareSpriteFX();
void									fxGetFlareSpriteProps(CFastList<FX_PROP> *pList);

CBaseFX*								fxCreateLightningFX();
void									fxGetLightningProps(CFastList<FX_PROP> *pList);


// Globals....


CBankedList<CParticleSystemFX>		g_ParticleSystemFX_Bank;
CBankedList<CSpriteFX>				g_SpriteFX_Bank;
CBankedList<CLTBModelFX>			g_LTBModelFX_Bank;
CBankedList<CDynaLightFX>			g_DynaLightFX_Bank;
CBankedList<CPlaySoundFX>			g_PlaySoundFX_Bank;
CBankedList<CCamJitterFX>			g_CamJitterFX_Bank;
CBankedList<CCamWobbleFX>			g_CamWobbleFX_Bank;
CBankedList<CLTBBouncyChunkFX>		g_LTBBouncyChunkFX_Bank;
CBankedList<CNullFX>				g_NullFX_Bank;
CBankedList<CPolyTubeFX>			g_PolyTubeFX_Bank;
CBankedList<CPlayRandomSoundFX>		g_PlayRandomSoundFX_Bank;
CBankedList<CSpriteSystem>			g_SpriteSystem_Bank;
CBankedList<CCreateFX>				g_CreateFX_Bank;
CBankedList<CFlareSpriteFX>			g_FlareSpriteFX_Bank;
CBankedList<CLightningFX>			g_LightningFX_Bank;	


LT_POLYGT3			g_pTris	[MAX_BUFFER_TRIS];
LTVector			g_pVerts[MAX_BUFFER_VERTS];
HOBJECT				g_hPlayer;
LTBOOL				g_bAppFocus;

//the function to call in order to create client FX
TCreateClientFXFn	g_pCreateClientFxFn;
void*				g_pCreateClientFxUserData;


// Extern....

extern "C"
{

bool APIENTRY DllMain( HANDLE hModule, 
                       uint32  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{

	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
    }
    return true;
}

//------------------------------------------------------------------
//
//   FUNCTION : fxGetNum()
//
//   PURPOSE  : Returns the number of special fx in this dll
//
//------------------------------------------------------------------

__declspec(dllexport) int fxGetNum()
{
	// Success !!

	return NUM_FX;
}

//------------------------------------------------------------------
//
//   FUNCTION : fxGetRef()
//
//   PURPOSE  : Returns a reference structure for a given fx
//
//------------------------------------------------------------------

__declspec(dllexport) FX_REF fxGetRef(int nFx)
{
	FX_REF fxRef;

	switch (nFx)
	{
		case 0 :
		{
			LTStrCpy( fxRef.m_sName, "ParticleSystem", sizeof(fxRef.m_sName) );
			fxRef.m_dwType		= FX_NEEDOBJECT | FX_NEEDCOLOURKEY | FX_NEEDSCALEKEY | FX_NEEDMOTIONKEY;
			fxRef.m_pfnCreate	= fxCreateParticleSystem;
			fxRef.m_pfnDelete	= fxDelete;
			fxRef.m_pfnGetProps	= fxGetParticleSystemProps;
		}
		break;

		case 1 :
		{
			LTStrCpy(fxRef.m_sName, "Sprite", sizeof(fxRef.m_sName) );
			fxRef.m_dwType		= FX_NEEDOBJECT | FX_NEEDCOLOURKEY | FX_NEEDSCALEKEY | FX_NEEDMOTIONKEY;
			fxRef.m_pfnCreate	= fxCreateSprite;
			fxRef.m_pfnDelete	= fxDelete;
			fxRef.m_pfnGetProps = fxGetSpriteProps;
		}
		break;

		case 2 :
		{
			LTStrCpy(fxRef.m_sName, "Null", sizeof(fxRef.m_sName) );
			fxRef.m_dwType		= FX_NEEDOBJECT | FX_NEEDMOTIONKEY;
			fxRef.m_pfnCreate	= fxCreateNull;
			fxRef.m_pfnDelete	= fxDelete;
			fxRef.m_pfnGetProps = fxGetNullProps;
		}
		break;

		case 3 :
		{
			LTStrCpy(fxRef.m_sName, "LTBModel", sizeof(fxRef.m_sName) );
			fxRef.m_dwType		= FX_NEEDOBJECT | FX_NEEDCOLOURKEY | FX_NEEDSCALEKEY | FX_NEEDMOTIONKEY;
			fxRef.m_pfnCreate	= fxCreateLTBModel;
			fxRef.m_pfnDelete	= fxDelete;
			fxRef.m_pfnGetProps = fxGetLTBModelProps;
		}
		break;

		case 4 :
		{
			LTStrCpy(fxRef.m_sName, "DynaLight", sizeof(fxRef.m_sName) );
			fxRef.m_dwType		= FX_NEEDOBJECT | FX_NEEDCOLOURKEY | FX_NEEDSCALEKEY | FX_NEEDMOTIONKEY;
			fxRef.m_pfnCreate	= fxCreateDynaLight;
			fxRef.m_pfnDelete	= fxDelete;
			fxRef.m_pfnGetProps = fxGetDynaLightProps;
		}
		break;

		case 5 :
		{
			LTStrCpy(fxRef.m_sName, "PlaySound", sizeof(fxRef.m_sName) );
			fxRef.m_dwType		= FX_NEEDOBJECT | FX_NEEDMOTIONKEY | FX_NEEDVOLUMEKEY;
			fxRef.m_pfnCreate	= fxCreatePlaySound;
			fxRef.m_pfnDelete	= fxDelete;
			fxRef.m_pfnGetProps = fxGetPlaySoundProps;
		}
		break;

		case 6 :
		{
			LTStrCpy(fxRef.m_sName, "CamJitter", sizeof(fxRef.m_sName) );
			fxRef.m_dwType		= FX_NEEDCAMERA | FX_NEEDSCALEKEY;
			fxRef.m_pfnCreate	= fxCreateCamJitter;
			fxRef.m_pfnDelete	= fxDelete;
			fxRef.m_pfnGetProps = fxGetCamJitterProps;
		}
		break;

		case 7 :
		{
			LTStrCpy(fxRef.m_sName, "WonkyFX", sizeof(fxRef.m_sName) );
			fxRef.m_dwType		= FX_NEEDCAMERA | FX_NEEDSCALEKEY;
			fxRef.m_pfnCreate	= fxCreateCamWobble;
			fxRef.m_pfnDelete	= fxDelete;
			fxRef.m_pfnGetProps = fxGetCamWobbleProps;
		}
		break;

		case 8 :
		{
			LTStrCpy(fxRef.m_sName, "BouncyChunk", sizeof(fxRef.m_sName) );
			fxRef.m_dwType		= FX_NEEDOBJECT | FX_NEEDCOLOURKEY | FX_NEEDSCALEKEY;
			fxRef.m_pfnCreate	= fxCreateBouncyChunk;
			fxRef.m_pfnDelete	= fxDelete;
			fxRef.m_pfnGetProps = fxGetBouncyChunkProps;
		}
		break;

		case 9 :
		{
			LTStrCpy(fxRef.m_sName, "LTBBouncyChunk", sizeof(fxRef.m_sName) );
			fxRef.m_dwType		= FX_NEEDOBJECT | FX_NEEDCOLOURKEY | FX_NEEDSCALEKEY;
			fxRef.m_pfnCreate	= fxCreateLTBBouncyChunk;
			fxRef.m_pfnDelete	= fxDelete;
			fxRef.m_pfnGetProps = fxGetLTBBouncyChunkProps;
		}
		break;

		case 10 :
		{
			LTStrCpy(fxRef.m_sName, "FallingStuff", sizeof(fxRef.m_sName) );
			fxRef.m_dwType		= FX_NEEDCAMERA | FX_NEEDOBJECT | FX_NEEDMOTIONKEY | FX_NEEDCOLOURKEY | FX_NEEDSCALEKEY;
			fxRef.m_pfnCreate	= fxCreateFallingStuff;
			fxRef.m_pfnDelete	= fxDelete;
			fxRef.m_pfnGetProps = fxGetFallingStuffProps;
		}
		break;

		case 11 :
		{
			LTStrCpy(fxRef.m_sName, "PolyTrail", sizeof(fxRef.m_sName) );
			fxRef.m_dwType		= FX_NEEDOBJECT | FX_NEEDMOTIONKEY | FX_NEEDCOLOURKEY | FX_NEEDSCALEKEY;
			fxRef.m_pfnCreate	= fxCreatePolyTube;
			fxRef.m_pfnDelete	= fxDelete;
			fxRef.m_pfnGetProps = fxGetPolyTubeProps;
		}
		break;

		case 12 :
		{
			LTStrCpy(fxRef.m_sName, "PlayRandomSound", sizeof(fxRef.m_sName) );
			fxRef.m_dwType		= FX_NEEDOBJECT | FX_NEEDMOTIONKEY;
			fxRef.m_pfnCreate	= fxCreatePlayRandomSound;
			fxRef.m_pfnDelete	= fxDelete;
			fxRef.m_pfnGetProps = fxGetPlayRandomSoundProps;
		}
		break;

		case 13 :
		{
			LTStrCpy(fxRef.m_sName, "SpriteSystem", sizeof(fxRef.m_sName) );
			fxRef.m_dwType		= FX_NEEDOBJECT | FX_NEEDCOLOURKEY | FX_NEEDSCALEKEY | FX_NEEDMOTIONKEY;
			fxRef.m_pfnCreate	= fxCreateSpriteSystem;
			fxRef.m_pfnDelete	= fxDelete;
			fxRef.m_pfnGetProps = fxGetSpriteSystemProps;
		}
		break;

		case 14 :
		{
			LTStrCpy(fxRef.m_sName, "CreateFX", sizeof(fxRef.m_sName) );
			fxRef.m_dwType		= 0;
			fxRef.m_pfnCreate	= fxCreateCreateFX;
			fxRef.m_pfnDelete	= fxDelete;
			fxRef.m_pfnGetProps = fxGetCreateProps;
		}
		break;

		case 15 :
		{
			LTStrCpy(fxRef.m_sName, "FlareSpriteFX", sizeof(fxRef.m_sName) );
			fxRef.m_dwType		= FX_NEEDOBJECT | FX_NEEDCOLOURKEY;
			fxRef.m_pfnCreate	= fxCreateFlareSpriteFX;
			fxRef.m_pfnDelete	= fxDelete;
			fxRef.m_pfnGetProps = fxGetFlareSpriteProps;
		}	
		break;

		case 16 :
		{
			LTStrCpy(fxRef.m_sName, "LightningFX", sizeof(fxRef.m_sName) );
			fxRef.m_dwType		= FX_NEEDOBJECT | FX_NEEDMOTIONKEY | FX_NEEDCOLOURKEY | FX_NEEDSCALEKEY;
			fxRef.m_pfnCreate	= fxCreateLightningFX;
			fxRef.m_pfnDelete	= fxDelete;
			fxRef.m_pfnGetProps = fxGetLightningProps;
		}
		break;

		default:
		{
			LTStrCpy(fxRef.m_sName, "--INVALID_FX--", sizeof(fxRef.m_sName) );
			fxRef.m_dwType		= 0;
			fxRef.m_pfnCreate	= LTNULL;
			fxRef.m_pfnDelete	= LTNULL;
			fxRef.m_pfnGetProps = LTNULL;
		}	
		break;		
	}
	
	// Success !!
	
	return fxRef;
}

//------------------------------------------------------------------
//
//   FUNCTION : fxDelete()
//
//   PURPOSE  : Deletes the FX
//
//------------------------------------------------------------------

__declspec(dllexport) void fxDelete(CBaseFX *pDeleteFX)
{
	// Figure out what kind of FX we are deleting and make sure the propper bank handles it...

	switch( pDeleteFX->GetFXType() )
	{
		case CBaseFX::eParticleSystemFX :
		{
			g_ParticleSystemFX_Bank.Delete( (CParticleSystemFX*)pDeleteFX );
		}
		break;

		case CBaseFX::eSpriteFX :
		{
			g_SpriteFX_Bank.Delete( (CSpriteFX*)pDeleteFX );
		}
		break;

		case CBaseFX::eLTBModelFX :
		{
			g_LTBModelFX_Bank.Delete( (CLTBModelFX*)pDeleteFX );
		}
		break;

		case CBaseFX::eDynaLightFX :
		{
			g_DynaLightFX_Bank.Delete( (CDynaLightFX*)pDeleteFX );
		}
		break;

		case CBaseFX::ePlaySoundFX :
		{
			g_PlaySoundFX_Bank.Delete( (CPlaySoundFX*)pDeleteFX );
		}
		break;

		case CBaseFX::eCamJitterFX :
		{
			g_CamJitterFX_Bank.Delete( (CCamJitterFX*)pDeleteFX );
		}
		break;

		case CBaseFX::eCamWobbleFX :
		{
			g_CamWobbleFX_Bank.Delete( (CCamWobbleFX*)pDeleteFX );
		}
		break;

		case CBaseFX::eLTBBouncyChunkFX :
		{
			g_LTBBouncyChunkFX_Bank.Delete( (CLTBBouncyChunkFX*)pDeleteFX );
		}
		break;

		case CBaseFX::eNullFX :
		{
			g_NullFX_Bank.Delete( (CNullFX*)pDeleteFX );
		}
		break;

		case CBaseFX::ePolyTubeFX :
		{
			g_PolyTubeFX_Bank.Delete( (CPolyTubeFX*)pDeleteFX );
		}
		break;

		case CBaseFX::ePlayRandomSoundFX :
		{
			g_PlayRandomSoundFX_Bank.Delete( (CPlayRandomSoundFX*)pDeleteFX );
		}
		break;

		case CBaseFX::eSpriteSystem :
		{
			g_SpriteSystem_Bank.Delete( (CSpriteSystem*)pDeleteFX );
		}
		break;

		case CBaseFX::eCreateFX :
		{
			g_CreateFX_Bank.Delete( (CCreateFX*)pDeleteFX );
		}
		break;

		case CBaseFX::eFlareSpriteFX :
		{
			g_FlareSpriteFX_Bank.Delete( (CFlareSpriteFX*)pDeleteFX );
		}
		break;

		case CBaseFX::eLightningFX :
		{
			g_LightningFX_Bank.Delete( (CLightningFX*)pDeleteFX );
		}
		break;

		default:
		{	
			// We should never be getting here!
			ASSERT( LTFALSE );
		}
		break;
	}
	
}

//------------------------------------------------------------------
//
//   FUNCTION : fxSetPlayer()
//
//   PURPOSE  : Sets the current player (client) object
//
//------------------------------------------------------------------
__declspec(dllexport) CBaseFXProps* fxCreatePropList(int nFx)
{
	switch(nFx)
	{
		case 0 :
			return debug_new(CParticleSystemProps);
			break;

		case 1 :
			return debug_new(CSpriteProps);
			break;

		case 2 :
			return debug_new(CNullProps);
			break;

		case 3 :
			return debug_new(CLTBModelProps);
			break;

		case 4 :
			return debug_new(CDynaLightProps);
			break;

		case 5 :
			return debug_new(CPlaySoundProps);
			break;

		case 6 :
			return debug_new(CCamJitterProps);
			break;

		case 7 :
			return debug_new(CCamWobbleProps);
			break;

		case 8 :
			return debug_new(CBouncyChunkProps);
			break;

		case 9 :
			return debug_new(CLTBBouncyChunkProps);
			break;

		case 10 :
			return debug_new(CFallingStuffProps);
			break;

		case 11 :
			return debug_new(CPolyTubeProps);
			break;

		case 12 :
			return debug_new(CPlayRandomSoundProps);
			break;

		case 13 :
			return debug_new(CSpriteSystemProps);
			break;

		case 14 :
			return debug_new(CCreateProps);
			break;

		case 15 :
			return debug_new(CFlareSpriteProps);
			break;

		case 16 :
			return debug_new(CLightningProps);
			break;

		default:
		{
			assert(!"Invalid property type requested");
			return NULL;
		}	
		break;		
	}
}

//------------------------------------------------------------------
//
//   FUNCTION : fxFreePropList()
//
//   PURPOSE  : Sets the current player (client) object
//
//------------------------------------------------------------------
__declspec(dllexport) void fxFreePropList(CBaseFXProps* pPropList)
{
	debug_delete(pPropList);
}


//------------------------------------------------------------------
//
//   FUNCTION : fxSetPlayer()
//
//   PURPOSE  : Sets the current player (client) object
//
//------------------------------------------------------------------

__declspec(dllexport) void fxSetPlayer(HOBJECT hPlayer)
{
	g_hPlayer = hPlayer;
}

//------------------------------------------------------------------
//
//   FUNCTION : fxSetParams()
//
//   PURPOSE  : Sets up parameters for FX engine
//
//------------------------------------------------------------------

__declspec(dllexport) void fxSetAppFocus(bool bAppFocus)
{
	g_bAppFocus = bAppFocus;
}

//------------------------------------------------------------------
//
//   FUNCTION : fxSetParams()
//
//   PURPOSE  : Sets up the function for creating new client FX
//
//------------------------------------------------------------------

__declspec(dllexport) void fxSetCreateFunction(TCreateClientFXFn pFn, void* pUserData)
{
	g_pCreateClientFxFn			= pFn;
	g_pCreateClientFxUserData	= pUserData;
}

// End extern "C"
}


//------------------------------------------------------------------
//
//   FUNCTION : fxCreateSpriteSystem()
//
//   PURPOSE  : Creates a particle system FX
//
//------------------------------------------------------------------

CBaseFX* fxCreateParticleSystem()
{
	return g_ParticleSystemFX_Bank.New();
}

//------------------------------------------------------------------
//
//   FUNCTION : fxCreateSprite
//
//   PURPOSE  : Creates a sprite FX
//
//------------------------------------------------------------------

CBaseFX* fxCreateSprite()
{
	return g_SpriteFX_Bank.New();
}

//------------------------------------------------------------------
//
//   FUNCTION : fxCreateLTBModel
//
//   PURPOSE  : Creates a model FX
//
//------------------------------------------------------------------

CBaseFX* fxCreateLTBModel()
{
	return g_LTBModelFX_Bank.New();
}

//------------------------------------------------------------------
//
//   FUNCTION : fxCreateDynaLight
//
//   PURPOSE  : Creates a dynamic light FX
//
//------------------------------------------------------------------

CBaseFX* fxCreateDynaLight()
{
	return g_DynaLightFX_Bank.New();
}

//------------------------------------------------------------------
//
//   FUNCTION : fxCreatePlaySound
//
//   PURPOSE  : Creates a play sound FX
//
//------------------------------------------------------------------

CBaseFX* fxCreatePlaySound()
{
	return g_PlaySoundFX_Bank.New();
}

//------------------------------------------------------------------
//
//   FUNCTION : fxCreateCamJitter
//
//   PURPOSE  : Creates a camera jitter FX
//
//------------------------------------------------------------------

CBaseFX* fxCreateCamJitter()
{
	return g_CamJitterFX_Bank.New();
}

//------------------------------------------------------------------
//
//   FUNCTION : fxCreateCamWobble
//
//   PURPOSE  : Creates a camera wobble FX
//
//------------------------------------------------------------------

CBaseFX* fxCreateCamWobble()
{
	return g_CamWobbleFX_Bank.New();
}

//------------------------------------------------------------------
//
//   FUNCTION : fxCreateBouncyChunk
//
//   PURPOSE  : Creates a bouncy chunk FX
//
//------------------------------------------------------------------

CBaseFX* fxCreateBouncyChunk()
{
	return LTNULL;
}

//------------------------------------------------------------------
//
//   FUNCTION : fxCreateLTBBouncyChunk
//
//   PURPOSE  : Creates a bouncy chunk FX
//
//------------------------------------------------------------------

CBaseFX* fxCreateLTBBouncyChunk()
{
	return g_LTBBouncyChunkFX_Bank.New();
}

//------------------------------------------------------------------
//
//   FUNCTION : fxCreateNumbers
//
//   PURPOSE  : Creates a null FX
//
//------------------------------------------------------------------

CBaseFX* fxCreateNull()
{
	return g_NullFX_Bank.New();
}

//------------------------------------------------------------------
//
//   FUNCTION : fxCreateFallingStuff
//
//   PURPOSE  : Creates a falling stuff FX
//
//------------------------------------------------------------------

CBaseFX* fxCreateFallingStuff()
{
	return LTNULL;
}

//------------------------------------------------------------------
//
//   FUNCTION : fxCreatePolyTube
//
//   PURPOSE  : Creates a poly tube FX
//
//------------------------------------------------------------------

CBaseFX* fxCreatePolyTube()
{
	return g_PolyTubeFX_Bank.New();
}

//------------------------------------------------------------------
//
//   FUNCTION : fxCreatePlayRandomSound
//
//   PURPOSE  : Creates a play random sound FX
//
//------------------------------------------------------------------

CBaseFX* fxCreatePlayRandomSound()
{
	return g_PlayRandomSoundFX_Bank.New();
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	fxCreateSpriteSystem
//
//  PURPOSE:	Creates a Sprite System FX
//
// ----------------------------------------------------------------------- //

CBaseFX* fxCreateSpriteSystem()
{
	return g_SpriteSystem_Bank.New();
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	fxCreateCreateFX
//
//  PURPOSE:	Creates a CreateFX
//
// ----------------------------------------------------------------------- //

CBaseFX* fxCreateCreateFX()
{
	return g_CreateFX_Bank.New();
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	fxCreateFlareSpriteFX
//
//  PURPOSE:	Create a FlareSpriteFX
//
// ----------------------------------------------------------------------- //

CBaseFX* fxCreateFlareSpriteFX()
{
	return g_FlareSpriteFX_Bank.New();
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	fxCreateLightningFX
//
//  PURPOSE:	Create a LightningFX
//
// ----------------------------------------------------------------------- //

CBaseFX* fxCreateLightningFX()
{
	return g_LightningFX_Bank.New();
}

//------------------------------------------------------------------
//
//   FUNCTION : AddBaseProps()
//
//   PURPOSE  : Adds base properties
//
//------------------------------------------------------------------

void AddBaseProps(CFastList<FX_PROP> *pList)
{
	FX_PROP fxProp;
	float fVec[3];
	fVec[0] = 0.0f;
	fVec[1] = 0.0f;
	fVec[2] = 0.0f;

	fxProp.Combo( FXPROP_UPDATEPOS,"0,Fixed,Follow,PlayerView,NodeAttach,SocketAttach,PV_NodeAttach,PV_SocketAttach");
	pList->AddTail(fxProp);

	fxProp.String( FXPROP_ATTACHNAME, "" );
	pList->AddTail( fxProp );

	fxProp.Vector( FXPROP_OFFSET, fVec);
	pList->AddTail(fxProp);

	fxProp.Vector( FXPROP_ROTATEADD, fVec);
	pList->AddTail(fxProp);

	fxProp.Combo( FXPROP_DISABLEATDIST, "1,No,Yes");
	pList->AddTail(fxProp);

	fxProp.Int( FXPROP_MENULAYER, 0 );
	pList->AddTail(fxProp);

	fxProp.Float( FXPROP_MAXSTARTOFFSET, 0.0f);
	pList->AddTail(fxProp);

	fxProp.Combo( FXPROP_RANDOMSTARTOFFSET, "1,No,Yes");
	pList->AddTail(fxProp);

	fxProp.Float( FXPROP_STARTOFFSETINTERVAL, 0.0f);
	pList->AddTail(fxProp);

	fxProp.Combo( FXPROP_SMOOTHSHUTDOWN, "1,No,Yes");
	pList->AddTail(fxProp);

	//the detail settings properties. Note that this must match the table in clientfxmgr.cpp
	fxProp.Combo( FXPROP_DETAILLEVEL, "0,All,High,Medium,Low,Medium+High,Low+Medium,Low+High");
	pList->AddTail(fxProp);

	fxProp.Combo( FXPROP_ISGORE, "0,No,Yes");
	pList->AddTail(fxProp);
}

bool CreateNewFX(const CLIENTFX_CREATESTRUCT& CreateInfo, bool bStartInst)
{
	if(g_pCreateClientFxFn)
	{
		return g_pCreateClientFxFn(CreateInfo, bStartInst, g_pCreateClientFxUserData);
	}

	return false;
}