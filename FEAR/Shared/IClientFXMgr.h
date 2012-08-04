//---------------------------------------------------------------------------------
// IClientFXMgr.h
//
// Provides the definition for an interface that is made available to all ClientFX
// and allows the ClientFX to access data and services such as creation of other
// effects and registering for services.
//
//---------------------------------------------------------------------------------

#ifndef __ICLIENTFXMGR_H__
#define __ICLIENTFXMGR_H__

#ifndef __LTLINK_H__
#include "ltlink.h"
#endif

//forward declarations
struct CLIENTFX_CREATESTRUCT;

//the number of motors we support for the controller
#define NUM_CLIENTFX_CONTROLLER_MOTORS	2

//------------------------------------------------------------
// IClientFXOverlay
// This interface must be implemented by any effects that want to subscribe to the overlay
// functionality.
class IClientFXOverlay
{
public:
	IClientFXOverlay()	{}
	virtual ~IClientFXOverlay()	{}

	//For overlay effects, it should provide the layer value and whether or not it allows 
	//higher layers to be displayed, or return false if it should be omitted
	virtual bool	GetOverlayLayer(uint32& nLayer, bool& bAllowHigher) = 0;

	//called to render the overlay. This should return false if the effect does not support overlays
	virtual bool	RenderOverlay() = 0;
};

//------------------------------------------------------------
// IClientFXController
// This interface must be implemented by any effects that want to subscribe to the controller
// functionality.
class IClientFXController
{
public:
	IClientFXController()	{}
	virtual ~IClientFXController()	{}

	//called in response to a query for the intensity that should be used for the controller motors
	//and allows this effect to influence the motors appropriately. Motor intensities range from [0..1]
	//but this function should only accumulate and should not clamp in order to allow for interactions.
	virtual void	GetControllerModifier(	const LTRigidTransform& tCameraTrans,
											float fMotorIntensity[NUM_CLIENTFX_CONTROLLER_MOTORS]) = 0;
};

//------------------------------------------------------------
// IClientFXCamera
// This interface must be implemented by any effects that want to subscribe to the camera
// functionality.
class IClientFXCamera
{
public:
	IClientFXCamera()	{}
	virtual ~IClientFXCamera()	{}

	//called in response to a query for a modifier to a camera transform, in order to allow 
	//effects to provide a mechanism to influence cameras. This should return true if the 
	//output data should be used. tOutTrans is the relative offset to apply to the camera, and
	//vOutFov is the relative scale for the FOV
	virtual bool	GetCameraModifier(	const LTRigidTransform& tCameraTrans,
										LTRigidTransform& tOutTrans,
										LTVector2& vOutFov) = 0;
};

class IClientFXMgr
{
public:

	IClientFXMgr() {}
	virtual ~IClientFXMgr()	{};

	//called to access the camera object that is associated with this manager
	//(note: This is deprecated due to the fact that this will not work with multiple
	//cameras such as with render targets or with split screen)
	virtual HOBJECT			GetCamera() = 0;

	//called to access the timer object associated with this FX manager
	virtual HENGINETIMER	GetTimer() = 0;

	//---------------------------
	// Effect creation

	//called to create a new effect given the provided creation structure. This will return
	//whether or not that effect could be created
	virtual bool	CreateEffect(const CLIENTFX_CREATESTRUCT& cs, bool bStartInst) = 0;

	//---------------------------
	// System Subscription
	//
	// All of the following take link objects to avoid introducing unnecessary list overhead
	// and each link must point to the designated interface that should be called. To unsubscribe
	// from a service, the links should simply be removed.

	//called to subscribe to the overlay functionality which allows for rendering an overlay on top
	//of the screen
	virtual void	SubscribeOverlay(LTLink<IClientFXOverlay*>& Link) = 0;

	//called to subscribe to the controller functionality which allows for modifying controller state
	//such as force feedback
	virtual void	SubscribeController(LTLink<IClientFXController*>& Link) = 0;

	//called to subscribe to the camera functionality which allows for modifying the camera positioning
	virtual void	SubscribeCamera(LTLink<IClientFXCamera*>& Link) = 0;
};

#endif
