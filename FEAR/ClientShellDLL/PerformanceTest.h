 // ----------------------------------------------------------------------- //
//
// MODULE  : PerformanceTest.cpp
//
// PURPOSE : PerformanceTest class - Implementation
//
// CREATED : 08/28/02
//
// (c) 2002-2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __PERFORMANCE_TEST_H__
#define __PERFORMANCE_TEST_H__

class CPerformanceTest
{
  public :

	CPerformanceTest()
	{
		Reset();
	}

	void		Reset();
	bool		Start(uint32 nMin=kDefaultMin, uint32 nMax=kDefaultMax);
	void		Update(float fFrameTime);
	void		Stop();

	uint32		GetCurFPS() const { return m_nSmoothedCurFPS; }
	uint32		GetAveFPS() const { return m_nAveFPS; }
	uint32		GetMinFPS()	const { return m_nMinFPS; }
	uint32		GetMaxFPS()	const { return m_nMaxFPS; }

	uint32		GetPercentFPSBelowMin()	const { return m_nPercentBelowMin; }
	uint32		GetPercentFPSMintoMax()	const { return m_nPercentMintoMax; }
	uint32		GetPercentFPSAboveMax()	const { return m_nPercentAboveMax; }

	uint32		GetMinTestFPS()	const { return m_nMinTestFPS; }
	uint32		GetMaxTestFPS() const { return m_nMaxTestFPS; }

  private :

	enum	Constants { kMaxFPS = 10000, kDefaultMin = 30, kDefaultMax = 60};

	uint32		m_nAveFPS;
	uint32		m_nMinFPS;
	uint32		m_nMaxFPS;
	uint32		m_nThrowAwaySamples;
	uint32		m_nTotalSamples;
	uint32		m_nSamplesBelowMin;
	uint32		m_nSamplesAboveMax;
	uint32		m_nSmoothedCurFPS;
	uint32		m_nTotalCurFPSSamples;
	uint32		m_nTotalCurFPS;
	uint32		m_nPercentBelowMin;
	uint32		m_nPercentMintoMax;
	uint32		m_nPercentAboveMax;
	uint32		m_nTotalSmoothSamples;

	float		m_fTotalTime;
	float		m_fFPSSmoothTime;

	uint32		m_nMinTestFPS;
	uint32		m_nMaxTestFPS;
};

#endif // __PERFORMANCE_TEST_H__