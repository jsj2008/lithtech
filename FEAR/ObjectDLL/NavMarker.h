// ----------------------------------------------------------------------- //
//
// MODULE  : NavMarker.h
//
// PURPOSE : definition of NavMarker object
//
// CREATED : 11/05/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //
#ifndef __NAVMARKER_H__
#define __NAVMARKER_H__

#include "GameBase.h"
#include "ClientServerShared.h"
#include "CommandMgr.h"
#include "GameDatabaseMgr.h"
#include "StringEditMgrPlugin.h"
#include "istringeditmgr.h"

LINKTO_MODULE( NavMarker );

class NavMarker;

class NavMarkerCreator
{
public:
	NavMarkerCreator() : m_hTarget(NULL),m_hType(NULL),m_nTeamId(0),m_bBroadcast(false), m_vPos(0,0,0), 
		m_bInstant(false), m_bIsActive(true), m_nStringId(INVALID_STRINGEDIT_INDEX) {}
	virtual ~NavMarkerCreator() {}
	LTObjRef	m_hTarget;
	HRECORD		m_hType;
	uint8		m_nTeamId;
	LTVector	m_vPos;
	bool		m_bBroadcast;
	bool		m_bInstant;
	bool		m_bIsActive;
	int32		m_nStringId;

	NavMarker*  SpawnMarker() const;
		
private:
	// Copy ctor and assignment operator not implemented and should never be used.
	NavMarkerCreator( NavMarkerCreator const& other );
	NavMarkerCreator& operator=( NavMarkerCreator const& other );
};


class NavMarker : public GameBase
{
	public :

		NavMarker();
		~NavMarker();

		virtual void	Activate(bool bActive) {	m_bIsActive = bActive; CreateSpecialFX(true); }

		virtual void	SetTargetObject( HOBJECT hTarget ) { m_hTarget = hTarget; CreateSpecialFX( true ); }

	protected :

		uint32			EngineMessageFn (uint32 messageID, void *pData, float lData);

		virtual void	CreateSpecialFX( bool bUpdateClients = false );

		virtual void	OnLinkBroken( LTObjRefNotifier *pRef, HOBJECT hObj );




	protected:

		std::string			m_sTargetName;
		LTObjRefNotifier	m_hTarget;
		uint8				m_nTeamId;
		uint32				m_nStringId;
		HRECORD				m_hType;
		bool				m_bIsActive;

		//support for temporary NavMarkers
		StopWatchTimer		m_LifeTimeTimer;
		float				m_fLifeTime;
		bool				m_bBroadcast;

	private :

		bool	ReadProp(const GenericPropList *pProps);
		bool	InitialUpdate();
		bool	Update();
  		void	AssignTarget();

		void	Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
		void	Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);
		
		void	SaveSFXMessage( ILTMessage_Write *pMsg, uint32 dwSaveFlags );
		void	LoadSFXMessage( ILTMessage_Read *pMsg, uint32 dwLoadFlags );

		
		// Message Handlers...

		DECLARE_MSG_HANDLER( NavMarker, HandleOnMsg );
		DECLARE_MSG_HANDLER( NavMarker, HandleOffMsg );
		DECLARE_MSG_HANDLER( NavMarker, HandleTargetMsg );
		DECLARE_MSG_HANDLER( NavMarker, HandleTypeMsg );
		DECLARE_MSG_HANDLER( NavMarker, HandleTextMsg );
		DECLARE_MSG_HANDLER( NavMarker, HandleTeamMsg );
};

class CNavMarkerPlugin : public IObjectPlugin
{
	public:	

		virtual LTRESULT PreHook_EditStringList(
				const char* szRezPath,
				const char* szPropName,
				char** aszStrings,
				uint32* pcStrings,
				const uint32 cMaxStrings,
				const uint32 cMaxStringLength);

	virtual LTRESULT PreHook_PropChanged( 
				const char *szObjName,
				const char *szPropName,
				const int nPropType,
				const GenericProp &gpPropValue,
				ILTPreInterface *pInterface,
				const	char *szModifiers );

	protected:

		CCommandMgrPlugin m_CommandMgrPlugin;
		CStringEditMgrPlugin m_StringEditMgrPlugin;

};

#endif // __AimMagnet_H__
