//------------------------------------------------------------------
//
//   MODULE  : BASEFX.H
//
//   PURPOSE : Defines class CBaseFX
//
//   CREATED : On 10/6/98 At 2:57:18 PM
//
//------------------------------------------------------------------

#ifndef __BASEFX_H__
#define __BASEFX_H__

// Includes....
#include "ltbasedefs.h"
#include "fxedinterface.h"
#include "ClientFxProp.h"
#include "iltphysicssim.h"
#include "FastList.h"
#include "IClientFXMgr.h"

// Defines....
extern ILTClient * g_pLTClient;

// FX state defines

#define FS_INITIALFRAME					(1<<0)		//first frame after being activated, can be set with active
#define FS_ACTIVE						(1<<1)		//we are active
#define FS_SHUTTINGDOWN					(1<<2)		//user wants to shut down?
#define FS_SUSPENDED					(1<<3)		//we are temporarily suspended (don't update)

// Detail level settings
#define	FX_NUM_DETAIL_SETTINGS			7

//the maximum length allowed for the name of a ClientFX effect
#define MAX_CLIENTFX_NAME_LEN			127

// FX follow defines
enum EFXFollowType
{
	eFXFollowType_Fixed,
	eFXFollowType_Follow,
	eFXFollowType_NodeAttach,
	eFXFollowType_SocketAttach,
};

// FX Gore settings
enum EFXGoreSetting
{
	// Should not be considered gore.
	eFXGoreSetting_No,
	// Should be considered gore.
	eFXGoreSetting_Yes,
	// Should only be used in low violence
	eFXGoreSetting_LowViolenceOnly,
};

// FX Slow Motion settings
enum EFXSlowMotionSetting
{
	//this should be available in slow motion and normal
	eFXSlowMotionSetting_All,
	//this should only be in slow motion
	eFXSlowMotionSetting_SlowMoOnly,
	//this should not be in slow motion
	eFXSlowMotionSetting_NoSlowMo
};


// Forwards declarations
class CBaseFX;
class CBaseFXProps;
class CMemBlockAllocator;

struct FX_PARAMS;
struct CLIENTFX_CREATESTRUCT;

// Type defines....

//an interface that is provided to a GetResources call on an effect, the effect can then call into
//the appropriate function as it encounters resources or effects
class IFXResourceCollector
{
public:
	virtual ~IFXResourceCollector()	{}

	//this function will be called with each resource that is encountered. The filename is
	//relative to the resource root
	virtual void	CollectResource(const char* pszResource) = 0;

	//this function will be called with each child effect that is encountered. Note that this
	//may be a semicolon delimited listing of effects
	virtual void	CollectEffect(const char* pszEffectName) = 0;
};

// FX function interface structure
struct FX_REF
{
	//functions that each type implements
	typedef CBaseFX*		(*FX_CREATEFUNC)();
	typedef CBaseFXProps*	(*FX_CREATEPROPFUNC)(CMemBlockAllocator& Allocator);
	typedef uint32			(*FX_GETPROPSIZE)();

	//the name of this effect type
	const char*			m_sName;

	//the function that should be called to create an instance of this effect type
	FX_CREATEFUNC		m_pfnCreate;

	//the function that should be called to create a property object for this object
	FX_CREATEPROPFUNC	m_pfnCreateProps;

	//this function determines the size of the property structure for this object
	FX_GETPROPSIZE		m_pfnGetPropSize;
};

// FX Creation functions
void		AddBaseProps(CFastList<CEffectPropertyDesc> *pList);

//called to recursively search down the attachment tree, looking for the node provided. This
//will return a handle to the node, or an invalid handle if no match could be found
bool FindModelNodeRecurse(HOBJECT hObj, const char* pszNodeName, HOBJECT& hFoundModel, HMODELNODE& hFoundNode);

//called to recursively search down the attachment tree, looking for the socket provided. This
//will return a handle to the socket, or an invalid handle if no match could be found
bool FindModelSocketRecurse(HOBJECT hObj, const char* pszNodeName, HOBJECT& hFoundModel, HMODELSOCKET& hFoundSocket);

//given a transform and also potential parent information (the rigid body or object) and a possible node
//if an object has been specified.
LTRigidTransform	GetWorldSpaceTransform(HPHYSICSRIGIDBODY hRigidBodyParent, HOBJECT hObjectParent,
										   HMODELNODE hNode, HMODELSOCKET hSocket, 
										   const LTRigidTransform& tRelTrans);

//DLL export functions
typedef uint32			(*FX_GETNUM)();
typedef FX_REF			(*FX_GETREF)(int);
typedef uint32			(*FX_GETVERSION)();
typedef void			(*FX_DELETEFUNC)(CBaseFX *);
typedef void			(*FX_SETAPPFOCUS)(bool bAppFocus);
typedef void			(*FX_FREEPROPLIST)(CBaseFXProps* pPropList);
typedef void			(*FX_INITDLLRUNTIME)();
typedef void			(*FX_TERMDLLRUNTIME)();

// FX Base data structure, all FX need these
struct FX_BASEDATA
{
	FX_BASEDATA()
	{
		memset(this, 0, sizeof(FX_BASEDATA));
		m_hParentRigidBody	= INVALID_PHYSICS_RIGID_BODY;
		m_hNodeAttach		= INVALID_MODEL_NODE;
		m_hSocketAttach		= INVALID_MODEL_SOCKET;
	}
	
	//the FXMgr that owns this effect that we should use to communicate with
	IClientFXMgr*		m_pFxMgr;

	//information about where the effect should be placed
	HOBJECT				m_hParentObject;
	HPHYSICSRIGIDBODY	m_hParentRigidBody;
	HMODELNODE			m_hNodeAttach;
	HMODELSOCKET		m_hSocketAttach;
	LTRigidTransform	m_tTransform;

	//target data that is used to indicate where an effect should
	//attach to if appropriate
	bool				m_bUseTargetData;
	LTVector			m_vTargetOffset;
	HOBJECT				m_hTargetObject;

	//the flags used to create this effect, a combination of the FXFLAG_ flags
	uint32				m_dwFlags;
};

// structure that needs to be filled out in order to create a new effect
struct CLIENTFX_CREATESTRUCT
{
	CLIENTFX_CREATESTRUCT()
	{
		Init("", 0);
	}

	CLIENTFX_CREATESTRUCT(const char *sName, uint32 dwFlags)
	{
		Init(sName, dwFlags);
	}

	CLIENTFX_CREATESTRUCT(const char *sName, uint32 dwFlags, const LTRigidTransform& tTransform)
	{
		Init(sName, dwFlags);
		m_tTransform = tTransform;
	}

	CLIENTFX_CREATESTRUCT(const char *sName, uint32 dwFlags, HOBJECT hParent)
	{
		Init(sName, dwFlags);
		m_hParentObject	= hParent;
	}

	CLIENTFX_CREATESTRUCT(const char *sName, uint32 dwFlags, HOBJECT hParent, const LTRigidTransform& tTransform)
	{
		Init(sName, dwFlags);
		m_tTransform	= tTransform;
		m_hParentObject	= hParent;
	}

	CLIENTFX_CREATESTRUCT(const char *sName, uint32 dwFlags, HPHYSICSRIGIDBODY hParent, const LTRigidTransform& tTransform)
	{
		Init(sName, dwFlags);
		m_tTransform		= tTransform;
		m_hParentRigidBody	= hParent;
	}

	//the name of the client effect to create
	char				m_sName[MAX_CLIENTFX_NAME_LEN + 1];

	//flags to specify for the client effect, a combination of the FXFLAG_ flags
	uint32				m_dwFlags;

	//the parent object that this effect should follow
	HOBJECT				m_hParentObject;

	//the parent rigid body that this effect should follow (ignored if an object is also provided)
	HPHYSICSRIGIDBODY	m_hParentRigidBody;

	//an optional transform that indicates which node or socket the effects should attach to
	HMODELNODE			m_hNode;
	HMODELSOCKET		m_hSocket;

	//a transform in the space of the world, object, or node (depending upon which are provided)
	LTRigidTransform	m_tTransform;

	//target information
	bool				m_bUseTargetData;
	HOBJECT				m_hTargetObject;
	LTVector			m_vTargetOffset;

private:

	//called to initialize all the data to the default values, and set common parameters
	void	Init(const char* pszName, uint32 nFlags)
	{
		memset(this, 0, sizeof(CLIENTFX_CREATESTRUCT));
		LTStrCpy(m_sName, pszName, LTARRAYSIZE(m_sName));
		m_dwFlags			= nFlags;
		m_hNode				= INVALID_MODEL_NODE;
		m_hSocket			= INVALID_MODEL_SOCKET;
		m_hParentRigidBody	= INVALID_PHYSICS_RIGID_BODY;
		m_tTransform.Init();
	}
};

// Classes....

class CBaseFXProps
{
public:

	CBaseFXProps();
	virtual ~CBaseFXProps();

	//sets up parameters for the effects lifetime
	virtual void			SetLifetime(float fStartTime, float fEndTime, bool bContinuous);

	//this will be called per property to load. This should return true if it is handled,
	//or false otherwise
	virtual bool			LoadProperty(	ILTInStream* pStream, const char* pszName, 
											const char* pszStringTable, const uint8* pCurveData);

	//this will be called once a property has been loaded
	virtual bool			PostLoadProperties();

	//this is called to collect the resources associated with these properties. For more information
	//see the IFXResourceCollector interface.
	virtual void			CollectResources(IFXResourceCollector& Collector)	{}

	//Fields that control the lifetime of an effect. This must be setup before properties
	//are parsed

	//the starting time of this effect within the key
	float					m_tmStart;
	float					m_tmEnd;

	//the length of a single repetition of an effect
	float					m_tmLifetime;

	//ordering used for when FX are played in the menu
	uint8					m_nMenuLayer;

	//the detail level of this effect
	uint8					m_nDetailLevel;

	//determines whether or not this effect is related to gore so that it can be disabled
	//on low violence settings
	LTEnum<uint8, EFXGoreSetting>		m_eGoreSetting;

	//determines how this effect should interact with slow motion
	LTEnum<uint8, EFXSlowMotionSetting>	m_eSlowMotion;

	//the type of method that this effect should use to follow it's parent if it has one
	LTEnum<uint8, EFXFollowType>		m_eFollowType;

	//whether or not this effect is continuously played.
	bool					m_bContinuous;

	//whether or not this effect is going to shut down smooth
	bool					m_bSmoothShutdown;

	//whether or not we need to update our rotation
	bool					m_bUpdateRotation;

	//the node to attach to
	const char*				m_pszAttach;

	//the starting rotation of the effect
	LTRotation				m_rInitialRotation;

	//the offset to apply in the effect space
	TVectorFunctionCurveI	m_vfcOffset;

	//the rotation to apply each frame in degrees per second
	TVectorFunctionCurveI	m_vfcRotation;
};


class CBaseFX
{
public :

	enum FXType
	{
		eBaseFX,
		eParticleSystemFX,
		eSpriteFX,
		eLTBModelFX,
		eDynaLightFX,
		eSoundFX,
		eCameraShakeFX,
		eCreateFX,
		eCreateRayFX,
		eFlareSpriteFX,
		eLightningFX,
		eTracerFX,
		eLensFlareFX,
		eDebrisSystemFX,
		eVideoControllerFX,
		eDecalFX,
		eOverlayFX,
		ePolyTrailFX,
		eRumbleFX,
	};
	
	CBaseFX( FXType nType );
	virtual ~CBaseFX();

	//initializes the effect based upon the passed in data
	virtual bool	Init(const FX_BASEDATA *pData, const CBaseFXProps *pProps);

	//terminates the effect
	virtual void	Term() = 0;

	//called to update the effect for the provided time interval. This can return false to enter a shutting down state
	virtual bool	Update(float tmFrameTime) = 0;

	//This version of update is called while the effect is suspended so that it can do
	//things like smooth shutdown depending upon the effect type
	virtual bool	SuspendedUpdate(float tmFrameTime);

	//This is called while an effect is suspended to determine whether or not it should
	//be visible
	virtual bool	IsVisibleWhileSuspended()					{ return false; }

	//called to determine if the object has completed shutting down
	virtual bool	IsFinishedShuttingDown()					{ return true; }

	//called in response to a query for a modifier to a camera transform, in order to allow 
	//called when the target object associated with an effect goes away, allowing effects
	//that might reference it to clean up references to it
	virtual void	ReleaseTargetObject(HOBJECT hTarget)					{}

	//called to enumerate all of the objects used by an effect. This will call into the provided
	//callback for each object used and provide the effect, the object, and the provided user data.
	//The reason it uses a callback approach instead of building up a vector is to avoid passing
	//allocated memory across DLL boundaries and so as to ensure that it uses as minimal memory as possible
	typedef void (*TEnumerateObjectsFn)(const CBaseFX* pFx, HOBJECT hObject, void* pUserData);
	virtual void	EnumerateObjects(TEnumerateObjectsFn pfnObjectCB, void* pUserData)	{}

	//handles setting and clearing state flags (bit mask operations)
	void					SetState(uint32 nState)			{ m_dwState |= nState; }
	void					ClearState(uint32 nState)		{ m_dwState &= ~nState; }
	bool					IsStateSet(uint32 nState) const { return !!(m_dwState & nState); }

	//Utility state detection functions
	bool					IsSuspended() const				{ return IsStateSet(FS_SUSPENDED); }
	bool					IsShuttingDown() const			{ return IsStateSet(FS_SHUTTINGDOWN); }
	bool					IsActive() const				{ return IsStateSet(FS_ACTIVE); }
	bool					IsInitialFrame() const			{ return IsStateSet(FS_INITIALFRAME); }

	// Accessors
	HOBJECT					GetParentObject()		const		{ return m_hParentObject; }
	HPHYSICSRIGIDBODY		GetParentRigidBody()	const		{ return m_hParentRigidBody; }
	FXType					GetFXType()				const		{ return m_nFXType; }
	uint32					GetMenuLayer()			const		{ return GetProps()->m_nMenuLayer; }
	uint32					GetDetailLevel()		const		{ return GetProps()->m_nDetailLevel; }
	EFXGoreSetting			GetGoreSetting()		const		{ return GetProps()->m_eGoreSetting; }
	EFXSlowMotionSetting	GetSlowMotionSetting()	const		{ return GetProps()->m_eSlowMotion; }
	uint32					GetSmoothShutdown()		const		{ return GetProps()->m_bSmoothShutdown; }


	void					SetElapsed(float fElapsed)		{ m_tmElapsed = fElapsed; }
	float					GetElapsed()		const		{ return m_tmElapsed; }
	float					GetStartTime()		const		{ return GetProps()->m_tmStart; }
	float					GetEndTime()		const		{ return GetProps()->m_tmEnd; }
	float					GetLifetime()		const		{ return GetProps()->m_tmLifetime; }
	bool					IsContinuous()		const		{ return GetProps()->m_bContinuous; }

	//called to get a unit lifetime value, this is from 0..1
	float					GetUnitLifetime() const			{ return LTMIN(GetElapsed() / GetLifetime(), 1.0f); }

	LTRigidTransform		GetParentTransform() const		{ return m_tParentTransform; }

	//the node and sockets that we are attached to, either or both can be invalid
	HMODELNODE				GetNodeAttach() const			{ return m_hNodeAttach; }
	HMODELSOCKET			GetSocketAttach() const			{ return m_hSocketAttach; }

	//toggles visibility of this object
	virtual void			SetVisible(bool bVisible);

	//called to update the parent information so that it will track the specified transform accordingly.
	//This will also update the positioning of fixed effects. If a node or socket is provided,
	//it will attach the effect to the node/socket and the transform will be in the node/socket space.
	//Otherwise, if an object is specified, it will track the parent, or else be in world space
	virtual void			SetParent(HOBJECT hParent, HMODELNODE hNode, HMODELSOCKET hSocket, const LTRigidTransform& tTransform);

	//called to track the specified rigid body as it's parent. This will clear any existing parent information
	//This will also update the positioning of fixed effects.
	virtual void			SetParent(HPHYSICSRIGIDBODY hParent, const LTRigidTransform& tTransform);

	//this function behaves the same as above, but only updates the parent offset.
	virtual void			SetParentOffset(const LTRigidTransform& tTransform);

	//this link is used to allow FX instances to store us in their list without having to perform
	//lots of allocations
	LTLink<CBaseFX*>		m_FXListLink;

protected :

	//an internal update functionality that must be called at the start of each effect's update
	//function. This does not return any value so it does not need to be checked
	void					BaseUpdate(float fTimeInterval);

	//returns the offset in the parent object's space that this effect should be placed at
	const LTRigidTransform&	GetParentOffset() const			{ return m_tParentOffset; }

	//this will determine the current additional offset that should be applied to the object
	LTRigidTransform		GetAdditionalTransform();

	// Member Functions
	const CBaseFXProps*		GetProps() const { assert(m_pProps); return m_pProps; }

	//given a point in time, this will determine the position and orientation of this effect based upon
	//the parent attachments and other such factors
	void					GetCurrentTransform(float fUnitLifetime, LTVector& vPos, LTRotation& rRot);

	// Member Variables

	//the Fx manager interface for the manager that created us
	IClientFXMgr*			m_pFxMgr;

	//the properties associated with this object
	const CBaseFXProps		*m_pProps;

private:

	//called to reset all parent information
	void					ClearParentInformation();

	//called to set the parent rigid body. This will make sure to handle proper releasing of reference
	//counting as appropriate, INVALID_PHYSICS_RIGID_BODY can be passed safely
	void					SetRigidBodyParent(HPHYSICSRIGIDBODY hParentRigidBody);

	//called to calculate the transform of the effect in world space. This does not apply the offsetting
	//of position or orientation
	void					UpdateParentTransform();

	//the current state of this effect
	uint32					m_dwState;

	//the type of this effect
	FXType					m_nFXType;

	//the current amount of time within the effect (between 0..m_tmActualEnd, but
	//can be more if doing a smooth shutdown)
	float					m_tmElapsed;

	//the last updated position of the parent. This will be updated as long as there is a parent,
	//and the update mode is not fixed
	LTRigidTransform		m_tParentTransform;

	//the transform relative to the parent
	LTRigidTransform		m_tParentOffset;

	//the parent object that we are attached to (can be NULL)
	HOBJECT					m_hParentObject;

	//the parent rigid body (can be NULL, and cannot be set with an object parent)
	HPHYSICSRIGIDBODY		m_hParentRigidBody;

	//the node and sockets that we are attached to, either or both can be invalid
	HMODELNODE				m_hNodeAttach;
	HMODELSOCKET			m_hSocketAttach;

	//an additional rotation applied to the parent aligned orientation
	LTRotation				m_rAdditional;

	PREVENT_OBJECT_COPYING(CBaseFX);
};

#endif
