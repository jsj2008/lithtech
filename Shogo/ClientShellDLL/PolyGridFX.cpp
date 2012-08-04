// ----------------------------------------------------------------------- //
//
// MODULE  : PolyGridFX.cpp
//
// PURPOSE : PolyGrid special FX - Implementation
//
// CREATED : 10/13/97
//
// ----------------------------------------------------------------------- //

#include "PolyGridFX.h"
#include "Plasma.h"
#include "clientheaders.h"
#include "ClientServerShared.h"
#include "RiotClientShell.h"
#include "RiotSettings.h"
#include "ltobjectcreate.h"


static LTFLOAT s_PolyGridYaw		= 0.0f;
static LTFLOAT s_PolyGridPitch	= 0.0f;
static LTFLOAT s_PolyGridRoll	= 0.0f;

static char g_SinTable[256];
static LTBOOL g_bSinTableInitted=LTFALSE;

extern CRiotClientShell* g_pRiotClientShell;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPolyGridFX::Init
//
//	PURPOSE:	Create the poly grid
//
// ----------------------------------------------------------------------- //

LTBOOL CPolyGridFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!psfxCreateStruct) return LTFALSE;

	CSpecialFX::Init(psfxCreateStruct);

	PGCREATESTRUCT* pg = (PGCREATESTRUCT*)psfxCreateStruct;

	VEC_COPY(m_vDims, pg->vDims);
	VEC_COPY(m_vColor1, pg->vColor1);
	VEC_COPY(m_vColor2, pg->vColor2);
	m_fXScaleMin		= pg->fXScaleMin; 
	m_fXScaleMax		= pg->fXScaleMax; 
	m_fYScaleMin		= pg->fYScaleMin; 
	m_fYScaleMax		= pg->fYScaleMax; 
	m_fXScaleDuration	= pg->fXScaleDuration;
	m_fYScaleDuration	= pg->fYScaleDuration;
	m_fXPan				= pg->fXPan;
	m_fYPan				= pg->fYPan;
	m_hstrSurfaceSprite = pg->hstrSurfaceSprite;
	m_dwNumPolies		= pg->dwNumPolies;

	m_fAlpha			= pg->fAlpha;
	m_nPlasmaType		= pg->nPlasmaType;

	m_nRingRate[0]		= pg->nRingRate[0];
	m_nRingRate[1]		= pg->nRingRate[1];
	m_nRingRate[2]		= pg->nRingRate[2];
	m_nRingRate[3]		= pg->nRingRate[3];

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPolyGridFX::CreateObject
//
//	PURPOSE:	Create object associated the poly grid
//
// ----------------------------------------------------------------------- //

LTBOOL CPolyGridFX::CreateObject(ILTClient *pClientDE)
{
	ColorRamp colorRamps[] = 
	{
		ColorRamp(LTVector(m_vColor1.x, m_vColor1.y, m_vColor1.z), 0),
		ColorRamp(LTVector(m_vColor2.x, m_vColor2.y, m_vColor2.z), 256)
	};

	if (!CSpecialFX::CreateObject(pClientDE)) return LTFALSE;

	if (!g_pRiotClientShell) return LTFALSE;

	CRiotSettings* pSettings = g_pRiotClientShell->GetSettings();
	if (!pSettings) return LTFALSE;

	LTVector vPos;
	pClientDE->GetObjectPos(m_hServerObject, &vPos);

	LTRotation rRot;
	pClientDE->GetObjectRotation(m_hServerObject, &rRot);

	// Setup the PolyGrid...

	ObjectCreateStruct createStruct;
	INIT_OBJECTCREATESTRUCT(createStruct);

	createStruct.m_ObjectType = OT_POLYGRID;
	createStruct.m_Flags = FLAG_VISIBLE;
	VEC_COPY(createStruct.m_Pos, vPos);
	createStruct.m_Rotation = rRot;

	uint32 dwSize = 2;
	if (m_dwNumPolies > 4)
	{
		dwSize = (uint32)sqrt((LTFLOAT)(m_dwNumPolies/2)) + 1;

		if (m_nPlasmaType == PLASMA_FOUR_RING)
		{
			// Make sure dwSize is a factor of 2 if using four ring plasma...

			if (2 <= dwSize && dwSize < 4) dwSize = 2;
			else if (4 <= dwSize && dwSize <= 6) dwSize = 4;
			else if (6 < dwSize && dwSize <= 12) dwSize = 8;
			else dwSize = 16;
		}
	}

	
	// Adjust the size of the polygrid based on the current polygrids and
	// special fx detail setting...

	if (m_bUseGlobalSettings)
	{
		uint8 nVal = pSettings->SpecialFXSetting();
		LTBOOL bOn  = pSettings->PolyGrids();

		if (!bOn || nVal == RS_LOW)
		{
			dwSize = 4;		// Smallest possible
		}
		else if (bOn && nVal == RS_MED)
		{
			dwSize /= 2;	// Half normal size
		}
	}


	m_hObject = m_pClientDE->CreateObject(&createStruct);
	m_pClientDE->SetupPolyGrid(m_hObject, dwSize, dwSize, LTFALSE);
	
	// Set alpha value...

	LTFLOAT r, g, b, a;
	m_pClientDE->GetObjectColor(m_hObject, &r, &g, &b, &a);
	m_pClientDE->SetObjectColor(m_hObject, r, g, b, m_fAlpha);


	LTVector vMin, vMax, vScale;
	VEC_SUB(vMin, vPos, m_vDims);
	VEC_ADD(vMax, vPos, m_vDims);

	m_pClientDE->FitPolyGrid(m_hObject, &vMin, &vMax, &vPos, &vScale);

	if (m_hstrSurfaceSprite)
	{
		char* pSpriteName = const_cast<char *>(m_pClientDE->GetStringData(m_hstrSurfaceSprite));
		if (pSpriteName && pSpriteName[0])
		{
			m_pClientDE->SetPolyGridTexture(m_hObject, pSpriteName);
			m_pClientDE->SetPolyGridTextureInfo(m_hObject, m_fXPan, m_fYPan, m_fXScaleMin, m_fYScaleMin);
		}
	}

	m_pClientDE->SetObjectPos(m_hObject, &vPos);
	m_pClientDE->SetObjectScale(m_hObject, &vScale);

	SetPolyGridPalette(colorRamps, sizeof(colorRamps)/sizeof(colorRamps[0]));
	PrecalculatePlasma();

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPolyGridFX::Update
//
//	PURPOSE:	Update the grid
//
// ----------------------------------------------------------------------- //

LTBOOL CPolyGridFX::Update()
{
	if(!m_hObject || !m_pClientDE || !m_hServerObject) return LTFALSE;


	if(m_bWantRemove)
		return LTFALSE;


	// Set the flags of the polygrid based on the the server object...

	uint32 dwUserFlags;
	m_pClientDE->Common()->GetObjectFlags(m_hServerObject, OFT_User, dwUserFlags);

	uint32 dwFlags;
	m_pClientDE->Common()->GetObjectFlags(m_hObject, OFT_Flags, dwFlags);

	if (dwUserFlags & USRFLG_VISIBLE)
	{
		dwFlags |= FLAG_VISIBLE;
		m_pClientDE->Common()->SetObjectFlags(m_hObject, OFT_Flags, dwFlags, FLAGMASK_ALL);
	}
	else  // We're hidden, no need to update...
	{
		dwFlags &= ~FLAG_VISIBLE;
		m_pClientDE->Common()->SetObjectFlags(m_hObject, OFT_Flags, dwFlags, FLAGMASK_ALL);
		return LTTRUE;
	}

	
	// Don't update if not drawn :)

	if (!(dwFlags & FLAG_WASDRAWN) && !m_bAlwaysUpdate) 
	{
		return LTTRUE;
	}

	
	// Update the position of the polygrid to reflect the position of the
	// server object...

	LTVector vPos;
	m_pClientDE->GetObjectPos(m_hServerObject, &vPos);
	m_pClientDE->SetObjectPos(m_hObject, &vPos);


	// If we're not using polygrids (or special fx are set to the lowest
	// detail setting), don't update...

	if (m_bUseGlobalSettings)
	{
		CRiotSettings* pSettings = g_pRiotClientShell->GetSettings();
		if (!pSettings) return LTTRUE;

		uint8 nVal = pSettings->SpecialFXSetting();
		LTBOOL bOn  = pSettings->PolyGrids();

		if (!bOn || nVal == RS_LOW)
		{
			return LTTRUE;
		}
	}

		
	// Update the plasma based on the type of plasma...

	switch (m_nPlasmaType)
	{
		case PLASMA_FOUR_RING :
			UpdateFourRingPlasma();
		break;

		case PLASMA_NORMAL:
		default :
			 UpdatePlasma();
		break;
	}


	if (m_hstrSurfaceSprite)
	{
		UpdateSurface();
	}


	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPolyGridFX::UpdateSurface
//
//	PURPOSE:	Update the polygrid's surface fx
//
// ----------------------------------------------------------------------- //

void CPolyGridFX::UpdateSurface()
{
	if (!m_pClientDE || !m_hstrSurfaceSprite) return;

	LTFLOAT fXPan, fYPan, fXScale, fYScale;
	m_pClientDE->GetPolyGridTextureInfo(m_hObject, &fXPan, &fYPan, &fXScale, &fYScale);

	LTFLOAT fDeltaTime = m_pClientDE->GetFrameTime();

	// Scale in X direction...

	if (m_fXScaleDuration > 0.0f)
	{
		if (m_bScalingXUp)
		{
			fXScale += fDeltaTime * (m_fXScaleMax - m_fXScaleMin) / m_fXScaleDuration;
			if (fXScale > m_fXScaleMax) 
			{
				fXScale = m_fXScaleMax;
				m_bScalingXUp = LTFALSE;
			}
		}
		else
		{
			fXScale -= fDeltaTime * (m_fXScaleMax - m_fXScaleMin) / m_fXScaleDuration;
			if (fXScale < m_fXScaleMin) 
			{
				fXScale = m_fXScaleMin;
				m_bScalingXUp = LTTRUE;
			}
		}
	}

	// Scale in Y direction...

	if (m_fYScaleDuration > 0.0f)
	{
		if (m_bScalingYUp)
		{
			fYScale += fDeltaTime * (m_fYScaleMax - m_fYScaleMin) / m_fYScaleDuration;
			if (fYScale > m_fYScaleMax) 
			{
				fYScale = m_fYScaleMax;
				m_bScalingYUp = LTFALSE;
			}
		}
		else
		{
			fYScale -= fDeltaTime * (m_fYScaleMax - m_fYScaleMin) / m_fYScaleDuration;
			if (fYScale < m_fYScaleMin) 
			{
				fYScale = m_fYScaleMin;
				m_bScalingYUp = LTTRUE;
			}
		}
	}

	m_pClientDE->SetPolyGridTextureInfo(m_hObject, m_fXPan, m_fYPan, fXScale, fYScale);
}



// ----------------------------------------------------------------------- //
// Precalculates the lookup tables for plasma FX.
// ----------------------------------------------------------------------- //

void CPolyGridFX::PrecalculatePlasma()
{
	uint32 width, height;
	int halfWidth, halfHeight, x, y;
	char *pData;
	PGColor *pColorTable;
	LTFLOAT scale, val, maxDistSqr, testDist, t;
	int i;


	// Create the (scaled) sin table.
	if(!g_bSinTableInitted)
	{
		scale = (MATH_CIRCLE / 255.0f) * 3.0f;
		for(i=0; i < 256; i++)
		{
			val = (LTFLOAT)i * scale;
			g_SinTable[i] = (char)(sin(val) * 128.0f);
		}
	
		g_bSinTableInitted = LTTRUE;
	}


	// Fill in the distance grid.
	m_pClientDE->GetPolyGridInfo(m_hObject, &pData, &width, &height, &pColorTable);


	m_DistanceGrid = (uint8*)malloc(width*height);
	halfWidth = width >> 1;
	halfHeight = height >> 1;
	maxDistSqr = (LTFLOAT)(halfWidth*halfWidth + halfHeight*halfHeight);
	for(y=0; y < (int)height; y++)
	{
		for(x=0; x < (int)width; x++)
		{
			testDist = (LTFLOAT)((x-halfWidth)*(x-halfWidth) + (y-halfHeight)*(y-halfHeight));
			t = testDist / maxDistSqr;
			m_DistanceGrid[y*height+x] = (uint8)(t * 255.0f);
		}
	}
}


// ----------------------------------------------------------------------- //
// Inits the palette for the grid.  Pass in a ramp list for it to interpolate thru.
// ----------------------------------------------------------------------- //

void CPolyGridFX::SetPolyGridPalette(ColorRamp *pRamps, int nRamps)
{
	LTFLOAT t;
	uint32 width, height;
	char *pData;
	PGColor *pColorTable, color1, color2;
	int ramp, i, index1, index2;

	// Randomize the poly grid values.
	m_pClientDE->GetPolyGridInfo(m_hObject, &pData, &width, &height, &pColorTable);

	// Make the color table go from white to black.
	for(ramp=0; ramp < nRamps-1; ramp++)
	{
		index1 = pRamps[ramp].m_Index;
		index2 = pRamps[ramp+1].m_Index;
		VEC_COPY(color1, pRamps[ramp].m_Color);
		VEC_COPY(color2, pRamps[ramp+1].m_Color);

		for(i=index1; i < index2; i++)
		{
			t = (LTFLOAT)(i - index1) / (index2 - index1);
			VEC_LERP(pColorTable[i], color1, color2, t);
			pColorTable[i].a = 255.0f;
		}
	}
}


// ----------------------------------------------------------------------- //
// Updates the plasma effect.
// ----------------------------------------------------------------------- //

void CPolyGridFX::UpdatePlasma()
{
	uint32 width, height, x, y;
	char *pData, *pCur;
	PGColor *pColorTable;
	uint8 count, *pDistanceGrid;

	m_fCount += 50.0f * m_pClientDE->GetFrameTime();
	count = (uint8)m_fCount;

	// Randomize the poly grid values.
	m_pClientDE->GetPolyGridInfo(m_hObject, &pData, &width, &height, &pColorTable);
	for(y=0; y < height; y++)
	{
		pCur = pData + (y*width);
		
		pDistanceGrid = &m_DistanceGrid[y*width];
		x = width;
		while(x--)
		{
			*pCur = g_SinTable[(uint8)((*pDistanceGrid) + count)];
			++pDistanceGrid;
			++pCur;
		}
	}
}

LTFLOAT g_Counts[4] = {0.0f, 1.0f, 4.0f, 3.0f};
long g_Offsets[4][2] =
{
	6, 10,
	13, 3,
	17, 1,
	31, 22
};

// ----------------------------------------------------------------------- //
// Updates the plasma effect.
// ----------------------------------------------------------------------- //

void CPolyGridFX::UpdateFourRingPlasma()
{

	uint32 dwWidth, dwHeight;
	long width, height, x, y, xMask, yMask, yOffsets[4];
	char *pData, *pCur;
	PGColor *pColorTable;
	uint8 val[4], counts[4], *pDistanceGrid;
	long i;

	for(i=0; i < 4; i++)
	{
		g_Counts[i] += ((LTFLOAT)m_nRingRate[i]) * m_pClientDE->GetFrameTime();
		counts[i] = (uint8)g_Counts[i];
	}

	m_pClientDE->GetPolyGridInfo(m_hObject, &pData, &dwWidth, &dwHeight, &pColorTable);
	width = (long)dwWidth;
	height = (long)dwHeight;

	xMask = width - 1;
	yMask = height - 1;

	for(y=0; y < height; y++)
	{
		pCur = pData + (y*width);
		//pDistanceGrid = &m_DistanceGrid[y*width];
		pDistanceGrid = m_DistanceGrid;

		yOffsets[0] = ((y+g_Offsets[0][1]) & yMask)*width;
		yOffsets[1] = ((y+g_Offsets[1][1]) & yMask)*width;
		yOffsets[2] = ((y+g_Offsets[2][1]) & yMask)*width;
		yOffsets[3] = ((y+g_Offsets[3][1]) & yMask)*width;

		x = width;
		while(x--)
		{
			val[0] = pDistanceGrid[yOffsets[0] + ((x+g_Offsets[0][0]) & xMask)];
			val[1] = pDistanceGrid[yOffsets[1] + ((x+g_Offsets[1][0]) & xMask)];
			val[2] = pDistanceGrid[yOffsets[2] + ((x+g_Offsets[2][0]) & xMask)];
			val[3] = pDistanceGrid[yOffsets[3] + ((x+g_Offsets[3][0]) & xMask)];

			*pCur = (
				g_SinTable[(uint8)(val[0] + counts[0])] + 
				g_SinTable[(uint8)(val[1] + counts[1])] + 
				g_SinTable[(uint8)(val[2] + counts[2])] + 
				g_SinTable[(uint8)(val[3] + counts[3])] 
					) >> 2;
			
			//++pDistanceGrid;
			++pCur;
		}
	}
}