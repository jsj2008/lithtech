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

        uint32  EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);
};


class Camera : public GameBase
{
	public :

		Camera();
		~Camera();

		static void ClearActiveCameras() { sm_nActiveCamera = 0; }
		static LTBOOL IsActive() { return (sm_nActiveCamera != 0); }

		LTBOOL IsOn() const { return m_bOn; }

	protected :

        uint32  EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);
		virtual bool OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg);

		static int sm_nActiveCamera;
	
	private :

		void	Update();
		void	InitialUpdate(int nInfo);

        LTBOOL  ReadProps(LTBOOL bCreateSFXMsg=LTTRUE);

		void	CreateSFXMsg();
		
		void	TurnOn();
		void	TurnOff(bool bSkip=false);

        void    Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
        void    Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);

        LTFLOAT m_fActiveTime;          // How long camera stays on
        LTFLOAT m_fTurnOffTime;         // Time to turn the camera off
        uint8	m_nCameraType;          // Camera type (CT_XXX in ClientServerShared.h)
        LTBOOL  m_bAllowPlayerMovement; // Can the player move when the camera is live
        LTBOOL  m_bOneTime;             // Do we activate only one time
        LTBOOL  m_bStartActive;         // The camera starts active
        LTBOOL  m_bIsListener;          // Listen for sounds at camera position
		LTBOOL	m_bOn;					// Is the camera on?
		LTBOOL	m_bCanSkip;				// Can the player turn this camera off?
		LTBOOL	m_bOnSkipCleanupOnly;	// If the we receive the "skip" command, only do clean up
		HSTRING	m_hstrCleanupCmd;		// Command to process when the camera is turned off
		LTFLOAT m_fFovX;
		LTFLOAT	m_fFovY;
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