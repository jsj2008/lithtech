//------------------------------------------------------------------
//
//   MODULE  : BASEFX.H
//
//   PURPOSE : Defines class CBaseFX
//
//   CREATED : On 10/6/98 At 2:57:18 PM
//
//------------------------------------------------------------------

#ifndef __BASEFX_H_
	#define __BASEFX_H_

	// Includes....

	#include "ltbasedefs.h"
	#include "ltbasetypes.h"
	#include "iclientshell.h"
	#include "iltcommon.h"
	#include "iltclient.h"
	#include "iltmodel.h"
	#include "ltobjref.h"
	#include "FastList.h"
	#include "LinkList.h"
	#include "FXProp.h"

	#include <stdio.h>
	

	// Defines....

	// FX state defines
	
	#define FS_INITIALFRAME					0x00000001		//first frame after being activated, can be set with active
	#define FS_ACTIVE						0x00000002		//we are active
	#define FS_SHUTTINGDOWN					0x00000004		//user wants to shut down?
	#define FS_SUSPENDED					0x00000008		//we are temporarily suspended (don't update)

	// FX follow defines

	#define UP_FIXED						0
	#define UP_FOLLOW						1
	#define UP_PLAYERVIEW					2
	#define UP_NODEATTACH					3
	#define	UP_SOCKETATTACH					4
	#define	UP_PVNODEATTACH					5
	#define	UP_PVSOCKETATTACH				6

	// FX property strings

	#define FXPROP_UPDATEPOS				"UpdatePos"
	#define	FXPROP_ATTACHNAME				"AttachName"
	#define	FXPROP_OFFSET					"Offset"
	#define	FXPROP_ROTATEADD				"RotateAdd"
	#define FXPROP_DISABLEATDIST			"DisableAtDist"
	#define FXPROP_MENULAYER				"MenuLayer"
	#define FXPROP_MAXSTARTOFFSET			"MaxStartOffset"
	#define FXPROP_RANDOMSTARTOFFSET		"RandomizeStartOffset"
	#define FXPROP_STARTOFFSETINTERVAL		"StartOffsetInterval"
	#define FXPROP_SMOOTHSHUTDOWN			"SmoothShutDown"
	#define FXPROP_DETAILLEVEL				"DetailLevel"
	#define FXPROP_ISGORE					"IsGore"

	// FX key requirements
	
	#define FX_NEEDCOLOURKEY				0x00000001
	#define FX_NEEDSCALEKEY					0x00000002
	#define FX_NEEDMOTIONKEY				0x00000004
	#define FX_NEEDVOLUMEKEY				0x00000008	
	#define FX_NEEDOBJECT					0x00000010
	#define FX_NEEDCAMERA					0x00000020

	// Detail level settings
	#define	FX_NUM_DETAIL_SETTINGS			7

	// FX LOD
	
	enum EFXLOD
	{
		FXLOD_LOW,
		FXLOD_MED,
		FXLOD_HIGH	
	};

	// Forwards....

	class CBaseFX;
	class CBaseFXProps;

	struct FX_PARAMS;
	struct CLIENTFX_CREATESTRUCT;

	// Type defines....

	//function prototype for a hook so that objects can be created inside of ClientFX itself
	typedef bool (*TCreateClientFXFn)(const CLIENTFX_CREATESTRUCT& CreateInfo, bool bStartInst, void* pUserData);


	//DLL export functions
	typedef int				(*FX_NUMFUNC)();
	typedef char*			(*FX_NAMEFUNC)(int);
	typedef CBaseFX*		(*FX_CREATEFUNC)();
	typedef void			(*FX_DELETEFUNC)(CBaseFX *);
	typedef void			(*FX_PROPFUNC)(CFastList<FX_PROP> *);
	typedef void			(*FX_SETPLAYERFUNC)(HOBJECT hPlayer);
	typedef void			(*FX_SETAPPFOCUS)(bool bAppFocus);
	typedef void			(*FX_SETCREATEFUNCTION)(TCreateClientFXFn pFn, void* pUserData);
	typedef CBaseFXProps*	(*FX_CREATEPROPLIST)(int nFx);
	typedef void			(*FX_FREEPROPLIST)(CBaseFXProps* pPropList);

	// Structures....

	// FX function interface structure
	
	struct FX_REF
	{
		// Member Variables
		
		char								m_sName[128];
		uint32								m_dwType;
		FX_CREATEFUNC						m_pfnCreate;
		FX_DELETEFUNC						m_pfnDelete;
		FX_PROPFUNC							m_pfnGetProps;
	};
	
	struct FX_SCALEKEY
	{
		float								m_tmKey;
		float								m_scale;
	};

	struct FX_COLOURKEY
	{
		bool operator == (FX_COLOURKEY k)		{
												return (m_red == k.m_red) && 
													   (m_green == k.m_green) && 
													   (m_blue == k.m_blue) &&
													   (m_alpha == k.m_alpha);
											}
		
		float								m_tmKey;
		float								m_red;
		float								m_green;
		float								m_blue;
		float								m_alpha;
	};



	// FX Base data structure, all FX need these
	struct FX_BASEDATA
	{
											// Constructor

											FX_BASEDATA()
											{
												memset(this, 0, sizeof(FX_BASEDATA));
											}
		
		uint32			 					m_dwID;
		uint32								m_dwFlags;
		uint32								m_dwObjectFlags;
		uint32								m_dwObjectFlags2;
		LTVector							m_vPos;
		LTRotation							m_rRot;

		bool								m_bUseTargetData;
		LTVector							m_vTargetPos;
		LTVector							m_vTargetNorm;
		
		HOBJECT								m_hParent;
		HOBJECT								m_hTarget;
		HOBJECT								m_hCamera;
		char							    m_sNode[32];
	};

	// structure that needs to be filled out in order to create a new effect
	struct CLIENTFX_CREATESTRUCT
	{
											CLIENTFX_CREATESTRUCT()
											{
												memset(this, 0, sizeof(CLIENTFX_CREATESTRUCT));
											}

											CLIENTFX_CREATESTRUCT(const char *sName, uint32 dwFlags, LTVector &vPos)
											{
												memset(this, 0, sizeof(CLIENTFX_CREATESTRUCT));

												LTStrCpy(m_sName, sName, sizeof(m_sName));
												m_dwFlags    = dwFlags;
												m_vPos       = vPos;
												m_rRot.Init();
											}

											CLIENTFX_CREATESTRUCT(const char *sName, uint32 dwFlags, LTVector &vPos, LTRotation &rRot)
											{
												memset(this, 0, sizeof(CLIENTFX_CREATESTRUCT));

												LTStrCpy(m_sName, sName, sizeof(m_sName));
												m_dwFlags    = dwFlags;
												m_vPos       = vPos;
												m_rRot		 = rRot;
											}

											CLIENTFX_CREATESTRUCT(const char *sName, uint32 dwFlags, HOBJECT hParent)
											{
												memset(this, 0, sizeof(CLIENTFX_CREATESTRUCT));

												LTStrCpy(m_sName, sName, sizeof(m_sName));
												m_dwFlags    = dwFlags;
												m_hParent	 = hParent;
											}

		char								m_sName[32];
		uint32								m_dwFlags;
		LTVector							m_vPos;
		LTRotation							m_rRot;
	
		HOBJECT								m_hParent;
		char							    m_sParentNode[32];
		HOBJECT								m_hTarget;

		bool								m_bUseTargetData;
		LTVector							m_vTargetPos;
		LTVector							m_vTargetNorm;
	};

	// Classes....

	class CBaseFXProps
	{
	public:

		CBaseFXProps();
		virtual ~CBaseFXProps();

		//sets up parameters for the effects lifetime
		virtual void					SetLifetime(float fLifespan, uint32 nNumRepeats);

		//this will take a list of properties and convert it to internal values
		virtual bool					ParseProperties(FX_PROP* pProps, uint32 nNumProps);

		//Fields that control the lifetime of an effect. This must be setup before properties
		//are parsed

		//the length of the effect including all repititions
		float							m_tmActualEnd;

		//the length of a single repitition of an effect
		float							m_tmLifespan;


		//Fields that are loaded in from the property lists
		//-------------------------------------------------
		FX_COLOURKEY					*m_pColorKeys;
		uint32							m_nNumColorKeys;

		FX_SCALEKEY						*m_pScaleKeys;
		uint32							m_nNumScaleKeys;

		uint32							m_nMenuLayer;
		uint32							m_nFollowType;

		char						    m_szAttach[32];

		LTVector						m_vOffset;
		LTVector						m_vRotAdd;
	};


	class CBaseFX
	{
		public :

			enum FXType
			{
				eBaseFX,
				eParticleSystemFX,
				eSpriteFX,
				eLTBModelFX,
				eDynaLightFX,
				ePlaySoundFX,
				eCamJitterFX,
				eCamWobbleFX,
				eLTBBouncyChunkFX,
				eNullFX,
				ePolyTubeFX,
				ePlayRandomSoundFX,
				eSpriteSystem,
				eCreateFX,
				eFlareSpriteFX,
				eLightningFX,
			};
			
											CBaseFX( FXType nType = eBaseFX );
			virtual 						~CBaseFX()	{}

			//intializes the effect based upon the passed in data
			virtual bool					Init(ILTClient *pLTClient, FX_BASEDATA *pData, const CBaseFXProps *pProps);

			//terminates the effect
			virtual void					Term() = 0;

			//allows the effect to pause anything it needs to
			virtual void					Pause(bool bPause);

			//called to update the effect
			virtual bool					Update(float tmFrameTime);

			//This version of update is called while the effect is suspended so that it can do
			//things like smooth shutdown depending upon the effect type
			virtual bool					SuspendedUpdate(float tmFrameTime);

			//This is called while an effect is suspended to determine whether or not it should
			//be visible
			virtual bool					IsVisibleWhileSuspended()		{ return false; }

			//called to render the effect. Used only for effects that use draw prim
			virtual bool					Render();

			//called to determine if the object has completed shutting down
			virtual bool					IsFinishedShuttingDown()	{ return true; }

			//called when the renderer is changing
			virtual void					OnRendererShutdown() { }

			//calculates the color based upon the color keys and passed in timings
			void							CalcColour(float tmCur, float tmLifespan, float *pRed, float *pGreen, float *pBlue, float *pAlpha, uint32* pHint = NULL);

			//calculates the scale based upon the scale keys and passed in timings
			void							CalcScale(float tmCur, float tmLifespan, float *pScale, uint32* pHint = NULL);
			
			//handles setting and clearing state flags (bit mask operations)
			void							SetState(uint32 nState)			{ m_dwState |= nState; }
			void							ClearState(uint32 nState)		{ m_dwState &= ~nState; }
			bool							IsStateSet(uint32 nState) const { return !!(m_dwState & nState); }

			//Utility state detection functions
			bool							IsSuspended() const				{ return IsStateSet(FS_SUSPENDED); }
			bool							IsShuttingDown() const			{ return IsStateSet(FS_SHUTTINGDOWN); }
			bool							IsActive() const				{ return IsStateSet(FS_ACTIVE); }
			bool							IsInitialFrame() const			{ return IsStateSet(FS_INITIALFRAME); }

			//toggles visibility of this object
			void							SetVisible(bool bVisible)		{ m_pLTClient->Common()->SetObjectFlags(m_hObject, OFT_Flags, bVisible ? FLAG_VISIBLE : 0, FLAG_VISIBLE); }

			// Accessors
			HOBJECT							GetFXObject()	const			{ return m_hObject; }
			HOBJECT							GetParent()		const			{ return m_hParent; }
			FXType							GetFXType()		const			{ return m_nFXType; }
			uint32							GetID()			const			{ return m_dwID; }
			uint8							GetMenuLayer()	const			{ return (m_pProps) ? m_pProps->m_nMenuLayer : 0; }


			void							SetElapsed(float fElapsed)		{ m_tmElapsed = fElapsed; }
			float							GetElapsed()	const			{ return m_tmElapsed; }
			float							GetLifespan()					{ return GetProps()->m_tmLifespan; }
			float							GetEndTime()					{ return GetProps()->m_tmActualEnd; }

			void							SetParent(HOBJECT hObj)			{ m_hParent = hObj; }
			void							SetCamera(HOBJECT hCam)			{ m_hCamera = hCam; }

			void							SetPos(const LTVector &vPos )	{ m_vPos = vPos; }

			ILTClient						*m_pLTClient;

		protected :

			// Member Functions

			void							CreateDummyObject();
			const CBaseFXProps*				GetProps() { assert(m_pProps); return m_pProps; }

			// Member Variables

			HOBJECT							m_hObject;
			HOBJECT							m_hParent;
			HOBJECT							m_hCamera;

			uint32							m_dwID;
			uint32							m_dwState;
			FXType							m_nFXType;

			//the current amount of time within the effect (between 0..m_tmActualEnd, but
			//can be more if doing a smooth shutdown)
			float							m_tmElapsed;

			float							m_red;
			float							m_green;
			float							m_blue;
			float							m_alpha;
			float							m_scale;

			LTVector						m_vCreatePos;
			LTRotation						m_rCreateRot;
			LTVector						m_vPos;

			bool							m_bUpdateColour;
			bool							m_bUpdateScale;

			uint32							m_nCurrColorKey;			
			uint32							m_nCurrScaleKey;

			const CBaseFXProps				*m_pProps;

		private:

			CBaseFX( const CBaseFX &SourceFX );
			CBaseFX& operator = ( const CBaseFX &rhs);
	};

#endif