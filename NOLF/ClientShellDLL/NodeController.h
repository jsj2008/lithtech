// ----------------------------------------------------------------------- //
//
// MODULE  : NodeController.h
//
// PURPOSE : NodeController definition
//
// CREATED : 05.20.1999
//
// ----------------------------------------------------------------------- //

#ifndef __NODE_CONTROLLER_H__
#define __NODE_CONTROLLER_H__

#include "ModelButeMgr.h"

class CCharacterFX;

#define MAX_NODES				(32)
#define MAX_NODECONTROLS		(32)
#define MAX_RECOILS				(2)

enum Control
{
	eControlInvalid = -1,
	eControlRotationTimed,
	eControlLipSync,
	eControlScript,
	eControlHeadFollowObj,
	eControlHeadFollowPos,
};

// Our node control struct

typedef LTFLOAT (*RotationFn)(LTFLOAT fDistance, LTFLOAT fTimer, LTFLOAT fDuration);

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

		vRotationAxis.Init();
		fRotationDistance = 0.0f;
		fRotationDuration = 0.0f;
		fRotationTimer = 0.0f;
        fnRotationFunction = LTNULL;

        hFollowObj = LTNULL;
		hFollowObjNode = INVALID_MODEL_NODE;

		vFollowPos.Init();

		fFollowRate = 0.0f;
		vFollowAngles.Init();
		fFollowExpireTime = 0.0f;
        bFollowOn = LTFALSE;

        hLipSyncSound = LTNULL;
        pSixteenBitBuffer = LTNULL;
        pEightBitBuffer = LTNULL;
		dwSamplesPerSecond = 0;
		bShowingSubtitles = LTFALSE;


		fScriptTime = 0.0f;
		nScript = eModelNScriptInvalid;
		nCurrentScriptPt = 0;
		nLastScriptPt = 0;
	}

	// Members used by all control methods
    LTBOOL       bValid;         // Is the control active or not?
	ModelNode	eModelNode;		// What model node does it control?
	Control		eControl;		// What type of control is this?

	// Timed Rotation members
    LTVector     vRotationAxis;          // What axis are we rotating around?
    LTFLOAT      fRotationDistance;      // How far (in radians) will we rotate maximally?
    LTFLOAT      fRotationDuration;      // How long does the rotation last?
    LTFLOAT      fRotationTimer;         // How far into the rotation are we?
	RotationFn	fnRotationFunction;		// The function that maps time to radians

	// Head follow object rotation members
	HOBJECT		hFollowObj;
	HMODELNODE	hFollowObjNode;

	// Head follow position rotation members
    LTVector     vFollowPos;

	// General head follow members
    LTFLOAT      fFollowRate;
    LTVector     vFollowAngles;
    LTFLOAT      fFollowExpireTime;
    LTBOOL       bFollowOn;

	// Sound rotation members
    HLTSOUND    hLipSyncSound;
	int16 *		pSixteenBitBuffer;
	int8  *     pEightBitBuffer;
    uint32      dwSamplesPerSecond;
	LTBOOL		bShowingSubtitles;


	// Script members
    LTFLOAT      fScriptTime;
    uint8       nScript;
	int			nCurrentScriptPt;
	int			nLastScriptPt;
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
    LTRotation   rRot;
    LTMatrix     matTransform;
	int			cControllers;
}
NSTRUCT;

class CNodeController
{
	protected : // Protected inner classes

		class CNode;
		class CNodeControl;

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
		void	UpdateLipSyncControl(NCSTRUCT *pNodeControl);
		void	UpdateScriptControl(NCSTRUCT *pNodeControl);
		void	UpdateHeadFollowObjControl(NCSTRUCT *pNodeControl);
		void	UpdateHeadFollowPosControl(NCSTRUCT *pNodeControl);

		// Message handlers
        LTBOOL   HandleNodeControlMessage(uint8 byMsgId, HMESSAGEREAD hMessage);
		void	HandleNodeControlRecoilMessage(HMESSAGEREAD hMessage);
		void	HandleNodeControlLipSyncMessage(HMESSAGEREAD hMessage);
		void	HandleNodeControlScriptMessage(HMESSAGEREAD hMessage);
		void	HandleNodeControlHeadFollowObjMessage(HMESSAGEREAD hMessage);
		void	HandleNodeControlHeadFollowPosMessage(HMESSAGEREAD hMessage);
		void	HandleNodeConrolLipSync(HSTRING hSound, LTFLOAT fRadius);

		// Node methods
		NSTRUCT*	FindNode(HMODELNODE hModelNode);
		int			FindNodeControl(ModelNode eNode, Control eControl);

		// Node control methods
		int		AddNodeControl();
        void    AddNodeControlRotationTimed(ModelNode eModelNode, const LTVector& vAxis, LTFLOAT fDistance, LTFLOAT fDuration);

		void	RemoveNodeControl(int iNodeControl);

		// Node control callback methods
        static void NodeControlFn(HOBJECT hObj, HMODELNODE hNode, LTMatrix *pGlobalMat, void *pUserData);
        void        HandleNodeControl(HOBJECT hObj, HMODELNODE hNode, LTMatrix *pGlobalMat);

	protected : // Protected member variables

		CCharacterFX*	m_pCharacterFX;					// Back pointer to our Character FX

		int			m_cNodeControls;					// The number of active node controls
		NCSTRUCT	m_aNodeControls[MAX_NODECONTROLS];	// The node controls
		int			m_cNodes;							// The number of nodes
		NSTRUCT		m_aNodes[MAX_NODES];				// The nodes

		int			m_cRecoils;							// How many recoils we're controlling
        LTFLOAT      m_fRecoilTimers[MAX_RECOILS];       // Our recoil timers

        LTFLOAT      m_fCurLipSyncRot;                   // Current lip-sync node rotation
        LTBOOL       m_bOpeningMouth;                    // Is mouth opening or closing?
};

#endif