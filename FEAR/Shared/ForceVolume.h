//---------------------------------------------------------------------------------
// ForceVolume.h
//
// Defines the shared object for a force volume which is what represents a physics
// container that can apply a force in a certain direction and also implement
// buoyancy on rigid bodies. This should be setup by both the client and server
// users of this system and allowed to update each frame so that it can influence
// objects within the volume.
//
//---------------------------------------------------------------------------------
#ifndef __FORCEVOLUME_H__
#define __FORCEVOLUME_H__

#ifndef __ILTPHYSICSSIM_H__
#include "iltphysicssim.h"
#endif

class CForceVolume
{
public:

	CForceVolume();
	~CForceVolume();

	//called to initialize the force volume given the rigid body that it should follow. This will reset
	//the active state to deactive and the force scale to 1.0
	bool	Init(	HOBJECT hWorldModel, float fDensity, const LTVector& vRelForceDir, 
					float fForceMag, float fWaveFrequency, float fWaveAmplitude, float fWaveBaseOffset,
					float fLinearDrag, float fAngularDrag);

	//determines if this has been initialized
	bool	IsInitted() const		{ return m_hContainerBody != NULL; }

	//called to free all objects associated with this object and go into an inactive state. This should
	//not be used until a subsequent successful Init call is made
	void	Term();

	//called to control whether or not this force volume is active or not. If the object was not
	//properly initialized, it will not activate.
	void	SetActive(bool bActive);
	bool	GetActive() const			{ return m_bActive; }

	//called to control the overall force scale for this container. This is used to scale the final
	//resulting force that will be applied to the objects (not buoyancy forces though)
	void	SetForceScale(float fScale);
	float	GetForceScale() const		{ return m_fForceScale; }

	//called to update the object. This will do nothing if the object is not active, otherwise it will
	//handle updating the container position, and updating the elapsed time
	void	Update(float fElapsedS);

private:

	//called to apply the physics forces to all of the objects that overlap
	void	ApplyForces(float fElapsedS);

	//callback function that it is triggered by the physics simulation
	static void ForceVolumeActionCB(HPHYSICSRIGIDBODY hBody, float fUpdateTimeS, void* pUserData);
	
	//don't allow copying
	PREVENT_OBJECT_COPYING(CForceVolume);

	//called internally to apply the specified world space force to the provided rigid body
	void	ApplyDirectForce(HPHYSICSRIGIDBODY hBody, const LTVector& vWSForce) const;

	//called internally to apply buoyancy force to the specified rigid body given a world space
	//gravity force
	void	ApplyBuoyancyForce(HPHYSICSRIGIDBODY hBody, const LTVector& vWSGravity, const LTPlane& WSSurface, float fElapsedS) const;

	//called internally to apply drag forces. This is only done when applying buoyancy forces though
	void	ApplyDragForces(HPHYSICSRIGIDBODY hBody, float fElapsedS, float fSurfaceArea, float fTotalMass) const;

	//the rigid body that we are associated with. This is the source of our shape and also what
	//we need to update our rigid body to follow
	HPHYSICSRIGIDBODY	m_hSourceRigidBody;

	//our container rigid body. This is the rigid body that we own and controls where our container
	//is located
	HPHYSICSRIGIDBODY	m_hContainerBody;

	//our container that we can use to determine overlapping objects
	HPHYSICSCONTAINER	m_hContainer;

	//our action to receive our callback
	HPHYSICSACTION		m_hAction;

	//whether or not this volume is active
	bool		m_bActive;

	//the density of this volume, if non-zero we need to do buoyancy forces. This is in kg/cm^3
	float		m_fDensity;

	//the rigid body space plane of the surface used for buoyancy calculations
	LTPlane		m_RelSurfacePlane;

	//the height of this object. This is used to determine the buoyancy plane by moving along the
	//up vector of the object by this height
	float		m_fObjectHeight;

	//the relative orientation of our force vector. 
	LTVector	m_vRelForceDir;

	//the magnitude of our force direction in Newtons
	float		m_fForceMag;

	//the parameters used to control the sine wave using the formula:
	//Scale = (sin((Time / Frequency) * two pi) * 0.5 + 0.5) * Amplitude + Base Offset
	float		m_fWaveFrequency;
	float		m_fWaveAmplitude;
	float		m_fWaveBaseOffset;

	//the current additional scale that the artists have specified to allow programatic control
	//over the force
	float		m_fForceScale;

	//the accumulated time that we need to use to compute our sin wave fluctuations
	float		m_fAccumulatedTime;

	//the linear and angular drag values associated with this volume. For every square centimeter
	//submerged, this percentage of the forward velocity will be applied as a force in the negative
	//direction
	float		m_fLinearDrag;
	float		m_fAngularDrag;
};

#endif
