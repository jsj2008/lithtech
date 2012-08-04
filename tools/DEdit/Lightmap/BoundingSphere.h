#ifndef __BOUNDINGSPHERE_H__
#define __BOUNDINGSPHERE_H__

class CBoundingSphere
{
public:

	CBoundingSphere() :
		m_fRadius(0)
	{
	}

	CBoundingSphere(const LTVector& vPos, float fRadius)
	{
		Init(vPos, fRadius);
	}

	//initializes a bounding sphere based upon the center and radius
	void Init(const LTVector& vPos, float fRadius)
	{
		SetPos(vPos);
		SetRadius(fRadius);
	}

	//accessors for setting
	void SetPos(const LTVector& vPos)
	{
		m_vPos = vPos;
	}

	void SetRadius(float fRad)
	{
		m_fRadius = fRad;
	}

	//accessors for getting
	float GetRadius() const
	{
		return m_fRadius;
	}

	float GetRadiusSqr() const
	{
		return m_fRadius * m_fRadius;
	}

	const LTVector&	GetPos() const
	{
		return m_vPos;
	}

	//collision functions

	//determines if two spheres intersect given their center points and radii
	inline bool IntersectsSphere(const LTVector& vO, float fR);

	//determines if two spheres intersect given their center points and radii
	inline bool IntersectsSphere(const CBoundingSphere& Other)
	{
		return IntersectsSphere(Other.GetPos(), Other.GetRadius());
	}

	//Calculates whether or not the specified sphere intersects a specified segment.
	// The segment is given as an origin, a normalized direction, and length of
	// the segment. 
	inline bool IntersectsSegment(	const LTVector& vRayO,
									const LTVector& vRayD,
									float fRayLen);

	//Calculates whether or not the specified sphere intersects a specified ray.
	// the ray is given by an origin and a normalized direction vector
	inline bool IntersectsRay(	const LTVector& vRayO,
								const LTVector& vRayD);


private:

	LTVector	m_vPos;
	float		m_fRadius;

};




//determines if two spheres intersect given their center points and radii
inline bool CBoundingSphere::IntersectsSphere(const LTVector& vO, float fR)
{
	//find the distance squared
	float fDistSqr = (m_vPos.x - vO.x) * (m_vPos.x - vO.x) +
					 (m_vPos.y - vO.y) * (m_vPos.y - vO.y) +
					 (m_vPos.z - vO.z) * (m_vPos.z - vO.z);

	//now see if it is closer than (fR1 + fR2)^2
	if(fDistSqr > (m_fRadius + fR) * (m_fRadius + fR))
	{
		return false;
	}

	return true;
}

								

//Calculates whether or not the specified sphere intersects a specified segment.
// The segment is given as an origin, a normalized direction, and length of
// the segment. 
inline bool CBoundingSphere::IntersectsSegment(	const LTVector& vRayO,
												const LTVector& vRayD,
												float fRayLen)
{
	//find a vector to the center of the sphere
	LTVector vToSphere = m_vPos - vRayO;

	//find the distance to the center of the sphere
	float fDistToCenterSqr = vToSphere.MagSqr();

	//the distance squared
	float fDistSqr = GetRadiusSqr();

	//check to see if we are inside the sphere
	if(fDistToCenterSqr < fDistSqr)
	{
		//we are inside the sphere
		return true;
	}

	//find the distance to the center of the sphere projected onto the
	//top distance
	float fToCenterProj = vToSphere.Dot(vRayD);

	//if the distance projected is negative, it is behind our start (and
	//is therefore invalid since we already bailed if inside)
	if(fToCenterProj < 0.0f)
	{
		//the sphere is behind us
		return false;
	}

	//find the distance from the center to the ray
	float fDistToRaySqr = fDistToCenterSqr - fToCenterProj * fToCenterProj;

	//bail if the sphere doesn't come close enough to the ray
	if(fDistToRaySqr > fDistSqr)
	{
		//sphere is too far away from the line
		return false;
	}

	//now determine if the sphere intersects on the segment t E [0..RayLen]

	//quick test to see if center is within range
	if(fToCenterProj < fRayLen)
	{
		//hits the segment
		return true;
	}

	//this is now the tricky case where the sphere hits the ray on the forward part
	//but the projected center is past the ray length, so it could possibly intersect
	//the segment up to the radius, or not at all. Find the distance from the endpoint
	//to the center and check against the radius
	if((fToCenterProj - fRayLen) * (fToCenterProj - fRayLen) + fDistToRaySqr > fDistSqr)
	{
		//too far away
		return false;
	}

	//we have an intersection
	return true;
}

//Calculates whether or not the specified sphere intersects a specified ray.
// the ray is given by an origin and a normalized direction vector
bool CBoundingSphere::IntersectsRay(const LTVector& vRayO, const LTVector& vRayD)
{
	//find a vector to the center of the sphere
	LTVector vToSphere = m_vPos - vRayO;

	//find the distance to the center of the sphere
	float fDistToCenterSqr = vToSphere.MagSqr();

	//the distance squared
	float fDistSqr = GetRadiusSqr();

	//check to see if we are inside the sphere
	if(fDistToCenterSqr < fDistSqr)
	{
		//we are inside the sphere
		return true;
	}

	//find the distance to the center of the sphere projected onto the
	//top distance
	float fToCenterProj = vToSphere.Dot(vRayD);

	//if the distance projected is negative, it is behind our start (and
	//is therefore invalid since we already bailed if inside)
	if(fToCenterProj < 0.0f)
	{
		//the sphere is behind us
		return false;
	}

	//find the distance from the center to the ray
	float fDistToRaySqr = fDistToCenterSqr - fToCenterProj * fToCenterProj;

	//bail if the sphere doesn't come close enough to the ray
	if(fDistToRaySqr > fDistSqr)
	{
		//sphere is too far away from the line
		return false;
	}

	//we have an intersection
	return true;
}

#endif
