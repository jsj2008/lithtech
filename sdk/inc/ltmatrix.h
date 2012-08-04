#ifndef __LTMATRIX_H__
#define __LTMATRIX_H__

#ifndef __MATH_H__
#include <math.h>
#define __MATH_H__
#endif

 
#include <math.h>
#ifndef __LTPLANE_H__
	#include "ltplane.h"
#endif

#ifndef __LTSYSOPTIM_H__
	#include "ltsysoptim.h"
#endif



#ifndef DOXYGEN_SHOULD_SKIP_THIS
struct LTMatrix {
    float   m[4][4];
    void Apply(LTMatrix& B) const  
	{
        B = (*this) * B;
    }

    void Apply(LTVector& p) const
	{
        float x, y, z;

        x = p.x*m[0][0] + p.y*m[0][1] + p.z*m[0][2] + m[0][3];
        y = p.x*m[1][0] + p.y*m[1][1] + p.z*m[1][2] + m[1][3];
        z = p.x*m[2][0] + p.y*m[2][1] + p.z*m[2][2] + m[2][3];

        p.x = x;
        p.y = y;
        p.z = z;
    }

    void Apply(const LTVector &p, LTVector &d) const
	{
        d.x = p.x*m[0][0] + p.y*m[0][1] + p.z*m[0][2] + m[0][3];
        d.y = p.x*m[1][0] + p.y*m[1][1] + p.z*m[1][2] + m[1][3];
        d.z = p.x*m[2][0] + p.y*m[2][1] + p.z*m[2][2] + m[2][3];
    }

    void Apply4x4(const LTVector &p, LTVector &d) const
	{
        float w, oneOverW;

        w = p.x*m[3][0] + p.y*m[3][1] + p.z*m[3][2] + m[3][3];
        oneOverW = 1.0f / w;

        d.x = (p.x*m[0][0] + p.y*m[0][1] + p.z*m[0][2] + m[0][3]) * oneOverW;
        d.y = (p.x*m[1][0] + p.y*m[1][1] + p.z*m[1][2] + m[1][3]) * oneOverW;
        d.z = (p.x*m[2][0] + p.y*m[2][1] + p.z*m[2][2] + m[2][3]) * oneOverW;
    }

    void Apply3x3(LTVector &v) const
	{
        LTVector temp;
        this->Apply3x3(v, temp);
        v = temp;
    }

 
    void Apply3x3(const LTVector &v, LTVector &d) const
	{
        d.x = v.x*m[0][0] + v.y*m[0][1] + v.z*m[0][2];
        d.y = v.x*m[1][0] + v.y*m[1][1] + v.z*m[1][2];
        d.z = v.x*m[2][0] + v.y*m[2][1] + v.z*m[2][2];
    }


	const float& El(uint32 i, uint32 j) const
	{
        return m[i][j];
    }

    float& El(uint32 i, uint32 j) 
	{
        return m[i][j];
    }

    void GetBasisVectors(LTVector *R0, LTVector *R1, LTVector *R2) const
	{
        R0->x = m[0][0];
        R0->y = m[1][0];
        R0->z = m[2][0];

        R1->x = m[0][1];
        R1->y = m[1][1];
        R1->z = m[2][1];

        R2->x = m[0][2];
        R2->y = m[1][2];
        R2->z = m[2][2];
    }

    LTVector GetScale() const
	{
        return LTVector(
            (float)sqrt(m[0][0]*m[0][0] + m[1][0]*m[1][0] + m[2][0]*m[2][0]),
            (float)sqrt(m[0][1]*m[0][1] + m[1][1]*m[1][1] + m[2][1]*m[2][1]),
            (float)sqrt(m[0][2]*m[0][2] + m[1][2]*m[1][2] + m[2][2]*m[2][2])
          );
    }

    void GetTranslation(LTVector &t) const
	{
        t.x = m[0][3];
        t.y = m[1][3];
        t.z = m[2][3];
    }

    void Identity() 
	{
        Init(1.0f, 0.0f, 0.0f, 0.0f,
             0.0f, 1.0f, 0.0f, 0.0f,
             0.0f, 0.0f, 1.0f, 0.0f,
             0.0f, 0.0f, 0.0f, 1.0f);
    }

    void Init (float m00, float m01, float m02, float m03,
               float m10, float m11, float m12, float m13,
               float m20, float m21, float m22, float m23,
               float m30, float m31, float m32, float m33)
    {
        m[0][0] = m00;  m[0][1] = m01;  m[0][2] = m02;  m[0][3] = m03;
        m[1][0] = m10;  m[1][1] = m11;  m[1][2] = m12;  m[1][3] = m13;
        m[2][0] = m20;  m[2][1] = m21;  m[2][2] = m22;  m[2][3] = m23;
        m[3][0] = m30;  m[3][1] = m31;  m[3][2] = m32;  m[3][3] = m33;
    }

    bool Inverse();

    LTMatrix MakeInverse() const
	{
        LTMatrix ret;

        ret = *this;
        ret.Inverse();
        return ret;
    }

    LTMatrix MakeInverseTransform() const;

    void Normalize() 
	{
        LTVector right, up, forward;
        float len;


        GetBasisVectors(&right, &up, &forward);
        
        len = VEC_MAG(right);
        VEC_DIVSCALAR(right, right, len);
        
        len = VEC_MAG(up);
        VEC_DIVSCALAR(up, up, len);
        
        VEC_CROSS(forward, up, right);
        SetBasisVectors2(&right, &up, &forward);
    }

    void Scale(float x, float y, float z) 
	{
        m[0][0] *= x;
        m[1][0] *= x;
        m[2][0] *= x;

        m[0][1] *= y;
        m[1][1] *= y;
        m[2][1] *= y;

        m[0][2] *= z;
        m[1][2] *= z;
        m[2][2] *= z;
    }

    void SetBasisVectors(const LTVector *R0, const LTVector *R1, const LTVector *R2) 
	{
        Init(R0->x, R1->x, R2->x, 0.0f, 
             R0->y, R1->y, R2->y, 0.0f, 
             R0->z, R1->z, R2->z, 0.0f, 
             0.0f, 0.0f, 0.0f, 1.0f);
    }

    void SetBasisVectors2(const LTVector* R0, const LTVector* R1, const LTVector* R2) 
	{
        m[0][0] = R0->x;
        m[1][0] = R0->y;
        m[2][0] = R0->z;

        m[0][1] = R1->x;
        m[1][1] = R1->y;
        m[2][1] = R1->z;
        
        m[0][2] = R2->x;
        m[1][2] = R2->y;
        m[2][2] = R2->z;
    }

    void SetTranslation(const LTVector& t) 
	{
        SetTranslation(VEC_EXPAND(t));
    }

    void SetTranslation(float x, float y, float z) 
	{
        m[0][3] = x;
        m[1][3] = y;
        m[2][3] = z;
    }

    void SetupProjectionMatrix(const LTVector& O, const LTPlane& plane) 
	{
        const LTVector& n = plane.m_Normal;
        const float D = plane.m_Dist;
        float dot;

        //Find the dot product between light position
        //RVector and ground plane normal.
        dot = n[0]*O.x + n[1]*O.y + n[2]*O.z - D;

        m[0][0] = dot - O.x * n[0];
        m[0][1] = -O.x * n[1];
        m[0][2] = -O.x * n[2];
        m[0][3] = -O.x * -D;

        m[1][0] = -O.y * n[0];
        m[1][1] = dot - O.y * n[1];
        m[1][2] = -O.y * n[2];
        m[1][3] = -O.y * -D;

        m[2][0] = -O.z * n[0];
        m[2][1] = -O.z * n[1];
        m[2][2] = dot - O.z * n[2];
        m[2][3] = -O.z * -D;

        m[3][0] = -n[0];
        m[3][1] = -n[1];
        m[3][2] = -n[2];
        m[3][3] = dot + D;
    }

    void SetupReflectionMatrix(const LTVector& n, const LTVector& p0);

    void SetupRot(const LTVector& u, float a);

    void SetupScalingMatrix(const LTVector& s) 
	{
        Init(s.x, 0.0f, 0.0f, 0.0f,
             0.0f, s.y, 0.0f, 0.0f,
             0.0f, 0.0f, s.z, 0.0f,
             0.0f, 0.0f, 0.0f, 1.0f);
    }

	void Transpose()
	{
		float temp;
		float *d = (float*)m;

		temp = d[1]; d[1] = d[4]; d[4] = temp;
		temp = d[2]; d[2] = d[8]; d[8] = temp;
		temp = d[9]; d[9] = d[6]; d[6] = temp;

		temp = d[3];  d[3] =  d[12]; d[12] = temp;
		temp = d[7];  d[7] =  d[13]; d[13] = temp;
		temp = d[11]; d[11] = d[14]; d[14] = temp;
	}

	void Transpose3x3()
	{
		float temp;
		float *d = (float*)m;

		temp = d[1]; d[1] = d[4]; d[4] = temp;
		temp = d[2]; d[2] = d[8]; d[8] = temp;
		temp = d[9]; d[9] = d[6]; d[6] = temp;
	}


    LTPlane TransformPlane(const LTPlane& P);

    LTVector operator*(const LTVector& v) const;

    LTMatrix operator*(const LTMatrix& M) const;

    LTMatrix operator~() const 
	{
        return MakeInverseTransform();
    }
};


inline void Mat_Identity(LTMatrix *M) 
{
    M->Identity();
}

inline void Mat_GetTranslation(const LTMatrix &mat, LTVector &vec) 
{
    mat.GetTranslation(vec);
}

inline void Mat_SetTranslation(LTMatrix &mat, const LTVector &vec) 
{
    mat.SetTranslation(vec);
}

inline void MatTranspose3x3(LTMatrix *pMat) 
{
	pMat->Transpose3x3();
}

inline void MatTranspose(LTMatrix *pMat) 
{
	pMat->Transpose();
}

inline void MatMul(LTMatrix *pDest, const LTMatrix *pMat1, const LTMatrix *pMat2) 
{
    pDest->m[0][0] = pMat1->m[0][0]*pMat2->m[0][0] + pMat1->m[0][1]*pMat2->m[1][0] + pMat1->m[0][2]*pMat2->m[2][0] + pMat1->m[0][3]*pMat2->m[3][0];
    pDest->m[1][0] = pMat1->m[1][0]*pMat2->m[0][0] + pMat1->m[1][1]*pMat2->m[1][0] + pMat1->m[1][2]*pMat2->m[2][0] + pMat1->m[1][3]*pMat2->m[3][0];
    pDest->m[2][0] = pMat1->m[2][0]*pMat2->m[0][0] + pMat1->m[2][1]*pMat2->m[1][0] + pMat1->m[2][2]*pMat2->m[2][0] + pMat1->m[2][3]*pMat2->m[3][0];
    pDest->m[3][0] = pMat1->m[3][0]*pMat2->m[0][0] + pMat1->m[3][1]*pMat2->m[1][0] + pMat1->m[3][2]*pMat2->m[2][0] + pMat1->m[3][3]*pMat2->m[3][0];

    pDest->m[0][1] = pMat1->m[0][0]*pMat2->m[0][1] + pMat1->m[0][1]*pMat2->m[1][1] + pMat1->m[0][2]*pMat2->m[2][1] + pMat1->m[0][3]*pMat2->m[3][1];
    pDest->m[1][1] = pMat1->m[1][0]*pMat2->m[0][1] + pMat1->m[1][1]*pMat2->m[1][1] + pMat1->m[1][2]*pMat2->m[2][1] + pMat1->m[1][3]*pMat2->m[3][1];
    pDest->m[2][1] = pMat1->m[2][0]*pMat2->m[0][1] + pMat1->m[2][1]*pMat2->m[1][1] + pMat1->m[2][2]*pMat2->m[2][1] + pMat1->m[2][3]*pMat2->m[3][1];
    pDest->m[3][1] = pMat1->m[3][0]*pMat2->m[0][1] + pMat1->m[3][1]*pMat2->m[1][1] + pMat1->m[3][2]*pMat2->m[2][1] + pMat1->m[3][3]*pMat2->m[3][1];

    pDest->m[0][2] = pMat1->m[0][0]*pMat2->m[0][2] + pMat1->m[0][1]*pMat2->m[1][2] + pMat1->m[0][2]*pMat2->m[2][2] + pMat1->m[0][3]*pMat2->m[3][2];
    pDest->m[1][2] = pMat1->m[1][0]*pMat2->m[0][2] + pMat1->m[1][1]*pMat2->m[1][2] + pMat1->m[1][2]*pMat2->m[2][2] + pMat1->m[1][3]*pMat2->m[3][2];
    pDest->m[2][2] = pMat1->m[2][0]*pMat2->m[0][2] + pMat1->m[2][1]*pMat2->m[1][2] + pMat1->m[2][2]*pMat2->m[2][2] + pMat1->m[2][3]*pMat2->m[3][2];
    pDest->m[3][2] = pMat1->m[3][0]*pMat2->m[0][2] + pMat1->m[3][1]*pMat2->m[1][2] + pMat1->m[3][2]*pMat2->m[2][2] + pMat1->m[3][3]*pMat2->m[3][2];

    pDest->m[0][3] = pMat1->m[0][0]*pMat2->m[0][3] + pMat1->m[0][1]*pMat2->m[1][3] + pMat1->m[0][2]*pMat2->m[2][3] + pMat1->m[0][3]*pMat2->m[3][3];
    pDest->m[1][3] = pMat1->m[1][0]*pMat2->m[0][3] + pMat1->m[1][1]*pMat2->m[1][3] + pMat1->m[1][2]*pMat2->m[2][3] + pMat1->m[1][3]*pMat2->m[3][3];
    pDest->m[2][3] = pMat1->m[2][0]*pMat2->m[0][3] + pMat1->m[2][1]*pMat2->m[1][3] + pMat1->m[2][2]*pMat2->m[2][3] + pMat1->m[2][3]*pMat2->m[3][3];
    pDest->m[3][3] = pMat1->m[3][0]*pMat2->m[0][3] + pMat1->m[3][1]*pMat2->m[1][3] + pMat1->m[3][2]*pMat2->m[2][3] + pMat1->m[3][3]*pMat2->m[3][3];
}

inline void MatMul_InPlace(const LTMatrix *pMat1, LTMatrix *pMat2) 
{
    LTMatrix temp;
    MatMul(&temp, pMat1, pMat2);
    *pMat2 = temp;
}

inline void MatVMul(LTVector *pDest, const LTMatrix *pMat, const LTVector *pSrc) 
{
    pDest->x = pMat->m[0][0]*pSrc->x + pMat->m[0][1]*pSrc->y + pMat->m[0][2]*pSrc->z + pMat->m[0][3];
    pDest->y = pMat->m[1][0]*pSrc->x + pMat->m[1][1]*pSrc->y + pMat->m[1][2]*pSrc->z + pMat->m[1][3];
    pDest->z = pMat->m[2][0]*pSrc->x + pMat->m[2][1]*pSrc->y + pMat->m[2][2]*pSrc->z + pMat->m[2][3];
}

inline void MatVMul(float *pDest, const LTMatrix *pMat, const LTVector *pSrc) {
    pDest[0] = pMat->m[0][0]*pSrc->x + pMat->m[0][1]*pSrc->y + pMat->m[0][2]*pSrc->z + pMat->m[0][3];
    pDest[1] = pMat->m[1][0]*pSrc->x + pMat->m[1][1]*pSrc->y + pMat->m[1][2]*pSrc->z + pMat->m[1][3];
    pDest[2] = pMat->m[2][0]*pSrc->x + pMat->m[2][1]*pSrc->y + pMat->m[2][2]*pSrc->z + pMat->m[2][3];
}

inline void MatVMul_InPlace(const LTMatrix *pMat, LTVector *pSrc) 
{
    LTVector temp;
    MatVMul(&temp, pMat, pSrc);
    *pSrc = temp;
}

inline float MatVMul_H(LTVector *pDest, const LTMatrix *pMat, const LTVector *pSrc) 
{
    float one_over_w = 1.0f / (pMat->m[3][0]*pSrc->x + pMat->m[3][1]*pSrc->y + pMat->m[3][2]*pSrc->z + pMat->m[3][3]);

    pDest->x = one_over_w * (pMat->m[0][0]*pSrc->x + pMat->m[0][1]*pSrc->y + pMat->m[0][2]*pSrc->z + pMat->m[0][3]);
    pDest->y = one_over_w * (pMat->m[1][0]*pSrc->x + pMat->m[1][1]*pSrc->y + pMat->m[1][2]*pSrc->z + pMat->m[1][3]);
    pDest->z = one_over_w * (pMat->m[2][0]*pSrc->x + pMat->m[2][1]*pSrc->y + pMat->m[2][2]*pSrc->z + pMat->m[2][3]);

    return one_over_w;
}

inline float MatVMul_InPlace_H(const LTMatrix *pMat, LTVector *pSrc) 
{
    float one_over_w = 1.0f / (pMat->m[3][0]*pSrc->x + pMat->m[3][1]*pSrc->y + pMat->m[3][2]*pSrc->z + pMat->m[3][3]);
    LTVector temp;

    temp.x = one_over_w * (pMat->m[0][0]*pSrc->x + pMat->m[0][1]*pSrc->y + pMat->m[0][2]*pSrc->z + pMat->m[0][3]);
    temp.y = one_over_w * (pMat->m[1][0]*pSrc->x + pMat->m[1][1]*pSrc->y + pMat->m[1][2]*pSrc->z + pMat->m[1][3]);
    temp.z = one_over_w * (pMat->m[2][0]*pSrc->x + pMat->m[2][1]*pSrc->y + pMat->m[2][2]*pSrc->z + pMat->m[2][3]);

    *pSrc = temp;
    return one_over_w;
}

inline void MatVMul_Add(float *pOut, const LTMatrix *pMat, const float *pVec) 
{
    pOut[0] += pMat->m[0][0]*pVec[0] + pMat->m[0][1]*pVec[1] + pMat->m[0][2]*pVec[2] + pMat->m[0][3]*pVec[3];
    pOut[1] += pMat->m[1][0]*pVec[0] + pMat->m[1][1]*pVec[1] + pMat->m[1][2]*pVec[2] + pMat->m[1][3]*pVec[3];
    pOut[2] += pMat->m[2][0]*pVec[0] + pMat->m[2][1]*pVec[1] + pMat->m[2][2]*pVec[2] + pMat->m[2][3]*pVec[3];
    pOut[3] += pMat->m[3][0]*pVec[0] + pMat->m[3][1]*pVec[1] + pMat->m[3][2]*pVec[2] + pMat->m[3][3]*pVec[3];
}

inline void MatVMul_Add_3x3(float *pOut, const LTMatrix *pMat, const float *pVec) 
{
    pOut[0] += pMat->m[0][0]*pVec[0] + pMat->m[0][1]*pVec[1] + pMat->m[0][2]*pVec[2];
    pOut[1] += pMat->m[1][0]*pVec[0] + pMat->m[1][1]*pVec[1] + pMat->m[1][2]*pVec[2];
    pOut[2] += pMat->m[2][0]*pVec[0] + pMat->m[2][1]*pVec[1] + pMat->m[2][2]*pVec[2];
    pOut[3] += pMat->m[3][0]*pVec[0] + pMat->m[3][1]*pVec[1] + pMat->m[3][2]*pVec[2] + pMat->m[3][3]*pVec[3];
}

inline void MatVMul_3x3(LTVector *pDest, const LTMatrix *pMat, const LTVector *pSrc) 
{
    pDest->x = pMat->m[0][0]*pSrc->x + pMat->m[0][1]*pSrc->y + pMat->m[0][2]*pSrc->z;
    pDest->y = pMat->m[1][0]*pSrc->x + pMat->m[1][1]*pSrc->y + pMat->m[1][2]*pSrc->z;
    pDest->z = pMat->m[2][0]*pSrc->x + pMat->m[2][1]*pSrc->y + pMat->m[2][2]*pSrc->z;
}

inline void MatVMul_InPlace_3x3(const LTMatrix *pMat, LTVector *pSrc) 
{
    LTVector temp;
    MatVMul_3x3(&temp, pMat, pSrc);
    *pSrc = temp;
}

inline void MatVMul_Transposed3x3(LTVector *pDest, const LTMatrix *pMat, const LTVector *pSrc) 
{
    pDest->x = pMat->m[0][0]*pSrc->x + pMat->m[1][0]*pSrc->y + pMat->m[2][0]*pSrc->z;
    pDest->y = pMat->m[0][1]*pSrc->x + pMat->m[1][1]*pSrc->y + pMat->m[2][1]*pSrc->z;
    pDest->z = pMat->m[0][2]*pSrc->x + pMat->m[1][2]*pSrc->y + pMat->m[2][2]*pSrc->z;
}

inline void MatVMul_InPlace_Transposed3x3(const LTMatrix *pMat, LTVector *pSrc) 
{
    LTVector temp;

    temp.x = pMat->m[0][0]*pSrc->x + pMat->m[1][0]*pSrc->y + pMat->m[2][0]*pSrc->z;
    temp.y = pMat->m[0][1]*pSrc->x + pMat->m[1][1]*pSrc->y + pMat->m[2][1]*pSrc->z;
    temp.z = pMat->m[0][2]*pSrc->x + pMat->m[1][2]*pSrc->y + pMat->m[2][2]*pSrc->z;

    *pSrc = temp;
}

inline void Mat_InverseTransformation(const LTMatrix *pSrc, LTMatrix *pDest) 
{
    LTVector trans, newTrans;

    trans.x = pSrc->m[0][3];
    trans.y = pSrc->m[1][3];
    trans.z = pSrc->m[2][3];

    //Transpose the upper 3x3.
    pDest->m[0][0] = pSrc->m[0][0];  pDest->m[0][1] = pSrc->m[1][0]; pDest->m[0][2] = pSrc->m[2][0];
    pDest->m[1][0] = pSrc->m[0][1];  pDest->m[1][1] = pSrc->m[1][1]; pDest->m[1][2] = pSrc->m[2][1];
    pDest->m[2][0] = pSrc->m[0][2];  pDest->m[2][1] = pSrc->m[1][2]; pDest->m[2][2] = pSrc->m[2][2];

 
    MatVMul_3x3(&newTrans, pDest, &trans);
    pDest->m[0][3] = -newTrans.x;
    pDest->m[1][3] = -newTrans.y;
    pDest->m[2][3] = -newTrans.z;

    //Fill in the bottom row.
    pDest->m[3][0] = pDest->m[3][1] = pDest->m[3][2] = 0.0f;
    pDest->m[3][3] = 1.0f;
}

inline void Mat_SetupRot(LTMatrix *pMat, const LTVector *pVec, float amount) 
{
    float s, c, t;
    float tx, ty, tz;
    float sx, sy, sz;
    
    s = (float)sin(amount);
    c = (float)cos(amount);
    t = 1.0f - c;

    tx = t * pVec->x;   ty = t * pVec->y;   tz = t * pVec->z;
    sx = s * pVec->x;   sy = s * pVec->y;   sz = s * pVec->z;

    pMat->m[0][0] = tx*pVec->x + c;
    pMat->m[1][0]  = tx*pVec->y + sz;
    pMat->m[2][0] = tx*pVec->z - sy;

    pMat->m[0][1] = tx*pVec->y - sz;
    pMat->m[1][1] = ty*pVec->y + c;
    pMat->m[2][1] = ty*pVec->z + sx;

    pMat->m[0][2] = tx*pVec->z + sy;
    pMat->m[1][2] = ty*pVec->z - sx;
    pMat->m[2][2] = tz*pVec->z + c;


    //Make the unfulled parts identity.
    pMat->m[3][0] = 0.0f;   pMat->m[3][1] = 0.0f;   pMat->m[3][2] = 0.0f;
    pMat->m[0][3] = 0.0f;   pMat->m[1][3] = 0.0f;   pMat->m[2][3] = 0.0f;
    pMat->m[3][3] = 1.0f;   
}

inline void Mat_GetBasisVectors(const LTMatrix *pMat, LTVector *pRight, LTVector *pUp, LTVector *pForward) 
{
    pMat->GetBasisVectors(pRight, pUp, pForward);
}

inline void Mat_SetBasisVectors(LTMatrix *pMat, const LTVector *pRight, const LTVector *pUp, const LTVector *pForward) 
{
    pMat->SetBasisVectors(pRight, pUp, pForward);
}

inline bool LTMatrix::Inverse() 
{
    int iRow, i, j, iTemp, iTest;
    float mul, fTest, fLargest;
    float mat[4][8];
    int rowMap[4], iLargest;
    float *pIn, *pOut, *pRow, *pScaleRow;

    //How it's done.
    //AX = I
    //A = this
    //X = the matrix we're looking for
    //I = identity

    //Setup AI
    for (i=0; i < 4; i++) {
        pIn = m[i];
        pOut = mat[i];

        for (j=0; j < 4; j++)
        {
            pOut[j] = pIn[j];
        }

        pOut[4] = 0.0f;
        pOut[5] = 0.0f;
        pOut[6] = 0.0f;
        pOut[7] = 0.0f;
        pOut[i+4] = 1.0f;

        rowMap[i] = i;
    }


    //Use row operations to get to reduced row-echelon form using these rules:
    //1. Multiply or divide a row by a nonzero number.
    //2. Add a multiple of one row to another.
    //3. Interchange two rows.
    for (iRow=0; iRow < 4; iRow++) {

        //Find the row with the largest element in this column.
        fLargest = 0.001f;
        iLargest = -1;
        for (iTest=iRow; iTest < 4; iTest++)
        {
            fTest = (float)fabs(mat[rowMap[iTest]][iRow]);
            if (fTest > fLargest)
            {
                iLargest = iTest;
                fLargest = fTest;
            }
        }

        //They're all too small.. sorry.
        if (iLargest == -1)
        {
            return false;
        }

        //Swap the rows.
        iTemp = rowMap[iLargest];
        rowMap[iLargest] = rowMap[iRow];
        rowMap[iRow] = iTemp;

        pRow = mat[rowMap[iRow]];

        //Divide this row by the element.
        mul = 1.0f / pRow[iRow];
        for (j=0; j < 8; j++)
            pRow[j] *= mul;

        pRow[iRow] = 1.0f; //! Preserve accuracy...
        
        //Eliminate this element from the other rows using operation 2.
        for (i=0; i < 4; i++)
        {
            if (i == iRow)
                continue;

            pScaleRow = mat[rowMap[i]];
        
            //Multiply this row by -(iRow*the element).
            mul = -pScaleRow[iRow];
            for (j=0; j < 8; j++)
            {
                pScaleRow[j] += pRow[j] * mul;
            }

            pScaleRow[iRow] = 0.0f; //! Preserve accuracy...
        }
    }

    //The inverse is on the right side of AX now (the identity is on the left).
    for (i=0; i < 4; i++)
    {
        pIn = mat[rowMap[i]] + 4;
        pOut = m[i];

        for (j=0; j < 4; j++)
        {
            pOut[j] = pIn[j];
        }
    }

    return true;
}

inline void LTMatrix::SetupReflectionMatrix(const LTVector& N, const LTVector& vOrigin)
{
    LTMatrix mReflect, mBack, mForward;

    /*
        How it works:
            N = normal
            V = vector
            R = reflected vector
        
            R = V - N(2NV)
        so...

            R.x = V.x - 2*N.x*N.x*V.x - 2*N.x*N.y*V.y - 2*N.x*N.z*V.z
        or...
            R.x = V.x + (-2*N.x*N.x)*V.x + (-2*N.x*N.y)*V.y + (-2*N.x*N.z)*V.z
        and to get rid of the extra V.x on the left, add 1 to the multiplier like this:
            R.x = ((-2*N.x*N.x) + 1)*V.x + (-2*N.x*N.y)*V.y + (-2*N.x*N.z)*V.z

        Then just put those coefficients in the matrix.
    */

    mReflect.Init(
        -2.0f*N.x*N.x + 1.0f,   -2.0f*N.x*N.y,          -2.0f*N.x*N.z,          0.0f,
        -2.0f*N.y*N.x,          -2.0f*N.y*N.y + 1.0f,   -2.0f*N.y*N.z,          0.0f,
        -2.0f*N.z*N.x,          -2.0f*N.z*N.y,          -2.0f*N.z*N.z + 1.0f,   0.0f,
        0.0f,                   0.0f,                   0.0f,                   1.0f);

    mBack.Identity();
    mBack.SetTranslation(-vOrigin);

    mForward.Identity();
    mForward.SetTranslation(vOrigin);

    //(multiplied in reverse order, so it translates to the origin point,
    //reflects, and translates back).

    *this = mForward * mReflect * mBack;
}

inline void LTMatrix::SetupRot
(
    const LTVector&		u,
    float				a
)
{
 
	float s, c, t;
	float tx, ty, tz;
	float sx, sy, sz;
	
	
	s = ltsinf(a);
	c = ltcosf(a);
	t = 1.f - c;


    tx = t * u.x;   ty = t * u.y;   tz = t * u.z;
    sx = s * u.x;   sy = s * u.y;   sz = s * u.z;

    m[0][0] = tx*u.x + c;
    m[1][0] = tx*u.y + sz;
    m[2][0] = tx*u.z - sy;

    m[0][1] = tx*u.y - sz;
    m[1][1] = ty*u.y + c;
    m[2][1] = ty*u.z + sx;

    m[0][2] = tx*u.z + sy;
    m[1][2] = ty*u.z - sx;
    m[2][2] = tz*u.z + c;

    //Make the unfilled parts identity.
    m[3][0] = 0.0f;     m[3][1] = 0.0f;     m[3][2] = 0.0f;
    m[0][3] = 0.0f;     m[1][3] = 0.0f;     m[2][3] = 0.0f;
    m[3][3] = 1.0f;
}

inline LTMatrix LTMatrix::MakeInverseTransform() const
{
    LTMatrix mRet;

    Mat_InverseTransformation(this, &mRet);
    return mRet;
}

inline LTPlane  LTMatrix::TransformPlane(const LTPlane& P)
{
    LTVector startPt;
    LTPlane ret;

    startPt = P.m_Normal * P.m_Dist;
    MatVMul_3x3(&ret.m_Normal, this, &P.m_Normal);
    MatVMul_InPlace_H(this, &startPt);
    ret.m_Dist = ret.m_Normal.Dot(startPt);

    return ret;
}

inline LTVector LTMatrix::operator * (const LTVector& v) const
{
    LTVector vRet;
    MatVMul_H(&vRet, this, &v);
    return vRet;
}

inline LTMatrix LTMatrix::operator * (const LTMatrix& M) const
{
    LTMatrix mRet;

    MatMul(&mRet, this, &M);
    return mRet;
}
#endif//doxygen


#endif
//EOF
