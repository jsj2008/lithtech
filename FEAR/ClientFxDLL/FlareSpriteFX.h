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

#include "BaseSpriteFX.h"

class CFlareSpriteProps : 
	public CBaseSpriteProps
{
public:

	CFlareSpriteProps();

	//handles loading up a single property from the specified file
	virtual bool LoadProperty(ILTInStream* pStream, const char* pszName, const char* pszStringTable, const uint8* pCurveData);

	//called after all the properties have been loaded to perform any custom initialization
	virtual bool PostLoadProperties();

	float		m_fCosMinAngle;
	float		m_fMinAlpha;
	float		m_fMaxAlpha;
	float		m_fMinScale;
	float		m_fMaxScale;
	float		m_fScaleRange;
	float		m_fCosBlindSprAngle;
	float		m_fCosBlindCamAngle;
	float		m_fBlindMaxScale;
	bool		m_bUseCameraAngle;
	bool		m_bUseCamBlindAngle;
	bool		m_bBlindingFlare;	

	TColor4fFunctionCurve	m_cfcColor;		//the color of the flare sprite
};


class CFlareSpriteFX : 
	public CBaseSpriteFX
{
public:
	CFlareSpriteFX();
	virtual ~CFlareSpriteFX();

	virtual bool		Init(const FX_BASEDATA *pBaseData, const CBaseFXProps *pProps);
	virtual bool		Update(float tmCur);

private:

	CFlareSpriteProps*		GetProps()	{ return (CFlareSpriteProps*)m_pProps; }			

	//hook for the custom render object, this will just call into the render function
	static bool				CustomRenderVisibleCallback(const LTRigidTransform& tCamera, const LTRigidTransform& tSkyCamera, void* pUser);

	//function that handles the visible callback
	bool					HandleVisibleCallback(const LTRigidTransform& tCamera, const LTRigidTransform& tSkyCamera);

	PREVENT_OBJECT_COPYING(CFlareSpriteFX);
};

#endif // __FLARESPRITEFX_H__
