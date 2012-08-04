//-----------------------------------------------------------------------------
// LightningFX.h 
//
// A collection of procedurally animated 'bolts' that are composed of a sum
// of several mathmatical functions that are animated to create for an
// infinite bolt that can be subdivided and sampled as necessary, allowing
// for a very fast and memory light effect that has an unlimited amount
// of resolution.
//
//-----------------------------------------------------------------------------

#ifndef __LIGHTNINGFX_H__
#define __LIGHTNINGFX_H__

#ifndef __BASEFX_H__
#	include "BaseFx.h"
#endif

#ifndef __LIGHTNINGPROPS_H__
#	include "LightningProps.h"
#endif

#ifndef __ILTCUSTOMRENDER_H__
#	include "iltcustomrender.h"
#endif

class CLightningBolt;

//--------------------------------------------------------------------------------
// CAttractor
// 
// Utility class that represents a point of attraction (a socket or node on a model)
//--------------------------------------------------------------------------------
typedef uint32	HATTRACTOR;
#define INVALID_ATTRACTOR ((HATTRACTOR)-1)

class CAttractor
{
public:

	CAttractor();

	enum EAttractorType
	{
		eAttractorType_Invalid,
		eAttractorType_Node,
		eAttractorType_Socket,
	};

	//the node/socket that this is attracted to
	HATTRACTOR		m_hAttractor;

	//whether or not this attractor is valid, and if so, what type
	EAttractorType	m_eType;
};


//--------------------------------------------------------------------------------
// CLightningFX
//--------------------------------------------------------------------------------
class CLightningFX : 
	public CBaseFX
{
public:

	CLightningFX();
	~CLightningFX();

	//Overrides from CBaseFX
	virtual bool	Init(const FX_BASEDATA *pData, const CBaseFXProps *pProps);
	virtual bool	Update(float tmFrameTime);
	virtual bool	SuspendedUpdate(float tmFrameTime);
	virtual void	Term();
	virtual void	ReleaseTargetObject(HOBJECT hTarget);
	virtual void	EnumerateObjects(TEnumerateObjectsFn pfnObjectCB, void* pUserData);

	virtual bool	IsVisibleWhileSuspended()	{ return m_pBoltHead != NULL; }
	virtual bool	IsFinishedShuttingDown()	{ return m_pBoltHead == NULL; }

			
private:

	//called to free all the bolts assocaited with this object
	void FreeBolts();

	//called to emit a batch of bolts using the provided update time, and will update any
	//newly created bolts to be up to date with the specified elapsed time
	void EmitBolts(const LTRigidTransform& tEmitter, float fUnitLifetime, float fUpdateTime);

	//this will emit a batch of bolts at the specified time and update them for the interval specified
	void EmitBoltBatch(const LTRigidTransform& tEmitter, float fUnitLifetime, uint32 nNumBolts, float fUpdateTime);

	//called to update all existing bolts using the provided update time
	void UpdateExistingBolts(const LTVector& vEmitter, float fUpdateTime);

	//called to update a bolt based upon the update time provided. This will return true if the bolt
	//should be kept around, or false otherwise
	bool UpdateBolt(const LTVector& vEmitter, CLightningBolt& Bolt, float fUpdateTime, bool bUpdateTransform);

	//given a bolt, this will determine the world space position of the attractor to use for that
	//bolt
	LTVector GetBoltAttractorPos(const LTVector& vEmitter, const CLightningBolt& Bolt);

	//given a starting point an ending point, and a reference orientation, it will create a rotation
	//that will have the same relative up, but be oriented around the two points provided
	LTRotation GetBoltRotation(const LTVector& vEmitter, const LTVector& vTarget, const LTRotation& rRefRot);

	//called to calculate the AABB that encompasses the bolt
	void GetBoltExtents(const LTVector& vEmitter, const CLightningBolt& Bolt, LTVector& vMin, LTVector& vMax);

	//called to construct the list of attractors for this object using the node/socket attractors
	//specified in the properties. This assumes that the m_hTarget is setup when it is called.
	void BuildAttractorList();

	//called to update the visibility primitive for our lightning object
	void UpdateVisibility(const LTVector& vEmitter);

	//hook for the custom render object, this will just call into the render function
	static void CustomRenderCallback(ILTCustomRenderCallback* pInterface, const LTRigidTransform& tCamera, void* pUser);

	//function that handles the custom rendering
	void RenderLightning(ILTCustomRenderCallback* pInterface, const LTRigidTransform& tCamera);

	//called to access the lighting properties
	const CLightningProps*			GetProps()		{ return (const CLightningProps*)m_pProps; }

	//the list of attractors that we have
	static const uint32 knMaxAttractors = 16;
	CAttractor		m_Attractors[knMaxAttractors];

	//the number of valid attractors that we have
	uint16			m_nNumAttractors;

	//whether or not we should update attractor positions (need to stop if our target object
	//goes away)
	bool			m_bUpdateTargetPos;

	//the head to our list of lightning bolts
	CLightningBolt*	m_pBoltHead;

	//our custom render object for this effect
	HOBJECT			m_hObject;

	//the target object that the lightning is going towards
	HOBJECT			m_hTargetObject;

	//an offset in the target object's space (or WS if no target provided)
	LTVector		m_vTargetOffset;
	
	//the amount of time that has elapsed between bolt emissions
	float			m_fElapsedEmission;
};

#endif