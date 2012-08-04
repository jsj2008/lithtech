// ----------------------------------------------------------------------- //
//
// MODULE  : DOOR.h
//
// PURPOSE : A Door object
//
// CREATED : 8/5/97 5:07:00 PM
//
// (c) 1997-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __DOOR_H__
#define __DOOR_H__

#include "ltbasedefs.h"
#include "GameBase.h"
#include "DestructibleModel.h"
#include "iltserver.h"
#include "Editable.h"
#include "iltlightanim.h"


#define DF_ACTIVATETRIGGER		(1<<0)	// Activate triggerable
#define DF_STARTOPEN			(1<<1)	// Start in open state
#define DF_TRIGGERCLOSE			(1<<2)  // Must trigger door to close
#define DF_REMAINSOPEN			(1<<3)  // Door remains open
#define DF_FORCEMOVE			(1<<4)	// Force moving door to continue

#define DOORSTATE_CLOSED		1
#define DOORSTATE_CLOSING		2
#define DOORSTATE_OPEN			3
#define DOORSTATE_OPENING		4

#define MAX_DOOR_LIGHT_STEPS	64
#define MAX_DOOR_LIGHT_ANIMS	8

enum DoorWaveTypes
{
	DOORWAVE_LINEAR,
	DOORWAVE_SINE,
	DOORWAVE_SLOWOFF,
	DOORWAVE_SLOWON
};

LTFLOAT GetDoorWaveValue( LTFLOAT fSpeed, LTFLOAT fPercent, uint32 nWaveType );


// Each Door type has a SetupTransformFn used to generate light animations.
typedef void (*SetupTransformFn)(ILTPreLight *pInterface,
	HPREOBJECT hObject,
	float fPercent,
    LTVector &vOutPos,
    LTRotation &rOutRotation);


// The Door's tool plugins.
class CDoorPlugin : public IObjectPlugin
{
  public:

    virtual LTRESULT PreHook_Light(
        ILTPreLight *pInterface,
		HPREOBJECT hObject);

    virtual LTRESULT PreHook_EditStringList(
		const char* szRezPath,
		const char* szPropName,
		char** aszStrings,
        uint32* pcStrings,
        const uint32 cMaxStrings,
        const uint32 cMaxStringLength);

  protected :
	  CDestructibleModelPlugin m_DestructibleModelPlugin;
};

// Door structure
class Door : public GameBase
{
	public:

		Door();
		virtual ~Door();

        LTFLOAT  GetMoveDist()   const { return m_fMoveDist; }
        uint32  GetState()      const { return m_dwDoorState; }

        LTBOOL   IsAITriggerable() const { return m_bAITriggerable; }
        LTBOOL   IsLocked() const { return m_bLocked; }

	protected:

        uint32  EngineMessageFn(uint32 messageID, void *pData, float fData);
        uint32  ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead);

		void	FirstUpdate();

		virtual void Open();
		virtual void SetLightAnimOpen();
		virtual void Opening();
		virtual void Closed();
		virtual void SetLightAnimClosed();
		virtual void Closing();

        virtual LTBOOL GetMoveTestPosRot(LTVector & vTestPos, LTRotation & rTestRot);
        inline LTBOOL ActivateObjectCollision(LTVector vTestPos, LTRotation rTestRot)
		{
            if (!m_hActivateObj) return LTFALSE;
			return TestObjectCollision(m_hActivateObj, vTestPos, rTestRot);
		}
        virtual LTBOOL TestObjectCollision(HOBJECT hTest, LTVector vTestPos,
            LTRotation rTestRot, HOBJECT* pCollisionObj=LTNULL);

        virtual void TriggerHandler(LTBOOL bTriggerLink=LTTRUE);
		virtual void TriggerClose();
        virtual void SetOpen(LTBOOL bInitialize=LTFALSE);
		virtual void SetOpening();
        virtual void SetClosed(LTBOOL bInitialize=LTFALSE);
		virtual void SetClosing();
		virtual void TriggerMsg(HOBJECT hSender, const char* szMsg);

		virtual void TriggerLink(HOBJECT hActivateObj);

		virtual void Activate(HOBJECT hObj);
		virtual void SetActivateObj(HOBJECT hObj);
		virtual void TouchNotify(HOBJECT hObj);

		virtual void HandleLinkBroken(HOBJECT hObj);

        virtual void HandleAttach(char* pObjName=LTNULL);
		virtual void HandleDetach();

        void    DetachObject(HOBJECT hObj, LTBOOL bBreakLink=LTTRUE);
        HOBJECT AttachObject(HOBJECT hObj, LTBOOL bAddToList=LTTRUE);
		void	RemoveAttachments();

        void    StartSound(HSTRING hstrSoundName, LTBOOL bLoop);
		void	StopSound();

		void	ChangeMoveDir();
        void    CalculateNewPos(LTVector & vNewPos, LTVector vFinalPos, LTFLOAT fSpeed, LTFLOAT *pPercent=NULL);

		// Set where the light animations are.
        void    SetLightAnimPos(LTBOOL bForce, LTBOOL bOnOff);
		void	ReallySetLightAnimPos(float percent);
		void	SetLightAnimRemoved();

        virtual LTBOOL   TreatLikeWorld();

		// places to keep the properties

		HSTRING m_hstrOpenSound;		// sound to play when opening starts
		HSTRING m_hstrCloseSound;		// sound to play when closing starts
		HSTRING m_hstrPortalName;		// Portal to open/close
		HSTRING	m_hstrLockedSound;		// Sound played when door is activated but locked
		HSTRING	m_hstrAttachments;		// Names of attached objects

		HSTRING m_hstrOpenCmd;			// Command to send when door is open
		HSTRING m_hstrCloseCmd;			// Command to send when door is closed
		HSTRING m_hstrLockedCmd;		// Command to send when a locked door is activated

        LTVector m_vMoveDir;             // direction to open
        LTVector m_vSoundPos;            // position to play sound (optional)
        LTVector m_vOpenPos;             // door's open position
        LTVector m_vClosedPos;           // door's closed position

        LTFLOAT  m_fSpeed;               // movement speed
        LTFLOAT  m_fMoveDist;            // distance to open
        LTFLOAT  m_fOpenWaitTime;        // length of time to stay open
        LTFLOAT  m_fCloseWaitTime;       // length of time to stay closed
        LTFLOAT  m_fClosingSpeed;        // movement speed while closing
        LTFLOAT  m_fMoveStartTime;       // Time movement started
        LTFLOAT  m_fMoveDelay;           // Time to wait to move
        LTFLOAT  m_fDoorStopTime;        // time door stopped moving
        LTFLOAT  m_fSoundRadius;         // Radius of sound

        LTBOOL   m_bBoxPhysics;          // Use box physics
        LTBOOL   m_bAITriggerable;       // AIs can trigger this baby
        LTBOOL   m_bPlayedBusySound;     // Did we play the busy sound yet?
        LTBOOL   m_bLocked;              // Is the door locked
        LTBOOL   m_bIsKeyframed;         // Is the door keyframed.
        LTBOOL   m_bFirstUpdate;         // Is this the first update
        LTBOOL   m_bRemoveAttachments;   // Remove objects or destroy objects...
        LTBOOL   m_bLoopSounds;          // Are we looping our open/close sounds?

        uint32  m_dwStateFlags;         // Property info
        uint32  m_dwDoorState;          // current door state
        uint32  m_dwWaveform;

        LTBOOL   m_bProcessTouch;        // Do we process touch notifies...

		HOBJECT	m_hActivateObj;			// Last object to activate us
        LTBOOL   m_bTestActiveCollision; // Should we test for collisions?

		ObjectList* m_pAttachmentList;	// Objects attached to us...
        LTVector m_vAttachDir;           // Direction to look for attachments
		HOBJECT m_hAttachmentObj;		// Object we attached to us

		HOBJECT	m_hDoorLink;			// Another door we are linked to
		HSTRING	m_hstrDoorLink;			// Name of the door link object

	// NOTE:  The following data members do not need to be saved / loaded
	// when saving games.  Any data members that don't need to be saved
	// should be added here (to keep them together)...

        HLTSOUND    m_sndLastSound;     // Handle of last sound playing

		CDestructibleModel	m_damage;
		CEditable			m_editable;

		// Light animations for opening/closing.
		HSTRING		m_hShadowLightsString;	// Names of shadow lights.
		HLIGHTANIM	m_hLightAnims[MAX_DOOR_LIGHT_ANIMS];
        uint32      m_nLightAnims;
        uint32      m_nLightFrames;

	private :

        LTBOOL   ReadProp(ObjectCreateStruct *pStruct);
		void	PostPropRead(ObjectCreateStruct *pStruct);
        LTBOOL   InitialUpdate(int nInfo);
        LTBOOL   Update();

        void    Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags);
        void    Load(HMESSAGEREAD hRead, uint32 dwLoadFlags);
		void	CacheFiles();

		void	PlayDoorKnobAni(char* pAniName);
};

#endif // __DOOR_H__