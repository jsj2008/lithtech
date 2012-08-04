#include "LightMapData.h"
#include <stdlib.h>
#include <string.h>

CLightMapData::CLightMapData() :
	m_nWidth(0),
	m_nHeight(0),
	m_pImage(NULL)
{
}

CLightMapData::~CLightMapData()
{
	delete [] m_pImage;
}

//sets the size of the lightmap data
bool CLightMapData::SetSize(uint8 nWidth, uint8 nHeight)
{
	//clear out any old data
	delete [] m_pImage;
	m_pImage = NULL;

	m_nWidth = m_nHeight = 0;

	//now allocate the new data
	m_pImage = new uint8 [nWidth * nHeight * 3];

	//make sure it worked
	if(m_pImage)
	{
		//save the dims
		m_nWidth  = nWidth;
		m_nHeight = nHeight;

		return true;
	}

	//out of memory
	return false;
}

//clears the entire lightmap image with black
void CLightMapData::Clear()
{
	//make sure we have an image
	if(GetImage())
	{
		//blit 0 over it
		memset(GetImage(), 0, GetWidth() * GetHeight() * 3);
	}
}
