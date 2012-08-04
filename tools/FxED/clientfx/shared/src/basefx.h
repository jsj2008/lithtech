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
	#include "iclientshell.h"
	#include "iltclient.h"
	#include "iltphysics.h"
	#include "iltmodel.h"
	#include "ltbasetypes.h"
	#include "FastList.h"
	#include "LinkList.h"
	#include "iltcommon.h"
	#include "ltvector.h"
	#include "iltmath.h"
//	#include "ltcompat.h"
	#include "ltobjectcreate.h"

	// Defines....

//	typedef CClientDE ILTClient;

	// FX loop defines
	
	#define FX_PLAYONCE						1
	#define FX_LOOP							2

	// FX colour fade-off defines
	
	#define FT_CONTINUAL					0x00000001
	#define FT_ENDFADE						0x00000002

	// FX state defines
	
	#define FS_INACTIVE						0x00000004
	#define FS_PLAYING						0x00000008
	#define FS_SHUTDOWN						0x00000010

	// FX follow defines

	#define UP_FIXED						0
	#define UP_FOLLOW						1
	#define UP_LEFTHAND						2
	#define UP_RIGHTHAND					3
	#define UP_LEFTFOOT						4
	#define UP_RIGHTFOOT					5
	#define UP_HEAD							6
	#define UP_TAIL							7
	#define UP_U1							8
	#define UP_U2							9
	#define UP_U3							10
	#define UP_U4							11
	#define UP_U5							12
	#define UP_U6							13
	#define UP_U7							14
	#define UP_U8							15
	#define UP_U9							16
	#define UP_U10							17

	// FX random generation defines

	#define RG_NONE							0
	#define RG_BOUNDINGBOX					1

	// FX key requirements
	
	#define FX_NEEDCOLOURKEY				0x00000001
	#define FX_NEEDSCALEKEY					0x00000002
	#define FX_NEEDMOTIONKEY				0x00000004
	#define FX_NEEDVOLUMEKEY				0x00000008	
	#define FX_NEEDOBJECT					0x00000010
	#define FX_NEEDCAMERA					0x00000020

	// FX interpolation defines

	#define IT_LINEAR						0x00000001
	#define IT_SPLINE						0x00000002

	// Forwards....

	class CBaseFX;

	struct FX_PROP;
	struct FX_PARAMS;

	// Type defines....

	typedef int (*FX_NUMFUNC)();
	typedef char* (*FX_NAMEFUNC)(int);
	typedef CBaseFX* (*FX_CREATEFUNC)();
	typedef void (*FX_DELETEFUNC)(CBaseFX *);
	typedef void (*FX_PROPFUNC)(CFastList<FX_PROP> *);
	typedef void (*FX_SETMULTIPLAYFUNC)(bool bMultiplay);
	typedef void (*FX_DETAILFUNC)(float fDetail);
	typedef void (*FX_SETPLAYERFUNC)(HOBJECT hPlayer);
	typedef void (*FX_SETPARAMS)(FX_PARAMS *pParams);

	// Structures....

	struct FX_PARAMS
	{
		bool			m_bAppFocus;
	};

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
	
	// Individual piece of data for an FX
	
	struct FX_PROP
	{
		struct FX_CLRKEY
		{
			float							m_tmKey;
			uint32							m_dwCol;
		};

		// Enumerations

		enum eDataType
		{
			STRING,
			INTEGER,
			FLOAT,
			COMBO,
			VECTOR,
			VECTOR4,
			CLRKEY,
			PATH,
			ENUM
		};

		// Constructor

											FX_PROP()
											{
												memset(this, 0, sizeof(FX_PROP));
											}

		// Member Functions

		void								Enum(char *sName, int value)
											{
												strcpy(m_sName, sName);
												m_nType = eDataType::ENUM;
												m_data.m_nVal = value;
											}
		
		void								Int(char *sName, int value) 
											{ 
												strcpy(m_sName, sName);
												m_nType = eDataType::INTEGER;
												m_data.m_nVal = value;
											}

		void								Float(char *sName, float value) 
											{ 
												strcpy(m_sName, sName);
												m_nType = eDataType::FLOAT;
												m_data.m_fVal = value;
											}

		void								String(char *sName, char *sString) 
											{ 
												strcpy(m_sName, sName);
												m_nType = eDataType::STRING;												
												strcpy(m_data.m_sVal, sString);
											}

		void								Combo(char *sName, char *sString) 
											{ 
												strcpy(m_sName, sName);
												m_nType = eDataType::COMBO;
												strcpy(m_data.m_sVal, sString);
											}

		void								Vector(char *sName, float *pfVec) 
											{ 
												strcpy(m_sName, sName);
												m_nType = eDataType::VECTOR;
												m_data.m_fVec[0] = pfVec[0];
												m_data.m_fVec[1] = pfVec[1];
												m_data.m_fVec[2] = pfVec[2];
											}

		void								Vector4(char *sName, float *pfVec4) 
											{ 
												strcpy(m_sName, sName);
												m_nType = eDataType::VECTOR;
												m_data.m_fVec[0] = pfVec4[0];
												m_data.m_fVec[1] = pfVec4[1];
												m_data.m_fVec[2] = pfVec4[2];
												m_data.m_fVec[3] = pfVec4[3];
											}

		void								ClrKey(char *sName, float tmKey, uint32 dwCol)
											{
												strcpy(m_sName, sName);
												m_nType = eDataType::CLRKEY;
												m_data.m_clrKey.m_tmKey = tmKey;
												m_data.m_clrKey.m_dwCol = dwCol;
											}

		void								Path(char *sName, char *sString) 
											{ 
												strcpy(m_sName, sName);
												m_nType = eDataType::PATH;
												strcpy(m_data.m_sVal, sString);
											}

		int									GetComboVal()
											{
												char sTmp[128];
												strcpy(sTmp, m_data.m_sVal);
												char *sVal = strtok(sTmp, ",");
												int nVal = atoi(sVal);
												return nVal;
											}
													
		// Member Variables


		char								m_sName[128];
		eDataType							m_nType;
		union								FX_DATATYPE
		{
			char							m_sVal[128];
			int								m_nVal;
			float							m_fVal;
			float							m_fVec[3];
			float							m_fVec4[4];
			FX_CLRKEY						m_clrKey;
		}									m_data;
	};
	
	// FX Base data structure, all FX need these

	struct FX_BASEDATA
	{
											// Constructor

											FX_BASEDATA()
											{
												memset(this, 0, sizeof(FX_BASEDATA));
												m_dwReps = 1;
											}
		
		uint32								m_dwGroupID;
		uint32			 					m_dwID;
		uint32								m_dwType;
		uint32								m_dwFlags;
		float								m_tmStart;
		float								m_tmEnd;
		uint32								m_dwReps;
		LTVector								m_vPos;
		LTVector								m_vScale;

		bool								m_bUseTargetData;
		LTVector								m_vTargetPos;
		LTVector								m_vTargetNorm;
		
		HLOCALOBJ							m_hParent;
		HLOCALOBJ							m_hTarget;
		HLOCALOBJ							m_hCamera;
		char							    m_sNode[32];
	};

	struct FX_MOVEKEY
	{
		float								m_tmKey;
		LTVector								m_vPos;
	};

	struct FX_ROTATIONKEY
	{
		float								m_tmKey;
		LTRotation							m_vRot;
	};

	struct FX_SCALEKEY
	{
		float								m_tmKey;
		float								m_scale;
	};

	struct FX_COLOURKEY
	{
		operator == (FX_COLOURKEY k)		{
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

	// Classes....

	class CBaseFX
	{
		public :
			
			// Constructor

											CBaseFX() 
											{
												memset(this, 0, sizeof(CBaseFX));

												m_pClientDE		 = NULL;
												m_hObject		 = NULL;
												m_hParent		 = NULL;
												m_hTarget		 = NULL;												
												m_dwState		 = FS_INACTIVE;
												m_dwType		 = 0;
												m_tmFade		 = 1.0f;
												m_nAlpha		 = 0;
												m_bInterpolate   = true;
												m_bUpdateColour  = true;
												m_bUpdateScale   = true;
												m_dwFollowType   = UP_FIXED;
												m_bFrozen		 = false;
												m_tmFreeze		 = 0.0f;
												m_fDetailSetting = 1.0f;

												m_vScale.Init(1.0f, 1.0f, 1.0f);												
												m_vVel.Init(0.0f, 200.0f, 0.0f);
												m_vAcc.Init(0.0f, -60.0f, 0.0f);
												m_vMotionOffset.Init(0.0f, 0.0f, 0.0f);
											}

			// Destructor

			virtual 								~CBaseFX() { }

			// Member Functions

			virtual bool					Init(ILTClient *pClientDE, FX_BASEDATA *pData, CLinkList<FX_PROP> *pProps);
			virtual void					Term() = 0;
			virtual bool					PreUpdate(float tmCur) { return true; };
			virtual bool					Update(float tmCur);
			virtual bool					IsFinished() { return true; }
			void							Shutdown();
			virtual void					OnRendererShutdown() { }

			virtual void					Freeze(float tmFreeze)
											{
												m_tmFreeze = tmFreeze;
												m_bFrozen  = true;
											}

			virtual void					Unfreeze(float tmUnfreeze)
											{												
												if (m_bFrozen)
												{
													float tmFreezeDelta = tmUnfreeze - m_tmFreeze;

													m_tmStart += tmFreezeDelta;
													m_tmEnd += tmFreezeDelta;
													m_tmActualEnd += tmFreezeDelta;
													m_tmBaseLastUpdate += tmFreezeDelta;
													m_tmLastUpdate += tmFreezeDelta;

													m_bFrozen = false;
												}
											}


			void							CalcColour(float tmStart, float tmCur, float tmLifespan, float *pRed, float *pGreen, float *pBlue, float *pAlpha);
			void							CalcScale(float tmStart, float tmCur, float tmLifespan, float *pScale);
			
			bool							HasExpired(float tmCur);

			virtual void					SetDetail(float fDetail) { }

			// Accessors

			HOBJECT							GetFXObject() { return m_hObject; }
			HOBJECT							GetParent() { return m_hParent; }
			uint32							GetType() { return m_dwType; }
			float							GetStartTime() { return m_tmStart; }
			float							GetLifespan() { return m_tmLifespan; }
			uint32							GetGroupID() { return m_dwGroupID; }
			uint32							GetID() { return m_dwID; }

			void							SetParent(HOBJECT hObj) { m_hParent = hObj; }
			void							SetCamera(HOBJECT hCam) { m_hCamera = hCam; }

		protected :

			// Member Functions

			void							CreateDummyObject()
											{
												ObjectCreateStruct ocs;
												INIT_OBJECTCREATESTRUCT(ocs);

												LTVector vScale;
												vScale.x = 1.0f;
												vScale.y = 1.0f;
												vScale.z = 1.0f;
												ocs.m_ObjectType = OT_NORMAL;
												ocs.m_Pos = m_vCreate;
												ocs.m_Scale = vScale;

												if (!m_hObject) m_hObject = m_pClientDE->CreateObject(&ocs);
											}

			// Member Variables

			ILTClient					   *m_pClientDE;

			HLOCALOBJ						m_hObject;
			HLOCALOBJ						m_hParent;
			HLOCALOBJ						m_hTarget;
			HLOCALOBJ						m_hCamera;

			bool							m_bUseTargetData;
			LTVector						m_vTargetPos;
			LTVector						m_vTargetNorm;

			uint32							m_dwGroupID;
			uint32							m_dwID;
			uint32							m_dwState;
			uint32							m_dwType;
			uint32							m_dwReps;
			uint32							m_dwRandGenType;

			char						    m_sNode[32];
			float							m_tmStart;
			float							m_tmEnd;
			float							m_tmActualEnd;
			float							m_tmLifespan;
			float							m_tmFade;
			float							m_tmBaseLastUpdate;
			float							m_tmLastUpdate;
			float							m_tmFreeze;
			bool							m_bFrozen;
			
			float							m_red;
			float							m_green;
			float							m_blue;
			float							m_alpha;
			int								m_nAlpha;
			float							m_scale;

			LTVector						m_vCreate;
			LTVector						m_vOffset;
			LTVector						m_vPos;
			LTVector						m_vScale;
			LTVector						m_vLastPos;
			LTVector						m_vMotionOffset;
			LTVector						m_vRandOffset;
			LTVector						m_vRotAdd;

			LTVector						m_vVel;
			LTVector						m_vAcc;
	
			LTVector						m_vInitialRight;
			LTVector						m_vInitialUp;
			LTVector						m_vInitialForward;

			bool							m_bInterpolate;
			bool							m_bUpdateColour;
			bool							m_bUpdateScale;

			uint32							m_dwFollowType;
			
			CLinkList<FX_COLOURKEY>			m_collColourKeys;
			CLinkListNode<FX_COLOURKEY>	   *m_pCurColour;

			CLinkList<FX_SCALEKEY>			m_collScaleKeys;
			CLinkListNode<FX_SCALEKEY>	   *m_pCurScale;

			CLinkList<FX_MOVEKEY>			m_collMoveKeys;
			CLinkListNode<FX_MOVEKEY>	   *m_pCurMove;

			float							m_fDetailSetting;
	};

	// Inline functions

	//------------------------------------------------------------------
	//
	//   FUNCTION : GetRandom()
	//
	//   PURPOSE  : Useful floating point random number generator
	//
	//------------------------------------------------------------------

	inline float fxGetRandom(float min, float max)
	{
		float randNum = (float)rand() / RAND_MAX;
		float num = min + (max - min) * randNum;
		return num;
	}

	//------------------------------------------------------------------
	//
	//   FUNCTION : Init()
	//
	//   PURPOSE  : Initialises base class CBaseFX
	//
	//------------------------------------------------------------------

	inline bool CBaseFX::Init(ILTClient *pClientDE, FX_BASEDATA *pData, CLinkList<FX_PROP> *pProps)
	{
		// Store the client DE pointer

		m_pClientDE = pClientDE;

		// Clamp the reps value

		if (m_dwReps == 0) m_dwReps = 1;
		
		// Store the base data

		m_dwGroupID			 = pData->m_dwGroupID;
		m_dwID				 = pData->m_dwID;
		m_tmStart			 = pData->m_tmStart;
		m_tmEnd				 = pData->m_tmEnd;
		m_tmActualEnd		 = m_tmEnd;
		m_dwReps			 = pData->m_dwReps;
		m_tmLifespan		 = (m_tmEnd - m_tmStart) / (float)m_dwReps;
		m_tmEnd				 = m_tmStart + m_tmLifespan;
		
		m_hParent			 = pData->m_hParent;
		m_hCamera			 = pData->m_hCamera;
		m_hTarget			 = pData->m_hTarget;
		m_vScale			 = pData->m_vScale;

		strcpy(m_sNode, pData->m_sNode);
		
		if (m_hParent)
		{
			LTVector tmp;
			m_pClientDE->GetObjectPos(m_hParent, &tmp);

			m_vCreate = tmp;

		}
		else
		{
			m_vCreate			 = pData->m_vPos;
		}

		// Record target data

		m_bUseTargetData = pData->m_bUseTargetData;
		m_vTargetPos	 = pData->m_vTargetPos;
		m_vTargetNorm	 = pData->m_vTargetNorm;

		// Scan the properties list for colour key frames

		CLinkListNode<FX_PROP> *pNode = pProps->GetHead();

		while (pNode)
		{
			if (!strcmp(pNode->m_Data.m_sName, "RandGen"))
			{
				char sTmp[256];
				strcpy(sTmp, pNode->m_Data.m_data.m_sVal);
				
				int nVal = atoi(strtok(sTmp, ","));
				
				switch (nVal)
				{
					case RG_BOUNDINGBOX :
					{
						// Choose a random offset within the parent's
						// bounding box

						LTVector vDims;
						m_pClientDE->Physics()->GetObjectDims(m_hParent, &vDims);

						m_vRandOffset.x = -vDims.x + fxGetRandom(0.0f, vDims.x * 2.0f);
						m_vRandOffset.y = -vDims.y + fxGetRandom(0.0f, vDims.y * 2.0f);
						m_vRandOffset.z = -vDims.z + fxGetRandom(0.0f, vDims.z * 2.0f);
					}
					break;
				}
			}
			if (!strcmp(pNode->m_Data.m_sName, "UpdatePos"))
			{
				char sTmp[256];
				strcpy(sTmp, pNode->m_Data.m_data.m_sVal);
				
				m_dwFollowType = (uint32)atoi(strtok(sTmp, ","));
			}
			else if (!strcmp(pNode->m_Data.m_sName, "Interpolate"))
			{
				char sTmp[256];
				strcpy(sTmp, pNode->m_Data.m_data.m_sVal);
				
				int nVal = atoi(strtok(sTmp, ","));
				m_bInterpolate = nVal ? false : true;
			}
			else if (!strcmp(pNode->m_Data.m_sName, "Offset"))
			{
				m_vOffset.x = pNode->m_Data.m_data.m_fVec[0];
				m_vOffset.y = pNode->m_Data.m_data.m_fVec[1];
				m_vOffset.z = pNode->m_Data.m_data.m_fVec[2];
			}
			else if (!strcmp(pNode->m_Data.m_sName, "RotateAdd"))
			{
				m_vRotAdd.x = pNode->m_Data.m_data.m_fVec[0];
				m_vRotAdd.y = pNode->m_Data.m_data.m_fVec[1];
				m_vRotAdd.z = pNode->m_Data.m_data.m_fVec[2];
			}
			else if (!strcmp(pNode->m_Data.m_sName, "Ck"))
			{
				// Add this key to the list of keys

				FX_COLOURKEY fxClrKey;

				fxClrKey.m_tmKey = pNode->m_Data.m_data.m_clrKey.m_tmKey;// * m_tmLifespan;
				fxClrKey.m_red   = (float)(pNode->m_Data.m_data.m_clrKey.m_dwCol & 0x000000FF);
				fxClrKey.m_green = (float)((pNode->m_Data.m_data.m_clrKey.m_dwCol & 0x0000FF00) >> 8);
				fxClrKey.m_blue  = (float)((pNode->m_Data.m_data.m_clrKey.m_dwCol & 0x00FF0000) >> 16);
				fxClrKey.m_alpha = (float)((pNode->m_Data.m_data.m_clrKey.m_dwCol & 0xFF000000) >> 24);

				m_collColourKeys.AddTail(fxClrKey);
			}
			else if (!strcmp(pNode->m_Data.m_sName, "Sk"))
			{
				// Add this key to the list of keys

				FX_SCALEKEY fxSclKey;

				fxSclKey.m_tmKey = pNode->m_Data.m_data.m_fVec4[0] * m_tmLifespan;
				fxSclKey.m_scale = pNode->m_Data.m_data.m_fVec4[1];

				m_collScaleKeys.AddTail(fxSclKey);
			}
			else if (!strcmp(pNode->m_Data.m_sName, "Mk"))
			{
				// Add this key to the list of keys

				FX_MOVEKEY fxMvKey;

				fxMvKey.m_tmKey  = pNode->m_Data.m_data.m_fVec4[0] * m_tmLifespan;
				fxMvKey.m_vPos.x = pNode->m_Data.m_data.m_fVec4[1];
				fxMvKey.m_vPos.y = pNode->m_Data.m_data.m_fVec4[2];
				fxMvKey.m_vPos.z = pNode->m_Data.m_data.m_fVec4[3];

				m_collMoveKeys.AddTail(fxMvKey);
			}

			pNode = pNode->m_pNext;
		}

		m_pCurColour = m_collColourKeys.GetHead();
		m_pCurScale  = m_collScaleKeys.GetHead();
		m_pCurMove   = m_collMoveKeys.GetHead();

		m_tmBaseLastUpdate = m_pClientDE->GetTime();
		m_vPos			   = m_vCreate;
		m_vPos += m_vOffset;
		m_vLastPos         = m_vPos;
		if (m_collMoveKeys.GetSize())
		{
			m_vMotionOffset	= m_collMoveKeys.GetHead()->m_Data.m_vPos;
		}

		if (m_hParent)
		{
			// Record the initial rotation of the parent object

			LTRotation dOrient;
			
			m_pClientDE->GetObjectRotation(m_hParent, &dOrient);
					
			m_vInitialUp		= dOrient.Up();
			m_vInitialRight		= dOrient.Right();
			m_vInitialForward	= dOrient.Forward();
		}
		else
		{
			m_vInitialRight = LTVector(1.0f, 0.0f, 0.0f);
			m_vInitialUp = LTVector(0.0f, 1.0f, 0.0f);
			m_vInitialForward = LTVector(0.0f, 0.0f, 1.0f);
		}

		// Success !!

		return true;
	}

	//------------------------------------------------------------------
	//
	//   FUNCTION : Update()
	//
	//   PURPOSE  : Performs base class updating of CBaseFX
	//
	//------------------------------------------------------------------

	inline bool CBaseFX::Update(float tmCur)
	{
		if (m_bFrozen)
		{
			return true;
		}

		// Check for a reset....
/*
		if (tmCur > m_tmEnd)
		{
			uint32 dwRepsToDeduct = (uint32)((tmCur - m_tmStart) / m_tmLifespan);

			if (dwRepsToDeduct > m_dwReps)
			{
				m_dwReps = 0;
			}
			else
			{
				m_dwReps -= dwRepsToDeduct;

				m_tmStart += (m_tmLifespan * (float)dwRepsToDeduct) - (tmCur - m_tmEnd);
				m_tmEnd = m_tmStart + m_tmLifespan;

				// Reset keys

				m_pCurColour = m_collColourKeys.GetHead();
				m_pCurScale  = m_collScaleKeys.GetHead();
				m_pCurMove   = m_collMoveKeys.GetHead();
			}
		}

		if (m_dwReps == 0) 
		{
			m_tmBaseLastUpdate = tmCur;
		
			return true;
		}
*/
		LTVector vRoot;
		LTRotation vRot;

		vRoot.x = 0.0f;
		vRoot.y = 0.0f;
		vRoot.z = 0.0f;

		bool bContinueProcess = true;

		// Set to active if the FX isn't already

		if (m_dwState == FS_INACTIVE)
		{
			m_tmLastUpdate = m_pClientDE->GetTime();
			m_pClientDE->Common()->SetObjectFlags(m_hObject, OFT_Flags, FLAG_VISIBLE, FLAG_VISIBLE);
			m_dwState = FS_PLAYING;
		}

		// We are in our main update
		
		if (m_hParent)
		{
			// Compute the aligned offset

			LTVector vAlignedOffset;

			LTVector vRight, vUp, vForward;

			LTRotation dOrient;

			m_pClientDE->GetObjectRotation(m_hParent, &dOrient);
			
			m_vInitialRight		= dOrient.Right();
			m_vInitialUp		= dOrient.Up();
			m_vInitialForward	= dOrient.Forward();

			ILTModel *pModelLT = m_pClientDE->GetModelLT();

			vAlignedOffset = (m_vInitialRight * m_vOffset.x) + (m_vInitialUp * m_vOffset.y) + (m_vInitialForward * m_vOffset.z);

			if (m_sNode[0] == 0)
			{
				switch (m_dwFollowType)
				{				
					case UP_FIXED :
					{
						m_vPos = m_vCreate + vAlignedOffset;
					}
					break;

					case UP_FOLLOW :
					{
						m_pClientDE->GetObjectPos(m_hParent, &m_vPos);
						m_vPos += vAlignedOffset;
					}
					break;

					case UP_LEFTHAND :
					{
						LTransform lTrans;
						HMODELNODE hNode;
						pModelLT->GetNode(m_hParent, "LeftHand", hNode);

						if (hNode)
						{
							pModelLT->GetNodeTransform(m_hParent, hNode, lTrans, true);//&m_vPos, &vRot);
							m_vPos = lTrans.m_Pos;
							vRot = lTrans.m_Rot;
						}

						m_vPos += vAlignedOffset;
					}
					break;

					case UP_RIGHTHAND :
					{
						LTransform lTrans;
						HMODELNODE hNode;
						pModelLT->GetNode(m_hParent, "RightHand", hNode);

						if (hNode)
						{
							pModelLT->GetNodeTransform(m_hParent, hNode, lTrans, true);//&m_vPos, &vRot);
							m_vPos = lTrans.m_Pos;
							vRot = lTrans.m_Rot;
						}
						m_vPos += vAlignedOffset;
					}
					break;

					case UP_LEFTFOOT :
					{
						LTransform lTrans;
						HMODELNODE hNode;
						pModelLT->GetNode(m_hParent, "LeftLeg", hNode);

						if (hNode)
						{
							pModelLT->GetNodeTransform(m_hParent, hNode, lTrans, true);//&m_vPos, &vRot);
							m_vPos = lTrans.m_Pos;
							vRot = lTrans.m_Rot;
						}
						m_vPos += vAlignedOffset;
					}
					break;

					case UP_RIGHTFOOT :
					{
						LTransform lTrans;
						HMODELNODE hNode;
						pModelLT->GetNode(m_hParent, "RightFoot", hNode);

						if (hNode)
						{
							pModelLT->GetNodeTransform(m_hParent, hNode, lTrans, true);//&m_vPos, &vRot);
							m_vPos = lTrans.m_Pos;
							vRot = lTrans.m_Rot;
						}
						m_vPos += vAlignedOffset;
					}
					break;

					case UP_HEAD :
					{
						LTransform lTrans;
						HMODELNODE hNode;
						pModelLT->GetNode(m_hParent, "Head", hNode);

						if (hNode)
						{
							pModelLT->GetNodeTransform(m_hParent, hNode, lTrans, true);//&m_vPos, &vRot);
							m_vPos = lTrans.m_Pos;
							vRot = lTrans.m_Rot;
						}
						m_vPos += vAlignedOffset;
					}
					break;

					case UP_TAIL :
					{
						LTransform lTrans;
						HMODELNODE hNode;
						pModelLT->GetNode(m_hParent, "Tail", hNode);

						if (hNode)
						{
							pModelLT->GetNodeTransform(m_hParent, hNode, lTrans, true);//&m_vPos, &vRot);
							m_vPos = lTrans.m_Pos;
							vRot = lTrans.m_Rot;
						}
						m_vPos += vAlignedOffset;
					}
					break;

					case UP_U1 :
					{
						LTransform lTrans;
						HMODELNODE hNode;
						pModelLT->GetNode(m_hParent, "u1", hNode);

						if (hNode)
						{
							pModelLT->GetNodeTransform(m_hParent, hNode, lTrans, true);//&m_vPos, &vRot);
							m_vPos = lTrans.m_Pos;
							vRot = lTrans.m_Rot;
						}
						m_vPos += vAlignedOffset;
					}
					break;

					case UP_U2 :
					{
						LTransform lTrans;
						HMODELNODE hNode;
						pModelLT->GetNode(m_hParent, "u2", hNode);

						if (hNode)
						{
							pModelLT->GetNodeTransform(m_hParent, hNode, lTrans, true);//&m_vPos, &vRot);
							m_vPos = lTrans.m_Pos;
							vRot = lTrans.m_Rot;
						}
						m_vPos += vAlignedOffset;
					}
					break;

					case UP_U3 :
					{
						LTransform lTrans;
						HMODELNODE hNode;
						pModelLT->GetNode(m_hParent, "u3", hNode);

						if (hNode)
						{
							pModelLT->GetNodeTransform(m_hParent, hNode, lTrans, true);//&m_vPos, &vRot);
							m_vPos = lTrans.m_Pos;
							vRot = lTrans.m_Rot;
						}
						m_vPos += vAlignedOffset;
					}
					break;

					case UP_U4 :
					{
						LTransform lTrans;
						HMODELNODE hNode;
						pModelLT->GetNode(m_hParent, "u4", hNode);

						if (hNode)
						{
							pModelLT->GetNodeTransform(m_hParent, hNode, lTrans, true);//&m_vPos, &vRot);
							m_vPos = lTrans.m_Pos;
							vRot = lTrans.m_Rot;
						}
						m_vPos += vAlignedOffset;
					}
					break;

					case UP_U5 :
					{
						LTransform lTrans;
						HMODELNODE hNode;
						pModelLT->GetNode(m_hParent, "u5", hNode);

						if (hNode)
						{
							pModelLT->GetNodeTransform(m_hParent, hNode, lTrans, true);//&m_vPos, &vRot);
							m_vPos = lTrans.m_Pos;
							vRot = lTrans.m_Rot;
						}
						m_vPos += vAlignedOffset;
					}
					break;

					case UP_U6 :
					{
						LTransform lTrans;
						HMODELNODE hNode;
						pModelLT->GetNode(m_hParent, "u6", hNode);

						if (hNode)
						{
							pModelLT->GetNodeTransform(m_hParent, hNode, lTrans, true);//&m_vPos, &vRot);
							m_vPos = lTrans.m_Pos;
							vRot = lTrans.m_Rot;
						}
						m_vPos += vAlignedOffset;
					}
					break;

					case UP_U7 :
					{
						LTransform lTrans;
						HMODELNODE hNode;
						pModelLT->GetNode(m_hParent, "u7", hNode);

						if (hNode)
						{
							pModelLT->GetNodeTransform(m_hParent, hNode, lTrans, true);//&m_vPos, &vRot);
							m_vPos = lTrans.m_Pos;
							vRot = lTrans.m_Rot;
						}
						m_vPos += vAlignedOffset;
					}
					break;

					case UP_U8 :
					{
						LTransform lTrans;
						HMODELNODE hNode;
						pModelLT->GetNode(m_hParent, "u8", hNode);

						if (hNode)
						{
							pModelLT->GetNodeTransform(m_hParent, hNode, lTrans, true);//&m_vPos, &vRot);
							m_vPos = lTrans.m_Pos;
							vRot = lTrans.m_Rot;
						}
						m_vPos += vAlignedOffset;
					}
					break;

					case UP_U9 :
					{
						LTransform lTrans;
						HMODELNODE hNode;
						pModelLT->GetNode(m_hParent, "u9", hNode);

						if (hNode)
						{
							pModelLT->GetNodeTransform(m_hParent, hNode, lTrans, true);//&m_vPos, &vRot);
							m_vPos = lTrans.m_Pos;
							vRot = lTrans.m_Rot;
						}
						m_vPos += vAlignedOffset;
					}
					break;

					case UP_U10 :
					{
						LTransform lTrans;
						HMODELNODE hNode;
						pModelLT->GetNode(m_hParent, "u10", hNode);

						if (hNode)
						{
							pModelLT->GetNodeTransform(m_hParent, hNode, lTrans, true);//&m_vPos, &vRot);
							m_vPos = lTrans.m_Pos;
							vRot = lTrans.m_Rot;
						}
						m_vPos += vAlignedOffset;
					}
					break;

				}
			}
			else
			{
				LTransform lTrans;
				HMODELNODE hNode;
				pModelLT->GetNode(m_hParent, m_sNode, hNode);

				if (hNode)
				{
					pModelLT->GetNodeTransform(m_hParent, hNode, lTrans, true);//&m_vPos, &vRot);
					m_vPos = lTrans.m_Pos;
					vRot = lTrans.m_Rot;
				}

				// Move to the node's position
	
//				m_pClientDE->GetModelNodeTransform(m_hParent, m_sNode, &m_vPos, &vRot);
			}
		}

		// Move object to correct position

		LTVector vReal = (m_vPos + m_vMotionOffset + m_vRandOffset);
		m_pClientDE->SetObjectPos(m_hObject, &vReal);

		// Update the colour

		if (m_bUpdateColour)
		{
			m_pClientDE->GetObjectColor(m_hObject, &m_red, &m_green, &m_blue, &m_alpha);

			if (m_pCurColour)
			{
				CalcColour(m_tmStart, tmCur, m_tmLifespan, &m_red, &m_green, &m_blue, &m_alpha);
			}

			m_pClientDE->SetObjectColor(m_hObject, m_red, m_green, m_blue, m_alpha);
		}
	
		// Compute the current scale based on keyframes and update the 
		// keyframe pointer

		if (m_bUpdateScale)
		{
			if ((m_pCurScale) && (m_collScaleKeys.GetSize() > 1))
			{
				// Advance the keyframes....
				
				bool bAdvancing = true;
				bool bReset = false;
						
				while (bAdvancing) 
				{
					float tmKey = tmCur - m_tmStart;
					
					// Make sure we wrap properly for continual looping

					tmKey = (float)fmod(tmKey, m_tmLifespan);
				
					if ((m_pCurScale->m_pNext) && (m_pCurScale) && (tmKey >= m_pCurScale->m_Data.m_tmKey) && (tmKey < m_pCurScale->m_pNext->m_Data.m_tmKey))
					{								
						bReset = true;
						bAdvancing = false;
					}
					else
					{
						m_pCurScale = m_pCurScale->m_pNext;
					}

					if (!m_pCurScale) bAdvancing = false;
				}

				if (m_pCurScale)
				{
					FX_SCALEKEY startKey;
					FX_SCALEKEY endKey;

					startKey = m_pCurScale->m_Data;
					if (m_pCurScale->m_pNext)
					{
						endKey = m_pCurScale->m_pNext->m_Data;
					}
					else
					{
						endKey = m_pCurScale->m_Data;
					}
					
					// Use this and the previous key to compute the scale

					float tmDist = endKey.m_tmKey - startKey.m_tmKey;
					float tmKey  = (float)fmod(tmCur - m_tmStart, m_tmLifespan) - startKey.m_tmKey;
					
					if (tmDist > 0.0f)
					{
						float rat = (endKey.m_scale - startKey.m_scale) / tmDist;

						m_scale = startKey.m_scale + (rat * tmKey);
					}
					else
					{
						m_scale = startKey.m_scale;
					}

					LTVector vScale;
					vScale.Init(m_scale, m_scale, m_scale);

					uint32 dwObjectType;
					m_pClientDE->Common()->GetObjectType(m_hObject, &dwObjectType);

					if ((dwObjectType == OT_MODEL) || (dwObjectType == OT_SPRITE))
					{
						m_pClientDE->SetObjectScale(m_hObject, &vScale);
					}
				}
				
				if (bReset)
				{
					// Reset the scale keys....

					m_pCurScale = m_collScaleKeys.GetHead();
				}
			}
		}

		// Compute the current movement offset based on keyframes and update the 
		// keyframe pointer

		if (m_pCurMove)
		{
			// Advance the keyframes....
			
			bool bAdvancing = true;
			bool bReset = false;
					
			while (bAdvancing) 
			{
				float tmKey = tmCur - m_tmStart;

				// Make sure we wrap properly for continual looping

				tmKey = (float)fmod(tmKey, m_tmLifespan);

				if ((m_pCurMove->m_pNext) && (m_pCurMove) && (tmKey >= m_pCurMove->m_Data.m_tmKey) && (tmKey < m_pCurMove->m_pNext->m_Data.m_tmKey))
				{				
					bReset = true;
					bAdvancing = false;
				}
				else
				{
					m_pCurMove = m_pCurMove->m_pNext;
				}

				if (!m_pCurMove) bAdvancing = false;
			}

			if (m_pCurMove)
			{
				FX_MOVEKEY startKey;
				FX_MOVEKEY endKey;

				startKey = m_pCurMove->m_Data;
				if (m_pCurMove->m_pNext)
				{
					endKey = m_pCurMove->m_pNext->m_Data;
				}
				else
				{
					endKey = m_pCurMove->m_Data;
				}
				
				// Use this and the previous key to compute the scale

				float tmDist = endKey.m_tmKey - startKey.m_tmKey;
				float tmKey  = (float)fmod(tmCur - m_tmStart, m_tmLifespan) - startKey.m_tmKey;

				float x, y, z;
				
				if (tmDist > 0.0f)
				{
					float xRat = (endKey.m_vPos.x - startKey.m_vPos.x) / tmDist;
					float yRat = (endKey.m_vPos.y - startKey.m_vPos.y) / tmDist;
					float zRat = (endKey.m_vPos.z - startKey.m_vPos.z) / tmDist;

					x = startKey.m_vPos.x + (xRat * tmKey);
					y = startKey.m_vPos.y + (yRat * tmKey);
					z = startKey.m_vPos.z + (zRat * tmKey);
				}
				else
				{
					x = startKey.m_vPos.x;
					y = startKey.m_vPos.y;
					z = startKey.m_vPos.z;
				}

				m_vMotionOffset = (m_vInitialRight * x) + (m_vInitialUp * y) + (m_vInitialForward * z);
			}

			if (bReset)
			{
				m_pCurMove = m_collMoveKeys.GetHead();
			}
		}

		m_tmBaseLastUpdate = tmCur;

		// Success !!

		return true;
	}

	//------------------------------------------------------------------
	//
	//   FUNCTION : Shutdown()
	//
	//   PURPOSE  : Shuts down an FX
	//
	//------------------------------------------------------------------

	inline void CBaseFX::Shutdown()
	{
		// Set shutdown state
		
		m_dwState = FS_SHUTDOWN;
		
		// Null out the parent
		
		m_hParent = NULL;	
	}

	//------------------------------------------------------------------
	//
	//   FUNCTION : HasExpired()
	//
	//   PURPOSE  : Checks for expiration of FX
	//
	//------------------------------------------------------------------

	inline bool CBaseFX::HasExpired(float tmCur)
	{
		if ((tmCur >= m_tmActualEnd) || (m_dwState == FS_SHUTDOWN))
		{
			return true;
		}

		return false;
	}

#endif