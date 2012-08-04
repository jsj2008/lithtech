// ----------------------------------------------------------------------- //
//
// MODULE  : Timer.h
//
// PURPOSE : Definition/Implementation of the CTimer class
//
// CREATED : 8/31/99 (header created)
//
// (c) 1996-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef _TIMER_H_
#define _TIMER_H_

#include "iltserver.h"

class CTimer
{
public:

    CTimer() { m_t1 = 0; m_dt = 0; m_tp = 0; m_bOn = FALSE;}

#ifdef _CLIENTBUILD
    void Start(float dt) { m_dt = dt; m_t1 = g_pLTClient->GetTime(); m_bOn = TRUE;}
    void Start() { m_dt = 0; m_tp = 0; m_t1 = g_pLTClient->GetTime(); m_bOn = TRUE;}
	void Pause() { m_tp = g_pLTClient->GetTime(); }
#else
    void Start(float dt) { m_dt = dt; m_t1 = g_pLTServer->GetTime(); m_bOn = TRUE;}
    void Start() { m_dt = 0; m_tp = 0; m_t1 = g_pLTServer->GetTime(); m_bOn = TRUE;}
	void Pause() { m_tp = g_pLTServer->GetTime(); }
#endif
    void Stop() { m_dt = 0; m_tp = 0; m_bOn = FALSE;}
    BOOL Stopped();
	BOOL Paused() {return (m_tp > 0);}
	BOOL On() { return m_bOn; }
	void Resume();
    float GetElapseTime();

	void Add(float t) { m_dt += t; m_dt = (m_dt < 0.0f ? 0 : m_dt); }

	float GetCountdownTime();
	float GetDuration() { return m_dt; }

#ifndef _CLIENTBUILD
	void Save(HMESSAGEWRITE hWrite);
	void Load(HMESSAGEREAD hRead);
#endif

private:

    float m_t1;
    float m_dt;
    float m_tp;
	BOOL  m_bOn;
};

inline BOOL CTimer::Stopped() 
{ 
	if (Paused())
	{
		return (m_tp - m_t1 >= m_dt) ? TRUE : FALSE; 
	}
	else
	{
#ifdef _CLIENTBUILD
	return (g_pLTClient->GetTime() - m_t1 >= m_dt) ? TRUE : FALSE; 
#else
	return (g_pLTServer->GetTime() - m_t1 >= m_dt) ? TRUE : FALSE; 
#endif
	}
}

inline void CTimer::Resume()
{
#ifdef _CLIENTBUILD
	float dp = g_pLTClient->GetTime() - m_tp;
#else
	float dp = g_pLTServer->GetTime() - m_tp;
#endif
	m_tp = 0;
	m_t1 += dp;
}

inline float CTimer::GetElapseTime()
{
	if (Paused())
	{
		return m_tp - m_t1;
	}
	else
	{
#ifdef _CLIENTBUILD
		return g_pLTClient->GetTime() - m_t1; 
#else
		return g_pLTServer->GetTime() - m_t1; 
#endif
	}
}

inline float CTimer::GetCountdownTime()
{
	if (Stopped())
		return 0;
	if (Paused())
	{
		return m_t1 + m_dt - m_tp;
	}
	else
	{
#ifdef _CLIENTBUILD
    return m_t1 + m_dt - g_pLTClient->GetTime();
#else
    return m_t1 + m_dt - g_pLTServer->GetTime();
#endif
	}
}

#ifndef _CLIENTBUILD
inline void CTimer::Save(HMESSAGEWRITE hWrite)
{
    g_pLTServer->WriteToMessageFloat(hWrite, m_t1);
    g_pLTServer->WriteToMessageFloat(hWrite, m_dt);
    g_pLTServer->WriteToMessageFloat(hWrite, m_tp);
}

inline void CTimer::Load(HMESSAGEREAD hRead)
{
    m_t1 = g_pLTServer->ReadFromMessageFloat(hRead);
    m_dt = g_pLTServer->ReadFromMessageFloat(hRead);
    m_tp = g_pLTServer->ReadFromMessageFloat(hRead);
}
#endif

#endif