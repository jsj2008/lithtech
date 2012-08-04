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

LINKTO_MODULE( PolyGrid );

class PolyGrid : public CClientSFX
{
	public :

		//the maximum number of modifiers that a polygrid may have on it
		enum	{	MAX_MODIFIERS = 4 };

		PolyGrid();
		~PolyGrid();

		//once all the member variables of the polygrid are setup, call Setup in order
		//to create the actual special effect message
        void Setup();

        LTVector	m_vDims;
        LTVector	m_vColor1;
        LTVector	m_vColor2;
        LTFLOAT		m_fXScaleMin;
        LTFLOAT		m_fXScaleMax;
        LTFLOAT		m_fYScaleMin;
        LTFLOAT		m_fYScaleMax;
        LTFLOAT		m_fXScaleDuration;
        LTFLOAT		m_fYScaleDuration;
        LTFLOAT		m_fXPan;
        LTFLOAT		m_fYPan;
        LTFLOAT		m_fAlpha;
        LTFLOAT		m_fDampenScale;
		LTFLOAT		m_fTimeScale;
		LTFLOAT		m_fSpringCoeff;
		LTFLOAT		m_fModelDisplace;
		LTFLOAT		m_fMinFrameRate;
		LTFLOAT		m_fBaseReflection;
		LTFLOAT		m_fVolumeIOR;
		uint8		m_nSurfaceType;
        uint8		m_nRingRate[4];
        uint32		m_dwNumPoliesX;
		uint32		m_dwNumPoliesY;
		uint32		m_nNumStartupFrames;
        bool		m_bAdditive;
        bool		m_bMultiply;
		bool		m_bFresnel;
		bool		m_bBackfaceCull;
		bool		m_bNormalMapSprite;
		bool		m_bRenderEarly;
		std::string	m_sSurfaceSprite;
		std::string	m_sSurfaceEnvMap;
		std::string	m_sDampenImage;

		//name of the modifier objects (does not need to be saved/loaded, only used
		//for creation the first time)
		std::string m_sModifierName[MAX_MODIFIERS];

		//parameters from the associated objects

		//bitlist indicating which modifiers are active
		uint8		m_nActiveModifiers;

		//number of points to accelerate each time
		uint16		m_nNumAccelPoints[MAX_MODIFIERS];

		//amount to accelerate
		LTFLOAT		m_fAccelAmount[MAX_MODIFIERS];

		//the min and max of each modifier's rectangle
		uint16		m_nXMin[MAX_MODIFIERS];
		uint16		m_nYMin[MAX_MODIFIERS];
		uint16		m_nXMax[MAX_MODIFIERS];
		uint16		m_nYMax[MAX_MODIFIERS];
		
		LTObjRef	m_hVolumeBrush;

	protected :

        uint32 EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);
		bool OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg);

	private :

		//given a message, this will fill it out with the polygrid data
		void CreateSFXMessage(ILTMessage_Write& cMsg);

		bool m_bCreatedFromSave;

		void ReadProp(ObjectCreateStruct *pStruct);
		void Update();
		void SetupModifiers();
		void FreeModifierStrings();
		void UpdateClients();
		void Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
		void Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);
};

#endif // __POLY_GRID_H__