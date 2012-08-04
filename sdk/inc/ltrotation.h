#ifndef _LTROTATION_H_
#define _LTROTATION_H_


#include "ltbasetypes.h"
#include "ltvector.h"
#include "ltquatbase.h"
#include "ltmatrix.h"
#include "physics/coordinate_frame.h"


#ifndef DOXYGEN_SHOULD_SKIP_THIS
//---------------------------------------------------------------------------//
/*!
Used for: Obsolete.
*/
struct LTRotation
{
	float m_Quat[4];//(x,y,z,r)

	LTRotation()
	{
		m_Quat[0] = 0;
		m_Quat[1] = 0;
		m_Quat[2] = 0;
		m_Quat[3] = 1;
	}

	LTRotation( const LTRotation &q )
	{
		m_Quat[0] = q.m_Quat[0];
		m_Quat[1] = q.m_Quat[1];
		m_Quat[2] = q.m_Quat[2];
		m_Quat[3] = q.m_Quat[3];
	}

	LTRotation( const LTOrientation& R )
	{
		m_Quat[0] = R.m_Q.v.x;
		m_Quat[1] = R.m_Q.v.y;
		m_Quat[2] = R.m_Q.v.z;
		m_Quat[3] = R.m_Q.r;
	}

	operator LTOrientation() const
	{
		//r,x,y,z
		return LTOrientation( LTQuaternionf(m_Quat[3], m_Quat[0], m_Quat[1], m_Quat[2]) );
	}

	// Note : vAxis must be unit-length
	LTRotation( const LTVector &vAxis, float fAngle )
	{
		fAngle *= 0.5f;
		float fSinAngle = sinf(fAngle);
		m_Quat[0] = vAxis.x * fSinAngle;
		m_Quat[1] = vAxis.y * fSinAngle;
		m_Quat[2] = vAxis.z * fSinAngle;
		m_Quat[3] = cosf(fAngle);
	}

	LTRotation( const LTVector &vForward, const LTVector &vUp )
	{
		// Are the vectors the same?
		LTVector vTempUp = vUp;
		float fDot = vForward.Dot(vUp);
		if(fDot > 0.9f || fDot < -0.9f)
		{
			// Generate a new up vector
			vTempUp.x = vForward.y;
			vTempUp.y = vForward.z;
			vTempUp.z = vForward.x;
		}

		LTMatrix mTemp;
		LTVector vRight = vForward.Cross(vTempUp);
		vRight.Normalize( );
		LTVector vTrueUp = vRight.Cross( vForward );
		mTemp.SetBasisVectors(&vRight, const_cast<LTVector*>(&vTrueUp), const_cast<LTVector*>(&vForward));
		quat_ConvertFromMatrix(m_Quat, mTemp.m);
	}

	LTRotation( float fPitch, float fYaw, float fRoll )
	{
		LTMatrix mat;
		
		float yc = (float)cos(fYaw), ys = (float)sin(fYaw);
		float pc = (float)cos(fPitch), ps = (float)sin(fPitch);
		float rc = (float)cos(fRoll), rs = (float)sin(fRoll);

		mat.m[0][0] = rc*yc + rs*ps*ys;
		mat.m[0][1] = -rs*yc + rc*ps*ys;
		mat.m[0][2] = pc*ys;
		mat.m[0][3] = 0.0f;
		
		mat.m[1][0] = rs*pc;
		mat.m[1][1] = rc*pc;
		mat.m[1][2] = -ps;
		mat.m[1][3] = 0.0f;

		mat.m[2][0] = -rc*ys + rs*ps*yc;
		mat.m[2][1] = rs*ys + rc*ps*yc;
		mat.m[2][2] = pc*yc;
		mat.m[2][3] = 0.0f;

		mat.m[3][0] = mat.m[3][1] = mat.m[3][2] = 0.0f;
		mat.m[3][3] = 1.0f;

		ConvertFromMatrix(mat);
	}

	LTRotation( float x, float y, float z, float r )
	{
		m_Quat[QX] = x;
		m_Quat[QY] = y;
		m_Quat[QZ] = z;
		m_Quat[QW] = r;
	}

	void Init( float a=0, float b=0, float c=0, float d=1 )
	{
		m_Quat[QX] = a;
		m_Quat[QY] = b;
		m_Quat[QZ] = c;
		m_Quat[QW] = d;
	}

	void Identity()
	{
		Init();
	}

	float& operator [] ( uint32 i )
	{
		return m_Quat[i];
	}

	LTRotation& operator = ( const LTRotation &q )
	{
		m_Quat[0] = q.m_Quat[0];
		m_Quat[1] = q.m_Quat[1];
		m_Quat[2] = q.m_Quat[2];
		m_Quat[3] = q.m_Quat[3];
		return *this;
	}

	LTRotation& operator = ( const LTMatrix& M )
	{
		ConvertFromMatrix(M);
		return *this;
	}

	void ConvertToMatrix( LTMatrix &M ) const
	{
		quat_ConvertToMatrix(m_Quat, M.m);
	}

	void ConvertFromMatrix( const LTMatrix &mat )
	{
		quat_ConvertFromMatrix(m_Quat, mat.m);
	}

	bool operator == ( const LTRotation &q ) const
	{
		return	m_Quat[0] == q.m_Quat[0] &&
				m_Quat[1] == q.m_Quat[1] &&
				m_Quat[2] == q.m_Quat[2] &&
				m_Quat[3] == q.m_Quat[3];
	}

	bool operator != ( const LTRotation &q ) const
	{
		return	m_Quat[0] != q.m_Quat[0] ||
				m_Quat[1] != q.m_Quat[1] ||
				m_Quat[2] != q.m_Quat[2] ||
				m_Quat[3] != q.m_Quat[3];
	}

	bool IsIdentity() const
	{
		return	m_Quat[0] == 0.0f &&
				m_Quat[1] == 0.0f &&
				m_Quat[2] == 0.0f &&
				m_Quat[3] == 1.0f;
	}

	LTRotation Conjugate() const
	{
		return LTRotation(-m_Quat[QX], -m_Quat[QY], -m_Quat[QZ], m_Quat[QW]);
	}
	LTRotation operator ~ () const
	{
		return Conjugate();
	}

	LTRotation operator * ( const LTRotation &q ) const
	{
		LTRotation qTemp;
		quat_Mul(qTemp.m_Quat, m_Quat, q.m_Quat);
		return qTemp;
	}

	void Slerp( const LTRotation &q0, const LTRotation &q1, float u )
	{
		quat_Slerp(m_Quat, q0.m_Quat, q1.m_Quat, u);
	}

	LTVector Right() const
	{
		return LTVector(
			1.0f - 2.0f * (m_Quat[1] * m_Quat[1] + m_Quat[2] * m_Quat[2]),
			2.0f * (m_Quat[0] * m_Quat[1] + m_Quat[3] * m_Quat[2]),
			2.0f * (m_Quat[0] * m_Quat[2] - m_Quat[3] * m_Quat[1]));
	}

	LTVector Up() const
	{
		return LTVector(
			2.0f * (m_Quat[0] * m_Quat[1] - m_Quat[3] * m_Quat[2]),
			1.0f - 2.0f * (m_Quat[0] * m_Quat[0] + m_Quat[2] * m_Quat[2]),
			2.0f * (m_Quat[1] * m_Quat[2] + m_Quat[3] * m_Quat[0]));
	}

	LTVector Forward() const
	{
		return LTVector(
			2.0f * (m_Quat[0] * m_Quat[2] + m_Quat[3] * m_Quat[1]),
			2.0f * (m_Quat[1] * m_Quat[2] - m_Quat[3] * m_Quat[0]),
			1.0f - 2.0f * (m_Quat[0] * m_Quat[0] + m_Quat[1] * m_Quat[1]));
	}

	// Rotate about unit-length axis vAxis by fAngle radians
	void Rotate(const LTVector &vAxis, float fAngle)
	{
		LTRotation rTemp(vAxis, fAngle);
		*this = rTemp * *this;
	}
};


// Read/write.
inline void LTStream_Read(ILTStream *pStream, LTRotation &quat)
{
	STREAM_READ(quat.m_Quat[0]);
	STREAM_READ(quat.m_Quat[1]);
	STREAM_READ(quat.m_Quat[2]);
	STREAM_READ(quat.m_Quat[3]);
}


inline void LTStream_Write(ILTStream *pStream, LTRotation &quat)
{
	STREAM_WRITE(quat.m_Quat[0]);
	STREAM_WRITE(quat.m_Quat[1]);
	STREAM_WRITE(quat.m_Quat[2]);
	STREAM_WRITE(quat.m_Quat[3]);
}

#endif//doxygen


#endif
//EOF

