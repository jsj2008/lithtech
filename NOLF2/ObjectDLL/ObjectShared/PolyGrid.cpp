// ----------------------------------------------------------------------- //
//
// MODULE  : PolyGrid.cpp
//
// PURPOSE : PolyGrid - Implementation
//
// CREATED : 10/20/97
//
// (c) 1997-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "PolyGrid.h"
#include "iltserver.h"
#include "ClientServerShared.h"
#include "SFXMsgIds.h"
#include "CommonUtilities.h"
#include "PolyGridModifier.h"
#include "ObjectMsgs.h"
#include "ParsedMsg.h"
#include "MsgIDs.h"

LINKFROM_MODULE( PolyGrid );

#pragma force_active on
BEGIN_CLASS(PolyGrid)
	ADD_VECTORPROP_FLAG(Dims, PF_DIMS | PF_LOCALDIMS)
    ADD_COLORPROP(Color1, 255.0f, 255.0f, 255.0f)
    ADD_COLORPROP(Color2, 255.0f, 255.0f, 255.0f)
    ADD_STRINGPROP_FLAG(SpriteSurfaceName, "Tex\\Water\\Wa01\\Wa0001.dtx", PF_FILENAME)
	ADD_STRINGPROP_FLAG(EnvMapName, "", PF_FILENAME)
	ADD_STRINGPROP_FLAG(DampenImage, "", PF_FILENAME)

	PROP_DEFINEGROUP(TextureInfo, PF_GROUP(3))
		ADD_REALPROP_FLAG(XScaleMin, 15.0f, PF_GROUP(3))
		ADD_REALPROP_FLAG(XScaleMax, 15.0f, PF_GROUP(3))
		ADD_REALPROP_FLAG(YScaleMin, 15.0f, PF_GROUP(3))
		ADD_REALPROP_FLAG(YScaleMax, 15.0f, PF_GROUP(3))
	    ADD_REALPROP_FLAG(XScaleDuration, 10.0f, PF_GROUP(3))
	    ADD_REALPROP_FLAG(YScaleDuration, 10.0f, PF_GROUP(3))
	    ADD_REALPROP_FLAG(XPan, 10.0f, PF_GROUP(3))
	    ADD_REALPROP_FLAG(YPan, 10.0f, PF_GROUP(3))

	ADD_REALPROP(Alpha, 0.7f)
    ADD_BOOLPROP(Additive, LTFALSE)
    ADD_BOOLPROP(Multiply, LTFALSE)
	ADD_BOOLPROP(Fresnel, LTTRUE)
	ADD_BOOLPROP(RenderEarly, LTFALSE)
	ADD_BOOLPROP(BackfaceCull, LTTRUE)
	ADD_BOOLPROP(NormalMapSprite, LTFALSE)
	ADD_LONGINTPROP(NumPoliesX, 16)
	ADD_LONGINTPROP(NumPoliesY, 16)
	ADD_LONGINTPROP_FLAG(SurfaceType, 1, 0)
	ADD_REALPROP(BaseReflection, 0.5f)
	ADD_REALPROP(VolumeIOR, 1.33f)
		
	PROP_DEFINEGROUP(PlasmaInfo, PF_GROUP(1))
		ADD_LONGINTPROP_FLAG(Ring1Rate, 50, PF_GROUP(1))
		ADD_LONGINTPROP_FLAG(Ring2Rate, 10, PF_GROUP(1))
		ADD_LONGINTPROP_FLAG(Ring3Rate, 30, PF_GROUP(1))
		ADD_LONGINTPROP_FLAG(Ring4Rate, 20, PF_GROUP(1))

	PROP_DEFINEGROUP(WavePropInfo, PF_GROUP(2))
		ADD_REALPROP_FLAG(TimeScale, 1.0f, PF_GROUP(2))
		ADD_REALPROP_FLAG(DampenScale, 0.99f, PF_GROUP(2))
		ADD_REALPROP_FLAG(SpringCoeff, 40.0f, PF_GROUP(2))		
		ADD_REALPROP_FLAG(ModelDisplace, 10.0f, PF_GROUP(2))
		ADD_REALPROP_FLAG(MinFrameRate, 10.0f, PF_GROUP(2))
		ADD_LONGINTPROP_FLAG(NumStartupFrames, 0, PF_GROUP(2))
		ADD_STRINGPROP_FLAG(Modifier1, "", PF_OBJECTLINK | PF_GROUP(2))
		ADD_STRINGPROP_FLAG(Modifier2, "", PF_OBJECTLINK | PF_GROUP(2))
		ADD_STRINGPROP_FLAG(Modifier3, "", PF_OBJECTLINK | PF_GROUP(2))
		ADD_STRINGPROP_FLAG(Modifier4, "", PF_OBJECTLINK | PF_GROUP(2))

END_CLASS_DEFAULT(PolyGrid, CClientSFX, NULL, NULL)
#pragma force_active off

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	LTBOOL ValidateMsgModifier
//
//  PURPOSE:	Validate the color message...
//
// ----------------------------------------------------------------------- //

static LTBOOL ValidateMsgModifier( ILTPreInterface *pInterface, ConParse &cpMsgParams )
{
	if( cpMsgParams.m_nArgs != 2 )
	{
		pInterface->ShowDebugWindow( LTTRUE );
		pInterface->CPrint( "ERROR! - ValidateMsgModifier()" );
		pInterface->CPrint( "    MSG - Invalid number of arguments." );
		return LTFALSE;
	}
	//make sure that the second parameter is a value in range
	uint32 nVal = (uint32)(atoi(cpMsgParams.m_Args[1]) - 1);
	if(nVal >= PolyGrid::MAX_MODIFIERS)
	{
		pInterface->ShowDebugWindow( LTTRUE );
		pInterface->CPrint( "ERROR! - Polygrid modifier index is too large" );
	}

	return LTTRUE;
}

CMDMGR_BEGIN_REGISTER_CLASS( PolyGrid )

	CMDMGR_ADD_MSG( ON, -1, ValidateMsgModifier, "ON <Modifier>" ) 
	CMDMGR_ADD_MSG( OFF, -1, ValidateMsgModifier, "OFF <Modifier>" ) 

CMDMGR_END_REGISTER_CLASS( PolyGrid, CClientSFX )


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ReadHStringProp
//
//	PURPOSE:	Utility function to read in a string property and create
//				an HString from it. The callee is responsible for freeing
//				the returned string
//
// ----------------------------------------------------------------------- //
HSTRING ReadHStringProp(const char* pszPropName, ILTServer* pServer)
{
	char buf[MAX_CS_FILENAME_LEN];

	buf[0] = '\0';
	pServer->GetPropString(pszPropName, buf, MAX_CS_FILENAME_LEN);

	if (buf[0]) 
		return pServer->CreateString(buf);

	return NULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PolyGrid::PolyGrid
//
//	PURPOSE:	Initialize
//
// ----------------------------------------------------------------------- //

PolyGrid::PolyGrid() : CClientSFX()
{
	//non-saved data
	m_bCreatedFromSave = false;

	//saved data

	VEC_SET(m_vDims, 32.0f, 32.0f, 0.0f);
	VEC_SET(m_vColor1, 50.0f, 50.0f, 200.0f);
	VEC_SET(m_vColor2, 0.0f, 0.0f, 220.0f);

	m_fXScaleMin = 15.0f;
	m_fXScaleMax = 15.0f;
	m_fYScaleMin = 15.0f;
	m_fYScaleMax = 15.0f;
	m_fXScaleDuration = 10.0f;
	m_fYScaleDuration = 10.0f;
	m_fXPan = 10.0f;
	m_fYPan = 10.0f;
	m_fBaseReflection = 0.5f;
	m_fVolumeIOR = 1.33f;
	m_fAlpha = 0.7f;
    m_bAdditive = LTFALSE;
    m_bMultiply = LTFALSE;
	m_bFresnel = LTTRUE;
	m_bBackfaceCull = LTTRUE;
	m_bRenderEarly = LTFALSE;
	m_bNormalMapSprite = LTFALSE;

	m_nSurfaceType = 1;

	m_dwNumPoliesX = 16;
	m_dwNumPoliesY = 16;

	m_nNumStartupFrames = 0;

	//plasma info
	m_nRingRate[0] = 50;
	m_nRingRate[1] = 10;
	m_nRingRate[2] = 30;
	m_nRingRate[3] = 20;

	//wave prop info
	m_fDampenScale = 0.99f;
	m_fTimeScale = 1.0f;
	m_fSpringCoeff = 40.0f;
	m_fModelDisplace = 10.0f;
	m_fMinFrameRate = 10.0f;

	//init each modifier
	m_nActiveModifiers = 0;

	for(uint32 nCurrMod = 0; nCurrMod < MAX_MODIFIERS; nCurrMod++)
	{
		m_nNumAccelPoints[nCurrMod]		= 0;
		m_fAccelAmount[nCurrMod]		= 0.0f;
		m_nXMin[nCurrMod]				= 0;
		m_nYMin[nCurrMod]				= 0;
		m_nXMax[nCurrMod]				= 0;
		m_nYMax[nCurrMod]				= 0;
	}

	m_hVolumeBrush = LTNULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PolyGrid::~PolyGrid
//
//	PURPOSE:	Deallocate
//
// ----------------------------------------------------------------------- //

PolyGrid::~PolyGrid()
{
    ILTServer* pServerDE = GetServerDE();
	if (!pServerDE) return;

	FreeModifierStrings();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PolyGrid::Setup
//
//	PURPOSE:	given a message, this will fill it out with the polygrid data
//
// ----------------------------------------------------------------------- //
void PolyGrid::CreateSFXMessage(ILTMessage_Write& cMsg)
{
	uint16 wColor1 = Color255VectorToWord( &m_vColor1 );
	uint16 wColor2 = Color255VectorToWord( &m_vColor2 );

	cMsg.WriteLTVector(m_vDims);
	cMsg.Writeuint16(wColor1);
	cMsg.Writeuint16(wColor2);
	cMsg.Writefloat(m_fXScaleMin);
	cMsg.Writefloat(m_fXScaleMax);
	cMsg.Writefloat(m_fYScaleMin);
	cMsg.Writefloat(m_fYScaleMax);
	cMsg.Writefloat(m_fXScaleDuration);
	cMsg.Writefloat(m_fYScaleDuration);
	cMsg.Writefloat(m_fXPan);
	cMsg.Writefloat(m_fYPan);
	cMsg.Writefloat(m_fAlpha);
	cMsg.Writefloat(m_fTimeScale);
	cMsg.Writefloat(m_fDampenScale);
	cMsg.Writefloat(m_fSpringCoeff);
	cMsg.Writefloat(m_fModelDisplace);
	cMsg.Writefloat(m_fMinFrameRate);
	cMsg.Writefloat(m_fBaseReflection);
	cMsg.Writefloat(m_fVolumeIOR);
	cMsg.WriteString(m_sSurfaceSprite.c_str( ));
	cMsg.WriteString(m_sSurfaceEnvMap.c_str( ));
	cMsg.WriteString(m_sDampenImage.c_str( ));
    cMsg.Writeuint16((uint16)m_dwNumPoliesX);
	cMsg.Writeuint16((uint16)m_dwNumPoliesY);
	cMsg.Writeuint16((uint16)m_nNumStartupFrames);
	cMsg.Writebool(m_bAdditive);
	cMsg.Writebool(m_bMultiply);
	cMsg.Writebool(m_bFresnel);
	cMsg.Writebool(m_bBackfaceCull);
	cMsg.Writebool(m_bRenderEarly);
	cMsg.Writebool(m_bNormalMapSprite);
	cMsg.Writeuint8(m_nSurfaceType);
	cMsg.Writeuint8(m_nRingRate[0]);
	cMsg.Writeuint8(m_nRingRate[1]);
	cMsg.Writeuint8(m_nRingRate[2]);
	cMsg.Writeuint8(m_nRingRate[3]);

	//write out our modifier data
	cMsg.Writeuint8(m_nActiveModifiers);
	for(uint32 nCurrMod = 0; nCurrMod < MAX_MODIFIERS; nCurrMod++)
	{
		cMsg.Writefloat(m_fAccelAmount[nCurrMod]);
		cMsg.Writeuint16(m_nNumAccelPoints[nCurrMod]);
		cMsg.Writeuint16(m_nXMin[nCurrMod]);
		cMsg.Writeuint16(m_nYMin[nCurrMod]);
		cMsg.Writeuint16(m_nXMax[nCurrMod]);
		cMsg.Writeuint16(m_nYMax[nCurrMod]);
	}

	cMsg.WriteObject( m_hVolumeBrush );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PolyGrid::Setup
//
//	PURPOSE:	Initialize
//
// ----------------------------------------------------------------------- //

void PolyGrid::Setup()
{
	if ((m_dwNumPoliesX < 2) || (m_dwNumPoliesY < 2)) 
		return;

	g_pPhysicsLT->SetObjectDims(m_hObject, &m_vDims, 0);


	// Limit the number of polies in each direciton to 32k...
	if ( m_dwNumPoliesX > 0x7FFF )
		m_dwNumPoliesX = 0x7FFF;

	if ( m_dwNumPoliesY > 0x7FFF )
		m_dwNumPoliesY = 0x7FFF;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PolyGrid::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 PolyGrid::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			if (fData == PRECREATE_WORLDFILE ||
				fData == PRECREATE_STRINGPROP ||
				fData == PRECREATE_NORMAL)
			{
				ReadProp((ObjectCreateStruct *)pData);
			}
			break;
		}

		case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
				Setup();
			}
			else
			{
				m_bCreatedFromSave = true;
			}
			break;
		}

		case MID_LOADOBJECT:
		{
			Load((ILTMessage_Read*)pData, (uint32)fData);
			break;
		}

		case MID_SAVEOBJECT:
		{
			Save((ILTMessage_Write*)pData, (uint32)fData);			
			break;
		}

		case MID_ALLOBJECTSCREATED:
		{
			if(!m_bCreatedFromSave)
			{
				SetupModifiers();
				UpdateClients();
			}

			// Don't eat ticks please...
			SetNextUpdate(UPDATE_NEVER);

			break;
		}


		case MID_UPDATE:
		{
		}
		break;

		default : break;
	}

	return CClientSFX::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PolyGrid::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

void PolyGrid::ReadProp(ObjectCreateStruct * pStruct)
{
    ILTServer* pServerDE = GetServerDE();
	if (!pServerDE) return;

	pStruct->m_Flags = FLAG_VISIBLE | FLAG_FORCECLIENTUPDATE;

	GenericProp genProp;
	if( g_pLTServer->GetPropGeneric( "SpriteSurfaceName", &genProp ) == LT_OK && genProp.m_String[0] )
	{
		m_sSurfaceSprite = genProp.m_String;
	}

	if( g_pLTServer->GetPropGeneric( "EnvMapName", &genProp ) == LT_OK && genProp.m_String[0] )
	{
		m_sSurfaceEnvMap = genProp.m_String;
	}

	if( g_pLTServer->GetPropGeneric( "DampenImage", &genProp ) == LT_OK && genProp.m_String[0] )
	{
		m_sDampenImage = genProp.m_String;
	}

    pServerDE->GetPropVector("Dims", &m_vDims);
	pServerDE->GetPropVector("Color1", &m_vColor1);
	pServerDE->GetPropVector("Color2", &m_vColor2);
	pServerDE->GetPropReal("XScaleMin", &m_fXScaleMin);
	pServerDE->GetPropReal("XScaleMax", &m_fXScaleMax);
	pServerDE->GetPropReal("YScaleMin", &m_fYScaleMin);
	pServerDE->GetPropReal("YScaleMax", &m_fYScaleMax);
	pServerDE->GetPropReal("XScaleDuration", &m_fXScaleDuration);
	pServerDE->GetPropReal("YScaleDuration", &m_fYScaleDuration);
	pServerDE->GetPropReal("XPan", &m_fXPan);
	pServerDE->GetPropReal("YPan", &m_fYPan);
	pServerDE->GetPropReal("BaseReflection", &m_fBaseReflection);
	pServerDE->GetPropReal("VolumeIOR", &m_fVolumeIOR);
	pServerDE->GetPropReal("Alpha", &m_fAlpha);
	pServerDE->GetPropReal("TimeScale", &m_fTimeScale);
	pServerDE->GetPropReal("DampenScale", &m_fDampenScale);
	pServerDE->GetPropReal("SpringCoeff", &m_fSpringCoeff);
	pServerDE->GetPropReal("ModelDisplace", &m_fModelDisplace);	
	pServerDE->GetPropReal("MinFrameRate", &m_fMinFrameRate);
	pServerDE->GetPropBool("Additive", &m_bAdditive);
	pServerDE->GetPropBool("Multiply", &m_bMultiply);
	pServerDE->GetPropBool("Fresnel", &m_bFresnel);
	pServerDE->GetPropBool("BackfaceCull", &m_bBackfaceCull);
	pServerDE->GetPropBool("RenderEarly", &m_bRenderEarly);
	pServerDE->GetPropBool("NormalMapSprite", &m_bNormalMapSprite);

	int32 nTempLong;
	if(pServerDE->GetPropLongInt("NumPoliesX", &nTempLong) == LT_OK)
	{
		m_dwNumPoliesX = nTempLong;
	}

	if(pServerDE->GetPropLongInt("NumPoliesY", &nTempLong) == LT_OK)
	{
		m_dwNumPoliesY = nTempLong;
	}

	if(pServerDE->GetPropLongInt("NumStartupFrames", &nTempLong) == LT_OK)
	{
		m_nNumStartupFrames = nTempLong;
	}

	if(pServerDE->GetPropLongInt("SurfaceType", &nTempLong) == LT_OK)
	{
		m_nSurfaceType = (uint8)nTempLong;
	}

	//Plasma information
	if(pServerDE->GetPropLongInt("Ring1Rate", &nTempLong) == LT_OK)
	{
		m_nRingRate[0] = (uint8)nTempLong;
	}

	if(pServerDE->GetPropLongInt("Ring2Rate", &nTempLong) == LT_OK)
	{
		m_nRingRate[1] = (uint8)nTempLong;
	}

	if(pServerDE->GetPropLongInt("Ring3Rate", &nTempLong) == LT_OK)
	{
		m_nRingRate[2] = (uint8)nTempLong;
	}

	if(pServerDE->GetPropLongInt("Ring4Rate", &nTempLong) == LT_OK)
	{
		m_nRingRate[3] = (uint8)nTempLong;
	}

	//read in all the modifier names
	if( g_pLTServer->GetPropGeneric( "Modifier1", &genProp ) == LT_OK && genProp.m_String[0] )
	{
		m_sModifierName[0] = genProp.m_String;
	}
	if( g_pLTServer->GetPropGeneric( "Modifier2", &genProp ) == LT_OK && genProp.m_String[0] )
	{
		m_sModifierName[1] = genProp.m_String;
	}
	if( g_pLTServer->GetPropGeneric( "Modifier3", &genProp ) == LT_OK && genProp.m_String[0] )
	{
		m_sModifierName[2] = genProp.m_String;
	}
	if( g_pLTServer->GetPropGeneric( "Modifier4", &genProp ) == LT_OK && genProp.m_String[0] )
	{
		m_sModifierName[3] = genProp.m_String;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PolyGrid::SetupModifiers
//
//	PURPOSE:	with all the object names filled out, it will search for the
//				objects and take information from them, filling out its internal
//				information
//
// ----------------------------------------------------------------------- //
void PolyGrid::SetupModifiers()
{
    ILTServer* pServerDE = GetServerDE();
	if (!pServerDE) return;

	for(uint32 nCurrMod = 0; nCurrMod < MAX_MODIFIERS; nCurrMod++)
	{
		//first off see if there is a name
		if(!m_sModifierName[nCurrMod].size( ))
			continue;

		//now grab the object that the name refers to
		ObjArray<HOBJECT, 1> ObjList;
		uint32 nNumFound;

		pServerDE->FindNamedObjects(m_sModifierName[nCurrMod].c_str( ), ObjList, &nNumFound);

		//grab the first object if applicable
		HOBJECT hObj = NULL;

		if(nNumFound > 0)
			hObj = ObjList.GetObject(0);

		//and now see if we found an object
		if(hObj == NULL)
			continue;

		//make sure that the object is the correct class
		if(pServerDE->GetObjectClass(hObj) != pServerDE->GetClass("PolyGridModifier"))
			continue;

		//we now have the object, we need to find the intersection with the polygrid
		PolyGridModifier* pMod = (PolyGridModifier*)pServerDE->HandleToObject(hObj);

		//alright, now we need to figure out the area we are projecting onto
		LTVector vPos, vModPos;
		pServerDE->GetObjectPos(hObj, &vModPos);
		pServerDE->GetObjectPos(m_hObject, &vPos);

		//ok, now we need to find the orienation of the polygrid
		LTRotation rRot;
		pServerDE->GetObjectRotation(m_hObject, &rRot);

		//now get the basis vectors of the object space
		LTVector vRight		= rRot.Right();
		LTVector vForward	= rRot.Forward();

		//now find the point of the object projected onto the plane relative to the
		//object position
		float fX = vRight.Dot(vModPos - vPos) + m_vDims.x;
		float fY = vForward.Dot(vModPos - vPos) + m_vDims.z;

		//now convert this to actual integer ranges
		int32 nXMin = (int32)floor((fX - pMod->m_vDims.x) * m_dwNumPoliesX / (m_vDims.x * 2.0f));
		int32 nYMin = (int32)floor((fY - pMod->m_vDims.z) * m_dwNumPoliesY / (m_vDims.z * 2.0f));
		int32 nXMax = (int32)ceil((fX + pMod->m_vDims.x) * m_dwNumPoliesX / (m_vDims.x * 2.0f));
		int32 nYMax = (int32)ceil((fY + pMod->m_vDims.z) * m_dwNumPoliesY / (m_vDims.z * 2.0f));
		
		//now we need to clamp this to our polygon
		nXMin = LTMAX(1, nXMin);
		nYMin = LTMAX(1, nYMin);
		nXMax = LTMIN((int32)m_dwNumPoliesX - 1, nXMax);
		nYMax = LTMIN((int32)m_dwNumPoliesY - 1, nYMax);

		//see if we actually have any area!
		if((nXMin >= nXMax) || (nYMin >= nYMax))
			continue;

		//alright, we actually have all the data we need, so we can setup this
		//modifier

		//see if this should be enabled or not
		if(pMod->m_bStartEnabled)
			m_nActiveModifiers |= (1 << nCurrMod);

		//copy over the data
		m_nNumAccelPoints[nCurrMod]		= pMod->m_nNumAccelPoints;
		m_fAccelAmount[nCurrMod]		= pMod->m_fAccelAmount;

		//save the rectangle
		m_nXMin[nCurrMod] = (uint16)nXMin;
		m_nYMin[nCurrMod] = (uint16)nYMin;
		m_nXMax[nCurrMod] = (uint16)nXMax;
		m_nYMax[nCurrMod] = (uint16)nYMax;

		//move onto our next one
	}

	//we can now free the modifier strings
	FreeModifierStrings();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PolyGrid::FreeModiferStrings
//
//	PURPOSE:	Clear the modifiers.
//
// ----------------------------------------------------------------------- //
void PolyGrid::FreeModifierStrings()
{
    ILTServer* pServerDE = GetServerDE();
	if (!pServerDE) return;

	for(uint32 nCurrMod = 0; nCurrMod < MAX_MODIFIERS; nCurrMod++)
	{
		m_sModifierName[nCurrMod].erase( );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PolyGrid::UpdateClients
//
//	PURPOSE:	Sends the current TextureFX status to the clients
//
// ----------------------------------------------------------------------- //

void PolyGrid::UpdateClients()
{
	{
		// Set up the update message
		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_SFX_MESSAGE);
		cMsg.Writeuint8(SFX_POLYGRID_ID);
		cMsg.WriteObject(m_hObject);
		cMsg.Writeuint8(m_nActiveModifiers);

		// Send the message to all connected clients
		g_pLTServer->SendToClient(cMsg.Read(), LTNULL, MESSAGE_GUARANTEED);
	}
	
	{
		CAutoMessage cMsg;
		cMsg.Writeuint8(SFX_POLYGRID_ID);
		CreateSFXMessage(cMsg);

		// Make sure new clients will get the message
		g_pLTServer->SetObjectSFXMessage(m_hObject, cMsg.Read());
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PolyGrid::OnTrigger()
//
//	PURPOSE:	Called when triggered.
//
// ----------------------------------------------------------------------- //

bool PolyGrid::OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg)
{
	static CParsedMsg::CToken s_cTok_On("ON");
	static CParsedMsg::CToken s_cTok_Off("OFF");

	//keep track of the original active list
	uint8 nPrevActive = m_nActiveModifiers;

	if ((cMsg.GetArg(0) == s_cTok_On) && (cMsg.GetArgCount() >= 2))
	{
		uint32 nModifier = (uint32)atoi(cMsg.GetArg(1)) - 1;

		if(nModifier < MAX_MODIFIERS)
		{
			m_nActiveModifiers |= (1 << nModifier);
		}
	}
	else if((cMsg.GetArg(0) == s_cTok_Off) && (cMsg.GetArgCount() >= 2))
	{
		uint32 nModifier = (uint32)atoi(cMsg.GetArg(1)) - 1;

		if(nModifier < MAX_MODIFIERS)
		{
			m_nActiveModifiers &= ~(1 << nModifier);
		}
	}
	else
		return CClientSFX::OnTrigger(hSender, cMsg);

	// Tell the clients about the change
	if(nPrevActive != m_nActiveModifiers)
		UpdateClients();

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PolyGrid::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void PolyGrid::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
 	if (!pMsg) 
		return;

	CreateSFXMessage(*pMsg);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PolyGrid::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void PolyGrid::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
 	if (!pMsg) 
		return;

	ILTMessage_Read& cMsg = *pMsg;

	m_vDims = cMsg.ReadLTVector();
	
	uint16 wColor1 = cMsg.Readuint16();
	Color255WordToVector(wColor1, &(m_vColor1));
	
	uint16 wColor2 = cMsg.Readuint16();
	Color255WordToVector(wColor2, &(m_vColor2));
	
	m_fXScaleMin = cMsg.Readfloat();
	m_fXScaleMax = cMsg.Readfloat();
	m_fYScaleMin = cMsg.Readfloat();
	m_fYScaleMax = cMsg.Readfloat();
	m_fXScaleDuration = cMsg.Readfloat();
	m_fYScaleDuration = cMsg.Readfloat();
	m_fXPan = cMsg.Readfloat();
	m_fYPan = cMsg.Readfloat();
	m_fAlpha = cMsg.Readfloat();
	m_fTimeScale = cMsg.Readfloat();
	m_fDampenScale = cMsg.Readfloat();
	m_fSpringCoeff = cMsg.Readfloat();
	m_fModelDisplace = cMsg.Readfloat();
	m_fMinFrameRate = cMsg.Readfloat();
	m_fBaseReflection = cMsg.Readfloat();
	m_fVolumeIOR = cMsg.Readfloat();

	char szString[256];

	cMsg.ReadString( szString, ARRAY_LEN( szString ));
	m_sSurfaceSprite = szString;

	cMsg.ReadString( szString, ARRAY_LEN( szString ));
	m_sSurfaceEnvMap = szString;

	cMsg.ReadString( szString, ARRAY_LEN( szString ));
	m_sDampenImage = szString;

    m_dwNumPoliesX = cMsg.Readuint16();
	m_dwNumPoliesY = cMsg.Readuint16();
	m_nNumStartupFrames = cMsg.Readuint16();
	m_bAdditive = cMsg.Readbool();
	m_bMultiply = cMsg.Readbool();
	m_bFresnel = cMsg.Readbool();
	m_bBackfaceCull = cMsg.Readbool();
	m_bRenderEarly = cMsg.Readbool();
	m_bNormalMapSprite = cMsg.Readbool();
	m_nSurfaceType = cMsg.Readuint8();
	m_nRingRate[0] = cMsg.Readuint8();
	m_nRingRate[1] = cMsg.Readuint8();
	m_nRingRate[2] = cMsg.Readuint8();
	m_nRingRate[3] = cMsg.Readuint8();

	//write out our modifier data
	m_nActiveModifiers = cMsg.Readuint8();
	for(uint32 nCurrMod = 0; nCurrMod < MAX_MODIFIERS; nCurrMod++)
	{
		m_fAccelAmount[nCurrMod] = cMsg.Readfloat();
		m_nNumAccelPoints[nCurrMod] = cMsg.Readuint16();
		m_nXMin[nCurrMod] = cMsg.Readuint16();
		m_nYMin[nCurrMod] = cMsg.Readuint16();
		m_nXMax[nCurrMod] = cMsg.Readuint16();
		m_nYMax[nCurrMod] = cMsg.Readuint16();
	}

	m_hVolumeBrush = cMsg.ReadObject();
}
