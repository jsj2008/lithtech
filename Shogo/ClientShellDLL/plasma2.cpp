
#include "plasma.h"


char g_SinTable[256];
uint8 *g_DistanceGrid;

int g_PlasmaSizeX = 20;
int g_PlasmaSizeY = 20;


void cs_PrecalculateData(ILTClient *pClientDE, HLOCALOBJ hPolyGrid)
{
	uint32 width, height;
	int halfWidth, halfHeight, x, y;
	char *pData;
	PGColor *pColorTable;
	LTFLOAT scale, val, maxDistSqr, testDist, t;
	int i;


	// Create the (scaled) sin table.
	scale = (MATH_CIRCLE / 255.0f) * 3.0f;
	for(i=0; i < 256; i++)
	{
		val = (LTFLOAT)i * scale;
		g_SinTable[i] = (char)(sin(val) * 128.0f);
	}


	// Fill in the distance grid.
	pClientDE->GetPolyGridInfo(hPolyGrid, &pData, &width, &height, &pColorTable);


	g_DistanceGrid = (uint8*)malloc(width*height);
	halfWidth = width >> 1;
	halfHeight = height >> 1;
	maxDistSqr = (LTFLOAT)(halfWidth*halfWidth + halfHeight*halfHeight) + 1.0f;
	for(y=0; y < (int)height; y++)
	{
		for(x=0; x < (int)width; x++)
		{
			testDist = (LTFLOAT)((x-halfWidth)*(x-halfWidth) + (y-halfHeight)*(y-halfHeight));
			t = testDist / maxDistSqr;
			g_DistanceGrid[y*height+x] = (uint8)(t * 255.0f);
		}
	}
}

LTFLOAT g_fCount=0.0f, g_Dir;

void cs_RandomizePolyGrid(ILTClient *pClientDE, HLOCALOBJ hPolyGrid)
{
	uint32 width, height, x, y;
	char *pData, *pCur;
	PGColor *pColorTable;
	uint8 count;

	g_fCount += 50.0f * g_Dir * pClientDE->GetFrameTime();
	if(g_fCount >= 254.0f)
		g_Dir = -1.0f;
	else if(g_fCount <= 1.0f)
		g_Dir = 1.0f;

	count = (uint8)g_fCount;

	// Randomize the poly grid values.
	pClientDE->GetPolyGridInfo(hPolyGrid, &pData, &width, &height, &pColorTable);
	for(y=0; y < height; y++)
	{
		pCur = pData + (y*width);

		for(x=0; x < width; x++)
		{
			*pCur++ = g_SinTable[g_DistanceGrid[y*width+x]+count];
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
	LTVector(0.0f, 0.0f, 255.0f), 128,
	LTVector(255.0f, 255.0f, 255.0f), 256,
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


