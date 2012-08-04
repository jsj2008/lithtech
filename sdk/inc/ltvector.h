#ifndef __LTVECTOR_H__
#define __LTVECTOR_H__

#ifndef __ILTSTREAM_H__
	#include "iltstream.h"
#endif

 
#ifndef _VECTOR_H_
	#include "physics/vector.h"//vectors for math and physics
#endif

#ifndef __LTSYSOPTIM_H__
	#include "ltsysoptim.h"
#endif


#ifndef DOXYGEN_SHOULD_SKIP_THIS
/*!
Used for:  Obsolete.
*/
//---------------------------------------------------------------------------//
template<class T>
struct TVector3 {
    T x, y, z;

 
	TVector3()
	{}
	TVector3( const T x, const T y, const T z )
		:	x(x), y(y), z(z)
	{}
	TVector3( const LTVector3f& v )
		:	x(v.x), y(v.y), z(v.z)
	{}
	operator LTVector3f() const
	{
		return LTVector3f(x,y,z);
	}
	void Init( T a=0, T b=0, T c=0)
	{
		x = a;
		y = b;
		z = c;
	}
	T Dot( const TVector3& b ) const
	{
		return x*b.x + y*b.y + z*b.z;
	}
	T MagSqr() const
	{
		return x*x + y*y + z*z;
	}
	T Mag() const
	{
		return (T)ltsqrtf( x*x + y*y + z*z );
	}
	T LengthSquared() const
	{
		return x*x + y*y + z*z;
	}
	T Length() const
	{
		return (T)ltsqrtf( x*x + y*y + z*z );
	}
	const TVector3 Unit() const
	{
		return (*this) / this->Length();
	}

	const TVector3 GetNormalized() const
	{
		T vMag = this->Mag();
		return (*this) / vMag;
	}

	const TVector3& Normalize()
	{
		(*this) /= this->Length();


        return *this;
    }
    bool NearlyEquals(const TVector3& b, const T r = 0) const
    {
        //within a tolerance
        const TVector3 t = *this - b;//difference
        return t.Dot(t) <= r*r;//radius
 
	}
 	const TVector3 Cross( const TVector3& b ) const
	{
		return TVector3( b.y*z - b.z*y, b.z*x - b.x*z, b.x*y - b.y*x );
	}
	const T& operator [] ( const int32 i ) const
	{
		return *((&x) + i);
	}
	T& operator [] ( const int32 i )
	{
		return *((&x) + i);
	}
	bool operator == ( const TVector3& b ) const
	{
		return (b.x==x && b.y==y && b.z==z);
	}
	bool operator != ( const TVector3& b ) const
	{
		return !(b == *this);
	}
	const TVector3 operator - () const
	{
		return TVector3( -x, -y, -z );
	}
	const TVector3& operator = ( const TVector3& b )
	{
		x = b.x;
		y = b.y;
		z = b.z;

        return *this;
    }
    const TVector3& operator += (const TVector3& b) 
    {
        x += b.x;
        y += b.y;
        z += b.z;

        return *this;
    } 
    const TVector3& operator -= (const TVector3& b) 
    {
        x -= b.x;
        y -= b.y;
        z -= b.z;

        return *this;
    } 
    const TVector3& operator *= (const T s)
    {
        x *= s;
        y *= s;
        z *= s;

        return *this;
    }
    const TVector3& operator /= (const T s)
    {
        const T r = 1 / s;//reciprocal

        x *= r;
        y *= r;
        z *= r;

        return *this;
    }
    const TVector3 operator + (const TVector3& b) const
    {
        return TVector3(x + b.x, y + b.y, z + b.z);
    }
    const TVector3 operator - (const TVector3& b) const
    {
        return TVector3(x - b.x, y - b.y, z - b.z);
    }
    const TVector3 operator * (const T s) const
    {
        return TVector3(x*s, y*s, z*s);
    }
    friend inline const TVector3 operator * (const T s, const TVector3& v)
    {
        return v * s;
    }
    const TVector3 operator / (const T s) const
    {
        const T r = 1 / s;//reciprocal

        return TVector3(x*r, y*r, z*r);
    }
    bool operator > (const TVector3 &other) const { return ((x>other.x) && (y>other.y) && (z>other.z)); }
    bool operator < (const TVector3 &other) const { return ((x<other.x) && (y<other.y) && (z<other.z)); }
    bool operator >= (const TVector3 &other) const { return ((x>=other.x) && (y>=other.y) && (z>=other.z)); }
    bool operator <= (const TVector3 &other) const { return ((x<=other.x) && (y<=other.y) && (z<=other.z)); }
    TVector3 operator * (const TVector3& v) const     { return TVector3(x * v.x, y * v.y, z * v.z); }
    TVector3 operator / (const TVector3& v) const     { return TVector3(x / v.x, y / v.y, z / v.z); }
    void operator += (T s)
    {
        x += s;
        y += s;
        z += s;
    }
    void operator -= (T s)
    {
        x -= s;
        y -= s;
        z -= s;
    }
    T Dist(const TVector3& b) const
    {
        return (*this - b).Mag();
    }
    T DistSqr(const TVector3& b) const
    {
        return (*this - b).MagSqr();
    }
    T MagApprox() const;
    void Norm(T nVal = 1);
    void NormApprox(T nVal = 1);
};


//Don't worry about all the template stuff, just always use this.
typedef TVector3<float> LTVector;

template<class T>
inline void LTStream_Read(ILTStream *pStream, TVector3<T> &vec)
{
    STREAM_READ(vec.x);
    STREAM_READ(vec.y);
    STREAM_READ(vec.z);
}

template<class T>
inline void LTStream_Write(ILTStream *pStream, const TVector3<T> &vec)
{
    STREAM_WRITE(vec.x);
    STREAM_WRITE(vec.y);
    STREAM_WRITE(vec.z);
}

template<class T>
inline T TVector3<T>::MagApprox () const
{
    T   min, med, max;
    T   temp;

    max = fabsf(x);
    med = fabsf(y);
    min = fabsf(z);

    if (max < med)
    {
        temp = max;
        max = med;
        med = temp;
    }
    
    if (max < min)
    {
        temp = max;
        max = min;
        min = temp;
    }

    return max + ((med + min) * 0.25f);
}

template<class T>
inline void TVector3<T>::Norm(T nVal)
{
    T inv;
    T mag = Mag();

    if (mag == 0) 
        return;

    inv = nVal / mag;
    x = x * inv;
    y = y * inv;
    z = z * inv;
}

template<class T>
inline void TVector3<T>::NormApprox(T nVal)
{
    T inv;
    T mag = MagApprox();

    if (mag == 0) 
        return;

    inv = nVal / mag;
    x = x * inv;
    y = y * inv;
    z = z * inv;
}

#define VEC_CROSS(dest, v1, v2) \
    {\
    (dest).x = ((v2).y*(v1).z - (v2).z*(v1).y);\
    (dest).y = ((v2).z*(v1).x - (v2).x*(v1).z);\
    (dest).z = ((v2).x*(v1).y - (v2).y*(v1).x);\
    }

#define VEC_ADD(d, v1, v2) \
    {\
    (d).x = (v1).x + (v2).x;\
    (d).y = (v1).y + (v2).y;\
    (d).z = (v1).z + (v2).z;\
    }

#define VEC_ADDSCALED(d, v1, v2, s) \
    {\
    (d).x = (v1).x + ((v2).x * (s));\
    (d).y = (v1).y + ((v2).y * (s));\
    (d).z = (v1).z + ((v2).z * (s));\
    }

#define VEC_SUB(d, v1, v2) \
    {\
    (d).x = (v1).x - (v2).x;\
    (d).y = (v1).y - (v2).y;\
    (d).z = (v1).z - (v2).z;\
    }

#define VEC_MUL(d, v1, v2) \
    {\
    (d).x = (v1).x * (v2).x;\
    (d).y = (v1).y * (v2).y;\
    (d).z = (v1).z * (v2).z;\
    }

#define VEC_DIV(d, v1, v2) \
    {\
    (d).x = (v1).x / (v2).x;\
    (d).y = (v1).y / (v2).y;\
    (d).z = (v1).z / (v2).z;\
    }

#define VEC_MULSCALAR(d, v1, s) \
    {\
    (d).x = (v1).x * (s); \
    (d).y = (v1).y * (s); \
    (d).z = (v1).z * (s); \
    }

#define VEC_DIVSCALAR(d, v1, s) \
    {\
    (d).x = (v1).x / (s); \
    (d).y = (v1).y / (s); \
    (d).z = (v1).z / (s);\
    }

#define VEC_LERP(d, v1, v2, t) \
    {\
    (d).x = (v1).x + ((v2).x - (v1).x) * t;\
    (d).y = (v1).y + ((v2).y - (v1).y) * t;\
    (d).z = (v1).z + ((v2).z - (v1).z) * t;\
    }

#define VEC_CLAMP(v, a, b) \
{\
    (v).x = LTCLAMP((v).x, a, b);\
    (v).y = LTCLAMP((v).y, a, b);\
    (v).z = LTCLAMP((v).z, a, b);\
}

#define VEC_MIN(v, a, b) \
{\
    (v).x = LTMIN((a).x, (b).x);\
    (v).y = LTMIN((a).y, (b).y);\
    (v).z = LTMIN((a).z, (b).z);\
}

#define VEC_MAX(v, a, b) \
{\
    (v).x = LTMAX((a).x, (b).x);\
    (v).y = LTMAX((a).y, (b).y);\
    (v).z = LTMAX((a).z, (b).z);\
}

#define VEC_NEGATE(d, s) \
    {\
    (d).x = -(s).x; \
    (d).y = -(s).y; \
    (d).z = -(s).z; \
    }

#define VEC_DOT(v1, v2) ((v1).x*(v2).x + (v1).y*(v2).y + (v1).z*(v2).z)

#define EXPANDVEC(vec) (vec).x, (vec).y, (vec).z

//---------------------------------------------------------------------------//
#define VEC_DISTSQR(v1, v2) ( \
	((v1).x-(v2).x) * ((v1).x-(v2).x) + \
	((v1).y-(v2).y) * ((v1).y-(v2).y) + \
	((v1).z-(v2).z) * ((v1).z-(v2).z) )


//---------------------------------------------------------------------------//
#define VEC_DIST(v1, v2) ((float)sqrt(VEC_DISTSQR(v1, v2)))

#define VEC_MAGSQR(v) ((v).x*(v).x + (v).y*(v).y + (v).z*(v).z)

//---------------------------------------------------------------------------//
#define VEC_MAG(v) ((float)sqrt(VEC_MAGSQR(v)))

#define VEC_INVMAG(v) (1.0f / VEC_MAG(v))

#define VEC_NORM(v) \
    {\
        float __temp_normalizer_____;\
        __temp_normalizer_____ = 1.0f / VEC_MAG(v);\
        (v).x *= __temp_normalizer_____;\
        (v).y *= __temp_normalizer_____;\
        (v).z *= __temp_normalizer_____;\
    }

#define VEC_COPY(dest, src) \
    {\
        (dest).x = (src).x; (dest).y = (src).y; (dest).z = (src).z;\
    }

#define VEC_SET(v, vx, vy, vz) \
    {\
        (v).x = (float)(vx); (v).y = (float)(vy); (v).z = (float)(vz);\
    }

#define VEC_EXPAND(v) (v).x, (v).y, (v).z

#define VEC_INIT(v) (v).x = (v).y = (v).z = 0;

#define VEC_INDEX(v, i) (((float*)&(v))[i])


#endif//doxygen
#endif
