// ----------------------------------------------------------------------- //
//
// MODULE  : LTQuadUVUtils.h
//
// PURPOSE : Utility functions for setting up and manipulating UV coordinates
//			 on quads. This helps with bilinear filtering.
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __LTQUADUVUTILS_H__
#define __LTQUADUVUTILS_H__


// Utility function that will take a texture, a polygon, and the dimensions needed
//and setup the UV coordinates of the polygon so that it will work properly with
//bilinear filtering on the specified texture
template <class T>
void SetupQuadUVs(T& Poly, HTEXTURE hTexture, float fX, float fY, float fWidth, float fHeight)
{
	//first off we need to get the dimensions of a pixel in texture space
	float fHalfPixelWidth  = 0.0f;
	float fHalfPixelHeight = 0.0f;

	uint32 nTexWidth, nTexHeight;
	if(g_pILTTextureMgr->GetTextureDims(hTexture, nTexWidth, nTexHeight) == LT_OK)
	{
		fHalfPixelWidth		= 0.5f / nTexWidth;
		fHalfPixelHeight	= 0.5f / nTexHeight;

		//handle adjustments for negative values
		if(fWidth < 0.0f)
			fHalfPixelWidth = -fHalfPixelWidth;
		if(fHeight < 0.0f)
			fHalfPixelHeight = -fHalfPixelHeight;
	}

	DrawPrimSetUV(Poly, 0, fX + fHalfPixelWidth, fY + fHalfPixelHeight);
	DrawPrimSetUV(Poly, 1, fX + fWidth - fHalfPixelWidth, fY + fHalfPixelHeight);
	DrawPrimSetUV(Poly, 2, fX + fWidth - fHalfPixelWidth, fY + fHeight - fHalfPixelHeight);
	DrawPrimSetUV(Poly, 3, fX + fHalfPixelWidth, fY + fHeight - fHalfPixelHeight);
}

//this will rotate the UVs in a quad clockwise in 90 degree increments
template <class T>
void RotateQuadUVs(T& Poly, uint32 n90DegIncs)
{
	//make sure it is within the range of 0..3
	n90DegIncs %= 4;

	//bail if no rotation
	if(n90DegIncs == 0)
		return;

	//alright, save the old values
	LTVector2 vUV[4];

	uint32 nCurrVert;
	for(nCurrVert = 0; nCurrVert < 4; nCurrVert++)
	{
		vUV[nCurrVert] = Poly.verts[nCurrVert].uv;
	}

	//now handle the offsetting
	for(nCurrVert = 0; nCurrVert < 4; nCurrVert++)
	{
		DrawPrimSetUV(Poly, nCurrVert, VEC2_EXPAND(vUV[(nCurrVert + n90DegIncs) % 4]));
	}	
}

#endif
