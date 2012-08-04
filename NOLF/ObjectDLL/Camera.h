// ----------------------------------------------------------------------- //
//
// MODULE  : Camera.h
//
// PURPOSE : Camera class definition
//
// CREATED : 5/20/98
//
// (c) 1998-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __CAMERA_H__
#define __CAMERA_H__

#include "ltengineobjects.h"

#define ADD_CAMERA_PROPERTIES(groupflag) \
	ADD_REALPROP_FLAG(ActiveTime, -1.0f, groupflag) \
    ADD_BOOLPROP_FLAG(AllowPlayerMovement, LTFALSE, groupflag) \
    ADD_BOOLPROP_FLAG(OneTime, LTTRUE, groupflag) \
	ADD_LONGINTPROP_FLAG(Type, CT_CINEMATIC, groupflag) \
    ADD_BOOLPROP_FLAG(StartActive, LTFALSE, groupflag) \
    ADD_BOOLPROP_FLAG(IsListener, LTFALSE, groupflag)


class Camera : public BaseClass
{
	friend class CinematicTrigger;

	public :

		Camera();
		~Camera();

		static void ClearActiveCameras() { sm_nActiveCamera = 0; }
		static LTBOOL IsActive() { return (sm_nActiveCamera != 0); }

	protected :

        uint32  EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);
        uint32  ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead);

		static int sm_nActiveCamera;
	
	private :

		void	Update();
		void	InitialUpdate(int nInfo);

		void	TriggerMsg(HOBJECT hSender, const char* szMsg);
        LTBOOL  ReadProps(LTBOOL bCreateSFXMsg=LTTRUE);

		void	CreateSFXMsg();
		void	TurnOn();
		void	TurnOff();

        void    Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags);
        void    Load(HMESSAGEREAD hRead, uint32 dwLoadFlags);

        LTFLOAT m_fActiveTime;          // How long camera stays on
        LTFLOAT m_fTurnOffTime;         // Time to turn the camera off
        uint8	m_nCameraType;          // Camera type (CT_XXX in ClientServerShared.h)
        LTBOOL  m_bAllowPlayerMovement; // Can the player move when the camera is live
        LTBOOL  m_bOneTime;             // Do we activate only one time
        LTBOOL  m_bStartActive;         // The camera starts active
        LTBOOL  m_bIsListener;          // Listen for sounds at camera position
		LTBOOL	m_bOn;					 // Is the camera on?
};

#endif // __CAMERA_H__