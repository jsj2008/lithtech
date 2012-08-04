//------------------------------------------------------------------
//
//	FILE	  : GeomRoutines.cpp
//
//	PURPOSE	  : Implementation for non-inlined GeomRoutines functions.
//
//	CREATED	  : 1st May 1996
//
//	COPYRIGHT : Microsoft 1996 All Rights Reserved
//
//------------------------------------------------------------------

#include "bdefs.h"
#include "geomroutines.h"
#include "ltsysoptim.h"


void gr_GetPerpendicularVector(LTVector *pVec, LTVector *pRef, LTVector *pPerp)
{
	float dot, t;
	LTVector temp, tempRef;

	if(!pRef)
	{
		tempRef.Init(0, 1, 0);
		pRef = &tempRef;
	}

	*pPerp = *pRef;

	// Are pRef and pVec the same?  If not, we can exit.
	dot = pVec->Dot(*pPerp);
	if(dot > 0.99f || dot < -0.99f)
	{
		// Try to modify it as little as possible.
		pPerp->z += 5.0f;
		pPerp->Norm();
		dot = VEC_DOT(*pVec, *pPerp);
		if(dot > 0.99f || dot < -0.99f)
		{
			pPerp->x += 5.0f;
			pPerp->y += 5.0f;
			pPerp->Norm();

			dot = pVec->Dot(*pPerp);
			if(dot > 0.99f || dot < -0.99f)
			{
				pPerp->x += 5.0f;
				pPerp->y -= 2.0f;
				pPerp->z -= 5.0f;
				pPerp->Norm();
			}
		}
	}
	
	// Make pVec and pPerp linear independent.
	t = -pVec->Dot(*pPerp);
	temp = *pVec * t;
	*pPerp += temp;
	pPerp->Norm();
}


void gr_BuildFrameOfReference(LTVector *pVec, LTVector *pUpRef, LTVector *pRight, LTVector *pUp, LTVector *pForward)
{
	LTVector tempRef;

	*pForward = *pVec;
	pForward->Norm();

	// Treat the vector as the forward vector and come up with 2 other vectors.
	if(pUpRef)
	{
		tempRef = *pUpRef;
		tempRef.Norm();
		gr_GetPerpendicularVector(pForward, &tempRef, pUp);
	}
	else
	{
		gr_GetPerpendicularVector(pForward, LTNULL, pUp);
	}

	// Create the right vector.	
	*pRight = pForward->Cross(*pUp);
}


void gr_SetupMatrixEuler(const LTVector vAngles, float mat[4][4])
{
	/*	
	CMatrix		yawMat( yaw_cos,	0.0f,	yaw_sin,	0.0f,
						0.0f,		1.0f,	0.0f,		0.0f,
						-yaw_sin,	0.0f,	yaw_cos,	0.0f,
						0.0f,		0.0f,	0.0f,		1.0f );

	CMatrix		pitchMat(	1.0f,	0.0f,		0.0f,		0.0f,
							0.0f,	pitch_cos,	-pitch_sin,	0.0f,
							0.0f,	pitch_sin,	pitch_cos,	0.0f,
							0.0f,	0.0f,		0.0f,		1.0f );

	CMatrix		rollMat(	roll_cos,	-roll_sin,	0.0f,	0.0f,
							roll_sin,	roll_cos,	0.0f,	0.0f,
							0.0f,		0.0f,		1.0f,	0.0f,
							0.0f,		0.0f,		0.0f,	1.0f );

	CMatrix fullMat = yawMat * pitchMat * rollMat;
	*/	


	float yc = ltcosf(vAngles.y), ys = ltsinf(vAngles.y);
	float pc = ltcosf(vAngles.x), ps = ltsinf(vAngles.x);
	float rc = ltcosf(vAngles.z), rs = ltsinf(vAngles.z);

	mat[0][0] = rc*yc + rs*ps*ys;
	mat[0][1] = -rs*yc + rc*ps*ys;
	mat[0][2] = pc*ys;
	mat[0][3] = 0.0f;
	
	mat[1][0] = rs*pc;
	mat[1][1] = rc*pc;
	mat[1][2] = -ps;
	mat[1][3] = 0.0f;

	mat[2][0] = -rc*ys + rs*ps*yc;
	mat[2][1] = rs*ys + rc*ps*yc;
	mat[2][2] = pc*yc;
	mat[2][3] = 0.0f;

	mat[3][0] = mat[3][1] = mat[3][2] = 0.0f;
	mat[3][3] = 1.0f;
}


void gr_SetupMatrixEuler(float mat[4][4], float pitch, float yaw, float roll)
{
	gr_SetupMatrixEuler(LTVector(pitch, yaw, roll), mat);
}


void RotationToMatrix(LTRotation *pRot, LTMatrix *pMatrix)
{
	quat_ConvertToMatrix((float*)pRot, pMatrix->m);
}


void MatrixToRotation(LTMatrix *pMatrix, LTRotation *pRot)
{
	quat_ConvertFromMatrix((float*)pRot, pMatrix->m);
}


void gr_GetRotationVectors(LTRotation *pRot, LTVector *pRight, LTVector *pUp, LTVector *pForward)
{
	quat_GetVectors((float*)pRot, (float*)pRight, (float*)pUp, (float*)pForward);
}


void gr_InterpolateRotation(LTRotation *pDest, LTRotation *pRot1, LTRotation *pRot2, float t)
{
	quat_Slerp((float*)pDest, (float*)pRot1, (float*)pRot2, t);
}


void gr_EulerToRotation(const LTVector vAngles, LTRotation *pRot)
{
	LTMatrix mat;
	
	gr_SetupMatrixEuler(mat.m, VEC_EXPAND(vAngles));
	MatrixToRotation(&mat, pRot);
}


void gr_EulerToRotation(float pitch, float yaw, float roll, LTRotation *pRot)
{
	gr_EulerToRotation(LTVector(pitch, yaw, roll), pRot);
}


void gr_GetEulerVectors(
	const LTVector vAngles,
	LTVector &vRight,
	LTVector &vUp,
	LTVector &vForward)
{
	LTMatrix mTemp;

	gr_SetupMatrixEuler(vAngles, mTemp.m);
	mTemp.GetBasisVectors(&vRight, &vUp, &vForward);
}


void gr_SetupTransformation(LTVector *pPos, LTRotation *pQuat, LTVector *pScale, LTMatrix *pMat)
{
	if( pQuat )
	{
		quat_ConvertToMatrix((float*)pQuat, pMat->m);
	}
	else	
	{
		Mat_Identity( pMat );
	}

	if( pScale )
	{
		pMat->m[0][0] *= pScale->x;
		pMat->m[1][0] *= pScale->x;
		pMat->m[2][0] *= pScale->x;

		pMat->m[0][1] *= pScale->y;
		pMat->m[1][1] *= pScale->y;
		pMat->m[2][1] *= pScale->y;

		pMat->m[0][2] *= pScale->z;
		pMat->m[1][2] *= pScale->z;
		pMat->m[2][2] *= pScale->z;
	}

	if( pPos )
	{
		pMat->m[0][3] = pPos->x;
		pMat->m[1][3] = pPos->y;
		pMat->m[2][3] = pPos->z;
	}
}


void gr_SetupRotationAroundVector(LTMatrix *pMat, LTVector v, float angle)
{
	pMat->SetupRot(v, angle);
}


void gr_SetupWMTransform(
	LTVector *pWorldTranslation,
	const LTVector *pPos,			// Position and rotation.
	LTRotation *pRot,
	LTMatrix *pOutForward,	// Output forward and backwards transforms.
	LTMatrix *pOutBack)
{
	LTVector pos;
	LTMatrix mBack, mForward, mRotation;


	pos = *pPos - *pWorldTranslation;

	// Setup the matrices we'll be using.
	mBack.Init(
		1.0f, 0.0f, 0.0f, -pWorldTranslation->x,
		0.0f, 1.0f, 0.0f, -pWorldTranslation->y,
		0.0f, 0.0f, 1.0f, -pWorldTranslation->z,
		0.0f, 0.0f, 0.0f, 1);

	mForward.Init(
		1.0f, 0.0f, 0.0f, pWorldTranslation->x + pos.x,
		0.0f, 1.0f, 0.0f, pWorldTranslation->y + pos.y,
		0.0f, 0.0f, 1.0f, pWorldTranslation->z + pos.z,
		0.0f, 0.0f, 0.0f, 1);

	RotationToMatrix(pRot, &mRotation);

	// Transform order = tBack, Rotate, tForward
	// So multiply matrices as tForward*Rotate*tBack
	*pOutForward = mForward * mRotation * mBack;
	*pOutBack = pOutForward->MakeInverseTransform();
}


LTBOOL gr_IntersectPlanes(
	LTPlane &plane0,
	LTPlane &plane1,
	LTPlane &plane2,
	LTVector &vOut)
{
	LTMatrix mPlanes;

	/*
		Math behind this:

		Plane equation is Ax + By + Cz - D = 0
		Standard matrix equation Ax = b.

		So stick the plane equations into the A matrix:

		A B C -D	(from plane 0)
		A B C -D	(from plane 1)
		A B C -D	(from plane 2)
		0 0 0  1

		then the b vector is:
		[0 0 0 1]

		and we're solving for the x vector so:
		~AAx = ~Ab
		x = ~Ab
	*/

	mPlanes.Init(
		plane0.m_Normal[0], plane0.m_Normal[1], plane0.m_Normal[2], -plane0.m_Dist,
		plane1.m_Normal[0], plane1.m_Normal[1], plane1.m_Normal[2], -plane1.m_Dist,
		plane2.m_Normal[0], plane2.m_Normal[1], plane2.m_Normal[2], -plane2.m_Dist,
		0.0f, 0.0f, 0.0f, 1.0f);

	// If we can't invert the matrix, then two or more planes are equal.
	if(!mPlanes.Inverse())
		return LTFALSE;

	// Since our b vector is all zeros, we don't need to do a full matrix multiply.
	// vOut = mPlaneNormal * vPlaneDist;
	vOut.Init(
		mPlanes.m[0][3],
		mPlanes.m[1][3],
		mPlanes.m[2][3]);

	vOut *= (1.0f / mPlanes.m[3][3]);
	return LTTRUE;
}


