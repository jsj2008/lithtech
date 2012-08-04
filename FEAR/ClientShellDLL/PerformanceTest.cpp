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

#include "stdafx.h"
#include "PerformanceTest.h"
#include "VarTrack.h"

VarTrack	g_vtPerformanceSmoothFPSTime;
VarTrack	g_vtPerformanceMinSampleFPS;
VarTrack	g_vtPerformanceMaxSampleFPS;
VarTrack	g_vtPerformanceThrowAwaySamples;
VarTrack	g_vtPerformanceProfile;
VarTrack	g_vtPerformanceSmooth;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPerformanceTest::Reset()
//
//	PURPOSE:	Reset the test
//
// ----------------------------------------------------------------------- //

void CPerformanceTest::Reset()
{
	m_nThrowAwaySamples		= 0;
	m_nTotalSamples			= 0;
	m_nSamplesBelowMin		= 0;
	m_nSamplesAboveMax		= 0;
	m_nSmoothedCurFPS		= 0;
	m_nTotalCurFPSSamples	= 0;
	m_nTotalCurFPS			= 0;
	m_nAveFPS				= 0;
	m_nMinFPS				= kMaxFPS;
	m_nMaxFPS				= 0;
	m_nPercentBelowMin		= 0;
	m_nPercentMintoMax		= 0;
	m_nPercentAboveMax		= 0;
	m_fTotalTime			= 0.0f;
	m_fFPSSmoothTime		= 0.0f;
	m_nTotalSmoothSamples	= 0;
	m_nMinTestFPS			= kDefaultMin;
	m_nMaxTestFPS			= kDefaultMax;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPerformanceTest::Start()
//
//	PURPOSE:	Start the test
//
// ----------------------------------------------------------------------- //

bool CPerformanceTest::Start(uint32 nMin /*=kDefaultMin*/, uint32 nMax /*=kDefaultMax*/)
{
	Reset();

	if (nMin < nMax)
	{
		m_nMinTestFPS = nMin;
		m_nMaxTestFPS = nMax;
	}
	else
	{
		return false;
	}

	if (!g_vtPerformanceSmoothFPSTime.IsInitted())
	{
		g_vtPerformanceSmoothFPSTime.Init(g_pLTClient, "PerformanceFPSSmoothTime", NULL, 1.0f);
	}
	if (!g_vtPerformanceMinSampleFPS.IsInitted())
	{
		g_vtPerformanceMinSampleFPS.Init(g_pLTClient, "PerformanceMinSampleFrameRate", NULL, 0.00001f);
	}
	if (!g_vtPerformanceMaxSampleFPS.IsInitted())
	{
		g_vtPerformanceMaxSampleFPS.Init(g_pLTClient, "PerformanceMaxSampleFrameRate", NULL, 1.0f);
	}
	if (!g_vtPerformanceThrowAwaySamples.IsInitted())
	{
		g_vtPerformanceThrowAwaySamples.Init(g_pLTClient, "PerformanceThrowAwaySamples", NULL, 10.0f);
	}
	if (!g_vtPerformanceProfile.IsInitted())
	{
		g_vtPerformanceProfile.Init(g_pLTClient, "PerformanceProfile", NULL, 0.0f);
	}
	if (!g_vtPerformanceSmooth.IsInitted())
	{
		g_vtPerformanceSmooth.Init(g_pLTClient, "PerformanceSmooth", NULL, 1.0f);
	}

#ifndef _FINAL
	// If requested turn on development profiling...
	if (g_vtPerformanceProfile.GetFloat() > 0.0f)
	{
		g_pLTClient->RunConsoleCommand("ProfilerResume");
	}
#endif

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPerformanceTest::Update()
//
//	PURPOSE:	Update our calculations
//
// ----------------------------------------------------------------------- //

void CPerformanceTest::Update(float fFrameTime)
{
	// Skip the first few samples since they skew the results...
	m_nThrowAwaySamples++;
	if (m_nThrowAwaySamples <= uint32(g_vtPerformanceThrowAwaySamples.GetFloat()))
	{
		return;
	}

	// Throw out any bogus frame times (too fast or too slow)...
	if (fFrameTime < g_vtPerformanceMinSampleFPS.GetFloat() || 
		fFrameTime > g_vtPerformanceMaxSampleFPS.GetFloat())
	{
		return;
	}

	uint32 nCurFPS = (fFrameTime > 0.0 ? int(1.0/fFrameTime) : kMaxFPS);

	m_nTotalSamples++;
	m_fTotalTime += fFrameTime;
	m_fFPSSmoothTime += fFrameTime;

	// Smooth out the "current" fps over kNumSmoothFrames...

	if (g_vtPerformanceSmooth.GetFloat() != 0.0f)
	{
		if (m_fFPSSmoothTime <= g_vtPerformanceSmoothFPSTime.GetFloat())
		{
			m_nTotalCurFPS += nCurFPS;
			m_nTotalCurFPSSamples++;
		}
		else
		{
			// Time to calculate smooth current fps...

			if (m_nTotalCurFPSSamples > 0)
			{
				m_nSmoothedCurFPS = (m_nTotalCurFPS / m_nTotalCurFPSSamples);

				// Update the min/max samples if appropriate...

				if (m_nSmoothedCurFPS < m_nMinFPS)
				{
					m_nMinFPS = m_nSmoothedCurFPS;
				}

				if (m_nSmoothedCurFPS > m_nMaxFPS)
				{
					m_nMaxFPS = m_nSmoothedCurFPS;
				}

				if (m_nSmoothedCurFPS < m_nMinTestFPS)
				{
					m_nSamplesBelowMin++;
				}
				else if (m_nSmoothedCurFPS > m_nMaxTestFPS)
				{
					m_nSamplesAboveMax++;
				}

				m_nTotalSmoothSamples++;
			}

			m_nTotalCurFPS = 0;
			m_nTotalCurFPSSamples = 0;
			m_fFPSSmoothTime = 0.0f;
		}
	}
	else
	{
		// If we're not smoothing the frame rate, update the min/max values
		// as appropriate...

		if (nCurFPS < m_nMinFPS)
		{
			m_nMinFPS = nCurFPS;
		}

		if (nCurFPS > m_nMaxFPS)
		{
			m_nMaxFPS = nCurFPS;
		}

		if (nCurFPS < m_nMinTestFPS)
		{
			m_nSamplesBelowMin++;
		}
		else if (nCurFPS > m_nMaxTestFPS)
		{
			m_nSamplesAboveMax++;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPerformanceTest::Stop()
//
//	PURPOSE:	Stop the test and calculate the results
//
// ----------------------------------------------------------------------- //

void CPerformanceTest::Stop()
{
	if (m_nTotalSamples > 0)
	{
		float fAverage = m_fTotalTime / float(m_nTotalSamples);

		m_nAveFPS = (fAverage > 0.0 ? int(1.0/fAverage) : kMaxFPS);

		uint32 nTotalSamples = ((g_vtPerformanceSmooth.GetFloat() != 0.0f) ? 
								 m_nTotalSmoothSamples : m_nTotalSamples);
		
		m_nPercentBelowMin = int(100.0 * float(m_nSamplesBelowMin) / float(nTotalSamples));
		m_nPercentAboveMax = int(100.0 * float(m_nSamplesAboveMax) / float(nTotalSamples));
		m_nPercentMintoMax  = (100 - (m_nPercentBelowMin + m_nPercentAboveMax));

		if (m_nPercentMintoMax < 0) 
			m_nPercentMintoMax = 0;
	}

#ifndef _FINAL
	// If requested turn off development profiling...
	if (g_vtPerformanceProfile.GetFloat() > 0.0f)
	{
		g_pLTClient->RunConsoleCommand("ProfilerPause");
	}
#endif
}
