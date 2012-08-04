// ----------------------------------------------------------------------- //
//
// MODULE  : ControlPoint.h
//
// PURPOSE : ControlPoint object to place in Control Point level.
//
// CREATED : 02/09/06
//
// (c) 2006 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __CONTROLPOINT_H__
#define __CONTROLPOINT_H__

//
// Includes...
//

#include "GameBase.h"

LINKTO_MODULE( ControlPoint );

class ControlPointStateMachine;

class ControlPoint : public GameBase
{
	public: // Methods...

		DEFINE_CAST( ControlPoint );

		ControlPoint( );
		virtual ~ControlPoint( );

		// Called by the touch monitor object.
		void OnTouched( HOBJECT hToucher );

		// Gets the zonedims for the object.
		LTVector const& GetZoneDims( ) const { return m_vZoneDims; }

		// List of existing ControlPoint objects.
		typedef std::vector<ControlPoint*> TControlPointList;
		TControlPointList& GetControlPointList( ) const { return m_lstControlPoints; }

		// Check if conquest win achieved.
		static void CheckForConquestWin( );

		// Prefetch resources for this object.
		static void GetPrefetchResourceList(const char* pszObjectName, IObjectResourceGatherer* pInterface, ResourceList& Resources );

	protected: // Methods... 

		// Handle messages from the engine...
		uint32 EngineMessageFn( uint32 messageID, void *pData, float fData );

		// Send relevant information to clients...
		void CreateSpecialFX( bool bUpdateClients );

	private: // Methods...

		// Handle a MID_INITIALUPDATE message from the engine....
		void InitialUpdate( );

		// Read in the properties of the object... 
		bool ReadProp( const GenericPropList *pProps );
		
		// Update the create struct of the object
		bool PostReadProp( ObjectCreateStruct *pStruct );

		// Per-frame update.
		void Update( );

		// Creates a touch monitor object to receive MID_TOUCHNOTIFY's.
		bool CreateTouchMonitor( );

	protected: // Members...

		// Handles state machine.
		friend class ControlPointStateMachine;
		ControlPointStateMachine* m_pControlPointStateMachine;

		// Special object that relays touch notifies to the ControlPoint.
		LTObjRef m_hTouchMonitor;

		// CPTypes record to use.
		HRECORD m_hControlPointRec;

		// ZoneDims for object.
		LTVector m_vZoneDims;

		// Teleports the object to the floor once it is created...
		bool m_bMoveToFloor;

		// The team to use when starting.
		uint8 m_nInitialTeamId;

		// Tracks which control point id's have been used.
		static uint8 m_nAllocatedControlPointIds;

		// ControlPoint id
		uint16 m_nControlPointId;

		// Command sent if Team0 neutralizes control of the control point.
		char const* m_pszTeam0NeutralizedControlCmd;
		// Command sent if Team1 neutralizes control of the control point.
		char const* m_pszTeam1NeutralizedControlCmd;
		// Command sent if Team0 captures control of the control point.
		char const* m_pszTeam0CapturedControlCmd;
		// Command sent if Team1 captures control of the control point.
		char const* m_pszTeam1CapturedControlCmd;

		// List of existing control point objects.
		static TControlPointList m_lstControlPoints;
};

#endif // __CONTROLPOINT_H__
