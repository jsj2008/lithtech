//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//

#include "bdefs.h"
#include "dedit.h"
#include "draw_d3d.h"
#include "d3d_texturemgr.h"
#include "projectbar.h"
#include "cyclemgr.h"
#include "lightmapdefs.h"
#include "meshshapelist.h"
#include "meshshape.h"
#include "geomroutines.h"
#include "optionsmodels.h"
#include "tdguard.h"


// ------------------------------------------------------------------------ //
// Structures.
// ------------------------------------------------------------------------ //

//#define TLVERTEX_FORMAT (D3DFVF_XYZRHW | D3DFVF_TEX1 | D3DFVF_DIFFUSE | D3DFVF_SPECULAR)
#define DRAWPRIM_FLAGS	(D3DDP_DONOTCLIP|D3DDP_DONOTLIGHT|D3DDP_DONOTUPDATEEXTENTS)

#define RMFLAG_VIDEOMEMORY	1	// Use video memory surfaces.
#define RMFLAG_DONTUSE		2	// Don't try to use this mode.

typedef struct RenderModeInfo_t
{
	CLSID	*m_clsid;
	char	*m_ModeName;
	DWORD	m_Flags;
} RenderModeInfo;


// Vertex borders aren't drawn when vertex is smaller than this number.
#define VERT_SIZE_BORDER_CUTOFF 2.0f

#define VIEW pRender->m_pView
#define LIGHTMAP_BIAS_AMT	-1.0f


// ------------------------------------------------------------------------ //
// Globals.
// ------------------------------------------------------------------------ //

static DLink g_Direct3dDrawMgrs(LTLink_Init);


// Set by the texture manager.
extern SDWORD g_bBilinear;


float g_TextureMipScale;
float g_uMul, g_vMul;

float g_fDetailTextureScale;
float g_fDetailTextureAngleCos;
float g_fDetailTextureAngleSin;

static float g_xMax, g_yMax;

static int g_WantedDevice, g_DeviceCount;
static GUID *g_pGuid;
static BOOL g_bFoundDevice;
static DWORD g_DriverCaps;

int g_nNames, g_nMaxNames;
char **g_pNames;


// Render mode infos, in order of preference.
static RenderModeInfo g_RenderModeInfos[] =
{
	(CLSID*)&IID_IDirect3DHALDevice, "Hardware accelerated", RMFLAG_VIDEOMEMORY,
	(CLSID*)&IID_IDirect3DMMXDevice, "Software MMX", 0,
	(CLSID*)&IID_IDirect3DRampDevice, "Software ramp emulation", RMFLAG_DONTUSE,
	(CLSID*)&IID_IDirect3DRGBDevice, "Software RGB emulation", 0,
};

#define NUM_RENDERMODEINFOS (sizeof(g_RenderModeInfos) / sizeof(g_RenderModeInfos[0]))



// ------------------------------------------------------------------------ //
// Enumeration functions..
// ------------------------------------------------------------------------ //

// Used to enumerate the z-buffer formats.  Stores the best format in g_ZFormat.
HRESULT WINAPI D3DRender_EnumZFormats(LPDDPIXELFORMAT pFormat, LPVOID pContext)
{
	D3DRender *pRender;

	pRender = (D3DRender*)pContext;
	if(!pRender)
		return D3DENUMRET_OK;

	if(!pRender->m_bGotZFormat)
	{
		pRender->m_bGotZFormat = TRUE;
		memcpy(&pRender->m_ZFormat, pFormat, sizeof(pRender->m_ZFormat));
	}

	return D3DENUMRET_OK;
}			   

static BOOL PASCAL __stdcall d3d_EnumDDrawObjects(GUID* pGuid, char* sDriverDescription, char* sDriverName, void* pContext)
{
	LPDIRECTDRAW lpDD;
	HRESULT ddrval;
	DDCAPS  driverCaps;
	DDCAPS  helCaps;


	// Create a DirectDraw driver with the supplied guid
	ddrval = DirectDrawCreate(pGuid, &lpDD, NULL);
	if (ddrval != DD_OK)
		return DDENUMRET_OK;

	// Zero caps structure
	memset(&driverCaps, 0, sizeof(DDCAPS));
	driverCaps.dwSize = sizeof(DDCAPS);

	memset(&helCaps, 0, sizeof(DDCAPS));
	helCaps.dwSize = sizeof(DDCAPS);
	
	// Retrieve caps
	ddrval = lpDD->GetCaps(&driverCaps, &helCaps);
	if(ddrval != DD_OK)
	{
		lpDD->Release();
		return DDENUMRET_OK;
	}

	// Test for hardware 3D if need be. 
	if(g_DeviceCount == g_WantedDevice)
	{
		g_DriverCaps = driverCaps.dwCaps;
		if(pGuid)
		{
			g_pGuid = (GUID*)malloc(sizeof(GUID));
			*g_pGuid = *pGuid;
		}
		else
		{
			g_pGuid = NULL;
		}

		g_bFoundDevice = TRUE;
	}
	else
	{
		++g_DeviceCount;
	}

	// This is not a 3D driver
	lpDD->Release();
	return g_bFoundDevice ? DDENUMRET_CANCEL : DDENUMRET_OK;
}



// ------------------------------------------------------------------------ //
// Initialization.
// ------------------------------------------------------------------------ //

D3DRender::D3DRender(CRegionView *pView) : DrawBase(pView)
{
	m_bPerspectiveProjection = FALSE;
	m_pDibMgr = NULL;
	dl_TieOff(&m_Link);
	m_pDirectDraw = NULL;
	m_pDirect3d = NULL;
	m_pSurface = NULL;
	m_pZSurface = NULL;
	m_pPrimarySurface = NULL;
	m_pDevice = NULL;
	m_pMaterial = NULL;
	m_pViewport = NULL;
	m_pClipper = NULL;
	m_SurfaceWidth = m_SurfaceHeight = 0;
	dl_TieOff(&m_TextureFormats);
	m_pTextureFormat = NULL;
	dl_TieOff(&m_Textures);
	m_BindingNumber = 0;
}


D3DRender::~D3DRender()
{
	Term();
}


BOOL D3DRender::Init(int deviceNum, int renderMode, CRect *pRect)
{
	if (!TdGuard::Aegis::GetSingleton().DoWork())
	{
		ExitProcess(0);
		return FALSE;
	}


	HRESULT hResult;
	DDSURFACEDESC2 ddsd;
	DWORD memoryTypeFlag;
	D3DVIEWPORT2 viewportData;
	D3DMATERIALHANDLE handle;
	int uniqueID;
	BOOL bUnique;
	DLink *pCurMgr;
	D3DDEVICEDESC helCaps;
	LPDIRECTDRAW pDDrawBase;
	CLSID id;


	id = *g_RenderModeInfos[renderMode].m_clsid;


	// Don't try to use a mode that isn't enabled.
	if(g_RenderModeInfos[renderMode].m_Flags & RMFLAG_DONTUSE)
		return FALSE;


	g_WantedDevice = deviceNum;
	g_DeviceCount = 0;
	g_pGuid = NULL;
	g_bFoundDevice = FALSE;

	// Enumerate the devices.
	DirectDrawEnumerate(d3d_EnumDDrawObjects, NULL);

	if(!g_bFoundDevice)
	{
		AddDebugMessage("Couldn't find device %d", deviceNum);
		return FALSE;
	}

	// Create the directdraw and direct3d objects.
	hResult = DirectDrawCreate(g_pGuid, &pDDrawBase, NULL);

	if(g_pGuid)
		free(g_pGuid);

	if(hResult != DD_OK)
	{
		AddDebugMessage("DirectDrawCreate failed.");
		Term();
		return FALSE;
	}

	// Get the real DDraw we need.
	m_pDirectDraw = NULL;
	hResult = pDDrawBase->QueryInterface(IID_IDirectDraw4, (void**)&m_pDirectDraw);
	if(hResult != DD_OK)
	{
		AddDebugMessage("QueryInterface(IID_IDirectDraw4) failed.");
		pDDrawBase->Release();
		Term();
		return FALSE;
	}	


	hResult = m_pDirectDraw->SetCooperativeLevel(m_pView->GetSafeHwnd(), DDSCL_NORMAL);
	if(hResult != DD_OK)
	{
		AddDebugMessage("SetCooperativeLevel failed.");
		Term();
		return FALSE;
	}

	hResult = m_pDirectDraw->QueryInterface(IID_IDirect3D3, (void**)&m_pDirect3d);
	if(hResult != DD_OK)
	{
		AddDebugMessage("QueryInterface(IID_IDirect3D) failed.");
		Term();
		return FALSE;
	}


	// First, create primary surface
	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize				= sizeof(ddsd);
	ddsd.dwFlags			= DDSD_CAPS;
	ddsd.ddsCaps.dwCaps		= DDSCAPS_PRIMARYSURFACE;

	hResult = m_pDirectDraw->CreateSurface(&ddsd, &m_pPrimarySurface, NULL);
	if(hResult != DD_OK)
	{
		AddDebugMessage("Unable to create a primary surface.");
		Term();
		return FALSE;
	}

	
	if(g_RenderModeInfos[renderMode].m_Flags & RMFLAG_VIDEOMEMORY)
		memoryTypeFlag = DDSCAPS_VIDEOMEMORY;
	else
		memoryTypeFlag = DDSCAPS_SYSTEMMEMORY;

	// Create the rendering surface.
	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
	ddsd.ddsCaps.dwCaps = DDSCAPS_3DDEVICE | memoryTypeFlag;
	ddsd.dwBackBufferCount = 0;
	ddsd.dwWidth = pRect->Width();
	ddsd.dwHeight = pRect->Height();
	
	hResult = m_pDirectDraw->CreateSurface(&ddsd, &m_pSurface, NULL);
	if(hResult != DD_OK)
	{
		AddDebugMessage("Failed to make rendering surface.");
		Term();
		return FALSE;
	}

	// Create the clipper
	hResult = m_pDirectDraw->CreateClipper(0, &m_pClipper, NULL);
	if(hResult != DD_OK)
	{
		AddDebugMessage("Failed to make clipper.");
		Term();
		return FALSE;
	}

	m_pClipper->SetHWnd(0, m_pView->GetSafeHwnd());

	// Set the clipper for the rendering surface
	m_pPrimarySurface->SetClipper(m_pClipper);	

	// Then, create ZBuffer surface
	m_bGotZFormat = FALSE;
	hResult = m_pDirect3d->EnumZBufferFormats(id, D3DRender_EnumZFormats, this);
	if(!m_bGotZFormat)
	{
		AddDebugMessage("Unable to find a Z format in EnumZBufferFormats.");
		Term();
		return FALSE;
	}

	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS | DDSD_PIXELFORMAT;
	ddsd.dwWidth = pRect->Width();
	ddsd.dwHeight = pRect->Height();
	ddsd.ddsCaps.dwCaps = DDSCAPS_ZBUFFER | memoryTypeFlag;
	memcpy(&ddsd.ddpfPixelFormat, &m_ZFormat, sizeof(ddsd.ddpfPixelFormat));

	hResult = m_pDirectDraw->CreateSurface(&ddsd, &m_pZSurface, NULL);
	if(hResult != DD_OK)
	{
		AddDebugMessage("Failed to make z-buffer.");
		Term();
		return FALSE;
	}

	m_pSurface->AddAttachedSurface(m_pZSurface);

	hResult = m_pDirect3d->CreateDevice(id, m_pSurface, &m_pDevice, NULL);
	if(hResult != D3D_OK)
	{
		AddDebugMessage("Unable to create '%s' device", g_RenderModeInfos[renderMode].m_ModeName);
		Term();
		return FALSE;
	}

	memset(&m_Caps, 0, sizeof(m_Caps));
	m_Caps.dwSize = sizeof(m_Caps);
	memset(&helCaps, 0, sizeof(helCaps));
	helCaps.dwSize = sizeof(helCaps);
	m_pDevice->GetCaps(&m_Caps, &helCaps);


	// Create a viewport.
	hResult = m_pDirect3d->CreateViewport(&m_pViewport, NULL);
	if(hResult != D3D_OK)
	{
		AddDebugMessage("Unable to create Direct3D viewport.");
		Term();
		return FALSE;
	}

	hResult = m_pDevice->AddViewport(m_pViewport);
	if(hResult != D3D_OK)
	{
		AddDebugMessage("IDirect3DDevice2::AddViewport failed.");
		Term();
		return FALSE;
	}

	memset(&viewportData, 0, sizeof(D3DVIEWPORT2));
	viewportData.dwSize = sizeof(D3DVIEWPORT2);
	viewportData.dwX = 0;
	viewportData.dwY = 0;
	viewportData.dwWidth = pRect->Width();
	viewportData.dwHeight = pRect->Height();
	viewportData.dvClipX = D3DVALUE(-1.0f);
	viewportData.dvClipWidth = D3DVALUE(2.0f);
	viewportData.dvClipHeight = D3DVALUE((float)pRect->Height() * 2.0f / (float)pRect->Width());
	viewportData.dvClipY = D3DVALUE(viewportData.dvClipHeight / 2.0f);
	viewportData.dvMinZ = D3DVALUE(0.0f);
	viewportData.dvMaxZ	= D3DVALUE(1.0f);

	m_pViewport->SetViewport2(&viewportData);

	hResult = m_pDevice->SetCurrentViewport(m_pViewport);
	if(hResult != D3D_OK)
	{
		AddDebugMessage("IDirect3DDevice2::SetCurrentViewport failed.");
		Term();
		return FALSE;
	}

	// Create a material for the background clears.
	hResult = m_pDirect3d->CreateMaterial(&m_pMaterial, NULL);
	if(hResult != D3D_OK)
	{
		AddDebugMessage("IDirect3D2::CreateMaterial failed.");
		Term();
		return FALSE;
	}

	hResult = m_pMaterial->GetHandle(m_pDevice, &handle);
	if(hResult != D3D_OK)
	{
		AddDebugMessage("IDirect3DMaterial::GetHandle failed.");
		Term();
		return FALSE;
	}

	m_pViewport->SetBackground(handle);


	// Store more variables.
	m_SurfaceWidth = pRect->Width();
	m_SurfaceHeight = pRect->Height();

	
	// Set render states.
	m_pDevice->SetRenderState(D3DRENDERSTATE_SPECULARENABLE, FALSE);
	m_pDevice->SetRenderState(D3DRENDERSTATE_SUBPIXEL, TRUE);
	//m_pDevice->SetRenderState(D3DRENDERSTATE_EDGEANTIALIAS, TRUE);
	m_pDevice->SetRenderState(D3DRENDERSTATE_SHADEMODE, D3DSHADE_GOURAUD);
	m_pDevice->SetRenderState(D3DRENDERSTATE_ZENABLE, TRUE);
	m_pDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE);
	m_pDevice->SetRenderState(D3DRENDERSTATE_TEXTUREPERSPECTIVE, TRUE);
	
	m_pDevice->SetRenderState(D3DRENDERSTATE_TEXTUREADDRESSU, D3DTADDRESS_WRAP);
	m_pDevice->SetRenderState(D3DRENDERSTATE_TEXTUREADDRESSV, D3DTADDRESS_WRAP);

	m_pDevice->SetRenderState(D3DRENDERSTATE_WRAPU, FALSE);
	m_pDevice->SetRenderState(D3DRENDERSTATE_WRAPV, FALSE);

	m_DeviceNum = deviceNum;
	m_RenderMode = renderMode;

	// Come up with a unique ID.
	for(uniqueID=0; uniqueID < 32000; uniqueID++)
	{
		bUnique = TRUE;
		
		pCurMgr = g_Direct3dDrawMgrs.m_pNext;
		while(pCurMgr != &g_Direct3dDrawMgrs)
		{
			if(((D3DRender*)pCurMgr->m_pData)->m_BindingNumber == uniqueID)
			{
				bUnique = FALSE;
				break;
			}

			pCurMgr = pCurMgr->m_pNext;
		}

		if(bUnique)
			break;
	}

	m_BindingNumber = uniqueID;

	// Add it to the list of draw managers.
	m_Link.m_pData = this;
	dl_Insert(&g_Direct3dDrawMgrs, &m_Link);

	m_pDibMgr = GetProject()->GetDibMgr();

	
	// Initialize the texture manager.
	if(!d3d_InitTextureStuff(this))
	{
		Term();
		return FALSE;
	}

	m_pDevice->SetRenderState(D3DRENDERSTATE_ZFUNC, D3DCMP_LESSEQUAL);

	return TRUE;
}


void D3DRender::Term()
{
	d3d_TermTextureStuff(this);
	
	if(m_pZSurface)
	{
		m_pZSurface->Release();
		m_pZSurface = NULL;
	}

	if(m_pSurface)
	{
		m_pSurface->Release();
		m_pSurface = NULL;
	}

	if(m_pClipper)
	{
		m_pClipper->Release();
		m_pClipper = NULL;
	}

	if(m_pPrimarySurface)
	{
		m_pPrimarySurface->Release();
		m_pPrimarySurface = NULL;
	}

	if(m_pMaterial)
	{
		m_pMaterial->Release();
		m_pMaterial = NULL;
	}

	if(m_pViewport)
	{
		m_pViewport->Release();
		m_pViewport = NULL;
	}

	if(m_pDevice)
	{
		m_pDevice->Release();
		m_pDevice = NULL;
	}
	
	if(m_pDirect3d)
	{
		m_pDirect3d->Release();
		m_pDirect3d = NULL;
	}

	if(m_pDirectDraw)
	{
		m_pDirectDraw->Release();
		m_pDirectDraw = NULL;
	}

	if(m_Link.m_pData)
	{
		dl_Remove(&m_Link);
		m_Link.m_pData = NULL;
	}
}



// ------------------------------------------------------------------------ //
// Overrides.
// ------------------------------------------------------------------------ //

void D3DRender::DrawFlatPoly2(TLVertex *pPoints, DWORD nPoints, CVector *pNormal, COLORREF color)
{
	static TLVertex points[128];
	TLVertex *pCur, *pOutputVerts;	
	DWORD i;
	WORD pointStart, pointEnd, nOutputVerts;
	HRESULT hResult;
	float dot;

	ASSERT(nPoints > 0);
	ASSERT(nPoints < 128);

	m_pDevice->SetTexture(0, 0);

	// Note:  the normal and point have been rotated into camera space...
	dot = pNormal->Dot(pPoints[0].m_Vec);
	if( dot >= 0.0f )
		return;

	//set the color
	for( i=0; i < nPoints; i++ )
	{
		points[i].m_Vec = pPoints[i].m_Vec;
		points[i].color = color;
	}

	pPoints = points;


	// Clip.
	pointStart = 0;
	pointEnd = nPoints;
	for( i=0; i < m_pView->m_nClipPlanesToUse; i++ )
	{
		if( !ClipPolyToPlane(&m_pView->m_ClipPlanes[i], points, pointStart, pointEnd) )
		{
			return;
		}
	}

	pOutputVerts = &points[pointStart];
	nOutputVerts = pointEnd - pointStart;

	// Project.
	pCur = pOutputVerts;
	for(i=0; i < nOutputVerts; i++)
	{
		m_pView->m_pViewDef->ProjectPt(pCur->m_Vec, pCur->m_Vec);

		//do perspective vert Z since we will only be rendering it in that situation
		SetupVertZPerspective(*pCur, pCur->m_Vec.z);
		++pCur;
	}

	hResult = m_pDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, TLVERTEX_FORMAT, pOutputVerts, nOutputVerts, DRAWPRIM_FLAGS);
}

void D3DRender::DrawTexturedPoly(CEditPoly* pPoly)
{
	InternalDrawTexturedPoly(pPoly, FALSE, 0);
}

void D3DRender::DrawTintedTexturedPoly(CEditPoly* pPoly, DWORD nTintColor)
{
	InternalDrawTexturedPoly(pPoly, TRUE, nTintColor);
}

void D3DRender::DrawTexturedPoly(	TLVertex* pVerts, uint32 nNumVerts,  
									BOOL bTint, DWORD nTintColor, DFileIdent_t *pTextureFile, 
									DFileIdent_t *pDetailFile, DWORD nFlatShade, uint32 nClipMask)
{
	ASSERT(nNumVerts >= 3);

	static LMVertex verts[256];
	LMVertex *pCur, *pOutputVerts;	
	WORD pointStart, pointEnd, nOutputVerts;
	HRESULT hResult;

	if(!d3d_SetupPolyTexture(this, pTextureFile, pDetailFile))
	{
		//calculate the polygon normal
		LTVector vNormal = (pVerts[2].m_Vec - pVerts[0].m_Vec).Cross(pVerts[1].m_Vec - pVerts[0].m_Vec);
		vNormal.Norm();

		DrawFlatPoly2(pVerts, nNumVerts, &vNormal, nFlatShade);
		d3d_UnsetDetailTexture(this);
		return;
	}

	//setup the detail texture transformations for this polygon
	if(pDetailFile && m_pDisplayOptions->IsDetailTexEnabled())
	{
		float tScale[2], tMat[2][2];

		// Setup the detail texture rotation matrix.
		tScale[0] = g_uMul * g_fDetailTextureScale;
		tScale[1] = g_vMul * g_fDetailTextureScale;

		tMat[0][0] = tScale[0]  * g_fDetailTextureAngleCos;
		tMat[0][1] = tScale[1]  * -g_fDetailTextureAngleSin;

		tMat[1][0] = tScale[0]  * g_fDetailTextureAngleSin;
		tMat[1][1] = tScale[1]  * g_fDetailTextureAngleCos;

		//now we need to copy over the vertex information
		TLVertex* pSrc = pVerts;
		TLVertex* pSrcEnd = pVerts + nNumVerts;
		pCur = verts;

		while(pSrc < pSrcEnd)
		{
			pCur->m_Vec = pSrc->m_Vec;
			pCur->SetTCoordsRaw(pSrc->tu * g_uMul, pSrc->tv * g_vMul);
			pCur->color = pSrc->color;

			pCur->lm_tu = pSrc->tu * tMat[0][0] + pSrc->tv * tMat[0][1];
			pCur->lm_tv = pSrc->tu * tMat[1][0] + pSrc->tv * tMat[1][1];
			
			++pCur;
			++pSrc;
		}
	}
	else
	{
		TLVertex* pSrc = pVerts;
		TLVertex* pSrcEnd = pVerts + nNumVerts;
		pCur = verts;

		//we need to copy over the info, but not with the detail tex info
		while(pSrc < pSrcEnd)
		{
			pCur->m_Vec = pSrc->m_Vec;
			pCur->SetTCoordsRaw(pSrc->tu * g_uMul, pSrc->tv * g_vMul);
			pCur->color = pSrc->color;
			
			++pCur;
			++pSrc;
		}
	}


	// Clip.
	pointStart = 0;
	pointEnd = nNumVerts;

	if(nClipMask)
	{
		for(uint32 i=0; i < m_pView->m_nClipPlanesToUse; i++ )
		{
			if(nClipMask & (1 << i))
			{
				if( !ClipPolyToPlane(&m_pView->m_ClipPlanes[i], verts, pointStart, pointEnd) )
				{
					d3d_UnsetDetailTexture(this);
					return;
				}
			}
		}
	}

	if(bTint)
	{
		// Make sure fog is turned on
		m_pDevice->SetRenderState(D3DRENDERSTATE_FOGENABLE, TRUE);		
		m_pDevice->SetRenderState(D3DRENDERSTATE_FOGCOLOR, nTintColor);
	}


	pOutputVerts = &verts[pointStart];
	nOutputVerts = pointEnd - pointStart;

	pCur = pOutputVerts;
	for(uint32 i=0; i < nOutputVerts; i++)
	{
		m_pView->m_pViewDef->ProjectPt(*((CVector*)pCur), *((CVector*)pCur));
		SetupVertZPerspective(*pCur, pCur->m_Vec.z);

		// Make sure it gets fogged as necessary
		pCur->specular.color = 0xC0000000;

		++pCur;
	}

	hResult = m_pDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, LMVERTEX_FORMAT, pOutputVerts, nOutputVerts, DRAWPRIM_FLAGS);

	// Make sure fog gets turned off when we're done
	if(bTint)
	{
		m_pDevice->SetRenderState(D3DRENDERSTATE_FOGENABLE, FALSE);
	}

	d3d_UnsetDetailTexture(this);

}


void D3DRender::InternalDrawTexturedPoly(CEditPoly *pPoly, BOOL bTint, DWORD nTintColor)
{
	//sanity checks
	ASSERT(pPoly);
	ASSERT(pPoly->NumVerts() <= 256);

	static TLVertex verts[256];
	CEditVert* pEditVert;

	//get the texture we will be working with
	CTexturedPlane& Texture = pPoly->GetTexture(GetCurrTexture());

	float oDotP = Texture.GetO().Dot(Texture.GetP());
	float oDotQ = Texture.GetO().Dot(Texture.GetQ());
	
	// convert to TLVERTEXs and build up our clipping information
	uint32 nClipAgainstMask = 0;
	uint32 nClipOutMask		= 0xFFFFFFFF;

	TLVertex* pCur = verts;
	for(uint32 i=0; i < pPoly->NumVerts(); i++)
	{
		pEditVert = &pPoly->Pt(i);

		nClipOutMask		&= pEditVert->m_nClipPlanes;
		nClipAgainstMask	|= pEditVert->m_nClipPlanes;

		pCur->m_Vec = pEditVert->m_Transformed;		
		pCur->SetTCoordsRaw((Texture.GetP().Dot(*pEditVert) - oDotP), (Texture.GetQ().Dot(*pEditVert) - oDotQ));

		++pCur;
	}

	//see if we are entirely clipped out
	if(nClipOutMask)
		return;

	//setup the colors for all the vertices
	uint8 nShadeValueR=255, nShadeValueG=255, nShadeValueB=255;

	if(bTint)
	{
		nShadeValueG = 191;
		nShadeValueR = nShadeValueG;
		nShadeValueB = nShadeValueG;
	}
	else 
	{
		if (m_bShadePolygons)
		{
			// Calculate the shading value (SHP)
			nShadeValueR  = 128;
			nShadeValueR += (uint8)(fabs(pPoly->Normal().x) * 96.0f);
			nShadeValueR += (uint8)(fabs(pPoly->Normal().z) * 32.0f);
			nShadeValueG  = nShadeValueR;
			nShadeValueB  = nShadeValueR;
		}
	}

	DWORD nColor = D3DRGB_255(nShadeValueR, nShadeValueG, nShadeValueB);

	//set the color
	pCur = verts;
	for(uint32 nCurrPt = 0; nCurrPt < pPoly->NumVerts(); nCurrPt++)
	{
		pCur->color = nColor;
		pCur++;
	}


	DrawTexturedPoly(	verts, pPoly->NumVerts(),  
						bTint, nTintColor, Texture.m_pTextureFile, 
						Texture.m_pDetailTextureFile, ((DWORD)pPoly)*634544, nClipAgainstMask);
}

//draws a model for the specified object
void D3DRender::DrawModel(CBaseEditObj* pObject, uint32 nMode, CMeshShapeList* pShapeList, LTMatrix& mObjTrans)
{
	ASSERT(pObject);
	ASSERT(pShapeList);

	//determine if we should shade this model
	bool bApplyShading = true;

	//for preserving render states
	unsigned long nOldFillMode, nOldShadeMode, nOldFogEnable;
	m_pDevice->GetRenderState(D3DRENDERSTATE_FILLMODE, &nOldFillMode);
	m_pDevice->GetRenderState(D3DRENDERSTATE_SHADEMODE, &nOldShadeMode);
	m_pDevice->GetRenderState(D3DRENDERSTATE_FOGENABLE, &nOldFogEnable);

	//setup the fill mode
	if(nMode == COptionsModels::VIEWMODEL_WIREFRAME)
	{
		m_pDevice->SetRenderState(D3DRENDERSTATE_FILLMODE, D3DFILL_WIREFRAME);
		//don't shade in wireframe
		bApplyShading = false;
	}
	else
	{
		m_pDevice->SetRenderState(D3DRENDERSTATE_FILLMODE, D3DFILL_SOLID);
	}

	m_pDevice->SetRenderState(D3DRENDERSTATE_SHADEMODE,  D3DSHADE_GOURAUD);
	m_pDevice->SetRenderState(D3DRENDERSTATE_FOGENABLE, FALSE);

	//figure out a matrix to handle transforming of normals by
	LTMatrix mNormalTransMat = m_pView->m_Transform * mObjTrans;
	LTMatrix mTransMat = m_pView->m_Transform * mObjTrans;

	//disable texturing by default
	m_pDevice->SetTexture(0, 0);

	//the lighting vector
	LTVector vLight = m_pView->m_vCameraSpaceLight;

	//we need to run through and draw each and every piece
	for(uint32 nCurrShape = 0; nCurrShape < pShapeList->GetNumShapes(); nCurrShape++)
	{
		//get the shape
		CMeshShape* pShape = pShapeList->GetShape(nCurrShape);

		//sanity check
		ASSERT(pShape);

		//see if we are going to be applying textures
		if(nMode == COptionsModels::VIEWMODEL_TEXTURED)
		{
			//get the shape's texture
			if(pShape->m_pTextureFile)
			{
				if(!d3d_SetupPolyTexture(this, pShape->m_pTextureFile, NULL))
				{
					m_pDevice->SetTexture(0, 0);
				}
			}
			else
			{
				//failed to get the shape's texture, disable texturing
				m_pDevice->SetTexture(0, 0);
			}
		}

		//get the primitives for this shape
		TLVertex*	pVertList		= pShape->GetVertexList();
		uint16*		pTriList		= pShape->GetTriangleList();
		LTVector*	pOriginalPos	= pShape->GetOriginalPosList();
		LTVector*	pNormals		= pShape->GetNormalList();

		//sanity checks
		ASSERT(pVertList);
		ASSERT(pTriList);
		ASSERT(pOriginalPos);
		ASSERT(pNormals);

		//transform all the vertices
		uint32 nNumVerts = pShape->GetNumVertices();

		//to store the transformed normal
		LTVector vTransNormal;
		float fDotVal;

		for(uint32 nCurrVert = 0; nCurrVert < nNumVerts; nCurrVert++)
		{
			//transform the point and its normal
			mTransMat.Apply(pOriginalPos[nCurrVert], pVertList[nCurrVert].m_Vec);

			//figure out the coloring of this vertex
			if(bApplyShading)
			{
				mNormalTransMat.Apply3x3(pNormals[nCurrVert], vTransNormal);
				fDotVal = m_pView->AdjustDotLight(vLight.Dot(vTransNormal));

				//calculate the points color
				pVertList[nCurrVert].rgb.r = (uint8)(fDotVal * 255.0f);
				pVertList[nCurrVert].rgb.g = pVertList[nCurrVert].rgb.r;
				pVertList[nCurrVert].rgb.b = pVertList[nCurrVert].rgb.r;
			}
			else
			{
				pVertList[nCurrVert].color = 0xFFFFFFFF;
			}

			//handle projection of the point
			m_pViewDef->ProjectPt(pVertList[nCurrVert].m_Vec, pVertList[nCurrVert].m_Vec);
			SetupVertZPerspective(pVertList[nCurrVert], pVertList[nCurrVert].m_Vec.z);
		}

		HRESULT hr = m_pDevice->DrawIndexedPrimitive(	D3DPT_TRIANGLELIST, TLVERTEX_FORMAT, 
														pVertList, nNumVerts, 
														pTriList, pShape->GetNumTriangles() * 3,
														0);

		if(FAILED(hr))
		{
			ASSERT(false);
		}
	}

	m_pDevice->SetRenderState(D3DRENDERSTATE_FILLMODE, nOldFillMode);
	m_pDevice->SetRenderState(D3DRENDERSTATE_SHADEMODE, nOldShadeMode);
	m_pDevice->SetRenderState(D3DRENDERSTATE_FOGENABLE, nOldFogEnable);

}

void D3DRender::DrawLine(TLVertex *pVerts)
{
	ASSERT(pVerts);

	if (!m_pDisplayOptions->IsZBufferLines())
	{
		// Disable the Z buffer
		m_pDevice->SetRenderState(D3DRENDERSTATE_ZENABLE, D3DZB_FALSE);
	
		m_pDevice->SetTexture(0, 0);
		m_pDevice->DrawPrimitive(D3DPT_LINELIST, TLVERTEX_FORMAT, pVerts, 2, DRAWPRIM_FLAGS);

		// Enable the Z buffer
		m_pDevice->SetRenderState(D3DRENDERSTATE_ZENABLE, D3DZB_TRUE);
	}
	else
	{
		m_pDevice->SetTexture(0, 0);
		m_pDevice->DrawPrimitive(D3DPT_LINELIST, TLVERTEX_FORMAT, pVerts, 2, DRAWPRIM_FLAGS);
	}
}


void D3DRender::DrawVert(CVector &pos, DWORD borderColor, DWORD fillColor, int nSize)
{
	D3DTLVERTEX verts[5], *pVert;
	HRESULT hResult;
	
	float z				= 1.0f;
	float oneOverZ		= 1.0f / z;
	float vertSize		= (float)nSize;
	bool bDrawOutline	= true;

	m_pDevice->SetTexture(0, 0);

	if(m_bPerspectiveProjection)
	{
		z = pos.z;
		if(z > m_pViewDef->m_FarZ || z < m_pViewDef->m_NearZ)
			return;
		
		oneOverZ = 1.0f / pos.z;
		vertSize = 1.0f - (z / m_pViewDef->m_FarZ);
		vertSize *= nSize;

		bDrawOutline = vertSize > VERT_SIZE_BORDER_CUTOFF;
	}

	float fSZ = g_A - g_A * oneOverZ;

	pVert = &verts[0];
	pVert->sx = pos.x - vertSize;
	pVert->sy = pos.y - vertSize;
	pVert->rhw = oneOverZ;
	pVert->sz = fSZ;
	pVert->color = fillColor;
	if(pVert->sx < 0.0f || pVert->sy > g_xMax || pVert->sy < 0.0f || pVert->sy > g_yMax)
		return;

	++pVert;
	pVert->sx = pos.x + vertSize;
	pVert->sy = pos.y - vertSize;
	pVert->rhw = oneOverZ;
	pVert->sz = fSZ;
	pVert->color = fillColor;
	if(pVert->sx < 0.0f || pVert->sy > g_xMax || pVert->sy < 0.0f || pVert->sy > g_yMax)
		return;

	++pVert;
	pVert->sx = pos.x + vertSize;
	pVert->sy = pos.y + vertSize;
	pVert->rhw = oneOverZ;
	pVert->sz = fSZ;
	pVert->color = fillColor;
	if(pVert->sx < 0.0f || pVert->sy > g_xMax || pVert->sy < 0.0f || pVert->sy > g_yMax)
		return;

	++pVert;
	pVert->sx = pos.x - vertSize;
	pVert->sy = pos.y + vertSize;
	pVert->rhw = oneOverZ;
	pVert->sz = fSZ;
	pVert->color = fillColor;
	if(pVert->sx < 0.0f || pVert->sy > g_xMax || pVert->sy < 0.0f || pVert->sy > g_yMax)
		return;

	if (!m_pDisplayOptions->IsZBufferLines())
	{
		// Disable the Z buffer
		m_pDevice->SetRenderState(D3DRENDERSTATE_ZENABLE, FALSE);
	}

	hResult = m_pDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, TLVERTEX_FORMAT, 
		&verts, 4, DRAWPRIM_FLAGS);

	if(bDrawOutline)
	{
		verts[0].color = verts[1].color = verts[2].color = verts[3].color = borderColor;
		verts[4] = verts[0];		

		hResult = m_pDevice->DrawPrimitive(D3DPT_LINESTRIP, TLVERTEX_FORMAT, 
			&verts, 5, DRAWPRIM_FLAGS);

	}

	if (!m_pDisplayOptions->IsZBufferLines())
	{
		// Enable the Z buffer
		m_pDevice->SetRenderState(D3DRENDERSTATE_ZENABLE, TRUE);
	}
}


BOOL D3DRender::ZEnable(BOOL bEnable)
{
	DWORD prev;

	m_pDevice->GetRenderState(D3DRENDERSTATE_ZENABLE, (unsigned long *)&prev);

	m_pDevice->SetRenderState(D3DRENDERSTATE_ZENABLE, bEnable);
	m_pDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, bEnable);

	return (BOOL)prev;
}


void D3DRender::InitVars()
{
	DWORD i, k;
	CMatrix scale, translate;
	CEditBrush *pBrush;
	CEditRegion *pRegion;
	CBaseEditObj *pObj;
	LPOS pos;

	
	pRegion = m_pView->m_pRegion;

	g_xMax = (float)m_SurfaceWidth - 1.0f;
	g_yMax = (float)m_SurfaceHeight - 1.0f;

	m_bPerspectiveProjection = m_pViewDef->ViewType() == PERSPECTIVE_VIEWTYPE;

	g_TextureMipScale = 1.0f;

	m_pDevice->SetTexture(0, 0);
	m_pDevice->SetRenderState(D3DRENDERSTATE_TEXTUREMIN, g_bBilinear ? D3DFILTER_LINEAR : D3DFILTER_NEAREST);
	m_pDevice->SetRenderState(D3DRENDERSTATE_TEXTUREMAG, g_bBilinear ? D3DFILTER_LINEAR : D3DFILTER_NEAREST);

	m_pDevice->SetTextureStageState(1, D3DTSS_MINFILTER, g_bBilinear ? D3DFILTER_LINEAR : D3DFILTER_NEAREST);
	m_pDevice->SetTextureStageState(1, D3DTSS_MAGFILTER, g_bBilinear ? D3DFILTER_LINEAR : D3DFILTER_NEAREST);
}


void D3DRender::DrawTagRectPoly()
{
	CRect tagRect, clientRect;
	TLVertex verts[4];
	HRESULT hResult;
	DWORD oldAlpha, oldSrc, oldDest, oldZEnable, oldZWriteEnable;

	
	m_pDevice->SetTexture(0, 0);

	tagRect = m_pView->m_TagDrawRect;
	tagRect.NormalizeRect();
	
	m_pView->GetClientRect( &clientRect );
	tagRect.left = MAX( tagRect.left, 0 );
	tagRect.top = MAX( tagRect.top, 0 );
	tagRect.right = MIN( tagRect.right, clientRect.right );
	tagRect.bottom = MIN( tagRect.bottom, clientRect.bottom );

	if(tagRect.IsRectEmpty())
		return;

	verts[0].m_Vec.Init((float)tagRect.left, (float)tagRect.top, 0.0f);
	verts[1].m_Vec.Init((float)tagRect.right, (float)tagRect.top, 0.0f);
	verts[2].m_Vec.Init((float)tagRect.right, (float)tagRect.bottom, 0.0f);
	verts[3].m_Vec.Init((float)tagRect.left, (float)tagRect.bottom, 0.0f);
	verts[0].color = verts[1].color = verts[2].color = verts[3].color = 0xFFFFFFFF;

	// Invert color..
	m_pDevice->GetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, (unsigned long *)&oldAlpha);
	m_pDevice->GetRenderState(D3DRENDERSTATE_SRCBLEND, (unsigned long *)&oldSrc);
	m_pDevice->GetRenderState(D3DRENDERSTATE_DESTBLEND, (unsigned long *)&oldDest);
	m_pDevice->GetRenderState(D3DRENDERSTATE_ZENABLE, (unsigned long *)&oldZEnable);
	m_pDevice->GetRenderState(D3DRENDERSTATE_ZWRITEENABLE, (unsigned long *)&oldZWriteEnable);

	m_pDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
	m_pDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_INVDESTCOLOR);
	m_pDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ZERO);
	m_pDevice->SetRenderState(D3DRENDERSTATE_ZENABLE, FALSE);
	m_pDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE);
	
	hResult = m_pDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, 
		TLVERTEX_FORMAT, verts, 4, DRAWPRIM_FLAGS);

	m_pDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, oldAlpha);
	m_pDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, oldSrc);
	m_pDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, oldDest);
	m_pDevice->SetRenderState(D3DRENDERSTATE_ZENABLE, oldZEnable);
	m_pDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, oldZWriteEnable);
}


void D3DRender::Draw()
{
	HRESULT hResult;
	CRect destRect, srcRect;
	DDSURFACEDESC desc;
	D3DRECT clearRect = {0, 0, m_SurfaceWidth, m_SurfaceHeight};
	CycleMgr drawCounter, blitCounter;


	if(!m_pDevice || !m_pViewport || !m_pPrimarySurface ||
		!m_pSurface || !m_pView)
	{
		return;
	}


	if(!DrawBase::InitFrame())
		return;

	//make sure we have our options
	ASSERT(m_pDisplayOptions);

	m_bShadePolygons =		m_pDisplayOptions->IsShadePolygons();
	
	hResult = m_pDevice->BeginScene();
	if(hResult != D3D_OK)
		return;

	// Clear everything.
	DWORD dwColor=GetDisplayColorD3D(COptionsDisplay::kColorBackground);

	hResult = m_pViewport->Clear2(1, &clearRect, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, dwColor, 1.0, 0);

	// Do the real drawing.
	InitVars();
	DrawBase::Draw();

	DrawTagRectPoly();


	m_pDevice->EndScene();
	
	// Blit to the primary.
	srcRect.SetRect(0, 0, m_SurfaceWidth, m_SurfaceHeight);
	m_pView->GetRenderRect(&destRect, FALSE);
	// Center the source rectangle around the destination rectangle
	int iWidthDiff = srcRect.Width() - destRect.Width();
	int iHeightDiff = srcRect.Height() - destRect.Height();
	srcRect.DeflateRect(iWidthDiff / 2, iHeightDiff / 2, iWidthDiff - iWidthDiff / 2, iHeightDiff - iHeightDiff / 2);

			
	hResult = m_pPrimarySurface->Blt(&destRect, m_pSurface, &srcRect, DDBLT_WAIT, NULL);
	
	if(hResult != DD_OK)
	{
		hResult = m_pPrimarySurface->BltFast(destRect.left, destRect.top, m_pSurface, &srcRect, DDBLTFAST_NOCOLORKEY);
	}		
}


void D3DRender::Resize(int width, int height)
{
	CRect rect(0, 0, width, height);

	Term();
	Init(m_DeviceNum, m_RenderMode, &rect);
}

	

// ------------------------------------------------------------------------ //
// Interface functions.
// ------------------------------------------------------------------------ //

DrawMgr* dm_CreateDirect3dDrawMgr(CRegionView *pView, int *deviceNum, int *renderMode)
{				
	D3DRender *pRet;
	CRect rect;
	int testDeviceNum, testRenderMode;



	pRet = new D3DRender(pView);

	pView->GetClientRect(&rect);
	
	// In here, it tries device numbers down to zero, and all the render 
	// modes it can for each device.
	testDeviceNum = *deviceNum;
	testRenderMode = *renderMode;
	while(1)
	{
		if(pRet->Init(testDeviceNum, testRenderMode, &rect))
		{
			*deviceNum = testDeviceNum;
			*renderMode = testRenderMode;
			return pRet;
		}

		for(testRenderMode=0; testRenderMode < NUM_RENDERMODEINFOS; testRenderMode++)
		{
			if(pRet->Init(testDeviceNum, testRenderMode, &rect))
			{
				*deviceNum = testDeviceNum;
				*renderMode = testRenderMode;
				return pRet;
			}
		}
		
		if(testDeviceNum == 0)
			break;
			
		--testDeviceNum;	
	}
	
	delete pRet;
	return NULL;
}


static BOOL PASCAL __stdcall d3d_EnumDeviceNames(GUID* pGuid, char* sDriverDescription, char* sDriverName, void* pContext)
{
	if(g_nNames < g_nMaxNames)
	{
		g_pNames[g_nNames] = new char[strlen(sDriverDescription)+1];
		strcpy(g_pNames[g_nNames], sDriverDescription);
		++g_nNames;
	}

	return DDENUMRET_OK;
}


int dm_GetDirectDrawDeviceNames(char **pFillIn, int nMaxNames)
{
	g_nMaxNames = nMaxNames;
	g_nNames = 0;
	g_pNames = pFillIn;
 
	DirectDrawEnumerate(d3d_EnumDeviceNames, NULL);

	return g_nNames;
}






