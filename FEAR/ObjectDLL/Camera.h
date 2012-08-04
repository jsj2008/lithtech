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

#include "GameBase.h"
#include "ltengineobjects.h"
#include "Point.h"
#include "CommandMgr.h"

LINKTO_MODULE( Camera );

class CameraPoint : public BaseClass
{
	public :
	
		CameraPoint() : BaseClass	( OT_NORMAL )
		{
		}
			
	protected :

		uint32  EngineMessageFn(uint32 messageID, void *pData, float fData);
};


class Camera : public GameBase
{
	public :

		Camera();
		~Camera();

		static void ClearActiveCameras() { sm_nActiveCamera = 0; }
		static bool IsActive() { return (sm_nActiveCamera != 0); }

		bool IsOn() const { return m_bOn; }

	protected :

		uint32  EngineMessageFn(uint32 messageID, void *pData, float fData);

		static int sm_nActiveCamera;
	
	private :

		void	Update();
		void	InitialUpdate(int nInfo);

		bool	ReadProps(const GenericPropList *pProps, bool bCreateSFXMsg=true);

		void	CreateSFXMsg();
		
		void	TurnOn();
		void	TurnOff(bool bSkip=false);

		void	Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
		void	Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);

		float			m_fActiveTime;				// How long camera stays on
		double			m_fTurnOffTime;				// Time to turn the camera off
		uint8			m_nCameraType;				// Camera type (CT_XXX in ClientServerShared.h)
		bool			m_bAllowPlayerMovement;		// Can the player move when the camera is live
		bool			m_bOneTime;					// Do we activate only one time
		bool			m_bStartActive;				// The camera starts active
		bool			m_bIsListener;				// Listen for sounds at camera position
		bool			m_bOn;						// Is the camera on?
		bool			m_bCanSkip;					// Can the player turn this camera off?
		bool			m_bOnSkipCleanupOnly;		// If the we receive the "skip" command, only do clean up
		std::string		m_sCleanupCmd;				// Command to process when the camera is turned off
		float			m_fFovY;					// The vertical full FOV measured in degrees
		float			m_fFovAspectScale;			// The aspect ratio scale used to apply distortion to the X FOV


		// Message Handler...

		DECLARE_MSG_HANDLER( Camera, HandleOnMsg );
		DECLARE_MSG_HANDLER( Camera, HandleOffMsg );
		DECLARE_MSG_HANDLER( Camera, HandleSkipMsg );
		DECLARE_MSG_HANDLER( Camera, HandleMoveToMsg );
		DECLARE_MSG_HANDLER( Camera, HandleFOVMsg );
};

class CCameraPlugin : public IObjectPlugin
{
	public:

		virtual LTRESULT PreHook_PropChanged( 
			const	char		*szObjName,
			const	char		*szPropName,
			const	int			nPropType,
			const	GenericProp	&gpPropValue,
					ILTPreInterface	*pInterface,
			const	char		*szModifiers );

	protected:

		CCommandMgrPlugin	m_CommandMgrPlugin;

};

#endif // __CAMERA_H__
