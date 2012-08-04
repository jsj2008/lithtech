/*!  This module contains functions to deal with 2D bezier curve
segments.  DEdit uses the same functions.  */

#ifndef __LTBEZIERCURVE_H__
#define __LTBEZIERCURVE_H__


#include "ltbasetypes.h"
#include "ltvector.h"


/*!
The default number of subdivisions when evaluating a cubic Bezier curve.
*/
#define DEFAULT_BEZIER_SUBDIVISIONS 20


/*!
\param	out		\f$ {\bf B}(t) \f$, the value of the curve at t
\param	p1		the first control point
\param	p2		the second control point
\param	p3		the third control point
\param	p4		the fourth control point
\param	t		the value of the curve parameter

Given the control points
\f$ \left{ {\bf p}_1, {\bf p}_2, {\bf p}_3, {\bf p}_4 \right} \f$,
evaluate the cubic Bezier curve at \f$ t \in [0,1] \f$.

Used for: Math.
*/
inline void Bezier_Evaluate
(
	LTVector& out,
	LTVector& p1,
	LTVector& p2,
	LTVector& p3,
	LTVector& p4,
	float t
)
{
	float oneMinusT[3];
	float tSquared;

	oneMinusT[0] = 1.0f - t;
	oneMinusT[1] = oneMinusT[0] * oneMinusT[0];
	oneMinusT[2] = oneMinusT[1] * oneMinusT[0];

	tSquared = t * t;

	out = p1 * oneMinusT[2] +
		p2 * (3.0f * t * oneMinusT[1]) +
		p3 * (3.0f * tSquared * oneMinusT[0]) +
		p4 * (tSquared * t);
}


/*!
\param	p1		the first control point
\param	p2		the first control point
\param	p3		the first control point
\param	p4		the first control point
\param	t1		the beginning value of the curve parameter
\param	t2		the ending value of the curve parameter
\param	n		the number of subdivisions

Given the control points
\f$ \left{ {\bf p}_1, {\bf p}_2, {\bf p}_3, {\bf p}_4 \right} \f$,
approximate the arclength of the cubic Bezier curve from \f$ t_1 \f$
to \f$ t_2 \f$ using \f$ n \f$ chordlengths.

Used for: Math.
*/
inline float Bezier_SubSegmentLength
(
	LTVector& p1,
	LTVector& p2,
	LTVector& p3,
	LTVector& p4,
	float t1,
	float t2,
	uint32 n = DEFAULT_BEZIER_SUBDIVISIONS
)
{
	LTVector pa, pb;//chord endpoints
	float S;//arclength
	float t;//curve parameter
	float dt;//parameter increment

    n = LTMAX(n, 1);//check for negative?

	S = 0.0f;

	//B(t1)
	Bezier_Evaluate( pa, p1, p2, p3, p4, t1 );

	dt = (t2 - t1) / n;
	t = t1 + dt;

	while( n )
	{
		n--;

		Bezier_Evaluate( pb, p1, p2, p3, p4, t );

		//ith chordlength
		S += (pb - pa).Mag();
		t += dt;
		pa = pb;
	}

	//approximate arclength
	return S;
}

/*!
\param	p1		the first control point
\param	p2		the first control point
\param	p3		the first control point
\param	p4		the first control point
\param	n		the number of subdivisions

Given the control points
\f$ \left{ {\bf p}_1, {\bf p}_2, {\bf p}_3, {\bf p}_4 \right} \f$,
approximate the entire arclength of the cubic Bezier curve using
\f$ n \f$ chordlengths.

Used for: Math.
*/
inline float Bezier_SegmentLength
(
	LTVector &p1,
	LTVector &p2,
	LTVector &p3,
	LTVector &p4,
	uint32 n = DEFAULT_BEZIER_SUBDIVISIONS
)
{
	return Bezier_SubSegmentLength( p1, p2, p3, p4, 0, 1, n );
}

#endif
//EOF
