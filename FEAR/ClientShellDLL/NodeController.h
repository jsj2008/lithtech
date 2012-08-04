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


class CCharacterFX;

// Our node struct

typedef struct NSTRUCT
{
	NSTRUCT()
	{
		hModelNode = INVALID_MODEL_NODE;
		hNode = NULL;
		rRot.Init();
		matTransform.Identity();
		cControllers = 0;
	}

	HMODELNODE	hModelNode;
	ModelsDB::HNODE	hNode;
	LTRotation  rRot;
	LTMatrix    matTransform;
	int			cControllers;
}
NSTRUCT;


enum Control
{
	eControlInvalid = -1,
	eControlLipFlap,
	eControlLipSync,
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
        bValid = false;
		pNStruct = NULL;
		eControl = eControlInvalid;

		fTimer = 0.0f;

		cKeyframes = 0;
		aKeyframes = NULL;

        hFollowObj = NULL;
		hFollowObjNode = INVALID_MODEL_NODE;
		vFollowPos.Init();
		fFollowRate = 0.0f;
		vFollowAngles.Init();
		fFollowExpireTime = 0.0f;
        bFollowOn = false;
	}

	// Members used by all control methods
    bool		bValid;
	NSTRUCT*	pNStruct;
	Control		eControl;

	// Lip sync members
	float		fTimer;
	int32		cKeyframes;
	NCKEYFRAME*	aKeyframes;

	// Head follow object rotation members
	LTObjRef	hFollowObj;
	HMODELNODE	hFollowObjNode;

	// Head follow position rotation members
    LTVector	vFollowPos;

	// General head follow members
    float		fFollowRate;
    LTVector	vFollowAngles;
    float		fFollowExpireTime;
    bool		bFollowOn;
}
NCSTRUCT;

class CNodeController
{
	public : // Public methods

		// Ctors/dtors/etc
		CNodeController();
		~CNodeController();

        bool Init(CCharacterFX* pCFX);

		// Simple accessors
		CCharacterFX*	GetCFX() const { return m_pCharacterFX; }

		bool	IsLipSynching();

		// Updates
		void	Update();
		void	UpdateRotationTimed(NCSTRUCT *pNodeControl);
		void	UpdateLipFlapControl(NCSTRUCT *pNodeControl);
		void	UpdateLipSyncControl(NCSTRUCT *pNodeControl);

		// Message handlers
        bool	HandleNodeControlMessage(uint8 byMsgId, ILTMessage_Read *pMsg);
		void	HandleNodeControlLipFlap( char const* pszSoundFile, float fOuterRadius, float fInnerRadius, int16 nMixChannel, const char *szIcon);
		void	HandleNodeControlLipSyncMessage(ILTMessage_Read *pMsg);
		void	HandleNodeControlLipSync( char const* pszSoundFile, float fOuterRadius, float fInnerRadius, int16 nMixChannel, const char *szIcon);

		// Node methods
		NSTRUCT*	FindNode(HMODELNODE hModelNode);
		NSTRUCT*	FindNode(ModelsDB::HNODE hNode);
		int			FindNodeControl(ModelsDB::HNODE hNode, Control eControl);

		// Node control methods
		int		AddNodeControl();

		void	RemoveNodeControl(int iNodeControl);

		// Node control callback methods
        static void NodeControlFn(const NodeControlData& Data, void *pUserData);
        void        HandleNodeControl(HOBJECT hObj, HMODELNODE hNode, LTMatrix *pGlobalMat);

		void	ResetSoundBufferData()
		{
			m_pSixteenBitBuffer = NULL;
			m_pEightBitBuffer = NULL;
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

		int				m_cNodes;
		NSTRUCT			m_aNodes[kMaxNodes];

        float			m_fCurLipFlapRot;
		bool			m_bUseRadioSound;

		// Lip flap members
		HLTSOUND		m_hSound;
		HLTSOUND		m_hRadioSound;
		uint8			m_nUniqueDialogueId;
		bool			m_bSubtitlePriority;
		int16*			m_pSixteenBitBuffer;
		int8*			m_pEightBitBuffer;
		uint32			m_dwSamplesPerSecond;
		bool			m_bShowingSubtitles;
		std::string		m_sDialogueIcon;

	private :

		void CleanUpLipSyncNodeControls();
};

#endif