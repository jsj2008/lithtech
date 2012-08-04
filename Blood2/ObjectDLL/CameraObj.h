// ----------------------------------------------------------------------- //
//
// MODULE  : CameraObj.h
//
// PURPOSE : CameraObj header
//
// CREATED : 02/17/98
//
// ----------------------------------------------------------------------- //

#ifndef __CAMERAOBJ_H__
#define __CAMERAOBJ_H__

#include "cpp_engineobjects_de.h"
#include "cpp_server_de.h"
#include "B2BaseClass.h"
#include "DLink.h"
//#include "PathListData.h"


class CameraObj : public B2BaseClass
{
	public :

		CameraObj();
		~CameraObj();
		DBOOL		Setup(DBYTE nType);
		void		SetActive(DBOOL bActive);
		DBOOL		SetLinkObject(HOBJECT hObject);

		HOBJECT		GetLinkObject() const { return m_hLinkObject; }

		DBOOL		IsListener() const { return m_bIsListener; }
		DBOOL		IsActive() const { return m_bActive; }
		DBOOL		AllowPlayerMovement() const { return m_bPlayerMovement; }

		//efficient means to track destructable objects in a level
		static DLink	m_CameraHead;
		static DDWORD	m_dwNumCameras;
		DLink			m_Link;

	protected :

		DDWORD  EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData);
		DDWORD	ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);

	private :

		DBOOL		InitialUpdate();
		DBOOL		Update();
		void		PostPropRead(ObjectCreateStruct *pStruct);
		DBOOL		ReadProp(ObjectCreateStruct *pStruct);
		void		HandleTrigger(HOBJECT hSender, HSTRING hMsg);
		DBOOL		DisplayRay();
		void		Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void		Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags);

	public:

		DBOOL		m_bActive;
		DBOOL		m_bStartActive;		// Camera starts active
		DBOOL		m_bIsListener;		// Camera becomes the listener when active
		DFLOAT		m_fActiveTime;		// Length of time to start active -1
		DFLOAT		m_fDeactivateTime;	// Time we should deactivate
		DBOOL		m_bPlayerMovement;	// Allow player movement when active
		DBYTE		m_nType;			// Camera type
		DBOOL		m_bHidePlayer;		// Hide the player object
		HOBJECT		m_hRay;
		HOBJECT		m_hLinkObject;
};


extern HOBJECT g_hActiveCamera;

#endif // __CAMERAOBJ_H__
