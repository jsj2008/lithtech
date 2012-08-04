
// D3D Utility Functions...
#include "precompile.h"

#include "d3d_utils.h"
#include "pixelformat.h"
#include "renderstruct.h"
#include "common_stuff.h"


D3DFORMAT d3d_PFormatToD3DFormat(const PFormat *pFormat)
{
	switch (pFormat->GetType()) 
	{
	case BPP_S3TC_DXT1 : 
		return D3DFMT_DXT1; break;
	case BPP_S3TC_DXT3 : 
		return D3DFMT_DXT3; break;
	case BPP_S3TC_DXT5 : 
		return D3DFMT_DXT5; break;
	case BPP_32		   : 
		if (pFormat->m_Masks[CP_ALPHA] == 0xFF000000 && pFormat->m_Masks[CP_RED]  == 0x00FF0000 && pFormat->m_Masks[CP_GREEN] == 0x0000FF00 && pFormat->m_Masks[CP_BLUE] == 0x000000FF) return D3DFMT_A8R8G8B8;
		else if (pFormat->m_Masks[CP_ALPHA] == 0x00000000 && pFormat->m_Masks[CP_RED]  == 0x00FF0000 && pFormat->m_Masks[CP_GREEN] == 0x0000FF00 && pFormat->m_Masks[CP_BLUE] == 0x000000FF) return D3DFMT_X8R8G8B8;
		assert(0); break;
	case BPP_24		   : 
		if (pFormat->m_Masks[CP_ALPHA] == 0x00000000 && pFormat->m_Masks[CP_RED]  == 0x00FF0000 && pFormat->m_Masks[CP_GREEN] == 0x0000FF00 && pFormat->m_Masks[CP_BLUE] == 0x000000FF) return D3DFMT_R8G8B8;
		assert(0); break;
	case BPP_16		   : 
		if (pFormat->m_Masks[CP_ALPHA] == 0x00000000 && pFormat->m_Masks[CP_RED]  == 0x0000F800 && pFormat->m_Masks[CP_GREEN] == 0x000007E0 && pFormat->m_Masks[CP_BLUE] == 0x0000001F) return D3DFMT_R5G6B5;
		else if (pFormat->m_Masks[CP_ALPHA] == 0x00008000 && pFormat->m_Masks[CP_RED]  == 0x00007C00 && pFormat->m_Masks[CP_GREEN] == 0x000003E0 && pFormat->m_Masks[CP_BLUE] == 0x0000001F) return D3DFMT_A1R5G5B5;
		else if (pFormat->m_Masks[CP_ALPHA] == 0x00000000 && pFormat->m_Masks[CP_RED]  == 0x00007C00 && pFormat->m_Masks[CP_GREEN] == 0x000003E0 && pFormat->m_Masks[CP_BLUE] == 0x0000001F) return D3DFMT_X1R5G5B5;
		else if (pFormat->m_Masks[CP_ALPHA] == 0x0000F000 && pFormat->m_Masks[CP_RED]  == 0x00000F00 && pFormat->m_Masks[CP_GREEN] == 0x000000F0 && pFormat->m_Masks[CP_BLUE] == 0x0000000F) return D3DFMT_A4R4G4B4;
		assert(0); break;
	case BPP_8		   :	
		return D3DFMT_P8;	break; 
	}
	return (D3DFORMAT)NULL;
}

void d3d_GetColorMasks(D3DFORMAT iD3DFormat, uint32& iBitCount, uint32& iAlphaMask, uint32& iRedMask, uint32& iGreenMask, uint32& iBlueMask)
{
	switch (iD3DFormat) 
	{
	case D3DFMT_R8G8B8 :
		iBitCount = 24; iAlphaMask = 0x00000000; iRedMask = 0x00FF0000; iGreenMask = 0x0000FF00; iBlueMask = 0x000000FF; break;
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

	//For bumpmaps - Red = Luminance, Green = V, Blue = U
	case D3DFMT_L6V5U5:
		iBitCount = 16; iAlphaMask = 0x00000000; iRedMask = 0x0000F800; iGreenMask = 0x000007C0; iBlueMask = 0x0000003F; break;
	case D3DFMT_X8L8V8U8:
		iBitCount = 32; iAlphaMask = 0x00000000; iRedMask = 0x00FF0000; iGreenMask = 0x0000FF00; iBlueMask = 0x000000FF; break;
	case D3DFMT_V8U8:
		iBitCount = 16; iAlphaMask = 0x00000000; iRedMask = 0x00000000; iGreenMask = 0x0000FF00; iBlueMask = 0x000000FF; break;
	case D3DFMT_V16U16:
		iBitCount = 32; iAlphaMask = 0x00000000; iRedMask = 0x00000000; iGreenMask = 0xFFFF0000; iBlueMask = 0x0000FFFF; break;
	case D3DFMT_Q8W8V8U8:
		iBitCount = 32; iAlphaMask = 0x00000000; iRedMask = 0x00000000; iGreenMask = 0x00FF0000; iBlueMask = 0x000000FF; break;
// - This went away in DX9?
//	case D3DFMT_W11V11U10:
//		iBitCount = 32; iAlphaMask = 0x00000000; iRedMask = 0x00000000; iGreenMask = 0x001FFC00; iBlueMask = 0x000003FF; break;

	default : 
		assert(0 && "Unknown Format"); 
	}
}

bool d3d_D3DFormatToPFormat(D3DFORMAT iD3DFormat, PFormat *pFormat)
{
	//check for invalid formats
	if(iD3DFormat == D3DFMT_UNKNOWN)
		return false;

	uint32 iBitCount = 0, iAlphaMask = 0, iRedMask = 0, iGreenMask = 0, iBlueMask = 0;
	switch (iD3DFormat) 
	{	
	// Check for compressed formats first...
	case D3DFMT_DXT1 : pFormat->Init(BPP_S3TC_DXT1,iAlphaMask,iRedMask,iGreenMask,iBlueMask); return true; break;
	case D3DFMT_DXT3 : pFormat->Init(BPP_S3TC_DXT3,iAlphaMask,iRedMask,iGreenMask,iBlueMask); return true; break;
	case D3DFMT_DXT5 : pFormat->Init(BPP_S3TC_DXT5,iAlphaMask,iRedMask,iGreenMask,iBlueMask); return true; break; 
	}

	d3d_GetColorMasks(iD3DFormat,iBitCount,iAlphaMask,iRedMask,iGreenMask,iBlueMask);
	switch (iBitCount) 
	{
	case 32 : pFormat->Init(BPP_32,iAlphaMask,iRedMask,iGreenMask,iBlueMask); return true; break;
	case 24 : pFormat->Init(BPP_24,iAlphaMask,iRedMask,iGreenMask,iBlueMask); return true; break;
	case 16 : pFormat->Init(BPP_16,iAlphaMask,iRedMask,iGreenMask,iBlueMask); return true; break;
	case 8  : pFormat->Init( BPP_8,iAlphaMask,iRedMask,iGreenMask,iBlueMask); return true; break;
	}

	//unknown format
	assert(!"Unknown texture format");
	return false; 
}

// Get the ZBuffer and Stencil Buffer Bit Depths...
void d3d_GetDepthStencilBits(D3DFORMAT iD3DFormat, uint32& iZDepth, uint32& iStencilDepth)
{
	iZDepth = 0; iStencilDepth = 0; 
	switch (iD3DFormat) {
		case D3DFMT_D16_LOCKABLE : 
		case D3DFMT_D16 : 
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
		default : assert(0 && "Unknown DepthStencil Format"); 
	}
}

void d3d_AddToString(char *pStr, const char *pToAdd, uint32 nBufferLen)
{
	LTStrCat(pStr, pToAdd, nBufferLen);
	LTStrCat(pStr, " ", nBufferLen);
}

void  d3d_D3DFormatToString(D3DFORMAT Format, char *pStr, uint32 nBufferLen)
{
	switch (Format) 
	{

	case D3DFMT_R8G8B8 :			d3d_AddToString(pStr, "D3DFMT_R8G8B8", nBufferLen); break;
	case D3DFMT_A8R8G8B8 :			d3d_AddToString(pStr, "D3DFMT_A8R8G8B8", nBufferLen); break;
	case D3DFMT_X8R8G8B8 :			d3d_AddToString(pStr, "D3DFMT_X8R8G8B8", nBufferLen); break;
	case D3DFMT_R5G6B5 :			d3d_AddToString(pStr, "D3DFMT_R5G6B5", nBufferLen); break;
	case D3DFMT_X1R5G5B5 :			d3d_AddToString(pStr, "D3DFMT_X1R5G5B5", nBufferLen); break;
	case D3DFMT_A1R5G5B5 :			d3d_AddToString(pStr, "D3DFMT_A1R5G5B5", nBufferLen); break;
	case D3DFMT_A4R4G4B4 :			d3d_AddToString(pStr, "D3DFMT_A4R4G4B4", nBufferLen); break;
	case D3DFMT_R3G3B2 :			d3d_AddToString(pStr, "D3DFMT_R3G3B2", nBufferLen); break;
	case D3DFMT_A8 :				d3d_AddToString(pStr, "D3DFMT_A8", nBufferLen); break;
	case D3DFMT_A8R3G3B2 :			d3d_AddToString(pStr, "D3DFMT_A8R3G3B2", nBufferLen); break;
	case D3DFMT_X4R4G4B4 :			d3d_AddToString(pStr, "D3DFMT_X4R4G4B4", nBufferLen); break;
	case D3DFMT_A8P8 :				d3d_AddToString(pStr, "D3DFMT_A8P8", nBufferLen); break;
	case D3DFMT_P8 :				d3d_AddToString(pStr, "D3DFMT_P8", nBufferLen); break;
	case D3DFMT_L8 :				d3d_AddToString(pStr, "D3DFMT_L8", nBufferLen); break;
	case D3DFMT_A8L8 :				d3d_AddToString(pStr, "D3DFMT_A8L8", nBufferLen); break;
	case D3DFMT_A4L4 :				d3d_AddToString(pStr, "D3DFMT_A4L4", nBufferLen); break;
	case D3DFMT_V8U8 :				d3d_AddToString(pStr, "D3DFMT_V8U8", nBufferLen); break;
	case D3DFMT_L6V5U5 :			d3d_AddToString(pStr, "D3DFMT_L6V5U5", nBufferLen); break;
	case D3DFMT_X8L8V8U8 :			d3d_AddToString(pStr, "D3DFMT_X8L8V8U8", nBufferLen); break;
	case D3DFMT_Q8W8V8U8 :			d3d_AddToString(pStr, "D3DFMT_Q8W8V8U8", nBufferLen); break;
	case D3DFMT_V16U16 :			d3d_AddToString(pStr, "D3DFMT_V16U16", nBufferLen); break;
// This went away in DX9
//	case D3DFMT_W11V11U10 :			d3d_AddToString(pStr, "D3DFMT_W11V11U10", nBufferLen); break;
	case D3DFMT_UYVY :				d3d_AddToString(pStr, "D3DFMT_UYVY", nBufferLen); break;
	case D3DFMT_YUY2 :				d3d_AddToString(pStr, "D3DFMT_YUY2", nBufferLen); break;
	case D3DFMT_DXT1 :				d3d_AddToString(pStr, "D3DFMT_DXT1", nBufferLen); break;
	case D3DFMT_DXT2 :				d3d_AddToString(pStr, "D3DFMT_DXT2", nBufferLen); break;
	case D3DFMT_DXT3 :				d3d_AddToString(pStr, "D3DFMT_DXT3", nBufferLen); break;
	case D3DFMT_DXT4 :				d3d_AddToString(pStr, "D3DFMT_DXT4", nBufferLen); break;
	case D3DFMT_DXT5 :				d3d_AddToString(pStr, "D3DFMT_DXT5", nBufferLen); break;
	case D3DFMT_D16_LOCKABLE :		d3d_AddToString(pStr, "D3DFMT_D16_LOCKABLE", nBufferLen); break;
	case D3DFMT_D32 :				d3d_AddToString(pStr, "D3DFMT_D32", nBufferLen); break;
	case D3DFMT_D15S1 :				d3d_AddToString(pStr, "D3DFMT_D15S1", nBufferLen); break;
	case D3DFMT_D24S8 :				d3d_AddToString(pStr, "D3DFMT_D24S8", nBufferLen); break;
	case D3DFMT_D16 :				d3d_AddToString(pStr, "D3DFMT_D16", nBufferLen); break;
	case D3DFMT_D24X8 :				d3d_AddToString(pStr, "D3DFMT_D24X8", nBufferLen); break;
	case D3DFMT_D24X4S4 :			d3d_AddToString(pStr, "D3DFMT_D24X4S4", nBufferLen); break;
	case D3DFMT_VERTEXDATA :		d3d_AddToString(pStr, "D3DFMT_VERTEXDATA", nBufferLen); break;
	case D3DFMT_INDEX16 :			d3d_AddToString(pStr, "D3DFMT_INDEX16", nBufferLen); break;
	case D3DFMT_INDEX32 :			d3d_AddToString(pStr, "D3DFMT_INDEX32", nBufferLen); break;
	
	default : 
		assert(0 && "Unknown Format Type"); 
	}
}

bool d3d_CheckCVar(char *pVarName, const char *pVal)
{
	HLTPARAM hParam;
	if (hParam = g_pStruct->GetParameter(pVarName)) {
		if (stricmp(g_pStruct->GetParameterValueString(hParam), pVal) == 0) return true; }
	return false;
}



