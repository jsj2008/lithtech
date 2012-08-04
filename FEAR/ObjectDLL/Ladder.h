// ----------------------------------------------------------------------- //
//
// MODULE  : Ladder.h
//
// PURPOSE : Definition of Ladder class
//
// CREATED : 06/21/04
//
// (c) 1999-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __LADDER_H__
#define __LADDER_H__


#include "GameBase.h"
#include "SurfaceDefs.h"

LINKTO_MODULE( Ladder );

class Ladder : public GameBase 
{
public:	// Methods...

	Ladder( );
	~Ladder( );

	static void GetPrefetchResourceList(const char* pszObjectName, IObjectResourceGatherer* pInterface, ResourceList& Resources );

protected :

	uint32 EngineMessageFn(uint32 messageID, void *pData, float lData);
	virtual void ReadProp(const GenericPropList *pProps);
	virtual void PostReadProp(ObjectCreateStruct *pStruct);

private :

	void InitialUpdate( );

	SurfaceType		m_eSurfaceOverrideType; // This associates an override surface, from SurfacesDB, with the ladder.

};


class LadderPlugin : public IObjectPlugin
{
public:
	LadderPlugin();
	virtual ~LadderPlugin();

	virtual LTRESULT PreHook_EditStringList(
		const char* szRezPath,
		const char* szPropName,
		char** aszStrings,
		uint32* pcStrings,
		const uint32 cMaxStrings,
		const uint32 cMaxStringLength);

};


#endif  // __LADDER_H__
