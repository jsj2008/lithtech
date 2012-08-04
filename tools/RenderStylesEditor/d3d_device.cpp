// D3D_Device.h
//	Contains the active D3D Device. CD3D_Shell & CD3D_Device are the main containers of D3D shtuff...

#include "stdafx.h"

#include "d3d_device.h"
#include "d3d_shell.h"
#include "d3dmeshrendobj_rigid.h"
#include "d3dmeshrendobj_skel.h"
#include "d3d_renderstatemgr.h"
#include "tdguard.h"

// GLOBALS
CD3D_Device		g_Device;						// The global D3D Shell...

// EXTERNS
extern HWND		g_hWnd_RenderWindow;
extern bool		g_bRunWindowed;
extern uint32	g_ScreenWidth;					// Screen Size...
extern uint32	g_ScreenHeight;

// Create the Sucka...
bool CD3D_Device::CreateDevice(D3DAdapterInfo* pAdapter,D3DDeviceInfo* pDevice,D3DModeInfo* pMode)
{
	if (!TdGuard::Aegis::GetSingleton().DoWork())
	{
		return false;
	}

	FreeDevice();								// Make sure it's all released and groovie...

	m_pAdapter	= pAdapter;						// Initialization...
	m_pDevice	= pDevice;
	m_pMode		= pMode;

	// Create the sucka...
	uint32 BehaviorFlags = D3DCREATE_MULTITHREADED;
	// Set device caps so we have mixed vertex option ( software and hardware if hardware supports hwtl )
	if (pDevice->d3dCaps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) 
	{
		BehaviorFlags |= D3DCREATE_MIXED_VERTEXPROCESSING;
	}
	else
	{
		BehaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;
	}

	D3DPRESENT_PARAMETERS PresentationParam;
	SetPresentationParams(PresentationParam,pMode);

	HRESULT hResult = PDIRECT3D->CreateDevice(pAdapter->iAdapterNum,pDevice->DeviceType,g_hWnd_RenderWindow,BehaviorFlags,&PresentationParam,&m_pD3DDevice);
	
	if ((hResult != D3D_OK) || !m_pD3DDevice) 
	{	// Give it more more try - Presentation params might have been changed by D3D to something acceptable...
		OutputDebugString("Warning: Create failed. Attempting to fall back...\n");
		TryFallingBack_OnFailedDevCreate(&PresentationParam);
		hResult = PDIRECT3D->CreateDevice(pAdapter->iAdapterNum,pDevice->DeviceType,g_hWnd_RenderWindow,BehaviorFlags,&PresentationParam,&m_pD3DDevice);
		if ((hResult != D3D_OK) || !m_pD3DDevice) 
		{ 
			FreeAll(); 
			return false; 
		} 
	}

	if (FAILED(m_pD3DDevice->GetDeviceCaps(&m_DeviceCaps)))	{ 
		FreeAll(); 
		return false; 
	}

	SetDefaultRenderStates();					// Set the default render states...
	PreCalcSomeDeviceCaps();					// Do some precalcing of device caps (figure out if we can do some puff)...

	// Display a warning message
//	if (m_pDevice->DeviceType == D3DDEVTYPE_REF) AddDebugMessage(1,"Warning: Couldnt' find any HAL devices, Using reference rasterizer");

	return true;
}

void d3d_GetColorMasks(D3DFORMAT iD3DFormat, uint32& iBitCount, uint32& iAlphaMask, uint32& iRedMask, uint32& iGreenMask, uint32& iBlueMask)
{
	switch (iD3DFormat) {
	case D3DFMT_X8R8G8B8 :
		iBitCount = 32; iAlphaMask = 0x00000000; iRedMask = 0x00FF0000; iGreenMask = 0x0000FF00; iBlueMask = 0x000000FF; break;
	case D3DFMT_A8R8G8B8 :
		iBitCount = 32; iAlphaMask = 0xFF000000; iRedMask = 0x00FF0000; iGreenMask = 0x0000FF00; iBlueMask = 0x000000FF; break;
	case D3DFMT_R5G6B5 :
		iBitCount = 16; iAlphaMask = 0x00000000; iRedMask = 0x0000F800; iGreenMask = 0x000007E0; iBlueMask = 0x0000001F; break;
	case D3DFMT_A1R5G5B5 :
		iBitCount = 16; iAlphaMask = 0x00008000; iRedMask = 0x00007C00; iGreenMask = 0x000003E0; iBlueMask = 0x0000001F; break;
	case D3DFMT_X1R5G5B5 :
		iBitCount = 16; iAlphaMask = 0x00000000; iRedMask = 0x00007C00; iGreenMask = 0x000003E0; iBlueMask = 0x0000001F; break;
	case D3DFMT_A4R4G4B4 :
		iBitCount = 16; iAlphaMask = 0x0000F000; iRedMask = 0x00000F00; iGreenMask = 0x000000F0; iBlueMask = 0x0000000F; break;
	case D3DFMT_P8 :
		iBitCount = 8;  iAlphaMask = 0xFF000000; iRedMask = 0x00FF0000; iGreenMask = 0x0000FF00; iBlueMask = 0x000000FF; break;
	default : assert(0 && "Unknown Format"); }
}

void CD3D_Device::TryFallingBack_OnFailedDevCreate(D3DPRESENT_PARAMETERS* pPresentationParam)
{
	// Try forcing the ZBitDepth to be the same as the screen bitdepth...
	uint32 iBitCount, iAlphaMask, iRedMask, iGreenMask, iBlueMask;
	d3d_GetColorMasks(pPresentationParam->BackBufferFormat,iBitCount,iAlphaMask,iRedMask,iGreenMask,iBlueMask);
	pPresentationParam->AutoDepthStencilFormat = GetDefaultDepthStencilFormat(iBitCount,0);
}



bool CD3D_Device::SetMode(D3DModeInfo* pMode)
{
	D3DPRESENT_PARAMETERS PresentationParam;
	SetPresentationParams(PresentationParam,pMode);
	if (m_pD3DDevice->Reset(&PresentationParam) != D3D_OK) return false;
	return true;
}

// Releases the device (and resets all that should be reset on device release)...
void CD3D_Device::FreeDevice()
{
	if (m_pD3DDevice) 
	{
		uint32 iRefCnt = m_pD3DDevice->Release(); 
	} // assert(iRefCnt==0);

	ResetDeviceVars();
}


// Release all the device objects (the Render Objects)...
bool CD3D_Device::ReleaseDevObjects()
{
	// Notify all RenderObjects that we're going to free the device (to give up all there device stuff)
	CRenderObject* pRenderObject = m_pRenderObjectList_Head;
	while (pRenderObject) 
	{ 
		pRenderObject->FreeDeviceObjects(); 
		pRenderObject = pRenderObject->GetNext(); 
	}

	return true; 
}

// If device we lost, restores the objects (calls ReCreateObject on all the Render Objects)...
bool CD3D_Device::RestoreDevObjects()
{
	// Notify all RenderObjects that they need to re-create themselves...
	CRenderObject* pRenderObject = m_pRenderObjectList_Head;
	while (pRenderObject) 
	{ 
		pRenderObject->ReCreateObject(); 
		pRenderObject = pRenderObject->GetNext(); 
	}

	return true;
}

// Do all the resetting that we need to do (device release stuff)...
void CD3D_Device::ResetDeviceVars()
{
	m_pD3DDevice				= NULL;
	m_pAdapter					= NULL;
	m_pDevice					= NULL;
	m_pMode						= NULL;
	m_bWindowed					= false;
	m_bIn3D						= false;
}

// Resets back to initial conditions (doesn't try to free anything)...
void CD3D_Device::Reset()
{
	ResetDeviceVars();
	m_pRenderObjectList_Head	= NULL;
}

// Frees all the member vars and resets afterwards...
void CD3D_Device::FreeAll()
{
	// Destoy all the Render Objects in the list...
	while (m_pRenderObjectList_Head) {
		CRenderObject* pTmp = m_pRenderObjectList_Head->GetNext();
		DestroyRenderObject(m_pRenderObjectList_Head);
		m_pRenderObjectList_Head = pTmp; } 

	FreeDevice();

	Reset();
}

void CD3D_Device::SetPresentationParams(D3DPRESENT_PARAMETERS& PresentationParam,D3DModeInfo* pMode)
{
	D3DSWAPEFFECT	SwapEffect		= D3DSWAPEFFECT_DISCARD;
	uint32			BackBufferCount	= 1; //g_CV_BackBufferCount;
	if (g_bRunWindowed) { SwapEffect = D3DSWAPEFFECT_COPY; BackBufferCount = 1; }

	// Clear all parameters 
	ZeroMemory(&PresentationParam, sizeof(PresentationParam));

	PresentationParam.BackBufferWidth					= g_bRunWindowed ? g_ScreenWidth : pMode->Width;
	PresentationParam.BackBufferHeight					= g_bRunWindowed ? g_ScreenHeight : pMode->Height;
	PresentationParam.BackBufferFormat					= pMode->Format;
	PresentationParam.BackBufferCount					= BackBufferCount;						// Number of back buffers (1 "double buffer", 2 "triple buffer", etc)
	PresentationParam.MultiSampleType					= GetDefaultMultiSampleType(0);
	PresentationParam.SwapEffect						= SwapEffect;
	PresentationParam.hDeviceWindow						= g_hWnd_RenderWindow;
	PresentationParam.Windowed							= g_bRunWindowed;
	PresentationParam.EnableAutoDepthStencil			= true;
	PresentationParam.AutoDepthStencilFormat			= GetDefaultDepthStencilFormat(32,0); //g_CV_ZBitDepth,g_CV_StencilBitDepth);
	PresentationParam.Flags								= D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;	// puffs, but need it for the console (should get rid of this eventually)...
	PresentationParam.FullScreen_RefreshRateInHz		= D3DPRESENT_RATE_DEFAULT; //D3DPRESENT_RATE_UNLIMITED
	PresentationParam.PresentationInterval				= D3DPRESENT_INTERVAL_DEFAULT;
}

// Go for the best one that works and fallback till we find one...
//	Note: Need to watch for loops here. Would be very bad.
//#define IsDepthFormatOk(MyFormat) (
//PDIRECT3D->CheckDeviceFormat(m_pAdapter->iAdapterNum,
//									  m_pDevice->DeviceType,
//									  m_pMode->Format,
//									  D3DUSAGE_DEPTHSTENCIL,D3DRTYPE_SURFACE,MyFormat) == D3D_OK ? true : false)

bool CD3D_Device::IsDepthFormatOk(D3DFORMAT DepthFormat )
{

    // Verify that the depth format exists
    HRESULT hr = PDIRECT3D->CheckDeviceFormat(m_pAdapter->iAdapterNum,
                                         m_pDevice->DeviceType,
                                         m_pMode->Format,
                                         D3DUSAGE_DEPTHSTENCIL,
                                         D3DRTYPE_SURFACE,
                                         DepthFormat);

    if ( hr != D3D_OK ) return false;

    // Verify that the depth format is compatible 
	 // !! ASSUMES backbuffer and adapter format are the same
    hr = PDIRECT3D->CheckDepthStencilMatch(m_pAdapter->iAdapterNum,
                                           m_pDevice->DeviceType,
                                           m_pMode->Format,
                                           m_pMode->Format,
                                           DepthFormat);

    if ( hr != D3D_OK ) return false;

    return true;

}




D3DFORMAT CD3D_Device::GetDefaultDepthStencilFormat(uint32 iZBitDepth,uint32 iStencilBitDepth)
{
	D3DFORMAT DepthStencilFormat = D3DFMT_UNKNOWN;
	switch (iZBitDepth) {
	case 32 : 
		switch (iStencilBitDepth) {
		case 8  : DepthStencilFormat = D3DFMT_D24S8;	if (!IsDepthFormatOk(DepthStencilFormat)) return GetDefaultDepthStencilFormat(32,4); break;
		case 4  : DepthStencilFormat = D3DFMT_D24X4S4;	if (!IsDepthFormatOk(DepthStencilFormat)) return GetDefaultDepthStencilFormat(24,8); break;
		case 1  : DepthStencilFormat = D3DFMT_D24S8;	if (!IsDepthFormatOk(DepthStencilFormat)) return GetDefaultDepthStencilFormat(32,4); break;
		case 0  : DepthStencilFormat = D3DFMT_D32;		if (!IsDepthFormatOk(DepthStencilFormat)) return GetDefaultDepthStencilFormat(24,8); break;
		default : assert(0); return D3DFMT_UNKNOWN; break; } break;
	case 24 : 
		switch (iStencilBitDepth) {
		case 8  : DepthStencilFormat = D3DFMT_D24S8;	if (!IsDepthFormatOk(DepthStencilFormat)) return GetDefaultDepthStencilFormat(24,4); break;
		case 4  : DepthStencilFormat = D3DFMT_D24X4S4;	if (!IsDepthFormatOk(DepthStencilFormat)) return GetDefaultDepthStencilFormat(24,0); break;
		case 1  : DepthStencilFormat = D3DFMT_D24S8;	if (!IsDepthFormatOk(DepthStencilFormat)) return GetDefaultDepthStencilFormat(24,4); break;
		case 0  : DepthStencilFormat = D3DFMT_D24X8;	if (!IsDepthFormatOk(DepthStencilFormat)) return GetDefaultDepthStencilFormat(16,8); break;
		default : assert(0); return D3DFMT_UNKNOWN; break; } break;
	case 16 : 
		switch (iStencilBitDepth) {
		case 8  : DepthStencilFormat = D3DFMT_D15S1;	if (!IsDepthFormatOk(DepthStencilFormat)) return GetDefaultDepthStencilFormat(16,4); break;
		case 4  : DepthStencilFormat = D3DFMT_D15S1;	if (!IsDepthFormatOk(DepthStencilFormat)) return GetDefaultDepthStencilFormat(16,0); break;
		case 1  : DepthStencilFormat = D3DFMT_D15S1;	if (!IsDepthFormatOk(DepthStencilFormat)) return GetDefaultDepthStencilFormat(16,4); break;
		case 0  : DepthStencilFormat = D3DFMT_D16;		if (!IsDepthFormatOk(DepthStencilFormat)) return D3DFMT_UNKNOWN; break;
		default : assert(0); return D3DFMT_UNKNOWN; break; } break;
	default : assert(0); return D3DFMT_UNKNOWN; }

	return DepthStencilFormat;
}

D3DMULTISAMPLE_TYPE CD3D_Device::GetDefaultMultiSampleType(uint32 Samples)
{
	D3DMULTISAMPLE_TYPE MultiSampleType = (D3DMULTISAMPLE_TYPE)Samples;
	if (PDIRECT3D->CheckDeviceMultiSampleType(m_pAdapter->iAdapterNum,m_pDevice->DeviceType,m_pMode->Format,g_bRunWindowed,MultiSampleType, NULL ) != D3D_OK) {
		if ((uint32)MultiSampleType > 0) return GetDefaultMultiSampleType((uint32)MultiSampleType-1); else return (D3DMULTISAMPLE_TYPE)0; }
	return MultiSampleType;
}

// Get the ZBuffer and Stencil Buffer Bit Depths...
void d3d_GetDepthStencilBits(D3DFORMAT iD3DFormat, uint32& iZDepth, uint32& iStencilDepth)
{
	iZDepth = 0; iStencilDepth = 0; 
	switch (iD3DFormat) {
		case D3DFMT_D16_LOCKABLE : case D3DFMT_D16 : 
			iZDepth = 16; iStencilDepth = 0; break;
		case D3DFMT_D32 :
			iZDepth = 32; iStencilDepth = 0; break;
		case D3DFMT_D15S1 :
			iZDepth = 16; iStencilDepth = 1; break;
		case D3DFMT_D24S8 :
			iZDepth = 24; iStencilDepth = 8; break;
		case D3DFMT_D24X8 :
			iZDepth = 24; iStencilDepth = 0; break;
		case D3DFMT_D24X4S4 :
			iZDepth = 24; iStencilDepth = 0; break;
		default : assert(0 && "Unknown DepthStencil Format"); }
}

// Figure out if we can render some puff or not (note that this is state dependent), 
//	I'm assuming it called with the render states set as they will be during rendering...
void CD3D_Device::PreCalcSomeDeviceCaps()
{
	if (!m_pD3DDevice) return;

	// Figure out if we can do specular highlights on models.
//	ModelSpecularStateBackup stateBackup;
//	SetModelSpecularStates(&stateBackup); unsigned long nPasses = 0;
//	m_ExtraDevCaps.m_bModelSpecularCapable = (m_pD3DDevice->ValidateDevice((unsigned long*)&nPasses) == D3D_OK);
//	UnsetModelSpecularStates(&stateBackup);

//	SetDetailTextureStates();
//	m_ExtraDevCaps.m_bDetailTextureCapable = (m_pD3DDevice->ValidateDevice((unsigned long*)&nPasses) == D3D_OK);
//	if (m_DeviceCaps.MaxSimultaneousTextures < 2) { m_ExtraDevCaps.m_bDetailTextureCapable = false; }
//	UnsetDetailTextureStates();

	// Check for stencil buffer & get the format...
	LPDIRECT3DSURFACE9 pDepthStencilBuffer = NULL;
	m_pD3DDevice->GetDepthStencilSurface(&pDepthStencilBuffer);
	if (pDepthStencilBuffer) 
	{
		D3DSURFACE_DESC SurfDesc; 
		pDepthStencilBuffer->GetDesc(&SurfDesc);
		m_ExtraDevCaps.m_DepthStencilBufferFormat = SurfDesc.Format; uint32 iZDepth,iStencilDepth;
		d3d_GetDepthStencilBits(m_ExtraDevCaps.m_DepthStencilBufferFormat,iZDepth,iStencilDepth);
		m_ExtraDevCaps.m_bHasZBuffer			  = (iZDepth>0		 ? true : false);
		m_ExtraDevCaps.m_bHasStencilBuffer		  = (iStencilDepth>0 ? true : false);
		int iRefCnt = pDepthStencilBuffer->Release(); 
	}
}

void CD3D_Device::SetDefaultRenderStates()
{
	// Basic Render State defaults...
	m_pD3DDevice->SetRenderState(D3DRS_FOGTABLEMODE, D3DFOG_LINEAR);
	m_pD3DDevice->SetRenderState(D3DRS_FOGENABLE, false); //g_CV_FogEnable);
	m_pD3DDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	m_pD3DDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
	m_pD3DDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
	m_pD3DDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
	m_pD3DDevice->SetRenderState(D3DRS_SPECULARENABLE, TRUE);	 // FALSE
	m_pD3DDevice->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
	m_pD3DDevice->SetRenderState(D3DRS_DITHERENABLE, TRUE);
	m_pD3DDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID); //g_CV_Wireframe ? D3DFILL_WIREFRAME : D3DFILL_SOLID);
	m_pD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	m_pD3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	m_pD3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	m_pD3DDevice->SetRenderState(D3DRS_COLORVERTEX, FALSE);
	m_pD3DDevice->SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_DISABLE);
	m_pD3DDevice->SetRenderState(D3DRS_INDEXEDVERTEXBLENDENABLE, FALSE);

	m_pD3DDevice->SetRenderState(D3DRS_NORMALIZENORMALS, TRUE);

	// Disable Clipping..
 	m_pD3DDevice->SetRenderState(D3DRS_CLIPPING, TRUE);

	// Setup Material (for D3D lighting)...
	m_pD3DDevice->SetRenderState(D3DRS_LIGHTING, TRUE);
	m_pD3DDevice->SetRenderState(D3DRS_AMBIENTMATERIALSOURCE,D3DMCS_MATERIAL);
	m_pD3DDevice->SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE,D3DMCS_MATERIAL);
	m_pD3DDevice->SetRenderState(D3DRS_EMISSIVEMATERIALSOURCE,D3DMCS_MATERIAL);
	m_pD3DDevice->SetRenderState(D3DRS_SPECULARMATERIALSOURCE,D3DMCS_MATERIAL);


	// Texture Stage States...
	m_pD3DDevice->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);

//dx9
	m_pD3DDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
	m_pD3DDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);

	m_pD3DDevice->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1);

//dx9
	m_pD3DDevice->SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
	m_pD3DDevice->SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);

	m_pD3DDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	m_pD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	m_pD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	m_pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	m_pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	m_pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	m_pD3DDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	m_pD3DDevice->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	m_pD3DDevice->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
	m_pD3DDevice->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	m_pD3DDevice->SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	m_pD3DDevice->SetTextureStageState(1, D3DTSS_ALPHAARG2, D3DTA_CURRENT);

//dx9
	m_pD3DDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
	m_pD3DDevice->SetSamplerState(1, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

	// Clear the Textures...
	m_pD3DDevice->SetTexture(0,NULL);
	m_pD3DDevice->SetTexture(1,NULL);

	// Setup the Material...This should be set on the per piece basis, but for the fall release I'm just setting it the once and moving on...
	D3DMATERIAL9 Material;
	D3DCOLORVALUE MatColor1; MatColor1.r = 1.0f; MatColor1.g = 1.0f; MatColor1.b = 1.0f; MatColor1.a = 1.0f;
	D3DCOLORVALUE MatColor2; MatColor2.r = 0.0f; MatColor2.g = 0.0f; MatColor2.b = 0.0f; MatColor2.a = 0.0f;
	Material.Diffuse  = MatColor1; Material.Ambient  = MatColor1;
	Material.Specular = MatColor2; Material.Emissive = MatColor2; Material.Power = 0.0f;
	g_RenderStateMgr.SetMaterial(Material);

	assert(m_pD3DDevice);
}

void CD3D_Device::SetupViewport(int32 iWidth, int32 iHeight, float fMinZ, float fMaxZ)
{
	D3DVIEWPORT9 viewportData;
	viewportData.X		= 0;	
	viewportData.Y		= 0;
	viewportData.Width	= iWidth;
	viewportData.Height = iHeight;
	viewportData.MinZ	= fMinZ;
	viewportData.MaxZ	= fMaxZ;

	HRESULT hResult = PD3DDEVICE->SetViewport(&viewportData);
	if (hResult != D3D_OK) 
	{
		OutputDebugString("IDirect3DDevice::SetCurrentViewport failed.");
		assert(0); return; 
	}
}

// Creates the requested Render Object and returns to the Engine...
CRenderObject* CD3D_Device::CreateRenderObject(CRenderObject::RENDER_OBJECT_TYPES ObjectType) 
{
	CRenderObject* pNewObject = NULL;			// Create the Sucka...
	switch (ObjectType) 
	{
//	case CRenderObject::eDebugLine :
//		pNewObject = new CDIDebugLine(); break;
//	case CRenderObject::eDebugPolygon :
//		pNewObject = new CDIDebugPolygon(); break;
//	case CRenderObject::eDebugText :
//		pNewObject = new CDIDebugText(); break;
	case CRenderObject::eRigidMesh :
		pNewObject = new CD3DRigidMesh(); break;
	case CRenderObject::eSkelMesh :
		pNewObject = new CD3DSkelMesh(); break;
//	case CRenderObject::eVAMesh :
//		pNewObject = new CD3DVAMesh(); break;
//	case CRenderObject::eABCPieceLOD :
//		pNewObject = new ABCPieceLOD(); break;
	default : return NULL; 
	} //assert(0);  }

	if (pNewObject) 
	{							// Add it to the Render Object List...
		pNewObject->SetNext(g_Device.m_pRenderObjectList_Head); 
		g_Device.m_pRenderObjectList_Head = pNewObject; 
	}

	return pNewObject;
}

bool CD3D_Device::DestroyRenderObject(CRenderObject* pObject)
{
	if (pObject == g_Device.m_pRenderObjectList_Head) {	// Remove the sucka from the Render Object List...
		g_Device.m_pRenderObjectList_Head = pObject->GetNext(); }
	else {
		CRenderObject* pPrevObject = g_Device.m_pRenderObjectList_Head;
		while (pPrevObject->GetNext() && pPrevObject->GetNext() != pObject) {
			pPrevObject = pPrevObject->GetNext(); }
		if (pPrevObject->GetNext()) {
			pPrevObject->SetNext(pPrevObject->GetNext()->GetNext()); }
		else { assert(0); return false; } }		// It's not in the list!?!

	delete pObject;								// Ok, now delete the sucka...

	return true; 
}

CD3DRenderStyle* CD3D_Device::CreateRenderStyle()
{
	// NEED TO FIGURE OUT A WAY TO SHARE RENDER STYLES - DO SOME CHECKING HERE (OR SOMETHING)...
	return (new CD3DRenderStyle);
}

void CD3D_Device::DestroyRenderStyle(CRenderStyle* pRenderStyle)
{
	if (pRenderStyle) { 
		assert(pRenderStyle->GetRefCount() == 0);
		delete pRenderStyle; pRenderStyle = NULL; }
}

// Call before you start rendering a frame...
bool CD3D_Device::Start3D()
{
	if (g_Device.m_bIn3D || !g_Device.m_pD3DDevice) return false;

	HRESULT hResult = g_Device.m_pD3DDevice->BeginScene();
	if (hResult == D3D_OK) { return (g_Device.m_bIn3D = true); }
	else { return false; }
}

// Call when you are done rendering a frame...
bool CD3D_Device::End3D()
{
	if (!g_Device.m_bIn3D || !g_Device.m_pD3DDevice) return false;
	g_Device.m_bIn3D = false;

	HRESULT hResult = g_Device.m_pD3DDevice->EndScene();

	if (hResult == D3D_OK) { return true; }
	else { return false; }
}
bool CD3D_Device::IsIn3D() { return g_Device.m_bIn3D; }

