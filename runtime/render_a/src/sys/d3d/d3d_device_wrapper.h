//////////////////////////////////////////////////////////////////////////////
// D3D Device wrapper
// Imposter class for IDirect3DDevice9 to allow state buffering

#ifndef __D3D_DEVICE_WRAPPER_H__
#define __D3D_DEVICE_WRAPPER_H__

class CDirect3DDevice9Wrapper
{
public:
	CDirect3DDevice9Wrapper();
	~CDirect3DDevice9Wrapper();

	// Access to the actual D3D device interface
	IDirect3DDevice9 *GetDevice() { return m_pDevice; }
	void SetDevice(IDirect3DDevice9 *pDevice) 
	{
		ASSERT(m_nRefCount == 0);
		// Remember that the device is created with a positive refcount
		m_nRefCount = 1;
		m_pDevice = pDevice; 
	}


	void SetStates() { ReadCurrentDeviceState(); } 

	// Flush the dirty rendering states
	void FlushStates();

	// Flush one rendering state
	void FlushState(uint32 nID);

	// Buffer a state by ID
	void BufferState(uint32 nID, uint32 nValue) {
		ASSERT(nID < k_nNumD3DStates);
		// If this state is already dirty, update the dirty list
		if (m_aCurrentDirtyIndex[nID] != k_nInvalidIndex)
		{
			m_aDirtyStates[m_aCurrentDirtyIndex[nID]] = nValue;
			ASSERT(m_aDirtyStateIDs[m_aCurrentDirtyIndex[nID]] == nID);
		}
		// Otherwise, add it to the dirty list if it needs to change
		else if (m_aCurrentStates[nID] != nValue)
		{
			m_aDirtyStates[m_nDirtyCount] = nValue;
			m_aDirtyStateIDs[m_nDirtyCount] = nID;
			m_aCurrentDirtyIndex[nID] = m_nDirtyCount;
			++m_nDirtyCount;
			ASSERT(m_nDirtyCount <= k_nNumD3DStates);
		}
	}

	// Get the state ID for a TSS
	uint32 GetTSSStateID(uint32 nStage, uint32 nID) { return k_nNumRenderStates + (nStage * k_nNumTSSStates) + nID; }

// IDirect3DDevice9 wrapper implementation
public:
    /*** IUnknown methods ***/
    HRESULT QueryInterface(REFIID riid, void** ppvObj) { return m_pDevice->QueryInterface(riid, ppvObj); }
    ULONG AddRef() {
		if (!m_pDevice) return 0;
		++m_nRefCount;
		return m_pDevice->AddRef();
	}
    ULONG Release() {
		if (!m_nRefCount) return 0;
		if (!m_pDevice) return 0;
		ULONG nResult = m_pDevice->Release();
		--m_nRefCount;
		if (!m_nRefCount) m_pDevice = 0;
		return nResult;
	}

    /*** IDirect3DDevice9 methods ***/
    HRESULT TestCooperativeLevel() { return m_pDevice->TestCooperativeLevel(); }
    UINT GetAvailableTextureMem() { return m_pDevice->GetAvailableTextureMem(); }
    HRESULT EvictManagedResources() { return m_pDevice->EvictManagedResources(); }
    HRESULT GetDirect3D(IDirect3D9** ppD3D9) { return m_pDevice->GetDirect3D(ppD3D9); }
    HRESULT GetDeviceCaps(D3DCAPS9* pCaps) { return m_pDevice->GetDeviceCaps(pCaps); }
    HRESULT GetDisplayMode(D3DDISPLAYMODE* pMode) { return m_pDevice->GetDisplayMode(0, pMode); }
    HRESULT GetCreationParameters(D3DDEVICE_CREATION_PARAMETERS *pParameters) { return m_pDevice->GetCreationParameters(pParameters); }
    HRESULT SetCursorProperties(UINT XHotSpot,UINT YHotSpot,IDirect3DSurface9* pCursorBitmap) { m_pDevice->SetCursorProperties(XHotSpot, YHotSpot, pCursorBitmap); }
    void SetCursorPosition(UINT XScreenSpace,UINT YScreenSpace,DWORD Flags) { m_pDevice->SetCursorPosition(XScreenSpace, YScreenSpace, Flags); }
    BOOL ShowCursor(BOOL bShow) { return m_pDevice->ShowCursor(bShow); }
    HRESULT CreateAdditionalSwapChain(D3DPRESENT_PARAMETERS* pPresentationParameters,IDirect3DSwapChain9** pSwapChain) { return m_pDevice->CreateAdditionalSwapChain(pPresentationParameters, pSwapChain); }
    HRESULT Reset(D3DPRESENT_PARAMETERS* pPresentationParameters) { return m_pDevice->Reset(pPresentationParameters); }
    HRESULT Present(CONST RECT* pSourceRect,CONST RECT* pDestRect,HWND hDestWindowOverride,CONST RGNDATA* pDirtyRegion) { return m_pDevice->Present(pSourceRect,pDestRect, hDestWindowOverride, pDirtyRegion); }
    HRESULT GetBackBuffer(UINT BackBuffer,D3DBACKBUFFER_TYPE Type,IDirect3DSurface9** ppBackBuffer) { return m_pDevice->GetBackBuffer(0, BackBuffer, Type, ppBackBuffer); }
    HRESULT GetRasterStatus(D3DRASTER_STATUS* pRasterStatus) { return m_pDevice->GetRasterStatus(0, pRasterStatus); }
    void SetGammaRamp(DWORD Flags,CONST D3DGAMMARAMP* pRamp) { m_pDevice->SetGammaRamp(0, Flags, pRamp); }
    void GetGammaRamp(D3DGAMMARAMP* pRamp) { m_pDevice->GetGammaRamp(0, pRamp); }
    HRESULT CreateTexture(UINT Width,UINT Height,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DTexture9** ppTexture) { return m_pDevice->CreateTexture(Width,Height,Levels,Usage,Format,Pool,ppTexture, NULL); }
    HRESULT CreateVolumeTexture(UINT Width,UINT Height,UINT Depth,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DVolumeTexture9** ppVolumeTexture) { return m_pDevice->CreateVolumeTexture(Width,Height,Depth,Levels,Usage,Format,Pool,ppVolumeTexture, NULL); }
    HRESULT CreateCubeTexture(UINT EdgeLength,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DCubeTexture9** ppCubeTexture) { return m_pDevice->CreateCubeTexture(EdgeLength,Levels,Usage,Format,Pool,ppCubeTexture, NULL); }
    HRESULT CreateVertexBuffer(UINT Length,DWORD Usage,DWORD FVF,D3DPOOL Pool,IDirect3DVertexBuffer9** ppVertexBuffer) { return m_pDevice->CreateVertexBuffer(Length,Usage,FVF,Pool,ppVertexBuffer, NULL); }
    HRESULT CreateIndexBuffer(UINT Length,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DIndexBuffer9** ppIndexBuffer) { return m_pDevice->CreateIndexBuffer(Length,Usage,Format,Pool,ppIndexBuffer, NULL); }
    HRESULT CreateRenderTarget(UINT Width,UINT Height,D3DFORMAT Format,D3DMULTISAMPLE_TYPE MultiSample,DWORD MultisampleQuality,BOOL Lockable,IDirect3DSurface9** ppSurface) { return m_pDevice->CreateRenderTarget(Width,Height,Format,MultiSample,MultisampleQuality,Lockable,ppSurface, NULL); }
    HRESULT CreateDepthStencilSurface(UINT Width,UINT Height,D3DFORMAT Format,D3DMULTISAMPLE_TYPE MultiSample,DWORD MultisampleQuality,BOOL discard, IDirect3DSurface9** ppSurface, HANDLE *pHandle) { return m_pDevice->CreateDepthStencilSurface(Width,Height,Format,MultiSample,MultisampleQuality, discard, ppSurface, pHandle); }
    HRESULT CreateOffscreenPlainSurface(UINT Width,UINT Height,D3DFORMAT Format,D3DPOOL Pool,IDirect3DSurface9** ppSurface) { return m_pDevice->CreateOffscreenPlainSurface(Width,Height,Format,Pool,ppSurface, NULL); }
    HRESULT CreateQuery(D3DQUERYTYPE Type,IDirect3DQuery9** ppQuery) { return m_pDevice->CreateQuery(Type,ppQuery); }
    HRESULT UpdateSurface(IDirect3DSurface9* pSourceSurface,CONST RECT* pSourceRectsArray,IDirect3DSurface9* pDestinationSurface,CONST POINT* pDestPointsArray) { return m_pDevice->UpdateSurface(pSourceSurface,pSourceRectsArray,pDestinationSurface,pDestPointsArray); }
    HRESULT UpdateTexture(IDirect3DBaseTexture9* pSourceTexture,IDirect3DBaseTexture9* pDestinationTexture) { return m_pDevice->UpdateTexture(pSourceTexture,pDestinationTexture); }
    HRESULT SetRenderTarget(IDirect3DSurface9* pRenderTarget,IDirect3DSurface9* pNewZStencil) {
        HRESULT hr;
        hr = m_pDevice->SetRenderTarget(0, pRenderTarget);
        if (hr != D3D_OK)
            return hr;
        return m_pDevice->SetDepthStencilSurface(pNewZStencil);
    }
    HRESULT GetRenderTarget(IDirect3DSurface9** ppRenderTarget) { return m_pDevice->GetRenderTarget(0, ppRenderTarget); }
    HRESULT GetDepthStencilSurface(IDirect3DSurface9** ppZStencilSurface) { return m_pDevice->GetDepthStencilSurface(ppZStencilSurface); }
    HRESULT BeginScene() { return m_pDevice->BeginScene(); }
    HRESULT EndScene() { return m_pDevice->EndScene(); }
    HRESULT Clear(DWORD Count,CONST D3DRECT* pRects,DWORD Flags,D3DCOLOR Color,float Z,DWORD Stencil) { return m_pDevice->Clear(Count,pRects,Flags,Color,Z,Stencil); }
    HRESULT SetTransform(D3DTRANSFORMSTATETYPE State,CONST D3DMATRIX* pMatrix) { return m_pDevice->SetTransform(State,pMatrix); }
    HRESULT GetTransform(D3DTRANSFORMSTATETYPE State,D3DMATRIX* pMatrix) { return m_pDevice->GetTransform(State,pMatrix); }
    HRESULT MultiplyTransform(D3DTRANSFORMSTATETYPE State,CONST D3DMATRIX* pMatrix) { return m_pDevice->MultiplyTransform(State,pMatrix); }
    HRESULT SetViewport(CONST D3DVIEWPORT9* pViewport) { return m_pDevice->SetViewport(pViewport); }
    HRESULT GetViewport(D3DVIEWPORT9* pViewport) { return m_pDevice->GetViewport(pViewport); }
    HRESULT SetMaterial(CONST D3DMATERIAL9* pMaterial) { return m_pDevice->SetMaterial(pMaterial); }
    HRESULT GetMaterial(D3DMATERIAL9* pMaterial) { return m_pDevice->GetMaterial(pMaterial); }
    HRESULT SetLight(DWORD Index,CONST D3DLIGHT9* pLight) { return m_pDevice->SetLight(Index,pLight); }
    HRESULT GetLight(DWORD Index,D3DLIGHT9* pLight) { return m_pDevice->GetLight(Index,pLight); }
    HRESULT LightEnable(DWORD Index,BOOL Enable) { return m_pDevice->LightEnable(Index, Enable); }
    HRESULT GetLightEnable(DWORD Index,BOOL* pEnable) { return m_pDevice->GetLightEnable(Index, pEnable); }
    HRESULT SetClipPlane(DWORD Index,CONST float* pPlane) { return m_pDevice->SetClipPlane(Index, pPlane); }
    HRESULT GetClipPlane(DWORD Index,float* pPlane) { return m_pDevice->GetClipPlane(Index, pPlane); }
	// Rendering state buffering replacement
	/*
    HRESULT SetRenderState(D3DRENDERSTATETYPE State,DWORD Value) { BufferState((uint32)State, (uint32)Value); return S_OK; }
    HRESULT GetRenderState(D3DRENDERSTATETYPE State,DWORD* pValue) { *pValue = (m_aCurrentDirtyIndex[State] == k_nInvalidIndex) ? m_aCurrentStates[State] : m_aDirtyStates[m_aCurrentDirtyIndex[State]]; return S_OK; }
	//*/
	//*
    HRESULT SetRenderState(D3DRENDERSTATETYPE State,DWORD Value) { return m_pDevice->SetRenderState(State, Value); }
    HRESULT GetRenderState(D3DRENDERSTATETYPE State,DWORD* pValue) { return m_pDevice->GetRenderState(State, pValue); }
	//*/
    HRESULT BeginStateBlock() { FlushStates(); return m_pDevice->BeginStateBlock(); }
    HRESULT EndStateBlock(IDirect3DStateBlock9 ** ppSB) { FlushStates(); return m_pDevice->EndStateBlock(ppSB); }
    HRESULT CreateStateBlock(D3DSTATEBLOCKTYPE Type,IDirect3DStateBlock9 **ppSB) { return m_pDevice->CreateStateBlock(Type, ppSB); }
    HRESULT SetClipStatus(CONST D3DCLIPSTATUS9* pClipStatus) { return m_pDevice->SetClipStatus(pClipStatus); }
    HRESULT GetClipStatus(D3DCLIPSTATUS9* pClipStatus) { return m_pDevice->GetClipStatus(pClipStatus); }
    HRESULT GetTexture(DWORD Stage,IDirect3DBaseTexture9** ppTexture) { return m_pDevice->GetTexture(Stage,ppTexture); }
    HRESULT SetTexture(DWORD Stage,IDirect3DBaseTexture9* pTexture) { return m_pDevice->SetTexture(Stage,pTexture); }
	// Rendering state buffering replacement, which operates by passing it off to Get/SetRenderState with a TSS-specific ID
	/*
    HRESULT GetTextureStageState(DWORD Stage,D3DTEXTURESTAGESTATETYPE Type,DWORD* pValue) { return GetRenderState((D3DRENDERSTATETYPE)GetTSSStateID(Stage,Type), pValue); }
    HRESULT SetTextureStageState(DWORD Stage,D3DTEXTURESTAGESTATETYPE Type,DWORD Value) { return SetRenderState((D3DRENDERSTATETYPE)GetTSSStateID(Stage,Type), Value); }
	//*/
	//*
    HRESULT GetTextureStageState(DWORD Stage,D3DTEXTURESTAGESTATETYPE Type,DWORD* pValue) { return m_pDevice->GetTextureStageState(Stage,Type,pValue); }
    HRESULT SetTextureStageState(DWORD Stage,D3DTEXTURESTAGESTATETYPE Type,DWORD Value) { return m_pDevice->SetTextureStageState(Stage,Type,Value); }
	//*/
    HRESULT ValidateDevice(DWORD* pNumPasses) { FlushStates(); return m_pDevice->ValidateDevice(pNumPasses); }
    HRESULT SetPaletteEntries(UINT PaletteNumber,CONST PALETTEENTRY* pEntries) { return m_pDevice->SetPaletteEntries(PaletteNumber,pEntries); }
    HRESULT GetPaletteEntries(UINT PaletteNumber,PALETTEENTRY* pEntries) { return m_pDevice->GetPaletteEntries(PaletteNumber,pEntries); }
    HRESULT SetCurrentTexturePalette(UINT PaletteNumber) { return m_pDevice->SetCurrentTexturePalette(PaletteNumber); }
    HRESULT GetCurrentTexturePalette(UINT *PaletteNumber) { return m_pDevice->GetCurrentTexturePalette(PaletteNumber); }
    HRESULT DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType,UINT StartVertex,UINT PrimitiveCount) { FlushStates(); return m_pDevice->DrawPrimitive(PrimitiveType,StartVertex,PrimitiveCount); }
    HRESULT DrawIndexedPrimitive(D3DPRIMITIVETYPE PrimitiveType,UINT iVertexIndex, UINT minIndex,UINT NumVertices,UINT startIndex,UINT primCount) {
		FlushStates();
		return m_pDevice->DrawIndexedPrimitive(PrimitiveType,iVertexIndex, minIndex,NumVertices,startIndex,primCount);
	}
    HRESULT DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType,UINT PrimitiveCount,CONST void* pVertexStreamZeroData,UINT VertexStreamZeroStride) {
		FlushStates();
		return m_pDevice->DrawPrimitiveUP(PrimitiveType,PrimitiveCount,pVertexStreamZeroData,VertexStreamZeroStride);
	}

    HRESULT DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType,UINT MinVertexIndex,UINT NumVertexIndices,UINT PrimitiveCount,CONST void* pIndexData,D3DFORMAT IndexDataFormat,CONST void* pVertexStreamZeroData,UINT VertexStreamZeroStride) { FlushStates(); return m_pDevice->DrawIndexedPrimitiveUP(PrimitiveType,MinVertexIndex,NumVertexIndices,PrimitiveCount,pIndexData,IndexDataFormat,pVertexStreamZeroData,VertexStreamZeroStride); }
    HRESULT ProcessVertices(UINT SrcStartIndex,UINT DestIndex,UINT VertexCount,IDirect3DVertexBuffer9* pDestBuffer,IDirect3DVertexDeclaration9* pVertexDecl,DWORD Flags) { FlushStates(); return m_pDevice->ProcessVertices(SrcStartIndex,DestIndex,VertexCount,pDestBuffer,pVertexDecl,Flags); }
	HRESULT CreateVertexDeclaration(CONST D3DVERTEXELEMENT9* pVertexElements, IDirect3DVertexDeclaration9** ppDecl)	{ return m_pDevice->CreateVertexDeclaration(pVertexElements, ppDecl); }
	HRESULT SetVertexDeclaration(IDirect3DVertexDeclaration9 *pDecl)	{ return m_pDevice->SetVertexDeclaration(pDecl); }
	HRESULT GetVertexDeclaration(IDirect3DVertexDeclaration9** ppDecl)	{ return m_pDevice->GetVertexDeclaration(ppDecl); }
	HRESULT CreateVertexShader(CONST DWORD *pFunction,IDirect3DVertexShader9** ppShader) { return m_pDevice->CreateVertexShader(pFunction, ppShader); }
    HRESULT SetVertexShader(IDirect3DVertexShader9* pShader) {
		// Flush the states that have to be set before changing the vertex shader
		FlushCriticalStates();
		return m_pDevice->SetVertexShader(pShader);
	}
	HRESULT SetFVF(DWORD FVF) { return m_pDevice->SetFVF(FVF); }
	HRESULT SetSamplerState(DWORD Sampler,D3DSAMPLERSTATETYPE Type,DWORD Value) { return m_pDevice->SetSamplerState(Sampler,Type,Value); }
	HRESULT GetSamplerState(DWORD Sampler,D3DSAMPLERSTATETYPE Type,DWORD *Value) { return m_pDevice->GetSamplerState(Sampler,Type,Value); }
    HRESULT GetVertexShader(IDirect3DVertexShader9** ppShader) { return m_pDevice->GetVertexShader(ppShader); }
	HRESULT SetVertexShaderConstantB(UINT StartRegister,CONST BOOL *pConstantData,UINT BoolCount) { return m_pDevice->SetVertexShaderConstantB(StartRegister, pConstantData, BoolCount); }
	HRESULT GetVertexShaderConstantB(UINT StartRegister,BOOL *pConstantData,UINT BoolCount) { return m_pDevice->GetVertexShaderConstantB(StartRegister, pConstantData, BoolCount); }
	HRESULT SetVertexShaderConstantF(UINT StartRegister,CONST float *pConstantData,UINT Vector4fCount) { return m_pDevice->SetVertexShaderConstantF(StartRegister, pConstantData, Vector4fCount); }
	HRESULT GetVertexShaderConstantF(UINT StartRegister,float *pConstantData,UINT Vector4fCount) { return m_pDevice->GetVertexShaderConstantF(StartRegister, pConstantData, Vector4fCount); }
	HRESULT SetVertexShaderConstantI(UINT StartRegister,CONST int *pConstantData,UINT Vector4iCount) { return m_pDevice->SetVertexShaderConstantI(StartRegister, pConstantData, Vector4iCount); }
	HRESULT GetVertexShaderConstantI(UINT StartRegister,int *pConstantData,UINT Vector4iCount) { return m_pDevice->GetVertexShaderConstantI(StartRegister, pConstantData, Vector4iCount); }

    HRESULT SetStreamSource(UINT StreamNumber,IDirect3DVertexBuffer9* pStreamData,UINT OffsetInBytes,UINT Stride) { FlushCriticalStates(); return m_pDevice->SetStreamSource(StreamNumber,pStreamData,OffsetInBytes,Stride); }
    HRESULT GetStreamSource(UINT StreamNumber,IDirect3DVertexBuffer9** ppStreamData,UINT *OffsetInBytes,UINT* pStride) { return m_pDevice->GetStreamSource(StreamNumber,ppStreamData,OffsetInBytes,pStride); }
    HRESULT SetIndices(IDirect3DIndexBuffer9* pIndexData) { FlushCriticalStates(); return m_pDevice->SetIndices(pIndexData); }
    HRESULT GetIndices(IDirect3DIndexBuffer9** ppIndexData) { return m_pDevice->GetIndices(ppIndexData); }
    HRESULT CreatePixelShader(CONST DWORD *pFunction,IDirect3DPixelShader9** ppShader) { return m_pDevice->CreatePixelShader(pFunction,ppShader); }
    HRESULT SetPixelShader(IDirect3DPixelShader9* pShader) { return m_pDevice->SetPixelShader(pShader); }
    HRESULT GetPixelShader(IDirect3DPixelShader9** ppShader) { return m_pDevice->GetPixelShader(ppShader); }
	HRESULT SetPixelShaderConstantB(UINT StartRegister,CONST BOOL *pConstantData,UINT BoolCount) { return m_pDevice->SetPixelShaderConstantB(StartRegister, pConstantData, BoolCount); }
	HRESULT GetPixelShaderConstantB(UINT StartRegister,BOOL *pConstantData,UINT BoolCount) { return m_pDevice->GetPixelShaderConstantB(StartRegister, pConstantData, BoolCount); }
	HRESULT SetPixelShaderConstantF(UINT StartRegister,CONST float *pConstantData,UINT Vector4fCount) { return m_pDevice->SetPixelShaderConstantF(StartRegister, pConstantData, Vector4fCount); }
	HRESULT GetPixelShaderConstantF(UINT StartRegister,float *pConstantData,UINT Vector4fCount) { return m_pDevice->GetPixelShaderConstantF(StartRegister, pConstantData, Vector4fCount); }
	HRESULT SetPixelShaderConstantI(UINT StartRegister,CONST int *pConstantData,UINT Vector4iCount) { return m_pDevice->SetPixelShaderConstantI(StartRegister, pConstantData, Vector4iCount); }
	HRESULT GetPixelShaderConstantI(UINT StartRegister,int *pConstantData,UINT Vector4iCount) { return m_pDevice->GetPixelShaderConstantI(StartRegister, pConstantData, Vector4iCount); }

    HRESULT DrawRectPatch(UINT Handle,CONST float* pNumSegs,CONST D3DRECTPATCH_INFO* pRectPatchInfo) { FlushStates(); return m_pDevice->DrawRectPatch(Handle,pNumSegs,pRectPatchInfo); }
    HRESULT DrawTriPatch(UINT Handle,CONST float* pNumSegs,CONST D3DTRIPATCH_INFO* pTriPatchInfo) { FlushStates(); return m_pDevice->DrawTriPatch(Handle,pNumSegs,pTriPatchInfo); }
    HRESULT DeletePatch(UINT Handle) { return m_pDevice->DeletePatch(Handle); }
	HRESULT SetSoftwareVertexProcessing(BOOL bSoftware) { return m_pDevice->SetSoftwareVertexProcessing(bSoftware); }


private:
	// The actual device pointer
	IDirect3DDevice9 *m_pDevice;

	// Flushes states that are critical to being flushed before changing a key primitive
	// such as an index buffer, vertex shader, etc.
	void FlushCriticalStates()
	{
		// no longer a state for DX9
		FlushState(k_nSVPState);
		FlushState((uint32)D3DRS_LIGHTING);
	}

	// D3D state statistics
	enum { k_nNumTSSStates = 33 };
	enum { k_nNumTextureStages = 8 };
	enum { k_nNumRenderStates = 210 };
	enum { k_nNumD3DStates = k_nNumRenderStates + (k_nNumTSSStates * k_nNumTextureStages) + 1 };	// +1 is for software vertex processing
	enum { k_nSVPState = k_nNumD3DStates-1 };
	enum { k_nInvalidIndex = 0xFFFFFFFF };

	// The current rendering state of the device
	uint32 m_aCurrentStates[k_nNumD3DStates];
	// If a state change has been requested, this is the index in the dirty list
	// Otherwise, it is set to k_nInvalidIndex
	uint32 m_aCurrentDirtyIndex[k_nNumD3DStates];
	// The values of the states which haven't been uploaded to the device yet
	uint32 m_aDirtyStates[k_nNumD3DStates];
	// The ID's of the states which haven't been uploaded to the device yet
	uint32 m_aDirtyStateIDs[k_nNumD3DStates];

	// Number of dirty states currently awaiting uploading
	uint32 m_nDirtyCount;

	// Refcount so we know when to let go of the device
	uint32 m_nRefCount;

	// Read the current device state from the device
	void ReadCurrentDeviceState();
};


#endif //__D3D_DEVICE_WRAPPER_H__
