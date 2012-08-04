// ----------------------------------------------------------------------- //
//
// MODULE  : GameWorldEditImpl.h
//
// PURPOSE : Declares the CGameWorldEditImpl class.  This class
//           implements the IGameWorldEdit interface.  This interface
//           is used to communicate with WorldEdit.
//
// CREATED : 01/28/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __GAMEWORLDEDITIMPL_H__
#define __GAMEWORLDEDITIMPL_H__

#include "igameworldedit.h"

class CGameWorldEditImpl : public IGameWorldEdit
{
private:

public:
	CGameWorldEditImpl();
	virtual ~CGameWorldEditImpl();

	// returns the number of surface records in the database
	virtual uint32		GetSurfaceCount();
	// returns the name of the specified surface
	virtual bool		GetSurfaceName( uint32 nSurface, char* szSurface, uint32 nLength );
	// returns the Id of the specified surface
	virtual bool		GetSurfaceId( uint32 nSurface, uint32& nId );

	// validates the surface records and returns the first error encountered in the passed in string
	virtual bool		ValidateSurfaceRecords( char* szError, uint32 nLength );
};

#endif  // __GAMEWORLDEDITIMPL_H__
