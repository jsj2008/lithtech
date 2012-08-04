//------------------------------------------------------------------
//
//   MODULE  : FALLINGSTUFFFX.CPP
//
//   PURPOSE : Implements class CFallingStuffFX
//
//   CREATED : On 10/26/98 At 3:59:54 PM
//
//------------------------------------------------------------------

// Includes....

#include "stdafx.h"
#include "ClientFX.h"
#include "FallingStuffFX.h"
#include "FastList.h"
#include "math.h"

// Globals....

uint32 g_dwSplash = 0;

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CFallingStuffProps::CFallingStuffProps
//
//  PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //
CFallingStuffProps::CFallingStuffProps() 
{
	m_nFallingStuffFXEmission	= 1;
	m_tmFallingStuffFXEmission	= 1.0f;
	m_tmSpriteLifespan			= 1.0f;
	m_fRadius					= 100.0f;
	m_fVel						= 10.0f;

	m_vWind.x					= 0.0f;
	m_vWind.y					= -1.0f;
	m_vWind.z					= 0.0f;
	m_fWindAmount				= 1.0f;

	m_vPlaneDir.Init(0.0f, 1.0f, 0.0f);

	m_fStretchMul = 12.0f;

	m_nImpactCreate = 20;

	memset(m_sImpactSpriteName, 0, 128);
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CFallingStuffProps::ReadProps
//
//  PURPOSE:	Read in the proporty values that were set in FxED
//
// ----------------------------------------------------------------------- //

bool CFallingStuffProps::ParseProperties(FX_PROP* pProps, uint32 nNumProps)
{
	if(!CBaseFXProps::ParseProperties(pProps, nNumProps))
		return false;

	//
	// Loop through the props to initialize data
	//
	for(uint32 nCurrProp = 0; nCurrProp < nNumProps; nCurrProp++)
	{
		FX_PROP& fxProp = pProps[nCurrProp];

		if (!stricmp(fxProp.m_sName, "Sprite"))
		{
			char sTmp[128];
			strcpy(sTmp, fxProp.m_data.m_sVal);

			// Get the path name

			char *sExt  = strtok(sTmp, "|");
			char *sPath = strtok(NULL, "|");
			if (sPath) strcpy(m_sSpriteName, sPath);
		}
		else if (!stricmp(fxProp.m_sName, "StuffPerEmission"))
		{
			m_nFallingStuffFXEmission = fxProp.m_data.m_nVal;
		}
		else if (!stricmp(fxProp.m_sName, "EmissionInterval"))
		{
			m_tmFallingStuffFXEmission = fxProp.m_data.m_fVal;
		}
		else if (!stricmp(fxProp.m_sName, "StuffLifespan"))
		{
			m_tmSpriteLifespan = fxProp.m_data.m_fVal;
		}
		else if (!stricmp(fxProp.m_sName, "Radius"))
		{
			m_fRadius = fxProp.m_data.m_fVal;
		}
		else if (!stricmp(fxProp.m_sName, "Velocity"))
		{
			m_fVel = fxProp.m_data.m_fVal;
		}
		else if (!stricmp(fxProp.m_sName, "Stretch"))
		{
			m_fStretchMul = fxProp.m_data.m_fVal;
		}
		else if (!stricmp(fxProp.m_sName, "WindDir"))
		{
			m_vWind.x = fxProp.m_data.m_fVec[0];
			m_vWind.y = fxProp.m_data.m_fVec[1];
			m_vWind.z = fxProp.m_data.m_fVec[2];
		}
		else if (!stricmp(fxProp.m_sName, "WindAmount"))
		{
			m_fWindAmount = fxProp.m_data.m_fVal;
		}
		else if (!stricmp(fxProp.m_sName, "PlaneDir"))
		{
			m_vPlaneDir.x = fxProp.m_data.m_fVec[0];
			m_vPlaneDir.y = fxProp.m_data.m_fVec[1];
			m_vPlaneDir.z = fxProp.m_data.m_fVec[2];
		}		
		else if (!stricmp(fxProp.m_sName, "ImpactSprite"))
		{
			char sTmp[128];
			strcpy(sTmp, fxProp.m_data.m_sVal);

			// Get the path name

			char *sExt  = strtok(sTmp, "|");
			char *sPath = strtok(NULL, "|");
			if (sPath) strcpy(m_sImpactSpriteName, sPath);
		}
		else if (!stricmp(fxProp.m_sName, "ImpactScale1"))
		{
			m_fImpactScale1 = fxProp.m_data.m_fVal;
		}
		else if (!stricmp(fxProp.m_sName, "ImpactScale2"))
		{
			m_fImpactScale2 = fxProp.m_data.m_fVal;
		}
		else if (!stricmp(fxProp.m_sName, "ImpactLifespan"))
		{
			m_tmImpactLifespan = fxProp.m_data.m_fVal;
		}
		else if (!stricmp(fxProp.m_sName, "ImpactCreate"))
		{
			m_nImpactCreate = fxProp.m_data.m_nVal;
		}
	}

	m_bUseSpin = ((m_vRotAdd.x != 0) || (m_vRotAdd.y != 0.0f) || (m_vRotAdd.z != 0.0f)) ? true : false;

	return true;
}


//------------------------------------------------------------------
//
//   FUNCTION : CFallingStuffFX()
//
//   PURPOSE  : Standard constuctor
//
//------------------------------------------------------------------

CFallingStuffFX::CFallingStuffFX()
{
	m_tmElapsedEmission			= 0.0f;
	m_xRot						= 0.0f;
	m_yRot						= 0.0f;
	m_zRot						= 0.0f;

}

//------------------------------------------------------------------
//
//   FUNCTION : ~CFallingStuffFX
//
//   PURPOSE  : Standard destructor
//
//------------------------------------------------------------------

CFallingStuffFX::~CFallingStuffFX()
{
	Term();
}

//------------------------------------------------------------------
//
//   FUNCTION : Init()
//
//   PURPOSE  : Initialises class CFallingStuffFX
//
//------------------------------------------------------------------

bool CFallingStuffFX::Init(ILTClient *pClientDE, FX_BASEDATA *pBaseData, const CBaseFXProps *pProps)
{
	// Perform base class initialisation

	if (!CBaseFX::Init(pClientDE, pBaseData, pProps)) 
		return false;

	// Store the first position as the last position

	m_vLastPos = pBaseData->m_vPos;

	// If we have a parent object, get it and apply it's rotation
	// to the plane direction
	m_vPlaneDir = GetProps()->m_vPlaneDir;
	if (m_hParent)
	{
		LTRotation orient;
		m_pLTClient->GetObjectRotation(m_hParent, &orient);
		
		LTMatrix mRot;
		Mat_SetBasisVectors(&mRot, &orient.Right(), &orient.Up(), &orient.Forward());
		
		LTVector vTmp = m_vPlaneDir;

		MatVMul(&m_vPlaneDir, &mRot, &vTmp);
	}

	LTVector vUp;
	vUp.x = 0.0f;
	vUp.y = 1.0f;
	vUp.z = 0.0f;

	LTVector vTest = m_vPlaneDir;
	vTest.x = (float)fabs(vTest.x);
	vTest.y = (float)fabs(vTest.y);
	vTest.z = (float)fabs(vTest.z);
	
	if (vTest == vUp)
	{
		// Gotsta use another axis

		vUp.x = -1.0f;
		vUp.y = 0.0f;
		vUp.z = 0.0f;
	}

	m_vRight = m_vPlaneDir.Cross(vUp);
	m_vUp = m_vPlaneDir.Cross(m_vRight);


	// Create the base object

	CreateDummyObject();

	// Success !!

	return true;
}

//------------------------------------------------------------------
//
//   FUNCTION : Term()
//
//   PURPOSE  : Terminates class CFallingStuffFX
//
//------------------------------------------------------------------

void CFallingStuffFX::Term()
{
	// Delete any left over sprites

	if (m_collSprites.GetSize())
	{
		CLinkListNode<FALLING_THING *> *pNode = m_collSprites.GetHead();

		while (pNode)
		{
			m_pLTClient->RemoveObject(pNode->m_Data->m_hObject);
			debug_delete( pNode->m_Data );
			
			pNode = pNode->m_pNext;
		}

		m_collSprites.RemoveAll();
	}

	if (m_collSplashes.GetSize())
	{
		CLinkListNode<SPLASH *> *pNode = m_collSplashes.GetHead();

		while (pNode)
		{
			m_pLTClient->RemoveObject(pNode->m_Data->m_hObject);
			debug_delete( pNode->m_Data );
			
			pNode = pNode->m_pNext;
		}

		m_collSplashes.RemoveAll();
	}

	if (m_hObject) m_pLTClient->RemoveObject(m_hObject);
	m_hObject = NULL;
}

//------------------------------------------------------------------
//
//   FUNCTION : Update()
//
//   PURPOSE  : Updates class CFallingStuffFX
//
//------------------------------------------------------------------

bool CFallingStuffFX::Update(float tmFrameTime)
{
	// Base class update first
	
	m_vLastPos = m_vPos;
	
	if (!CBaseFX::Update(tmFrameTime)) 
		return false;

	//increment our emission time by the elapsed frame time
	m_tmElapsedEmission += tmFrameTime;

	if (!IsShuttingDown() && !IsSuspended() && (m_tmElapsedEmission > GetProps()->m_tmFallingStuffFXEmission))
	{
		ObjectCreateStruct ocs;
		INIT_OBJECTCREATESTRUCT(ocs);

		LTVector vScale;
		vScale.Init(m_scale, m_scale, m_scale);

		LTVector vInterp;
		LTVector vInterpCur  = m_vPos;
		
		// Calculate interpolant for particle system
		
		if (GetProps()->m_nFallingStuffFXEmission)
		{
			vInterp = m_vPos - m_vLastPos;
			vInterp /= (float)GetProps()->m_nFallingStuffFXEmission;
		}

		for (uint32 i = 0; i < GetProps()->m_nFallingStuffFXEmission; i ++)
		{
			ocs.m_ObjectType		= OT_SPRITE;
			ocs.m_Flags				= FLAG_VISIBLE | FLAG_NOLIGHT | FLAG_ROTATABLESPRITE;
			
			// Compute the initial position

			float xRand = GetProps()->m_fRadius * ((-10000.0f + (rand() % 20000)) / 10000.0f);
			float zRand = GetProps()->m_fRadius * ((-10000.0f + (rand() % 20000)) / 10000.0f);
			
			ocs.m_Pos = m_vPos + (m_vRight * xRand) + (m_vUp * zRand);
			
			ocs.m_Scale				= vScale;
			strcpy(ocs.m_Filename, GetProps()->m_sSpriteName);

			// Move the start point

			vInterpCur += vInterp;

			HLOCALOBJ hNewSprite = m_pLTClient->CreateObject(&ocs);

			if (hNewSprite)
			{
				// Create a new sprite

				FALLING_THING *pNewSprite = debug_new( FALLING_THING );

				if (GetProps()->m_nImpactCreate)
				{
					if (g_dwSplash > (uint32)GetProps()->m_nImpactCreate)
					{
						pNewSprite->m_bSplash = true;
						g_dwSplash = 0;
					}
					else
					{
						pNewSprite->m_bSplash = false;
					}
				}
				else
				{
					pNewSprite->m_bSplash = false;
				}

				g_dwSplash ++;

				if (pNewSprite)
				{
					LTVector v;

					// Compute the initial velocity

					v = m_vPlaneDir * GetProps()->m_fVel;

					pNewSprite->m_hObject	= hNewSprite;
					pNewSprite->m_vVel		= v;
					pNewSprite->m_tmElapsed	= 0.0f;
					pNewSprite->m_vPos		= ocs.m_Pos;
					pNewSprite->m_vLastPos	= ocs.m_Pos;
				
					m_collSprites.AddTail(pNewSprite);
				}
			}
		}

		m_tmElapsedEmission = 0.0f;

		// And store the last position

		m_vLastPos = m_vPos;
	}

	LTMatrix mSpin;

	if (GetProps()->m_bUseSpin)
	{
		// Setup rotation

		LTMatrix vRight;
		LTMatrix vUp;
		LTMatrix vForward;
		LTMatrix vTmp;

		Mat_SetupRot(&vRight, &m_vRight, m_xRot);
		Mat_SetupRot(&vUp, &m_vUp, m_yRot);
		Mat_SetupRot(&vForward, &m_vPlaneDir, m_zRot);

		MatMul(&vTmp, &vRight, &vUp);
		MatMul(&mSpin, &vTmp, &vForward);

		m_xRot += GetProps()->m_vRotAdd.x * tmFrameTime;
		m_yRot += GetProps()->m_vRotAdd.y * tmFrameTime;
		m_zRot += GetProps()->m_vRotAdd.z * tmFrameTime;
	}

	// Get the camera rotation

	LTRotation orient;
	m_pLTClient->GetObjectRotation(m_hCamera, &orient);

	LTRotation dRot(orient);
	
	LTVector vF = orient.Forward();
	
	float rot = (float)atan2(vF.x, vF.z);

	// Update the sprites....

	CLinkListNode<FALLING_THING *> *pNode = m_collSprites.GetHead();
	CLinkListNode<FALLING_THING *> *pDelNode;

	while (pNode)
	{
		pDelNode = NULL;

		FALLING_THING *pSprite = pNode->m_Data;

		//adjust our elapsed time
		pSprite->m_tmElapsed += tmFrameTime;

		// Check for expiration

		if (pSprite->m_tmElapsed > GetProps()->m_tmSpriteLifespan)
		{
			// Destroy this object

			m_pLTClient->RemoveObject(pSprite->m_hObject);

			pDelNode = pNode;			
		}
		else
		{
			// Update !!

			pSprite->m_vLastPos = pSprite->m_vPos;
	
			pSprite->m_vPos += (pSprite->m_vVel * tmFrameTime);

			// Rotate if neccessary

			TVector3<float> vPos = pSprite->m_vPos;
						
			if (GetProps()->m_bUseSpin)
			{
				MatVMul_InPlace(&mSpin, &vPos);
			}

			// Add in wind

			vPos += (GetProps()->m_vWind * GetProps()->m_fWindAmount) * tmFrameTime;

			// Setup the new sprite position

			LTVector vPos2 = vPos;
			m_pLTClient->SetObjectPos(pSprite->m_hObject, &vPos2);

			
			// Setup the colour
			
			float r, g, b, a;
			
			m_pLTClient->GetObjectColor(pSprite->m_hObject, &r, &g, &b, &a);			
			CalcColour(pSprite->m_tmElapsed, GetProps()->m_tmSpriteLifespan, &r, &g, &b, &a);			
			m_pLTClient->SetObjectColor(pSprite->m_hObject, r, g, b, a);

			// Setup the scale

			float scale = 0.1f;

			CalcScale(pSprite->m_tmElapsed, GetProps()->m_tmSpriteLifespan, &scale);

			LTVector vScale;
			vScale.Init(scale, scale * GetProps()->m_fStretchMul, scale);
			m_pLTClient->SetObjectScale(pSprite->m_hObject, &vScale);

			// Setup the rotation
			
			dRot = LTRotation(0, 0, 0, 1);
			LTRotation orient(dRot);

			orient.Rotate( orient.Up(), rot );
			
			m_pLTClient->SetObjectRotation(pSprite->m_hObject, &orient);

			// Check to see if we need to start a splash sprite

			if (pSprite->m_bSplash)
			{
				ClientIntersectQuery ciq;
				ClientIntersectInfo  cii;

				ciq.m_From		= pSprite->m_vLastPos;
				ciq.m_To		= pSprite->m_vPos;

				if ((GetProps()->m_sImpactSpriteName[0]) && (m_pLTClient->IntersectSegment(&ciq, &cii)))
				{
					// Create a splash sprite
									
					SPLASH *pSplash = debug_new( SPLASH );

					ObjectCreateStruct ocs;
					INIT_OBJECTCREATESTRUCT(ocs);

					LTVector vScale;
					vScale.Init(0.0f, 0.0f, 0.0f);

					ocs.m_ObjectType = OT_SPRITE;
					ocs.m_Flags		 = FLAG_VISIBLE | FLAG_ROTATABLESPRITE | FLAG_NOLIGHT;
					ocs.m_Pos		 = cii.m_Point + (cii.m_Plane.m_Normal * 2.0f);
					ocs.m_Scale		 = vScale;

					LTRotation dOrient( cii.m_Plane.m_Normal, LTVector(0.0f, 1.0f, 0.0f) );

					strcpy(ocs.m_Filename, GetProps()->m_sImpactSpriteName);

					pSplash->m_hObject = m_pLTClient->CreateObject(&ocs);
					pSplash->m_scale = 0.0f;

					LTRotation orient(dRot);
					m_pLTClient->SetObjectRotation(pSplash->m_hObject, &orient);

					pSplash->m_tmElapsed = 0.0f;
					
					m_collSplashes.AddTail(pSplash);
					
					// Destroy this object

					m_pLTClient->RemoveObject(pSprite->m_hObject);

					// Delete the sprite

					pDelNode = pNode;
				}
			}
		}

		pNode = pNode->m_pNext;

		if (pDelNode) m_collSprites.Remove(pDelNode);
	}

	// Update our splashes

	CLinkListNode<SPLASH *> *pSplashNode = m_collSplashes.GetHead();

	while (pSplashNode)
	{
		CLinkListNode<SPLASH *> *pDelNode = NULL;

		SPLASH *pSplash = pSplashNode->m_Data;

		//update the elapsed time on the splash
		pSplash->m_tmElapsed += tmFrameTime;
		
		// Calculate the new scale

		float scale = GetProps()->m_fImpactScale1 + ((GetProps()->m_fImpactScale2 - GetProps()->m_fImpactScale1) * (pSplash->m_tmElapsed / GetProps()->m_tmImpactLifespan));

		LTVector vScale(scale, scale, scale);
		m_pLTClient->SetObjectScale(pSplash->m_hObject, &vScale);

		float r, g, b, a;

		m_pLTClient->GetObjectColor(pSplash->m_hObject, &r, &g, &b, &a);

		a = (float)(int)(pSplash->m_tmElapsed / GetProps()->m_tmImpactLifespan);
		if (a < 0.0f) a = 0.0f;
		if (a > 1.0f) a = 1.0f;

		m_pLTClient->SetObjectColor(pSplash->m_hObject, r, g, b, a);

		if (pSplash->m_tmElapsed > GetProps()->m_tmImpactLifespan)
		{
			m_pLTClient->RemoveObject(pSplash->m_hObject);
			pDelNode = pSplashNode;
		}

		pSplashNode = pSplashNode->m_pNext;

		if (pDelNode) m_collSplashes.Remove(pDelNode);
	}

	// Success !!

	return true;
}

//------------------------------------------------------------------
//
//   FUNCTION : fxGetFallingStuffProps()
//
//   PURPOSE  : Returns a list of properties for this FX
//
//------------------------------------------------------------------

void fxGetFallingStuffProps(CFastList<FX_PROP> *pList)
{
	FX_PROP fxProp;
	float fVec[3];
	fVec[0] = 0.0f;
	fVec[1] = 1.0f;
	fVec[2] = 0.0f;

	// Add the base props

	AddBaseProps(pList);

	// Add all the props to the list

	fxProp.Combo("Interpolate", "0,Yes,No");
	pList->AddTail(fxProp);

	fxProp.Path("Sprite", "spr|...");
	pList->AddTail(fxProp);

	fxProp.Int("StuffPerEmission", 5);
	pList->AddTail(fxProp);

	fxProp.Float("EmissionInterval", 0.01f);
	pList->AddTail(fxProp);

	fxProp.Float("StuffLifespan", 2.0f);
	pList->AddTail(fxProp);

	fxProp.Float("Radius", 10.0f);
	pList->AddTail(fxProp);

	fxProp.Vector("PlaneDir", fVec);
	pList->AddTail(fxProp);

	fxProp.Float("Velocity", 10.0f);
	pList->AddTail(fxProp);

	fxProp.Float("Stretch", 10.0f);
	pList->AddTail(fxProp);

	fxProp.Path("ImpactSprite", "spr|...");
	pList->AddTail(fxProp);

	fxProp.Float("ImpactLifespan", 1.0f);
	pList->AddTail(fxProp);

	fxProp.Float("ImpactScale1", 1.0f);
	pList->AddTail(fxProp);

	fxProp.Float("ImpactScale2", 1.0f);
	pList->AddTail(fxProp);

	fxProp.Int("ImpactCreate", 20);
	pList->AddTail(fxProp);

	fxProp.Combo("ImpactPerturb", "0,None,Sine,Pendulum");
	pList->AddTail(fxProp);
}