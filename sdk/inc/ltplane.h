#ifndef __LTPLANE_H__
#define __LTPLANE_H__

#ifndef __LTVECTOR_H__
#include "ltvector.h"
#endif

//---------------------------------------------------------------------------//
/*!
The LTPlane structure describes a plane in 3-dimensional space.  Positive
distances from the plane to a point are in the same direction as its normal.

Used for: Math.
*/
struct LTPlane {
/*!
unit normal
*/
    LTVector m_Normal;

/*!
negative of the signed distance from
the origin \f$ {\bf O} = (0,0,0)\f$
to the plane
*/
    float m_Dist;

/*!
The default constructor.

Used for: Math.
*/
    LTPlane() {}

/*!
\param  n   a unit normal
\param  P   a point on the plane

Construct a plane with a unit normal \f$ {\bf n} \f$ and
a point \f$ {\bf p} \f$ on the plane.

Used for: Math.
*/
    LTPlane
    (
        const LTVector& n,  //unit normal
        const LTVector& P   //any point on plane
    )
    {
        m_Normal = n;
        m_Dist = n.Dot(P);
    }

/*!
\param  n   a unit normal
\param  P   a point on the plane

Initialize a plane with a unit normal \f$ {\bf n} \f$ and
a point \f$ {\bf p} \f$ on the plane.

Used for: Math.
*/
    void Init(const LTVector& n, const LTVector& P) {
        m_Normal = n;
        m_Dist = n.Dot(P);
    }

/*!
\param  x   \f$ n_x \f$
\param  y   \f$ n_y \f$
\param  z   \f$ n_z \f$
\param  d   the negative of the signed distance from
            the origin to the plane

Construct a plane with a unit normal \f$ {\bf n}=(x,y,z) \f$ and
the negative of the signed distance from the origin
\f$ {\bf O} = (0,0,0)\f$ to the plane.

Used for: Math.
*/
    LTPlane(float x, float y, float z, float d) {
        m_Normal.Init(x,y,z);
        m_Dist = d;
    }

/*!
\param  n   a unit normal
\param  P   a point on the plane

Construct a plane with a unit normal \f$ {\bf n} \f$ and
the negative of the signed distance from the origin
\f$ {\bf O} = (0,0,0)\f$ to the plane.

Used for: Math.
*/
    LTPlane(const LTVector& n, float d) {
        m_Normal = n;
        m_Dist = d;
    }

/*!
\param  n   a unit normal
\param  P   a point on the plane

Initialize a plane with a unit normal \f$ {\bf n} \f$ and
the negative of the signed distance from the origin
\f$ {\bf O} = (0,0,0)\f$ to the plane.

Used for: Math.
*/
    void Init(const LTVector& n, float d) {
        m_Normal = n;
        m_Dist = d;
    }

/*!
\return     the unit normal \f$ {\bf n} \f$ of the plane

Used for: Math.
*/
    LTVector& Normal() {
        return m_Normal;
    }

/*!
\return     the negative of the signed distance from the origin
            \f$ {\bf O} = (0,0,0)\f$ to the plane

Used for: Math.
*/
    float& Dist() {
        return m_Dist;
    }

/*!
\param  p   a 3D point
\return     the signed distance from the plane to the point \f${\bf p}\f$

Compute the signed distance from the plane to the point \f${\bf p}\f$.

Used for: Math.
*/
    float DistTo(const LTVector& p) const {
        return m_Normal.Dot(p) - m_Dist;
    }

/*!
\return     a plane with the values \f$ \left{ -{\bf n}, -dist \right} \f$

Negate the values of \b m_Normal and \b m_Dist.  This creates a plane
that is a reflection about the origin of the first (so they are parallel),
and whose normal points the opposite direction.

Used for: Math.
*/
    const LTPlane operator-() {
        return LTPlane(-m_Normal, -m_Dist);
    }
};


#endif
//EOF
