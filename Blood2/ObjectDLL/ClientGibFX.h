// ----------------------------------------------------------------------- //
//
// MODULE  : ClientGibFX.h
//
// PURPOSE : ClientGibFX - Definition
//
// CREATED : 8/3/98
//
// ----------------------------------------------------------------------- //

#ifndef __CLIENTGIBFX_H__
#define __CLIENTGIBFX_H__

#include "ClientSFX.h"
#define TYPE_STONE			0x00000001
#define TYPE_METAL			0x00000002
#define TYPE_WOOD			0x00000003
#define TYPE_GLASS			0x00000004
#define TYPE_FLESH			0x00000005
#define SIZE_LARGE			0x00000100
#define SIZE_SMALL			0x00000200
#define TRAIL_BLOOD			0x00001000
#define TRAIL_SMOKE			0x00002000
#define TYPEFLAG_CUSTOM		0x00004000

/*
class CClientGibFX : public CClientSFX
{
	public :
		
//		void Setup(DVector *pvPos, DVector *pvDir, DVector* pvMinVel, DVector* pvMaxVel, DDWORD dwFlags, 
//				   HSTRING m_hstrModel, HSTRING m_hstrSkin);
		void Setup(DVector *pvPos, DVector *pvDir, DVector *pvDims, DDWORD dwFlags, DFLOAT fScale, DBYTE nCount, HSTRING hstrModel=DNULL, HSTRING hstrTexture=DNULL, HSTRING hstrSound=DNULL);

	protected :

		DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData);
	
};

*/

void SetupClientGibFX( DVector *pvPos, DVector *pvDir, DVector *pvDims, DDWORD dwFlags, DFLOAT fScale, DBYTE nCount, HSTRING hstrModel=DNULL, HSTRING hstrTexture=DNULL, HSTRING hstrSound=DNULL );

#endif // __CLIENTGIBFX_H__
