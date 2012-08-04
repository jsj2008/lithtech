//------------------------------------------------------------------
//
//   MODULE  : BaseSpriteFX.H
//
//   PURPOSE : Provides a base class for the different sprite effects
//				to derive from and get common functionality
//
//   CREATED : On 11/23/98 At 6:21:38 PM
//
//------------------------------------------------------------------

#ifndef __BASESPRITEFX_H__
#define __BASESPRITEFX_H__

// Includes....

#ifndef __BASEFX_H__
#	include "basefx.h"
#endif

#ifndef __ILTCUSTOMRENDER_H__
#	include "iltcustomrender.h"
#endif

#ifndef __CLIENTFXSKYUTILS_H__
#	include "ClientFXSkyUtils.h"
#endif

class CBaseSpriteProps : public CBaseFXProps
{
public:

	CBaseSpriteProps();

	//handles loading up a single property from the specified file
	virtual bool LoadProperty(ILTInStream* pStream, const char* pszName, const char* pszStringTable, const uint8* pCurveData);

	//called after all the properties have been loaded to perform any custom initialization
	virtual bool PostLoadProperties();

	//this is called to collect the resources associated with these properties. For more information
	//see the IFXResourceCollector interface.
	virtual void CollectResources(IFXResourceCollector& Collector);

	const char*		m_pszMaterial;

	//dimensions to use when determining the aspect ratio of the sprite
	float			m_fAspectWidth;
	float			m_fAspectHeight;

	//the result of the aspect width / aspect height
	float			m_fAspectRatio;

	//the offset to apply towards the camera when camera aligned
	float			m_fToCameraOffset;

	//the rotational velocity of this sprite in radians/second
	float			m_fRotationVelRad;

	//is this sprite solid or not?
	bool			m_bSolid;

	//should this be lit when translcuent
	bool			m_bTranslucentLight;

	//should this sprite have both sides?
	bool			m_bTwoSided;

	//should this sprite always be oriented to face the camera?
	bool			m_bAlignToCamera;

	//should we always align around the Z axis?
	bool			m_bAlignAroundZ;

	//should we have this object render in the player rendering layer?
	bool			m_bPlayerView;

	//should we randomly determine a rotation for this sprite?
	bool			m_bRandomRotation;

	//should this effect be placed in the sky
	LTEnum<uint8, EFXSkySetting>	m_eInSky;
};

class CBaseSpriteFX : public CBaseFX
{
public :

	CBaseSpriteFX( CBaseFX::FXType nType = CBaseFX::eSpriteFX );
	virtual ~CBaseSpriteFX();

	// Member Functions

	virtual bool Init(const FX_BASEDATA *pBaseData, const CBaseFXProps *pProps);
	virtual bool Update(float tmCur);
	virtual void Term();
	virtual void EnumerateObjects(TEnumerateObjectsFn pfnObjectCB, void* pUserData);


protected :

	//called to set the width used by the visibility, this will update the visibility primitive
	//of the sprite based upon its rendering style. 
	void					SetVisScale(float fScale);

	//called to access the current scale of the sprite
	float					GetScale() const			{ return m_fWidth; }

	//called to change the scale of the sprite. This will update the visibile bounding area
	void					SetScale(float fScale);

	//the current color of the sprite
	LTVector4				m_vColor;

	//our actual object
	HOBJECT					m_hObject;

private:

	//hook for the custom render object, this will just call into the render function
	static void CustomRenderCallback(ILTCustomRenderCallback* pInterface, const LTRigidTransform& tCamera, void* pUser);

	//function that handles the actual custom rendering
	void RenderSprite(ILTCustomRenderCallback* pInterface, const LTRigidTransform& tCamera);

	const CBaseSpriteProps*	GetProps() { return (const CBaseSpriteProps*)m_pProps; }

	//the rotation used for this sprite when screen aligned in radians
	float					m_fCurrRotationRad;

	//the current scale of the sprite. This scale represents the total width of the sprite in game
	//units.
	float					m_fWidth;

	PREVENT_OBJECT_COPYING(CBaseSpriteFX);

};

//function that will add all the base sprite properties
void fxGetBaseSpriteProps(CFastList<CEffectPropertyDesc> *pList);

#endif