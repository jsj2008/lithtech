// ----------------------------------------------------------------------- //
//
// MODULE  : RadarObject.h
//
// PURPOSE : The RadarObject object
//
// CREATED : 6/6/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#ifndef __RADAR_OBJECT_H__
#define __RADAR_OBJECT_H__

//
// Include...
//
	
	#include "GameBase.h"
	#include "RadarTypeMgr.h"
	#include "SharedFXStructs.h"

LINKTO_MODULE( RadarObject );


class RadarObject : public GameBase
{
	public:	// Methods...

		RadarObject();
		~RadarObject();

		
	protected: // Methods...

		uint32	EngineMessageFn( uint32 messageID, void *pData, LTFLOAT fData );
		bool	OnTrigger( HOBJECT hSender, const CParsedMsg &cMsg );

		void	Update();
		void	AssignTarget();
		
		bool	OnSave( ILTMessage_Write *pMsg ); 
		bool	OnLoad( ILTMessage_Read *pMsg );
		
		void	ReadProps( ObjectCreateStruct *pOCS );
		void	ObjectCreated( );
		void	CreateSpecialFX( bool bUpdateClients = false );

		void	SetTeamId( uint8 nTeamId );


	protected: // Members...

		RADAROBJCREATESTRUCT	m_ROCS;
		
		std::string		m_sTargetName;
		LTObjRef		m_hTarget;
		uint8			m_nTeamId;
};

class CRadarObjectPlugin : public IObjectPlugin
{
	public: // Methods...
		
	virtual LTRESULT PreHook_EditStringList(
			const char* szRezPath,
			const char* szPropName,
			char** aszStrings,
			uint32* pcStrings,
			const uint32 cMaxStrings,
			const uint32 cMaxStringLength);


	protected: // Members...

		CRadarTypeMgrPlugin m_RadarTypeMgrPlugin;
};

#endif // __RADAR_OBJECT_H__