//------------------------------------------------------------------
//
//  FILE      : LTLTShaderDeviceStateImpImp.cpp
//
//  PURPOSE   :	Shader device state implementation. This class is
//              used by the vertex and pixel shader callback functions
//              in IClientShell to give the client access to device
//              states such as world matrices, camera settings, and
//              light settings.
//
//  COPYRIGHT : LithTech Inc., 1996-2003
//
//------------------------------------------------------------------

#include "precompile.h"
#include "LTShaderDeviceStateImp.h"
#include "d3d_device.h"
#include "common_draw.h"
#include "renderstruct.h"



//-------------------------------------------------------------------------------------------------
// LTShaderDeviceStateImp
//-------------------------------------------------------------------------------------------------


LTShaderDeviceStateImp& LTShaderDeviceStateImp::GetSingleton()
{
	static LTShaderDeviceStateImp s_Singleton;
	return s_Singleton;
}



bool LTShaderDeviceStateImp::GetMatrix(LTMatrixType MatrixType, bool Transpose, LTMatrix *pMatrix) const
{
	// Make sure there is a valid matrix.
	if (NULL == pMatrix)
	{
		return false;
	}

	// Get the desired matrix from the device.
	D3DXMATRIX Mat;
	switch (MatrixType)
	{
		case MATRIXTYPE_WORLD0:
			PD3DDEVICE->GetTransform(D3DTS_WORLD, &Mat);
			break;
		case MATRIXTYPE_WORLD1:
			PD3DDEVICE->GetTransform(D3DTS_WORLD1, &Mat);
			break;
		case MATRIXTYPE_WORLD2:
			PD3DDEVICE->GetTransform(D3DTS_WORLD2, &Mat);
			break;
		case MATRIXTYPE_WORLD3:
			PD3DDEVICE->GetTransform(D3DTS_WORLD3, &Mat);
			break;
		case MATRIXTYPE_VIEW:
			PD3DDEVICE->GetTransform(D3DTS_VIEW, &Mat);
			break;
		case MATRIXTYPE_PROJECTION:
			PD3DDEVICE->GetTransform(D3DTS_PROJECTION, &Mat);
			break;
		case MATRIXTYPE_VIEWPROJECTION:
		{
			//
			// This is a special case in which we need two matrices.
			//

			D3DXMATRIX MatView;
			PD3DDEVICE->GetTransform(D3DTS_VIEW, &MatView);

			D3DXMATRIX MatProj;
			PD3DDEVICE->GetTransform(D3DTS_PROJECTION, &MatProj);

			D3DXMATRIX MatViewProj;
    		D3DXMatrixMultiply(&MatViewProj, &MatView, &MatProj);

			if (Transpose)
			{
				D3DXMatrixTranspose(&MatViewProj, &MatViewProj);
			}

    		pMatrix->Init(MatViewProj._11, MatViewProj._12, MatViewProj._13, MatViewProj._14,
    			          MatViewProj._21, MatViewProj._22, MatViewProj._23, MatViewProj._24,
    			          MatViewProj._31, MatViewProj._32, MatViewProj._33, MatViewProj._34,
						  MatViewProj._41, MatViewProj._42, MatViewProj._43, MatViewProj._44);

			// all done
			return true;
		}
		case MATRIXTYPE_TEXTURE0:
			PD3DDEVICE->GetTransform(D3DTS_TEXTURE0, &Mat);
			break;
		case MATRIXTYPE_TEXTURE1:
			PD3DDEVICE->GetTransform(D3DTS_TEXTURE1, &Mat);
			break;
		case MATRIXTYPE_TEXTURE2:
			PD3DDEVICE->GetTransform(D3DTS_TEXTURE2, &Mat);
			break;
		case MATRIXTYPE_TEXTURE3:
			PD3DDEVICE->GetTransform(D3DTS_TEXTURE3, &Mat);
			break;
		case MATRIXTYPE_TEXTURE4:
			PD3DDEVICE->GetTransform(D3DTS_TEXTURE4, &Mat);
			break;
		case MATRIXTYPE_TEXTURE5:
			PD3DDEVICE->GetTransform(D3DTS_TEXTURE5, &Mat);
			break;
		case MATRIXTYPE_TEXTURE6:
			PD3DDEVICE->GetTransform(D3DTS_TEXTURE6, &Mat);
			break;
		case MATRIXTYPE_TEXTURE7:
			PD3DDEVICE->GetTransform(D3DTS_TEXTURE7, &Mat);
			break;
		default:
			// Invalid option
			return false;
	}

	// Transpose the matrix.
	if (Transpose)
	{
    	D3DXMatrixTranspose(&Mat, &Mat);
	}

    pMatrix->Init(Mat._11, Mat._12, Mat._13, Mat._14,
    		      Mat._21, Mat._22, Mat._23, Mat._24,
    		      Mat._31, Mat._32, Mat._33, Mat._34,
				  Mat._41, Mat._42, Mat._43, Mat._44);

	return true;
}



bool LTShaderDeviceStateImp::GetLight(unsigned LightIndex, LTLightDesc *pLightDesc) const
{
	// Make sure there is a valid light.
	if (NULL == pLightDesc)
	{
		return false;
	}

	// Validate the index.
	if (LightIndex > 7)
	{
		return false;
	}

	// Get the device light.
  	D3DLIGHT9 d3dLight;
	HRESULT hr = PD3DDEVICE->GetLight(LightIndex, &d3dLight);
	if (FAILED(hr))
	{
		return false;
	}

	// See if this light is active.
  	BOOL bEnable = FALSE;
	hr = PD3DDEVICE->GetLightEnable(LightIndex, &bEnable);
	if (FAILED(hr))
	{
		return false;
	}

	// type
	switch (d3dLight.Type)
	{
		case D3DLIGHT_POINT:
			pLightDesc->m_LightType = LIGHTTYPE_POINT;
			break;
		case D3DLIGHT_SPOT:
			pLightDesc->m_LightType = LIGHTTYPE_SPOT;
			break;
		case D3DLIGHT_DIRECTIONAL:
			pLightDesc->m_LightType = LIGHTTYPE_DIRECTIONAL;
			break;
		default:
			// Invalid light type.
			return false;
	}

	// active
	pLightDesc->m_Active 		= (bEnable == TRUE);

	// diffuse
	pLightDesc->m_Diffuse.r 	= static_cast<uint8>(d3dLight.Diffuse.r * 255.0f);
	pLightDesc->m_Diffuse.g 	= static_cast<uint8>(d3dLight.Diffuse.g * 255.0f);
	pLightDesc->m_Diffuse.b 	= static_cast<uint8>(d3dLight.Diffuse.b * 255.0f);
	pLightDesc->m_Diffuse.a 	= static_cast<uint8>(d3dLight.Diffuse.a * 255.0f);

	// specular
	pLightDesc->m_Specular.r	 = static_cast<uint8>(d3dLight.Specular.r * 255.0f);
	pLightDesc->m_Specular.g	 = static_cast<uint8>(d3dLight.Specular.g * 255.0f);
	pLightDesc->m_Specular.b	 = static_cast<uint8>(d3dLight.Specular.b * 255.0f);
	pLightDesc->m_Specular.a	 = static_cast<uint8>(d3dLight.Specular.a * 255.0f);

	// ambient
	pLightDesc->m_Ambient.r 	= static_cast<uint8>(d3dLight.Ambient.r * 255.0f);
	pLightDesc->m_Ambient.g 	= static_cast<uint8>(d3dLight.Ambient.g * 255.0f);
	pLightDesc->m_Ambient.b 	= static_cast<uint8>(d3dLight.Ambient.b * 255.0f);
	pLightDesc->m_Ambient.a 	= static_cast<uint8>(d3dLight.Ambient.a * 255.0f);

	// position
	pLightDesc->m_Position.x 	= d3dLight.Position.x;
	pLightDesc->m_Position.y 	= d3dLight.Position.y;
	pLightDesc->m_Position.z 	= d3dLight.Position.z;

	// direction
	pLightDesc->m_Direction.x 	= d3dLight.Direction.x;
	pLightDesc->m_Direction.y 	= d3dLight.Direction.y;
	pLightDesc->m_Direction.z 	= d3dLight.Direction.z;

	// range
	pLightDesc->m_Range 		= d3dLight.Range;

	// falloff
	pLightDesc->m_Falloff 		= d3dLight.Falloff;

	// atennuation
	pLightDesc->m_Atten0 		= d3dLight.Attenuation0;
	pLightDesc->m_Atten1 		= d3dLight.Attenuation1;
	pLightDesc->m_Atten2 		= d3dLight.Attenuation2;

	// theta
	pLightDesc->m_Theta 		= d3dLight.Theta;

	// phi
	pLightDesc->m_Phi 			= d3dLight.Phi;

	return true;
}



bool LTShaderDeviceStateImp::GetCamera(LTCameraDesc *pCameraDesc) const
{
	// Make sure there is a valid camera.
	if (NULL == pCameraDesc)
	{
		return false;
	}

	// position
	pCameraDesc->m_Position		= g_pSceneDesc->m_Pos;

	// rotation
	pCameraDesc->m_Rotation		= g_pSceneDesc->m_Rotation;

	// field of view
	pCameraDesc->m_xFov 		= g_pSceneDesc->m_xFov;
	pCameraDesc->m_yFov			= g_pSceneDesc->m_yFov;

	return true;
}

