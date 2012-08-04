// ----------------------------------------------------------------------- //
//
// MODULE  : EngineTimer.h
//
// PURPOSE : Definition/Implementation of the engine timer classes.
//
// CREATED : 4/6/04
//
// (c) 1996-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef _ENGINETIMER_H_
#define _ENGINETIMER_H_

#include "ilttimer.h"
#include "iltcsbase.h"
#include "iltmessage.h"
#include "CommonUtilities.h"

// ----------------------------------------------------------------------- //
//
// class EngineTimer
//
// Definition/Implementation of the EngineTimer class.  EngineTimer
// Wraps the HENGINETIMER and provides auto-release of the
// reference counted object.  It provides a base
// class for game system specific times, like player, interface
// and simulation.  By isolating these timers into
// separate classes, it reduces the likelihood that
// different timers will get compared to each other.
// Comparing different timers against each other won't
// make sense, since some may move faster than others.
//
// ----------------------------------------------------------------------- //
class EngineTimer
{
	// Ctors/dtor/operators
	public:

		EngineTimer( )
		{
			m_hTimer = NULL;
		}

		// Convert ctor from engine timer handle.
		EngineTimer( HENGINETIMER hTimer )
		{
			g_pLTBase->Timer()->AddTimerRef( hTimer );
			m_hTimer = hTimer;
		}

		// Convert ctor from object specific timer.  If object doesn't have a object specific
		// timer, the enginetimer will be invalid.
		EngineTimer( HOBJECT hObject )
		{
			m_hTimer = g_pLTBase->Timer( )->GetObjectTimer( hObject );
		}

		// Copy ctor.
		EngineTimer( EngineTimer const& other )
		{
			g_pLTBase->Timer()->AddTimerRef( other.m_hTimer );
			m_hTimer = other.m_hTimer;
		}

		// Destructor.  Releases reference to engine sdk's timer handle.
		virtual ~EngineTimer( )
		{
			if( !m_hTimer )
				return;

			g_pLTBase->Timer()->ReleaseTimer( m_hTimer );
			m_hTimer = NULL;
		}

		// Assignment operator.
		virtual EngineTimer& operator=( EngineTimer const& other )
		{
			if( this != &other )
			{
				this->~EngineTimer( );
				new( this ) EngineTimer( other );
			}
			return *this;
		}

		// Convert operator based on engine sdk's timer handle.
		virtual EngineTimer& operator=( HENGINETIMER hTimer )
		{
			// Don't add another refernce if we're already pointing at this timer.
			if( m_hTimer != hTimer )
			{
				this->~EngineTimer( );
				new( this ) EngineTimer( hTimer );
			}
			return *this;
		}

		// Convert operator based on object specific timer.  If object doesn't have a object specific
		// timer, the enginetimer will be invalid.
		virtual EngineTimer& operator=( HOBJECT hObject )
		{
			this->~EngineTimer( );
			new( this ) EngineTimer( hObject );
			return *this;
		}

	// Methods.
	public:

		// Access the underlying HENGINETIMER handle.  Direct use of this
		// handle should be avoided.  Use the EngineTimer interface to handle
		// most timer interactions.  
		void SetHandle( HENGINETIMER hTimer )
		{
			if( m_hTimer == hTimer )
				return;

			// Make sure we release any timer we had.
			if( m_hTimer )
			{
				g_pLTBase->Timer()->ReleaseTimer( m_hTimer );
				m_hTimer = NULL;
			}

			if( hTimer )
			{
				// Store the new timer.
				g_pLTBase->Timer()->AddTimerRef( hTimer );
				m_hTimer = hTimer;
			}
		}

		HENGINETIMER GetHandle( ) const
		{
			return m_hTimer;
		}

		// Check if the object has been initialized.
		bool IsValid( ) const
		{
			return ( GetHandle() != NULL );
		}

		// Set the object to use this as its timer.
		bool ApplyTimerToObject( HOBJECT hObject )
		{
			if( g_pLTBase->Timer( )->SetObjectTimer( hObject, m_hTimer ) != LT_OK )
				return false;
			return true;
		}

		// Remove the timer from the object.
		bool RemoveTimerFromObject( HOBJECT hObject )
		{
			if( g_pLTBase->Timer( )->SetObjectTimer( hObject, NULL ) != LT_OK )
				return false;
			return true;
		}

		// Creates a child timer based on an existing timer.
		bool CreateChildTimer( EngineTimer const& engineTimer )
		{
			// Make sure the other timer is valid.
			if( !engineTimer.IsValid( ))
				return false;

			// Make a new timer.
			HENGINETIMER hTimer = g_pLTBase->Timer( )->CreateTimer();
			if( !hTimer )
				return false;

			// Add it as a child.
			if( g_pLTBase->Timer( )->AddChildTimer( engineTimer.GetHandle(), hTimer ) != LT_OK )
			{
				g_pLTBase->Timer( )->ReleaseTimer( hTimer );
				hTimer = NULL;
				return false;
			}

			// Make it our own.
			SetHandle( hTimer );

			// Release refernce for this scoped variable.
			g_pLTBase->Timer( )->ReleaseTimer( hTimer );
			hTimer = NULL;

			return true;
		}

	// ILTTimer interfaces.
	public:

		//called to set the time scale associated with the timer. This is specified as an integer fraction,
		//so for 50%, 1/2 could be used, for 200% 2/1 can be used, and so on. This value will be used to
		//control the speed that systems are updated at. These provided fractional values should each be less
		//than 65k
		bool SetTimerTimeScale(uint32 nNumerator, uint32 nDenominator)
		{
			if( g_pLTBase->Timer( )->SetTimerTimeScale( m_hTimer, nNumerator, nDenominator ) != LT_OK )
			{
				LTERROR( "Invalid timer." );
				return false;
			}
			return true;
		}

		//called to get the time scale from the associated timer. If the timer is invalid, the default will
		//just be 0/1
		bool GetTimerTimeScale(uint32& nNumerator, uint32& nDenominator) const
		{
			if( g_pLTBase->Timer( )->GetTimerTimeScale( m_hTimer, nNumerator, nDenominator ) != LT_OK )
			{
				LTERROR( "Invalid timer." );
				return false;
			}
			return true;
		}

		//called to set the range of times that a single update can be within. This will clamp the update time
		//to be within this range. This is specified in integer milliseconds.
		bool SetTimerUpdateRange(uint32 nMinMS, uint32 nMaxMS)
		{
			if( g_pLTBase->Timer( )->SetTimerUpdateRange( m_hTimer, nMinMS, nMaxMS ) != LT_OK )
			{
				LTERROR( "Invalid timer." );
				return false;
			}
			return true;
		}

		//called to get the range from the timer. If the timer is invalid, the values will default to
		//both be zero.
		bool GetTimerUpdateRange(uint32& nMinMS, uint32& nMaxMS) const
		{
			if( g_pLTBase->Timer( )->GetTimerUpdateRange( m_hTimer, nMinMS, nMaxMS ) != LT_OK )
			{
				LTERROR( "Invalid timer." );
				return false;
			}
			return true;
		}

		//called to get the amount of time that has elapsed for this update interval for this timer in seconds.
		//If the timer is invalid, it will default the value to 0
		float GetTimerElapsedS( ) const
		{
			float fValue = 0.0f;
			if( g_pLTBase->Timer( )->GetTimerElapsedS( m_hTimer, fValue ) != LT_OK )
			{
				LTERROR( "Invalid timer." );
				return false;
			}
			return fValue;
		}

		//called to get the amount of time that has elapsed for this update interval for this timer in milliseconds.
		//If the timer is invalid, it will default the value to 0
		uint32 GetTimerElapsedMS( ) const
		{
			uint32 nValue = 0;
			if( g_pLTBase->Timer( )->GetTimerElapsedMS( m_hTimer, nValue ) != LT_OK )
			{
				LTERROR( "Invalid timer." );
				return false;
			}
			return nValue;
		}

		//called to get the length of time that this timer has been running in seconds.
		//If the timer is invalid, it will default the value to 0
		double GetTimerAccumulatedS( ) const
		{
			double fValue = 0.0f;
			if( g_pLTBase->Timer( )->GetTimerAccumulatedS( m_hTimer, fValue ) != LT_OK )
			{
				LTERROR( "Invalid timer." );
				return false;
			}
			return fValue;
		}

		//called to get the length of time that this timer has been running in milliseconds.
		//If the timer is invalid, it will default the value to 0
		uint32 GetTimerAccumulatedMS( ) const
		{
			uint32 nValue = 0;
			if( g_pLTBase->Timer( )->GetTimerAccumulatedMS( m_hTimer, nValue ) != LT_OK )
			{
				LTERROR( "Invalid timer." );
				return false;
			}
			return nValue;
		}

		//Pause/Unpause the timer.
		bool PauseTimer( bool bPause )
		{
			if( g_pLTBase->Timer( )->PauseTimer( m_hTimer, bPause ) != LT_OK )
			{
				LTERROR( "Invalid timer." );
				return false;
			}
			return true;
		}

		//Get the current pause state of the timer.
		bool IsTimerPaused( ) const
		{
			bool bPause = false;
			if( g_pLTBase->Timer( )->IsTimerPaused( m_hTimer, bPause ) != LT_OK )
			{
				LTERROR( "Invalid timer." );
			}
			return bPause;
		}

	// Fields
	private:

		// Engine sdk's timer handle.  This is properly referneced counted by this class.
		HENGINETIMER m_hTimer;
};

// ----------------------------------------------------------------------- //
//
// class SerializeableEngineTimer
//
// Allows serialization of EngineTimer.  This is a separate
// class because it incurs additional overhead in the ctor/dtors.
//
// ----------------------------------------------------------------------- //
class SerializeableEngineTimer : public EngineTimer
{
	// Ctors/dtor/operators
	public:

		SerializeableEngineTimer( )
		{
			CommonConstruct( );
		}

		// Convert ctor from engine timer handle.
		SerializeableEngineTimer( HENGINETIMER hTimer ) :
			EngineTimer( hTimer )
		{
			CommonConstruct( );
		}

		// Convert ctor from object specific timer.  If object doesn't have a object specific
		// timer, the enginetimer will be invalid.
		SerializeableEngineTimer( HOBJECT hObject ) :
			EngineTimer( hObject )
		{
			CommonConstruct( );
		}

		// Copy ctor.
		SerializeableEngineTimer( EngineTimer const& other ) :
			EngineTimer( other )
		{
			CommonConstruct( );
		}

		// Copy ctor.
		SerializeableEngineTimer( SerializeableEngineTimer const& other ) : 
			EngineTimer( other )
		{
			CommonConstruct( );
		}

		// Destructor.
		virtual ~SerializeableEngineTimer( )
		{
			RemoveFromList();
		}

		// Assignment operator.
		SerializeableEngineTimer& operator=( SerializeableEngineTimer const& other )
		{
			if( GetHandle( ) != other.GetHandle( ))
			{
				// Just set our handle.  
				// Don't touch the object's entry in the m_lstSerializeableEngineTimers,
				// since we're not going away.
				SetHandle( other.GetHandle( ));
			}
			return *this;
		}

		// Assignment operator.
		SerializeableEngineTimer& operator=( EngineTimer const& other )
		{
			if( GetHandle( ) != other.GetHandle( ))
			{
				// Just set our handle.  
				// Don't touch the object's entry in the m_lstSerializeableEngineTimers,
				// since we're not going away.
				SetHandle( other.GetHandle( ));
			}
			return *this;
		}

		// Convert operator based on engine sdk's timer handle.
		SerializeableEngineTimer& operator=( HENGINETIMER hTimer )
		{
			// Don't add another refernce if we're already pointing at this timer.
			if( GetHandle( ) != hTimer )
			{
				// Just set our handle.  
				// Don't touch the object's entry in the m_lstSerializeableEngineTimers,
				// since we're not going away.
				SetHandle( hTimer );
			}
			return *this;
		}

		// Convert operator based on object specific timer.  If object doesn't have a object specific
		// timer, the enginetimer will be invalid.
		SerializeableEngineTimer& operator=( HOBJECT hObject )
		{
			// Just set our handle.  
			// Don't touch the object's entry in the m_lstSerializeableEngineTimers,
			// since we're not going away.
			SetHandle( EngineTimer( hObject ).GetHandle( ));
			return *this;
		}


	// Methods.
	public:

		// Type of timer. Timers must be one of these
		// types to properly get saved/loaded.
		enum EngineTimerType
		{
			eEngineTimerType_Invalid,
			eEngineTimerType_SimulationTimer,
			eEngineTimerType_GameTimer,
			eEngineTimerType_InterfaceTimer,
			eEngineTimerType_ObjectTimer,
		};

// Saving of objecttimers only supported on server, since the engine
// only saves the timers on the server.
#ifdef _SERVERBUILD

		// Called when the server does a load of a save game.  It is necessary to run through 
		// all persistant timers and clear them out since the engine server creates
		// new timer objects internally.  All timers must be save/loaded manually to have them
		// persist across save/load.
		// This only needs to be done on the server since the engine only recreates
		// the timers on the server for a load.
		static void OnLoad( bool bKeepAliveLoad );

		// Serialize to a message.
		void Save(ILTMessage_Write& msg) const;

		// Deserialize from a message.
		virtual void Load(ILTMessage_Read& msg);

#endif // _SERVERBUILD

	// Private Methods.
	private:

		// Common constructor logic.
		void CommonConstruct( )
		{
			// Add us to the list of timers.
			AddToList( );
		}

// Saving of objecttimers only supported on server, since the engine
// only saves the timers on the server.
#ifdef _SERVERBUILD

		// A static list of timers is kept so that all persistent timers can
		// be rebound after load.  The engine recreates timers for a loaded game.
		typedef std::vector<SerializeableEngineTimer*> SerializeableEngineTimerList;
		// Accessor to the global ist of serializable engine timers.
		static SerializeableEngineTimerList& GetSerializeableEngineTimerList( );

#endif // _SERVERBUILD

		// Add the timer to the global list of timers.
		void AddToList( )
		{
#ifdef _SERVERBUILD
			SerializeableEngineTimerList& lst = GetSerializeableEngineTimerList();
			lst.push_back( this );
#endif // _SERVERBUILD
		}

		// Removes this instance from the global list of timers.
		void RemoveFromList( )
		{
#ifdef _SERVERBUILD
			SerializeableEngineTimerList& lst = GetSerializeableEngineTimerList();

			// Find this instance in the global list and remove it.
			SerializeableEngineTimerList::iterator iter = lst.begin();
			for( ; iter != lst.end( ); iter++ )
			{
				if( *iter == this )
				{
					lst.erase( iter );
					break;
				}
			}
#endif // _SERVERBUILD
		}

};

// ----------------------------------------------------------------------- //
//
// class RealTimeTimer
//
// Interface timer will run at the system time. All interface timing should
// use this single timer.  This is a singleton.
//
// ----------------------------------------------------------------------- //

class RealTimeTimer : public SerializeableEngineTimer
{
	public:

		NO_INLINE static RealTimeTimer& Instance( )
		{
			if (GetCurExecutionShellContext() == eExecutionShellContext_Client)
			{
				static RealTimeTimer instance;
				return instance;
			}
			else
			{
				static RealTimeTimer instance;
				return instance;
			}
		}

		// Deserialize from a message.
		virtual void Load(ILTMessage_Read& msg)
		{
			EngineTimerType eEngineTimerType = ( EngineTimerType )msg.Readuint8( );
			switch( eEngineTimerType )
			{
				case eEngineTimerType_InterfaceTimer:
					Bind( );
					break;
				default:
					LTERROR( "Invalid engine timer type." );
					break;
			}
		}

	private:

		RealTimeTimer( )
		{
			Bind( );
		}

		~RealTimeTimer( )
		{
		}

		// Binds the handle to the engine's timer.
		void Bind( )
		{
			// Create a timer based on the system timer.
			HENGINETIMER hTimer = g_pLTBase->Timer( )->CreateTimer( );
			g_pLTBase->Timer()->AddSystemChildTimer( hTimer );
			SetHandle( hTimer );
			g_pLTBase->Timer( )->ReleaseTimer( hTimer );
		}

		PREVENT_OBJECT_COPYING( RealTimeTimer );
};


// ----------------------------------------------------------------------- //
//
// class GameTimeTimer
//
// This timer will run at the system time, but will be paused by the game.
// This is a singleton.
//
// ----------------------------------------------------------------------- //

class GameTimeTimer : public SerializeableEngineTimer
{
public:

	NO_INLINE static GameTimeTimer& Instance( )
	{
		if (GetCurExecutionShellContext() == eExecutionShellContext_Client)
		{
			static GameTimeTimer instance;
			return instance;
		}
		else
		{
			static GameTimeTimer instance;
			return instance;
		}
	}

	// Deserialize from a message.
	virtual void Load(ILTMessage_Read& msg)
	{
		EngineTimerType eEngineTimerType = ( EngineTimerType )msg.Readuint8( );
		switch( eEngineTimerType )
		{
		case eEngineTimerType_GameTimer:
			Bind( );
			break;
		default:
			LTERROR( "Invalid engine timer type." );
			break;
		}
	}

private:

	GameTimeTimer( )
	{
		Bind( );
	}

	~GameTimeTimer( )
	{
	}

	// Binds the handle to the engine's timer.
	void Bind( )
	{
		// Create a timer based on the system timer.
		HENGINETIMER hTimer = g_pLTBase->Timer( )->CreateTimer( );
		g_pLTBase->Timer()->AddSystemChildTimer( hTimer );
		SetHandle( hTimer );
		g_pLTBase->Timer( )->ReleaseTimer( hTimer );
	}

	PREVENT_OBJECT_COPYING( GameTimeTimer );
};

// ----------------------------------------------------------------------- //
//
// class SimulationTimer
//
// Simulation timer allows access to the simulation time.  All systems
// should use this timer to access the simulation time.  This is a singleton.
//
// ----------------------------------------------------------------------- //

class SimulationTimer : public SerializeableEngineTimer
{
	public:

		enum
		{
			kMaxFrameDeltaMs	= 100,
		};

	public:

		NO_INLINE static SimulationTimer& Instance( )
		{
			if (GetCurExecutionShellContext() == eExecutionShellContext_Client)
			{
				static SimulationTimer instance;
				return instance;
			}
			else
			{
				static SimulationTimer instance;
				return instance;
			}
		}

		// Deserialize from a message.
		virtual void Load(ILTMessage_Read& msg)
		{
			EngineTimerType eEngineTimerType = ( EngineTimerType )msg.Readuint8( );
			switch( eEngineTimerType )
			{
				case eEngineTimerType_SimulationTimer:
					Bind( );
					break;
				default:
					LTERROR( "Invalid engine timer type." );
					break;
			}
		}

	private:

		SimulationTimer( )
		{
			Bind( );
		}

		~SimulationTimer( )
		{
		}

		// Binds the handle to the engine's timer.
		void Bind( )
		{
			// Setup reference to simulation timer.
			HENGINETIMER hTimer = g_pLTBase->Timer( )->GetSimulationTimer( );
			SetHandle( hTimer );
			g_pLTBase->Timer()->SetTimerUpdateRange( hTimer, 0, kMaxFrameDeltaMs );
			g_pLTBase->Timer( )->ReleaseTimer( hTimer );
			hTimer = NULL;
		}

		PREVENT_OBJECT_COPYING( SimulationTimer );
};

// ----------------------------------------------------------------------- //
//
// class ObjectContextTimer
//
// This timer will ask the object for its specific timer if it has one.
// If it doesn't then this object will use the simulation timer.  Because
// this can represent the simulation timer, care must be used to not
// accidentally modify the simulation timer when an object timer was
// expected.
//
// ----------------------------------------------------------------------- //

class ObjectContextTimer : public EngineTimer
{
	public:

		ObjectContextTimer( HOBJECT hObject ) :
			EngineTimer( hObject )
		{
			// Check if the convert constructor for EngineTimer
			// was able to find a object specific timer.  If not,
			// then use the simulation timer.
			if( !GetHandle( ))
				SetHandle( SimulationTimer::Instance( ).GetHandle());
		}

		// Checks if the timer used by the object is the simulation timer.
		bool IsSimulationTimer( ) const
		{
			return ( GetHandle() == SimulationTimer::Instance().GetHandle( ));
		}
};



// ----------------------------------------------------------------------- //
//
// class StopWatchTimer
//
// Used for keeping track of elapsed time.  
// This uses template inheritance in order to help prevent different
// types of timers from being compared againsts each other.
//
// ----------------------------------------------------------------------- //

class StopWatchTimer
{
	// Ctors/dtor/operators.
	public:

		StopWatchTimer()
		{ 
			CommonConstruct( );
		}

		StopWatchTimer( EngineTimer& engineTimer )
		{
			CommonConstruct( );
			m_EngineTimer = engineTimer;
		}

	// Methods.
	public:

		// Accessor to the EngineTimer used.
		void SetEngineTimer( EngineTimer const& engineTimer )
		{
			// Receiving a new timer will throw off the calculations of any currently
			// started timers, so just stop them now...
			Stop( );
			m_EngineTimer = engineTimer;
		}
		EngineTimer const& GetEngineTimer( ) const
		{
			return m_EngineTimer;
		}

		// Start the timer with a countdown duration.
		void Start( double fCountDownDuration )
		{ 
			m_fDuration = fCountDownDuration; 
			m_fTimeOfPause = 0; 
			m_fStartTime = m_EngineTimer.GetTimerAccumulatedS( );
			m_bTimerIsOn = true;
			m_bTimerIsPaused = false;
		}

		// Start the timer with no countdown duration.
		void Start( ) 
		{ 
			Start( 0.0f );
		}

		// Stop the countdown.
		void Stop( ) 
		{ 
			m_fDuration = 0; 
			m_fTimeOfPause = 0; 
			m_bTimerIsOn = false;
			m_bTimerIsPaused = false;
		}

		// Pause the countdown duration.
        void Pause() 
		{ 
			// Check if already paused.
			if( IsPaused( ))
				return;

			// Record the time of the pause so we can adjust the absolute
			// time of our duration.
			m_fTimeOfPause = m_EngineTimer.GetTimerAccumulatedS( );
			m_bTimerIsPaused = true;
		}

		// Resume from a previous pause.
		void Resume()
		{
			// Make sure we're already paused.
			if( !IsPaused( ))
				return;

			// Adjust the start time to account for paused time.
			double fTimeFromPause = m_EngineTimer.GetTimerAccumulatedS() - m_fTimeOfPause;
			m_fStartTime += fTimeFromPause;
			m_fTimeOfPause = 0;
			m_bTimerIsPaused = false;
		}

		// Check if our countdown timed out.
		bool IsTimedOut( ) const
		{ 
			if( !IsStarted( ))
				return true;

			if( IsPaused( ))
				return ( m_fTimeOfPause - m_fStartTime >= m_fDuration) ? true : false; 

			return ( m_EngineTimer.GetTimerAccumulatedS( ) - m_fStartTime >= m_fDuration) ? true : false; 
		}

		// Check if we have the timer paused.
		bool IsPaused( ) const 
		{
			return m_bTimerIsPaused;
		}

		// Check if the timer is started.
		bool IsStarted( ) const
		{ 
			return m_bTimerIsOn; 
		}

		// Get time since timer was started.
		double GetElapseTime( ) const
		{
			if( IsPaused( ))
				return m_fTimeOfPause - m_fStartTime;

			return (m_EngineTimer.GetTimerAccumulatedS( ) - m_fStartTime);
		}

		// Add time to a countdown duration.
		void AddDuration( float fAdditialTime ) 
		{ 
			m_fDuration += fAdditialTime; 
			m_fDuration = ( m_fDuration < 0.0f ? 0 : m_fDuration);
		}

		// Get the time left.
		double GetTimeLeft() const
		{
			if( IsTimedOut( ))
				return 0.0f;

			if( IsPaused( ))
				return (m_fStartTime + m_fDuration - m_fTimeOfPause);

			return (m_fStartTime + m_fDuration - m_EngineTimer.GetTimerAccumulatedS());
		}

		// Get the amount of time we are counting down.
		double GetDuration() const
		{ 
			return m_fDuration; 
		}

// Saving of objecttimers only supported on server, since the engine
// only saves the timers on the server.
#ifdef _SERVERBUILD
		// Serialize to a message.
		void Save(ILTMessage_Write& msg) const
		{
			m_EngineTimer.Save( msg );
			msg.Writedouble( m_fStartTime - m_EngineTimer.GetTimerAccumulatedS( ));
			msg.Writedouble( m_fTimeOfPause - m_EngineTimer.GetTimerAccumulatedS( ));
			msg.Writedouble( m_fDuration );
			msg.Writebool( m_bTimerIsOn );
			msg.Writebool( m_bTimerIsPaused );
		}

		// Deserialize from a message.
		void Load(ILTMessage_Read& msg)
		{
			m_EngineTimer.Load( msg );
			m_fStartTime = m_EngineTimer.GetTimerAccumulatedS( ) + msg.Readdouble( );
			m_fTimeOfPause = m_EngineTimer.GetTimerAccumulatedS( ) + msg.Readdouble( );
			m_fDuration = msg.Readdouble( );
			m_bTimerIsOn = msg.Readbool( );
			m_bTimerIsPaused = msg.Readbool( );
		}
#endif // _SERVERBUILD

	private:

		// Clears data structures for ctors.
		void CommonConstruct( )
		{ 
			m_fStartTime = 0; 
			m_fDuration = 0; 
			m_fTimeOfPause = 0; 
			m_bTimerIsOn = false;
			m_bTimerIsPaused = false;
		}

	private:

		// Absolute time the timer was started.
		double m_fStartTime;

		// Duration to countdown.  0 means not counting down.
		double m_fDuration;

		// Records time of pause pause time can be removed from calculations.
		double m_fTimeOfPause;

		// Flag to indicate timer has been paused...
		bool m_bTimerIsPaused;

		// Flag to indicate timer has been started.
		bool  m_bTimerIsOn;

		// Engine timer to use.
		SerializeableEngineTimer m_EngineTimer;
};

#endif
