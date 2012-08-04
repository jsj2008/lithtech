//------------------------------------------------------------------
//
//  FILE      : LTShaderDeviceStateImp.h
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

#ifndef __LTSHADERDEVICESTATEIMP_H__
#define __LTSHADERDEVICESTATEIMP_H__


#include "ltbasedefs.h"



// Shader device state (used in IClientShell shader callback functions)
class LTShaderDeviceStateImp : public LTShaderDeviceState
{
public:

	~LTShaderDeviceStateImp()
	{
	}

	// singleton access
	static LTShaderDeviceStateImp&	GetSingleton();

	// get a matrix from the device; set Transpose to true to have the matrix transposed
	virtual bool			GetMatrix(LTMatrixType MatrixType, bool Transpose, LTMatrix *pMatrix) const;

	// get a light from the device; LightIndex is in the range [0,7]
	virtual bool			GetLight(unsigned LightIndex, LTLightDesc *pLightDesc) const;

	// get the camera from the device
	virtual  bool			GetCamera(LTCameraDesc *pCameraDesc) const;

protected:

	LTShaderDeviceStateImp()
	{
	}

};



#endif // __LTSHADERDEVICESTATEIMP_H__
