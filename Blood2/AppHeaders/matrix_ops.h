
// Defines all the matrix operations.

#ifndef __MATRIX_OPS_H__
#define __MATRIX_OPS_H__


	// Matrices are indexed by (y,x), so a translation matrix looks like:
	// 1 0 0 tx  (tx is in m[0][3])
	// 0 1 0 ty  (ty is in m[1][3])
	// 0 0 1 tz  (tz is in m[2][3])
	// 0 0 0 1

	typedef struct
	{
		float m[4][4];
	} DMatrix;


	
	INLINE_FN void Mat_Identity(DMatrix *pMat)
	{
		MAT_SET(*pMat, 
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f);
	}


	INLINE_FN void Mat_GetBasisVectors(DMatrix *pMat, DVector *pRight, DVector *pUp, DVector *pForward)
	{
		pRight->x = pMat->m[0][0];
		pRight->y = pMat->m[1][0];
		pRight->z = pMat->m[2][0];

		pUp->x = pMat->m[0][1];
		pUp->y = pMat->m[1][1];
		pUp->z = pMat->m[2][1];

		pForward->x = pMat->m[0][2];
		pForward->y = pMat->m[1][2];
		pForward->z = pMat->m[2][2];
	}


	INLINE_FN void Mat_SetBasisVectors(DMatrix *pMat, DVector *pRight, DVector *pUp, DVector *pForward)
	{
		MAT_SET(*pMat,
			pRight->x, pUp->x, pForward->x, 0.0f,
			pRight->y, pUp->y, pForward->y, 0.0f,
			pRight->z, pUp->z, pForward->z, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f);
	}


	// Transpose the upper left 3x3.
	INLINE_FN void MatTranspose3x3(DMatrix *pMat)
	{
		float temp;
		float *d = (float*)pMat->m;

		temp = d[1]; d[1] = d[4]; d[4] = temp;
		temp = d[2]; d[2] = d[8]; d[8] = temp;
		temp = d[9]; d[9] = d[6]; d[6] = temp;
	}


	// Mat3 = Mat1 * Mat2
	#define M1 pMat1->m
	#define M2 pMat2->m

	INLINE_FN void MatMul(DMatrix *pDest, DMatrix *pMat1, DMatrix *pMat2)
	{
		pDest->m[0][0] = M1[0][0]*M2[0][0] + M1[0][1]*M2[1][0] + M1[0][2]*M2[2][0] + M1[0][3]*M2[3][0];
		pDest->m[1][0] = M1[1][0]*M2[0][0] + M1[1][1]*M2[1][0] + M1[1][2]*M2[2][0] + M1[1][3]*M2[3][0];
		pDest->m[2][0] = M1[2][0]*M2[0][0] + M1[2][1]*M2[1][0] + M1[2][2]*M2[2][0] + M1[2][3]*M2[3][0];
		pDest->m[3][0] = M1[3][0]*M2[0][0] + M1[3][1]*M2[1][0] + M1[3][2]*M2[2][0] + M1[3][3]*M2[3][0];

		pDest->m[0][1] = M1[0][0]*M2[0][1] + M1[0][1]*M2[1][1] + M1[0][2]*M2[2][1] + M1[0][3]*M2[3][1];
		pDest->m[1][1] = M1[1][0]*M2[0][1] + M1[1][1]*M2[1][1] + M1[1][2]*M2[2][1] + M1[1][3]*M2[3][1];
		pDest->m[2][1] = M1[2][0]*M2[0][1] + M1[2][1]*M2[1][1] + M1[2][2]*M2[2][1] + M1[2][3]*M2[3][1];
		pDest->m[3][1] = M1[3][0]*M2[0][1] + M1[3][1]*M2[1][1] + M1[3][2]*M2[2][1] + M1[3][3]*M2[3][1];

		pDest->m[0][2] = M1[0][0]*M2[0][2] + M1[0][1]*M2[1][2] + M1[0][2]*M2[2][2] + M1[0][3]*M2[3][2];
		pDest->m[1][2] = M1[1][0]*M2[0][2] + M1[1][1]*M2[1][2] + M1[1][2]*M2[2][2] + M1[1][3]*M2[3][2];
		pDest->m[2][2] = M1[2][0]*M2[0][2] + M1[2][1]*M2[1][2] + M1[2][2]*M2[2][2] + M1[2][3]*M2[3][2];
		pDest->m[3][2] = M1[3][0]*M2[0][2] + M1[3][1]*M2[1][2] + M1[3][2]*M2[2][2] + M1[3][3]*M2[3][2];

		pDest->m[0][3] = M1[0][0]*M2[0][3] + M1[0][1]*M2[1][3] + M1[0][2]*M2[2][3] + M1[0][3]*M2[3][3];
		pDest->m[1][3] = M1[1][0]*M2[0][3] + M1[1][1]*M2[1][3] + M1[1][2]*M2[2][3] + M1[1][3]*M2[3][3];
		pDest->m[2][3] = M1[2][0]*M2[0][3] + M1[2][1]*M2[1][3] + M1[2][2]*M2[2][3] + M1[2][3]*M2[3][3];
		pDest->m[3][3] = M1[3][0]*M2[0][3] + M1[3][1]*M2[1][3] + M1[3][2]*M2[2][3] + M1[3][3]*M2[3][3];
	}


	// Mat2 = Mat1 * Mat2
	INLINE_FN void MatMul_InPlace(DMatrix *pMat1, DMatrix *pMat2)
	{
		DMatrix temp;
		MatMul(&temp, pMat1, pMat2);
		MAT_COPY(*pMat2, temp);
	}


	// Non-homogenous matrix multiply.
	INLINE_FN void MatVMul(DVector *pDest, DMatrix *pMat, DVector *pSrc)
	{
		pDest->x = pMat->m[0][0]*pSrc->x + pMat->m[0][1]*pSrc->y + pMat->m[0][2]*pSrc->z + pMat->m[0][3];
		pDest->y = pMat->m[1][0]*pSrc->x + pMat->m[1][1]*pSrc->y + pMat->m[1][2]*pSrc->z + pMat->m[1][3];
		pDest->z = pMat->m[2][0]*pSrc->x + pMat->m[2][1]*pSrc->y + pMat->m[2][2]*pSrc->z + pMat->m[2][3];
	}

	INLINE_FN void MatVMul_InPlace(DMatrix *pMat, DVector *pSrc)
	{
		DVector temp;
		MatVMul(&temp, pMat, pSrc);
		VEC_COPY(*pSrc, temp);
	}


	// Matrix multiply with the homogenous divide.
	INLINE_FN float MatVMul_H(DVector *pDest, DMatrix *pMat, DVector *pSrc)
	{
		float one_over_w = 1.0f / (pMat->m[3][0]*pSrc->x + pMat->m[3][1]*pSrc->y + pMat->m[3][2]*pSrc->z + pMat->m[3][3]);

		pDest->x = one_over_w * (pMat->m[0][0]*pSrc->x + pMat->m[0][1]*pSrc->y + pMat->m[0][2]*pSrc->z + pMat->m[0][3]);
		pDest->y = one_over_w * (pMat->m[1][0]*pSrc->x + pMat->m[1][1]*pSrc->y + pMat->m[1][2]*pSrc->z + pMat->m[1][3]);
		pDest->z = one_over_w * (pMat->m[2][0]*pSrc->x + pMat->m[2][1]*pSrc->y + pMat->m[2][2]*pSrc->z + pMat->m[2][3]);
	
		return one_over_w;
	}

	INLINE_FN float MatVMul_InPlace_H(DMatrix *pMat, DVector *pSrc)
	{
		float one_over_w = 1.0f / (pMat->m[3][0]*pSrc->x + pMat->m[3][1]*pSrc->y + pMat->m[3][2]*pSrc->z + pMat->m[3][3]);
		DVector temp;

		temp.x = one_over_w * (pMat->m[0][0]*pSrc->x + pMat->m[0][1]*pSrc->y + pMat->m[0][2]*pSrc->z + pMat->m[0][3]);
		temp.y = one_over_w * (pMat->m[1][0]*pSrc->x + pMat->m[1][1]*pSrc->y + pMat->m[1][2]*pSrc->z + pMat->m[1][3]);
		temp.z = one_over_w * (pMat->m[2][0]*pSrc->x + pMat->m[2][1]*pSrc->y + pMat->m[2][2]*pSrc->z + pMat->m[2][3]);
	
		VEC_COPY(*pSrc, temp);
		return one_over_w;
	}


	// Matrix multiply using upper left 3x3.
	INLINE_FN void MatVMul_3x3(DVector *pDest, DMatrix *pMat, DVector *pSrc)
	{
		pDest->x = pMat->m[0][0]*pSrc->x + pMat->m[0][1]*pSrc->y + pMat->m[0][2]*pSrc->z;
		pDest->y = pMat->m[1][0]*pSrc->x + pMat->m[1][1]*pSrc->y + pMat->m[1][2]*pSrc->z;
		pDest->z = pMat->m[2][0]*pSrc->x + pMat->m[2][1]*pSrc->y + pMat->m[2][2]*pSrc->z;
	}

	INLINE_FN void MatVMul_InPlace_3x3(DMatrix *pMat, DVector *pSrc)
	{
		DVector temp;
		MatVMul_3x3(&temp, pMat, pSrc);
		VEC_COPY(*pSrc, temp);
	}

	
	// Transposed matrix multiply (using upper left 3x3).
	INLINE_FN void MatVMul_Transposed3x3(DVector *pDest, DMatrix *pMat, DVector *pSrc)
	{
		pDest->x = pMat->m[0][0]*pSrc->x + pMat->m[1][0]*pSrc->y + pMat->m[2][0]*pSrc->z;
		pDest->y = pMat->m[0][1]*pSrc->x + pMat->m[1][1]*pSrc->y + pMat->m[2][1]*pSrc->z;
		pDest->z = pMat->m[0][2]*pSrc->x + pMat->m[1][2]*pSrc->y + pMat->m[2][2]*pSrc->z;
	}

	INLINE_FN void MatVMul_InPlace_Transposed3x3(DMatrix *pMat, DVector *pSrc)
	{
		DVector temp;

		temp.x = pMat->m[0][0]*pSrc->x + pMat->m[1][0]*pSrc->y + pMat->m[2][0]*pSrc->z;
		temp.y = pMat->m[0][1]*pSrc->x + pMat->m[1][1]*pSrc->y + pMat->m[2][1]*pSrc->z;
		temp.z = pMat->m[0][2]*pSrc->x + pMat->m[1][2]*pSrc->y + pMat->m[2][2]*pSrc->z;
	
		VEC_COPY(*pSrc, temp);
	}


	// Set the matrix up to rotate 'amount' radians around the given axis.
	INLINE_FN void Mat_SetupRot(DMatrix *pMat, DVector *pVec, float amount)
	{
		float s, c, t;
		float tx, ty, tz;
		float sx, sy, sz;
		
		s = (float)sin(amount);
		c = (float)cos(amount);
		t = 1.0f - c;

		tx = t * pVec->x;	ty = t * pVec->y;	tz = t * pVec->z;
		sx = s * pVec->x;	sy = s * pVec->y;	sz = s * pVec->z;

		pMat->m[0][0] = tx*pVec->x + c;
		pMat->m[1][0]  = tx*pVec->y + sz;
		pMat->m[2][0] = tx*pVec->z - sy;

		pMat->m[0][1] = tx*pVec->y - sz;
		pMat->m[1][1] = ty*pVec->y + c;
		pMat->m[2][1] = ty*pVec->z + sx;

		pMat->m[0][2] = tx*pVec->z + sy;
		pMat->m[1][2] = ty*pVec->z - sx;
		pMat->m[2][2] = tz*pVec->z + c;

		// Make the unfulled parts identity.
		pMat->m[3][0] = 0.0f; 	pMat->m[3][1] = 0.0f;	pMat->m[3][2] = 0.0f;
		pMat->m[0][3] = 0.0f;	pMat->m[1][3] = 0.0f;	pMat->m[2][3] = 0.0f;
		pMat->m[3][3] = 1.0f;	
	}


#endif


