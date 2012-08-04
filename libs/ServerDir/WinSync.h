//////////////////////////////////////////////////////////////////////////////
// Windows-specific synchronization object wrapping classes

#ifndef __WINSYNC_H__
#define __WINSYNC_H__

// Critical section
class CWinSync_CS
{
public:
	CWinSync_CS() {
		InitializeCriticalSection(&GetCS());
	}
	~CWinSync_CS() {
		DeleteCriticalSection(&GetCS());
	}

	void Enter() { 
		EnterCriticalSection(&GetCS());
	}

	void Leave() {
		LeaveCriticalSection(&GetCS());
	}

	CRITICAL_SECTION &GetCS() const { return m_CS; }

private:
	mutable CRITICAL_SECTION m_CS;
};

// Auto enter/leave wrapper for a critical section based on variable scope
class CWinSync_CSAuto
{
public:
	CWinSync_CSAuto(const CWinSync_CS &cCS) : m_pCS(&cCS.GetCS())
	{
		EnterCriticalSection(m_pCS);
	}
	CWinSync_CSAuto(CRITICAL_SECTION *pCS) : m_pCS(pCS)
	{
		EnterCriticalSection(m_pCS);
	}
	~CWinSync_CSAuto()
	{
		LeaveCriticalSection(m_pCS);
	}

private:
	mutable CRITICAL_SECTION *m_pCS;
};

// Event
class CWinSync_Event
{
public:
	CWinSync_Event() {
		SetEvent(CreateEvent(0, TRUE, FALSE, 0));
	}
	~CWinSync_Event() {
		CloseHandle(m_hEvent);
	}
	HANDLE GetEvent() const { return m_hEvent; }
	void Set() { ::SetEvent(GetEvent()); }
	bool Clear() { return ResetEvent(GetEvent()) == TRUE; }
	bool IsSet() { return WaitForSingleObject(GetEvent(), 0) == WAIT_OBJECT_0; }
	bool Block(uint32 nTimeout = INFINITE) { return WaitForSingleObject(GetEvent(), nTimeout) == WAIT_OBJECT_0; }
protected:
	void SetEvent(HANDLE hEvent) { m_hEvent = hEvent; }
private:
	HANDLE m_hEvent;
};

// Event which resets after releasing a block
class CWinSync_PulseEvent : public CWinSync_Event
{
public:
	CWinSync_PulseEvent() {
		SetEvent(CreateEvent(0, FALSE, FALSE, 0));
	}
};

#endif //__WINSYNC_H__