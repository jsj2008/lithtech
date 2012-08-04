//------------------------------------------------------------------
//
//   MODULE    : SCREENEFFECT.CPP
//
//   PURPOSE   : Implements the ScreenEffect class
//
//   CREATED   : 11/30/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
//------------------------------------------------------------------

#include "Stdafx.h"
#include "ScreenEffect.h"
#include "ScreenEffectDB.h"

LINKFROM_MODULE( ScreenEffect );

//Note: Since this object can also be created at runtime, any updates to these properties must also
//be updated in ScreenEffect::HandleEffectMsg
BEGIN_CLASS( ScreenEffect )
	ADD_BOOLPROP( StartOn, true, "If true the screen effects will start playing immediately." )
END_CLASS( ScreenEffect, GameBase, "This object adds screen effects to the client, and handles messages for controlling the effects." )

CMDMGR_BEGIN_REGISTER_CLASS( ScreenEffect )

	ADD_MESSAGE( ON, 1, NULL, MSG_HANDLER( ScreenEffect, HandleOnMsg ), "ON", "Tells the ScreenEffect object to play its effect", "msg ScreenEffect ON" )
	ADD_MESSAGE( OFF, 1, NULL, MSG_HANDLER( ScreenEffect, HandleOffMsg ), "OFF", "Tells the ScreenEffect object to stop playing its effect", "msg ScreenEffect OFF" )
	ADD_MESSAGE( TOGGLE, 1, NULL, MSG_HANDLER( ScreenEffect, HandleToggleMsg ), "TOGGLE", "Tells the ScreenEffect object to toggle its on/off state.", "msg ScreenEffect TOGGLE" )
	ADD_MESSAGE( PARAM, 4, ScreenEffect::ValidateMsgParam, MSG_HANDLER( ScreenEffect, HandleParamMsg ), "PARAM", "Tells the ScreenEffect object to change the specified parameter to the specified value over the specified time period", "msg ScreenEffect PARAM <parameter> <value> <ramptime>" )

CMDMGR_END_REGISTER_CLASS( ScreenEffect, GameBase )

ScreenEffect::ScreenEffect() : GameBase(OT_NORMAL)
{
}

ScreenEffect::~ScreenEffect()
{
}

//////////////////////////////////////////////////////////////////////////
// Engine event handlers

uint32 ScreenEffect::OnObjectCreated(const GenericPropList* pProps, float createType)
{
	uint32 nResult = GameBase::OnObjectCreated(pProps, createType);
	
	DATABASE_CATEGORY(ScreenEffect).Init();
	m_hRecord = DATABASE_CATEGORY(ScreenEffect).GetRecordByName("Default");
	m_hParameters = DATABASE_CATEGORY(ScreenEffect).GetAttribute(m_hRecord, "Parameters");

	g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, FLAG_FORCECLIENTUPDATE, FLAG_FORCECLIENTUPDATE);

	SetOnOff(pProps->GetBool("StartOn", true));
	
	CreateSFX();
	
	return nResult;
}

//////////////////////////////////////////////////////////////////////////
// Message handlers

bool ScreenEffect::ValidateMsgParam(ILTPreInterface *pInterface, ConParse &cParams)
{
	char szMsgName[16] = {0};

	LTStrCpy( szMsgName, cParams.m_Args[0], LTARRAYSIZE(szMsgName));

	if (cParams.m_nArgs != 4)
	{
		if( CCommandMgrPlugin::s_bShowMsgErrors )
		{
			pInterface->ShowDebugWindow(true);
			pInterface->CPrint( "ERROR! - %s", __FUNCTION__ );
			pInterface->CPrint("    MSG - %s - Invalid number of parameters!", LTStrUpr(szMsgName));
		}
		return false;
	}

	//DATABASE_CATEGORY(ScreenEffect).Init();
	HRECORD hRecord = DATABASE_CATEGORY(ScreenEffect).GetRecordByName("Default");
	HATTRIBUTE hParameters = DATABASE_CATEGORY(ScreenEffect).GetAttribute(hRecord, "Parameters");

	if (GetParamIndex(hParameters, cParams.m_Args[1]) == k_nInvalidParamIndex)
	{
		if( CCommandMgrPlugin::s_bShowMsgErrors )
		{
			pInterface->ShowDebugWindow(true);
			pInterface->CPrint( "ERROR! - %s", __FUNCTION__ );
			pInterface->CPrint("    MSG - %s - Invalid parameter %s!", LTStrUpr(szMsgName), cParams.m_Args[1]);
		}
		return false;
	}
	
	return true;
}

void ScreenEffect::HandleOnMsg(HOBJECT hSender, const CParsedMsg& cParsedMsg)
{
	SetOnOff(true);
}

void ScreenEffect::HandleOffMsg(HOBJECT hSender, const CParsedMsg& cParsedMsg)
{
	SetOnOff(false);
}

void ScreenEffect::HandleToggleMsg(HOBJECT hSender, const CParsedMsg& cParsedMsg)
{
	SetOnOff(!IsOn());
}

void ScreenEffect::HandleParamMsg(HOBJECT hSender, const CParsedMsg& cParsedMsg)
{
	// Verify the argument count
	if (cParsedMsg.GetArgCount() < 4)
	{
		g_pLTServer->CPrint("ScreenEffect: Invalid PARAM message - Please use the form PARAM Parameter Value RampTime");
		return;
	}
		
	uint32 nParamIndex = GetParamIndex(cParsedMsg.GetArg(1));
	if (nParamIndex == k_nInvalidParamIndex)
	{
		g_pLTServer->CPrint("ScreenEffect: Invalid PARAM message - Unknown Parameter \"%s\"", cParsedMsg.GetArg(1).c_str());
		return;
	}

	CAutoMessage cMsg((uint8)MID_SFX_MESSAGE);
	cMsg.Writeuint8(SFX_SCREENEFFECT_ID);
	cMsg.WriteObject(m_hObject);
	LTASSERT(nParamIndex < 0xFF, "Invalid parameter index encountered");
	cMsg.Writeuint8((uint8)nParamIndex);
	cMsg.Writefloat((float)atof(cParsedMsg.GetArg(2)));
	cMsg.Writefloat((float)atof(cParsedMsg.GetArg(3)));
	g_pLTServer->SendToClient(cMsg.Read(), NULL, MESSAGE_GUARANTEED);
}

//////////////////////////////////////////////////////////////////////////
// Internal functions

bool ScreenEffect::IsOn()
{
	uint32 nUserFlags;
	if (g_pCommonLT->GetObjectFlags(m_hObject, OFT_User, nUserFlags) != LT_OK)
		return false;
	return (nUserFlags & USRFLG_SFX_ON) != 0;
}

void ScreenEffect::SetOnOff(bool bOn)
{
	g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, bOn ? USRFLG_SFX_ON : 0, USRFLG_SFX_ON);
}	

void ScreenEffect::CreateSFX()
{
	CAutoMessage cMsg((uint8)SFX_SCREENEFFECT_ID);
	g_pLTServer->SetObjectSFXMessage(m_hObject, cMsg.Read());
}

uint32 ScreenEffect::GetParamIndex(HATTRIBUTE hAttribute, const char *pParam)
{
	uint32 nNumValues = DATABASE_CATEGORY(ScreenEffect).GetNumValues(hAttribute);
	for (uint32 nCurValue = 0; nCurValue < nNumValues; ++nCurValue)
	{
		if (LTStrICmp(pParam, DATABASE_CATEGORY(ScreenEffect).GetString(hAttribute, nCurValue)) == 0)
			return nCurValue;		
	}
	return k_nInvalidParamIndex;
}
