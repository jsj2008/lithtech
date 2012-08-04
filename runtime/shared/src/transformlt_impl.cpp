#include "bdefs.h"

#include "ilttransform.h"
#include "impl_common.h"

//
//Our implementation class for ILTTransform
//
class CLTTransform : public ILTTransform {
public:
    declare_interface(CLTTransform);

    //the ILTTransform functions.
    LTRESULT Get(LTransform& T, LTVector &p, LTRotation &q);
    LTRESULT Set(LTransform &T, LTVector &p, LTRotation &q);
    LTRESULT GetPos(LTransform &T, LTVector &p);
    LTRESULT GetRot(LTransform &T, LTRotation &q);
    LTRESULT ToMatrix(LTransform &T, LTMatrix &M);
    LTRESULT FromMatrix(LTransform &T, LTMatrix &M);
    LTRESULT Inverse(LTransform &T_inv, LTransform &T);
    LTRESULT Multiply(LTransform &Tab, LTransform &Ta, LTransform &Tb);
    LTRESULT Difference(LTransform &diff, LTransform &t1, LTransform &t2);
};

//instantiate our implementation class
define_interface(CLTTransform, ILTTransform);


// ---------------------------------------------------------------------- //
// Helpers.
// ---------------------------------------------------------------------- //
inline void _TransformToMatrix(LTransform &transform, LTMatrix &mat)
{
	quat_ConvertToMatrix((float*)&transform.m_Rot, mat.m);
	Mat_SetTranslation(mat, transform.m_Pos);
	mat.Scale(transform.m_Scale.x, transform.m_Scale.y, transform.m_Scale.z);
}

inline void _TransformFromMatrix(LTransform &transform, LTMatrix &mat)
{
	transform.m_Scale = mat.GetScale();
	// remove scale before getting rotation.
	mat.Normalize();
	ic_GetTransform(mat, transform.m_Pos, transform.m_Rot);
	// put scale back... 
	mat.Scale(transform.m_Scale.x, transform.m_Scale.y, transform.m_Scale.z);
}


LTRESULT CLTTransform::Get(LTransform &transform, LTVector &pos, LTRotation &rot)
{
	pos = transform.m_Pos;
	rot = transform.m_Rot;
	return LT_OK;
}

LTRESULT CLTTransform::Set(LTransform &transform, LTVector &pos, LTRotation &rot)
{
	transform.m_Pos = pos;
	transform.m_Rot = rot;
	return LT_OK;
}

LTRESULT CLTTransform::GetPos(LTransform &transform, LTVector &pos)
{
	pos = transform.m_Pos;
	return LT_OK;
}

LTRESULT CLTTransform::GetRot(LTransform &transform, LTRotation &rot)
{
	rot = transform.m_Rot;
	return LT_OK;
}

LTRESULT	CLTTransform::ToMatrix(LTransform &transform, LTMatrix &mat)
{
	_TransformToMatrix(transform, mat);
	return LT_OK;
}

LTRESULT	CLTTransform::FromMatrix(LTransform &transform, LTMatrix &mat)
{
	_TransformFromMatrix(transform, mat);
	return LT_OK;
}

LTRESULT CLTTransform::Inverse(LTransform &inverse, LTransform &transform)
{
	LTMatrix mTransform, mInverse;

	_TransformToMatrix(transform, mTransform);
	Mat_InverseTransformation(&mTransform, &mInverse);
	_TransformFromMatrix(inverse, mInverse);
	  

	// ( R2-1 * R1 ) , ( R2-1  * t1 ) - t2

	return LT_OK;		
}

LTRESULT CLTTransform::Multiply(LTransform &out, LTransform &t1, LTransform &t2)
{
	/* old (reference)
	LTMatrix m1, m2, mOut;

	_TransformToMatrix(t1, m1);
	_TransformToMatrix(t2, m2);
	MatMul(&mOut, &m1, &m2);
	_TransformFromMatrix(out, mOut);
	*/	
	
	// basic formula : R2 R1, R2 t1 + t2)
 	// (R', T', S' ) = (( R1 * R2 ) , ( R1 * P2 ) + P1 , S1 * S2
	out.m_Rot = t1.m_Rot * t2.m_Rot ;
	quat_RotVec( &out.m_Pos.x, t1.m_Rot.m_Quat, &t2.m_Pos.x);
	out.m_Pos += t1.m_Pos ;

	return LT_OK;
}

LTRESULT CLTTransform::Difference(LTransform &diff, LTransform &t1, LTransform &t2)
{
	LTMatrix m1, m2, mOut, mInverse;

	// t2 * diff = t1
	// diff = ~t2 * t1

	_TransformToMatrix(t1, m1);
	_TransformToMatrix(t2, m2);
	Mat_InverseTransformation(&m2, &mInverse);
	MatMul(&mOut, &mInverse, &m1);
	_TransformFromMatrix(diff, mOut);

	

	return LT_OK;
}
