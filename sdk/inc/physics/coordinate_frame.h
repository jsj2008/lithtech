#ifndef __COORDINATE_FRAME_H__
#define __COORDINATE_FRAME_H__

#ifndef __MATH_PHYS_H__
#include "math_phys.h"
#endif


//---------------------------------------------------------------------------//
/*!
The LTBasis data type uses a 3x3 orthogonal matrix to represent a local
orientation, and provides methods for rotation and transformation.

Used for: Math.
*/
class LTBasis
{
public:

	//!The columns of m_M are the base vectors.
	LTMatrix3f m_M;

public:

	//!The default constructor creates an identity LTBasis.
	LTBasis()
	{
		m_M.Identity();
	}

	/*!
	\param	q	A unit quaternion

	Construct from a unit quaternion.

	Used for: Math.
	*/
	LTBasis( const LTQuaternionf& q )
		: m_M(q)//convert to a matrix
	{}

	/*!
	\param	m	A 3x3 matrix

	Construct from a 3x3 matrix whose columns
	are the x,y and z local axes

	Used for: Math.
	*/
	LTBasis( const LTMatrix3f& m )
		: m_M(m)//base vectors in columns
	{}

	/*!
	\param	x	Local x-axis
	\param	y	Local y-axis
	\param	x	Local z-axis

	Construct the 3 local axes.

	Used for: Math.
	*/
	LTBasis( const LTVector3f& x, const LTVector3f& y, const LTVector3f& z )
		:	m_M(x, y, z)
	{}

	/*!
	\param	pitch	Rotation about world x-axis in radians
	\param	yaw		Rotation about world y-axis in radians
	\param	roll	Rotation about world z-axis in radians

	Given pitch, roll and yaw, construct the composite Euler rotation.

	Used for: Math.
	*/
	LTBasis( const float pitch, const float yaw, const float roll );

	/*!
	\param	f	The unnormalized "forward" reference direction
	\param	u	The unnormalized "upward" reference direction

	Given two unnormalized vectors \f${\bf f}\f$ and \f${\bf u}\f$,
	create a LTBasis that corresponds to the	orthonormal basis
	\f$\left\{ {{\bf R}^0 ,{\bf R}^1 ,{\bf R}^2} \right\}\f$, where

	\f[
		\begin{array}{l}
			{\bf R}^2  = \frac{ {\bf v}_f }{ ||{\bf v}_f|| }   \\
			{\bf R}^1  = \frac{{{\bf \hat v}_u  - ({\bf \hat v}_u
						\cdot {\bf R}^2 ){\bf R}^2 }}
						{{\left| {{\bf \hat v}_u  - ({\bf \hat v}_u
						\cdot {\bf R}^2 ){\bf R}^2 } \right|}} 	\\
			{\bf R}^0 = {\bf R}^1 \times {\bf R}^2	 \\
		\end{array}
	\f]

	Used for: Math.
	*/
	LTBasis( const LTVector3f& f, const LTVector3f& u );

	/*!
	\return		\b true if the LTBasis's are equal,
				\b false otherwise

	Used for: Math.
	*/
	bool operator == ( const LTBasis& B ) const
	{
		return	m_M == B.m_M;
	}

	/*!
	\param	B	An LTBasis

	Set the orientation.

	Used For: Math.
	*/
	void Orientation( const LTBasis& B )
	{
		this->m_M = B.m_M;
	}

	/*!
	Create an identity LTBasis.

	Used For: Math.
	*/
	void Init()
	{
		m_M.Identity();
	}

	/*!
	\return	the local x-axis.

	Used for: Math.
	*/
	const LTVector3f X() const
	{
		return m_M(0);
	}

	/*!
	\return the local y-axis.

	Used for: Math.
	*/
	const LTVector3f Y() const
	{
		return m_M(1);
	}

	/*!
	\return the local z-axis.

	Used for: Math.
	*/
	const LTVector3f Z() const
	{
		return m_M(2);
	}

	/*!
	Convert to a quaternion.

	Used for: Math.
	*/
	operator LTQuaternionf () const
	{
		return m_M;
	}

	/*!
	Convert to a 3x3 matrix.

	Used for: Math.
	*/
	operator LTMatrix3f() const
	{
		return m_M;
	}

	/*!
	\param	a	the angle through which to rotate
	\param	u	the unit axis about which to rotate

	Rotate through an angle \f$a\f$ about a unit axis
	\f$ {\bf \hat u} \f$.

	Used for: Math.
	*/
	void Rotate( const float a, const LTVector3f& u );

	/*!
	\param	v	the rotation "vector"

	Rotate through an angle \f$ a=||v|| \f$ about a unit axis
	\f$ {\bf \hat v} \f$.

	Used for: Math.
	*/
	void Rotate( const LTVector3f& v )
	{
		float a = v.Dot(v);//square of the angle

			if( 0 != a )//don't divide by 0
			{
				a = sqrtf( a );//angle through which to Rotate
				this->Rotate( a, v/a );//u = v/a
			}
	}

	/*!
	\param	a	the angle through which to rotate in radians

	Rotate this about its x-axis.

	Used for: Math.
	*/
	void RotateAboutX( const float a );

	/*!
	\param	a	the angle through which to rotate in radians

	Rotate this about its y-axis.

	Used for: Math.
	*/
	void RotateAboutY( const float a );

	/*!
	\param	a	the angle through which to rotate in radians

	Rotate this about its z-axis.

	Used for: Math.
	*/
	void RotateAboutZ( const float a );

	/*!
	\param	v	the vector to transform
	\return	\f$ {\bf v_{parent}} \f$

	Transform a vector \f$ {\bf v_{local}} \f$ from the local frame to the
	parent frame.

	Used for: Math.
	*/
	const LTVector3f TransformVectorToParent( const LTVector3f& v ) const
	{
		return m_M * v;
	}

	/*!
	\param	v	the vector to transform
	\return	\f$ {\bf v_{local}} \f$

	Transform a vector \f$ {\bf v_{parent}} \f$ from the parent frame to the
	local frame.

	Used for: Math.
	*/
	const LTVector3f TransformVectorToLocal( const LTVector3f& v ) const
	{
		return v * m_M;
	}

	/*!
	\param	B	another LTBasis

	Given two bases \b A and \b B both specified with respect to the
	same parent frame, transform \b A to be in \b B's local frame (so
	that a vector \b v in \b A's frame will be in \b B's frame after
	A.TransformVectorToParent( v ) is called).

	\see	LTBasis::TransformToParent()

	Used For: Math.
	*/
	const LTBasis TransformToLocal( const LTBasis& B ) const
	{
		//transform each base vector of A to B's local frame
		const LTMatrix3f M = B.m_M.Transpose() * m_M;

		return LTBasis( M );
	}

	/*!
	\param	B	another LTBasis

	Given a basis \b A specified with respect to the basis \b B,
	transform \b A to be in \b B's parent frame (so that a vector \b v in
	\b A's frame will be in \b B's parent frame after
	A.TransformVectorToParent( v ) is called).

	\see	LTBasis::TransformToLocal()

	Used For: Math.
	*/
	const LTBasis TransformToParent( const LTBasis& B ) const
	{
		//transform each base vector of A to B's parent frame
		const LTMatrix3f M = B.m_M * this->m_M;

		return LTBasis( M );
	}

	/*!
	\param	R0	the LTBasis at \f$u=0\f$
	\param	R1	the LTBasis at \f$u=1\f$
	\param	u	the interpolation parameter \f$ u \in [0,1] \f$
	\return	Ru	the interpolated LTBasis

	Given two LTBasis's \f${\bf R}_0\f$ and \f${\bf R}_1\f$ and
	a parameter \f$ u \in [0,1] \f$, spherically linearly interpolate
	an LTBasis \f${\bf a}_u\f$.

	\see Slerp()

	Used for: Math.
	*/
	friend const LTBasis Interpolate
	(
		const LTBasis& R0,
		const LTBasis& R1,
		const float u
	)
	{
		const LTQuaternionf q0 = R0.m_M;
		const LTQuaternionf q1 = R1.m_M;

		return Slerp( q0, q1, u );
	}

#ifdef __PHYS_RIGHT_HANDED_Z_UP__

//don't double-document this stuff
#ifndef DOXYGEN_SHOULD_SKIP_THIS
	void Init( const LTVector3f& r, const LTVector3f& f, const LTVector3f& u )
	{
		//convert basis vectors to a quaternion
		m_M = LTMatrix3f( r, f, u );
	}
	const LTVector3f Right() const		{ return X(); }
	const LTVector3f Forward() const	{ return Y(); }
	const LTVector3f Up() const			{ return Z(); }
	void RotateRight( const float a )	{ RotateAboutZ( -a ); }
	void RotateUp( const float a )		{ RotateAboutX( a ); }
	void RollClockwise( const float a )	{ RotateAboutY( a ); }
#endif

#else//left-handed, y-up

	/*!
	\param	r	right
	\param	f	forward
	\param	u	up

	Given the three orthonormal vectors \f$ \left\{ {\bf \hat r},{\bf \hat f},
	{\bf \hat u} \right\} \f$ initialize the local axes.

	Used for: Math.
	*/
	void Init( const LTVector3f& r, const LTVector3f& f, const LTVector3f& u )
	{
		//convert basis vectors to a quaternion
		m_M = LTMatrix3f( r, u, f );
	}

	/*!
	\return the local rightward direction.

	Used For: Math.
	*/
	const LTVector3f Right() const		{ return X(); }
	/*!
	\return the local forward direction.

	Used For: Math.
	*/
	const LTVector3f Forward() const	{ return Z(); }
	/*!
	\return the local upward direction.

	Used For: Math.
	*/
	const LTVector3f Up() const			{ return Y(); }

	/*!
	\param	a	the angle through which to rotate in radians

	Rotate about the upward direction.

	Used For: Math.
	*/
	void RotateRight( const float a )	{ RotateAboutY( a ); }
	/*!
	\param	a	the angle through which to rotate, radians

	Rotate about the rightward direction.

	Used For: Math.
	*/
	void RotateUp( const float a )		{ RotateAboutX( -a ); }
	/*!
	\param	a	the angle through which to rotate, radians

	Rotate about the forward direction.

	Used For: Math.
	*/
	void RollClockwise( const float a )	{ RotateAboutZ( -a ); }
#endif
};


//---------------------------------------------------------------------------//
/*!
The LTOrientation data type uses unit quaternion to represent a local
orientation, and provides methods for rotation and transformation.

Used for: Math.
*/
class LTOrientation
{
public:

	LTQuaternionf m_Q;//a unit quaternion

public:

	/*!
	The default constructor creates an identity transformation.
	*/
	LTOrientation()
		:	m_Q(1,0,0,0)
	{}

	/*!
	\param	q	a unit quaternion

	Construct from a unit quaternion.

	Used for: Math.
	*/
	LTOrientation( const LTQuaternionf& q )
		:	m_Q( q )
	{}

	/*!
	\param	m	a 3x3 matrix

	Construct from a 3x3 matrix whose columns
	are the x,y and z local axes

	Used for: Math.
	*/
	LTOrientation( const LTMatrix3f& m )
		:	m_Q( m )//convert to a quaternion
	{}

	/*!
	\param	R	A LTBasis

	Construct from a LTBasis object.

	Used for: Math.
	*/
	LTOrientation( const LTBasis& R )
		:	m_Q( R.m_M )//convert to a quaternion
	{}

	/*!
	\param	x	Local x-axis.
	\param	y	Local y-axis.
	\param	z	Local z-axis.

	Construct the 3 local axes.

	Used for: Math.
	*/
	LTOrientation( const LTVector3f& x, const LTVector3f& y, const LTVector3f& z )
		:	m_Q( LTMatrix3f(x,y,z) )//convert to a quaternion
	{}

	/*!
	\param	ax	The rotation about x, in radians.
	\param	ay	The rotation about y, in radians.
	\param	az	The rotation about z, in radians.

	Given three Euler rotations \f${\theta}_x\f$, \f${\theta}_y\f$, and
	\f${\theta}_z\f$ (in radians), construct the composite Euler rotation.

	Used for: Math.
	*/
	LTOrientation( const float ax, const float ay, const float az )
	{
		m_Q = LTBasis(ax,ay,az);//convert to a quaternion
	}

	/*!
	\param	f	The unnormalized "forward" reference direction.
	\param	u	The unnormalized "upward" reference direction.

	Given two unnormalized vectors \f${\bf f}\f$ and \f${\bf u}\f$,
	create a LTOrientation that corresponds to the	orthonormal basis
	\f$\left\{ {{\bf R}^0 ,{\bf R}^1 ,{\bf R}^2} \right\}\f$, where

	\f[
		\begin{array}{l}
			{\bf R}^2  = \frac{{{\bf v}_f }}{{\left|{{\bf v}_f }\right|}}   \\
			{\bf R}^1  = \frac{{{\bf \hat v}_u  - ({\bf \hat v}_u
					\cdot {\bf R}^2 ){\bf R}^2 }}
					{{\left| {{\bf \hat v}_u  - ({\bf \hat v}_u
					\cdot {\bf R}^2 ){\bf R}^2 } \right|}} 	\\
			{\bf R}^0 = {\bf R}^1 \times {\bf R}^2	 \\
		\end{array}
	\f]

	Used for: Math.
	*/
	LTOrientation( const LTVector3f& f, const LTVector3f& u );

	/*!
	\param	R	an LTOrientation

	Set the orientation.

	Used For: Math.
	*/
	void Orientation( const LTOrientation& R )
	{
		this->m_Q = R.m_Q;
	}

	/*!
	Create an identity orientation.

	Used For: Math.
	*/
	void Init()
	{
		//corresponds to identity
		this->m_Q.r = 1;
		this->m_Q.v.Init();
	}

	/*!
	\return the local x-axis.

	Used for: Math.
	*/
	const LTVector3f X() const
	{
		//NOTE:  optimizer eliminates these temps
		const float r = m_Q.r;
		const float x = m_Q.v.x;
		const float y = m_Q.v.y;
		const float z = m_Q.v.z;

		//column 0
		return LTVector3f( 1 - 2*y*y - 2*z*z,	2*x*y + 2*z*r, 2*x*z - 2*y*r );
	}

	/*!
	\return the local y-axis.

	Used for: Math.
	*/
	const LTVector3f Y() const
	{
		const float r = m_Q.r;
		const float x = m_Q.v.x;
		const float y = m_Q.v.y;
		const float z = m_Q.v.z;

		//column 1
		return LTVector3f( 2*x*y - 2*z*r, 1 - 2*x*x - 2*z*z,	2*y*z + 2*x*r );
	}

	/*!
	\return the local z-axis.

	Used for: Math.
	*/
	const LTVector3f Z() const
	{
		const float r = m_Q.r;
		const float x = m_Q.v.x;
		const float y = m_Q.v.y;
		const float z = m_Q.v.z;

		//column 2
		return LTVector3f( 2*x*z + 2*y*r, 2*y*z - 2*x*r, 1 - 2*x*x - 2*y*y );
	}

	/*!
	Convert to a quaternion.

	Used for: Math.
	*/
	operator LTQuaternionf () const
	{
		return m_Q;
	}

	/*!
	Convert to a 3x3 matrix.

	Used for: Math.
	*/
	operator LTMatrix3f() const
	{
		return m_Q;
	}

	/*!
	Convert to a LTBasis.

	Used for: Math.
	*/
	operator LTBasis() const
	{
		return LTBasis( m_Q );
	}

	/*!
	\return		\b true if the LTOrientation's are equal
				\b false otherwise

	Used for: Math.
	*/
	bool operator == ( const LTOrientation& q ) const
	{
		return	m_Q == q.m_Q;
	}

	/*!
	\param	a	The angle through which to rotate.
	\param	u	The unit axis about which to rotate.

	Rotate through an angle \f$ a \f$ about a unit axis
	\f$ {\bf \hat u} \f$.

	Used for: Math.
	*/
	void Rotate( const float a, const LTVector3f& u )
	{
		LTQuaternionf q( cosf(0.5f * a), u * sinf(0.5f * a) );

		m_Q = q * m_Q;
	}

	/*!
	\param	v	The rotation vector.

	Rotate through an angle \f$ a=||v|| \f$ about a unit axis
	\f$ {\bf \hat v} \f$.

	Used for: Math.
	*/
	void Rotate( const LTVector3f& v )
	{
		float a = v.Dot(v);//square of the angle

			if( 0 != a )//don't divide by 0
			{
				a = sqrtf( a );//angle through which to Rotate
				this->Rotate( a, v/a );//unit vector is axis
			}
	}

	/*!
	\param	a The angle through which to rotate, in radians.

	Rotate this about its x-axis.

	Used for: Math.
	*/
	void RotateAboutX( const float a )
	{
		this->Rotate( a, this->X() );
	}

	/*!
	\param	a	The angle through which to rotate, in radians.

	Rotate this about its y-axis.

	Used for: Math.
	*/
	void RotateAboutY( const float a )
	{
		this->Rotate( a, this->Y() );
	}

	/*!
	\param	a	The angle through which to rotate, in radians.

	Rotate this about its z-axis.

	Used for: Math.
	*/
	void RotateAboutZ( const float a )
	{
		this->Rotate( a, this->Z() );
	}

	/*!
	\param	v	the vector to transform
	\return	\f$ {\bf v_{parent}} \f$

	Transform a vector \f$ {\bf v_{local}} \f$ from the local frame to the
	parent frame.

	Used for: Math.
	*/
	const LTVector3f TransformVectorToParent( const LTVector3f& v ) const
	{
		return m_Q * v * m_Q.Conjugate();
	}

	/*!
	\param	v	the vector to transform
	\return	\f$ {\bf v_{local}} \f$

	Transform a vector \f$ {\bf v_{parent}} \f$ from the parent frame to the
	local frame.

	Used for: Math.
	*/
	const LTVector3f TransformVectorToLocal( const LTVector3f& v ) const
	{
		return m_Q.Conjugate() * v * m_Q;
	}

	/*!
	\param	B	another LTOrientation

	Given two orientations \b A and \b B both specified with respect to the
	same parent frame, transform \b A to be in \b B's local frame (so
	that a vector \b v in \b A's frame will be in \b B's frame after
	A.TransformVectorToParent( v ) is called).

	\see	LTOrientation::TransformToParent()

	Used For: Math.
	*/
	const LTOrientation TransformToLocal( const LTOrientation& B ) const
	{
		const LTQuaternionf q = B.m_Q.Conjugate() * this->m_Q;

		return LTOrientation( q );
	}

	/*!
	\param	B	another LTOrientation

	Given an orientation \b A specified with respect to the orientation \b B,
	transform \b A to be in \b B's parent frame (so that a vector \b v in
	\b A's frame will be in \b B's parent frame after
	A.TransformVectorToParent( v ) is called).

	\see	LTOrientation::TransformToLocal()

	Used For: Math.
	*/
	const LTOrientation TransformToParent( const LTOrientation& B ) const
	{
		const LTQuaternionf q = B.m_Q * this->m_Q;

		return LTOrientation( q );
	}

	/*!
	\param	R0	the LTOrientation at \f$u=0\f$
	\param	R1	the LTOrientation at \f$u=1\f$
	\param	u	the interpolation parameter \f$ u \in [0,1] \f$
	\return	Ru	the interpolated LTOrientation

	Given two LTOrientation's \f${\bf R}_0\f$ and \f${\bf R}_1\f$ and
	a parameter \f$ u \in [0,1] \f$, spherically linearly interpolate
	an LTOrientation \f${\bf a}_u\f$.

	\see Slerp()

	Used for: Math.
	*/
	friend const LTOrientation Interpolate
	(
		const LTOrientation& R0,
		const LTOrientation& R1,
		const float u
	)
	{
		return Slerp( R0.m_Q, R1.m_Q, u );
	}

#ifdef __PHYS_RIGHT_HANDED_Z_UP__

//don't double-document this stuff
#ifndef DOXYGEN_SHOULD_SKIP_THIS
	void Init( const LTVector3f& r, const LTVector3f& f, const LTVector3f& u )
	{
		//convert basis vectors to a quaternion
		m_Q = LTMatrix3f( r, f, u );
	}

	const LTVector3f Right() const		{ return X(); }
	const LTVector3f Forward() const		{ return Y(); }
	const LTVector3f Up() const			{ return Z(); }
	void RotateRight( const float a )	{ RotateAboutZ( -a ); }
	void RotateUp( const float a )		{ RotateAboutX( a ); }
	void RollClockwise( const float a )	{ RotateAboutY( a ); }
#endif

#else//left-handed, y-up

	/*!
	\param	r	right
	\param	f	forward
	\param	u	up

	Given the three orthonormal vectors \f$ \left\{ {\bf \hat r},{\bf \hat f},
	{\bf \hat u} \right\} \f$ initialize the local axes.

	Used for: Math.
	*/
	void Init( const LTVector3f& r, const LTVector3f& f, const LTVector3f& u )
	{
		//convert basis vectors to a quaternion
		m_Q = LTMatrix3f( r, u, f );
	}

	/*!
	\return the rightward direction.

	Used For: Math.
	*/
	const LTVector3f Right() const		{ return X(); }
	/*!
	\return the forward direction.

	Used For: Math.
	*/
	const LTVector3f Forward() const	{ return Z(); }
	/*!
	\return the upward direction.

	Used For: Math.
	*/
	const LTVector3f Up() const			{ return Y(); }

	/*!
	\param a the angle through which to rotate in radians

	Rotate about the upward direction.

	Used For: Math.
	*/
	void RotateRight( const float a )	{ RotateAboutY( a ); }
	/*!
	\param a the angle through which to rotate in radians

	Rotate about the rightward direction.

	Used For: Math.
	*/
	void RotateUp( const float a )		{ RotateAboutX( -a ); }
	/*!
	\param a the angle through which to rotate in radians

	Rotate about the forward direction.

	Used For: Math.
	*/
	void RollClockwise( const float a )	{ RotateAboutZ( -a ); }

#endif
};


//---------------------------------------------------------------------------//
/*!
The LTCoordinateFrame data type represents a local coordinate frame, and
provides methods for rotations, translations and transformations.
*/
template< class T >
class LTCoordinateFrame : public T
{
public:

	//!The origin (position) with respect to the parent frame.
	LTVector3f m_O;

public:

	LTCoordinateFrame()
		:	m_O(0,0,0)
	{}

	/*!
	\param	o	the origin (position)
	\param	R	the orientation

	Construct from a positon and a LTBasis.

	Used for: Math.
	*/
	LTCoordinateFrame( const LTVector3f& o, const LTBasis& R )
		:	T( R.m_M ), m_O( o )
	{}

	/*!
	\param	o	the origin (position)
	\param	R	the orientation

	Construct from a positon and a LTOrientation.

	Used for: Math.
	*/
	LTCoordinateFrame( const LTVector3f& o, const LTOrientation& R )
		:	T( R.m_Q ), m_O( o )
	{}

	/*!
	\param	o	local origin (position)
	\param	R0	local x-axis
	\param	R1	local y-axis
	\param	R2	local z-axis

	Construct from a positon and 3 orthonormal vectors.

	Used for: Math.
	*/
	LTCoordinateFrame
	(
		const LTVector3f&	o,	//origin
		const LTVector3f&	R0,	//first basis vector
		const LTVector3f&	R1,	//second basis vector
		const LTVector3f&	R2	//third basis vector
	)
		:	T( R0, R1, R2 ), m_O( o )
	{}

	/*!
	\return		the position of the coordinate frame

	Used For: Math.
	*/
	const LTVector3f& Position() const
	{
		return m_O;
	}

	/*!
	\param	p	a position

	Set the position of the coordinate frame.

	Used For: Math.
	*/
	void Position( const LTVector3f& p )
	{
		m_O = p;
	}

	/*!
	\param	p	a point in the parent frame

	Transform \b p to the local frame.

	Used For: Math.
	*/
	const LTVector3f TransformPointToLocal(  const LTVector3f& p ) const
	{
		//Translate to this frame's origin, then project onto this LTBasis
		return TransformVectorToLocal( p - m_O );
	}

	/*!
	\param	p	a point in the local frame

	Transform \b p to the parent frame.

	Used For: Math.
	*/
	const LTVector3f TransformPointToParent( const LTVector3f& p ) const
	{
		//transform the coordinate vector and Translate by this origin
		return TransformVectorToParent( p ) + m_O;
	}

	/*!
	\param	B	another LTCoordinateFrame

	Given two frames \b A and \b B both specified with respect to the
	same parent frame, transform \b A to be in \b B's local frame (so
	that a point \b p in \b A's frame will be in \b B's frame after
	A.TransformPointToParent( p ) is called).

	\see	LTCoordinateFrame::TransformToParent()

	Used For: Math.
	*/
	const LTCoordinateFrame TransformToLocal( const LTCoordinateFrame& B ) const
	{
		//origin and orientation with respect to local coordinates
		const LTVector3f O = B.TransformPointToLocal( m_O );
		const T R = B.T::TransformToLocal( *this );//properly resolve function name

		return LTCoordinateFrame( O, R );
	}

	/*!
	\param	B	another LTCoordinateFrame

	Given a coordinate frame \b A specified with respect to the frame \b B,
	transform \b A to be in \b B's parent frame (so that a point \b p in
	\b A's frame will be in \b B's parent frame after
	A.TransformPointToParent( p ) is called).

	\see	LTCoordinateFrame::TransformToLocal()

	Used For: Math.
	*/
	const LTCoordinateFrame TransformToParent( const LTCoordinateFrame& B ) const
	{
		//origin and orientation with respect to local coordinates
		const LTVector3f O = B.TransformPointToParent( m_O );
		const T R = B.T::TransformToParent( *this );//properly resolve function name

		return LTCoordinateFrame( O, R );
	}

	/*!
	\param	v	translation vector

	Translate the position of the frame by the given vector.

	Used For: Math.
	*/
	void Translate( const LTVector3f& v )
	{
		m_O += v;
	}

	//NOTE:  For some reason, the MS compiler can't resolve
	//the two functions below, even though it can resolve
	//RotateAboutX(), etc.
	/*!
	\param	a	The angle through which to rotate.
	\param	u	The unit axis about which to rotate.

	Rotate through an angle \f$ a \f$ about a unit axis
	\f$ {\bf \hat u} \f$.

	Used for: Math.
	*/
	void Rotate( const float a, const LTVector3f& u )
	{
		//rotate the local axes
		T::Rotate( a, u );
	}
	/*!
	\param	v	The rotation vector.

	Rotate through an angle \f$ a=||v|| \f$ about a unit axis
	\f$ {\bf \hat v} \f$.

	Used for: Math.
	*/
	void Rotate( const LTVector3f& v )
	{
		T::Rotate( v );
	}

	/*!
	\param	a	the angle through which to rotate this frame
	\param	u	the axis about which to rotate this frame
	\param	p0	a point on the line about which to rotate this frame

	Rotate this coordinate frame through and angle \f$a\f$ about a line
	passing through \f$ {\bf p}_0 \f$ in the direction of
	\f$ \hat {\bf u} \f$.

	Used For: Math.
	*/
	void Rotate( const float a, const LTVector3f& u, const LTVector3f& p0 )
	{
		//rotate the position of the coordinate frame about the axis
		const LTMatrix3f M = RotationMatrix( a, u );
		const LTMatrix3f v = M * (this->m_O - p0);

		this->m_O = p0 + (const LTVector3f)v;//new position

		//rotate the local axes
		this->Rotate( a, u );
	}

	/*!
	\param v	
	\param p0	

	Rotate this coordinate frame through and angle \f$a=||{\bf v}||\f$ about
	a line passing through \f$ {\bf p}_0 \f$ in the direction of
	\f$ \hat{\bf u}=\frac{ {\bf v} }{ ||{\bf v}|| }\f$.

	Used For: Math.
	*/
	void Rotate( const LTVector3f& v, const LTVector3f& p0 )
	{
		float a = v.Dot(v);//square of the angle

			if( 0 != a )//don't divide by 0
			{
				a = sqrtf( a );//angle through which to Rotate
				this->Rotate( a, v/a, p0 );//u = v/a
			}
	}

	//given u \in [0,1], linearly interpolate between F0 and F1
	friend const LTCoordinateFrame Interpolate
	(
		const LTCoordinateFrame& F0,
		const LTCoordinateFrame& F1,
		const float u
	)
	{
		const LTVector3f O = Lerp( F0.m_O, F1.m_O, u );
		//NOTE:  cast allows proper resolution of Interpolate(),
		//otherwise infinite recursion occurs
		const T R = Interpolate( (T&)F0, (T&)F1, u );

		return LTCoordinateFrame( O, R );
	}
};


/*!
Lithtech defines two types of coordinate frame data types:  LTCoordinateFrameM,
which derives from LTBasis and thus uses a 3x3 matrix to store orientation,
and LTCoordinateFrameQ, which derives from LTOrientation and thus uses a unit
quaternion to store orientation.  LTCoordinateFrameM transforms vectors and
points more quickly, but uses 48 bytes.  LTCoordinateFrameQ uses only 28 bytes
but transforms points and vectors less quickly.
*/
typedef	LTCoordinateFrame<LTBasis>			LTCoordinateFrameM;
typedef	LTCoordinateFrame<LTOrientation>	LTCoordinateFrameQ;


#endif
//EOF
