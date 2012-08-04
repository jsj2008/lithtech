#ifndef __LIGHTMAPDATA_H__
#define __LIGHTMAPDATA_H__

#include "ltinteger.h"

class CLightMapData
{
public:

	CLightMapData();
	~CLightMapData();

	//sets the size of the lightmap data
	bool	SetSize(uint8 nWidth, uint8 nHeight);

	//clears the entire lightmap image with black
	void	Clear();

	//gets the dimensions of the lightmap
	uint8	GetWidth() const			{ return m_nWidth;  }
	uint8	GetHeight() const			{ return m_nHeight; }

	//gets the actual image data
	uint8*	GetImage()					{ return m_pImage; }	

private:

	//the dimensions of this lightmap data
	uint8		m_nWidth;
	uint8		m_nHeight;

	//the image data
	uint8*		m_pImage;
};

#endif



