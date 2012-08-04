//------------------------------------------------------------------
//
//   MODULE  : LIGHTNINGFX.H
//
//   PURPOSE : Defines class CLightningFX
//
//   CREATED : On 10/12/98 At 5:07:15 PM
//
//------------------------------------------------------------------

#ifndef __LIGHTNINGFX__H_
	#define __LIGHTNINGFX__H_

	// Includes....

	#include "polytubefx.h"

	typedef uint32	HATTRACTOR;
	#define INVALID_ATTRACTOR ((HATTRACTOR)-1)


	class CLightningProps : public CBaseFXProps
	{
	public:

		CLightningProps();

		//this will take a list of properties and convert it to internal values
		virtual bool ParseProperties(FX_PROP* pProps, uint32 nNumProps);

		char			m_szTexture[128];
			
		ELTBlendMode	m_eBlendMode;
		ELTTestMode		m_eAlphaTest;
		ELTColorOp		m_eColorOp;
		ELTDPFillMode	m_eFillMode;
		EPTAllignment	m_eAllignment;	
		
		char			m_szNodeAttractors[128];
		char			m_szSocketAttractors[128];
		
		uint32			m_nMinNumBolts;
		uint32			m_nMaxNumBolts;
	
		uint32			m_nMinSegmentsPerBolt;
		uint32			m_nMaxSegmentsPerBolt;
	
		float			m_fMinBoltWidth;
		float			m_fMaxBoltWidth;

		float			m_fMinPerturb;
		float			m_fMaxPerturb;
		
		float			m_fMinLifetime;
		float			m_fMaxLifetime;
		
		float			m_fMinDelay;
		float			m_fMaxDelay;

		float			m_fPulse;
		float			m_fOmniDirectionalRadius;						
	};

	class CLightningFX : public CBaseFX
	{
		public :

			CLightningFX();
			~CLightningFX();

			// Member Functions

			bool			Init(ILTClient *pClientDE, FX_BASEDATA *pBaseData, const CBaseFXProps *pProps);
			bool			Update(float tmCur);
			bool			Render();
			void			ReadProps( CLinkList<FX_PROP> *pProps );
			void			Term();
			void			OnRendererShutdown();

						
		protected:

			void			EmitBolts( float tmCur );
			void			PreRender( float tmCur );

		protected:
			
			class CLightningBolt
			{
				public:

					CLightningBolt()
						:	m_fLifetime		( 0.0f ),
							m_tmElapsed		( 0.0f ),
							m_fWidth		( 0.0f ),
							m_nNumSegments	( 0 ),
							m_bActive		( false )
							
					{

					}

					~CLightningBolt()
					{
						m_collPathPts.RemoveAll();
					}

					CLinkList<PT_TRAIL_SECTION>	m_collPathPts;
					
					float		m_fLifetime;
					float		m_tmElapsed;
					float		m_fWidth;
					uint32		m_nNumSegments;
					bool		m_bActive;
					
					LTVector	m_vLastEmitterPos;
					LTVector	m_vLastAttractorPos;
			};

			class CAttractor
			{
				public:

					CAttractor()
					:	m_hModel		( LTNULL ),
						m_hAttractor	( INVALID_ATTRACTOR ),
						m_eType			( eInvalid )		
					{
					}

					LTRESULT GetTransform( LTransform &trans, bool bWorldSpace = false )
					{
						if( m_hModel && (m_hAttractor != INVALID_ATTRACTOR) )
						{
							switch( m_eType )
							{
								case eNode:
								{
									return g_pLTClient->GetModelLT()->GetNodeTransform( m_hModel, m_hAttractor, trans, bWorldSpace );
								}
								break;

								case eSocket:
								{
									return g_pLTClient->GetModelLT()->GetSocketTransform( m_hModel, m_hAttractor, trans, bWorldSpace );
								}
								break;

								default:
								break;
							}
						}

						return LT_ERROR;
					}
						
					enum AttractorType
					{
						eInvalid,
						eNode,
						eSocket,
					};
					
					HOBJECT			m_hModel;
					HATTRACTOR		m_hAttractor;
					AttractorType	m_eType;
			};

			const CLightningProps*			GetProps()		{ return (const CLightningProps*)m_pProps; }

			typedef std::vector<CLightningBolt*> LightningBolts;
			LightningBolts					m_lstBolts;

			typedef std::vector<CAttractor> AttractorList;
			AttractorList					m_lstAttractors;

			HTEXTURE		m_hTexture;			
			
			bool			m_bReallyClose;

			HOBJECT			m_hTarget;
			LTVector		m_vTargetPos;
			float			m_fDelay;
			
			float			m_tmElapsedEmission;

	};

#endif