#include "bdefs.h"

#include "iltmath.h"
#include "geomroutines.h"


class CLTMath : public ILTMath {
public:
    declare_interface(CLTMath);

    //ILTMath functions.
    LTRESULT AlignRotation(LTRotation &q, LTVector &vf, LTVector &vu);
    LTRESULT EulerRotateX(LTRotation &q, float angle);
    LTRESULT EulerRotateY(LTRotation &q, float angle);
    LTRESULT EulerRotateZ(LTRotation &q, float angle);
    LTRESULT GetRotationVectors(LTRotation &q, LTVector &R0, LTVector &R1, LTVector &R2);
    LTRESULT InterpolateRotation(LTRotation &qu, LTRotation &q0, LTRotation &q1, float u);
    LTRESULT SetupEuler(LTRotation &q, float tx, float ty, float tz);
    LTRESULT SetupRotationAroundPoint(LTMatrix &M, LTRotation &q, LTVector &pl);
    LTRESULT SetupRotationMatrix(LTMatrix &M, LTRotation &q);
    LTRESULT SetupTransformationMatrix(LTMatrix &M, LTVector &O, LTRotation &q);
    LTRESULT SetupTranslationFromMatrix(LTVector &t, LTMatrix &M);
    LTRESULT SetupTranslationMatrix(LTMatrix &M, LTVector &t);
    LTRESULT SetupRotationFromMatrix(LTRotation &q, LTMatrix &M);
    LTRESULT RotateAroundAxis(LTRotation &q, LTVector &u, float angle);
    LTRESULT GetRotationVectorsFromMatrix(LTMatrix &M, LTVector &R0, LTVector &R1, LTVector &R2);
};

//instantiate the class and put in into the interface mgr.
define_interface(CLTMath, ILTMath);

LTRESULT CLTMath::GetRotationVectors(LTRotation &rot, LTVector &right, LTVector &up, LTVector &forward)
{
    gr_GetRotationVectors(&rot, &right, &up, &forward);
    return LT_OK;
}

LTRESULT CLTMath::SetupEuler(LTRotation &rot, float pitch, float yaw, float roll)
{
    gr_EulerToRotation(pitch, yaw, roll, &rot);
    return LT_OK;
}

LTRESULT CLTMath::InterpolateRotation(LTRotation &rDest, LTRotation &rot1, LTRotation &rot2, float t)
{
    gr_InterpolateRotation(&rDest, &rot1, &rot2, t);
    return LT_OK;
}

LTRESULT CLTMath::SetupTransformationMatrix(LTMatrix &mMat, LTVector &vTranslation, LTRotation &rRot)
{
    gr_SetupTransformation(&vTranslation, &rRot, LTNULL, &mMat);
    return LT_OK;
}

LTRESULT CLTMath::SetupTranslationMatrix(LTMatrix &mMat, LTVector &vTranslation)
{
    gr_SetupTransformation(&vTranslation, LTNULL, LTNULL, &mMat);
    return LT_OK;
}

LTRESULT CLTMath::SetupRotationMatrix(LTMatrix &mMat, LTRotation &rRot)
{
    gr_SetupTransformation(LTNULL, &rRot, LTNULL, &mMat);
    return LT_OK;
}

LTRESULT CLTMath::SetupTranslationFromMatrix(LTVector &vTranslation, LTMatrix &mMat)
{
    mMat.GetTranslation(vTranslation);
    return LT_OK;
}

LTRESULT CLTMath::SetupRotationFromMatrix(LTRotation &rRot, LTMatrix &mMat)
{
    MatrixToRotation(&mMat, &rRot);
    return LT_OK;
}

LTRESULT CLTMath::SetupRotationAroundPoint(LTMatrix &mMat, LTRotation &rRot, LTVector &vPoint)
{
    LTMatrix mForward, mRotate, mBackward;
    LTVector negativePoint;

    negativePoint = -vPoint;
    SetupTranslationMatrix(mForward, vPoint);
    SetupTranslationMatrix(mBackward, negativePoint);
    SetupRotationMatrix(mRotate, rRot);

    mMat = mForward * mRotate * mBackward;
    return LT_OK;
}

LTRESULT CLTMath::AlignRotation(LTRotation &rOutRot, LTVector &vVector, LTVector &vUp)
{
    LTMatrix theMat;
    LTVector ref[3];

    // Get a frame of reference.
    gr_BuildFrameOfReference(&vVector, &vUp, &ref[0], &ref[1], &ref[2]);

    theMat.SetBasisVectors(&ref[0], &ref[1], &ref[2]);
    MatrixToRotation(&theMat, &rOutRot);
    return LT_OK;
}

LTRESULT CLTMath::EulerRotateX(LTRotation &rRot, float amount)
{
    LTVector vecs[3];

    gr_GetRotationVectors(&rRot, &vecs[0], &vecs[1], &vecs[2]);
    return RotateAroundAxis(rRot, vecs[0], amount);
}

LTRESULT CLTMath::EulerRotateY(LTRotation &rRot, float amount)
{
    LTVector vecs[3];

    gr_GetRotationVectors(&rRot, &vecs[0], &vecs[1], &vecs[2]);
    return RotateAroundAxis(rRot, vecs[1], amount);
}

LTRESULT CLTMath::EulerRotateZ(LTRotation &rRot, float amount)
{
    LTVector vecs[3];

    gr_GetRotationVectors(&rRot, &vecs[0], &vecs[1], &vecs[2]);
    return RotateAroundAxis(rRot, vecs[2], amount);
}

LTRESULT CLTMath::RotateAroundAxis(LTRotation &rRot, LTVector &vAxis, float amount)
{
    LTMatrix rotation, mat;

    // Get the quaternion.
    RotationToMatrix(&rRot, &mat);

    // Setup a rotation matrix and apply it.
    gr_SetupRotationAroundVector(&rotation, vAxis, amount);
    mat = rotation * mat;
    
    MatrixToRotation(&mat, &rRot);
    return LT_OK;
}

LTRESULT CLTMath::GetRotationVectorsFromMatrix(LTMatrix &mMat, 
        LTVector &vRight, LTVector &vUp, LTVector &vForward)
{
    mMat.GetBasisVectors(&vRight, &vUp, &vForward);
    return LT_OK;
}






