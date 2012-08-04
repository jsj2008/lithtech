// ----------------------------------------------------------------------- //
//
// MODULE  : FlareSpriteFX.h
//
// PURPOSE : This FX is used as a blinding flare
//
// CREATED : 8/01/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __FLARESPRITEFX_H__
#define __FLARESPRITEFX_H__

//
// Includes...
//

	#include "SpriteFX.h"

	class CFlareSpriteProps : public CSpriteProps
	{
	public:

		CFlareSpriteProps();

		//this will take a list of properties and convert it to internal values
		virtual bool ParseProperties(FX_PROP* pProps, uint32 nNumProps);

		LTFLOAT		m_fMinAngle;
		LTBOOL		m_bUseCameraAngle;
		LTFLOAT		m_fMinAlpha;
		LTFLOAT		m_fMaxAlpha;
		LTFLOAT		m_fMinScale;
		LTFLOAT		m_fMaxScale;
		LTFLOAT		m_fScaleRange;
		LTBOOL		m_bUseCamBlindAngle;
		LTFLOAT		m_fBlindSprAngle;
		LTFLOAT		m_fBlindCamAngle;
		LTFLOAT		m_fBlindMaxScale;
		LTBOOL		m_bBlindingFlare;	
	};


	class CFlareSpriteFX : public CSpriteFX
	{
	protected: // Members...

		CFlareSpriteProps*		GetProps()	{ return (CFlareSpriteProps*)m_pProps; }			
			
	public: // Methods...

			CFlareSpriteFX();
			~CFlareSpriteFX();

			bool		Init(ILTClient *pClientDE, FX_BASEDATA *pBaseData, CBaseFXProps *pProps);
			bool		Update(float tmCur);
	};

#endif // __FLARESPRITEFX_H__