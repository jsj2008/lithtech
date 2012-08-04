// ----------------------------------------------------------------------- //
//
// MODULE  : AISpawner.h
//
// PURPOSE : AISpawner class - implementation
//
// CREATED : 1/16/2004
//
// (c) 1997-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __AI_SPAWNER_H__
#define __AI_SPAWNER_H__

#include "Spawner.h"
#include "AIConfig.h"

LINKTO_MODULE( AISpawner );

class AISpawner : public Spawner
{
	typedef Spawner super;

	public:

		AISpawner();

	protected:

		virtual bool		ReadProp(const GenericPropList *pProps);
		virtual BaseClass*	Spawn( const char *pszSpawnString, const char *pszObjName, const LTVector &vPos, const LTRotation &rRot );

		virtual void		Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
		virtual void		Load(ILTMessage_Read *pMsg, uint32 dwSaveFlags);

	private:

		std::string		m_strSpawnedAIName;
		CAIConfig		m_AIConfig;
};

class CAISpawnerPlugin : public CSpawnerPlugin
{
	typedef CSpawnerPlugin super;

public:

	virtual LTRESULT PreHook_EditStringList(const char* szRezPath, const char* szPropName, char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength);

	virtual LTRESULT PreHook_PropChanged( 
		const	char		*szObjName,
		const	char		*szPropName,
		const	int			nPropType,
		const	GenericProp	&gpPropValue,
		ILTPreInterface	*pInterface,
		const	char		*szModifiers );

protected:

	CAIConfigPlugin		m_AIConfigPlugin;
};

#endif // __AI_SPAWNER_H__
