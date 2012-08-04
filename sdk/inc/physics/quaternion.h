#ifndef __QUATERNION_H__
#define __QUATERNION_H__


#ifndef __MATRIX_H__
#include "matrix.h"
#endif


//---------------------------------------------------------------------------//
/*!
The LTQuaternionf data type represents a quaternion with four \b float's, and
provides arithmetic operations and conversion routines from/to 3x3 matrices.
*/
class LTQuaternionf
{
public:

/*!
Real part.
*/
    float r;

/*! 
Imaginary parts.
*/
    LTVector3f v;   

/*!
\note
For efficiency, the LTQuaternionf default constructor does not
initialize the components.
*/
    LTQuaternionf()
    {}

/*!
Construct the quaternion \f$ {\bf q}=(r,x,y,z) \f$.
*/
    LTQuaternionf(const float r, const float x, const float y, const float z)
        :   r(r), v(x,y,z)
    {}

/*!
Construct the quaternion \f$ {\bf q}=(r,v_x,v_y,v_z) \f$.
*/
    LTQuaternionf(const float r, const LTVector3f& v)
        :   r(r), v(v)
    {}

/*!
Construct the quaternion \f$ {\bf q}=(0,v_x,v_y,v_z) \f$.
*/
    LTQuaternionf(const LTVector3f& v)
        :   r(0), v(v)
    {}

/*!
\param  M   a 3x3 matrix

Given a 3x3 matrix of the form

\f[
    {\bf M} =
    \left[
    {\begin{array}{*{20}c}
        {1 - 2y^2  - 2z^2 } & {2xy - 2rz} & {2xz + 2ry}\\
        {2xy + 2rz} & {1 - 2x^2  - 2z^2 } & {2yz - 2rx}\\
        {2xz - 2ry} & {2yz + 2rx} & {1 - 2x^2  - 2y^2 }\\
    \end{array}}
    \right],
\f]

solve for the unit quaternion \f${\bf q}=(r,x,y,z)\f$.

Used for: Math.
*/
    LTQuaternionf(const LTMatrix3f& M);

/*!
\return the vector \f$ {\bf v}=(x,y,z) \f$.
*/
    operator LTVector3f () const
    {
        return v;
    }

/*!
\return \b M

Convert this quaternion to a 3x3 matrix of the form

\f[
    {\bf M} =
    \left[
    {\begin{array}{*{20}c}
        {1 - 2y^2  - 2z^2 } & {2xy - 2rz} & {2xz + 2ry}\\
        {2xy + 2rz} & {1 - 2x^2  - 2z^2 } & {2yz - 2rx}\\
        {2xz - 2ry} & {2yz + 2rx} & {1 - 2x^2  - 2y^2 }\\
    \end{array}}
    \right].
\f]

Used for: Math.
*/
    operator LTMatrix3f () const
    {
        //placeholders
        const float x = v.x;
        const float y = v.y;
        const float z = v.z;
        //these products eliminate 18 multiplies
        const float _2x = 2*x;
        const float _2y = 2*y;
        const float _2z = 2*z;

        // This formula taken from Hearn and Baker,
        // "Computer Graphics", 2nd Edition, p. 420
        //
        // This also matches David Baraff's formula
        // in his "Intro to Rigid Body Sim I".

        //initialize the COLUMNS of the matrix
        return LTMatrix3f
        (
            //col 0
            LTVector3f(1 - _2y*y - _2z*z,  _2x*y + _2z*r,      _2x*z - _2y*r),
            //col 1
            LTVector3f(_2x*y - _2z*r,      1 - _2x*x - _2z*z,  _2y*z + _2x*r),
            //col 2
            LTVector3f(_2x*z + _2y*r,      _2y*z - _2x*r,      1 - _2x*x - _2y*y)
       );
    }

    //NOTE:  Indexing operators are not implemented so that the
    //order of the elements cannot be hard coded

/*!
\param  b   another quaternion
\return     \b true if the quaternion's are equal,
            \b false otherwise

Used For: Math.
*/
    bool operator == (const LTQuaternionf& b) const
    {
        return (r==b.r && v==b.v);
    }

/*!
\param  b   another quaternion
\return     \b true if the quaternion's are not equal,
            \b false otherwise

Used For: Math.
*/
    bool operator != (const LTQuaternionf& b) const
    {
        return !(*this == b);
    }

/*!
\param  b   another quaternion
\param  r   a distance
\return     \b true if they are within a distance \f$ r \f$ of each other,
            \b false otherwise

Check if this quaternion is within a distance \f$ r \f$ of \f${\bf b}\f$.

Used For: Math.
*/
    bool NearlyEquals(const LTQuaternionf& b, const float e) const
    {
        //within a radial tolerance
        const LTQuaternionf t = *this - b;//difference

        return t.Dot(t) < e*e;//error
    }

/*!
\return \f$ -{\bf a} \f$

Used for: Math.
*/
    const LTQuaternionf operator - () const
    {
        return LTQuaternionf(-r, -v);
    }

/*!
\return \f$ {\bf a}+={\bf b} \f$

Used for: Math.
*/
    const LTQuaternionf& operator += (const LTQuaternionf& b) 
    {
        r += b.r;
        v += b.v;

        return *this;
    }

/*!
\return \f$ {\bf a}-={\bf b} \f$

Used for: Math.
*/
    const LTQuaternionf& operator -= (const LTQuaternionf& b) 
    {
        r -= b.r;
        v -= b.v;

        return *this;
    }

/*!
\return \f$ {\bf a}*=s \f$

Used for: Math.
*/
    const LTQuaternionf& operator *= (const float s)
    {
        r *= s;
        v *= s;

        return *this;
    }

/*!
\return \f$ {\bf a}/=s \f$

Used for: Math.
*/
    const LTQuaternionf& operator /= (const float s)
    {
        const float rs = 1/s;//reciprocal of s

        r *= rs;
        v *= rs;

        return *this;
    }

/*!
\return \f$ {\bf a}+{\bf b} \f$

Used for: Math.
*/
    const LTQuaternionf operator + (const LTQuaternionf& b) const
    {
        return LTQuaternionf(r + b.r, v + b.v);
    }

/*!
\return \f$ {\bf a}-{\bf b} \f$

Used for: Math.
*/
    const LTQuaternionf operator - (const LTQuaternionf& b) const
    {
        return LTQuaternionf(r - b.r, v - b.v);
    }

/*!
\return \f$ s*{\bf a} \f$

Used for: Math.
*/
    const LTQuaternionf operator * (const float s) const
    {
        return LTQuaternionf(r*s, v*s);
    }

/*!
\return \f$ s*{\bf a} \f$

Used for: Math.
*/
    friend const LTQuaternionf operator * (const float s, const LTQuaternionf& q)
    {
        return q * s;
    }

/*!
\return \f$ {\bf a}{\bf b} = (r_ar_b - {\bf v}_a \cdot {\bf v}_b,
            r_a{\bf v}_b + r_b{\bf v}_a + {\bf v}_a \times {\bf v}_b) \f$

Used for: Math.
*/
    const LTQuaternionf operator * (const LTQuaternionf& b) const
    {
        //quaternion product is (r1*r2 - v1*v2, r1*v2 + r2*v1 + v1.Cross(v2))
        return LTQuaternionf(r*b.r - v.Dot(b.v), r*b.v + b.r*v + v.Cross(b.v));
    }

/*!
\return \f$ {\bf a} *= {\bf b} \f$

Used for: Math.
*/
    const LTQuaternionf& operator *= (const LTQuaternionf& b)
    {
        //store off 'r' so it doesn't get changed
        //halfway through the operation
        const float ra = this->r;

        this->r = ra*b.r - v.Dot(b.v);
        this->v = ra*b.v + b.r*v + v.Cross(b.v);

        return *this;
    }

/*!
\return \f$ {\bf a}/s \f$

Used for: Math.
*/
    const LTQuaternionf operator / (const float s) const
    {
        const float rs = 1/s;//only do the divide once

        return LTQuaternionf(r*rs, v*rs);
    }

/*!
\return \f$ {\bf a} \cdot {\bf b} = (a_x*b_x + a_y*b_y) \f$

Compute the dot product of \f${\bf a}\f$ and \f${\bf b}\f$.

Used for: Math.
*/
    float Dot(const LTQuaternionf& b) const
    {
        return(r*b.r + v.Dot(b.v));
    }

/*!
\return \f$ ||{\bf a}||^2 \f$

Used for: Math.
*/
    float LengthSqr() const
    {
        return this->Dot(*this);
    }

/*!
\return \f$ ||{\bf a}|| \f$

Used for: Math.
*/
    float Length() const
    {
        return sqrtf(LengthSqr());
    }

/*!
\return \f$ ||{\bf a}||^2 \f$

Used for: Math.
*/
    float MagSqr() const
    {
        return LengthSqr();
    }

/*!
\return \f$ ||{\bf a}|| \f$

Used for: Math.
*/
    float Mag() const
    {
        return Length();
    }

/*!
\return \f$ ||{\bf a}-{\bf b}|| \f$

Used for: Math.
*/
    float Dist(const LTQuaternionf& b) const
    {
        return (*this - b).Length();
    }

/*!
\return \f$ ||{\bf a}-{\bf b}||^2 \f$

Used for: Math.
*/
    float DistSqr(const LTQuaternionf& b) const
    {
        return (*this - b).LengthSqr();
    }

/*!
\return \f$ {\bf \hat a} = \frac{{\bf a}}{||{\bf a}||} \f$.

Calculate a unit quaternion from \f$ {\bf a} \f$.

Used for: Math.
*/
    const LTQuaternionf Unit() const
    {
        return (*this) / Length();
    }

/*!
\return \f$ {\bf \hat a} = \frac{{\bf a}}{||{\bf a}||} \f$.

Normalize this quaternion.

Used for: Math.
*/
    const LTQuaternionf& Normalize()
    {
        (*this) /= Length();

        return *this;
    }

/*!
\return \f$ \bar {\bf a} = (r,-x,-y,-z) \f$.

Complex conjugate.

Used for: Math.
*/
    const LTQuaternionf Conjugate() const
    {
        return LTQuaternionf(r, -v);
    }
};


#endif
//EOF
