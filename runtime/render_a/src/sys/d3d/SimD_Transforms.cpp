/*
//#include "bdefs.h"
#pragma warning(disable: 858)

// SimD
#include <d3d9.h>
#include "ltinteger.h"
//#include "ltbasetypes.h"
//#include "Model.h"
//#include "de_world.h"
//#include "VertexBufferController.h"
#include "SimD_Utilities.h"


//#include "setupmodel.h"
class ModelDraw;
class TLVertex;
#ifndef ModelLightInfo
class ModelLightInfo {		// This if very ing lame, but I can't include the header for this because the intel compiler can't handle it. So I'm left to do this...
public:
	LTVector	m_Pos;
	LTVector	m_WorldPos;
	LTVector	m_Dir;
	float		m_RadiusSquared;
	float		m_InvRadiusSquared;
	LTVector	m_Color;
	float		m_fD3DLightAtten1;
	float		m_fD3DLightAtten2; };
#endif

#ifndef TCalcFn				// See comment above entitlesd "This is very ing lame"
	typedef void (__fastcall *TCalcFn)(ModelDraw* pDraw, ModelVert *pSrc, TLVertex *pDest);
#endif

void CustomTransform_D3D_SimD(VERTEX_0B_TEX2* pOut,ModelVert* pVert,LTMatrix* pTransforms,LTMatrix* pToWorldRot,uint32 counter)
{
	GPMatrix* gpToWorldRot = (GPMatrix*)pToWorldRot;
	while (counter) {
		--counter;
		
		GPVector gpOutX(0,0,0,0);
		NewVertexWeight* pWeight = pVert->m_Weights;
		uint32 i = pVert->m_nWeights;
		while (i) {
			GPMatrix* p_gpMat = (GPMatrix*)&pTransforms[pWeight->m_iNode]; float* pVec = pWeight->m_Vec;
			gpOutX += F32vec4(pVec[0]) * p_gpMat->_L1;
			gpOutX += F32vec4(pVec[1]) * p_gpMat->_L2;
			gpOutX += F32vec4(pVec[2]) * p_gpMat->_L3;
			gpOutX += F32vec4(pVec[3]) * p_gpMat->_L4;
			++pWeight; --i; }

		float fW = 1.0f / gpOutX.w;					// Divide by W
		pOut->x  = gpOutX.x * fW; pOut->y = gpOutX.y * fW; pOut->z = gpOutX.z * fW;

		gpOutX  = gpToWorldRot->_L4;				// Need to rotate the normal (into world space)...
		gpOutX += F32vec4(pVert->m_Normal.x) * gpToWorldRot->_L1;
		gpOutX += F32vec4(pVert->m_Normal.y) * gpToWorldRot->_L2;
		gpOutX += F32vec4(pVert->m_Normal.z) * gpToWorldRot->_L3;
		pOut->nx = gpOutX.x; pOut->ny = gpOutX.y; pOut->nz = gpOutX.z;

		++pOut; ++pVert; }
}

void CustomTransform_SimD(VERTEX_TRANSFORMED_TEX1_SPEC* pOut,ModelVert* pVert,LTMatrix* pTransforms,uint32 counter,LTVector& DirLightDir,float& DirLightAmount,LTVector& AmbientLightColor,LTVector& DirLightColor,uint32& nLightInfos,ModelLightInfo* LightInfos,LTVector& InstanceColor,LTVector& LightAdd,uint8& ModelAlpha,TCalcFn tCalc,ModelDraw* pModelDraw)
{
	GPVector3 gpColor;
	GPVector3 gpAmbientLightColor(AmbientLightColor.x,AmbientLightColor.y,AmbientLightColor.z);
	GPVector3 gpDirLightColor(DirLightColor.x,DirLightColor.y,DirLightColor.z);
	GPVector3 gpInstanceColor(InstanceColor.x,InstanceColor.y,InstanceColor.z);
	GPVector3 gpLightAdd(LightAdd.x,LightAdd.y,LightAdd.z);
	uint32	  iModelAlpha = ModelAlpha; uint32 iOutColorR,iOutColorG,iOutColorB;

	while (counter) {
		counter--;

		// Apply all the weights.
		GPVector gpOutX(0,0,0,0);
		NewVertexWeight* pWeight = pVert->m_Weights;
		uint32 i = pVert->m_nWeights;
		while (i) {
			GPMatrix* p_gpMat = (GPMatrix*)&pTransforms[pWeight->m_iNode]; 	float* pVec = pWeight->m_Vec;
			gpOutX += F32vec4(pVec[0]) * p_gpMat->_L1;
			gpOutX += F32vec4(pVec[1]) * p_gpMat->_L2;
			gpOutX += F32vec4(pVec[2]) * p_gpMat->_L3;
			gpOutX += F32vec4(pVec[3]) * p_gpMat->_L4;

			++pWeight; --i; }

		gpColor = gpAmbientLightColor;				// Apply global lighting.
		float dot = pVert->m_Normal.Dot(DirLightDir) * DirLightAmount;
		if (dot > 0.0f) { gpColor += (gpDirLightColor - gpColor) * dot; }

		for (i=0; i < nLightInfos; i++) {
			ModelLightInfo* pInfo = &LightInfos[i];
			dot = pInfo->m_Dir.Dot(pVert->m_Normal);
			if (dot > 0.0f) {
				float dist = (pVert->m_Vec - pInfo->m_Pos).MagSqr();
				if (dist < pInfo->m_RadiusSquared) {
					GPVector3 gpLightColor(pInfo->m_Color.x,pInfo->m_Color.y,pInfo->m_Color.z);
					gpColor += gpLightColor * ((1.0f - dist*pInfo->m_InvRadiusSquared) * dot); } } }

		// Modulate by the instance color (Note : This also does the saturation multiply..)
		gpColor = _mm_mul_ps(gpColor,gpInstanceColor);

		// Add the lighting add value
		gpColor = gpColor + gpLightAdd;
		gpColor = _mm_min_ps(gpColor,F32vec4(255.0f));

		pOut->rhw = 1.0f / gpOutX.w;					// Divide by W
		pOut->x  = gpOutX.x * pOut->rhw; pOut->y = gpOutX.y * pOut->rhw; pOut->z = gpOutX.z * pOut->rhw;

//		pOut->color = RGBA_MAKE((uint8)RoundFloatToInt(gpColor.x),(uint8)RoundFloatToInt(gpColor.y),(uint8)RoundFloatToInt(gpColor.z),ModelAlpha);
		__asm { 
			fld   gpColor.x
			fistp iOutColorR
			fld   gpColor.y
			fistp iOutColorG
			fld   gpColor.z
			fistp iOutColorB }
		pOut->color = RGBA_MAKE(iOutColorR,iOutColorG,iOutColorB,ModelAlpha);

		tCalc(pModelDraw, pVert, (TLVertex*)pOut);  
		pOut++; pVert++; } 
} 


// Checking for Streaming SIMD Extensions support in the processor.
bool SIMD_fp_HWSupport()
{
	bool HWSupport = false;
	char brand[12];
	unsigned *str = (unsigned *) brand;

	//
	// Make sure that the processor supports CPUID, and that this processor is
	// "Genuine Intel".  This is done in a try/except clause to catch an
	// invalid opcode if the CPUID is not supported.  "AP-485, Intel Processor
	// Identification with CPUID Instruction",(Order # 241618-008), describes a
	// more exact algorithm to determine the type and features of your processor.
	// 
	__try {
		_asm{
			mov		eax, 0			//First, check to make sure this is an Intel processor
			cpuid					//by getting the processor information string with CPUID
			mov	    str, ebx		//  ebx contains "Genu"
			mov     str+4, edx		//  edx contains "ineI"
			mov		str+8, ecx		//  ecx contains "ntel"  -- "GenuineIntel"
		}
	}
    __except(EXCEPTION_EXECUTE_HANDLER) {
		if (_exception_code() == STATUS_ILLEGAL_INSTRUCTION) {
			cout << endl << "****CPUID is not enabled****" << endl;
			return (false);
		}
		return (false); // If we get here, an unexpected exception occurred.
	}

	// Now make sure the processor is "GenuineIntel".
	if (!strncmp(brand, "GenuineIntel", 12)) {
		cout << endl << "****This is not an Intel processor!****" << endl;
		return (false);
	}
	
	// And finally, check the CPUID for Streaming SIMD Extensions support.
	_asm{
		mov		eax, 1			// Put a "1" in eax to tell CPUID to get the feature bits
		cpuid					// Perform CPUID (puts processor feature info into EDX)
		test	edx, 02000000h	// Test bit 25, for Streaming SIMD Extensions existence.
		jz		NotFound		// If not set, jump over the next instruction (No Streaming
		mov		[HWSupport],1	// SIMD Extensions).  Set return value to 1 to indicate,
								// that the processor does support Streaming SIMD Extensions.
	NotFound:				
	}
	
	return (HWSupport);
}	

// Check for OS Support of SimD...
bool SIMD_fp_OSSupport()
{
	__try
	{
		_asm  xorps xmm0, xmm0  //Execute a Streaming SIMD Extensions
								//to see if support exists.
	}
	//
	// Catch any exception.  If an Invalid Opcode exception (ILLEGAL_INSTRUCTION)
	// occurs, and you have already checked the XMM bit in CPUID, Streaming SIMD
	// Extensions technology is not supported by the OS.
	//
    __except(EXCEPTION_EXECUTE_HANDLER) {
		unsigned long code = _exception_code();
		if (code == STATUS_ILLEGAL_INSTRUCTION) {
			cout << endl << "****OS does not support fxsave/fxrstor!****" << endl;
			return (false);
		}
		// If we get here, something else happened on your system.
		cout << endl << "Something else is wrong!!!" << endl;
		return (false);
	}
	return (true);
}
*/
