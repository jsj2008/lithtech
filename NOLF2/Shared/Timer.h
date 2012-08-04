// ----------------------------------------------------------------------- //
//
// MODULE  : Timer.h
//
// PURPOSE : Definition/Implementation of the CTimer class
//
// CREATED : 8/31/99 (header created)
//
// (c) 1996-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef _TIMER_H_
#define _TIMER_H_

#include "iltserver.h"

class CTimer
{
public:

    CTimer() { m_t1 = 0; m_dt = 0; m_tp = 0; m_bOn = false;}

#ifdef _CLIENTBUILD
    void Start(float dt) { m_dt = dt; m_t1 = g_pLTClient->GetTime(); m_bOn = true;}
    void Start() { m_dt = 0; m_tp = 0; m_t1 = g_pLTClient->GetTime(); m_bOn = true;}
	void Pause() { m_tp = g_pLTClient->GetTime(); }
#else
    void Start(float dt) { m_dt = dt; m_t1 = g_pLTServer->GetTime(); m_bOn = true;}
    void Start() { m_dt = 0; m_tp = 0; m_t1 = g_pLTServer->GetTime(); m_bOn = true;}
	void Pause() { m_tp = g_pLTServer->GetTime(); }
#endif
    void Stop() { m_dt = 0; m_tp = 0; m_bOn = false;}
    bool Stopped()  const;
	bool Paused()  const {return (m_tp > 0);}
	bool On() { return m_bOn; }
	void Resume();
    float GetElapseTime() const;

	void Add(float t) { m_dt += t; m_dt = (m_dt < 0.0f ? 0 : m_dt); }

	float GetCountdownTime() const;
	float GetDuration() { return m_dt; }

#ifndef _CLIENTBUILD
	void Save(ILTMessage_Write *pMsg);
	void Load(ILTMessage_Read *pMsg);
#endif

private:

    float m_t1;
    float m_dt;
    float m_tp;
	bool  m_bOn;
};

inline bool CTimer::Stopped()  const
{ 
	if (Paused())
	{
		return (m_tp - m_t1 >= m_dt) ? true : false; 
	}
	else
	{
#ifdef _CLIENTBUILD
	return (g_pLTClient->GetTime() - m_t1 >= m_dt) ? true : false; 
#else
	return (g_pLTServer->GetTime() - m_t1 >= m_dt) ? true : false; 
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

inline float CTimer::GetElapseTime() const
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

inline float CTimer::GetCountdownTime() const
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
inline void CTimer::Save(ILTMessage_Write *pMsg)
{
	SAVE_TIME( m_t1 );
	SAVE_TIME( m_tp );
    SAVE_FLOAT( m_dt );
	SAVE_bool( m_bOn );
}

inline void CTimer::Load(ILTMessage_Read *pMsg)
{
	LOAD_TIME( m_t1 );
	LOAD_TIME( m_tp );
    LOAD_FLOAT( m_dt );
	LOAD_bool( m_bOn );
}
#endif

#endif