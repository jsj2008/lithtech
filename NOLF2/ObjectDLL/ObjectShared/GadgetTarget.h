// ----------------------------------------------------------------------- //
//
// MODULE  : GadgetTarget.h
//
// PURPOSE : The GadgetTarget object
//
// CREATED : 8/27/01
//
// (c) 2001-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __GADGET_TARGET_H__
#define __GADGET_TARGET_H__

#include "GadgetTargetMgr.h"
#include "Prop.h"
#include "GadgetTargetTypes.h"

// 
// Forwards...
//

LINKTO_MODULE( GadgetTarget );

class GadgetTarget : public Prop
{
	public :	// Methods...
		
		GadgetTarget();
		virtual ~GadgetTarget();

		// Implementing classes will have this function called
		// when HOBJECT ref points to gets deleted.
		virtual void OnLinkBroken( LTObjRefNotifier *pRef, HOBJECT hObj );
		
		GadgetTargetType GetType() const { return (m_pGTInfo ? m_pGTInfo->m_eTargetType : eINVALID); }

	protected : // Methods...

		virtual uint32	EngineMessageFn( uint32 messageID, void *pData, LTFLOAT fData );
		virtual uint32	ObjectMessageFn( HOBJECT hSender, ILTMessage_Read *pMsg );
		virtual bool	OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg);

		virtual LTBOOL	ReadProp( ObjectCreateStruct *pStruct );

		virtual LTBOOL	OnObjectCreated( );
		virtual LTBOOL	OnSave( ILTMessage_Write *pMsg ); 
		virtual LTBOOL	OnLoad( ILTMessage_Read *pMsg );

		virtual void	HandleGadgetMsg( const CParsedMsg &cMsg, HOBJECT hSender );
		virtual void	HandleStoppedMsg( const CParsedMsg &cMsg );
		virtual void	HandleActivateMsg( HOBJECT hSender );
		virtual void	HandleCanActivateMsg( );
		virtual void	SetupDisablingState( HOBJECT hSender );
		virtual void	SetupDisabledState( LTBOOL bDestroyed = LTFALSE );
		virtual void	SpecialDisabledHandling();

		enum LightPosition
		{
			eLightPos1=1,
			eLightPos2=2
		};

		virtual void	TurnOn(bool bOn=true);
		virtual void	TurnLightOn(bool bOn=true);
		virtual void	SetLightPosition(LightPosition eLightPos);

		void			CreateSpecialFX( bool bUpdateClients /* = false  */ );

	protected : // Members...

		GTINFO		*m_pGTInfo;

		LTFLOAT		m_fMinTime;
		LTFLOAT		m_fMaxTime;
		LTFLOAT		m_fDisableTime;
		LTFLOAT		m_fTotalTime;
		LTFLOAT		m_fSoundRadius;
		HSTRING		m_hstrDisablingSnd;
		HSTRING		m_hstrDisabledSnd;
		HSTRING		m_hstrDisabledCmd;
		HSTRING		m_hstrPowerOffCmd;
		uint32		m_dwGadgetUsrFlgs;
		HLTSOUND	m_hSound;
		LTBOOL		m_bDisabled;
		LTBOOL		m_bDisableRequested;
		uint32		m_dwCodeID;
		LTBOOL		m_bRemoveWhenDisabled;
		LTBOOL		m_bInfiniteDisables;
		LTBOOL		m_bInfiniteActivates;
		LTBOOL		m_bOn;
		LTBOOL		m_bRestoreFlagsOnLoad;

		LTObjRefNotifier	m_hAttachedModel;
		LTObjRefNotifier	m_hLight;
 		LightPosition		m_eLightPos;

		uint32				m_nGTID;

		uint8		m_nTeamID;

	private :

		void	CreateLight();
};

class CGadgetTargetPlugin : public CPropPlugin
{
	public:

		virtual LTRESULT PreHook_EditStringList(
			const char* szRezPath,
			const char* szPropName,
			char** aszStrings,
			uint32* pcStrings,
			const uint32 cMaxStrings,
			const uint32 cMaxStringLength);

		virtual LTRESULT PreHook_Dims(
				const char* szRezPath,
				const char* szPropValue,
				char* szModelFilenameBuf,
				int	  nModelFilenameBufLen,
				LTVector & vDims);

		virtual LTRESULT PreHook_PropChanged(
				const char *szObjName,
				const char *szPropName,
				const int nPropType,
				const GenericProp &gpPropValue,
				ILTPreInterface *pInterface,
				const char *szModifiers );

	protected:

		CGadgetTargetMgrPlugin	m_GadgetTargetMgrPlugin;
		CCommandMgrPlugin		m_CommandMgrPlugin;


};

#endif //__GADGET_TARGET_H__