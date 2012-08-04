

#include "plasma.h"


int g_PlasmaSizeX = 16;
int g_PlasmaSizeY = 16;


void cs_RandomizePolyGrid(ILTClient *pClientDE, HLOCALOBJ hPolyGrid)
{
}



void cs_PrecalculateData(ILTClient *pClientDE, HLOCALOBJ hPolyGrid)
{
	uint32 width, height, x, y;
	char *pData, *pCur;
	PGColor *pColorTable;


	// Randomize the poly grid values.
	pClientDE->GetPolyGridInfo(hPolyGrid, &pData, &width, &height, &pColorTable);
	for(y=0; y < height; y++)
	{
		pCur = pData + (y*width);

		for(x=0; x < width; x++)
		{
			*pCur++ = (char)rand();
		}
	}
}


typedef struct
{
	LTVector m_Color;
	int m_Index;
} ColorRamp;


ColorRamp g_ColorRamps[] = 
{
	LTVector(0.0f, 0.0f, 0.0f), 0,
	LTVector(255.0f, 255.0f, 255.0f), 256
};

#define NUM_COLORRAMPS() (sizeof(g_ColorRamps) / sizeof(ColorRamp))


void cs_SetPolyGridPalette(ILTClient *pClientDE, HLOCALOBJ hPolyGrid)
{
	LTFLOAT t;
	uint32 width, height;
	char *pData;
	PGColor *pColorTable, color1, color2;
	int ramp, i, index1, index2;

	// Randomize the poly grid values.
	pClientDE->GetPolyGridInfo(hPolyGrid, &pData, &width, &height, &pColorTable);

	// Make the color table go from white to black.
	for(ramp=0; ramp < NUM_COLORRAMPS()-1; ramp++)
	{
		index1 = g_ColorRamps[ramp].m_Index;
		index2 = g_ColorRamps[ramp+1].m_Index;
		VEC_COPY(color1, g_ColorRamps[ramp].m_Color);
		VEC_COPY(color2, g_ColorRamps[ramp+1].m_Color);

		for(i=index1; i < index2; i++)
		{
			t = (LTFLOAT)(i - index1) / (index2 - index1);
			VEC_LERP(pColorTable[i], color1, color2, t);
		}
	}
}


