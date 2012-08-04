// ----------------------------------------------------------------------- //
//
// MODULE  : CServerMark.h
//
// PURPOSE : CServerMark definition - Server-side mark fx
//
// CREATED : 1/15/99
//
// ----------------------------------------------------------------------- //

#ifndef __SERVER_MARK_H__
#define __SERVER_MARK_H__

#include "ltengineobjects.h"
#include "ClientWeaponSFX.h"

LINKTO_MODULE( ServerMark );

class CServerMark : public BaseClass
{
	public :

		CServerMark();
		virtual ~CServerMark();

	protected :

        uint32 EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);

		void OnSave( ILTMessage_Write *pMsg, uint32 dwSaveFlags );
		void OnLoad( ILTMessage_Read *pMsg, uint32 dwSaveFlags );

		//the amount of time that this mark has been alive for
		float	m_fElapsedTime;
};

#endif // __SERVER_MARK_H__