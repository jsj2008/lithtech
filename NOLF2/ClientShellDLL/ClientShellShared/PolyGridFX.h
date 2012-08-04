 // ----------------------------------------------------------------------- //
//
// MODULE  : PolyGridFX.h
//
// PURPOSE : Polygrid special fx class - Definition
//
// CREATED : 10/13/97
//
// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __POLY_GRID_FX_H__
#define __POLY_GRID_FX_H__

#include "SpecialFX.h"
#include "PolyGridBuffer.h"

#define PGSURFACE_PLASMA_NORMAL		0
#define PGSURFACE_PLASMA_FOUR_RING	1
#define PGSURFACE_WAVE_PROP			2

#define PG_MAX_MODIFIERS			4

// These are used for SetPolyGridPalette.
struct ColorRamp
{
    ColorRamp(const LTVector &color, int index)
	{
		m_Color = color;
		m_Index = index;
	}

    LTVector m_Color;
	int m_Index;
};


struct PGCREATESTRUCT : public SFXCREATESTRUCT
{
    PGCREATESTRUCT();

    LTVector vDims;
    LTVector vColor1;
    LTVector vColor2;
    LTFLOAT fXScaleMin;
    LTFLOAT fXScaleMax;
    LTFLOAT fYScaleMin;
    LTFLOAT fYScaleMax;
    LTFLOAT fXScaleDuration;
    LTFLOAT fYScaleDuration;
    LTFLOAT fXPan;
    LTFLOAT fYPan;
	LTFLOAT fBaseReflection;
	LTFLOAT fVolumeIOR;
	std::string sSurfaceSprite;
	std::string sSurfaceEnvMap;
    LTFLOAT fAlpha;
    uint32 dwNumPoliesX;
	uint32 dwNumPoliesY;
	uint32 nNumStartupFrames;
    uint8 nPlasmaType;
    uint8 nRingRate[4];
    LTBOOL bAdditive;
    LTBOOL bMultiply;
	LTBOOL bFresnel;
	LTBOOL bBackfaceCull;
	LTBOOL bRenderEarly;
	LTBOOL bNormalMapSprite;
	std::string sDampenImage;
	float fDampenScale;
	float fTimeScale;
	float fSpringCoeff;
	float fModelDisplace;
	float fMinFrameRate;
	uint8 nActiveModifiers;
	uint16 nXMin[PG_MAX_MODIFIERS];
	uint16 nYMin[PG_MAX_MODIFIERS];
	uint16 nXMax[PG_MAX_MODIFIERS];
	uint16 nYMax[PG_MAX_MODIFIERS];
	uint16 nNumAccelPoints[PG_MAX_MODIFIERS];
	float fAccelAmount[PG_MAX_MODIFIERS];
	HOBJECT hVolumeBrush;
};

inline PGCREATESTRUCT::PGCREATESTRUCT()
{
	vDims.Init();
	vColor1.Init();
	vColor2.Init();
	fXScaleMin			= 0.0f;
	fXScaleMax			= 0.0f;
	fYScaleMin			= 0.0f;
	fYScaleMax			= 0.0f;
	fXScaleDuration		= 0.0f;
	fYScaleDuration		= 0.0f;
	fXPan				= 0.0f;
	fYPan				= 0.0f;
	fBaseReflection		= 0.5f;
	fVolumeIOR			= 1.33f;
	fAlpha				= 0.0f;
	dwNumPoliesX		= 0;
	dwNumPoliesY		= 0;
	nNumStartupFrames	= 0;
	nPlasmaType			= 0;
    bAdditive           = LTFALSE;
    bMultiply           = LTFALSE;
	bFresnel			= LTTRUE;
	bBackfaceCull		= LTTRUE;
	bRenderEarly		= LTFALSE;
	bNormalMapSprite	= LTFALSE;

	for (int i=0; i < 4; i++)
	{
		nRingRate[i] = 0;
	}

	fDampenScale		= 0.99f;
	fTimeScale			= 1.0f;
	fSpringCoeff		= 40.0f;
	fModelDisplace		= 10.0f;
	fMinFrameRate		= 10.0f;
	nActiveModifiers	= 0;

	for(uint32 nCurrMod = 0; nCurrMod < PG_MAX_MODIFIERS; nCurrMod++)
	{
		nXMin[nCurrMod]				= 0;
		nYMin[nCurrMod]				= 0;
		nXMax[nCurrMod]				= 0;
		nYMax[nCurrMod]				= 0;
		nNumAccelPoints[nCurrMod]	= 0;
		fAccelAmount[nCurrMod]		= 0.0f;
	}

	hVolumeBrush		= LTNULL;
}


class CPolyGridFX : public CSpecialFX
{
	public :

		CPolyGridFX() : CSpecialFX()
		{
			VEC_INIT(m_vDims);
			VEC_SET(m_vColor1, 255.0f, 255.0f, 255.0f);
			VEC_SET(m_vColor2, 255.0f, 255.0f, 255.0f);
			m_fXScaleMin		= 15.0f;
			m_fXScaleMax		= 25.0f;
			m_fYScaleMin		= 15.0f;
			m_fYScaleMax		= 25.0f;
			m_fXScaleDuration	= 10.0f;
			m_fYScaleDuration	= 10.0f;
			m_fXPan				= 0.0f;
			m_fYPan				= 0.0f;
			m_fBaseReflection	= 0.5f;
			m_fVolumeIOR		= 1.33f;
			m_fAlpha			= 0.7f;

            m_bAlwaysUpdate     = LTFALSE;
			m_fCount			= 0.0f;

            m_bScalingXUp       = LTTRUE;
            m_bScalingYUp       = LTTRUE;

			m_dwNumPoliesX		= 16;
			m_dwNumPoliesY		= 16;

			m_nNumStartupFrames	= 0;

			m_nSurfaceType		= PGSURFACE_PLASMA_FOUR_RING;
			m_nRingRate[0]		= 50;
			m_nRingRate[1]		= 10;
			m_nRingRate[2]		= 30;
			m_nRingRate[3]		= 20;

            m_bUseGlobalSettings = LTTRUE;

            m_bAdditive         = LTFALSE;
            m_bMultiply         = LTFALSE;
			m_bFresnel			= LTTRUE;
			m_bBackfaceCull		= LTTRUE;
			m_bRenderEarly		= LTFALSE;
			m_bNormalMapSprite	= LTFALSE;

			m_nCurrWaveBuffer		= 0;

			m_fDampenScale		= 0.99f;
			m_fTimeScale		= 1.0f;
			m_fSpringCoeff		= 40.0f;
			m_fModelDisplace	= 10.0f;
			m_fMinFrameRate		= 10.0f;
			m_nActiveModifiers	= 0;

			for(uint32 nCurrMod = 0; nCurrMod < PG_MAX_MODIFIERS; nCurrMod++)
			{
				m_nXMin[nCurrMod]					= 0;
				m_nYMin[nCurrMod]					= 0;
				m_nXMax[nCurrMod]					= 0;
				m_nYMax[nCurrMod]					= 0;
				m_nNumAccelPoints[nCurrMod]			= 0;
				m_fAccelAmount[nCurrMod]			= 0.0f;
				m_fPrevImpactAmountLeft[nCurrMod]	= 0.0f;
				m_nPrevImpactX[nCurrMod]			= 0;
				m_nPrevImpactY[nCurrMod]			= 0;
			}

			m_hVolumeBrush		= LTNULL;
		}

		~CPolyGridFX()
		{
		}

        virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
        virtual LTBOOL Update();
        virtual LTBOOL CreateObject(ILTClient* pClientDE);

		virtual uint32 GetSFXID() { return SFX_POLYGRID_ID; }

		//Given a point and a direction to look in, this function will find out where it intersects
		//the polygrid, and determine the displacement from the polygrid at that point. It will
		//return false if it doesn't intersect. This assumes an axis aligned polygrid.
		bool GetOrientedIntersectionHeight(const LTVector& vPos, const LTVector& vUnitDir, float& fDisplacement, LTVector& vIntersection);

		HOBJECT GetVolumeBrush() const { return m_hVolumeBrush; }

	protected:

		void LoadDampenImage(uint32 nWidth, uint32 nHeight);
		void SetPolyGridPalette(ColorRamp *pRamps, int nRamps);
		void UpdateSurface();

		//different updaters for the different surface styles
		void PrecalculatePlasma();
		void UpdatePlasma();
		void UpdateFourRingPlasma();

		LTBOOL OnServerMessage(ILTMessage_Read *pMsg);

		//wave propagation methods
		void InitWaveProp();
		void UpdateWaveProp(float fFrameTime);
		void CreateModelWaves(uint32 nKernalSize, uint32 nBuffer, float fFrameTime);

		//runs through several iterations of updating specified in NumStartupFrames so that
		//the water won't be completely calm when starting
		void HandleStartupFrames();
		
	private :

		// Plasma stuff.
        CPolyGridBuffer<uint8>	m_DistanceGrid;
		float					m_fCount;
        LTBOOL					m_bScalingXUp;
        LTBOOL					m_bScalingYUp;

		// Dampening image
		CPolyGridBuffer<uint8>	m_DampenBuffer;

		//Wave propagation buffers
		CPolyGridBuffer<float>	m_WaveBuffer[2];
		uint32					m_nCurrWaveBuffer;
		float					m_fPrevFrameTime;

		//maximum number of models in the water that we can keep track of
		enum { MAX_MODELS_TO_TRACK = 32	};

		//the handles to each tracked model
		HLOCALOBJ				m_hTrackedModels[MAX_MODELS_TO_TRACK];

		//the last position of the models we are tracking
		LTVector				m_vTrackedModelsPos[MAX_MODELS_TO_TRACK];

		//wave prop data
		float					m_fDampenScale;
		float					m_fTimeScale;
		float					m_fSpringCoeff;
		float					m_fModelDisplace;
		float					m_fMinFrameRate;


		//wave prop modifiers
		uint8					m_nActiveModifiers;
		uint16					m_nXMin[PG_MAX_MODIFIERS];
		uint16					m_nYMin[PG_MAX_MODIFIERS];
		uint16					m_nXMax[PG_MAX_MODIFIERS];
		uint16					m_nYMax[PG_MAX_MODIFIERS];
		uint16					m_nNumAccelPoints[PG_MAX_MODIFIERS];
		float					m_fAccelAmount[PG_MAX_MODIFIERS];

		//information about the previous unfinished impact
		float					m_fPrevImpactAmountLeft[PG_MAX_MODIFIERS];
		uint32					m_nPrevImpactX[PG_MAX_MODIFIERS];
		uint32					m_nPrevImpactY[PG_MAX_MODIFIERS];

        LTVector    m_vDims;
        LTVector    m_vColor1;
        LTVector    m_vColor2;
        LTFLOAT     m_fXScaleMin;
        LTFLOAT     m_fXScaleMax;
        LTFLOAT     m_fYScaleMin;
        LTFLOAT     m_fYScaleMax;
        LTFLOAT     m_fXScaleDuration;
        LTFLOAT     m_fYScaleDuration;
        LTFLOAT     m_fXPan;
        LTFLOAT     m_fYPan;
		LTFLOAT		m_fBaseReflection;
		LTFLOAT		m_fVolumeIOR;
		std::string m_sSurfaceSprite;
		std::string m_sSurfaceEnvMap;
		std::string m_sDampenImage;
        LTFLOAT     m_fAlpha;
        uint32      m_dwNumPoliesX;
		uint32      m_dwNumPoliesY;
		uint32		m_nNumStartupFrames;

        uint8       m_nSurfaceType;
        uint8       m_nRingRate[4];

        LTBOOL      m_bAlwaysUpdate;
        LTBOOL      m_bUseGlobalSettings;

        LTBOOL      m_bAdditive;
        LTBOOL      m_bMultiply;
		LTBOOL		m_bFresnel;
		LTBOOL		m_bBackfaceCull;
		LTBOOL		m_bRenderEarly;
		LTBOOL		m_bNormalMapSprite;

		HOBJECT		m_hVolumeBrush;
};

#endif // __POLY_GRID_FX_H__
