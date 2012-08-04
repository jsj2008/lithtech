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

#define PLASMA_NORMAL		0
#define PLASMA_FOUR_RING	1

// These are used for SetPolyGridPalette.
typedef struct ColorRamp_t
{
    ColorRamp_t(LTVector color, int index)
	{
		m_Color = color;
		m_Index = index;
	}

    LTVector m_Color;
	int m_Index;
} ColorRamp;


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
	HSTRING hstrSurfaceSprite;
    LTFLOAT fAlpha;
    uint32 dwNumPolies;
    uint8 nPlasmaType;
    uint8 nRingRate[4];
    LTBOOL bAdditive;
    LTBOOL bMultiply;
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
	fAlpha				= 0.0f;
	dwNumPolies			= 0;
	nPlasmaType			= 0;
    hstrSurfaceSprite   = LTNULL;
    bAdditive           = LTFALSE;
    bMultiply           = LTFALSE;

	for (int i=0; i < 4; i++)
	{
		nRingRate[i] = 0;
	}
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
            m_hstrSurfaceSprite = LTNULL;
			m_fAlpha			= 0.7f;

            m_bAlwaysUpdate     = LTFALSE;
            m_DistanceGrid      = LTNULL;
			m_fCount			= 0.0f;

            m_bScalingXUp       = LTTRUE;
            m_bScalingYUp       = LTTRUE;

			m_dwNumPolies		= 160;

			m_nPlasmaType		= PLASMA_FOUR_RING;
			m_nRingRate[0]		= 50;
			m_nRingRate[1]		= 10;
			m_nRingRate[2]		= 30;
			m_nRingRate[3]		= 20;

            m_bUseGlobalSettings = LTTRUE;

            m_bAdditive         = LTFALSE;
            m_bMultiply         = LTFALSE;
		}

		~CPolyGridFX()
		{
			if (m_DistanceGrid)
			{
				free(m_DistanceGrid);
                m_DistanceGrid = LTNULL;
			}

			if (m_hstrSurfaceSprite && m_pClientDE)
			{
				m_pClientDE->FreeString(m_hstrSurfaceSprite);
			}

		}

        virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
        virtual LTBOOL Update();
        virtual LTBOOL CreateObject(ILTClient* pClientDE);

        void SetAlwaysUpdate(LTBOOL b=LTTRUE)         { m_bAlwaysUpdate = b; }
        void SetUseGlobalSettings(LTBOOL b=LTTRUE)    { m_bUseGlobalSettings = b; }

		virtual uint32 GetSFXID() { return SFX_POLYGRID_ID; }

	protected:

		void PrecalculatePlasma();
		void SetPolyGridPalette(ColorRamp *pRamps, int nRamps);
		void UpdatePlasma();
		void UpdateFourRingPlasma();
		void UpdateSurface();


	private :

		// Plasma stuff.
        uint8       *m_DistanceGrid;
		float		m_fCount;
        LTBOOL       m_bScalingXUp;
        LTBOOL       m_bScalingYUp;

        LTVector     m_vDims;
        LTVector     m_vColor1;
        LTVector     m_vColor2;
        LTFLOAT      m_fXScaleMin;
        LTFLOAT      m_fXScaleMax;
        LTFLOAT      m_fYScaleMin;
        LTFLOAT      m_fYScaleMax;
        LTFLOAT      m_fXScaleDuration;
        LTFLOAT      m_fYScaleDuration;
        LTFLOAT      m_fXPan;
        LTFLOAT      m_fYPan;
		HSTRING		m_hstrSurfaceSprite;
        LTFLOAT      m_fAlpha;
        uint32      m_dwNumPolies;

        uint8       m_nPlasmaType;
        uint8       m_nRingRate[4];

        LTBOOL       m_bAlwaysUpdate;
        LTBOOL       m_bUseGlobalSettings;

        LTBOOL       m_bAdditive;
        LTBOOL       m_bMultiply;
};

#endif // __POLY_GRID_FX_H__