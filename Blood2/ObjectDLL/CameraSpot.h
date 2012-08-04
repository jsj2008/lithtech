// ----------------------------------------------------------------------- //
//
// MODULE  : CameraSpot.h
//
// PURPOSE : CameraSpot header
//
// CREATED : 02/17/98
//
// ----------------------------------------------------------------------- //

#ifndef __CAMERASPOT_H__
#define __CAMERASPOT_H__

#include "cpp_engineobjects_de.h"
#include "cpp_server_de.h"
#include "PathListData.h"
#include "B2BaseClass.h"

class CameraSpot : public B2BaseClass
{
	public :

		CameraSpot();
		~CameraSpot();

	protected :

		virtual DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData);
//		DBOOL m_bActive;   

	private :

		void ObjectTouch(HOBJECT hObj);

		DBOOL InitialUpdate(DVector *pMovement);
//		DBOOL Update(DVector *pMovement);
//		DBOOL ReadProp(ReadPropInfo *pInfo);
		void  PostPropRead(ObjectCreateStruct *pStruct);

    public:
        
};

#endif // __CAMERASPOT_H__
