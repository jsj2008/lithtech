//------------------------------------------------------------------
//
//   MODULE  : PolyFanFX.CPP
//
//   PURPOSE : Implements class CPolyFanFX
//
//   CREATED : On 11/23/98 At 6:21:37 PM
//
//------------------------------------------------------------------

// Includes....

#include "stdafx.h"
#include "PolyFanFX.h"
#include "ClientFX.h"

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CPolyFanProps::CPolyFanProps
//
//  PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //
CPolyFanProps::CPolyFanProps() : 
	m_nAlongNormal	(0),
	m_bParentRotate	(false)
{
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CPolyFanProps::ReadProps
//
//  PURPOSE:	Read in the proporty values that were set in FxED
//
// ----------------------------------------------------------------------- //

bool CPolyFanProps::ParseProperties(FX_PROP* pProps, uint32 nNumProps)
{
	if(!CBaseFXProps::ParseProperties(pProps, nNumProps))
		return false;

	//
	// Loop through the props to initialize data
	//
	for(uint32 nCurrProp = 0; nCurrProp < nNumProps; nCurrProp++)
	{
		FX_PROP& fxProp = pProps[nCurrProp];
	
		if (!stricmp(fxProp.m_sName, "PolyFan"))
		{
			char sTmp[128];
			strcpy(sTmp, fxProp.m_data.m_sVal);

			// Get the path name

			char *sExt  = strtok(sTmp, "|");
			char *sPath = strtok(NULL, "|");
			if (sPath) strcpy(m_sPolyFanName, sPath);
		}
		if (!stricmp(fxProp.m_sName, "Facing"))
		{
			m_nAlongNormal = fxProp.GetComboVal();
		}
		if (!stricmp(fxProp.m_sName, "ParentRotate"))
		{
			m_bParentRotate = fxProp.GetComboVal() ? true : false;
		}
		else if (!stricmp(fxProp.m_sName, "Normal"))
		{
			m_vRot.x = fxProp.m_data.m_fVec[0];
			m_vRot.y = fxProp.m_data.m_fVec[1];
			m_vRot.z = fxProp.m_data.m_fVec[2];
		}
	}

	return true;
}


//------------------------------------------------------------------
//
//   FUNCTION : CPolyFanFX()
//
//   PURPOSE  : Standard constuctor
//
//------------------------------------------------------------------

CPolyFanFX::CPolyFanFX()
{
}

//------------------------------------------------------------------
//
//   FUNCTION : ~CPolyFanFX
//
//   PURPOSE  : Standard destructor
//
//------------------------------------------------------------------

CPolyFanFX::~CPolyFanFX()
{
	Term();
}

//------------------------------------------------------------------
//
//   FUNCTION : Init()
//
//   PURPOSE  : Initialises class CPolyFanFX
//
//------------------------------------------------------------------

bool CPolyFanFX::Init(ILTClient *pClientDE, FX_BASEDATA *pBaseData, const CBaseFXProps *pProps)
{
	// Perform base class initialisation

	if (!CBaseFX::Init(pClientDE, pBaseData, pProps)) 
		return false;

	LTVector vPos;
	if( m_hParent )
	{
		m_pLTClient->GetObjectPos(m_hParent, &vPos);
	}
	else
	{
		vPos = m_vCreatePos;
	}

	LTVector vScale;
	vScale.Init(1.0f, 1.0f, 1.0f);

	ObjectCreateStruct ocs;
	INIT_OBJECTCREATESTRUCT(ocs);

	ocs.m_ObjectType		= OT_NORMAL;
	ocs.m_Flags				= 0;
	ocs.m_Pos				= vPos;
	ocs.m_Scale				= vScale;
	strcpy(ocs.m_Filename, GetProps()->m_sPolyFanName);

	m_hObject = m_pLTClient->CreateObject(&ocs);

	// Success !!

	return true;
}

//------------------------------------------------------------------
//
//   FUNCTION : Term()
//
//   PURPOSE  : Terminates class CPolyFanFX
//
//------------------------------------------------------------------

void CPolyFanFX::Term()
{
	if (m_hObject) m_pLTClient->RemoveObject(m_hObject);
	m_hObject = NULL;
}

//------------------------------------------------------------------
//
//   FUNCTION : Update()
//
//   PURPOSE  : Updates class CPolyFanFX
//
//------------------------------------------------------------------

bool CPolyFanFX::Update(float tmCur)
{
	// Base class update first
	
	if (!CBaseFX::Update(tmCur)) return false;

	// Align if neccessary, to the rotation of our parent

	if ((m_hParent) && (GetProps()->m_nAlongNormal == 2))
	{
		LTRotation parentRot;
		m_pLTClient->GetObjectRotation(m_hParent, &parentRot);
		m_pLTClient->SetObjectRotation(m_hObject, &parentRot);
	}

	// Success !!

	return true;
}


//------------------------------------------------------------------
//
//   FUNCTION : fxGetPolyFanFXProps()
//
//   PURPOSE  : Returns a list of properties for this FX
//
//------------------------------------------------------------------

void fxGetPolyFanProps(CFastList<FX_PROP> *pList)
{
	FX_PROP fxProp;
	float vTmp[3];
	vTmp[0] = 0.0f;
	vTmp[1] = 0.0f;
	vTmp[2] = 1.0f;

	// Add the base props

	AddBaseProps(pList);

	// Add all the props to the list

	fxProp.Path("Texture", "dtx|...");
	pList->AddTail(fxProp);

	fxProp.Combo("2nd Node","0,LeftHand,RightHand,LeftFoot,RightFoot,Head,Tail,u1,u2,u3,u4,u5,u6,u7,u8,u9,u10");
	pList->AddTail(fxProp);	
}