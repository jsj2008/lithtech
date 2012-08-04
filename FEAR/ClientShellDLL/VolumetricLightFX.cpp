// ----------------------------------------------------------------------- //
//
// MODULE  : VolumetricLightFX.cpp
//
// PURPOSE : Client side implementation of the volumetric lighting effect
//			 associated with LightSpot objects
//
// CREATED : 9/21/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "VolumetricLightFX.h"
#include "iltrenderer.h"
#include "PlayerCamera.h"
#include "CategoryDB.h"
#include "iperformancemonitor.h"

#include <algorithm>

#include "ILTCustomRender.h"
static ILTCustomRender *g_pLTCustomRender;
define_holder(ILTCustomRender, g_pLTCustomRender);

#include "ILTRenderer.h"
static ILTRenderer *g_pLTRenderer;
define_holder(ILTRenderer, g_pLTRenderer);

//our object used for tracking performance for volumetric lights
static CTimedSystem g_tsClientVolLights("GameClient_VolLights", "GameClient");

// Maximum allowable volumetric light LOD
VarTrack g_cvarEnableVolumetricLight;
// Control for the number of slices in the volume
VarTrack g_cvarVolumetricLightSlices;
// Control for the resolution of the slice buffer
VarTrack g_cvarVolumetricLightSliceRes;
// Control for toggling the shadows
VarTrack g_cvarVolumetricLightShadow;
// Control for the resolution of the shadow map
VarTrack g_cvarVolumetricLightShadowRes;
// Control for toggling the noise
VarTrack g_cvarVolumetricLightNoise;
// Control for toggling the floating point buffers
VarTrack g_cvarVolumetricLightFloat;

#if defined(PLATFORM_XENON)
	#define DEFAULT_SLICES 256.0f
#else
	#define DEFAULT_SLICES 64.0f
#endif

//////////////////////////////////////////////////////////////////////////
// Internal manager for render targets, materials, vertex buffers, and etc.

class CVLFXResourceMgr
{
public:
	CVLFXResourceMgr() : 
		m_nRefCount(0),
		m_hVertexDecl(NULL), 
		m_hShellMaterial(NULL),
		m_hShellIB(NULL),
		m_nCurShadowRes(0),
		m_nCurSliceRes(0),
		m_bShadowsEnabled(false),
		m_bFloatBuffersEnabled(false)
	{
		for (uint32 nLoop = 0; nLoop < LTARRAYSIZE(m_aSliceMaterials); ++nLoop)
		{
			m_aSliceMaterials[nLoop] = NULL;
		}
	}
	
	~CVLFXResourceMgr()
	{
		Term();
	}
	
	bool Init()
	{
		// Only truly initialize on the first volumetric light
		++m_nRefCount;
		if (m_nRefCount > 1)
			return true;

		// Setup any console variables
		if (!g_cvarEnableVolumetricLight.IsInitted())
		{
			g_cvarEnableVolumetricLight.Init(g_pLTClient, "EnableVolumetricLight", NULL, 1.0f);
			g_cvarVolumetricLightSlices.Init(g_pLTClient, "VolumetricLightSlices", NULL, DEFAULT_SLICES);
			g_cvarVolumetricLightSliceRes.Init(g_pLTClient, "VolumetricLightSliceRes", NULL, 320.0f);
			g_cvarVolumetricLightShadow.Init(g_pLTClient, "VolumetricLightShadow", NULL, 1.0f);
			g_cvarVolumetricLightShadowRes.Init(g_pLTClient, "VolumetricLightShadowSize", NULL, 128.0f);
			g_cvarVolumetricLightNoise.Init(g_pLTClient, "VolumetricLightNoise", NULL, 1.0f);
			g_cvarVolumetricLightFloat.Init(g_pLTClient, "VolumetricLightFloat", NULL, 1.0f);
		}

		if (!DATABASE_CATEGORY(VolumetricLight).Init())
		{
			Term();
			return false;
		}

		if (!CreateVertexDecl())
		{
			Term();
			return false;
		}

		if (!CreateMaterials())
		{
			Term();
			return false;
		}					

		if (!CreateShellIB())
		{
			Term();
			return false;
		}

		return true;
	}
	
	void Term()
	{
		// Don't over-term
		if (!m_nRefCount)
			return;
		// Only truly terminate when the last volumetric light goes away
		--m_nRefCount;
		if (m_nRefCount)
			return;
			
		// Let go of the vertex decl
		if (m_hVertexDecl)
			g_pLTCustomRender->ReleaseVertexDeclaration(m_hVertexDecl);
		m_hVertexDecl = NULL;
		
		// Let go of our materials
		for (uint32 nLoop = 0; nLoop < LTARRAYSIZE(m_aSliceMaterials); ++nLoop)
		{
			if (m_aSliceMaterials[nLoop])
				g_pLTRenderer->ReleaseMaterialInstance(m_aSliceMaterials[nLoop]);
			m_aSliceMaterials[nLoop] = NULL;
		}
		
		if (m_hShellMaterial)
			g_pLTRenderer->ReleaseMaterialInstance(m_hShellMaterial);
		m_hShellMaterial = NULL;
		
		// Let go of the shell IB
		if (m_hShellIB)
			g_pLTCustomRender->ReleaseIndexBuffer(m_hShellIB);
		m_hShellIB = NULL;
		
		// Let go of the render targets
		ReleaseRenderTargets();

		// Let go of the game database category
		DATABASE_CATEGORY(VolumetricLight).Term();
	}
	
	// Lock a set of render targets, and associate with a token for later release
	bool LockRenderTarget(uint32 nToken, HRENDERTARGET *pShadowBuffer, HRENDERTARGET *pSliceBuffer)
	{
		// Reset our render target list if our resolution has changed
		if ((m_nCurSliceRes != (uint32)g_cvarVolumetricLightSliceRes.GetFloat()) || 
			(m_nCurShadowRes != (uint32)g_cvarVolumetricLightShadowRes.GetFloat()) || 
			(m_bShadowsEnabled != (g_cvarVolumetricLightShadow.GetFloat() != 0.0f)) || 
			(m_bFloatBuffersEnabled != (g_cvarVolumetricLightFloat.GetFloat() != 0.0f)))
		{
			m_nCurSliceRes = (uint32)g_cvarVolumetricLightSliceRes.GetFloat();
			m_nCurShadowRes = (uint32)g_cvarVolumetricLightShadowRes.GetFloat();
			m_bShadowsEnabled = (g_cvarVolumetricLightShadow.GetFloat() != 0);
			m_bFloatBuffersEnabled = (g_cvarVolumetricLightFloat.GetFloat() != 0);
			FlushRenderTargets();
		}

		// Allocate a new target if necessary
		if (m_aFreeList.empty())
		{
			// Add null buffers to the free list			
			m_aFreeList.push_back(m_aShadowBuffers.size());		
			m_aShadowBuffers.push_back(NULL);
			m_aSliceBuffers.push_back(NULL);
			m_aLockTokens.push_back(0);
		}
		
		// Pull a buffer set from the free list
		uint32 nIndex = m_aFreeList.back();
		m_aFreeList.pop_back();
		m_aLockTokens[nIndex] = nToken;
		
		HRENDERTARGET hShadowBuffer = m_aShadowBuffers[nIndex];

		// Allocate a shadow buffer if necessary
		// Note: These aren't allocated until they're requested so we don't allocate shadow
		// buffers if they're turned off.
		if (m_bShadowsEnabled && (hShadowBuffer == NULL))
		{
			LTRESULT nResult = LT_ERROR;
			nResult = g_pLTRenderer->CreateRenderTarget(m_nCurShadowRes, m_nCurShadowRes, eRTO_DepthBuffer, hShadowBuffer);
			if (nResult != LT_OK)
				return false;
			m_aShadowBuffers[nIndex] = hShadowBuffer;
		}

		HRENDERTARGET hSliceBuffer = m_aSliceBuffers[nIndex];
		
		// Allocate a slice buffer (use floating point precision if possible) if necessary
		if (hSliceBuffer == NULL)
		{
			LTRESULT nResult = LT_ERROR;
			
			LTVector2 vSliceRes = GetSliceBufferRes();
			if (m_bFloatBuffersEnabled)
			{
				nResult = g_pLTRenderer->CreateRenderTarget((uint32)vSliceRes.x, (uint32)vSliceRes.y, eRTO_DepthBuffer | eRTO_FloatBuffer, hSliceBuffer);
				if (nResult != LT_OK)
				{
					// Floating point buffers not supported.  Stop trying to allocate them.
					m_bFloatBuffersEnabled = false;
					g_cvarVolumetricLightFloat.SetFloat(0.0f);
				}
			}
			if (!m_bFloatBuffersEnabled)
			{
				nResult = g_pLTRenderer->CreateRenderTarget((uint32)vSliceRes.x, (uint32)vSliceRes.y, eRTO_DepthBuffer, hSliceBuffer);
				if (nResult != LT_OK)
				{
					g_pLTRenderer->ReleaseRenderTarget(hShadowBuffer);
					return false;
				}
			}
			m_aSliceBuffers[nIndex] = hSliceBuffer;
		}

		// Return those buffers
		*pShadowBuffer = hShadowBuffer;
		*pSliceBuffer = hSliceBuffer;
		
		return true;
	}
	
	// Let go of a render target set
	void ReleaseRenderTarget(uint32 nToken)
	{
		// Find the token in our list
		Tuint32List::iterator iTokenEntry = std::find(m_aLockTokens.begin(), m_aLockTokens.end(), nToken);
		if (iTokenEntry == m_aLockTokens.end())
		{
			LTERROR("Invalid lock token provided for release");
			return;
		}
		// Release the token's lock on the render targets
		*iTokenEntry = 0;
		m_aFreeList.push_back(iTokenEntry - m_aLockTokens.begin());
	}

	// Get the current resolution of the slice buffers	
	LTVector2 GetSliceBufferRes() const
	{
		LTVector2 vResult;
		vResult.x = (float)m_nCurSliceRes;
		vResult.y = vResult.x * 0.75f;
		return vResult;
	}
	
	// Get the appropriate material for rendering slices given shadow and noise settings
	HMATERIAL GetSliceMaterial(bool bShadow, bool bNoise, bool bDirectional)
	{
		bShadow &= (g_cvarVolumetricLightShadow.GetFloat() != 0.0f);
		bNoise &= (g_cvarVolumetricLightNoise.GetFloat() != 0.0f);
		return m_aSliceMaterials[(bShadow ? 1 : 0) | (bNoise ? 2 : 0) | (bDirectional ? 4 : 0)];
	}
	
private:
	// Create an index buffer for rendering the shell frustum geometry
	bool CreateShellIB()
	{
		if (g_pLTCustomRender->CreateStaticIndexBuffer(36, m_hShellIB) != LT_OK)
			return false;
			
		uint16 *pShellIBData;
		if (g_pLTCustomRender->LockIndexBuffer(m_hShellIB, 0, 36, pShellIBData) != LT_OK)
			return false;

		uint16 aIndices[36] = {
			// top quad
			0,	1,	5,		5,	4,	0,
			// left quad
			1,	2,	6,		6,	5,	1,
			// bottom quad
			2,	3,	7,		7,	6,	2,
			// right quad
			3,	0,	4,		4,	7,	3,
			// front quad
			2,	1,	0,		0,	3,	2,
			// back quad
			4,	5,	6,		6,	7,	4,
		};
		memcpy(pShellIBData, aIndices, sizeof(aIndices));

		g_pLTCustomRender->UnlockIndexBuffer(m_hShellIB);
		
		return true;
	}

	bool CreateVertexDecl()
	{
		// Create a vertex declaration for our vertex format
		SVertexDeclElement VertexDecl[] =
		{
			{ 0, eVertexDeclDataType_Float3, eVertexDeclUsage_Position, 0 },
			{ 0, eVertexDeclDataType_Float4, eVertexDeclUsage_Color, 0 },
			{ 0, eVertexDeclDataType_Float4, eVertexDeclUsage_TexCoord, 0 },
		};

		if (g_pLTCustomRender->CreateVertexDeclaration(LTARRAYSIZE(VertexDecl), VertexDecl, m_hVertexDecl) != LT_OK)
			return false;

		return true;
	}

	// Create the materials based on the database
	bool CreateMaterials()
	{
		HRECORD hRecord = DATABASE_CATEGORY(VolumetricLight).GetRecordByName("Default");
		if (hRecord == NULL)
			return false;
			
		HATTRIBUTE hSliceAttribute = DATABASE_CATEGORY(VolumetricLight).GetAttribute(hRecord, "SliceMaterials");
		
		for (uint32 nLoop = 0; nLoop < LTARRAYSIZE(m_aSliceMaterials); ++nLoop)
		{
			const char *pMaterialFile = g_pLTDatabase->GetString(hSliceAttribute, nLoop, NULL);
			if (pMaterialFile == NULL)
				return false;
			m_aSliceMaterials[nLoop] = g_pLTRenderer->CreateMaterialInstance(pMaterialFile);
			if (m_aSliceMaterials[nLoop] == NULL)
				return false;
		}

		m_hShellMaterial = g_pLTRenderer->CreateMaterialInstance(GETCATRECORDATTRIB(VolumetricLight, hRecord, ShellMaterial));
		if (m_hShellMaterial == NULL)
			return false;
		
		return true;
	}
	
	// Load a single material (And display appropriate error messages if necessary.)
	HMATERIAL LoadMaterial(const char *pFileName)
	{
		HMATERIAL hResult = g_pLTRenderer->CreateMaterialInstance(pFileName);
		if (hResult == NULL)
		{
			g_pLTClient->CPrint("VolumetricLightFX: Error loading material file %s", pFileName);
		}
		return NULL;
	}

	// Let go of all current render targets	
	void FlushRenderTargets()
	{
		TRenderTargetList::iterator iCurTarget;
		for (iCurTarget = m_aShadowBuffers.begin(); iCurTarget != m_aShadowBuffers.end(); ++iCurTarget)
		{
			g_pLTRenderer->ReleaseRenderTarget(*iCurTarget);
			*iCurTarget = NULL;
		}
		for (iCurTarget = m_aSliceBuffers.begin(); iCurTarget != m_aSliceBuffers.end(); ++iCurTarget)
		{
			g_pLTRenderer->ReleaseRenderTarget(*iCurTarget);
			*iCurTarget = NULL;
		}
	}

	// Clear out the render target list
	void ReleaseRenderTargets()
	{
		FlushRenderTargets();
				
		m_aShadowBuffers.resize(0);
		m_aSliceBuffers.resize(0);
		
		m_aLockTokens.resize(0);
		m_aFreeList.resize(0);
	}
	
	// Volumetric lighting database category
	BEGIN_DATABASE_CATEGORY(VolumetricLight, "FX/VolumetricLight")
		DEFINE_GETRECORDATTRIB(ShellMaterial, char const*);
		DEFINE_GETRECORDATTRIB(SliceMaterial, char const*);
	END_DATABASE_CATEGORY();
	
private:
	// Number of volumetric light effects using the resource manager
	uint32 m_nRefCount;

	// The render target lists
	typedef std::vector<HRENDERTARGET> TRenderTargetList;
	TRenderTargetList m_aShadowBuffers;
	TRenderTargetList m_aSliceBuffers;
	Tuint32List m_aLockTokens;
	Tuint32List m_aFreeList;
	
	// Current settings, cached for determining when the console variables change
	uint32 m_nCurShadowRes, m_nCurSliceRes;
	bool m_bShadowsEnabled, m_bFloatBuffersEnabled;
	
public:
	// Structure representing a vertex used by the volumetric lighting (for both slices & the shell)
	// Note: the color is full-precision to allow two things: better low-end precision on the slices,
	// and greater range for the shell.
	struct SVertex
	{
		LTVector m_vPos;
		LTVector4 m_vColor;
		LTVector4 m_vLightSpace;
	};

	// Vertex declaration to use when rendering	
	HVERTEXDECL m_hVertexDecl;
	
	// Materials to use for rendering
	HMATERIAL m_aSliceMaterials[8], m_hShellMaterial;
	
	// Index buffer to use for rendering the shell frustum
	HINDEXBUFFER m_hShellIB;
	
};

// The volumetric lighting resource manager
static CVLFXResourceMgr g_VLFXResourceMgr;

//////////////////////////////////////////////////////////////////////////
// CVolumetricLightFX implementation

CVolumetricLightFX::CVolumetricLightFX() :
	CSpecialFX(),
	m_hSliceMaterial(NULL),
	m_hBaseSliceMaterial(NULL),
	m_hShellMaterial(NULL),
	m_bShellWasDrawn(false),
	m_bSliceWasDrawn(false),
	m_bShellWasDrawnLastFrame(false),
	m_bMgrRef(false),
	m_bRenderTargetRef(false),
	m_tCurTransform(LTVector::GetIdentity(), LTRotation::GetIdentity()),
	m_vCurFOV(0.0f, 0.0f), 
	m_vCurClip(0.0f, 0.0f),
	m_bDirectional(false)
{
}

CVolumetricLightFX::~CVolumetricLightFX()
{
	ReleaseMaterials();
	ReleaseRenderTargets();
	if (m_bMgrRef)
		g_VLFXResourceMgr.Term();
}

bool CVolumetricLightFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	// Validate the parameters
    if (!psfxCreateStruct) 
		return false;

	// Initialize the base object
	if(!CSpecialFX::Init(psfxCreateStruct))
		return false;

	// Save the creation structure
	m_CS = *(VOLUMETRICLIGHTCREATESTRUCT*)psfxCreateStruct;

    return true;
}

bool CVolumetricLightFX::CreateObject(ILTClient *pClientDE)
{
	if (!CSpecialFX::CreateObject(pClientDE))
		return false;

	// Make sure our resources are initialized
	if (!g_VLFXResourceMgr.Init())
	{
		g_pLTClient->CPrint("VolumetricLightFX: Error initializing resource manager!");
		return false;
	}
	m_bMgrRef = true;

	ObjectCreateStruct cOCS;

	//////////////////////////////////////////////////////////////////////////
	// Set up the slice object	
	cOCS.m_ObjectType = OT_CUSTOMRENDER;

	g_pLTClient->GetObjectPos(m_hServerObject, &(cOCS.m_Pos));

	cOCS.m_Flags = 0;
	cOCS.m_Flags2 = FLAG2_FORCETRANSLUCENT;

	m_hSliceObject = g_pLTClient->CreateObject(&cOCS);
	if (m_hSliceObject == INVALID_HOBJECT) 
		return false;

	g_pLTCustomRender->SetRenderCallback(m_hSliceObject, RenderSlicesCallback);
	g_pLTCustomRender->SetCallbackUserData(m_hSliceObject, this);
	g_pLTCustomRender->SetRenderingSpace(m_hSliceObject, eRenderSpace_View);

	//////////////////////////////////////////////////////////////////////////
	// Set up the shell object	
	g_pLTClient->GetObjectPos(m_hServerObject, &(cOCS.m_Pos));

	cOCS.m_Flags = FLAG_VISIBLE;
	cOCS.m_Flags2 = FLAG2_FORCETRANSLUCENT;

	m_hShellObject = g_pLTClient->CreateObject(&cOCS);
	if (m_hShellObject == INVALID_HOBJECT) 
		return false;

	g_pLTCustomRender->SetRenderCallback(m_hShellObject, RenderShellCallback);
	g_pLTCustomRender->SetCallbackUserData(m_hShellObject, this);
	g_pLTCustomRender->SetRenderingSpace(m_hShellObject, eRenderSpace_World);

	UpdateLightProperties();

	UpdateMaterials();
	
	return true;
}


bool CVolumetricLightFX::Update()
{
	if( !m_pClientDE || !m_hServerObject )
		return false;

	if( m_bWantRemove )
		return false;

	//track our performance
	CTimedSystemBlock TimingBlock(g_tsClientVolLights);

	// KLS 1/11/05 - Comment out following line as the volumetic light console variable
	// is being used as a boolean instead of an LOD...Added following code to better
	// support how this console variable is being used (and how content has been setup)
	//
	// OLD CODE: bool bEnabled = (g_cvarEnableVolumetricLight.GetFloat() > m_CS.m_eLOD);
	//
	bool bEnabled = (g_cvarEnableVolumetricLight.GetFloat() && eEngineLOD_Never != m_CS.m_eLOD);

	// Test for server object visibility
	uint32 nServerFlags;
	g_pCommonLT->GetObjectFlags(m_hServerObject, OFT_Flags, nServerFlags);
	bEnabled &= (nServerFlags & FLAG_VISIBLE) != 0;
	
	// Let go of our render targets from last frame
	// Note: This is delayed until the "next" frame so nobody else uses our render targets during the rendering loop
	ReleaseRenderTargets();
	
	if (bEnabled)
	{
		/*
			Rendering overview
			
			This basically happens in three phases:
				- Render the shadow buffer
				- Render the slices
				- Render the shell
				
			The shadow buffer is filled by rendering the FogVolume_Depth technique from
			the perspective of the light.  This puts depth values in the depth render
			target in the fog volume overlapped precision format.  (Future implementations
			are encouraged to use a different technique if a "FillDepthBuffer" technique
			becomes available that's not wired for use by the fog volumes...)
			
			The slices are rendered from the perspective of the player camera, and are 
			accumulated into an off-screen buffer.  Accumulating into an off-screen buffer
			allows rendering at a lower resolution (helping fill rate issues immensely), as
			well as allowing greater precision by writing to each color channel in sequence
			using a color mask.  (These are then accumulated in the shell pixel shader using
			a dot product.)  The shadow buffer is used by this pass for adding shadowing 
			effects.  Additionally, this pass uses clipping planes to further reduce fill 
			rate use.
			
			The above happens as part of the object's update, which constitutes the majority
			of this function.  During the actual scene rendering, the back-facing polygons 
			of the frustum described by the spot light (referred to as the "shell") are 
			rendered without z-buffering.  This shader samples the slice buffer (including 
			anti-alias filtering), normalizes the 0-1 range to match the intensity based 
			on the maximum view depth (for the VolumetricDepth property), adjusts the color, 
			and applies a pre-multiplied alpha blend.  (For selecting between additive and 
			alpha blended effects.)
			
			Two very important mechanisms are worth pointing out.  First, the depth and
			slice buffers do not get rendered unless the shell was rendered the last frame.
			This means that the first frame the shell receives a render call, it does not
			render anything to avoid using an invalid buffer.  Second, the shell will not
			be rendered if it turns out the slices were not rendered.  This allows early-
			out behavior on the shell rendering if the slices were out of frustum for the
			current frame.
		*/
	
		UpdateLightProperties();

		UpdateMaterials();
		
		// Make sure we're visible
		g_pCommonLT->SetObjectFlags(m_hShellObject, OFT_Flags, FLAG_VISIBLE, FLAG_VISIBLE);

		// Only render if the shell was drawn last frame
		bool bRender = m_bShellWasDrawn;
		
		HRENDERTARGET hShadowTarget = NULL, hSliceTarget = NULL;
		
		if (bRender)
		{
			// Get a render target pair from the resource mgr
			if (g_VLFXResourceMgr.LockRenderTarget((uint32)this, &hShadowTarget, &hSliceTarget))
			{
				// Install the render targets
				g_pLTRenderer->SetInstanceParamRenderTarget(m_hSliceMaterial, "tVLDepthMap", hShadowTarget);
				g_pLTRenderer->SetInstanceParamRenderTarget(m_hShellMaterial, "tSliceMap", hSliceTarget);
				LTVector2 vSliceRes = g_VLFXResourceMgr.GetSliceBufferRes();
				g_pLTRenderer->SetInstanceParamFloat(m_hShellMaterial, "fSliceResX", vSliceRes.x);
				g_pLTRenderer->SetInstanceParamFloat(m_hShellMaterial, "fSliceResY", vSliceRes.y);
				// Remember we're using a render target
				m_bRenderTargetRef = true;
			}
			else
			{
				// If we can't lock a render target, we can't render
				g_pCommonLT->SetObjectFlags(m_hShellObject, OFT_Flags, 0, FLAG_VISIBLE);
				bRender = false;
			}
		}
				
		// Don't update the offscreen buffers unless we were rendered last frame
		if (bRender)
		{
			if (LT_OK == g_pLTClient->GetRenderer()->Start3D())
			{
				HOBJECT aObjects[1];
				
				// Render the depth buffer
				if (m_CS.m_bShadow && g_cvarVolumetricLightShadow.GetFloat() != 0.0f)
				{
					g_pLTRenderer->SetRenderTarget(hShadowTarget);
					g_pLTRenderer->ClearRenderTarget(CLEARRTARGET_ALL, 0xFFFFFFFF);
					// Adjust the z range
					float fOldNearZ = g_pLTClient->GetConsoleVariableFloat(g_pLTClient->GetConsoleVariable("NearZ"));
					float fOldFarZ = g_pLTClient->GetConsoleVariableFloat(g_pLTClient->GetConsoleVariable("FarZ"));
					g_pLTClient->SetConsoleVariableFloat("NearZ", LTMAX(m_vCurClip.x, 0.001f));
					g_pLTClient->SetConsoleVariableFloat("FarZ", m_vCurClip.y);
					// Turn off sky rendering
					float fOldDrawSky = g_pLTClient->GetConsoleVariableFloat(g_pLTClient->GetConsoleVariable("DrawSky"));
					g_pLTClient->SetConsoleVariableFloat("DrawSky", 0.0f);
					// Calculate depth values
					LTRect2f rViewport(0.0f, 0.0f, 1.0f, 1.0f);
					LTVector2 vCameraFOV = m_vCurFOV;
					if (!m_bDirectional)
						vCameraFOV *= 2.0f;
					g_pLTRenderer->RenderCamera(m_tCurTransform, m_tCurTransform.m_vPos, vCameraFOV, m_bDirectional, rViewport, NULL, "FogVolume_Depth");
					// Reset everything we changed
					g_pLTClient->SetConsoleVariableFloat("NearZ", fOldNearZ);
					g_pLTClient->SetConsoleVariableFloat("FarZ", fOldFarZ);
					g_pLTClient->SetConsoleVariableFloat("DrawSky", fOldDrawSky);
				}

				// Reset the camera viewport so we don't get letterboxing issues
				HOBJECT hCamera = g_pPlayerMgr->GetPlayerCamera()->GetCamera();
				LTRect2f rOldViewport;
				g_pLTClient->GetCameraRect(hCamera, rOldViewport);
				g_pLTClient->SetCameraRect(hCamera, LTRect2f(0.0f, 0.0f, 1.0f, 1.0f));
				
				g_pLTRenderer->SetRenderTarget(hSliceTarget);
				g_pLTRenderer->ClearRenderTarget(CLEARRTARGET_ALL, 0);
				// Fill the z-buffer
				g_pLTRenderer->RenderCamera(hCamera, "Ambient");
				// We didn't actually want the color buffer, but can't disable that right now
				g_pLTRenderer->ClearRenderTarget(CLEARRTARGET_COLOR, 0);

				// Track whether or not the slices were successfully drawn
				m_bSliceWasDrawn = false;
				
				// Render the slices		
				g_pCommonLT->SetObjectFlags(m_hSliceObject, OFT_Flags, FLAG_VISIBLE, FLAG_VISIBLE);
				// Update the depth scale
				// Note: This adjusts for the fact that the fog depth fill only uses farz for the range of the values
				float fDepthScale = m_vCurClip.y / (m_vCurClip.y - m_vCurClip.x);
				g_pLTRenderer->SetInstanceParamFloat(m_hSliceMaterial, "fDepthScale", fDepthScale);
				aObjects[0] = m_hSliceObject;
				g_pLTRenderer->RenderObjects(hCamera, aObjects, LTARRAYSIZE(aObjects));
				g_pCommonLT->SetObjectFlags(m_hSliceObject, OFT_Flags, 0, FLAG_VISIBLE);
				
				// If the slices weren't drawn, skip drawing the shell
				m_bShellWasDrawn = m_bSliceWasDrawn;

				// Go back to screen rendering
				g_pLTRenderer->SetRenderTargetScreen();
				
				// Reset the camera viewport
				g_pLTClient->SetCameraRect(hCamera, rOldViewport);
				
				g_pLTClient->GetRenderer()->End3D();
			}
		}
	}
	else
	{
		// If we're not enabled, hide so we don't get rendered
		g_pCommonLT->SetObjectFlags(m_hShellObject, OFT_Flags, 0, FLAG_VISIBLE);
	}

	// Rotate the last/cur draw flags	
	m_bShellWasDrawnLastFrame = m_bShellWasDrawn;
	m_bShellWasDrawn = false;
	
    return true;
}

void CVolumetricLightFX::RenderSlices(ILTCustomRenderCallback* pInterface, const LTRigidTransform& tCamera)
{
	if (pInterface->SetVertexDeclaration(g_VLFXResourceMgr.m_hVertexDecl) != LT_OK)
		return;
	
	// Remember that slices were drawn
	m_bSliceWasDrawn = true;
	
	// Get the light frustum
	LTVector aLightFrustum[8];
	GetLightFrustumVertices(aLightFrustum, sizeof(aLightFrustum[0]));
	
	// Set up the clipping planes
	
	// Save off any pre-existing planes
	LTPlane aCurPlanes[6];
	uint32 nOldNumPlanes = LTARRAYSIZE(aCurPlanes);
	pInterface->GetClipPlanes(aCurPlanes, nOldNumPlanes);
	
	// Set up the clipping planes
	LTVector vLightForward = m_tCurTransform.m_rRot.Forward();
	// Add the planes in the order we would prefer they get dropped out
	// Far
	if (nOldNumPlanes < 1)
		aCurPlanes[0] = LTPlane(-vLightForward, m_tCurTransform.m_vPos + vLightForward * m_vCurClip.y);
	// Top
	if (nOldNumPlanes < 2)
		aCurPlanes[1] = LTPlane((aLightFrustum[7] - aLightFrustum[4]).Cross(aLightFrustum[0] - aLightFrustum[4]).GetUnit(), aLightFrustum[0]);
	// Bottom
	if (nOldNumPlanes < 3)
		aCurPlanes[2] = LTPlane((aLightFrustum[5] - aLightFrustum[6]).Cross(aLightFrustum[2] - aLightFrustum[6]).GetUnit(), aLightFrustum[2]);
	// Left
	if (nOldNumPlanes < 4)
		aCurPlanes[3] = LTPlane((aLightFrustum[6] - aLightFrustum[7]).Cross(aLightFrustum[3] - aLightFrustum[7]).GetUnit(), aLightFrustum[3]);
	// Right
	if (nOldNumPlanes < 5)
		aCurPlanes[4] = LTPlane((aLightFrustum[4] - aLightFrustum[5]).Cross(aLightFrustum[1] - aLightFrustum[5]).GetUnit(), aLightFrustum[1]);
	// Near -- note : Must come last, as this is the most likely plane to cause artifacts, due to back-projection.
	if (nOldNumPlanes < 6)
		aCurPlanes[5] = LTPlane(vLightForward, m_tCurTransform.m_vPos + vLightForward * m_vCurClip.x);
	pInterface->SetClipPlanes(aCurPlanes, 6);
	
	// Transform the light frustum into view space
	LTMatrix3x4 mWorldToView;
	tCamera.GetInverse().ToMatrix(mWorldToView);
	uint32 nCurFrustumVertex;
	for (nCurFrustumVertex = 0; nCurFrustumVertex < 8; ++nCurFrustumVertex)
	{
		aLightFrustum[nCurFrustumVertex] = mWorldToView.Transform(aLightFrustum[nCurFrustumVertex]);
	}

	// Calculate the extents
	LTVector vMinExtents(aLightFrustum[0]), vMaxExtents(aLightFrustum[0]);
	for (nCurFrustumVertex = 1; nCurFrustumVertex < 8; ++nCurFrustumVertex)
	{
		vMinExtents.Min(aLightFrustum[nCurFrustumVertex]);
		vMaxExtents.Max(aLightFrustum[nCurFrustumVertex]);
	}

	// Compose the transform to get view-space positions into world space, and then into projected light space
	LTMatrix3x4 mViewToWorld;
	tCamera.ToMatrix(mViewToWorld);
	LTMatrix3x4 mInvLightTransform;
	m_tCurTransform.GetInverse().ToMatrix(mInvLightTransform);
	LTMatrix mLightTransform = m_mLightPerspectiveTransform * (mInvLightTransform * mViewToWorld);

	uint32 nNumSlices = (uint32)g_cvarVolumetricLightSlices.GetFloat();

	// Lock the vertex buffer
	SDynamicVertexBufferLockRequest LockRequest;
	if (pInterface->LockDynamicVertexBuffer(nNumSlices * 4, LockRequest) != LT_OK)
		return;

	// Fill the vertex buffer with slices
	CVLFXResourceMgr::SVertex* pVertexBuffer = (CVLFXResourceMgr::SVertex*)LockRequest.m_pData;
	CVLFXResourceMgr::SVertex* pCurVertex = pVertexBuffer;
	float fSliceZOffset = (vMaxExtents.z - vMinExtents.z) / (float)(nNumSlices + 1);
	float fCurZ = vMaxExtents.z - fSliceZOffset * 0.5f;
	// Note: The intensity for each slice would normally be 4/nNumSlices because of color channel overlapping.
	// The extra *2 is to compensate for the slices usually being diagonal relative to any pixel.  This causes
	// the volume to have a tendency to saturate slightly, but allows more dynamic range for the most of the
	// slices.
	float fIntensity = 8.0f / (float)nNumSlices;
	for (uint32 nCurSlice = 0; nCurSlice < nNumSlices; ++nCurSlice)
	{
		LTVector4 vLocalColor(0.0f, 0.0f, 0.0f, 0.0f);
		vLocalColor[nCurSlice & 3] = fIntensity;
		// Write out a slice
		pCurVertex->m_vPos.Init(vMinExtents.x, vMaxExtents.y, fCurZ);
		pCurVertex->m_vColor = vLocalColor;
		mLightTransform.Apply4x4(LTVector4(pCurVertex->m_vPos, 1.0f), pCurVertex->m_vLightSpace);
		++pCurVertex;
		pCurVertex->m_vPos.Init(vMaxExtents.x, vMaxExtents.y, fCurZ);
		pCurVertex->m_vColor = vLocalColor;
		mLightTransform.Apply4x4(LTVector4(pCurVertex->m_vPos, 1.0f), pCurVertex->m_vLightSpace);
		++pCurVertex;
		pCurVertex->m_vPos.Init(vMaxExtents.x, vMinExtents.y, fCurZ);
		pCurVertex->m_vColor = vLocalColor;
		mLightTransform.Apply4x4(LTVector4(pCurVertex->m_vPos, 1.0f), pCurVertex->m_vLightSpace);
		++pCurVertex;
		pCurVertex->m_vPos.Init(vMinExtents.x, vMinExtents.y, fCurZ);
		pCurVertex->m_vColor = vLocalColor;
		mLightTransform.Apply4x4(LTVector4(pCurVertex->m_vPos, 1.0f), pCurVertex->m_vLightSpace);
		++pCurVertex;
		
		// Move forward
		fCurZ -= fSliceZOffset;
	}
	pInterface->UnlockAndBindDynamicVertexBuffer(LockRequest);
	
	// Draw the quads
	pInterface->BindQuadIndexStream();
	pInterface->RenderIndexed(eCustomRenderPrimType_TriangleList, 0, nNumSlices * 6, LockRequest.m_nStartIndex, 0, nNumSlices * 4);
	
	// Turn off the new clipping planes
	pInterface->SetClipPlanes(aCurPlanes, nOldNumPlanes);
}

void CVolumetricLightFX::RenderShell(ILTCustomRenderCallback* pInterface, const LTRigidTransform& tCamera)
{
	// Remember we're visible
	m_bShellWasDrawn = true;
	
	// If we weren't visible last frame, we won't have the slice data, so don't render this frame.
	if (!m_bShellWasDrawnLastFrame)
		return;
		
	// Get the alpha
	float fAlpha;
	g_pLTClient->GetObjectColor(m_hShellObject, NULL, NULL, NULL, &fAlpha);
	// Don't render if it's black anyway
	if (fAlpha < (1.0f / 255.0f))
		return;

	if (pInterface->SetVertexDeclaration(g_VLFXResourceMgr.m_hVertexDecl) != LT_OK)
		return;

	// Lock the vertex buffer
	SDynamicVertexBufferLockRequest LockRequest;
	if (pInterface->LockDynamicVertexBuffer(8, LockRequest) != LT_OK)
		return;

	// Fill the vertex buffer
	CVLFXResourceMgr::SVertex* pVertexBuffer = (CVLFXResourceMgr::SVertex*)LockRequest.m_pData;
	CVLFXResourceMgr::SVertex* pCurVertex = pVertexBuffer;
	// Set the positions
	GetLightFrustumVertices(&pVertexBuffer->m_vPos, sizeof(CVLFXResourceMgr::SVertex));
	// Get the depth range
	LTVector2 vDepthRange(pVertexBuffer->m_vPos.z, pVertexBuffer->m_vPos.z);
	uint32 nLoop;
	for (nLoop = 1; nLoop < 8; ++nLoop)
	{
		vDepthRange.x = LTMIN(vDepthRange.x, pVertexBuffer[nLoop].m_vPos.z);
		vDepthRange.y = LTMAX(vDepthRange.y, pVertexBuffer[nLoop].m_vPos.z);
	}
	// Set the colors
	LTVector vColor = m_CS.m_vColor;
	float fIntensity = fAlpha * (vDepthRange.y - vDepthRange.x) / m_CS.m_fDepth;
	for (nLoop = 0; nLoop < 8; ++nLoop)
	{
		pVertexBuffer[nLoop].m_vColor.Init(VEC_EXPAND(vColor), fIntensity);
	}

	pInterface->UnlockAndBindDynamicVertexBuffer(LockRequest);

	// Draw the quads
	pInterface->BindIndexStream(g_VLFXResourceMgr.m_hShellIB);
	pInterface->RenderIndexed(eCustomRenderPrimType_TriangleList, 0, 36, LockRequest.m_nStartIndex, 0, 8);
}

static LTVector& GetVertex(LTVector *pBase, uint32 nStride, uint32 nIndex)
{
	return *(LTVector*)(((uint8*)pBase) + nStride * nIndex);
}

void CVolumetricLightFX::GetLightFrustumVertices(LTVector* pVertices, uint32 nStride)
{
	// Get the exterior points of the light frustum
	LTVector vLightForward, vLightRight, vLightUp;
	m_tCurTransform.m_rRot.GetVectors(vLightRight, vLightUp, vLightForward);
	vLightRight *= m_vCurFOV.x;
	vLightUp *= m_vCurFOV.y;
	
	LTVector2 vFOVScale, vForwardScale;
	if (!m_bDirectional)
	{
		vFOVScale = m_vCurClip;
		vForwardScale = LTVector2(1.0f, 1.0f);
	}
	else
	{
		vFOVScale = LTVector2(1.0f, 1.0f);
		vForwardScale = m_vCurClip;
	}

	GetVertex(pVertices, nStride, 0) = m_tCurTransform.m_vPos + (vLightForward * vForwardScale.x + vLightRight + vLightUp) * vFOVScale.x;
	GetVertex(pVertices, nStride, 1) = m_tCurTransform.m_vPos + (vLightForward * vForwardScale.x + -vLightRight + vLightUp) * vFOVScale.x;
	GetVertex(pVertices, nStride, 2) = m_tCurTransform.m_vPos + (vLightForward * vForwardScale.x + -vLightRight + -vLightUp) * vFOVScale.x;
	GetVertex(pVertices, nStride, 3) = m_tCurTransform.m_vPos + (vLightForward * vForwardScale.x + vLightRight + -vLightUp) * vFOVScale.x;
	GetVertex(pVertices, nStride, 4) = m_tCurTransform.m_vPos + (vLightForward * vForwardScale.y + vLightRight + vLightUp) * vFOVScale.y;
	GetVertex(pVertices, nStride, 5) = m_tCurTransform.m_vPos + (vLightForward * vForwardScale.y + -vLightRight + vLightUp) * vFOVScale.y;
	GetVertex(pVertices, nStride, 6) = m_tCurTransform.m_vPos + (vLightForward * vForwardScale.y + -vLightRight + -vLightUp) * vFOVScale.y;
	GetVertex(pVertices, nStride, 7) = m_tCurTransform.m_vPos + (vLightForward * vForwardScale.y + vLightRight + -vLightUp) * vFOVScale.y;
}

void CVolumetricLightFX::ReleaseRenderTargets()
{
	if (!m_bRenderTargetRef)
		return;

	g_pLTRenderer->SetInstanceParamRenderTarget(m_hSliceMaterial, "tVLDepthMap", NULL);
	g_pLTRenderer->SetInstanceParamRenderTarget(m_hShellMaterial, "tSliceMap", NULL);
	
	g_VLFXResourceMgr.ReleaseRenderTarget((uint32)this);
	m_bRenderTargetRef = false;
}

void CVolumetricLightFX::UpdateLightProperties()
{
	// Mirror the light object's alpha
	float fAlpha;
	g_pLTClient->GetObjectColor(m_hServerObject, NULL, NULL, NULL, &fAlpha);
	g_pLTClient->SetObjectColor(m_hShellObject, 1.0f, 1.0f, 1.0f, fAlpha);

	EEngineLightType eLightType;
	g_pLTClient->GetLightType(m_hServerObject, eLightType);
	// Get the current parameters of the light
	bool bDirectional = eLightType == eEngineLight_Directional;
	LTVector2 vFOV, vClip;
	if (!bDirectional)
	{
		vClip.y = m_CS.m_fFarZ;
		g_pLTClient->GetLightSpotInfo(m_hServerObject, vFOV.x, vFOV.y, vClip.x);
	}
	else if (bDirectional)
	{
		LTVector vDims;
		g_pLTClient->GetLightDirectionalDims(m_hServerObject, vDims);
		vFOV.x = vDims.x;
		vFOV.y = vDims.y;
		vClip.x = 0;
		vClip.y = vDims.z;
	}
	
	LTRigidTransform tTransform;
	g_pLTClient->GetObjectTransform(m_hServerObject, &tTransform);

	// Don't update the properties if they haven't changed
	if ((tTransform.m_vPos == m_tCurTransform.m_vPos) &&
		(tTransform.m_rRot == m_tCurTransform.m_rRot) &&
		(vFOV == m_vCurFOV) &&
		(vClip == m_vCurClip) &&
		(bDirectional == m_bDirectional))
		return;
	
	// Remember the new properties	
	m_tCurTransform = tTransform;
	m_vCurFOV = vFOV;
	m_vCurClip = vClip;
	m_bDirectional = bDirectional;
	
	// Make sure we follow the server object
	g_pLTClient->SetObjectTransform(m_hShellObject, tTransform);
	g_pLTClient->SetObjectTransform(m_hSliceObject, tTransform);

	// Calculate the bounding box for the light
	LTVector aLightFrustum[8];
	GetLightFrustumVertices(aLightFrustum, sizeof(aLightFrustum[0]));
	LTVector vMinExtents(aLightFrustum[0]), vMaxExtents(aLightFrustum[0]);
	for (uint32 nLoop = 1; nLoop < 8; ++nLoop)
	{
		vMinExtents.Min(aLightFrustum[nLoop]);
		vMaxExtents.Max(aLightFrustum[nLoop]);
	}
	g_pLTCustomRender->SetVisBoundingBox(m_hShellObject, vMinExtents - tTransform.m_vPos, vMaxExtents - tTransform.m_vPos);

	// Compose a perspective matrix for the space of the light
	if (!m_bDirectional)
	{
		float fQ = 1.0f / (vClip.y - vClip.x);
		m_mLightPerspectiveTransform.Init(
			1/tanf(vFOV.x), 0.0f, 0.0f, 0.0f,
			0.0f, 1/tanf(vFOV.y), 0.0f, 0.0f,
			0.0f, 0.0f, fQ, fQ * -vClip.x,
			0.0f, 0.0f, 1.0f, 0.0f
			);
	}
	else // m_bDirectional
	{
		float fQ = 1.0f / (vClip.y - vClip.x);
		m_mLightPerspectiveTransform.Init(
			1.0f / vFOV.x, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f / vFOV.y, 0.0f, 0.0f,
			0.0f, 0.0f, fQ, -vClip.x * fQ,
			0.0f, 0.0f, 0.0f, 1.0f
			);
	}
	// Adjust for texture space
	LTMatrix3x4 mAdj(	0.5f, 0.0f, 0.0f, 0.5f,
						0.0f, -0.5f, 0.0f, 0.5f,
						0.0f, 0.0f, 1.0f, 0.0f);

	m_mLightPerspectiveTransform = mAdj * m_mLightPerspectiveTransform;
}

void CVolumetricLightFX::ReleaseMaterials()
{
	if (m_hSliceMaterial)
		g_pLTRenderer->ReleaseMaterialInstance(m_hSliceMaterial);
	m_hSliceMaterial = NULL;
	if (m_hShellMaterial)
		g_pLTRenderer->ReleaseMaterialInstance(m_hShellMaterial);
	m_hShellMaterial = NULL;
}

void CVolumetricLightFX::UpdateMaterials()
{
	// See if we're already using the right material
	HMATERIAL hSliceMaterial = g_VLFXResourceMgr.GetSliceMaterial(m_CS.m_bShadow, m_CS.m_fNoiseIntensity != 0.0f, m_bDirectional);
	if (m_hBaseSliceMaterial == hSliceMaterial)
		return;
	m_hBaseSliceMaterial = hSliceMaterial;
		
	ReleaseMaterials();

	// Clone the slice material and set up the parameters	
	m_hSliceMaterial = g_pLTRenderer->CloneMaterialInstance(m_hBaseSliceMaterial);
	HTEXTURE hCookieTexture;
	g_pLTClient->GetTextureMgr()->CreateTextureFromFile(hCookieTexture, m_CS.m_sTexture.c_str());
	g_pLTRenderer->SetInstanceParamTexture(m_hSliceMaterial, "tCookieMap", hCookieTexture);
	g_pLTClient->GetTextureMgr()->ReleaseTexture(hCookieTexture);
	g_pLTRenderer->SetInstanceParamFloat(m_hSliceMaterial, "fNoiseScale", m_CS.m_fNoiseScale);
	g_pLTRenderer->SetInstanceParamFloat(m_hSliceMaterial, "fNoiseIntensity", m_CS.m_fNoiseIntensity);
	g_pLTRenderer->SetInstanceParamFloat(m_hSliceMaterial, "fDistanceAttenuationScale", m_CS.m_fAttenuation);
	g_pLTCustomRender->SetMaterial(m_hSliceObject, m_hSliceMaterial);
	
	if (m_bDirectional)
	{
		char aAttenuationTexture[MAX_PATH];
		g_pLTClient->GetLightAttenuationTexture(m_hServerObject, aAttenuationTexture, LTARRAYSIZE(aAttenuationTexture));
		HTEXTURE hAttenuationTexture;
		g_pLTClient->GetTextureMgr()->CreateTextureFromFile(hAttenuationTexture, aAttenuationTexture);
		g_pLTRenderer->SetInstanceParamTexture(m_hSliceMaterial, "tAttenuationMap", hAttenuationTexture);
		g_pLTClient->GetTextureMgr()->ReleaseTexture(hAttenuationTexture);
	}

	// Clone the shell material and set up the parameters	
	m_hShellMaterial = g_pLTRenderer->CloneMaterialInstance(g_VLFXResourceMgr.m_hShellMaterial);
	g_pLTRenderer->SetInstanceParamFloat(m_hShellMaterial, "fAlphaBlend", m_CS.m_bAdditive ? 0.0f : 1.0f);
	g_pLTCustomRender->SetMaterial(m_hShellObject, m_hShellMaterial);

}
