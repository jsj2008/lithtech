#include "counter.h"
#define LOCK_AT_MICROSECONDS
static uint64 g_CountDiv = 1;

class CountDivSetter
{
public:
	CountDivSetter()
	{
		g_CountDiv = 1;
	}
};
static CountDivSetter __g_CountDivSetter;



unsigned long cnt_NumTicksPerSecond()
{
	/*
	LARGE_INTEGER perSec;

	QueryPerformanceFrequency(&perSec);
	return (unsigned long)(perSec.QuadPart / g_CountDiv);
	*/
return 0;      // DAN - temporary
}



CounterFinal::CounterFinal(unsigned long startMode)
{
/*
	if(startMode == CSTART_MICRO)
		StartMicro();
	else if(startMode == CSTART_MILLI)
		StartMS();
*/
}


void CounterFinal::StartMS()
{
	/*
	LARGE_INTEGER *pInt;

	pInt = (LARGE_INTEGER*)m_Data;
	QueryPerformanceCounter(pInt);
	*/
}

unsigned long CounterFinal::EndMS()
{
	/*
	LARGE_INTEGER curCount, *pInCount, perSec;
	LONGLONG timeElapsed;

	pInCount = (LARGE_INTEGER*)m_Data;
	QueryPerformanceCounter(&curCount);

	QueryPerformanceFrequency(&perSec);
	timeElapsed = curCount.QuadPart - pInCount->QuadPart;
	return (unsigned long)((timeElapsed*(LONGLONG)1000) / perSec.QuadPart);
	*/
return 0;      // DAN - temporary
}

unsigned long CounterFinal::CountMS()
{
	return EndMS();
}



void CounterFinal::StartMicro()
{
	/*
	LARGE_INTEGER *pInt;

	pInt = (LARGE_INTEGER*)m_Data;
	QueryPerformanceCounter(pInt);
	*/
}

unsigned long CounterFinal::EndMicro()
{
	/*
	LARGE_INTEGER curCount, *pInCount;

	pInCount = (LARGE_INTEGER*)m_Data;
	QueryPerformanceCounter(&curCount);

	return (unsigned long)((curCount.QuadPart - pInCount->QuadPart) / g_CountDiv);
	*/
return 0;      // DAN - temporary
}

unsigned long CounterFinal::CountMicro()
{
	return EndMicro();
}



void cnt_StartCounter(Counter &pCounter)
{
	pCounter.StartMicro();
}


unsigned long cnt_EndCounter(Counter &pCounter)
{
	return pCounter.EndMicro();
}

void cnt_StartCounterFinal(CounterFinal &pCounter)
{
	pCounter.StartMicro();
}


unsigned long cnt_EndCounterFinal(CounterFinal &pCounter)
{
	return pCounter.EndMicro();
}
