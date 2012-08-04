//------------------------------------------------------------------
//
//   MODULE  : SPRITEFX.H
//
//   PURPOSE : Defines class CSpriteFX
//
//   CREATED : On 11/23/98 At 6:21:38 PM
//
//------------------------------------------------------------------

#ifndef __SpriteFX__H_
	#define __SpriteFX__H_

	// Includes....

	#include "basefx.h"

	// 
	// Defines...
	//	

	#define FACE_CAMERAFACING	0
	#define FACE_ALONGNORMAL	1
	#define	FACE_PARENTALIGN	2

	class CSpriteProps : public CBaseFXProps
	{
	public:

		CSpriteProps();

		//this will take a list of properties and convert it to internal values
		virtual bool ParseProperties(FX_PROP* pProps, uint32 nNumProps);

		char							m_sSpriteName[128];
		LTBOOL							m_bCastVisibleRay;
		uint8							m_nFacing;
		uint32							m_dwObjectFlags2;
		uint32							m_dwObjectFlags;
		LTVector						m_vNorm;
		LTBOOL							m_bRotate;
	};



	class CSpriteFX : public CBaseFX
	{
		public :

			// Constuctor

											CSpriteFX( CBaseFX::FXType nType = CBaseFX::eSpriteFX );

			// Destructor

			virtual							~CSpriteFX();

			// Member Functions

			virtual bool					Init(ILTClient *pClientDE, FX_BASEDATA *pBaseData, const CBaseFXProps *pProps);
			virtual bool					Update(float tmCur);
			virtual void					Term();
					void					ReadProps( CLinkList<FX_PROP> *pProps );

			// Accessors

		protected :

			const CSpriteProps*				GetProps() { return (const CSpriteProps*)m_pProps; }

			// Member Variables

			LTRotation						m_rRot;
			LTRotation						m_rNormalRot;
	};

#endif