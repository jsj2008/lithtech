// ----------------------------------------------------------------------- //
//
// MODULE  : PathPoint.h
//
// PURPOSE : PathPoint header
//
// CREATED : 10/17/97
//
// ----------------------------------------------------------------------- //

#ifndef __PATHPOINT_H__
#define __PATHPOINT_H__

#include "cpp_engineobjects_de.h"
#include "cpp_server_de.h"
#include "PathListData.h"
#include "B2BaseClass.h"


class PathPoint : public B2BaseClass
{
	public :

		PathPoint();
		~PathPoint();

        DFLOAT m_fStartTime;                // PathPoint was started at this time (for Smells)
		HSTRING	GetActionTarget()	 { return m_hstrActionTarget; }
		HSTRING	GetActionMessage()	 { return m_hstrActionMessage; }
        char   sConnectName[128];           // Next Connection...
        
        
	protected :

		virtual DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData);
		DBOOL m_bActive;   

	private :

		void ObjectTouch(HOBJECT hObj);

		DBOOL InitialUpdate(DVector *pMovement);
		DBOOL Update(DVector *pMovement);
		DBOOL ReadProp(ObjectCreateStruct *pStruct);
		void  PostPropRead(ObjectCreateStruct *pStruct);

        HSTRING m_hstrActionTarget;               // Action (optional) AI script command
        HSTRING m_hstrActionMessage;              // Action (optional) message
};

#endif // __PATHPOINT_H__
