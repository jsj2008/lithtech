//------------------------------------------------------------------
//
//   MODULE  : CLIENTFX.H
//
//   PURPOSE : Defines client fx stuff
//
//   CREATED : On 10/6/98 At 9:21:50 PM
//
//------------------------------------------------------------------

#ifndef __CLIENTFX_H_
	#define __CLIENTFX_H_

	// Includes....

	#include "basefx.h"
	#include "debugnew.h"
	#include "iltdrawprim.h"
	#include "iltsoundmgr.h"

	// Defines....

	#define NUM_FX								17
	
	extern "C"
	{

		__declspec(dllexport) int				fxGetNum();
		__declspec(dllexport) FX_REF			fxGetRef(int nFx);
	
		__declspec(dllexport) void				fxDelete(CBaseFX *pDeleteFX);

		__declspec(dllexport) void				fxSetPlayer(HOBJECT hPlayer);

		__declspec(dllexport) void				fxSetAppFocus(bool bAppFocus);

		__declspec(dllexport) void				fxSetCreateFunction(TCreateClientFXFn pFn, void* pUserData);

		__declspec(dllexport) CBaseFXProps*		fxCreatePropList(int nFx);

		__declspec(dllexport) void				fxFreePropList(CBaseFXProps* pPropList);
	}

	// FX Creation functions

	void									AddBaseProps(CFastList<FX_PROP> *pList);

	// Externs....

	#define MAX_BUFFER_TRIS						256
	#define MAX_BUFFER_VERTS					(MAX_BUFFER_TRIS * 3)
	
	extern LT_POLYGT3 g_pTris[MAX_BUFFER_TRIS];
	extern LTVector g_pVerts[MAX_BUFFER_VERTS];

	extern LTBOOL g_bAppFocus;

	// Helper functions

	inline void SetupVert(LT_POLYGT3 *pPoly, int nIndex, const LTVector& vVert, uint8 r, uint8 g, uint8 b, uint8 a, float u, float v)
	{
		pPoly->verts[nIndex].x = vVert.x;
		pPoly->verts[nIndex].y = vVert.y;
		pPoly->verts[nIndex].z = vVert.z;
		pPoly->verts[nIndex].rgba.r = r;
		pPoly->verts[nIndex].rgba.g = g;
		pPoly->verts[nIndex].rgba.b = b;
		pPoly->verts[nIndex].rgba.a = a;
		pPoly->verts[nIndex].u = u;
		pPoly->verts[nIndex].v = v;
	}

	inline LTMatrix GetCamTransform(ILTClient *pClientDE, HOBJECT hCamera)
	{
		LTVector vPos;
		LTVector vRight, vUp, vForward;
		LTRotation orient;
	
		pClientDE->GetObjectPos(hCamera, &vPos);
		pClientDE->GetObjectRotation(hCamera, &orient);
		
		vPos.x = -vPos.x;
		vPos.y = -vPos.y;
		vPos.z = -vPos.z;

		LTMatrix mTran, mRot, mFull;
		
		Mat_SetBasisVectors(&mRot, &orient.Right(), &orient.Up(), &orient.Forward());
		MatTranspose3x3(&mRot);

		Mat_Identity(&mTran);
		mTran.m[0][3] = vPos.x;
		mTran.m[1][3] = vPos.y;
		mTran.m[2][3] = vPos.z;

		MatMul(&mFull, &mRot, &mTran);

		return mFull;	
	}


	inline LTFLOAT GetRandom( LTFLOAT min, LTFLOAT max )
	{
		LTFLOAT randNum = (LTFLOAT)rand() / RAND_MAX;
		return (min + (max - min ) * randNum);
	}

	inline int GetRandom( int min, int max )
	{
		if( (max - min + 1) == 0)		// check for divide-by-zero case
		{
			if( rand() & 1 ) return( min );
			else return( max );
		}

		return( (rand() % (max - min + 1)) + min );
	}

	
	inline void SetupRotationAroundPoint(LTMatrix &mMat, const LTRotation &rRot, const LTVector &vPoint)
	{
		LTMatrix mForward, mRotate, mBackward;

		mForward.Identity();
		mRotate.Identity();
		mBackward.Identity();

		mForward.SetTranslation( vPoint );
		mBackward.SetTranslation( -vPoint );
		
		rRot.ConvertToMatrix( mRotate );

		mMat = mForward * mRotate * mBackward;	
	}

	inline void SetupRotation(LTMatrix &mMat, const LTRotation &rRot)
	{
		rRot.ConvertToMatrix( mMat );
	}

	inline void FindPerps(const LTVector vPlaneDir, LTVector& vPerp1, LTVector& vPerp2)
	{
		// Get coplanar perp vector to normalized direction
		if( (1.0f == vPlaneDir.y) || (-1.0f == vPlaneDir.y) )
		{
			vPerp1 = LTVector( 1.0f, 0.0f, 0.0f ).Cross( vPlaneDir );
		}
		else
		{	
			vPerp1 = LTVector( 0.0f, 1.0f, 0.0f ).Cross( vPlaneDir );
		}	

		// Get coplanar perp vector to initial perp
		vPerp2 = vPerp1.Cross( vPlaneDir );
	}

	bool CreateNewFX(const CLIENTFX_CREATESTRUCT& CreateInfo, bool bStartInst);


#endif