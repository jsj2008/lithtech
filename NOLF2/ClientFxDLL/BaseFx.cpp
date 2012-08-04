#include "stdafx.h"
#include "BaseFx.h"
#include "ClientFX.h"

CBaseFXProps::CBaseFXProps() : 
	m_pScaleKeys		(NULL),
	m_nNumScaleKeys		(0),
	m_pColorKeys		(NULL),
	m_nNumColorKeys		(0),
	m_nFollowType		(UP_FIXED),
	m_nMenuLayer		(0)
{
	m_szAttach[0] = '\0';
}

CBaseFXProps::~CBaseFXProps()
{
	debug_deletea( m_pColorKeys );
	debug_deletea( m_pScaleKeys );
}

//sets up parameters for the effects lifetime
void CBaseFXProps::SetLifetime(float fLifespan, uint32 nNumRepeats)
{
	m_tmLifespan	= fLifespan;
	m_tmActualEnd	= fLifespan * nNumRepeats;
}


//this will take a list of properties and convert it to internal values
bool CBaseFXProps::ParseProperties(FX_PROP* pProps, uint32 nNumProps)
{
	//counts of how many of each key
	uint32 nNumColorKeys = 0;
	uint32 nNumScaleKeys = 0;

	//go through the property list and parse in all the known variables and
	//count up how many of each key type we have
	uint32 nCurrProp = 0;

	for(nCurrProp = 0; nCurrProp < nNumProps; nCurrProp++)
	{
		FX_PROP& fxProp = pProps[nCurrProp];

		if( !_stricmp( fxProp.m_sName, FXPROP_UPDATEPOS ))
		{
			m_nFollowType = (uint32)fxProp.GetComboVal();
		}
		else if( !_stricmp( fxProp.m_sName, FXPROP_ATTACHNAME ))
		{
			fxProp.GetStringVal( m_szAttach );
		}
		else if( !_stricmp( fxProp.m_sName, FXPROP_OFFSET ))
		{
			m_vOffset = fxProp.GetVector();
		}
		else if( !_stricmp( fxProp.m_sName, FXPROP_ROTATEADD ))
		{
			m_vRotAdd = fxProp.GetVector();
		}
		else if( !_stricmp( fxProp.m_sName, FXPROP_MENULAYER ))
		{
			m_nMenuLayer = fxProp.GetIntegerVal();
		}
		else if( !_stricmp( fxProp.m_sName, "Ck" ))
		{
			nNumColorKeys++;
		}
		else if( !_stricmp( fxProp.m_sName, "Sk" ))
		{
			nNumScaleKeys++;
		}
	}

	//allocate our arrays of key types
	debug_deletea( m_pColorKeys );
	m_pColorKeys = debug_newa( FX_COLOURKEY, nNumColorKeys );
	m_nNumColorKeys = 0;

	debug_deletea( m_pScaleKeys );
	m_pScaleKeys = debug_newa( FX_SCALEKEY, nNumScaleKeys );
	m_nNumScaleKeys = 0;

	//now actually read in each key type
	for(nCurrProp = 0; nCurrProp < nNumProps; nCurrProp++)
	{
		FX_PROP& fxProp = pProps[nCurrProp];

		if( !_stricmp( fxProp.m_sName, "Ck" ))
		{
			// Add this key to the list of keys

			FX_COLOURKEY fxClrKey;

			fxClrKey.m_tmKey = fxProp.m_data.m_clrKey.m_tmKey;
			fxClrKey.m_red   = (float) (fxProp.m_data.m_clrKey.m_dwCol & 0x000000FF);
			fxClrKey.m_green = (float)((fxProp.m_data.m_clrKey.m_dwCol & 0x0000FF00) >> 8);
			fxClrKey.m_blue  = (float)((fxProp.m_data.m_clrKey.m_dwCol & 0x00FF0000) >> 16);
			fxClrKey.m_alpha = (float)((fxProp.m_data.m_clrKey.m_dwCol & 0xFF000000) >> 24);

			if(m_pColorKeys)
				m_pColorKeys[m_nNumColorKeys++] = fxClrKey;
		}
		else if( !_stricmp( fxProp.m_sName, "Sk" ))
		{
			// Add this key to the list of keys

			FX_SCALEKEY fxSclKey;

			fxSclKey.m_tmKey = fxProp.m_data.m_fVec4[0];
			fxSclKey.m_scale = fxProp.m_data.m_fVec4[1];

			if(m_pScaleKeys)
				m_pScaleKeys[m_nNumScaleKeys++] = fxSclKey;
		}
	}
	
	return true;
}


CBaseFX::CBaseFX( FXType nType) :
	m_pLTClient			(NULL),
	m_hObject			(NULL),
	m_hParent			(NULL),											
	m_dwState			(0),
	m_nFXType			(nType),
	m_bUpdateColour		(true),
	m_bUpdateScale		(true),
	m_nCurrScaleKey		(0),
	m_tmElapsed			(0),
	m_pProps			(NULL)
{
}


//------------------------------------------------------------------
//
//   FUNCTION : Init()
//
//   PURPOSE  : Initialises base class CBaseFX
//
//------------------------------------------------------------------

bool CBaseFX::Init(ILTClient *pLTClient, FX_BASEDATA *pData, const CBaseFXProps *pProps)
{
	//make sure we have properties
	if(!pProps)
		return false;

	//save our prop pointer
	m_pProps = pProps;

	// Store the client LT pointers

	m_pLTClient = pLTClient;

	// Store the base data

	m_dwID				 = pData->m_dwID;	
	m_hParent			 = pData->m_hParent;
	m_hCamera			 = pData->m_hCamera;

	if (m_hParent)
	{
		m_pLTClient->GetObjectPos(m_hParent, &m_vCreatePos);
		m_pLTClient->GetObjectRotation(m_hParent, &m_rCreateRot);
	}
	else
	{
		m_vCreatePos = pData->m_vPos;
		m_rCreateRot = pData->m_rRot;
	}

	m_nCurrScaleKey					= 0;
	m_nCurrColorKey					= 0;
	m_vPos							= m_vCreatePos + GetProps()->m_vOffset;

	// Success !!

	return true;
}

//------------------------------------------------------------------
//
//   FUNCTION : Render()
//
//   PURPOSE  : This will determine whether or not the effect needs updating
//
//------------------------------------------------------------------

bool CBaseFX::Render()
{
	if(!g_bAppFocus || IsSuspended() || !IsActive())
		return false;

	return true;
}

//------------------------------------------------------------------
//
//   FUNCTION : SuspendedUpdate()
//
//   PURPOSE  : This version of update is called while the effect is suspended so that it can do
//				things like smooth shutdown depending upon the effect type
//
//------------------------------------------------------------------
bool CBaseFX::SuspendedUpdate(float tmFrameTime)
{
	if(!IsActive())
		return false;

	return true;
}

//------------------------------------------------------------------
//
//   FUNCTION : Update()
//
//   PURPOSE  : Called to update the effect. Returns whether or not the effect should update
//
//------------------------------------------------------------------

bool CBaseFX::Update(float tmFrameTime)
{
	if (IsSuspended() || !IsActive())
	{
		return false;
	}

	m_tmElapsed += tmFrameTime;

	LTRotation rRot;
	m_pLTClient->GetObjectRotation(m_hObject, &rRot);

	// We are in our pre update
	
	if (m_hParent)
	{
		// Compute the aligned offset

		LTVector vAlignedOffset;

		LTVector vRight, vUp, vForward;

		m_pLTClient->GetObjectRotation(m_hParent, &rRot);
				
		ILTModel *pModelLT = m_pLTClient->GetModelLT();

		vAlignedOffset =	(rRot.Right() * GetProps()->m_vOffset.x) + 
							(rRot.Up() * GetProps()->m_vOffset.y) + 
							(rRot.Forward() * GetProps()->m_vOffset.z);

		// Depending on the Update type find the position we should be at...

		switch (GetProps()->m_nFollowType)
		{				
			case UP_FIXED :
			{
				// Keep us in place

				m_vPos = m_vCreatePos + vAlignedOffset;
			}
			break;

			case UP_FOLLOW :
			{
				// Move with our parent

				m_pLTClient->GetObjectPos(m_hParent, &m_vPos);
				m_vPos += vAlignedOffset;
			}
			break;

			case UP_PLAYERVIEW :		
			{
				// Let game code set our position.	
			}
			break;

			case UP_NODEATTACH :
			{
				// Move with our attach node if one was specified...

				LTransform lTrans;
				HMODELNODE hNode;
				
				if( LT_OK == pModelLT->GetNode( m_hParent, GetProps()->m_szAttach, hNode ) )
				{
					if( LT_OK == pModelLT->GetNodeTransform( m_hParent, hNode, lTrans, LTTRUE ) )
					{	
						m_vPos	= lTrans.m_Pos + vAlignedOffset;
						rRot	= lTrans.m_Rot;
					}
				}
			}
			break;

			case UP_SOCKETATTACH :
			{
				// Move with our attach socket if one was specified...

				LTransform		lTrans;
				HMODELSOCKET	hSocket;

				if( LT_OK == pModelLT->GetSocket( m_hParent, GetProps()->m_szAttach, hSocket ) )
				{
					if( LT_OK == pModelLT->GetSocketTransform( m_hParent, hSocket, lTrans, LTTRUE ) )
					{
						m_vPos	= lTrans.m_Pos + vAlignedOffset;
						rRot	= lTrans.m_Rot;
					}
				}
				else
				{
					if( GetAttachmentSocketTransform( m_hParent, GetProps()->m_szAttach, lTrans.m_Pos, lTrans.m_Rot ))
					{
						m_vPos	= lTrans.m_Pos + vAlignedOffset;
						rRot	= lTrans.m_Rot;
					}
				}
			}
			break;

			case UP_PVNODEATTACH :
			{
				// Move with our attach node if one was specified...

				LTransform lTrans;
				HMODELNODE hNode;
				
				if( LT_OK == pModelLT->GetNode( m_hParent, GetProps()->m_szAttach, hNode ) )
				{
					if( LT_OK == pModelLT->GetNodeTransform( m_hParent, hNode, lTrans, LTFALSE ) )
					{	
						//grab the position of the object to compensate for offset
						LTVector vObjectPos;
						m_pLTClient->GetObjectPos(m_hParent, &vObjectPos);

						m_vPos	= lTrans.m_Pos + vAlignedOffset + vObjectPos;
						rRot	= lTrans.m_Rot;

					}
				}
			}
			break;

			case UP_PVSOCKETATTACH:
			{
				// Move with our attach socket if one was specified...

				LTransform		lTrans;
				HMODELSOCKET	hSocket;

				if( LT_OK == pModelLT->GetSocket( m_hParent, GetProps()->m_szAttach, hSocket ) )
				{
					if( LT_OK == pModelLT->GetSocketTransform( m_hParent, hSocket, lTrans, LTFALSE ) )
					{
						//grab the position of the object to compensate for offset
						LTVector vObjectPos;
						m_pLTClient->GetObjectPos(m_hParent, &vObjectPos);

						m_vPos	= lTrans.m_Pos + vAlignedOffset + vObjectPos;
						rRot	= lTrans.m_Rot;
					}
				}
			}
			break;

			default:
				ASSERT( false ); // Check the property values cause if we got here, "UpdatePos" has to be f'd up!!
				break;
		}
	}

	// Move object to correct position

	LTVector vReal = m_vPos;

	LTVector vCurPos;
	m_pLTClient->GetObjectPos( m_hObject, &vCurPos );
	if( vCurPos != vReal )
		m_pLTClient->SetObjectPos(m_hObject, &vReal );

	LTRotation rCurRot;
	m_pLTClient->GetObjectRotation( m_hObject, &rCurRot );
	if( rCurRot != rRot )
		m_pLTClient->SetObjectRotation( m_hObject, &rRot );

	// Update the colour

	if (m_bUpdateColour && GetProps()->m_pColorKeys)
	{
		CalcColour(m_tmElapsed, GetLifespan(), &m_red, &m_green, &m_blue, &m_alpha, &m_nCurrColorKey);
		m_pLTClient->SetObjectColor(m_hObject, m_red, m_green, m_blue, m_alpha);
	}

	// Compute the current scale based on keyframes and update the 
	// keyframe pointer
	if (m_bUpdateScale && GetProps()->m_pScaleKeys)
	{
		CalcScale(m_tmElapsed, GetLifespan(), &m_scale, &m_nCurrScaleKey);

		uint32 dwObjectType;
		m_pLTClient->Common()->GetObjectType(m_hObject, &dwObjectType);

		if ((dwObjectType == OT_MODEL) || (dwObjectType == OT_SPRITE))
		{
			LTVector vScale;
			vScale.Init(m_scale, m_scale, m_scale);

			LTVector vCurScale;
			m_pLTClient->GetObjectScale( m_hObject, &vCurScale );
			if( vCurScale != vScale )
				m_pLTClient->SetObjectScale(m_hObject, &vScale);
		}
	}

	return true;
}

//------------------------------------------------------------------
//
//   FUNCTION : CalcColour()
//
//   PURPOSE  : Calculates the current colour based on parms
//
//------------------------------------------------------------------

void CBaseFX::CalcColour(float tmElapsed, float tmLifespan, float *pRed, float *pGreen, float *pBlue, float *pAlpha, uint32* pKeyHint)
{
	float tmActual = (tmElapsed / tmLifespan);
	tmActual = (float)fmod(tmActual, 1.0f);

	uint32 nCurrColour = 0;

	FX_COLOURKEY*	pKeys = GetProps()->m_pColorKeys;
	uint32			nNumKeys = GetProps()->m_nNumColorKeys;

	//see if we should use the specified hint if it is provided (this saves us from having
	//to run through a lot of keys to get up to that point
	if(pKeyHint && (pKeys[*pKeyHint].m_tmKey < tmActual))
		nCurrColour = *pKeyHint;
	

	// Locate the keyframe
	for (; nCurrColour + 1 < nNumKeys; nCurrColour++)
	{
		FX_COLOURKEY& endKey	= pKeys[nCurrColour + 1];

		if (tmActual < endKey.m_tmKey)
		{
			FX_COLOURKEY& startKey	= pKeys[nCurrColour];

			// Use this and the previous key to compute the colour

			float tmDist = endKey.m_tmKey - startKey.m_tmKey;
			
			if (tmDist > 0.0f)
			{
				float ratio = (tmActual - startKey.m_tmKey) / tmDist;
				
				*pRed	= (startKey.m_red + ((endKey.m_red - startKey.m_red) * ratio)) / 255.0f;
				*pGreen = (startKey.m_green + ((endKey.m_green - startKey.m_green) * ratio)) / 255.0f;
				*pBlue	= (startKey.m_blue + ((endKey.m_blue - startKey.m_blue) * ratio)) / 255.0f;
				*pAlpha = (startKey.m_alpha + (endKey.m_alpha - startKey.m_alpha) * ratio) / 255.0f;
			}
			else
			{
				*pAlpha = endKey.m_red / 255.0f;
				*pGreen = endKey.m_green / 255.0f;
				*pBlue	= endKey.m_blue / 255.0f;
				*pAlpha = endKey.m_alpha / 255.0f;
			}

			//invert the alpha
			*pAlpha = 1.0f - *pAlpha;

			//all done calculating colors, might as well bail
			break;
		}
	}

	//save the hint back out
	if(pKeyHint)
		*pKeyHint = nCurrColour;
}

//------------------------------------------------------------------
//
//   FUNCTION : CalcScale()
//
//   PURPOSE  : Calulates object scale
//
//------------------------------------------------------------------

void CBaseFX::CalcScale(float tmElapsed, float tmLifespan, float *pScale, uint32* pKeyHint)
{
	float tmActual = (tmElapsed / tmLifespan);
	tmActual = (float)fmod(tmActual, 1.0f);

	uint32 nCurrScaleKey = 0;

	FX_SCALEKEY*	pKeys = GetProps()->m_pScaleKeys;
	uint32			nNumKeys = GetProps()->m_nNumScaleKeys;

	//see if we should use the specified hint if it is provided (this saves us from having
	//to run through a lot of keys to get up to that point
	if(pKeyHint && (pKeys[*pKeyHint].m_tmKey < tmActual))
		nCurrScaleKey = *pKeyHint;

	// Locate the keyframe
	for (; nCurrScaleKey + 1 < nNumKeys; nCurrScaleKey++)
	{
		FX_SCALEKEY& endKey		= pKeys[nCurrScaleKey + 1];

		if (tmActual < endKey.m_tmKey)
		{
			FX_SCALEKEY& startKey	= pKeys[nCurrScaleKey];

			// Use this and the previous key to compute the colour

			float tmDist = endKey.m_tmKey - startKey.m_tmKey;
			
			if (tmDist > 0.0f)
			{
				float rat = (endKey.m_scale - startKey.m_scale) / tmDist;
				float tmKey  = tmActual - startKey.m_tmKey;

				*pScale = startKey.m_scale + (rat * tmKey);
			}
			else
			{
				*pScale = startKey.m_scale;
			}

			//got the scale, bail
			break;
		}
	}
	
	//save the hint back out
	if(pKeyHint)
		*pKeyHint = nCurrScaleKey;

}


void CBaseFX::Pause(bool bPause)
{
	if(m_hObject && m_pLTClient)
	{
		m_pLTClient->Common()->SetObjectFlags(m_hObject, OFT_Flags, (bPause) ? FLAG_PAUSED : 0, FLAG_PAUSED);
	}
}


void CBaseFX::CreateDummyObject()
{
	ObjectCreateStruct ocs;
	
	LTVector vScale;
	vScale.x = 1.0f;
	vScale.y = 1.0f;
	vScale.z = 1.0f;
	ocs.m_ObjectType = OT_NORMAL;
	ocs.m_Pos = m_vCreatePos;
	ocs.m_Rotation = m_rCreateRot;
	ocs.m_Scale = vScale;

	if (!m_hObject) m_hObject = m_pLTClient->CreateObject(&ocs);
}
