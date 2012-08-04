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
#include "ParsedMsg.h"
#include "MsgIds.h"
#include <stdio.h>
#include "AIVolumeMgr.h"
#include "VersionMgr.h"
#include "AIStimulusMgr.h"
#include "ServerMissionMgr.h"

LINKFROM_MODULE( WorldProperties );


#define CHROME_TEXTURE	"Tex\\Chrome.dtx"
#define SKYPAN_TEXTURE	"Tex\\SkyPan.dtx"

extern CGameServerShell *g_pGameServerShell;
extern CAIStimulusMgr *g_pAIStimulusMgr;

WorldProperties* g_pWorldProperties = LTNULL;

BEGIN_CLASS(WorldProperties)
	ADD_REALPROP(FarZ, 100000.0f)
	ADD_BOOLPROP(ClampFarZ, LTTRUE)
    ADD_BOOLPROP(AllSkyPortals, LTFALSE)
	ADD_VECTORPROP_VAL(Wind, 0.0f, 0.0f, 0.0f)
	ADD_STRINGPROP(EnvironmentMap, CHROME_TEXTURE)
	ADD_STRINGPROP_FLAG( LevelEndCmd, "", PF_NOTIFYCHANGE )

 	ADD_STRINGPROP_FLAG(GlobalSoundFilter, "UnFiltered", PF_STATICLIST)

	PROP_DEFINEGROUP(FogInfo, PF_GROUP(1))
        ADD_BOOLPROP_FLAG(FogEnable, LTTRUE, PF_GROUP(1))
		ADD_COLORPROP_FLAG(FogColor, 127.0f, 127.0f, 127.0f, PF_GROUP(1))
		ADD_REALPROP_FLAG(FogNearZ, 1.0f, PF_GROUP(1))
		ADD_REALPROP_FLAG(FogFarZ, 5000.0f, PF_GROUP(1))
        ADD_BOOLPROP_FLAG(SkyFogEnable, LTFALSE, PF_GROUP(1))
		ADD_REALPROP_FLAG(SkyFogNearZ, 100.0f, PF_GROUP(1))
		ADD_REALPROP_FLAG(SkyFogFarZ, 1000.0f, PF_GROUP(1))

	PROP_DEFINEGROUP(VFogInfo, PF_GROUP(2))
		ADD_BOOLPROP_FLAG(VFog, 0.0f, PF_GROUP(2))
		ADD_REALPROP_FLAG(VFogMinY, 0.0f, PF_GROUP(2))
		ADD_REALPROP_FLAG(VFogMaxY, 1300.0f, PF_GROUP(2))
		ADD_REALPROP_FLAG(VFogMinYVal, 0.5f, PF_GROUP(2))
		ADD_REALPROP_FLAG(VFogMaxYVal, 0.0f, PF_GROUP(2))
		ADD_REALPROP_FLAG(VFogDensity, 1800.0f, PF_GROUP(2))
		ADD_REALPROP_FLAG(VFogMax, 120.0f, PF_GROUP(2))

	PROP_DEFINEGROUP(SkyPanning, PF_GROUP(3))
		ADD_STRINGPROP_FLAG(PanSkyTexture, SKYPAN_TEXTURE, PF_GROUP(3))
		ADD_BOOLPROP_FLAG(PanSky, 0, PF_GROUP(3))
		ADD_REALPROP_FLAG(PanSkyOffsetX, 10.0f, PF_GROUP(3))
		ADD_REALPROP_FLAG(PanSkyOffsetZ, 10.0f, PF_GROUP(3))
		ADD_REALPROP_FLAG(PanSkyScaleX, 10.0f, PF_GROUP(3))
		ADD_REALPROP_FLAG(PanSkyScaleZ, 10.0f, PF_GROUP(3))

	PROP_DEFINEGROUP(ProjShadows, PF_GROUP(4))
		ADD_REALPROP_FLAG(ShadowMinColorComponent, 40.0f, PF_GROUP(4))
		ADD_REALPROP_FLAG(ShadowMaxProjDist, 200.0f, PF_GROUP(4))
		ADD_REALPROP_FLAG(ShadowAlpha, 1.0f, PF_GROUP(4))

	//PROP_DEFINEGROUP(Music, PF_GROUP(5))
		ADD_STRINGPROP_FLAG(MusicControlFile, "Music\\NOLFOrch\\NOLFOrch.txt", /*PF_GROUP(5) |*/ PF_FILENAME)
	ADD_STRINGPROP_FLAG(MinMusicMood, "Routine", PF_STATICLIST)

    ADD_BOOLPROP(LMAnimStatic, LTFALSE)
	ADD_LONGINTPROP( AIMaxNumber, 30 )
	ADD_LONGINTPROP( VehicleMaxNumber, 12 )

END_CLASS_DEFAULT_FLAGS_PLUGIN(WorldProperties, BaseClass, NULL, NULL, 0, CWorldPropertiesPlugin)


//
// Register the class with the command mgr plugin and add any messages to the class
//

CMDMGR_BEGIN_REGISTER_CLASS( WorldProperties )
	
	CMDMGR_ADD_MSG( ON, 1, NULL, "ON" )
	CMDMGR_ADD_MSG( OFF, 1, NULL, "OFF" )
	CMDMGR_ADD_MSG( WORLDTIMESPEED, 2, NULL, "WORLDTIMESPEED <speed>" )
	CMDMGR_ADD_MSG( WORLDTIME, 2, NULL, "WORLDTIME <time>" )
	CMDMGR_ADD_MSG( FOGCOLOR, 4, NULL, "FOGCOLOR <r> <g> <b>" )
	CMDMGR_ADD_MSG( FOGENABLE, 2, NULL, "FOGENABLE <1-or-0>" )
	CMDMGR_ADD_MSG( FOGNEARZ, 2, NULL, "FOGNEARZ <near z>" )
	CMDMGR_ADD_MSG( FOGFARZ, 2, NULL, "FOGFARZ <far z>" )
	CMDMGR_ADD_MSG( SKYFOGENABLE, 2, NULL, "SKYFOGENABLE <1-or-0>" )
	CMDMGR_ADD_MSG( SKYFOGFARZ, 2, NULL, "SKYFOGFARZ <far z>" )
	CMDMGR_ADD_MSG( SKYFOGNEARZ, 2, NULL, "SKYFOGNEARZ <near z>" )
	CMDMGR_ADD_MSG( TRANSMISSION, -1, NULL, "TRANSMISSION <id> <optional: sound id>" )
	CMDMGR_ADD_MSG( DRAWALLMODELSHADOWS, 2, NULL, "DRAWALLMODELSHADOWS <1-or-0>" )
	CMDMGR_ADD_MSG( SHADOWMINCOLORCOMPONENT, 2, NULL, "SHADOWMINCOLORCOMPONENT <0-to-255>" )
	CMDMGR_ADD_MSG( SHADOWMAXPROJDIST, 2, NULL, "SHADOWMAXPROJDIST <projection distance>" )
	CMDMGR_ADD_MSG( SHADOWALPHA, 2, NULL, "SHADOWALPHA <0-to-1>" )
	CMDMGR_ADD_MSG( NEXTROUND, 1, NULL, "NEXTROUND" )
	CMDMGR_ADD_MSG( ROUNDWON, 2, NULL, "ROUNDWON <0 or 1>" )

CMDMGR_END_REGISTER_CLASS( WorldProperties, BaseClass )

static char szMusicDirectory[] = "MusicDirectory";
static char szMusicControlFile[] = "MusicControlFile";
static char szMinMusicMood[] = "MinMusicMood";
static char szWindX[] = "WindX";
static char szWindY[] = "WindY";
static char szWindZ[] = "WindZ";
static char szFarZ[] = "FarZ";
static char szClampFarZ[] = "ClampFarZ";
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
static char szDrawAllModelShadows[] = "DrawAllModelShadows";
static char szShadowMaxProjDist[] = "ModelShadow_Proj_MaxProjDist";
static char szShadowMinColorComponent[] = "ModelShadow_Proj_MinColorComponent";
static char szShadowAlpha[] = "ModelShadow_Proj_Alpha";
static char szAIMaxNumber[] = "AIMaxNumber";
static char szVehicleMaxNumber[] = "VehicleMaxNumber";

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CWorldPropertiesPlugin::PreHook_PropChanged
//
//  PURPOSE:	Make sure the Command is valid
//
// ----------------------------------------------------------------------- //

LTRESULT CWorldPropertiesPlugin::PreHook_PropChanged( const char *szObjName,
													 const char *szPropName,
												     const int nPropType,
												     const GenericProp &gpPropValue,
												     ILTPreInterface *pInterface,
													 const	char *szModifiers )
{
	if( LT_OK == m_CmdMgrPlugin.PreHook_PropChanged( szObjName,
														 szPropName,
														 nPropType, 
														 gpPropValue,
														 pInterface,
														 szModifiers ))
	{
		return LT_OK;
	}

	return LT_UNSUPPORTED;
}

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

    else if (_stricmp(szMinMusicMood, szPropName) == 0)
	{
		strcpy( aszStrings[(*pcStrings)++], "Routine" );
		strcpy( aszStrings[(*pcStrings)++], "Investigate" );
		strcpy( aszStrings[(*pcStrings)++], "Aggressive" );

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
    g_pLTServer->SetGameConVar( szMinMusicMood, "" );
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
	g_pLTServer->SetGameConVar( szDrawAllModelShadows, "" );
	g_pLTServer->SetGameConVar( szShadowMinColorComponent, "" );
	g_pLTServer->SetGameConVar( szShadowMaxProjDist, "" );
	g_pLTServer->SetGameConVar( szShadowAlpha, "" );
	g_pLTServer->SetGameConVar( szShadowMinColorComponent, "" );
	g_pLTServer->SetGameConVar( szShadowMaxProjDist, "" );
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
			if( fData == PRECREATE_WORLDFILE )
			{
				ReadProps();
			}
		}
		break;

		case MID_INITIALUPDATE:
		{
			LTRotation rRot;
			g_pLTServer->GetObjectRotation( m_hObject, &rRot );

			// Get the forward vector...
			LTVector vForward = rRot.Forward();

			// ...convert the xz rotation to an angle...
			float fAngle = static_cast< float >( atan2( vForward.x, vForward.z ) );
			char tempStr[128];
			sprintf(tempStr, "%5f", fAngle);

			g_pLTServer->SetGameConVar("WorldNorth",tempStr);

            g_pLTServer->SetObjectState(m_hObject, OBJSTATE_INACTIVE);
		}
		break;

		case MID_SAVEOBJECT:
		{
            Save((ILTMessage_Write*)pData, (uint32)fData);
		}
		break;

		case MID_LOADOBJECT:
		{
            Load((ILTMessage_Read*)pData, (uint32)fData);
		}
		break;
		
		case MID_ALLOBJECTSCREATED:
		{
			// Since we must have a WorldProperties object in our levels this is a perfect 
			// place to do things that require every object to be created.

			g_pCommandButeMgr->AllObjectsCreated();
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
    bool bVal;
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

	float fFarZ = 100000.0f;
    if (g_pLTServer->GetPropReal(szFarZ, &fFarZ) == LT_OK)
	{
		sprintf(buf, "%f", fFarZ);
        g_pLTServer->SetGameConVar(szFarZ, buf);
	}

    if (g_pLTServer->GetPropBool(szAllSkyPortals, &bVal) == LT_OK)
	{
		sprintf(buf, "%d", bVal ? 1 : 0);
        g_pLTServer->SetGameConVar(szAllSkyPortals, buf);
	}

	bool bFogEnable = false;
    if (g_pLTServer->GetPropBool(szFogEnable, &bFogEnable) == LT_OK)
	{
		sprintf(buf, "%d", bFogEnable ? 1 : 0);
        g_pLTServer->SetGameConVar(szFogEnable, buf);
	}

    if (g_pLTServer->GetPropReal(szFogNearZ, &fVal) == LT_OK)
	{
		sprintf(buf, "%f", fVal);
        g_pLTServer->SetGameConVar(szFogNearZ, buf);
	}

	float fFogFarZ;
    if (g_pLTServer->GetPropReal(szFogFarZ, &fFogFarZ) == LT_OK)
	{
		sprintf(buf, "%f", fFogFarZ);
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
		sprintf(buf, "%d", bVal ? 1 : 0);
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
		sprintf(buf, "%d", bVal ? 1 : 0);
        g_pLTServer->SetGameConVar(szPanSky, buf);
	}

	//determine if we should clamp the far z value
	bool bClampFarZ = true;
	g_pLTServer->GetPropBool(szClampFarZ, &bClampFarZ); 

	if(bClampFarZ)
	{
		// Override FarZ if FogFarZ is closer
		if (bFogEnable && ((fFogFarZ * 2.0f) < fFarZ))
		{
			sprintf(buf, "%f", fFogFarZ * 2.0f);
			g_pLTServer->SetGameConVar(szFarZ, buf);
		}

		// Don't let FarZ get bigger than the dims of the level
		LTVector vWorldMin, vWorldMax;
		g_pLTServer->GetWorldBox(vWorldMin, vWorldMax);
		float fMaxFarZ = (vWorldMin - vWorldMax).Mag();
		if (fFarZ > fMaxFarZ)
		{
			g_pLTServer->CPrint("!!! FarZ for level is too large!  Please adjust the FarZ value in the WorldProperties object.");
			sprintf(buf, "%f", fMaxFarZ);
			g_pLTServer->SetGameConVar(szFarZ, buf);
		}
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
    if (g_pLTServer->GetPropString(szMinMusicMood, buf, 500) == LT_OK)
	{
        if (buf[0])
		{
			g_pLTServer->SetGameConVar(szMinMusicMood, buf);
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

	if (g_pLTServer->GetPropReal(szShadowMinColorComponent, &fVal) == LT_OK)
	{
		sprintf(buf, "%f", fVal);
		g_pLTServer->SetGameConVar(szShadowMinColorComponent, buf);
	}

	if (g_pLTServer->GetPropReal(szShadowMaxProjDist, &fVal) == LT_OK)
	{
		sprintf(buf, "%f", fVal);
		g_pLTServer->SetGameConVar(szShadowMaxProjDist, buf);
	}

	if (g_pLTServer->GetPropReal(szShadowAlpha, &fVal) == LT_OK)
	{
		sprintf(buf, "%f", fVal);
		g_pLTServer->SetGameConVar(szShadowAlpha, buf);
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

	//read in shadow properties. Note that this is not handled quite like the other
	//properties since the name of the property is different than the name of the
	//console variable. This had to be done because the property names can only be
	//32 characters long, and some of the shadow names were longer
	if (g_pLTServer->GetPropReal("ShadowMaxProjDist", &fVal) == LT_OK)
	{
		sprintf(buf, "%f", fVal);
		g_pLTServer->SetGameConVar(szShadowMaxProjDist, buf);
	}
	if (g_pLTServer->GetPropReal("ShadowMinColorComponent", &fVal) == LT_OK)
	{
		sprintf(buf, "%f", fVal);
		g_pLTServer->SetGameConVar(szShadowMinColorComponent, buf);
	}
	if (g_pLTServer->GetPropReal("ShadowAlpha", &fVal) == LT_OK)
	{
		sprintf(buf, "%f", fVal);
		g_pLTServer->SetGameConVar(szShadowAlpha, buf);
	}

	// Read in WorldTimeSpeed.
    if (g_pLTServer->GetPropGeneric("WorldTimeSpeed", &genProp) == LT_OK)
	{
		m_fWorldTimeSpeed = genProp.m_Float;
        g_pLTServer->SetGameConVar("WorldTimeSpeed", genProp.m_String);
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

    if (g_pLTServer->GetPropGeneric( szAIMaxNumber, &genProp) == LT_OK)
	{
        g_pLTServer->SetGameConVar( szAIMaxNumber, genProp.m_String );
	}

    if (g_pLTServer->GetPropGeneric( szVehicleMaxNumber, &genProp) == LT_OK)
	{
        g_pLTServer->SetGameConVar( szVehicleMaxNumber, genProp.m_String );
	}

	if( g_pLTServer->GetPropGeneric( "LevelEndCmd", &genProp ) == LT_OK )
	{
		if( genProp.m_String[0] )
		{
			m_sLevelEndCmd = genProp.m_String;
		}
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	WorldProperties::OnTrigger()
//
//	PURPOSE:	Handle trigger messages
//
// --------------------------------------------------------------------------- //

bool WorldProperties::OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg)
{
	static CParsedMsg::CToken s_cTok_On("ON");
	static CParsedMsg::CToken s_cTok_Off("OFF");
	static CParsedMsg::CToken s_cTok_WorldTimeSpeed("WORLDTIMESPEED");
	static CParsedMsg::CToken s_cTok_WorldTime("WORLDTIME");
	static CParsedMsg::CToken s_cTok_FogColor(szFogColor);
	static CParsedMsg::CToken s_cTok_FogEnable(szFogEnable);
	static CParsedMsg::CToken s_cTok_FogNearZ(szFogNearZ);
	static CParsedMsg::CToken s_cTok_FogFarZ(szFogFarZ);
	static CParsedMsg::CToken s_cTok_SkyFogEnable(szSkyFogEnable);
	static CParsedMsg::CToken s_cTok_SkyFogFarZ(szSkyFogFarZ);
	static CParsedMsg::CToken s_cTok_SkyFogNearZ(szSkyFogNearZ);
	static CParsedMsg::CToken s_cTok_Transmission("transmission");
	static CParsedMsg::CToken s_cTok_DrawAllModelShadows("DrawAllModelShadows");
	static CParsedMsg::CToken s_cTok_ShadowMaxProjDist("ShadowMaxProjDist");
	static CParsedMsg::CToken s_cTok_ShadowMinColorComponent("ShadowMinColorComponent");
	static CParsedMsg::CToken s_cTok_ShadowAlpha("ShadowAlpha");
	static CParsedMsg::CToken s_cTok_NextRound("NextRound");
	static CParsedMsg::CToken s_cTok_RoundWon("RoundWon");

	//Flag indicating whether or not a message should be sent down to the client
	//indicating that we have changed the settings and that they need to update
	//their console commands
	LTBOOL bNotifyClient	= LTFALSE;

	if (cMsg.GetArg(0) == s_cTok_On)
	{
		char buf[20];
		sprintf(buf,"%.2f", m_fWorldTimeSpeed);

        g_pLTServer->SetGameConVar("WorldTimeSpeed", buf);
	}
	else if (cMsg.GetArg(0) == s_cTok_Off)
	{
        g_pLTServer->SetGameConVar("WorldTimeSpeed", "-1.0");
	}
	else if (cMsg.GetArg(0) == s_cTok_WorldTimeSpeed)
	{
		if (cMsg.GetArgCount() > 1)
		{
            m_fWorldTimeSpeed = (LTFLOAT) atof(cMsg.GetArg(1));
            g_pLTServer->SetGameConVar("WorldTimeSpeed", const_cast<char *>(cMsg.GetArg(1).c_str()));
		}
	}
	else if (cMsg.GetArg(0) == s_cTok_WorldTime)
	{
		if (cMsg.GetArgCount() > 1)
		{
            g_pLTServer->SetGameConVar("WorldTime", const_cast<char *>(cMsg.GetArg(1).c_str()));
		}
	}
	else if (cMsg.GetArg(0) == s_cTok_FogColor)
	{
		if (cMsg.GetArgCount() > 3)
		{
            g_pLTServer->SetGameConVar(szFogR, const_cast<char *>(cMsg.GetArg(1).c_str()));
            g_pLTServer->SetGameConVar(szFogG, const_cast<char *>(cMsg.GetArg(2).c_str()));
            g_pLTServer->SetGameConVar(szFogB, const_cast<char *>(cMsg.GetArg(3).c_str()));
            bNotifyClient = LTTRUE;
		}
	}
	else if (cMsg.GetArg(0) == s_cTok_FogEnable)
	{
		if (cMsg.GetArgCount() > 1)
		{
            g_pLTServer->SetGameConVar(szFogEnable, const_cast<char *>(cMsg.GetArg(1).c_str()));
            bNotifyClient = LTTRUE;
		}
	}
	else if (cMsg.GetArg(0) == s_cTok_FogNearZ)
	{
		if (cMsg.GetArgCount() > 1)
		{
            g_pLTServer->SetGameConVar(szFogNearZ, const_cast<char *>(cMsg.GetArg(1).c_str()));
            bNotifyClient = LTTRUE;
		}
	}
	else if (cMsg.GetArg(0) == s_cTok_FogFarZ)
	{
		if (cMsg.GetArgCount() > 1)
		{
            g_pLTServer->SetGameConVar(szFogFarZ, const_cast<char *>(cMsg.GetArg(1).c_str()));
            bNotifyClient = LTTRUE;
		}
	}
	else if (cMsg.GetArg(0) == s_cTok_SkyFogEnable)
	{
		if (cMsg.GetArgCount() > 1)
		{
            g_pLTServer->SetGameConVar(szSkyFogEnable, const_cast<char *>(cMsg.GetArg(1).c_str()));
            bNotifyClient = LTTRUE;
		}
	}
	else if (cMsg.GetArg(0) == s_cTok_SkyFogFarZ)
	{
		if (cMsg.GetArgCount() > 1)
		{
            g_pLTServer->SetGameConVar(szSkyFogFarZ, const_cast<char *>(cMsg.GetArg(1).c_str()));
            bNotifyClient = LTTRUE;
		}
	}
	else if (cMsg.GetArg(0) == s_cTok_SkyFogNearZ)
	{
		if (cMsg.GetArgCount() > 1)
		{
            g_pLTServer->SetGameConVar(szSkyFogNearZ, const_cast<char *>(cMsg.GetArg(1).c_str()));
            bNotifyClient = LTTRUE;
		}
	}
	else if (cMsg.GetArg(0) == s_cTok_Transmission)
	{
		HandleTransmissionMsg(cMsg);
	}
	else if(cMsg.GetArg(0) == s_cTok_DrawAllModelShadows)
	{
		if (cMsg.GetArgCount() > 1)
		{
			g_pLTServer->SetGameConVar(szSkyFogNearZ, const_cast<char *>(cMsg.GetArg(1).c_str()));
			bNotifyClient = LTTRUE;
		}
	}
	else if(cMsg.GetArg(0) == s_cTok_ShadowMaxProjDist)
	{
		if (cMsg.GetArgCount() > 1)
		{
			g_pLTServer->SetGameConVar(szShadowMaxProjDist, const_cast<char *>(cMsg.GetArg(1).c_str()));
			bNotifyClient = LTTRUE;
		}
	}
	else if(cMsg.GetArg(0) == s_cTok_ShadowMinColorComponent)
	{
		if (cMsg.GetArgCount() > 1)
		{
			g_pLTServer->SetGameConVar(szShadowMinColorComponent, const_cast<char *>(cMsg.GetArg(1).c_str()));
			bNotifyClient = LTTRUE;
		}
	}
	else if(cMsg.GetArg(0) == s_cTok_ShadowAlpha)
	{
		if (cMsg.GetArgCount() > 1)
		{
			g_pLTServer->SetGameConVar(szShadowAlpha, const_cast<char *>(cMsg.GetArg(1).c_str()));
			bNotifyClient = LTTRUE;
		}
	}
	else if(cMsg.GetArg(0) == s_cTok_NextRound)
	{
		if( !IsCoopMultiplayerGameType( ))
			g_pServerMissionMgr->NextRound( );
	}
	else if(cMsg.GetArg(0) == s_cTok_RoundWon)
	{
		if( !IsCoopMultiplayerGameType( ))
		{
			if( cMsg.GetArgCount() > 1 )
			{
				uint8 nTeamId = atoi( cMsg.GetArg( 1 ));
				if( nTeamId < MAX_TEAMS )
				{
					CTeamMgr::Instance( ).WonRound( nTeamId );
				}
			}
		}
	}
	else
		return GameBase::OnTrigger(hSender, cMsg);

	if (bNotifyClient)
	{
		SendClientsChangedValues();
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WorldProperties::SendClientsChangedValues
//
//	PURPOSE:	Send the the notification to the clients that the values
//				have changed on the server and that they need to sync their
//				console variables with the server's
//
// ----------------------------------------------------------------------- //

void WorldProperties::SendClientsChangedValues()
{
	SendEmptyClientMsg(MID_CHANGE_WORLDPROPERTIES, LTNULL);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WorldProperties::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void WorldProperties::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
	if (!pMsg) return;

	SAVE_DWORD(g_pVersionMgr->GetSaveVersion());

	// First save global game server shell stuff...(this only works because the
	// GameServerShell saves the WorldProperties object first)

	g_pGameServerShell->Save(pMsg, dwSaveFlags);

    SAVE_FLOAT(m_fWorldTimeSpeed);
	SAVE_CHARSTRING( m_sLevelEndCmd.c_str( ));
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WorldProperties::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void WorldProperties::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
	if (!pMsg) return;

	// This old version didn't save the version number at the top.  We need
	// to use an engine value to determine the version.  This is equivalent to 
	// checking for version 1.0.
	uint32 nSaveFileVersion = 0;
	g_pLTServer->GetSaveFileVersion( nSaveFileVersion );
	if( nSaveFileVersion == 2001 )
	{
		// First load global game server shell stuff...(this only works because the
		// GameServerShell saves the WorldProperties object first)
		g_pGameServerShell->Load(pMsg, dwLoadFlags);

		LOAD_FLOAT(m_fWorldTimeSpeed);

		uint32 nSaveVersion;
		LOAD_DWORD(nSaveVersion);
		g_pVersionMgr->SetCurrentSaveVersion( nSaveVersion );
		HSTRING hDummy = NULL;
		LOAD_HSTRING(hDummy);
		g_pLTServer->FreeString( hDummy );
		hDummy = NULL;
	}
	else
	{
		uint32 nSaveVersion;
		LOAD_DWORD(nSaveVersion);
		g_pVersionMgr->SetCurrentSaveVersion( nSaveVersion );

		// First load global game server shell stuff...(this only works because the
		// GameServerShell saves the WorldProperties object first)
		g_pGameServerShell->Load(pMsg, dwLoadFlags);

		LOAD_FLOAT(m_fWorldTimeSpeed);

		if( g_pVersionMgr->GetCurrentSaveVersion( ) < CVersionMgr::kSaveVersion__1_3 )
		{
		}
		else
		{
			char szTemp[256];
			LOAD_CHARSTRING( szTemp, ARRAY_LEN( szTemp ));
			m_sLevelEndCmd = szTemp;
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WorldProperties::HandleTransmissionMsg
//
//	PURPOSE:	process arguments of a transmission message
//
// ----------------------------------------------------------------------- //

void WorldProperties::HandleTransmissionMsg(const CParsedMsg &cMsg)
{
	GameType eGameType = g_pGameServerShell->GetGameType();

	if (cMsg.GetArgCount() > 1)
	{

        uint32 dwId = 0;
        uint32 nSound = 0;
        dwId = (uint32) atol(cMsg.GetArg(1));

		if (cMsg.GetArgCount() > 2)
		{
			nSound = (uint32) atol(cMsg.GetArg(2));
		}

		CAutoMessage cClientMsg;
		cClientMsg.Writeuint8(MID_PLAYER_TRANSMISSION);
		cClientMsg.Writeuint32(dwId);
	    cClientMsg.Writeuint32(nSound);
		g_pLTServer->SendToClient(cClientMsg.Read(), LTNULL, MESSAGE_GUARANTEED);

	}
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WorldProperties::SendLevelEndCmd
//
//	PURPOSE:	Sends the level end command.
//
// ----------------------------------------------------------------------- //

void WorldProperties::SendLevelEndCmd( )
{
	if( !g_pCmdMgr->IsValidCmd( m_sLevelEndCmd.c_str( )))
		return;

	g_pCmdMgr->Process( m_sLevelEndCmd.c_str( ), m_hObject, m_hObject );

	// Forget level end command, now that we've sent it.
	m_sLevelEndCmd.erase( );
}