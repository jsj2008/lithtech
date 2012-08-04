// ----------------------------------------------------------------------- //
//
// MODULE  : WorldProperties.cpp
//
// PURPOSE : WorldProperties object - Implementation
//
// CREATED : 9/25/98
//
// (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "stdafx.h"
#include "WorldProperties.h"
#include "ltengineobjects.h"
#include "gameservershell.h"
#include "ObjectMsgs.h"
#include "MsgIds.h"
#include <stdio.h>
#include "AIVolumeMgr.h"

#define CHROME_TEXTURE	"Tex\\Chrome.dtx"
#define SKYPAN_TEXTURE	"Tex\\SkyPan.dtx"

#define ADD_RAMP(num, time, r, g, b, group) \
	ADD_REALPROP_FLAG(RampTime##num##, time, group)\
	ADD_COLORPROP_FLAG(RampColor##num##, r, g, b, group)\
	ADD_STRINGPROP_FLAG(Target##num##, "", group)\
	ADD_STRINGPROP_FLAG(Message##num##, "", group)

extern CGameServerShell *g_pGameServerShell;

WorldProperties* g_pWorldProperties = LTNULL;

BEGIN_CLASS(WorldProperties)
	ADD_REALPROP(FarZ, 100000.0f)
    ADD_BOOLPROP(AllSkyPortals, LTFALSE)
	ADD_VECTORPROP_VAL(Wind, 0.0f, 0.0f, 0.0f)
	ADD_STRINGPROP(EnvironmentMap, CHROME_TEXTURE)

 	ADD_STRINGPROP_FLAG(GlobalSoundFilter, "UnFiltered", PF_STATICLIST)

	ADD_LONGINTPROP(MPMissionName,0)
	ADD_LONGINTPROP(MPMissionBriefing,0)
	ADD_LONGINTPROP_FLAG(TeamVictory,0,PF_HIDDEN)

	PROP_DEFINEGROUP(FogInfo, PF_GROUP1)
        ADD_BOOLPROP_FLAG(FogEnable, LTTRUE, PF_GROUP1)
		ADD_COLORPROP_FLAG(FogColor, 127.0f, 127.0f, 127.0f, PF_GROUP1)
		ADD_REALPROP_FLAG(FogNearZ, 1.0f, PF_GROUP1)
		ADD_REALPROP_FLAG(FogFarZ, 5000.0f, PF_GROUP1)
        ADD_BOOLPROP_FLAG(SkyFogEnable, LTFALSE, PF_GROUP1)
		ADD_REALPROP_FLAG(SkyFogNearZ, 100.0f, PF_GROUP1)
		ADD_REALPROP_FLAG(SkyFogFarZ, 1000.0f, PF_GROUP1)

	PROP_DEFINEGROUP(VFogInfo, PF_GROUP2)
		ADD_BOOLPROP_FLAG(VFog, 0.0f, PF_GROUP2)
		ADD_REALPROP_FLAG(VFogMinY, 0.0f, PF_GROUP2)
		ADD_REALPROP_FLAG(VFogMaxY, 1300.0f, PF_GROUP2)
		ADD_REALPROP_FLAG(VFogMinYVal, 0.5f, PF_GROUP2)
		ADD_REALPROP_FLAG(VFogMaxYVal, 0.0f, PF_GROUP2)
		ADD_REALPROP_FLAG(VFogDensity, 1800.0f, PF_GROUP2)
		ADD_REALPROP_FLAG(VFogMax, 120.0f, PF_GROUP2)

	PROP_DEFINEGROUP(SkyPanning, PF_GROUP3)
		ADD_STRINGPROP_FLAG(PanSkyTexture, SKYPAN_TEXTURE, PF_GROUP3)
		ADD_BOOLPROP_FLAG(PanSky, 0, PF_GROUP3)
		ADD_REALPROP_FLAG(PanSkyOffsetX, 10.0f, PF_GROUP3)
		ADD_REALPROP_FLAG(PanSkyOffsetZ, 10.0f, PF_GROUP3)
		ADD_REALPROP_FLAG(PanSkyScaleX, 10.0f, PF_GROUP3)
		ADD_REALPROP_FLAG(PanSkyScaleZ, 10.0f, PF_GROUP3)

	PROP_DEFINEGROUP(TimeOfDay, PF_GROUP4)
        ADD_BOOLPROP_FLAG(StartActive, LTFALSE, PF_GROUP4)
		ADD_REALPROP_FLAG(StartTime, 12.0f, PF_GROUP4)
		ADD_REALPROP_FLAG(WorldTimeSpeed, 600.0f, PF_GROUP4)
		ADD_RAMP(0, 0.0f, 40.0f, 40.0f, 255.0f, PF_GROUP4)
		ADD_RAMP(1, 6.0f, 120.0f, 120.0f, 255.0f, PF_GROUP4)
		ADD_RAMP(2, 12.0f, 255.0f, 255.0f, 255.0f, PF_GROUP4)
		ADD_RAMP(3, 21.0f, 255.0f, 60.0f, 60.0f, PF_GROUP4)
		ADD_RAMP(4, 24.0f, 40.0f, 40.0f, 255.0f, PF_GROUP4)
		ADD_RAMP(5, -1.0f, 255.0f, 255.0f, 255.0f, PF_GROUP4)
		ADD_RAMP(6, -1.0f, 255.0f, 255.0f, 255.0f, PF_GROUP4)
		ADD_RAMP(7, -1.0f, 255.0f, 255.0f, 255.0f, PF_GROUP4)
		ADD_RAMP(8, -1.0f, 255.0f, 255.0f, 255.0f, PF_GROUP4)
		ADD_RAMP(9, -1.0f, 255.0f, 255.0f, 255.0f, PF_GROUP4)
		ADD_RAMP(10, -1.0f, 255.0f, 255.0f, 255.0f, PF_GROUP4)
		ADD_RAMP(11, -1.0f, 255.0f, 255.0f, 255.0f, PF_GROUP4)

	//PROP_DEFINEGROUP(Music, PF_GROUP5)
		ADD_STRINGPROP_FLAG(MusicControlFile, "Music\\NOLFOrch\\NOLFOrch.txt", /*PF_GROUP5 |*/ PF_FILENAME)

    ADD_BOOLPROP(LMAnimStatic, LTFALSE)

END_CLASS_DEFAULT_FLAGS_PLUGIN(WorldProperties, BaseClass, NULL, NULL, 0, CWorldPropertiesPlugin)


static char szMusicDirectory[] = "MusicDirectory";
static char szMusicControlFile[] = "MusicControlFile";
static char szWindX[] = "WindX";
static char szWindY[] = "WindY";
static char szWindZ[] = "WindZ";
static char szFarZ[] = "FarZ";
static char szAllSkyPortals[] = "AllSkyPortals";
static char szFogEnable[] = "FogEnable";
static char szFogColor[] = "FogColor";
static char szFogR[] = "FogR";
static char szFogG[] = "FogG";
static char szFogB[] = "FogB";
static char szFogNearZ[] = "FogNearZ";
static char szFogFarZ[] = "FogFarZ";
static char szSkyFogEnable[] = "SkyFogEnable";
static char szSkyFogNearZ[] = "SkyFogNearZ";
static char szSkyFogFarZ[] = "SkyFogFarZ";
static char szPanSky[] = "PanSky";
static char szPanSkyTexture[] = "PanSkyTexture";
static char szPanSkyOffsetX[] = "PanSkyOffsetX";
static char szPanSkyOffsetZ[] = "PanSkyOffsetZ";
static char szPanSkyScaleX[] = "PanSkyScaleX";
static char szPanSkyScaleZ[] = "PanSkyScaleZ";
static char szEnvironmentMap[] = "EnvironmentMap";
static char szSoundFilter[] = "GlobalSoundFilter";
static char szLMAnimStatic[] = "LMAnimStatic";

LTRESULT CWorldPropertiesPlugin::PreHook_EditStringList(
	const char* szRezPath,
	const char* szPropName,
	char** aszStrings,
    uint32* pcStrings,
    const uint32 cMaxStrings,
    const uint32 cMaxStringLength)
{
    if (_stricmp(szSoundFilter, szPropName) == 0)
	{
		m_SoundFilterMgrPlugin.PreHook_EditStringList(szRezPath, szPropName,
			aszStrings, pcStrings, cMaxStrings, cMaxStringLength);

		if (!m_SoundFilterMgrPlugin.PopulateStringList(aszStrings, pcStrings,
			 cMaxStrings, cMaxStringLength)) return LT_UNSUPPORTED;

		return LT_OK;
	}
	return LT_UNSUPPORTED;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	WorldProperties::WorldProperties()
//
//	PURPOSE:	Constructor
//
// --------------------------------------------------------------------------- //

WorldProperties::WorldProperties()
{
	// There should be only one world properties object per level...
	_ASSERT(!g_pWorldProperties);
	if (g_pWorldProperties)
	{
        g_pLTServer->CPrint("WorldProperties ERROR!  More than one WorldProperties object detected!");
	}

	g_pWorldProperties = this;
	m_fWorldTimeSpeed = 600.0f;
	m_nMPMissionName = 0;
	m_nMPMissionBriefing = 0;
	m_nTeamVictory = 0;
	m_nVictoryString = 0;

	m_nSaveVersion		= CVersionMgr::GetSaveVersion();
	m_hstrSave			= LTNULL;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	WorldProperties::~WorldProperties()
//
//	PURPOSE:	Destructor
//
// --------------------------------------------------------------------------- //

WorldProperties::~WorldProperties( )
{
    g_pWorldProperties = LTNULL;

    g_pLTServer->SetGameConVar( szMusicDirectory, "" );
    g_pLTServer->SetGameConVar( szMusicControlFile, "" );
    g_pLTServer->SetGameConVar( szWindX, "" );
    g_pLTServer->SetGameConVar( szWindY, "" );
    g_pLTServer->SetGameConVar( szWindZ, "" );
    //g_pLTServer->SetGameConVar( szFarZ, "" );
    g_pLTServer->SetGameConVar( szAllSkyPortals, "" );
    g_pLTServer->SetGameConVar( szFogEnable, "" );
    g_pLTServer->SetGameConVar( szFogR, "" );
    g_pLTServer->SetGameConVar( szFogG, "" );
    g_pLTServer->SetGameConVar( szFogB, "" );
    g_pLTServer->SetGameConVar( szFogNearZ, "" );
    g_pLTServer->SetGameConVar( szFogFarZ, "" );
    g_pLTServer->SetGameConVar( szSkyFogEnable, "" );
    g_pLTServer->SetGameConVar( szSkyFogNearZ, "" );
    g_pLTServer->SetGameConVar( szSkyFogFarZ, "" );
    g_pLTServer->SetGameConVar( szPanSky, "" );
    g_pLTServer->SetGameConVar( szPanSkyTexture, "" );
    g_pLTServer->SetGameConVar( szPanSkyOffsetX, "" );
    g_pLTServer->SetGameConVar( szPanSkyOffsetZ, "" );
    g_pLTServer->SetGameConVar( szPanSkyScaleX, "" );
    g_pLTServer->SetGameConVar( szPanSkyScaleZ, "" );
    g_pLTServer->SetGameConVar( szEnvironmentMap, "" );
	g_pLTServer->SetGameConVar( szSoundFilter, "" );
    g_pLTServer->SetGameConVar( szLMAnimStatic, "" );

	FREE_HSTRING(m_hstrSave);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	WorldProperties::EngineMessageFn()
//
//	PURPOSE:	Handle engine messages
//
// --------------------------------------------------------------------------- //

uint32 WorldProperties::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			ReadProps();
		}
		break;

		case MID_INITIALUPDATE:
		{
            g_pLTServer->SetObjectState(m_hObject, OBJSTATE_INACTIVE);
		}
		break;

		case MID_SAVEOBJECT:
		{
            Save((HMESSAGEWRITE)pData, (uint32)fData);
		}
		break;

		case MID_LOADOBJECT:
		{
            Load((HMESSAGEREAD)pData, (uint32)fData);
		}
		break;

		case MID_LINKBROKEN :
		{
			if ( g_pAIVolumeMgr )
			{
				g_pAIVolumeMgr->HandleBrokenLink((HOBJECT)pData);
			}
		}
		break;

		default : break;
	}

	return BaseClass::EngineMessageFn(messageID, pData, fData);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	WorldProperties::ReadProps()
//
//	PURPOSE:	Read in properties
//
// --------------------------------------------------------------------------- //

void WorldProperties::ReadProps()
{
	char buf[512];
	TimeRamp *pRamp;
	GenericProp genProp;
    LTBOOL bVal;
    LTFLOAT fVal;

	buf[0] = '\0';
    if (g_pLTServer->GetPropString(szMusicControlFile, buf, 500) == LT_OK)
	{
		if (buf[0])
		{
			// Get the directory name and control file name from the buf...

			char* pPos = strstr(buf, "\\");
            char* pLastPos = LTNULL;

			while (pPos)
			{
				pPos++;
				pLastPos = pPos;

				pPos = strstr(pLastPos, "\\");
			}

			if (pLastPos)
			{
                g_pLTServer->SetGameConVar(szMusicControlFile, pLastPos);

				int nLen = strlen(buf);
				int nCFileLen = strlen(pLastPos);

				buf[nLen - (nCFileLen + 1)] = '\0';
                g_pLTServer->SetGameConVar(szMusicDirectory, buf);
			}
		}
	}

	//buf[0] = '\0';
    //if (g_pLTServer->GetPropString(szMusicDirectory, buf, 500) == LT_OK)
	//{
    //  if (buf[0]) g_pLTServer->SetGameConVar(szMusicDirectory, buf);
	//}

    if (g_pLTServer->GetPropGeneric("Wind", &genProp) == LT_OK)
	{
		sprintf(buf, "%f", genProp.m_Vec.x);
        g_pLTServer->SetGameConVar(szWindX, buf);

		sprintf(buf, "%f", genProp.m_Vec.y);
        g_pLTServer->SetGameConVar(szWindY, buf);

		sprintf(buf, "%f", genProp.m_Vec.z);
        g_pLTServer->SetGameConVar(szWindZ, buf);
	}

    if (g_pLTServer->GetPropReal(szFarZ, &fVal) == LT_OK)
	{
		sprintf(buf, "%f", fVal);
        g_pLTServer->SetGameConVar(szFarZ, buf);
	}

    if (g_pLTServer->GetPropBool(szAllSkyPortals, &bVal) == LT_OK)
	{
		sprintf(buf, "%d", bVal);
        g_pLTServer->SetGameConVar(szAllSkyPortals, buf);
	}

    if (g_pLTServer->GetPropBool(szFogEnable, &bVal) == LT_OK)
	{
		sprintf(buf, "%d", bVal);
        g_pLTServer->SetGameConVar(szFogEnable, buf);
	}

    if (g_pLTServer->GetPropReal(szFogNearZ, &fVal) == LT_OK)
	{
		sprintf(buf, "%f", fVal);
        g_pLTServer->SetGameConVar(szFogNearZ, buf);
	}

    if (g_pLTServer->GetPropReal(szFogFarZ, &fVal) == LT_OK)
	{
		sprintf(buf, "%f", fVal);
        g_pLTServer->SetGameConVar(szFogFarZ, buf);
	}

    if (g_pLTServer->GetPropGeneric(szFogColor, &genProp) == LT_OK)
	{
		sprintf(buf, "%f", genProp.m_Vec.x);
        g_pLTServer->SetGameConVar(szFogR, buf);

		sprintf(buf, "%f", genProp.m_Vec.y);
        g_pLTServer->SetGameConVar(szFogG, buf);

		sprintf(buf, "%f", genProp.m_Vec.z);
        g_pLTServer->SetGameConVar(szFogB, buf);
	}

    if (g_pLTServer->GetPropBool(szSkyFogEnable, &bVal) == LT_OK)
	{
		sprintf(buf, "%d", bVal);
        g_pLTServer->SetGameConVar(szSkyFogEnable, buf);
	}

    if (g_pLTServer->GetPropReal(szSkyFogNearZ, &fVal) == LT_OK)
	{
		sprintf(buf, "%f", fVal);
        g_pLTServer->SetGameConVar(szSkyFogNearZ, buf);
	}

    if (g_pLTServer->GetPropReal(szSkyFogFarZ, &fVal) == LT_OK)
	{
		sprintf(buf, "%f", fVal);
        g_pLTServer->SetGameConVar(szSkyFogFarZ, buf);
	}

    if (g_pLTServer->GetPropBool(szPanSky, &bVal) == LT_OK)
	{
		sprintf(buf, "%d", bVal);
        g_pLTServer->SetGameConVar(szPanSky, buf);
	}

	buf[0] = '\0';
    if (g_pLTServer->GetPropString(szEnvironmentMap, buf, 500) == LT_OK)
	{
        if (buf[0]) g_pLTServer->SetGameConVar(szEnvironmentMap, buf);
	}

	buf[0] = '\0';
    if (g_pLTServer->GetPropString(szSoundFilter, buf, 500) == LT_OK)
	{
        if (buf[0])
		{
			g_pLTServer->SetGameConVar(szSoundFilter, buf);
		}
	}
	buf[0] = '\0';
    if (g_pLTServer->GetPropString(szPanSkyTexture, buf, 500) == LT_OK)
	{
        if (buf[0]) g_pLTServer->SetGameConVar(szPanSkyTexture, buf);
	}

    if (g_pLTServer->GetPropReal(szPanSkyOffsetX, &fVal) == LT_OK)
	{
		sprintf(buf, "%f", fVal);
        g_pLTServer->SetGameConVar(szPanSkyOffsetX, buf);
	}

    if (g_pLTServer->GetPropReal(szPanSkyOffsetZ, &fVal) == LT_OK)
	{
		sprintf(buf, "%f", fVal);
        g_pLTServer->SetGameConVar(szPanSkyOffsetZ, buf);
	}

    if (g_pLTServer->GetPropReal(szPanSkyScaleX, &fVal) == LT_OK)
	{
		sprintf(buf, "%f", fVal);
        g_pLTServer->SetGameConVar(szPanSkyScaleX, buf);
	}

    if (g_pLTServer->GetPropReal(szPanSkyScaleZ, &fVal) == LT_OK)
	{
		sprintf(buf, "%f", fVal);
        g_pLTServer->SetGameConVar(szPanSkyScaleZ, buf);
	}


	// Read in the time ramps.
	if (g_pGameServerShell)
	{
        for(uint32 i=0; i < MAX_TIME_RAMPS; i++)
		{
			pRamp = g_pGameServerShell->GetTimeRamp(i);

			sprintf(buf, "RampTime%d", i);
            if (g_pLTServer->GetPropGeneric(buf, &genProp) == LT_OK)
			{
				pRamp->m_Time = genProp.m_Float;
			}

			sprintf(buf, "RampColor%d", i);
            if (g_pLTServer->GetPropGeneric(buf, &genProp) == LT_OK)
			{
				pRamp->m_Color = genProp.m_Color / 255.0f;
			}

			sprintf(buf, "Target%d", i);
            if (g_pLTServer->GetPropGeneric(buf, &genProp) == LT_OK)
			{
                pRamp->m_hTarget = g_pLTServer->CreateString(genProp.m_String);
			}

			sprintf(buf, "Message%d", i);
            if (g_pLTServer->GetPropGeneric(buf, &genProp) == LT_OK)
			{
                pRamp->m_hMessage = g_pLTServer->CreateString(genProp.m_String);
			}
		}
	}

	// Read in WorldTimeSpeed.
    if (g_pLTServer->GetPropGeneric("WorldTimeSpeed", &genProp) == LT_OK)
	{
		m_fWorldTimeSpeed = genProp.m_Float;
        g_pLTServer->SetGameConVar("WorldTimeSpeed", genProp.m_String);
	}

	// Read in Mission Name.
    if (g_pLTServer->GetPropGeneric("MPMissionName", &genProp) == LT_OK)
	{
		m_nMPMissionName = (int)genProp.m_Long;
	}
	// Read in Mission Briefing.
    if (g_pLTServer->GetPropGeneric("MPMissionBriefing", &genProp) == LT_OK)
	{
		m_nMPMissionBriefing = (int)genProp.m_Long;
	}

    if (g_pLTServer->GetPropGeneric("StartActive", &genProp) == LT_OK)
	{
		if (!genProp.m_Bool)
		{
            g_pLTServer->SetGameConVar("WorldTimeSpeed", "-1.0");
		}
	}

    if (g_pLTServer->GetPropGeneric("StartTime", &genProp) == LT_OK)
	{
        g_pLTServer->SetGameConVar("WorldTime", genProp.m_String);
	}

	// VFog stuff.
    if (g_pLTServer->GetPropGeneric("VFog", &genProp) == LT_OK)
	{
        g_pLTServer->SetGameConVar("SC_VFog", genProp.m_String);
	}

    if (g_pLTServer->GetPropGeneric("VFogMinY", &genProp) == LT_OK)
	{
        g_pLTServer->SetGameConVar("SC_VFogMinY", genProp.m_String);
	}

    if (g_pLTServer->GetPropGeneric("VFogMaxYVal", &genProp) == LT_OK)
	{
        g_pLTServer->SetGameConVar("SC_VFogMaxYVal", genProp.m_String);
	}

    if (g_pLTServer->GetPropGeneric("VFogMinYVal", &genProp) == LT_OK)
	{
        g_pLTServer->SetGameConVar("SC_VFogMinYVal", genProp.m_String);
	}

    if (g_pLTServer->GetPropGeneric("VFogMaxY", &genProp) == LT_OK)
	{
        g_pLTServer->SetGameConVar("SC_VFogMaxY", genProp.m_String);
	}

    if (g_pLTServer->GetPropGeneric("VFogDensity", &genProp) == LT_OK)
	{
        g_pLTServer->SetGameConVar("SC_VFogDensity", genProp.m_String);
	}

    if (g_pLTServer->GetPropGeneric("VFogMax", &genProp) == LT_OK)
	{
        g_pLTServer->SetGameConVar("SC_VFogMax", genProp.m_String);
	}

	// Bias toward semi-static lightmap animations
    if (g_pLTServer->GetPropBool(szLMAnimStatic, &bVal) == LT_OK)
	{
		sprintf(buf, "%d", bVal);
        g_pLTServer->SetGameConVar(szLMAnimStatic, buf);
	}

}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	WorldProperties::ObjectMessageFn()
//
//	PURPOSE:	Handler for server object messages.
//
// --------------------------------------------------------------------------- //

uint32 WorldProperties::ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead)
{
    ILTServer* pServerDE = GetServerDE();
	switch (messageID)
	{
		case MID_TRIGGER:
		{
            const char* szMsg = (const char*)g_pLTServer->ReadFromMessageDWord(hRead);
			HandleMsg(hSender, szMsg);
		}
		break;

		default : break;
	}

	return BaseClass::ObjectMessageFn(hSender, messageID, hRead);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	WorldProperties::HandleMsg()
//
//	PURPOSE:	Handle trigger messages
//
// --------------------------------------------------------------------------- //

void WorldProperties::HandleMsg(HOBJECT hSender, const char* szMsg)
{
    ILTCommon* pCommon = g_pLTServer->Common();
	if (!pCommon) return;

	// ConParse does not destroy szMsg, so this is safe
	ConParse parse;
	parse.Init((char*)szMsg);

    LTBOOL bChangedFog = LTFALSE;

	while (pCommon->Parse(&parse) == LT_OK)
	{
		if (parse.m_nArgs > 0 && parse.m_Args[0])
		{
			if (_stricmp(parse.m_Args[0], "ON") == 0)
			{
				char buf[20];
				sprintf(buf,"%.2f", m_fWorldTimeSpeed);

                g_pLTServer->SetGameConVar("WorldTimeSpeed", buf);
			}
			else if (_stricmp(parse.m_Args[0], "OFF") == 0)
			{
                g_pLTServer->SetGameConVar("WorldTimeSpeed", "-1.0");
			}
			else if (_stricmp(parse.m_Args[0], "WorldTimeSpeed") == 0)
			{
				if (parse.m_nArgs > 1 && parse.m_Args[1])
				{
                    m_fWorldTimeSpeed = (LTFLOAT) atof(parse.m_Args[1]);
                    g_pLTServer->SetGameConVar("WorldTimeSpeed", parse.m_Args[1]);
				}
			}
			else if (_stricmp(parse.m_Args[0], "WorldTime") == 0)
			{
				if (parse.m_nArgs > 1 && parse.m_Args[1])
				{
                    g_pLTServer->SetGameConVar("WorldTime", parse.m_Args[1]);
				}
			}
			else if (_stricmp(parse.m_Args[0], szFogColor) == 0)
			{
				if (parse.m_nArgs > 3 && parse.m_Args[1] && parse.m_Args[2] && parse.m_Args[3])
				{
                    g_pLTServer->SetGameConVar(szFogR, parse.m_Args[1]);
                    g_pLTServer->SetGameConVar(szFogG, parse.m_Args[2]);
                    g_pLTServer->SetGameConVar(szFogB, parse.m_Args[3]);
                    bChangedFog = LTTRUE;
				}
			}
			else if (_stricmp(parse.m_Args[0], szFogEnable) == 0)
			{
				if (parse.m_nArgs > 1 && parse.m_Args[1])
				{
                    g_pLTServer->SetGameConVar(szFogEnable, parse.m_Args[1]);
                    bChangedFog = LTTRUE;
				}
			}
			else if (_stricmp(parse.m_Args[0], szFogNearZ) == 0)
			{
				if (parse.m_nArgs > 1 && parse.m_Args[1])
				{
                    g_pLTServer->SetGameConVar(szFogNearZ, parse.m_Args[1]);
                    bChangedFog = LTTRUE;
				}
			}
			else if (_stricmp(parse.m_Args[0], szFogFarZ) == 0)
			{
				if (parse.m_nArgs > 1 && parse.m_Args[1])
				{
                    g_pLTServer->SetGameConVar(szFogFarZ, parse.m_Args[1]);
                    bChangedFog = LTTRUE;
				}
			}
			else if (_stricmp(parse.m_Args[0], szSkyFogEnable) == 0)
			{
				if (parse.m_nArgs > 1 && parse.m_Args[1])
				{
                    g_pLTServer->SetGameConVar(szSkyFogEnable, parse.m_Args[1]);
                    bChangedFog = LTTRUE;
				}
			}
			else if (_stricmp(parse.m_Args[0], szSkyFogFarZ) == 0)
			{
				if (parse.m_nArgs > 1 && parse.m_Args[1])
				{
                    g_pLTServer->SetGameConVar(szSkyFogFarZ, parse.m_Args[1]);
                    bChangedFog = LTTRUE;
				}
			}
			else if (_stricmp(parse.m_Args[0], szSkyFogNearZ) == 0)
			{
				if (parse.m_nArgs > 1 && parse.m_Args[1])
				{
                    g_pLTServer->SetGameConVar(szSkyFogNearZ, parse.m_Args[1]);
                    bChangedFog = LTTRUE;
				}
			}
			else if (_stricmp(parse.m_Args[0], "objective") == 0)
			{
				HandleObjectiveMsg(&parse);
			}
			else if (_stricmp(parse.m_Args[0], "transmission") == 0)
			{
				HandleTransmissionMsg(&parse);
			}
			else if (_stricmp(parse.m_Args[0], "victory") == 0)
			{
				HandleVictoryMsg(&parse);
			}
		}
	}

	if (bChangedFog)
	{
		SendClientsFogValues();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WorldProperties::SendClientsFogValues
//
//	PURPOSE:	Send the fog values to the client(s)
//
// ----------------------------------------------------------------------- //

void WorldProperties::SendClientsFogValues()
{
    HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(LTNULL, MID_CHANGE_FOG);
    g_pLTServer->EndMessage(hMessage);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WorldProperties::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void WorldProperties::Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags)
{
	if (!hWrite) return;

	// First save global game server shell stuff...(this only works because the
	// GameServerShell saves the WorldProperties object first)

	g_pGameServerShell->Save(hWrite, dwSaveFlags);


    g_pLTServer->WriteToMessageFloat(hWrite, m_fWorldTimeSpeed);

	SAVE_DWORD(m_nSaveVersion);
	SAVE_HSTRING(m_hstrSave);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WorldProperties::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void WorldProperties::Load(HMESSAGEREAD hRead, uint32 dwLoadFlags)
{
	if (!hRead) return;

	// First load global game server shell stuff...(this only works because the
	// GameServerShell saves the WorldProperties object first)

	g_pGameServerShell->Load(hRead, dwLoadFlags);


    m_fWorldTimeSpeed = g_pLTServer->ReadFromMessageFloat(hRead);

	LOAD_DWORD(m_nSaveVersion);
	LOAD_HSTRING(m_hstrSave);
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WorldProperties::HandleObjectiveMsg
//
//	PURPOSE:	process arguments of an objective message
//
// ----------------------------------------------------------------------- //

void WorldProperties::HandleObjectiveMsg(ConParse *pParse)
{
	GameType eGameType = g_pGameServerShell->GetGameType();
	if (eGameType != COOPERATIVE_ASSAULT) return;

	if (pParse->m_nArgs > 1 && pParse->m_Args[1])
	{

        uint8 nRequest = OBJECTIVE_ADD_ID;
		if (_stricmp(pParse->m_Args[1], "Add") == 0)
		{
			nRequest = OBJECTIVE_ADD_ID;
		}
		else if (_stricmp(pParse->m_Args[1], "Remove") == 0)
		{
			nRequest = OBJECTIVE_REMOVE_ID;
		}
		else if (_stricmp(pParse->m_Args[1], "RemoveAll") == 0)
		{
			nRequest = OBJECTIVE_CLEAR_ID;
		}
		else if (_stricmp(pParse->m_Args[1], "Completed") == 0)
		{
			nRequest = OBJECTIVE_COMPLETE_ID;
		}
		else
			return;

        uint32 dwId = 0;
        uint8 nTeam = 0;
		if (nRequest != OBJECTIVE_CLEAR_ID && pParse->m_nArgs > 2 && pParse->m_Args[2])
		{
            dwId = (uint32) atol(pParse->m_Args[2]);
			// Need the team info...

			if (pParse->m_nArgs > 3 && pParse->m_Args[3])
			{
                nTeam = (uint8) atol(pParse->m_Args[3]);
			}
			else
			{
				// Everybody gets it...
				nTeam = 0;
			}


			ObjectivesList *pObjList = g_pGameServerShell->GetObjectives(nTeam);
			ObjectivesList *pCompObjList = g_pGameServerShell->GetCompletedObjectives(nTeam);
			switch (nRequest)
			{
				case OBJECTIVE_ADD_ID:
				{
					pObjList->Add(dwId);
				}
				break;

				case OBJECTIVE_REMOVE_ID:
				{
					pObjList->Remove(dwId);
					pCompObjList->Remove(dwId);
				}
				break;

				case OBJECTIVE_COMPLETE_ID:
				{
					pObjList->Add(dwId);
					pCompObjList->Add(dwId);

				}
				break;

				case OBJECTIVE_CLEAR_ID:
				{
					pObjList->Clear();
					pCompObjList->Clear();

					if (nTeam == 0)
					{
                        for (uint8 nTemp = 1; nTemp <= NUM_TEAMS; nTemp++)
						{
							pObjList = g_pGameServerShell->GetObjectives(nTemp);
							pCompObjList = g_pGameServerShell->GetCompletedObjectives(nTemp);
							pObjList->Clear();
							pCompObjList->Clear();
						}
					}
				}
				break;
			}
		}

        HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(LTNULL, MID_PLAYER_INFOCHANGE);
        g_pLTServer->WriteToMessageByte(hMessage, IC_OBJECTIVE_ID);
        g_pLTServer->WriteToMessageByte(hMessage, nRequest);
        g_pLTServer->WriteToMessageByte(hMessage, nTeam);
        g_pLTServer->WriteToMessageFloat(hMessage, (LTFLOAT)dwId);
        g_pLTServer->EndMessage2(hMessage, MESSAGE_GUARANTEED | MESSAGE_NAGGLE);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WorldProperties::HandleTransmissionMsg
//
//	PURPOSE:	process arguments of a transmission message
//
// ----------------------------------------------------------------------- //

void WorldProperties::HandleTransmissionMsg(ConParse *pParse)
{
	GameType eGameType = g_pGameServerShell->GetGameType();
	if (eGameType != COOPERATIVE_ASSAULT) return;

	if (pParse->m_nArgs > 1 && pParse->m_Args[1])
	{

        uint32 dwId = 0;
        uint8 nTeam = 0;
        uint32 nSound = 0;
        dwId = (uint32) atol(pParse->m_Args[1]);
		// Need the team info...

		if (pParse->m_nArgs > 2 && pParse->m_Args[2])
		{
            nTeam = (uint8) atol(pParse->m_Args[2]);
			if (pParse->m_nArgs > 3 && pParse->m_Args[3])
			{
				nSound = (uint32) atol(pParse->m_Args[3]);
			}
		}
		else
		{
			// Everybody gets it...
			nTeam = 0;
		}

        HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(LTNULL, MID_PLAYER_TRANSMISSION);
        g_pLTServer->WriteToMessageDWord(hMessage, dwId);
        g_pLTServer->WriteToMessageByte(hMessage, nTeam);
	    g_pLTServer->WriteToMessageDWord(hMessage, nSound);
        g_pLTServer->EndMessage2(hMessage, MESSAGE_GUARANTEED | MESSAGE_NAGGLE);


	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WorldProperties::HandleVictoryMsg
//
//	PURPOSE:	process arguments of a victory condition message
//
// ----------------------------------------------------------------------- //

void WorldProperties::HandleVictoryMsg(ConParse *pParse)
{
	GameType eGameType = g_pGameServerShell->GetGameType();
	if (eGameType != COOPERATIVE_ASSAULT) return;

	if (pParse->m_nArgs > 1 && pParse->m_Args[1])
	{
	    m_nTeamVictory = atoi(pParse->m_Args[1]);
		if (pParse->m_nArgs > 2 && pParse->m_Args[2])
		{
		    m_nVictoryString = atoi(pParse->m_Args[2]);
		}

		g_pLTServer->CPrint("team victory set to %d,%d",m_nTeamVictory,m_nVictoryString);
	}
}