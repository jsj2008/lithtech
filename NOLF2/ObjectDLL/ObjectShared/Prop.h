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
#include "ActivateTypeMgr.h"

// Forward declarations.
struct PROP_DISTURB;
class CAttachmentPosition;

typedef std::vector<HMODELANIM> HMODELANIM_LIST;


LINKTO_MODULE( Prop );

enum EnumPropAnimationType
{
	kPA_Touch,
	kPA_Knock,
};

struct PropDisturbStruct
{
	 PropDisturbStruct();
	~PropDisturbStruct();

	void Save(ILTMessage_Write *pMsg);
	void Load(ILTMessage_Read *pMsg);

    HLTSOUND				hTouchSound;
	HLTSOUND				hHitSound;
	std::string				sTouchSoundName;
	HMODELANIM				hTouchAnim;
	HMODELANIM				hHitAnim;
	EnumPropAnimationType	eTouchAnimType;
	PROP_DISTURB*			pPD;
};


class Prop : public GameBase
{
	public :

 		Prop();
		virtual ~Prop();

		CDestructible*	GetDestructible() { return &m_damage; }
		EnumAIStateType	GetState() const { return m_eState; }
		
		void	HandleAttachmentImpact( CAttachmentPosition *pAttachPos, const LTVector& vDir );

	protected :

        uint32  EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);
        uint32  ObjectMessageFn(HOBJECT hSender, ILTMessage_Read *pMsg);

		// Sets up object based on member varaibles.
		virtual bool Setup( );

		bool	OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg);

		// [KLS 5/20/02] Lots of things derive from prop, give them a chance to 
		// handle this.
		virtual void HandleDestroy(HOBJECT hDamager);

		void	StartFade( float fDuration, float fDelayStartTime = 0.0f, float fEndAlpha = 0.0f, bool bRemove = true );

		CDestructibleModel			m_damage;

        LTBOOL						m_bMoveToFloor;
        LTBOOL                      m_bFirstUpdate;
		bool						m_bCanDeactivate;
        LTVector                    m_vScale;
        LTVector                    m_vObjectColor;
        LTFLOAT                     m_fAlpha;
        uint32                      m_dwUsrFlgs;
        uint32                      m_dwFlags;
        uint32                      m_dwFlags2;

		char*						m_pDebrisOverride;

		LTBOOL						m_bTouchable;
		HMODELANIM_LIST				m_lstWorldAnims;		
		EnumAIStateType				m_eState;
		PropDisturbStruct*			m_pDisturb;

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
		float						m_fFadeStartTime;
		float						m_fFadeDuration;
		float						m_fStartAlpha;
		float						m_fEndAlpha;
		bool						m_bFadeRemoveWhenDone;

		CActivateTypeHandler		m_ActivateTypeHandler;
		
		bool						m_bCanTransition;

	private :

		void	ReadProp(ObjectCreateStruct *pStruct);
		void	PostPropRead(ObjectCreateStruct *pStruct);
		void	InitialUpdate();
		void	HandleModelString( ArgList *pArgList );

        void    Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
        void    Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);

		void	HandleTouch(HOBJECT hToucher);
		void	HandleHit( HOBJECT hDamager );
		LTBOOL	ClearTouchSoundIfDone(LTBOOL bForceDone);
		LTBOOL	ClearTouchAnimIfDone(HMODELANIM hAnim, LTBOOL bForceDone);
		LTBOOL	ClearHitSoundIfDone( LTBOOL bForceDone );
		LTBOOL	ClearHitAnimIfDone( LTBOOL bForceDone );
		void	PlayRandomWorldAnim();
		void	PlayTouchSound(LTVector& vPos);
		void	PlayHitSound( LTVector &vPos );

		// Used only for sending event trigger messages.
		void	SendActivateMessage( );

		void	HandleAttachmentTouch( HOBJECT hToucher );

		void	UpdateFade();
		void	Update();
};

#ifndef __PSX2
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

  protected :

		CDestructibleModelPlugin	m_DestructibleModelPlugin;
		CCommandMgrPlugin			m_CommandMgrPlugin;

};
#endif

#endif // __PROP_H__