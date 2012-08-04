


/*!
 Implementation of quaternion routines.

*/

#include "ltquatbase.h"
#include "ltsysoptim.h"

static int g_QNext[3] = { QY, QZ, QX };



void quat_ConvertToMatrix(const float *pQuat, float mat[4][4])
{
	float s, xs, ys, zs, wx, wy, wz, xx, xy, xz, yy, yz, zz;



/*!
	 get the values for matrix calcuation.
*/
	s = 2.0f / ((pQuat[0] * pQuat[0]) + (pQuat[1] * pQuat[1]) + 
		(pQuat[2] * pQuat[2]) + (pQuat[3] * pQuat[3]));

	xs = pQuat[0] * s;
	ys = pQuat[1] * s;
	zs = pQuat[2] * s;

	wx = pQuat[3] * xs;
	wy = pQuat[3] * ys;
	wz = pQuat[3] * zs;

	xx = pQuat[0] * xs;
	xy = pQuat[0] * ys;
	xz = pQuat[0] * zs;

	yy = pQuat[1] * ys;
	yz = pQuat[1] * zs;

	zz = pQuat[2] * zs;



/*!
	 Fill in matrix

*/
	mat[0][0] = 1.0f - (yy + zz);
	mat[0][1] = xy - wz;
	mat[0][2] = xz + wy;
	
	mat[1][0] = xy + wz;
	mat[1][1] = 1.0f - (xx + zz);
	mat[1][2] = yz - wx;

	mat[2][0] = xz - wy;
	mat[2][1] = yz + wx;
	mat[2][2] = 1.0f - (xx + yy);

	mat[0][3] = mat[1][3] = mat[2][3] = mat[3][0] = mat[3][1] = mat[3][2] = 0.0f;
	mat[3][3] = 1.0f;
}

void quat_ConvertFromMatrix(float *pQuat, const float mat[4][4])
{
	float diag, s;
	int i, j, k;

	
	diag = mat[0][0] + mat[1][1] + mat[2][2];

	if(diag < -0.999f )
	{
		i = QX;
		if( mat[QY][QY] > mat[QX][QX] )
			i = QY;
		if( mat[QZ][QZ] > mat[i][i] )
			i = QZ;
		
		j = g_QNext[i];
		k = g_QNext[j];

		s = ltsqrtf( mat[i][i] - ( mat[j][j] + mat[k][k] ) + /*mat[3][3]*/ 1.0f );

		pQuat[i] = s * 0.5f;
		s = 0.5f / s;
		pQuat[QW] = ( mat[k][j] - mat[j][k] ) * s;
		pQuat[j] = ( mat[j][i] + mat[i][j] ) * s;
		pQuat[k] = ( mat[k][i] + mat[i][k] ) * s;
		return;
	}

	s = ltsqrtf( diag + /*mat[3][3]*/ 1.0f );
	
	pQuat[3] = s * 0.5f;
	s = 0.5f / s;

	pQuat[0] = (mat[2][1] - mat[1][2]) * s;
	pQuat[1] = (mat[0][2] - mat[2][0]) * s;
	pQuat[2] = (mat[1][0] - mat[0][1]) * s;
}


void quat_GetVectors(const float *pQuat, float *pRight, float *pUp, float *pForward)
{
	float mat[4][4];

	quat_ConvertToMatrix(pQuat, mat);
	
	pRight[0] = mat[0][0];
	pRight[1] = mat[1][0];
	pRight[2] = mat[2][0];

	pUp[0] = mat[0][1];
	pUp[1] = mat[1][1];
	pUp[2] = mat[2][1];

	pForward[0] = mat[0][2];
	pForward[1] = mat[1][2];
	pForward[2] = mat[2][2];
}


void quat_Slerp(float *pDest, const float *pQ1, const float *pQ2, float t)
{
	float	rot1q[4];
	float	omega, cosom, oosinom;
	float	scalerot0, scalerot1;

/*!
	 Calculate the cosine

*/
	cosom = pQ1[0]*pQ2[0] + pQ1[1]*pQ2[1] + pQ1[2]*pQ2[2] + pQ1[3]*pQ2[3];

/*!
	 adjust signs if necessary

*/
	if(cosom < 0.0f)
	{
		cosom = -cosom;
		rot1q[0] = -pQ2[0];
		rot1q[1] = -pQ2[1];
		rot1q[2] = -pQ2[2];
		rot1q[3] = -pQ2[3];
	}
	else  
	{
		rot1q[0] = pQ2[0];
		rot1q[1] = pQ2[1];
		rot1q[2] = pQ2[2];
		rot1q[3] = pQ2[3];
	}

/*!
	 calculate interpolating coeffs

*/
	if ( (1.0f - cosom) > 0.0001f ) 
	{ 
/*!
		 standard case

*/
		omega   = ltacosf(cosom);
		oosinom = 1.0f / ltsinf(omega);
		scalerot0 = ltsinf((1.f - t) * omega) * oosinom;
		scalerot1 = ltsinf(t * omega) * oosinom;
	}
	else
	{ 
/*!
		 rot0 and rot1 very close - just do linear interp.

*/
		scalerot0 = 1.0f - t;
		scalerot1 = t;
	}

	//! build the new quarternion
	pDest[0] = (scalerot0 * pQ1[0] + scalerot1 * rot1q[0]);
	pDest[1] = (scalerot0 * pQ1[1] + scalerot1 * rot1q[1]);
	pDest[2] = (scalerot0 * pQ1[2] + scalerot1 * rot1q[2]);
	pDest[3] = (scalerot0 * pQ1[3] + scalerot1 * rot1q[3]);
}



