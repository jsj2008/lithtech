
#include "stdafx.h"
#include "DiscDeflector.h"


BEGIN_CLASS( CDiscDeflector )
END_CLASS_DEFAULT_FLAGS( CDiscDeflector, CProjectile, NULL, NULL, CF_HIDDEN )


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDisc::CDiscDeflector()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

CDiscDeflector::CDiscDeflector()
{
	// Construct
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDisc::~CDiscDeflector()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

CDiscDeflector::~CDiscDeflector()
{
	// Destruct
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDisc::EngineMessageFn()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

uint32 CDiscDeflector::EngineMessageFn(uint32 messageID,void *pData,LTFLOAT fData)
{
	// do nothing yet
	return GameBase::EngineMessageFn( messageID, pData, fData );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDisc::ObjectMessageFn()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

uint32 CDiscDeflector::ObjectMessageFn(HOBJECT hSender,ILTMessage_Read *pMsg)
{
	// do nothing yet
	return GameBase::ObjectMessageFn( hSender, pMsg );
}
