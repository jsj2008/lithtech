// ----------------------------------------------------------------------- //
//
// MODULE  : DynamicOccluderVolumeFX.cpp
//
// PURPOSE : DynamicOccluderVolume special fx class - Implementation
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
	#include "DynamicOccluderVolumeFX.h"


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CDynamicOccluderVolumeFX::CDynamicOccluderVolumeFX
//
//  PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

CDynamicOccluderVolumeFX::CDynamicOccluderVolumeFX()
:	CSpecialFX		(),
	m_bEnabled		( true ),
	m_nNumOccluderIds ( 0 )
{
	m_nOccluderIds[0] = 0;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CDynamicOccluderVolumeFX::~CDynamicOccluderVolumeFX
//
//  PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

CDynamicOccluderVolumeFX::~CDynamicOccluderVolumeFX()
{
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CDynamicOccluderVolumeFX::Init
//
//  PURPOSE:	Init the DynamicOccluderVolume....
//
// ----------------------------------------------------------------------- //

LTBOOL CDynamicOccluderVolumeFX::Init( HLOCALOBJ hServObj, ILTMessage_Read *pMsg )
{
	if( !CSpecialFX::Init( hServObj, pMsg )) 
		return LTFALSE;

	if( !pMsg ) 
		return LTFALSE;

	LTBOOL bRet = OnServerMessage(pMsg);

	// All occluders should be disabled by default...
	m_bEnabled = false;  
	EnableOccluders(false);

	// All groups are enabled by default, but we don't need to change anything...

	return bRet;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CDynamicOccluderVolumeFX::OnServerMessage
//
//  PURPOSE:	Handle a message from the server...
//
// ----------------------------------------------------------------------- //

LTBOOL CDynamicOccluderVolumeFX::OnServerMessage( ILTMessage_Read *pMsg )
{
	if( !pMsg ) return LTFALSE;

	//we need to read this initial list in a manner that we won't go beyond our storage, but
	//all data will still be read in preparation for our next list

	//read in the list of all the occluder ID's
	uint32 nNumOccluderIds = pMsg->Readuint8();
	m_nNumOccluderIds = 0;

	uint32 i;
	for (i=0; i < nNumOccluderIds; i++)
	{
		if(i < kMaxOccluderIds)
		{
			m_nOccluderIds[m_nNumOccluderIds] = pMsg->Readuint32();
			m_nNumOccluderIds++;
		}
	}

	//now read in the list of all the render groups
	uint32 nNumRenderGroups = pMsg->Readuint8();
	m_nNumRenderGroups = 0;

	for (i=0; i < nNumRenderGroups; i++)
	{
		if(i < kMaxRenderGroups)
		{
			m_nRenderGroups[m_nNumRenderGroups] = pMsg->Readuint8();
			m_nNumRenderGroups++;
		}
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CDynamicOccluderVolumeFX::EnableOccluders
//
//  PURPOSE:	Enable our occluders...
//
// ----------------------------------------------------------------------- //

void CDynamicOccluderVolumeFX::EnableOccluders(bool bEnable)
{
	for (int i=0; i < m_nNumOccluderIds; i++)
	{
		g_pLTClient->SetOccluderEnabled(m_nOccluderIds[i], bEnable);
	}

	m_bEnabled = true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CDynamicOccluderVolumeFX::EnableRenderGroups
//
//  PURPOSE:	Enable render groups associated with this volume
//
// ----------------------------------------------------------------------- //
void CDynamicOccluderVolumeFX::EnableRenderGroups(bool bEnable)
{
	for (uint32 nCurrRenderGroup = 0; nCurrRenderGroup < m_nNumRenderGroups; nCurrRenderGroup++)
	{
		g_pLTClient->SetObjectRenderGroupEnabled(m_nRenderGroups[nCurrRenderGroup], bEnable);
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CDynamicOccluderVolumeFX::Enable
//
//  PURPOSE:	Enable render data associated with this volume
//
// ----------------------------------------------------------------------- //
void CDynamicOccluderVolumeFX::Enable(bool bEnable)
{
	//avoid redundant changes
	if(bEnable == m_bEnabled)
		return;

	//alright, it has changed, apply the change
	EnableRenderGroups(!bEnable);
	EnableOccluders(bEnable);

	//set that to our new state
	m_bEnabled = bEnable;
}
