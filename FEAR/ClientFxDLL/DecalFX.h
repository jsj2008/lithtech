//------------------------------------------------------------------
//
//   MODULE  : DecalFX.H
//
//   PURPOSE : Provides the decal effect, which is like a sprite
//				that is projected onto world geometry
//
//   CREATED : On 11/23/98 At 6:21:38 PM
//
//------------------------------------------------------------------

#ifndef __DECALFX_H__
#define __DECALFX_H__

// Includes....

#ifndef __BASEFX_H__
#	include "basefx.h"
#endif

#ifndef __ILTCUSTOMRENDER_H__
#	include "iltcustomrender.h"
#endif

#ifndef __CLIENTFX_H__
#	include "clientfx.h"
#endif

// Forward declarations
class CMemoryPage;

class CDecalProps : public CBaseFXProps
{
public:

	CDecalProps();

	//handles loading up a single property from the specified file
	virtual bool LoadProperty(ILTInStream* pStream, const char* pszName, const char* pszStringTable, const uint8* pCurveData);

	//this is called to collect the resources associated with these properties. For more information
	//see the IFXResourceCollector interface.
	virtual void CollectResources(IFXResourceCollector& Collector);

	//the material that the decal will be rendered with
	const char*		m_pszMaterial;

	//the depth to project this decal up to in game units
	float			m_fProjectDepth;

	//the amount to allow in the forward direction of the decal
	float			m_fForwardProjectDepth;

	//the cosine of the attenuation angle, used to determine back facing polygons
	float			m_fCosAttenAngle;

	//the width and height of this decal in game units
	float			m_fWidth;
	float			m_fHeight;

	//a scale range that allows for more randomization of how large the decal is
	// (effects width, height, but not depth)
	float			m_fScaleMin;
	float			m_fScaleMax;

	//scale applied to the overlapping extents to allow artists to control how much a decal can
	//overlap
	float			m_fOverlapScale;

	//should this be rendered as two sided or not?
	bool			m_bTwoSided;

	//should this cast onto world models or not
	bool			m_bPlaceOnWorldModels;

	//is this sprite solid or not?
	bool			m_bSolid;

	//should this be lit when translucent
	bool			m_bTranslucentLight;

	//can this decal overlap with other decals?
	bool			m_bCanOverlap;

	//the layer that this decal should be placed into
	uint8			m_nRenderLayer;

	//the color of the decal over it's lifetime
	TColor4fFunctionCurveI	m_cfcColor;	
};

class CDecalFX : public CBaseFX
{
public :

	CDecalFX();
	virtual ~CDecalFX();

	// Member Functions

	virtual bool Init(const FX_BASEDATA *pBaseData, const CBaseFXProps *pProps);
	virtual bool Update(float tmCur);
	virtual void Term();
	virtual void EnumerateObjects(TEnumerateObjectsFn pfnObjectCB, void* pUserData);

private:

	//we don't want to have to implement all the copying operations
	PREVENT_OBJECT_COPYING(CDecalFX);
    
	//information about each world model that this decal is projected onto
	struct SWorldModelDecal
	{
		SWorldModelDecal();

		//called to terminate all the data associated with this world model decal
		//including freeing the object if it has one
		void			Term();

		//the world model that this decal is attached to
		LTObjRef		m_hAttachedWM;

		//the custom render object for the decal
		HOBJECT			m_hObject;

		//the visible radius of the decal geometry
		float			m_fVisRadius;

		//the center of the visible sphere in the object's space
		LTVector		m_vVisCenterOS;

		//the center of the visible sphere in the world's space
		LTVector		m_vVisCenterWS;

		//the AABB of the geometry in the world model's space, used for determining
		//overlapping decals
		LTRect3f		m_rOverlapAABB;

		//the memory page that the decal vertices start on
		CMemoryPage*	m_pStartPage;

		//the offset in that page to the start of the decal triangles
		uint32			m_nMemPageOffset;

		//the number of triangles that are associated with this decal
		uint32			m_nNumTris;

		//should we update state of this decal? (Don't for removed objects or main world)
		bool			m_bDisableFollowWM;

		//the owner object (necessary so we can use a single custom render callback for all 
		//world model decals)
		CDecalFX*		m_pParentDecal;
	};

	//called to update the decals so that they will follow their world models, or disappear if their
	//world models are gone
	void			UpdateWMDecals();

	//called to project the decal onto the world models it overlaps
	bool			ProjectDecal();

	//called to free any objects and memory associated with the decal
	void			FreeDecal();

	//called to determine if this decal overlaps with any of the other decals that are created
	bool			DoesOverlapExistingDecal(	HOBJECT* pObjects, uint32 nNumObjects, 
												const LTVector& vSphereCenter, float fRadius,
												const LTPlane* pPlanes, uint32 nNumPlanes);

	//this function will take a world model that overlaps with the decal, and will attempt to find
	//all of the clipped geometry for the decal on that world model. If none exists, it will return false.
	//If clipped geometry does exist, then it will create a custom render object and fill the data into
	//the provided decal world model structure and return true
	bool			ProjectDecalOntoWM(HOBJECT hWorldModel, const LTVector& vSphereCenter, float fSphereRadius,
										uint32 nNumPlanes, const LTPlane* pPlanes, const LTRigidTransform& tDecalTrans,
										const LTVector& vDecalDims, uint32 nWMDecalIndex);
	
	//called to add this decal to the global list of decals
	void	AddToGlobalDecalList();

	//called to remove this decal from the global list of decals
	void	RemoveFromGlobalDecalList();	

	//the maximum number of world models that this decal can project onto. This includes
	//the world
	enum	{	knMaxWMDecals = 4 };

	//information for each world model decal we possess
	SWorldModelDecal	m_WMDecals[knMaxWMDecals];

	//the number of actual world model decals we have
	uint32				m_nNumWMDecals;

	//the memory pages that hold our geometry data
	CMemoryPage*		m_pPageHead;

	//the current tail of our page list to allocate from
	CMemoryPage*		m_pPageTail;

	//hook for the custom render object, this will just call into the render function
	static void CustomRenderCallback(ILTCustomRenderCallback* pInterface, const LTRigidTransform& tCamera, void* pUser);

	//function that handles the actual custom rendering
	void RenderDecal(ILTCustomRenderCallback* pInterface, const SWorldModelDecal& WMDecal);

	const CDecalProps*	GetProps() { return (const CDecalProps*)m_pProps; }

	//this is used to maintain a global list of decals that are currently created, which allows
	//for us to determine when two decals overlap
	static CDecalFX* s_pDecalListHead;

	//links inside of the decal effect list
	CDecalFX*	m_pDecalListPrev;
	CDecalFX*	m_pDecalListNext;
};

//function that will add all the base sprite properties
void fxGetBaseDecalProps(CFastList<CEffectPropertyDesc> *pList);

#endif