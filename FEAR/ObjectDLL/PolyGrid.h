// ----------------------------------------------------------------------- //
//
// MODULE  : PolyGrid.h
//
// PURPOSE : PolyGrid - Definition
//
// CREATED : 10/20/97
//
// (c) 1997-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __POLYGRID_H__
#define __POLYGRID_H__

LINKTO_MODULE( PolyGrid );

#include "GameBase.h"

#ifndef __ENGINELODPROPUTIL_H__
#	include "EngineLODPropUtil.h"
#endif


class PolyGrid : public GameBase
{
	public :

		//the maximum number of modifiers that a polygrid may have on it
		enum	{	MAX_MODIFIERS = 4 };

		PolyGrid();
		~PolyGrid();

		LTVector	m_vDims;
		LTVector	m_vColor;
		float		m_fXScaleMin;
		float		m_fXScaleMax;
		float		m_fYScaleMin;
		float		m_fYScaleMax;
		float		m_fXScaleDuration;
		float		m_fYScaleDuration;
		float		m_fXPan;
		float		m_fYPan;
		float		m_fAlpha;
		float		m_fDampenScale;
		float		m_fTimeScale;
		float		m_fSpringCoeff;
		float		m_fModelDisplace;
		float		m_fMinFrameRate;
		float		m_fMinResolutionScale;
		uint32		m_dwNumPoliesX;
		uint32		m_dwNumPoliesY;
		uint32		m_nNumStartupFrames;
		bool		m_bRenderSolid;
		std::string	m_sMaterial;
		bool		m_bEnableCollisions;

		//name of the modifier objects (does not need to be saved/loaded, only used
		//for creation the first time)
		std::string m_sModifierName[MAX_MODIFIERS];

		//parameters from the associated objects

		//bitlist indicating which modifiers are active
		uint8		m_nActiveModifiers;

		//number of points to accelerate each time
		uint16		m_nNumAccelPoints[MAX_MODIFIERS];

		//amount to accelerate
		float		m_fAccelAmount[MAX_MODIFIERS];

		//the min and max of each modifier's rectangle
		float		m_fXMin[MAX_MODIFIERS];
		float		m_fYMin[MAX_MODIFIERS];
		float		m_fXMax[MAX_MODIFIERS];
		float		m_fYMax[MAX_MODIFIERS];

		//rendertarget parameters. These are not saved, these are just used to create
		//new render target objects
		bool		m_bReflectionMap;
		std::string	m_sReflectionMapLOD;
		std::string	m_sReflectionRTGroup;

		bool		m_bRefractionMap;
		std::string	m_sRefractionMapLOD;
		std::string	m_sRefractionRTGroup;

	protected :

		uint32	EngineMessageFn(uint32 messageID, void *pData, float fData);

	private :

		//given a message, this will fill it out with the polygrid data
		void	CreateSFXMessage(ILTMessage_Write& cMsg);

		bool	m_bCreatedFromSave;

		void	ReadProp(const GenericPropList *pProps);
		void	PostReadProp(ObjectCreateStruct *pStruct);
		void	InitialUpdate(const GenericPropList* pProps);
		void	Update();
		void	SetupModifiers();
		void	FreeModifierStrings();
		void	UpdateClients();
		void	Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
		void	Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);


		// Message Handlers...

		DECLARE_MSG_HANDLER( PolyGrid, HandleOnMsg );
		DECLARE_MSG_HANDLER( PolyGrid, HandleOffMsg );
};

#endif // __POLY_GRID_H__
