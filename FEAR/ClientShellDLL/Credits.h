// ----------------------------------------------------------------------- //
//
// MODULE  : Credits.h
//
// PURPOSE : Definition of class to manage in-game credits
//
// CREATED : 11/09/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __CREDITS_H__
#define __CREDITS_H__

class CreditOrder
{
public: 
	CreditOrder();
	virtual ~CreditOrder();

	void	Init();
	void	Term();

	void	Update();
	void	Render();

	void	Start(HRECORD hOrder);
	void	Start(const char* szOrder);
	void	Stop();
	bool	IsPlaying() const { return m_bPlaying; }

	HRECORD GetRecord() const { return m_hOrder; }

private:

	bool	StartEntry();
	void	ClearStrings();

	HRECORD			m_hOrder;

	uint32			m_nIndex;
	bool			m_bPlaying;

	LTGUIStringArray m_Lines;
	uint32			m_nLines;
	Tuint32List		m_cTextColors;
	bool			m_bDoFlicker;
	bool			m_bDoDistort;
	HRECORD			m_hFlickerSound;

	HRECORD			m_hEntry;

	enum CreditState
	{
		eState_FadeIn,
		eState_Hold,
		eState_FadeOut,
		eState_Delay,
	};

	CreditState		m_eState;
	StopWatchTimer	m_Timer;
	StopWatchTimer	m_FlickerTimer;
	bool			m_bFlicker;
	float			m_fFade;

};

typedef std::vector< CreditOrder*, LTAllocator<CreditOrder*, LT_MEM_TYPE_CLIENTSHELL> > CreditOrderList;

class Credits
{
	DECLARE_SINGLETON( Credits );

public: 
	void	Init();
	void	Term();

	void	Update();
	void	Render();

	void	Start(HRECORD hOrder);
	void	Start(const char* szOrder);
	void	Stop(HRECORD hOrder);
	void	StopAll();

	bool	IsPlaying() const { return !m_ActiveOrders.empty(); }
private:

	CreditOrderList	m_ActiveOrders;
	CreditOrderList	m_ReserveOrders;

};


#endif  // __CREDITS_H__
