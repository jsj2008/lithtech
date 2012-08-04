
//#include "bdefs.h"
#include <windows.h>
#include "counter.h"


//#define LOCK_AT_MICROSECONDS


static unsigned __int64 g_CountDiv = 1;


// Win2K has a VERY high counter frequency but it's more useful if it's in 
// microseconds.
class CountDivSetter
{
public:
	CountDivSetter()
	{
		LARGE_INTEGER perSec;
		
		QueryPerformanceFrequency(&perSec);
#ifdef LOCK_AT_MICROSECONDS
		g_CountDiv = perSec.QuadPart / 1000000;
#else
		g_CountDiv = 1;
#endif
	}
};
static CountDivSetter __g_CountDivSetter;



unsigned long cnt_NumTicksPerSecond()
{
	LARGE_INTEGER perSec;

	QueryPerformanceFrequency(&perSec);
	return (unsigned long)(perSec.QuadPart / g_CountDiv);	
}



CounterFinal::CounterFinal(unsigned long startMode)
{
	if(startMode == CSTART_MICRO)
		StartMicro();
	else if(startMode == CSTART_MILLI)
		StartMS();
}


void CounterFinal::StartMS()
{
	LARGE_INTEGER *pInt;

	pInt = (LARGE_INTEGER*)m_Data;
	QueryPerformanceCounter(pInt);
}

unsigned long CounterFinal::EndMS()
{
	LARGE_INTEGER curCount, *pInCount, perSec;
	LONGLONG timeElapsed;

	pInCount = (LARGE_INTEGER*)m_Data;
	QueryPerformanceCounter(&curCount);

	QueryPerformanceFrequency(&perSec);
	timeElapsed = curCount.QuadPart - pInCount->QuadPart;
	return (unsigned long)((timeElapsed*(LONGLONG)1000) / perSec.QuadPart);
}

unsigned long CounterFinal::CountMS()
{
	return EndMS();
}



void CounterFinal::StartMicro()
{
	LARGE_INTEGER *pInt;

	pInt = (LARGE_INTEGER*)m_Data;
	QueryPerformanceCounter(pInt);
}

unsigned long CounterFinal::EndMicro()
{
	LARGE_INTEGER curCount, *pInCount;

	pInCount = (LARGE_INTEGER*)m_Data;
	QueryPerformanceCounter(&curCount);

	return (unsigned long)((curCount.QuadPart - pInCount->QuadPart) / g_CountDiv);
}

unsigned long CounterFinal::CountMicro()
{
	return EndMicro();
}



void cnt_StartCounterFinal(CounterFinal &cCounter)
{
	cCounter.StartMicro();
}


unsigned long cnt_EndCounterFinal(CounterFinal &cCounter)
{
	return cCounter.EndMicro();
}


#ifndef _FINAL

float CountPercent::CalcPercent()
{
	LARGE_INTEGER *pTotalOut = (LARGE_INTEGER*)m_TotalOut;

	// Handle the never-got-called case
	if ((m_TotalOut[0] | m_TotalOut[1]) == 0)
		return 0.0f;

	LARGE_INTEGER *pTotalIn = (LARGE_INTEGER*)m_TotalIn;

	LONGLONG result = (pTotalIn->QuadPart * (LONGLONG)10000) / (pTotalIn->QuadPart + pTotalOut->QuadPart);

	return (float)result / 10000.0f;
}

// Clears the totals
void CountPercent::Clear()
{
	m_Finger[0] = 0;
	m_Finger[1] = 0;
	m_TotalIn[0] = 0;
	m_TotalIn[1] = 0;
	m_TotalOut[0] = 0;
	m_TotalOut[1] = 0;

	m_iIn = 0;
}

// Call to enter the profiled section
uint CountPercent::In()
{
	if (m_iIn++)
		return 0;

	LARGE_INTEGER now, *pThen = (LARGE_INTEGER*)m_Finger, result;
	QueryPerformanceCounter(&now);

	// Keep track of the out time
	if ((m_Finger[0] | m_Finger[1]) != 0)
	{
		LARGE_INTEGER *pTotalOut = (LARGE_INTEGER*)m_TotalOut;
		result.QuadPart = now.QuadPart - pThen->QuadPart;
		pTotalOut->QuadPart += result.QuadPart;
	}
	else
		result.QuadPart = 0;

	*pThen = now;

	return (uint)(result.QuadPart);
}
		
// Call to exit the profiled section
uint CountPercent::Out()
{
	if (--m_iIn)
		return 0;

	LARGE_INTEGER now, *pThen = (LARGE_INTEGER*)m_Finger, result;
	QueryPerformanceCounter(&now);

	// Keep track of the in time
	LARGE_INTEGER *pTotalIn = (LARGE_INTEGER*)m_TotalIn;
	result.QuadPart = now.QuadPart - pThen->QuadPart;
	pTotalIn->QuadPart += result.QuadPart;

	*pThen = now;

	return (uint)(result.QuadPart);
}

#endif // !_FINAL
