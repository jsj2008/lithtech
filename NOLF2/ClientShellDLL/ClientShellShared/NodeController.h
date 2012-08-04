// ----------------------------------------------------------------------- //
//
// MODULE  : NodeController.h
//
// PURPOSE : Model node control
//
// CREATED : 1997
//
// (c) 1997-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __NODE_CONTROLLER_H__
#define __NODE_CONTROLLER_H__

#include "ModelButeMgr.h"

class CCharacterFX;

enum Control
{
	eControlInvalid = -1,
	eControlLipFlap,
	eControlLipSync,
	eControlHeadFollowObj,
};

//structure to hold a single keyframe for the lip syncing. Note that the layout of this
//is very important since it must match the layout in which the keys are saved out for 
//very fast loading and saving. Changing this structure requires changing the lipcompiler
struct NCKEYFRAME
{
	LTVector vTranslation;
	LTRotation rRotation;
};

typedef struct NCSTRUCT
{
	NCSTRUCT()
	{
		Reset();
	}

	inline void Reset()
	{
        bValid = LTFALSE;
		eModelNode = eModelNodeInvalid;
		eControl = eControlInvalid;

		fTimer = 0.0f;

		cKeyframes = 0;
		aKeyframes = LTNULL;

        hFollowObj = LTNULL;
		hFollowObjNode = INVALID_MODEL_NODE;
		vFollowPos.Init();
		fFollowRate = 0.0f;
		vFollowAngles.Init();
		fFollowExpireTime = 0.0f;
        bFollowOn = LTFALSE;
	}

	// Members used by all control methods
    LTBOOL		bValid;
	ModelNode	eModelNode;
	Control		eControl;

	// Lip sync members
	LTFLOAT		fTimer;
	int32		cKeyframes;
	NCKEYFRAME*	aKeyframes;

	// Head follow object rotation members
	HOBJECT		hFollowObj;
	HMODELNODE	hFollowObjNode;

	// Head follow position rotation members
    LTVector	vFollowPos;

	// General head follow members
    LTFLOAT		fFollowRate;
    LTVector	vFollowAngles;
    LTFLOAT		fFollowExpireTime;
    LTBOOL		bFollowOn;
}
NCSTRUCT;

// Our node struct

typedef struct NSTRUCT
{
	NSTRUCT()
	{
		hModelNode = INVALID_MODEL_NODE;
		eModelNode = eModelNodeInvalid;
        rRot.Init();
		matTransform.Identity();
		cControllers = 0;
	}

	HMODELNODE	hModelNode;
	ModelNode	eModelNode;
    LTRotation  rRot;
    LTMatrix    matTransform;
	int			cControllers;
}
NSTRUCT;

class CNodeController
{
	public : // Public methods

		// Ctors/dtors/etc
		CNodeController();
		~CNodeController();

        LTBOOL Init(CCharacterFX* pCFX);

		// Simple accessors
		CCharacterFX*	GetCFX() const { return m_pCharacterFX; }

		LTBOOL	IsLipSynching();

		// Updates
		void	Update();
		void	UpdateRotationTimed(NCSTRUCT *pNodeControl);
		void	UpdateLipFlapControl(NCSTRUCT *pNodeControl);
		void	UpdateLipSyncControl(NCSTRUCT *pNodeControl);

		// Message handlers
        LTBOOL	HandleNodeControlMessage(uint8 byMsgId, ILTMessage_Read *pMsg);
		void	HandleNodeControlLipFlap( char const* pszSoundFile, LTFLOAT fRadius);
		void	HandleNodeControlLipSyncMessage(ILTMessage_Read *pMsg);
		void	HandleNodeControlLipSync( char const* pszSoundFile, LTFLOAT fRadius);

		// Node methods
		NSTRUCT*	FindNode(HMODELNODE hModelNode);
		int			FindNodeControl(ModelNode eNode, Control eControl);

		// Node control methods
		int		AddNodeControl();
        void    AddNodeControlRotationTimed(ModelNode eModelNode, const LTVector& vAxis, LTFLOAT fDistance, LTFLOAT fDuration);

		void	RemoveNodeControl(int iNodeControl);

		// Node control callback methods
        static void NodeControlFn(const NodeControlData& Data, void *pUserData);
        void        HandleNodeControl(HOBJECT hObj, HMODELNODE hNode, LTMatrix *pGlobalMat);

		void	UpdateHeadFollowObjControl(NCSTRUCT *pNodeControl);
		void	HandleNodeControlHeadFollowObjMessage(ILTMessage_Read *pMsg);

		void	ResetSoundBufferData()
		{
			m_pSixteenBitBuffer = LTNULL;
			m_pEightBitBuffer = LTNULL;
			m_dwSamplesPerSecond = 0;
		}

	protected :

		enum Constants
		{
			kMaxNodes			= 40,
			kMaxNodeControls	= 32,
		};

	protected :

		void			KillLipSyncSound( bool bSendNotification );

		//flag indicating whether or not the node control function was installed
		bool			m_bAddedNodeControlFn;

		CCharacterFX*	m_pCharacterFX;

		int				m_cNodeControls;
		NCSTRUCT		m_aNodeControls[kMaxNodeControls];

		NSTRUCT			m_aNodes[kMaxNodes];

        LTFLOAT			m_fCurLipFlapRot;

		// Lip flap members
		HLTSOUND		m_hSound;
		uint8			m_nUniqueDialogueId;
		bool			m_bSubtitlePriority;
		int16*			m_pSixteenBitBuffer;
		int8*			m_pEightBitBuffer;
		uint32			m_dwSamplesPerSecond;
		LTBOOL			m_bShowingSubtitles;

	private :

		void CleanUpLipSyncNodeControls();
};

#endif