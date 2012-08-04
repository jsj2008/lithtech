#ifndef __ILTTRANSFORM_H__
#define __ILTTRANSFORM_H__


#ifndef __LTBASEDEFS_H__
#include "ltbasedefs.h"
#endif

#ifndef __LTMODULE_H__
#include "ltmodule.h"
#endif


/*! 
The ILTTransform interface exposes various routines for dealing with
LTransforms, which can used to represent local coordinate frames. 

Define a holder to get this interface like this:
\code
define_holder(ILTTransform, your_var);
\endcode
*/


class ILTTransform : public IBase
{
public:
    interface_version(ILTTransform, 0);

/*!
\param T the LTransform object from which to copy \f${\bf p}\f$ and \f${\bf q}\f$
\param p the position of T
\param q the orientation of T
\return \b LT_OK

Copy the information contained in \f${\bf T}\f$ to \f${\bf p}\f$ and
\f${\bf q}\f$.

Used for: Math.
*/
    virtual LTRESULT Get(LTransform& T, LTVector &p, LTRotation &q) = 0;

/*!
\param T the LTransform object to which to copy \f${\bf p}\f$ and \f${\bf q}\f$
\param p a position
\param q an orientation
\return \b LT_OK

Copy the position \f${\bf p}\f$ and the rotation \f${\bf q}\f$ to
\f${\bf T}\f$.

Used for: Math.
*/
    virtual LTRESULT Set(LTransform &T, LTVector &p, LTRotation &q) = 0;

/*!
\param T the LTransform object from which to copy \f${\bf p}\f$
\param p the position of T
\return \b LT_OK

Copy the position of \f${\bf T}\f$ to \f${\bf p}\f$.

Used for: Math.
*/
    virtual LTRESULT GetPos(LTransform &T, LTVector &p) = 0;

/*!
\param  T   the LTransform object from which to copy \f${\bf q}\f$
\param  q   the orientation of T
\return \b LT_OK

Copy the rotation of \f${\bf T}\f$ to \f${\bf q}\f$.

Used for: Math.
*/
    virtual LTRESULT GetRot(LTransform &T, LTRotation &q) = 0;

/*!
\param  T   the LTransform object
\param  M   the 4x4 transformation matrix
\return \b LT_OK

Given a transform \f${\bf T}\f$, create a 4x4 matrix of the form

\f[
    {\bf M} =
    \left[
    {\begin{array}{*{20}c}
        {1 - 2y^2  - 2z^2 } & {2xy - 2rz} & {2xz + 2ry} & {O_x } \\
        {2xy + 2rz} & {1 - 2x^2  - 2z^2 } & {2yz - 2rx} & {O_y } \\
        {2xz - 2ry} & {2yz + 2rx} & {1 - 2x^2  - 2y^2 } & {O_z } \\
        0 & 0 & 0 & 1                           \\
    \end{array}}
    \right].
\f]

This matrix transforms a homogeneous point \f${\bf p}=(x,y,z,1)\f$ in a
local coordinate frame to a homogeneous point in its parent coordinate
frame according to the formula

\f[
{\bf p}_{parent} = {\bf M}*{\bf p}_{local}.
\f]

Used for: Math.
*/
    virtual LTRESULT ToMatrix(LTransform &T, LTMatrix &M) = 0;

/*!
\param  T   the LTransform object
\param  M   the 4x4 transformation matrix
\return \b LT_OK

Given a 4x4 matrix of the form

\f[
    {\bf M} =
    \left[
    {\begin{array}{*{20}c}
        {1 - 2y^2  - 2z^2 } & {2xy - 2rz} & {2xz + 2ry} & {O_x } \\
        {2xy + 2rz} & {1 - 2x^2  - 2z^2 } & {2yz - 2rx} & {O_y } \\
        {2xz - 2ry} & {2yz + 2rx} & {1 - 2x^2  - 2y^2 } & {O_z } \\
        0 & 0 & 0 & 1                           \\
    \end{array}}
    \right],
\f]

create the corresponding LTransform \f${\bf T}\f$.

Used for: Math.
*/
    virtual LTRESULT FromMatrix(LTransform &T, LTMatrix &M) = 0;

/*!
\param  T_inv   \f${\bf T^{-1}}\f$
\param  T       \f${\bf T}\f$
\return \b LT_OK

Given a LTransform object \f${\bf T}\f$, compute its inverse \f${\bf T^{-1}}\f$
such that the product \f$ {\bf T} * {\bf T^{-1}} \f$ results in a transformation
equal to \f$ \left{ {\bf p}=(0,0,0), {\bf q}=(0,0,0,1) \right} \f$.

Used for: Math.
*/
    virtual LTRESULT Inverse(LTransform &T_inv, LTransform &T) = 0;

/*!
\param  Tab     \f${\bf T_a} * {\bf T_b}\f$
\param  Ta      \f${\bf T_a}\f$
\param  Tb      \f${\bf T_b}\f$
\return \b LT_OK

Given two LTransform objects \f${\bf T_a}\f$ and \f${\bf T_b}\f$, compute
the composite transformation

\f[
    {\bf T_{ab}} = {\bf T_a} * {\bf T_b}.
\f]

Used for: Math.
*/
    virtual LTRESULT Multiply(LTransform &Tab, LTransform &Ta, LTransform &Tb) = 0;

/*!
\param  diff    \f${{\bf T}_b^{-1}} * {\bf T_a}\f$
\param  Ta      \f${\bf T_a}\f$
\param  Tb      \f${\bf T_b}\f$

Given two LTransform objects \f${\bf T_a}\f$ and \f${\bf T_b}\f$, compute
the composite transformation

\f[
    {\bf T_{diff}} = {{\bf T}_b^{-1}} * {\bf T_a}.
\f]

This is equivalent to rotating and translating by \f${\bf T_a}\f$, then
rotating and translating in the direction opposite to \f${\bf T_b}\f$.

Used for: Math.
*/
    virtual LTRESULT Difference(LTransform &diff, LTransform &t1, LTransform &t2) = 0;
};


#endif  //! __ILTTRANSFORM_H__



