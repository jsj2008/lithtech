// ----------------------------------------------------------------------- //
//
// MODULE  : RefillStation.h
//
// PURPOSE : Creates a designated area where players can refill their inventory supply...
//
// CREATED : 09/29/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __REFILL_STATION_H__
#define __REFILL_STATION_H__

//
// Includes...
//

#include "GameBase.h"

LINKTO_MODULE( RefillStation )

class RefillStation : public GameBase
{
	public: // Methods...

		RefillStation( );
		~RefillStation( );

		static void GetPrefetchResourceList(const char* pszObjectName, IObjectResourceGatherer* pInterface, ResourceList& Resources );

	protected: // Methods...

		// Handle messages from the engine...
		uint32 EngineMessageFn( uint32 messageID, void *pData, float fData );

		// Read in the properties of the object... 
		bool ReadProp( const GenericPropList *pProps );

		// Update the create struct of the object andother data after obtaining the property values...
		void PostReadProp( ObjectCreateStruct *pOCS );

		// Handle any adjustments to the object after it has been created...
		void ObjectCreated( const GenericPropList *pProps );

		// Handle an object touching the dims of the station...
		void ObjectTouch( HOBJECT hObject );

		// Handle an update for the refill station...
		void Update( );

		// Save the object...
		void Save( ILTMessage_Write *pMsg, uint32 dwSaveFlags );

		// Load the object...
		void Load( ILTMessage_Read *pMsg, uint32 dwLoadFlags );

		// Send the player the refill data and update inventory accordingly...
		bool UpdatePlayerRefill( CPlayerObj *pPlayer );

		// Test to see if the RefillStation has any inventory left...
		bool IsEmpty( );


	private: // Members...

		enum Constants
		{
			kRefillInfinite = INT_MIN,
		};

		// Record of the refill station	used...
		HRECORD m_hRefillStation;

		// Total amount of health in the RefillStations' inventory...
		int32 m_iHealth;

		// Total amount of armor in the RefillStations' inventory...
		int32 m_iArmor;

		// Total amount of Slo-Mo time in the RefillStations' inventory...
		int32 m_iSlowMo;

		typedef std::vector<int32, LTAllocator<int32, LT_MEM_TYPE_GAMECODE> > int32Array;
		int32Array m_vecAmmo;

		// Flag saying the refill station is currently distributing inventory...
		bool m_bUpdating;

		HLTSOUND m_hRefillSound;

		// Handle to the visual model object created...
		LTObjRef m_hModelObject;
};


class RefillStationPlugin : public IObjectPlugin
{
	public: // Methods...

		virtual LTRESULT PreHook_EditStringList( 
			const char *szRezPath,
			const char *szPropName,
			char **aszStrings,
			uint32 *pcStrings,
			const uint32 cMaxStrings,
			const uint32 cMaxStringLen );

		virtual LTRESULT PreHook_Dims(
			const char* szRezPath,
			const char* szPropName, 
			const char* szPropValue,
			char* szModelFilenameBuf,
			int	  nModelFilenameBufLen,
			LTVector & vDims,
			const char* pszObjName, 
			ILTPreInterface *pInterface);
};


#endif // __REFILL_STATION_H__
