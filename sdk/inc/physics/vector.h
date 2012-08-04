#ifndef __VECTOR_H__
#define __VECTOR_H__


#ifndef __CMATH__
#include <cmath>
#define __CMATH__
#endif

#ifndef __LTINTEGER_H__
#include "ltinteger.h"
#endif

#ifndef __LTSYSOPTIM_H__
#include "ltsysoptim.h"
#endif


//---------------------------------------------------------------------------//
/*!
The LTVector2 template allows the application to define a 2-dimensional vector
with arbitrary component types.  Currently, three overloads are provided:
LTVector2f (whose components are \b float's), LTVector2d (whose components are
\b double's), and LTVector2us (whose components are \b uint16's)

Used for: Math.
*/
template< class T >
class LTVector2
{
public:
	/*!
	The x and y components of the vector.
	*/
	T x,y;	

public:

	/*!
	\note
	For efficiency, the default constructor does not initialize the components.

	Used for: Math.
	*/
	LTVector2()
	{}

	/*!
	Initialize this vector to \f$ (a,b) \f$.

	\param x
	\param y

	Used for: Math.
	*/
	LTVector2( const T a, const T b )
		: x(a), y(b)
	{}

	/*!
	Set this vector equal to \f$ (a,b) \f$.

	Used for: Math.
	*/
	void Init( const T a=0, const T b=0 )
	{
		x = a;
		y = b;
	}

	/*!
	\return	\f$ {\bf a}_i \f$, read-only

	Used for: Math.
	*/
	const T& operator [] ( const int32 i ) const
	{
		return *((&x) + i);
	}

	/*!
	\return	\f$ {\bf a}_i \f$, assignable

	Used for: Math.
	*/
	T& operator [] ( const int32 i )
	{
		return *((&x) + i);
	}

	/*!
	\return	\b true if these vectors are equal, \b false otherwise

	Used for: Math.
	*/
	bool operator == ( const LTVector2& b ) const
	{
		return( b.x==x && b.y==y );
	}

	/*!
	\return	\b true if these vectors are not equal, \b false otherwise

	Used for: Math.
	*/
	bool operator != ( const LTVector2& b ) const
	{
		return !( b == *this );
	}

	/*!
	\return	\b true if they are within a distance \f$ r \f$ of each other,
	\b false otherwise

	Check if this vector is within a distance \f$ r \f$ of \f${\bf b}\f$.

	Used for: Math.
	*/
	bool NearlyEquals( const LTVector2& b, const T r ) const
	{
		const LTVector2 t = *this - b;//difference

		return t.Dot(t) < r*r;//radius
	}

	/*!
	\return	\f$ -{\bf a} \f$

	Used for: Math.
	*/
	const LTVector2 operator - () const
	{
		return LTVector2( -x, -y );
	}

	/*!
	\return	\f$ {\bf a}+={\bf b} \f$

	Used for: Math.
	*/
	const LTVector2& operator += ( const LTVector2& b )
	{
		x += b.x;
		y += b.y;

		return *this;
	}

	/*!
	\return	\f$ {\bf a}-={\bf b} \f$

	Used for: Math.
	*/
	const LTVector2& operator -= ( const LTVector2& b ) 
	{
		x -= b.x;
		y -= b.y;

		return *this;
	}

	/*!
	\return	\f$ {\bf a}*=s \f$

	Used for: Math.
	*/
	const LTVector2& operator *= ( const T s )
	{
		x *= s;
		y *= s;

		return *this;
	}

	/*!
	\return	\f$ {\bf a}/=s \f$

	Used for: Math.
	*/
	const LTVector2& operator /= ( const T s )
	{
		x /= s;
		y /= s;

		return *this;
	}

	/*!
	\return	\f$ {\bf a}+{\bf b} \f$

	Used for: Math.
	*/
	const LTVector2 operator + ( const LTVector2& b ) const
	{
		return LTVector2( x + b.x, y + b.y );
	}

	/*!
	\return	\f$ {\bf a}-{\bf b} \f$

	Used for: Math.
	*/
	const LTVector2 operator - ( const LTVector2& b ) const
	{
		return LTVector2( x - b.x, y - b.y );
	}

	/*!
	\return	\f$ s*{\bf a} \f$

	Used for: Math.
	*/
	const LTVector2 operator * ( const T s ) const
	{
		return LTVector2( x*s, y*s );
	}

	/*!
	\return	\f$ s*{\bf a} \f$

	Used for: Math.
	*/
	friend const LTVector2 operator * ( const T s, const LTVector2& v )
	{
		return v * s;
	}

	/*!
	\return	\f$ {\bf a}/s \f$

	Used for: Math.
	*/
	const LTVector2 operator / ( const T s ) const
	{
		return LTVector2( x/s, y/s );
	}

	//NOTE:  The Cross product is not defined for 2D vectors.

	/*!
	\return	\f$ {\bf a} \cdot {\bf b} = (a_x*b_x + a_y*b_y) \f$

	Compute the dot product of \f${\bf a}\f$ and \f${\bf b}\f$.

	Used for: Math.
	*/
	T Dot( const LTVector2& b ) const
	{
		return( x*b.x + y*b.y);
	}

	/*!
	\return	\f$ ||{\bf a}||^2 \f$

	Used for: Math.
	*/
	T LengthSqr() const
	{
		return this->Dot(*this);
	}

	/*!
	\return	\f$ ||{\bf a}|| \f$

	Used for: Math.
	*/
	T Length() const
	{
		//NOTE:  cast the return value of
		//ltsqrtf() from a float to a T
		return (T)ltsqrtf( LengthSqr() );
	}

	/*!
	\return	\f$ ||{\bf a}||^2 \f$

	Used for: Math.
	*/
	T MagSqr() const
	{
		return LengthSqr();
	}

	/*!
	\return	\f$ ||{\bf a}|| \f$

	Used for: Math.
	*/
	T Mag() const
	{
		return Length();
	}

	/*!
	\return	\f$ ||{\bf a}-{\bf b}|| \f$

	Used for: Math.
	*/
	T Dist( const LTVector2& b ) const
	{
		return (*this - b).Length();
	}

	/*!
	\return	\f$ ||{\bf a}-{\bf b}||^2 \f$

	Used for: Math.
	*/
	T DistSqr( const LTVector2& b ) const
	{
		return (*this - b).LengthSqr();
	}

	/*!
	\return	\f$ {\bf \hat a} = \frac{{\bf a}}{||{\bf a}||} \f$.

	Calculate a unit vector from \f$ {\bf a} \f$.

	Used for: Math.
	*/
	const LTVector2 Unit() const
	{
		return (*this) / Length();
	}

	/*!
	\return	\f$ {\bf \hat a} = \frac{{\bf a}}{||{\bf a}||} \f$.

	Normalize this vector.

	Used for: Math.
	*/
	const LTVector2& Normalize()
	{
		(*this) /= Length();

		return *this;
	}

	//each component will be set to the lower of the current value or the passed in value
	void Min(const LTVector2& v)
	{
		Init(LTMIN(x, v.x), LTMIN(y, v.y));
	}

	//each component will be set to the larger of the current value or the passed in value
	void Max(const LTVector2& v)
	{
		Init(LTMAX(x, v.x), LTMAX(y, v.y));
	}

	//each component will be set to the lower of the current value or the passed in value
	LTVector2 GetMin(const LTVector2& v) const
	{
		return LTVector2(LTMIN(x, v.x), LTMIN(y, v.y));
	}
	//each component will be set to the larger of the current value or the passed in value
	LTVector2 GetMax(const LTVector2& v) const
	{
		return LTVector2(LTMAX(x, v.x), LTMAX(y, v.y));
	}

};


//define types
typedef LTVector2<float>	LTVector2f;
typedef LTVector2<double>	LTVector2d;
typedef LTVector2<uint16>	LTVector2u16;
typedef LTVector2<int32>	LTVector2n;


//---------------------------------------------------------------------------//
/*!
The LTVector3 template allows the application to define a 3-dimensional vector
with arbitrary component types.  Currently, three overloads are provided:
LTVector3f (whose components are \b float's), LTVector3d (whose components are
\b double's), and LTVector3us (whose components are \b uint16's)

Used for: Math.
*/
template< class T >
class LTVector3
{
public:
	/*!
	The x, y and z components of the vector.
	*/
	T x,y,z;	

public:

	/*!
	\note
	For efficiency, the default constructor does not initialize the components.

	Used for: Math.
	*/
	LTVector3()
	{}

	/*!
	Initialize this vector to \f$ (x,y,z) \f$.

	\param x
	\param y
	\param z

	Used for: Math.
	*/
	LTVector3( const T a, const T b, const T c )
		:	x(a), y(b), z(c)
	{}

	/*!
	Set this vector equal to \f$ (a,b,c) \f$.

	Used for: Math.
	*/
	void Init( const T a=0, const T b=0, const T c=0 )
	{
		x = a;
		y = b;
		z = c;
	}

	/*!
	\return	\f$ {\bf a}_i \f$, read-only

	Used for: Math.
	*/
	const T& operator [] ( const int32 i ) const
	{
		return *((&x) + i);
	}

	/*!
	\return	\f$ {\bf a}_i \f$, assignable

	Used for: Math.
	*/
	T& operator [] ( const int32 i )
	{
		return *((&x) + i);
	}

	/*!
	\return	\b true if these vectors are equal, \b false otherwise

	Used for: Math.
	*/
	bool operator == ( const LTVector3& b ) const
	{
		return (b.x==x && b.y==y && b.z==z);
	}

	/*!
	\return	\b true if these vectors are not equal, \b false otherwise

	Used for: Math.
	*/
	bool operator != ( const LTVector3& b ) const
	{
		return !(b == *this);
	}

	/*!
	\return	\b true if they are within a distance \f$ r \f$ of each other,
	\b false otherwise

	Check if this vector is within a distance \f$ r \f$ of \f${\bf b}\f$.

	Used for: Math.
	*/
	bool NearlyEquals( const LTVector3& b, const T r ) const
	{
		const LTVector3 t = *this - b;//difference

		return t.Dot(t) < r*r;//radius
	}

	/*!
	\return	\f$ -{\bf a} \f$

	Used for: Math.
	*/
	const LTVector3 operator - () const
	{
		return LTVector3( -x, -y, -z );
	}

	/*!
	\return	\f$ {\bf a}={\bf b} \f$

	Used for: Math.
	*/
	const LTVector3& operator = ( const LTVector3& b )
	{
		x = b.x;
		y = b.y;
		z = b.z;

		return *this;
	}

	/*!
	\return	\f$ {\bf a}+={\bf b} \f$

	Used for: Math.
	*/
	const LTVector3& operator += ( const LTVector3& b ) 
	{
		x += b.x;
		y += b.y;
		z += b.z;

		return *this;
	} 

	/*!
	\return	\f$ {\bf a}-={\bf b} \f$

	Used for: Math.
	*/
	const LTVector3& operator -= ( const LTVector3& b ) 
	{
		x -= b.x;
		y -= b.y;
		z -= b.z;

		return *this;
	} 

	/*!
	\return	\f$ {\bf a}*=s \f$

	Used for: Math.
	*/
	const LTVector3& operator *= ( const T s )
	{
		x *= s;
		y *= s;
		z *= s;

		return *this;
	}

	/*!
	\return	\f$ {\bf a}/=s \f$

	Used for: Math.
	*/
	const LTVector3& operator /= ( const T s )
	{
		x /= s;
		y /= s;
		z /= s;

		return *this;
	}

	/*!
	\return	\f$ {\bf a}+{\bf b} \f$

	Used for: Math.
	*/
	const LTVector3 operator + ( const LTVector3& b ) const
	{
		return LTVector3( x + b.x, y + b.y, z + b.z );
	}

	/*!
	\return	\f$ {\bf a}-{\bf b} \f$

	Used for: Math.
	*/
	const LTVector3 operator - ( const LTVector3& b ) const
	{
		return LTVector3( x - b.x, y - b.y, z - b.z );
	}

	/*!
	\return	\f$ s*{\bf a} \f$

	Used for: Math.
	*/
	const LTVector3 operator * ( const T s) const
	{
		return LTVector3( x*s, y*s, z*s );
	}

	/*!
	\return	\f$ s*{\bf a} \f$

	Used for: Math.
	*/
	friend const LTVector3 operator * ( const T s, const LTVector3& v )
	{
		return v * s;
	}

	/*!
	\return	\f$ {\bf a}/s \f$

	Used for: Math.
	*/
	const LTVector3 operator / ( const T s ) const
	{
		return LTVector3( x/s, y/s, z/s );
	}

	/*!
	\return	\f${\bf a} \times {\bf b}\f$

	Compute the cross product of \f${\bf a}\f$ and \f${\bf b}\f$.

	Used for: Math.
	*/
	const LTVector3 Cross( const LTVector3& b ) const
	{
		return LTVector3( y*b.z - z*b.y, z*b.x - x*b.z, x*b.y - y*b.x );
	}

	/*!
	\return	\f$ {\bf a} \cdot {\bf b} = (a_x*b_x + a_y*b_y + a_z*b_z) \f$

	Compute the dot product of \f${\bf a}\f$ and \f${\bf b}\f$.

	Used for: Math.
	*/
	T Dot( const LTVector3& b ) const
	{
		return x*b.x + y*b.y + z*b.z;
	}

	/*!
	\return	\f$ ||{\bf a}||^2 \f$

	Used for: Math.
	*/
	T LengthSqr() const
	{
		return this->Dot(*this);
	}

	/*!
	\return	\f$ ||{\bf a}|| \f$

	Used for: Math.
	*/
	T Length() const
	{
		//NOTE:  cast the return value of
		//ltsqrtf() from a float to a T
		return (T)ltsqrtf( LengthSqr() );
	}

	/*!
	\return	\f$ ||{\bf a}||^2 \f$

	Used for: Math.
	*/
	T MagSqr() const
	{
		return LengthSqr();
	}

	/*!
	\return	\f$ ||{\bf a}|| \f$

	Used for: Math.
	*/
	T Mag() const
	{
		return Length();
	}

	/*!
	\return	\f$ ||{\bf a}-{\bf b}|| \f$

	Used for: Math.
	*/
	T Dist( const LTVector3& b ) const
	{
		return (*this - b).Length();
	}

	/*!
	\return	\f$ ||{\bf a}-{\bf b}||^2 \f$

	Used for: Math.
	*/
	T DistSqr( const LTVector3& b ) const
	{
		return (*this - b).LengthSqr();
	}

	/*!
	\return	\f$ {\bf \hat a} = \frac{{\bf a}}{||{\bf a}||} \f$.

	Calculate a unit vector from \f$ {\bf a} \f$.

	Used for: Math.
	*/
	const LTVector3 Unit() const
	{
		return (*this) / Length();
	}

	/*!
	\return	\f$ {\bf \hat a} = \frac{{\bf a}}{||{\bf a}||} \f$.

	Normalize this vector.

	Used for: Math.
	*/
	const LTVector3& Normalize()
	{
		(*this) /= Length();

		return *this;
	}
};


//define types
typedef LTVector3<float>		LTVector3f;	//used for most physics/math routines
typedef LTVector3<double>		LTVector3d;
typedef LTVector3<uint16>		LTVector3u16;	//used for collision data


//---------------------------------------------------------------------------//
/*!
The LTVector4 template allows the application to define a 4-dimensional vector
with arbitrary component types.  Currently, three overloads are provided:
LTVector4f (whose components are \b float's), LTVector4d (whose components are
\b double's), and LTVector4us (whose components are \b uint16's)

Used for: Math.
*/
template< class T >
class LTVector4
{
public:
	/*!
	The x, y, z, and w components of the vector.
	*/
	T x,y,z,w;	

public:

	/*!
	\note
	For efficiency, the default constructor does not initialize the components.

	Used for: Math.
	*/
	LTVector4()
	{}

	/*!
	Initialize this vector to \f$ (x,y,z,w) \f$.

	\param x
	\param y
	\param z
	\param w

	Used for: Math.
	*/
	LTVector4( const T a, const T b, const T c, const T d )
		:	x(a), y(b), z(c), w(d)
	{}

	/*!
	Set this vector equal to \f$ (a,b,c,d) \f$.

	Used for: Math.
	*/
	void Init( const T a=0, const T b=0, const T c=0, const T d=0 )
	{
		x = a;
		y = b;
		z = c;
		w = d;
	}

	/*!
	\return	\f$ {\bf a}_i \f$, read-only

	Used for: Math.
	*/
	const T& operator [] ( const int32 i ) const
	{
		return *((&x) + i);
	}

	/*!
	\return	\f$ {\bf a}_i \f$, assignable

	Used for: Math.
	*/
	T& operator [] ( const int32 i )
	{
		return *((&x) + i);
	}

	/*!
	\return	\b true if these vectors are equal, \b false otherwise

	Used for: Math.
	*/
	bool operator == ( const LTVector4& b ) const
	{
		return( b.x==x && b.y==y && b.z==z && b.w==w );
	}

	/*!
	\return	\b true if these vectors are not equal, \b false otherwise

	Used for: Math.
	*/
	bool operator != ( const LTVector4& b ) const
	{
		return !( b == *this );
	}

	/*!
	\return	\b true if they are within a distance \f$ r \f$ of each other,
	\b false otherwise

	Check if this vector is within a distance \f$ r \f$ of \f${\bf b}\f$.

	Used for: Math.
	*/
	bool NearlyEquals( const LTVector4& b, const T r ) const
	{
		const LTVector4 t = *this - b;//difference

		return t.Dot(t) < r*r;//radius
	}

	/*!
	\return	\f$ -{\bf a} \f$

	Used for: Math.
	*/
	LTVector4 operator - ( void ) const
	{
		return LTVector4( -x, -y, -z, -w );
	}

	/*!
	\return	\f$ {\bf a}={\bf b} \f$

	Used for: Math.
	*/
	const LTVector4& operator = ( const LTVector4& b )
	{
		x = b.x;
		y = b.y;
		z = b.z;
		w = b.w;

		return *this;
	}

	/*!
	\return	\f$ {\bf a}+={\bf b} \f$

	Used for: Math.
	*/
	const LTVector4& operator += ( const LTVector4& b ) 
	{
		x += b.x;
		y += b.y;
		z += b.z;
		w += b.w;

		return *this;
	} 

	/*!
	\return	\f$ {\bf a}-={\bf b} \f$

	Used for: Math.
	*/
	const LTVector4& operator -= ( const LTVector4& b ) 
	{
		x -= b.x;
		y -= b.y;
		z -= b.z;
		w -= b.w;

		return *this;
	} 

	/*!
	\return	\f$ {\bf a}*=s \f$

	Used for: Math.
	*/
	const LTVector4& operator *= ( const T s )
	{
		x *= s;
		y *= s;
		z *= s;
		w *= s;

		return *this;
	} 

	/*!
	\return	\f$ {\bf a}/=s \f$

	Used for: Math.
	*/
	const LTVector4& operator /= ( const T s )
	{
		x /= s;
		y /= s;
		z /= s;
		w /= s;

		return *this;
	}

	/*!
	\return	\f$ {\bf a}+{\bf b} \f$

	Used for: Math.
	*/
	const LTVector4 operator + ( const LTVector4& b ) const
	{
		return LTVector4( x+b.x, y+b.y, z+b.z, w+b.w );
	}

	/*!
	\return	\f$ {\bf a}-{\bf b} \f$

	Used for: Math.
	*/
	const LTVector4 operator - ( const LTVector4& b ) const
	{
		return LTVector4( x-b.x, y-b.y, z-b.z, w-b.w );
	}

	/*!
	\return	\f$ s*{\bf a} \f$

	Used for: Math.
	*/
	const LTVector4 operator * ( const T s ) const
	{
		return LTVector4( x*s, y*s, z*s, w*s );
	}

	/*!
	\return	\f$ s*{\bf a} \f$

	Used for: Math.
	*/
	friend const LTVector4 operator * ( const T s, const LTVector4& v )
	{
		return v * s;
	}

	/*!
	\return	\f$ {\bf a}/s \f$

	Used for: Math.
	*/
	const LTVector4 operator / ( const T s ) const
	{
		return LTVector4( x/s, y/s, z/s, w/s );
	}

	//NOTE:  Cross product not defined for 4-vectors

	/*!
	\return	\f$ {\bf a} \cdot {\bf b} = (a_x*b_x + a_y*b_y + a_z*b_z + a_w*b_w) \f$

	Compute the dot product of \f${\bf a}\f$ and \f${\bf b}\f$.

	Used for: Math.
	*/
	T Dot( const LTVector4& b ) const
	{
		return( x*b.x + y*b.y + z*b.z + w*b.w );
	}

	/*!
	\return	\f$ ||{\bf a}||^2 \f$

	Used for: Math.
	*/
	T LengthSqr() const
	{
		return this->Dot(*this);
	}

	/*!
	\return	\f$ ||{\bf a}|| \f$

	Used for: Math.
	*/
	T Length() const
	{
		//NOTE:  cast the return value of
		//ltsqrtf() from a float to a T
		return (T)ltsqrtf( LengthSqr() );
	}

	/*!
	\return	\f$ ||{\bf a}||^2 \f$

	Used for: Math.
	*/
	T MagSqr() const
	{
		return LengthSqr();
	}

	/*!
	\return	\f$ ||{\bf a}|| \f$

	Used for: Math.
	*/
	T Mag() const
	{
		return Length();
	}

	/*!
	\return	\f$ ||{\bf a}-{\bf b}|| \f$

	Used for: Math.
	*/
	T Dist( const LTVector4& b ) const
	{
		return (*this - b).Length();
	}

	/*!
	\return	\f$ ||{\bf a}-{\bf b}||^2 \f$

	Used for: Math.
	*/
	T DistSqr( const LTVector4& b ) const
	{
		return (*this - b).LengthSqr();
	}

	/*!
	\return	\f$ {\bf \hat a} = \frac{{\bf a}}{||{\bf a}||} \f$.

	Calculate a unit vector from \f$ {\bf a} \f$.

	Used for: Math.
	*/
	const LTVector4 Unit() const
	{
		return (*this) / Length();
	}

	/*!
	\return	\f$ {\bf \hat a} = \frac{{\bf a}}{||{\bf a}||} \f$.

	Normalize this vector.

	Used for: Math.
	*/
	const LTVector4& Normalize()
	{
		(*this) /= Length();

		return *this;
	}
};


//define types
typedef LTVector4<float>	LTVector4f;
typedef LTVector4<double>	LTVector4d;
typedef LTVector4<uint16>	LTVector4u16;


#endif
//EOF
