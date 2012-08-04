// ----------------------------------------------------------------------- //
//
// MODULE  : Camera.h
//
// PURPOSE : Camera class definition
//
// CREATED : 5/20/98
//
// ----------------------------------------------------------------------- //

#ifndef __CAMERA_H__
#define __CAMERA_H__

#include "cpp_engineobjects_de.h"

class Camera : public BaseClass
{
	public :

		Camera();

	protected :

		DDWORD	EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData);
		DDWORD	ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);
		void	TriggerMsg(HOBJECT hSender, HSTRING hMsg);

	private :

		DBOOL ReadProp(ObjectCreateStruct *pData);
		void  Update();
		void  InitialUpdate(int nInfo);

		void Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags);

		DFLOAT	m_fActiveTime;			// How long camera stays on
		DFLOAT	m_fTurnOffTime;			// Time to turn the camera off
		DBYTE	m_nCameraType;			// Camera type (CT_XXX in ClientServerShared.h)
		DBOOL	m_bAllowPlayerMovement;	// Can the player move when the camera is live
		DBOOL	m_bOneTime;				// Do we activate only one time
		DBOOL	m_bStartActive;			// The camera starts active
		DBOOL	m_bIsListener;			// Listen for sounds at camera position
};

#endif // __CAMERA_H__

