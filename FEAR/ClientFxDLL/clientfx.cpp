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
#include "ClientFX.h"
#include "SurfaceDefs.h"
#include "ClientFXVertexDeclMgr.h"
#include "memblockallocator.h"

// Dummy variable for ensuring proper external linkage.
// Note : This does not use LINKFROM_MODULE because that would require
// the game shared library, which ClientFX does not use.
int g_nClientFX = 0;

//------------------------------------------------------------------
//	Client FX Version
//
//	This should change whenever the properties are changed for an
//	effect, or when effects are added and removed. It isn't needed
//	in all cases, but it helps ensure that the data in the incoming
//	file matches the data that was packed from FXEdit
//------------------------------------------------------------------
#define	CLIENT_FX_VERSION			5

//utility function to allocate property objects from the provided allocator
template <typename T>
static T*	AllocProp(CMemBlockAllocator& Allocator)
{
	uint8* pPropMem = Allocator.Allocate(sizeof(T));
	return new (pPropMem) T;
}

//------------------------------------------------------------------
//	Effect management functions
//------------------------------------------------------------------

//Particle System
#include "ParticleSystemFX.h"
CBankedList<CParticleSystemFX>	g_ParticleSystemFX_Bank;
CBaseFX*		fxCreateParticleSystem()									{ return g_ParticleSystemFX_Bank.New(); }
uint32			fxGetParticleSystemPropSize()								{ return sizeof(CParticleSystemProps); }
CBaseFXProps*	fxCreateParticleSystemProps(CMemBlockAllocator& Allocator)	{ return AllocProp<CParticleSystemProps>(Allocator); }
void			fxGetParticleSystemProps(CFastList<CEffectPropertyDesc> *pList);

//Sprite
#include "SpriteFX.h"
CBankedList<CSpriteFX>			g_SpriteFX_Bank;
CBaseFX*		fxCreateSprite()											{ return g_SpriteFX_Bank.New(); }
uint32			fxGetSpritePropSize()										{ return sizeof(CSpriteProps); }
CBaseFXProps*	fxCreateSpriteProps(CMemBlockAllocator& Allocator)			{ return AllocProp<CSpriteProps>(Allocator); }
void			fxGetSpriteProps(CFastList<CEffectPropertyDesc> *pList);

//LTB Model
#include "LTBModelFX.h"
CBankedList<CLTBModelFX>		g_LTBModelFX_Bank;
CBaseFX*		fxCreateLTBModel()											{ return g_LTBModelFX_Bank.New(); }
uint32			fxGetLTBModelPropSize()										{ return sizeof(CLTBModelProps); }
CBaseFXProps*	fxCreateLTBModelProps(CMemBlockAllocator& Allocator)		{ return AllocProp<CLTBModelProps>(Allocator); }
void			fxGetLTBModelProps(CFastList<CEffectPropertyDesc> *pList);

//Dyna Light
#include "DynaLightFX.h"
CBankedList<CDynaLightFX>		g_DynaLightFX_Bank;
CBaseFX*		fxCreateDynaLight()											{ return g_DynaLightFX_Bank.New(); }
uint32			fxGetDynaLightPropSize()									{ return sizeof(CDynaLightProps); }
CBaseFXProps*	fxCreateDynaLightProps(CMemBlockAllocator& Allocator)		{ return AllocProp<CDynaLightProps>(Allocator); }
void			fxGetDynaLightProps(CFastList<CEffectPropertyDesc> *pList);

//Sound
#include "SoundFX.h"
CBankedList<CSoundFX>			g_SoundFX_Bank;
CBaseFX*		fxCreateSound()												{ return g_SoundFX_Bank.New(); }
uint32			fxGetSoundPropSize()										{ return sizeof(CSoundProps); }
CBaseFXProps*	fxCreateSoundProps(CMemBlockAllocator& Allocator)			{ return AllocProp<CSoundProps>(Allocator); }
void			fxGetSoundProps(CFastList<CEffectPropertyDesc> *pList);

//Camera Shake
#include "CameraShakeFX.h"
CBankedList<CCameraShakeFX>		g_CameraShakeFX_Bank;
CBaseFX*		fxCreateCameraShake()										{ return g_CameraShakeFX_Bank.New(); }
uint32			fxGetCameraShakePropSize()									{ return sizeof(CCameraShakeProps); }
CBaseFXProps*	fxCreateCameraShakeProps(CMemBlockAllocator& Allocator)		{ return AllocProp<CCameraShakeProps>(Allocator); }
void			fxGetCameraShakeProps(CFastList<CEffectPropertyDesc> *pList);

//Create Effect
#include "CreateFX.h"
CBankedList<CCreateFX>			g_CreateFX_Bank;
CBaseFX*		fxCreateCreate()											{ return g_CreateFX_Bank.New(); }
uint32			fxGetCreatePropSize()										{ return sizeof(CCreateProps); }
CBaseFXProps*	fxCreateCreateProps(CMemBlockAllocator& Allocator)			{ return AllocProp<CCreateProps>(Allocator); }
void			fxGetCreateProps(CFastList<CEffectPropertyDesc> *pList);

//Create Ray Effect
#include "CreateRayFX.h"
CBankedList<CCreateRayFX>		g_CreateRayFX_Bank;
CBaseFX*		fxCreateCreateRay()											{ return g_CreateRayFX_Bank.New(); }
uint32			fxGetCreateRayPropSize()									{ return sizeof(CCreateRayProps); }
CBaseFXProps*	fxCreateCreateRayProps(CMemBlockAllocator& Allocator)		{ return AllocProp<CCreateRayProps>(Allocator); }
void			fxGetCreateRayProps(CFastList<CEffectPropertyDesc> *pList);

//Flare Sprite
#include "FlareSpriteFX.h"
CBankedList<CFlareSpriteFX>		g_FlareSpriteFX_Bank;
CBaseFX*		fxCreateFlareSprite()										{ return g_FlareSpriteFX_Bank.New(); }
uint32			fxGetFlareSpritePropSize()									{ return sizeof(CFlareSpriteProps); }
CBaseFXProps*	fxCreateFlareSpriteProps(CMemBlockAllocator& Allocator)		{ return AllocProp<CFlareSpriteProps>(Allocator); }
void			fxGetFlareSpriteProps(CFastList<CEffectPropertyDesc> *pList);

//Lightning
#include "lightningfx.h"
CBankedList<CLightningFX>		g_LightningFX_Bank;	
CBaseFX*		fxCreateLightning()											{ return g_LightningFX_Bank.New(); }
uint32			fxGetLightningPropSize()									{ return sizeof(CLightningProps); }
CBaseFXProps*	fxCreateLightningProps(CMemBlockAllocator& Allocator)		{ return AllocProp<CLightningProps>(Allocator); }
void			fxGetLightningProps(CFastList<CEffectPropertyDesc> *pList);

//Tracer
#include "TracerFX.h"
CBankedList<CTracerFX>			g_TracerFX_Bank;	
CBaseFX*		fxCreateTracer()											{ return g_TracerFX_Bank.New(); }
uint32			fxGetTracerPropSize()										{ return sizeof(CTracerProps); }
CBaseFXProps*	fxCreateTracerProps(CMemBlockAllocator& Allocator)			{ return AllocProp<CTracerProps>(Allocator); }
void			fxGetTracerProps(CFastList<CEffectPropertyDesc> *pList);

//Lens Flare
#include "LensFlareFX.h"
CBankedList<CLensFlareFX>		g_LensFlareFX_Bank;	
CBaseFX*		fxCreateLensFlare()											{ return g_LensFlareFX_Bank.New(); }
uint32			fxGetLensFlarePropSize()									{ return sizeof(CLensFlareProps); }
CBaseFXProps*	fxCreateLensFlareProps(CMemBlockAllocator& Allocator)		{ return AllocProp<CLensFlareProps>(Allocator); }
void			fxGetLensFlareProps(CFastList<CEffectPropertyDesc> *pList);

//Debris System
#include "DebrisSystemFX.h"
CBankedList<CDebrisSystemFX>	g_DebrisSystemFX_Bank;	
CBaseFX*		fxCreateDebrisSystem()										{ return g_DebrisSystemFX_Bank.New(); }
uint32			fxGetDebrisSystemPropSize()									{ return sizeof(CDebrisSystemProps); }
CBaseFXProps*	fxCreateDebrisSystemProps(CMemBlockAllocator& Allocator)	{ return AllocProp<CDebrisSystemProps>(Allocator); }
void			fxGetDebrisSystemProps(CFastList<CEffectPropertyDesc> *pList);

//Video Controller
#include "VideoControllerFX.h"
CBankedList<CVideoControllerFX>	g_VideoControllerFX_Bank;	
CBaseFX*		fxCreateVideoController()									{ return g_VideoControllerFX_Bank.New(); }
uint32			fxGetVideoControllerPropSize()								{ return sizeof(CVideoControllerProps); }
CBaseFXProps*	fxCreateVideoControllerProps(CMemBlockAllocator& Allocator)	{ return AllocProp<CVideoControllerProps>(Allocator); }
void			fxGetVideoControllerProps(CFastList<CEffectPropertyDesc> *pList);

//Decal
#include "DecalFX.h"
CBankedList<CDecalFX>			g_DecalFX_Bank;	
CBaseFX*		fxCreateDecal()												{ return g_DecalFX_Bank.New(); }
uint32			fxGetDecalPropSize()										{ return sizeof(CDecalProps); }
CBaseFXProps*	fxCreateDecalProps(CMemBlockAllocator& Allocator)			{ return AllocProp<CDecalProps>(Allocator); }
void			fxGetDecalProps(CFastList<CEffectPropertyDesc> *pList);

//Overlay
#include "OverlayFX.h"
CBankedList<COverlayFX>			g_OverlayFX_Bank;	
CBaseFX*		fxCreateOverlay()											{ return g_OverlayFX_Bank.New(); }
uint32			fxGetOverlayPropSize()										{ return sizeof(COverlayProps); }
CBaseFXProps*	fxCreateOverlayProps(CMemBlockAllocator& Allocator)			{ return AllocProp<COverlayProps>(Allocator); }
void			fxGetOverlayProps(CFastList<CEffectPropertyDesc> *pList);

//PolyTrail
#include "PolyTrailFX.h"
CBankedList<CPolyTrailFX>			g_PolyTrailFX_Bank;	
CBaseFX*		fxCreatePolyTrail()											{ return g_PolyTrailFX_Bank.New(); }
uint32			fxGetPolyTrailPropSize()									{ return sizeof(CPolyTrailProps); }
CBaseFXProps*	fxCreatePolyTrailProps(CMemBlockAllocator& Allocator)		{ return AllocProp<CPolyTrailProps>(Allocator); }
void			fxGetPolyTrailProps(CFastList<CEffectPropertyDesc> *pList);

//Rumble
#include "RumbleFX.h"
CBankedList<CRumbleFX>			g_RumbleFX_Bank;	
CBaseFX*		fxCreateRumble()											{ return g_RumbleFX_Bank.New(); }
uint32			fxGetRumblePropSize()										{ return sizeof(CRumbleProps); }
CBaseFXProps*	fxCreateRumbleProps(CMemBlockAllocator& Allocator)			{ return AllocProp<CRumbleProps>(Allocator); }
void			fxGetRumbleProps(CFastList<CEffectPropertyDesc> *pList);


//------------------------------------------------------------------
//	Effect table
//
//	Table used for quick access to the different effect types
//------------------------------------------------------------------

struct SEffectType
{
	//prototype of functions for collecting properties used by an effect for the tools
	typedef void (*FX_GETEFFECTPROPS)(CFastList<CEffectPropertyDesc> *);

	//the name of the effect type
	const char*					m_pszName;

	//the function for creating the effect
	FX_REF::FX_CREATEFUNC		m_pfnCreate;

	//the function for creating the properties
	FX_REF::FX_CREATEPROPFUNC	m_pfnCreateProps;

	//the function for accessing properties
	FX_GETEFFECTPROPS	m_pfnGetProps;

	//the function for getting the property size
	FX_REF::FX_GETPROPSIZE		m_pfnGetPropSize;
};

static SEffectType g_EffectTable[] = 
{
	{ "ParticleSystem", fxCreateParticleSystem, fxCreateParticleSystemProps, fxGetParticleSystemProps, fxGetParticleSystemPropSize },
	{ "Sprite", fxCreateSprite, fxCreateSpriteProps, fxGetSpriteProps, fxGetSpritePropSize },
	{ "LTBModel", fxCreateLTBModel, fxCreateLTBModelProps, fxGetLTBModelProps, fxGetLTBModelPropSize },
	{ "DynaLight", fxCreateDynaLight, fxCreateDynaLightProps, fxGetDynaLightProps, fxGetDynaLightPropSize },
	{ "Sound", fxCreateSound, fxCreateSoundProps, fxGetSoundProps, fxGetSoundPropSize },
	{ "CameraShake", fxCreateCameraShake, fxCreateCameraShakeProps, fxGetCameraShakeProps, fxGetCameraShakePropSize },
	{ "Create", fxCreateCreate, fxCreateCreateProps, fxGetCreateProps, fxGetCreatePropSize },
	{ "CreateRay", fxCreateCreateRay, fxCreateCreateRayProps, fxGetCreateRayProps, fxGetCreateRayPropSize },
	{ "FlareSprite", fxCreateFlareSprite, fxCreateFlareSpriteProps, fxGetFlareSpriteProps, fxGetFlareSpritePropSize },
	{ "Lightning", fxCreateLightning, fxCreateLightningProps, fxGetLightningProps, fxGetLightningPropSize }, 
	{ "Tracer", fxCreateTracer, fxCreateTracerProps, fxGetTracerProps, fxGetTracerPropSize }, 
	{ "LensFlare", fxCreateLensFlare, fxCreateLensFlareProps, fxGetLensFlareProps, fxGetLensFlarePropSize },
	{ "DebrisSystem", fxCreateDebrisSystem, fxCreateDebrisSystemProps, fxGetDebrisSystemProps, fxGetDebrisSystemPropSize },
	{ "VideoController", fxCreateVideoController, fxCreateVideoControllerProps, fxGetVideoControllerProps, fxGetVideoControllerPropSize },
	{ "Decal", fxCreateDecal, fxCreateDecalProps, fxGetDecalProps, fxGetDecalPropSize },
	{ "Overlay", fxCreateOverlay, fxCreateOverlayProps, fxGetOverlayProps, fxGetOverlayPropSize },
	{ "PolyTrail", fxCreatePolyTrail, fxCreatePolyTrailProps, fxGetPolyTrailProps, fxGetPolyTrailPropSize },
	{ "Rumble", fxCreateRumble, fxCreateRumbleProps, fxGetRumbleProps, fxGetRumblePropSize },
};


//------------------------------------------------------------------
// Assorted global data (most to be removed eventually)
//------------------------------------------------------------------

//the interface to the client
//In SEM configurations, this is defined elsewhere
#if !defined(PLATFORM_SEM)
ILTClient*			g_pLTClient;
define_holder(ILTClient, g_pLTClient);
#endif // PLATFORM_SEM

//------------------------------------------------------------------
// Functions exported from this module
//------------------------------------------------------------------
extern "C"
{

// DllMain is not necessary in SEM configurations
#if !defined(PLATFORM_SEM)

bool APIENTRY DllMain( HANDLE /*hModule*/, 
                       uint32  /*ul_reason_for_call*/, 
                       LPVOID /*lpReserved*/
					 )
{
    return true;
}

#endif // PLATFORM_SEM

//------------------------------------------------------------------
//
//   FUNCTION : fxInitDLLRuntime()
//
//   PURPOSE  : Called to initialize the DLL when it has been loaded
//				by the game at runtime
//
//------------------------------------------------------------------

__declspec(dllexport) void fxInitDLLRuntime()
{
	//allocate our global vertex formats
	g_ClientFXVertexDecl.Init();
}

//------------------------------------------------------------------
//
//   FUNCTION : fxTermDLLRuntime()
//
//   PURPOSE  : Called to terminate the DLL prior to it being unloaded
//				by the game at runtime
//
//------------------------------------------------------------------

__declspec(dllexport) void fxTermDLLRuntime()
{
	//clean up our global vertex formats
	g_ClientFXVertexDecl.Term();
}

//------------------------------------------------------------------
//
//   FUNCTION : fxGetNum()
//
//   PURPOSE  : Returns the number of special fx in this dll
//
//------------------------------------------------------------------

__declspec(dllexport) uint32 fxGetNum()
{
	return LTARRAYSIZE(g_EffectTable);
}

//------------------------------------------------------------------
//
//   FUNCTION : fxGetVersion()
//
//   PURPOSE  : Returns the version of this DLL
//
//------------------------------------------------------------------

__declspec(dllexport) uint32 fxGetVersion()
{
	return CLIENT_FX_VERSION;
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

	if(nFx < LTARRAYSIZE(g_EffectTable))
	{
		//copy it from the effect table
		fxRef.m_sName			= g_EffectTable[nFx].m_pszName;
		fxRef.m_pfnCreate		= g_EffectTable[nFx].m_pfnCreate;
		fxRef.m_pfnCreateProps	= g_EffectTable[nFx].m_pfnCreateProps;
		fxRef.m_pfnGetPropSize	= g_EffectTable[nFx].m_pfnGetPropSize;
	}
	else
	{
		LTERROR( "Warning: Invalid access into the effect reference table");
		fxRef.m_sName			= "--INVALID_FX--";
		fxRef.m_pfnCreate		= NULL;
		fxRef.m_pfnCreateProps	= NULL;
		fxRef.m_pfnGetPropSize	= NULL;
	}
	
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

		case CBaseFX::eSoundFX :
		{
			g_SoundFX_Bank.Delete( (CSoundFX*)pDeleteFX );
		}
		break;

		case CBaseFX::eCameraShakeFX :
		{
			g_CameraShakeFX_Bank.Delete( (CCameraShakeFX*)pDeleteFX );
		}
		break;

		case CBaseFX::eCreateFX :
		{
			g_CreateFX_Bank.Delete( (CCreateFX*)pDeleteFX );
		}
		break;

		case CBaseFX::eCreateRayFX :
		{
			g_CreateRayFX_Bank.Delete( (CCreateRayFX*)pDeleteFX );
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

		case CBaseFX::eTracerFX :
		{	
			g_TracerFX_Bank.Delete( (CTracerFX*)pDeleteFX );
		}
		break;

		case CBaseFX::eLensFlareFX :
		{
			g_LensFlareFX_Bank.Delete((CLensFlareFX*)pDeleteFX);
		}
		break;

		case CBaseFX::eDebrisSystemFX :
		{
			g_DebrisSystemFX_Bank.Delete((CDebrisSystemFX*)pDeleteFX);
		}
		break;

		case CBaseFX::eVideoControllerFX :
		{
			g_VideoControllerFX_Bank.Delete((CVideoControllerFX*)pDeleteFX);
		}
		break;

		case CBaseFX::eDecalFX :
		{
			g_DecalFX_Bank.Delete((CDecalFX*)pDeleteFX);
		}
		break;

		case CBaseFX::eOverlayFX :
		{
			g_OverlayFX_Bank.Delete((COverlayFX*)pDeleteFX);
		}
		break;

		case CBaseFX::ePolyTrailFX :
		{
			g_PolyTrailFX_Bank.Delete((CPolyTrailFX*)pDeleteFX);
		}
		break;

		case CBaseFX::eRumbleFX :
		{
			g_RumbleFX_Bank.Delete((CRumbleFX*)pDeleteFX);
		}
		break;

		default:
		{
			// We should never be getting here!
			LTERROR( "Error: Attempted to delete an unknown effect type");
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
	pPropList->~CBaseFXProps();
}

//------------------------------------------------------------------
//
//   FUNCTION : FxEdGetInfo()
//
//   PURPOSE  : Fills in all data needed for FXEdit
//
//------------------------------------------------------------------
__declspec(dllexport) void FxEdGetInfo(CEffectSystemDesc*& pSystem)
{
	//default the value to something valid
	pSystem = NULL;

	//first allocate an object
	CEffectSystemDesc* pNewSystem = debug_new(CEffectSystemDesc);
	if(!pNewSystem)
		return;

	//fill in version information
	pNewSystem->m_nVersion = CLIENT_FX_VERSION;

	//we now need to fill in each type of effect, and for each of those fill
	//in their properties
	uint32 nNumTypes				= LTARRAYSIZE(g_EffectTable);
	pNewSystem->m_pEffectTypes		= debug_newa(CEffectTypeDesc, nNumTypes);
	pNewSystem->m_nNumEffectTypes	= nNumTypes;

	//check the allocation
	if(!pNewSystem->m_pEffectTypes)
	{
		delete pNewSystem;
		return;
	}

	//we can now fill in each effect type
	for(uint32 nCurrType = 0; nCurrType < nNumTypes; nCurrType++)
	{
		//figure out what we are operating on
		CEffectTypeDesc& EffectType	= pNewSystem->m_pEffectTypes[nCurrType];
		SEffectType& TableType		= g_EffectTable[nCurrType];

		//copy data over from the table
		LTStrCpy(EffectType.m_pszName, TableType.m_pszName, LTARRAYSIZE(EffectType.m_pszName));

		//setup an ID that will be used when this type is packed to a binary format
		EffectType.m_nIdentifier = nCurrType;

		//and finally, we need to create our property list for this type
		if(TableType.m_pfnGetProps)
		{
			CFastList<CEffectPropertyDesc> PropList;
			TableType.m_pfnGetProps(&PropList);

			//convert that list to a flat array we can use
			EffectType.m_pProperties = debug_newa(CEffectPropertyDesc, PropList.GetSize());

			//check the allocation
			if(!EffectType.m_pProperties)
			{
				delete pNewSystem;
				return;
			}

			//now convert over to the flat list
			CFastListNode<CEffectPropertyDesc>* pCurrNode = PropList.GetHead();
			for(uint32 nCurrProp = 0; pCurrNode && (nCurrProp < PropList.GetSize()); nCurrProp++)
			{
				EffectType.m_pProperties[nCurrProp] = pCurrNode->m_Data;
				pCurrNode = pCurrNode->GetNext();
			}
			EffectType.m_nNumProperties = PropList.GetSize();
		}
	}

	//everything was successfully setup, so return the user this object we created
	pSystem = pNewSystem;
}

//------------------------------------------------------------------
//
//   FUNCTION : FxEdReleaseInfo()
//
//   PURPOSE  : Releases information returned to FXEdit
//
//------------------------------------------------------------------
__declspec(dllexport) void FxEdReleaseInfo(CEffectSystemDesc* pSystem)
{
	delete pSystem;
}

// End extern "C"
}

//this function will be called to create the named effect. The creation structure is used
//to provide parent and placement information, but the name is not used. Instead the name
//is passed in separately in the first parameter. This name will be parsed as a semicolon
//delimited list of client fx and created accordingly with the information that is provided.
//This will return true if all of the effects were able to be created, or false if any failed.
bool CreateNewFX(IClientFXMgr* pFxMgr, const char* pszEffectName, 
				 const CLIENTFX_CREATESTRUCT& CreateInfo, bool bStartInst)
{
	if(!pFxMgr || !pszEffectName)
		return false;
	
	//assume that all succeeded. if any fail, we will set this value to false
	bool bSuccess = true;

	//copy the other creation information, so we can fill in the name of the effect
	CLIENTFX_CREATESTRUCT OurCreateInfo = CreateInfo;

	//the current character offset
	uint32 nDestOffset = 0;
	const char* pszCurrChar = pszEffectName;

	//we need to divide the name of the effect into a listing of client fx that are semicolon
	//delimited. For each effect name, we put this into the creation info, and
	//create the effect
	while(*pszCurrChar != '\0')
	{
		//if this is a semicolon, we need to create the client fx as long as we've copied over
		//something
		if(*pszCurrChar == ';')
		{
			if(nDestOffset > 0)
			{
				//make sure the string is delimited
				LTASSERT(nDestOffset < LTARRAYSIZE(OurCreateInfo.m_sName), "Error: The destination pointer went out of bounds");
				OurCreateInfo.m_sName[nDestOffset] = '\0';
	            
				if(!pFxMgr->CreateEffect(OurCreateInfo, bStartInst))
					bSuccess = false;

				//and reset our destination back to the begining
				nDestOffset = 0;
			}
		}
		else if (isspace(*pszCurrChar))
		{
			//we just want to ignore any whitespace at this point
		}
		else
		{
			//we have a character, add it to the end of the buffer if there is room
			if(nDestOffset < LTARRAYSIZE(OurCreateInfo.m_sName) - 1)
			{
				OurCreateInfo.m_sName[nDestOffset] = *pszCurrChar;
				nDestOffset++;
			}
		}

		//and move onto the next character
		++pszCurrChar;
	}

	//now see if there was an ending effect
	if(nDestOffset > 0)
	{
		//make sure the string is delimited
		LTASSERT(nDestOffset < LTARRAYSIZE(OurCreateInfo.m_sName), "Error: The destination pointer went out of bounds");
		OurCreateInfo.m_sName[nDestOffset] = '\0';
	    
		if(!pFxMgr->CreateEffect(OurCreateInfo, bStartInst))
			bSuccess = false;

		//and reset our destination back to the begining
		nDestOffset = 0;
	}

	return bSuccess;
}

//Function to handle filtering of the intersect segment calls needed by the flare sprite
static bool PointVisibleFilterFn(HOBJECT hTest, void *pUserData)
{
	LTUNREFERENCED_PARAMETER( pUserData );

	// Check for the object type. We only want to be blocked by world models since
	//otherwise models will cause the effect to flicker and we can get hit by lots of other items
	uint32 nObjType;
	if(g_pLTClient->Common()->GetObjectType(hTest, &nObjType) != LT_OK)
		return false;

	if(nObjType != OT_WORLDMODEL)
		return false;

	return true;
}

//this function should be used to determine visibility between the object and the viewer. This will
//only test visibility against world models, and will properly handle when the object is in the sky
bool IsPointVisible(const LTVector& vSrc, const LTVector& vDest, bool bDestInSky)
{
	IntersectQuery	iQuery;
	iQuery.m_Flags		= INTERSECT_HPOLY | INTERSECT_OBJECTS | IGNORE_NONSOLID;
	iQuery.m_FilterFn	= PointVisibleFilterFn;
	iQuery.m_pUserData	= NULL;

	if(bDestInSky)
	{
		//we are in the sky. This requires two steps, the first of which is to determine the direction
		//from the camera to the viewer
		LTRigidTransform tSkyCamera;
		g_pLTClient->GetSkyCameraTransform(tSkyCamera);

		//transform our point into sky space
		LTVector vSkyPt = tSkyCamera.GetInverse() * vDest;

		//now we can determine the direction that we want to head, so generate a ray of the appropraite
		//length in that direction
		static const float kfRayLength = 100000.0f;
		vSkyPt.SetMagnitude(kfRayLength);

		//and cast a ray from the viewer in that direction and see if we hit a sky portal
		iQuery.m_From		= vSrc;
		iQuery.m_To			= vSrc + vSkyPt;

		IntersectInfo	iInfo;
		if(g_pLTClient->IntersectSegment( iQuery, &iInfo ))
		{
			return IsSkyPoly(iInfo.m_hPoly);
		}

		return false;
	}
	else
	{
		//if we aren't in the sky, we can just perform a direct raycast
		iQuery.m_From		= vSrc;
		iQuery.m_To			= vDest;

		IntersectInfo	iInfo;
		return !g_pLTClient->IntersectSegment( iQuery, &iInfo );
	}
}

//given a polygon, this will determine if it is a sky polygon or not.
bool IsSkyPoly(HPOLY hPoly)
{
	if(hPoly == INVALID_HPOLY)
		return false;

	//get the poly surface type
	uint32 nTextureFlags;
	g_pLTClient->Common()->GetPolyTextureFlags(hPoly, &nTextureFlags);

	//see if it is sky
	if(nTextureFlags == ST_SKY)
		return true;

	return false;
}