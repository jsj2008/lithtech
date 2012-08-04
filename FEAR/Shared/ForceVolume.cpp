#include "Stdafx.h"
#include "ForceVolume.h"
#include "PhysicsUtilities.h"

//-----------------------------------------------------------------------------------------------------
// Buoyancy Calculations
//-----------------------------------------------------------------------------------------------------

//given the direction of gravity, the density of the volume, and the amount of volume displaced, this
//will return the force vector that should be applied in world space
static LTVector CalcBuoyancyForceVector(const LTVector& vGravity, float fDensity, float fVolume)
{
	return vGravity * -fDensity * fVolume;
}

//given a radius, this will determine the volume of the sphere
static float CalcSphereVolume(float fRadius)
{
	return 4.0f / 3.0f * MATH_PI * fRadius * fRadius * fRadius;
}

//given a radius, this will determine the surface area of the sphere
static float CalcSphereSurfaceArea(float fRadius)
{
	return 4.0f * MATH_PI * fRadius * fRadius;
}

//given a radius of a sphere and the height of the spherical cap, this will determine the volume
//of the spherical cap
static float CalcSphericalCapVolume(float fRadius, float fCapHeight)
{
	return (1.0f / 3.0f) * MATH_PI * fCapHeight * fCapHeight * (3.0f * fRadius - fCapHeight);
}

//given the radius of a sphere and the height of a spherical cap, this will determine the surface
//area of that spherical cap
static float CalcSphericalCapSurfaceArea(float fRadius, float fCapHeight)
{
	return MATH_TWOPI * fRadius * fCapHeight;
}

//given the density of the water, and properties of the sphere, this will determine the density that
//should be used for calculating the forces that should be used on the sphere
static float CalcSphereWaterDensity(float fWaterDensity, float fRadius, float fMassKg, float fDensityG)
{
	float fActualDensityKg		= fMassKg / CalcSphereVolume(fRadius);
	float fSpecifiedDensityKg	= (fDensityG / 1000.0f);
	return fWaterDensity * fActualDensityKg / fSpecifiedDensityKg;
}

//given the density of the water, and properties of the OBB, this will determine the density that
//should be used for calculating the forces that should be used on the sphere
static float CalcOBBWaterDensity(float fWaterDensity, const LTVector& vHalfDims, float fMassKg, float fDensityG)
{
	float fOBBVolume = 8.0f * vHalfDims.x * vHalfDims.y * vHalfDims.z;
	float fActualDensityKg		= fMassKg / fOBBVolume;
	float fSpecifiedDensityKg	= (fDensityG / 1000.0f);
	return fWaterDensity * fActualDensityKg / fSpecifiedDensityKg;
}

//given the density of the water, and properties of the capsule, this will determine the density that
//should be used for calculating the forces that should be used on the sphere
static float CalcCapsuleWaterDensity(float fWaterDensity, float fRadius, float fLength, float fMassKg, float fDensityG)
{
	//we are just using OBB approximation, so pass that into the OBB calculator
	return CalcOBBWaterDensity(fWaterDensity, LTVector(fRadius, fRadius, fLength * 0.5f + fRadius), fMassKg, fDensityG);
}



//given a sphere and a plane, this will determine the amount of volume that is submerged beneath the
//plane and apply an appropriate force to the center of the sphere
static bool ApplySphereBuoyancy(const LTVector& vWSCenter, float fRadius, 
								const LTPlane& WSPlane, float& fVolume, float& fSurfaceArea)
{
	//determine the distance to the plane
	float fDistToPlane = WSPlane.DistTo(vWSCenter);

	//see if we are completely out of the volume
	if(fDistToPlane >= fRadius)
		return false;

	//now do an early out if we are completely submerged
	if(fDistToPlane <= -fRadius)
	{
		//fully submerged
		fVolume			= CalcSphereVolume(fRadius);
		fSurfaceArea	= CalcSphereSurfaceArea(fRadius);
	}
	else
	{
		//we are partially submerged, we can either have our bottom hemisphere partially submerged
		//or fully submerged with part of our top hemisphere partially submerged
		if(fDistToPlane < 0.0f)
		{
			//our bottom hemisphere is submerged, so determine the spherical cap and subtract
			//that from our total volume
			float fTotalVolume = CalcSphereVolume(fRadius);

			//determine the amount of the sphere above the surface
			float fCapHeight = fRadius + fDistToPlane;
			float fCapVolume = CalcSphericalCapVolume(fRadius, fCapHeight);

			fVolume			= fTotalVolume - fCapVolume;
			fSurfaceArea	= CalcSphereSurfaceArea(fRadius) - CalcSphericalCapSurfaceArea(fRadius, fCapHeight);
		}
		else
		{
			//only our bottom hemisphere is partially submerged, so just compute the spherical cap of
			//the submerged part
			float fCapHeight = fRadius - fDistToPlane;
			fVolume			= CalcSphericalCapVolume(fRadius, fCapHeight);
			fSurfaceArea	= CalcSphericalCapSurfaceArea(fRadius, fCapHeight);
		}
	}

	return true;
}

//given an OBB specified with a transform and half dimensions, this will approximate how much is submerged
//and distribute that force to the appropriate points on the box
static bool ApplyOBBBuoyancy(const LTRigidTransform& tTransform, const LTVector& vHalfDims,
							 const LTPlane& WSPlane, float& fVolume, LTVector& vApplyAt, float& fSurfaceArea)
{
	//structure representing cached information about one of the vertices of an OBB
	struct SOBBInfo
	{
		LTVector	m_vPos;
		float		m_fDist;
		bool		m_bBackSide;
	};

	//determine a translation for the plane, so that our volume determination can
	//be relative to this point
	LTVector vPlaneTranslation = tTransform.m_vPos - WSPlane.Normal() * (WSPlane.DistTo(tTransform.m_vPos));

	//determine the center of the OBB, but do so in the translation of the plane
	LTVector vTranslatedCenter = tTransform.m_vPos - vPlaneTranslation;

	//determine the axis of the main transform
	LTVector vRight, vUp, vForward;
	tTransform.m_rRot.GetVectors(vRight, vUp, vForward);

	//scale the vectors based upon the half dimensions
	vRight		*= vHalfDims.x;
	vUp			*= vHalfDims.y;
	vForward	*= vHalfDims.z;

	//generate the eight vertices of the OBB
	SOBBInfo OBB[8];
	OBB[0].m_vPos = vTranslatedCenter + vRight + vUp + vForward;
	OBB[1].m_vPos = vTranslatedCenter + vRight + vUp - vForward;
	OBB[2].m_vPos = vTranslatedCenter + vRight - vUp - vForward;
	OBB[3].m_vPos = vTranslatedCenter + vRight - vUp + vForward;
	OBB[4].m_vPos = vTranslatedCenter - vRight + vUp + vForward;
	OBB[5].m_vPos = vTranslatedCenter - vRight + vUp - vForward;
	OBB[6].m_vPos = vTranslatedCenter - vRight - vUp - vForward;
	OBB[7].m_vPos = vTranslatedCenter - vRight - vUp + vForward;

	//now run through and generate the distances to each point on the OBB

	//also determine the minimum and maximum extents so we can early out of the more expensive
	//computations
	OBB[0].m_fDist		= OBB[0].m_vPos.Dot(WSPlane.Normal());
	OBB[0].m_bBackSide	= (OBB[0].m_fDist < 0.0f);

	float fMin = OBB[0].m_fDist;
	float fMax = OBB[0].m_fDist;

	for(uint32 nCurrPt = 1; nCurrPt < 8; nCurrPt++)
	{
		//since we already translated the OBB as if the plane was at the origin,
		//we can just do a dot with the normal to get the distance
		OBB[nCurrPt].m_fDist		= OBB[nCurrPt].m_vPos.Dot(WSPlane.Normal());
		OBB[nCurrPt].m_bBackSide	= (OBB[nCurrPt].m_fDist < 0.0f);

		fMin = LTMIN(fMin, OBB[nCurrPt].m_fDist);
		fMax = LTMAX(fMax, OBB[nCurrPt].m_fDist);
	}

	//handle early out conditions
	if(fMin >= 0.0f)
	{
		//completely out of the water, apply no forces
		return false;
	}

	if(fMax <= 0.0f)
	{
		//completely beneath the water, find the volume (*8 is for the *2 that is on each dimension)
		fVolume	 = vHalfDims.x * vHalfDims.y * vHalfDims.z * 8.0f;
		vApplyAt = tTransform.m_vPos;
		fSurfaceArea = 2.0f * (vHalfDims.x * vHalfDims.y + vHalfDims.y * vHalfDims.z + vHalfDims.z * vHalfDims.z);
		return true;
	}

	//we are spanning the water, we need to do the expensive tests to determine how much is underwater
	//and where exactly is the center of geometry

	//we now know all the distances, apply the clipping algorithm to each face in turn, the winding order
	//of this is very important. This table was derived by taking a cube with vertices labeled, and laying
	//it out like a laid out cube map, mapping the vertices, and then entering the vertices in a consistant
	//winding order. The first vertex is repeated to avoid having to do any wrapping around.
	static const uint32 knFaces[]	= {	0, 1, 2, 3, 0,
										7, 6, 5, 4, 7,
										0, 3, 7, 4, 0,
										6, 2, 1, 5, 6,
										1, 0, 4, 5, 1,
										7, 3, 2, 6, 7	};

	//the vertices we will clip into (max we can have is 5, since we have 4 source and 1 clip plane)
	LTVector vClipped[5];

	//the computed volume we have displaced
	float fSubmergedVolume = 0.0f;

	//the computed center of geometry
	LTVector vCenterOfGeom(0.0f, 0.0f, 0.0f);
	uint32 nNumGeomContributers = 0;

	//the surface area that is submerged
	fSurfaceArea = 0.0f;

	//now accumulate the data for each face
	for(uint32 nCurrFace = 0; nCurrFace < LTARRAYSIZE(knFaces); nCurrFace += 5)
	{
		//the current output vertex
		uint32 nOutputVert = 0;

		for(uint32 nCurrEdge = 0; nCurrEdge < 4; nCurrEdge++)
		{
			const SOBBInfo& Vert1 = OBB[knFaces[nCurrFace + nCurrEdge]];
			const SOBBInfo& Vert2 = OBB[knFaces[nCurrFace + nCurrEdge + 1]];

			//handle clipping
			if(Vert1.m_bBackSide == Vert2.m_bBackSide)
			{
				//both on the same side, handle case of both behind, in which case we add vert 1,
				//or case of both in front, in which case we add none
				if(Vert1.m_bBackSide)
				{
					vClipped[nOutputVert] = Vert1.m_vPos;
					nOutputVert++;
				}
			}
			else
			{
				if(Vert1.m_bBackSide)
				{
					vClipped[nOutputVert] = Vert1.m_vPos;
					nOutputVert++;
				}


				//we need to clip it and optionally add the first one if we are going out
				float fPercent = Vert1.m_fDist / (Vert1.m_fDist - Vert2.m_fDist);
				LTVector vClipVert = Vert1.m_vPos + (Vert2.m_vPos - Vert1.m_vPos) * fPercent;

				vClipped[nOutputVert] = vClipVert;
				nOutputVert++;
			}
		}

		//sanity check that we didn't overflow our array of clipped vertices
		LTASSERT(nOutputVert <= LTARRAYSIZE(vClipped), "Error: Overflowed clipped vertex array");

		//we now have our clipped polygon, bail if it is invalid, otherwise accumulate the volume and
		//the center of gravity
		if(nOutputVert < 3)
			continue;

		vCenterOfGeom += vClipped[0];
		vCenterOfGeom += vClipped[1];
		nNumGeomContributers += nOutputVert;

		for(uint32 nClipVert = 2; nClipVert < nOutputVert; nClipVert++)
		{
			//determine the volume of the parellpiped formed by this region (we'll do the divide
			//by 6 at a later time)
			fSubmergedVolume += vClipped[0].Dot(vClipped[nClipVert - 1].Cross(vClipped[nClipVert]));

			//accumulate the surface area (this is the area of a rectangle, so it is actually double
			//what we want, but we reduce that down below)
			fSurfaceArea += (vClipped[nClipVert - 1] - vClipped[0]).Cross(vClipped[nClipVert] - vClipped[0]).Mag();

			//and accumulate the center of geometry as simply the vertices
			vCenterOfGeom += vClipped[nClipVert];			
		}
	}

	//we should have positive volume and surface area
	LTASSERT(fSubmergedVolume >= 0.0f, "Warning: Found negative submerged volumes. Check handedness of cross product?");
	LTASSERT(fSurfaceArea >= 0.0f, "Warning: Found negative surface area. Check handedness of cross product?");
	LTASSERT(nNumGeomContributers > 0, "Error: Found submerged OBB but with no vertices beneath the plane");

	//apply the scales to the accumulated values that we were avoiding doing in the loop

	//1/6th the total volume since we were summing parallelpipeds, and which is 6 times what we want
	fSubmergedVolume /= 6.0f;

	//the surface area is double what it needs to be at this point since we were accumulating 
	//rectangle areas
	fSurfaceArea *= 0.5f;

	//the center of geometry needs to be averaged out on the number of points 
	//and move the center of geometry out of the plane translation space
	vCenterOfGeom = vCenterOfGeom / (float)nNumGeomContributers + vPlaneTranslation;

	//for the application of the point, we don't directly use the center of geometry as that produces
	//WAY too much noise and rapid fluctuations, so instead we do a weighting between that, and the
	//center of mass for the shape, producing much more stable results
	static const float kfCenterOfMassWeight = 0.7f;
	vApplyAt = vCenterOfGeom.Lerp(tTransform.m_vPos, kfCenterOfMassWeight);
	fVolume	 = fSubmergedVolume;

	return true;
}

//given a capsule specified with a transform, two points, and a radius, this will approximate how much is submerged
//and distribute that force to the appropriate points on the capsule
static bool ApplyCapsuleBuoyancy(const LTVector& vPt1, const LTVector& vPt2, float fLength, float fRadius, 
								 const LTPlane& WSPlane, float& fVolume, LTVector& vApplyAt,
								 float& fSurfaceArea)
{
	//convert the capsule to an OBB and apply it
	
	//determine information about the main axis
	LTVector vMainAxis = vPt2 - vPt1;
	LTASSERT( fLength > 0.0f, "Invalid capsule length." );
	LTVector vUnitAxis = vMainAxis / fLength;

	//we can now build up a rotation given the plane normal and the axis to build our transform
	LTVector vUp = WSPlane.Normal();
	if(fabsf(vUp.Dot(vUnitAxis)) > 0.99f)
	{
		//too close to use, built an arbitrary orthonormal
		vUp = vUnitAxis.BuildOrthonormal();
	}

	LTMatrix3x4 mTemp;
	LTVector vRight = vUnitAxis.Cross(vUp);
	vRight.Normalize( );
	LTVector vTrueUp = vRight.Cross( vUnitAxis );
	mTemp.SetBasisVectors(vRight, vTrueUp, vUnitAxis);

	LTRotation rRot;
	rRot.ConvertFromMatrix(mTemp);

	//now we can form our transform
	LTRigidTransform tTransform((vPt1 + vPt2) * 0.5f, rRot);
	LTVector vHalfDims(fRadius, fRadius, fLength * 0.5f + fRadius);

	return ApplyOBBBuoyancy(tTransform, vHalfDims, WSPlane, fVolume, vApplyAt, fSurfaceArea);
}

//-----------------------------------------------------------------------------------------------------
// CBuoyancyCalculator
//-----------------------------------------------------------------------------------------------------

//this object is intended to be created and then used to determine and apply buoyancy to the rigid
//body provided
class CBuoyancyCalculator :
	public IShapeTraversalCallback
{
public:

	CBuoyancyCalculator(HPHYSICSRIGIDBODY hBody, float fDensity, 
						const LTVector& vGravity, const LTPlane& WSPlane,
						float fElapsedS) :
		m_hRigidBody(hBody),
		m_fDensity(fDensity),
		m_vGravity(vGravity),
		m_SurfacePlane(WSPlane),
		m_fElapsedS(fElapsedS),
		m_fSurfaceArea(0.0f),
		m_fTotalMassKg(0.0f)
	{
	}

	//called to access the amount of submerged surface area that has been calculated
	float	GetSurfaceArea() const		{ return m_fSurfaceArea; }

	//called to obtain the total mass calculated
	float	GetTotalMassKg() const		{ return m_fTotalMassKg; }

protected:

	//called when a sphere shape is encountered. This will provide the center of the sphere relative
	//to the transform heirarchy and the radius of that sphere
	virtual void HandleSphere(const LTVector& vCenter, float fRadius, float fMassKg, float fDensityG)							
	{
		m_fTotalMassKg += fMassKg;

		float fVolume, fSurfaceArea;
		if(ApplySphereBuoyancy(vCenter, fRadius, m_SurfacePlane, fVolume, fSurfaceArea))
		{
			m_fSurfaceArea += fSurfaceArea;
			float fWaterDensity = CalcSphereWaterDensity(m_fDensity, fRadius, fMassKg, fDensityG);
            LTVector vForce = CalcBuoyancyForceVector(m_vGravity, fWaterDensity, fVolume);
			if( LTIsNaN( vForce ) || vForce.MagSqr() > 1000000.0f * 1000000.0f )
			{
				LTERROR( "Invalid force detected." );
				vForce.Init( 0.0f, 10.0f, 0.0f );
			}
			g_pLTBase->PhysicsSim()->ApplyRigidBodyForceWorldSpace(m_hRigidBody, vCenter, vForce);
		}
	}

	//called when a capsule shape is encounered. This will provide the two end points of the capsule
	//relative to the transform heirarchy and the radius of the capsule
	virtual void HandleCapsule(const LTVector& vPt1, const LTVector& vPt2, float fRadius, float fMassKg, float fDensityG)		
	{
		m_fTotalMassKg += fMassKg;

		float fVolume, fSurfaceArea;
		LTVector vApplyAt;

		//determine the lenght of the capsule axis
		float fLength = vPt1.Dist(vPt2);

		if(ApplyCapsuleBuoyancy(vPt1, vPt2, fLength, fRadius, m_SurfacePlane, fVolume, vApplyAt, fSurfaceArea))
		{
			m_fSurfaceArea += fSurfaceArea;
			float fWaterDensity = CalcCapsuleWaterDensity(m_fDensity, fRadius, fLength, fMassKg, fDensityG);
			LTVector vForce = CalcBuoyancyForceVector(m_vGravity, fWaterDensity, fVolume);
			if( LTIsNaN( vForce ) || vForce.MagSqr() > 1000000.0f * 1000000.0f )
			{
				LTERROR( "Invalid force detected." );
				vForce.Init( 0.0f, 10.0f, 0.0f );
			}
			g_pLTBase->PhysicsSim()->ApplyRigidBodyForceWorldSpace(m_hRigidBody, vApplyAt, vForce);
		}
	}

	//called when an OBB shape is encountered. This will provide the transform of the OBB relative to
	//the transform heirarchy and the half dimensions of the OBB
	virtual void HandleOBB(const LTRigidTransform& tTransform, const LTVector& vHalfDims, float fMassKg, float fDensityG)		
	{
		m_fTotalMassKg += fMassKg;

		float fVolume, fSurfaceArea;
		LTVector vApplyAt;

		if(ApplyOBBBuoyancy(tTransform, vHalfDims, m_SurfacePlane, fVolume, vApplyAt, fSurfaceArea))
		{
			m_fSurfaceArea += fSurfaceArea;
			float fWaterDensity = CalcOBBWaterDensity(m_fDensity, vHalfDims, fMassKg, fDensityG);
			LTVector vForce = CalcBuoyancyForceVector(m_vGravity, fWaterDensity, fVolume);
			if( LTIsNaN( vForce ) || vForce.MagSqr() > 1000000.0f * 1000000.0f )
			{
				LTERROR( "Invalid force detected." );
				vForce.Init( 0.0f, 10.0f, 0.0f );
			}
			g_pLTBase->PhysicsSim()->ApplyRigidBodyForceWorldSpace(m_hRigidBody, vApplyAt, vForce);
		}
	}

private:

	//the total mass of this shape
	float				m_fTotalMassKg;

	//the amount of submerged surface area
	float				m_fSurfaceArea;

	//the rigid body that we should apply the forces to
	HPHYSICSRIGIDBODY	m_hRigidBody;

	//the density of the fluid we are in
	float				m_fDensity;

	//the force of gravity
	LTVector			m_vGravity;

	//the amount of time that has elapsed for this update
	float				m_fElapsedS;

	//the world space plane that is the surface of our water
	LTPlane				m_SurfacePlane;
};

//-----------------------------------------------------------------------------------------------------
// CForceVolume
//-----------------------------------------------------------------------------------------------------

CForceVolume::CForceVolume() :
	m_hSourceRigidBody(INVALID_PHYSICS_RIGID_BODY),
	m_hContainerBody(INVALID_PHYSICS_RIGID_BODY),
	m_hContainer(INVALID_PHYSICS_CONTAINER),
	m_hAction(INVALID_PHYSICS_ACTION),
	m_bActive(false),
	m_fForceScale(1.0f),
	m_fWaveFrequency(1.0f),
	m_fWaveAmplitude(0.0f),
	m_fWaveBaseOffset(1.0f),
	m_fForceMag(0.0f),
	m_fDensity(0.0f),
	m_fAccumulatedTime(0.0f)
{
}

CForceVolume::~CForceVolume()
{
	Term();
}

//called to initialize the force volume given the rigid body that it should follow. This will reset
//the active state to deactive and the force scale to 1.0
bool CForceVolume::Init(HOBJECT hWorldModel, float fDensity, const LTVector& vRelForceDir, 
						float fForceMag, float fWaveFrequency, float fWaveAmplitude, float fWaveBaseOffset,
						float fLinearDrag, float fAngularDrag)
{
	//make sure to clean up any existing data that we might have
	Term();

	//now verify our parameters
	if(!hWorldModel)
		return false;

	//cache the physics simulation interface
	ILTPhysicsSim* pILTPhysicsSim = g_pLTBase->PhysicsSim();

	//extract the rigid body from our world model
	pILTPhysicsSim->GetWorldModelRigidBody(hWorldModel, m_hSourceRigidBody);
	if(m_hSourceRigidBody == INVALID_PHYSICS_RIGID_BODY)
	{
		Term();
		return false;
	}

	// we need to create the container object that will handle maintaining the list of overlapping objects
	m_hContainer = pILTPhysicsSim->CreateContainer();
	if(m_hContainer == INVALID_PHYSICS_CONTAINER)
	{
		Term();
		return false;
	}

	//now we need to get the shape from this rigid body
	HPHYSICSSHAPE hWMShape;
	pILTPhysicsSim->GetRigidBodyShape(m_hSourceRigidBody, hWMShape);
	if(hWMShape == INVALID_PHYSICS_SHAPE)
	{
		Term();
		return false;
	}

	//and now we need to create a shape that contains our container and is bound by the space of the 
	//source rigid body's shape
	HPHYSICSSHAPE hContainerShape = pILTPhysicsSim->CreateContainerShape(hWMShape, m_hContainer);

	//and we can release our world model shape now since the container is now referencing it
	pILTPhysicsSim->ReleaseShape(hWMShape);
	hWMShape = INVALID_PHYSICS_SHAPE;

	if(hContainerShape == INVALID_PHYSICS_SHAPE)
	{
		Term();
		return false;
	}

	//extract the transform from our source rigid body which will be used to control where our body goes
	LTRigidTransform tSourceTransform;
	pILTPhysicsSim->GetRigidBodyTransform(m_hSourceRigidBody, tSourceTransform);

	//we now need to create the rigid body that will be used to create an instance of our shape and also
	//move our shape around the environment
	m_hContainerBody = pILTPhysicsSim->CreateRigidBody( hContainerShape, tSourceTransform, true, 
													    PhysicsUtilities::ePhysicsGroup_UserFiltered, 0, 0.5f, 0.0f );

	//we can now release our reference to the container shape since the body is holding onto it now
	pILTPhysicsSim->ReleaseShape(hContainerShape);
	hContainerShape = INVALID_PHYSICS_SHAPE;

	//now verify that we successfully created our rigid body
	if(m_hContainerBody == INVALID_PHYSICS_RIGID_BODY)
	{
		Term();
		return false;
	}

	//and finally, create our action
	m_hAction = pILTPhysicsSim->CreateAction(m_hContainerBody);
	if(m_hAction == INVALID_PHYSICS_ACTION)
	{
		Term();
		return false;
	}

	//setup our callback to notify us when we need to apply forces
	pILTPhysicsSim->SetActionCallback(m_hAction, ForceVolumeActionCB, (void*)this);

	//determine the transform from the world model space to the rigid body space since some values are
	//in world model space but we need it in rigid body space
	LTRigidTransform tWMTransform;
	g_pLTBase->GetObjectTransform(hWorldModel, &tWMTransform);
	LTRigidTransform tWMToRigidBody = tWMTransform.GetDifference(tSourceTransform);

	//determine the water plane which is in world model space and located along the up of the volume
	//moved along the Y dim extents
	LTVector vDims;
	g_pLTBase->Physics()->GetObjectDims(hWorldModel, &vDims);
	LTPlane WMWaterPlane(tWMTransform.m_rRot.Up(), vDims.y);

	//we have successfully created our object, so now go ahead and copy over the provided parameters
	m_vRelForceDir		= tWMToRigidBody.m_rRot.RotateVector(vRelForceDir);
	m_RelSurfacePlane	= tWMToRigidBody * WMWaterPlane;

	m_fDensity			= fDensity;
	m_fForceMag			= fForceMag;
	m_fWaveFrequency	= fWaveFrequency;
	m_fWaveAmplitude	= fWaveAmplitude;
	m_fWaveBaseOffset	= fWaveBaseOffset;
	m_fLinearDrag		= fLinearDrag;
	m_fAngularDrag		= fAngularDrag;
	m_fAccumulatedTime	= 0.0f;

	//success
	return true;
}

//called to free all objects associated with this object and go into an inactive state. This should
//not be used until a subsequent successful Init call is made
void CForceVolume::Term()
{
	//free our action
	if(m_hAction != INVALID_PHYSICS_ACTION)
	{
		g_pLTBase->PhysicsSim()->ReleaseAction(m_hAction);
		m_hAction = INVALID_PHYSICS_ACTION;
	}

	//free our container object
	if(m_hContainer != INVALID_PHYSICS_CONTAINER)
	{
		g_pLTBase->PhysicsSim()->ReleaseContainer(m_hContainer);
		m_hContainer = INVALID_PHYSICS_CONTAINER;
	}

	//free our container rigid body
	if(m_hContainerBody != INVALID_PHYSICS_RIGID_BODY)
	{
		g_pLTBase->PhysicsSim()->ReleaseRigidBody(m_hContainerBody);
		m_hContainerBody = INVALID_PHYSICS_RIGID_BODY;
	}

	//release the reference to the source rigid body we were holding onto
	if(m_hSourceRigidBody != INVALID_PHYSICS_RIGID_BODY)
	{
		g_pLTBase->PhysicsSim()->ReleaseRigidBody(m_hSourceRigidBody);
		m_hSourceRigidBody = INVALID_PHYSICS_RIGID_BODY;
	}

	//deactivate our object
	m_bActive		= false;
	m_fForceScale	= 1.0f;
}

//called to control whether or not this force volume is active or not. If the object was not
//properly initialized, it will not activate.
void CForceVolume::SetActive(bool bActive)
{
	//we can only activate if we have a valid object
	m_bActive = bActive && m_hContainerBody;
}

//called to control the overall force scale for this container. This is used to scale the final
//resulting force that will be applied to the objects (not buoyancy forces though)
void CForceVolume::SetForceScale(float fScale)
{
	m_fForceScale = fScale;
}

//callback function that it is triggered by the physics simulation
void CForceVolume::ForceVolumeActionCB(HPHYSICSRIGIDBODY hBody, float fUpdateTimeS, void* pUserData)
{
// Track the current execution shell scope for proper SEM behavior
#if defined(PLATFORM_SEM)

	#if defined(_CLIENTBUILD) && defined(_SERVERBUILD)
		#error Force volumes require seperate compilation.  Please remove module from the shared project.
	#else

		CLIENT_CODE
		(
			CGameClientShell::CClientShellScopeTracker cScopeTracker;
		)
		SERVER_CODE
		(
			CGameServerShell::CServerShellScopeTracker cScopeTracker;
		)

	#endif

#endif // PLATFORM_SEM

	LTASSERT(pUserData, "Error: Found invalid user data passed into the force volume action callback");
	((CForceVolume*)pUserData)->ApplyForces(fUpdateTimeS);
}

//called to update the object. This will do nothing if the object is not active, otherwise it will
//handle updating the container position, and applying forces to any objects that it overlaps
void CForceVolume::Update(float fElapsedS)
{
	//do nothing if we are inactive
	if(!m_bActive)
		return;

	//cache the physics simulation interface
	ILTPhysicsSim* pILTPhysicsSim = g_pLTBase->PhysicsSim();

	//handle updating our object state
	m_fAccumulatedTime += fElapsedS;

	//we need to update our rigid body to follow along with our source rigid body
	LTRigidTransform tSourceTransform;
	pILTPhysicsSim->GetRigidBodyTransform(m_hSourceRigidBody, tSourceTransform);
	pILTPhysicsSim->TeleportRigidBody(m_hContainerBody, tSourceTransform);
}

//called to apply the physics forces to all of the objects that overlap
void CForceVolume::ApplyForces(float fElapsedS)
{
	//do nothing if we are inactive
	if(!m_bActive)
		return;

	//cache the physics simulation interface
	ILTPhysicsSim* pILTPhysicsSim = g_pLTBase->PhysicsSim();

	//we need to update our rigid body to follow along with our source rigid body
	LTRigidTransform tSourceTransform;
	pILTPhysicsSim->GetRigidBodyTransform(m_hSourceRigidBody, tSourceTransform);

	//perform our force computations

	//we can now precompute the force vector that we will apply to the objects
	float fSinScale = (sinf((m_fAccumulatedTime / m_fWaveFrequency) * MATH_TWOPI) * 0.5f + 0.5f) * m_fWaveAmplitude + m_fWaveBaseOffset;

	//calculate the final magnitude
	float fFinalMag = m_fForceScale * fSinScale * m_fForceMag;

	//see if we can avoid doing any of these force calculations
	bool bApplyDirectForce	= (fabsf(fFinalMag) > 0.1f);
	bool bApplyBuoyancy		= (fabsf(m_fDensity) > 0.00001f);

	//we can bail if we aren't applying any type of force
	if(!bApplyDirectForce && !bApplyBuoyancy)
		return;

	//what we need to do now is to apply the forces to all of the objects contained within our container
	uint32 nObjectsInContainer = 0;
	pILTPhysicsSim->GetNumRigidBodiesInContainer(m_hContainer, nObjectsInContainer);

	//and just bail if we have no objects
	if(nObjectsInContainer == 0)
		return;

	//determine the world space facing of the force vector
	LTVector vWSForce = tSourceTransform.m_rRot.RotateVector(m_vRelForceDir) * fFinalMag; 

	//and also determine the world space direction of gravity
	LTVector vWSGravity;
	g_pLTBase->Physics()->GetGlobalForce(vWSGravity);

	//and the world space positioning of the surface of the volume
	LTPlane WSSurface = tSourceTransform * m_RelSurfacePlane;

	//now we need to get that list of objects

	//for now just use a fixed sized list, but when we have a stack allocator, allocate a block big enough
	//to always hold the number of objects
	HPHYSICSRIGIDBODY hContainedObjects[256] = { INVALID_PHYSICS_RIGID_BODY };

	// The above list may contain duplicates.  We don't want to apply a force to a rigid body multiple times
	// so maintain this list of rigid bodies that have been applied a force and compare the current rigid body
	// against this list to determine if it is a duplicate.
	HPHYSICSRIGIDBODY hRigidBodiesUsed[256] = { INVALID_PHYSICS_RIGID_BODY };

	uint32 nNumReturnedObjects = 0;
	pILTPhysicsSim->GetRigidBodiesInContainer(m_hContainer, hContainedObjects, LTARRAYSIZE(hContainedObjects), nNumReturnedObjects);

	//now we need to apply the forces on the objects
	for(uint32 nCurrObject = 0; nCurrObject < nNumReturnedObjects; nCurrObject++)
	{
		HPHYSICSRIGIDBODY hBody = hContainedObjects[nCurrObject];
		LTASSERT(hBody != INVALID_PHYSICS_RIGID_BODY, "Error: Found invalid physics rigid body in the list of contained objects");

		//if the rigid body is pinned, we can avoid all force calculations
		bool bPinned;
		pILTPhysicsSim->IsRigidBodyPinned(hBody, bPinned);

		if(!bPinned)
		{
			// The contained object list may have duplicates so ignore any rigid bodies that previously had an impulse applied.
			if( std::find( hRigidBodiesUsed, hRigidBodiesUsed + nCurrObject, hBody ) == hRigidBodiesUsed + nCurrObject )
			{
				hRigidBodiesUsed[ nCurrObject ] = hBody;

				//we need to apply the initial force to the object through the center of their mass
				if(bApplyDirectForce)
				{
					ApplyDirectForce(hBody, vWSForce);			
				}

				//now we need to apply a buoyancy force if we have any density
				if(bApplyBuoyancy)
				{
					//we need to calculate the buoyant force and apply it to the object in the appropriate
					//locations
					ApplyBuoyancyForce(hBody, vWSGravity, WSSurface, fElapsedS);
				}
			}
		}

		//and finally, make sure to release our reference to the rigid body so we don't create
		//any resource leaks
		pILTPhysicsSim->ReleaseRigidBody(hBody);
	}
}

//called internally to apply the specified world space force to the provided rigid body
void CForceVolume::ApplyDirectForce(HPHYSICSRIGIDBODY hBody, const LTVector& vWSForce) const
{
	//cache the physics simulation interface
	ILTPhysicsSim* pILTPhysicsSim = g_pLTBase->PhysicsSim();

	//get the center of mass of the object
	LTVector vCOM;
	pILTPhysicsSim->GetRigidBodyCenterOfMassInWorld(hBody, vCOM);

	//and apply the force through that
	if( LTIsNaN( vWSForce ) || vWSForce.MagSqr() > 1000000.0f * 1000000.0f )
	{
		LTERROR( "Invalid force detected." );
		pILTPhysicsSim->ApplyRigidBodyForceWorldSpace(hBody, vCOM, LTVector( 0.0f, 10.0f, 0.0f ));
	}
	else
	{
		pILTPhysicsSim->ApplyRigidBodyForceWorldSpace(hBody, vCOM, vWSForce);
	}
}

//called internally to apply buoyancy force to the specified rigid body given a world space
//gravity force
void CForceVolume::ApplyBuoyancyForce(HPHYSICSRIGIDBODY hBody, const LTVector& vWSGravity, const LTPlane& WSSurface, float fElapsedS) const
{
	//cache the physics simulation interface
	ILTPhysicsSim* pILTPhysicsSim = g_pLTBase->PhysicsSim();

	//first off, we need to get the transform and the shape of this rigid body. Then we can 
	//traverse the shape heirarchy relative to the rigid body and apply buoyancy calculations
	LTRigidTransform tBodyTransform;
	pILTPhysicsSim->GetRigidBodyTransform(hBody, tBodyTransform);

	HPHYSICSSHAPE hBodyShape;
	pILTPhysicsSim->GetRigidBodyShape(hBody, hBodyShape);

	//we can now create one of our buoyancy calculators and have it apply the calculations
	CBuoyancyCalculator Calculator(hBody, m_fDensity, vWSGravity, WSSurface, fElapsedS);

	//now traverse the shape and calculate buoyancy
	pILTPhysicsSim->TraverseShapeHeirarchy(hBodyShape, &Calculator, tBodyTransform);

	//release our shape now that we are done with it
	pILTPhysicsSim->ReleaseShape(hBodyShape);

	//now apply the drag on the rigid body
	ApplyDragForces(hBody, fElapsedS, Calculator.GetSurfaceArea(), Calculator.GetTotalMassKg());	
}

//called internally to apply drag forces. This is only done when applying buoyancy forces though
void CForceVolume::ApplyDragForces(HPHYSICSRIGIDBODY hBody, float fElapsedS, float fSurfaceArea, float fTotalMassKg) const
{
	//cache the physics simulation interface
	ILTPhysicsSim* pILTPhysicsSim = g_pLTBase->PhysicsSim();

	//determine the maximum drag force we can apply
	float fMaxForce = fTotalMassKg / fElapsedS;

	//and apply a force in the opposite direction of the object's velocity based upon the drag scale
	LTVector vVelocity;
	pILTPhysicsSim->GetRigidBodyVelocity(hBody, vVelocity);

	LTVector vAngVelocity;
	pILTPhysicsSim->GetRigidBodyAngularVelocity(hBody, vAngVelocity);

	//and now determine the forces, and apply them through the center of mass
	LTVector vCOM;
	pILTPhysicsSim->GetRigidBodyCenterOfMassInWorld(hBody, vCOM);

	// Clamp the force so we don't create a feedback loop with linear velocity.  
	float fForce = LTMIN( fSurfaceArea * m_fLinearDrag, fMaxForce );

	LTVector vForce = -vVelocity * fForce;
	if( LTIsNaN( vForce ) || vForce.MagSqr() > 1000000.0f * 1000000.0f )
	{
		LTERROR( "Invalid force detected." );
		vForce.Init( 0.0f, 10.0f, 0.0f );
	}

	pILTPhysicsSim->ApplyRigidBodyForceWorldSpace(hBody, vCOM, vForce);	

	// Clamp the force so we don't create a feedback loop with angular velocity.  
	fForce = LTMIN( fSurfaceArea * m_fAngularDrag, fMaxForce );

	//now dampen the angular velocity
	pILTPhysicsSim->ApplyRigidBodyTorque(hBody, -vAngVelocity * fForce);

	//and we are done
}

