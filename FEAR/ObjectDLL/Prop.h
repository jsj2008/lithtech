// ----------------------------------------------------------------------- //
//
// MODULE  : Prop.h
//
// PURPOSE : Model Prop - Definition
//
// CREATED : 10/9/97
//
// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __PROP_H__
#define __PROP_H__

#include "GameBase.h"
#include "DestructibleModel.h"
#include "AIState.h"
#include "ActivateTypeHandler.h"
#include "PrefetchUtilities.h"

// Forward declarations.
class CAttachmentPosition;

typedef std::vector<HMODELANIM, LTAllocator<HMODELANIM, LT_MEM_TYPE_OBJECTSHELL> > HMODELANIM_LIST;


LINKTO_MODULE( Prop );

enum EnumPropAnimationType
{
	kPA_Touch,
	kPA_Knock,
};

class Prop : public GameBase
{
	public :

 		Prop();
		virtual ~Prop();

		CDestructible*	GetDestructible() { return &m_damage; }
		EnumAIStateType	GetState() const { return m_eState; }
		
		void	HandleAttachmentImpact( CAttachmentPosition *pAttachPos, const LTVector& vDir );

		bool			IsMoveToFloor( ) const { return !!m_bMoveToFloor; }
		void			SetMoveToFloor( bool bValue ) { m_bMoveToFloor = bValue; }

		static void		GetPrefetchResourceList(const char* pszObjectName, IObjectResourceGatherer* pInterface, ResourceList& Resources );

	protected :

        uint32  EngineMessageFn(uint32 messageID, void *pData, float fData);

		// Sets up object based on member varaibles.
		virtual bool Setup( );

		void	StartFade( float fDuration, float fDelayStartTime = 0.0f, float fEndAlpha = 0.0f, bool bRemove = true );

		CDestructibleModel			m_damage;

        bool						m_bMoveToFloor;
        bool						m_bFirstUpdate;
		bool						m_bCanDeactivate;
        LTVector                    m_vObjectColor;
        float	                    m_fScale;
        float						m_fAlpha;
        uint32                      m_dwUsrFlgs;
        uint32                      m_dwFlags;
        uint32                      m_dwFlags2;

		bool						m_bTouchable;
		HMODELANIM_LIST				m_lstWorldAnims;		
		EnumAIStateType				m_eState;

		// Used only for sending event trigger messages.
		bool						m_bActivatedOn;
		std::string					m_sActivateOnCommand;
		std::string					m_sActivateOffCommand;

		// Prop as an attachment...
		
		bool						m_bAttachmentShotOff;
		LTObjRef					m_hAttachmentOwner;

		// Rotation 

		float						m_fPitch;
        float						m_fYaw;
        float						m_fRoll;
        float						m_fPitchVel;
        float						m_fYawVel;
        float						m_fRollVel;
		bool						m_bRotatedToRest;
		bool						m_bRotating;

		// Fade properties...

		bool						m_bFading;
		double						m_fFadeStartTime;
		float						m_fFadeDuration;
		float						m_fStartAlpha;
		float						m_fEndAlpha;
		bool						m_bFadeRemoveWhenDone;

		CActivateTypeHandler		m_ActivateTypeHandler;

		bool						m_bCanTransition;

	private :

		void	ReadProp(const GenericPropList *pProps);
		void	PostPropRead(ObjectCreateStruct *pStruct);
		void	InitialUpdate();
		void	HandleModelString( ArgList *pArgList );

        void    Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
        void    Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);

		void	PlayRandomWorldAnim();

		// Used only for sending event trigger messages.
		void	SendActivateMessage( HOBJECT hSender );

		void	HandleAttachmentTouch( HOBJECT hToucher );

		void	UpdateFade();
		void	Update();


		// Message Handlers...

		DECLARE_MSG_HANDLER( Prop, HandleAnimMsg );
		DECLARE_MSG_HANDLER( Prop, HandleAnimLoopMsg );
		DECLARE_MSG_HANDLER( Prop, HandleActivateMsg );
		DECLARE_MSG_HANDLER( Prop, HandleDestroyMsg );
};

class CPropPlugin : public IObjectPlugin
{
  public:

    virtual LTRESULT PreHook_EditStringList(
		const char* szRezPath,
		const char* szPropName,
		char** aszStrings,
        uint32* pcStrings,
        const uint32 cMaxStrings,
        const uint32 cMaxStringLength);

	virtual LTRESULT PreHook_PropChanged( 
			const	char		*szObjName,
			const	char		*szPropName,
			const	int			nPropType,
			const	GenericProp	&gpPropValue,
					ILTPreInterface	*pInterface,
			const	char		*szModifiers );

	virtual LTRESULT PreHook_Dims(const char* szRezPath,
		const char* szPropValue,
		const char* szPropName, 
		char* szModelFilenameBuf,
		int nModelFilenameBufLen,
		LTVector & vDims,
		const char* pszObjName, 
		ILTPreInterface *pInterface);

  protected :

		CDestructibleModelPlugin	m_DestructibleModelPlugin;
		CCommandMgrPlugin			m_CommandMgrPlugin;

};

#endif // __PROP_H__
