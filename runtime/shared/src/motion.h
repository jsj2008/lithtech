#ifndef __MOTION_H__
#define __MOTION_H__


struct MotionInfo
{
	// Gravity info.
	LTVector	m_Force, m_UnitForce;
	float	m_ForceMag;
	float	m_SlideRatio;

	MotionInfo()
	{
		// Set the gravity...
		LTVector force(0.0f, -2000.0f, 0.0f);
		SetForce(&force);

		// Set the slide ratio.  This determines if gravity is applied as we stand on a slope.
		// This is inv cos between the normal of the plane we are standing and gravity.  The
		// default is 135 degrees...
		m_SlideRatio = -0.7071f;
	}

	void SetForce( const LTVector *pForce )
	{
		m_Force = *pForce;
		m_ForceMag = m_Force.Mag();
		if(m_ForceMag > 0.00001f)
		{
			m_UnitForce = m_Force;
			m_UnitForce /= m_ForceMag;
		}
		else
		{
			m_UnitForce.Init();
		}
	}
};


//NOTE:  Unfortunately, can't delete this because
//clientde_impl.cpp needs to know about this as well
void CalcMotion
(
	MotionInfo*	pInfo,
	LTObject*	pObj,			//the object
	LTVector&	dr,				//displacement
	LTVector&	v,				//velocity
	LTVector&	a,				//acceleration
	const bool	bApplyGravity,
	const float dt				//time step
);


#endif
//EOF
