// **************************************************************************** //
//
//	Project:	The Dark
//	File:		objectdetector.h
//
//	Purpose:	This class manages the tracking of a single object instance.
//
//	Copyright (C) 2004 (Monolith Productions)
//
// **************************************************************************** //

#ifndef __OBJECTDETECTOR_H__
#define __OBJECTDETECTOR_H__

// **************************************************************************** //
// Forward declarations

class ObjectDetector;


// **************************************************************************** //
// The ObjectDetectorLink is a registry link for an individual object, per
// ObjectDetector instance.  So if you want an object to be accessible by a
// detector, its management class should contain a link instance per detector
// that it should register with.
//
// For instance, if you have an SFX class for a Character... a link could exist
// in that class so that each instance of it could register with a detector. If
// the object managed by the Character class needed to be part of two detectors,
// then two link instances should be contained in the Character class and
// registered to the independent detectors.
//
// If the object reference becomes invalid, the link will release itself from
// the registered detector class upon the next attempt at using the link.
// Otherwise, it will also release itself upon destruction of the link.

class ObjectDetectorLink
{
	public:

		ObjectDetectorLink();
		~ObjectDetectorLink();

		bool		IsRegistered()		{ return ( m_pDetector != NULL ); }

		HOBJECT		GetObject()			{ return m_hObject; }
		void*		GetUserData()		{ return m_pUserData; }


	protected:

		LTObjRef				m_hObject;
		ObjectDetector*			m_pDetector;
		ObjectDetectorLink*		m_pPrev;
		ObjectDetectorLink*		m_pNext;
		void*					m_pUserData;

		friend ObjectDetector;
};


// **************************************************************************** //
// Common types

// Function declaration for overriding the line of site testing behavior.
// NOTE: The common behavior is to ignore all objects from the source to the test
// object, and only intersect against world geometry.  If nothing is interested,
// the test will pass and the object is detected.
typedef bool ( *ObjectDetectorLOSFn )( const LTVector& vSourcePos, ObjectDetectorLink* pLink, void* pUserData );

// Function declaration for overriding the position and dims that get used for object
// testing.  By default, it uses the center point of the object and engine set dims, but
// using this override would allow you to use a socket or node location... or anything
// else relative to the object, and some other dims size.
typedef void ( *ObjectDetectorSpatialFn )( ObjectDetectorLink* pLink, LTVector& vPos, LTVector& vDims, void* pUserData );

// Function declaration for adding a custom weighting test to acquire and verify
// tests.  You should return a value from 0.0 to 1.0 for fMetRR, where 0.0 is the best
// weighting and 1.0 is the worst.  Return a boolean for pass/fail.
typedef bool ( *ObjectDetectorCustomTestFn )( ObjectDetectorLink* pLink, float& fMetRR, void* pUserData );


// **************************************************************************** //
// An instance of an ObjectDetector is meant to track a single object at a time.
// In order to detect multiples, more than one instance of the ObjectDetector
// class would be used... and the user flag would need to be set in order to
// avoid overlapping detection.  NOTE: The user flag specified will be controlled
// internally by the ObjectDetector, so it's not recommended that you use a flag
// that is queried by other interfaces paying attention to that object.  However,
// it could be looked at to see if an object is currently acquired.

class ObjectDetector
{
	public:

		// ---------------------------------------------------------------------------- //
		// Local data types

		enum BehaviorFlags
		{
			// General options
			ODBF_INCLUSIVEACQUIRE			= 0x00000001,	// All used aquire modes need to pass for any given object
			ODBF_INCLUSIVEVERIFY			= 0x00000002,	// All used verify states need to pass for any given object

			// The number of tests that the flags below represent
			ODBF_TESTSAVAILABLE				= 7,

			// Acquire modes (if none of these flags are set, no object will ever be acquired).
			// Acquiring happens upon a call to the AcquireObject() function.  If you're re-acquiring
			// every frame, there's no need to call the Update() function or set any of the VERIFY flags.

			ODBF_ACQUIRELINEOFSITE			= 0x00010000,	// Will use intersection tests from the source position
			ODBF_ACQUIREFORWARD				= 0x00020000,	// Acquires along a vector in the forward direction
			ODBF_ACQUIREDIRECTION			= 0x00040000,	// Acquires along a vector in the specified direction
			ODBF_ACQUIREFOV					= 0x00080000,	// Acquires within an oriented frustum
			ODBF_ACQUIRESPHERE				= 0x00100000,	// Acquires within a sphere
			ODBF_ACQUIRECYLINDER			= 0x00200000,	// Acquires within an oriented cylinder (height going along the Y rotation axis)
			ODBF_ACQUIRECUSTOM				= 0x00400000,	// Acquires based on a custom test specified through a callback
			ODBF_ACQUIREMASK				= 0x00FF0000,

			// Verification states (if none of these flags are set, an object will remain tracked until it
			// is manually changed, or the object gets removed).  The verification happens during a call
			// to the Update() function.  If Update() is never called, it'll be the same effect as not
			// having any of the VERIFY flags set.

			ODBF_VERIFYLINEOFSITE			= 0x01000000,
			ODBF_VERIFYFORWARD				= 0x02000000,
			ODBF_VERIFYDIRECTION			= 0x04000000,
			ODBF_VERIFYFOV					= 0x08000000,
			ODBF_VERIFYSPHERE				= 0x10000000,
			ODBF_VERIFYCYLINDER				= 0x20000000,
			ODBF_VERIFYCUSTOM				= 0x40000000,
			ODBF_VERIFYMASK					= 0xFF000000,

			ODBF_FORCE32BIT					= 0x7FFFFFFF,

			// EXAMPLE:  If you wanted to acquire an object from the center of the player camera based on a button press,
			// and hang onto that object as long as it is within a radius of the camera... you would setup a Detector
			// using the ACQUIREFORWARD | VERIFYSPHERE flags and set the appropriate Params.  Then you would call AcquireObject()
			// when the button is pressed, and SetTransform() and Update() per frame... using the cameras transform.
		};


	public:

		// ---------------------------------------------------------------------------- //
		// Construction and destruction

		ObjectDetector();
		virtual ~ObjectDetector();


		// ---------------------------------------------------------------------------- //
		// Object registry control

		void		RegisterObject( ObjectDetectorLink& iLink, HOBJECT hObject, void* pUserData );
		static void	ReleaseLink( ObjectDetectorLink& iLink );

		uint32		GetNumRegisteredObjects();


		// ---------------------------------------------------------------------------- //
		// Detection source control

		// The transform of the source for the detector.
		void		SetTransform( HOBJECT hObject );
		void		SetTransform( const LTRigidTransform& tTrans );
		void		SetTransform( const LTVector& vPos, const LTRotation& rRot );

		// Parameters that will be used for both acquiring and verifying.
		void		SetParamsForward( float fDistance );
		void		SetParamsDirection( const LTVector& vDirection );
		void		SetParamsFOV( float fXDegrees, float fYDegrees, float fNearDist, float fFarDist, float fDistWeight, float fXWeight, float fYWeight );
		void		SetParamsSphere( float fInnerRadius,  float fOuterRadius );
		void		SetParamsCylinder( float fInnerRadius,  float fOuterRadius, float fHalfHeight );


		// ---------------------------------------------------------------------------- //
		// Detection state control

		// Behavior settings.
		void		SetBehaviorFlags( uint32 nFlags );
		void		SetVerifyFailureDelay( uint32 nVerifyFlags, float fSeconds );

		// Sets a user flag that will be managed internally on detected objects... this makes it
		// so you can avoid overlaps in case one object is affected by multiple detectors.
		void		SetUserFlagVerification( uint32 nFlag );

		// Override default behaviors...
		void		SetLineOfSiteFn( ObjectDetectorLOSFn pFn, void* pUserData );
		void		SetSpatialFn( ObjectDetectorSpatialFn pFn, void* pUserData );
		void		SetCustomTestFn( ObjectDetectorCustomTestFn pFn, void* pUserData );


		// ---------------------------------------------------------------------------- //
		// Updating

		// This should be called every frame for an active targeting system.
		void		Update( float fFrameTime );

		// A forced update of the spatial object data, in case you want to avoid other
		// updating behaviors.
		void		UpdateObjectSpatialData();


		// ---------------------------------------------------------------------------- //
		// Target management

		// Most acquire behaviors work outward from the center towards the edge of the
		// source parameters with each consecutive call, using the previous target as
		// the basis for expansion. If you don't want it to expand with each consecutive
		// use, just pass in false.
		HOBJECT		AcquireObject( bool bFromPrevious = true );

		// Gets the currently acquired object.
		HOBJECT		GetObject();

		// Gets the user data associated with the currently acquired object.
		void*		GetObjectUserData();

		// Gets the currently acquired object's spatial data
		LTVector	GetObjectSpatialPosition();
		LTVector	GetObjectSpatialDimensions();

		// Manual control of setting the tracked object. It will return 'false' if the
		// object was not found in the registered list.
		bool		SetObject( HOBJECT hObject );

		// Manual control of setting the tracked object. It will return 'false' if the
		// link was not registered with this detector.
		bool		SetLink( ObjectDetectorLink* pLink );

		// Manual control of clearing out the currently tracked object.
		void		ClearObject( float fDelay = 0.0f );

		// Returns true if we're in the middle of a delay for clearing the object
		bool		ClearingObject();


	protected:

		// ---------------------------------------------------------------------------- //
		// Requirement range calculators

		bool		TestLineOfSite( ObjectDetectorLink* pLink );
		void		GetObjectSpatialData( ObjectDetectorLink* pLink, LTVector& vPos, LTVector& vDims );

		bool		TestParamsForward( ObjectDetectorLink* pLink, LTVector& vPos, LTVector& vDims, float& fMetRR );
		bool		TestParamsDirection( ObjectDetectorLink* pLink, LTVector& vPos, LTVector& vDims, float& fMetRR );
		bool		TestParamsFOV( ObjectDetectorLink* pLink, LTVector& vPos, LTVector& vDims, float& fMetRR );
		bool		TestParamsSphere( ObjectDetectorLink* pLink, LTVector& vPos, LTVector& vDims, float& fMetRR );
		bool		TestParamsCylinder( ObjectDetectorLink* pLink, LTVector& vPos, LTVector& vDims, float& fMetRR );
		bool		TestParamsCustom( ObjectDetectorLink* pLink, float& fMetRR );


	protected:

		// ---------------------------------------------------------------------------- //
		// Externally controlled member variables

		// Tracking source information
		LTRigidTransform	m_tTransform;

		// Properties of the acquire/tracking behaviors
		float				m_fParamsForward;
		LTVector			m_vParamsDirection;
		LTVector4			m_vParamsFOV;
		LTVector			m_vParamsFOVWeights;
		LTVector2			m_vParamsSphere;
		LTVector			m_vParamsCylinder;

		// Determines what sort of tests to verify for this detector
		uint32				m_nBehaviorFlags;

		// Allows for non-overlapping object detection across multiple detectors
		uint32				m_nUserFlagVerification;

		// Time delay values for failure of all verification behaviors
		float				m_fVerifyFailureDelays[ ODBF_TESTSAVAILABLE ];

		// The override line of site function
		ObjectDetectorLOSFn				m_pLineOfSiteFn;
		void*							m_pLineOfSiteUserData;
		ObjectDetectorSpatialFn			m_pSpatialFn;
		void*							m_pSpatialUserData;
		ObjectDetectorCustomTestFn		m_pCustomTestFn;
		void*							m_pCustomTestUserData;


		// ---------------------------------------------------------------------------- //
		// Registry variables

		ObjectDetectorLink	m_iRootLink;
		uint32				m_nRegisteredObjects;


		// ---------------------------------------------------------------------------- //
		// Object tracking variables

		ObjectDetectorLink*	m_pTrackedLink;

		LTVector			m_vTrackedSpatialPosition;
		LTVector			m_vTrackedSpatialDimensions;


		// ---------------------------------------------------------------------------- //
		// Update variables that are managed internally

		float				m_fActiveVerifyFailureDelays[ ODBF_TESTSAVAILABLE ];
		float				m_fActiveClearDelay;
};

// **************************************************************************** //

#endif//__TARGETINGSYSTEM_H__

