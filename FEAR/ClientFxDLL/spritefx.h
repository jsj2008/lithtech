//------------------------------------------------------------------
//
//   MODULE  : SPRITEFX.H
//
//   PURPOSE : Defines class CSpriteFX
//
//   CREATED : On 11/23/98 At 6:21:38 PM
//
//------------------------------------------------------------------

#ifndef __SPRITEFX_H__
#define __SPRITEFX_H__

// Includes....

#include "BaseSpriteFX.h"

class CSpriteProps : 
	public CBaseSpriteProps
{
public:

	CSpriteProps();

	//handles loading up a single property from the specified file
	virtual bool LoadProperty(ILTInStream* pStream, const char* pszName, const char* pszStringTable, const uint8* pCurveData);

	//called after all the properties have been loaded to perform any custom initialization
	virtual bool PostLoadProperties();

	//should a ray be cast to the viewer and the sprite be disabled if not visible?
	bool					m_bCastVisibleRay;

	//the range for the additional scale of the sprite. This applies an overall scale after the
	//function curve scale has been applied, which allows different instances to have different
	//sizes which is useful for making things like bullet holes appear different
	float					m_fMinScale;
	float					m_fMaxScale;

	//the color of this sprite over time
	TColor4fFunctionCurveI	m_cfcColor;		

	//the size of this sprite over time
	TFloatFunctionCurveI	m_ffcScale;
};



class CSpriteFX : 
	public CBaseSpriteFX
{
public :

	CSpriteFX( CBaseFX::FXType nType = CBaseFX::eSpriteFX );
	virtual ~CSpriteFX();

	// Member Functions

	virtual bool			Init(const FX_BASEDATA *pBaseData, const CBaseFXProps *pProps);
	virtual bool			Update(float tmCur);
	virtual void			Term();

private :

	const CSpriteProps*		GetProps() { return (const CSpriteProps*)m_pProps; }

	//hook for the custom render object, this will just call into the render function
	static bool				CustomRenderVisibleCallback(const LTRigidTransform& tCamera, const LTRigidTransform& tSkyCamera, void* pUser);

	//function that handles the visible callback
	bool					HandleVisibleCallback(const LTRigidTransform& tCamera, const LTRigidTransform& tSkyCamera);

	//the overall scale of the sprite. This controls the overall scale of the sprite and is factored
	//in with the scale function curve
	float					m_fScale;

	PREVENT_OBJECT_COPYING(CSpriteFX);
};

#endif