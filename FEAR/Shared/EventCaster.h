// ----------------------------------------------------------------------- //
//
// MODULE  : EventCaster.h
//
// PURPOSE : EventCaster - defines classes to support casting and receiving events.
//
// There are 4 contributors to this design pattern:  Subject, EventCaster, Observer and 
// Delegate.  The Subject publishes an EventCaster.  An Observer listens to events of a
// Subject by listening to its EventCaster with its Delegate. 
//
// (c) 1997-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __EVENTCASTER_H__
#define __EVENTCASTER_H__

#include "stdafx.h"
#include <vector>
#include <algorithm>

class DelegateBase;

// EventCaster
//
// Used to encapsulate signaling event fired from a subject to multiple observers.
//
// Steps to setup:
// 1.  Add a DECLARE_EVENT with the name of the event.  This will define itself in the public scope.
// 2.  Reference the event object by its name (e.g. DECLARE_EVENT( MyEvent );  Can be referenced like
//     MyEvent.DoNotify();
// 3.  Observers can attach/detach delegates by calling <MyEvent>.Attach() or <MyEvent>.Detach().
// 4.  Subject can signal event by calling <MyEvent>.DoNotify().
class EventCaster
{
public:
	virtual ~EventCaster( );

	// Observer calls to attach a delegate.
	virtual void Attach( DelegateBase& delegate )
	{
		m_lstDelegates.push_back( &delegate );
	}
	// Observer calls to detach a delegate.
	virtual void Detach( DelegateBase& delegate )
	{
		DelegateList::iterator iter = std::find( m_lstDelegates.begin(), m_lstDelegates.end( ), &delegate );
		if( iter == m_lstDelegates.end( ))
			return;
		m_lstDelegates.erase( iter );
	}

	// Default parameters to send with notification event.  Subjects can derive from this
	// to send specialized data.
	struct NotifyParams
	{
		NotifyParams( EventCaster& eventCaster ) : m_EventCaster( eventCaster )
		{
		}

		EventCaster& m_EventCaster;
	};

	// Subject calls to signal event.
	virtual void DoNotify( )
	{
		NotifyParams cParams( *this );
		DoNotify( cParams );
	}

	// Subject calls to signal event with specialized notifyparams.
	virtual void DoNotify( NotifyParams& notifyParams );

private:

	// List of delegates attached.
	typedef std::vector< DelegateBase* > DelegateList;
	DelegateList m_lstDelegates;
};

// Base class delegate that EventCaster works with.  Observers need to use the Delegate version.
class DelegateBase
{
public:
	virtual ~DelegateBase( ) {}
	virtual void OnEvent( EventCaster::NotifyParams& notifyParams ) = 0;
	virtual void OnDestroy( EventCaster& event ) = 0;
};

// Delegate
//
// Used to encapsulate callback from an event signaled from a subject to an observer.
//
// Steps to setup:
// 1.  Define a static function of type EventFunction that will receive the event.  
// 2.  Declare a delegate object and give it the type of object that is observering (TObserver), the type
//     of object being observered (TSubject), and the static callback to call when event fires.
// 3.  Attach the delegate of the observer to the event of the subject.
template< typename TObserver, typename TSubject, void (*EventFunction)( TObserver* pObserver, TSubject* pSubject, EventCaster::NotifyParams& notifyParams ) >
class Delegate : public DelegateBase
{
public:

	Delegate( )
	{
		m_pObserver = NULL;
		m_pSubject = NULL;
		m_pEventCaster = NULL;
	}

	~Delegate( )
	{
		Detach( );
	}

	// Attach delegate to event with observer, subject and callback.
	void Attach( TObserver* pObserver, TSubject* pSubject, EventCaster& eventCaster )
	{
		// Make sure we're detached from previous event.
		if( m_pEventCaster )
		{
			m_pEventCaster->Detach( *this );
		}

		m_pObserver = pObserver;
		m_pSubject = pSubject;
		m_pEventCaster = &eventCaster;
		m_pEventCaster->Attach( *this );
	}

	// Detach delegate from event.
	void Detach( )
	{
		if( m_pEventCaster )
		{
			m_pEventCaster->Detach( *this );
			m_pEventCaster = NULL;
		}
		m_pObserver = NULL;
		m_pSubject = NULL;
	}

	// Called when event is signaled by the subject.
	virtual void OnEvent( EventCaster::NotifyParams& notifyParams )
	{
		EventFunction( m_pObserver, m_pSubject, notifyParams );
	}

	// Called when the subject is destroyed.
	virtual void OnDestroy( EventCaster& eventCaster )
	{
		// Make sure we're detached.
		Detach();
	}

private:

	// The object observing.
	TObserver* m_pObserver;
	// The object being observed.
	TSubject* m_pSubject;
	// The current event we are attached to.
	EventCaster* m_pEventCaster;
};

inline EventCaster::~EventCaster( ) 
{
	// Make a copy and use the copy to send the events.  This prevents problems when delegates
	// get removed during the event call.
	DelegateList lstDelegates;
	lstDelegates = m_lstDelegates;
	for( DelegateList::iterator iter = lstDelegates.begin( ); iter != lstDelegates.end( ); iter++ )
	{
		(*iter)->OnDestroy( *this );
	}
}

inline void EventCaster::DoNotify( NotifyParams& notifyParams )
{
	// Make a copy and use the copy to send the events.  This prevents problems when delegates
	// get removed during the event call.
	DelegateList lstDelegates;
	lstDelegates = m_lstDelegates;
	for( DelegateList::iterator iter = lstDelegates.begin( ); iter != lstDelegates.end( ); iter++ )
	{
		(*iter)->OnEvent( notifyParams );
	}
}


// Add a DECLARE_EVENT for each event your class supports.  When
// you want to signal the font, call <MyEvent>.Notify().
#define DECLARE_EVENT( EventName ) \
	public: \
		class EventName##EventCaster : public EventCaster { }; \
		EventName##EventCaster EventName;

// Example usage:
/*
class ClockTimer
{
public:

	ClockTimer( ) 
	{
		m_nTime = 0;
	}

	void AddTime( int nValue )
	{
		m_nTime += nValue;
		AddTimeEvent.DoNotify();
	}

	void DecTime( int nValue )
	{
		m_nTime -= nValue;
		DecTimeEvent.DoNotify();
	}

	int GetTime( ) const { return m_nTime; }

	DECLARE_EVENT( AddTimeEvent );
	DECLARE_EVENT( DecTimeEvent );

private:

	int m_nTime;
};

class DigitalClock
{
public:

	DigitalClock( )
	{
		m_nTime = 0;
	}

	void SetTimer( ClockTimer& timer )
	{
		m_dlgAddTime.Attach( *this, timer, timer.AddTimeEvent );
		m_dlgDecTime.Attach( *this, timer, timer.DecTimeEvent );
	}

	int GetTime( ) const { return m_nTime; }

private:

	static void OnAddTimeEvent( DigitalClock& clock, ClockTimer& timer, EventCaster& event )
	{
		printf( "%s:%d\n", __FUNCTION__, timer.GetTime( ));
		clock.m_nTime = timer.GetTime();
	}

	static void OnDecTimeEvent( DigitalClock& clock, ClockTimer& timer, EventCaster& event )
	{
		printf( "%s:%d\n", __FUNCTION__, timer.GetTime( ));
		clock.m_nTime = timer.GetTime();
	}

	Delegate< DigitalClock, ClockTimer, OnAddTimeEvent > m_dlgAddTime;
	Delegate< DigitalClock, ClockTimer, OnDecTimeEvent > m_dlgDecTime;

	int m_nTime;
};

int _tmain(int argc, _TCHAR* argv[])
{
	DigitalClock clock;

	{
		ClockTimer timer;
		clock.SetTimer( timer );
		timer.AddTime( 5 );
		timer.DecTime( 3 );
	}

	printf( "%s:%d\n", __FUNCTION__, clock.GetTime());

	return 0;
}

// Output:
// DigitalClock::OnAddTimeEvent:5 
// DigitalClock::OnDecTimeEvent:2
// _tmain:2
*/

#endif//__EVENTCASTER_H__

