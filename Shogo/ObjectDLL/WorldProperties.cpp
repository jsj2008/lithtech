#include "cpp_engineobjects_de.h"
#include <stdio.h>

class WorldProperties : public BaseClass
{
	public:

	~WorldProperties( );

	protected :

		DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData);
};

BEGIN_CLASS(WorldProperties)
	ADD_STRINGPROP(MusicDirectory, "")
	ADD_STRINGPROP(InstrumentFiles, "")
	ADD_STRINGPROP(AmbientList, "")
	ADD_STRINGPROP(CruisingList, "")
	ADD_STRINGPROP(HarddrivingList, "")
	ADD_STRINGPROP(CDTrack,"")
	ADD_VECTORPROP_VAL_FLAG(Wind, 0.0f, 0.0f, 0.0f, 0)
	ADD_BOOLPROP(EnableFog, DFALSE)
	ADD_COLORPROP(FogColor, 127.0f, 127.0f, 127.0f)
	ADD_REALPROP(FogNearZ, 1.0f)
	ADD_REALPROP(FogFarZ, 5000.0f)	
	ADD_REALPROP(FarZ, 5000.0f)
	ADD_BOOLPROP(SkyFog, DFALSE)
	ADD_REALPROP(SkyFogNearZ, 100.0f)
	ADD_REALPROP(SkyFogFarZ, 1000.0f)	
	ADD_STRINGPROP(EnvironmentMap, "Textures\\Chrome.dtx")
	PROP_DEFINEGROUP(SkyPanning, PF_GROUP1)
		ADD_STRINGPROP_FLAG(PanSkyTexture, "Textures\\SkyPan.dtx", PF_GROUP1)
		ADD_BOOLPROP_FLAG(PanSky, 0, PF_GROUP1)
		ADD_REALPROP_FLAG(PanSkyOffsetX, 10.0f, PF_GROUP1)
		ADD_REALPROP_FLAG(PanSkyOffsetZ, 10.0f, PF_GROUP1)
		ADD_REALPROP_FLAG(PanSkyScaleX, 10.0f, PF_GROUP1)
		ADD_REALPROP_FLAG(PanSkyScaleZ, 10.0f, PF_GROUP1)
	ADD_COLORPROP(LightScale, 255.0f, 255.0f, 255.0f)
	ADD_STRINGPROP(SoftSky,"textures\\environmentmaps\\clouds\\clouds")

END_CLASS_DEFAULT_FLAGS(WorldProperties, BaseClass, NULL, NULL, CF_ALWAYSLOAD)

static char szMusicDirectory[] = "MusicDirectory";
static char szInstrumentFiles[] = "InstrumentFiles";
static char szAmbientList[] = "AmbientList";
static char szCruisingList[] = "CruisingList";
static char szHarddrivingList[] = "HarddrivingList";
static char szCDTrack[] = "CDTrack";
static char szWindX[] = "WindX";
static char szWindY[] = "WindY";
static char szWindZ[] = "WindZ";
static char szEnableFog[] = "EnableFog";
static char szFogR[] = "FogR";
static char szFogG[] = "FogG";
static char szFogB[] = "FogB";
static char szFogNearZ[] = "FogNearZ";
static char szFogFarZ[] = "FogFarZ";
static char szSkyFog[] = "SkyFog";
static char szSkyFogNearZ[] = "SkyFogNearZ";
static char szSkyFogFarZ[] = "SkyFogFarZ";
static char szFarZ[] = "FarZ";
static char szPanSky[] = "PanSky";
static char szPanSkyTexture[] = "PanSkyTexture";
static char szPanSkyOffsetX[] = "PanSkyOffsetX";
static char szPanSkyOffsetZ[] = "PanSkyOffsetZ";
static char szPanSkyScaleX[] = "PanSkyScaleX";
static char szPanSkyScaleZ[] = "PanSkyScaleZ";
static char szEnvironmentMap[] = "EnvironmentMap";
static char szLightScaleR[] = "LightScaleR";
static char szLightScaleG[] = "LightScaleG";
static char szLightScaleB[] = "LightScaleB";
static char szSoftSky[] = "SoftSky";

WorldProperties::~WorldProperties( )
{
	g_pServerDE->SetGameConVar( szMusicDirectory, "" );
	g_pServerDE->SetGameConVar( szInstrumentFiles, "" );
	g_pServerDE->SetGameConVar( szAmbientList, "" );
	g_pServerDE->SetGameConVar( szCruisingList, "" );
	g_pServerDE->SetGameConVar( szHarddrivingList, "" );
	g_pServerDE->SetGameConVar( szCDTrack, "" );
	g_pServerDE->SetGameConVar( szWindX, "" );
	g_pServerDE->SetGameConVar( szWindY, "" );
	g_pServerDE->SetGameConVar( szWindZ, "" );
	g_pServerDE->SetGameConVar( szEnableFog, "" );
	g_pServerDE->SetGameConVar( szFogR, "" );
	g_pServerDE->SetGameConVar( szFogG, "" );
	g_pServerDE->SetGameConVar( szFogB, "" );
	g_pServerDE->SetGameConVar( szFogNearZ, "" );
	g_pServerDE->SetGameConVar( szFogFarZ, "" );
	g_pServerDE->SetGameConVar( szSkyFog, "" );
	g_pServerDE->SetGameConVar( szSkyFogNearZ, "" );
	g_pServerDE->SetGameConVar( szSkyFogFarZ, "" );
		
	// {MD 9/13/98}
	//g_pServerDE->SetGameConVar( szFarZ, "" );
	
	g_pServerDE->SetGameConVar( szPanSky, "" );
	g_pServerDE->SetGameConVar( szPanSkyTexture, "" );
	g_pServerDE->SetGameConVar( szPanSkyOffsetX, "" );
	g_pServerDE->SetGameConVar( szPanSkyOffsetZ, "" );
	g_pServerDE->SetGameConVar( szPanSkyScaleX, "" );
	g_pServerDE->SetGameConVar( szPanSkyScaleZ, "" );
	g_pServerDE->SetGameConVar( szEnvironmentMap, "" );
	g_pServerDE->SetGameConVar( szLightScaleR, "" );
	g_pServerDE->SetGameConVar( szLightScaleG, "" );
	g_pServerDE->SetGameConVar( szLightScaleB, "" );
	g_pServerDE->SetGameConVar( szSoftSky, "" );
}


DDWORD WorldProperties::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData)
{
	char buf[501];

	switch(messageID)
	{
		case MID_PRECREATE:
		{
			buf[0] = '\0';
			if (g_pServerDE->GetPropString(szMusicDirectory, buf, 500) == DE_OK)
			{
				if (buf[0]) g_pServerDE->SetGameConVar(szMusicDirectory, buf);
			}

			buf[0] = '\0';
			if (g_pServerDE->GetPropString(szInstrumentFiles, buf, 500) == DE_OK)
			{
				if (buf[0]) g_pServerDE->SetGameConVar(szInstrumentFiles, buf);
			}

			buf[0] = '\0';
			if (g_pServerDE->GetPropString(szAmbientList, buf, 500) == DE_OK)
			{
				if (buf[0]) g_pServerDE->SetGameConVar(szAmbientList, buf);
			}

			buf[0] = '\0';
			if (g_pServerDE->GetPropString(szCruisingList, buf, 500) == DE_OK)
			{
				if (buf[0]) g_pServerDE->SetGameConVar(szCruisingList, buf);
			}

			buf[0] = '\0';
			if (g_pServerDE->GetPropString(szHarddrivingList, buf, 500) == DE_OK)
			{
				if (buf[0]) g_pServerDE->SetGameConVar(szHarddrivingList, buf);
			}

			buf[0] = '\0';
			if (g_pServerDE->GetPropString(szCDTrack, buf, 500) == DE_OK)
			{
				if (buf[0]) g_pServerDE->SetGameConVar(szCDTrack, buf);
			}

			DVector vVec;
			if (g_pServerDE->GetPropVector("Wind", &vVec) == DE_OK)
			{
				sprintf(buf, "%f", vVec.x);
				g_pServerDE->SetGameConVar( szWindX, buf );

				sprintf(buf, "%f", vVec.y);
				g_pServerDE->SetGameConVar( szWindY, buf );

				sprintf(buf, "%f", vVec.z);
				g_pServerDE->SetGameConVar( szWindZ, buf );
			}

			DBOOL bVal;
			if (g_pServerDE->GetPropBool(szEnableFog, &bVal) == DE_OK)
			{
				sprintf(buf, "%d", bVal);
				g_pServerDE->SetGameConVar(szEnableFog, buf);
			}

			DFLOAT fVal;
			if (g_pServerDE->GetPropReal(szFogNearZ, &fVal) == DE_OK)
			{
				sprintf(buf, "%f", fVal);
				g_pServerDE->SetGameConVar(szFogNearZ, buf);
			}

			if (g_pServerDE->GetPropReal(szFogFarZ, &fVal) == DE_OK)
			{
				sprintf(buf, "%f", fVal);
				g_pServerDE->SetGameConVar(szFogFarZ, buf);
			}

			if (g_pServerDE->GetPropVector("FogColor", &vVec) == DE_OK)
			{
				sprintf(buf, "%f", vVec.x);
				g_pServerDE->SetGameConVar( szFogR, buf );

				sprintf(buf, "%f", vVec.y);
				g_pServerDE->SetGameConVar( szFogG, buf );

				sprintf(buf, "%f", vVec.z);
				g_pServerDE->SetGameConVar( szFogB, buf );
			}

			if (g_pServerDE->GetPropVector("LightScale", &vVec) == DE_OK)
			{
				sprintf(buf, "%f", vVec.x);
				g_pServerDE->SetGameConVar( szLightScaleR, buf );

				sprintf(buf, "%f", vVec.y);
				g_pServerDE->SetGameConVar( szLightScaleG, buf );

				sprintf(buf, "%f", vVec.z);
				g_pServerDE->SetGameConVar( szLightScaleB, buf );
			}

			if (g_pServerDE->GetPropBool(szSkyFog, &bVal) == DE_OK)
			{
				sprintf(buf, "%d", bVal);
				g_pServerDE->SetGameConVar(szSkyFog, buf);
			}

			if (g_pServerDE->GetPropReal(szSkyFogNearZ, &fVal) == DE_OK)
			{
				sprintf(buf, "%f", fVal);
				g_pServerDE->SetGameConVar(szSkyFogNearZ, buf);
			}

			if (g_pServerDE->GetPropReal(szSkyFogFarZ, &fVal) == DE_OK)
			{
				sprintf(buf, "%f", fVal);
				g_pServerDE->SetGameConVar(szSkyFogFarZ, buf);
			}

			if (g_pServerDE->GetPropReal(szFarZ, &fVal) == DE_OK)
			{
				sprintf(buf, "%f", fVal);
				g_pServerDE->SetGameConVar(szFarZ, buf);
			}

			if (g_pServerDE->GetPropBool(szPanSky, &bVal) == DE_OK)
			{
				sprintf(buf, "%d", bVal);
				g_pServerDE->SetGameConVar(szPanSky, buf);
			}

			buf[0] = '\0';
			if (g_pServerDE->GetPropString(szEnvironmentMap, buf, 500) == DE_OK)
			{
				if (buf[0]) g_pServerDE->SetGameConVar(szEnvironmentMap, buf);
			}

			buf[0] = '\0';
			if (g_pServerDE->GetPropString(szPanSkyTexture, buf, 500) == DE_OK)
			{
				if (buf[0]) g_pServerDE->SetGameConVar(szPanSkyTexture, buf);
			}

			if (g_pServerDE->GetPropReal(szPanSkyOffsetX, &fVal) == DE_OK)
			{
				sprintf(buf, "%f", fVal);
				g_pServerDE->SetGameConVar(szPanSkyOffsetX, buf);
			}

			if (g_pServerDE->GetPropReal(szPanSkyOffsetZ, &fVal) == DE_OK)
			{
				sprintf(buf, "%f", fVal);
				g_pServerDE->SetGameConVar(szPanSkyOffsetZ, buf);
			}

			if (g_pServerDE->GetPropReal(szPanSkyScaleX, &fVal) == DE_OK)
			{
				sprintf(buf, "%f", fVal);
				g_pServerDE->SetGameConVar(szPanSkyScaleX, buf);
			}

			if (g_pServerDE->GetPropReal(szPanSkyScaleZ, &fVal) == DE_OK)
			{
				sprintf(buf, "%f", fVal);
				g_pServerDE->SetGameConVar(szPanSkyScaleZ, buf);
			}

			buf[0] = '\0';
			if (g_pServerDE->GetPropString( szSoftSky, buf, 500 ) == DE_OK && buf[0] )
				g_pServerDE->SetGameConVar( szSoftSky, buf );
			else
				g_pServerDE->SetGameConVar( szSoftSky, "textures\\environmentmaps\\clouds\\clouds" );

			break;

		}
		case MID_INITIALUPDATE:
		{
			g_pServerDE->SetObjectState(m_hObject, OBJSTATE_INACTIVE);
			break;
		}

		default : break;
	}

	return BaseClass::EngineMessageFn(messageID, pData, lData);
}
