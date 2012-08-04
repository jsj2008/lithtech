#ifndef __RAGDOLLCONSTRAINT_H__
#define __RAGDOLLCONSTRAINT_H__

class CRagDoll;
class CRagDollNode;
typedef uint32 HRAGDOLLNODE;

class CRagDollConstraint
{
public:

	CRagDollConstraint()			{}
	virtual ~CRagDollConstraint()	{}

	//determines if this constraint is valid or not
	virtual bool	IsValid() const = 0;

	//called to apply the constraint
	virtual void	Apply(uint32 nPosIndex)	= 0;

	//clones the constraint, allocating a new one with the standard C++ allocator
	virtual CRagDollConstraint* Clone() const = 0;
};

//This is the standard distance constraint that keeps nodes a certain distance apart from one another
class CRagDollDistanceConstraint :
	public CRagDollConstraint
{
public:

	CRagDollDistanceConstraint(CRagDoll* pRagDoll, HRAGDOLLNODE hNode1, HRAGDOLLNODE hNode2, float fDistance = -1.0f, float fNode1Weight = -1.0f);
	CRagDollDistanceConstraint(const CRagDollDistanceConstraint& rhs);

	virtual void				Apply(uint32 nPosIndex);
	virtual bool				IsValid() const;
	virtual CRagDollConstraint* Clone() const;


private:

	CRagDollNode*	m_pNode[2];
	float			m_fDistance;
	float			m_fNode1Weight;
	float			m_fNode2Weight;
};

//This is the minimum distance constraint, meaning that nodes must be at least N units apart
// it doesn't care about max distances
class CRagDollMinDistanceConstraint :
	public CRagDollConstraint
{
public:

	CRagDollMinDistanceConstraint(CRagDoll* pRagDoll, HRAGDOLLNODE hNode1, HRAGDOLLNODE hNode2, float fMinDist = -1.0f, float fNode1Weight = -1.0f);
	CRagDollMinDistanceConstraint(const CRagDollMinDistanceConstraint& rhs);

	virtual void				Apply(uint32 nPosIndex);
	virtual bool				IsValid() const;
	virtual CRagDollConstraint* Clone() const;


private:

	CRagDollNode*	m_pNode[2];
	float			m_fDistance;
	float			m_fNode1Weight;
	float			m_fNode2Weight;
};

//This is the maximum distance constraint, meaning that nodes must be at most N units apart
// it doesn't care about min distances
class CRagDollMaxDistanceConstraint :
	public CRagDollConstraint
{
public:

	CRagDollMaxDistanceConstraint(CRagDoll* pRagDoll, HRAGDOLLNODE hNode1, HRAGDOLLNODE hNode2, float fMaxDist = -1.0f, float fNode1Weight = -1.0f);
	CRagDollMaxDistanceConstraint(const CRagDollMaxDistanceConstraint& rhs);

	virtual void				Apply(uint32 nPosIndex);
	virtual bool				IsValid() const;
	virtual CRagDollConstraint* Clone() const;


private:

	CRagDollNode*	m_pNode[2];
	float			m_fDistance;
	float			m_fNode1Weight;
	float			m_fNode2Weight;
};

//This will take two segments, from this create a plane. It will then create a plane using the first
//segment and the plane normal and constrain the third segment to that plane. This is useful for
//hinge joints in arms and legs
class CRagDollInPlaneConstraint :
	public CRagDollConstraint
{
public:

	CRagDollInPlaneConstraint(CRagDoll* pRagDoll, HRAGDOLLNODE hHinge, HRAGDOLLNODE hFirstPlaneCenter, HRAGDOLLNODE hFirstPlaneOther, HRAGDOLLNODE hEndFinalSeg, float fNormalScale, float fTolerance);
	CRagDollInPlaneConstraint(const CRagDollInPlaneConstraint& rhs);

	virtual void				Apply(uint32 nPosIndex);
	virtual bool				IsValid() const;
	virtual CRagDollConstraint* Clone() const;


private:

	CRagDollNode*	m_pHinge;
	CRagDollNode*	m_pPlaneCenter;
	CRagDollNode*	m_pPlaneOther;
	CRagDollNode*	m_pConstrain;
	float			m_fNormalScale;
	float			m_fTolerance;
};

//Given two points it will form a plane passing through the first point, and ensure that the specified point
//is below that plane
class CRagDollAbovePlaneConstraint :
	public CRagDollConstraint
{
public:

	CRagDollAbovePlaneConstraint(CRagDoll* pRagDoll, HRAGDOLLNODE hPtInPlane, HRAGDOLLNODE hPtOnNormal, HRAGDOLLNODE hConstrain, float fNormalScale = 1.0f, float fPlaneOffset = 0.0f);
	CRagDollAbovePlaneConstraint(const CRagDollAbovePlaneConstraint& rhs);

	virtual void				Apply(uint32 nPosIndex);
	virtual bool				IsValid() const;
	virtual CRagDollConstraint* Clone() const;


private:

	float			m_fNormalScale;
	float			m_fPlaneOffset;
	CRagDollNode*	m_pPtInPlane;
	CRagDollNode*	m_pPtOnNormal;
	CRagDollNode*	m_pConstrain;
};

//Same as above, but plane is specified with 3 points in the plane
class CRagDollAbovePlane3Constraint :
	public CRagDollConstraint
{
public:

	CRagDollAbovePlane3Constraint(CRagDoll* pRagDoll, HRAGDOLLNODE hPt1, HRAGDOLLNODE hPt2, HRAGDOLLNODE hPt3, HRAGDOLLNODE hConstrain, float fNormalScale = 1.0f, float fOffset = 0.0f);
	CRagDollAbovePlane3Constraint(const CRagDollAbovePlane3Constraint& rhs);

	virtual void				Apply(uint32 nPosIndex);
	virtual bool				IsValid() const;
	virtual CRagDollConstraint* Clone() const;


private:

	float			m_fNormalScale;
	float			m_fOffset;
	CRagDollNode*	m_pPt1;
	CRagDollNode*	m_pPt2;
	CRagDollNode*	m_pPt3;
	CRagDollNode*	m_pConstrain;
};

//Simlar to above, it forms a plane from the 3 points, and then finds a perpindicular plane that
//lies along the first edge
class CRagDollAbovePlaneOnEdgeConstraint :
	public CRagDollConstraint
{
public:

	CRagDollAbovePlaneOnEdgeConstraint(CRagDoll* pRagDoll, HRAGDOLLNODE hPt1, HRAGDOLLNODE hPt2, HRAGDOLLNODE hPt3, HRAGDOLLNODE hConstrain, float fNormalScale = 1.0f);
	CRagDollAbovePlaneOnEdgeConstraint(const CRagDollAbovePlaneOnEdgeConstraint& rhs);

	virtual void				Apply(uint32 nPosIndex);
	virtual bool				IsValid() const;
	virtual CRagDollConstraint* Clone() const;


private:

	float			m_fNormalScale;
	CRagDollNode*	m_pPt1;
	CRagDollNode*	m_pPt2;
	CRagDollNode*	m_pPt3;
	CRagDollNode*	m_pConstrain;
};

#endif
