// ----------------------------------------------------------------------- //
//
// MODULE  : PolyGrid.h
//
// PURPOSE : PolyGrid - Definition
//
// CREATED : 10/20/97
//
// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __POLY_GRID_H__
#define __POLY_GRID_H__

#include "ClientSFX.h"


class PolyGrid : public CClientSFX
{
	public :

		PolyGrid();
		~PolyGrid();

        void Setup(LTVector* pvDims, LTVector* pvColor1, LTVector* pvColor2,
                   HSTRING hstrSurfaceSprite, LTFLOAT fXScaleMin, LTFLOAT fXScaleMax,
                   LTFLOAT fYScaleMin, LTFLOAT fYScaleMax, LTFLOAT fXScaleDuration,
                   LTFLOAT fYScaleDuration, LTFLOAT fXPan, LTFLOAT fYPan,
                   LTFLOAT fAlpha, uint32 dwNumPolies, LTBOOL bAdditive, LTBOOL bMultiply);

        LTVector m_vDims;
        LTVector m_vColor1;
        LTVector m_vColor2;
        LTFLOAT  m_fXScaleMin;
        LTFLOAT  m_fXScaleMax;
        LTFLOAT  m_fYScaleMin;
        LTFLOAT  m_fYScaleMax;
        LTFLOAT  m_fXScaleDuration;
        LTFLOAT  m_fYScaleDuration;
        LTFLOAT  m_fXPan;
        LTFLOAT  m_fYPan;
        LTFLOAT  m_fAlpha;
        uint8   m_nPlasmaType;
        uint8   m_nRingRate[4];
        uint32  m_dwNumPolies;
        LTBOOL   m_bAdditive;
        LTBOOL   m_bMultiply;

	protected :

        uint32 EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);

	private :

		HSTRING m_hstrSurfaceSprite;
        LTBOOL   m_bSetup;

		void ReadProp(ObjectCreateStruct *pStruct);
		void Update();

		void CacheFiles();
};

#endif // __POLY_GRID_H__