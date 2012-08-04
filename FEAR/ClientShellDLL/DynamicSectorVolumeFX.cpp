// ----------------------------------------------------------------------- //
//
// MODULE  : DynamicSectorVolumeFX.cpp
//
// PURPOSE : DynamicSectorVolume special fx class - Implementation
//
// CREATED : 4/16/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//

	#include "stdafx.h"
	#include "DynamicSectorVolumeFX.h"


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CDynamicSectorVolumeFX::CDynamicSectorVolumeFX
//
//  PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

CDynamicSectorVolumeFX::CDynamicSectorVolumeFX()
:	CSpecialFX		(),
	m_bEnabled		( false ),
	m_nNumSectorIds ( 0 )
{
	m_nSectorIds[0] = 0;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CDynamicSectorVolumeFX::~CDynamicSectorVolumeFX
//
//  PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

CDynamicSectorVolumeFX::~CDynamicSectorVolumeFX()
{
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CDynamicSectorVolumeFX::Init
//
//  PURPOSE:	Init the DynamicSectorVolume....
//
// ----------------------------------------------------------------------- //

bool CDynamicSectorVolumeFX::Init( HLOCALOBJ hServObj, ILTMessage_Read *pMsg )
{
	if( !CSpecialFX::Init( hServObj, pMsg )) 
		return false;

	if( !pMsg ) 
		return false;

	bool bRet = OnServerMessage(pMsg);

	// All sectors should be enabled by default...
	m_bEnabled = true;  
	EnableSectors(true);

	return bRet;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CDynamicSectorVolumeFX::OnServerMessage
//
//  PURPOSE:	Handle a message from the server...
//
// ----------------------------------------------------------------------- //

bool CDynamicSectorVolumeFX::OnServerMessage( ILTMessage_Read *pMsg )
{
	if( !pMsg ) return false;

	//we need to read this initial list in a manner that we won't go beyond our storage, but
	//all data will still be read in preparation for our next list

	//read in the list of all the sector ID's
	uint32 nNumSectorIds = pMsg->Readuint8();
	m_nNumSectorIds = 0;

	uint32 i;
	for (i=0; i < nNumSectorIds; i++)
	{
		if(i < kMaxSectorIds)
		{
			m_nSectorIds[m_nNumSectorIds] = pMsg->Readuint32();
			m_nNumSectorIds++;
		}
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CDynamicSectorVolumeFX::EnableSectors
//
//  PURPOSE:	Enable our sectors...
//
// ----------------------------------------------------------------------- //

void CDynamicSectorVolumeFX::EnableSectors(bool bEnable)
{
	for (int i=0; i < m_nNumSectorIds; i++)
	{
		g_pLTClient->SetSectorVisibility(m_nSectorIds[i], bEnable);
	}

	m_bEnabled = true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CDynamicSectorVolumeFX::Enable
//
//  PURPOSE:	Enable render data associated with this volume
//
// ----------------------------------------------------------------------- //
void CDynamicSectorVolumeFX::Enable(bool bEnable)
{
	//avoid redundant changes
	if(bEnable == m_bEnabled)
		return;

	//alright, it has changed, apply the change
	EnableSectors(bEnable);

	//set that to our new state
	m_bEnabled = bEnable;
}
