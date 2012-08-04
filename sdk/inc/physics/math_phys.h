#ifndef __MATH_PHYS_H__
#define __MATH_PHYS_H__

#ifndef __CMATH__
#include <cmath>
#define __CMATH__
#endif

#ifndef __CFLOAT__
#include <cfloat>
#define __CFLOAT__
#endif

#ifndef __CSTDLIB__
#include <cstdlib>
#define __CSTDLIB__
#endif

#ifndef __QUATERNION_H__
#include "quaternion.h"
#endif


//---------------------------------------------------------------------------//
// Types


//---------------------------------------------------------------------------//
//
/*!
The LTFloat data type exposes the bits of a single-precision float in IEEE 754
format.

Used for:  Math.
*/
union LTFloat
{
    struct bits
    {
        unsigned m: 23; //!mantissa
        unsigned e: 8;  //!exponent
        unsigned s: 1;  //!sign bit
    };

    float   f;//!value
    bits    b;//!bits

    LTFloat( const float s )
        :   f(s)
    {}

    //cast to float
    operator float() const
    {
        return f;
    }
};


//---------------------------------------------------------------------------//
// Constants
// NOTE:  To ensure all the digits of PI (and its multiples) are accurate to
// PI*(1 + e_machine), compute PI as a double, then round to nearest float.
#undef PI//just in case
/*!
\f$ \pi \f$
Used For:  Math.
*/
const float PI			= (float)(4 * atan(1.0));
/*!
\f$ frac{\pi}{2} \f$
Used For:  Math.
*/
const float PI_2		= PI / 2;
/*!
\f$ frac{\pi}{3} \f$
Used For:  Math.
*/
const float PI_3		= PI / 3;
/*!
\f$ frac{\pi}{4} \f$
Used For:  Math.
*/
const float PI_4		= PI / 4;

//gravitational acceleration (as a positive constant)
const float GRAV_ACCEL	= 9.81f;//! \f$ g=9.81m/s^2 \f$

//world directions
#ifdef __PHYS_RIGHT_HANDED_Z_UP__

#ifndef DOXYGEN_SHOULD_SKIP_THIS
	const LTVector3f WORLD_RIGHT(1,0,0);
	const LTVector3f WORLD_FORWARD(0,1,0);
	const LTVector3f WORLD_UP(0,0,1);
#endif//doxygen

#else//left-handed, y-up

/*!
\f$ (1,0,0) \f$
Used For:  Math.
*/
const LTVector3f WORLD_RIGHT(1,0,0);
/*!
\f$ (0,0,1) \f$
Used For:  Math.
*/
const LTVector3f WORLD_FORWARD(0,0,1);
/*!
\f$ (0,1,0) \f$
Used For:  Math.
*/
const LTVector3f WORLD_UP(0,1,0);

#endif


//---------------------------------------------------------------------------//
// Conversions
/*!
\f$ frac{\pi}{180} \f$
Used For:  Math.
*/
const float RADIANS_PER_DEGREE = PI / 180;
/*!
\f$ frac{180}{\pi} \f$
Used For:  Math.
*/
const float DEGREES_PER_RADIAN = 180 / PI;

/*!
Convert an angle in radians to degrees.

Used for:  Math.
*/
template<class T>
inline	T RadiansToDegrees( T a )	{ return a*DEGREES_PER_RADIAN; }

/*!
Convert an angle in degrees to radians.

Used for:  Math.
*/
template<class T>
inline	T DegreesToRadians( T a )	{ return a*RADIANS_PER_DEGREE; }


//---------------------------------------------------------------------------//
#ifndef DOXYGEN_SHOULD_SKIP_THIS
/*!
Used for:  Obsolete.
*/
template<class T>
inline	T RADIANS_TO_DEGREES( T a )	{ return a*DEGREES_PER_RADIAN; }
/*!
Used for:  Obsolete.
*/
template<class T>
inline	T DEGREES_TO_RADIANS( T a )	{ return a*RADIANS_PER_DEGREE; }
#endif//doxygen


//---------------------------------------------------------------------------//
//read time stamp (cycle) counter
#ifndef DOXYGEN_SHOULD_SKIP_THIS
#ifdef _WIN32
inline int32 rdtsc()
{
    int t;

    __asm
    {
        rdtsc
        mov t, eax
    }

    return t;
}
#endif//win32
#endif//doxygen


//---------------------------------------------------------------------------//
/*!
\param  a   min
\param  b   max
\return     a random number \f$ r \in [a,b] \f$

Used for: Math.
*/
inline float RandomFloat(const float a, const float b)
{
    const float u = (float)rand() / RAND_MAX;//between 0 and 1

    return u*a + (1-u)*b;
}


//---------------------------------------------------------------------------//
/*!
\param  a   first value
\param  b   second value
\return     the lesser of a or b

Used for: Math.
*/
template< class T >
inline T Min(const T& a, const T& b)
{
    if (a<b)
        return a;
    else
        return b;
}


//---------------------------------------------------------------------------//
/*!
\param  a   first value
\param  b   second value
\return     the greater of a or b

Used for: Math.
*/
template< class T >
inline T Max(const T& a, const T& b)
{
    if (a>b)
        return a;
    else
        return b;
}


//---------------------------------------------------------------------------//
/*!
\param  a   first value
\param  b   second value

Swap the values of a and b.

Used for: Math.
*/
template< class T >
inline void Swap(T& a, T& b)
{
    const T temp = a;

    a = b;
    b = temp;
}


//---------------------------------------------------------------------------//
/*!
\param  s   the number
\return     +1 if s>=0, -1 otherwise

Used for: Math.
*/
template< class T >
inline T Sign(const T s)
{
    if (s>=0)
        return 1;
    else
        return -1;
}


//---------------------------------------------------------------------------//
/*!
\param  s1  the first root
\param  s2  the second root

\param  a   the coefficient of \f$ x^2 \f$
\param  b   the coefficient of \f$ x \f$
\param  c   the constant term

\return     \b true if the roots are real
            \b false if the roots are complex or if a=0

Calculate the real valued roots of the quadratic equation: \f$ ax^2 + bx + c = 0 \f$.

Used for: Math.
*/
inline bool QuadraticFormula
(
    float& s1,      //first solution
    float& s2,      //second solution
    const float a,  //coefficient of x^2
    const float b,  //coefficient of x
    const float c   //constant term
)
{
    if (a != 0)//don't divide by 0 (degenerate case)
    {
        const float q = b*b - 4*a*c;//radicand
        const float d = 1 / (2*a);//denominator

        if (q==0)
        {
            s1 = s2 = -b * d;

            return true;//one solution
        }
        else if (q > 0)
        {
            const float sq = sqrtf(q);

            s1 = (-b + sq) * d;
            s2 = (-b - sq) * d;

            return true;//two solutions
        }
    }

    return false;//a real solution could not be evaluated
}


//---------------------------------------------------------------------------//
/*!

\param	a0	the quantity at \f$u=0\f$
\param	a1	the quantity at \f$u=1\f$
\param	u	the interpolation parameter \f$ u \in [0,1] \f$
\return	qu	the interpolated quantity

Given two numerical quantities \f${\bf a}_0\f$ and \f${\bf a}_1\f$ and
a parameter \f$ u \in [0,1] \f$, linearly interpolate a quantity
\f${\bf a}_u\f$.  If \f$ u \nin [0,1] \f$, then extrapolation is performed.

Used for: Math.
*/
template<class S, class T>
const T Lerp( const T& a0, const T& a1, const S& u )
{
	return (1-u)*a0 + u*a1;
}

//for clarity
template<class S, class T>
const T LinearlyInterpolate( const T& a0, const T& a1, const S& u )
{
	return Lerp( a0, a1, u );
}


//---------------------------------------------------------------------------//
/*!
\param	a0	the vector/quaternion at \f$u=0\f$
\param	a1	the vector/quaternion at \f$u=1\f$
\param	u	the interpolation parameter \f$ u \in [0,1] \f$
\return	qu	the interpolated vector/quaternion


Given two unit vectors/quaternions \f${\bf a}_0\f$ and \f${\bf a}_1\f$ and
a parameter \f$ u \in [0,1] \f$, spherically linearly interpolate a
unit vector/quaternion \f${\bf a}_u\f$ according to the formula

\f[
{\bf a}_u = {\bf a}_0\frac{ \sin{(1-u)\theta} }{ \sin{\theta} } +
            {\bf a}_1\frac{ \sin{u\theta} }{ \sin{\theta} },
\f]

where \f$\theta\f$ is given by

\f[
\theta = \arccos{ \left({{\bf a}_0 \cdot {\bf a}_1} \right) }.
\f]

Used for: Math.
*/
template<class S, class T>
const T Slerp(const T& a0, const T& a1, const S& u)
{
 
	const S theta = (S)acos( a0.Dot(a1) );//the angle between v0 and v1
	T au;

	if( theta != 0 )
	{
		const S t = 1 / (S)sin(theta);

		au = t*( a0 * (S)sin((1-u)*theta) + a1 * (S)sin(u*theta) );
	}
	else//don't divide by 0
	{
		au = a0;
	}


 
	return au;

}


//---------------------------------------------------------------------------//
//Create a 3x3 matrix that rotates a vector 'v' about a unit axis 'u' through
//an angle 'a' (in radians).
/*!
\param  M   the 4x4 transformation matrix
\param  q   the unit quaternion
\return \b LT_OK

Given an angle \f$ a \f$ and a unit axis of rotation \f${\bf u}\f$, create a
3x3 matrix \f${\bf M}\f$ that rotates any vector \f${\bf v}\f$ through
\f$ a \f$ about \f${\bf u}\f$ with the matrix multiply

\f[
{\bf v}_{rotated} = {\bf M}{\bf v}.
\f]

Used for: Math.
*/
inline const LTMatrix3f RotationMatrix(const float a, const LTVector3f& u)
{
    //create a quaternion from the rotation information,
    //which is then converted to a 3x3 matrix
    return LTQuaternionf(cosf(0.5f * a), u * sinf(0.5f * a));
}


#endif
//EOF
