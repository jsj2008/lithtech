// ----------------------------------------------------------------------- //
//
// MODULE  : ObjectTransformHistory.h
//
// PURPOSE : ObjectTransformHistory is used to cache an object's transform on a specific
//			 interval up to a specific amount of time in the past.  Transforms can then 
//			 be retrieved from a specified time stamp.
//
// CREATED : 01/21/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __OBJECT_TRANSFORM_HISTORY_H__
#define __OBJECT_TRANSFORM_HISTORY_H__


class CObjectTransformHistory
{
	friend class CObjectTransformHistoryMgr;

	public:  // Methods...

		CObjectTransformHistory( );
		~CObjectTransformHistory( );

		// Initialize the transform history with the specified object...
		bool Init( HOBJECT hObject );

		// Clear the transform history
		void Term();
		
		// Retrieve the interpolated historical transform the specified number of milliseconds in the past...
		// Returns false and sets the transform to the identity if failed to properly calculate the transform...
		bool GetTransform( uint32 nTimeMSInPast, LTRigidTransform &rTransform );


	private: // Methods...

		// Add a new event to the history...
		void LogEvent( );


	private: // Members...

		// Object to keep a history of...
		LTObjRefNotifier m_hObject;

		class CEvent
		{
			public: // Methods...

				CEvent( )
				:	m_Transform	( ),
					m_nTimeMS	( 0 )
				{ }

				LTRigidTransform m_Transform;

				// Time stamp, in milliseconds, for this event...
				uint32 m_nTimeMS;
		};

		// Array of transform events
		typedef std::vector<CEvent, LTAllocator<CEvent, LT_MEM_TYPE_OBJECTSHELL> >	TEventArray;
	
		TEventArray m_aEvents;

		// Keep track of where in the array the most recent transform event was logged...
		uint32 m_nMostRecentEvent;
};


class CObjectTransformHistoryMgr : public ILTObjRefReceiver
{
	DECLARE_SINGLETON( CObjectTransformHistoryMgr );

	public: // Methods...

		// Check update timer and log events when timer has elapsed...
		void Update( );

		// Add an object to track and log transform events to the history...
		bool AddObject( HOBJECT hObject );

		// Retrieve the interpolated historical transform of the specified object
		// the specified number of milliseconds in the past...
		// Returns false and sets the transform to the identity if failed to properly calculate the transform
		// or if the object was never specified to log events...
		void GetHistoricalTransform( HOBJECT hObject, uint32 nTimeMSInPast, LTRigidTransform &rTransform );

		// If an object goes invalid remove it's transform history and stop logging events for it...         
		void OnLinkBroken( LTObjRefNotifier *pRef, HOBJECT hObj );

		// Retrieves the next object tracked in the list of transforms histories...
		// Returns NULL if list is empty or there is no next object...
		// Pass in NULL to get the first object.
		// Use this to iterate through the objects to get each ones transform...
		HOBJECT GetNextTrackedObject( HOBJECT hObject );

	private: // Methods...

		// Initialize...
        void Init( );


	private: // Members...

		// Real-time timer for when the next event should be logged...
		StopWatchTimer m_LogTimer;

		// Flag to determine if the manager has been initialized...
		bool m_bInitialized;

		// Array of object transform histories to manage...
		typedef std::vector<CObjectTransformHistory*, LTAllocator<CObjectTransformHistory*, LT_MEM_TYPE_OBJECTSHELL> > TObjectHistoryArray;
		
		// Use an allocation array to avoid as many allocations as possible...
		TObjectHistoryArray m_aObjectHistoryPool;
		TObjectHistoryArray m_aObjectHistory;
};

#endif // __OBJECT_TRANSFORM_HISTORY_H__

// EOF
