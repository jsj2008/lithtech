
// This file defines some helpful profiling counter routines.

#ifndef __COUNTER_H__
#define __COUNTER_H__

	#include "ltbasetypes.h"

	void dsi_PrintToConsole(const char *pMsg, ...);


	// You can start a counter in its constructor with these.
	#define CSTART_NONE			0
	#define CSTART_MILLI		1
	#define CSTART_MICRO		2

	
	class CounterFinal
	{
	public:

						CounterFinal(unsigned long startMode=CSTART_NONE);

		// Measure in milliseconds (1/1,000).
		void			StartMS();
		unsigned long	EndMS();
		unsigned long	CountMS();

		// Measure in microseconds (1/1,000,000).
		void			StartMicro();
		unsigned long	EndMicro();
		unsigned long	CountMicro();

		unsigned long m_Data[2];
	};

	typedef CounterFinal Counter;


	// How many ticks per second are there?
	unsigned long cnt_NumTicksPerSecond();

	
	// Start a counter.
	void cnt_StartCounter(Counter &pCounter);

	// Returns the number of ticks since you called StartCounter.
	unsigned long cnt_EndCounter(Counter &pCounter);

	// Start a counter.
	void cnt_StartCounterFinal(CounterFinal &pCounter);

	// Returns the number of ticks since you called StartCounter.
	unsigned long cnt_EndCounterFinal(CounterFinal &pCounter);

	
	// C++ helpers..
	class CountAdder
	{
		public:

			CountAdder(uint32 *pNum)
			{
				m_pNum = pNum;
				cnt_StartCounter(m_Counter);
			}

			~CountAdder()
			{
				*m_pNum += cnt_EndCounter(m_Counter);
			}

			Counter m_Counter;
			uint32 *m_pNum;
	};

	class CountPrinter
	{
	public:
				CountPrinter(char *pMsg)
				{
					m_pMsg = pMsg;
					cnt_StartCounter(m_Counter);
				}

				~CountPrinter()
				{
					dsi_PrintToConsole(m_pMsg, cnt_EndCounter(m_Counter));
				}
		
		char	*m_pMsg;
		Counter	m_Counter;
	};

	
	
#endif  // __COUNTER_H__



