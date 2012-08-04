//------------------------------------------------------------------
//
//   MODULE  : FXMGR.H
//
//   PURPOSE : Defines class CClientFXMgr
//
//   CREATED : On 10/5/98 At 6:58:51 PM
//
//------------------------------------------------------------------

#ifndef __CLIENTFXMGR__H_
	#define __CLIENTFXMGR__H_

	// Includes....

	#include "iltclient.h"
	#include "basefx.h"
	#include "FxFlags.h"
	#include "FxDefs.h"

	// Forwards....
	struct CLIENTFX_INSTANCE;
	struct CLIENTFX_LINK;
	struct FX_KEY;
	struct FX_GROUP;

	struct FX_LINK
	{
		uint32								m_dwID;
		CBaseFX							   *m_pFX;
		const FX_KEY					   *m_pRef;
	};

	// Classes....
	class CClientFXMgr
	{
		public :

			// Constuctor

											CClientFXMgr();

			// Destructor
							
											~CClientFXMgr();

			// Member Functions

			bool							Init(ILTClient *pLTClient, LTBOOL bGlobal = LTTRUE);
			void							Term();

			bool							UpdateAllActiveFX(LTBOOL bAppHasFocus);
			bool							RenderAllActiveFX(LTBOOL bAppHasFocus);

			void							OnSpecialEffectNotify(HOBJECT hObject, ILTMessage_Read *pMsg);
			bool							OnObjectRemove(HOBJECT hObject);
			void							OnRendererShutdown();

			bool							CreateClientFX(CLIENTFX_LINK *pLink, const CLIENTFX_CREATESTRUCT &fxInit, LTBOOL bStartInst = LTFALSE, bool bAddNextUpdate = false);

			void							ShutdownAllFX();
			void							ShutdownClientFX(CLIENTFX_LINK* pLink);
		
			static uint32					GetUniqueID();

			//specifies whether or not gore is enabled
			void							SetGoreEnabled(bool bEnabled);

			// Accessors

			CLinkList<CLIENTFX_INSTANCE *>*	GetActiveFXList() { return &m_collActiveGroupFX; }

			void							SetDetailLevel( int nLOD );


			void							SetDisableDistances( float fLow, float fMed, float fHigh );
			void							SetDisableDistance( int nLOD, float fDistance );

			static ILTClient*				GetClientDE();

			void							UseSystemTime(bool bUseSystemTime) {m_bUseSystemTime = bUseSystemTime;}

			void							Pause(bool bPause);
			bool							IsPaused() const		{ return m_bPaused; }

			//sets the camera that will be used for this effect manager
			void							SetCamera(HOBJECT hCamera);

		private :

			//used to determine the amount of time that has elapsed for this frame
			float							GetFrameTime();

			//given an effect, it will look at the properties for the random offset, and apply
			//a series of fake updates in order to simulate offsetting the effect. This is good
			//for effects that need to start in the middle somewhere or be randomized to prevent
			//all effects looking the same
			void							ApplyEffectStartingOffset(CBaseFX* pFX, const FX_KEY* pKey);

			//Given an instance and an effect that has just finished shutting down, it will take 
			//the appropriate course of action. Note that this will invalidate the node that is passed into it
			void							HandleShutdownEffect(CLIENTFX_INSTANCE* pInst, CLinkListNode<FX_LINK>* pKeyNode);

			//Given an instance and a time interval, this will appropriately update
			//all effects contained within that interval
			void							UpdateInstanceInterval(CLIENTFX_INSTANCE* pInst, float fStartInterval, float fEndInterval);


			//creates an effect key and adds it to the specified instances list of active effects
			bool							CreateFXKey(CLIENTFX_INSTANCE* pInstance, FX_KEY* pKey);

			//Updates the suspended status of the instance and returns that status
			bool							UpdateInstanceSuspended(const LTVector& vCameraPos, CLIENTFX_INSTANCE* pInst);

			//given a detail level of an effect, this will determine if the effect key should
			//be played based upon the current LOD settings on the object
			bool							IsDetailLevelEnabled(uint32 nDetailLevel);

			// Member Functions
			CLIENTFX_INSTANCE*				CreateClientFX(const CLIENTFX_CREATESTRUCT &fxInit, LTBOOL bStartInst = LTFALSE, bool bAddNextUpdate = false);
			void							ShutdownClientFX(CLIENTFX_INSTANCE* pFxGroup);
			void							SetGroupParent(CLIENTFX_INSTANCE *pInstance, HOBJECT hParent);

			CBaseFX*						CreateFX(const char *sName, FX_BASEDATA *pBaseData, CBaseFXProps* pProps, HOBJECT hInstParent);

			void							SuspendInstance(CLIENTFX_INSTANCE *pInst);
			void							UnsuspendInstance(CLIENTFX_INSTANCE *pInst);

			//Called to setup the create effect callback so that any effects created during the updating of an
			//object will be associated with this client effect manager
			void							SetupCreateEffectCallback();			


			// Member Variables

			EFXLOD							m_eDetailLevel;
			LTFLOAT							m_dDetailDistSqr[3];

			ILTClient					   *m_pClientDE;
			static ILTClient			   *s_pClientDE;
			

			CLinkList<CLIENTFX_INSTANCE *>	m_collActiveGroupFX;

			//effects that need to be added to the active effects list at the next update
			//(this is for effects that are created in mid-update)
			CLinkList<CLIENTFX_INSTANCE *>	m_collNextUpdateGroupFX;

			//used for keeping track of the system based time, for when game time cannot be used
			bool							m_bUseSystemTime;
			uint32							m_nPrevSystemTime;
			uint32							m_nSystemTimeBase;

			//the paused status
			bool							m_bPaused;

			//is gore enabled?
			bool							m_bGoreEnabled;

			//the camera that effects can use
			HOBJECT							m_hCamera;
	};
	
	struct CLIENTFX_LINK
	{
	public:

		CLIENTFX_LINK() : m_pInstance(NULL) {}
		~CLIENTFX_LINK()	{ ClearLink(); }

		//determines if this link is connected to an object
		bool				IsValid() const			{ return m_pInstance != NULL; }

		//gets the object the link is connected to
		CLIENTFX_INSTANCE*	GetInstance()			{ return m_pInstance; }

		//clears the connenction and breaks the link
		void				ClearLink();

	private:
		//only clientfxmgr should handle setting up the links, and then we can break
		//the link from our end with clearlink
		friend class CClientFXMgr;
		
		CLIENTFX_INSTANCE* m_pInstance;

		//we cannot allow copying since that will lead to the possibility that items
		//are deleted twice
		CLIENTFX_LINK(const CLIENTFX_LINK&)					{}
		CLIENTFX_LINK& operator=(const CLIENTFX_LINK&)		{}
	};

	//a node for a linked list of CLIENTFX_LINKs
	struct CLIENTFX_LINK_NODE
	{
		CLIENTFX_LINK_NODE() : m_pNext(NULL)		{}
		~CLIENTFX_LINK_NODE()						{ DeleteList(); }

		void AddToEnd(CLIENTFX_LINK_NODE* pNode)
		{
			if(m_pNext)
				m_pNext->AddToEnd(pNode);
			else
				m_pNext = pNode;
		}

		CLIENTFX_LINK* GetElement(uint32 nElement)
		{
			if(nElement == 0)
				return &m_Link;
			if(m_pNext == NULL)
				return NULL;
			return m_pNext->GetElement(nElement - 1);
		}

		void DeleteList()
		{
			//just to avoid any possibility of cyclic deleting
			CLIENTFX_LINK_NODE* pToDel = m_pNext;
			m_pNext = NULL;

			debug_delete(pToDel);
		}

		CLIENTFX_LINK			m_Link;
		CLIENTFX_LINK_NODE		*m_pNext;
	};

	struct CLIENTFX_INSTANCE
	{
		// Member Functions

				CLIENTFX_INSTANCE();
				~CLIENTFX_INSTANCE();

		bool	ExistFX(CBaseFX *pFX);

		//are all FX inactive?
		bool	IsDone();

		//is this suspended
		bool	IsSuspended() const;


		//have all FX finished playing? (some might still be "active")
		bool	IsFinished();

		void	Hide();

		void	Show();

		void	SetPos( const LTVector &vWorldPos, const LTVector &vCamRelPos );

		void	ClearLink();

		//this will delete a single effect from its list of effects. Note that this will invalidate
		//the node, so the next pointer should have been cached previously if iterating
		void	DeleteFX(CLinkListNode<FX_LINK> *pDelNode);

		void	RemoveAllEffects();

		

		// Member Variables

		const FX_GROUP					   *m_pData;
		CLinkList<FX_LINK>					m_collActiveFX;
		float								m_tmElapsed;
		float								m_fDuration;
		float								m_tmSuspended;			//time that this object was frozen (used when unfreezing to find the delta)
		LTVector							m_vPos;
		LTRotation							m_rRot;
		uint32								m_dwID;
		HOBJECT								m_hParent;
		uint32								m_dwObjectFlags;
		uint32								m_dwObjectFlags2;
		HOBJECT								m_hTarget;
		bool								m_bLoop;
		bool								m_bSmoothShutdown;
		bool								m_bShutdown;

		bool								m_bUseTargetData;
		LTVector							m_vTargetPos;
		LTVector							m_vTargetNorm;

		HOBJECT								m_hAlternateParent;
		bool								m_bSuspended;

		LTBOOL								m_bShow;
		LTBOOL								m_bPlayerView;

		CLIENTFX_LINK*						m_pLink;
	};

	extern CClientFXMgr *g_pClientFXMgr;

#endif // __CLIENTFXMGR__H_