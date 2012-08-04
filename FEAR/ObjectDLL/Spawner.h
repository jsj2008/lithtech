// ----------------------------------------------------------------------- //
//
// MODULE  : Spawner.h
//
// PURPOSE : Spawner class - implementation
//
// CREATED : 1/9/98
//
// (c) 1997-2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __SPAWNER_H__
#define __SPAWNER_H__

#include "ltengineobjects.h"
#include "GameBase.h"
#include "CommandMgr.h"

using namespace std;

LINKTO_MODULE( Spawner );

BaseClass *SpawnObject( char const* pszSpawn, const LTVector& vPos, const LTRotation& rRot );

class Spawner : public GameBase
{
	public :

		Spawner();
		virtual ~Spawner();

		void	Setup( );

	protected :

		virtual uint32      EngineMessageFn(uint32 messageID, void *pData, float lData);
		virtual bool		ReadProp(const GenericPropList *pProps);

		virtual BaseClass*	Spawn( const char *pszSpawnString, const char *pszObjName, const LTVector &vPos, const LTRotation &rRot );

		virtual void	Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
		virtual void	Load(ILTMessage_Read *pMsg, uint32 dwSaveFlags);

	private :

		bool	PostPropRead(ObjectCreateStruct* pData);
		bool	InitialUpdate();

		string	m_sTarget;
		string	m_sSpawnSound;
		string	m_sInitialCommand;
		float	m_fSoundRadius;


		// Message Handler...

		DECLARE_MSG_HANDLER( Spawner, HandleSpawnMsg );
		DECLARE_MSG_HANDLER( Spawner, HandleSpawnFromMsg );
};

class CSpawnerPlugin : public IObjectPlugin
{
	public :

		virtual LTRESULT PreHook_PropChanged( 
			const	char		*szObjName,
			const	char		*szPropName,
			const	int			nPropType,
			const	GenericProp	&gpPropValue,
					ILTPreInterface	*pInterface,
			const	char		*szModifiers );

	protected :

		CCommandMgrPlugin m_CommandMgrPlugin;
};


#endif // __SPAWNER_H__
