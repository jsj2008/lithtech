/****************************************************************************
;
;	MODULE:			SkeletonObject.h
;
;	PURPOSE:		SkeletonObject class header
;
;	HISTORY:		2/15/2002 [kml] This file was created
;
;	COMMENT:		Copyright (c) 2002, Monolith Productions, Inc.
;
****************************************************************************/

#ifndef _SKELETONOBJECT_H_
#define _SKELETONOBJECT_H_

#include "GameBase.h"

// This is for a generic object derived from GameBase
class SkeletonObject : public GameBase
{
	public :

		SkeletonObject();
		virtual ~SkeletonObject();

	protected :

        virtual uint32 EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);
		virtual uint32 ObjectMessageFn(HOBJECT hSender, ILTMessage_Read *pMsg);

	private :

        void Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
        void Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);

        LTBOOL ReadProp(ObjectCreateStruct *pStruct);
		LTBOOL PostPropRead(ObjectCreateStruct *pStruct);
		LTBOOL InitialUpdate();
		LTBOOL Update();

		// Example member variables
		uint32	m_dwSomeVariable;
};

#endif // _SKELETONOBJECT_H_