//------------------------------------------------------------------
//
//   MODULE    : D3DDRAWPRIM.CPP
//
//   PURPOSE   : Implements interface CD3DDrawPrim
//
//   CREATED   : On 8/11/00 At 2:06:07 PM
//
//   COPYRIGHT : (C) 2000 LithTech Inc
//
//------------------------------------------------------------------

// Includes....
#include "bdefs.h"

//IClientShell game client shell object.
#include "iclientshell.h"
static IClientShell *i_client_shell;
define_holder(IClientShell, i_client_shell);

#include "d3ddrawprim.h"
#include "d3d_draw.h"
#include "d3d_viewparams.h"
#include "common_draw.h"

#include "LTEffectImpl.h"
#include "lteffectshadermgr.h"
#include "ltshaderdevicestateimp.h"
#include "rendererconsolevars.h"

#ifndef __ILTTEXINTERFACE_H__
#include "ilttexinterface.h"
#endif

static ILTTexInterface *pTexInterface;
define_holder(ILTTexInterface, pTexInterface);


// EXTERNS...
extern uint32 g_ScreenWidth;
extern uint32 g_ScreenHeight;

// Note : This is an evil hack so we can get access to the most recent near/farz
extern ViewParams g_ViewParams;

//Not a fan of this, but it makes the code below much easier to read.
#define EFFECT_SHADER_MACRO(result, primcall)\
LTEffectImpl* pEffectShader = (LTEffectImpl*)LTEffectShaderMgr::GetSingleton().GetEffectShader(m_nEffectShaderID);\
if(pEffectShader)\
{\
	ID3DXEffect* pD3DEffect = pEffectShader->GetEffect();\
	if(pD3DEffect)\
	{\
		i_client_shell->OnEffectShaderSetParams(pEffectShader, NULL, NULL, LTShaderDeviceStateImp::GetSingleton());\
		UINT nPasses = 0;\
		pD3DEffect->Begin(&nPasses, 0);\
		for(UINT i = 0; i < nPasses; ++i)\
		{\
			pD3DEffect->BeginPass(i);\
			result = primcall;\
			pD3DEffect->EndPass();\
		}\
		pD3DEffect->End();\
	}\
}\
else\
{\
	result = primcall;\
}\

//------------------------------------------------------------------------------
//Utility classes

//CAutoDrawPrimBlock
//
// class to automatically handle beginning and ending a draw primitive block.
// this is intended for use inside of draw primitive functions so that the
// block doesn't have to be ended in every single return branch. It will begin
// the block in the constructor, and end it in the destructor
class CAutoDrawPrimBlock
{
public:
	CAutoDrawPrimBlock(ILTDrawPrim* pDrawPrim) :
		m_pDrawPrim(pDrawPrim)
	{
		//make sure that we have a valid draw primitive
		assert(m_pDrawPrim && "Invalid construction of CAutoDrawPrimBlock, it must have a valid draw primitive");

		//we do, so begin the block
		m_pDrawPrim->BeginDrawPrim();
	}

	~CAutoDrawPrimBlock()
	{
		//just end the block
		m_pDrawPrim->EndDrawPrim();
	}

private:
	//the draw primitive we are going through
	ILTDrawPrim*	m_pDrawPrim;
};

//------------------------------------------------------------------------------
// interface database
define_interface(CD3DDrawPrim, ILTDrawPrim); 
instantiate_interface(CD3DDrawPrim, ILTDrawPrim, Internal);

//------------------------------------------------------------------------------
//CD3DDrawPrim

CD3DDrawPrim::CD3DDrawPrim() :
	m_nBlockCount(0)
{
}

LTRESULT CD3DDrawPrim::BeginDrawPrim()
{
	//don't bother pushing them if we are already in a block
	if(m_nBlockCount++ > 0)
		return LT_OK;

	PushRenderStates(r_GetRenderStruct()->GetD3DDevice());

	//success
	return LT_OK;
}

LTRESULT CD3DDrawPrim::EndDrawPrim()
{
	//make sure we can pop
	if(m_nBlockCount == 0)
	{
		//too many pops!
		assert(!"Too many calls to EndDrawPrim");
		return LT_ERROR;
	}

	//ok, now we need to see if we are bailing out
	if(m_nBlockCount == 1)
	{
		//we indeed are
		PopRenderStates(r_GetRenderStruct()->GetD3DDevice());
	}

	//decrement our count
	m_nBlockCount--;

	//success
	return LT_OK;
}

//helper macro to aid in the setting of states. This sets the specified variable
//if it is different, and if inside of a draw primitive block, it will call the
//specified function
//
// Note that it always sets the value. For some reason, everything gets completely
// screwed up if it doesn't. Looking into it.
#define SETDRAWPRIMSTATE(Value, SetVal, Func)							\
{																		\
	if((Value) != (SetVal))												\
	{																	\
		(Value) = (SetVal);												\
		if(m_nBlockCount > 0)											\
			(Func)(r_GetRenderStruct()->GetD3DDevice());				\
	}																	\
	return LT_OK;														\
}



// Sets the current camera to use (viewport, field of view etc)
LTRESULT CD3DDrawPrim::SetCamera(const HOBJECT hCamera) 
{
	SETDRAWPRIMSTATE(m_pCamera, hCamera, SetCamera);
}

//Specifiy whether or not to be in really close space for rendering
LTRESULT CD3DDrawPrim::SetReallyClose(bool bReallyClose)
{
	SETDRAWPRIMSTATE(m_bReallyClose, bReallyClose, SetReallyClose);
}


// Sets current texture
LTRESULT CD3DDrawPrim::SetTexture(const HTEXTURE hTexture) 
{
	SETDRAWPRIMSTATE(m_pTexture, hTexture, SetTexture);
}

// Sets transform type
LTRESULT CD3DDrawPrim::SetTransformType(const ELTTransformType eType) 
{
	SETDRAWPRIMSTATE(m_eTransType, eType, SetTransformMode);
}

// Sets color operation
LTRESULT CD3DDrawPrim::SetColorOp(const ELTColorOp eColorOp) 
{
	SETDRAWPRIMSTATE(m_ColorOp, eColorOp, SetColorOp);
}

// Sets source/dest alpha blending operation
LTRESULT CD3DDrawPrim::SetAlphaBlendMode(const ELTBlendMode eBlendMode) 
{
	SETDRAWPRIMSTATE(m_BlendMode, eBlendMode, SetBlendMode);
}

// Enables/disables z buffering
LTRESULT CD3DDrawPrim::SetZBufferMode(const ELTZBufferMode eZBufferMode) 
{
	SETDRAWPRIMSTATE(m_eZBufferMode, eZBufferMode, SetZBufferMode);
}

// Set AlphaTest Mode (on/off)
LTRESULT CD3DDrawPrim::SetAlphaTestMode(const ELTTestMode eTestMode) 
{
	SETDRAWPRIMSTATE(m_eTestMode, eTestMode, SetTestMode);
}

// set the type of clipping to be done
LTRESULT CD3DDrawPrim::SetClipMode(const ELTClipMode eClipMode) 
{
	SETDRAWPRIMSTATE(m_eClipType, eClipMode, SetClipMode);
}

// set the fill mode
LTRESULT CD3DDrawPrim::SetFillMode(ELTDPFillMode eFillMode)
{
	SETDRAWPRIMSTATE(m_eFillMode, eFillMode, SetFillMode);
}

// set the cull mode
LTRESULT CD3DDrawPrim::SetCullMode(ELTDPCullMode eCullMode)
{
	SETDRAWPRIMSTATE(m_eCullMode, eCullMode, SetCullMode);
}

// set the fog enable status
LTRESULT CD3DDrawPrim::SetFogEnable(bool bFogEnable)
{
	SETDRAWPRIMSTATE(m_bFogEnable, bFogEnable, SetFogEnable);
}

//saves the current D3D state into the member variables
void CD3DDrawPrim::SaveStates(LPDIRECT3DDEVICE9 pDevice)
{
	// Save the render states...
	pDevice->GetTextureStageState(0,D3DTSS_COLOROP,(unsigned long*)&m_PrevColorOp);
	pDevice->GetRenderState(D3DRS_ALPHABLENDENABLE,(unsigned long*)&m_PrevAlphaBlendEnable);
	pDevice->GetRenderState(D3DRS_SRCBLEND,(unsigned long*)&m_PrevSrcBlend);
	pDevice->GetRenderState(D3DRS_DESTBLEND,(unsigned long*)&m_PrevDstBlend);
	pDevice->GetRenderState(D3DRS_ZENABLE,(unsigned long*)&m_PrevZEnable);
	pDevice->GetRenderState(D3DRS_ZWRITEENABLE,(unsigned long*)&m_PrevZWriteEnable);
	pDevice->GetRenderState(D3DRS_ALPHATESTENABLE,(unsigned long*)&m_PrevAlphaTestEnable);
	pDevice->GetRenderState(D3DRS_ALPHAFUNC,(unsigned long*)&m_PrevAlphaTestFunc);
	pDevice->GetRenderState(D3DRS_FILLMODE,(unsigned long*)&m_PrevFillMode);
	pDevice->GetRenderState(D3DRS_CLIPPING,(unsigned long*)&m_PrevClipMode);
	pDevice->GetRenderState(D3DRS_CULLMODE,(unsigned long*)&m_PrevCullMode);
	pDevice->GetRenderState(D3DRS_FOGENABLE,(unsigned long*)&m_PrevFogMode);

	pDevice->GetTransform(D3DTS_VIEW,&m_PrevTransView);
	pDevice->GetTransform(D3DTS_PROJECTION,&m_PrevTransProj);

	pDevice->GetViewport(&m_PrevViewport);
	m_bResetViewport = false;
}


//sets up the appropriate state for the each section
void CD3DDrawPrim::SetClipMode(LPDIRECT3DDEVICE9 pDevice)
{
	switch (m_eClipType) 
	{
	case DRAWPRIM_NOCLIP	: pDevice->SetRenderState(D3DRS_CLIPPING,false); break;
	case DRAWPRIM_FASTCLIP	: pDevice->SetRenderState(D3DRS_CLIPPING,true);  break;
	case DRAWPRIM_FULLCLIP  : pDevice->SetRenderState(D3DRS_CLIPPING,true);  break; 

	default:
		assert(false);
		break;
	} 

}
void CD3DDrawPrim::SetTexture(LPDIRECT3DDEVICE9 pDevice)
{
	if (m_pTexture) 
	{
		r_GetRenderStruct()->DrawPrimSetTexture(m_pTexture);
		
		// If we've got an effect
		LTEffectImpl* pEffectShader = (LTEffectImpl*)LTEffectShaderMgr::GetSingleton().GetEffectShader(m_nEffectShaderID);
		if(pEffectShader)
		{
			ID3DXEffect* pD3DEffect = pEffectShader->GetEffect();
			if(pD3DEffect)
			{
				if(m_pTexture)
				{				
					RTexture* pRTexture = (RTexture*)m_pTexture->m_pRenderData;
					pD3DEffect->SetTexture("texture0", pRTexture->m_pD3DTexture);
				}
			}
		}
	}
	else
	{
		r_GetRenderStruct()->DrawPrimDisableTextures();
	}
}

void CD3DDrawPrim::SetReallyClose(LPDIRECT3DDEVICE9 pDevice)
{
	SetCamera(pDevice);
	SetTransformMode(pDevice);
}

LTRESULT CD3DDrawPrim::SetEffectShaderID(uint32 nEffectShaderID)
{
	m_nEffectShaderID = nEffectShaderID;
	return LT_OK;
}

void CD3DDrawPrim::SetCamera(LPDIRECT3DDEVICE9 pDevice)
{
	// Check the current viewport, may need to get changed...
	D3DVIEWPORT9 PrevViewport;
	pDevice->GetViewport(&PrevViewport);

	float MinZ = (m_bReallyClose) ? 0.0f : m_PrevViewport.MinZ;
	float MaxZ = (m_bReallyClose) ? 0.1f : m_PrevViewport.MaxZ;

	if (m_pCamera) 
	{
		LTObject* pLTCameraObj = (LTObject*)m_pCamera;				// Set the Camera (if there is one)...
		CameraInstance* pCamera = pLTCameraObj->ToCamera();

		if ((pCamera->m_Left != (int)PrevViewport.X) || 
			(pCamera->m_Top != (int)PrevViewport.Y) || 
			((pCamera->m_Right - pCamera->m_Left) != (int)PrevViewport.Width) || 
			((pCamera->m_Bottom - pCamera->m_Top) != (int)PrevViewport.Height)) 
		{
			D3DVIEWPORT9 vp;
			vp.MinZ = MinZ; 
			vp.MaxZ = MaxZ;

			vp.X = PrevViewport.X;
			vp.Y = PrevViewport.Y;

			vp.Width = PrevViewport.Width;
			vp.Height = PrevViewport.Height;

			PD3DDEVICE->SetViewport(&vp);

			m_bResetViewport = true; 
		} 
	}
	else 
	{
		if (0 != PrevViewport.X || 0 != PrevViewport.Y || g_ScreenWidth != PrevViewport.Width || g_ScreenHeight != PrevViewport.Height) 
		{
			D3DVIEWPORT9 vp;
			vp.MinZ = MinZ; 
			vp.MaxZ = MaxZ;

			vp.X = 0; 
			vp.Y = 0;
			
			vp.Width  = g_ScreenWidth;
			vp.Height = g_ScreenHeight;

			PD3DDEVICE->SetViewport(&vp);

			m_bResetViewport = true; 
		} 
	}
}

void CD3DDrawPrim::SetTransformMode(LPDIRECT3DDEVICE9 pDevice)
{
	D3DXMATRIX mIdentity; 
	D3DXMatrixIdentity(&mIdentity);

	CameraInstance* pCamera = NULL;

	if (m_pCamera) 
	{
		pCamera			= ((LTObject*)m_pCamera)->ToCamera();
	}

	//see if we are in really close, if so we need to assume camera space transform
	//with a different projection matrix
	if(m_bReallyClose)
	{
		// [RP] 4/24/02 - The viewport minZ & maxZ needs to be changed for accurately
		// setting the position for a reallyclose primitive.

		D3DVIEWPORT9 curVP;
		PD3DDEVICE->GetViewport(&curVP);

		D3DVIEWPORT9 ViewportData;
		ViewportData.X		= curVP.X;	
		ViewportData.Y		= curVP.Y;
		ViewportData.Width	= curVP.Width;
		ViewportData.Height = curVP.Height;
		ViewportData.MinZ	= 0;
		ViewportData.MaxZ	= 0.1f;

		PD3DDEVICE->SetViewport(&ViewportData);
		m_bResetViewport = true;

		//setup our new projection based on player view parameters.
		D3DXMATRIX NewProj;

		float aspect = g_ScreenWidth / float(g_ScreenHeight);

		D3DXMatrixPerspectiveFovLH(&NewProj,g_CV_PVModelFOV.m_Val * 0.01745329251994f, 
											aspect, 
											g_CV_ModelNear.m_Val, 
											g_CV_ModelFar.m_Val);

		//setup the new matrices
		PD3DDEVICE->SetTransform(D3DTS_PROJECTION, &NewProj);
		PD3DDEVICE->SetTransform(D3DTS_VIEW,&mIdentity);
	}
	else
	{
		//not really close, so pick the transform that is most applicable
		switch (m_eTransType) 
		{
		case DRAWPRIM_TRANSFORM_WORLD  : 
			if (m_pCamera) 
			{
				ViewParams cDrawPrimParams;
				d3d_InitFrustum(&cDrawPrimParams, pCamera->m_xFov, pCamera->m_yFov,
					// Note : This sucks, but we don't have any other way of getting the near/farZ at this point
					g_ViewParams.m_NearZ, g_ViewParams.m_FarZ, 
					pCamera->m_Left, pCamera->m_Top, pCamera->m_Right, pCamera->m_Bottom,
					&pCamera->m_Pos, &pCamera->m_Rotation, 
					ViewParams::eRenderMode_Normal);

				d3d_SetD3DTransformStates(cDrawPrimParams);
			} 
			break;

		case DRAWPRIM_TRANSFORM_CAMERA : 
			if (m_pCamera) 
			{
				ViewParams cDrawPrimParams;
				d3d_InitFrustum(&cDrawPrimParams, pCamera->m_xFov, pCamera->m_yFov,
					// Note : This sucks, but we don't have any other way of getting the near/farZ at this point
					g_ViewParams.m_NearZ, g_ViewParams.m_FarZ, 
					pCamera->m_Left, pCamera->m_Top, pCamera->m_Right, pCamera->m_Bottom,
					&pCamera->m_Pos, &pCamera->m_Rotation, 
					ViewParams::eRenderMode_Normal);

				d3d_SetD3DTransformStates(cDrawPrimParams);
			} 
			g_RenderStateMgr.SetTransform(D3DTS_VIEW,&mIdentity); 
			break;
			
		case DRAWPRIM_TRANSFORM_SCREEN : 
			g_RenderStateMgr.SetTransform(D3DTS_PROJECTION,&mIdentity); 
			break; 

		default:
			assert(false);
			break;
		}
	}
}

void CD3DDrawPrim::SetColorOp(LPDIRECT3DDEVICE9 pDevice)
{
	switch (m_ColorOp) 
	{
	case DRAWPRIM_NOCOLOROP : 
		pDevice->SetTextureStageState(0,D3DTSS_COLOROP,D3DTOP_SELECTARG2); 
		pDevice->SetTextureStageState(0,D3DTSS_ALPHAOP,D3DTOP_SELECTARG2); 
		break;
	case DRAWPRIM_MODULATE	: 
		pDevice->SetTextureStageState(0,D3DTSS_COLOROP,D3DTOP_MODULATE); 
		pDevice->SetTextureStageState(0,D3DTSS_ALPHAOP,D3DTOP_MODULATE); 
		break;
	case DRAWPRIM_ADD		: 
		pDevice->SetTextureStageState(0,D3DTSS_COLOROP,D3DTOP_ADD); 
		pDevice->SetTextureStageState(0,D3DTSS_ALPHAOP,D3DTOP_ADD); 
		break;
	case DRAWPRIM_DECAL		: 
		pDevice->SetTextureStageState(0,D3DTSS_COLOROP,D3DTOP_SELECTARG1); 
		pDevice->SetTextureStageState(0,D3DTSS_ALPHAOP,D3DTOP_SELECTARG1); 
		break;
	default:
		assert(false);
		break;
	}
}

void CD3DDrawPrim::SetBlendMode(LPDIRECT3DDEVICE9 pDevice)
{
	switch (m_BlendMode) 
	{
	case DRAWPRIM_NOBLEND	: 
		pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE,false); 
		break;
	case DRAWPRIM_BLEND_ADD	: 
		pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE,true); 
		pDevice->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_ONE); 
		pDevice->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_ONE); 
		break;
	case DRAWPRIM_BLEND_SATURATE :
		pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE,true); 
		pDevice->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_INVDESTCOLOR); 
		pDevice->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_ONE); 
		break;
	case DRAWPRIM_BLEND_MOD_SRCALPHA	: 
		pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE,true); 
		pDevice->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_SRCALPHA); 
		pDevice->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA); 
		break;
	case DRAWPRIM_BLEND_MOD_SRCCOLOR	: 
		pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE,true); 
		pDevice->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_SRCCOLOR); 
		pDevice->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_INVSRCCOLOR); 
		break;
	case DRAWPRIM_BLEND_MOD_DSTCOLOR	: 
		pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE,true); 
		pDevice->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_DESTCOLOR); 
		pDevice->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_INVDESTCOLOR); 
		break;
	case DRAWPRIM_BLEND_MUL_SRCALPHA_ONE :
		pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE,true); 
		pDevice->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_SRCALPHA); 
		pDevice->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_ONE); 
		break;
	case DRAWPRIM_BLEND_MUL_SRCALPHA :
		pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE,true); 
		pDevice->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_SRCALPHA); 
		pDevice->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_ZERO); 
		break;
	case DRAWPRIM_BLEND_MUL_SRCCOL_DSTCOL	: 
		pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE,true); 
		pDevice->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_SRCCOLOR); 
		pDevice->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_DESTCOLOR); 
		break;
	case DRAWPRIM_BLEND_MUL_SRCCOL_ONE	: 
		pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE,true); 
		pDevice->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_SRCCOLOR); 
		pDevice->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_ONE); 
		break;
	case DRAWPRIM_BLEND_MUL_DSTCOL_ZERO	: 
		pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE,true); 
		pDevice->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_DESTCOLOR); 
		pDevice->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_ZERO); 
		break; 
	default:
		assert(false);
		break;
	}
}

void CD3DDrawPrim::SetZBufferMode(LPDIRECT3DDEVICE9 pDevice)
{
	switch (m_eZBufferMode) 
	{
	case DRAWPRIM_ZRW :
		pDevice->SetRenderState(D3DRS_ZENABLE,true);
		pDevice->SetRenderState(D3DRS_ZWRITEENABLE,true); 
		break;
	case DRAWPRIM_ZRO :
		pDevice->SetRenderState(D3DRS_ZENABLE,true);
		pDevice->SetRenderState(D3DRS_ZWRITEENABLE,false); 
		break;
	case DRAWPRIM_NOZ :
		pDevice->SetRenderState(D3DRS_ZENABLE,false);
		pDevice->SetRenderState(D3DRS_ZWRITEENABLE,false); 
		break; 
	default:
		assert(false);
		break;
	}

}

void CD3DDrawPrim::SetTestMode(LPDIRECT3DDEVICE9 pDevice)
{
	switch (m_eTestMode) 
	{
	case DRAWPRIM_NOALPHATEST :
		pDevice->SetRenderState(D3DRS_ALPHATESTENABLE,false); 
		break;
	case DRAWPRIM_ALPHATEST_LESS :
		pDevice->SetRenderState(D3DRS_ALPHATESTENABLE,true);
		pDevice->SetRenderState(D3DRS_ALPHAFUNC,D3DCMP_LESS); 
		break;
	case DRAWPRIM_ALPHATEST_LESSEQUAL :
		pDevice->SetRenderState(D3DRS_ALPHATESTENABLE,true);
		pDevice->SetRenderState(D3DRS_ALPHAFUNC,D3DCMP_LESSEQUAL); 
		break;
	case DRAWPRIM_ALPHATEST_GREATER :
		pDevice->SetRenderState(D3DRS_ALPHATESTENABLE,true);
		pDevice->SetRenderState(D3DRS_ALPHAFUNC,D3DCMP_GREATER); 
		break;
	case DRAWPRIM_ALPHATEST_GREATEREQUAL :
		pDevice->SetRenderState(D3DRS_ALPHATESTENABLE,true);
		pDevice->SetRenderState(D3DRS_ALPHAFUNC,D3DCMP_GREATEREQUAL); 
		break;
	case DRAWPRIM_ALPHATEST_EQUAL :
		pDevice->SetRenderState(D3DRS_ALPHATESTENABLE,true);
		pDevice->SetRenderState(D3DRS_ALPHAFUNC,D3DCMP_EQUAL); 
		break;
	case DRAWPRIM_ALPHATEST_NOTEQUAL :
		pDevice->SetRenderState(D3DRS_ALPHATESTENABLE,true);
		pDevice->SetRenderState(D3DRS_ALPHAFUNC,D3DCMP_NOTEQUAL); 
		break; 
	default:
		assert(false);
		break;
	}

}

void CD3DDrawPrim::SetFillMode(LPDIRECT3DDEVICE9 pDevice)
{
	switch (m_eFillMode) 
	{
	case DRAWPRIM_WIRE : pDevice->SetRenderState(D3DRS_FILLMODE,D3DFILL_WIREFRAME); break;
	case DRAWPRIM_FILL : pDevice->SetRenderState(D3DRS_FILLMODE,D3DFILL_SOLID); break; 

	default: 
		assert(false);
		break;
	} 

}

void CD3DDrawPrim::SetCullMode(LPDIRECT3DDEVICE9 pDevice)
{
	switch (m_eCullMode) 
	{
	case DRAWPRIM_CULL_NONE : pDevice->SetRenderState(D3DRS_CULLMODE,D3DCULL_NONE); break;
	case DRAWPRIM_CULL_CCW	: pDevice->SetRenderState(D3DRS_CULLMODE,D3DCULL_CCW); break;
	case DRAWPRIM_CULL_CW	: pDevice->SetRenderState(D3DRS_CULLMODE,D3DCULL_CW); break; 

	default:
		assert(false);
		break;
	}
}

void CD3DDrawPrim::SetFogEnable(LPDIRECT3DDEVICE9 pDevice)
{
	pDevice->SetRenderState(D3DRS_FOGENABLE, m_bFogEnable);
}

// Set the render states (and save the old ones)...
void CD3DDrawPrim::PushRenderStates(LPDIRECT3DDEVICE9 pDevice)
{

	//saves the current D3D state into the member variables
	SaveStates(pDevice);

	// save the reallyclose setting so it will properly reset
	m_bPrevReallyClose = m_bReallyClose;

	//clear out any old textures
	r_GetRenderStruct()->DrawPrimDisableTextures();

	//sets up the appropriate state for the each section
	SetClipMode(pDevice);
	SetTransformMode(pDevice);
	SetColorOp(pDevice);
	SetBlendMode(pDevice);
	SetZBufferMode(pDevice);
	SetTestMode(pDevice);
	SetFillMode(pDevice);
	SetCullMode(pDevice);
	SetFogEnable(pDevice);
	SetTexture(pDevice);
	SetCamera(pDevice);

	//not necessary, it just calls set camera and set transform mode... 
	//SetReallyClose(pDevice);

	//setup states that are independant of the various possible settings
	pDevice->SetRenderState(D3DRS_LIGHTING,false);

	pDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	pDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_CURRENT);
	pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_CURRENT);

	D3DXMATRIX mIdentity;
	D3DXMatrixIdentity(&mIdentity);

	g_RenderStateMgr.SetTransform(D3DTS_WORLDMATRIX(0), &mIdentity); 
}

// Reset the old renderstates...
void CD3DDrawPrim::PopRenderStates(LPDIRECT3DDEVICE9 pDevice) 
{
	pDevice->SetTextureStageState(0,D3DTSS_COLOROP,m_PrevColorOp);
	pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE,m_PrevAlphaBlendEnable);
	pDevice->SetRenderState(D3DRS_SRCBLEND,m_PrevSrcBlend);
	pDevice->SetRenderState(D3DRS_DESTBLEND,m_PrevDstBlend);
	pDevice->SetRenderState(D3DRS_ZENABLE,m_PrevZEnable);
	pDevice->SetRenderState(D3DRS_ZWRITEENABLE,m_PrevZWriteEnable);
	pDevice->SetRenderState(D3DRS_ALPHATESTENABLE,m_PrevAlphaTestEnable);
	pDevice->SetRenderState(D3DRS_ALPHAFUNC,m_PrevAlphaTestFunc); 
	pDevice->SetRenderState(D3DRS_FILLMODE,m_PrevFillMode); 
	pDevice->SetRenderState(D3DRS_CLIPPING,m_PrevClipMode);
	pDevice->SetRenderState(D3DRS_CULLMODE,m_PrevCullMode);

	pDevice->SetRenderState(D3DRS_FOGENABLE,m_PrevFogMode);
	pDevice->SetTransform(D3DTS_VIEW,&m_PrevTransView);
	pDevice->SetTransform(D3DTS_PROJECTION,&m_PrevTransProj); 

	pDevice->SetTextureStageState(0,D3DTSS_COLOROP,D3DTOP_MODULATE); 
	pDevice->SetTextureStageState(0,D3DTSS_ALPHAOP,D3DTOP_MODULATE);
	pDevice->SetTextureStageState(0,D3DTSS_COLORARG1,D3DTA_TEXTURE);
	pDevice->SetTextureStageState(0,D3DTSS_COLORARG2,D3DTA_DIFFUSE);
	pDevice->SetTextureStageState(0,D3DTSS_ALPHAARG1,D3DTA_TEXTURE);
	pDevice->SetTextureStageState(0,D3DTSS_ALPHAARG2,D3DTA_DIFFUSE); 

	if (m_bResetViewport) 
	{ 
		PD3DDEVICE->SetViewport(&m_PrevViewport); 
	}

	SetTexture((HTEXTURE)0);
	
	// reset the reallyclose setting
	m_bReallyClose = m_bPrevReallyClose;
}


void CD3DDrawPrim::SetUVWH (LT_POLYGT4 *pPrim, HTEXTURE pTex,
							 float u, float v,
							 float w, float h)
{
	if (!pPrim) return;

	uint32 ttw,tth;
	float tw, th;
	
	if (pTex) {
		pTexInterface->GetTextureDims(pTex, ttw, tth);
		tw = (float)ttw;
		th = (float)tth;

		float factor = 1.0f;

		float pixelcenterh = 0.05/tw;
		float pixelcenterv = 0.05/th;

		pPrim->verts[0].u = u / tw					+ pixelcenterh;
		pPrim->verts[0].v = v / th					+ pixelcenterv;
		pPrim->verts[1].u = (u + w + factor) / tw	+ pixelcenterh;
		pPrim->verts[1].v = v / th					+ pixelcenterv;
		pPrim->verts[2].u = (u + w + factor) / tw	+ pixelcenterh;
		pPrim->verts[2].v = (v + h + factor) / th	+ pixelcenterv;
		pPrim->verts[3].u = u / tw					+ pixelcenterh;
		pPrim->verts[3].v = (v + h + factor) / th	+ pixelcenterv;

	}
	else {
		pPrim->verts[0].u = pPrim->verts[1].u = pPrim->verts[2].u = pPrim->verts[3].u = 0.0f;
		pPrim->verts[0].v = pPrim->verts[1].v = pPrim->verts[2].v = pPrim->verts[3].v = 0.0f;
	}			
}


void CD3DDrawPrim::SaveViewport( void )
{
	PD3DDEVICE->GetViewport( &m_SavedViewport );
}


void CD3DDrawPrim::RestoreViewport( void )
{
	PD3DDEVICE->SetViewport( &m_SavedViewport );
}


// Draw primitive calls (triangles)
LTRESULT CD3DDrawPrim::DrawPrim(LT_POLYGT3 *pPrim, uint32 nCount) {
	LPDIRECT3DDEVICE9 pDevice = r_GetRenderStruct()->GetD3DDevice();
	if (!pDevice) return LT_ERROR;
	CAutoDrawPrimBlock AutoDPBlock(this);

	if (m_eTransType != DRAWPRIM_TRANSFORM_SCREEN) {

		HRESULT hRes = pDevice->SetVertexShader(NULL);
		if (hRes != D3D_OK) return LT_ERROR;

		hRes = pDevice->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1);
		if (hRes != D3D_OK) return LT_ERROR;

		EFFECT_SHADER_MACRO(hRes, pDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST, nCount, pPrim, sizeof(LT_VERTGT)));
		if (hRes != D3D_OK) 
			return LT_ERROR; 
	}
	else {	// Use Transform and Lit verts...
		HRESULT hRes = pDevice->SetVertexShader(NULL);
		if (hRes != D3D_OK) return LT_ERROR;

		hRes = pDevice->SetFVF(DRAWPRIM_D3DTRANS_TEX_FLAGS);
		if (hRes != D3D_OK) return LT_ERROR;

		LT_POLYGT3* pSrcPtr		= pPrim;
		while (nCount) {
			DRAWPRIM_D3DTRANS_TEX* pDstPtr = m_VertTransBufT;
			uint32 nDrawCount	= 0;
			int32 nDstBufSize	= CD3DDRAWPRIM_BUFSIZE - 3;
			while (nCount && nDstBufSize > 0) {		// Copy the tri to the buffer,..
				pDstPtr->x	    = pSrcPtr->verts[0].x; pDstPtr->y = pSrcPtr->verts[0].y; pDstPtr->z = pSrcPtr->verts[0].z;
				pDstPtr->rhw	= 1.0f;
				pDstPtr->rgba   = RGBA_MAKE(pSrcPtr->verts[0].rgba.r,pSrcPtr->verts[0].rgba.g,pSrcPtr->verts[0].rgba.b,pSrcPtr->verts[0].rgba.a);
				pDstPtr->u	    = pSrcPtr->verts[0].u; pDstPtr->v = pSrcPtr->verts[0].v; 
				++pDstPtr;

				pDstPtr->x	    = pSrcPtr->verts[1].x; pDstPtr->y = pSrcPtr->verts[1].y; pDstPtr->z = pSrcPtr->verts[1].z;
				pDstPtr->rhw	= 1.0f;
				pDstPtr->rgba   = RGBA_MAKE(pSrcPtr->verts[1].rgba.r,pSrcPtr->verts[1].rgba.g,pSrcPtr->verts[1].rgba.b,pSrcPtr->verts[1].rgba.a);
				pDstPtr->u	    = pSrcPtr->verts[1].u; pDstPtr->v = pSrcPtr->verts[1].v; 
				++pDstPtr;

				pDstPtr->x	    = pSrcPtr->verts[2].x; pDstPtr->y = pSrcPtr->verts[2].y; pDstPtr->z = pSrcPtr->verts[2].z;
				pDstPtr->rhw	= 1.0f;
				pDstPtr->rgba   = RGBA_MAKE(pSrcPtr->verts[2].rgba.r,pSrcPtr->verts[2].rgba.g,pSrcPtr->verts[2].rgba.b,pSrcPtr->verts[2].rgba.a);
				pDstPtr->u	    = pSrcPtr->verts[2].u; pDstPtr->v = pSrcPtr->verts[2].v; 
				++pDstPtr; 

				--nCount; nDstBufSize -= 3; ++nDrawCount; ++pSrcPtr; }

			EFFECT_SHADER_MACRO(hRes, pDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST, nDrawCount, m_VertTransBufT, sizeof(DRAWPRIM_D3DTRANS_TEX))); 
			if (hRes != D3D_OK) 
				return LT_ERROR; 
	} }

	return LT_OK;
}

LTRESULT CD3DDrawPrim::DrawPrim(LT_POLYFT3 *pPrim, uint32 nCount) {
	LPDIRECT3DDEVICE9 pDevice = r_GetRenderStruct()->GetD3DDevice();
	if (!pDevice) return LT_ERROR;
	CAutoDrawPrimBlock AutoDPBlock(this);

	if (m_eTransType != DRAWPRIM_TRANSFORM_SCREEN) {
		HRESULT hRes = pDevice->SetVertexShader(NULL);
		if (hRes != D3D_OK) return LT_ERROR;

		hRes = pDevice->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1);
		if (hRes != D3D_OK) return LT_ERROR;

		LT_POLYFT3* pSrcPtr		= pPrim;
		while (nCount) {
			LT_VERTGT*  pDstPtr = m_VertBufGT;
			uint32 nDrawCount	= 0;
			int32 nDstBufSize	= CD3DDRAWPRIM_BUFSIZE - 3;
			while (nCount && nDstBufSize > 0) {		// Copy the tri to the buffer,..
				pDstPtr->x	    = pSrcPtr->verts[0].x; pDstPtr->y = pSrcPtr->verts[0].y; pDstPtr->z = pSrcPtr->verts[0].z;
				pDstPtr->rgba   = pSrcPtr->rgba;
				pDstPtr->u	    = pSrcPtr->verts[0].u; pDstPtr->v = pSrcPtr->verts[0].v; 
				++pDstPtr;

				pDstPtr->x	    = pSrcPtr->verts[1].x; pDstPtr->y = pSrcPtr->verts[1].y; pDstPtr->z = pSrcPtr->verts[1].z;
				pDstPtr->rgba   = pSrcPtr->rgba;
				pDstPtr->u	    = pSrcPtr->verts[1].u; pDstPtr->v = pSrcPtr->verts[1].v; 
				++pDstPtr;

				pDstPtr->x	    = pSrcPtr->verts[2].x; pDstPtr->y = pSrcPtr->verts[2].y; pDstPtr->z = pSrcPtr->verts[2].z;
				pDstPtr->rgba   = pSrcPtr->rgba;
				pDstPtr->u	    = pSrcPtr->verts[2].u; pDstPtr->v = pSrcPtr->verts[2].v; 
				++pDstPtr; 

				--nCount; nDstBufSize -= 3; ++nDrawCount; ++pSrcPtr; }

			EFFECT_SHADER_MACRO(hRes, pDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST, nDrawCount, m_VertBufGT, sizeof(LT_VERTGT)));
			if (hRes != D3D_OK) 
				return LT_ERROR; 
	} }
	else {	// Use Transform and Lit verts...
		HRESULT hRes = pDevice->SetVertexShader(NULL);
		if (hRes != D3D_OK) return LT_ERROR;

		hRes = pDevice->SetFVF(DRAWPRIM_D3DTRANS_TEX_FLAGS);
		if (hRes != D3D_OK) return LT_ERROR;

		LT_POLYFT3* pSrcPtr		= pPrim;
		while (nCount) {
			DRAWPRIM_D3DTRANS_TEX* pDstPtr = m_VertTransBufT;
			uint32 nDrawCount	= 0;
			int32 nDstBufSize	= CD3DDRAWPRIM_BUFSIZE - 3;
			while (nCount && nDstBufSize > 0) {		// Copy the tri to the buffer,..
				pDstPtr->x	    = pSrcPtr->verts[0].x; pDstPtr->y = pSrcPtr->verts[0].y; pDstPtr->z = pSrcPtr->verts[0].z;
				pDstPtr->rhw	= 1.0f;
				pDstPtr->rgba   = RGBA_MAKE(pSrcPtr->rgba.r,pSrcPtr->rgba.g,pSrcPtr->rgba.b,pSrcPtr->rgba.a);
				pDstPtr->u	    = pSrcPtr->verts[0].u; pDstPtr->v = pSrcPtr->verts[0].v; 
				++pDstPtr;

				pDstPtr->x	    = pSrcPtr->verts[1].x; pDstPtr->y = pSrcPtr->verts[1].y; pDstPtr->z = pSrcPtr->verts[1].z;
				pDstPtr->rhw	= 1.0f;
				pDstPtr->rgba   = RGBA_MAKE(pSrcPtr->rgba.r,pSrcPtr->rgba.g,pSrcPtr->rgba.b,pSrcPtr->rgba.a);
				pDstPtr->u	    = pSrcPtr->verts[1].u; pDstPtr->v = pSrcPtr->verts[1].v; 
				++pDstPtr;

				pDstPtr->x	    = pSrcPtr->verts[2].x; pDstPtr->y = pSrcPtr->verts[2].y; pDstPtr->z = pSrcPtr->verts[2].z;
				pDstPtr->rhw	= 1.0f;
				pDstPtr->rgba   = RGBA_MAKE(pSrcPtr->rgba.r,pSrcPtr->rgba.g,pSrcPtr->rgba.b,pSrcPtr->rgba.a);
				pDstPtr->u	    = pSrcPtr->verts[2].u; pDstPtr->v = pSrcPtr->verts[2].v; 
				++pDstPtr; 

				--nCount; nDstBufSize -= 3; ++nDrawCount; ++pSrcPtr; }

			EFFECT_SHADER_MACRO(hRes, pDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST, nDrawCount, m_VertTransBufT, sizeof(DRAWPRIM_D3DTRANS_TEX)));
			if (hRes != D3D_OK) 
				return LT_ERROR; 
	} }

	return LT_OK;
}

LTRESULT CD3DDrawPrim::DrawPrim(LT_POLYG3 *pPrim, uint32 nCount) {
	LPDIRECT3DDEVICE9 pDevice = r_GetRenderStruct()->GetD3DDevice();
	if (!pDevice) return LT_ERROR;
	CAutoDrawPrimBlock AutoDPBlock(this);

	if (m_eTransType != DRAWPRIM_TRANSFORM_SCREEN) {
		HRESULT hRes = pDevice->SetVertexShader(NULL);
		if (hRes != D3D_OK) return LT_ERROR;

		hRes = pDevice->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);
		if (hRes != D3D_OK) return LT_ERROR;

		EFFECT_SHADER_MACRO(hRes, pDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST, nCount, pPrim, sizeof(LT_VERTG)));
		if (hRes != D3D_OK) 
			return LT_ERROR; 
	}
	else {	// Use Transform and Lit verts...
		HRESULT hRes = pDevice->SetVertexShader(NULL);
		if (hRes != D3D_OK) return LT_ERROR;

		hRes = pDevice->SetFVF(DRAWPRIM_D3DTRANS_FLAGS);
		if (hRes != D3D_OK) return LT_ERROR;

		LT_POLYG3* pSrcPtr		= pPrim;
		while (nCount) {
			DRAWPRIM_D3DTRANS* pDstPtr = m_VertTransBuf;
			uint32 nDrawCount	= 0;
			int32 nDstBufSize	= CD3DDRAWPRIM_BUFSIZE - 3;
			while (nCount && nDstBufSize > 0) {		// Copy the tri to the buffer,..
				pDstPtr->x	    = pSrcPtr->verts[0].x; pDstPtr->y = pSrcPtr->verts[0].y; pDstPtr->z = pSrcPtr->verts[0].z;
				pDstPtr->rhw	= 1.0f;
				pDstPtr->rgba   = RGBA_MAKE(pSrcPtr->verts[0].rgba.r,pSrcPtr->verts[0].rgba.g,pSrcPtr->verts[0].rgba.b,pSrcPtr->verts[0].rgba.a);
				++pDstPtr;

				pDstPtr->x	    = pSrcPtr->verts[1].x; pDstPtr->y = pSrcPtr->verts[1].y; pDstPtr->z = pSrcPtr->verts[1].z;
				pDstPtr->rhw	= 1.0f;
				pDstPtr->rgba   = RGBA_MAKE(pSrcPtr->verts[1].rgba.r,pSrcPtr->verts[1].rgba.g,pSrcPtr->verts[1].rgba.b,pSrcPtr->verts[1].rgba.a);
				++pDstPtr;

				pDstPtr->x	    = pSrcPtr->verts[2].x; pDstPtr->y = pSrcPtr->verts[2].y; pDstPtr->z = pSrcPtr->verts[2].z;
				pDstPtr->rhw	= 1.0f;
				pDstPtr->rgba   = RGBA_MAKE(pSrcPtr->verts[2].rgba.r,pSrcPtr->verts[2].rgba.g,pSrcPtr->verts[2].rgba.b,pSrcPtr->verts[2].rgba.a);
				++pDstPtr; 

				--nCount; nDstBufSize -= 3; ++nDrawCount; ++pSrcPtr; }

			EFFECT_SHADER_MACRO(hRes, pDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST, nDrawCount, m_VertTransBuf, sizeof(DRAWPRIM_D3DTRANS)));
			if (hRes != D3D_OK) 
				return LT_ERROR; 
	} }

	return LT_OK;
}

LTRESULT CD3DDrawPrim::DrawPrim(LT_POLYF3 *pPrim, uint32 nCount) {
	LPDIRECT3DDEVICE9 pDevice = r_GetRenderStruct()->GetD3DDevice();
	if (!pDevice) return LT_ERROR;
	CAutoDrawPrimBlock AutoDPBlock(this);

	if (m_eTransType != DRAWPRIM_TRANSFORM_SCREEN) {
		HRESULT hRes = pDevice->SetVertexShader(NULL);
		if (hRes != D3D_OK) return LT_ERROR;

		hRes = pDevice->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);
		if (hRes != D3D_OK) return LT_ERROR;

		LT_POLYF3* pSrcPtr		= pPrim;
		while (nCount) {
			LT_VERTG*  pDstPtr  = m_VertBufG;
			uint32 nDrawCount	= 0;
			int32 nDstBufSize	= CD3DDRAWPRIM_BUFSIZE - 3;
			while (nCount && nDstBufSize > 0) {		// Copy the tri to the buffer,..
				pDstPtr->x	    = pSrcPtr->verts[0].x; pDstPtr->y = pSrcPtr->verts[0].y; pDstPtr->z = pSrcPtr->verts[0].z;
				pDstPtr->rgba   = pSrcPtr->rgba;
				++pDstPtr;

				pDstPtr->x	    = pSrcPtr->verts[1].x; pDstPtr->y = pSrcPtr->verts[1].y; pDstPtr->z = pSrcPtr->verts[1].z;
				pDstPtr->rgba   = pSrcPtr->rgba;
				++pDstPtr;

				pDstPtr->x	    = pSrcPtr->verts[2].x; pDstPtr->y = pSrcPtr->verts[2].y; pDstPtr->z = pSrcPtr->verts[2].z;
				pDstPtr->rgba   = pSrcPtr->rgba;
				++pDstPtr; 

				--nCount; nDstBufSize -= 3; ++nDrawCount; ++pSrcPtr; }

			EFFECT_SHADER_MACRO(hRes, pDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST, nDrawCount, m_VertBufG, sizeof(LT_VERTG))); 
			if (hRes != D3D_OK)
				return LT_ERROR; 
	} }
	else {	// Use Transform and Lit verts...
		HRESULT hRes = pDevice->SetVertexShader(NULL);
		if (hRes != D3D_OK) return LT_ERROR;

		hRes = pDevice->SetFVF(DRAWPRIM_D3DTRANS_FLAGS);
		if (hRes != D3D_OK) return LT_ERROR;

		LT_POLYF3* pSrcPtr		= pPrim;
		while (nCount) {
			DRAWPRIM_D3DTRANS* pDstPtr = m_VertTransBuf;
			uint32 nDrawCount	= 0;
			int32 nDstBufSize	= CD3DDRAWPRIM_BUFSIZE - 3;
			while (nCount && nDstBufSize > 0) {		// Copy the tri to the buffer,..
				pDstPtr->x	    = pSrcPtr->verts[0].x; pDstPtr->y = pSrcPtr->verts[0].y; pDstPtr->z = pSrcPtr->verts[0].z;
				pDstPtr->rhw	= 1.0f;
				pDstPtr->rgba   = RGBA_MAKE(pSrcPtr->rgba.r,pSrcPtr->rgba.g,pSrcPtr->rgba.b,pSrcPtr->rgba.a);
				++pDstPtr;

				pDstPtr->x	    = pSrcPtr->verts[1].x; pDstPtr->y = pSrcPtr->verts[1].y; pDstPtr->z = pSrcPtr->verts[1].z;
				pDstPtr->rhw	= 1.0f;
				pDstPtr->rgba   = RGBA_MAKE(pSrcPtr->rgba.r,pSrcPtr->rgba.g,pSrcPtr->rgba.b,pSrcPtr->rgba.a);
				++pDstPtr;

				pDstPtr->x	    = pSrcPtr->verts[2].x; pDstPtr->y = pSrcPtr->verts[2].y; pDstPtr->z = pSrcPtr->verts[2].z;
				pDstPtr->rhw	= 1.0f;
				pDstPtr->rgba   = RGBA_MAKE(pSrcPtr->rgba.r,pSrcPtr->rgba.g,pSrcPtr->rgba.b,pSrcPtr->rgba.a);
				++pDstPtr; 

				--nCount; nDstBufSize -= 3; ++nDrawCount; ++pSrcPtr; }

			EFFECT_SHADER_MACRO(hRes,pDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST, nDrawCount, m_VertTransBuf, sizeof(DRAWPRIM_D3DTRANS))); 
			if (hRes != D3D_OK)
				return LT_ERROR;
	} }
	
	return LT_OK;
}

// Draw primitive calls (quadrilaterals)
LTRESULT CD3DDrawPrim::DrawPrim(LT_POLYGT4 *pPrim, uint32 nCount) {
	LPDIRECT3DDEVICE9 pDevice = r_GetRenderStruct()->GetD3DDevice();
	if (!pDevice) return LT_ERROR;
	CAutoDrawPrimBlock AutoDPBlock(this);

	if (m_eTransType != DRAWPRIM_TRANSFORM_SCREEN) {
		HRESULT hRes = pDevice->SetVertexShader(NULL);
		if (hRes != D3D_OK) return LT_ERROR;

		hRes = pDevice->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1);
		if (hRes != D3D_OK) return LT_ERROR;

		LT_POLYGT4* pPrimPtr = pPrim;
		while (nCount) 
		{	// Note: Not checking return everytime for speed...
			EFFECT_SHADER_MACRO(hRes, pDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, pPrimPtr, sizeof(LT_VERTGT)));

			--nCount; ++pPrimPtr;
		}
		if (hRes != D3D_OK)
			return LT_ERROR;
	}
	else {	// Use Transform and Lit verts...
		HRESULT hRes = pDevice->SetVertexShader(NULL);
		if (hRes != D3D_OK) return LT_ERROR;

		hRes = pDevice->SetFVF(DRAWPRIM_D3DTRANS_TEX_FLAGS);
		if (hRes != D3D_OK) return LT_ERROR;

		if (nCount > 2) {
			LT_POLYGT4* pSrcPtr		= pPrim;
			while (nCount) {
				DRAWPRIM_D3DTRANS_TEX* pDstPtr = m_VertTransBufT;
				uint32 nDrawCount	= 0;
				int32 nDstBufSize	= CD3DDRAWPRIM_BUFSIZE - 6;
				while (nCount && nDstBufSize > 0) {		// Copy the tri to the buffer,..
					pDstPtr->x	    = pSrcPtr->verts[0].x; pDstPtr->y = pSrcPtr->verts[0].y; pDstPtr->z = pSrcPtr->verts[0].z;
					pDstPtr->rhw	= 1.0f;
					pDstPtr->rgba   = RGBA_MAKE(pSrcPtr->verts[0].rgba.r,pSrcPtr->verts[0].rgba.g,pSrcPtr->verts[0].rgba.b,pSrcPtr->verts[0].rgba.a);
					pDstPtr->u	    = pSrcPtr->verts[0].u; pDstPtr->v = pSrcPtr->verts[0].v; 
					++pDstPtr;

					pDstPtr->x	    = pSrcPtr->verts[1].x; pDstPtr->y = pSrcPtr->verts[1].y; pDstPtr->z = pSrcPtr->verts[1].z;
					pDstPtr->rhw	= 1.0f;
					pDstPtr->rgba   = RGBA_MAKE(pSrcPtr->verts[1].rgba.r,pSrcPtr->verts[1].rgba.g,pSrcPtr->verts[1].rgba.b,pSrcPtr->verts[1].rgba.a);
					pDstPtr->u	    = pSrcPtr->verts[1].u; pDstPtr->v = pSrcPtr->verts[1].v; 
					++pDstPtr;

					pDstPtr->x	    = pSrcPtr->verts[2].x; pDstPtr->y = pSrcPtr->verts[2].y; pDstPtr->z = pSrcPtr->verts[2].z;
					pDstPtr->rhw	= 1.0f;
					pDstPtr->rgba   = RGBA_MAKE(pSrcPtr->verts[2].rgba.r,pSrcPtr->verts[2].rgba.g,pSrcPtr->verts[2].rgba.b,pSrcPtr->verts[2].rgba.a);
					pDstPtr->u	    = pSrcPtr->verts[2].u; pDstPtr->v = pSrcPtr->verts[2].v; 
					++pDstPtr;

					pDstPtr->x	    = pSrcPtr->verts[0].x; pDstPtr->y = pSrcPtr->verts[0].y; pDstPtr->z = pSrcPtr->verts[0].z;
					pDstPtr->rhw	= 1.0f;
					pDstPtr->rgba   = RGBA_MAKE(pSrcPtr->verts[0].rgba.r,pSrcPtr->verts[0].rgba.g,pSrcPtr->verts[0].rgba.b,pSrcPtr->verts[0].rgba.a);
					pDstPtr->u	    = pSrcPtr->verts[0].u; pDstPtr->v = pSrcPtr->verts[0].v; 
					++pDstPtr;

					pDstPtr->x	    = pSrcPtr->verts[2].x; pDstPtr->y = pSrcPtr->verts[2].y; pDstPtr->z = pSrcPtr->verts[2].z;
					pDstPtr->rhw	= 1.0f;
					pDstPtr->rgba   = RGBA_MAKE(pSrcPtr->verts[2].rgba.r,pSrcPtr->verts[2].rgba.g,pSrcPtr->verts[2].rgba.b,pSrcPtr->verts[2].rgba.a);
					pDstPtr->u	    = pSrcPtr->verts[2].u; pDstPtr->v = pSrcPtr->verts[2].v; 
					++pDstPtr;

					pDstPtr->x	    = pSrcPtr->verts[3].x; pDstPtr->y = pSrcPtr->verts[3].y; pDstPtr->z = pSrcPtr->verts[3].z;
					pDstPtr->rhw	= 1.0f;
					pDstPtr->rgba   = RGBA_MAKE(pSrcPtr->verts[3].rgba.r,pSrcPtr->verts[3].rgba.g,pSrcPtr->verts[3].rgba.b,pSrcPtr->verts[3].rgba.a);
					pDstPtr->u	    = pSrcPtr->verts[3].u; pDstPtr->v = pSrcPtr->verts[3].v; 
					++pDstPtr;

					--nCount; nDstBufSize -= 6; nDrawCount += 2; ++pSrcPtr; }
				EFFECT_SHADER_MACRO(hRes, pDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST, nDrawCount, m_VertTransBufT, sizeof(DRAWPRIM_D3DTRANS_TEX)));
				if (hRes != D3D_OK)
				{
					return LT_ERROR;
				}
		} }
		else {
  			LT_POLYGT4* pPrimPtr = pPrim;
  			while (nCount) {	// Note: Not checking return everytime for speed...
  				for (int i=0;i<4;++i) {
  					m_VertTransBufT[i].x	= pPrimPtr->verts[i].x; m_VertTransBufT[i].y = pPrimPtr->verts[i].y; m_VertTransBufT[i].z = pPrimPtr->verts[i].z; m_VertTransBufT[i].rhw = 1.0f;
  					m_VertTransBufT[i].rgba = RGBA_MAKE(pPrimPtr->verts[i].rgba.r,pPrimPtr->verts[i].rgba.g,pPrimPtr->verts[i].rgba.b,pPrimPtr->verts[i].rgba.a);
  					m_VertTransBufT[i].u	= pPrimPtr->verts[i].u; m_VertTransBufT[i].v = pPrimPtr->verts[i].v; }

  			EFFECT_SHADER_MACRO(hRes, pDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, m_VertTransBufT, sizeof(DRAWPRIM_D3DTRANS_TEX)));
			if (hRes != D3D_OK) 
				return LT_ERROR;
			--nCount; ++pPrimPtr; } }

		 }

	return LT_OK;
}


// special version added by adam s. for optimized wide font rendering
LTRESULT CD3DDrawPrim::DrawPrim(LT_POLYGT4 **ppPrim, uint32 nCount) {
	LPDIRECT3DDEVICE9 pDevice = r_GetRenderStruct()->GetD3DDevice();
	if (!pDevice) return LT_ERROR;
	CAutoDrawPrimBlock AutoDPBlock(this);

	if (m_eTransType != DRAWPRIM_TRANSFORM_SCREEN) {
		HRESULT hRes = pDevice->SetVertexShader(NULL);
		if (hRes != D3D_OK) return LT_ERROR;

		hRes = pDevice->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1);
		if (hRes != D3D_OK) return LT_ERROR;

		LT_POLYGT4** ppPrimPtr = ppPrim;
		while (nCount) 
		{	// Note: Not checking return everytime for speed...
			EFFECT_SHADER_MACRO(hRes, pDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, *ppPrimPtr, sizeof(LT_VERTGT)));
			if (hRes != D3D_OK)
				return LT_ERROR;
			--nCount; ++ppPrimPtr; }
		 }
	else {	// Use Transform and Lit verts...
		HRESULT hRes = pDevice->SetVertexShader(NULL);
		if (hRes != D3D_OK) return LT_ERROR;
		hRes = pDevice->SetFVF(DRAWPRIM_D3DTRANS_TEX_FLAGS);
		if (hRes != D3D_OK) return LT_ERROR;

		if (nCount > 2) {
			LT_POLYGT4**	pPtrArray	= ppPrim;
			LT_POLYGT4*		pSrcPtr;
			//LT_POLYGT4* pSrcPtr		= pPrim;
			while (nCount) {
				
				DRAWPRIM_D3DTRANS_TEX* pDstPtr = m_VertTransBufT;
				uint32 nDrawCount	= 0;
				int32 nDstBufSize	= CD3DDRAWPRIM_BUFSIZE - 6;
				while (nCount && nDstBufSize > 0) {		// Copy the tri to the buffer,..
					pSrcPtr = *pPtrArray;

					pDstPtr->x	    = pSrcPtr->verts[0].x; pDstPtr->y = pSrcPtr->verts[0].y; pDstPtr->z = pSrcPtr->verts[0].z;
					pDstPtr->rhw	= 1.0f;
					pDstPtr->rgba   = RGBA_MAKE(pSrcPtr->verts[0].rgba.r,pSrcPtr->verts[0].rgba.g,pSrcPtr->verts[0].rgba.b,pSrcPtr->verts[0].rgba.a);
					pDstPtr->u	    = pSrcPtr->verts[0].u; pDstPtr->v = pSrcPtr->verts[0].v; 
					++pDstPtr;

					pDstPtr->x	    = pSrcPtr->verts[1].x; pDstPtr->y = pSrcPtr->verts[1].y; pDstPtr->z = pSrcPtr->verts[1].z;
					pDstPtr->rhw	= 1.0f;
					pDstPtr->rgba   = RGBA_MAKE(pSrcPtr->verts[1].rgba.r,pSrcPtr->verts[1].rgba.g,pSrcPtr->verts[1].rgba.b,pSrcPtr->verts[1].rgba.a);
					pDstPtr->u	    = pSrcPtr->verts[1].u; pDstPtr->v = pSrcPtr->verts[1].v; 
					++pDstPtr;

					pDstPtr->x	    = pSrcPtr->verts[2].x; pDstPtr->y = pSrcPtr->verts[2].y; pDstPtr->z = pSrcPtr->verts[2].z;
					pDstPtr->rhw	= 1.0f;
					pDstPtr->rgba   = RGBA_MAKE(pSrcPtr->verts[2].rgba.r,pSrcPtr->verts[2].rgba.g,pSrcPtr->verts[2].rgba.b,pSrcPtr->verts[2].rgba.a);
					pDstPtr->u	    = pSrcPtr->verts[2].u; pDstPtr->v = pSrcPtr->verts[2].v; 
					++pDstPtr;

					pDstPtr->x	    = pSrcPtr->verts[0].x; pDstPtr->y = pSrcPtr->verts[0].y; pDstPtr->z = pSrcPtr->verts[0].z;
					pDstPtr->rhw	= 1.0f;
					pDstPtr->rgba   = RGBA_MAKE(pSrcPtr->verts[0].rgba.r,pSrcPtr->verts[0].rgba.g,pSrcPtr->verts[0].rgba.b,pSrcPtr->verts[0].rgba.a);
					pDstPtr->u	    = pSrcPtr->verts[0].u; pDstPtr->v = pSrcPtr->verts[0].v; 
					++pDstPtr;

					pDstPtr->x	    = pSrcPtr->verts[2].x; pDstPtr->y = pSrcPtr->verts[2].y; pDstPtr->z = pSrcPtr->verts[2].z;
					pDstPtr->rhw	= 1.0f;
					pDstPtr->rgba   = RGBA_MAKE(pSrcPtr->verts[2].rgba.r,pSrcPtr->verts[2].rgba.g,pSrcPtr->verts[2].rgba.b,pSrcPtr->verts[2].rgba.a);
					pDstPtr->u	    = pSrcPtr->verts[2].u; pDstPtr->v = pSrcPtr->verts[2].v; 
					++pDstPtr;
					pDstPtr->x	    = pSrcPtr->verts[3].x; pDstPtr->y = pSrcPtr->verts[3].y; pDstPtr->z = pSrcPtr->verts[3].z;
					pDstPtr->rhw	= 1.0f;
					pDstPtr->rgba   = RGBA_MAKE(pSrcPtr->verts[3].rgba.r,pSrcPtr->verts[3].rgba.g,pSrcPtr->verts[3].rgba.b,pSrcPtr->verts[3].rgba.a);
					pDstPtr->u	    = pSrcPtr->verts[3].u; pDstPtr->v = pSrcPtr->verts[3].v; 
					++pDstPtr;

					--nCount; nDstBufSize -= 6; nDrawCount += 2; ++pPtrArray; }
				EFFECT_SHADER_MACRO(hRes, pDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST, nDrawCount, m_VertTransBufT, sizeof(DRAWPRIM_D3DTRANS_TEX)));
				if (hRes != D3D_OK)
					return LT_ERROR;
		} }
		else {
  			LT_POLYGT4** pPrimPtr = ppPrim;
  			while (nCount) 
			{	// Note: Not checking return everytime for speed...
  				for (int i=0;i<4;++i) {
  					m_VertTransBufT[i].x	= (*pPrimPtr)->verts[i].x; m_VertTransBufT[i].y = (*pPrimPtr)->verts[i].y; m_VertTransBufT[i].z = (*pPrimPtr)->verts[i].z; m_VertTransBufT[i].rhw = 1.0f;
  					m_VertTransBufT[i].rgba = RGBA_MAKE((*pPrimPtr)->verts[i].rgba.r,(*pPrimPtr)->verts[i].rgba.g,(*pPrimPtr)->verts[i].rgba.b,(*pPrimPtr)->verts[i].rgba.a);
  					m_VertTransBufT[i].u	= (*pPrimPtr)->verts[i].u; m_VertTransBufT[i].v = (*pPrimPtr)->verts[i].v; }

  			EFFECT_SHADER_MACRO(hRes, pDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, m_VertTransBufT, sizeof(DRAWPRIM_D3DTRANS_TEX)));
			if (hRes != D3D_OK)
				return LT_ERROR; 
			--nCount; ++pPrimPtr; } }

		}

	return LT_OK;
}


LTRESULT CD3DDrawPrim::DrawPrim(LT_POLYFT4 *pPrim, uint32 nCount) {
	LPDIRECT3DDEVICE9 pDevice = r_GetRenderStruct()->GetD3DDevice();
	if (!pDevice) return LT_ERROR;
	CAutoDrawPrimBlock AutoDPBlock(this);

	if (m_eTransType != DRAWPRIM_TRANSFORM_SCREEN) {
		HRESULT hRes = pDevice->SetVertexShader(NULL);
		if (hRes != D3D_OK) return LT_ERROR;

		hRes = pDevice->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1);
		if (hRes != D3D_OK) return LT_ERROR;

		LT_POLYFT4* pPrimPtr = pPrim;
		while (nCount) {	// Note: Not checking return everytime for speed...
			for (int i=0;i<4;++i) {
				m_VertBufGT[i].x	= pPrimPtr->verts[i].x; m_VertBufGT[i].y = pPrimPtr->verts[i].y; m_VertBufGT[i].z = pPrimPtr->verts[i].z;
				m_VertBufGT[i].rgba = pPrimPtr->rgba;
				m_VertBufGT[i].u	= pPrimPtr->verts[i].u; m_VertBufGT[i].v = pPrimPtr->verts[i].v; }
			EFFECT_SHADER_MACRO(hRes, pDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, m_VertBufGT, sizeof(LT_VERTGT)));
			if (hRes != D3D_OK)
				return LT_ERROR;
			--nCount; ++pPrimPtr; }
		 }
	else {	// Use Transform and Lit verts...
		HRESULT hRes = pDevice->SetVertexShader(NULL);
		if (hRes != D3D_OK) return LT_ERROR;

		hRes = pDevice->SetFVF(DRAWPRIM_D3DTRANS_TEX_FLAGS);
		if (hRes != D3D_OK) return LT_ERROR;

		LT_POLYFT4* pPrimPtr = pPrim;
		while (nCount) {	// Note: Not checking return everytime for speed...
			for (int i=0;i<4;++i) {
				m_VertTransBufT[i].x	= pPrimPtr->verts[i].x; m_VertTransBufT[i].y = pPrimPtr->verts[i].y; m_VertTransBufT[i].z = pPrimPtr->verts[i].z; m_VertTransBufT[i].rhw = 1.0f;
				m_VertTransBufT[i].rgba = RGBA_MAKE(pPrimPtr->rgba.r,pPrimPtr->rgba.g,pPrimPtr->rgba.b,pPrimPtr->rgba.a);
				m_VertTransBufT[i].u	= pPrimPtr->verts[i].u; m_VertTransBufT[i].v = pPrimPtr->verts[i].v; }
			EFFECT_SHADER_MACRO(hRes, pDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, m_VertTransBufT, sizeof(DRAWPRIM_D3DTRANS_TEX)));
			if (hRes != D3D_OK)
				return LT_ERROR; 
			--nCount; ++pPrimPtr; }
		}

	return LT_OK;
}

LTRESULT CD3DDrawPrim::DrawPrim(LT_POLYG4 *pPrim, uint32 nCount) {
	LPDIRECT3DDEVICE9 pDevice = r_GetRenderStruct()->GetD3DDevice();
	if (!pDevice) return LT_ERROR;
	CAutoDrawPrimBlock AutoDPBlock(this);

	if (m_eTransType != DRAWPRIM_TRANSFORM_SCREEN) {
		HRESULT hRes = pDevice->SetVertexShader(NULL);
		if (hRes != D3D_OK) return LT_ERROR;

		hRes = pDevice->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);
		if (hRes != D3D_OK) return LT_ERROR;

		LT_POLYG4* pPrimPtr = pPrim;
		while (nCount) 
		{	// Note: Not checking return everytime for speed...
			EFFECT_SHADER_MACRO(hRes, pDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, pPrimPtr, sizeof(LT_VERTG)));
			if (hRes != D3D_OK)
				return LT_ERROR; 
			--nCount; ++pPrimPtr; }
		}
	else {	// Use Transform and Lit verts...
		HRESULT hRes = pDevice->SetVertexShader(NULL);
		if (hRes != D3D_OK) return LT_ERROR;
		hRes = pDevice->SetFVF(DRAWPRIM_D3DTRANS_FLAGS);
		if (hRes != D3D_OK) return LT_ERROR;

		LT_POLYG4* pPrimPtr = pPrim;
		while (nCount) {	// Note: Not checking return everytime for speed...
			for (int i=0;i<4;++i) {
				m_VertTransBuf[i].x	= pPrimPtr->verts[i].x; m_VertTransBuf[i].y = pPrimPtr->verts[i].y; m_VertTransBuf[i].z = pPrimPtr->verts[i].z; m_VertTransBuf[i].rhw = 1.0f;
				m_VertTransBuf[i].rgba = RGBA_MAKE(pPrimPtr->verts[i].rgba.r,pPrimPtr->verts[i].rgba.g,pPrimPtr->verts[i].rgba.b,pPrimPtr->verts[i].rgba.a); }
			EFFECT_SHADER_MACRO(hRes, pDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, m_VertTransBuf, sizeof(DRAWPRIM_D3DTRANS)));
			if (hRes != D3D_OK)
				return LT_ERROR; 
			--nCount; ++pPrimPtr; }
		}

	return LT_OK;
}

LTRESULT CD3DDrawPrim::DrawPrim(LT_POLYF4 *pPrim, uint32 nCount) {
	LPDIRECT3DDEVICE9 pDevice = r_GetRenderStruct()->GetD3DDevice();
	if (!pDevice) return LT_ERROR;
	CAutoDrawPrimBlock AutoDPBlock(this);

	if (m_eTransType != DRAWPRIM_TRANSFORM_SCREEN) {
		HRESULT hRes = pDevice->SetVertexShader(NULL);
		if (hRes != D3D_OK) return LT_ERROR;

		hRes = pDevice->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);
		if (hRes != D3D_OK) return LT_ERROR;

		LT_POLYF4* pPrimPtr = pPrim;
		while (nCount) {	// Note: Not checking return everytime for speed...
			for (int i=0;i<4;++i) {
				m_VertBufG[i].x	= pPrimPtr->verts[i].x; m_VertBufG[i].y = pPrimPtr->verts[i].y; m_VertBufG[i].z = pPrimPtr->verts[i].z;
				m_VertBufG[i].rgba = pPrimPtr->rgba; }
			EFFECT_SHADER_MACRO(hRes, pDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, m_VertBufG, sizeof(LT_VERTG)));
			if (hRes != D3D_OK)
				return LT_ERROR; 
			--nCount; ++pPrimPtr; }
		}
	else {	// Use Transform and Lit verts...
		HRESULT hRes = pDevice->SetVertexShader(NULL);
		if (hRes != D3D_OK) return LT_ERROR;

		hRes = pDevice->SetFVF(DRAWPRIM_D3DTRANS_FLAGS);
		if (hRes != D3D_OK) return LT_ERROR;

		LT_POLYF4* pPrimPtr = pPrim;
		while (nCount) {	// Note: Not checking return everytime for speed...
			for (int i=0;i<4;++i) {
				m_VertTransBuf[i].x	= pPrimPtr->verts[i].x; m_VertTransBuf[i].y = pPrimPtr->verts[i].y; m_VertTransBuf[i].z = pPrimPtr->verts[i].z; m_VertTransBuf[i].rhw = 1.0f;
				m_VertTransBuf[i].rgba = RGBA_MAKE(pPrimPtr->rgba.r,pPrimPtr->rgba.g,pPrimPtr->rgba.b,pPrimPtr->rgba.a); }
			EFFECT_SHADER_MACRO(hRes, pDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, m_VertTransBuf, sizeof(DRAWPRIM_D3DTRANS)));
			if (hRes != D3D_OK)
				return LT_ERROR; 
			--nCount; ++pPrimPtr; }
		}

	return LT_OK;
}

// Draw primitives using lines (Note: nCount is Line count).
LTRESULT CD3DDrawPrim::DrawPrim (LT_LINEGT *pPrim, uint32 nCount)
{
	LPDIRECT3DDEVICE9 pDevice = r_GetRenderStruct()->GetD3DDevice();
	if (!pDevice) return LT_ERROR;
	CAutoDrawPrimBlock AutoDPBlock(this);

	if (m_eTransType != DRAWPRIM_TRANSFORM_SCREEN) {
		HRESULT hRes = pDevice->SetVertexShader(NULL);
		if (hRes != D3D_OK) return LT_ERROR;

		hRes = pDevice->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1);
		if (hRes != D3D_OK) return LT_ERROR;

		EFFECT_SHADER_MACRO(hRes,pDevice->DrawPrimitiveUP(D3DPT_LINELIST, nCount, pPrim, sizeof(LT_VERTGT)));
		if (hRes != D3D_OK)
			return LT_ERROR;
	}
	else {	// Use Transform and Lit verts...
		HRESULT hRes = pDevice->SetVertexShader(NULL);
		if (hRes != D3D_OK) return LT_ERROR;

		hRes = pDevice->SetFVF(DRAWPRIM_D3DTRANS_TEX_FLAGS);
		if (hRes != D3D_OK) return LT_ERROR;

		LT_LINEGT* pSrcPtr		= pPrim;
		while (nCount) {
			DRAWPRIM_D3DTRANS_TEX* pDstPtr = m_VertTransBufT;
			uint32 nDrawCount	= 0;
			int32 nDstBufSize	= CD3DDRAWPRIM_BUFSIZE - 2;
			while (nCount && nDstBufSize > 0) {		// Copy the tri to the buffer,..
				pDstPtr->x	    = pSrcPtr->verts[0].x; pDstPtr->y = pSrcPtr->verts[0].y; pDstPtr->z = pSrcPtr->verts[0].z;
				pDstPtr->rhw	= 1.0f;
				pDstPtr->rgba   = RGBA_MAKE(pSrcPtr->verts[0].rgba.r,pSrcPtr->verts[0].rgba.g,pSrcPtr->verts[0].rgba.b,pSrcPtr->verts[0].rgba.a);
				pDstPtr->u	    = pSrcPtr->verts[0].u; pDstPtr->v = pSrcPtr->verts[0].v; 
				++pDstPtr;

				pDstPtr->x	    = pSrcPtr->verts[1].x; pDstPtr->y = pSrcPtr->verts[1].y; pDstPtr->z = pSrcPtr->verts[1].z;
				pDstPtr->rhw	= 1.0f;
				pDstPtr->rgba   = RGBA_MAKE(pSrcPtr->verts[1].rgba.r,pSrcPtr->verts[1].rgba.g,pSrcPtr->verts[1].rgba.b,pSrcPtr->verts[1].rgba.a);
				pDstPtr->u	    = pSrcPtr->verts[1].u; pDstPtr->v = pSrcPtr->verts[1].v; 
				++pDstPtr;

				--nCount; nDstBufSize -= 2; ++nDrawCount; ++pSrcPtr; }

			EFFECT_SHADER_MACRO(hRes, pDevice->DrawPrimitiveUP(D3DPT_LINELIST, nDrawCount, m_VertTransBufT, sizeof(DRAWPRIM_D3DTRANS_TEX))); 
			if (hRes != D3D_OK)
				return LT_ERROR;
	} }

	return LT_OK;
}

LTRESULT CD3DDrawPrim::DrawPrim (LT_LINEFT *pPrim, uint32 nCount)
{
	LPDIRECT3DDEVICE9 pDevice = r_GetRenderStruct()->GetD3DDevice();
	if (!pDevice) return LT_ERROR;
	CAutoDrawPrimBlock AutoDPBlock(this);

	if (m_eTransType != DRAWPRIM_TRANSFORM_SCREEN) {
		HRESULT hRes = pDevice->SetVertexShader(NULL);
		if (hRes != D3D_OK) return LT_ERROR;

		hRes = pDevice->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1);
		if (hRes != D3D_OK) return LT_ERROR;

		LT_LINEFT* pSrcPtr		= pPrim;
		while (nCount) {
			LT_VERTGT*  pDstPtr = m_VertBufGT;
			uint32 nDrawCount	= 0;
			int32 nDstBufSize	= CD3DDRAWPRIM_BUFSIZE - 2;
			while (nCount && nDstBufSize > 0) {		// Copy the tri to the buffer,..
				pDstPtr->x	    = pSrcPtr->verts[0].x; pDstPtr->y = pSrcPtr->verts[0].y; pDstPtr->z = pSrcPtr->verts[0].z;
				pDstPtr->rgba   = pSrcPtr->rgba;
				pDstPtr->u	    = pSrcPtr->verts[0].u; pDstPtr->v = pSrcPtr->verts[0].v; 
				++pDstPtr;

				pDstPtr->x	    = pSrcPtr->verts[1].x; pDstPtr->y = pSrcPtr->verts[1].y; pDstPtr->z = pSrcPtr->verts[1].z;
				pDstPtr->rgba   = pSrcPtr->rgba;
				pDstPtr->u	    = pSrcPtr->verts[1].u; pDstPtr->v = pSrcPtr->verts[1].v; 
				++pDstPtr;

				--nCount; nDstBufSize -= 2; ++nDrawCount; ++pSrcPtr; }

			EFFECT_SHADER_MACRO(hRes, pDevice->DrawPrimitiveUP(D3DPT_LINELIST, nDrawCount, m_VertBufGT, sizeof(LT_VERTGT))); 
			if (hRes != D3D_OK)
				return LT_ERROR;
	} }
	else {	// Use Transform and Lit verts...
		HRESULT hRes = pDevice->SetVertexShader(NULL);
		if (hRes != D3D_OK) return LT_ERROR;

		hRes = pDevice->SetFVF(DRAWPRIM_D3DTRANS_TEX_FLAGS);
		if (hRes != D3D_OK) return LT_ERROR;

		LT_LINEFT* pSrcPtr		= pPrim;
		while (nCount) {
			DRAWPRIM_D3DTRANS_TEX* pDstPtr = m_VertTransBufT;
			uint32 nDrawCount	= 0;
			int32 nDstBufSize	= CD3DDRAWPRIM_BUFSIZE - 2;
			while (nCount && nDstBufSize > 0) {		// Copy the tri to the buffer,..
				pDstPtr->x	    = pSrcPtr->verts[0].x; pDstPtr->y = pSrcPtr->verts[0].y; pDstPtr->z = pSrcPtr->verts[0].z;
				pDstPtr->rhw	= 1.0f;
				pDstPtr->rgba   = RGBA_MAKE(pSrcPtr->rgba.r,pSrcPtr->rgba.g,pSrcPtr->rgba.b,pSrcPtr->rgba.a);
				pDstPtr->u	    = pSrcPtr->verts[0].u; pDstPtr->v = pSrcPtr->verts[0].v; 
				++pDstPtr;

				pDstPtr->x	    = pSrcPtr->verts[1].x; pDstPtr->y = pSrcPtr->verts[1].y; pDstPtr->z = pSrcPtr->verts[1].z;
				pDstPtr->rhw	= 1.0f;
				pDstPtr->rgba   = RGBA_MAKE(pSrcPtr->rgba.r,pSrcPtr->rgba.g,pSrcPtr->rgba.b,pSrcPtr->rgba.a);
				pDstPtr->u	    = pSrcPtr->verts[1].u; pDstPtr->v = pSrcPtr->verts[1].v; 
				++pDstPtr;

				--nCount; nDstBufSize -= 2; ++nDrawCount; ++pSrcPtr; }

			EFFECT_SHADER_MACRO(hRes, pDevice->DrawPrimitiveUP(D3DPT_LINELIST, nDrawCount, m_VertTransBufT, sizeof(DRAWPRIM_D3DTRANS_TEX))); 
			if (hRes != D3D_OK)
				return LT_ERROR;
	} }

	return LT_OK;
}

LTRESULT CD3DDrawPrim::DrawPrim (LT_LINEG *pPrim, uint32 nCount)
{
	LPDIRECT3DDEVICE9 pDevice = r_GetRenderStruct()->GetD3DDevice();
	if (!pDevice) return LT_ERROR;
	CAutoDrawPrimBlock AutoDPBlock(this);

	if (m_eTransType != DRAWPRIM_TRANSFORM_SCREEN) {
		HRESULT hRes = pDevice->SetVertexShader(NULL);
		if (hRes != D3D_OK) return LT_ERROR;

		hRes = pDevice->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);
		if (hRes != D3D_OK) return LT_ERROR;

		EFFECT_SHADER_MACRO(hRes, pDevice->DrawPrimitiveUP(D3DPT_LINELIST, nCount, pPrim, sizeof(LT_VERTG)));
		if (hRes != D3D_OK)
			return LT_ERROR;
	}
	else {	// Use Transform and Lit verts...
		HRESULT hRes = pDevice->SetVertexShader(NULL);
		if (hRes != D3D_OK) return LT_ERROR;

		hRes = pDevice->SetFVF(DRAWPRIM_D3DTRANS_FLAGS);
		if (hRes != D3D_OK) return LT_ERROR;

		LT_LINEG* pSrcPtr		= pPrim;
		while (nCount) {
			DRAWPRIM_D3DTRANS* pDstPtr = m_VertTransBuf;
			uint32 nDrawCount	= 0;
			int32 nDstBufSize	= CD3DDRAWPRIM_BUFSIZE - 2;
			while (nCount && nDstBufSize > 0) {		// Copy the tri to the buffer,..
				pDstPtr->x	    = pSrcPtr->verts[0].x; pDstPtr->y = pSrcPtr->verts[0].y; pDstPtr->z = pSrcPtr->verts[0].z;
				pDstPtr->rhw	= 1.0f;
				pDstPtr->rgba   = RGBA_MAKE(pSrcPtr->verts[0].rgba.r,pSrcPtr->verts[0].rgba.g,pSrcPtr->verts[0].rgba.b,pSrcPtr->verts[0].rgba.a);
				++pDstPtr;

				pDstPtr->x	    = pSrcPtr->verts[1].x; pDstPtr->y = pSrcPtr->verts[1].y; pDstPtr->z = pSrcPtr->verts[1].z;
				pDstPtr->rhw	= 1.0f;
				pDstPtr->rgba   = RGBA_MAKE(pSrcPtr->verts[1].rgba.r,pSrcPtr->verts[1].rgba.g,pSrcPtr->verts[1].rgba.b,pSrcPtr->verts[1].rgba.a);
				++pDstPtr;

				--nCount; nDstBufSize -= 2; ++nDrawCount; ++pSrcPtr; }

			EFFECT_SHADER_MACRO(hRes, pDevice->DrawPrimitiveUP(D3DPT_LINELIST, nDrawCount, m_VertTransBuf, sizeof(DRAWPRIM_D3DTRANS))); 
			if (hRes != D3D_OK)
				return LT_ERROR;
	} }

	return LT_OK;
}

LTRESULT CD3DDrawPrim::DrawPrim (LT_LINEF *pPrim, uint32 nCount)
{
	LPDIRECT3DDEVICE9 pDevice = r_GetRenderStruct()->GetD3DDevice();
	if (!pDevice) return LT_ERROR;
	CAutoDrawPrimBlock AutoDPBlock(this);

	if (m_eTransType != DRAWPRIM_TRANSFORM_SCREEN) {
		HRESULT hRes = pDevice->SetVertexShader(NULL);
		if (hRes != D3D_OK) return LT_ERROR;

		hRes = pDevice->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);
		if (hRes != D3D_OK) return LT_ERROR;

		LT_LINEF* pSrcPtr		= pPrim;
		while (nCount) {
			LT_VERTG*  pDstPtr  = m_VertBufG;
			uint32 nDrawCount	= 0;
			int32 nDstBufSize	= CD3DDRAWPRIM_BUFSIZE - 2;
			while (nCount && nDstBufSize > 0) {		// Copy the tri to the buffer,..
				pDstPtr->x	    = pSrcPtr->verts[0].x; pDstPtr->y = pSrcPtr->verts[0].y; pDstPtr->z = pSrcPtr->verts[0].z;
				pDstPtr->rgba   = pSrcPtr->rgba;
				++pDstPtr;

				pDstPtr->x	    = pSrcPtr->verts[1].x; pDstPtr->y = pSrcPtr->verts[1].y; pDstPtr->z = pSrcPtr->verts[1].z;
				pDstPtr->rgba   = pSrcPtr->rgba;
				++pDstPtr;

				--nCount; nDstBufSize -= 2; ++nDrawCount; ++pSrcPtr; }

			EFFECT_SHADER_MACRO(hRes, pDevice->DrawPrimitiveUP(D3DPT_LINELIST, nDrawCount, m_VertBufG, sizeof(LT_VERTG))); 
			if (hRes != D3D_OK)
				return LT_ERROR;
	} }
	else {	// Use Transform and Lit verts...
		HRESULT hRes = pDevice->SetVertexShader(NULL);
		if (hRes != D3D_OK) return LT_ERROR;

		hRes = pDevice->SetFVF(DRAWPRIM_D3DTRANS_FLAGS);
		if (hRes != D3D_OK) return LT_ERROR;

		LT_LINEF* pSrcPtr		= pPrim;
		while (nCount) {
			DRAWPRIM_D3DTRANS* pDstPtr = m_VertTransBuf;
			uint32 nDrawCount	= 0;
			int32 nDstBufSize	= CD3DDRAWPRIM_BUFSIZE - 2;
			while (nCount && nDstBufSize > 0) {		// Copy the tri to the buffer,..
				pDstPtr->x	    = pSrcPtr->verts[0].x; pDstPtr->y = pSrcPtr->verts[0].y; pDstPtr->z = pSrcPtr->verts[0].z;
				pDstPtr->rhw	= 1.0f;
				pDstPtr->rgba   = RGBA_MAKE(pSrcPtr->rgba.r,pSrcPtr->rgba.g,pSrcPtr->rgba.b,pSrcPtr->rgba.a);
				++pDstPtr;

				pDstPtr->x	    = pSrcPtr->verts[1].x; pDstPtr->y = pSrcPtr->verts[1].y; pDstPtr->z = pSrcPtr->verts[1].z;
				pDstPtr->rhw	= 1.0f;
				pDstPtr->rgba   = RGBA_MAKE(pSrcPtr->rgba.r,pSrcPtr->rgba.g,pSrcPtr->rgba.b,pSrcPtr->rgba.a);
				++pDstPtr;

				--nCount; nDstBufSize -= 2; ++nDrawCount; ++pSrcPtr; }

			EFFECT_SHADER_MACRO(hRes, pDevice->DrawPrimitiveUP(D3DPT_LINELIST, nDrawCount, m_VertTransBuf, sizeof(DRAWPRIM_D3DTRANS))); 
			if (hRes != D3D_OK)
				return LT_ERROR;
	} }
	
	return LT_OK;
}

// Draw primitives using points (Note: nCount is Point count).
LTRESULT CD3DDrawPrim::DrawPrimPoint (LT_VERTGT *pVerts, uint32 nCount)
{
	LPDIRECT3DDEVICE9 pDevice = r_GetRenderStruct()->GetD3DDevice();
	if (!pDevice) return LT_ERROR;
	CAutoDrawPrimBlock AutoDPBlock(this);

	if (m_eTransType != DRAWPRIM_TRANSFORM_SCREEN) {

		HRESULT hRes = pDevice->SetVertexShader(NULL);
		if (hRes != D3D_OK) return LT_ERROR;

		hRes = pDevice->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1);
		if (hRes != D3D_OK) return LT_ERROR;

		EFFECT_SHADER_MACRO(hRes, pDevice->DrawPrimitiveUP(D3DPT_POINTLIST, nCount, pVerts, sizeof(LT_VERTGT)));
		if (hRes != D3D_OK)
			return LT_ERROR;
	}
	else {	// Use Transform and Lit verts...
		HRESULT hRes = pDevice->SetVertexShader(NULL);
		if (hRes != D3D_OK) return LT_ERROR;

		hRes = pDevice->SetFVF(DRAWPRIM_D3DTRANS_TEX_FLAGS);
		if (hRes != D3D_OK) return LT_ERROR;

		LT_VERTGT* pSrcPtr		= pVerts;
		while (nCount) {
			DRAWPRIM_D3DTRANS_TEX* pDstPtr = m_VertTransBufT;
			uint32 nDrawCount	= 0;
			int32 nDstBufSize	= CD3DDRAWPRIM_BUFSIZE - 1;
			while (nCount && nDstBufSize > 0) {		// Copy the tri to the buffer,..
				pDstPtr->x	    = pSrcPtr->x; pDstPtr->y = pSrcPtr->y; pDstPtr->z = pSrcPtr->z;
				pDstPtr->rhw	= 1.0f;
				pDstPtr->rgba   = RGBA_MAKE(pSrcPtr->rgba.r,pSrcPtr->rgba.g,pSrcPtr->rgba.b,pSrcPtr->rgba.a);
				pDstPtr->u	    = pSrcPtr->u; pDstPtr->v = pSrcPtr->v; 
				++pDstPtr;

				--nCount; --nDstBufSize; ++nDrawCount; ++pSrcPtr; }

			EFFECT_SHADER_MACRO(hRes, pDevice->DrawPrimitiveUP(D3DPT_POINTLIST, nDrawCount, m_VertTransBufT, sizeof(DRAWPRIM_D3DTRANS_TEX))); 
			if (hRes != D3D_OK)
				return LT_ERROR;
	} }

	return LT_OK;
}

LTRESULT CD3DDrawPrim::DrawPrimPoint (LT_VERTG *pVerts, uint32 nCount)
{
	LPDIRECT3DDEVICE9 pDevice = r_GetRenderStruct()->GetD3DDevice();
	if (!pDevice) return LT_ERROR;
	CAutoDrawPrimBlock AutoDPBlock(this);

	if (m_eTransType != DRAWPRIM_TRANSFORM_SCREEN) {
		HRESULT hRes = pDevice->SetVertexShader(NULL);
		if (hRes != D3D_OK) return LT_ERROR;

		hRes = pDevice->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);
		if (hRes != D3D_OK) return LT_ERROR;

		EFFECT_SHADER_MACRO(hRes, pDevice->DrawPrimitiveUP(D3DPT_POINTLIST, nCount, pVerts, sizeof(LT_VERTG)));
		if (hRes != D3D_OK)
			return LT_ERROR;
	}
	else {	// Use Transform and Lit verts...
		HRESULT hRes = pDevice->SetVertexShader(NULL);
		if (hRes != D3D_OK) return LT_ERROR;

		hRes = pDevice->SetFVF(DRAWPRIM_D3DTRANS_FLAGS);
		if (hRes != D3D_OK) return LT_ERROR;

		LT_VERTG* pSrcPtr		= pVerts;
		while (nCount) {
			DRAWPRIM_D3DTRANS* pDstPtr = m_VertTransBuf;
			uint32 nDrawCount	= 0;
			int32 nDstBufSize	= CD3DDRAWPRIM_BUFSIZE - 1;
			while (nCount && nDstBufSize > 0) {		// Copy the tri to the buffer,..
				pDstPtr->x	    = pSrcPtr->x; pDstPtr->y = pSrcPtr->y; pDstPtr->z = pSrcPtr->z;
				pDstPtr->rhw	= 1.0f;
				pDstPtr->rgba   = RGBA_MAKE(pSrcPtr->rgba.r,pSrcPtr->rgba.g,pSrcPtr->rgba.b,pSrcPtr->rgba.a);
				++pDstPtr;

				--nCount; --nDstBufSize; ++nDrawCount; ++pSrcPtr; }

			EFFECT_SHADER_MACRO(hRes, pDevice->DrawPrimitiveUP(D3DPT_POINTLIST, nDrawCount, m_VertTransBuf, sizeof(DRAWPRIM_D3DTRANS))); 
			if (hRes != D3D_OK)
				return LT_ERROR;
	} }

	return LT_OK;
}

// Draw primitive calls using triangle fans
LTRESULT CD3DDrawPrim::DrawPrimFan(LT_VERTGT *pVerts, uint32 nCount) {
	LPDIRECT3DDEVICE9 pDevice = r_GetRenderStruct()->GetD3DDevice();
	if (!pDevice) return LT_ERROR;
	CAutoDrawPrimBlock AutoDPBlock(this);

	if (m_eTransType != DRAWPRIM_TRANSFORM_SCREEN) {
		HRESULT hRes = pDevice->SetVertexShader(NULL);
		if (hRes != D3D_OK) return LT_ERROR;

		hRes = pDevice->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1);
		if (hRes != D3D_OK) return LT_ERROR;

		EFFECT_SHADER_MACRO(hRes, pDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, nCount-2, pVerts, sizeof(LT_VERTGT)));
		if (hRes != D3D_OK)
			return LT_ERROR;
	}
	else {	// Use Transform and Lit verts...
		HRESULT hRes = pDevice->SetVertexShader(NULL);
		if (hRes != D3D_OK) return LT_ERROR;

		hRes = pDevice->SetFVF(DRAWPRIM_D3DTRANS_TEX_FLAGS);
		if (hRes != D3D_OK) return LT_ERROR;

		bool bNotFirstLoop = false;
		LT_VERTGT* pSrcPtr		= pVerts;
		while (nCount) {
			DRAWPRIM_D3DTRANS_TEX* pDstPtr = m_VertTransBufT;
			uint32 nDrawCount	= 0;
			int32 nDstBufSize	= CD3DDRAWPRIM_BUFSIZE - 3;
			if (bNotFirstLoop) {					// If it's not the first, need to grab the first vert (already moved back the source pointer by one)...
				pDstPtr->x	    = pVerts->x; pDstPtr->y = pVerts->y; pDstPtr->z = pVerts->z; pDstPtr->rhw = 1.0f;
				pDstPtr->rgba   = RGBA_MAKE(pVerts->rgba.r,pVerts->rgba.g,pVerts->rgba.b,pVerts->rgba.a);
				pDstPtr->u	    = pVerts->u; pDstPtr->v = pVerts->v; 
				++pDstPtr; --nCount; ++nDrawCount; }
			else bNotFirstLoop = true;
			while (nCount && nDstBufSize > 0) {		// Copy the tri to the buffer,..
				pDstPtr->x	    = pSrcPtr->x; pDstPtr->y = pSrcPtr->y; pDstPtr->z = pSrcPtr->z; pDstPtr->rhw = 1.0f;
				pDstPtr->rgba   = RGBA_MAKE(pVerts->rgba.r,pVerts->rgba.g,pVerts->rgba.b,pVerts->rgba.a);
				pDstPtr->u	    = pSrcPtr->u; pDstPtr->v = pSrcPtr->v; 
				++pDstPtr; ++pSrcPtr;
				--nCount; --nDstBufSize; ++nDrawCount; }

			EFFECT_SHADER_MACRO(hRes, pDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, nDrawCount-2, m_VertTransBufT, sizeof(DRAWPRIM_D3DTRANS_TEX))); 
			if (hRes != D3D_OK)
				return LT_ERROR; 

			if (nCount) { --pSrcPtr; nCount += 2; } } }				// Skip back one (see notes by bNotFirstLoop)... 

	return LT_OK;
}

LTRESULT CD3DDrawPrim::DrawPrimFan(LT_VERTFT *pVerts, uint32 nCount, LT_VERTRGBA rgba) {
	LPDIRECT3DDEVICE9 pDevice = r_GetRenderStruct()->GetD3DDevice();
	if (!pDevice) return LT_ERROR;
	CAutoDrawPrimBlock AutoDPBlock(this);

	if (m_eTransType != DRAWPRIM_TRANSFORM_SCREEN) {
		HRESULT hRes = pDevice->SetVertexShader(NULL);
		if (hRes != D3D_OK) return LT_ERROR;

		hRes = pDevice->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1);
		if (hRes != D3D_OK) return LT_ERROR;

		bool bNotFirstLoop = false;
		LT_VERTFT* pSrcPtr		= pVerts;
		while (nCount) {
			LT_VERTGT*  pDstPtr = m_VertBufGT;
			uint32 nDrawCount	= 0;
			int32 nDstBufSize	= CD3DDRAWPRIM_BUFSIZE - 3;
			if (bNotFirstLoop) {					// If it's not the first, need to grab the first vert (already moved back the source pointer by one)...
				pDstPtr->x	    = pVerts->x; pDstPtr->y = pVerts->y; pDstPtr->z = pVerts->z;
				pDstPtr->rgba   = rgba;
				pDstPtr->u	    = pVerts->u; pDstPtr->v = pVerts->v; 
				++pDstPtr; --nCount; ++nDrawCount; }
			else bNotFirstLoop = true;
			while (nCount && nDstBufSize > 0) {		// Copy the tri to the buffer,..
				pDstPtr->x	    = pSrcPtr->x; pDstPtr->y = pSrcPtr->y; pDstPtr->z = pSrcPtr->z;
				pDstPtr->rgba   = rgba;
				pDstPtr->u	    = pSrcPtr->u; pDstPtr->v = pSrcPtr->v; 
				++pDstPtr; ++pSrcPtr;
				--nCount; --nDstBufSize; ++nDrawCount; }

			EFFECT_SHADER_MACRO(hRes, pDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, nDrawCount-2, m_VertBufGT, sizeof(LT_VERTGT))); 
			if (hRes != D3D_OK)
				return LT_ERROR; 

			if (nCount) { --pSrcPtr; nCount += 2; } } }				// Skip back one (see notes by bNotFirstLoop)... 
	else {	// Use Transform and Lit verts...
		HRESULT hRes = pDevice->SetVertexShader(NULL);
		if (hRes != D3D_OK) return LT_ERROR;

		hRes = pDevice->SetFVF(DRAWPRIM_D3DTRANS_TEX_FLAGS);
		if (hRes != D3D_OK) return LT_ERROR;

		bool bNotFirstLoop = false;
		LT_VERTFT* pSrcPtr		= pVerts;
		while (nCount) {
			DRAWPRIM_D3DTRANS_TEX* pDstPtr = m_VertTransBufT;
			uint32 nDrawCount	= 0;
			int32 nDstBufSize	= CD3DDRAWPRIM_BUFSIZE - 3;
			if (bNotFirstLoop) {					// If it's not the first, need to grab the first vert (already moved back the source pointer by one)...
				pDstPtr->x	    = pVerts->x; pDstPtr->y = pVerts->y; pDstPtr->z = pVerts->z; pDstPtr->rhw = 1.0f;
				pDstPtr->rgba   = RGBA_MAKE(rgba.r,rgba.g,rgba.b,rgba.a);
				pDstPtr->u	    = pVerts->u; pDstPtr->v = pVerts->v; 
				++pDstPtr; --nCount; ++nDrawCount; }
			else bNotFirstLoop = true;
			while (nCount && nDstBufSize > 0) {		// Copy the tri to the buffer,..
				pDstPtr->x	    = pSrcPtr->x; pDstPtr->y = pSrcPtr->y; pDstPtr->z = pSrcPtr->z; pDstPtr->rhw = 1.0f;
				pDstPtr->rgba   = RGBA_MAKE(rgba.r,rgba.g,rgba.b,rgba.a);
				pDstPtr->u	    = pSrcPtr->u; pDstPtr->v = pSrcPtr->v; 
				++pDstPtr; ++pSrcPtr;
				--nCount; --nDstBufSize; ++nDrawCount; }

			EFFECT_SHADER_MACRO(hRes, pDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, nDrawCount-2, m_VertTransBufT, sizeof(DRAWPRIM_D3DTRANS_TEX))); 
			if (hRes != D3D_OK)
				return LT_ERROR; 

			if (nCount) { --pSrcPtr; nCount += 2; } } }				// Skip back one (see notes by bNotFirstLoop)... 

	return LT_OK;
}

LTRESULT CD3DDrawPrim::DrawPrimFan(LT_VERTG *pVerts, uint32 nCount) {
	LPDIRECT3DDEVICE9 pDevice = r_GetRenderStruct()->GetD3DDevice();
	if (!pDevice) return LT_ERROR;
	CAutoDrawPrimBlock AutoDPBlock(this);

	if (m_eTransType != DRAWPRIM_TRANSFORM_SCREEN) {
		HRESULT hRes = pDevice->SetVertexShader(NULL);
		if (hRes != D3D_OK) return LT_ERROR;

		hRes = pDevice->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);
		if (hRes != D3D_OK) return LT_ERROR;

		EFFECT_SHADER_MACRO(hRes, pDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, nCount-2, pVerts, sizeof(LT_VERTG)));
		if (hRes != D3D_OK)
			return LT_ERROR;
	}
	else {	// Use Transform and Lit verts...
		HRESULT hRes = pDevice->SetVertexShader(NULL);
		if (hRes != D3D_OK) return LT_ERROR;

		hRes = pDevice->SetFVF(DRAWPRIM_D3DTRANS_FLAGS);
		if (hRes != D3D_OK) return LT_ERROR;

		bool bNotFirstLoop = false;
		LT_VERTG* pSrcPtr		= pVerts;
		while (nCount) {
			DRAWPRIM_D3DTRANS* pDstPtr = m_VertTransBuf;
			uint32 nDrawCount	= 0;
			int32 nDstBufSize	= CD3DDRAWPRIM_BUFSIZE - 3;
			if (bNotFirstLoop) {					// If it's not the first, need to grab the first vert (already moved back the source pointer by one)...
				pDstPtr->x	    = pVerts->x; pDstPtr->y = pVerts->y; pDstPtr->z = pVerts->z; pDstPtr->rhw = 1.0f;
				pDstPtr->rgba   = RGBA_MAKE(pVerts->rgba.r,pVerts->rgba.g,pVerts->rgba.b,pVerts->rgba.a);
				++pDstPtr; --nCount; ++nDrawCount; }
			else bNotFirstLoop = true;
			while (nCount && nDstBufSize > 0) {		// Copy the tri to the buffer,..
				pDstPtr->x	    = pSrcPtr->x; pDstPtr->y = pSrcPtr->y; pDstPtr->z = pSrcPtr->z; pDstPtr->rhw = 1.0f;
				pDstPtr->rgba   = RGBA_MAKE(pVerts->rgba.r,pVerts->rgba.g,pVerts->rgba.b,pVerts->rgba.a);
				++pDstPtr; ++pSrcPtr;
				--nCount; --nDstBufSize; ++nDrawCount; }

			EFFECT_SHADER_MACRO(hRes, pDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, nDrawCount-2, m_VertTransBuf, sizeof(DRAWPRIM_D3DTRANS))); 
			if (hRes != D3D_OK)
				return LT_ERROR; 

			if (nCount) { --pSrcPtr; nCount += 2; } } }				// Skip back one (see notes by bNotFirstLoop)... 

	return LT_OK;
}

LTRESULT CD3DDrawPrim::DrawPrimFan(LT_VERTF *pVerts, uint32 nCount, LT_VERTRGBA rgba) {
	LPDIRECT3DDEVICE9 pDevice = r_GetRenderStruct()->GetD3DDevice();
	if (!pDevice) return LT_ERROR;
	CAutoDrawPrimBlock AutoDPBlock(this);

	if (m_eTransType != DRAWPRIM_TRANSFORM_SCREEN) {
		HRESULT hRes = pDevice->SetVertexShader(NULL);
		if (hRes != D3D_OK) return LT_ERROR;

		hRes = pDevice->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);
		if (hRes != D3D_OK) return LT_ERROR;

		bool bNotFirstLoop = false;
		LT_VERTF* pSrcPtr		= pVerts;
		while (nCount) {
			LT_VERTG*  pDstPtr = m_VertBufG;
			uint32 nDrawCount	= 0;
			int32 nDstBufSize	= CD3DDRAWPRIM_BUFSIZE - 3;
			if (bNotFirstLoop) {					// If it's not the first, need to grab the first vert (already moved back the source pointer by one)...
				pDstPtr->x	    = pVerts->x; pDstPtr->y = pVerts->y; pDstPtr->z = pVerts->z;
				pDstPtr->rgba   = rgba;
				++pDstPtr; --nCount; ++nDrawCount; }
			else bNotFirstLoop = true;
			while (nCount && nDstBufSize > 0) {		// Copy the tri to the buffer,..
				pDstPtr->x	    = pSrcPtr->x; pDstPtr->y = pSrcPtr->y; pDstPtr->z = pSrcPtr->z;
				pDstPtr->rgba   = rgba;
				++pDstPtr; ++pSrcPtr;
				--nCount; --nDstBufSize; ++nDrawCount; }

			EFFECT_SHADER_MACRO(hRes, pDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, nDrawCount-2, m_VertBufG, sizeof(LT_VERTG))); 
			if (hRes != D3D_OK)
				return LT_ERROR; 

			if (nCount) { --pSrcPtr; nCount += 2; } } }				// Skip back one (see notes by bNotFirstLoop)... 
	else {	// Use Transform and Lit verts...
		HRESULT hRes = pDevice->SetVertexShader(NULL);
		if (hRes != D3D_OK) return LT_ERROR;

		hRes = pDevice->SetFVF(DRAWPRIM_D3DTRANS_FLAGS);
		if (hRes != D3D_OK) return LT_ERROR;

		bool bNotFirstLoop = false;
		LT_VERTF* pSrcPtr		= pVerts;
		while (nCount) {
			DRAWPRIM_D3DTRANS* pDstPtr = m_VertTransBuf;
			uint32 nDrawCount	= 0;
			int32 nDstBufSize	= CD3DDRAWPRIM_BUFSIZE - 3;
			if (bNotFirstLoop) {					// If it's not the first, need to grab the first vert (already moved back the source pointer by one)...
				pDstPtr->x	    = pVerts->x; pDstPtr->y = pVerts->y; pDstPtr->z = pVerts->z; pDstPtr->rhw = 1.0f;
				pDstPtr->rgba   = RGBA_MAKE(rgba.r,rgba.g,rgba.b,rgba.a);
				++pDstPtr; --nCount; ++nDrawCount; }
			else bNotFirstLoop = true;
			while (nCount && nDstBufSize > 0) {		// Copy the tri to the buffer,..
				pDstPtr->x	    = pSrcPtr->x; pDstPtr->y = pSrcPtr->y; pDstPtr->z = pSrcPtr->z; pDstPtr->rhw = 1.0f;
				pDstPtr->rgba   = RGBA_MAKE(rgba.r,rgba.g,rgba.b,rgba.a);
				++pDstPtr; ++pSrcPtr;
				--nCount; --nDstBufSize; ++nDrawCount; }

			EFFECT_SHADER_MACRO(hRes, pDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, nDrawCount-2, m_VertTransBuf, sizeof(DRAWPRIM_D3DTRANS))); 
			if (hRes != D3D_OK)
				return LT_ERROR; 

			if (nCount) { --pSrcPtr; nCount += 2; } } }				// Skip back one (see notes by bNotFirstLoop)... 

	return LT_OK;
}

// Draw primitive calls using triangle strips
LTRESULT CD3DDrawPrim::DrawPrimStrip(LT_VERTGT *pVerts, uint32 nCount) {
	LPDIRECT3DDEVICE9 pDevice = r_GetRenderStruct()->GetD3DDevice();
	if (!pDevice) return LT_ERROR;
	CAutoDrawPrimBlock AutoDPBlock(this);

	if (m_eTransType != DRAWPRIM_TRANSFORM_SCREEN) {
		HRESULT hRes = pDevice->SetVertexShader(NULL);
		if (hRes != D3D_OK) return LT_ERROR;

		hRes = pDevice->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1);
		if (hRes != D3D_OK) return LT_ERROR;

		EFFECT_SHADER_MACRO(hRes, pDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, nCount-2, pVerts, sizeof(LT_VERTGT)));
		if (hRes != D3D_OK)
			return LT_ERROR;
	}
	else {
		HRESULT hRes = pDevice->SetVertexShader(NULL);
		if (hRes != D3D_OK) return LT_ERROR;

		hRes = pDevice->SetFVF(DRAWPRIM_D3DTRANS_TEX_FLAGS);
		if (hRes != D3D_OK) return LT_ERROR;

		LT_VERTGT* pSrcPtr		= pVerts;
		while (nCount) {
			DRAWPRIM_D3DTRANS_TEX*  pDstPtr = m_VertTransBufT;
			uint32 nDrawCount	= 0;
			int32 nDstBufSize	= CD3DDRAWPRIM_BUFSIZE - 3;
			while (nCount && nDstBufSize > 0) {		// Copy the tri to the buffer,..
				pDstPtr->x	    = pSrcPtr->x; pDstPtr->y = pSrcPtr->y; pDstPtr->z = pSrcPtr->z;
				pDstPtr->rgba   = RGBA_MAKE(pSrcPtr->rgba.r,pSrcPtr->rgba.g,pSrcPtr->rgba.b,pSrcPtr->rgba.a);
				pDstPtr->u	    = pSrcPtr->u; pDstPtr->v = pSrcPtr->v; 
				++pDstPtr; ++pSrcPtr;
				--nCount; --nDstBufSize; ++nDrawCount; }

			EFFECT_SHADER_MACRO(hRes, pDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, nDrawCount-2, m_VertTransBufT, sizeof(DRAWPRIM_D3DTRANS_TEX))); 
			if (hRes != D3D_OK)
				return LT_ERROR; 

			if (nCount) { pSrcPtr -= 2; nCount += 2; } } }	// Skip back two (since it's a strip - need to send those two again)...Need to check the Cull direction (might be backwards).

	return LT_OK;
}

LTRESULT CD3DDrawPrim::DrawPrimStrip(LT_VERTFT *pVerts, uint32 nCount, LT_VERTRGBA rgba) {
	LPDIRECT3DDEVICE9 pDevice = r_GetRenderStruct()->GetD3DDevice();
	if (!pDevice) return LT_ERROR;
	CAutoDrawPrimBlock AutoDPBlock(this);

	if (m_eTransType != DRAWPRIM_TRANSFORM_SCREEN) {
		HRESULT hRes = pDevice->SetVertexShader(NULL);
		if (hRes != D3D_OK) return LT_ERROR;

		hRes = pDevice->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1);
		if (hRes != D3D_OK) return LT_ERROR;

		LT_VERTFT* pSrcPtr		= pVerts;
		while (nCount) {
			LT_VERTGT*  pDstPtr = m_VertBufGT;
			uint32 nDrawCount	= 0;
			int32 nDstBufSize	= CD3DDRAWPRIM_BUFSIZE - 3;
			while (nCount && nDstBufSize > 0) {		// Copy the tri to the buffer,..
				pDstPtr->x	    = pSrcPtr->x; pDstPtr->y = pSrcPtr->y; pDstPtr->z = pSrcPtr->z;
				pDstPtr->rgba   = rgba;
				pDstPtr->u	    = pSrcPtr->u; pDstPtr->v = pSrcPtr->v; 
				++pDstPtr; ++pSrcPtr;
				--nCount; --nDstBufSize; ++nDrawCount; }

			EFFECT_SHADER_MACRO(hRes, pDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, nDrawCount-2, m_VertBufGT, sizeof(LT_VERTGT))); 
			if (hRes != D3D_OK)
				return LT_ERROR; 

			if (nCount) { pSrcPtr -= 2; nCount += 2; } } }	// Skip back two (since it's a strip - need to send those two again)...Need to check the Cull direction (might be backwards).
	else {
		HRESULT hRes = pDevice->SetVertexShader(NULL);
		if (hRes != D3D_OK) return LT_ERROR;

		hRes = pDevice->SetFVF(DRAWPRIM_D3DTRANS_TEX_FLAGS);
		if (hRes != D3D_OK) return LT_ERROR;

		LT_VERTFT* pSrcPtr		= pVerts;
		while (nCount) {
			DRAWPRIM_D3DTRANS_TEX*  pDstPtr = m_VertTransBufT;
			uint32 nDrawCount	= 0;
			int32 nDstBufSize	= CD3DDRAWPRIM_BUFSIZE - 3;
			while (nCount && nDstBufSize > 0) {		// Copy the tri to the buffer,..
				pDstPtr->x	    = pSrcPtr->x; pDstPtr->y = pSrcPtr->y; pDstPtr->z = pSrcPtr->z;
				pDstPtr->rgba   = RGBA_MAKE(rgba.r,rgba.g,rgba.b,rgba.a);
				pDstPtr->u	    = pSrcPtr->u; pDstPtr->v = pSrcPtr->v; 
				++pDstPtr; ++pSrcPtr;
				--nCount; --nDstBufSize; ++nDrawCount; }

			EFFECT_SHADER_MACRO(hRes, pDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, nDrawCount-2, m_VertTransBufT, sizeof(DRAWPRIM_D3DTRANS_TEX))); 
			if (hRes != D3D_OK)
				return LT_ERROR; 

			if (nCount) { pSrcPtr -= 2; nCount += 2; } } }	// Skip back two (since it's a strip - need to send those two again)...Need to check the Cull direction (might be backwards).

	return LT_OK;
}

LTRESULT CD3DDrawPrim::DrawPrimStrip(LT_VERTG *pVerts, uint32 nCount) {
	LPDIRECT3DDEVICE9 pDevice = r_GetRenderStruct()->GetD3DDevice();
	if (!pDevice) return LT_ERROR;
	CAutoDrawPrimBlock AutoDPBlock(this);

	if (m_eTransType != DRAWPRIM_TRANSFORM_SCREEN) {
		HRESULT hRes = pDevice->SetVertexShader(NULL);
		if (hRes != D3D_OK) return LT_ERROR;

		hRes = pDevice->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);
		if (hRes != D3D_OK) return LT_ERROR;

		EFFECT_SHADER_MACRO(hRes, pDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, nCount-2, pVerts, sizeof(LT_VERTG)));
		if (hRes != D3D_OK)
			return LT_ERROR;
	}
	else {
		HRESULT hRes = pDevice->SetVertexShader(NULL);
		if (hRes != D3D_OK) return LT_ERROR;

		hRes = pDevice->SetFVF(DRAWPRIM_D3DTRANS_FLAGS);
		if (hRes != D3D_OK) return LT_ERROR;

		LT_VERTG* pSrcPtr		= pVerts;
		while (nCount) {
			DRAWPRIM_D3DTRANS*  pDstPtr = m_VertTransBuf;
			uint32 nDrawCount	= 0;
			int32 nDstBufSize	= CD3DDRAWPRIM_BUFSIZE - 3;
			while (nCount && nDstBufSize > 0) {		// Copy the tri to the buffer,..
				pDstPtr->x	    = pSrcPtr->x; pDstPtr->y = pSrcPtr->y; pDstPtr->z = pSrcPtr->z;
				pDstPtr->rgba   = RGBA_MAKE(pSrcPtr->rgba.r,pSrcPtr->rgba.g,pSrcPtr->rgba.b,pSrcPtr->rgba.a);
				++pDstPtr; ++pSrcPtr;
				--nCount; --nDstBufSize; ++nDrawCount; }

			EFFECT_SHADER_MACRO(hRes, pDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, nDrawCount-2, m_VertTransBuf, sizeof(DRAWPRIM_D3DTRANS))); 
			if (hRes != D3D_OK)
				return LT_ERROR; 

			if (nCount) { pSrcPtr -= 2; nCount += 2; } } }	// Skip back two (since it's a strip - need to send those two again)...Need to check the Cull direction (might be backwards).

	return LT_OK;
}

LTRESULT CD3DDrawPrim::DrawPrimStrip(LT_VERTF *pVerts, uint32 nCount, LT_VERTRGBA rgba) {
	LPDIRECT3DDEVICE9 pDevice = r_GetRenderStruct()->GetD3DDevice();
	if (!pDevice) return LT_ERROR;
	CAutoDrawPrimBlock AutoDPBlock(this);

	if (m_eTransType != DRAWPRIM_TRANSFORM_SCREEN) {
		HRESULT hRes = pDevice->SetVertexShader(NULL);
		if (hRes != D3D_OK) return LT_ERROR;

		hRes = pDevice->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);
		if (hRes != D3D_OK) return LT_ERROR;

		LT_VERTF* pSrcPtr		= pVerts;
		while (nCount) {
			LT_VERTG*  pDstPtr  = m_VertBufG;
			uint32 nDrawCount	= 0;
			int32 nDstBufSize	= CD3DDRAWPRIM_BUFSIZE - 3;
			while (nCount && nDstBufSize > 0) {		// Copy the tri to the buffer,..
				pDstPtr->x	    = pSrcPtr->x; pDstPtr->y = pSrcPtr->y; pDstPtr->z = pSrcPtr->z;
				pDstPtr->rgba   = rgba;
				++pDstPtr; ++pSrcPtr;
				--nCount; --nDstBufSize; ++nDrawCount; }

			EFFECT_SHADER_MACRO(hRes, pDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, nDrawCount-2, m_VertBufG, sizeof(LT_VERTG))); 
			if (hRes != D3D_OK)
				return LT_ERROR; 

			if (nCount) { pSrcPtr -= 2; nCount += 2; } } } 	// Skip back two (since it's a strip - need to send those two again)...Need to check the Cull direction (might be backwards).
	else {
		HRESULT hRes = pDevice->SetVertexShader(NULL);
		if (hRes != D3D_OK) return LT_ERROR;

		hRes = pDevice->SetFVF(DRAWPRIM_D3DTRANS_FLAGS);
		if (hRes != D3D_OK) return LT_ERROR;

		LT_VERTF* pSrcPtr		= pVerts;
		while (nCount) {
			DRAWPRIM_D3DTRANS*  pDstPtr = m_VertTransBuf;
			uint32 nDrawCount	= 0;
			int32 nDstBufSize	= CD3DDRAWPRIM_BUFSIZE - 3;
			while (nCount && nDstBufSize > 0) {		// Copy the tri to the buffer,..
				pDstPtr->x	    = pSrcPtr->x; pDstPtr->y = pSrcPtr->y; pDstPtr->z = pSrcPtr->z;
				pDstPtr->rgba   = RGBA_MAKE(rgba.r,rgba.g,rgba.b,rgba.a);
				++pDstPtr; ++pSrcPtr;
				--nCount; --nDstBufSize; ++nDrawCount; }

			EFFECT_SHADER_MACRO(hRes, pDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, nDrawCount-2, m_VertTransBuf, sizeof(DRAWPRIM_D3DTRANS))); 
			if (hRes != D3D_OK)
				return LT_ERROR; 

			if (nCount) { pSrcPtr -= 2; nCount += 2; } } }	// Skip back two (since it's a strip - need to send those two again)...Need to check the Cull direction (might be backwards).

	return LT_OK;
}
