//------------------------------------------------------------------
//
//   MODULE  : POLYTUBEFX.H
//
//   PURPOSE : Defines class CPolyTubeFX
//
//   CREATED : On 3/22/99 At 4:00:54 PM
//
//------------------------------------------------------------------

#ifndef __POLYTUBEFX_H_
	#define __POLYTUBEFX_H_

	// Includes....

	#include "clientfx.h"

	enum EPTWidthStyle
	{
		ePTWS_Constant,
		ePTWS_SmallToBig,
		ePTWS_SmallToSmall,
		ePTWS_BigToSmall
	};

	enum EPTAllignment
	{
		ePTA_Camera,
		ePTA_Up,
		ePTA_Right,
		ePTA_Forward
	};

	struct PT_TRAIL_SECTION
	{
		LTVector			m_vPos;
		LTVector			m_vTran;
		LTVector			m_vBisector;
		uint8				m_red;
		uint8				m_blue;
		uint8				m_green;
		uint8				m_alpha;
		float				m_fScale;
		float				m_uVal;
		float				m_tmElapsed;
	};

	class CPolyTubeProps : public CBaseFXProps
	{
	public:

		CPolyTubeProps();

		//this will take a list of properties and convert it to internal values
		virtual bool ParseProperties(FX_PROP* pProps, uint32 nNumProps);

		float							m_tmAddPtInterval;
		float							m_tmSectionLifespan;
		uint32							m_nMaxTrailLength;
		float							m_fTrailWidth;
		float							m_uAdd;
		char							m_sPath[128];
		ELTBlendMode					m_eBlendMode;
		ELTTestMode						m_eAlphaTest;
		ELTColorOp						m_eColorOp;
		ELTDPFillMode					m_eFillMode;
		EPTWidthStyle					m_eWidthStyle;
		EPTAllignment					m_eAllignment;	
	};

	class CPolyTubeFX : public CBaseFX
	{
		public :

			// Constuctor

									CPolyTubeFX();
			// Destructor

									~CPolyTubeFX();

			// Member Functions

			bool					Init(ILTClient *pClientDE, FX_BASEDATA *pBaseData, const CBaseFXProps *pProps);
			bool					Update(float tmCur);
			bool					Render();
			void					ReadProps( CLinkList<FX_PROP> *pProps );
			void					Term();

			void					OnRendererShutdown();

			bool					IsFinishedShuttingDown() { return (m_collPathPts.GetSize() == 0) ? true : false; }

			LTFLOAT					CalcCurWidth( );							

			// Accessors

		protected :

			const CPolyTubeProps*			GetProps() { return (const CPolyTubeProps*)m_pProps; }

			CLinkList<PT_TRAIL_SECTION>		m_collPathPts;
			float							m_tmElapsedEmission;
			uint32							m_dwWidth;
			HTEXTURE						m_hTexture;			
			bool							m_bReallyClose;
			bool							m_bLoadFailed;
			float							m_uOffset;
	};

#endif