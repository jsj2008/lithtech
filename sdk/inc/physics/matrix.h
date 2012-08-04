#ifndef __MATRIX_H__
#define __MATRIX_H__

#ifndef __VECTOR_H__
#include "vector.h"
#endif


//---------------------------------------------------------------------------//
/*!
The LTMatrix3f data type represents a 3x3 matrix as three floating point column
vectors.  Functions are provided for all basic matrix operations.

Used for: Math.
*/
struct LTMatrix3f
{
    /*!Column vectors*/
    LTVector3f A[3];

    /*!
    \note For efficiency, the default constructor does nothing.

    Used for:  Math.
    */
    LTMatrix3f()
    {}

    /*!
    \param a0 first column
    \param a1 second column
    \param a2 third column

    Used For: Math.
    */
    LTMatrix3f
    (
        const LTVector3f& a0,
        const LTVector3f& a1,
        const LTVector3f& a2
    )
    {
        A[0] = a0;
        A[1] = a1;
        A[2] = a2;
    }

    /*!
    \return     the identity

    Set to Identity.

    Used For: Math.
    */
    const LTMatrix3f& Identity()
    {
        //Identity ([col][row] indexing)
        A[0][0] = 1;    A[1][0] = 0;    A[2][0] = 0;
        A[0][1] = 0;    A[1][1] = 1;    A[2][1] = 0;
        A[0][2] = 0;    A[1][2] = 0;    A[2][2] = 1;

        return *this;
    }

    /*!
    \param  row
    \param  col
    \return     \f${\bf M_{row,col}}\f$

    Used For: Math.
    */
    float operator () (const int32 row, const int32 col) const
    {
        return A[col][row];
    }

    /*!
    \param  row
    \param  col
    \return     \f${\bf M_{row,col}}\f$

    Used For: Math.
    */
    float& operator () (const int32 row, const int32 col)
    {
        return A[col][row];
    }

    /*!
    \param  col
    \return     \f${\bf M_{col}}\f$

    Index a column vector

    Used For: Math.
    */
    const LTVector3f& operator () (const int32 col) const
    {
        return A[col];
    }

    /*!
    \param  col
    \return     \f${\bf M_{col}}\f$

    Index a column vector

    Used For: Math.
    */
    LTVector3f& operator () (const int32 col)
    {
        return A[col];
    }

    /*!
    \param  B   Another matrix.
    \return     \b true if the matrices are equal,
                \b false otherwise

    Used For: Math.
    */
    bool operator == (const LTMatrix3f& B) const
    {
        return A[0]==B(0) && A[1]==B(1) && A[2]==B(2);
    }

    /*!
    \param  B   Another matrix.
    \return     \b true if the matrices are \b not equal,
                \b false otherwise

    Used For: Math.
    */
    bool operator != (const LTMatrix3f& B) const
    {
        return !(B == *this);
    }

    /*!
    \return \f$ -{\bf A} \f$.

    Used For: Math.
    */
    const LTMatrix3f operator - () const
    {
        return LTMatrix3f(-A[0], -A[1], -A[2]);
    }

    /*!
    \param  B   Another matrix.
    \return     \f$ {\bf A}+={\bf B} \f$.

    Used For: Math.
    */
    const LTMatrix3f& operator += (const LTMatrix3f& B)
    {
        A[0] += B(0);
        A[1] += B(1);
        A[2] += B(2);

        return *this;
    }

    /*!
    \param  B   Another matrix.
    \return     \f$ {\bf A}-={\bf B} \f$.

    Used For: Math.
    */
    const LTMatrix3f& operator -= (const LTMatrix3f& B) 
    {
        A[0] -= B(0);
        A[1] -= B(1);
        A[2] -= B(2);

        return *this;
    }

    /*!
    \param  s   A scalar value.
    \return     \f$ {\bf A}*=s \f$.

    Used For: Math.
    */
    const LTMatrix3f& operator *= (const float s)
    {
        A[0] *= s;
        A[1] *= s;
        A[2] *= s;

        return *this;
    }

    /*!
    \param  s   A scalar value.
    \return     \f$ {\bf A}/=s \f$.

    Used For: Math.
    */
    const LTMatrix3f& operator /= (const float s)
    {
        const float r = 1/s;

        A[0] *= r;
        A[1] *= r;
        A[2] *= r;

        return *this;
    }

    /*!
    \param  B   Another matrix.
    \return     \f$ {\bf A}+{\bf B} \f$.

    Used For: Math.
    */
    const LTMatrix3f operator + (const LTMatrix3f& B) const
    {
        return LTMatrix3f(A[0] + B(0), A[1] + B(1), A[2] + B(2));
    }

    /*!
    \param  B   Another matrix.
    \return     \f$ {\bf A}-{\bf B} \f$.

    Used For: Math.
    */
    const LTMatrix3f operator - (const LTMatrix3f& B) const
    {
        return LTMatrix3f(A[0] - B(0), A[1] - B(1), A[2] - B(2));
    }

    /*!
    \param  s   A scalar value.
    \return     \f$ {\bf M}/s \f$.

    Used For: Math.
    */
    const LTMatrix3f operator / (const float s) const
    {
        const float r = 1/s;

        return LTMatrix3f(A[0]*r, A[1]*r, A[2]*r);
    }

    /*!
    \param  s   A scalar value.
    \return     \f$ s{\bf M} \f$.

    Used For: Math.
    */
    const LTMatrix3f operator * (const float s) const
    {
        return LTMatrix3f(A[0]*s, A[1]*s, A[2]*s);
    }

    /*!
    \param  s   A scalar value.
    \param  M   A 3x3 matrix.
    \return     \f$ s{\bf M} \f$.

    Used For: Math.
    */
    friend const LTMatrix3f operator * (const float s, const LTMatrix3f& M)
    {
        return M * s;
    }

    /*!
    \param  v   A 3D vector.
    \return     \f$ {\bf A}{\bf v} \f$.

    Used For: Math.
    */
    const LTVector3f operator * (const LTVector3f& v) const
    {
        return v.x*A[0] + v.y*A[1] + v.z*A[2];
    }

    /*!
    \param  v   A 3D vector.
    \param  A   Another matrix
    \return     \f$ {\bf v}{\bf A} \f$.

    Used For: Math.
    */
    friend const LTVector3f operator * (const LTVector3f&  v, const LTMatrix3f& A)
    {
        //same as 
        return LTVector3f(v.Dot(A(0)), v.Dot(A(1)), v.Dot(A(2)));
    }

    /*!
    \param  B   A 3x3 matrix
    \return     \f$ {\bf A}{\bf B} \f$.

    Used For: Math.
    */
    const LTMatrix3f operator * (const LTMatrix3f& B) const
    {
        return LTMatrix3f((*this) * B(0), (*this) * B(1), (*this) * B(2));
    }

    /*!
    \return \f$ {\bf A}^T \f$.

    Used For: Math.
    */
    const LTMatrix3f Transpose() const
    {
        //place columns into rows
        return LTMatrix3f(
                        LTVector3f(A[0].x, A[1].x, A[2].x),   //col 0
                        LTVector3f(A[0].y, A[1].y, A[2].y),   //col 1
                        LTVector3f(A[0].z, A[1].z, A[2].z)    //col 2
                     );
    }

    /*!
    \param  M   A 3x3 matrix.
    \return     the determinant of \f$ {\bf M} \f$

    Used For: Math.
    */
    friend float Det(const LTMatrix3f& M)
    {
        //volume of the parallelpiped formed
        //from the column vectors
        return M(0).Dot(M(1).Cross(M(2)));
    }

    /*!
    \return \f$ {\bf M^{-1}} \f$

    Used For: Math.
    */
    const LTMatrix3f Inverse() const
    {
        const float s = 1 / Det(*this);
        const LTMatrix3f M(A[1].Cross(A[2]),
                            A[2].Cross(A[0]),
                            A[0].Cross(A[1]));

        return s * M.Transpose();
    }
};


//---------------------------------------------------------------------------//
/*!
The LTSymMat3f data type represents a 3x3 symmetric matrix with 6
single-precision floating-point values (3 diagonal components and 3 off-
diagonal elements).  Functions are provided for all basic matrix operations.

\see    LTVector3f, LTMatrix3f

Used for: Math.
*/
class LTSymMat3f
{
public:

    /*! diagonal elements */
    float xx, yy, zz;
    /*! off-diagonal elements */
    float xy, xz, yz;

public:

    /*!
    \note For efficiency, the default constructor does nothing.

    Used for:  Math.
    */
    LTSymMat3f()
    {
        xx = yy = zz = 1;
        xy = xz = yz = 0;
    }

    /*!
    Initialization constructor.

    Used For: Math.
    */
    LTSymMat3f
    (
        const float xx,
        const float yy,
        const float zz,
        const float xy,
        const float xz,
        const float yz
    )
    {
        this->xx = xx;
        this->yy = yy;
        this->zz = zz;

        this->xy = xy;
        this->xz = xz;
        this->yz = yz;
    }

 
	/*!
	\return \f$ -{\bf A} \f$.

	Used For: Math.
	*/
	const LTSymMat3f operator - () const
	{
		return LTSymMat3f( -xx, -yy, -zz, -xy, -xz, -yz );
	}

	/*!
	\param B The matrix to be incremented.
	\return \f$ {\bf A}+={\bf B} \f$.


    Used For: Math.
    */
    const LTSymMat3f& operator += (const LTSymMat3f& B)
    {
        this->xx += B.xx;
        this->yy += B.yy;
        this->zz += B.zz;

        this->xy += B.xy;
        this->xz += B.xz;
        this->yz += B.yz;

        return *this;
    }

    /*!
    \param B The matrix to be decremented.
    \return \f$ {\bf A}-={\bf B} \f$.

    Used For: Math.
    */
    const LTSymMat3f& operator -=(const LTSymMat3f& B) 
    {
        this->xx -= B.xx;
        this->yy -= B.yy;
        this->zz -= B.zz;

        this->xy -= B.xy;
        this->xz -= B.xz;
        this->yz -= B.yz;

        return *this;
    }

    /*!
    \param B The matrix to add to this matrix.
    \return \f$ {\bf A}+{\bf B} \f$.

    Used For: Math.
    */
    LTSymMat3f operator + (const LTSymMat3f& B) const
    {
        return LTSymMat3f(xx + B.xx,yy + B.yy,zz + B.zz,
                            xy + B.xy,xz + B.xz,yz + B.yz);
    }

    /*!
    \param B The matrix to subtract from this matrix.
    \return \f$ {\bf A}-{\bf B} \f$.

    Used For: Math.
    */
    LTSymMat3f operator - (const LTSymMat3f& B) const
    {
        return LTSymMat3f(xx - B.xx,yy - B.yy,zz - B.zz,
                            xy - B.xy,xz - B.xz,yz - B.yz);
    }

    /*!
    \param  v   a 3 dimensional vector
    \return     \f$ {\bf A}{\bf v} \f$.

    Used For: Math.
    */
    LTVector3f operator * (const LTVector3f& v) const
    {
        return LTVector3f(v.x*xx + v.y*xy + v.z*xz,
                            v.x*xy + v.y*yy + v.z*yz,
                            v.z*xz + v.y*yz + v.z*zz);
    }

    /*!
    \param v    a 3 dimensional vector
    \param B    A 3x3 matrix
    \return     \f$ {\bf v}{\bf A} \f$.

    Used For: Math.
    */
    friend LTVector3f operator * (const LTVector3f& v, const LTSymMat3f& A)
    {
        //this works because A is symmetric
        return A * v;
    }
};


//---------------------------------------------------------------------------//
/*!
\param  a   a 3 dimensional vector
\param  b   a 3 dimensional vector
\return     the 3x3 outer product matrix

Compute the outer product of the vectors \b a and \b b:

    \f[
        {\bf M} = {\bf a}{\bf b}^T = 
        \left[
        {\begin{array}{*{20}c}
            {a_0 b_0} & {a_0 b_1} & {a_0 b_2}   \\
            {a_1 b_0} & {a_1 b_1} & {a_1 b_2}   \\
            {a_2 b_0} & {a_2 b_1} & {a_2 b_2}   \\
        \end{array}}
        \right].
    \f]

Used For: Math.
*/
inline const LTMatrix3f OuterProduct(const LTVector3f& a, const LTVector3f& b)
{
    LTMatrix3f M;

    //rows
    M.A[0] = a.x * b;
    M.A[1] = a.y * b;
    M.A[2] = a.z * b;

    return M;
}


#endif
//EOF
