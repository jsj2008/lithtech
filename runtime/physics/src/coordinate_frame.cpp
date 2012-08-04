#include "coordinate_frame.h"


//---------------------------------------------------------------------------//
LTBasis::LTBasis( const float pitch, const float yaw, const float roll )
{
	/*
	CMatrix		yawMat( yaw_cos,	0.0f,	yaw_sin,
						0.0f,		1.0f,	0.0f,	
						-yaw_sin,	0.0f,	yaw_cos);

	CMatrix		pitchMat(	1.0f,	0.0f,		0.0f,
							0.0f,	pitch_cos,	-pitch_sin,
							0.0f,	pitch_sin,	pitch_cos);

	CMatrix		rollMat(	roll_cos,	-roll_sin,	0.0f,
							roll_sin,	roll_cos,	0.0f,
							0.0f,		0.0f,		1.0f);

	CMatrix fullMat = yawMat * pitchMat * rollMat;
	*/
	const float yc = cosf(yaw), ys = sinf(yaw);
	const float pc = cosf(pitch), ps = sinf(pitch);
	const float rc = cosf(roll), rs = sinf(roll);

	m_M(0,0) = rc*yc + rs*ps*ys;
	m_M(0,1) = -rs*yc + rc*ps*ys;
	m_M(0,2) = pc*ys;

	m_M(1,0) = rs*pc;
	m_M(1,1) = rc*pc;
	m_M(1,2) = -ps;

	m_M(2,0) = -rc*ys + rs*ps*yc;
	m_M(2,1) = rs*ys + rc*ps*yc;
	m_M(2,2) = pc*yc;
}


//---------------------------------------------------------------------------//
LTBasis::LTBasis( const LTVector3f& f, const LTVector3f& u )
{
	const float lf = f.Length();
	float lu = u.Length();

	if( lf>0 && lu>0 )//don't divide by 0
	{
		const LTVector3f forw = f / lf;//normalize
		LTVector3f up = u / lu;//normalize

		//subtract any component along 'f'
		up -= up.Dot(forw)*forw;

		lu = up.Length();

		if( lu > 0 )
		{
			up /= lu;//re-normalize

			const LTVector3f right = up.Cross(forw);

			m_M(2) = forw;
			m_M(1) = up;
			m_M(0) = right;
		}
	}
}


//---------------------------------------------------------------------------//
void LTBasis::RotateAboutX( const float a )
{
	const float c = cosf(a);
	const float s = sinf(a);
	LTVector3f b1 =  this->Y()*c + this->Z()*s;
	LTVector3f b2 = -this->Y()*s + this->Z()*c;

		//set y and z
		this->m_M(1) = b1;
		this->m_M(2) = b2;
		//x is unchanged
}


//---------------------------------------------------------------------------//
void LTBasis::RotateAboutY( const float a )
{
	const float c = cosf(a);
	const float s = sinf(a);
	LTVector3f b0 = -this->Z()*s + this->X()*c;	//rotated x
	LTVector3f b2 =  this->Z()*c + this->X()*s;	//rotated z

		//set x and z
		this->m_M(0) = b0;
		this->m_M(2) = b2;
		//y is unchanged
}


//---------------------------------------------------------------------------//
void LTBasis::RotateAboutZ( const float a )
{
	const float c = cosf(a);
	const float s = sinf(a);
	//don't over-write basis before calculation is done
	LTVector3f b0 =  this->X()*c + this->Y()*s;	//rotated x
	LTVector3f b1 = -this->X()*s + this->Y()*c;	//rotated y

		//set x and y
		this->m_M(0) = b0;
		this->m_M(1) = b1;
		//z is unchanged
}


//---------------------------------------------------------------------------//
void LTBasis::Rotate( const float a, const LTVector3f& u )
{
	//create a rotation matrix from the given info
	const LTMatrix3f R = RotationMatrix( a, u );

		//rotate each base (column) vector
		this->m_M = R * this->m_M;
}


//---------------------------------------------------------------------------//
LTOrientation::LTOrientation
(
	const LTVector3f& f,//"forward" direction
	const LTVector3f& u	//"upward" direction
)
{
	LTBasis b(f,u);

	(*this) = b.m_M;
}


//EOF
