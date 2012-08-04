#ifndef __RIGID_BODY_H__
#define __RIGID_BODY_H__

#ifndef __COORDINATE_FRAME_H__
#include "coordinate_frame.h"
#endif


//---------------------------------------------------------------------------//
/*!
The LTRigidBodyState data type describes the position, orientation, velocity
and angular velocity of a rigid body.

Used For: Physics.
*/
class LTRigidBodyState : public LTCoordinateFrameQ
{
public:

	/*! The linear velocity of the body */
	LTVector3f m_V;
	/*! The angular velocity of the body */
	LTVector3f m_W;

public:

	LTRigidBodyState()
		:	m_V(0,0,0), m_W(0,0,0)
	{}

	/*!
	\param v	Velocity
	\param w	Angular velocity

	Used For: Physics.
	*/
	LTRigidBodyState( const LTVector3f& v, const LTVector3f& w )
		:	m_V(v), m_W(w)
	{}

	/*!
	\param	v	the rotation "vector"

	Rotate through an angle \f$ a=||v|| \f$ about a unit axis
	\f$ {\bf \hat v} \f$.

	Used for: Physics.
	*/
	void Rotate( const LTVector3f& v )
	{
		LTOrientation::Rotate( v );
	}
};


//---------------------------------------------------------------------------//
/*!
The LTRigidBody data type describes the mass and principal moments of inertia
of a rigid body, as well as its position, orientation, velocity and angular
velocity.

Used For: Physics.
*/
class LTRigidBody : public LTRigidBodyState
{
public:

	/*! The mass of the body */
	float		m_Mass;
	/*! The principal moments of inertia of the body */
	LTVector3f	m_Ip;

public:

	/*!
	The default constructor initializes \f$ mass=1 \f$ and
	\f$ {\bf I}_p=(1/12,1/12,1/12) \f$, corresponding to a box whose
	dimensions are 1 on each side.

	Used For: Physics.
	*/
	LTRigidBody()
		: m_Mass(1), m_Ip(1.f/12,1.f/12,1.f/12)
	{}

	/*!
	\param m	Mass, in kg.
	\param ip	Principal moments of inertia in \f$ kg \dot m^2 \f$
	\param v	Velocity, in meters/second.
	\param w	Angular velocity, in radians/second.

	Used For: Physics.
	*/
	LTRigidBody
	(
		const float			m,	
		const LTVector3f&	ip,	
		const LTVector3f&	v,	
		const LTVector3f&	w	
	)
		:	LTRigidBodyState(v,w),
			m_Mass(m),
			m_Ip(ip)
	{}
};


//---------------------------------------------------------------------------//
/*!
\param	R	A 3x3 matrix whose column vectors are the local axes of the body.
\param	Ip	Principal moments of inertia in \f$ kg \dot m^2 \f$
\return	\f$ {\bf I}={\bf R}{\bf I}_p{\bf R}^T \f$

Given the principal moments of inertia \f$ {\bf I}_p \f$ and an orientation
matrix R of a rigid body, calculate its symmetric inertia tensor.

Used For: Physics.
*/
const LTSymMat3f CalculateInertiaTensor( const LTMatrix3f& R, const LTVector3f& Ip );


//---------------------------------------------------------------------------//
/*!
\param	R	A 3x3 matrix whose column vectors are the local axes of the body.
\param	Ip	Principal moments of inertia in \f$ kg \dot m^2 \f$
\return	\f$ {\bf I}^{-1}={\bf R}{\bf I}_p^{-1}{\bf R}^T \f$

Given the principal moments of inertia \f$ {\bf I}_p \f$ and an orientation
matrix R of a rigid body, calculate its inverse inertia tensor.

Used For: Physics.
*/
const LTSymMat3f CalculateInverseInertiaTensor( const LTMatrix3f& R, const LTVector3f& Ip );


//---------------------------------------------------------------------------//
/*!
\param m	mass, in kg
\param d	dimensions, in meters
\return		\f$ {\bf I}_p=frac{m}{12}(d_yd_y + d_zd_z,d_xd_x + d_zd_z,
			d_xd_x + d_yd_y) \f$

Calculate the principal moments of inertia for a box with mass \b m and
dimensions \b d.

Used For: Physics.
*/
inline const LTVector3f Ip_Box( const float m, const LTVector3f& d )
{
	LTVector3f ip;

	ip.x = d.y*d.y + d.z*d.z;
	ip.y = d.x*d.x + d.z*d.z;
	ip.z = d.x*d.x + d.y*d.y;

	return (m/12) * ip;
}


#ifndef DOXYGEN_SHOULD_SKIP_THIS
//---------------------------------------------------------------------------//
/*!
\param I	Inertia tensor.
\param Inv	The inverse of the interia tensor.
\param T	Torque.
\param W	Angular velocity.
\return		Newton-Euler calculation of frac{d{\bf \omega}}{dt}.

Calculate the time derivative of angular velocity for a rigid body:
\f$ frac{d{\bf \omega}}{dt} =
{\bf I}^{-1}[{\bf \tau} - {\bf \omega}\times({\bf I}{\bf \omega}) ] \f$.

Used For: Physics.
*/
inline const LTVector3f dWdt
(
	const LTSymMat3f& I,
	const LTSymMat3f& Inv,
	const LTVector3f& T,
	const LTVector3f& W
)
{
	return Inv * (T - W.Cross(I*W));
}
#endif//doxygen


//---------------------------------------------------------------------------//
/*!
\param	cor		restitution coefficient \f$ \in [0,1] \f$
\param	pc		contact point
\param	n		contact surface normal
\param	m		mass
\param	p		position
\param	v		velocity
\param	w		angular velocity
\param	Inv		inverse inertia tensor
\return			\b J, the impulse vector

Calculate the collision impulse \b J for a rigid body colliding with a
static (immovable) object.

Used For: Physics.
*/
inline const LTVector3f Impulse
(
	const float			cor,	//coefficient of restitution
	const LTVector3f&	pc,		//contact point
	const LTVector3f&	n,		//contact surface normal
	const float			m,		//mass
	const LTVector3f&	p,		//position
	const LTVector3f&	v,		//velocity
	const LTVector3f&	w,		//angular velocity
	const LTSymMat3f&	Inv	//inverse inertia tensor
)
{
	//NOTE:  This impulse formula is taken from David Baraff's "An
	//Introduction to Physically Based Modeling:  Rigid Body Simulation
	//II - Nonpenetration Constraints", p. G47

	//vector from center of mass to contact point
	const LTVector3f r = pc - p;
	//velocity relative to contact point
	const LTVector3f v_rel = v + w.Cross(r);
	const float s = - n.Dot( r.Cross(Inv * r.Cross(n)) );
	const float j = -(1 + cor) * n.Dot(v_rel) / (1/m + s);

	return j * n;
}


//---------------------------------------------------------------------------//
/*!
\param	cor		restitution coefficient \f$ \in [0,1] \f$
\param	pc		contact point
\param	n		A's contact surface normal
\param	ma		A's mass
\param	pa		A's position
\param	va		A's velocity
\param	wa		A's angular velocity
\param	InvA	A's inverse inertia tensor
\param	mb		B's mass
\param	pb		B's position
\param	vb		B's velocity
\param	wb		B's angular velocity
\param	InvB	B's inverse inertia tensor
\return			\b J, the impulse vector

Calculate the collision impulse \b J for two colliding rigid bodies.

Used For: Physics.
*/
inline const LTVector3f Impulse
(
	const float			cor,	//coefficient of restitution
	const LTVector3f&	pc,		//contact point
	const LTVector3f&	n,		//A's contact surface normal
	const float			ma,		//A's mass
	const LTVector3f&	pa,		//A's position
	const LTVector3f&	va,		//A's velocity
	const LTVector3f&	wa,		//A's angular velocity
	const LTSymMat3f&	InvA,	//A's inverse inertia tensor
	const float			mb,		//B's mass
	const LTVector3f&	pb,		//B's position
	const LTVector3f&	vb,		//B's velocity
	const LTVector3f&	wb,		//B's angular velocity
	const LTSymMat3f&	InvB	//B's inverse inertia tensor
)
{
	//NOTE:  This impulse formula is taken from David Baraff's "An
	//Introduction to Physically Based Modeling:  Rigid Body Simulation
	//II - Nonpenetration Constraints", p. G47

	//vector from center of mass to contact point
	const LTVector3f ra = pc - pa;
	const LTVector3f rb = pc - pb;
	//velocity of contact points
	const LTVector3f vA = va + wa.Cross(ra);
	const LTVector3f vB = vb + wb.Cross(rb);
	//A's velocity relative to B at the contact point
	const LTVector3f v_rel = vA - vB;
	const float sa = - n.Dot( ra.Cross(InvA * ra.Cross(n)) );
	const float sb = - n.Dot( rb.Cross(InvB * rb.Cross(n)) );
	const float j = -(1+cor)*n.Dot(v_rel) / (1/ma + 1/mb + sa + sb);

	return j * n;
}


#endif

//EOF
