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

class CServerMark : public BaseClass
{
	public :

		CServerMark();
		virtual ~CServerMark();

		void Setup(CLIENTWEAPONFX & theStruct);

	protected :

        uint32 EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);
};

#endif // __SERVER_MARK_H__