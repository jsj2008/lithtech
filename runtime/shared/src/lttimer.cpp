
#include "bdefs.h"
#include "syscounter.h"
#include "lttimer.h"


LTTimer::LTTimer()
{
	m_UpdateRate = 0.0f;
	m_Adjust = 0.0f;
}


void LTTimer::SetUpdateRate(float rate)
{
	m_UpdateRate = rate;
	m_Adjust = 0.0f;
	m_Counter.StartMS();
}


float LTTimer::GetUpdateRate()
{
	return m_UpdateRate;
}


LTBOOL LTTimer::Update()
{
	float secondsElapsed, invUpdateRate;

	if(m_UpdateRate == 0.0f)
		return LTFALSE;

	invUpdateRate = 1.0f / m_UpdateRate;

	secondsElapsed = (m_Counter.CountMS() / 1000.0f) + m_Adjust;
	if(secondsElapsed > invUpdateRate)
	{
		m_Adjust = (float)fmod(secondsElapsed, invUpdateRate);
		m_Counter.StartMS();
		return LTTRUE;
	}
	else
	{
		return LTFALSE;
	}
}





