//------------------------------------------------------------------
//
//   MODULE  : FXMGR.H
//
//   PURPOSE : Defines class CClientFXMgr
//
//   CREATED : On 10/5/98 At 6:58:51 PM
//
//------------------------------------------------------------------

#ifndef __CLIENTFXMGR_H__
#define __CLIENTFXMGR_H__

// Includes....
#include "iltclient.h"
#include "FxFlags.h"
#include "BaseFx.h"
#include "EngineTimer.h"

// Forwards....
class CClientFXInstance;
class CClientFXLink;
struct FX_KEY;

// Classes....
class CClientFXMgr :
	public IClientFXMgr
{
public :

	// Constuctor
	CClientFXMgr();
	virtual ~CClientFXMgr();

	// Member Functions
	bool							Init(ILTClient *pLTClient, EngineTimer& timer );
	void							Term();

	bool							UpdateAllActiveFX();

	//called to determine for the provided camera transform, what the relative transform and scales
	//for the field of view should be applied given the current state of the active effects
	void							GetCameraModifier(	const LTRigidTransform& tCameraTrans,
														LTRigidTransform& tOutRelTrans,
														LTVector2& vOutFovScale);

	//called to get the controller modifier given a camera position. This controller modifier
	//is the intensity of each of the controller motors, wich will be in the range of [0..1]
	void							GetControllerModifier(	const LTRigidTransform& tCameraTrans,
															float fMotors[NUM_CLIENTFX_CONTROLLER_MOTORS]);

	//called to render the overlays to the current render target
	void							RenderOverlays();

	void							OnSpecialEffectNotify(HOBJECT hObject, ILTMessage_Read *pMsg);

	bool							CreateClientFX(CClientFXLink *pLink, const CLIENTFX_CREATESTRUCT &fxInit, bool bStartInst = false, bool bAddNextUpdate = false);

	void							ShutdownAllFX();
	void							ShutdownClientFX(CClientFXLink* pLink);

	//specifies whether or not gore is enabled
	void							SetGoreEnabled(bool bEnabled);

	//specifies whether or not we are in slow motion
	void							SetInSlowMotion(bool bInSlowMotion);

	// Accessors
	LTList<CClientFXInstance*>&		GetFXInstanceList() { return m_FXInstanceList; }

	//sets the camera that will be used for this effect manager
	void							SetCamera(HOBJECT hCamera);

	//called to enumerate the FX that this manager contains, and for each one it will query each
	//key for the objects it uses, and for each object the provided callback will be called
	//and provided with the effect, the object, and the provided user data
	void							EnumerateObjects(CBaseFX::TEnumerateObjectsFn pfnObjectCB, void* pUserData);

	//these functions are intended only for development support of reloading of effects mid-game
	//and therefore are not included in final builds. The calling of these should be to release
	//the effect database, which will shut down all of the effects, clear out the database, load in
	//the new database, and then restart the effects
#ifndef _FINAL
	void	ReleaseEffectDatabase();
	bool	RestartEffects();
#endif

private :

	//------------------------------
	//IClientFXMgr implementation
	virtual HOBJECT	GetCamera();
	virtual HENGINETIMER GetTimer();

	// Effect creation
	virtual bool	CreateEffect(const CLIENTFX_CREATESTRUCT& cs, bool bStartInst);

	// System Subscription
	virtual void	SubscribeOverlay(LTLink<IClientFXOverlay*>& Link);
	virtual void	SubscribeController(LTLink<IClientFXController*>& Link);
	virtual void	SubscribeCamera(LTLink<IClientFXCamera*>& Link);

	//called to delete a ClientFXInstance
	void			DeleteClientFXInstance(CClientFXInstance* pInstance);

	//creates an effect key and adds it to the specified instances list of active effects
	bool							CreateFXKey(const CLIENTFX_CREATESTRUCT &fxInit, CClientFXInstance* pInst, const FX_KEY* pKey);

	//given a detail level of an effect, this will determine if the effect key should
	//be played based upon the current LOD settings on the object
	bool							IsDetailLevelEnabled(uint32 nDetailLevel) const;

	//given a gore setting, this will determine if it should be allowed based upon the current
	//gore settings
	bool							IsGoreLevelEnabled(EFXGoreSetting eGoreSetting) const;

	//given a slow motion setting, this will determine if the specified slow motion setting should
	//be allowed
	bool							IsSlowMotionLevelEnabled(EFXSlowMotionSetting eSlowMoSetting) const;

	//called to create a new client FX. This will allocate the object, start the effect, and handle setting
	//up whether or not it is visible
	CClientFXInstance*				StartNewClientFX(	const CLIENTFX_CREATESTRUCT &fxInit, 
														bool bStartVisible, bool bAddNextUpdate);

	//internal function called to handle creation of a client effect into the provided object using
	//the provided values
	bool							StartClientFX(	CClientFXInstance* pInstance, const CLIENTFX_CREATESTRUCT &fxInit);

	CBaseFX*						CreateFX(const char *sName, FX_BASEDATA *pBaseData, CBaseFXProps* pProps);

	// Member Variables

	ILTClient					   *m_pClientDE;

	//the listing of effects registered for the overlay services
	LTList<IClientFXOverlay*>		m_OverlayList;

	//the listing of effects registered for the controller services
	LTList<IClientFXController*>	m_ControllerList;

	//the listing of effects registered for the camera services
	LTList<IClientFXCamera*>		m_CameraList;	

	LTList<CClientFXInstance*>		m_FXInstanceList;

	//effects that need to be added to the active effects list at the next update
	//(this is for effects that are created in mid-update)
	LTList<CClientFXInstance*>		m_NextUpdateFXList;

	//the paused status
	bool							m_bPaused;

	//is gore enabled?
	bool							m_bGoreEnabled;

	//are we currently in slow motion state
	bool							m_bInSlowMotion;

	//the camera that effects can use
	LTObjRef						m_hCamera;

	// Our timer object.
	EngineTimer						m_Timer;
};

#endif // __CLIENTFXMGR__H_
