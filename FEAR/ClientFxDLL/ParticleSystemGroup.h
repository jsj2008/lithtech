#ifndef __PARTICLESYSTEMGROUP_H__
#define __PARTICLESYSTEMGROUP_H__

#ifndef __PARTICLESIMULATION_H__
#	include "ParticleSimulation.h"
#endif

#ifndef __PARTICLESYSTEMPROPS_H__
#	include "ParticleSystemProps.h"
#endif

#ifndef __ILTCUSTOMRENDERCALLBACK_H__
#	include "iltcustomrendercallback.h"
#endif

//forward declarations
struct SParticle;
class CParticleSystemProps;

//----------------------------------------------------------------------------------
// CParticleSystemGroup
//
//a particle group represents a collection of particles and maintains a single custom render object
//for visibility and rendering of the group

class CParticleSystemGroup
{
public:

	CParticleSystemGroup();
	~CParticleSystemGroup();

	//called to initialize a group and create the appropriate objects
	bool	Init(	IClientFXMgr* pFxMgr, HMATERIAL hMaterial, const LTRigidTransform& tObjTrans, 
					bool* pVisibleFlag, const CParticleSystemProps* pProps);

	//called to terminate this object and place it into an invalid state
	void	Term();

	//called to emit a batch of particles given the properties 
	void	EmitParticleBatch(float fUnitLifetime, float fUpdateTime, const LTRigidTransform& tObjTrans);

	//called to handle updating of a batch of particles given the appropriate properties
	void	UpdateParticles(float tmFrame, const LTVector& vGravity, float fFrictionCoef, const LTRigidTransform& tObjTrans);

	//called to get the number of particles in this system
	uint32	GetNumParticles() const			{ return m_Particles.GetNumParticles(); }

	//called to add a particle batch marker onto our listing of particles
	void	AddParticleBatchMarker(float fUpdateTime, bool bDefault);

	//provides access to the object
	HOBJECT	GetObject() const				{ return m_hCustomRender; }

	//the next particle system group in our list
	CParticleSystemGroup*		m_pNextGroup;

private:

	//---------------------------------------
	// Particle Creation utilities

	//this will randomly generate an object space position for the starting of a particle based upon
	//the current properties of this effect
	LTVector	GenerateObjectSpaceParticlePos(	const LTVector& vEmissionOffset, const LTVector& vEmissionDims,
												float fMinRadius, float fMaxRadius);

	//called to remove a particle using the particle iterator
	CParticleReverseIterator	RemoveParticle(CParticleReverseIterator& Iterator);

	//---------------------------------------
	// Particle Rendering

	//hook for the custom render object, this will just call into the render function
	static void CustomRenderCallback(ILTCustomRenderCallback* pInterface, const LTRigidTransform& tCamera, void* pUser);

	//function that handles the custom rendering
	void RenderParticleSystem(ILTCustomRenderCallback* pInterface, const LTRigidTransform& tCamera);

	//given a particle, this will determine the size and color of that particle
	void GetParticleSizeAndColor(SParticle* pParticle, uint32& nColor, float& fScale);

	// the number of currently outstanding particles that require ray testing
	uint32				m_nNumRayTestParticles;	

	//a boolean we should set each time we render to track visibility
	bool*				m_pVisibleFlag;

	//our effect manager
	IClientFXMgr*		m_pFxMgr;

	//the actual particle system
	CParticleSimulation	m_Particles;

	//the custom render object that we represent
	HOBJECT				m_hCustomRender;

	//the properties associated with this particle system
	const CParticleSystemProps*	m_pProps;
};

#endif