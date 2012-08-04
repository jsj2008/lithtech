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
	if(g_pTexInterface->GetTextureDims(hTexture, nTexWidth, nTexHeight) == LT_OK)
	{
		fHalfPixelWidth		= 0.5f / nTexWidth;
		fHalfPixelHeight	= 0.5f / nTexHeight;

		//handle adjustments for negative values
		if(fWidth < 0.0f)
			fHalfPixelWidth = -fHalfPixelWidth;
		if(fHeight < 0.0f)
			fHalfPixelHeight = -fHalfPixelHeight;
	}

	Poly.verts[0].u = fX + fHalfPixelWidth;
	Poly.verts[0].v = fY + fHalfPixelHeight;
	Poly.verts[1].u = fX + fWidth - fHalfPixelWidth;
	Poly.verts[1].v = fY + fHalfPixelHeight;
	Poly.verts[2].u = fX + fWidth - fHalfPixelWidth;
	Poly.verts[2].v = fY + fHeight - fHalfPixelHeight;
	Poly.verts[3].u = fX + fHalfPixelWidth;
	Poly.verts[3].v = fY + fHeight - fHalfPixelHeight;
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
	float fU[4];
	float fV[4];

	uint32 nCurrVert;
	for(nCurrVert = 0; nCurrVert < 4; nCurrVert++)
	{
		fU[nCurrVert] = Poly.verts[nCurrVert].u;
		fV[nCurrVert] = Poly.verts[nCurrVert].v;
	}

	//now handle the offsetting
	for(nCurrVert = 0; nCurrVert < 4; nCurrVert++)
	{
		Poly.verts[nCurrVert].u = fU[(nCurrVert + n90DegIncs) % 4];
		Poly.verts[nCurrVert].v = fV[(nCurrVert + n90DegIncs) % 4];
	}	
}

#endif
