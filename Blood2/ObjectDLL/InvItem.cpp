//----------------------------------------------------------
//
// MODULE  : InvItem.cpp
//
// PURPOSE : Inventory item base class
//
// CREATED : 12/12/97
//
//----------------------------------------------------------

// Includes....
#include <stdlib.h>
#include "InvItem.h"
#include "PlayerObj.h"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInvItem::SendActionMessage()
//
//	PURPOSE:	Send action message to client
//
// ----------------------------------------------------------------------- //

void CInvItem::SendActionMessage()
{
	HMESSAGEWRITE hWrite;
	HCLIENT hClient;
	CPlayerObj *pObj;
	DBYTE nVal;

	if( IsPlayer( m_hOwner ))
	{
		pObj = ( CPlayerObj * )g_pServerDE->HandleToObject(m_hOwner);
		hClient = pObj->GetClient( );
		hWrite = g_pServerDE->StartMessage( hClient, SMSG_INVITEMACTION );
		nVal = GetType( );
		if( m_bIsActive )
			nVal |= 0x80;
		g_pServerDE->WriteToMessageByte( hWrite, nVal );
		g_pServerDE->EndMessage2( hWrite, MESSAGE_GUARANTEED | MESSAGE_NAGGLE );
	}
}


