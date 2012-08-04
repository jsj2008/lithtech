#include "rigid_body.h"


//---------------------------------------------------------------------------//
const LTSymMat3f CalculateInertiaTensor( const LTMatrix3f& R, const LTVector3f& Ip )
{
	const LTVector3f xx( R(0,0)*R(0,0), R(0,1)*R(0,1), R(0,2)*R(0,2) );
	const LTVector3f yy( R(1,0)*R(1,0), R(1,1)*R(1,1), R(1,2)*R(1,2) );
	const LTVector3f zz( R(2,0)*R(2,0), R(2,1)*R(2,1), R(2,2)*R(2,2) );

	const LTVector3f xy( R(0,0)*R(1,0), R(0,1)*R(1,1), R(0,2)*R(1,2) );
	const LTVector3f xz( R(0,0)*R(2,0), R(0,1)*R(2,1), R(0,2)*R(2,2) );
	const LTVector3f yz( R(1,0)*R(2,0), R(1,1)*R(2,1), R(1,2)*R(2,2) );

	LTSymMat3f I;

	//diagonal elements
	I.xx = Ip.Dot(xx);
	I.yy = Ip.Dot(yy);
	I.zz = Ip.Dot(zz);

	//off-diagonal elements
	I.xy = Ip.Dot(xy);
	I.xz = Ip.Dot(xz);
	I.yz = Ip.Dot(yz);

	return I;
}


//---------------------------------------------------------------------------//
const LTSymMat3f CalculateInverseInertiaTensor( const LTMatrix3f& R, const LTVector3f& Ip )
{
	const LTVector3f ip( 1/Ip.x, 1/Ip.y, 1/Ip.z );

	return CalculateInertiaTensor( R, ip );
}

//EOF
